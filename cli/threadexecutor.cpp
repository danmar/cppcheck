/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "importproject.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif
#ifdef THREADING_MODEL_FORK
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#include <sys/select.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#ifdef THREADING_MODEL_WIN
#include <process.h>
#include <windows.h>
#endif

// required for FD_ZERO
using std::memset;

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : mFiles(files), mSettings(settings), mErrorLogger(errorLogger), mFileCount(0)
      // Not initialized mFileSync, mErrorSync, mReportSync
{
#if defined(THREADING_MODEL_FORK)
    mWpipe = 0;
#elif defined(THREADING_MODEL_WIN)
    mProcessedFiles = 0;
    mTotalFiles = 0;
    mProcessedSize = 0;
    mTotalFileSize = 0;
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
    mFileContents[ path ] = content;
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
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }

    unsigned int len = 0;
    if (read(rpipe, &len, sizeof(len)) <= 0) {
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }

    // Don't rely on incoming data being null-terminated.
    // Allocate +1 element and null-terminate the buffer.
    char *buf = new char[len + 1];
    const ssize_t readIntoBuf = read(rpipe, buf, len);
    if (readIntoBuf <= 0) {
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(0);
    }
    buf[readIntoBuf] = 0;

    if (type == REPORT_OUT) {
        mErrorLogger.reportOut(buf);
    } else if (type == REPORT_ERROR || type == REPORT_INFO) {
        ErrorMessage msg;
        msg.deserialize(buf);

        if (!mSettings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage())) {
            // Alert only about unique errors
            std::string errmsg = msg.toString(mSettings.verbose);
            if (std::find(mErrorList.begin(), mErrorList.end(), errmsg) == mErrorList.end()) {
                mErrorList.emplace_back(errmsg);
                if (type == REPORT_ERROR)
                    mErrorLogger.reportErr(msg);
                else
                    mErrorLogger.reportInfo(msg);
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
    if (!nchildren || !mSettings.loadAverage) {
        return true;
    }

    double sample(0);
    if (getloadavg(&sample, 1) != 1) {
        // disable load average checking on getloadavg error
        return true;
    } else if (sample < mSettings.loadAverage) {
        return true;
    }
    return false;
#endif
}

unsigned int ThreadExecutor::check()
{
    mFileCount = 0;
    unsigned int result = 0;

    std::size_t totalfilesize = 0;
    for (std::map<std::string, std::size_t>::const_iterator i = mFiles.begin(); i != mFiles.end(); ++i) {
        totalfilesize += i->second;
    }

    std::list<int> rpipes;
    std::map<pid_t, std::string> childFile;
    std::map<int, std::string> pipeFile;
    std::size_t processedsize = 0;
    std::map<std::string, std::size_t>::const_iterator iFile = mFiles.begin();
    std::list<ImportProject::FileSettings>::const_iterator iFileSettings = mSettings.project.fileSettings.begin();
    for (;;) {
        // Start a new child
        size_t nchildren = childFile.size();
        if ((iFile != mFiles.end() || iFileSettings != mSettings.project.fileSettings.end()) && nchildren < mSettings.jobs && checkLoadAverage(nchildren)) {
            int pipes[2];
            if (pipe(pipes) == -1) {
                std::cerr << "#### ThreadExecutor::check, pipe() failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            int flags = 0;
            if ((flags = fcntl(pipes[0], F_GETFL, 0)) < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_GETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (fcntl(pipes[0], F_SETFL, flags | O_NONBLOCK) < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_SETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid < 0) {
                // Error
                std::cerr << "#### ThreadExecutor::check, Failed to create child process: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            } else if (pid == 0) {
#if defined(__linux__)
                prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif
                close(pipes[0]);
                mWpipe = pipes[1];

                CppCheck fileChecker(*this, false, CppCheckExecutor::executeCommand);
                fileChecker.settings() = mSettings;
                unsigned int resultOfCheck = 0;

                if (iFileSettings != mSettings.project.fileSettings.end()) {
                    resultOfCheck = fileChecker.check(*iFileSettings);
                } else if (!mFileContents.empty() && mFileContents.find(iFile->first) != mFileContents.end()) {
                    // File content was given as a string
                    resultOfCheck = fileChecker.check(iFile->first, mFileContents[ iFile->first ]);
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(iFile->first);
                }

                std::ostringstream oss;
                oss << resultOfCheck;
                writeToPipe(CHILD_END, oss.str());
                std::exit(0);
            }

            close(pipes[1]);
            rpipes.push_back(pipes[0]);
            if (iFileSettings != mSettings.project.fileSettings.end()) {
                childFile[pid] = iFileSettings->filename + ' ' + iFileSettings->cfg;
                pipeFile[pipes[0]] = iFileSettings->filename + ' ' + iFileSettings->cfg;
                ++iFileSettings;
            } else {
                childFile[pid] = iFile->first;
                pipeFile[pipes[0]] = iFile->first;
                ++iFile;
            }
        }
        if (!rpipes.empty()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            for (std::list<int>::const_iterator rp = rpipes.begin(); rp != rpipes.end(); ++rp)
                FD_SET(*rp, &rfds);
            struct timeval tv; // for every second polling of load average condition
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int r = select(*std::max_element(rpipes.begin(), rpipes.end()) + 1, &rfds, nullptr, nullptr, &tv);

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
                                std::map<std::string, std::size_t>::const_iterator fs = mFiles.find(name);
                                if (fs != mFiles.end()) {
                                    size = fs->second;
                                }
                            }

                            mFileCount++;
                            processedsize += size;
                            if (!mSettings.quiet)
                                CppCheckExecutor::reportStatus(mFileCount, mFiles.size() + mSettings.project.fileSettings.size(), processedsize, totalfilesize);

                            close(*rp);
                            rp = rpipes.erase(rp);
                        } else
                            ++rp;
                    } else
                        ++rp;
                }
            }
        }
        if (!childFile.empty()) {
            int stat = 0;
            pid_t child = waitpid(0, &stat, WNOHANG);
            if (child > 0) {
                std::string childname;
                std::map<pid_t, std::string>::iterator c = childFile.find(child);
                if (c != childFile.end()) {
                    childname = c->second;
                    childFile.erase(c);
                }

                if (WIFEXITED(stat)) {
                    const int exitstaus = WEXITSTATUS(stat);
                    if (exitstaus != 0) {
                        std::ostringstream oss;
                        oss << "Child process exited with " << exitstaus;
                        reportInternalChildErr(childname, oss.str());
                    }
                } else if (WIFSIGNALED(stat)) {
                    std::ostringstream oss;
                    oss << "Child process crashed with signal " << WTERMSIG(stat);
                    reportInternalChildErr(childname, oss.str());
                }
            }
        }
        if (iFile == mFiles.end() && iFileSettings == mSettings.project.fileSettings.end() && rpipes.empty() && childFile.empty()) {
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
    if (write(mWpipe, out, len + 1 + sizeof(len)) <= 0) {
        delete [] out;
        out = nullptr;
        std::cerr << "#### ThreadExecutor::writeToPipe, Failed to write to pipe" << std::endl;
        std::exit(0);
    }

    delete [] out;
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    writeToPipe(REPORT_OUT, outmsg);
}

void ThreadExecutor::reportErr(const ErrorMessage &msg)
{
    writeToPipe(REPORT_ERROR, msg.serialize());
}

void ThreadExecutor::reportInfo(const ErrorMessage &msg)
{
    writeToPipe(REPORT_INFO, msg.serialize());
}

void ThreadExecutor::bughuntingReport(const std::string &str)
{
    writeToPipe(REPORT_VERIFICATION, str.c_str());
}

void ThreadExecutor::reportInternalChildErr(const std::string &childname, const std::string &msg)
{
    std::list<ErrorMessage::FileLocation> locations;
    locations.emplace_back(childname, 0, 0);
    const ErrorMessage errmsg(locations,
                              emptyString,
                              Severity::error,
                              "Internal error: " + msg,
                              "cppcheckError",
                              false);

    if (!mSettings.nomsg.isSuppressed(errmsg.toSuppressionsErrorMessage()))
        mErrorLogger.reportErr(errmsg);
}

#elif defined(THREADING_MODEL_WIN)

void ThreadExecutor::addFileContent(const std::string &path, const std::string &content)
{
    mFileContents[path] = content;
}

unsigned int ThreadExecutor::check()
{
    HANDLE *threadHandles = new HANDLE[mSettings.jobs];

    mItNextFile = mFiles.begin();
    mItNextFileSettings = mSettings.project.fileSettings.begin();

    mProcessedFiles = 0;
    mProcessedSize = 0;
    mTotalFiles = mFiles.size() + mSettings.project.fileSettings.size();
    mTotalFileSize = 0;
    for (std::map<std::string, std::size_t>::const_iterator i = mFiles.begin(); i != mFiles.end(); ++i) {
        mTotalFileSize += i->second;
    }

    InitializeCriticalSection(&mFileSync);
    InitializeCriticalSection(&mErrorSync);
    InitializeCriticalSection(&mReportSync);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        threadHandles[i] = (HANDLE)_beginthreadex(nullptr, 0, threadProc, this, 0, nullptr);
        if (!threadHandles[i]) {
            std::cerr << "#### ThreadExecutor::check error, errno :" << errno << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    const DWORD waitResult = WaitForMultipleObjects(mSettings.jobs, threadHandles, TRUE, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        if (waitResult == WAIT_FAILED) {
            std::cerr << "#### ThreadExecutor::check wait failed, result: " << waitResult << " error: " << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cerr << "#### ThreadExecutor::check wait failed, result: " << waitResult << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    unsigned int result = 0;
    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        DWORD exitCode;

        if (!GetExitCodeThread(threadHandles[i], &exitCode)) {
            std::cerr << "#### ThreadExecutor::check get exit code failed, error:" << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }

        result += exitCode;

        if (!CloseHandle(threadHandles[i])) {
            std::cerr << "#### ThreadExecutor::check close handle failed, error:" << GetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    DeleteCriticalSection(&mFileSync);
    DeleteCriticalSection(&mErrorSync);
    DeleteCriticalSection(&mReportSync);

    delete[] threadHandles;

    return result;
}

unsigned int __stdcall ThreadExecutor::threadProc(void *args)
{
    unsigned int result = 0;

    ThreadExecutor *threadExecutor = static_cast<ThreadExecutor*>(args);
    std::map<std::string, std::size_t>::const_iterator &itFile = threadExecutor->mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator &itFileSettings = threadExecutor->mItNextFileSettings;

    // guard static members of CppCheck against concurrent access
    EnterCriticalSection(&threadExecutor->mFileSync);

    CppCheck fileChecker(*threadExecutor, false, CppCheckExecutor::executeCommand);
    fileChecker.settings() = threadExecutor->mSettings;

    for (;;) {
        if (itFile == threadExecutor->mFiles.end() && itFileSettings == threadExecutor->mSettings.project.fileSettings.end()) {
            LeaveCriticalSection(&threadExecutor->mFileSync);
            break;
        }

        std::size_t fileSize = 0;
        if (itFile != threadExecutor->mFiles.end()) {
            const std::string &file = itFile->first;
            fileSize = itFile->second;
            ++itFile;

            LeaveCriticalSection(&threadExecutor->mFileSync);

            const std::map<std::string, std::string>::const_iterator fileContent = threadExecutor->mFileContents.find(file);
            if (fileContent != threadExecutor->mFileContents.end()) {
                // File content was given as a string
                result += fileChecker.check(file, fileContent->second);
            } else {
                // Read file from a file
                result += fileChecker.check(file);
            }
        } else { // file settings..
            const ImportProject::FileSettings &fs = *itFileSettings;
            ++itFileSettings;
            LeaveCriticalSection(&threadExecutor->mFileSync);
            result += fileChecker.check(fs);
            if (threadExecutor->mSettings.clangTidy)
                fileChecker.analyseClangTidy(fs);
        }

        EnterCriticalSection(&threadExecutor->mFileSync);

        threadExecutor->mProcessedSize += fileSize;
        threadExecutor->mProcessedFiles++;
        if (!threadExecutor->mSettings.quiet) {
            EnterCriticalSection(&threadExecutor->mReportSync);
            CppCheckExecutor::reportStatus(threadExecutor->mProcessedFiles, threadExecutor->mTotalFiles, threadExecutor->mProcessedSize, threadExecutor->mTotalFileSize);
            LeaveCriticalSection(&threadExecutor->mReportSync);
        }
    }
    return result;
}

void ThreadExecutor::reportOut(const std::string &outmsg)
{
    EnterCriticalSection(&mReportSync);

    mErrorLogger.reportOut(outmsg);

    LeaveCriticalSection(&mReportSync);
}
void ThreadExecutor::reportErr(const ErrorMessage &msg)
{
    report(msg, MessageType::REPORT_ERROR);
}

void ThreadExecutor::reportInfo(const ErrorMessage &msg)
{

}

void ThreadExecutor::bughuntingReport(const std::string  &/*str*/)
{
    // TODO
}

void ThreadExecutor::report(const ErrorMessage &msg, MessageType msgType)
{
    if (mSettings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
        return;

    // Alert only about unique errors
    bool reportError = false;
    const std::string errmsg = msg.toString(mSettings.verbose);

    EnterCriticalSection(&mErrorSync);
    if (std::find(mErrorList.begin(), mErrorList.end(), errmsg) == mErrorList.end()) {
        mErrorList.emplace_back(errmsg);
        reportError = true;
    }
    LeaveCriticalSection(&mErrorSync);

    if (reportError) {
        EnterCriticalSection(&mReportSync);

        switch (msgType) {
        case MessageType::REPORT_ERROR:
            mErrorLogger.reportErr(msg);
            break;
        case MessageType::REPORT_INFO:
            mErrorLogger.reportInfo(msg);
            break;
        }

        LeaveCriticalSection(&mReportSync);
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
void ThreadExecutor::reportErr(const ErrorMessage &/*msg*/)
{

}

void ThreadExecutor::reportInfo(const ErrorMessage &/*msg*/)
{

}

void ThreadExecutor::bughuntingReport(const std::string &/*str*/)
{
}

#endif
