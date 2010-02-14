/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <list>
#include <string>
#include <istream>
#include <map>
#include <set>

/// @addtogroup Core
/// @{


/**
 * This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class Settings
{
private:
    /** classes that are automaticly deallocated */
    std::set<std::string> _autoDealloc;

    /** Code to append in the checks */
    std::string _append;

    /** enable extra checks by id */
    std::map<std::string, bool> _enabled;

    /** terminate checking */
    bool _terminate;
public:
    Settings();
    virtual ~Settings();

    bool _debug;
    bool _showAll;
    bool _checkCodingStyle;
    bool _errorsOnly;
    bool _inlineSuppressions;
    bool _verbose;

    /** Request termination of checking */
    void terminate()
    {
        _terminate = true;
    }

    /** termination? */
    bool terminated() const
    {
        return _terminate;
    }

    /** Force checking t he files with "too many" configurations. */
    bool _force;

    /** write xml results */
    bool _xml;

    /** How many processes/threads should do checking at the same
        time. Default is 1. */
    unsigned int _jobs;

    /** If errors are found, this value is returned from main().
        Default value is 0. */
    int _exitCode;

    /** The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string _outputFormat;

    /** show timing information */
    bool _showtime;

    /** List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. */
    std::list<std::string> _includePaths;

    /** Fill list of automaticly deallocated classes */
    void autoDealloc(std::istream &istr);

    /** Add class to list of automatically deallocated classes */
    void addAutoAllocClass(const std::string &name);

    /** is a class automaticly deallocated? */
    bool isAutoDealloc(const std::string &classname) const;

    /** assign append code */
    void append(const std::string &filename);

    /** get append code */
    std::string append() const;

    /**
     * Returns true if given id is in the list of
     * enabled extra checks. See addEnabled()
     * @param str id for the extra check, e.g. "style"
     * @return true if the check is enabled.
     */
    bool isEnabled(const std::string &str) const;

    /**
     * Enable extra checks by id. See isEnabled()
     * @param str single id or list of id values to be enabled
     * or empty string to enable all. e.g. "style,possibleError"
     */
    void addEnabled(const std::string &str);

    /** class for handling suppressions */
    class Suppressions
    {
    private:
        /** List of error which the user doesn't want to see. */
        std::map<std::string, std::map<std::string, std::list<int> > > _suppressions;
    public:
        /**
         * Don't show errors listed in the file.
         * @param istr Open file stream where errors can be read.
         * @return true on success, false in syntax error is noticed.
         */
        bool parseFile(std::istream &istr);

        /**
         * Don't show this error. If file and/or line are optional. In which case
         * the errorId alone is used for filtering.
         * @param errorId, the id for the error, e.g. "arrayIndexOutOfBounds"
         * @param file File name with the path, e.g. "src/main.cpp"
         * @param line number, e.g. "123"
         */
        void addSuppression(const std::string &errorId, const std::string &file = "", unsigned int line = 0);

        /**
         * Returns true if this message should not be shown to the user.
         * @param errorId, the id for the error, e.g. "arrayIndexOutOfBounds"
         * @param file File name with the path, e.g. "src/main.cpp"
         * @param line number, e.g. "123"
         * @return true if this error is suppressed.
         */
        bool isSuppressed(const std::string &errorId, const std::string &file, unsigned int line);

    };

    /** suppress message */
    Suppressions nomsg;

    /** suppress exitcode */
    Suppressions nofail;
};

/// @}

#endif // SETTINGS_H
