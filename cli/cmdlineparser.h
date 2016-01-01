/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#include <vector>
#include <string>

class Settings;

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
     */
    explicit CmdLineParser(Settings *settings);

    /**
     * Parse given command line.
     * @return true if command line was ok, false if there was an error.
     */
    bool ParseFromArgs(int argc, const char* const argv[]);

    /**
     * Return if user wanted to see program version.
     */
    bool GetShowVersion() const {
        return _showVersion;
    }

    /**
     * Return if user wanted to see list of error messages.
     */
    bool GetShowErrorMessages() const {
        return _showErrorMessages;
    }

    /**
     * Return the path names user gave to command line.
     */
    const std::vector<std::string>& GetPathNames() const {
        return _pathnames;
    }

    /**
     * Return if help is shown to user.
     */
    bool GetShowHelp() const {
        return _showHelp;
    }

    /**
     * Return if we should exit after printing version, help etc.
     */
    bool ExitAfterPrinting() const {
        return _exitAfterPrint;
    }

    /**
     * Return a list of paths user wants to ignore.
     */
    const std::vector<std::string>& GetIgnoredPaths() const {
        return _ignoredPaths;
    }

protected:

    /**
     * Print help text to the console.
     */
    static void PrintHelp();

    /**
     * Print message (to console?).
     */
    static void PrintMessage(const std::string &message);
    static void PrintMessage(const char* message);

private:
    std::vector<std::string> _pathnames;
    std::vector<std::string> _ignoredPaths;
    Settings *_settings;
    bool _showHelp;
    bool _showVersion;
    bool _showErrorMessages;
    bool _exitAfterPrint;
};

/// @}

#endif // CMDLINE_PARSER_H
