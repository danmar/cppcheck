/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "threadexecutor.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include <iostream>
#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif
#ifdef THREADING_MODEL_FORK
#include <algorithm>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <sstream>
#endif
#ifdef THREADING_MODEL_WIN
#include <process.h>
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <errno.h>
#endif

// required for FD_ZERO
using std::memset;

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : _files(files), _settings(settings), _errorLogger(errorLogger), _fileCount(0)
{
#if defined(THREADING_MODEL_FORK)
    _wpipe = 0;
#elif defined(THREADING_MODEL_WIN)
    _processedFiles = 0;
    _totalFiles = 0;
    _processedSize = 0;
    _totalFileSize = 0;
#endif
}

ThreadExecutor::~ThreadExecutor()
{
    //dtor
}


///////////////////////////////////////////////////////////////////////////////
////// This code is for platforms that support fork() only ////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(THREADING_MODEL_FORK)

void ThreadExecutor::addFileContent(const std::string &path, const std::string &content)
{
    _fileContents[ path ] = content;
}

int ThreadExecutor::handleRead(int rpipe, unsigned int &result)
{
    char type = 0;
    if (read(rpipe, &type, 1) <= 0) {
        if (errno == EAGAIN)
            return 0;

        return -1;
    }

    if (type != REPORT_OUT && type != REPORT_ERROR && type != REPORT_INFO && type != CHILD_END) {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }

    unsigned int len = 0;
    if (read(rpipe, &len, sizeof(len)) <= 0) {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }

    char *buf = new char[len];
    if (read(rpipe, buf, len) <= 0) {
        std::cerr << "#### You found a bug from cppcheck.\nThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }

    if (type == REPORT_OUT) {
        _errorLogger.reportOut(buf);
    } else if (type == REPORT_ERROR || type == REPORT_INFO) {
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
                if (type == REPORT_ERROR)
                    _errorLogger.reportErr(msg);
                else
                    _errorLogger.reportInfo(msg);
            }
        }
    } else if (type == CHILD_END) {
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

bool ThreadExecutor::checkLoadAverage(size_t nchildren)
{
#if defined(__CYGWIN__) || defined(__QNX__)  // getloadavg() is unsupported on Cygwin, Qnx.
    return true;
#else
    if (!nchildren || !_settings._loadAverage) {
        return true;
    }

    double sample(0);
    if (getloadavg(&sample, 1) != 1) {
        // disable load average checking on getloadavg error
        return true;
    } else if (sample < _settings._loadAverage) {
        return true;
    }
    return false;
#endif
}

unsigned int ThreadExecutor::check()
{
    _fileCount = 0;
    unsigned int result = 0;

    std::size_t totalfilesize = 0;
    for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
        totalfilesize += i->second;
    }

    std::list<int> rpipes;
    std::map<pid_t, std::string> childFile;
    std::map<int, std::string> pipeFile;
    std::size_t processedsize = 0;
    std::map<std::string, std::size_t>::const_iterator i = _files.begin();
    for (;;) {
        // Start a new child
        size_t nchildren = rpipes.size();
        if (i != _files.end() && nchildren < _settings._jobs && checkLoadAverage(nchildren)) {
            int pipes[2];
            if (pipe(pipes) == -1) {
                std::cerr << "pipe() failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            int flags = 0;
            if ((flags = fcntl(pipes[0], F_GETFL, 0)) < 0) {
                std::cerr << "fcntl(F_GETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (fcntl(pipes[0], F_SETFL, flags | O_NONBLOCK) < 0) {
                std::cerr << "fcntl(F_SETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid < 0) {
                // Error
                std::cerr << "Failed to create child process: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            } else if (pid == 0) {
                close(pipes[0]);
                _wpipe = pipes[1];

                CppCheck fileChecker(*this, false);
                fileChecker.settings() = _settings;
                unsigned int resultOfCheck = 0;

                if (!_fileContents.empty() && _fileContents.find(i->first) != _fileContents.end()) {
                    // File content was given as a string
                    resultOfCheck = fileChecker.check(i->first, _fileContents[ i->first ]);
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(i->first);
                }

                std::ostringstream oss;
                oss << resultOfCheck;
                writeToPipe(CHILD_END, oss.str());
                std::exit(0);
            }

            close(pipes[1]);
            rpipes.push_back(pipes[0]);
            childFile[pid] = i->first;
            pipeFile[pipes[0]] = i->first;

            ++i;
        } else if (!rpipes.empty()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            for (std::list<int>::const_iterator rp = rpipes.begin(); rp != rpipes.end(); ++rp)
                FD_SET(*rp, &rfds);
            struct timeval tv; // for every second polling of load average condition
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int r = select(*std::max_element(rpipes.begin(), rpipes.end()) + 1, &rfds, NULL, NULL, &tv);

            if (r > 0) {
                std::list<int>::iterator rp = rpipes.begin();
                while (rp != rpipes.end()) {
                    if (FD_ISSET(*rp, &rfds)) {
                        int readRes = handleRead(*rp, result);
                        if (readRes == -1) {
                            std::size_t size = 0;
                            std::map<int, std::string>::iterator p = pipeFile.find(*rp);
                            if (p != pipeFile.end()) {
                                std::string name = p->second;
                                pipeFile.erase(p);
                                std::map<std::string, std::size_t>::const_iterator fs = _files.find(name);
                                if (fs != _files.end()) {
                                    size = fs->second;
                                }
                            }

                            _fileCount++;
                            processedsize += size;
                            if (!_settings._errorsOnly)
                                CppCheckExecutor::reportStatus(_fileCount, _files.size(), processedsize, totalfilesize);

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

void ThreadExecutor::writeToPipe(PipeSignal type, const std::string &data)
{
    unsigned int len = static_cast<unsigned int>(data.length() + 1);
    char *out = new char[ len + 1 + sizeof(len)];
    out[0] = static_cast<char>(type);
    std::memcpy(&(out[1]), &len, sizeof(len));
    std::memcpy(&(out[1+sizeof(len)]), data.c_str(), len);
    if (write(_wpipe, out, len + 1 + sizeof(len)) <= 0) {
        delete [] out;
        out = 0;
        std::cerr << "#### ThreadExecutor::writeToPipe, Failed to write to pipe" << std::endl;
        std::exit(0);
    }

    delete [] out;
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    writeToPipe(REPORT_OUT, outmsg);
}

void ThreadExecutor::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    writeToPipe(REPORT_ERROR, msg.serialize());
}

void ThreadExecutor::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    writeToPipe(REPORT_INFO, msg.serialize());
}

#elif defined(THREADING_MODEL_WIN)

void ThreadExecutor::addFileContent(const std::string &path, const std::string &content)
{
    _fileContents[path] = content;
}

unsigned int ThreadExecutor::check()
{
    HANDLE *threadHandles = new HANDLE[_settings._jobs];

    _itNextFile = _files.begin();

    _processedFiles = 0;
    _processedSize = 0;
    _totalFiles = _files.size();
    _totalFileSize = 0;
    for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
        _totalFileSize += i->second;
    }

    InitializeCriticalSection(&_fileSync);
    InitializeCriticalSection(&_errorSync);
    InitializeCriticalSection(&_reportSync);

    for (unsigned int i = 0; i < _settings._jobs; ++i) {
        threadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadProc, this, 0, NULL);
        if (!threadHandles[i]) {
            std::cerr << "#### .\nThreadExecutor::check error, errno :" << errno << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    DWORD waitResult = WaitForMultipleObjects(_settings._jobs, threadHandles, TRUE, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        if (waitResult == WAIT_FAILED) {
            std::cerr << "#### .\nThreadExecutor::check wait failed, result: " << waitResult << " error: " << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cerr << "#### .\nThreadExecutor::check wait failed, result: " << waitResult << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    unsigned int result = 0;
    for (unsigned int i = 0; i < _settings._jobs; ++i) {
        DWORD exitCode;

        if (!GetExitCodeThread(threadHandles[i], &exitCode)) {
            std::cerr << "#### .\nThreadExecutor::check get exit code failed, error:" << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }

        result += exitCode;

        if (!CloseHandle(threadHandles[i])) {
            std::cerr << "#### .\nThreadExecutor::check close handle failed, error:" << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    DeleteCriticalSection(&_fileSync);
    DeleteCriticalSection(&_errorSync);
    DeleteCriticalSection(&_reportSync);

    delete[] threadHandles;

    return result;
}

unsigned int __stdcall ThreadExecutor::threadProc(void *args)
{
    unsigned int result = 0;

    ThreadExecutor *threadExecutor = static_cast<ThreadExecutor*>(args);
    std::map<std::string, std::size_t>::const_iterator &it = threadExecutor->_itNextFile;

    // guard static members of CppCheck against concurrent access
    EnterCriticalSection(&threadExecutor->_fileSync);

    CppCheck fileChecker(*threadExecutor, false);
    fileChecker.settings() = threadExecutor->_settings;

    for (;;) {
        if (it == threadExecutor->_files.end()) {
            LeaveCriticalSection(&threadExecutor->_fileSync);
            break;

        }
        const std::string &file = it->first;
        const std::size_t fileSize = it->second;
        ++it;

        LeaveCriticalSection(&threadExecutor->_fileSync);

        std::map<std::string, std::string>::const_iterator fileContent = threadExecutor->_fileContents.find(file);
        if (fileContent != threadExecutor->_fileContents.end()) {
            // File content was given as a string
            result += fileChecker.check(file, fileContent->second);
        } else {
            // Read file from a file
            result += fileChecker.check(file);
        }

        EnterCriticalSection(&threadExecutor->_fileSync);

        threadExecutor->_processedSize += fileSize;
        threadExecutor->_processedFiles++;
        if (!threadExecutor->_settings._errorsOnly) {
            EnterCriticalSection(&threadExecutor->_reportSync);
            CppCheckExecutor::reportStatus(threadExecutor->_processedFiles, threadExecutor->_totalFiles, threadExecutor->_processedSize, threadExecutor->_totalFileSize);
            LeaveCriticalSection(&threadExecutor->_reportSync);
        }
    }
    return result;
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    EnterCriticalSection(&_reportSync);

    _errorLogger.reportOut(outmsg);

    LeaveCriticalSection(&_reportSync);
}
void ThreadExecutor::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    report(msg, REPORT_ERROR);
}

void ThreadExecutor::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    report(msg, REPORT_INFO);
}

void ThreadExecutor::report(const ErrorLogger::ErrorMessage &msg, MessageType msgType)
{
    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty()) {
        file = msg._callStack.back().getfile(false);
        line = msg._callStack.back().line;
    }

    if (_settings.nomsg.isSuppressed(msg._id, file, line))
        return;

    // Alert only about unique errors
    bool reportError = false;
    std::string errmsg = msg.toString(_settings._verbose);

    EnterCriticalSection(&_errorSync);
    if (std::find(_errorList.begin(), _errorList.end(), errmsg) == _errorList.end()) {
        _errorList.push_back(errmsg);
        reportError = true;
    }
    LeaveCriticalSection(&_errorSync);

    if (reportError) {
        EnterCriticalSection(&_reportSync);

        switch (msgType) {
        case REPORT_ERROR:
            _errorLogger.reportErr(msg);
            break;
        case REPORT_INFO:
            _errorLogger.reportInfo(msg);
            break;
        }

        LeaveCriticalSection(&_reportSync);
    }
}

#else

void ThreadExecutor::addFileContent(const std::string &/*path*/, const std::string &/*content*/)
{

}

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

void ThreadExecutor::reportInfo(const ErrorLogger::ErrorMessage &/*msg*/)
{

}

#endif
