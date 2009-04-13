/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
    reportError(tok, "error", "iterators", "Same iterator is used with both " + container1 + " and " + container2);
}


void CheckStl::iterators()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // it = foo.begin(); it != bar.end()
        if (Token::Match(tok, "%var% = %var% . begin ( ) ; %var% != %var% . end ( ) ;"))
        {
            // Different iterators..
            if (tok->str() != tok->tokAt(8)->str())
                continue;
            // Same container..
            if (tok->tokAt(2)->str() == tok->tokAt(10)->str())
                continue;
            iteratorsError(tok, tok->strAt(2), tok->strAt(10));
        }

        // it = foo.begin();
        // while (it != bar.end())
        if (Token::Match(tok, "%var% = %var% . begin ( ) ; while ( %var% != %var% . end ( )"))
        {
            // Different iterators..
            if (tok->str() != tok->tokAt(10)->str())
                continue;
            // Same container..
            if (tok->tokAt(2)->str() == tok->tokAt(12)->str())
                continue;

            iteratorsError(tok, tok->strAt(2), tok->strAt(12));
        }
    }
}


void CheckStl::stlOutOfBounds()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::simpleMatch(tok, "for ("))
            continue;


        int indent = 1;
        tok = tok->tokAt(2);
        const Token *num = 0;
        const Token *var = 0;
        while (tok)
        {

            if (tok->str() == "(")
                ++indent;
            if (tok->str() == ")")
            {
                --indent;
                if (indent == 0)
                    break;
            }

            if (Token::Match(tok, "; %var% <= %var% . size ( ) ;"))
            {
                num = tok->tokAt(1);
                var = tok->tokAt(3);
            }

            tok = tok->next();
        }

        if (!tok)
            return;

        tok = tok->next();
        if (!num || tok->str() != "{")
            continue;

        std::string pattern = var->str() + " [ " + num->str() + " ]";
        while (tok)
        {

            if (tok->str() == "{")
                ++indent;
            if (tok->str() == "}")
            {
                --indent;
                if (indent == 0)
                    break;
            }


            if (Token::Match(tok, pattern.c_str()))
            {
                stlOutOfBoundsError(tok, num->str(), var->str());
            }

            tok = tok->next();
        }
    }
}

// Error message for bad iterator usage..
void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var)
{
    reportError(tok, "error", "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + "[" + num + "] is out of bounds");
}



void CheckStl::erase()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "for ("))
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
    reportError(tok, "error", "erase", "Dangerous usage of erase");
}



void CheckStl::pushback()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "vector <"))
        {
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;
            if (Token::Match(tok, "> :: iterator|const_iterator %var% =|;"))
            {
                const std::string iteratorname(tok->strAt(3));
                std::string vectorname;
                int indent = 0;
                bool invalidIterator = false;
                for (const Token *tok2 = tok;indent >= 0 && tok2; tok2 = tok2->next())
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

                    // Assigning iterator..
                    if (Token::Match(tok2, (iteratorname + " = %var% . begin ( )").c_str()))
                    {
                        vectorname = tok2->strAt(2);
                        invalidIterator = false;
                    }

                    // push_back on vector..
                    if (vectorname.size() && Token::Match(tok2, (vectorname + " . push_front|push_back").c_str()))
                        invalidIterator = true;

                    // Using invalid iterator..
                    if (invalidIterator)
                    {
                        if (Token::Match(tok2, ("++|--|*|+|-|(|, " + iteratorname).c_str()))
                            pushbackError(tok2, iteratorname);
                        if (Token::Match(tok2, (iteratorname + " ++|--|+|-").c_str()))
                            pushbackError(tok2, iteratorname);
                    }
                }
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::pushbackError(const Token *tok, const std::string &iterator_name)
{
    reportError(tok, "error", "pushback", "After push_back or push_front, the iterator '" + iterator_name + "' may be invalid");
}




void CheckStl::stlBoundries()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "for ("))
        {
            for (const Token *tok2 = tok->tokAt(2); tok2 && tok2->str() != ";"; tok2 = tok2->next())
            {
                if (Token::Match(tok2, "%var% = %var% . begin ( ) ; %var% < %var% . end ( ) ") &&
                    tok2->str() == tok2->tokAt(8)->str() &&
                    tok2->tokAt(2)->str() == tok2->tokAt(10)->str())
                {
                    stlBoundriesError(tok2);
                    break;
                }
            }
        }

        if (Token::Match(tok, "while ( %var% < %var% . end ( )"))
        {
            stlBoundriesError(tok);
        }
    }
}

// Error message for bad boundry usage..
void CheckStl::stlBoundriesError(const Token *tok)
{
    reportError(tok, "error", "stlBoundries", "STL range check should be using != and not < since the order of the pointers isn't guaranteed");
}
