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
#include <cassert>
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
#include <utility>
#include <fcntl.h>


#ifdef __SVR4  // Solaris
#include <sys/loadavg.h>
#endif

#if defined(__linux__)
#include <sys/prctl.h>
#endif

enum class Color;

// NOLINTNEXTLINE(misc-unused-using-decls) - required for FD_ZERO
using std::memset;


ProcessExecutor::ProcessExecutor(const std::map<std::string, std::size_t> &files, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger)
    : Executor(files, settings, suppressions, errorLogger)
{
    assert(mSettings.jobs > 1);
}

class PipeWriter : public ErrorLogger {
public:
    enum PipeSignal {REPORT_OUT='1',REPORT_ERROR='2', CHILD_END='5'};

    explicit PipeWriter(int pipe) : mWpipe(pipe) {}

    void reportOut(const std::string &outmsg, Color c) override {
        writeToPipe(REPORT_OUT, static_cast<char>(c) + outmsg);
    }

    void reportErr(const ErrorMessage &msg) override {
        writeToPipe(REPORT_ERROR, msg.serialize());
    }

    void writeEnd(const std::string& str) const {
        writeToPipe(CHILD_END, str);
    }

private:
    // TODO: how to log file name in error?
    void writeToPipeInternal(PipeSignal type, const void* data, std::size_t to_write) const
    {
        const ssize_t bytes_written = write(mWpipe, data, to_write);
        if (bytes_written <= 0) {
            const int err = errno;
            std::cerr << "#### ThreadExecutor::writeToPipeInternal() error for type " << type << ": " << std::strerror(err) << std::endl;
            std::exit(EXIT_FAILURE);
        }
        // TODO: write until everything is written
        if (bytes_written != to_write) {
            std::cerr << "#### ThreadExecutor::writeToPipeInternal() error for type " << type << ": insufficient data written (expected: " << to_write << " / got: " << bytes_written << ")" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void writeToPipe(PipeSignal type, const std::string &data) const
    {
        {
            const char t = static_cast<char>(type);
            writeToPipeInternal(type, &t, 1);
        }

        const unsigned int len = static_cast<unsigned int>(data.length());
        {
            static constexpr std::size_t l_size = sizeof(unsigned int);
            writeToPipeInternal(type, &len, l_size);
        }

        writeToPipeInternal(type, data.c_str(), len);
    }

    const int mWpipe;
};

bool ProcessExecutor::handleRead(int rpipe, unsigned int &result, const std::string& filename)
{
    std::size_t bytes_to_read;
    ssize_t bytes_read;

    char type = 0;
    bytes_to_read = sizeof(char);
    bytes_read = read(rpipe, &type, bytes_to_read);
    if (bytes_read <= 0) {
        if (errno == EAGAIN)
            return true;

        // TODO: log details about failure

        // need to increment so a missing pipe (i.e. premature exit of forked process) results in an error exitcode
        ++result;
        return false;
    }
    if (bytes_read != bytes_to_read) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (type): insufficient data read (expected: " << bytes_to_read << " / got: " << bytes_read << ")" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (type != PipeWriter::REPORT_OUT && type != PipeWriter::REPORT_ERROR && type != PipeWriter::CHILD_END) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") invalid type " << int(type) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    unsigned int len = 0;
    bytes_to_read = sizeof(len);
    bytes_read = read(rpipe, &len, bytes_to_read);
    if (bytes_read <= 0) {
        const int err = errno;
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (len) for type " << int(type) << ": " << std::strerror(err) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (bytes_read != bytes_to_read) {
        std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (len) for type" << int(type) << ": insufficient data read (expected: " << bytes_to_read << " / got: " << bytes_read << ")"  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string buf(len, '\0');
    char *data_start = &buf[0];
    bytes_to_read = len;
    do {
        bytes_read = read(rpipe, data_start, bytes_to_read);
        if (bytes_read <= 0) {
            const int err = errno;
            std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") error (buf) for type" << int(type) << ": " << std::strerror(err) << std::endl;
            std::exit(EXIT_FAILURE);
        }
        bytes_to_read -= bytes_read;
        data_start += bytes_read;
    } while (bytes_to_read != 0);

    bool res = true;
    if (type == PipeWriter::REPORT_OUT) {
        // the first character is the color
        const Color c = static_cast<Color>(buf[0]);
        // TODO: avoid string copy
        mErrorLogger.reportOut(buf.substr(1), c);
    } else if (type == PipeWriter::REPORT_ERROR) {
        ErrorMessage msg;
        try {
            msg.deserialize(buf);
        } catch (const InternalError& e) {
            std::cerr << "#### ThreadExecutor::handleRead(" << filename << ") internal error: " << e.errorMessage << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (hasToLog(msg))
            mErrorLogger.reportErr(msg);
    } else if (type == PipeWriter::CHILD_END) {
        result += std::stoi(buf);
        res = false;
    }

    return res;
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
    }
    if (sample < mSettings.loadAverage) {
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

            if (fcntl(pipes[0], F_SETFL, flags) < 0) {
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
                    // TODO: call analyseClangTidy()
                } else {
                    // Read file from a file
                    resultOfCheck = fileChecker.check(iFile->first);
                    // TODO: call analyseClangTidy()?
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
                        std::string name;
                        const std::map<int, std::string>::iterator p = pipeFile.find(*rp);
                        if (p != pipeFile.end()) {
                            name = p->second;
                        }
                        const bool readRes = handleRead(*rp, result, name);
                        if (!readRes) {
                            std::size_t size = 0;
                            if (p != pipeFile.end()) {
                                pipeFile.erase(p);
                                const std::map<std::string, std::size_t>::const_iterator fs = mFiles.find(name);
                                if (fs != mFiles.end()) {
                                    size = fs->second;
                                }
                            }

                            fileCount++;
                            processedsize += size;
                            if (!mSettings.quiet)
                                Executor::reportStatus(fileCount, mFiles.size() + mSettings.project.fileSettings.size(), processedsize, totalfilesize);

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

    if (!mSuppressions.isSuppressed(errmsg))
        mErrorLogger.reportErr(errmsg);
}

#endif // !WIN32
