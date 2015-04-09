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

#include "token.h"
#include "errorlogger.h"
#include "check.h"
#include "settings.h"
#include "symboldatabase.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <cctype>
#include <sstream>
#include <map>
#include <stack>
#include <algorithm>


Token::Token(Token **t) :
    tokensBack(t),
    _next(0),
    _previous(0),
    _link(0),
    _scope(0),
    _function(0), // Initialize whole union
    _varId(0),
    _fileIndex(0),
    _linenr(0),
    _progressValue(0),
    _type(eNone),
    _flags(0),
    _astOperand1(nullptr),
    _astOperand2(nullptr),
    _astParent(nullptr),
    _originalName(nullptr)
{
}

Token::~Token()
{
    delete _originalName;
}

void Token::update_property_info()
{
    if (!_str.empty()) {
        if (_str == "true" || _str == "false")
            _type = eBoolean;
        else if (_str[0] == '_' || std::isalpha((unsigned char)_str[0])) { // Name
            if (_varId)
                _type = eVariable;
            else if (_type != eVariable && _type != eFunction && _type != eType && _type != eKeyword)
                _type = eName;
        } else if (std::isdigit((unsigned char)_str[0]) || (_str.length() > 1 && _str[0] == '-' && std::isdigit((unsigned char)_str[1])))
            _type = eNumber;
        else if (_str.length() > 1 && _str[0] == '"' && _str[_str.length()-1] == '"')
            _type = eString;
        else if (_str.length() > 1 && _str[0] == '\'' && _str[_str.length()-1] == '\'')
            _type = eChar;
        else if (_str == "=" || _str == "<<=" || _str == ">>=" ||
                 (_str.size() == 2U && _str[1] == '=' && std::strchr("+-*/%&^|", _str[0])))
            _type = eAssignmentOp;
        else if (_str.size() == 1 && _str.find_first_of(",[]()?:") != std::string::npos)
            _type = eExtendedOp;
        else if (_str=="<<" || _str==">>" || (_str.size()==1 && _str.find_first_of("+-*/%") != std::string::npos))
            _type = eArithmeticalOp;
        else if (_str.size() == 1 && _str.find_first_of("&|^~") != std::string::npos)
            _type = eBitOp;
        else if (_str.size() <= 2 &&
                 (_str == "&&" ||
                  _str == "||" ||
                  _str == "!"))
            _type = eLogicalOp;
        else if (_str.size() <= 2 && !_link &&
                 (_str == "==" ||
                  _str == "!=" ||
                  _str == "<"  ||
                  _str == "<=" ||
                  _str == ">"  ||
                  _str == ">="))
            _type = eComparisonOp;
        else if (_str.size() == 2 &&
                 (_str == "++" ||
                  _str == "--"))
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
    isStandardType(false);

    if (_str.size() < 3)
        return;

    static const char * const stdtype[] = { "bool", "char", "char16_t", "char32_t", "double", "float", "int", "long", "short", "size_t", "void", "wchar_t"};
    if (std::binary_search(stdtype, stdtype + sizeof(stdtype) / sizeof(stdtype[0]), _str)) {
        isStandardType(true);
        _type = eType;
    }
}


bool Token::isUpperCaseName() const
{
    if (!isName())
        return false;
    for (size_t i = 0; i < _str.length(); ++i) {
        if (std::islower(_str[i]))
            return false;
    }
    return true;
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
    while (_next && index) {
        Token *n = _next;
        _next = n->next();
        delete n;
        --index;
    }

    if (_next)
        _next->previous(this);
    else if (tokensBack)
        *tokensBack = this;
}

void Token::swapWithNext()
{
    if (_next) {
        Token temp(0);

        temp._str = _next->_str;
        temp._type = _next->_type;
        temp._flags = _next->_flags;
        temp._varId = _next->_varId;
        temp._fileIndex = _next->_fileIndex;
        temp._link = _next->_link;
        temp._scope = _next->_scope;
        temp._function = _next->_function;
        temp._originalName = _next->_originalName;
        temp.values = _next->values;
        temp._progressValue = _next->_progressValue;

        _next->_str = _str;
        _next->_type = _type;
        _next->_flags = _flags;
        _next->_varId = _varId;
        _next->_fileIndex = _fileIndex;
        _next->_link = _link;
        _next->_scope = _scope;
        _next->_function = _function;
        _next->_originalName = _originalName;
        _next->values = values;
        _next->_progressValue = _progressValue;

        _str = temp._str;
        _type = temp._type;
        _flags = temp._flags;
        _varId = temp._varId;
        _fileIndex = temp._fileIndex;
        _link = temp._link;
        _scope = temp._scope;
        _function = temp._function;
        _originalName = temp._originalName;
        values = temp.values;
        _progressValue = temp._progressValue;
    }
}

void Token::deleteThis()
{
    if (_next) { // Copy next to this and delete next
        _str = _next->_str;
        _type = _next->_type;
        _flags = _next->_flags;
        _varId = _next->_varId;
        _fileIndex = _next->_fileIndex;
        _linenr = _next->_linenr;
        _link = _next->_link;
        _scope = _next->_scope;
        _function = _next->_function;
        _variable = _next->_variable;
        if (_next->_originalName) {
            _originalName = _next->_originalName;
            _next->_originalName = nullptr;
        }
        values = _next->values;
        if (_link)
            _link->link(this);

        deleteNext();
    } else if (_previous && _previous->_previous) { // Copy previous to this and delete previous
        _str = _previous->_str;
        _type = _previous->_type;
        _flags = _previous->_flags;
        _varId = _previous->_varId;
        _fileIndex = _previous->_fileIndex;
        _linenr = _previous->_linenr;
        _link = _previous->_link;
        _scope = _previous->_scope;
        _function = _previous->_function;
        _variable = _previous->_variable;
        if (_previous->_originalName) {
            _originalName = _previous->_originalName;
            _previous->_originalName = nullptr;
        }
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
    const Token *tok = this->tokAt(index);
    return tok ? tok->_str : emptyString;
}

static int multiComparePercent(const Token *tok, const char*& haystack, bool emptyStringFound, unsigned int varid)
{
    ++haystack;
    // Compare only the first character of the string for optimization reasons
    switch (haystack[0]) {
    case '\0':
    case ' ':
    case '|':
        //simple '%' character
        haystack += 1;
        if (tok->isArithmeticalOp() && tok->str() == "%")
            return 1;
        break;
    case 'v':
        if (haystack[3] == '%') { // %var%
            haystack += 4;
            if (tok->varId() != 0)
                return 1;
        } else { // %varid%
            if (varid == 0) {
                throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");
            }

            haystack += 6;

            if (tok->varId() == varid)
                return 1;
        }
        break;
    case 't':
        // Type (%type%)
    {
        haystack += 5;
        if (tok->isName() && tok->varId() == 0 && !tok->isKeyword())
            return 1;
    }
    break;
    case 'a':
        // Accept any token (%any%)
    {
        haystack += 4;
        return 1;
    }
    case 'n':
        // Number (%num%) or name (%name%)
    {
        if (haystack[4] == '%') { // %name%
            haystack += 5;
            if (tok->isName())
                return 1;
        } else {
            haystack += 4;
            if (tok->isNumber())
                return 1;
        }
    }
    break;
    case 'c': {
        haystack += 1;
        // Character (%char%)
        if (haystack[0] == 'h') {
            haystack += 4;
            if (tok->type() == Token::eChar)
                return 1;
        }
        // Const operator (%cop%)
        else if (haystack[1] == 'p') {
            haystack += 3;
            if (tok->isConstOp())
                return 1;
        }
        // Comparison operator (%comp%)
        else {
            haystack += 4;
            if (tok->isComparisonOp())
                return 1;
        }
    }
    break;
    case 's':
        // String (%str%)
    {
        haystack += 4;
        if (tok->type() == Token::eString)
            return 1;
    }
    break;
    case 'b':
        // Bool (%bool%)
    {
        haystack += 5;
        if (tok->isBoolean())
            return 1;
    }
    break;
    case 'o': {
        ++haystack;
        if (haystack[1] == '%') {
            // Op (%op%)
            if (haystack[0] == 'p') {
                haystack += 2;
                if (tok->isOp())
                    return 1;
            }
            // Or (%or%)
            else {
                haystack += 2;
                if (tok->type() == Token::eBitOp && tok->str() == "|")
                    return 1;
            }
        }

        // Oror (%oror%)
        else {
            haystack += 4;
            if (tok->type() == Token::eLogicalOp && tok->str() == "||")
                return 1;
        }
    }
    break;
    default:
        //unknown %cmd%, abort
        throw InternalError(tok, "Unexpected command");
    }

    if (*haystack == '|')
        haystack += 1;
    else if (*haystack == ' ' || *haystack == '\0')
        return emptyStringFound ? 0 : -1;
    else
        return -1;

    return 0xFFFF;
}

int Token::multiCompare(const Token *tok, const char *haystack, unsigned int varid)
{
    bool emptyStringFound = false;
    const char *needle = tok->str().c_str();
    const char *needlePointer = needle;
    for (;;) {
        if (needlePointer == needle && haystack[0] == '%' && haystack[1] != '|' && haystack[1] != '\0' && haystack[1] != ' ') {
            int ret = multiComparePercent(tok, haystack, emptyStringFound, varid);
            if (ret < 2)
                return ret;
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
        } else if (*needlePointer == *haystack) {
            if (*needlePointer == '\0')
                return 1;
            ++needlePointer;
            ++haystack;
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
    if (!tok)
        return false; // shortcut
    const char *current  = pattern;
    const char *next = std::strchr(pattern, ' ');
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

bool Token::Match(const Token *tok, const char pattern[], unsigned int varid)
{
    const char *p = pattern;
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

        // [.. => search for a one-character token..
        if (p[0] == '[' && chrInFirstWord(p, ']')) {
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

        // Parse "not" options. Token can be anything except the given one
        else if (p[0] == '!' && p[1] == '!' && p[2] != '\0') {
            p += 2;
            if (firstWordEquals(p, tok->str().c_str()))
                return false;
            while (*p && *p != ' ')
                ++p;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else {
            int res = multiCompare(tok, p, varid);
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

        while (*p && *p != ' ')
            ++p;

        tok = tok->next();
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

std::size_t Token::getStrLength(const Token *tok)
{
    assert(tok != nullptr);

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

std::size_t Token::getStrSize(const Token *tok)
{
    assert(tok != nullptr && tok->type() == eString);
    const std::string &str = tok->str();
    unsigned int sizeofstring = 1U;
    for (unsigned int i = 1U; i < str.size() - 1U; i++) {
        if (str[i] == '\\')
            ++i;
        ++sizeofstring;
    }
    return sizeofstring;
}

std::string Token::getCharAt(const Token *tok, std::size_t index)
{
    assert(tok != nullptr);

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
        else if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ")|;"))
            return 0;
    }
    return 0;
}

Token* Token::nextArgumentBeforeCreateLinks2() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        else if (tok->link() && Token::Match(tok, "(|{|["))
            tok = tok->link();
        else if (tok->str() == "<") {
            const Token* temp = tok->findClosingBracket();
            if (temp)
                tok = temp;
        } else if (Token::Match(tok, ")|;"))
            return 0;
    }
    return 0;
}

Token* Token::nextTemplateArgument() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        else if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ">|;"))
            return 0;
    }
    return 0;
}

const Token * Token::findClosingBracket() const
{
    const Token *closing = nullptr;

    if (_str == "<") {
        unsigned int depth = 0;
        for (closing = this; closing != nullptr; closing = closing->next()) {
            if (Token::Match(closing, "{|[|("))
                closing = closing->link();
            else if (Token::Match(closing, "}|]|)|;"))
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
    //TODO: Find a solution for the first token on the list
    if (prepend && !this->previous())
        return;
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
    //TODO: Find a solution for the first token on the list
    if (prepend && !this->previous())
        return;

    Token *newToken;
    if (_str.empty())
        newToken = this;
    else
        newToken = new Token(tokensBack);
    newToken->str(tokenStr);
    if (!originalNameStr.empty())
        newToken->originalName(originalNameStr);
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
    assert(begin != nullptr);
    assert(end != nullptr);
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

void Token::stringify(std::ostream& os, bool varid, bool attributes, bool macro) const
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
    if (macro && isExpandedMacro())
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

        tok->stringify(ret, varid, attributes, attributes); // print token
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
    if (_astOperand1)
        _astOperand1->_astParent = nullptr;
    // goto parent operator
    if (tok) {
        while (tok->_astParent)
            tok = tok->_astParent;
        tok->_astParent = this;
    }
    _astOperand1 = tok;
}

void Token::astOperand2(Token *tok)
{
    if (_astOperand2)
        _astOperand2->_astParent = nullptr;
    // goto parent operator
    if (tok) {
        while (tok->_astParent)
            tok = tok->_astParent;
        tok->_astParent = this;
    }
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

static bool isUnaryPreOp(const Token *op)
{
    if (!op->astOperand1() || op->astOperand2())
        return false;
    if (!Token::Match(op, "++|--"))
        return true;
    const Token *tok = op->astOperand1();
    for (int distance = 1; distance < 10; distance++) {
        if (tok == op->tokAt(-distance))
            return false;
        if (tok == op->tokAt(distance))
            return true;
    }
    return false; // <- guess
}

std::string Token::expressionString() const
{
    const Token * const top = this;
    const Token *start = top;
    while (start->astOperand1() && start->astOperand2())
        start = start->astOperand1();
    const Token *end = top;
    while (end->astOperand1() && (end->astOperand2() || isUnaryPreOp(end))) {
        if (Token::Match(end,"(|[")) {
            end = end->link();
            break;
        }
        end = end->astOperand2() ? end->astOperand2() : end->astOperand1();
    }
    std::string ret;
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        ret += tok->str();
        if (Token::Match(tok, "%name%|%num% %name%|%num%"))
            ret += " ";
    }
    return ret + end->str();

}

static void astStringXml(const Token *tok, std::size_t indent, std::ostream &out)
{
    const std::string strindent(indent, ' ');

    out << strindent << "<token str=\"" << tok->str() << '\"';
    if (tok->varId() > 0U)
        out << " varId=\"" << MathLib::toString(tok->varId()) << '\"';
    if (tok->variable())
        out << " variable=\"" << tok->variable() << '\"';
    if (tok->function())
        out << " function=\"" << tok->function() << '\"';
    if (!tok->values.empty())
        out << " values=\"" << &tok->values << '\"';

    if (!tok->astOperand1() && !tok->astOperand2()) {
        out << "/>" << std::endl;
    }

    else {
        out << '>' << std::endl;
        if (tok->astOperand1())
            astStringXml(tok->astOperand1(), indent+2U, out);
        if (tok->astOperand2())
            astStringXml(tok->astOperand2(), indent+2U, out);
        out << strindent << "</token>" << std::endl;
    }
}

void Token::printAst(bool verbose, bool xml, std::ostream &out) const
{
    bool title = false;

    bool print = true;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (print && tok->_astOperand1) {
            if (!title && !xml)
                out << "\n\n##AST" << std::endl;
            title = true;
            if (xml) {
                out << "<ast scope=\"" << tok->scope() << "\" fileIndex=\"" << tok->fileIndex() << "\" linenr=\"" << tok->linenr() << "\">" << std::endl;
                astStringXml(tok->astTop(), 2U, out);
                out << "</ast>" << std::endl;
            } else if (verbose)
                out << tok->astTop()->astStringVerbose(0,0) << std::endl;
            else
                out << tok->astTop()->astString(" ") << std::endl;
            print = false;
            if (tok->str() == "(")
                tok = tok->link();
        }
        if (Token::Match(tok, "[;{}]"))
            print = true;
    }
}

static std::string indent(const unsigned int indent1, const unsigned int indent2)
{
    std::string ret(indent1,' ');
    for (unsigned int i = indent1; i < indent2; i += 2)
        ret += "| ";
    return ret;
}

std::string Token::astStringVerbose(const unsigned int indent1, const unsigned int indent2) const
{
    std::string ret;

    if (isExpandedMacro())
        ret += "$";
    ret += _str + "\n";

    if (_astOperand1) {
        unsigned int i1 = indent1, i2 = indent2 + 2;
        if (indent1==indent2 && !_astOperand2)
            i1 += 2;
        ret += indent(indent1,indent2) + (_astOperand2 ? "|-" : "`-") + _astOperand1->astStringVerbose(i1,i2);
    }
    if (_astOperand2) {
        unsigned int i1 = indent1, i2 = indent2 + 2;
        if (indent1==indent2)
            i1 += 2;
        ret += indent(indent1,indent2) + "`-" + _astOperand2->astStringVerbose(i1,i2);
    }
    return ret;
}


void Token::printValueFlow(bool xml, std::ostream &out) const
{
    unsigned int line = 0;
    if (xml)
        out << "  <valueflow>" << std::endl;
    else
        out << "\n\n##Value flow" << std::endl;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (tok->values.empty())
            continue;
        if (xml)
            out << "    <values id=\"" << &tok->values << "\">" << std::endl;
        else if (line != tok->linenr())
            out << "Line " << tok->linenr() << std::endl;
        line = tok->linenr();
        if (!xml)
            out << "  " << tok->str() << ":{";
        for (std::list<ValueFlow::Value>::const_iterator it=tok->values.begin(); it!=tok->values.end(); ++it) {
            if (xml) {
                out << "      <value ";
                if (it->tokvalue)
                    out << "tokvalue=\"" << it->tokvalue << '\"';
                else
                    out << "intvalue=\"" << it->intvalue << '\"';
                if (it->condition)
                    out << " condition-line=\"" << it->condition->linenr() << '\"';
                out << "/>" << std::endl;
            }

            else {
                if (it != tok->values.begin())
                    out << ",";
                if (it->tokvalue)
                    out << it->tokvalue->str();
                else
                    out << it->intvalue;
            }
        }
        if (xml)
            out << "    </values>" << std::endl;
        else
            out << "}" << std::endl;
    }
    if (xml)
        out << "  </valueflow>" << std::endl;
}

const ValueFlow::Value * Token::getValueLE(const MathLib::bigint val, const Settings *settings) const
{
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        if (it->intvalue <= val && !it->tokvalue) {
            if (!ret || ret->inconclusive || (ret->condition && !it->inconclusive))
                ret = &(*it);
            if (!ret->inconclusive && !ret->condition)
                break;
        }
    }
    if (settings && ret) {
        if (ret->inconclusive && !settings->inconclusive)
            return nullptr;
        if (ret->condition && !settings->isEnabled("warning"))
            return nullptr;
    }
    return ret;
}

const ValueFlow::Value * Token::getValueGE(const MathLib::bigint val, const Settings *settings) const
{
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        if (it->intvalue >= val && !it->tokvalue) {
            if (!ret || ret->inconclusive || (ret->condition && !it->inconclusive))
                ret = &(*it);
            if (!ret->inconclusive && !ret->condition)
                break;
        }
    }
    if (settings && ret) {
        if (ret->inconclusive && !settings->inconclusive)
            return nullptr;
        if (ret->condition && !settings->isEnabled("warning"))
            return nullptr;
    }
    return ret;
}

const Token *Token::getValueTokenMinStrSize() const
{
    const Token *ret = nullptr;
    std::size_t minsize = ~0U;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        if (it->tokvalue && it->tokvalue->type() == Token::eString) {
            std::size_t size = getStrSize(it->tokvalue);
            if (!ret || size < minsize) {
                minsize = size;
                ret = it->tokvalue;
            }
        }
    }
    return ret;
}

const Token *Token::getValueTokenMaxStrLength() const
{
    const Token *ret = nullptr;
    std::size_t maxlength = 0U;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        if (it->tokvalue && it->tokvalue->type() == Token::eString) {
            std::size_t length = getStrLength(it->tokvalue);
            if (!ret || length > maxlength) {
                maxlength = length;
                ret = it->tokvalue;
            }
        }
    }
    return ret;
}

static const Scope *getfunctionscope(const Scope *s)
{
    while (s && s->type != Scope::eFunction)
        s = s->nestedIn;
    return s;
}

const Token *Token::getValueTokenDeadPointer() const
{
    const Scope * const functionscope = getfunctionscope(this->scope());

    std::list<ValueFlow::Value>::const_iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        // Is this a pointer alias?
        if (!it->tokvalue || it->tokvalue->str() != "&")
            continue;
        // Get variable
        const Token *vartok = it->tokvalue->astOperand1();
        if (!vartok || !vartok->isName() || !vartok->variable())
            continue;
        const Variable * const var = vartok->variable();
        if (var->isStatic() || var->isReference())
            continue;
        if (var->scope()->type == Scope::eUnion && var->scope()->nestedIn == this->scope())
            continue;
        // variable must be in same function (not in subfunction)
        if (functionscope != getfunctionscope(var->scope()))
            continue;
        // Is variable defined in this scope or upper scope?
        const Scope *s = this->scope();
        while ((s != nullptr) && (s != var->scope()))
            s = s->nestedIn;
        if (!s)
            return it->tokvalue;
    }
    return nullptr;
}

void Token::assignProgressValues(Token *tok)
{
    unsigned int total_count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        ++total_count;
    unsigned int count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        tok2->_progressValue = count++ * 100 / total_count;
}
