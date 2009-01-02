/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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

#ifndef ERRORLOGGER_H
#define ERRORLOGGER_H

#include <string>

/**
 * This is an interface, which the class responsible of error logging
 * should implement.
 */
class ErrorLogger
{
public:
    virtual ~ErrorLogger() {}

    /**
     * Errors and warnings are directed here.
     *
     * @param errmsg Errors messages are normally in format
     * "[filepath:line number] Message", e.g.
     * "[main.cpp:4] Uninitialized member variable"
     */
    virtual void reportErr( const std::string &errmsg) = 0;

    /**
     * Information about progress is directed here.
     *
     * @param outmsg, E.g. "Checking main.cpp..."
     */
    virtual void reportOut( const std::string &outmsg) = 0;
};

#endif // #ifndef ERRORLOGGER_H
