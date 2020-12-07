/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "astutils.h"
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenlist.h"
#include "utils.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <utility>

const std::list<ValueFlow::Value> TokenImpl::mEmptyValueList;

Token::Token(TokensFrontBack *tokensFrontBack) :
    mTokensFrontBack(tokensFrontBack),
    mNext(nullptr),
    mPrevious(nullptr),
    mLink(nullptr),
    mTokType(eNone),
    mFlags(0)
{
    mImpl = new TokenImpl();
}

Token::~Token()
{
    delete mImpl;
}

static const std::set<std::string> controlFlowKeywords = {
    "goto",
    "do",
    "if",
    "else",
    "for",
    "while",
    "switch",
    "case",
    "break",
    "continue",
    "return"
};

void Token::update_property_info()
{
    setFlag(fIsControlFlowKeyword, controlFlowKeywords.find(mStr) != controlFlowKeywords.end());

    if (!mStr.empty()) {
        if (mStr == "true" || mStr == "false")
            tokType(eBoolean);
        else if (isStringLiteral(mStr))
            tokType(eString);
        else if (isCharLiteral(mStr))
            tokType(eChar);
        else if (std::isalpha((unsigned char)mStr[0]) || mStr[0] == '_' || mStr[0] == '$') { // Name
            if (mImpl->mVarId)
                tokType(eVariable);
            else if (mTokensFrontBack && mTokensFrontBack->list && mTokensFrontBack->list->isKeyword(mStr))
                tokType(eKeyword);
            else if (mTokType != eVariable && mTokType != eFunction && mTokType != eType && mTokType != eKeyword)
                tokType(eName);
        } else if (std::isdigit((unsigned char)mStr[0]) || (mStr.length() > 1 && mStr[0] == '-' && std::isdigit((unsigned char)mStr[1])))
            tokType(eNumber);
        else if (mStr == "=" || mStr == "<<=" || mStr == ">>=" ||
                 (mStr.size() == 2U && mStr[1] == '=' && std::strchr("+-*/%&^|", mStr[0])))
            tokType(eAssignmentOp);
        else if (mStr.size() == 1 && mStr.find_first_of(",[]()?:") != std::string::npos)
            tokType(eExtendedOp);
        else if (mStr=="<<" || mStr==">>" || (mStr.size()==1 && mStr.find_first_of("+-*/%") != std::string::npos))
            tokType(eArithmeticalOp);
        else if (mStr.size() == 1 && mStr.find_first_of("&|^~") != std::string::npos)
            tokType(eBitOp);
        else if (mStr.size() <= 2 &&
                 (mStr == "&&" ||
                  mStr == "||" ||
                  mStr == "!"))
            tokType(eLogicalOp);
        else if (mStr.size() <= 2 && !mLink &&
                 (mStr == "==" ||
                  mStr == "!=" ||
                  mStr == "<"  ||
                  mStr == "<=" ||
                  mStr == ">"  ||
                  mStr == ">="))
            tokType(eComparisonOp);
        else if (mStr.size() == 2 &&
                 (mStr == "++" ||
                  mStr == "--"))
            tokType(eIncDecOp);
        else if (mStr.size() == 1 && (mStr.find_first_of("{}") != std::string::npos || (mLink && mStr.find_first_of("<>") != std::string::npos)))
            tokType(eBracket);
        else if (mStr == "...")
            tokType(eEllipsis);
        else
            tokType(eOther);
    } else {
        tokType(eNone);
    }

    update_property_char_string_literal();
    update_property_isStandardType();
}

static const std::set<std::string> stdTypes = { "bool"
                                                , "_Bool"
                                                , "char"
                                                , "double"
                                                , "float"
                                                , "int"
                                                , "long"
                                                , "short"
                                                , "size_t"
                                                , "void"
                                                , "wchar_t"
                                              };

void Token::update_property_isStandardType()
{
    isStandardType(false);

    if (mStr.size() < 3)
        return;

    if (stdTypes.find(mStr)!=stdTypes.end()) {
        isStandardType(true);
        tokType(eType);
    }
}

void Token::update_property_char_string_literal()
{
    if (mTokType != Token::eString && mTokType != Token::eChar)
        return;

    isLong(((mTokType == Token::eString) && isPrefixStringCharLiteral(mStr, '"', "L")) ||
           ((mTokType == Token::eChar) && isPrefixStringCharLiteral(mStr, '\'', "L")));
}

bool Token::isUpperCaseName() const
{
    if (!isName())
        return false;
    for (char i : mStr) {
        if (std::islower(i))
            return false;
    }
    return true;
}

void Token::concatStr(std::string const& b)
{
    mStr.erase(mStr.length() - 1);
    mStr.append(getStringLiteral(b) + "\"");

    if (isCChar() && isStringLiteral(b) && b[0] != '"') {
        mStr.insert(0, b.substr(0, b.find('"')));
    }
    update_property_info();
}

std::string Token::strValue() const
{
    assert(mTokType == eString);
    std::string ret(getStringLiteral(mStr));
    std::string::size_type pos = 0U;
    while ((pos = ret.find('\\', pos)) != std::string::npos) {
        ret.erase(pos,1U);
        if (ret[pos] >= 'a') {
            if (ret[pos] == 'n')
                ret[pos] = '\n';
            else if (ret[pos] == 'r')
                ret[pos] = '\r';
            else if (ret[pos] == 't')
                ret[pos] = '\t';
        }
        if (ret[pos] == '0')
            return ret.substr(0,pos);
        pos++;
    }
    return ret;
}

void Token::deleteNext(nonneg int count)
{
    while (mNext && count > 0) {
        Token *n = mNext;

        // #8154 we are about to be unknown -> destroy the link to us
        if (n->mLink && n->mLink->mLink == n)
            n->mLink->link(nullptr);

        mNext = n->next();
        delete n;
        --count;
    }

    if (mNext)
        mNext->previous(this);
    else if (mTokensFrontBack)
        mTokensFrontBack->back = this;
}

void Token::deletePrevious(nonneg int count)
{
    while (mPrevious && count > 0) {
        Token *p = mPrevious;

        // #8154 we are about to be unknown -> destroy the link to us
        if (p->mLink && p->mLink->mLink == p)
            p->mLink->link(nullptr);

        mPrevious = p->previous();
        delete p;
        --count;
    }

    if (mPrevious)
        mPrevious->next(this);
    else if (mTokensFrontBack)
        mTokensFrontBack->front = this;
}

void Token::swapWithNext()
{
    if (mNext) {
        std::swap(mStr, mNext->mStr);
        std::swap(mTokType, mNext->mTokType);
        std::swap(mFlags, mNext->mFlags);
        std::swap(mImpl, mNext->mImpl);
        if (mImpl->mTemplateSimplifierPointers)
            for (auto *templateSimplifierPointer : *mImpl->mTemplateSimplifierPointers) {
                templateSimplifierPointer->token(this);
            }

        if (mNext->mImpl->mTemplateSimplifierPointers)
            for (auto *templateSimplifierPointer : *mNext->mImpl->mTemplateSimplifierPointers) {
                templateSimplifierPointer->token(mNext);
            }
        if (mNext->mLink)
            mNext->mLink->mLink = this;
        if (this->mLink)
            this->mLink->mLink = mNext;
        std::swap(mLink, mNext->mLink);
    }
}

void Token::takeData(Token *fromToken)
{
    mStr = fromToken->mStr;
    tokType(fromToken->mTokType);
    mFlags = fromToken->mFlags;
    delete mImpl;
    mImpl = fromToken->mImpl;
    fromToken->mImpl = nullptr;
    if (mImpl->mTemplateSimplifierPointers)
        for (auto *templateSimplifierPointer : *mImpl->mTemplateSimplifierPointers) {
            templateSimplifierPointer->token(this);
        }
    mLink = fromToken->mLink;
    if (mLink)
        mLink->link(this);
}

void Token::deleteThis()
{
    if (mNext) { // Copy next to this and delete next
        takeData(mNext);
        mNext->link(nullptr); // mark as unlinked
        deleteNext();
    } else if (mPrevious) { // Copy previous to this and delete previous
        takeData(mPrevious);
        mPrevious->link(nullptr);
        deletePrevious();
    } else {
        // We are the last token in the list, we can't delete
        // ourselves, so just make us empty
        str(";");
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

    if (end->mTokensFrontBack && end->mTokensFrontBack->back == end) {
        while (end->next())
            end = end->next();
        end->mTokensFrontBack->back = end;
    }

    // Update mProgressValue, fileIndex and linenr
    for (Token *tok = start; tok != end->next(); tok = tok->next())
        tok->mImpl->mProgressValue = replaceThis->mImpl->mProgressValue;

    // Delete old token, which is replaced
    delete replaceThis;
}

const Token *Token::tokAt(int index) const
{
    const Token *tok = this;
    while (index > 0 && tok) {
        tok = tok->next();
        --index;
    }
    while (index < 0 && tok) {
        tok = tok->previous();
        ++index;
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
    return tok ? tok->mStr : emptyString;
}

static int multiComparePercent(const Token *tok, const char*& haystack, nonneg int varid)
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
        if (tok->isName() && tok->varId() == 0 && (tok->str() != "delete" || !tok->isKeyword())) // HACK: this is legacy behaviour, it should return false for all keywords, except types
            return 1;
    }
    break;
    case 'a':
        // Accept any token (%any%) or assign (%assign%)
    {
        if (haystack[3] == '%') { // %any%
            haystack += 4;
            return 1;
        } else { // %assign%
            haystack += 7;
            if (tok->isAssignmentOp())
                return 1;
        }
    }
    break;
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
            if (tok->tokType() == Token::eChar)
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
        if (tok->tokType() == Token::eString)
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
                if (tok->tokType() == Token::eBitOp && tok->str() == "|")
                    return 1;
            }
        }

        // Oror (%oror%)
        else {
            haystack += 4;
            if (tok->tokType() == Token::eLogicalOp && tok->str() == "||")
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
    else
        return -1;

    return 0xFFFF;
}

int Token::multiCompare(const Token *tok, const char *haystack, nonneg int varid)
{
    const char *needle = tok->str().c_str();
    const char *needlePointer = needle;
    for (;;) {
        if (needlePointer == needle && haystack[0] == '%' && haystack[1] != '|' && haystack[1] != '\0' && haystack[1] != ' ') {
            const int ret = multiComparePercent(tok, haystack, varid);
            if (ret < 2)
                return ret;
        } else if (*haystack == '|') {
            if (*needlePointer == 0) {
                // If needle is at the end, we have a match.
                return 1;
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
                return -1;
            }

            ++haystack;
        }
    }

    if (*needlePointer == '\0')
        return 1;

    return -1;
}

bool Token::simpleMatch(const Token *tok, const char pattern[], size_t pattern_len)
{
    if (!tok)
        return false; // shortcut
    const char *current = pattern;
    const char *end = pattern + pattern_len;
    const char *next = (const char*)std::memchr(pattern, ' ', pattern_len);
    if (!next)
        next = end;

    while (*current) {
        const std::size_t length = next - current;

        if (!tok || length != tok->mStr.length() || std::strncmp(current, tok->mStr.c_str(), length))
            return false;

        current = next;
        if (*next) {
            next = std::strchr(++current, ' ');
            if (!next)
                next = end;
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
            return nullptr;

        if (*str == c)
            return str;

        ++str;
    }
}

bool Token::Match(const Token *tok, const char pattern[], nonneg int varid)
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
            int count = 0;
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
            const int res = multiCompare(tok, p, varid);
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

nonneg int Token::getStrLength(const Token *tok)
{
    assert(tok != nullptr);
    assert(tok->mTokType == eString);

    int len = 0;
    const std::string str(getStringLiteral(tok->str()));
    std::string::const_iterator it = str.begin();
    const std::string::const_iterator end = str.end();

    while (it != end) {
        if (*it == '\\') {
            ++it;

            // string ends at '\0'
            if (*it == '0')
                return len;
        }

        if (*it == '\0')
            return len;

        ++it;
        ++len;
    }

    return len;
}

nonneg int Token::getStrArraySize(const Token *tok)
{
    assert(tok != nullptr);
    assert(tok->tokType() == eString);
    const std::string str(getStringLiteral(tok->str()));
    int sizeofstring = 1;
    for (int i = 0; i < (int)str.size(); i++) {
        if (str[i] == '\\')
            ++i;
        ++sizeofstring;
    }
    return sizeofstring;
}

nonneg int Token::getStrSize(const Token *tok, const Settings *settings)
{
    assert(tok != nullptr && tok->tokType() == eString);
    nonneg int sizeofType = 1;
    if (tok->valueType()) {
        ValueType vt(*tok->valueType());
        vt.pointer = 0;
        sizeofType = ValueFlow::getSizeOf(vt, settings);
    }
    return getStrArraySize(tok) * sizeofType;
}

std::string Token::getCharAt(const Token *tok, MathLib::bigint index)
{
    assert(tok != nullptr);
    std::string str(getStringLiteral(tok->str()));
    std::string::const_iterator it = str.begin();
    const std::string::const_iterator end = str.end();

    while (it != end) {
        if (index == 0) {
            if (*it == '\0')
                return "\\0";

            std::string ret(1, *it);
            if (*it == '\\') {
                ++it;
                ret += *it;
            }
            return ret;
        }

        if (*it == '\\')
            ++it;
        ++it;
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
        tok->mImpl->mProgressValue = newLocation->mImpl->mProgressValue;
}

Token* Token::nextArgument() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        else if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ")|;"))
            return nullptr;
    }
    return nullptr;
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
            return nullptr;
    }
    return nullptr;
}

Token* Token::nextTemplateArgument() const
{
    for (const Token* tok = this; tok; tok = tok->next()) {
        if (tok->str() == ",")
            return tok->next();
        else if (tok->link() && Token::Match(tok, "(|{|[|<"))
            tok = tok->link();
        else if (Token::Match(tok, ">|;"))
            return nullptr;
    }
    return nullptr;
}

static bool isOperator(const Token *tok)
{
    if (tok->link())
        tok = tok->link();
    // TODO handle multi token operators
    return tok->strAt(-1) == "operator";
}

const Token * Token::findClosingBracket() const
{
    if (mStr != "<")
        return nullptr;

    if (!mPrevious)
        return nullptr;

    if (!(mPrevious->isName() ||
          Token::Match(mPrevious->previous(), "operator %op% <") ||
          Token::Match(mPrevious->tokAt(-2), "operator [([] [)]] <")))
        return nullptr;

    const Token *closing = nullptr;
    const bool templateParameter(strAt(-1) == "template");
    std::set<std::string> templateParameters;

    bool isDecl = true;
    for (const Token *prev = previous(); prev; prev = prev->previous()) {
        if (prev->str() == "=")
            isDecl = false;
        if (Token::simpleMatch(prev, "template <"))
            isDecl = true;
        if (Token::Match(prev, "[;{}]"))
            break;
    }

    unsigned int depth = 0;
    for (closing = this; closing != nullptr; closing = closing->next()) {
        if (Token::Match(closing, "{|[|(")) {
            closing = closing->link();
            if (!closing)
                return nullptr; // #6803
        } else if (Token::Match(closing, "}|]|)|;"))
            return nullptr;
        // we can make some guesses for template parameters
        else if (closing->str() == "<" && closing->previous() &&
                 (closing->previous()->isName() || isOperator(closing->previous())) &&
                 (templateParameter ? templateParameters.find(closing->strAt(-1)) == templateParameters.end() : true))
            ++depth;
        else if (closing->str() == ">") {
            if (--depth == 0)
                return closing;
        } else if (closing->str() == ">>" || closing->str() == ">>=") {
            if (!isDecl && depth == 1)
                continue;
            if (depth <= 2)
                return closing;
            depth -= 2;
        }
        // save named template parameter
        else if (templateParameter && depth == 1 && closing->str() == "," &&
                 closing->previous()->isName() && !Match(closing->previous(), "class|typename|."))
            templateParameters.insert(closing->strAt(-1));
    }

    return closing;
}

Token * Token::findClosingBracket()
{
    // return value of const function
    return const_cast<Token*>(const_cast<const Token*>(this)->findClosingBracket());
}

const Token * Token::findOpeningBracket() const
{
    if (mStr != ">")
        return nullptr;

    const Token *opening = nullptr;

    unsigned int depth = 0;
    for (opening = this; opening != nullptr; opening = opening->previous()) {
        if (Token::Match(opening, "}|]|)")) {
            opening = opening->link();
            if (!opening)
                return nullptr;
        } else if (Token::Match(opening, "{|{|(|;"))
            return nullptr;
        else if (opening->str() == ">")
            ++depth;
        else if (opening->str() == "<") {
            if (--depth == 0)
                return opening;
        }
    }

    return opening;
}

Token * Token::findOpeningBracket()
{
    // return value of const function
    return const_cast<Token*>(const_cast<const Token*>(this)->findOpeningBracket());
}

//---------------------------------------------------------------------------

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len)
{
    for (const Token* tok = startTok; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern, pattern_len))
            return tok;
    }
    return nullptr;
}

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len, const Token * const end)
{
    for (const Token* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern, pattern_len))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const nonneg int varId)
{
    for (const Token* tok = startTok; tok; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const Token * const end, const nonneg int varId)
{
    for (const Token* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
}

void Token::function(const Function *f)
{
    mImpl->mFunction = f;
    if (f) {
        if (f->isLambda())
            tokType(eLambda);
        else
            tokType(eFunction);
    } else if (mTokType == eFunction)
        tokType(eName);
}

void Token::insertToken(const std::string &tokenStr, const std::string &originalNameStr, bool prepend)
{
    Token *newToken;
    if (mStr.empty())
        newToken = this;
    else
        newToken = new Token(mTokensFrontBack);
    newToken->str(tokenStr);
    if (!originalNameStr.empty())
        newToken->originalName(originalNameStr);

    if (newToken != this) {
        newToken->mImpl->mLineNumber = mImpl->mLineNumber;
        newToken->mImpl->mFileIndex = mImpl->mFileIndex;
        newToken->mImpl->mProgressValue = mImpl->mProgressValue;

        if (prepend) {
            if (this->previous()) {
                newToken->previous(this->previous());
                newToken->previous()->next(newToken);
            } else if (mTokensFrontBack) {
                mTokensFrontBack->front = newToken;
            }
            this->previous(newToken);
            newToken->next(this);
        } else {
            if (this->next()) {
                newToken->next(this->next());
                newToken->next()->previous(newToken);
            } else if (mTokensFrontBack) {
                mTokensFrontBack->back = newToken;
            }
            this->next(newToken);
            newToken->previous(this);
        }

        if (mImpl->mScopeInfo) {
            // If the brace is immediately closed there is no point opening a new scope for it
            if (newToken->str() == "{") {
                std::string nextScopeNameAddition;
                // This might be the opening of a member function
                Token *tok1 = newToken;
                while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                    tok1 = tok1->previous();
                if (tok1->previous() && tok1->strAt(-1) == ")") {
                    tok1 = tok1->linkAt(-1);
                    if (Token::Match(tok1->previous(), "throw|noexcept")) {
                        tok1 = tok1->previous();
                        while (Token::Match(tok1->previous(), "const|volatile|final|override|&|&&|noexcept"))
                            tok1 = tok1->previous();
                        if (tok1->strAt(-1) != ")")
                            return;
                    } else if (Token::Match(newToken->tokAt(-2), ":|, %name%")) {
                        tok1 = tok1->tokAt(-2);
                        if (tok1->strAt(-1) != ")")
                            return;
                    }
                    if (tok1->strAt(-1) == ">")
                        tok1 = tok1->previous()->findOpeningBracket();
                    if (tok1 && Token::Match(tok1->tokAt(-3), "%name% :: %name%")) {
                        tok1 = tok1->tokAt(-2);
                        std::string scope = tok1->strAt(-1);
                        while (Token::Match(tok1->tokAt(-2), ":: %name%")) {
                            scope = tok1->strAt(-3) + " :: " + scope;
                            tok1 = tok1->tokAt(-2);
                        }
                        nextScopeNameAddition += scope;
                    }
                }

                // Or it might be a namespace/class/struct
                if (Token::Match(newToken->previous(), "%name%|>")) {
                    Token* nameTok = newToken->previous();
                    while (nameTok && !Token::Match(nameTok, "namespace|class|struct|union %name% {|::|:|<")) {
                        nameTok = nameTok->previous();
                    }
                    if (nameTok) {
                        for (nameTok = nameTok->next(); nameTok && !Token::Match(nameTok, "{|:|<"); nameTok = nameTok->next()) {
                            nextScopeNameAddition.append(nameTok->str());
                            nextScopeNameAddition.append(" ");
                        }
                        if (nextScopeNameAddition.length() > 0) nextScopeNameAddition = nextScopeNameAddition.substr(0, nextScopeNameAddition.length() - 1);
                    }
                }

                // New scope is opening, record it here
                std::shared_ptr<ScopeInfo2> newScopeInfo = std::make_shared<ScopeInfo2>(mImpl->mScopeInfo->name, nullptr, mImpl->mScopeInfo->usingNamespaces);

                if (!newScopeInfo->name.empty() && !nextScopeNameAddition.empty()) newScopeInfo->name.append(" :: ");
                newScopeInfo->name.append(nextScopeNameAddition);
                nextScopeNameAddition = "";

                newToken->scopeInfo(newScopeInfo);
            } else if (newToken->str() == "}") {
                Token* matchingTok = newToken->previous();
                int depth = 0;
                while (matchingTok && (depth != 0 || !Token::simpleMatch(matchingTok, "{"))) {
                    if (Token::simpleMatch(matchingTok, "}")) depth++;
                    if (Token::simpleMatch(matchingTok, "{")) depth--;
                    matchingTok = matchingTok->previous();
                }
                if (matchingTok && matchingTok->previous()) {
                    newToken->mImpl->mScopeInfo = matchingTok->previous()->scopeInfo();
                }
            } else {
                if (prepend && newToken->previous()) {
                    newToken->mImpl->mScopeInfo = newToken->previous()->scopeInfo();
                } else {
                    newToken->mImpl->mScopeInfo = mImpl->mScopeInfo;
                }
                if (newToken->str() == ";") {
                    const Token* statementStart;
                    for (statementStart = newToken; statementStart->previous() && !Token::Match(statementStart->previous(), ";|{"); statementStart = statementStart->previous());
                    if (Token::Match(statementStart, "using namespace %name% ::|;")) {
                        const Token * tok1 = statementStart->tokAt(2);
                        std::string nameSpace;
                        while (tok1 && tok1->str() != ";") {
                            if (!nameSpace.empty())
                                nameSpace += " ";
                            nameSpace += tok1->str();
                            tok1 = tok1->next();
                        }
                        mImpl->mScopeInfo->usingNamespaces.insert(nameSpace);
                    }
                }
            }
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
    std::cout << stringifyList(stringifyOptions::forPrintOut(), nullptr, nullptr) << std::endl;
}

void Token::printOut(const char *title, const std::vector<std::string> &fileNames) const
{
    if (title && title[0])
        std::cout << "\n### " << title << " ###\n";
    std::cout << stringifyList(stringifyOptions::forPrintOut(), &fileNames, nullptr) << std::endl;
}

void Token::printLines(int lines) const
{
    const Token *end = this;
    while (end && end->linenr() < lines + linenr())
        end = end->next();
    std::cout << stringifyList(stringifyOptions::forDebugExprId(), nullptr, end) << std::endl;
}

void Token::stringify(std::ostream& os, const stringifyOptions& options) const
{
    if (options.attributes) {
        if (isUnsigned())
            os << "unsigned ";
        else if (isSigned())
            os << "signed ";
        if (isComplex())
            os << "_Complex ";
        if (isLong()) {
            if (!(mTokType == eString || mTokType == eChar))
                os << "long ";
        }
    }
    if (options.macro && isExpandedMacro())
        os << "$";
    if (isName() && mStr.find(' ') != std::string::npos) {
        for (char i : mStr) {
            if (i != ' ')
                os << i;
        }
    } else if (mStr[0] != '\"' || mStr.find('\0') == std::string::npos)
        os << mStr;
    else {
        for (char i : mStr) {
            if (i == '\0')
                os << "\\0";
            else
                os << i;
        }
    }
    if (options.varid && mImpl->mVarId != 0)
        os << "@" << (options.idtype ? "var" : "") << mImpl->mVarId;
    else if (options.exprid && mImpl->mExprId != 0)
        os << "@" << (options.idtype ? "expr" : "") << mImpl->mExprId;
}

void Token::stringify(std::ostream& os, bool varid, bool attributes, bool macro) const
{
    stringifyOptions options;
    options.varid = varid;
    options.attributes = attributes;
    options.macro = macro;
    stringify(os, options);
}

std::string Token::stringifyList(const stringifyOptions& options, const std::vector<std::string>* fileNames, const Token* end) const
{
    if (this == end)
        return "";

    std::ostringstream ret;

    unsigned int lineNumber = mImpl->mLineNumber - (options.linenumbers ? 1U : 0U);
    unsigned int fileIndex = options.files ? ~0U : mImpl->mFileIndex;
    std::map<int, unsigned int> lineNumbers;
    for (const Token *tok = this; tok != end; tok = tok->next()) {
        bool fileChange = false;
        if (tok->mImpl->mFileIndex != fileIndex) {
            if (fileIndex != ~0U) {
                lineNumbers[fileIndex] = tok->mImpl->mFileIndex;
            }

            fileIndex = tok->mImpl->mFileIndex;
            if (options.files) {
                ret << "\n\n##file ";
                if (fileNames && fileNames->size() > tok->mImpl->mFileIndex)
                    ret << fileNames->at(tok->mImpl->mFileIndex);
                else
                    ret << fileIndex;
                ret << '\n';
            }

            lineNumber = lineNumbers[fileIndex];
            fileChange = true;
        }

        if (options.linebreaks && (lineNumber != tok->linenr() || fileChange)) {
            if (lineNumber+4 < tok->linenr() && fileIndex == tok->mImpl->mFileIndex) {
                ret << '\n' << lineNumber+1 << ":\n|\n";
                ret << tok->linenr()-1 << ":\n";
                ret << tok->linenr() << ": ";
            } else if (this == tok && options.linenumbers) {
                ret << tok->linenr() << ": ";
            } else if (lineNumber > tok->linenr()) {
                lineNumber = tok->linenr();
                ret << '\n';
                if (options.linenumbers) {
                    ret << lineNumber << ':';
                    if (lineNumber == tok->linenr())
                        ret << ' ';
                }
            } else {
                while (lineNumber < tok->linenr()) {
                    ++lineNumber;
                    ret << '\n';
                    if (options.linenumbers) {
                        ret << lineNumber << ':';
                        if (lineNumber == tok->linenr())
                            ret << ' ';
                    }
                }
            }
            lineNumber = tok->linenr();
        }

        tok->stringify(ret, options); // print token
        if (tok->next() != end && (!options.linebreaks || (tok->next()->linenr() == tok->linenr() && tok->next()->fileIndex() == tok->fileIndex())))
            ret << ' ';
    }
    if (options.linebreaks && (options.files || options.linenumbers))
        ret << '\n';
    return ret.str();
}
std::string Token::stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames, const Token* end) const
{
    stringifyOptions options;
    options.varid = varid;
    options.attributes = attributes;
    options.macro = attributes;
    options.linenumbers = linenumbers;
    options.linebreaks = linebreaks;
    options.files = files;
    return stringifyList(options, fileNames, end);
}

std::string Token::stringifyList(const Token* end, bool attributes) const
{
    return stringifyList(false, attributes, false, false, false, nullptr, end);
}

std::string Token::stringifyList(bool varid) const
{
    return stringifyList(varid, false, true, true, true, nullptr, nullptr);
}

void Token::astOperand1(Token *tok)
{
    if (mImpl->mAstOperand1)
        mImpl->mAstOperand1->mImpl->mAstParent = nullptr;
    // goto parent operator
    if (tok) {
        std::set<Token*> visitedParents;
        while (tok->mImpl->mAstParent) {
            if (!visitedParents.insert(tok->mImpl->mAstParent).second) // #6838/#6726/#8352 avoid hang on garbage code
                throw InternalError(this, "Internal error. Token::astOperand1() cyclic dependency.");
            tok = tok->mImpl->mAstParent;
        }
        tok->mImpl->mAstParent = this;
    }
    mImpl->mAstOperand1 = tok;
}

void Token::astOperand2(Token *tok)
{
    if (mImpl->mAstOperand2)
        mImpl->mAstOperand2->mImpl->mAstParent = nullptr;
    // goto parent operator
    if (tok) {
        std::set<Token*> visitedParents;
        while (tok->mImpl->mAstParent) {
            //std::cout << tok << " -> " << tok->mAstParent ;
            if (!visitedParents.insert(tok->mImpl->mAstParent).second) // #6838/#6726 avoid hang on garbage code
                throw InternalError(this, "Internal error. Token::astOperand2() cyclic dependency.");
            tok = tok->mImpl->mAstParent;
        }
        tok->mImpl->mAstParent = this;
    }
    mImpl->mAstOperand2 = tok;
}

static const Token* goToLeftParenthesis(const Token* start, const Token* end)
{
    // move start to lpar in such expression: '(*it).x'
    int par = 0;
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        if (tok->str() == "(")
            ++par;
        else if (tok->str() == ")") {
            if (par == 0)
                start = tok->link();
            else
                --par;
        }
    }
    return start;
}

static const Token* goToRightParenthesis(const Token* start, const Token* end)
{
    // move end to rpar in such expression: '2>(x+1)'
    int par = 0;
    for (const Token *tok = end; tok && tok != start; tok = tok->previous()) {
        if (tok->str() == ")")
            ++par;
        else if (tok->str() == "(") {
            if (par == 0)
                end = tok->link();
            else
                --par;
        }
    }
    return end;
}

std::pair<const Token *, const Token *> Token::findExpressionStartEndTokens() const
{
    const Token * const top = this;

    // find start node in AST tree
    const Token *start = top;
    while (start->astOperand1() && precedes(start->astOperand1(), start))
        start = start->astOperand1();

    // find end node in AST tree
    const Token *end = top;
    while (end->astOperand1() && (end->astOperand2() || end->isUnaryPreOp())) {
        // lambda..
        if (end->str() == "[") {
            const Token *lambdaEnd = findLambdaEndToken(end);
            if (lambdaEnd) {
                end = lambdaEnd;
                break;
            }
        }
        if (Token::Match(end,"(|[|{") &&
            !(Token::Match(end, "( %type%") && !end->astOperand2())) {
            end = end->link();
            break;
        }
        end = end->astOperand2() ? end->astOperand2() : end->astOperand1();
    }

    // skip parentheses
    start = goToLeftParenthesis(start, end);
    end = goToRightParenthesis(start, end);
    if (Token::simpleMatch(end, "{"))
        end = end->link();
    return std::pair<const Token *, const Token *>(start,end);
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

bool Token::isUnaryPreOp() const
{
    if (!astOperand1() || astOperand2())
        return false;
    if (!Token::Match(this, "++|--"))
        return true;
    const Token *tokbefore = mPrevious;
    const Token *tokafter = mNext;
    for (int distance = 1; distance < 10 && tokbefore; distance++) {
        if (tokbefore == mImpl->mAstOperand1)
            return false;
        if (tokafter == mImpl->mAstOperand1)
            return true;
        tokbefore = tokbefore->mPrevious;
        tokafter  = tokafter->mPrevious;
    }
    return false; // <- guess
}

static std::string stringFromTokenRange(const Token* start, const Token* end)
{
    std::ostringstream ret;
    if (end)
        end = end->next();
    for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
        if (tok->isUnsigned())
            ret << "unsigned ";
        if (tok->isLong() && !tok->isLiteral())
            ret << "long ";
        if (tok->tokType() == Token::eString) {
            for (unsigned char c: tok->str()) {
                if (c == '\n')
                    ret << "\\n";
                else if (c == '\r')
                    ret << "\\r";
                else if (c == '\t')
                    ret << "\\t";
                else if (c >= ' ' && c <= 126)
                    ret << c;
                else {
                    char str[10];
                    sprintf(str, "\\x%02x", c);
                    ret << str;
                }
            }
        } else if (tok->originalName().empty() || tok->isUnsigned() || tok->isLong()) {
            ret << tok->str();
        } else
            ret << tok->originalName();
        if (Token::Match(tok, "%name%|%num% %name%|%num%"))
            ret << ' ';
    }
    return ret.str();
}

std::string Token::expressionString() const
{
    const auto tokens = findExpressionStartEndTokens();
    return stringFromTokenRange(tokens.first, tokens.second);
}

static void astStringXml(const Token *tok, nonneg int indent, std::ostream &out)
{
    const std::string strindent(indent, ' ');

    out << strindent << "<token str=\"" << tok->str() << '\"';
    if (tok->varId() > 0U)
        out << " varId=\"" << MathLib::toString(tok->varId()) << '\"';
    if (tok->variable())
        out << " variable=\"" << tok->variable() << '\"';
    if (tok->function())
        out << " function=\"" << tok->function() << '\"';
    if (!tok->values().empty())
        out << " values=\"" << &tok->values() << '\"';

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

void Token::printAst(bool verbose, bool xml, const std::vector<std::string> &fileNames, std::ostream &out) const
{
    if (!xml)
        out << "\n\n##AST" << std::endl;

    std::set<const Token *> printed;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (!tok->mImpl->mAstParent && tok->mImpl->mAstOperand1) {
            if (printed.find(tok) != printed.end())
                continue;
            printed.insert(tok);

            if (xml) {
                out << "<ast scope=\"" << tok->scope() << "\" fileIndex=\"" << tok->fileIndex() << "\" linenr=\"" << tok->linenr()
                    << "\" column=\"" << tok->column() << "\">" << std::endl;
                astStringXml(tok, 2U, out);
                out << "</ast>" << std::endl;
            } else if (verbose)
                out << "[" << fileNames[tok->fileIndex()] << ":" << tok->linenr() << "]" << std::endl << tok->astStringVerbose() << std::endl;
            else
                out << tok->astString(" ") << std::endl;
            if (tok->str() == "(")
                tok = tok->link();
        }
    }
}

static void indent(std::string &str, const nonneg int indent1, const nonneg int indent2)
{
    for (int i = 0; i < indent1; ++i)
        str += ' ';
    for (int i = indent1; i < indent2; i += 2)
        str += "| ";
}

void Token::astStringVerboseRecursive(std::string& ret, const nonneg int indent1, const nonneg int indent2) const
{
    if (isExpandedMacro())
        ret += '$';
    ret += mStr;
    if (mImpl->mValueType)
        ret += " \'" + mImpl->mValueType->str() + '\'';
    if (function()) {
        std::ostringstream ostr;
        ostr << std::hex << function();
        ret += " f:" + ostr.str();
    }
    ret += '\n';

    if (mImpl->mAstOperand1) {
        int i1 = indent1, i2 = indent2 + 2;
        if (indent1 == indent2 && !mImpl->mAstOperand2)
            i1 += 2;
        indent(ret, indent1, indent2);
        ret += mImpl->mAstOperand2 ? "|-" : "`-";
        mImpl->mAstOperand1->astStringVerboseRecursive(ret, i1, i2);
    }
    if (mImpl->mAstOperand2) {
        int i1 = indent1, i2 = indent2 + 2;
        if (indent1 == indent2)
            i1 += 2;
        indent(ret, indent1, indent2);
        ret += "`-";
        mImpl->mAstOperand2->astStringVerboseRecursive(ret, i1, i2);
    }
}

std::string Token::astStringVerbose() const
{
    std::string ret;
    astStringVerboseRecursive(ret);
    return ret;
}

std::string Token::astStringZ3() const
{
    if (!astOperand1())
        return str();
    if (!astOperand2())
        return "(" + str() + " " + astOperand1()->astStringZ3() + ")";
    return "(" + str() + " " + astOperand1()->astStringZ3() + " " + astOperand2()->astStringZ3() + ")";
}

void Token::printValueFlow(bool xml, std::ostream &out) const
{
    int line = 0;
    if (xml)
        out << "  <valueflow>" << std::endl;
    else
        out << "\n\n##Value flow" << std::endl;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (!tok->mImpl->mValues)
            continue;
        if (tok->mImpl->mValues->empty()) // Values might be removed by removeContradictions
            continue;
        if (xml)
            out << "    <values id=\"" << tok->mImpl->mValues << "\">" << std::endl;
        else if (line != tok->linenr())
            out << "Line " << tok->linenr() << std::endl;
        line = tok->linenr();
        if (!xml) {
            ValueFlow::Value::ValueKind valueKind = tok->mImpl->mValues->front().valueKind;
            bool same = true;
            for (const ValueFlow::Value &value : *tok->mImpl->mValues) {
                if (value.valueKind != valueKind) {
                    same = false;
                    break;
                }
            }
            out << "  " << tok->str() << " ";
            if (same) {
                switch (valueKind) {
                case ValueFlow::Value::ValueKind::Impossible:
                case ValueFlow::Value::ValueKind::Known:
                    out << "always ";
                    break;
                case ValueFlow::Value::ValueKind::Inconclusive:
                case ValueFlow::Value::ValueKind::Possible:
                    out << "possible ";
                    break;
                }
            }
            if (tok->mImpl->mValues->size() > 1U)
                out << '{';
        }
        for (const ValueFlow::Value &value : *tok->mImpl->mValues) {
            if (xml) {
                out << "      <value ";
                switch (value.valueType) {
                case ValueFlow::Value::INT:
                    if (tok->valueType() && tok->valueType()->sign == ValueType::UNSIGNED)
                        out << "intvalue=\"" << (MathLib::biguint)value.intvalue << '\"';
                    else
                        out << "intvalue=\"" << value.intvalue << '\"';
                    break;
                case ValueFlow::Value::TOK:
                    out << "tokvalue=\"" << value.tokvalue << '\"';
                    break;
                case ValueFlow::Value::FLOAT:
                    out << "floatvalue=\"" << value.floatValue << '\"';
                    break;
                case ValueFlow::Value::MOVED:
                    out << "movedvalue=\"" << ValueFlow::Value::toString(value.moveKind) << '\"';
                    break;
                case ValueFlow::Value::UNINIT:
                    out << "uninit=\"1\"";
                    break;
                case ValueFlow::Value::BUFFER_SIZE:
                    out << "buffer-size=\"" << value.intvalue << "\"";
                    break;
                case ValueFlow::Value::CONTAINER_SIZE:
                    out << "container-size=\"" << value.intvalue << '\"';
                    break;
                case ValueFlow::Value::ITERATOR_START:
                    out << "iterator-start=\"" << value.intvalue << '\"';
                    break;
                case ValueFlow::Value::ITERATOR_END:
                    out << "iterator-end=\"" << value.intvalue << '\"';
                    break;
                case ValueFlow::Value::LIFETIME:
                    out << "lifetime=\"" << value.tokvalue << '\"';
                    break;
                }
                if (value.condition)
                    out << " condition-line=\"" << value.condition->linenr() << '\"';
                if (value.isKnown())
                    out << " known=\"true\"";
                else if (value.isPossible())
                    out << " possible=\"true\"";
                else if (value.isImpossible())
                    out << " impossible=\"true\"";
                else if (value.isInconclusive())
                    out << " inconclusive=\"true\"";
                out << "/>" << std::endl;
            }

            else {
                if (&value != &tok->mImpl->mValues->front())
                    out << ",";
                if (value.isImpossible())
                    out << "!";
                if (value.bound == ValueFlow::Value::Bound::Lower)
                    out << ">";
                if (value.bound == ValueFlow::Value::Bound::Upper)
                    out << "<";
                switch (value.valueType) {
                case ValueFlow::Value::INT:
                    if (tok->valueType() && tok->valueType()->sign == ValueType::UNSIGNED)
                        out << (MathLib::biguint)value.intvalue;
                    else
                        out << value.intvalue;
                    break;
                case ValueFlow::Value::TOK:
                    out << value.tokvalue->str();
                    break;
                case ValueFlow::Value::FLOAT:
                    out << value.floatValue;
                    break;
                case ValueFlow::Value::MOVED:
                    out << ValueFlow::Value::toString(value.moveKind);
                    break;
                case ValueFlow::Value::UNINIT:
                    out << "Uninit";
                    break;
                case ValueFlow::Value::BUFFER_SIZE:
                case ValueFlow::Value::CONTAINER_SIZE:
                    out << "size=" << value.intvalue;
                    break;
                case ValueFlow::Value::ITERATOR_START:
                    out << "start=" << value.intvalue;
                    break;
                case ValueFlow::Value::ITERATOR_END:
                    out << "end=" << value.intvalue;
                    break;
                case ValueFlow::Value::LIFETIME:
                    out << "lifetime=" << value.tokvalue->str();
                    break;
                }
                if (value.indirect > 0)
                    for (int i=0; i<value.indirect; i++)
                        out << "*";
                if (value.path > 0)
                    out << "@" << value.path;
            }
        }
        if (xml)
            out << "    </values>" << std::endl;
        else if (tok->mImpl->mValues->size() > 1U)
            out << '}' << std::endl;
        else
            out << std::endl;
    }
    if (xml)
        out << "  </valueflow>" << std::endl;
}

const ValueFlow::Value * Token::getValueLE(const MathLib::bigint val, const Settings *settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isImpossible())
            continue;
        if (it->isIntValue() && it->intvalue <= val) {
            if (!ret || ret->isInconclusive() || (ret->condition && !it->isInconclusive()))
                ret = &(*it);
            if (!ret->isInconclusive() && !ret->condition)
                break;
        }
    }
    if (settings && ret) {
        if (ret->isInconclusive() && !settings->inconclusive)
            return nullptr;
        if (ret->condition && !settings->isEnabled(Settings::WARNING))
            return nullptr;
    }
    return ret;
}

const ValueFlow::Value * Token::getValueGE(const MathLib::bigint val, const Settings *settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isImpossible())
            continue;
        if (it->isIntValue() && it->intvalue >= val) {
            if (!ret || ret->isInconclusive() || (ret->condition && !it->isInconclusive()))
                ret = &(*it);
            if (!ret->isInconclusive() && !ret->condition)
                break;
        }
    }
    if (settings && ret) {
        if (ret->isInconclusive() && !settings->inconclusive)
            return nullptr;
        if (ret->condition && !settings->isEnabled(Settings::WARNING))
            return nullptr;
    }
    return ret;
}

const ValueFlow::Value * Token::getInvalidValue(const Token *ftok, nonneg int argnr, const Settings *settings) const
{
    if (!mImpl->mValues || !settings)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isImpossible())
            continue;
        if ((it->isIntValue() && !settings->library.isIntArgValid(ftok, argnr, it->intvalue)) ||
            (it->isFloatValue() && !settings->library.isFloatArgValid(ftok, argnr, it->floatValue))) {
            if (!ret || ret->isInconclusive() || (ret->condition && !it->isInconclusive()))
                ret = &(*it);
            if (!ret->isInconclusive() && !ret->condition)
                break;
        }
    }
    if (ret) {
        if (ret->isInconclusive() && !settings->inconclusive)
            return nullptr;
        if (ret->condition && !settings->isEnabled(Settings::WARNING))
            return nullptr;
    }
    return ret;
}

const Token *Token::getValueTokenMinStrSize(const Settings *settings) const
{
    if (!mImpl->mValues)
        return nullptr;
    const Token *ret = nullptr;
    int minsize = INT_MAX;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const int size = getStrSize(it->tokvalue, settings);
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
    if (!mImpl->mValues)
        return nullptr;
    const Token *ret = nullptr;
    int maxlength = 0;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const int length = getStrLength(it->tokvalue);
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
    for (it = values().begin(); it != values().end(); ++it) {
        // Is this a pointer alias?
        if (!it->isTokValue() || (it->tokvalue && it->tokvalue->str() != "&"))
            continue;
        // Get variable
        const Token *vartok = it->tokvalue->astOperand1();
        if (!vartok || !vartok->isName() || !vartok->variable())
            continue;
        const Variable * const var = vartok->variable();
        if (var->isStatic() || var->isReference())
            continue;
        if (!var->scope())
            return nullptr; // #6804
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

static bool removeContradiction(std::list<ValueFlow::Value>& values)
{
    bool result = false;
    for (ValueFlow::Value& x : values) {
        if (x.isNonValue())
            continue;
        for (ValueFlow::Value& y : values) {
            if (y.isNonValue())
                continue;
            if (x == y)
                continue;
            if (x.valueType != y.valueType)
                continue;
            if (x.isImpossible() == y.isImpossible())
                continue;
            if (!x.equalValue(y))
                continue;
            if (x.bound == y.bound ||
                (x.bound != ValueFlow::Value::Bound::Point && y.bound != ValueFlow::Value::Bound::Point)) {
                const bool removex = !x.isImpossible() || y.isKnown();
                const bool removey = !y.isImpossible() || x.isKnown();
                if (removex)
                    values.remove(x);
                if (removey)
                    values.remove(y);
                return true;
            } else if (x.bound == ValueFlow::Value::Bound::Point) {
                y.decreaseRange();
                result = true;
            }
        }
    }
    return result;
}

static void removeOverlaps(std::list<ValueFlow::Value>& values)
{
    for (ValueFlow::Value& x : values) {
        if (x.isNonValue())
            continue;
        values.remove_if([&](ValueFlow::Value& y) {
            if (y.isNonValue())
                return false;
            if (&x == &y)
                return false;
            if (x.valueType != y.valueType)
                return false;
            if (x.valueKind != y.valueKind)
                return false;
            // TODO: Remove points coverd in a lower or upper bound
            // TODO: Remove lower or upper bound already covered by a lower and upper bound
            if (!x.equalValue(y))
                return false;
            if (x.bound != y.bound)
                return false;
            return true;
        });
    }
}

// Removing contradictions is an NP-hard problem. Instead we run multiple
// passes to try to catch most contradictions
static void removeContradictions(std::list<ValueFlow::Value>& values)
{
    for (int i = 0; i < 4; i++) {
        if (!removeContradiction(values))
            return;
        removeOverlaps(values);
    }
}

bool Token::addValue(const ValueFlow::Value &value)
{
    if (value.isKnown() && mImpl->mValues) {
        // Clear all other values of the same type since value is known
        mImpl->mValues->remove_if([&](const ValueFlow::Value & x) {
            return x.valueType == value.valueType;
        });
    }

    if (mImpl->mValues) {
        // Don't handle more than 10 values for performance reasons
        // TODO: add setting?
        if (mImpl->mValues->size() >= 10U)
            return false;

        // if value already exists, don't add it again
        std::list<ValueFlow::Value>::iterator it;
        for (it = mImpl->mValues->begin(); it != mImpl->mValues->end(); ++it) {
            // different types => continue
            if (it->valueType != value.valueType)
                continue;

            if (it->isImpossible() != value.isImpossible())
                continue;

            // different value => continue
            bool differentValue = true;
            switch (it->valueType) {
            case ValueFlow::Value::ValueType::INT:
            case ValueFlow::Value::ValueType::CONTAINER_SIZE:
            case ValueFlow::Value::ValueType::BUFFER_SIZE:
            case ValueFlow::Value::ValueType::ITERATOR_START:
            case ValueFlow::Value::ValueType::ITERATOR_END:
                differentValue = (it->intvalue != value.intvalue);
                break;
            case ValueFlow::Value::ValueType::TOK:
            case ValueFlow::Value::ValueType::LIFETIME:
                differentValue = (it->tokvalue != value.tokvalue);
                break;
            case ValueFlow::Value::ValueType::FLOAT:
                // TODO: Write some better comparison
                differentValue = (it->floatValue > value.floatValue || it->floatValue < value.floatValue);
                break;
            case ValueFlow::Value::ValueType::MOVED:
                differentValue = (it->moveKind != value.moveKind);
                break;
            case ValueFlow::Value::ValueType::UNINIT:
                differentValue = false;
                break;
            }
            if (differentValue)
                continue;

            if ((value.isTokValue() || value.isLifetimeValue()) && (it->tokvalue != value.tokvalue) && (it->tokvalue->str() != value.tokvalue->str()))
                continue;

            // same value, but old value is inconclusive so replace it
            if (it->isInconclusive() && !value.isInconclusive() && !value.isImpossible()) {
                *it = value;
                if (it->varId == 0)
                    it->varId = mImpl->mVarId;
                break;
            }

            // Same value already exists, don't  add new value
            return false;
        }

        // Add value
        if (it == mImpl->mValues->end()) {
            ValueFlow::Value v(value);
            if (v.varId == 0)
                v.varId = mImpl->mVarId;
            if (v.isKnown() && v.isIntValue())
                mImpl->mValues->push_front(v);
            else
                mImpl->mValues->push_back(v);
        }
    } else {
        ValueFlow::Value v(value);
        if (v.varId == 0)
            v.varId = mImpl->mVarId;
        mImpl->mValues = new std::list<ValueFlow::Value>(1, v);
    }

    removeContradictions(*mImpl->mValues);

    return true;
}

void Token::assignProgressValues(Token *tok)
{
    int total_count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        ++total_count;
    int count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        tok2->mImpl->mProgressValue = count++ * 100 / total_count;
}

void Token::assignIndexes()
{
    int index = (mPrevious ? mPrevious->mImpl->mIndex : 0) + 1;
    for (Token *tok = this; tok; tok = tok->next())
        tok->mImpl->mIndex = index++;
}

void Token::setValueType(ValueType *vt)
{
    if (vt != mImpl->mValueType) {
        delete mImpl->mValueType;
        mImpl->mValueType = vt;
    }
}

void Token::type(const ::Type *t)
{
    mImpl->mType = t;
    if (t) {
        tokType(eType);
        isEnumType(mImpl->mType->isEnumType());
    } else if (mTokType == eType)
        tokType(eName);
}

const ::Type *Token::typeOf(const Token *tok)
{
    if (!tok)
        return nullptr;
    if (Token::simpleMatch(tok, "return")) {
        const Scope *scope = tok->scope();
        if (!scope)
            return nullptr;
        const Function *function = scope->function;
        if (!function)
            return nullptr;
        return function->retType;
    } else if (Token::Match(tok, "%type%")) {
        return tok->type();
    } else if (Token::Match(tok, "%var%")) {
        const Variable *var = tok->variable();
        if (!var)
            return nullptr;
        return var->type();
    } else if (Token::Match(tok, "%name%")) {
        const Function *function = tok->function();
        if (!function)
            return nullptr;
        return function->retType;
    } else if (Token::Match(tok->previous(), "%type% (|{")) {
        return typeOf(tok->previous());
    } else if (Token::simpleMatch(tok, "=")) {
        return Token::typeOf(tok->astOperand1());
    } else if (Token::simpleMatch(tok, ".")) {
        return Token::typeOf(tok->astOperand2());
    } else if (Token::simpleMatch(tok, "[")) {
        return Token::typeOf(tok->astOperand1());
    }
    return nullptr;
}

std::pair<const Token*, const Token*> Token::typeDecl(const Token * tok)
{
    if (!tok)
        return {};
    if (Token::simpleMatch(tok, "return")) {
        const Scope *scope = tok->scope();
        if (!scope)
            return {};
        const Function *function = scope->function;
        if (!function)
            return {};
        return {function->retDef, function->returnDefEnd()};
    } else if (Token::Match(tok, "%type%")) {
        return {tok, tok->next()};
    } else if (Token::Match(tok, "%var%")) {
        const Variable *var = tok->variable();
        if (!var)
            return {};
        if (!var->typeStartToken() || !var->typeEndToken())
            return {};
        if (Token::simpleMatch(var->typeStartToken(), "auto")) {
            const Token * tok2 = var->declEndToken();
            if (Token::Match(tok2, "; %varid% =", var->declarationId()))
                tok2 = tok2->tokAt(2);
            if (Token::simpleMatch(tok2, "=") && Token::Match(tok2->astOperand2(), "!!=")) {
                std::pair<const Token*, const Token*> r = typeDecl(tok2->astOperand2());
                if (r.first)
                    return r;
            }
        }
        return {var->typeStartToken(), var->typeEndToken()->next()};
    } else if (Token::Match(tok->previous(), "%name% (")) {
        const Function *function = tok->previous()->function();
        if (!function)
            return {};
        return {function->retDef, function->returnDefEnd()};
    } else if (Token::simpleMatch(tok, "=")) {
        return Token::typeDecl(tok->astOperand1());
    } else if (Token::simpleMatch(tok, ".")) {
        return Token::typeDecl(tok->astOperand2());
    } else {
        const ::Type * t = typeOf(tok);
        if (!t || !t->classDef)
            return {};
        return {t->classDef->next(), t->classDef->tokAt(2)};
    }
}
std::string Token::typeStr(const Token* tok)
{
    if (tok->valueType()) {
        const ValueType * vt = tok->valueType();
        std::string ret = vt->str();
        if (!ret.empty())
            return ret;
    }
    std::pair<const Token*, const Token*> r = Token::typeDecl(tok);
    if (!r.first || !r.second)
        return "";
    return r.first->stringifyList(r.second, false);
}

void Token::scopeInfo(std::shared_ptr<ScopeInfo2> newScopeInfo)
{
    mImpl->mScopeInfo = newScopeInfo;
}
std::shared_ptr<ScopeInfo2> Token::scopeInfo() const
{
    return mImpl->mScopeInfo;
}

bool Token::hasKnownIntValue() const
{
    if (!mImpl->mValues)
        return false;
    return std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), [](const ValueFlow::Value& value) {
        return value.isKnown() && value.isIntValue();
    });
}

bool Token::hasKnownValue() const
{
    return mImpl->mValues && std::any_of(mImpl->mValues->begin(), mImpl->mValues->end(), std::mem_fn(&ValueFlow::Value::isKnown));
}

bool Token::isImpossibleIntValue(const MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return false;
    for (const auto& v : *mImpl->mValues) {
        if (v.isIntValue() && v.isImpossible() && v.intvalue == val)
            return true;
        if (v.isIntValue() && v.bound == ValueFlow::Value::Bound::Lower && val > v.intvalue)
            return true;
        if (v.isIntValue() && v.bound == ValueFlow::Value::Bound::Upper && val < v.intvalue)
            return true;
    }
    return false;
}

const ValueFlow::Value* Token::getValue(const MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [=](const ValueFlow::Value& value) {
        return value.isIntValue() && !value.isImpossible() && value.intvalue == val;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

const ValueFlow::Value* Token::getMaxValue(bool condition) const
{
    if (!mImpl->mValues)
        return nullptr;
    const ValueFlow::Value* ret = nullptr;
    for (const ValueFlow::Value& value : *mImpl->mValues) {
        if (!value.isIntValue())
            continue;
        if (value.isImpossible())
            continue;
        if ((!ret || value.intvalue > ret->intvalue) &&
            ((value.condition != nullptr) == condition))
            ret = &value;
    }
    return ret;
}

const ValueFlow::Value* Token::getMovedValue() const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [](const ValueFlow::Value& value) {
        return value.isMovedValue() && !value.isImpossible() &&
               value.moveKind != ValueFlow::Value::MoveKind::NonMovedVariable;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

const ValueFlow::Value* Token::getContainerSizeValue(const MathLib::bigint val) const
{
    if (!mImpl->mValues)
        return nullptr;
    const auto it = std::find_if(mImpl->mValues->begin(), mImpl->mValues->end(), [=](const ValueFlow::Value& value) {
        return value.isContainerSizeValue() && !value.isImpossible() && value.intvalue == val;
    });
    return it == mImpl->mValues->end() ? nullptr : &*it;
}

TokenImpl::~TokenImpl()
{
    delete mOriginalName;
    delete mValueType;
    delete mValues;

    if (mTemplateSimplifierPointers)
        for (auto *templateSimplifierPointer : *mTemplateSimplifierPointers) {
            templateSimplifierPointer->token(nullptr);
        }
    delete mTemplateSimplifierPointers;

    while (mCppcheckAttributes) {
        struct CppcheckAttributes *c = mCppcheckAttributes;
        mCppcheckAttributes = mCppcheckAttributes->next;
        delete c;
    }
}

void TokenImpl::setCppcheckAttribute(TokenImpl::CppcheckAttributes::Type type, MathLib::bigint value)
{
    struct CppcheckAttributes *attr = mCppcheckAttributes;
    while (attr && attr->type != type)
        attr = attr->next;
    if (attr)
        attr->value = value;
    else {
        attr = new CppcheckAttributes;
        attr->type = type;
        attr->value = value;
        attr->next = mCppcheckAttributes;
        mCppcheckAttributes = attr;
    }
}

bool TokenImpl::getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type type, MathLib::bigint *value) const
{
    struct CppcheckAttributes *attr = mCppcheckAttributes;
    while (attr && attr->type != type)
        attr = attr->next;
    if (attr)
        *value = attr->value;
    return attr != nullptr;
}
