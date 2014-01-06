/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include <stack>

Token::Token(Token **t) :
    tokensBack(t),
    _next(0),
    _previous(0),
    _link(0),
    _scope(0),
    _function(0), // Initialize whole union
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
    _isStandardType(false),
    _isExpandedMacro(false),
    _isAttributeConstructor(false),
    _isAttributeUnused(false),
    _astOperand1(NULL),
    _astOperand2(NULL),
    _astParent(NULL)
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
            else if (_type != eVariable && _type != eFunction && _type != eType)
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

    static const char * const stdtype[] = {"int", "char", "bool", "long", "short", "float", "double", "wchar_t", "size_t", "void", 0};
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
        _isStandardType = _next->_isStandardType;
        _isExpandedMacro = _next->_isExpandedMacro;
        _isAttributeConstructor = _next->_isAttributeConstructor;
        _isAttributeUnused = _next->_isAttributeUnused;
        _varId = _next->_varId;
        _fileIndex = _next->_fileIndex;
        _linenr = _next->_linenr;
        _link = _next->_link;
        _scope = _next->_scope;
        _function = _next->_function;
        _variable = _next->_variable;
        _originalName = _next->_originalName;
        values = _next->values;
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
        _isStandardType = _previous->_isStandardType;
        _isExpandedMacro = _previous->_isExpandedMacro;
        _isAttributeConstructor = _previous->_isAttributeConstructor;
        _isAttributeUnused = _previous->_isAttributeUnused;
        _varId = _previous->_varId;
        _fileIndex = _previous->_fileIndex;
        _linenr = _previous->_linenr;
        _link = _previous->_link;
        _scope = _previous->_scope;
        _function = _previous->_function;
        _variable = _previous->_variable;
        _originalName = _previous->_originalName;
        values = _previous->values;
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

const Token *Token::linkAt(int index) const
{
    const Token *tok = this->tokAt(index);
    if (!tok) {
        throw InternalError(this, "Internal error. Token::linkAt called with index outside the tokens range.");
    }
    return tok->link();
}

const std::string &Token::strAt(int index) const
{
    static const std::string empty_str;

    const Token *tok = this->tokAt(index);
    return tok ? tok->_str : empty_str;
}

static int multiComparePercent(const Token *tok, const char ** haystack_p,
                               const char * needle,
                               bool emptyStringFound)
{
    const char *haystack = *haystack_p;

    if (haystack[0] == '%' && haystack[1] != '|' && haystack[1] != '\0' && haystack[1] != ' ') {
        if (haystack[1] == 'o' && // "%op%"
            haystack[2] == 'p' &&
            haystack[3] == '%') {
            if (tok->isOp())
                return 1;
            *haystack_p = haystack = haystack + 4;
        } else if (haystack[1] == 'c' && // "%cop%"
                   haystack[2] == 'o' &&
                   haystack[3] == 'p' &&
                   haystack[4] == '%') {
            if (tok->isConstOp())
                return 1;
            *haystack_p = haystack = haystack + 5;
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
        } else if (haystack[1] == 'v' && // "%var%"
                   haystack[2] == 'a' &&
                   haystack[3] == 'r' &&
                   haystack[4] == '%') {
            if (tok->isName())
                return 1;
            *haystack_p = haystack = haystack + 5;
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

int Token::multiCompare(const Token *tok, const char *haystack, const char *needle)
{
    if (haystack[0] == '%' && haystack[1] == 'o') {
        if (haystack[2] == 'p' && // "%op%|"
            haystack[3] == '%' &&
            haystack[4] == '|') {
            haystack = haystack + 5;
            if (tok->isOp())
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
    } else if (haystack[0] == '%' && haystack[1] == 'c' && haystack[2] == 'o' && // "%cop%|"
               haystack[3] == 'p' && haystack[4] == '%' &&
               haystack[5] == '|') {
        haystack = haystack + 6;
        if (tok->isConstOp())
            return 1;
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

            int ret = multiComparePercent(tok, &haystack, needle, emptyStringFound);
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

            int ret = multiComparePercent(tok, &haystack, needle, emptyStringFound);
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
    next = std::strchr(pattern, ' ');
    if (!next)
        next = pattern + std::strlen(pattern);

    while (*current) {
        std::size_t length = static_cast<std::size_t>(next - current);

        if (!tok || length != tok->_str.length() || std::strncmp(current, tok->_str.c_str(), length))
            return false;

        current = next;
        if (*next) {
            next = std::strchr(++current, ' ');
            if (!next)
                next = current + std::strlen(current);
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

#define multicompare(p,cond,ismulticomp)        \
{                                               \
    if (!(cond)) {                              \
        if (*(p) != '|')                        \
            return false;                       \
        ++(p);                                  \
        ismulticomp = (*(p) && *(p) != ' ');    \
        continue;                               \
    }                                           \
    if (*(p) == '|') {                          \
        while (*(p) && *(p) != ' ')             \
            ++(p);                              \
    }                                           \
    ismulticomp = false;                        \
}

bool Token::Match(const Token *tok, const char pattern[], unsigned int varid)
{
    const char *p = pattern;
    bool ismulticomp = false;
    while (*p) {
        // Skip spaces in pattern..
        while (*p == ' ')
            ++p;

        // No token => Success!
        if (*p == '\0')
            break;

        if (!tok) {
            // If we have no tokens, pattern "!!else" should return true
            if (p[0] == '!' && p[1] == '!' && p[2] != '\0') {
                while (*p && *p != ' ')
                    ++p;
                continue;
            } else
                return false;
        }

        // Compare the first character of the string for optimization reasons
        // before doing more detailed checks.
        if (p[0] == '%') {
            ++p;
            switch (p[0]) {
            case '\0':
            case ' ':
            case '|':
                //simple '%' character
            {
                multicompare(p, tok->str() == "%", ismulticomp);
            }
            break;
            case 'v':
                // TODO: %var% should match only for
                // variables that have varId != 0, but that needs a lot of
                // work, before that change can be made.
                // Any symbolname..
                if (p[3] == '%') { // %var%
                    p += 4;
                    multicompare(p,tok->isName(),ismulticomp);
                } else { // %varid%
                    if (varid == 0) {
                        throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");
                    }

                    if (tok->varId() != varid)
                        return false;

                    p += 6;
                }
                break;
            case 't':
                // Type (%type%)
            {
                p += 5;
                multicompare(p,tok->isName() && tok->varId() == 0 && tok->str() != "delete",ismulticomp);
            }
            break;
            case 'a':
                // Accept any token (%any%)
            {
                p += 4;
                if (p[0] == '|') {
                    while (*p && *p != ' ')
                        ++p;
                }
                ismulticomp = false;
            }
            break;
            case 'n':
                // Number (%num%)
            {
                p += 4;
                multicompare(p,tok->isNumber(),ismulticomp);
            }
            break;
            case 'c': {
                p += 1;
                // Character (%char%)
                if (p[0] == 'h') {
                    p += 4;
                    multicompare(p,tok->type() == eChar,ismulticomp);
                }
                // Const operator (%cop%)
                else if (p[1] == 'p') {
                    p += 3;
                    multicompare(p,tok->isConstOp(),ismulticomp);
                }
                // Comparison operator (%comp%)
                else {
                    p += 4;
                    multicompare(p,tok->isComparisonOp(),ismulticomp);
                }
            }
            break;
            case 's':
                // String (%str%)
            {
                p += 4;
                multicompare(p,tok->type() == eString,ismulticomp);
            }
            break;
            case 'b':
                // Bool (%bool%)
            {
                p += 5;
                multicompare(p,tok->isBoolean(),ismulticomp);
            }
            break;
            case 'o': {
                ++p;
                if (p[1] == '%') {
                    // Op (%op%)
                    if (p[0] == 'p') {
                        p += 2;
                        multicompare(p,tok->isOp(),ismulticomp);
                    }
                    // Or (%or%)
                    else {
                        p += 2;
                        multicompare(p,tok->str() == "|",ismulticomp)
                    }
                }

                // Oror (%oror%)
                else {
                    p += 4;
                    multicompare(p,tok->str() == "||",ismulticomp);
                }
            }
            break;
            default:
                //unknown %cmd%, abort
                std::abort();
            }
        }

        else if (ismulticomp) {
            ismulticomp = false;
            continue;
        }

        // [.. => search for a one-character token..
        else if (p[0] == '[' && chrInFirstWord(p, ']')) {
            if (tok->str().length() != 1)
                return false;

            const char *temp = p+1;
            bool chrFound = false;
            unsigned int count = 0;
            while (*temp && *temp != ' ') {
                if (*temp == ']') {
                    ++count;
                }

                else if (*temp == tok->str()[0]) {
                    chrFound = true;
                    break;
                }

                ++temp;
            }

            if (count > 1 && tok->str()[0] == ']')
                chrFound = true;

            if (!chrFound)
                return false;

            p = temp;
            while (*p && *p != ' ')
                ++p;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else if (chrInFirstWord(p, '|') && (p[0] != '|' || firstWordLen(p) > 2)) {
            int res = multiCompare(tok, p, tok->_str.c_str());
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
        else if (p[0] == '!' && p[1] == '!' && p[2] != '\0') {
            p += 2;
            if (firstWordEquals(p, tok->str().c_str()))
                return false;
            while (*p && *p != ' ')
                ++p;
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

std::string Token::getCharAt(const Token *tok, std::size_t index)
{
    assert(tok != NULL);

    const std::string strValue(tok->strValue());
    const char *str = strValue.c_str();

    while (*str) {
        if (index == 0) {
            std::string ret;
            if (*str == '\\') {
                ret = *str;
                ++str;
            }
            ret += *str;
            return ret;
        }

        if (*str == '\\')
            ++str;
        ++str;
        --index;
    }
    assert(index == 0);

    return "\\0";
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

const Token * Token::findClosingBracket() const
{
    const Token *closing = 0;

    if (_str == "<") {
        unsigned int depth = 0;
        for (closing = this; closing != NULL; closing = closing->next()) {
            if (closing->str() == "{" || closing->str() == "[" || closing->str() == "(")
                closing = closing->link();
            else if (closing->str() == "}" || closing->str() == "]" || closing->str() == ")" || closing->str() == ";" || closing->str() == "=")
                break;
            else if (closing->str() == "<")
                ++depth;
            else if (closing->str() == ">") {
                if (--depth == 0)
                    break;
            } else if (closing->str() == ">>") {
                if (--depth == 0)
                    break;
                if (--depth == 0)
                    break;
            }
        }
    }

    return closing;
}

Token * Token::findClosingBracket()
{
    // return value of const function
    return const_cast<Token*>(const_cast<const Token*>(this)->findClosingBracket());
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

void Token::insertToken(const std::string &tokenStr, bool prepend)
{
    Token *newToken;

    //TODO: Find a solution for the first token on the list
    if (prepend && !this->previous())
        return;

    if (_str.empty())
        newToken = this;
    else
        newToken = new Token(tokensBack);
    newToken->str(tokenStr);
    newToken->_linenr = _linenr;
    newToken->_fileIndex = _fileIndex;
    newToken->_progressValue = _progressValue;

    if (newToken != this) {
        if (prepend) {
            /*if (this->previous())*/ {
                newToken->previous(this->previous());
                newToken->previous()->next(newToken);
            } /*else if (tokensFront?) {
                *tokensFront? = newToken;
            }*/
            this->previous(newToken);
            newToken->next(this);
        } else {
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
}

void Token::insertToken(const std::string &tokenStr, const std::string &originalNameStr, bool prepend)
{
    Token *newToken;

    //TODO: Find a solution for the first token on the list
    if (prepend && !this->previous())
        return;

    if (_str.empty())
        newToken = this;
    else
        newToken = new Token(tokensBack);
    newToken->str(tokenStr);
    newToken->_originalName = originalNameStr;
    newToken->_linenr = _linenr;
    newToken->_fileIndex = _fileIndex;
    newToken->_progressValue = _progressValue;

    if (newToken != this) {
        if (prepend) {
            /*if (this->previous())*/ {
                newToken->previous(this->previous());
                newToken->previous()->next(newToken);
            } /*else if (tokensFront?) {
                *tokensFront? = newToken;
            }*/
            this->previous(newToken);
            newToken->next(this);
        } else {
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
    if (title && title[0])
        std::cout << "\n### " << title << " ###\n";
    std::cout << stringifyList(true, true, true, true, true, 0, 0) << std::endl;
}

void Token::printOut(const char *title, const std::vector<std::string> &fileNames) const
{
    if (title && title[0])
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
        if (isLong()) {
            if (_type == eString || _type == eChar)
                os << "L";
            else
                os << "long ";
        }
    }
    if (isExpandedMacro())
        os << "$";
    if (_str[0] != '\"' || _str.find("\0") == std::string::npos)
        os << _str;
    else {
        for (std::size_t i = 0U; i < _str.size(); ++i) {
            if (_str[i] == '\0')
                os << "\\0";
            else
                os << _str[i];
        }
    }
    if (varid && _varId != 0)
        os << '@' << _varId;
}

std::string Token::stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames, const Token* end) const
{
    if (this == end)
        return "";

    std::ostringstream ret;

    unsigned int lineNumber = _linenr;
    int fileInd = files ? -1 : static_cast<int>(_fileIndex);
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

void Token::astOperand1(Token *tok)
{
    // goto parent operator
    while (tok->_astParent)
        tok = tok->_astParent;
    tok->_astParent = this;
    _astOperand1 = tok;
}

void Token::astOperand2(Token *tok)
{
    // goto parent operator
    while (tok->_astParent)
        tok = tok->_astParent;
    tok->_astParent = this;
    _astOperand2 = tok;
}

bool Token::isCalculation() const
{
    if (!Token::Match(this, "%cop%|++|--"))
        return false;

    if (Token::Match(this, "*|&")) {
        // dereference or address-of?
        if (!this->astOperand2())
            return false;

        if (this->astOperand2()->str() == "[")
            return false;

        // type specification?
        std::stack<const Token *> operands;
        operands.push(this);
        while (!operands.empty()) {
            const Token *op = operands.top();
            operands.pop();
            if (op->isNumber() || op->varId() > 0)
                return true;
            if (op->astOperand1())
                operands.push(op->astOperand1());
            if (op->astOperand2())
                operands.push(op->astOperand2());
            else if (Token::Match(op, "*|&"))
                return false;
        }

        // type specification => return false
        return false;
    }

    return true;
}

void Token::printAst() const
{
    bool title = false;

    bool print = true;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (print && tok->_astOperand1) {
            if (!title)
                std::cout << "\n\n##AST" << std::endl;
            title = true;
            std::cout << tok->astTop()->astString(" ") << std::endl;
            print = false;
        }
        if (Token::Match(tok, "[;{}]"))
            print = true;
    }
}
