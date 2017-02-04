/*
 * simplecpp - A simple and high-fidelity C/C++ preprocessor library
 * Copyright (C) 2016 Daniel Marjamäki.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simplecpp.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib> // strtoll, etc
#include <sstream>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define NOMINMAX
#include <windows.h>
#undef ERROR
#undef TRUE
#define SIMPLECPP_WINDOWS
#endif

namespace {
    const simplecpp::TokenString DEFINE("define");
    const simplecpp::TokenString UNDEF("undef");

    const simplecpp::TokenString INCLUDE("include");

    const simplecpp::TokenString ERROR("error");
    const simplecpp::TokenString WARNING("warning");

    const simplecpp::TokenString IF("if");
    const simplecpp::TokenString IFDEF("ifdef");
    const simplecpp::TokenString IFNDEF("ifndef");
    const simplecpp::TokenString DEFINED("defined");
    const simplecpp::TokenString ELSE("else");
    const simplecpp::TokenString ELIF("elif");
    const simplecpp::TokenString ENDIF("endif");

    const simplecpp::TokenString PRAGMA("pragma");
    const simplecpp::TokenString ONCE("once");

    template<class T> std::string toString(T t)
    {
        std::ostringstream ostr;
        ostr << t;
        return ostr.str();
    }

    long long stringToLL(const std::string &s)
    {
        long long ret;
        const bool hex = (s.length()>2 && s.compare(0, 2, "0x") == 0);
        std::istringstream istr(hex ? s.substr(2) : s);
        if (hex)
            istr >> std::hex;
        istr >> ret;
        return ret;
    }

    unsigned long long stringToULL(const std::string &s)
    {
        unsigned long long ret;
        const bool hex = (s.length()>2 && s.compare(0, 2, "0x") == 0);
        std::istringstream istr(hex ? s.substr(2) : s);
        if (hex)
            istr >> std::hex;
        istr >> ret;
        return ret;
    }

    bool startsWith(const std::string &str, const std::string &s)
    {
        return (str.size() >= s.size() && str.compare(0, s.size(), s) == 0);
    }

    bool endsWith(const std::string &s, const std::string &e)
    {
        return (s.size() >= e.size() && s.compare(s.size() - e.size(), e.size(), e) == 0);
    }

    bool sameline(const simplecpp::Token *tok1, const simplecpp::Token *tok2)
    {
        return tok1 && tok2 && tok1->location.sameline(tok2->location);
    }


    static bool isAlternativeBinaryOp(const simplecpp::Token *tok, const std::string &alt)
    {
        return (tok->name &&
                tok->str == alt &&
                tok->previous &&
                tok->next &&
                (tok->previous->number || tok->previous->name || tok->previous->op == ')') &&
                (tok->next->number || tok->next->name || tok->next->op == '('));
    }

    static bool isAlternativeUnaryOp(const simplecpp::Token *tok, const std::string &alt)
    {
        return ((tok->name && tok->str == alt) &&
                (!tok->previous || tok->previous->op == '(') &&
                (tok->next && (tok->next->name || tok->next->number)));
    }
}

void simplecpp::Location::adjust(const std::string &str)
{
    if (str.find_first_of("\r\n") == std::string::npos) {
        col += str.size();
        return;
    }

    for (std::size_t i = 0U; i < str.size(); ++i) {
        col++;
        if (str[i] == '\n' || str[i] == '\r') {
            col = 0;
            line++;
            if (str[i] == '\r' && (i+1)<str.size() && str[i+1]=='\n')
                ++i;
        }
    }
}

bool simplecpp::Token::isOneOf(const char ops[]) const
{
    return (op != '\0') && (std::strchr(ops, op) != 0);
}

bool simplecpp::Token::startsWithOneOf(const char c[]) const
{
    return std::strchr(c, str[0]) != 0;
}

bool simplecpp::Token::endsWithOneOf(const char c[]) const
{
    return std::strchr(c, str[str.size() - 1U]) != 0;
}

void simplecpp::Token::printAll() const
{
    const Token *tok = this;
    while (tok->previous)
        tok = tok->previous;
    for (; tok; tok = tok->next) {
        if (tok->previous) {
            std::cout << (sameline(tok, tok->previous) ? ' ' : '\n');
        }
        std::cout << tok->str;
    }
    std::cout << std::endl;
}

void simplecpp::Token::printOut() const
{
    for (const Token *tok = this; tok; tok = tok->next) {
        if (tok != this) {
            std::cout << (sameline(tok, tok->previous) ? ' ' : '\n');
        }
        std::cout << tok->str;
    }
    std::cout << std::endl;
}

simplecpp::TokenList::TokenList(std::vector<std::string> &filenames) : frontToken(NULL), backToken(NULL), files(filenames) {}

simplecpp::TokenList::TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename, OutputList *outputList)
    : frontToken(NULL), backToken(NULL), files(filenames)
{
    readfile(istr,filename,outputList);
}

simplecpp::TokenList::TokenList(const TokenList &other) : frontToken(NULL), backToken(NULL), files(other.files)
{
    *this = other;
}

simplecpp::TokenList::~TokenList()
{
    clear();
}

void simplecpp::TokenList::operator=(const TokenList &other)
{
    if (this == &other)
        return;
    clear();
    for (const Token *tok = other.cfront(); tok; tok = tok->next)
        push_back(new Token(*tok));
    sizeOfType = other.sizeOfType;
}

void simplecpp::TokenList::clear()
{
    backToken = NULL;
    while (frontToken) {
        Token *next = frontToken->next;
        delete frontToken;
        frontToken = next;
    }
    sizeOfType.clear();
}

void simplecpp::TokenList::push_back(Token *tok)
{
    if (!frontToken)
        frontToken = tok;
    else
        backToken->next = tok;
    tok->previous = backToken;
    backToken = tok;
}

void simplecpp::TokenList::dump() const
{
    std::cout << stringify() << std::endl;
}

std::string simplecpp::TokenList::stringify() const
{
    std::ostringstream ret;
    Location loc(files);
    for (const Token *tok = cfront(); tok; tok = tok->next) {
        if (tok->location.line < loc.line || tok->location.fileIndex != loc.fileIndex) {
            ret << "\n#line " << tok->location.line << " \"" << tok->location.file() << "\"\n";
            loc = tok->location;
        }

        while (tok->location.line > loc.line) {
            ret << '\n';
            loc.line++;
        }

        if (sameline(tok->previous, tok))
            ret << ' ';

        ret << tok->str;

        loc.adjust(tok->str);
    }

    return ret.str();
}

static unsigned char readChar(std::istream &istr, unsigned int bom)
{
    unsigned char ch = (unsigned char)istr.get();

    // For UTF-16 encoded files the BOM is 0xfeff/0xfffe. If the
    // character is non-ASCII character then replace it with 0xff
    if (bom == 0xfeff || bom == 0xfffe) {
        const unsigned char ch2 = (unsigned char)istr.get();
        const int ch16 = (bom == 0xfeff) ? (ch<<8 | ch2) : (ch2<<8 | ch);
        ch = (unsigned char)((ch16 >= 0x80) ? 0xff : ch16);
    }

    // Handling of newlines..
    if (ch == '\r') {
        ch = '\n';
        if (bom == 0 && (char)istr.peek() == '\n')
            (void)istr.get();
        else if (bom == 0xfeff || bom == 0xfffe) {
            int c1 = istr.get();
            int c2 = istr.get();
            int ch16 = (bom == 0xfeff) ? (c1<<8 | c2) : (c2<<8 | c1);
            if (ch16 != '\n') {
                istr.unget();
                istr.unget();
            }
        }
    }

    return ch;
}

static unsigned char peekChar(std::istream &istr, unsigned int bom)
{
    unsigned char ch = (unsigned char)istr.peek();

    // For UTF-16 encoded files the BOM is 0xfeff/0xfffe. If the
    // character is non-ASCII character then replace it with 0xff
    if (bom == 0xfeff || bom == 0xfffe) {
        (void)istr.get();
        const unsigned char ch2 = (unsigned char)istr.peek();
        istr.unget();
        const int ch16 = (bom == 0xfeff) ? (ch<<8 | ch2) : (ch2<<8 | ch);
        ch = (unsigned char)((ch16 >= 0x80) ? 0xff : ch16);
    }

    // Handling of newlines..
    if (ch == '\r')
        ch = '\n';

    return ch;
}

static void ungetChar(std::istream &istr, unsigned int bom)
{
    istr.unget();
    if (bom == 0xfeff || bom == 0xfffe)
        istr.unget();
}

static unsigned short getAndSkipBOM(std::istream &istr)
{
    const unsigned char ch1 = istr.peek();

    // The UTF-16 BOM is 0xfffe or 0xfeff.
    if (ch1 >= 0xfe) {
        unsigned short bom = ((unsigned char)istr.get() << 8);
        if (istr.peek() >= 0xfe)
            return bom | (unsigned char)istr.get();
        return 0;
    }

    // Skip UTF-8 BOM 0xefbbbf
    if (ch1 == 0xef) {
        istr.get();
        if (istr.get() == 0xbb && istr.peek() == 0xbf) {
            (void)istr.get();
        } else {
            istr.unget();
            istr.unget();
        }
    }

    return 0;
}

bool isNameChar(unsigned char ch)
{
    return std::isalnum(ch) || ch == '_' || ch == '$';
}

static std::string escapeString(const std::string &str)
{
    std::ostringstream ostr;
    ostr << '\"';
    for (std::size_t i = 1U; i < str.size() - 1; ++i) {
        char c = str[i];
        if (c == '\\' || c == '\"' || c == '\'')
            ostr << '\\';
        ostr << c;
    }
    ostr << '\"';
    return ostr.str();
}

static void portabilityBackslash(simplecpp::OutputList *outputList, const std::vector<std::string> &files, const simplecpp::Location &location)
{
    if (!outputList)
        return;
    simplecpp::Output err(files);
    err.type = simplecpp::Output::PORTABILITY_BACKSLASH;
    err.location = location;
    err.msg = "Combination 'backslash space newline' is not portable.";
    outputList->push_back(err);
}

void simplecpp::TokenList::readfile(std::istream &istr, const std::string &filename, OutputList *outputList)
{
    std::stack<simplecpp::Location> loc;

    unsigned int multiline = 0U;

    const Token *oldLastToken = NULL;

    const unsigned short bom = getAndSkipBOM(istr);

    Location location(files);
    location.fileIndex = fileIndex(filename);
    location.line = 1U;
    location.col  = 1U;
    while (istr.good()) {
        unsigned char ch = readChar(istr,bom);
        if (!istr.good())
            break;

        if (ch == '\n') {
            if (cback() && cback()->op == '\\') {
                if (location.col > cback()->location.col + 1U)
                    portabilityBackslash(outputList, files, cback()->location);
                ++multiline;
                deleteToken(back());
            } else {
                location.line += multiline + 1;
                multiline = 0U;
            }
            if (!multiline)
                location.col = 1;

            if (oldLastToken != cback()) {
                oldLastToken = cback();
                const std::string lastline(lastLine());

                if (lastline == "# file %str%") {
                    loc.push(location);
                    location.fileIndex = fileIndex(cback()->str.substr(1U, cback()->str.size() - 2U));
                    location.line = 1U;
                } else if (lastline == "# line %num%") {
                    loc.push(location);
                    location.line = std::atol(cback()->str.c_str());
                } else if (lastline == "# line %num% %str%") {
                    loc.push(location);
                    location.fileIndex = fileIndex(cback()->str.substr(1U, cback()->str.size() - 2U));
                    location.line = std::atol(cback()->previous->str.c_str());
                }
                // #endfile
                else if (lastline == "# endfile" && !loc.empty()) {
                    location = loc.top();
                    loc.pop();
                }
            }

            continue;
        }

        if (std::isspace(ch)) {
            location.col++;
            continue;
        }

        TokenString currentToken;

        // number or name
        if (isNameChar(ch)) {
            while (istr.good() && isNameChar(ch)) {
                currentToken += ch;
                ch = readChar(istr,bom);
            }

            ungetChar(istr,bom);
        }

        // comment
        else if (ch == '/' && peekChar(istr,bom) == '/') {
            while (istr.good() && ch != '\r' && ch != '\n') {
                currentToken += ch;
                ch = readChar(istr, bom);
            }
            const std::string::size_type pos = currentToken.find_last_not_of(" \t");
            if (pos < currentToken.size() - 1U && currentToken[pos] == '\\')
                portabilityBackslash(outputList, files, location);
            if (currentToken[currentToken.size() - 1U] == '\\') {
                ++multiline;
                currentToken.erase(currentToken.size() - 1U);
            } else {
                istr.unget();
            }
        }

        // comment
        else if (ch == '/' && peekChar(istr,bom) == '*') {
            currentToken = "/*";
            (void)readChar(istr,bom);
            ch = readChar(istr,bom);
            while (istr.good()) {
                currentToken += ch;
                if (currentToken.size() >= 4U && endsWith(currentToken, "*/"))
                    break;
                ch = readChar(istr,bom);
            }
            // multiline..

            std::string::size_type pos = 0;
            while ((pos = currentToken.find("\\\n",pos)) != std::string::npos) {
                currentToken.erase(pos,2);
                ++multiline;
            }
            if (multiline || startsWith(lastLine(10),"# ")) {
                pos = 0;
                while ((pos = currentToken.find('\n',pos)) != std::string::npos) {
                    currentToken.erase(pos,1);
                    ++multiline;
                }
            }
        }

        // string / char literal
        else if (ch == '\"' || ch == '\'') {
            // C++11 raw string literal
            if (ch == '\"' && cback() && cback()->op == 'R') {
                std::string delim;
                ch = readChar(istr,bom);
                while (istr.good() && ch != '(' && ch != '\n') {
                    delim += ch;
                    ch = readChar(istr,bom);
                }
                if (!istr.good() || ch == '\n')
                    // TODO report
                    return;
                currentToken = '\"';
                const std::string endOfRawString(')' + delim + '\"');
                while (istr.good() && !endsWith(currentToken, endOfRawString))
                    currentToken += readChar(istr,bom);
                if (!endsWith(currentToken, endOfRawString))
                    // TODO report
                    return;
                currentToken.erase(currentToken.size() - endOfRawString.size(), endOfRawString.size() - 1U);
                back()->setstr(escapeString(currentToken));
                location.col += currentToken.size() + 2U + 2 * delim.size();
                continue;
            }

            currentToken = readUntil(istr,location,ch,ch,outputList);
            if (currentToken.size() < 2U)
                // TODO report
                return;
        }

        else {
            currentToken += ch;
        }

        if (currentToken == "<" && lastLine() == "# include") {
            currentToken = readUntil(istr, location, '<', '>', outputList);
            if (currentToken.size() < 2U)
                return;
        }

        push_back(new Token(currentToken, location));

        if (multiline)
            location.col += currentToken.size();
        else
            location.adjust(currentToken);
    }

    combineOperators();
}

void simplecpp::TokenList::constFold()
{
    while (cfront()) {
        // goto last '('
        Token *tok = back();
        while (tok && tok->op != '(')
            tok = tok->previous;

        // no '(', goto first token
        if (!tok)
            tok = front();

        // Constant fold expression
        constFoldUnaryNotPosNeg(tok);
        constFoldMulDivRem(tok);
        constFoldAddSub(tok);
        constFoldComparison(tok);
        constFoldBitwise(tok);
        constFoldLogicalOp(tok);
        constFoldQuestionOp(&tok);

        // If there is no '(' we are done with the constant folding
        if (tok->op != '(')
            break;

        if (!tok->next || !tok->next->next || tok->next->next->op != ')')
            break;

        tok = tok->next;
        deleteToken(tok->previous);
        deleteToken(tok->next);
    }
}

void simplecpp::TokenList::combineOperators()
{
    for (Token *tok = front(); tok; tok = tok->next) {
        if (tok->op == '.') {
            // float literals..
            if (tok->previous && tok->previous->number) {
                tok->setstr(tok->previous->str + '.');
                deleteToken(tok->previous);
                if (tok->next && tok->next->startsWithOneOf("Ee")) {
                    tok->setstr(tok->str + tok->next->str);
                    deleteToken(tok->next);
                }
            }
            if (tok->next && tok->next->number) {
                tok->setstr(tok->str + tok->next->str);
                deleteToken(tok->next);
            }
        }
        // match: [0-9.]+E [+-] [0-9]+
        const char lastChar = tok->str[tok->str.size() - 1];
        if (tok->number && (lastChar == 'E' || lastChar == 'e') && tok->next && tok->next->isOneOf("+-") && tok->next->next && tok->next->next->number) {
            tok->setstr(tok->str + tok->next->op + tok->next->next->str);
            deleteToken(tok->next);
            deleteToken(tok->next);
        }

        if (tok->op == '\0' || !tok->next || tok->next->op == '\0')
            continue;
        if (!sameline(tok,tok->next))
            continue;
        if (tok->location.col + 1U != tok->next->location.col)
            continue;

        if (tok->next->op == '=' && tok->isOneOf("=!<>+-*/%&|^")) {
            tok->setstr(tok->str + "=");
            deleteToken(tok->next);
        } else if ((tok->op == '|' || tok->op == '&') && tok->op == tok->next->op) {
            tok->setstr(tok->str + tok->next->str);
            deleteToken(tok->next);
        } else if (tok->op == ':' && tok->next->op == ':') {
            tok->setstr(tok->str + tok->next->str);
            deleteToken(tok->next);
        } else if (tok->op == '-' && tok->next->op == '>') {
            tok->setstr(tok->str + tok->next->str);
            deleteToken(tok->next);
        } else if ((tok->op == '<' || tok->op == '>') && tok->op == tok->next->op) {
            tok->setstr(tok->str + tok->next->str);
            deleteToken(tok->next);
            if (tok->next && tok->next->op == '=') {
                tok->setstr(tok->str + tok->next->str);
                deleteToken(tok->next);
            }
        } else if ((tok->op == '+' || tok->op == '-') && tok->op == tok->next->op) {
            if (tok->location.col + 1U != tok->next->location.col)
                continue;
            if (tok->previous && tok->previous->number)
                continue;
            if (tok->next->next && tok->next->next->number)
                continue;
            tok->setstr(tok->str + tok->next->str);
            deleteToken(tok->next);
        }
    }
}

static const std::string NOT("not");
void simplecpp::TokenList::constFoldUnaryNotPosNeg(simplecpp::Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        // "not" might be !
        if (isAlternativeUnaryOp(tok, NOT))
            tok->op = '!';

        if (tok->op == '!' && tok->next && tok->next->number) {
            tok->setstr(tok->next->str == "0" ? "1" : "0");
            deleteToken(tok->next);
        } else {
            if (tok->previous && (tok->previous->number || tok->previous->name))
                continue;
            if (!tok->next || !tok->next->number)
                continue;
            switch (tok->op) {
            case '+':
                tok->setstr(tok->next->str);
                deleteToken(tok->next);
                break;
            case '-':
                tok->setstr(tok->op + tok->next->str);
                deleteToken(tok->next);
                break;
            }
        }
    }
}

void simplecpp::TokenList::constFoldMulDivRem(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (!tok->previous || !tok->previous->number)
            continue;
        if (!tok->next || !tok->next->number)
            continue;

        long long result;
        if (tok->op == '*')
            result = (stringToLL(tok->previous->str) * stringToLL(tok->next->str));
        else if (tok->op == '/' || tok->op == '%') {
            long long rhs = stringToLL(tok->next->str);
            if (rhs == 0)
                throw std::overflow_error("division/modulo by zero");
            long long lhs = stringToLL(tok->previous->str);
            if (rhs == -1 && lhs == std::numeric_limits<long long>::min())
                throw std::overflow_error("division overflow");
            if (tok->op == '/')
                result = (lhs / rhs);
            else
                result = (lhs % rhs);
        } else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

void simplecpp::TokenList::constFoldAddSub(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (!tok->previous || !tok->previous->number)
            continue;
        if (!tok->next || !tok->next->number)
            continue;

        long long result;
        if (tok->op == '+')
            result = stringToLL(tok->previous->str) + stringToLL(tok->next->str);
        else if (tok->op == '-')
            result = stringToLL(tok->previous->str) - stringToLL(tok->next->str);
        else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

static const std::string NOTEQ("not_eq");
void simplecpp::TokenList::constFoldComparison(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (isAlternativeBinaryOp(tok,NOTEQ))
            tok->setstr("!=");

        if (!tok->startsWithOneOf("<>=!"))
            continue;
        if (!tok->previous || !tok->previous->number)
            continue;
        if (!tok->next || !tok->next->number)
            continue;

        int result;
        if (tok->str == "==")
            result = (stringToLL(tok->previous->str) == stringToLL(tok->next->str));
        else if (tok->str == "!=")
            result = (stringToLL(tok->previous->str) != stringToLL(tok->next->str));
        else if (tok->str == ">")
            result = (stringToLL(tok->previous->str) > stringToLL(tok->next->str));
        else if (tok->str == ">=")
            result = (stringToLL(tok->previous->str) >= stringToLL(tok->next->str));
        else if (tok->str == "<")
            result = (stringToLL(tok->previous->str) < stringToLL(tok->next->str));
        else if (tok->str == "<=")
            result = (stringToLL(tok->previous->str) <= stringToLL(tok->next->str));
        else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

static const std::string BITAND("bitand");
static const std::string BITOR("bitor");
static const std::string XOR("xor");
void simplecpp::TokenList::constFoldBitwise(Token *tok)
{
    Token * const tok1 = tok;
    for (const char *op = "&^|"; *op; op++) {
        const std::string* altop;
        if (*op == '&')
            altop = &BITAND;
        else if (*op == '|')
            altop = &BITOR;
        else
            altop = &XOR;
        for (tok = tok1; tok && tok->op != ')'; tok = tok->next) {
            if (tok->op != *op && !isAlternativeBinaryOp(tok, *altop))
                continue;
            if (!tok->previous || !tok->previous->number)
                continue;
            if (!tok->next || !tok->next->number)
                continue;
            long long result;
            if (*op == '&')
                result = (stringToLL(tok->previous->str) & stringToLL(tok->next->str));
            else if (*op == '^')
                result = (stringToLL(tok->previous->str) ^ stringToLL(tok->next->str));
            else /*if (*op == '|')*/
                result = (stringToLL(tok->previous->str) | stringToLL(tok->next->str));
            tok = tok->previous;
            tok->setstr(toString(result));
            deleteToken(tok->next);
            deleteToken(tok->next);
        }
    }
}

static const std::string AND("and");
static const std::string OR("or");
void simplecpp::TokenList::constFoldLogicalOp(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (tok->name) {
            if (isAlternativeBinaryOp(tok,AND))
                tok->setstr("&&");
            else if (isAlternativeBinaryOp(tok,OR))
                tok->setstr("||");
        }
        if (tok->str != "&&" && tok->str != "||")
            continue;
        if (!tok->previous || !tok->previous->number)
            continue;
        if (!tok->next || !tok->next->number)
            continue;

        int result;
        if (tok->str == "||")
            result = (stringToLL(tok->previous->str) || stringToLL(tok->next->str));
        else /*if (tok->str == "&&")*/
            result = (stringToLL(tok->previous->str) && stringToLL(tok->next->str));

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

void simplecpp::TokenList::constFoldQuestionOp(Token **tok1)
{
    bool gotoTok1 = false;
    for (Token *tok = *tok1; tok && tok->op != ')'; tok =  gotoTok1 ? *tok1 : tok->next) {
        gotoTok1 = false;
        if (tok->str != "?")
            continue;
        if (!tok->previous || !tok->next || !tok->next->next)
            throw std::runtime_error("invalid expression");
        if (!tok->previous->number)
            continue;
        if (tok->next->next->op != ':')
            continue;
        Token * const condTok = tok->previous;
        Token * const trueTok = tok->next;
        Token * const falseTok = trueTok->next->next;
        if (!falseTok)
            throw std::runtime_error("invalid expression");
        if (condTok == *tok1)
            *tok1 = (condTok->str != "0" ? trueTok : falseTok);
        deleteToken(condTok->next); // ?
        deleteToken(trueTok->next); // :
        deleteToken(condTok->str == "0" ? trueTok : falseTok);
        deleteToken(condTok);
        gotoTok1 = true;
    }
}

void simplecpp::TokenList::removeComments()
{
    Token *tok = frontToken;
    while (tok) {
        Token *tok1 = tok;
        tok = tok->next;
        if (tok1->comment)
            deleteToken(tok1);
    }
}

std::string simplecpp::TokenList::readUntil(std::istream &istr, const Location &location, const char start, const char end, OutputList *outputList)
{
    std::string ret;
    ret += start;

    char ch = 0;
    while (ch != end && ch != '\r' && ch != '\n' && istr.good()) {
        ch = (unsigned char)istr.get();
        ret += ch;
        if (ch == '\\')
            ret += (unsigned char)istr.get();
    }

    if (!istr.good() || ch != end) {
        clear();
        if (outputList) {
            Output err(files);
            err.type = Output::SYNTAX_ERROR;
            err.location = location;
            err.msg = std::string("No pair for character (") + start + "). Can't process file. File is either invalid or unicode, which is currently not supported.";
            outputList->push_back(err);
        }
        return "";
    }

    return ret;
}

std::string simplecpp::TokenList::lastLine(int maxsize) const
{
    std::string ret;
    int count = 0;
    for (const Token *tok = cback(); sameline(tok,cback()); tok = tok->previous) {
        if (tok->comment)
            continue;
        if (!ret.empty())
            ret = ' ' + ret;
        ret = (tok->str[0] == '\"' ? std::string("%str%")
               : std::isdigit(static_cast<unsigned char>(tok->str[0])) ? std::string("%num%") : tok->str) + ret;
        if (++count > maxsize)
            return "";
    }
    return ret;
}

unsigned int simplecpp::TokenList::fileIndex(const std::string &filename)
{
    for (unsigned int i = 0; i < files.size(); ++i) {
        if (files[i] == filename)
            return i;
    }
    files.push_back(filename);
    return files.size() - 1U;
}


namespace simplecpp {
    class Macro {
    public:
        explicit Macro(std::vector<std::string> &f) : nameTokDef(NULL), variadic(false), valueToken(NULL), endToken(NULL), files(f), tokenListDefine(f) {}

        Macro(const Token *tok, std::vector<std::string> &f) : nameTokDef(NULL), files(f), tokenListDefine(f) {
            if (sameline(tok->previous, tok))
                throw std::runtime_error("bad macro syntax");
            if (tok->op != '#')
                throw std::runtime_error("bad macro syntax");
            const Token * const hashtok = tok;
            tok = tok->next;
            if (!tok || tok->str != DEFINE)
                throw std::runtime_error("bad macro syntax");
            tok = tok->next;
            if (!tok || !tok->name || !sameline(hashtok,tok))
                throw std::runtime_error("bad macro syntax");
            if (!parseDefine(tok))
                throw std::runtime_error("bad macro syntax");
        }

        Macro(const std::string &name, const std::string &value, std::vector<std::string> &f) : nameTokDef(NULL), files(f), tokenListDefine(f) {
            const std::string def(name + ' ' + value);
            std::istringstream istr(def);
            tokenListDefine.readfile(istr);
            if (!parseDefine(tokenListDefine.cfront()))
                throw std::runtime_error("bad macro syntax");
        }

        Macro(const Macro &macro) : nameTokDef(NULL), files(macro.files), tokenListDefine(macro.files) {
            *this = macro;
        }

        void operator=(const Macro &macro) {
            if (this != &macro) {
                if (macro.tokenListDefine.empty())
                    parseDefine(macro.nameTokDef);
                else {
                    tokenListDefine = macro.tokenListDefine;
                    parseDefine(tokenListDefine.cfront());
                }
            }
        }

        /**
         * Expand macro. This will recursively expand inner macros.
         * @param output   destination tokenlist
         * @param rawtok   macro token
         * @param macros   list of macros
         * @param files    the files
         * @return token after macro
         * @throw Can throw wrongNumberOfParameters or invalidHashHash
         */
        const Token * expand(TokenList * const output,
                             const Token * rawtok,
                             const std::map<TokenString,Macro> &macros,
                             std::vector<std::string> &files) const {
            std::set<TokenString> expandedmacros;

            TokenList output2(files);

            if (functionLike() && rawtok->next && rawtok->next->op == '(') {
                // Copy macro call to a new tokenlist with no linebreaks
                const Token * const rawtok1 = rawtok;
                TokenList rawtokens2(files);
                rawtokens2.push_back(new Token(rawtok->str, rawtok1->location));
                rawtok = rawtok->next;
                rawtokens2.push_back(new Token(rawtok->str, rawtok1->location));
                rawtok = rawtok->next;
                int par = 1;
                while (rawtok && par > 0) {
                    if (rawtok->op == '(')
                        ++par;
                    else if (rawtok->op == ')')
                        --par;
                    rawtokens2.push_back(new Token(rawtok->str, rawtok1->location));
                    rawtok = rawtok->next;
                }
                if (expand(&output2, rawtok1->location, rawtokens2.cfront(), macros, expandedmacros))
                    rawtok = rawtok1->next;
            } else {
                rawtok = expand(&output2, rawtok->location, rawtok, macros, expandedmacros);
            }
            while (output2.cback() && rawtok) {
                unsigned int par = 0;
                Token* macro2tok = output2.back();
                while (macro2tok) {
                    if (macro2tok->op == '(') {
                        if (par==0)
                            break;
                        --par;
                    } else if (macro2tok->op == ')')
                        ++par;
                    macro2tok = macro2tok->previous;
                }
                if (macro2tok) { // macro2tok->op == '('
                    macro2tok = macro2tok->previous;
                    expandedmacros.insert(name());
                } else if (rawtok->op == '(')
                    macro2tok = output2.back();
                if (!macro2tok || !macro2tok->name)
                    break;
                if (output2.cfront() != output2.cback() && macro2tok->str == this->name())
                    break;
                const std::map<TokenString,Macro>::const_iterator macro = macros.find(macro2tok->str);
                if (macro == macros.end() || !macro->second.functionLike())
                    break;
                TokenList rawtokens2(files);
                const Location loc(macro2tok->location);
                while (macro2tok) {
                    Token *next = macro2tok->next;
                    rawtokens2.push_back(new Token(macro2tok->str, loc));
                    output2.deleteToken(macro2tok);
                    macro2tok = next;
                }
                par = (rawtokens2.cfront() != rawtokens2.cback()) ? 1U : 0U;
                const Token *rawtok2 = rawtok;
                for (; rawtok2; rawtok2 = rawtok2->next) {
                    rawtokens2.push_back(new Token(rawtok2->str, loc));
                    if (rawtok2->op == '(')
                        ++par;
                    else if (rawtok2->op == ')') {
                        if (par <= 1U)
                            break;
                        --par;
                    }
                }
                if (!rawtok2 || par != 1U)
                    break;
                if (macro->second.expand(&output2, rawtok->location, rawtokens2.cfront(), macros, expandedmacros) != NULL)
                    break;
                rawtok = rawtok2->next;
            }
            output->takeTokens(output2);
            return rawtok;
        }

        /** macro name */
        const TokenString &name() const {
            return nameTokDef->str;
        }

        /** location for macro definition */
        const Location &defineLocation() const {
            return nameTokDef->location;
        }

        /** how has this macro been used so far */
        const std::list<Location> &usage() const {
            return usageList;
        }

        /** is this a function like macro */
        bool functionLike() const {
            return nameTokDef->next &&
                   nameTokDef->next->op == '(' &&
                   sameline(nameTokDef, nameTokDef->next) &&
                   nameTokDef->next->location.col == nameTokDef->location.col + nameTokDef->str.size();
        }

        /** base class for errors */
        struct Error {
            Error(const Location &loc, const std::string &s) : location(loc), what(s) {}
            Location location;
            std::string what;
        };

        /** Struct that is thrown when macro is expanded with wrong number of parameters */
        struct wrongNumberOfParameters : public Error {
            wrongNumberOfParameters(const Location &loc, const std::string &macroName) : Error(loc, "Wrong number of parameters for macro \'" + macroName + "\'.") {}
        };

        /** Struct that is thrown when there is invalid ## usage */
        struct invalidHashHash : public Error {
            invalidHashHash(const Location &loc, const std::string &macroName) : Error(loc, "Invalid ## usage when expanding \'" + macroName + "\'.") {}
        };
    private:
        /** Create new token where Token::macro is set for replaced tokens */
        Token *newMacroToken(const TokenString &str, const Location &loc, bool replaced) const {
            Token *tok = new Token(str,loc);
            if (replaced)
                tok->macro = nameTokDef->str;
            return tok;
        }

        bool parseDefine(const Token *nametoken) {
            nameTokDef = nametoken;
            variadic = false;
            if (!nameTokDef) {
                valueToken = endToken = NULL;
                args.clear();
                return false;
            }

            // function like macro..
            if (functionLike()) {
                args.clear();
                const Token *argtok = nameTokDef->next->next;
                while (sameline(nametoken, argtok) && argtok->op != ')') {
                    if (argtok->op == '.' &&
                        argtok->next && argtok->next->op == '.' &&
                        argtok->next->next && argtok->next->next->op == '.' &&
                        argtok->next->next->next && argtok->next->next->next->op == ')') {
                        variadic = true;
                        if (!argtok->previous->name)
                            args.push_back("__VA_ARGS__");
                        argtok = argtok->next->next->next; // goto ')'
                        break;
                    }
                    if (argtok->op != ',')
                        args.push_back(argtok->str);
                    argtok = argtok->next;
                }
                if (!sameline(nametoken, argtok)) {
                    return false;
                }
                valueToken = argtok ? argtok->next : NULL;
            } else {
                args.clear();
                valueToken = nameTokDef->next;
            }

            if (!sameline(valueToken, nameTokDef))
                valueToken = NULL;
            endToken = valueToken;
            while (sameline(endToken, nameTokDef))
                endToken = endToken->next;
            return true;
        }

        unsigned int getArgNum(const TokenString &str) const {
            unsigned int par = 0;
            while (par < args.size()) {
                if (str == args[par])
                    return par;
                par++;
            }
            return ~0U;
        }

        std::vector<const Token *> getMacroParameters(const Token *nameTokInst, bool calledInDefine) const {
            if (!nameTokInst->next || nameTokInst->next->op != '(' || !functionLike())
                return std::vector<const Token *>();

            std::vector<const Token *> parametertokens;
            parametertokens.push_back(nameTokInst->next);
            unsigned int par = 0U;
            for (const Token *tok = nameTokInst->next->next; calledInDefine ? sameline(tok, nameTokInst) : (tok != NULL); tok = tok->next) {
                if (tok->op == '(')
                    ++par;
                else if (tok->op == ')') {
                    if (par == 0U) {
                        parametertokens.push_back(tok);
                        break;
                    }
                    --par;
                } else if (par == 0U && tok->op == ',' && (!variadic || parametertokens.size() < args.size()))
                    parametertokens.push_back(tok);
            }
            return parametertokens;
        }

        const Token *appendTokens(TokenList *tokens,
                                  const Token *lpar,
                                  const std::map<TokenString,Macro> &macros,
                                  const std::set<TokenString> &expandedmacros,
                                  const std::vector<const Token*> &parametertokens) const {
            if (!lpar || lpar->op != '(')
                return NULL;
            unsigned int par = 0;
            const Token *tok = lpar;
            while (sameline(lpar, tok)) {
                if (tok->op == '#' && sameline(tok,tok->next) && tok->next->op == '#' && sameline(tok,tok->next->next)) {
                    // A##B => AB
                    tok = expandHashHash(tokens, tok->location, tok, macros, expandedmacros, parametertokens);
                } else if (tok->op == '#' && sameline(tok, tok->next) && tok->next->op != '#') {
                    tok = expandHash(tokens, tok->location, tok, macros, expandedmacros, parametertokens);
                } else {
                    if (!expandArg(tokens, tok, tok->location, macros, expandedmacros, parametertokens)) {
                        bool expanded = false;
                        if (macros.find(tok->str) != macros.end() && expandedmacros.find(tok->str) == expandedmacros.end()) {
                            const std::map<TokenString, Macro>::const_iterator it = macros.find(tok->str);
                            const Macro &m = it->second;
                            if (!m.functionLike()) {
                                m.expand(tokens, tok, macros, files);
                                expanded = true;
                            }
                        }
                        if (!expanded)
                            tokens->push_back(new Token(*tok));
                    }

                    if (tok->op == '(')
                        ++par;
                    else if (tok->op == ')') {
                        --par;
                        if (par == 0U)
                            break;
                    }
                    tok = tok->next;
                }
            }
            return sameline(lpar,tok) ? tok : NULL;
        }

        const Token * expand(TokenList * const output, const Location &loc, const Token * const nameTokInst, const std::map<TokenString,Macro> &macros, std::set<TokenString> expandedmacros) const {
            expandedmacros.insert(nameTokInst->str);

            usageList.push_back(loc);

            if (nameTokInst->str == "__FILE__") {
                output->push_back(new Token('\"'+loc.file()+'\"', loc));
                return nameTokInst->next;
            }
            if (nameTokInst->str == "__LINE__") {
                output->push_back(new Token(toString(loc.line), loc));
                return nameTokInst->next;
            }
            if (nameTokInst->str == "__COUNTER__") {
                output->push_back(new Token(toString(usageList.size()-1U), loc));
                return nameTokInst->next;
            }

            const bool calledInDefine = (loc.fileIndex != nameTokInst->location.fileIndex ||
                                         loc.line < nameTokInst->location.line);

            std::vector<const Token*> parametertokens1(getMacroParameters(nameTokInst, calledInDefine));

            if (functionLike()) {
                // No arguments => not macro expansion
                if (nameTokInst->next && nameTokInst->next->op != '(') {
                    output->push_back(new Token(nameTokInst->str, loc));
                    return nameTokInst->next;
                }

                // Parse macro-call
                if (variadic) {
                    if (parametertokens1.size() < args.size()) {
                        throw wrongNumberOfParameters(nameTokInst->location, name());
                    }
                } else {
                    if (parametertokens1.size() != args.size() + (args.empty() ? 2U : 1U))
                        throw wrongNumberOfParameters(nameTokInst->location, name());
                }
            }

            // If macro call uses __COUNTER__ then expand that first
            TokenList tokensparams(files);
            std::vector<const Token *> parametertokens2;
            if (!parametertokens1.empty()) {
                bool counter = false;
                for (const Token *tok = parametertokens1[0]; tok != parametertokens1.back(); tok = tok->next) {
                    if (tok->str == "__COUNTER__") {
                        counter = true;
                        break;
                    }
                }

                const std::map<TokenString,Macro>::const_iterator m = macros.find("__COUNTER__");

                if (!counter || m == macros.end())
                    parametertokens2.swap(parametertokens1);
                else {
                    const Macro &counterMacro = m->second;
                    unsigned int par = 0;
                    for (const Token *tok = parametertokens1[0]; tok && par < parametertokens1.size(); tok = tok->next) {
                        if (tok->str == "__COUNTER__") {
                            tokensparams.push_back(new Token(toString(counterMacro.usageList.size()), tok->location));
                            counterMacro.usageList.push_back(tok->location);
                        } else {
                            tokensparams.push_back(new Token(*tok));
                            if (tok == parametertokens1[par]) {
                                parametertokens2.push_back(tokensparams.cback());
                                par++;
                            }
                        }
                    }
                }
            }

            Token * const output_end_1 = output->back();

            // expand
            for (const Token *tok = valueToken; tok != endToken;) {
                if (tok->op != '#') {
                    // A##B => AB
                    if (tok->next && tok->next->op == '#' && tok->next->next && tok->next->next->op == '#') {
                        if (!sameline(tok, tok->next->next->next))
                            throw invalidHashHash(tok->location, name());
                        output->push_back(newMacroToken(expandArgStr(tok, parametertokens2), loc, isReplaced(expandedmacros)));
                        tok = tok->next;
                    } else {
                        tok = expandToken(output, loc, tok, macros, expandedmacros, parametertokens2);
                    }
                    continue;
                }

                int numberOfHash = 1;
                const Token *hashToken = tok->next;
                while (sameline(tok,hashToken) && hashToken->op == '#') {
                    hashToken = hashToken->next;
                    ++numberOfHash;
                }
                if (numberOfHash == 4) {
                    // # ## #  => ##
                    output->push_back(newMacroToken("##", loc, isReplaced(expandedmacros)));
                    tok = hashToken;
                    continue;
                }

                tok = tok->next;
                if (tok == endToken) {
                    output->push_back(new Token(*tok->previous));
                    break;
                }
                if (tok->op == '#') {
                    // A##B => AB
                    tok = expandHashHash(output, loc, tok->previous, macros, expandedmacros, parametertokens2);
                } else {
                    // #123 => "123"
                    tok = expandHash(output, loc, tok->previous, macros, expandedmacros, parametertokens2);
                }
            }

            if (!functionLike()) {
                for (Token *tok = output_end_1 ? output_end_1->next : output->front(); tok; tok = tok->next) {
                    tok->macro = nameTokInst->str;
                }
            }

            if (!parametertokens1.empty())
                parametertokens1.swap(parametertokens2);

            return functionLike() ? parametertokens2.back()->next : nameTokInst->next;
        }

        const Token *expandToken(TokenList *output, const Location &loc, const Token *tok, const std::map<TokenString,Macro> &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens) const {
            // Not name..
            if (!tok->name) {
                output->push_back(newMacroToken(tok->str, loc, true));
                return tok->next;
            }

            // Macro parameter..
            {
                TokenList temp(files);
                if (expandArg(&temp, tok, loc, macros, expandedmacros, parametertokens)) {
                    if (!(temp.cback() && temp.cback()->name && tok->next && tok->next->op == '(')) {
                        output->takeTokens(temp);
                        return tok->next;
                    }

                    const std::map<TokenString, Macro>::const_iterator it = macros.find(temp.cback()->str);
                    if (it == macros.end() || expandedmacros.find(temp.cback()->str) != expandedmacros.end()) {
                        output->takeTokens(temp);
                        return tok->next;
                    }

                    const Macro &calledMacro = it->second;
                    if (!calledMacro.functionLike()) {
                        output->takeTokens(temp);
                        return tok->next;
                    }

                    TokenList temp2(files);
                    temp2.push_back(new Token(temp.cback()->str, tok->location));

                    const Token *tok2 = appendTokens(&temp2, tok->next, macros, expandedmacros, parametertokens);
                    if (!tok2)
                        return tok->next;

                    output->takeTokens(temp);
                    output->deleteToken(output->back());
                    calledMacro.expand(output, loc, temp2.cfront(), macros, expandedmacros);

                    return tok2->next;
                }
            }

            // Macro..
            const std::map<TokenString, Macro>::const_iterator it = macros.find(tok->str);
            if (it != macros.end() && expandedmacros.find(tok->str) == expandedmacros.end()) {
                const Macro &calledMacro = it->second;
                if (!calledMacro.functionLike())
                    return calledMacro.expand(output, loc, tok, macros, expandedmacros);
                if (!sameline(tok, tok->next) || tok->next->op != '(') {
                    output->push_back(newMacroToken(tok->str, loc, true));
                    return tok->next;
                }
                TokenList tokens(files);
                tokens.push_back(new Token(*tok));
                const Token *tok2 = appendTokens(&tokens, tok->next, macros, expandedmacros, parametertokens);
                if (!tok2) {
                    output->push_back(newMacroToken(tok->str, loc, true));
                    return tok->next;
                }
                calledMacro.expand(output, loc, tokens.cfront(), macros, expandedmacros);
                return tok2->next;
            }

            output->push_back(newMacroToken(tok->str, loc, true));
            return tok->next;
        }

        bool expandArg(TokenList *output, const Token *tok, const std::vector<const Token*> &parametertokens) const {
            if (!tok->name)
                return false;

            const unsigned int argnr = getArgNum(tok->str);
            if (argnr >= args.size())
                return false;

            // empty variadic parameter
            if (variadic && argnr + 1U >= parametertokens.size())
                return true;

            for (const Token *partok = parametertokens[argnr]->next; partok != parametertokens[argnr + 1U]; partok = partok->next)
                output->push_back(new Token(*partok));

            return true;
        }

        bool expandArg(TokenList *output, const Token *tok, const Location &loc, const std::map<TokenString, Macro> &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens) const {
            if (!tok->name)
                return false;
            const unsigned int argnr = getArgNum(tok->str);
            if (argnr >= args.size())
                return false;
            if (variadic && argnr + 1U >= parametertokens.size()) // empty variadic parameter
                return true;
            for (const Token *partok = parametertokens[argnr]->next; partok != parametertokens[argnr + 1U];) {
                const std::map<TokenString, Macro>::const_iterator it = macros.find(partok->str);
                if (it != macros.end() && (partok->str == name() || expandedmacros.find(partok->str) == expandedmacros.end()))
                    partok = it->second.expand(output, loc, partok, macros, expandedmacros);
                else {
                    output->push_back(newMacroToken(partok->str, loc, isReplaced(expandedmacros)));
                    partok = partok->next;
                }
            }
            return true;
        }

        /**
         * Get string for token. If token is argument, the expanded string is returned.
         * @param tok              The token
         * @param parametertokens  parameters given when expanding this macro
         * @return string
         */
        std::string expandArgStr(const Token *tok, const std::vector<const Token *> &parametertokens) const {
            TokenList tokens(files);
            if (expandArg(&tokens, tok, parametertokens)) {
                std::string s;
                for (const Token *tok2 = tokens.cfront(); tok2; tok2 = tok2->next)
                    s += tok2->str;
                return s;
            }
            return tok->str;
        }

        /**
         * Expand #X => "X"
         * @param output  destination tokenlist
         * @param loc     location for expanded token
         * @param tok     The # token
         * @param macros  all macros
         * @param expandedmacros   set with expanded macros, with this macro
         * @param parametertokens  parameters given when expanding this macro
         * @return token after the X
         */
        const Token *expandHash(TokenList *output, const Location &loc, const Token *tok, const std::map<TokenString, Macro> &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens) const {
            TokenList tokenListHash(files);
            tok = expandToken(&tokenListHash, loc, tok->next, macros, expandedmacros, parametertokens);
            std::ostringstream ostr;
            ostr << '\"';
            for (const Token *hashtok = tokenListHash.cfront(); hashtok; hashtok = hashtok->next)
                ostr << hashtok->str;
            ostr << '\"';
            output->push_back(newMacroToken(escapeString(ostr.str()), loc, isReplaced(expandedmacros)));
            return tok;
        }

        /**
         * Expand A##B => AB
         * The A should already be expanded. Call this when you reach the first # token
         * @param output  destination tokenlist
         * @param loc     location for expanded token
         * @param tok     first # token
         * @param macros  all macros
         * @param expandedmacros   set with expanded macros, with this macro
         * @param parametertokens  parameters given when expanding this macro
         * @return token after B
         */
        const Token *expandHashHash(TokenList *output, const Location &loc, const Token *tok, const std::map<TokenString, Macro> &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens) const {
            Token *A = output->back();
            if (!A)
                throw invalidHashHash(tok->location, name());
            if (!sameline(tok, tok->next) || !sameline(tok, tok->next->next))
                throw invalidHashHash(tok->location, name());

            Token *B = tok->next->next;
            std::string strAB;

            const bool varargs = variadic && args.size() >= 1U && B->str == args[args.size()-1U];

            TokenList tokensB(files);
            if (expandArg(&tokensB, B, parametertokens)) {
                if (tokensB.empty())
                    strAB = A->str;
                else if (varargs && A->op == ',') {
                    strAB = ",";
                } else {
                    strAB = A->str + tokensB.cfront()->str;
                    tokensB.deleteToken(tokensB.front());
                }
            } else {
                strAB = A->str + B->str;
            }

            const Token *nextTok = B->next;

            if (varargs && tokensB.empty() && tok->previous->str == ",")
                output->deleteToken(A);
            else {
                output->deleteToken(A);
                TokenList tokens(files);
                tokens.push_back(new Token(strAB, tok->location));
                // for function like macros, push the (...)
                if (tokensB.empty() && sameline(B,B->next) && B->next->op=='(') {
                    const std::map<TokenString,Macro>::const_iterator it = macros.find(strAB);
                    if (it != macros.end() && expandedmacros.find(strAB) == expandedmacros.end() && it->second.functionLike()) {
                        const Token *tok2 = appendTokens(&tokens, B->next, macros, expandedmacros, parametertokens);
                        if (tok2)
                            nextTok = tok2->next;
                    }
                }
                expandToken(output, loc, tokens.cfront(), macros, expandedmacros, parametertokens);
                for (Token *b = tokensB.front(); b; b = b->next)
                    b->location = loc;
                output->takeTokens(tokensB);
            }

            return nextTok;
        }

        bool isReplaced(const std::set<std::string> &expandedmacros) const {
            // return true if size > 1
            std::set<std::string>::const_iterator it = expandedmacros.begin();
            if (it == expandedmacros.end())
                return false;
            ++it;
            return (it != expandedmacros.end());
        }

        /** name token in definition */
        const Token *nameTokDef;

        /** arguments for macro */
        std::vector<TokenString> args;

        /** is macro variadic? */
        bool variadic;

        /** first token in replacement string */
        const Token *valueToken;

        /** token after replacement string */
        const Token *endToken;

        /** files */
        std::vector<std::string> &files;

        /** this is used for -D where the definition is not seen anywhere in code */
        TokenList tokenListDefine;

        /** usage of this macro */
        mutable std::list<Location> usageList;
    };
}


namespace simplecpp {
#ifdef SIMPLECPP_WINDOWS

    static bool realFileName(const std::vector<CHAR> &buf, std::ostream &ostr)
    {
        // Detect root directory, see simplecpp:realFileName returns the wrong root path #45
        if ((buf.size()==2 || (buf.size()>2 && buf[2]=='\0'))
            && std::isalpha(buf[0]) && buf[1]==':') {
            ostr << (char)buf[0];
            ostr << (char)buf[1];
            return true;
        }
        WIN32_FIND_DATAA FindFileData;
        HANDLE hFind = FindFirstFileA(&buf[0], &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
            return false;
        ostr << FindFileData.cFileName;
        FindClose(hFind);
        return true;
    }

    std::string realFilename(const std::string &f)
    {
        std::vector<CHAR> buf(f.size()+1U, 0);
        for (unsigned int i = 0; i < f.size(); ++i)
            buf[i] = f[i];
        std::ostringstream ostr;
        std::string::size_type sep = 0;
        while ((sep = f.find_first_of("\\/", sep + 1U)) != std::string::npos) {
            buf[sep] = 0;
            if (!realFileName(buf,ostr))
                return f;
            ostr << '/';
            buf[sep] = '/';
        }
        if (!realFileName(buf, ostr))
            return f;
        return ostr.str();
    }
#else
#define realFilename(f)  f
#endif

    /**
     * perform path simplifications for . and ..
     */
    std::string simplifyPath(std::string path)
    {
        std::string::size_type pos;

        // replace backslash separators
        std::replace(path.begin(), path.end(), '\\', '/');

        // "./" at the start
        if (path.size() > 3 && path.compare(0,2,"./") == 0 && path[2] != '/')
            path.erase(0,2);

        // remove "/./"
        pos = 0;
        while ((pos = path.find("/./",pos)) != std::string::npos) {
            path.erase(pos,2);
        }

        // remove "xyz/../"
        pos = 1U;
        while ((pos = path.find("/../", pos)) != std::string::npos) {
            const std::string::size_type pos1 = path.rfind('/', pos - 1U);
            if (pos1 == std::string::npos)
                pos++;
            else {
                path.erase(pos1,pos-pos1+3);
                pos = std::min((std::string::size_type)1, pos1);
            }
        }

        return realFilename(path);
    }
}

namespace {
    /** Evaluate sizeof(type) */
    void simplifySizeof(simplecpp::TokenList &expr, const std::map<std::string, std::size_t> &sizeOfType)
    {
        for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
            if (tok->str != "sizeof")
                continue;
            simplecpp::Token *tok1 = tok->next;
            if (!tok1) {
                throw std::runtime_error("missed sizeof argument");
            }
            simplecpp::Token *tok2 = tok1->next;
            if (!tok2) {
                throw std::runtime_error("missed sizeof argument");
            }
            if (tok1->op == '(') {
                tok1 = tok1->next;
                while (tok2->op != ')')
                    tok2 = tok2->next;
            }

            std::string type;
            for (simplecpp::Token *typeToken = tok1; typeToken != tok2; typeToken = typeToken->next) {
                if ((typeToken->str == "unsigned" || typeToken->str == "signed") && typeToken->next->name)
                    continue;
                if (typeToken->str == "*" && type.find('*') != std::string::npos)
                    continue;
                if (!type.empty())
                    type += ' ';
                type += typeToken->str;
            }

            const std::map<std::string, std::size_t>::const_iterator it = sizeOfType.find(type);
            if (it != sizeOfType.end())
                tok->setstr(toString(it->second));
            else
                continue;

            tok2 = tok2->next;
            while (tok->next != tok2)
                expr.deleteToken(tok->next);
        }
    }

    void simplifyName(simplecpp::TokenList &expr)
    {
        std::set<std::string> altop;
        altop.insert("and");
        altop.insert("or");
        altop.insert("bitand");
        altop.insert("bitor");
        altop.insert("not");
        altop.insert("not_eq");
        altop.insert("xor");
        for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
            if (tok->name) {
                if (altop.find(tok->str) != altop.end()) {
                    bool alt;
                    if (tok->str == "not") {
                        alt = isAlternativeUnaryOp(tok,tok->str);
                    } else {
                        alt = isAlternativeBinaryOp(tok,tok->str);
                    }
                    if (alt)
                        continue;
                }
                tok->setstr("0");
            }
        }
    }

    void simplifyNumbers(simplecpp::TokenList &expr)
    {
        for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
            if (tok->str.size() == 1U)
                continue;
            if (tok->str.compare(0,2,"0x") == 0)
                tok->setstr(toString(stringToULL(tok->str)));
            else if (tok->str[0] == '\'')
                tok->setstr(toString(tok->str[1] & 0xffU));
        }
    }

    long long evaluate(simplecpp::TokenList &expr, const std::map<std::string, std::size_t> &sizeOfType)
    {
        simplifySizeof(expr, sizeOfType);
        simplifyName(expr);
        simplifyNumbers(expr);
        expr.constFold();
        // TODO: handle invalid expressions
        return expr.cfront() && expr.cfront() == expr.cback() && expr.cfront()->number ? stringToLL(expr.cfront()->str) : 0LL;
    }

    const simplecpp::Token *gotoNextLine(const simplecpp::Token *tok)
    {
        const unsigned int line = tok->location.line;
        const unsigned int file = tok->location.fileIndex;
        while (tok && tok->location.line == line && tok->location.fileIndex == file)
            tok = tok->next;
        return tok;
    }

    std::string openHeader(std::ifstream &f, const simplecpp::DUI &dui, const std::string &sourcefile, const std::string &header, bool systemheader)
    {
        if (!systemheader) {
            if (sourcefile.find_first_of("\\/") != std::string::npos) {
                const std::string s = sourcefile.substr(0, sourcefile.find_last_of("\\/") + 1U) + header;
                f.open(s.c_str());
                if (f.is_open())
                    return simplecpp::simplifyPath(s);
            } else {
                f.open(header.c_str());
                if (f.is_open())
                    return simplecpp::simplifyPath(header);
            }
        }

        for (std::list<std::string>::const_iterator it = dui.includePaths.begin(); it != dui.includePaths.end(); ++it) {
            std::string s = *it;
            if (!s.empty() && s[s.size()-1U]!='/' && s[s.size()-1U]!='\\')
                s += '/';
            s += header;
            f.open(s.c_str());
            if (f.is_open())
                return simplecpp::simplifyPath(s);
        }

        return "";
    }

    std::string getFileName(const std::map<std::string, simplecpp::TokenList *> &filedata, const std::string &sourcefile, const std::string &header, const simplecpp::DUI &dui, bool systemheader)
    {
        if (!systemheader) {
            if (sourcefile.find_first_of("\\/") != std::string::npos) {
                const std::string s(simplecpp::simplifyPath(sourcefile.substr(0, sourcefile.find_last_of("\\/") + 1U) + header));
                if (filedata.find(s) != filedata.end())
                    return s;
            } else {
                std::string s = simplecpp::simplifyPath(header);
                if (filedata.find(s) != filedata.end())
                    return s;
            }
        }

        for (std::list<std::string>::const_iterator it = dui.includePaths.begin(); it != dui.includePaths.end(); ++it) {
            std::string s = *it;
            if (!s.empty() && s[s.size()-1U]!='/' && s[s.size()-1U]!='\\')
                s += '/';
            s += header;
            s = simplecpp::simplifyPath(s);
            if (filedata.find(s) != filedata.end())
                return s;
        }

        return "";
    }

    bool hasFile(const std::map<std::string, simplecpp::TokenList *> &filedata, const std::string &sourcefile, const std::string &header, const simplecpp::DUI &dui, bool systemheader)
    {
        return !getFileName(filedata, sourcefile, header, dui, systemheader).empty();
    }
}

std::map<std::string, simplecpp::TokenList*> simplecpp::load(const simplecpp::TokenList &rawtokens, std::vector<std::string> &fileNumbers, const simplecpp::DUI &dui, simplecpp::OutputList *outputList)
{
    std::map<std::string, simplecpp::TokenList*> ret;

    std::list<const Token *> filelist;

    // -include files
    for (std::list<std::string>::const_iterator it = dui.includes.begin(); it != dui.includes.end(); ++it) {
        const std::string &filename = realFilename(*it);

        if (ret.find(filename) != ret.end())
            continue;

        std::ifstream fin(filename.c_str());
        if (!fin.is_open())
            continue;

        TokenList *tokenlist = new TokenList(fin, fileNumbers, filename, outputList);
        if (!tokenlist->front()) {
            delete tokenlist;
            continue;
        }

        ret[filename] = tokenlist;
        filelist.push_back(tokenlist->front());
    }

    for (const Token *rawtok = rawtokens.cfront(); rawtok || !filelist.empty(); rawtok = rawtok ? rawtok->next : NULL) {
        if (rawtok == NULL) {
            rawtok = filelist.back();
            filelist.pop_back();
        }

        if (rawtok->op != '#' || sameline(rawtok->previousSkipComments(), rawtok))
            continue;

        rawtok = rawtok->nextSkipComments();
        if (!rawtok || rawtok->str != INCLUDE)
            continue;

        const std::string &sourcefile = rawtok->location.file();

        const Token *htok = rawtok->nextSkipComments();
        if (!sameline(rawtok, htok))
            continue;

        bool systemheader = (htok->str[0] == '<');

        const std::string header(realFilename(htok->str.substr(1U, htok->str.size() - 2U)));
        if (hasFile(ret, sourcefile, header, dui, systemheader))
            continue;

        std::ifstream f;
        const std::string header2 = openHeader(f,dui,sourcefile,header,systemheader);
        if (!f.is_open())
            continue;

        TokenList *tokens = new TokenList(f, fileNumbers, header2, outputList);
        ret[header2] = tokens;
        if (tokens->front())
            filelist.push_back(tokens->front());
    }

    return ret;
}

static bool preprocessToken(simplecpp::TokenList &output, const simplecpp::Token **tok1, std::map<std::string, simplecpp::Macro> &macros, std::vector<std::string> &files, simplecpp::OutputList *outputList)
{
    const simplecpp::Token *tok = *tok1;
    const std::map<std::string,simplecpp::Macro>::const_iterator it = macros.find(tok->str);
    if (it != macros.end()) {
        simplecpp::TokenList value(files);
        try {
            *tok1 = it->second.expand(&value, tok, macros, files);
        } catch (simplecpp::Macro::Error &err) {
            if (outputList) {
                simplecpp::Output out(files);
                out.type = simplecpp::Output::SYNTAX_ERROR;
                out.location = err.location;
                out.msg = "failed to expand \'" + tok->str + "\', " + err.what;
                outputList->push_back(out);
            }
            return false;
        }
        output.takeTokens(value);
    } else {
        if (!tok->comment)
            output.push_back(new simplecpp::Token(*tok));
        *tok1 = tok->next;
    }
    return true;
}

void simplecpp::preprocess(simplecpp::TokenList &output, const simplecpp::TokenList &rawtokens, std::vector<std::string> &files, std::map<std::string, simplecpp::TokenList *> &filedata, const simplecpp::DUI &dui, simplecpp::OutputList *outputList, std::list<simplecpp::MacroUsage> *macroUsage)
{
    std::map<std::string, std::size_t> sizeOfType(rawtokens.sizeOfType);
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("char"), sizeof(char)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("short"), sizeof(short)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("short int"), sizeOfType["short"]));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("int"), sizeof(int)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long"), sizeof(long)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long int"), sizeOfType["long"]));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long long"), sizeof(long long)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("float"), sizeof(float)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("double"), sizeof(double)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long double"), sizeof(long double)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("char *"), sizeof(char *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("short *"), sizeof(short *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("short int *"), sizeOfType["short *"]));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("int *"), sizeof(int *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long *"), sizeof(long *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long int *"), sizeOfType["long *"]));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long long *"), sizeof(long long *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("float *"), sizeof(float *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("double *"), sizeof(double *)));
    sizeOfType.insert(std::pair<std::string, std::size_t>(std::string("long double *"), sizeof(long double *)));

    std::map<TokenString, Macro> macros;
    for (std::list<std::string>::const_iterator it = dui.defines.begin(); it != dui.defines.end(); ++it) {
        const std::string &macrostr = *it;
        const std::string::size_type eq = macrostr.find('=');
        const std::string::size_type par = macrostr.find('(');
        const std::string macroname = macrostr.substr(0, std::min(eq,par));
        if (dui.undefined.find(macroname) != dui.undefined.end())
            continue;
        const std::string lhs(macrostr.substr(0,eq));
        const std::string rhs(eq==std::string::npos ? std::string("1") : macrostr.substr(eq+1));
        const Macro macro(lhs, rhs, files);
        macros.insert(std::pair<TokenString,Macro>(macro.name(), macro));
    }

    macros.insert(std::pair<TokenString,Macro>("__FILE__", Macro("__FILE__", "__FILE__", files)));
    macros.insert(std::pair<TokenString,Macro>("__LINE__", Macro("__LINE__", "__LINE__", files)));
    macros.insert(std::pair<TokenString,Macro>("__COUNTER__", Macro("__COUNTER__", "__COUNTER__", files)));

    // TRUE => code in current #if block should be kept
    // ELSE_IS_TRUE => code in current #if block should be dropped. the code in the #else should be kept.
    // ALWAYS_FALSE => drop all code in #if and #else
    enum IfState { TRUE, ELSE_IS_TRUE, ALWAYS_FALSE };
    std::stack<int> ifstates;
    ifstates.push(TRUE);

    std::list<TokenList *> includes;
    std::stack<const Token *> includetokenstack;

    std::set<std::string> pragmaOnce;

    includetokenstack.push(rawtokens.cfront());
    for (std::list<std::string>::const_iterator it = dui.includes.begin(); it != dui.includes.end(); ++it) {
        const std::map<std::string, TokenList*>::const_iterator f = filedata.find(*it);
        if (f != filedata.end())
            includetokenstack.push(f->second->cfront());
    }

    for (const Token *rawtok = NULL; rawtok || !includetokenstack.empty();) {
        if (rawtok == NULL) {
            rawtok = includetokenstack.top();
            includetokenstack.pop();
            continue;
        }

        if (rawtok->op == '#' && !sameline(rawtok->previous, rawtok)) {
            rawtok = rawtok->next;
            if (!rawtok || !rawtok->name) {
                if (rawtok)
                    rawtok = gotoNextLine(rawtok);
                continue;
            }

            if (ifstates.size() <= 1U && (rawtok->str == ELIF || rawtok->str == ELSE || rawtok->str == ENDIF)) {
                if (outputList) {
                    simplecpp::Output err(files);
                    err.type = Output::SYNTAX_ERROR;
                    err.location = rawtok->location;
                    err.msg = "#" + rawtok->str + " without #if";
                    outputList->push_back(err);
                }
                output.clear();
                return;
            }

            if (ifstates.top() == TRUE && (rawtok->str == ERROR || rawtok->str == WARNING)) {
                if (outputList) {
                    simplecpp::Output err(rawtok->location.files);
                    err.type = rawtok->str == ERROR ? Output::ERROR : Output::WARNING;
                    err.location = rawtok->location;
                    for (const Token *tok = rawtok->next; tok && sameline(rawtok,tok); tok = tok->next) {
                        if (!err.msg.empty() && isNameChar(tok->str[0]))
                            err.msg += ' ';
                        err.msg += tok->str;
                    }
                    err.msg = '#' + rawtok->str + ' ' + err.msg;
                    outputList->push_back(err);
                }
                if (rawtok->str == ERROR) {
                    output.clear();
                    return;
                }
            }

            if (rawtok->str == DEFINE) {
                if (ifstates.top() != TRUE)
                    continue;
                try {
                    const Macro &macro = Macro(rawtok->previous, files);
                    if (dui.undefined.find(macro.name()) == dui.undefined.end()) {
                        std::map<TokenString, Macro>::iterator it = macros.find(macro.name());
                        if (it == macros.end())
                            macros.insert(std::pair<TokenString, Macro>(macro.name(), macro));
                        else
                            it->second = macro;
                    }
                } catch (const std::runtime_error &) {
                    if (outputList) {
                        simplecpp::Output err(files);
                        err.type = Output::SYNTAX_ERROR;
                        err.location = rawtok->location;
                        err.msg = "Failed to parse #define";
                        outputList->push_back(err);
                    }
                    output.clear();
                    return;
                }
            } else if (ifstates.top() == TRUE && rawtok->str == INCLUDE) {
                TokenList inc1(files);
                for (const Token *inctok = rawtok->next; sameline(rawtok,inctok); inctok = inctok->next) {
                    if (!inctok->comment)
                        inc1.push_back(new Token(*inctok));
                }
                TokenList inc2(files);
                if (!inc1.empty() && inc1.cfront()->name) {
                    const Token *inctok = inc1.cfront();
                    if (!preprocessToken(inc2, &inctok, macros, files, outputList)) {
                        output.clear();
                        return;
                    }
                } else {
                    inc2.takeTokens(inc1);
                }

                if (inc2.empty() || inc2.cfront()->str.size() <= 2U) {
                    if (outputList) {
                        simplecpp::Output err(files);
                        err.type = Output::SYNTAX_ERROR;
                        err.location = rawtok->location;
                        err.msg = "No header in #include";
                        outputList->push_back(err);
                    }
                    output.clear();
                    return;
                }

                const Token *inctok = inc2.cfront();

                const bool systemheader = (inctok->op == '<');
                const std::string header(realFilename(inctok->str.substr(1U, inctok->str.size() - 2U)));
                std::string header2 = getFileName(filedata, rawtok->location.file(), header, dui, systemheader);
                if (header2.empty()) {
                    // try to load file..
                    std::ifstream f;
                    header2 = openHeader(f, dui, rawtok->location.file(), header, systemheader);
                    if (f.is_open()) {
                        TokenList *tokens = new TokenList(f, files, header2, outputList);
                        filedata[header2] = tokens;
                    }
                }
                if (header2.empty()) {
                    if (outputList) {
                        simplecpp::Output out(files);
                        out.type = Output::MISSING_HEADER;
                        out.location = rawtok->location;
                        out.msg = "Header not found: " + rawtok->next->str;
                        outputList->push_back(out);
                    }
                } else if (includetokenstack.size() >= 400) {
                    if (outputList) {
                        simplecpp::Output out(files);
                        out.type = Output::INCLUDE_NESTED_TOO_DEEPLY;
                        out.location = rawtok->location;
                        out.msg = "#include nested too deeply";
                        outputList->push_back(out);
                    }
                } else if (pragmaOnce.find(header2) == pragmaOnce.end()) {
                    includetokenstack.push(gotoNextLine(rawtok));
                    const TokenList *includetokens = filedata.find(header2)->second;
                    rawtok = includetokens ? includetokens->cfront() : 0;
                    continue;
                }
            } else if (rawtok->str == IF || rawtok->str == IFDEF || rawtok->str == IFNDEF || rawtok->str == ELIF) {
                if (!sameline(rawtok,rawtok->next)) {
                    if (outputList) {
                        simplecpp::Output out(files);
                        out.type = Output::SYNTAX_ERROR;
                        out.location = rawtok->location;
                        out.msg = "Syntax error in #" + rawtok->str;
                        outputList->push_back(out);
                    }
                    output.clear();
                    return;
                }

                bool conditionIsTrue;
                if (ifstates.top() == ALWAYS_FALSE || (ifstates.top() == ELSE_IS_TRUE && rawtok->str != ELIF))
                    conditionIsTrue = false;
                else if (rawtok->str == IFDEF)
                    conditionIsTrue = (macros.find(rawtok->next->str) != macros.end());
                else if (rawtok->str == IFNDEF)
                    conditionIsTrue = (macros.find(rawtok->next->str) == macros.end());
                else { /*if (rawtok->str == IF || rawtok->str == ELIF)*/
                    TokenList expr(files);
                    for (const Token *tok = rawtok->next; tok && tok->location.sameline(rawtok->location); tok = tok->next) {
                        if (!tok->name) {
                            expr.push_back(new Token(*tok));
                            continue;
                        }

                        if (tok->str == DEFINED) {
                            tok = tok->next;
                            const bool par = (tok && tok->op == '(');
                            if (par)
                                tok = tok->next;
                            if (tok) {
                                if (macros.find(tok->str) != macros.end())
                                    expr.push_back(new Token("1", tok->location));
                                else
                                    expr.push_back(new Token("0", tok->location));
                            }
                            if (par)
                                tok = tok ? tok->next : NULL;
                            if (!tok || !sameline(rawtok,tok) || (par && tok->op != ')')) {
                                if (outputList) {
                                    Output out(rawtok->location.files);
                                    out.type = Output::SYNTAX_ERROR;
                                    out.location = rawtok->location;
                                    out.msg = "failed to evaluate " + std::string(rawtok->str == IF ? "#if" : "#elif") + " condition";
                                    outputList->push_back(out);
                                }
                                output.clear();
                                return;
                            }
                            continue;
                        }

                        const Token *tmp = tok;
                        if (!preprocessToken(expr, &tmp, macros, files, outputList)) {
                            output.clear();
                            return;
                        }
                    }
                    try {
                        conditionIsTrue = (evaluate(expr, sizeOfType) != 0);
                    } catch (const std::exception &) {
                        if (outputList) {
                            Output out(rawtok->location.files);
                            out.type = Output::SYNTAX_ERROR;
                            out.location = rawtok->location;
                            out.msg = "failed to evaluate " + std::string(rawtok->str == IF ? "#if" : "#elif") + " condition";
                            outputList->push_back(out);
                        }
                        output.clear();
                        return;
                    }
                }

                if (rawtok->str != ELIF) {
                    // push a new ifstate..
                    if (ifstates.top() != TRUE)
                        ifstates.push(ALWAYS_FALSE);
                    else
                        ifstates.push(conditionIsTrue ? TRUE : ELSE_IS_TRUE);
                } else if (ifstates.top() == TRUE) {
                    ifstates.top() = ALWAYS_FALSE;
                } else if (ifstates.top() == ELSE_IS_TRUE && conditionIsTrue) {
                    ifstates.top() = TRUE;
                }
            } else if (rawtok->str == ELSE) {
                ifstates.top() = (ifstates.top() == ELSE_IS_TRUE) ? TRUE : ALWAYS_FALSE;
            } else if (rawtok->str == ENDIF) {
                ifstates.pop();
            } else if (rawtok->str == UNDEF) {
                if (ifstates.top() == TRUE) {
                    const Token *tok = rawtok->next;
                    while (sameline(rawtok,tok) && tok->comment)
                        tok = tok->next;
                    if (sameline(rawtok, tok))
                        macros.erase(tok->str);
                }
            } else if (ifstates.top() == TRUE && rawtok->str == PRAGMA && rawtok->next && rawtok->next->str == ONCE && sameline(rawtok,rawtok->next)) {
                pragmaOnce.insert(rawtok->location.file());
            }
            rawtok = gotoNextLine(rawtok);
            continue;
        }

        if (ifstates.top() != TRUE) {
            // drop code
            rawtok = gotoNextLine(rawtok);
            continue;
        }

        bool hash=false, hashhash=false;
        if (rawtok->op == '#' && sameline(rawtok,rawtok->next)) {
            if (rawtok->next->op != '#') {
                hash = true;
                rawtok = rawtok->next; // skip '#'
            } else if (sameline(rawtok,rawtok->next->next)) {
                hashhash = true;
                rawtok = rawtok->next->next; // skip '#' '#'
            }
        }

        const Location loc(rawtok->location);
        TokenList tokens(files);

        if (!preprocessToken(tokens, &rawtok, macros, files, outputList)) {
            output.clear();
            return;
        }

        if (hash || hashhash) {
            std::string s;
            for (const Token *hashtok = tokens.cfront(); hashtok; hashtok = hashtok->next)
                s += hashtok->str;
            if (hash)
                output.push_back(new Token('\"' + s + '\"', loc));
            else if (output.back())
                output.back()->setstr(output.cback()->str + s);
            else
                output.push_back(new Token(s, loc));
        } else {
            output.takeTokens(tokens);
        }
    }

    if (macroUsage) {
        for (std::map<TokenString, simplecpp::Macro>::const_iterator macroIt = macros.begin(); macroIt != macros.end(); ++macroIt) {
            const Macro &macro = macroIt->second;
            const std::list<Location> &usage = macro.usage();
            for (std::list<Location>::const_iterator usageIt = usage.begin(); usageIt != usage.end(); ++usageIt) {
                MacroUsage mu(usageIt->files);
                mu.macroName = macro.name();
                mu.macroLocation = macro.defineLocation();
                mu.useLocation = *usageIt;
                macroUsage->push_back(mu);
            }
        }
    }
}

void simplecpp::cleanup(std::map<std::string, TokenList*> &filedata)
{
    for (std::map<std::string, TokenList*>::iterator it = filedata.begin(); it != filedata.end(); ++it)
        delete it->second;
    filedata.clear();
}
