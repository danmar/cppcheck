/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "color.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "importproject.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <utility>

#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif

#ifdef THREADING_MODEL_FORK
#include "config.h"
#include "errortypes.h"

#if defined(__linux__)
#include <sys/prctl.h>
#endif
#include <cerrno>
#include <cstring>
#include <sys/select.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <csignal>
#include <unistd.h>

// NOLINTNEXTLINE(misc-unused-using-decls) - required for FD_ZERO
using std::memset;
#endif

#ifdef THREADING_MODEL_THREAD
#include <future>
#include <numeric>
#endif

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : mFiles(files), mSettings(settings), mErrorLogger(errorLogger)
{}

ThreadExecutor::~ThreadExecutor()
{}

///////////////////////////////////////////////////////////////////////////////
////// This code is for platforms that support fork() only ////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(THREADING_MODEL_FORK)

class PipeWriter : public ErrorLogger {
public:
    enum PipeSignal {REPORT_OUT='1',REPORT_ERROR='2', REPORT_INFO='3', REPORT_VERIFICATION='4', CHILD_END='5'};

    explicit PipeWriter(int pipe) : mWpipe(pipe) {}

    void reportOut(const std::string &outmsg, Color c) override {
        writeToPipe(REPORT_OUT, ::toString(c) + outmsg + ::toString(Color::Reset));
    }

    void reportErr(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_ERROR);
    }

    void reportInfo(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_INFO);
    }

    void writeEnd(const std::string& str) {
        writeToPipe(CHILD_END, str);
    }

private:
    enum class MessageType {REPORT_ERROR, REPORT_INFO};

    void report(const ErrorMessage &msg, MessageType msgType) {
        PipeSignal pipeSignal;
        switch (msgType) {
        case MessageType::REPORT_ERROR:
            pipeSignal = REPORT_ERROR;
            break;
        case MessageType::REPORT_INFO:
            pipeSignal = REPORT_INFO;
            break;
        }

        writeToPipe(pipeSignal, msg.serialize());
    }

    void writeToPipe(PipeSignal type, const std::string &data)
    {
        unsigned int len = static_cast<unsigned int>(data.length() + 1);
        char *out = new char[len + 1 + sizeof(len)];
        out[0] = static_cast<char>(type);
        std::memcpy(&(out[1]), &len, sizeof(len));
        std::memcpy(&(out[1+sizeof(len)]), data.c_str(), len);
        if (write(mWpipe, out, len + 1 + sizeof(len)) <= 0) {
            delete[] out;
            out = nullptr;
            std::cerr << "#### ThreadExecutor::writeToPipe, Failed to write to pipe" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        delete[] out;
    }

    const int mWpipe;
};

int ThreadExecutor::handleRead(int rpipe, unsigned int &result)
{
    char type = 0;
    if (read(rpipe, &type, 1) <= 0) {
        if (errno == EAGAIN)
            return 0;

        // need to increment so a missing pipe (i.e. premature exit of forked process) results in an error exitcode
        ++result;
        return -1;
    }

    if (type != PipeWriter::REPORT_OUT && type != PipeWriter::REPORT_ERROR && type != PipeWriter::REPORT_INFO && type != PipeWriter::CHILD_END) {
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(EXIT_FAILURE);
    }

    unsigned int len = 0;
    if (read(rpipe, &len, sizeof(len)) <= 0) {
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Don't rely on incoming data being null-terminated.
    // Allocate +1 element and null-terminate the buffer.
    char *buf = new char[len + 1];
    const ssize_t readIntoBuf = read(rpipe, buf, len);
    if (readIntoBuf <= 0) {
        std::cerr << "#### ThreadExecutor::handleRead error, type was:" << type << std::endl;
        std::exit(EXIT_FAILURE);
    }
    buf[readIntoBuf] = 0;

    if (type == PipeWriter::REPORT_OUT) {
        mErrorLogger.reportOut(buf);
    } else if (type == PipeWriter::REPORT_ERROR || type == PipeWriter::REPORT_INFO) {
        ErrorMessage msg;
        try {
            msg.deserialize(buf);
        } catch (const InternalError& e) {
            std::cerr << "#### ThreadExecutor::handleRead error, internal error:" << e.errorMessage << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (!mSettings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage())) {
            // Alert only about unique errors
            std::string errmsg = msg.toString(mSettings.verbose);
            if (std::find(mErrorList.begin(), mErrorList.end(), errmsg) == mErrorList.end()) {
                mErrorList.emplace_back(errmsg);
                if (type == PipeWriter::REPORT_ERROR)
                    mErrorLogger.reportErr(msg);
                else
                    mErrorLogger.reportInfo(msg);
            }
        }
    } else if (type == PipeWriter::CHILD_END) {
        std::istringstream iss(buf);
        unsigned int fileResult = 0;
        iss >> fileResult;
        result += fileResult;
        delete[] buf;
        return -1;
    }

    delete[] buf;
    return 1;
}

bool ThreadExecutor::checkLoadAverage(size_t nchildren)
{
#if defined(__CYGWIN__) || defined(__QNX__) || defined(__HAIKU__)  // getloadavg() is unsupported on Cygwin, Qnx, Haiku.
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
    unsigned int fileCount = 0;
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

                PipeWriter pipewriter(pipes[1]);
                CppCheck fileChecker(pipewriter, false, CppCheckExecutor::executeCommand);
                fileChecker.settings() = mSettings;
                unsigned int resultOfCheck = 0;

                if (iFileSettings != mSettings.project.fileSettings.end()) {
                    resultOfCheck = fileChecker.check(*iFileSettings);
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(iFile->first);
                }

                std::ostringstream oss;
                oss << resultOfCheck;
                pipewriter.writeEnd(oss.str());
                std::exit(EXIT_SUCCESS);
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

                            fileCount++;
                            processedsize += size;
                            if (!mSettings.quiet)
                                CppCheckExecutor::reportStatus(fileCount, mFiles.size() + mSettings.project.fileSettings.size(), processedsize, totalfilesize);

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
                    const int exitstatus = WEXITSTATUS(stat);
                    if (exitstatus != EXIT_SUCCESS) {
                        std::ostringstream oss;
                        oss << "Child process exited with " << exitstatus;
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

void ThreadExecutor::reportInternalChildErr(const std::string &childname, const std::string &msg)
{
    std::list<ErrorMessage::FileLocation> locations;
    locations.emplace_back(childname, 0, 0);
    const ErrorMessage errmsg(locations,
                              emptyString,
                              Severity::error,
                              "Internal error: " + msg,
                              "cppcheckError",
                              Certainty::normal);

    if (!mSettings.nomsg.isSuppressed(errmsg.toSuppressionsErrorMessage()))
        mErrorLogger.reportErr(errmsg);
}

#elif defined(THREADING_MODEL_THREAD)

class ThreadExecutor::SyncLogForwarder : public ErrorLogger
{
public:
    SyncLogForwarder(ThreadExecutor &threadExecutor)
        : mThreadExecutor(threadExecutor), mProcessedFiles(0), mTotalFiles(0), mProcessedSize(0), mTotalFileSize(0) {

        mItNextFile = threadExecutor.mFiles.begin();
        mItNextFileSettings = threadExecutor.mSettings.project.fileSettings.begin();

        mTotalFiles = threadExecutor.mFiles.size() + threadExecutor.mSettings.project.fileSettings.size();
        for (std::map<std::string, std::size_t>::const_iterator i = threadExecutor.mFiles.begin(); i != threadExecutor.mFiles.end(); ++i) {
            mTotalFileSize += i->second;
        }
    }

    void reportOut(const std::string &outmsg, Color c) override
    {
        std::lock_guard<std::mutex> lg(mReportSync);

        mThreadExecutor.mErrorLogger.reportOut(outmsg, c);
    }

    void reportErr(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_ERROR);
    }

    void reportInfo(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_INFO);
    }

    ThreadExecutor &mThreadExecutor;

    std::map<std::string, std::size_t>::const_iterator mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator mItNextFileSettings;

    std::size_t mProcessedFiles;
    std::size_t mTotalFiles;
    std::size_t mProcessedSize;
    std::size_t mTotalFileSize;

    std::mutex mFileSync;
    std::mutex mErrorSync;
    std::mutex mReportSync;

private:
    enum class MessageType {REPORT_ERROR, REPORT_INFO};

    void report(const ErrorMessage &msg, MessageType msgType)
    {
        if (mThreadExecutor.mSettings.nomsg.isSuppressed(msg.toSuppressionsErrorMessage()))
            return;

        // Alert only about unique errors
        bool reportError = false;
        const std::string errmsg = msg.toString(mThreadExecutor.mSettings.verbose);

        {
            std::lock_guard<std::mutex> lg(mErrorSync);
            if (std::find(mThreadExecutor.mErrorList.begin(), mThreadExecutor.mErrorList.end(), errmsg) == mThreadExecutor.mErrorList.end()) {
                mThreadExecutor.mErrorList.emplace_back(errmsg);
                reportError = true;
            }
        }

        if (reportError) {
            std::lock_guard<std::mutex> lg(mReportSync);

            switch (msgType) {
            case MessageType::REPORT_ERROR:
                mThreadExecutor.mErrorLogger.reportErr(msg);
                break;
            case MessageType::REPORT_INFO:
                mThreadExecutor.mErrorLogger.reportInfo(msg);
                break;
            }
        }
    }
};

unsigned int ThreadExecutor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    SyncLogForwarder logforwarder(*this);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, threadProc, &logforwarder));
        }
        catch (const std::system_error &e) {
            std::cerr << "#### ThreadExecutor::check exception :" << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return std::accumulate(threadFutures.begin(), threadFutures.end(), 0U, [](unsigned int v, std::future<unsigned int>& f) {
        return v + f.get();
    });
}

unsigned int STDCALL ThreadExecutor::threadProc(SyncLogForwarder* logForwarder)
{
    unsigned int result = 0;

    std::map<std::string, std::size_t>::const_iterator &itFile = logForwarder->mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator &itFileSettings = logForwarder->mItNextFileSettings;

    // guard static members of CppCheck against concurrent access
    logForwarder->mFileSync.lock();

    for (;;) {
        if (itFile == logForwarder->mThreadExecutor.mFiles.end() && itFileSettings == logForwarder->mThreadExecutor.mSettings.project.fileSettings.end()) {
            logForwarder->mFileSync.unlock();
            break;
        }

        CppCheck fileChecker(*logForwarder, false, CppCheckExecutor::executeCommand);
        fileChecker.settings() = logForwarder->mThreadExecutor.mSettings;

        std::size_t fileSize = 0;
        if (itFile != logForwarder->mThreadExecutor.mFiles.end()) {
            const std::string &file = itFile->first;
            fileSize = itFile->second;
            ++itFile;

            logForwarder->mFileSync.unlock();

            // Read file from a file
            result += fileChecker.check(file);
        } else { // file settings..
            const ImportProject::FileSettings &fs = *itFileSettings;
            ++itFileSettings;
            logForwarder->mFileSync.unlock();
            result += fileChecker.check(fs);
            if (logForwarder->mThreadExecutor.mSettings.clangTidy)
                fileChecker.analyseClangTidy(fs);
        }

        logForwarder->mFileSync.lock();

        logForwarder->mProcessedSize += fileSize;
        logForwarder->mProcessedFiles++;
        if (!logForwarder->mThreadExecutor.mSettings.quiet) {
            std::lock_guard<std::mutex> lg(logForwarder->mReportSync);
            CppCheckExecutor::reportStatus(logForwarder->mProcessedFiles, logForwarder->mTotalFiles, logForwarder->mProcessedSize, logForwarder->mTotalFileSize);
        }
    }
    return result;
}
#endif

bool ThreadExecutor::isEnabled() {
    return true;
}
