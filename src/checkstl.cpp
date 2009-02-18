/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
#include "errorlogger.h"
#include "token.h"
#include "tokenize.h"


CheckStl::CheckStl(const Tokenizer *tokenizer, ErrorLogger *errorLogger)
        : _tokenizer(tokenizer), _errorLogger(errorLogger)
{

}

CheckStl::~CheckStl()
{

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
            _errorLogger->iteratorUsage(_tokenizer, tok, tok->tokAt(2)->str(), tok->tokAt(10)->str());
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
            _errorLogger->iteratorUsage(_tokenizer, tok, tok->tokAt(2)->str(), tok->tokAt(12)->str());
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
                _errorLogger->stlOutOfBounds(_tokenizer, tok, "When " + num->str() + " == size(), " + pattern);
            }

            tok = tok->next();
        }
    }

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
            if (indentlevel < 0)
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
        _errorLogger->erase(_tokenizer, tok2);
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
                            _errorLogger->pushback(_tokenizer, tok2, iteratorname);
                        if (Token::Match(tok2, (iteratorname + " ++|--|+|-").c_str()))
                            _errorLogger->pushback(_tokenizer, tok2, iteratorname);
                    }
                }
            }
        }
    }
}


