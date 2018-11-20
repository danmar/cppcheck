/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
#include "library.h"
#include "settings.h"
#include "symboldatabase.h"
#include "utils.h"

#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <utility>

const std::list<ValueFlow::Value> Token::mEmptyValueList;

Token::Token(TokensFrontBack *tokensFrontBack) :
    mTokensFrontBack(tokensFrontBack),
    mNext(nullptr),
    mPrevious(nullptr),
    mLink(nullptr),
    mScope(nullptr),
    mFunction(nullptr), // Initialize whole union
    mVarId(0),
    mFileIndex(0),
    mLineNumber(0),
    mColumn(0),
    mProgressValue(0),
    mTokType(eNone),
    mFlags(0),
    mBits(0),
    mAstOperand1(nullptr),
    mAstOperand2(nullptr),
    mAstParent(nullptr),
    mOriginalName(nullptr),
    mValueType(nullptr),
    mValues(nullptr)
{
}

Token::~Token()
{
    delete mOriginalName;
    delete mValueType;
    delete mValues;
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
        else if (std::isalpha((unsigned char)mStr[0]) || mStr[0] == '_' || mStr[0] == '$') { // Name
            if (mVarId)
                tokType(eVariable);
            else if (mTokType != eVariable && mTokType != eFunction && mTokType != eType && mTokType != eKeyword)
                tokType(eName);
        } else if (std::isdigit((unsigned char)mStr[0]) || (mStr.length() > 1 && mStr[0] == '-' && std::isdigit((unsigned char)mStr[1])))
            tokType(eNumber);
        else if (mStr.length() > 1 && mStr[0] == '"' && endsWith(mStr,'"'))
            tokType(eString);
        else if (mStr.length() > 1 && mStr[0] == '\'' && endsWith(mStr,'\''))
            tokType(eChar);
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
        else
            tokType(eOther);
    } else {
        tokType(eNone);
    }

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


bool Token::isUpperCaseName() const
{
    if (!isName())
        return false;
    for (size_t i = 0; i < mStr.length(); ++i) {
        if (std::islower(mStr[i]))
            return false;
    }
    return true;
}

void Token::concatStr(std::string const& b)
{
    mStr.erase(mStr.length() - 1);
    mStr.append(b.begin() + 1, b.end());

    update_property_info();
}

std::string Token::strValue() const
{
    assert(mTokType == eString);
    std::string ret(mStr.substr(1, mStr.length() - 2));
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

void Token::deleteNext(unsigned long index)
{
    while (mNext && index) {
        Token *n = mNext;

        // #8154 we are about to be unknown -> destroy the link to us
        if (n->mLink && n->mLink->mLink == n)
            n->mLink->link(nullptr);

        mNext = n->next();
        delete n;
        --index;
    }

    if (mNext)
        mNext->previous(this);
    else if (mTokensFrontBack)
        mTokensFrontBack->back = this;
}

void Token::deletePrevious(unsigned long index)
{
    while (mPrevious && index) {
        Token *p = mPrevious;

        // #8154 we are about to be unknown -> destroy the link to us
        if (p->mLink && p->mLink->mLink == p)
            p->mLink->link(nullptr);

        mPrevious = p->previous();
        delete p;
        --index;
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
        std::swap(mVarId, mNext->mVarId);
        std::swap(mFileIndex, mNext->mFileIndex);
        std::swap(mLineNumber, mNext->mLineNumber);
        if (mNext->mLink)
            mNext->mLink->mLink = this;
        if (this->mLink)
            this->mLink->mLink = mNext;
        std::swap(mLink, mNext->mLink);
        std::swap(mScope, mNext->mScope);
        std::swap(mFunction, mNext->mFunction);
        std::swap(mOriginalName, mNext->mOriginalName);
        std::swap(mValues, mNext->mValues);
        std::swap(mValueType, mNext->mValueType);
        std::swap(mProgressValue, mNext->mProgressValue);
    }
}

void Token::takeData(Token *fromToken)
{
    mStr = fromToken->mStr;
    tokType(fromToken->mTokType);
    mFlags = fromToken->mFlags;
    mVarId = fromToken->mVarId;
    mFileIndex = fromToken->mFileIndex;
    mLineNumber = fromToken->mLineNumber;
    mLink = fromToken->mLink;
    mScope = fromToken->mScope;
    mFunction = fromToken->mFunction;
    if (fromToken->mOriginalName) {
        delete mOriginalName;
        mOriginalName = fromToken->mOriginalName;
        fromToken->mOriginalName = nullptr;
    }
    delete mValues;
    mValues = fromToken->mValues;
    fromToken->mValues = nullptr;
    delete mValueType;
    mValueType = fromToken->mValueType;
    fromToken->mValueType = nullptr;
    if (mLink)
        mLink->link(this);
}

void Token::deleteThis()
{
    if (mNext) { // Copy next to this and delete next
        takeData(mNext);
        mNext->link(nullptr); // mark as unlinked
        deleteNext();
    } else if (mPrevious && mPrevious->mPrevious) { // Copy previous to this and delete previous
        takeData(mPrevious);

        Token* toDelete = mPrevious;
        mPrevious = mPrevious->mPrevious;
        mPrevious->mNext = this;

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

    if (end->mTokensFrontBack && end->mTokensFrontBack->back == end) {
        while (end->next())
            end = end->next();
        end->mTokensFrontBack->back = end;
    }

    // Update mProgressValue, fileIndex and linenr
    for (Token *tok = start; tok != end->next(); tok = tok->next())
        tok->mProgressValue = replaceThis->mProgressValue;

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

static int multiComparePercent(const Token *tok, const char*& haystack, unsigned int varid)
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

int Token::multiCompare(const Token *tok, const char *haystack, unsigned int varid)
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

bool Token::simpleMatch(const Token *tok, const char pattern[])
{
    if (!tok)
        return false; // shortcut
    const char *current  = pattern;
    const char *next = std::strchr(pattern, ' ');
    if (!next)
        next = pattern + std::strlen(pattern);

    while (*current) {
        const std::size_t length = next - current;

        if (!tok || length != tok->mStr.length() || std::strncmp(current, tok->mStr.c_str(), length))
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
            return nullptr;

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

std::size_t Token::getStrLength(const Token *tok)
{
    assert(tok != nullptr);
    assert(tok->mTokType == eString);

    std::size_t len = 0;
    std::string::const_iterator it = tok->str().begin() + 1U;
    const std::string::const_iterator end = tok->str().end() - 1U;

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

std::size_t Token::getStrSize(const Token *tok)
{
    assert(tok != nullptr && tok->tokType() == eString);
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

    std::string::const_iterator it = tok->str().begin() + 1U;
    const std::string::const_iterator end = tok->str().end() - 1U;

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
        tok->mProgressValue = newLocation->mProgressValue;
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

const Token * Token::findClosingBracket() const
{
    if (mStr != "<")
        return nullptr;

    const Token *closing = nullptr;

    unsigned int depth = 0;
    for (closing = this; closing != nullptr; closing = closing->next()) {
        if (Token::Match(closing, "{|[|(")) {
            closing = closing->link();
            if (!closing)
                return nullptr; // #6803
        } else if (Token::Match(closing, "}|]|)|;"))
            return nullptr;
        else if (closing->str() == "<")
            ++depth;
        else if (closing->str() == ">") {
            if (--depth == 0)
                return closing;
        } else if (closing->str() == ">>") {
            if (depth <= 2)
                return closing;
            depth -= 2;
        }
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

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[])
{
    for (const Token* tok = startTok; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern))
            return tok;
    }
    return nullptr;
}

const Token *Token::findsimplematch(const Token * const startTok, const char pattern[], const Token * const end)
{
    for (const Token* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::simpleMatch(tok, pattern))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const unsigned int varId)
{
    for (const Token* tok = startTok; tok; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
}

const Token *Token::findmatch(const Token * const startTok, const char pattern[], const Token * const end, const unsigned int varId)
{
    for (const Token* tok = startTok; tok && tok != end; tok = tok->next()) {
        if (Token::Match(tok, pattern, varId))
            return tok;
    }
    return nullptr;
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
        newToken->mLineNumber = mLineNumber;
        newToken->mFileIndex = mFileIndex;
        newToken->mProgressValue = mProgressValue;

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
    std::cout << stringifyList(true, true, true, true, true, nullptr, nullptr) << std::endl;
}

void Token::printOut(const char *title, const std::vector<std::string> &fileNames) const
{
    if (title && title[0])
        std::cout << "\n### " << title << " ###\n";
    std::cout << stringifyList(true, true, true, true, true, &fileNames, nullptr) << std::endl;
}

void Token::stringify(std::ostream& os, bool varid, bool attributes, bool macro) const
{
    if (attributes) {
        if (isUnsigned())
            os << "unsigned ";
        else if (isSigned())
            os << "signed ";
        if (isComplex())
            os << "_Complex ";
        if (isLong()) {
            if (mTokType == eString || mTokType == eChar)
                os << "L";
            else
                os << "long ";
        }
    }
    if (macro && isExpandedMacro())
        os << "$";
    if (isName() && mStr.find(' ') != std::string::npos) {
        for (std::size_t i = 0U; i < mStr.size(); ++i) {
            if (mStr[i] != ' ')
                os << mStr[i];
        }
    } else if (mStr[0] != '\"' || mStr.find('\0') == std::string::npos)
        os << mStr;
    else {
        for (std::size_t i = 0U; i < mStr.size(); ++i) {
            if (mStr[i] == '\0')
                os << "\\0";
            else
                os << mStr[i];
        }
    }
    if (varid && mVarId != 0)
        os << '@' << mVarId;
}

std::string Token::stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames, const Token* end) const
{
    if (this == end)
        return "";

    std::ostringstream ret;

    unsigned int lineNumber = mLineNumber - (linenumbers ? 1U : 0U);
    unsigned int fileInd = files ? ~0U : mFileIndex;
    std::map<int, unsigned int> lineNumbers;
    for (const Token *tok = this; tok != end; tok = tok->next()) {
        bool fileChange = false;
        if (tok->mFileIndex != fileInd) {
            if (fileInd != ~0U) {
                lineNumbers[fileInd] = tok->mFileIndex;
            }

            fileInd = tok->mFileIndex;
            if (files) {
                ret << "\n\n##file ";
                if (fileNames && fileNames->size() > tok->mFileIndex)
                    ret << fileNames->at(tok->mFileIndex);
                else
                    ret << fileInd;
                ret << '\n';
            }

            lineNumber = lineNumbers[fileInd];
            fileChange = true;
        }

        if (linebreaks && (lineNumber != tok->linenr() || fileChange)) {
            if (lineNumber+4 < tok->linenr() && fileInd == tok->mFileIndex) {
                ret << '\n' << lineNumber+1 << ":\n|\n";
                ret << tok->linenr()-1 << ":\n";
                ret << tok->linenr() << ": ";
            } else if (this == tok && linenumbers) {
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
    if (linebreaks && (files || linenumbers))
        ret << '\n';
    return ret.str();
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
    if (mAstOperand1)
        mAstOperand1->mAstParent = nullptr;
    // goto parent operator
    if (tok) {
        std::set<Token*> visitedParents;
        while (tok->mAstParent) {
            if (!visitedParents.insert(tok->mAstParent).second) // #6838/#6726/#8352 avoid hang on garbage code
                throw InternalError(this, "Internal error. Token::astOperand1() cyclic dependency.");
            tok = tok->mAstParent;
        }
        tok->mAstParent = this;
    }
    mAstOperand1 = tok;
}

void Token::astOperand2(Token *tok)
{
    if (mAstOperand2)
        mAstOperand2->mAstParent = nullptr;
    // goto parent operator
    if (tok) {
        std::set<Token*> visitedParents;
        while (tok->mAstParent) {
            //std::cout << tok << " -> " << tok->mAstParent ;
            if (!visitedParents.insert(tok->mAstParent).second) // #6838/#6726 avoid hang on garbage code
                throw InternalError(this, "Internal error. Token::astOperand2() cyclic dependency.");
            tok = tok->mAstParent;
        }
        tok->mAstParent = this;
    }
    mAstOperand2 = tok;
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
    const Token *start = top;
    while (start->astOperand1() &&
           (start->astOperand2() || !start->isUnaryPreOp() || Token::simpleMatch(start, "( )") || start->str() == "{"))
        start = start->astOperand1();
    const Token *end = top;
    while (end->astOperand1() && (end->astOperand2() || end->isUnaryPreOp())) {
        if (Token::Match(end,"(|[") &&
            !(Token::Match(end, "( %type%") && !end->astOperand2())) {
            end = end->link();
            break;
        }
        end = end->astOperand2() ? end->astOperand2() : end->astOperand1();
    }

    start = goToLeftParenthesis(start, end);
    end = goToRightParenthesis(start, end);
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
        if (tokbefore == mAstOperand1)
            return false;
        if (tokafter == mAstOperand1)
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
        if (tok->isLong())
            ret << (tok->isLiteral() ? "L" : "long ");
        if (tok->originalName().empty() || tok->isUnsigned() || tok->isLong()) {
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

void Token::printAst(bool verbose, bool xml, std::ostream &out) const
{
    std::set<const Token *> printed;
    for (const Token *tok = this; tok; tok = tok->next()) {
        if (!tok->mAstParent && tok->mAstOperand1) {
            if (printed.empty() && !xml)
                out << "\n\n##AST" << std::endl;
            else if (printed.find(tok) != printed.end())
                continue;
            printed.insert(tok);

            if (xml) {
                out << "<ast scope=\"" << tok->scope() << "\" fileIndex=\"" << tok->fileIndex() << "\" linenr=\"" << tok->linenr() << "\">" << std::endl;
                astStringXml(tok, 2U, out);
                out << "</ast>" << std::endl;
            } else if (verbose)
                out << tok->astStringVerbose(0,0) << std::endl;
            else
                out << tok->astString(" ") << std::endl;
            if (tok->str() == "(")
                tok = tok->link();
        }
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
        ret += '$';
    ret += mStr;
    if (mValueType)
        ret += " \'" + mValueType->str() + '\'';
    ret += '\n';

    if (mAstOperand1) {
        unsigned int i1 = indent1, i2 = indent2 + 2;
        if (indent1==indent2 && !mAstOperand2)
            i1 += 2;
        ret += indent(indent1,indent2) + (mAstOperand2 ? "|-" : "`-") + mAstOperand1->astStringVerbose(i1,i2);
    }
    if (mAstOperand2) {
        unsigned int i1 = indent1, i2 = indent2 + 2;
        if (indent1==indent2)
            i1 += 2;
        ret += indent(indent1,indent2) + "`-" + mAstOperand2->astStringVerbose(i1,i2);
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
        if (!tok->mValues)
            continue;
        if (xml)
            out << "    <values id=\"" << tok->mValues << "\">" << std::endl;
        else if (line != tok->linenr())
            out << "Line " << tok->linenr() << std::endl;
        line = tok->linenr();
        if (!xml) {
            out << "  " << tok->str() << (tok->mValues->front().isKnown() ? " always " : " possible ");
            if (tok->mValues->size() > 1U)
                out << '{';
        }
        for (const ValueFlow::Value &value : *tok->mValues) {
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
                case ValueFlow::Value::CONTAINER_SIZE:
                    out << "container-size=\"" << value.intvalue << '\"';
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
                else if (value.isInconclusive())
                    out << " inconclusive=\"true\"";
                out << "/>" << std::endl;
            }

            else {
                if (&value != &tok->mValues->front())
                    out << ",";
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
                case ValueFlow::Value::CONTAINER_SIZE:
                    out << "size=" << value.intvalue;
                    break;
                case ValueFlow::Value::LIFETIME:
                    out << "lifetime=" << value.tokvalue->str();
                    break;
                }
            }
        }
        if (xml)
            out << "    </values>" << std::endl;
        else if (tok->mValues->size() > 1U)
            out << '}' << std::endl;
        else
            out << std::endl;
    }
    if (xml)
        out << "  </valueflow>" << std::endl;
}

const ValueFlow::Value * Token::getValueLE(const MathLib::bigint val, const Settings *settings) const
{
    if (!mValues)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mValues->begin(); it != mValues->end(); ++it) {
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
    if (!mValues)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mValues->begin(); it != mValues->end(); ++it) {
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

const ValueFlow::Value * Token::getInvalidValue(const Token *ftok, unsigned int argnr, const Settings *settings) const
{
    if (!mValues || !settings)
        return nullptr;
    const ValueFlow::Value *ret = nullptr;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mValues->begin(); it != mValues->end(); ++it) {
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

const Token *Token::getValueTokenMinStrSize() const
{
    if (!mValues)
        return nullptr;
    const Token *ret = nullptr;
    std::size_t minsize = ~0U;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mValues->begin(); it != mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const std::size_t size = getStrSize(it->tokvalue);
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
    if (!mValues)
        return nullptr;
    const Token *ret = nullptr;
    std::size_t maxlength = 0U;
    std::list<ValueFlow::Value>::const_iterator it;
    for (it = mValues->begin(); it != mValues->end(); ++it) {
        if (it->isTokValue() && it->tokvalue && it->tokvalue->tokType() == Token::eString) {
            const std::size_t length = getStrLength(it->tokvalue);
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

bool Token::addValue(const ValueFlow::Value &value)
{
    if (value.isKnown() && mValues) {
        // Clear all other values of the same type since value is known
        mValues->remove_if([&](const ValueFlow::Value & x) {
            return x.valueType == value.valueType;
        });
    }

    if (mValues) {
        // Don't handle more than 10 values for performance reasons
        // TODO: add setting?
        if (mValues->size() >= 10U)
            return false;

        // if value already exists, don't add it again
        std::list<ValueFlow::Value>::iterator it;
        for (it = mValues->begin(); it != mValues->end(); ++it) {
            // different intvalue => continue
            if (it->intvalue != value.intvalue)
                continue;

            // different types => continue
            if (it->valueType != value.valueType)
                continue;
            if ((value.isTokValue() || value.isLifetimeValue()) && (it->tokvalue != value.tokvalue) && (it->tokvalue->str() != value.tokvalue->str()))
                continue;

            // same value, but old value is inconclusive so replace it
            if (it->isInconclusive() && !value.isInconclusive()) {
                *it = value;
                if (it->varId == 0)
                    it->varId = mVarId;
                break;
            }

            // Same value already exists, don't  add new value
            return false;
        }

        // Add value
        if (it == mValues->end()) {
            ValueFlow::Value v(value);
            if (v.varId == 0)
                v.varId = mVarId;
            if (v.isKnown() && v.isIntValue())
                mValues->push_front(v);
            else
                mValues->push_back(v);
        }
    } else {
        ValueFlow::Value v(value);
        if (v.varId == 0)
            v.varId = mVarId;
        mValues = new std::list<ValueFlow::Value>(1, v);
    }

    return true;
}

void Token::assignProgressValues(Token *tok)
{
    unsigned int total_count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        ++total_count;
    unsigned int count = 0;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        tok2->mProgressValue = count++ * 100 / total_count;
}

void Token::setValueType(ValueType *vt)
{
    if (vt != mValueType) {
        delete mValueType;
        mValueType = vt;
    }
}

void Token::type(const ::Type *t)
{
    mType = t;
    if (t) {
        tokType(eType);
        isEnumType(mType->isEnumType());
    } else if (mTokType == eType)
        tokType(eName);
}

