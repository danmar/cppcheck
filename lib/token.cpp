/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#include "token.h"
#include "errorlogger.h"
#include "check.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <cctype>
#include <sstream>
#include <map>

Token::Token(Token **t) :
    tokensBack(t),
    _next(0),
    _previous(0),
    _link(0),
    _scope(0),
    _str(""),
    _varId(0),
    _fileIndex(0),
    _linenr(0),
    _progressValue(0),
    _type(eNone),
    _isUnsigned(false),
    _isSigned(false),
    _isPointerCompare(false),
    _isLong(false),
    _isUnused(false),
    _isStandardType(false),
    _isExpandedMacro(false)
{
}

Token::~Token()
{

}

void Token::update_property_info()
{
    if (!_str.empty()) {
        if (_str == "true" || _str == "false")
            _type = eBoolean;
        else if (_str[0] == '_' || std::isalpha(_str[0])) { // Name
            if (_varId)
                _type = eVariable;
            _type = eName;
        } else if (std::isdigit(_str[0]) || (_str.length() > 1 && _str[0] == '-' && std::isdigit(_str[1])))
            _type = eNumber;
        else if (_str.length() > 1 && _str[0] == '"' && _str[_str.length()-1] == '"')
            _type = eString;
        else if (_str.length() > 1 && _str[0] == '\'' && _str[_str.length()-1] == '\'')
            _type = eChar;
        else if (_str == "="   ||
                 _str == "+="  ||
                 _str == "-="  ||
                 _str == "*="  ||
                 _str == "/="  ||
                 _str == "%="  ||
                 _str == "&="  ||
                 _str == "^="  ||
                 _str == "|="  ||
                 _str == "<<=" ||
                 _str == ">>=")
            _type = eAssignmentOp;
        else if (_str.size() == 1 && _str.find_first_of(",[]()?:") != std::string::npos)
            _type = eExtendedOp;
        else if (_str=="<<" || _str==">>" || (_str.size()==1 && _str.find_first_of("+-*/%") != std::string::npos))
            _type = eArithmeticalOp;
        else if (_str.size() == 1 && _str.find_first_of("&|^~") != std::string::npos)
            _type = eBitOp;
        else if (_str == "&&" ||
                 _str == "||" ||
                 _str == "!")
            _type = eLogicalOp;
        else if ((_str == "==" ||
                  _str == "!=" ||
                  _str == "<"  ||
                  _str == "<=" ||
                  _str == ">"  ||
                  _str == ">=") && !_link)
            _type = eComparisonOp;
        else if (_str == "++" ||
                 _str == "--")
            _type = eIncDecOp;
        else if (_str.size() == 1 && (_str.find_first_of("{}") != std::string::npos || (_link && _str.find_first_of("<>") != std::string::npos)))
            _type = eBracket;
        else
            _type = eOther;
    } else {
        _type = eNone;
    }

    update_property_isStandardType();
}

void Token::update_property_isStandardType()
{
    _isStandardType = false;

    if (_str.size() < 3)
        return;

    static const char * const stdtype[] = {"int", "char", "bool", "long", "short", "float", "double", "size_t", 0};
    for (int i = 0; stdtype[i]; i++) {
        if (_str == stdtype[i]) {
            _isStandardType = true;
            _type = eType;
            break;
        }
    }
}


bool Token::isUpperCaseName() const
{
    if (!isName())
        return false;
    for (unsigned int i = 0; i < _str.length(); ++i) {
        if (std::islower(_str[i]))
            return false;
    }
    return true;
}

void Token::str(const std::string &s)
{
    _str = s;
    _varId = 0;

    update_property_info();
}

void Token::concatStr(std::string const& b)
{
    _str.erase(_str.length() - 1);
    _str.append(b.begin() + 1, b.end());

    update_property_info();
}

std::string Token::strValue() const
{
    assert(_type == eString);
    return _str.substr(1, _str.length() - 2);
}

void Token::deleteNext(unsigned long index)
{
    while (_next && index--) {
        Token *n = _next;
        _next = n->next();
        delete n;
    }

    if (_next)
        _next->previous(this);
    else if (tokensBack)
        *tokensBack = this;
}

void Token::deleteThis()
{
    if (_next) { // Copy next to this and delete next
        _str = _next->_str;
        _type = _next->_type;
        _isUnsigned = _next->_isUnsigned;
        _isSigned = _next->_isSigned;
        _isPointerCompare = _next->_isPointerCompare;
        _isLong = _next->_isLong;
        _isUnused = _next->_isUnused;
        _isStandardType = _next->_isStandardType;
        _isExpandedMacro = _next->_isExpandedMacro;
        _varId = _next->_varId;
        _fileIndex = _next->_fileIndex;
        _linenr = _next->_linenr;
        _link = _next->_link;
        _scope = _next->_scope;
        if (_link)
            _link->link(this);

        deleteNext();
    } else if (_previous && _previous->_previous) { // Copy previous to this and delete previous
        _str = _previous->_str;
        _type = _previous->_type;
        _isUnsigned = _previous->_isUnsigned;
        _isSigned = _previous->_isSigned;
        _isPointerCompare = _previous->_isPointerCompare;
        _isLong = _previous->_isLong;
        _isUnused = _previous->_isUnused;
        _isStandardType = _previous->_isStandardType;
        _isExpandedMacro = _previous->_isExpandedMacro;
        _varId = _previous->_varId;
        _fileIndex = _previous->_fileIndex;
        _linenr = _previous->_linenr;
        _link = _previous->_link;
        _scope = _previous->_scope;
        if (_link)
            _link->link(this);

        Token* toDelete = _previous;
        _previous = _previous->_previous;
        _previous->_next = this;

        delete toDelete;
    } else {
        // We are the last token in the list, we can't delete
        // ourselves, so just make us empty
        str("");
    }
}

void Token::replace(Token *replaceThis, Token *start, Token *end)
{
    // Fix the whole in the old location of start and end
    if (start->previous())
        start->previous()->next(end->next());

    if (end->next())
        end->next()->previous(start->previous());

    // Move start and end to their new location
    if (replaceThis->previous())
        replaceThis->previous()->next(start);

    if (replaceThis->next())
        replaceThis->next()->previous(end);

    start->previous(replaceThis->previous());
    end->next(replaceThis->next());

    if (end->tokensBack && *(end->tokensBack) == end) {
        while (end->next())
            end = end->next();
        *(end->tokensBack) = end;
    }

    // Update _progressValue, fileIndex and linenr
    for (Token *tok = start; tok != end->next(); tok = tok->next())
        tok->_progressValue = replaceThis->_progressValue;

    // Delete old token, which is replaced
    delete replaceThis;
}

const Token *Token::tokAt(int index) const
{
    const Token *tok = this;
    int num = std::abs(index);
    while (num > 0 && tok) {
        if (index > 0)
            tok = tok->next();
        else
            tok = tok->previous();
        --num;
    }
    return tok;
}

Token *Token::tokAt(int index)
{
    Token *tok = this;
    int num = std::abs(index);
    while (num > 0 && tok) {
        if (index > 0)
            tok = tok->next();
        else
            tok = tok->previous();
        --num;
    }
    return tok;
}

const Token *Token::linkAt(int index) const
{
    const Token *tok = this->tokAt(index);
    if (!tok) {
        throw InternalError(this, "Internal error. Token::linkAt called with index outside the tokens range.");
    }
    return tok ? tok->link() : 0;
}

Token *Token::linkAt(int index)
{
    Token *tok = this->tokAt(index);
    if (!tok) {
        throw InternalError(this, "Internal error. Token::linkAt called with index outside the tokens range.");
    }
    return tok ? tok->link() : 0;
}

const std::string &Token::strAt(int index) const
{
    static const std::string empty_str;

    const Token *tok = this->tokAt(index);
    return tok ? tok->_str : empty_str;
}

static bool strisop(const char str[])
{
    if (str[1] == 0) {
        if (strchr("+-*/%&|^~!<>", *str))
            return true;
    } else if (str[2] == 0) {
        if ((str[0] == '&' && str[1] == '&') ||
            (str[0] == '|' && str[1] == '|') ||
            (str[0] == '=' && str[1] == '=') ||
            (str[0] == '!' && str[1] == '=') ||
            (str[0] == '>' && str[1] == '=') ||
            (str[0] == '<' && str[1] == '=') ||
            (str[0] == '>' && str[1] == '>') ||
            (str[0] == '<' && str[1] == '<'))
            return true;
    }
    return false;
}

static int multiComparePercent(const char * * haystack_p,
                               const char * needle,
                               bool emptyStringFound)
{
    const char *haystack = *haystack_p;

    if (haystack[0] == '%' && haystack[1] != '|' && haystack[1] != '\0' && haystack[1] != ' ') {
        if (haystack[1] == 'o' && // "%op%"
            haystack[2] == 'p' &&
            haystack[3] == '%') {
            if (strisop(needle))
                return 1;
            *haystack_p = haystack = haystack + 4;
        } else if (haystack[1] == 'o' && // "%or%"
                   haystack[2] == 'r' &&
                   haystack[3] == '%') {
            if (*needle == '|' && needle[1] != '|' && needle[1] != '=')
                return 1;
            *haystack_p = haystack = haystack + 4;
        } else if (haystack[1] == 'o' && // "%oror%"
                   haystack[2] == 'r' &&
                   haystack[3] == 'o' &&
                   haystack[4] == 'r' &&
                   haystack[5] == '%') {
            if (needle[0] == '|' && needle[1] == '|')
                return 1;
            *haystack_p = haystack = haystack + 6;
        }

        if (*haystack == '|')
            *haystack_p = haystack = haystack + 1;
        else if (*haystack == ' ' || *haystack == '\0')
            return emptyStringFound ? 0 : -1;
        else
            return -1;
    }

    return 0xFFFF;
}

int Token::multiCompare(const char *haystack, const char *needle)
{
    if (haystack[0] == '%' && haystack[1] == 'o') {
        if (haystack[2] == 'p' && // "%op%|"
            haystack[3] == '%' &&
            haystack[4] == '|') {
            haystack = haystack + 5;
            if (strisop(needle))
                return 1;
        } else if (haystack[2] == 'r' && // "%or%|"
                   haystack[3] == '%' &&
                   haystack[4] == '|') {
            haystack = haystack + 5;
            if (*needle == '|' && needle[1] != '|' && needle[1] != '=')
                return 1;
        } else if (haystack[2] == 'r' && // "%oror%|"
                   haystack[3] == 'o' &&
                   haystack[4] == 'r' &&
                   haystack[5] == '%' &&
                   haystack[6] == '|') {
            haystack = haystack + 7;
            if (needle[0] == '|' && needle[1] == '|')
                return 1;
        }
    }

    bool emptyStringFound = false;
    const char *needlePointer = needle;
    for (;;) {
        if (*needlePointer == *haystack) {
            if (*needlePointer == '\0')
                return 1;
            ++needlePointer;
            ++haystack;
        } else if (*haystack == '|') {
            if (*needlePointer == 0) {
                // If needle is at the end, we have a match.
                return 1;
            } else if (needlePointer == needle) {
                // If needlePointer was not increased at all, we had a empty
                // string in the haystack
                emptyStringFound = true;
            }

            needlePointer = needle;
            ++haystack;

            int ret = multiComparePercent(&haystack, needle, emptyStringFound);
            if (ret < 2)
                return ret;
        } else if (*haystack == ' ' || *haystack == '\0') {
            if (needlePointer == needle)
                return 0;
            break;
        }
        // If haystack and needle don't share the same character,
        // find next '|' character.
        else {
            needlePointer = needle;

            do {
                ++haystack;
            } while (*haystack != ' ' && *haystack != '|' && *haystack);

            if (*haystack == ' ' || *haystack == '\0') {
                return emptyStringFound ? 0 : -1;
            }

            ++haystack;

            int ret = multiComparePercent(&haystack, needle, emptyStringFound);
            if (ret < 2)
                return ret;
        }
    }

    if (*needlePointer == '\0')
        return 1;

    // If empty string was found earlier from the haystack
    if (emptyStringFound)
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

    while (*current) {
        std::size_t length = static_cast<std::size_t>(next - current);

        if (!tok || length != tok->_str.length() || strncmp(current, tok->_str.c_str(), length))
            return false;

        current = next;
        if (*next) {
            next = strchr(++current, ' ');
            if (!next)
                next = current + strlen(current);
        }
        tok = tok->next();
    }

    return true;
}

bool Token::firstWordEquals(const char *str, const char *word)
{
    for (;;) {
        if (*str != *word) {
            return (*str == ' ' && *word == 0);
        } else if (*str == 0)
            break;

        ++str;
        ++word;
    }

    return true;
}

const char *Token::chrInFirstWord(const char *str, char c)
{
    for (;;) {
        if (*str == ' ' || *str == 0)
            return 0;

        if (*str == c)
            return str;

        ++str;
    }
}

int Token::firstWordLen(const char *str)
{
    int len = 0;
    for (;;) {
        if (*str == ' ' || *str == 0)
            break;

        ++len;
        ++str;
    }

    return len;
}

bool Token::Match(const Token *tok, const char pattern[], unsigned int varid)
{
    const char *p = pattern;
    bool firstpattern = true;
    while (*p) {
        // Skip spaces in pattern..
        while (*p == ' ')
            ++p;

        // No token => Success!
        if (*p == 0)
            return true;

        if (!tok) {
            // If we have no tokens, pattern "!!else" should return true
            if (p[1] == '!' && p[0] == '!' && p[2] != '\0') {
                while (*p && *p != ' ')
                    ++p;
                continue;
            } else
                return false;
        }

        // If we are in the first token, we skip all initial !! patterns
        if (firstpattern && !tok->previous() && tok->next() && p[1] == '!' && p[0] == '!' && p[2] != '\0') {
            while (*p && *p != ' ')
                ++p;
            continue;
        }

        firstpattern = false;

        // Compare the first character of the string for optimization reasons
        // before doing more detailed checks.
        if (p[0] == '%' && p[1]!='|') {
            bool patternUnderstood = false;
            switch (p[1]) {
            case 'v':
                // TODO: %var% should match only for
                // variables that have varId != 0, but that needs a lot of
                // work, before that change can be made.
                // Any symbolname..
                if (p[4] == '%') { // %var%
                    if (!tok->isName())
                        return false;
                    p += 5;
                    patternUnderstood = true;
                } else { // %varid%
                    if (varid == 0) {
                        throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");
                    }

                    if (tok->varId() != varid)
                        return false;

                    p += 7;
                    patternUnderstood = true;
                }
                break;
            case 't':
                // Type (%type%)
            {
                if (!tok->isName())
                    return false;

                if (tok->varId() != 0)
                    return false;

                if (tok->str() == "delete")
                    return false;

                p += 6;
                patternUnderstood = true;
            }
            break;
            case 'a':
                // Accept any token (%any%)
            {
                p += 5;
                patternUnderstood = true;
            }
            break;
            case 'n':
                // Number (%num%)
            {
                if (!tok->isNumber())
                    return false;
                p += 5;
                patternUnderstood = true;
            }
            break;
            case 's':
                // String (%str%)
            {
                if (tok->_str[0] != '\"')
                    return false;
                p += 5;
                patternUnderstood = true;
            }
            break;
            case 'b':
                // Bool (%bool%)
            {
                if (!tok->isBoolean())
                    return false;
                p += 6;
                patternUnderstood = true;
            }
            break;
            case 'o':
                // Or (%or%) and Op (%op%)
                if (p[3] == '%') {
                    patternUnderstood = true;

                    // multicompare..
                    if (p[4] == '|') {
                        int result = multiCompare(p, tok->str().c_str());
                        if (result == -1)
                            return false;   // No match

                        while (*p && *p != ' ')
                            p++;
                    }

                    // single compare..
                    else if (p[2] == 'r') {
                        if (tok->str() != "|")
                            return false;
                        p += 4;
                    } else if (p[2] == 'p') {
                        if (!tok->isOp())
                            return false;
                        p += 4;
                    } else
                        patternUnderstood = false;
                }

                // Oror (%oror%)
                else if (p[5] == '%') {
                    // multicompare..
                    if (p[6] == '|') {
                        int result = multiCompare(p, tok->str().c_str());
                        if (result == -1)
                            return false;   // No match

                        while (*p && *p != ' ')
                            p++;
                    }

                    // single compare..
                    else if (tok->str() != "||")
                        return false;

                    else
                        p += 6;

                    patternUnderstood = true;
                }
                break;
            default:
                if (firstWordEquals(p, tok->_str.c_str())) {
                    p += tok->_str.length();
                    patternUnderstood = true;
                }
                break;
            }

            if (!patternUnderstood) {
                return false;
            }

            // debugging: assert that this is not part of a multicompare pattern..
            assert(*p != '|');

            tok = tok->next();
            continue;
        }

        // [.. => search for a one-character token..
        else if (p[0] == '[' && chrInFirstWord(p, ']')) {
            if (tok->_str.length() != 1)
                return false;

            const char *temp = p + 1;
            bool chrFound = false;
            int count = 0;
            while (*temp && *temp != ' ') {
                if (*temp == ']') {
                    ++count;
                    ++temp;
                    continue;
                }

                if (*temp == tok->_str[0]) {
                    chrFound = true;
                    break;
                }

                ++temp;
            }

            if (count > 1) {
                if (tok->_str[0] == ']')
                    chrFound = true;
            }

            if (!chrFound)
                return false;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else if (chrInFirstWord(p, '|') && (p[0] != '|' || firstWordLen(p) > 2)) {
            int res = multiCompare(p, tok->_str.c_str());
            if (res == 0) {
                // Empty alternative matches, use the same token on next round
                while (*p && *p != ' ')
                    ++p;
                continue;
            } else if (res == -1) {
                // No match
                return false;
            }
        }

        // Parse "not" options. Token can be anything except the given one
        else if (p[1] == '!' && p[0] == '!' && p[2] != '\0') {
            if (firstWordEquals(&(p[2]), tok->str().c_str()))
                return false;
        }

        else if (!firstWordEquals(p, tok->_str.c_str())) {
            return false;
        }

        while (*p && *p != ' ')
            ++p;

        tok = tok->next();
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

std::size_t Token::getStrLength(const Token *tok)
{
    assert(tok != NULL);

    std::size_t len = 0;
    const std::string strValue(tok->strValue());
    const char *str = strValue.c_str();

    while (*str) {
        if (*str == '\\') {
            ++str;

            // string ends at '\0'
            if (*str == '0')
                break;
        }

        ++str;
        ++len;
    }

    return len;
}

void Token::move(Token *srcStart, Token *srcEnd, Token *newLocation)
{
    /**[newLocation] -> b -> c -> [srcStart] -> [srcEnd] -> f */

    // Fix the gap, which tokens to be moved will leave
    srcStart->previous()->next(srcEnd->next());
    srcEnd->next()->previous(srcStart->previous());

    // Fix the tokens to be moved
    srcEnd->next(newLocation->next());
    srcStart->previous(newLocation);

    // Fix the tokens at newLocation
    newLocation->next()->previous(srcEnd);
    newLocation->next(srcStart);

    // Update _progressValue
    for (Token *tok = srcStart; tok != srcEnd->next(); tok = tok->next())
        tok->_progressValue = newLocation->_progressValue;
}

Token* Token::nextArgument() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        else if (tok->str() == "(" || tok->str() == "{" || tok->str() == "[")
            tok = tok->link();
        else if (tok->str() == "<" && tok->link())
            tok = tok->link();
        else if (tok->str() == ")" || tok->str() == ";")
            return 0;
    }
    return 0;
}

bool Token::findClosingBracket(const Token*& closing) const
{
    if (_str == "<") {
        unsigned int depth = 0;
        for (closing = this; closing != NULL; closing = closing->next()) {
            if (closing->str() == "{" || closing->str() == "[" || closing->str() == "(")
                closing = closing->link();
            else if (closing->str() == "}" || closing->str() == "]" || closing->str() == ")" || closing->str() == ";" || closing->str() == "=")
                return false;
            else if (closing->str() == "<")
                ++depth;
            else if (closing->str() == ">") {
                if (--depth == 0)
                    return true;
            } else if (closing->str() == ">>") {
                if (--depth == 0)
                    return true;
                if (--depth == 0)
                    return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------

const Token *Token::findsimplematch(const Token *tok, const char pattern[])
{
    for (; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern))
            return tok;
    }
    return 0;
}

const Token *Token::findsimplematch(const Token *tok, const char pattern[], const Token *end)
{
    for (; tok && tok != end; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern))
            return tok;
    }
    return 0;
}

const Token *Token::findmatch(const Token *tok, const char pattern[], unsigned int varId)
{
    for (; tok; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return 0;
}

const Token *Token::findmatch(const Token *tok, const char pattern[], const Token *end, unsigned int varId)
{
    for (; tok && tok != end; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return 0;
}

void Token::insertToken(const std::string &tokenStr)
{
    Token *newToken;
    if (_str.empty())
        newToken = this;
    else
        newToken = new Token(tokensBack);
    newToken->str(tokenStr);
    newToken->_linenr = _linenr;
    newToken->_fileIndex = _fileIndex;
    newToken->_progressValue = _progressValue;

    if (newToken != this) {
        if (this->next()) {
            newToken->next(this->next());
            newToken->next()->previous(newToken);
        } else if (tokensBack) {
            *tokensBack = newToken;
        }
        this->next(newToken);
        newToken->previous(this);
    }
}

void Token::eraseTokens(Token *begin, const Token *end)
{
    if (!begin || begin == end)
        return;

    while (begin->next() && begin->next() != end) {
        begin->deleteNext();
    }
}

void Token::createMutualLinks(Token *begin, Token *end)
{
    assert(begin != NULL);
    assert(end != NULL);
    assert(begin != end);
    begin->link(end);
    end->link(begin);
}

void Token::printOut(const char *title) const
{
    if (title)
        std::cout << "\n### " << title << " ###\n";
    std::cout << stringifyList(true, true, true, true, true, 0, 0) << std::endl;
}

void Token::printOut(const char *title, const std::vector<std::string> &fileNames) const
{
    if (title)
        std::cout << "\n### " << title << " ###\n";
    std::cout << stringifyList(true, true, true, true, true, &fileNames, 0) << std::endl;
}

void Token::stringify(std::ostream& os, bool varid, bool attributes) const
{
    if (attributes) {
        if (isUnsigned())
            os << "unsigned ";
        else if (isSigned())
            os << "signed ";
        if (isLong())
            os << "long ";
    }
    os << _str;
    if (varid && _varId != 0)
        os << '@' << _varId;
}

std::string Token::stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames, const Token* end) const
{
    if (this == end)
        return "";

    std::ostringstream ret;

    unsigned int lineNumber = _linenr;
    int fileInd = files?-1:_fileIndex;
    std::map<int, unsigned int> lineNumbers;
    for (const Token *tok = this; tok != end; tok = tok->next()) {
        bool fileChange = false;
        if (static_cast<int>(tok->_fileIndex) != fileInd) {
            if (fileInd != -1) {
                lineNumbers[fileInd] = tok->_fileIndex;
            }

            fileInd = static_cast<int>(tok->_fileIndex);
            if (files) {
                ret << "\n\n##file ";
                if (fileNames && fileNames->size() > tok->_fileIndex)
                    ret << fileNames->at(tok->_fileIndex);
                else
                    ret << fileInd;
            }

            lineNumber = lineNumbers[fileInd];
            fileChange = true;
        }

        if (linebreaks && (lineNumber != tok->linenr() || fileChange)) {
            if (lineNumber+4 < tok->linenr() && fileInd == static_cast<int>(tok->_fileIndex)) {
                ret << '\n' << lineNumber+1 << ":\n|\n";
                ret << tok->linenr()-1 << ":\n";
                ret << tok->linenr() << ": ";
            } else {
                while (lineNumber < tok->linenr()) {
                    ++lineNumber;
                    ret << '\n';
                    if (linenumbers) {
                        ret << lineNumber << ':';
                        if (lineNumber == tok->linenr())
                            ret << ' ';
                    }
                }
            }
            lineNumber = tok->linenr();
        }

        tok->stringify(ret, varid, attributes); // print token
        if (tok->next() != end && (!linebreaks || (tok->next()->linenr() <= tok->linenr() && tok->next()->fileIndex() == tok->fileIndex())))
            ret << ' ';
    }
    if (linebreaks && files)
        ret << '\n';
    return ret.str();
}

std::string Token::stringifyList(const Token* end, bool attributes) const
{
    return stringifyList(false, attributes, false, false, false, 0, end);
}

std::string Token::stringifyList(bool varid) const
{
    return stringifyList(varid, false, true, true, true, 0, 0);
}
