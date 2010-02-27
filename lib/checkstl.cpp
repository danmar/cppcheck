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

#include "checkstl.h"
#include "tokenize.h"
#include "token.h"



// Register this check class (by creating a static instance of it)
namespace
{
CheckStl instance;
}


// Error message for bad iterator usage..
void CheckStl::iteratorsError(const Token *tok, const std::string &container1, const std::string &container2)
{
    reportError(tok, Severity::error, "iterators", "Same iterator is used with both " + container1 + " and " + container2);
}

// Error message used when dereferencing an iterator that has been erased..
void CheckStl::dereferenceErasedError(const Token *tok, const std::string &itername)
{
    reportError(tok, Severity::error, "eraseDereference", "Dereferenced iterator '" + itername + "' has been erased");
}

void CheckStl::iterators()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::Match(tok, "%var% = %var% . begin ( ) ;|+"))
            continue;

        const unsigned int iteratorId(tok->varId());
        const unsigned int containerId(tok->tokAt(2)->varId());
        if (iteratorId == 0 || containerId == 0)
            continue;

        bool validIterator = true;
        for (const Token *tok2 = tok->tokAt(7); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "}")
                break;

            if (Token::Match(tok2, "%varid% != %var% . end ( )", iteratorId) && tok2->tokAt(2)->varId() != containerId)
            {
                iteratorsError(tok2, tok->strAt(2), tok2->strAt(2));
                tok2 = tok2->tokAt(6);
            }
            else if (Token::Match(tok2, "%var% . insert|erase ( %varid% )|,", iteratorId))
            {
                if (tok2->varId() != containerId && tok2->tokAt(5)->str() != ".")
                    iteratorsError(tok2, tok->strAt(2), tok2->str());
                else if (tok2->strAt(2) == std::string("erase"))
                    validIterator = false;

                tok2 = tok2->tokAt(4);
            }
            else if (Token::Match(tok2, "%varid% = %var% . erase (", iteratorId))
            {
                validIterator = true;
                tok2 = tok2->tokAt(5)->link();
                if (!tok2)
                    break;
            }
            else if (!validIterator && Token::Match(tok2, "* %varid%", iteratorId))
            {
                dereferenceErasedError(tok2, tok2->strAt(1));
                tok2 = tok2->next();
            }
            else if (!validIterator && Token::Match(tok2, "%varid% . %var%", iteratorId))
            {
                dereferenceErasedError(tok2, tok2->strAt(0));
                tok2 = tok2->tokAt(2);
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::mismatchingContainersError(const Token *tok)
{
    reportError(tok, Severity::error, "mismatchingContainers", "mismatching containers");
}

void CheckStl::mismatchingContainers()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() != "std")
            continue;

        if (Token::Match(tok, "std :: find|find_if|count|transform|replace|replace_if|sort ( %var% . begin|rbegin ( ) , %var% . end|rend ( ) ,"))
        {
            if (tok->tokAt(4)->str() != tok->tokAt(10)->str())
            {
                mismatchingContainersError(tok);
            }
        }
    }
}


void CheckStl::stlOutOfBounds()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::simpleMatch(tok, "for ("))
            continue;

        unsigned int indent = 0;
        for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
        {

            if (tok2->str() == "(")
                ++indent;

            else if (tok2->str() == ")")
            {
                if (indent == 0)
                    break;
                --indent;
            }

            if (Token::Match(tok2, "; %var% <= %var% . size ( ) ;"))
            {
                unsigned int indent2 = 0;
                unsigned int numId = tok2->tokAt(1)->varId();
                unsigned int varId = tok2->tokAt(3)->varId();
                for (const Token *tok3 = tok2->tokAt(8); tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "{")
                        ++indent2;
                    else if (tok3->str() == "}")
                    {
                        if (indent2 <= 1)
                            break;
                        --indent2;
                    }
                    else if (tok3->varId() == varId)
                    {
                        if (Token::simpleMatch(tok3->next(), ". size ( )"))
                            break;
                        else if (Token::Match(tok3->next(), "[ %varid% ]", numId))
                            stlOutOfBoundsError(tok3, tok3->tokAt(2)->str(), tok3->str());
                    }
                }
                break;
            }
        }
    }
}

// Error message for bad iterator usage..
void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var)
{
    reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + "[" + num + "] is out of bounds");
}



void CheckStl::erase()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "for ("))
        {
            for (const Token *tok2 = tok->tokAt(2); tok2 && tok2->str() != ";"; tok2 = tok2->next())
            {
                if (Token::Match(tok2, "%var% = %var% . begin ( ) ; %var% != %var% . end ( ) ") &&
                    tok2->str() == tok2->tokAt(8)->str() &&
                    tok2->tokAt(2)->str() == tok2->tokAt(10)->str())
                {
                    eraseCheckLoop(tok2);
                    break;
                }
            }
        }

        if (Token::Match(tok, "while ( %var% != %var% . end ( )"))
        {
            eraseCheckLoop(tok->tokAt(2));
        }
    }
}

void CheckStl::eraseCheckLoop(const Token *it)
{
    const Token *tok = it;

    // Search for the start of the loop body..
    int indentlevel = 1;
    while (indentlevel > 0 && 0 != (tok = tok->next()))
    {
        if (tok->str() == "(")
            ++indentlevel;
        else if (tok->str() == ")")
            --indentlevel;
    }

    if (! Token::simpleMatch(tok, ") {"))
        return;

    // Parse loop..
    // Error if it contains "erase(it)" but neither "break;", "=it" nor "it="
    indentlevel = 0;
    const Token *tok2 = 0;
    while (0 != (tok = tok->next()))
    {
        if (tok->str() == "{")
            ++indentlevel;
        else if (tok->str() == "}")
        {
            --indentlevel;
            if (indentlevel <= 0)
                break;
        }
        else if (Token::Match(tok, "break|return|goto") ||
                 Token::simpleMatch(tok, (it->str() + " =").c_str()) ||
                 Token::simpleMatch(tok, ("= " + it->str()).c_str()))
        {
            tok2 = 0;
            break;
        }
        else if (Token::simpleMatch(tok, ("erase ( " + it->str() + " )").c_str()))
            tok2 = tok;
    }

    // Write error message..
    if (tok2)
        eraseError(tok2);
}


// Error message for bad iterator usage..
void CheckStl::eraseError(const Token *tok)
{
    reportError(tok, Severity::error, "erase", "Dangerous iterator usage. After erase the iterator is invalid so dereferencing it or comparing it with another iterator is invalid.");
}



void CheckStl::pushback()
{
    // Pointer can become invalid after push_back or push_front..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var% = & %var% ["))
        {
            const unsigned int pointerId(tok->varId());
            const unsigned int containerId(tok->tokAt(3)->varId());
            if (pointerId == 0 || containerId == 0)
                continue;

            int indent = 0;
            bool invalidPointer = false;
            for (const Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{" || tok2->str() == "(")
                    ++indent;
                else if (tok2->str() == "}" || tok2->str() == ")")
                {
                    if (indent == 0 && Token::simpleMatch(tok2, ") {"))
                        tok2 = tok2->next();
                    else
                        --indent;
                }

                // push_back on vector..
                if (Token::Match(tok2, "%varid% . push_front|push_back", containerId))
                    invalidPointer = true;

                // Using invalid pointer..
                if (invalidPointer && tok2->varId() == pointerId)
                {
                    if (tok2->previous()->str() == "*")
                        invalidPointerError(tok2, tok2->str());
                    else if (tok2->next()->str() == ".")
                        invalidPointerError(tok2, tok2->str());
                    break;
                }
            }
        }
    }

    // Iterator becomes invalid after push_back or push_front..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::simpleMatch(tok, "vector <"))
            continue;

        // if iterator declaration inside for() loop
        bool iteratorDeclaredInsideLoop = false;
        if ((tok->tokAt(-2) && Token::simpleMatch(tok->tokAt(-2), "for (")) ||
            (tok->tokAt(-4) && Token::simpleMatch(tok->tokAt(-4), "for ( std ::")))
        {
            iteratorDeclaredInsideLoop = true;
        }

        while (tok && tok->str() != ">")
            tok = tok->next();
        if (!tok)
            break;
        if (!Token::Match(tok, "> :: iterator|const_iterator %var% =|;"))
            continue;

        const unsigned int iteratorid(tok->tokAt(3)->varId());
        if (iteratorid == 0)
            continue;

        if (iteratorDeclaredInsideLoop && tok->tokAt(4)->str() == "=")
        {
            // skip "> :: iterator|const_iterator"
            tok = tok->tokAt(3);
        }

        unsigned int vectorid = 0;
        int indent = 0;
        std::string invalidIterator;
        for (const Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{" || tok2->str() == "(")
                ++indent;
            else if (tok2->str() == "}" || tok2->str() == ")")
            {
                if (indent == 0 && Token::simpleMatch(tok2, ") {"))
                    tok2 = tok2->next();
                else
                    --indent;
            }

            // Using push_back or push_front inside a loop..
            if (Token::Match(tok2, "for ("))
            {
                tok2 = tok2->tokAt(2);
            }

            if (Token::Match(tok2, "%varid% = %var% . begin ( ) ; %varid% != %var% . end ( ) ; ++| %varid% ++| ) {", iteratorid))
            {
                const unsigned int vectorid(tok2->tokAt(2)->varId());
                if (vectorid == 0)
                    continue;

                const Token *pushback = 0;
                unsigned int indent3 = 0;
                for (const Token *tok3 = tok2->tokAt(20); tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "{")
                        ++indent3;
                    else if (tok3->str() == "}")
                    {
                        if (indent3 <= 1)
                            break;
                        --indent3;
                    }
                    else if (tok3->str() == "break")
                    {
                        pushback = 0;
                        break;
                    }
                    else if (Token::Match(tok3, "%varid% . push_front|push_back|insert (", vectorid))
                    {
                        pushback = tok3->tokAt(2);
                    }
                }

                if (pushback)
                    invalidIteratorError(pushback, pushback->str(), tok2->strAt(0));
            }

            // Assigning iterator..
            if (Token::Match(tok2, "%varid% =", iteratorid))
            {
                if (Token::Match(tok2->tokAt(2), "%var% . begin|end|rbegin|rend ( )"))
                {
                    vectorid = tok2->tokAt(2)->varId();
                    tok2 = tok2->tokAt(6);
                }
                else
                {
                    vectorid = 0;
                }
                invalidIterator = "";
            }

            // push_back on vector..
            if (vectorid > 0 && Token::Match(tok2, "%varid% . push_front|push_back|insert (", vectorid))
            {
                if (!invalidIterator.empty() && Token::Match(tok2->tokAt(2), "insert ( %varid% ,", iteratorid))
                {
                    invalidIteratorError(tok2, invalidIterator, tok2->strAt(4));
                    break;
                }

                invalidIterator = tok2->strAt(2);
                if (!iteratorDeclaredInsideLoop)
                {
                    tok2 = tok2->tokAt(3)->link();
                    if (!tok2)
                        break;
                }
            }

            // Using invalid iterator..
            if (!invalidIterator.empty())
            {
                if (Token::Match(tok2, "++|--|*|+|-|(|,|=|!= %varid%", iteratorid))
                    invalidIteratorError(tok2, invalidIterator, tok2->strAt(1));
                if (Token::Match(tok2, "%varid% ++|--|+|-", iteratorid))
                    invalidIteratorError(tok2, invalidIterator, tok2->str());
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name)
{
    reportError(tok, Severity::error, "invalidIterator", "After " + func + ", the iterator '" + iterator_name + "' may be invalid");
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "invalidPointer", "Invalid pointer '" + pointer_name + "' after push_back / push_front");
}




void CheckStl::stlBoundries()
{
    // containers (not the vector)..
    static const char STL_CONTAINER_LIST[] = "bitset|deque|list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set";

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Declaring iterator..
        const std::string checkStr = (std::string(STL_CONTAINER_LIST) + " <");
        if (Token::Match(tok, checkStr.c_str()))
        {
            const std::string container_name(tok->strAt(0));
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;

            if (Token::Match(tok, "> :: iterator|const_iterator %var% =|;"))
            {
                const unsigned int iteratorid(tok->tokAt(3)->varId());
                if (iteratorid == 0)
                    continue;

                // Using "iterator < ..." is not allowed
                unsigned int indentlevel = 0;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel == 0)
                            break;
                        --indentlevel;
                    }
                    else if (Token::Match(tok2, "!!* %varid% <", iteratorid))
                    {
                        stlBoundriesError(tok2, container_name);
                    }
                }
            }
        }
    }
}

// Error message for bad boundry usage..
void CheckStl::stlBoundriesError(const Token *tok, const std::string &container_name)
{
    reportError(tok, Severity::error, "stlBoundries", container_name + " range check should use != and not < since the order of the pointers isn't guaranteed");
}



void CheckStl::find()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() != ";")
            continue;
        if (!Token::Match(tok->next(), "%var% = std :: find ("))
            continue;
        const unsigned int iteratorid = tok->next()->varId();
        if (iteratorid == 0)
            continue;
        tok = tok->tokAt(6)->link();
        if (!tok)
            break;
        for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{" || tok2->str() == "}" || tok2->str() == "(" || tok2->str() == ")")
                break;
            if (tok2->varId() == iteratorid && Token::simpleMatch(tok2->previous(), "*"))
                findError(tok2);
        }
    }
}


void CheckStl::findError(const Token *tok)
{
    reportError(tok, Severity::error, "stlfind", "dangerous usage of find result");
}



void CheckStl::if_find()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "if ( !| %var% . find ( %any% ) )"))
        {
            // goto %var%
            tok = tok->tokAt(2);
            if (!tok->isName())
                tok = tok->next();

            const unsigned int varid = tok->varId();
            if (varid > 0)
            {
                // Locate variable declaration..
                const Token * const decl = Token::findmatch(_tokenizer->tokens(), "%varid%", varid);
                if (Token::Match(decl->tokAt(-4), ",|;|( std :: string"))
                    if_findError(tok, true);
                else if (Token::Match(decl->tokAt(-7), ",|;|( std :: %type% < %type% >"))
                    if_findError(tok, false);
            }
        }
    }
}


void CheckStl::if_findError(const Token *tok, bool str)
{
    if (str)
        reportError(tok, Severity::possibleStyle, "stlIfStrFind", "Suspicious condition. string::find will return 0 if the string is found at position 0. If this is what you want to check then string::compare is a faster alternative because it doesn't scan through the string.");
    else
        reportError(tok, Severity::style, "stlIfFind", "Suspicious condition. The result of find is an iterator, but it is not properly checked.");
}



bool CheckStl::isStlContainer(const Token *tok)
{
    // check if this token is defined
    if (tok->varId())
    {
        // find where this token is defined
        const Token *type = Token::findmatch(_tokenizer->tokens(), "%varid%", tok->varId());

        // find where this tokens type starts
        while (type->previous() && !Token::Match(type->previous(), "[;{]"))
            type = type->previous();

        // discard namespace if supplied
        if (Token::Match(type, "std ::"))
            type = type->next()->next();

        // all possible stl containers
        static const char STL_CONTAINER_LIST[] = "bitset|deque|list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|vector";

        // container template string
        const std::string checkStr(std::string(STL_CONTAINER_LIST) + " <");

        // check if it's an stl template
        if (Token::Match(type, checkStr.c_str()))
            return true;
    }

    return false;
}

void CheckStl::size()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var% . size ( )"))
        {
            if (Token::Match(tok->tokAt(5), "==|!=|> 0"))
            {
                if (isStlContainer(tok))
                    sizeError(tok);
            }
            else if ((tok->tokAt(5)->str() == ")" ||
                      tok->tokAt(5)->str() == "&&" ||
                      tok->tokAt(5)->str() == "||" ||
                      tok->tokAt(5)->str() == "!") &&
                     (tok->tokAt(-1)->str() == "(" ||
                      tok->tokAt(-1)->str() == "&&" ||
                      tok->tokAt(-1)->str() == "||" ||
                      tok->tokAt(-1)->str() == "!"))
            {
                if (tok->tokAt(-1)->str() == "(" &&
                    tok->tokAt(5)->str() == ")")
                {
                    // check for passing size to function call
                    if (Token::Match(tok->tokAt(-2), "if|while"))
                    {
                        if (isStlContainer(tok))
                            sizeError(tok);
                    }
                }
                else if (isStlContainer(tok))
                    sizeError(tok);
            }
        }
    }
}

void CheckStl::sizeError(const Token *tok)
{
    const std::string varname(tok ? tok->str().c_str() : "list");
    const bool verbose(_settings ? _settings->_verbose : true);
    reportError(tok, Severity::possibleStyle, "stlSize", "Use " + varname + ".empty() instead of " + varname + ".size() to guarantee fast code." + (verbose ? " size() can take linear time but empty() is guaranteed to take constant time." : ""));
}
