/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"
#include <memory>
#include <set>
#include <iterator>
#include <iostream>



// default : bail out if the condition is has variable handling
bool ExecutionPath::parseCondition(const Token &tok, std::list<ExecutionPath *> & checks)
{
    unsigned int parlevel = 0;
    for (const Token *tok2 = &tok; tok2; tok2 = tok2->next()) {
        if (tok2->str() == "(")
            ++parlevel;
        else if (tok2->str() == ")") {
            if (parlevel == 0)
                break;
            --parlevel;
        } else if (Token::Match(tok2, "[;{}]"))
            break;
        if (tok2->varId() != 0) {
            bailOutVar(checks, tok2->varId());
        }
    }

    for (std::list<ExecutionPath *>::iterator it = checks.begin(); it != checks.end();) {
        if ((*it)->varId > 0 && (*it)->numberOfIf >= 1) {
            delete *it;
            checks.erase(it++);
        } else {
            ++it;
        }
    }


    return false;
}


void ExecutionPath::print() const
{
    std::cout << " varId=" << varId
              << " numberOfIf=" << numberOfIf
              << "\n";
}

// I use this function when debugging ExecutionPaths with GDB
/*
static void printchecks(const std::list<ExecutionPath *> &checks)
{
    for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it)
        (*it)->print();
}
*/



/**
 * @brief Parse If/Switch body recursively.
 * @param tok First token in body.
 * @param checks The current checks
 * @param newchecks new checks
 * @param countif The countif set - count number of if for each execution path
 */
static void parseIfSwitchBody(const Token * const tok,
                              const std::list<ExecutionPath *> &checks,
                              std::list<ExecutionPath *> &newchecks,
                              std::set<unsigned int> &countif)
{
    std::set<unsigned int> countif2;
    std::list<ExecutionPath *> c;
    if (!checks.empty()) {
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            if ((*it)->numberOfIf == 0)
                c.push_back((*it)->copy());
            if ((*it)->varId != 0)
                countif2.insert((*it)->varId);
        }
    }
    ExecutionPath::checkScope(tok, c);
    while (!c.empty()) {
        if (c.back()->varId == 0) {
            delete c.back();
            c.pop_back();
            continue;
        }

        bool duplicate = false;
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it) {
            if (*(*it) == *c.back() && (*it)->numberOfIf == c.back()->numberOfIf) {
                duplicate = true;
                countif2.erase((*it)->varId);
                break;
            }
        }
        if (!duplicate)
            newchecks.push_back(c.back());
        else
            delete c.back();
        c.pop_back();
    }

    // Add countif2 ids to countif.. countif.
    countif.insert(countif2.begin(), countif2.end());
}


void ExecutionPath::checkScope(const Token *tok, std::list<ExecutionPath *> &checks)
{
    if (!tok || tok->str() == "}" || checks.empty())
        return;

    const std::auto_ptr<ExecutionPath> check(checks.front()->copy());

    for (; tok; tok = tok->next()) {
        // might be a noreturn function..
        if (Token::simpleMatch(tok->tokAt(-2), ") ; }") &&
            Token::Match(tok->linkAt(-2)->tokAt(-2), "[;{}] %name% (") &&
            tok->linkAt(-2)->previous()->varId() == 0) {
            ExecutionPath::bailOut(checks);
            return;
        }

        // ({ is not handled well
        if (Token::simpleMatch(tok, "( {")) {
            ExecutionPath::bailOut(checks);
            return;
        }

        if (Token::simpleMatch(tok, "union {")) {
            tok = tok->next()->link();
            continue;
        }

        if (tok->str() == "}")
            return;

        if (tok->str() == "break") {
            ExecutionPath::bailOut(checks);
            return;
        }

        if (Token::simpleMatch(tok, "while (")) {
            // parse condition
            if (checks.size() > 10 || check->parseCondition(*tok->tokAt(2), checks)) {
                ExecutionPath::bailOut(checks);
                return;
            }

            // skip "while (fgets()!=NULL)"
            if (Token::simpleMatch(tok, "while ( fgets (")) {
                const Token *tok2 = tok->linkAt(3);
                if (Token::simpleMatch(tok2, ") ) {")) {
                    tok = tok2->linkAt(2);
                    if (!tok)
                        break;
                    continue;
                }
            }
        }

        // goto/setjmp/longjmp => bailout
        else if (Token::Match(tok, "goto|setjmp|longjmp")) {
            ExecutionPath::bailOut(checks);
            return;
        }

        // ?: => bailout
        if (tok->str() == "?") {
            for (const Token *tok2 = tok; tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId() > 0)
                    ExecutionPath::bailOutVar(checks, tok2->varId());
            }
        }

        // for/while/switch/do .. bail out
        else if (Token::Match(tok, "for|while|switch|do")) {
            // goto {
            const Token *tok2 = tok->next();
            if (tok2 && tok2->str() == "(")
                tok2 = tok2->link();
            if (tok2 && tok2->str() == ")")
                tok2 = tok2->next();
            if (!tok2 || tok2->str() != "{") {
                ExecutionPath::bailOut(checks);
                return;
            }

            if (tok->str() == "switch") {
                // parse condition
                if (checks.size() > 10 || check->parseCondition(*tok->next(), checks)) {
                    ExecutionPath::bailOut(checks);
                    return;
                }

                // what variable ids should the if be counted for?
                std::set<unsigned int> countif;

                std::list<ExecutionPath *> newchecks;

                for (const Token* tok3 = tok2->next(); tok3; tok3 = tok3->next()) {
                    if (tok3->str() == "{")
                        tok3 = tok3->link();
                    else if (tok3->str() == "}")
                        break;
                    else if (tok3->str() == "case" &&
                             !Token::Match(tok3, "case %num% : ; case")) {
                        parseIfSwitchBody(tok3, checks, newchecks, countif);
                    }
                }

                // Add newchecks to checks..
                std::copy(newchecks.begin(), newchecks.end(), std::back_inserter(checks));

                // Increase numberOfIf
                std::list<ExecutionPath *>::iterator it;
                for (it = checks.begin(); it != checks.end(); ++it) {
                    if (countif.find((*it)->varId) != countif.end())
                        (*it)->numberOfIf++;
                }
            }
            // no switch
            else {
                for (const Token *tok3 = tok; tok3 && tok3 != tok2; tok3 = tok3->next()) {
                    if (tok3->varId())
                        ExecutionPath::bailOutVar(checks, tok3->varId());
                }

                // it is not certain that a for/while will be executed:
                for (std::list<ExecutionPath *>::iterator it = checks.begin(); it != checks.end();) {
                    if ((*it)->numberOfIf > 0) {
                        delete *it;
                        checks.erase(it++);
                    } else
                        ++it;
                }

                // #2231 - loop body only contains a conditional initialization..
                if (Token::simpleMatch(tok2->next(), "if (")) {
                    // Start { for the if block
                    const Token *tok3 = tok2->linkAt(2);
                    if (Token::simpleMatch(tok3,") {")) {
                        tok3 = tok3->next();

                        // End } for the if block
                        const Token *tok4 = tok3->link();
                        if (Token::Match(tok3, "{ %name% =") &&
                            Token::simpleMatch(tok4, "} }") &&
                            Token::simpleMatch(tok4->tokAt(-2), "break ;")) {
                            // Is there a assignment and then a break?
                            const Token *t = Token::findsimplematch(tok3, ";");
                            if (t && t->tokAt(3) == tok4) {
                                for (std::list<ExecutionPath *>::iterator it = checks.begin(); it != checks.end(); ++it) {
                                    if ((*it)->varId == tok3->next()->varId()) {
                                        (*it)->numberOfIf++;
                                        break;
                                    }
                                }
                                tok = tok2->link();
                                continue;
                            }
                        }
                    }
                }

                // parse loop bodies
                check->parseLoopBody(tok2->next(), checks);
            }

            // skip { .. }
            tok2 = tok2->link();

            // if "do { .. } while ( .." , goto end of while..
            if (Token::simpleMatch(tok, "do {") && Token::simpleMatch(tok2, "} while ("))
                tok2 = tok2->linkAt(2);

            // bail out all variables if the scope contains a "return"
            // bail out all variables used in this for/while/switch/do
            for (; tok && tok != tok2; tok = tok->next()) {
                if (tok->str() == "return")
                    ExecutionPath::bailOut(checks);
                if (tok->varId())
                    ExecutionPath::bailOutVar(checks, tok->varId());
            }

            continue;
        }

        // bailout used variables in '; FOREACH ( .. ) { .. }'
        else if (tok->str() != "if" && Token::Match(tok->previous(), "[;{}] %name% (")) {
            // goto {
            const Token *tok2 = tok->next()->link()->next();
            if (tok2 && tok2->str() == "{") {
                // goto "}"
                tok2 = tok2->link();

                // bail out all variables used in "{ .. }"
                for (; tok && tok != tok2; tok = tok->next()) {
                    if (tok->varId())
                        ExecutionPath::bailOutVar(checks, tok->varId());
                }
                if (!tok)
                    break;
            }
        }

        // .. ) { ... }  => bail out
        if (tok->str() == ")" && tok->next() && tok->next()->str() == "{") {
            ExecutionPath::bailOut(checks);
            return;
        }

        if ((tok->str() == "abort" || tok->str() == "exit") &&
            tok->next() && tok->next()->str() == "(") {
            ExecutionPath::bailOut(checks);
            return;
        }

        // don't parse into "struct type { .."
        if (Token::Match(tok, "struct|union|class %type% {|:")) {
            while (tok && tok->str() != "{" && tok->str() != ";")
                tok = tok->next();
            tok = tok ? tok->link() : 0;
            if (!tok) {
                ExecutionPath::bailOut(checks);
                return;
            }
        }

        if (Token::simpleMatch(tok, "= {")) {
            // GCC struct initialization.. bail out
            if (Token::Match(tok->tokAt(2), ". %name% =")) {
                ExecutionPath::bailOut(checks);
                return;
            }

            const Token * const end = tok->next()->link();
            while (tok && tok != end) {
                if (Token::Match(tok, "[{,] & %var% [,}]"))
                    ExecutionPath::bailOutVar(checks, tok->tokAt(2)->varId());
                tok = tok->next();
            }
            if (!tok) {
                ExecutionPath::bailOut(checks);
                return;
            }
            continue;
        }

        // ; { ... }
        if (Token::Match(tok->previous(), "[;{}:] {")) {
            ExecutionPath::checkScope(tok->next(), checks);
            tok = tok->link();
            continue;
        }

        if (tok->str() == "if" && tok->next() && tok->next()->str() == "(") {
            // what variable ids should the numberOfIf be counted for?
            std::set<unsigned int> countif;

            std::list<ExecutionPath *> newchecks;
            while (tok->str() == "if" && tok->next() && tok->next()->str() == "(") {
                // goto "("
                tok = tok->next();

                // parse condition
                if (checks.size() > 10 || check->parseCondition(*tok->next(), checks)) {
                    ExecutionPath::bailOut(checks);
                    ExecutionPath::bailOut(newchecks);
                    return;
                }

                // goto ")"
                tok = tok->link();

                // goto "{"
                tok = tok->next();

                if (!tok || tok->str() != "{") {
                    ExecutionPath::bailOut(checks);
                    ExecutionPath::bailOut(newchecks);
                    return;
                }

                // Recursively check into the if ..
                parseIfSwitchBody(tok->next(), checks, newchecks, countif);

                // goto "}"
                tok = tok->link();

                // there is no else => break out
                if (!tok->next() || tok->next()->str() != "else")
                    break;

                // parse next "if"..
                tok = tok->tokAt(2);
                if (!tok) {
                    ExecutionPath::bailOut(newchecks);
                    return;
                }
                if (tok->str() == "if")
                    continue;

                // there is no "if"..
                ExecutionPath::checkScope(tok->next(), checks);
                tok = tok->link();
                if (!tok) {
                    ExecutionPath::bailOut(newchecks);
                    return;
                }
            }

            // Add newchecks to checks..
            std::copy(newchecks.begin(), newchecks.end(), std::back_inserter(checks));

            // Increase numberOfIf
            std::list<ExecutionPath *>::iterator it;
            for (it = checks.begin(); it != checks.end(); ++it) {
                if (countif.find((*it)->varId) != countif.end())
                    (*it)->numberOfIf++;
            }

            // Delete checks that have numberOfIf >= 2
            for (it = checks.begin(); it != checks.end();) {
                if ((*it)->varId > 0 && (*it)->numberOfIf >= 2) {
                    delete *it;
                    checks.erase(it++);
                } else {
                    ++it;
                }
            }
        }

        tok = check->parse(*tok, checks);
        if (!tok || checks.empty())
            return;

        // return/throw ends all execution paths
        if (tok->str() == "return" ||
            tok->str() == "throw" ||
            tok->str() == "continue" ||
            tok->str() == "break") {
            ExecutionPath::bailOut(checks);
        }
    }
}

void checkExecutionPaths(const SymbolDatabase *symbolDatabase, ExecutionPath *c)
{
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eFunction || !i->classStart)
            continue;

        // Check function
        std::list<ExecutionPath *> checks;
        checks.push_back(c->copy());
        ExecutionPath::checkScope(i->classStart, checks);

        c->end(checks, i->classEnd);

        // Cleanup
        while (!checks.empty()) {
            delete checks.back();
            checks.pop_back();
        }
    }
}
