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

#ifndef CMDLINE_PARSER_H
#define CMDLINE_PARSER_H

#include <cstddef>
#include <string>
#include <vector>

#include "utils.h"

class Settings;
class Suppressions;

/// @addtogroup CLI
/// @{

/**
 * @brief The command line parser.
 * The command line parser parses options and parameters user gives to
 * cppcheck command line.
 *
 * The parser takes a pointer to Settings instance which it will update
 * based on options user has given. Couple of options are handled as
 * class internal options.
 */
class CmdLineParser {
public:
    /**
     * The constructor.
     * @param settings Settings instance that will be modified according to
     * options user has given.
     * @param suppressions Suppressions instance that keeps the suppressions
     * @param suppressionsNoFail Suppressions instance that keeps the "do not fail" suppressions
     */
    CmdLineParser(Settings &settings, Suppressions &suppressions, Suppressions &suppressionsNoFail);

    /**
     * Parse given command line.
     * @return true if command line was ok, false if there was an error.
     */
    bool parseFromArgs(int argc, const char* const argv[]);

    /**
     * Return if user wanted to see program version.
     */
    bool getShowVersion() const {
        return mShowVersion;
    }

    /**
     * Return if user wanted to see list of error messages.
     */
    bool getShowErrorMessages() const {
        return mShowErrorMessages;
    }

    /**
     * Return the path names user gave to command line.
     */
    const std::vector<std::string>& getPathNames() const {
        return mPathNames;
    }

    /**
     * Return if help is shown to user.
     */
    bool getShowHelp() const {
        return mShowHelp;
    }

    /**
     * Return if we should exit after printing version, help etc.
     */
    bool exitAfterPrinting() const {
        return mExitAfterPrint;
    }

    /**
     * Return a list of paths user wants to ignore.
     */
    const std::vector<std::string>& getIgnoredPaths() const {
        return mIgnoredPaths;
    }

#if defined(_WIN64) || defined(_WIN32)
    // temporary variable to "un-break" tests
    static bool SHOW_DEF_PLATFORM_MSG;
#endif

protected:

    /**
     * Print help text to the console.
     */
    void printHelp();

    /**
     * Print message (to stdout).
     */
    static void printMessage(const std::string &message);

    /**
     * Print error message (to stdout).
     */
    static void printError(const std::string &message);

private:
    bool isCppcheckPremium() const;

    template<typename T>
    static bool parseNumberArg(const char* const arg, std::size_t offset, T& num, bool mustBePositive = false)
    {
        T tmp;
        std::string err;
        if (!strToInt(arg + offset, tmp, &err)) {
            printError("argument to '" + std::string(arg, offset) + "' is not valid - " + err + ".");
            return false;
        }
        if (mustBePositive && tmp < 0) {
            printError("argument to '" + std::string(arg, offset) + "' needs to be a positive integer.");
            return false;
        }
        num = tmp;
        return true;
    }

    std::vector<std::string> mPathNames;
    std::vector<std::string> mIgnoredPaths;
    Settings &mSettings;
    Suppressions &mSuppressions;
    Suppressions &mSuppressionsNoFail;
    bool mShowHelp{};
    bool mShowVersion{};
    bool mShowErrorMessages{};
    bool mExitAfterPrint{};
    std::string mVSConfig;
};

/// @}

#endif // CMDLINE_PARSER_H
