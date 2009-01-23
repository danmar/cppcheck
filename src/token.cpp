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

#include "token.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <cctype>

Token::Token() :
        _str(""),
        _cstr(0),
        _isName(false),
        _isNumber(false),
        _isBoolean(false),
        _varId(0),
        _next(0),
        _previous(0),
        _fileIndex(0),
        _linenr(0)
{
}

Token::~Token()
{
    std::free(_cstr);
}

void Token::str(const char s[])
{
    _str = s;
    std::free(_cstr);
    _cstr = strdup(s);
    _isName = bool(_str[0] == '_' || std::isalpha(_str[0]));
    _isNumber = bool(std::isdigit(_str[0]) != 0);
    if (_str == "true" || _str == "false")
        _isBoolean = true;
    else
        _isBoolean = false;

    _varId = 0;
}

void Token::deleteNext()
{
    Token *n = _next;
    _next = n->next();
    delete n;
    if (_next)
        _next->previous(this);
}

const Token *Token::tokAt(int index) const
{
    const Token *tok = this;
    while (index > 0 && tok)
    {
        tok = tok->next();
        --index;
    }
    return tok;
}

Token *Token::tokAt(int index)
{
    Token *tok = this;
    while (index > 0 && tok)
    {
        tok = tok->next();
        --index;
    }
    return tok;
}

const char *Token::strAt(int index) const
{
    const Token *tok = this->tokAt(index);
    return tok ? tok->_cstr : "";
}

int Token::multiCompare(const char *needle, const char *haystack)
{
    bool emptyStringFound = false;
    bool findNextOr = false;
    const char *haystackPointer = haystack;
    for (; *needle; ++needle)
    {
        if (*needle == '|')
        {
            // If needle and haystack are both at the end, we have a match.
            if (*haystackPointer == 0)
                return 1;

            haystackPointer = haystack;
            if (findNextOr)
                findNextOr = false;
            else
                emptyStringFound = true;

            continue;
        }

        if (findNextOr)
            continue;

        // If haystack and needle don't share the same character, reset
        // haystackpointer and find next '|' character.
        if (*haystackPointer != *needle)
        {
            haystackPointer = haystack;
            findNextOr = true;
            continue;
        }

        // All characters in haystack and needle have matched this far
        ++haystackPointer;
    }

    // If both needle and haystack are at the end, then we have a match.
    if (*haystackPointer == 0)
        return 1;

    // If empty string was found or if last character in needle was '|'
    if (emptyStringFound || findNextOr == false)
        return 0;

    return -1;
}

bool Token::simpleMatch(const Token *tok, const char pattern[])
{
    const char *current, *next;

    current = pattern;
    next = strchr(pattern, ' ');
    if (!next)
        next = pattern + strlen(pattern);

    while (*current)
    {
        size_t length = static_cast<size_t>(next - current);

        if (!tok || length != tok->_str.length() || strncmp(current, tok->_cstr, length))
            return false;

        current = next;
        if (*next)
        {
            next = strchr(++current, ' ');
            if (!next)
                next = current + strlen(current);
        }
        tok = tok->next();
    }

    return true;
}

bool Token::Match(const Token *tok, const char pattern[], unsigned int varid)
{
    const char *p = pattern;
    bool firstpattern = true;
    while (*p)
    {
        // Skip spaces in pattern..
        while (*p == ' ')
            ++p;

        // Extract token from pattern..
        // TODO: Refactor this so there can't be buffer overflows
        char str[500];
        char *s = str;
        while (*p && *p != ' ')
        {
            *s = *p;
            ++s;
            ++p;
        }
        *s = 0;

        // No token => Success!
        if (str[0] == 0)
            return true;

        if (!tok)
        {
            // If we have no tokens, pattern "!!else" should return true
            if (str[1] == '!' && str[0] == '!' && str[2] != '\0')
                continue;
            else
                return false;
        }

        // If we are in the first token, we skip all initial !! patterns
        if (firstpattern && !tok->previous() && tok->next() && str[1] == '!' && str[0] == '!' && str[2] != '\0')
            continue;

        firstpattern = false;

        // Compare the first character of the string for optimization reasons
        // before doing more detailed checks.
        bool patternIdentified = false;
        if (str[0] == '%')
        {
            // Any symbolname..
            if (strcmp(str, "%var%") == 0 || strcmp(str, "%type%") == 0)
            {
                if (!tok->isName())
                    return false;

                patternIdentified = true;
            }

            // Accept any token
            else if (strcmp(str, "%any%") == 0)
            {
                patternIdentified = true;
            }

            else if (strcmp(str, "%varid%") == 0)
            {
                if (varid == 0)
                {
                    std::cout << "\n###### If you see this, there is a bug ###### Token::Match() - varid was 0" << std::endl;
                }

                if (tok->varId() != varid)
                    return false;

                patternIdentified = true;
            }

            else if (strcmp(str, "%num%") == 0)
            {
                if (!tok->isNumber())
                    return false;

                patternIdentified = true;
            }

            else if (strcmp(str, "%bool%") == 0)
            {
                if (!tok->isBoolean())
                    return false;

                patternIdentified = true;
            }

            else if (strcmp(str, "%str%") == 0)
            {
                if (tok->_str[0] != '\"')
                    return false;

                patternIdentified = true;
            }
        }

        if (patternIdentified)
        {
            // Pattern was identified already above.
        }

        // [.. => search for a one-character token..
        else if (str[0] == '[' && strchr(str, ']') && tok->_str[1] == 0)
        {
            *strrchr(str, ']') = 0;
            if (strchr(str + 1, tok->_str[0]) == 0)
                return false;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else if (strchr(str, '|') && (str[0] != '|' || strlen(str) > 2))
        {
            int res = multiCompare(str, tok->_cstr);
            if (res == 0)
            {
                // Empty alternative matches, use the same token on next round
                continue;
            }
            else if (res == -1)
            {
                // No match
                return false;
            }
        }

        // Parse "not" options. Token can be anything except the given one
        else if (str[1] == '!' && str[0] == '!' && str[2] != '\0')
        {
            if (strcmp(tok->aaaa(), &(str[2])) == 0)
                return false;
        }

        else if (str != tok->_str)
            return false;

        tok = tok->next();
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

bool Token::isName() const
{
    return _isName;
}

bool Token::isNumber() const
{
    return _isNumber;
}

bool Token::isBoolean() const
{
    return _isBoolean;
}

bool Token::isStandardType() const
{
    bool ret = false;
    const char *type[] = {"bool", "char", "short", "int", "long", "float", "double", 0};
    for (int i = 0; type[i]; i++)
        ret |= (_str == type[i]);
    return ret;
}

//---------------------------------------------------------------------------

const Token *Token::findmatch(const Token *tok, const char pattern[], unsigned int varId)
{
    for (; tok; tok = tok->next())
    {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return 0;
}

unsigned int Token::varId() const
{
    return _varId;
}

void Token::varId(unsigned int id)
{
    _varId = id;
}

Token *Token::next() const
{
    return _next;
}

void Token::next(Token *next)
{
    _next = next;
}

Token *Token::previous() const
{
    return _previous;
}

void Token::previous(Token *previous)
{
    _previous = previous;
}

void Token::insertToken(const char str[])
{
    Token *newToken = new Token;
    newToken->str(str);
    newToken->_linenr = _linenr;
    newToken->_fileIndex = _fileIndex;
    if (this->next())
    {
        newToken->next(this->next());
        newToken->next()->previous(newToken);
    }

    this->next(newToken);
    newToken->previous(this);
}

void Token::eraseTokens(Token *begin, const Token *end)
{
    if (! begin)
        return;

    while (begin->next() && begin->next() != end)
    {
        begin->deleteNext();
    }
}

unsigned int Token::fileIndex() const
{
    return _fileIndex;
}

void Token::fileIndex(unsigned int fileIndex)
{
    _fileIndex = fileIndex;
}

unsigned int Token::linenr() const
{
    return _linenr;
}

void Token::linenr(unsigned int linenr)
{
    _linenr = linenr;
}

void Token::printOut(const char *title) const
{
    std::cout << std::endl << "###";
    if (title)
        std::cout << " " << title << " ";
    else
        std::cout << "########";

    std::cout << "###" << std::endl;
    for (const Token *t = this; t; t = t->next())
    {
        std::cout << t->linenr() << ": " << t->str();
        if (t->varId())
            std::cout << " (" << t->varId() << ")";

        std::cout << std::endl;
    }
}
