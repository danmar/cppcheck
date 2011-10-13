/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cppcheckexecutor.h"
#include "threadexecutor.h"
#include "cppcheck.h"
#include <iostream>
#include <algorithm>
#ifdef THREADING_MODEL_FORK
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <time.h>
#endif

ThreadExecutor::ThreadExecutor(const std::vector<std::string> &filenames, const std::map<std::string, long> &filesizes, Settings &settings, ErrorLogger &errorLogger)
    : _filenames(filenames), _filesizes(filesizes), _settings(settings), _errorLogger(errorLogger), _fileCount(0)
{
#ifdef THREADING_MODEL_FORK
    _wpipe = 0;
#endif
}

ThreadExecutor::~ThreadExecutor()
{
    //dtor
}

void ThreadExecutor::addFileContent(const std::string &path, const std::string &content)
{
    _fileContents[ path ] = content;
}

///////////////////////////////////////////////////////////////////////////////
////// This code is for platforms that support fork() only ////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef THREADING_MODEL_FORK

int ThreadExecutor::handleRead(int rpipe, unsigned int &result)
{
    char type = 0;
    if (read(rpipe, &type, 1) <= 0) {
        if (errno == EAGAIN)
            return 0;

        return -1;
    }

    if (type != '1' && type != '2' && type != '3') {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        exit(0);
    }

    unsigned int len = 0;
    if (read(rpipe, &len, sizeof(len)) <= 0) {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        exit(0);
    }

    char *buf = new char[len];
    if (read(rpipe, buf, len) <= 0) {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        exit(0);
    }

    if (type == '1') {
        _errorLogger.reportOut(buf);
    } else if (type == '2') {
        ErrorLogger::ErrorMessage msg;
        msg.deserialize(buf);

        std::string file;
        unsigned int line(0);
        if (!msg._callStack.empty()) {
            file = msg._callStack.back().getfile(false);
            line = msg._callStack.back().line;
        }

        if (!_settings.nomsg.isSuppressed(msg._id, file, line)) {
            // Alert only about unique errors
            std::string errmsg = msg.toString(_settings._verbose);
            if (std::find(_errorList.begin(), _errorList.end(), errmsg) == _errorList.end()) {
                _errorList.push_back(errmsg);
                _errorLogger.reportErr(msg);
            }
        }
    } else if (type == '3') {
        std::istringstream iss(buf);
        unsigned int fileResult = 0;
        iss >> fileResult;
        result += fileResult;
        delete [] buf;
        return -1;
    }

    delete [] buf;
    return 1;
}

unsigned int ThreadExecutor::check()
{
    _fileCount = 0;
    unsigned int result = 0;

    long totalfilesize = 0;
    for (std::map<std::string, long>::const_iterator i = _filesizes.begin(); i != _filesizes.end(); ++i) {
        totalfilesize += i->second;
    }

    std::list<int> rpipes;
    std::map<pid_t, std::string> childFile;
    std::map<int, std::string> pipeFile;
    long processedsize = 0;
    unsigned int i = 0;
    while (true) {
        // Start a new child
        if (i < _filenames.size() && rpipes.size() < _settings._jobs) {
            int pipes[2];
            if (pipe(pipes) == -1) {
                perror("pipe");
                exit(1);
            }

            int flags = 0;
            if ((flags = fcntl(pipes[0], F_GETFL, 0)) < 0) {
                perror("fcntl");
                exit(1);
            }

            if (fcntl(pipes[0], F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("fcntl");
                exit(1);
            }

            pid_t pid = fork();
            if (pid < 0) {
                // Error
                std::cerr << "Failed to create child process" << std::endl;
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                close(pipes[0]);
                _wpipe = pipes[1];

                CppCheck fileChecker(*this, false);
                fileChecker.settings(_settings);
                unsigned int resultOfCheck = 0;

                if (_fileContents.size() > 0 && _fileContents.find(_filenames[i]) != _fileContents.end()) {
                    // File content was given as a string
                    resultOfCheck = fileChecker.check(_filenames[i], _fileContents[ _filenames[i] ]);
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(_filenames[i]);
                }

                std::ostringstream oss;
                oss << resultOfCheck;
                writeToPipe('3', oss.str());
                exit(0);
            }

            close(pipes[1]);
            rpipes.push_back(pipes[0]);
            childFile[pid] = _filenames[i];
            pipeFile[pipes[0]] = _filenames[i];

            ++i;
        } else if (!rpipes.empty()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            for (std::list<int>::const_iterator rp = rpipes.begin(); rp != rpipes.end(); ++rp)
                FD_SET(*rp, &rfds);

            int r = select(*std::max_element(rpipes.begin(), rpipes.end()) + 1, &rfds, NULL, NULL, NULL);

            if (r > 0) {
                std::list<int>::iterator rp = rpipes.begin();
                while (rp != rpipes.end()) {
                    if (FD_ISSET(*rp, &rfds)) {
                        int readRes = handleRead(*rp, result);
                        if (readRes == -1) {
                            long size = 0;
                            std::map<int, std::string>::iterator p = pipeFile.find(*rp);
                            if (p != pipeFile.end()) {
                                std::string name = p->second;
                                pipeFile.erase(p);
                                std::map<std::string, long>::const_iterator fs = _filesizes.find(name);
                                if (fs != _filesizes.end()) {
                                    size = fs->second;
                                }
                            }

                            _fileCount++;
                            processedsize += size;
                            if (!_settings._errorsOnly)
                                CppCheckExecutor::reportStatus(_fileCount, _filenames.size(), processedsize, totalfilesize);

                            close(*rp);
                            rp = rpipes.erase(rp);
                        } else
                            ++rp;
                    } else
                        ++rp;
                }
            }

            int stat = 0;
            pid_t child = waitpid(0, &stat, WNOHANG);
            if (child > 0) {
                std::string childname;
                std::map<pid_t, std::string>::iterator c = childFile.find(child);
                if (c != childFile.end()) {
                    childname = c->second;
                    childFile.erase(c);
                }

                if (WIFSIGNALED(stat)) {
                    std::ostringstream oss;
                    oss << "Internal error: Child process crashed with signal " << WTERMSIG(stat);

                    std::list<ErrorLogger::ErrorMessage::FileLocation> locations;
                    locations.push_back(ErrorLogger::ErrorMessage::FileLocation(childname, 0));
                    const ErrorLogger::ErrorMessage errmsg(locations,
                                                           Severity::error,
                                                           oss.str(),
                                                           "cppcheckError",
                                                           false);

                    if (!_settings.nomsg.isSuppressed(errmsg._id, childname, 0))
                        _errorLogger.reportErr(errmsg);
                }
            }
        } else {
            // All done
            break;
        }
    }


    return result;
}

void ThreadExecutor::writeToPipe(char type, const std::string &data)
{
    unsigned int len = data.length() + 1;
    char *out = new char[ len + 1 + sizeof(len)];
    out[0] = type;
    std::memcpy(&(out[1]), &len, sizeof(len));
    std::memcpy(&(out[1+sizeof(len)]), data.c_str(), len);
    if (write(_wpipe, out, len + 1 + sizeof(len)) <= 0) {
        delete [] out;
        out = 0;
        std::cerr << "#### ThreadExecutor::writeToPipe, Failed to write to pipe" << std::endl;
        exit(0);
    }

    delete [] out;
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    writeToPipe('1', outmsg);
}

void ThreadExecutor::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    writeToPipe('2', msg.serialize());
}

#else
unsigned int ThreadExecutor::check()
{
    return 0;
}

void ThreadExecutor::reportOut(const std::string &/*outmsg*/)
{

}
void ThreadExecutor::reportErr(const ErrorLogger::ErrorMessage &/*msg*/)
{

}

#endif
