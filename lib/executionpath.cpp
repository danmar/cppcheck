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


#include "executionpath.h"
#include "token.h"
#include <memory>


const Token *checkExecutionPaths(const Token *tok, std::list<ExecutionPath *> &checks)
{
    const std::auto_ptr<ExecutionPath> check(checks.front()->copy());

    // Number of "if" blocks in current scope
    unsigned int numberOfIf = 0;

    for (; tok; tok = tok->next())
    {
        if (tok->str() == "}")
            return 0;

        // todo: handle for/while
        if (Token::Match(tok, "for|while|switch"))
        {
            ExecutionPath::bailOut(checks);
            return 0;
        }

        if (Token::Match(tok, "= {") || Token::Match(tok, "= ( %type% !!="))
        {
            tok = tok->next()->link();
            if (Token::simpleMatch(tok, ") {"))
                tok = tok->next()->link();
            if (!tok)
            {
                ExecutionPath::bailOut(checks);
                return 0;
            }
            continue;
        }

        if (tok->str() == "if")
        {
            ++numberOfIf;
            if (numberOfIf > 1)
                return 0;

            std::list<ExecutionPath *> newchecks;
            while (tok->str() == "if")
            {
                // goto "("
                tok = tok->next();

                // goto ")"
                tok = tok ? tok->link() : 0;

                // Check if the condition contains the variable
                if (tok)
                {
                    for (const Token *tok2 = tok->link(); tok2 && tok2 != tok; tok2 = tok2->next())
                    {
                        // The variable may be initialized..
                        // if (someFunction(&var)) ..
                        if (tok2->varId())
                        {
                            ExecutionPath::bailOut(checks);
                            return 0;
                        }
                    }
                }

                // goto "{"
                tok = tok ? tok->next() : 0;

                if (!Token::simpleMatch(tok, "{"))
                    return 0;

                // Recursively check into the if ..
                {
                    std::list<ExecutionPath *> c;
                    std::list<ExecutionPath *>::iterator it;
                    for (it = checks.begin(); it != checks.end(); ++it)
                        c.push_back((*it)->copy());
                    const Token *tokerr = checkExecutionPaths(tok->next(), c);
                    if (tokerr)
                        return tokerr;
                    while (!c.empty())
                    {
                        newchecks.push_back(c.back());
                        c.pop_back();
                    }
                }

                // goto "}"
                tok = tok->link();

                // there is no else => break out
                if (Token::Match(tok, "} !!else"))
                    break;

                // parse next "if"..
                tok = tok->tokAt(2);
                if (tok->str() == "if")
                    continue;

                // there is no "if"..
                const Token *tokerr = checkExecutionPaths(tok->next(), checks);
                if (tokerr)
                    return tokerr;

                tok = tok->link();
                if (!tok)
                    return 0;
            }

            std::list<ExecutionPath *>::iterator it;
            for (it = newchecks.begin(); it != newchecks.end(); ++it)
                checks.push_back((*it)->copy());
        }


        {
            bool foundError = false;
            tok = check->parse(*tok, foundError, checks);
            std::list<ExecutionPath *>::iterator it;
            for (it = checks.begin(); it != checks.end();)
            {
                if ((*it)->bailOut())
                {
                    delete *it;
                    it = checks.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            if (checks.empty())
                return 0;
            else if (foundError)
                return tok;
        }

        // return ends all execution paths
        if (tok->str() == "return")
        {
            ExecutionPath::bailOut(checks);
        }
    }
    return 0;
}

