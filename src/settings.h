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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <list>
#include <string>
#include <istream>

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
    std::list<std::string> _autoDealloc;

    /** Code to append in the checks */
    std::string _append;

public:
    Settings();
    virtual ~Settings();

    bool _debug;
    bool _showAll;
    bool _checkCodingStyle;
    bool _errorsOnly;
    bool _verbose;

    /** Force checking t he files with "too many" configurations. */
    bool _force;

    /** write xml results */
    bool _xml;

    /** Checking if there are unused functions */
    bool _unusedFunctions;

    /** How many processes/threads should do checking at the same
        time. Default is 1. */
    unsigned int _jobs;

    /** If errors are found, this value is returned from main().
        Default value is 0. */
    int _exitCode;

    /** The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string _outputFormat;

#ifdef __GNUC__
    /** show timing information */
    bool _showtime;
#endif

    /** List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. */
    std::list<std::string> _includePaths;

    /** Fill list of automaticly deallocated classes */
    void autoDealloc(std::istream &istr);

    /** Add class to list of automatically deallocated classes */
    void addAutoAllocClass(const std::string &name);

    /** is a class automaticly deallocated? */
    bool isAutoDealloc(const char classname[]) const;

    /** assign append code */
    void append(const std::string &filename);

    /** get append code */
    std::string append() const;
};

/// @}

#endif // SETTINGS_H
