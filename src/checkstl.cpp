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
            else if (Token::Match(tok2, "%var% . insert|erase ( %varid%", iteratorId))
            {
                if (tok2->varId() != containerId && tok2->tokAt(5)->str() != ".")
                    iteratorsError(tok2, tok->strAt(2), tok2->str());
                else if (tok2->strAt(2) == std::string("erase"))
                    validIterator = false;

                tok2 = tok2->tokAt(4);
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
    // Error if it contains "erase(it)" but neither "break;" nor "it="
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
        else if (Token::Match(tok, "break|return|goto") || Token::simpleMatch(tok, (it->str() + " =").c_str()))
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
    reportError(tok, Severity::error, "erase", "Dangerous usage of erase\nAfter erase has been used the iterator may be invalid so dereferencing it or comparing it with other iterator is invalid.");
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
        if (Token::simpleMatch(tok, "vector <"))
        {
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
            if (Token::Match(tok, "> :: iterator|const_iterator %var% =|;"))
            {
                const unsigned int iteratorid(tok->tokAt(3)->varId());
                if (iteratorid == 0)
                    continue;

                if (iteratorDeclaredInsideLoop && tok->tokAt(4)->str() == "=")
                {
                    // skip "> :: iterator|const_iterator"
                    tok = tok->tokAt(3);
                }

                std::string vectorname;
                int indent = 0;
                bool invalidIterator = false;
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
                            else if (Token::Match(tok3, "%varid% . push_front|push_back (", vectorid))
                            {
                                pushback = tok3;
                            }
                        }

                        if (pushback)
                            pushbackError(pushback, tok2->strAt(0));
                    }

                    // Assigning iterator..
                    if (Token::Match(tok2, "%varid% =", iteratorid))
                    {
                        if (Token::Match(tok2->tokAt(2), "%var% . begin ( )"))
                            vectorname = tok2->strAt(2);
                        else
                            vectorname = "";
                        invalidIterator = false;
                    }

                    // push_back on vector..
                    if (vectorname.size() && Token::Match(tok2, (vectorname + " . push_front|push_back").c_str()))
                        invalidIterator = true;

                    // Using invalid iterator..
                    if (invalidIterator)
                    {
                        if (Token::Match(tok2, "++|--|*|+|-|(|, %varid%", iteratorid))
                            pushbackError(tok2, tok2->strAt(1));
                        if (Token::Match(tok2, "%varid% ++|--|+|-", iteratorid))
                            pushbackError(tok2, tok2->str());
                    }
                }
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::pushbackError(const Token *tok, const std::string &iterator_name)
{
    reportError(tok, Severity::error, "pushback", "After push_back or push_front, the iterator '" + iterator_name + "' may be invalid");
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "pushback", "Invalid pointer '" + pointer_name + "' after push_back / push_front");
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
