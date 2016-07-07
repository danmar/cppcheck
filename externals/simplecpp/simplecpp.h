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

#ifndef simplecppH
#define simplecppH

#include <istream>
#include <string>
#include <map>
#include <cctype>
#include <list>

namespace simplecpp {

typedef std::string TokenString;

/**
 * Location in source code
 */
class Location {
public:
    Location() : line(1U), col(0U) {}

    std::string file;
    unsigned int line;
    unsigned int col;

    Location &operator=(const Location &other) {
        if (this != &other) {
            file = other.file;
            line = other.line;
            col  = other.col;
        }
        return *this;
    }

    /** increment this location by string */
    void adjust(const std::string &str);

    bool operator<(const Location &rhs) const {
        if (file != rhs.file)
            return file < rhs.file;
        if (line != rhs.line)
            return line < rhs.line;
        return col < rhs.col;
    }
};

/**
 * token class.
 * @todo don't use std::string representation - for both memory and performance reasons
 */
class Token {
public:
    Token(const TokenString &s, const Location &location) :
        str(string), location(location), previous(nullptr), next(nullptr), string(s)
    {
        flags();
    }

    Token(const Token &tok) :
        str(string), macro(tok.macro), location(tok.location), previous(nullptr), next(nullptr), string(tok.str)
    {
        flags();
    }

    void flags() {
        name = (str[0] == '_' || std::isalpha(str[0]));
        comment = (str.compare(0, 2, "//") == 0 || str.compare(0, 2, "/*") == 0);
        number = std::isdigit(str[0]) || (str.size() > 1U && str[0] == '-' && std::isdigit(str[1]));
        op = (str.size() == 1U) ? str[0] : '\0';
    }

    void setstr(const std::string &s) {
        string = s;
        flags();
    }

    bool isOneOf(const char ops[]) const;
    bool startsWithOneOf(const char c[]) const;
    bool endsWithOneOf(const char c[]) const;

    char op;
    const TokenString &str;
    TokenString macro;
    bool comment;
    bool name;
    bool number;
    Location location;
    Token *previous;
    Token *next;
private:
    TokenString string;
};

/** Output from preprocessor */
struct Output {
    enum Type {
        ERROR, /* error */
        WARNING /* warning */
    } type;
    Location location;
    std::string msg;
};

typedef std::list<struct Output> OutputList;

/** List of tokens. */
class TokenList {
public:
    TokenList();
    TokenList(std::istringstream &istr, const std::string &filename=std::string(), OutputList *outputList = 0);
    TokenList(const TokenList &other);
    ~TokenList();
    void operator=(const TokenList &other);

    void clear();
    bool empty() const {
        return !cbegin();
    }
    void push_back(Token *token);

    void dump() const;
    std::string stringify() const;

    void readfile(std::istream &istr, const std::string &filename=std::string(), OutputList *outputList = 0);
    void constFold();

    Token *begin() {
        return first;
    }

    const Token *cbegin() const {
        return first;
    }

    Token *end() {
        return last;
    }

    const Token *cend() const {
        return last;
    }

    void deleteToken(Token *tok) {
        if (!tok)
            return;
        Token *prev = tok->previous;
        Token *next = tok->next;
        if (prev)
            prev->next = next;
        if (next)
            next->previous = prev;
        if (first == tok)
            first = next;
        if (last == tok)
            last = prev;
        delete tok;
    }

private:
    void combineOperators();

    void constFoldUnaryNotPosNeg(Token *tok);
    void constFoldMulDivRem(Token *tok);
    void constFoldAddSub(Token *tok);
    void constFoldComparison(Token *tok);
    void constFoldBitwise(Token *tok);
    void constFoldLogicalOp(Token *tok);
    void constFoldQuestionOp(Token **tok);

    std::string lastLine() const;

    Token *first;
    Token *last;
};

/** Tracking how macros are used */
struct MacroUsage {
    std::string macroName;
    Location    macroLocation;
    Location    useLocation;
};

typedef std::map<std::string, std::string> Defines;

/**
 * Preprocess
 *
 * Preprocessing is done in two steps currently:
 *   const simplecpp::TokenList tokens1 = simplecpp::TokenList(f);
 *   const simplecpp::TokenList tokens2 = simplecpp::preprocess(tokens1, defines);
 *
 * The "tokens1" will contain tokens for comments and for preprocessor directives. And there is no preprocessing done.
 * This "tokens1" can be used if you need to see what comments/directives there are. Or what code is hidden in #if.
 *
 * The "tokens2" will have normal preprocessor output. No comments nor directives are seen.
 *
 * @todo simplify interface
 */
TokenList preprocess(const TokenList &rawtokens, const Defines &defines, OutputList *outputList = 0, std::list<struct MacroUsage> *macroUsage = 0);
}

#endif
