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
