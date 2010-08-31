/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

class CppCheck;
class Settings;

class CmdLineParser
{
public:
    CmdLineParser(Settings *settings);
    bool ParseFromArgs(int argc, const char* const argv[]);
    bool GetShowVersion() const
    {
        return _showVersion;
    }
    bool GetShowErrorMessages() const
    {
        return _showErrorMessages;
    }
    std::vector<std::string> GetPathNames() const
    {
        return _pathnames;
    }

protected:
    void PrintHelp();

private:
    Settings *_settings;
    bool _showHelp;
    bool _showVersion;
    bool _showErrorMessages;
    std::vector<std::string> _pathnames;
};

#endif // CMDLINE_PARSER_H
