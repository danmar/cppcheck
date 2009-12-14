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

#ifndef executionpathH
#define executionpathH

#include <list>

class Token;

/**
 * Base class for Execution Paths checking
 * An execution path is a linear list of statements. There are no "if"/.. to worry about.
 **/
class ExecutionPath
{
private:
    mutable bool bailout_;

public:
    ExecutionPath() : bailout_(false)
    { }

    virtual ~ExecutionPath()
    { }

    virtual ExecutionPath *copy() = 0;

    bool bailOut() const
    {
        return bailout_;
    }

    /**
     * bail out all execution paths
     * @param checks the execution paths to bail out on
     **/
    static void bailOut(const std::list<ExecutionPath *> &checks)
    {
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
            (*it)->bailout_ = true;
    }

    /**
     * Parse tokens at given location
     * @param tok token to parse
     * @param foundError If an error is found this is set to true and the return token is the error token
     * @param checks The execution paths. All execution paths in the list are executed in the current scope.
     * @return if error is found => error token. if you want to skip tokens, return the last skipped token. otherwise return tok.
     **/
    virtual const Token *parse(const Token &tok, bool &foundError, std::list<ExecutionPath *> &checks) const = 0;
};


const Token *checkExecutionPaths(const Token *tok, std::list<ExecutionPath *> &checks);


#endif
