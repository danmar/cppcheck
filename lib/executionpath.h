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
class Check;

/**
 * Base class for Execution Paths checking
 * An execution path is a linear list of statements. There are no "if"/.. to worry about.
 **/
class ExecutionPath
{
private:
    bool bailout_;

    /** No implementation */
    void operator=(const ExecutionPath &);

protected:
    const unsigned int varId;
    Check * const owner;

public:
    ExecutionPath(Check *c, unsigned int id) : bailout_(false), varId(id), owner(c), ifinfo(0)
    { }

    virtual ~ExecutionPath()
    { }

    /** Implement this in each derived class. This function must create a copy of the current instance */
    virtual ExecutionPath *copy() = 0;

    /** Some kind of if-information */
    unsigned int ifinfo;

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
     * bail out execution paths with given variable id
     * @param checks the execution paths to bail out on
     * @param varid the specific variable id
     **/
    static void bailOutVar(const std::list<ExecutionPath *> &checks, const unsigned int varid)
    {
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
            (*it)->bailout_ |= ((*it)->varId == varid);
    }

    /**
     * Parse tokens at given location
     * @param tok token to parse
     * @param foundError If an error is found this is set to true
     * @param checks The execution paths. All execution paths in the list are executed in the current scope.
     * @return the token before the "next" token.
     **/
    virtual const Token *parse(const Token &tok, bool &foundError, std::list<ExecutionPath *> &checks) const = 0;

    /**
     * Parse condition
     * @param tok first token in condition.
     * @param checks The execution paths. All execution paths in the list are executed in the current scope
     * @return true => bail out all checking
     **/
    virtual bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks);

    /** going out of scope - all execution paths end */
    virtual void end(const std::list<ExecutionPath *> & /*checks*/, const Token * /*tok*/) const
    { }
};


void checkExecutionPaths(const Token *tok, ExecutionPath *c);


#endif
