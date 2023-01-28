/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "processexecutor.h"

#if !defined(WIN32) && !defined(__MINGW32__)

#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "importproject.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <numeric>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <sstream> // IWYU pragma: keep
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif

#if defined(__linux__)
#include <sys/prctl.h>
#endif

// NOLINTNEXTLINE(misc-unused-using-decls) - required for FD_ZERO
using std::memset;


ProcessExecutor::ProcessExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : Executor(files, settings, errorLogger)
{}

ProcessExecutor::~ProcessExecutor()
{}

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

    void writeEnd(const std::string& str) const {
        writeToPipe(CHILD_END, str);
    }

private:
    enum class MessageType {REPORT_ERROR, REPORT_INFO};

    void report(const ErrorMessage &msg, MessageType msgType) const {
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

    void writeToPipe(PipeSignal type, const std::string &data) const
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

int ProcessExecutor::handleRead(int rpipe, unsigned int &result)
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
            if (std::find(mErrorList.cbegin(), mErrorList.cend(), errmsg) == mErrorList.cend()) {
                mErrorList.emplace_back(std::move(errmsg));
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

bool ProcessExecutor::checkLoadAverage(size_t nchildren)
{
#if defined(__QNX__) || defined(__HAIKU__)  // getloadavg() is unsupported on Qnx, Haiku.
    (void)nchildren;
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

unsigned int ProcessExecutor::check()
{
    unsigned int fileCount = 0;
    unsigned int result = 0;

    const std::size_t totalfilesize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& p) {
        return v + p.second;
    });

    std::list<int> rpipes;
    std::map<pid_t, std::string> childFile;
    std::map<int, std::string> pipeFile;
    std::size_t processedsize = 0;
    std::map<std::string, std::size_t>::const_iterator iFile = mFiles.cbegin();
    std::list<ImportProject::FileSettings>::const_iterator iFileSettings = mSettings.project.fileSettings.cbegin();
    for (;;) {
        // Start a new child
        const size_t nchildren = childFile.size();
        if ((iFile != mFiles.cend() || iFileSettings != mSettings.project.fileSettings.cend()) && nchildren < mSettings.jobs && checkLoadAverage(nchildren)) {
            int pipes[2];
            if (pipe(pipes) == -1) {
                std::cerr << "#### ThreadExecutor::check, pipe() failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const int flags = fcntl(pipes[0], F_GETFL, 0);
            if (flags < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_GETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (fcntl(pipes[0], F_SETFL, flags | O_NONBLOCK) < 0) {
                std::cerr << "#### ThreadExecutor::check, fcntl(F_SETFL) failed: "<< std::strerror(errno) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const pid_t pid = fork();
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

                pipewriter.writeEnd(std::to_string(resultOfCheck));
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
            for (std::list<int>::const_iterator rp = rpipes.cbegin(); rp != rpipes.cend(); ++rp)
                FD_SET(*rp, &rfds);
            struct timeval tv; // for every second polling of load average condition
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            const int r = select(*std::max_element(rpipes.cbegin(), rpipes.cend()) + 1, &rfds, nullptr, nullptr, &tv);

            if (r > 0) {
                std::list<int>::iterator rp = rpipes.begin();
                while (rp != rpipes.end()) {
                    if (FD_ISSET(*rp, &rfds)) {
                        const int readRes = handleRead(*rp, result);
                        if (readRes == -1) {
                            std::size_t size = 0;
                            const std::map<int, std::string>::iterator p = pipeFile.find(*rp);
                            if (p != pipeFile.end()) {
                                std::string name = p->second;
                                pipeFile.erase(p);
                                const std::map<std::string, std::size_t>::const_iterator fs = mFiles.find(name);
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
            const pid_t child = waitpid(0, &stat, WNOHANG);
            if (child > 0) {
                std::string childname;
                const std::map<pid_t, std::string>::iterator c = childFile.find(child);
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

void ProcessExecutor::reportInternalChildErr(const std::string &childname, const std::string &msg)
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

#endif // !WIN32
