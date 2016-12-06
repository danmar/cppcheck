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

#include <cctype>
#include <istream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>


#ifdef _WIN32
#  ifdef SIMPLECPP_EXPORT
#    define SIMPLECPP_LIB __declspec(dllexport)
#  elif defined(SIMPLECPP_IMPORT)
#    define SIMPLECPP_LIB __declspec(dllimport)
#  else
#    define SIMPLECPP_LIB
#  endif
#else
#  define SIMPLECPP_LIB
#endif


namespace simplecpp {

    typedef std::string TokenString;

    /**
     * Location in source code
     */
    class SIMPLECPP_LIB Location {
    public:
        explicit Location(const std::vector<std::string> &f) : files(f), fileIndex(0), line(1U), col(0U) {}

        Location &operator=(const Location &other) {
            if (this != &other) {
                fileIndex = other.fileIndex;
                line = other.line;
                col  = other.col;
            }
            return *this;
        }

        /** increment this location by string */
        void adjust(const std::string &str);

        bool operator<(const Location &rhs) const {
            if (fileIndex != rhs.fileIndex)
                return fileIndex < rhs.fileIndex;
            if (line != rhs.line)
                return line < rhs.line;
            return col < rhs.col;
        }

        bool sameline(const Location &other) const {
            return fileIndex == other.fileIndex && line == other.line;
        }

        const std::string& file() const {
            static const std::string temp;
            return fileIndex < files.size() ? files[fileIndex] : temp;
        }

        const std::vector<std::string> &files;
        unsigned int fileIndex;
        unsigned int line;
        unsigned int col;
    };

    /**
     * token class.
     * @todo don't use std::string representation - for both memory and performance reasons
     */
    class SIMPLECPP_LIB Token {
    public:
        Token(const TokenString &s, const Location &loc) :
            str(string), location(loc), previous(NULL), next(NULL), string(s) {
            flags();
        }

        Token(const Token &tok) :
            str(string), macro(tok.macro), location(tok.location), previous(NULL), next(NULL), string(tok.str) {
            flags();
        }

        void flags() {
            name = (std::isalpha((unsigned char)str[0]) || str[0] == '_' || str[0] == '$');
            comment = (str.compare(0, 2, "//") == 0 || str.compare(0, 2, "/*") == 0);
            number = std::isdigit((unsigned char)str[0]) || (str.size() > 1U && str[0] == '-' && std::isdigit((unsigned char)str[1]));
            op = (str.size() == 1U) ? str[0] : '\0';
        }

        void setstr(const std::string &s) {
            string = s;
            flags();
        }

        bool isOneOf(const char ops[]) const;
        bool startsWithOneOf(const char c[]) const;
        bool endsWithOneOf(const char c[]) const;

        const TokenString &str;
        TokenString macro;
        char op;
        bool comment;
        bool name;
        bool number;
        Location location;
        Token *previous;
        Token *next;

        const Token *previousSkipComments() const {
            const Token *tok = this->previous;
            while (tok && tok->comment)
                tok = tok->previous;
            return tok;
        }

        const Token *nextSkipComments() const {
            const Token *tok = this->next;
            while (tok && tok->comment)
                tok = tok->next;
            return tok;
        }

        void printAll() const;
        void printOut() const;
    private:
        TokenString string;
    };

    /** Output from preprocessor */
    struct SIMPLECPP_LIB Output {
        explicit Output(const std::vector<std::string> &files) : type(ERROR), location(files) {}
        enum Type {
            ERROR, /* #error */
            WARNING, /* #warning */
            MISSING_HEADER,
            INCLUDE_NESTED_TOO_DEEPLY,
            SYNTAX_ERROR,
            PORTABILITY_BACKSLASH
        } type;
        Location location;
        std::string msg;
    };

    typedef std::list<Output> OutputList;

    /** List of tokens. */
    class SIMPLECPP_LIB TokenList {
    public:
        explicit TokenList(std::vector<std::string> &filenames);
        TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = 0);
        TokenList(const TokenList &other);
        ~TokenList();
        void operator=(const TokenList &other);

        void clear();
        bool empty() const {
            return !frontToken;
        }
        void push_back(Token *token);

        void dump() const;
        std::string stringify() const;

        void readfile(std::istream &istr, const std::string &filename=std::string(), OutputList *outputList = 0);
        void constFold();

        void removeComments();

        Token *front() {
            return frontToken;
        }

        const Token *cfront() const {
            return frontToken;
        }

        Token *back() {
            return backToken;
        }

        const Token *cback() const {
            return backToken;
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
            if (frontToken == tok)
                frontToken = next;
            if (backToken == tok)
                backToken = prev;
            delete tok;
        }

        void takeTokens(TokenList &other) {
            if (!other.frontToken)
                return;
            if (!frontToken) {
                frontToken = other.frontToken;
            } else {
                backToken->next = other.frontToken;
                other.frontToken->previous = backToken;
            }
            backToken = other.backToken;
            other.frontToken = other.backToken = NULL;
        }

        /** sizeof(T) */
        std::map<std::string, std::size_t> sizeOfType;

    private:
        void combineOperators();

        void constFoldUnaryNotPosNeg(Token *tok);
        void constFoldMulDivRem(Token *tok);
        void constFoldAddSub(Token *tok);
        void constFoldComparison(Token *tok);
        void constFoldBitwise(Token *tok);
        void constFoldLogicalOp(Token *tok);
        void constFoldQuestionOp(Token **tok);

        std::string readUntil(std::istream &istr, const Location &location, const char start, const char end, OutputList *outputList);

        std::string lastLine(int maxsize=10) const;

        unsigned int fileIndex(const std::string &filename);

        Token *frontToken;
        Token *backToken;
        std::vector<std::string> &files;
    };

    /** Tracking how macros are used */
    struct SIMPLECPP_LIB MacroUsage {
        explicit MacroUsage(const std::vector<std::string> &f) : macroLocation(f), useLocation(f) {}
        std::string macroName;
        Location    macroLocation;
        Location    useLocation;
    };

    struct SIMPLECPP_LIB DUI {
        std::list<std::string> defines;
        std::set<std::string> undefined;
        std::list<std::string> includePaths;
        std::list<std::string> includes;
    };

    SIMPLECPP_LIB std::map<std::string, TokenList*> load(const TokenList &rawtokens, std::vector<std::string> &filenames, const DUI &dui, OutputList *outputList = 0);

    /**
     * Preprocess
     * @todo simplify interface
     * @param output TokenList that receives the preprocessing output
     * @param rawtokens Raw tokenlist for top sourcefile
     * @param files internal data of simplecpp
     * @param filedata output from simplecpp::load()
     * @param dui defines, undefs, and include paths
     * @param outputList output: list that will receive output messages
     * @param macroUsage output: macro usage
     */
    SIMPLECPP_LIB void preprocess(TokenList &output, const TokenList &rawtokens, std::vector<std::string> &files, std::map<std::string, TokenList*> &filedata, const DUI &dui, OutputList *outputList = 0, std::list<MacroUsage> *macroUsage = 0);

    /**
     * Deallocate data
     */
    SIMPLECPP_LIB void cleanup(std::map<std::string, TokenList*> &filedata);
}

#endif
