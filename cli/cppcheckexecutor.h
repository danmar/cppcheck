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

#ifndef CPPCHECKEXECUTOR_H
#define CPPCHECKEXECUTOR_H

#include "color.h"
#include "errorlogger.h"

#include <cstdio>
#include <ctime>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

class CppCheck;
class Library;
class Settings;

/**
 * This class works as an example of how CppCheck can be used in external
 * programs without very little knowledge of the internal parts of the
 * program itself. If you wish to use cppcheck e.g. as a part of IDE,
 * just rewrite this class for your needs and possibly use other methods
 * from CppCheck class instead the ones used here.
 */
class CppCheckExecutor : public ErrorLogger {
public:
    friend class TestSuppressions;

    /**
     * Constructor
     */
    CppCheckExecutor() = default;
    CppCheckExecutor(const CppCheckExecutor &) = delete;
    void operator=(const CppCheckExecutor&) = delete;

    /**
     * Destructor
     */
    ~CppCheckExecutor() override;

    /**
     * Starts the checking.
     *
     * @param argc from main()
     * @param argv from main()
     * @return EXIT_FAILURE if arguments are invalid or no input files
     *         were found.
     *         If errors are found and --error-exitcode is used,
     *         given value is returned instead of default 0.
     *         If no errors are found, 0 is returned.
     */
    int check(int argc, const char* const argv[]);

    /**
     * Information about progress is directed here. This should be
     * called by the CppCheck class only.
     *
     * @param outmsg Progress message e.g. "Checking main.cpp..."
     */
    void reportOut(const std::string &outmsg, Color c = Color::Reset) override;

    /** xml output of errors */
    void reportErr(const ErrorMessage &msg) override;

    void reportProgress(const std::string &filename, const char stage[], const std::size_t value) override;

    /**
     * @param exceptionOutput Output file
     */
    static void setExceptionOutput(FILE* exceptionOutput);
    /**
     * @return file name to be used for output from exception handler. Has to be either "stdout" or "stderr".
     */
    static FILE* getExceptionOutput();

    /**
     * Tries to load a library and prints warning/error messages
     * @return false, if an error occurred (except unknown XML elements)
     */
    static bool tryLoadLibrary(Library& destination, const std::string& basepath, const char* filename);

    /**
     * Execute a shell command and read the output from it. Returns true if command terminated successfully.
     */
    static bool executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output_);

protected:

    /**
     * Helper function to print out errors. Appends a line change.
     * @param errmsg String printed to error stream
     */
    void reportErr(const std::string &errmsg);

    /**
     * @brief Parse command line args and get settings and file lists
     * from there.
     *
     * @param settings the settings to store into
     * @param argc argc from main()
     * @param argv argv from main()
     * @return false when errors are found in the input
     */
    bool parseFromArgs(Settings &settings, int argc, const char* const argv[]);

private:

    static bool reportSuppressions(const Settings &settings, bool unusedFunctionCheckEnabled, const std::map<std::string, std::size_t> &files, ErrorLogger& errorLogger);

    /**
     * Wrapper around check_internal
     *   - installs optional platform dependent signal handling
     *
     * @param cppcheck cppcheck instance
     **/
    int check_wrapper(CppCheck& cppcheck);

    /**
     * Starts the checking.
     *
     * @param cppcheck cppcheck instance
     * @return EXIT_FAILURE if arguments are invalid or no input files
     *         were found.
     *         If errors are found and --error-exitcode is used,
     *         given value is returned instead of default 0.
     *         If no errors are found, 0 is returned.
     */
    int check_internal(CppCheck& cppcheck);

    /**
     * @brief Load libraries
     * @param settings Settings
     * @return Returns true if successful
     */
    bool loadLibraries(Settings& settings);

    /**
     * @brief Write the checkers report
     */
    void writeCheckersReport(const Settings& settings) const;

    /**
     * Pointer to current settings; set while check() is running for reportError().
     */
    const Settings* mSettings{};

    /**
     * Used to filter out duplicate error messages.
     */
    std::set<std::string> mShownErrors;

    /**
     * Filename associated with size of file
     */
    std::map<std::string, std::size_t> mFiles;

    /**
     * Report progress time
     */
    std::time_t mLatestProgressOutputTime{};

    /**
     * Output file name for exception handler
     */
    static FILE* mExceptionOutput;

    /**
     * Error output
     */
    std::ofstream* mErrorOutput{};

    /**
     * Checkers that has been executed
     */
    std::set<std::string> mActiveCheckers;

    /**
     * True if there are critical errors
     */
    std::string mCriticalErrors;
};

#endif // CPPCHECKEXECUTOR_H
