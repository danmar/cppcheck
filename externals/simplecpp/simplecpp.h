/*
 * simplecpp - A simple and high-fidelity C/C++ preprocessor library
 * Copyright (C) 2016-2022 Daniel Marjamäki.
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
#include <cstring>
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

#if (__cplusplus < 201103L) && !defined(__APPLE__)
#define nullptr NULL
#endif

namespace simplecpp {

    typedef std::string TokenString;
    class Macro;

    /**
     * Location in source code
     */
    class SIMPLECPP_LIB Location {
    public:
        explicit Location(const std::vector<std::string> &f) : files(f), fileIndex(0), line(1U), col(0U) {}

        Location(const Location &loc) : files(loc.files), fileIndex(loc.fileIndex), line(loc.line), col(loc.col) {}

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
            return fileIndex < files.size() ? files[fileIndex] : emptyFileName;
        }

        const std::vector<std::string> &files;
        unsigned int fileIndex;
        unsigned int line;
        unsigned int col;
    private:
        static const std::string emptyFileName;
    };

    /**
     * token class.
     * @todo don't use std::string representation - for both memory and performance reasons
     */
    class SIMPLECPP_LIB Token {
    public:
        Token(const TokenString &s, const Location &loc) :
            location(loc), previous(nullptr), next(nullptr), string(s) {
            flags();
        }

        Token(const Token &tok) :
            macro(tok.macro), op(tok.op), comment(tok.comment), name(tok.name), number(tok.number), location(tok.location), previous(nullptr), next(nullptr), string(tok.string), mExpandedFrom(tok.mExpandedFrom) {
        }

        void flags() {
            name = (std::isalpha(static_cast<unsigned char>(string[0])) || string[0] == '_' || string[0] == '$')
                   && (std::memchr(string.c_str(), '\'', string.size()) == nullptr);
            comment = string.size() > 1U && string[0] == '/' && (string[1] == '/' || string[1] == '*');
            number = std::isdigit(static_cast<unsigned char>(string[0])) || (string.size() > 1U && (string[0] == '-' || string[0] == '+') && std::isdigit(static_cast<unsigned char>(string[1])));
            op = (string.size() == 1U && !name && !comment && !number) ? string[0] : '\0';
        }

        const TokenString& str() const {
            return string;
        }
        void setstr(const std::string &s) {
            string = s;
            flags();
        }

        bool isOneOf(const char ops[]) const;
        bool startsWithOneOf(const char c[]) const;
        bool endsWithOneOf(const char c[]) const;

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

        void setExpandedFrom(const Token *tok, const Macro* m) {
            mExpandedFrom = tok->mExpandedFrom;
            mExpandedFrom.insert(m);
        }
        bool isExpandedFrom(const Macro* m) const {
            return mExpandedFrom.find(m) != mExpandedFrom.end();
        }

        void printAll() const;
        void printOut() const;
    private:
        TokenString string;

        std::set<const Macro*> mExpandedFrom;

        // Not implemented - prevent assignment
        Token &operator=(const Token &tok);
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
            PORTABILITY_BACKSLASH,
            UNHANDLED_CHAR_ERROR,
            EXPLICIT_INCLUDE_NOT_FOUND
        } type;
        Location location;
        std::string msg;
    };

    typedef std::list<Output> OutputList;

    /** List of tokens. */
    class SIMPLECPP_LIB TokenList {
    public:
        class Stream;

        explicit TokenList(std::vector<std::string> &filenames);
        /** generates a token list from the given std::istream parameter */
        TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr);
        /** generates a token list from the given filename parameter */
        TokenList(const std::string &filename, std::vector<std::string> &filenames, OutputList *outputList = nullptr);
        TokenList(const TokenList &other);
#if __cplusplus >= 201103L
        TokenList(TokenList &&other);
#endif
        ~TokenList();
        TokenList &operator=(const TokenList &other);
#if __cplusplus >= 201103L
        TokenList &operator=(TokenList &&other);
#endif

        void clear();
        bool empty() const {
            return !frontToken;
        }
        void push_back(Token *tok);

        void dump() const;
        std::string stringify() const;

        void readfile(Stream &stream, const std::string &filename=std::string(), OutputList *outputList = nullptr);
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
            Token * const prev = tok->previous;
            Token * const next = tok->next;
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
            other.frontToken = other.backToken = nullptr;
        }

        /** sizeof(T) */
        std::map<std::string, std::size_t> sizeOfType;

    private:
        void combineOperators();

        void constFoldUnaryNotPosNeg(Token *tok);
        void constFoldMulDivRem(Token *tok);
        void constFoldAddSub(Token *tok);
        void constFoldShift(Token *tok);
        void constFoldComparison(Token *tok);
        void constFoldBitwise(Token *tok);
        void constFoldLogicalOp(Token *tok);
        void constFoldQuestionOp(Token **tok1);

        std::string readUntil(Stream &stream, const Location &location, char start, char end, OutputList *outputList);
        void lineDirective(unsigned int fileIndex, unsigned int line, Location *location);

        std::string lastLine(int maxsize=100000) const;
        bool isLastLinePreprocessor(int maxsize=100000) const;

        unsigned int fileIndex(const std::string &filename);

        Token *frontToken;
        Token *backToken;
        std::vector<std::string> &files;
    };

    /** Tracking how macros are used */
    struct SIMPLECPP_LIB MacroUsage {
        explicit MacroUsage(const std::vector<std::string> &f, bool macroValueKnown_) : macroLocation(f), useLocation(f), macroValueKnown(macroValueKnown_) {}
        std::string macroName;
        Location    macroLocation;
        Location    useLocation;
        bool        macroValueKnown;
    };

    /** Tracking #if/#elif expressions */
    struct SIMPLECPP_LIB IfCond {
        explicit IfCond(const Location& location, const std::string &E, long long result) : location(location), E(E), result(result) {}
        Location location; // location of #if/#elif
        std::string E; // preprocessed condition
        long long result; // condition result
    };

    /**
     * Command line preprocessor settings.
     * On the command line these are configured by -D, -U, -I, --include, -std
     */
    struct SIMPLECPP_LIB DUI {
        DUI() : clearIncludeCache(false) {}
        std::list<std::string> defines;
        std::set<std::string> undefined;
        std::list<std::string> includePaths;
        std::list<std::string> includes;
        std::string std;
        bool clearIncludeCache;
    };

    SIMPLECPP_LIB long long characterLiteralToLL(const std::string& str);

    SIMPLECPP_LIB std::map<std::string, TokenList*> load(const TokenList &rawtokens, std::vector<std::string> &filenames, const DUI &dui, OutputList *outputList = nullptr);

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
     * @param ifCond output: #if/#elif expressions
     */
    SIMPLECPP_LIB void preprocess(TokenList &output, const TokenList &rawtokens, std::vector<std::string> &files, std::map<std::string, TokenList*> &filedata, const DUI &dui, OutputList *outputList = nullptr, std::list<MacroUsage> *macroUsage = nullptr, std::list<IfCond> *ifCond = nullptr);

    /**
     * Deallocate data
     */
    SIMPLECPP_LIB void cleanup(std::map<std::string, TokenList*> &filedata);

    /** Simplify path */
    SIMPLECPP_LIB std::string simplifyPath(std::string path);

    /** Convert Cygwin path to Windows path */
    SIMPLECPP_LIB std::string convertCygwinToWindowsPath(const std::string &cygwinPath);

    /** Returns the __STDC_VERSION__ value for a given standard */
    SIMPLECPP_LIB std::string getCStdString(const std::string &std);

    /** Returns the __cplusplus value for a given standard */
    SIMPLECPP_LIB std::string getCppStdString(const std::string &std);
}

#if (__cplusplus < 201103L) && !defined(__APPLE__)
#undef nullptr
#endif

#endif
