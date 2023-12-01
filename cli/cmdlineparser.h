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
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "cmdlinelogger.h"
#include "filesettings.h"
#include "utils.h"

class Settings;
class Suppressions;
class Library;

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
     * @param logger The logger instance to log messages through
     * @param settings Settings instance that will be modified according to
     * options user has given.
     * @param suppressions Suppressions instance that keeps the suppressions
     * @param suppressionsNoFail Suppressions instance that keeps the "do not fail" suppressions
     */
    CmdLineParser(CmdLineLogger &logger, Settings &settings, Suppressions &suppressions, Suppressions &suppressionsNoFail);

    enum class Result { Success, Exit, Fail };

    /**
     * @brief Parse command line args and fill settings and file lists
     * from there.
     *
     * @param argc argc from main()
     * @param argv argv from main()
     * @return false when errors are found in the input
     */
    bool fillSettingsFromArgs(int argc, const char* const argv[]);

    /**
     * Parse given command line.
     * @return true if command line was ok, false if there was an error.
     */
    Result parseFromArgs(int argc, const char* const argv[]);

    /**
     * Return the path names user gave to command line.
     */
    const std::vector<std::string>& getPathNames() const {
        return mPathNames;
    }

    /**
     * Return the files user gave to command line.
     */
    const std::list<std::pair<std::string, std::size_t>>& getFiles() const {
        return mFiles;
    }

    /**
     * Return the file settings read from command line.
     */
    const std::list<FileSettings>& getFileSettings() const {
        return mFileSettings;
    }

    /**
     * Return a list of paths user wants to ignore.
     */
    const std::vector<std::string>& getIgnoredPaths() const {
        return mIgnoredPaths;
    }

protected:

    /**
     * Print help text to the console.
     */
    void printHelp() const;

private:
    static bool isCppcheckPremium();

    template<typename T>
    bool parseNumberArg(const char* const arg, std::size_t offset, T& num, bool mustBePositive = false)
    {
        T tmp;
        std::string err;
        if (!strToInt(arg + offset, tmp, &err)) {
            mLogger.printError("argument to '" + std::string(arg, offset) + "' is not valid - " + err + ".");
            return false;
        }
        if (mustBePositive && tmp < 0) {
            mLogger.printError("argument to '" + std::string(arg, offset) + "' needs to be a positive integer.");
            return false;
        }
        num = tmp;
        return true;
    }

    /**
     * Tries to load a library and prints warning/error messages
     * @return false, if an error occurred (except unknown XML elements)
     */
    bool tryLoadLibrary(Library& destination, const std::string& basepath, const char* filename);

    /**
     * @brief Load libraries
     * @param settings Settings
     * @return Returns true if successful
     */
    bool loadLibraries(Settings& settings);

    /**
     * @brief Load addons
     * @param settings Settings
     * @return Returns true if successful
     */
    bool loadAddons(Settings& settings);

    bool loadCppcheckCfg();

    CmdLineLogger &mLogger;

    std::vector<std::string> mPathNames;
    std::list<std::pair<std::string, std::size_t>> mFiles;
    std::list<FileSettings> mFileSettings;
    std::vector<std::string> mIgnoredPaths;
    Settings &mSettings;
    Suppressions &mSuppressions;
    Suppressions &mSuppressionsNoFail;
    std::string mVSConfig;
};

/// @}

#endif // CMDLINE_PARSER_H
