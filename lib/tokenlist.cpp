/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

//---------------------------------------------------------------------------
#include "tokenlist.h"
#include "token.h"
#include "mathlib.h"
#include "path.h"
#include "preprocessor.h"
#include "settings.h"
#include "errorlogger.h"

#include <cstring>
#include <sstream>
#include <cctype>
#include <stack>

// How many compileExpression recursions are allowed?
// For practical code this could be endless. But in some special torture test
// there needs to be a limit.
static const unsigned int AST_MAX_DEPTH = 50U;


TokenList::TokenList(const Settings* settings) :
    _front(0),
    _back(0),
    _settings(settings),
    _isC(false),
    _isCPP(false)
{
}

TokenList::~TokenList()
{
    deallocateTokens();
}

//---------------------------------------------------------------------------

const std::string& TokenList::getSourceFilePath() const
{
    if (getFiles().empty()) {
        return emptyString;
    }
    return getFiles()[0];
}

//---------------------------------------------------------------------------

// Deallocate lists..
void TokenList::deallocateTokens()
{
    deleteTokens(_front);
    _front = 0;
    _back = 0;
    _files.clear();
}

unsigned int TokenList::appendFileIfNew(const std::string &fileName)
{
    // Has this file been tokenized already?
    for (std::size_t i = 0; i < _files.size(); ++i)
        if (Path::sameFileName(_files[i], fileName))
            return (unsigned int)i;

    // The "_files" vector remembers what files have been tokenized..
    _files.push_back(Path::simplifyPath(fileName));

    // Update _isC and _isCPP properties
    if (_files.size() == 1) { // Update only useful if first file added to _files
        if (!_settings) {
            _isC = Path::isC(getSourceFilePath());
            _isCPP = Path::isCPP(getSourceFilePath());
        } else {
            _isC = _settings->enforcedLang == Settings::C || (_settings->enforcedLang == Settings::None && Path::isC(getSourceFilePath()));
            _isCPP = _settings->enforcedLang == Settings::CPP || (_settings->enforcedLang == Settings::None && Path::isCPP(getSourceFilePath()));
        }
    }
    return _files.size() - 1U;
}

void TokenList::deleteTokens(Token *tok)
{
    while (tok) {
        Token *next = tok->next();
        delete tok;
        tok = next;
    }
}

//---------------------------------------------------------------------------
// add a token.
//---------------------------------------------------------------------------

void TokenList::addtoken(std::string str, const unsigned int lineno, const unsigned int fileno, bool split)
{
    if (str.empty())
        return;

    // If token contains # characters, split it up
    if (split) {
        size_t begin = 0;
        size_t end = 0;
        while ((end = str.find("##", begin)) != std::string::npos) {
            addtoken(str.substr(begin, end - begin), lineno, fileno, false);
            addtoken("##", lineno, fileno, false);
            begin = end+2;
        }
        if (begin != 0) {
            addtoken(str.substr(begin), lineno, fileno, false);
            return;
        }
    }

    // Replace hexadecimal value with decimal
    if (MathLib::isIntHex(str) || MathLib::isOct(str) || MathLib::isBin(str)) {
        // TODO: It would be better if TokenList didn't simplify hexadecimal numbers
        std::string suffix;
        if (str.compare(0,2,"0x") == 0 &&
            str.size() == (2 + _settings->int_bit / 4) &&
            (str[2] >= '8'))  // includes A-F and a-f
            suffix = "U";
        str = MathLib::value(str).str() + suffix;
    }

    if (_back) {
        _back->insertToken(str);
    } else {
        _front = new Token(&_back);
        _back = _front;
        _back->str(str);
    }

    if (isCPP() && str == "delete")
        _back->isKeyword(true);
    _back->linenr(lineno);
    _back->fileIndex(fileno);
}

void TokenList::addtoken(const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    if (tok == nullptr)
        return;

    if (_back) {
        _back->insertToken(tok->str(), tok->originalName());
    } else {
        _front = new Token(&_back);
        _back = _front;
        _back->str(tok->str());
        if (!tok->originalName().empty())
            _back->originalName(tok->originalName());
    }

    _back->linenr(lineno);
    _back->fileIndex(fileno);
    _back->flags(tok->flags());
}
//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void TokenList::insertTokens(Token *dest, const Token *src, unsigned int n)
{
    std::stack<Token *> link;

    while (n > 0) {
        dest->insertToken(src->str(), src->originalName());
        dest = dest->next();

        // Set links
        if (Token::Match(dest, "(|[|{"))
            link.push(dest);
        else if (!link.empty() && Token::Match(dest, ")|]|}")) {
            Token::createMutualLinks(dest, link.top());
            link.pop();
        }

        dest->fileIndex(src->fileIndex());
        dest->linenr(src->linenr());
        dest->varId(src->varId());
        dest->tokType(src->tokType());
        dest->flags(src->flags());
        src  = src->next();
        --n;
    }
}

//---------------------------------------------------------------------------
// Tokenize - tokenizes a given file.
//---------------------------------------------------------------------------

bool TokenList::createTokens(std::istream &code, const std::string& file0)
{
    appendFileIfNew(file0);

    // line number in parsed code
    unsigned int lineno = 1;

    // The current token being parsed
    std::string CurrentToken;

    // lineNumbers holds line numbers for files in fileIndexes
    // every time an include file is completely parsed, last item in the vector
    // is removed and lineno is set to point to that value.
    std::stack<unsigned int> lineNumbers;

    // fileIndexes holds index for _files vector about currently parsed files
    // every time an include file is completely parsed, last item in the vector
    // is removed and FileIndex is set to point to that value.
    std::stack<unsigned int> fileIndexes;

    // FileIndex. What file in the _files vector is read now?
    unsigned int FileIndex = 0;

    bool expandedMacro = false;

    // Read one byte at a time from code and create tokens
    for (char ch = (char)code.get(); code.good() && ch; ch = (char)code.get()) {
        if (ch == Preprocessor::macroChar) {
            while (code.peek() == Preprocessor::macroChar)
                code.get();
            if (!CurrentToken.empty()) {
                addtoken(CurrentToken, lineno, FileIndex, true);
                _back->isExpandedMacro(expandedMacro);
                CurrentToken.clear();
            }
            expandedMacro = true;
            continue;
        }

        // char/string..
        // multiline strings are not handled. The preprocessor should handle that for us.
        else if (ch == '\'' || ch == '\"') {
            std::string line;

            // read char
            bool special = false;
            char c = ch;
            do {
                // Append token..
                line += c;

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)code.get();
            } while (code.good() && (special || c != ch));
            line += ch;

            // Handle #file "file.h"
            if (CurrentToken == "#file") {
                // Extract the filename
                line = line.substr(1, line.length() - 2);

                ++lineno;
                fileIndexes.push(FileIndex);
                FileIndex = appendFileIfNew(line);
                lineNumbers.push(lineno);
                lineno = 0;
            } else {
                // Add previous token
                addtoken(CurrentToken, lineno, FileIndex);
                if (!CurrentToken.empty())
                    _back->isExpandedMacro(expandedMacro);

                // Add content of the string
                addtoken(line, lineno, FileIndex);
                if (!line.empty())
                    _back->isExpandedMacro(expandedMacro);
            }

            CurrentToken.clear();

            continue;
        }

        if (ch == '.' &&
            !CurrentToken.empty() &&
            std::isdigit((unsigned char)CurrentToken[0])) {
            // Don't separate doubles "5.4"
        } else if (std::strchr("+-", ch) &&
                   CurrentToken.length() > 0 &&
                   std::isdigit((unsigned char)CurrentToken[0]) &&
                   (CurrentToken.back() == 'e' ||
                    CurrentToken.back() == 'E') &&
                   !MathLib::isIntHex(CurrentToken)) {
            // Don't separate doubles "4.2e+10"
        } else if (CurrentToken.empty() && ch == '.' && std::isdigit((unsigned char)code.peek())) {
            // tokenize .125 into 0.125
            CurrentToken = "0";
        } else if (std::strchr("+-*/%&|^?!=<>[](){};:,.~\n ", ch)) {
            if (CurrentToken == "#file") {
                // Handle this where strings are handled
                continue;
            } else if (CurrentToken == "#line") {
                // Read to end of line
                std::string line;

                std::getline(code, line);

                unsigned int row=0;
                std::istringstream fiss(line);
                if (fiss >> row) {
                    // Update the current line number
                    lineno = row;

                    std::string line2;
                    if (std::getline(fiss, line2) && line2.length() > 4U) {
                        // _"file_name" -> file_name
                        line2 = line2.substr(2, line2.length() - 3);

                        // Update the current file
                        FileIndex = appendFileIfNew(line2);
                    }
                } else
                    ++lineno;
                CurrentToken.clear();
                continue;
            } else if (CurrentToken == "#endfile") {
                if (lineNumbers.empty() || fileIndexes.empty()) { // error
                    deallocateTokens();
                    return false;
                }

                lineno = lineNumbers.top();
                lineNumbers.pop();
                FileIndex = fileIndexes.top();
                fileIndexes.pop();
                CurrentToken.clear();
                continue;
            }

            addtoken(CurrentToken, lineno, FileIndex, true);
            if (!CurrentToken.empty()) {
                _back->isExpandedMacro(expandedMacro);
                expandedMacro = false;
                CurrentToken.clear();
            }

            if (ch == '\n') {
                if (_settings->terminated())
                    return false;

                ++lineno;
                continue;
            } else if (ch == ' ') {
                continue;
            }

            CurrentToken += ch;
            // Add "++", "--", ">>" or ... token
            if (std::strchr("+-<>=:&|", ch) && (code.peek() == ch))
                CurrentToken += (char)code.get();
            addtoken(CurrentToken, lineno, FileIndex);
            _back->isExpandedMacro(expandedMacro);
            CurrentToken.clear();
            expandedMacro = false;
            continue;
        }

        CurrentToken += ch;
    }
    addtoken(CurrentToken, lineno, FileIndex, true);
    if (!CurrentToken.empty())
        _back->isExpandedMacro(expandedMacro);

    // Split up ++ and --..
    for (Token *tok = _front; tok; tok = tok->next()) {
        if (!Token::Match(tok, "++|--"))
            continue;
        if (Token::Match(tok->previous(), "%num% ++|--") ||
            Token::Match(tok, "++|-- %num%")) {
            tok->str(tok->str()[0]);
            tok->insertToken(tok->str());
        }
    }

    Token::assignProgressValues(_front);

    for (std::size_t i = 1; i < _files.size(); i++)
        _files[i] = Path::getRelativePath(_files[i], _settings->basePaths);

    return true;
}

//---------------------------------------------------------------------------

unsigned long long TokenList::calculateChecksum() const
{
    unsigned long long checksum = 0;
    for (const Token* tok = front(); tok; tok = tok->next()) {
        const unsigned int subchecksum1 = tok->flags() + tok->varId() + tok->tokType();
        unsigned int subchecksum2 = 0;
        for (std::size_t i = 0; i < tok->str().size(); i++)
            subchecksum2 += (unsigned int)tok->str()[i];
        if (!tok->originalName().empty()) {
            for (std::size_t i = 0; i < tok->originalName().size(); i++)
                subchecksum2 += (unsigned int) tok->originalName()[i];
        }

        checksum ^= ((static_cast<unsigned long long>(subchecksum1) << 32) | subchecksum2);

        const bool bit1 = (checksum & 1) != 0;
        checksum >>= 1;
        if (bit1)
            checksum |= (1ULL << 63);
    }
    return checksum;
}


//---------------------------------------------------------------------------

struct AST_state {
    std::stack<Token*> op;
    unsigned int depth;
    unsigned int inArrayAssignment;
    bool cpp;
    unsigned int assign;
    explicit AST_state(bool cpp_) : depth(0), inArrayAssignment(0), cpp(cpp_), assign(0U) {}
};

static Token * skipDecl(Token *tok)
{
    if (!Token::Match(tok->previous(), "( %name%"))
        return tok;

    Token *vartok = tok;
    while (Token::Match(vartok, "%name%|*|&|::|<")) {
        if (vartok->str() == "<") {
            if (vartok->link())
                vartok = vartok->link();
            else
                return tok;
        } else if (Token::Match(vartok, "%name% [:=]")) {
            return vartok;
        }
        vartok = vartok->next();
    }
    return tok;
}

static bool iscast(const Token *tok)
{
    if (!Token::Match(tok, "( ::| %name%"))
        return false;

    if (tok->previous() && tok->previous()->isName() && tok->previous()->str() != "return")
        return false;

    if (Token::simpleMatch(tok->previous(), ">") && tok->previous()->link())
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    bool type = false;
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        while (tok2->link() && Token::Match(tok2, "(|[|<"))
            tok2 = tok2->link()->next();

        if (tok2->str() == ")")
            return type || tok2->strAt(-1) == "*" || Token::Match(tok2, ") &|~") ||
                   (Token::Match(tok2, ") %any%") &&
                    !tok2->next()->isOp() &&
                    !Token::Match(tok2->next(), "[[]);,?:.]"));
        if (!Token::Match(tok2, "%name%|*|&|::"))
            return false;

        if (tok2->isStandardType() && tok2->next()->str() != "(")
            type = true;
    }

    return false;
}

// X{} X<Y>{} etc
static bool iscpp11init(const Token * const tok)
{
    const Token *nameToken = nullptr;
    if (tok->isName())
        nameToken = tok;
    else if (Token::Match(tok->previous(), "%name% {"))
        nameToken = tok->previous();
    else if (tok->linkAt(-1) && Token::simpleMatch(tok->previous(), "> {") && Token::Match(tok->linkAt(-1)->previous(),"%name% <"))
        nameToken = tok->linkAt(-1)->previous();
    if (!nameToken)
        return false;

    const Token *endtok = nullptr;
    if (Token::Match(nameToken,"%name% {"))
        endtok = nameToken->linkAt(1);
    else if (Token::Match(nameToken,"%name% <") && Token::simpleMatch(nameToken->linkAt(1),"> {"))
        endtok = nameToken->linkAt(1)->linkAt(1);
    else
        return false;
    // There is no initialisation for example here: 'class Fred {};'
    if (!Token::simpleMatch(endtok, "} ;"))
        return true;
    const Token *prev = nameToken;
    while (Token::Match(prev, "%name%|::|:|<|>")) {
        if (Token::Match(prev, "class|struct"))
            return false;

        prev = prev->previous();
    }
    return true;
}

static void compileUnaryOp(Token *&tok, AST_state& state, void(*f)(Token *&tok, AST_state& state))
{
    Token *unaryop = tok;
    if (f) {
        tok = tok->next();
        state.depth++;
        if (tok && state.depth <= AST_MAX_DEPTH)
            f(tok, state);
        state.depth--;
    }

    if (!state.op.empty()) {
        unaryop->astOperand1(state.op.top());
        state.op.pop();
    }
    state.op.push(unaryop);
}

static void compileBinOp(Token *&tok, AST_state& state, void(*f)(Token *&tok, AST_state& state))
{
    Token *binop = tok;
    if (f) {
        tok = tok->next();
        state.depth++;
        if (tok && state.depth <= AST_MAX_DEPTH)
            f(tok, state);
        state.depth--;
    }

    // TODO: Should we check if op is empty.
    // * Is it better to add assertion that it isn't?
    // * Write debug warning if it's empty?
    if (!state.op.empty()) {
        binop->astOperand2(state.op.top());
        state.op.pop();
    }
    if (!state.op.empty()) {
        binop->astOperand1(state.op.top());
        state.op.pop();
    }
    state.op.push(binop);
}

static void compileExpression(Token *&tok, AST_state& state);

static void compileTerm(Token *&tok, AST_state& state)
{
    if (!tok)
        return;
    if (Token::Match(tok, "L %str%|%char%"))
        tok = tok->next();
    if (state.inArrayAssignment && tok->str() == "." && Token::Match(tok->previous(), ",|{")) // Jump over . in C style struct initialization
        tok = tok->next();

    if (tok->isLiteral()) {
        state.op.push(tok);
        do {
            tok = tok->next();
        } while (Token::Match(tok, "%name%|%str%"));
    } else if (tok->isName() && tok->str() != "case") {
        if (tok->str() == "return") {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (Token::Match(tok, "sizeof !!(")) {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (state.cpp && iscpp11init(tok)) { // X{} X<Y>{} etc
            state.op.push(tok);
            tok = tok->next();
            if (tok->str() == "<")
                tok = tok->link()->next();
        } else if (!state.cpp || !Token::Match(tok, "new|delete %name%|*|&|::|(|[")) {
            tok = skipDecl(tok);
            while (tok->next() && tok->next()->isName())
                tok = tok->next();
            state.op.push(tok);
            if (Token::Match(tok, "%name% <") && tok->linkAt(1))
                tok = tok->linkAt(1);
            tok = tok->next();
            if (Token::Match(tok, "%str%")) {
                while (Token::Match(tok, "%name%|%str%"))
                    tok = tok->next();
            }
        }
    } else if (tok->str() == "{") {
        if (tok->previous() && tok->previous()->isName()) {
            compileBinOp(tok, state, compileExpression);
        } else if (!state.inArrayAssignment && tok->strAt(-1) != "=") {
            state.op.push(tok);
            tok = tok->link()->next();
        } else {
            if (tok->link() != tok->next()) {
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                while (Token::Match(tok, "} [,}]") && state.inArrayAssignment > 0U) {
                    tok = tok->next();
                    state.inArrayAssignment--;
                }
            } else {
                state.op.push(tok);
            }
        }
    }
}

static void compileScope(Token *&tok, AST_state& state)
{
    compileTerm(tok, state);
    while (tok) {
        if (tok->str() == "::") {
            Token *binop = tok;
            tok = tok->next();
            if (tok && tok->str() == "~") // Jump over ~ of destructor definition
                tok = tok->next();
            if (tok)
                compileTerm(tok, state);

            if (binop->previous() && (binop->previous()->isName() || (binop->previous()->link() && binop->strAt(-1) == ">")))
                compileBinOp(binop, state, nullptr);
            else
                compileUnaryOp(binop, state, nullptr);
        } else break;
    }
}

static bool isPrefixUnary(const Token* tok, bool cpp)
{
    if (!tok->previous()
        || ((Token::Match(tok->previous(), "(|[|{|%op%|;|}|?|:|,|.|return|::") || (cpp && tok->strAt(-1) == "throw"))
            && (tok->previous()->tokType() != Token::eIncDecOp || tok->tokType() == Token::eIncDecOp)))
        return true;

    return tok->strAt(-1) == ")" && iscast(tok->linkAt(-1));
}

static void compilePrecedence2(Token *&tok, AST_state& state)
{
    compileScope(tok, state);
    while (tok) {
        if (tok->tokType() == Token::eIncDecOp && !isPrefixUnary(tok, state.cpp)) {
            compileUnaryOp(tok, state, compileScope);
        } else if (tok->str() == "." && tok->strAt(1) != "*") {
            if (tok->strAt(1) == ".") {
                state.op.push(tok);
                tok = tok->tokAt(3);
                break;
            } else
                compileBinOp(tok, state, compileScope);
        } else if (tok->str() == "[") {
            bool lambda = false;
            if (state.cpp && isPrefixUnary(tok, state.cpp) && tok->link()->strAt(1) == "(") { // Lambda
                // What we do here:
                // - Nest the round bracket under the square bracket.
                // - Nest what follows the lambda (if anything) with the lambda opening [
                // - Compile the content of the lambda function as separate tree (this is done later)
                // this must be consistent with isLambdaCaptureList
                Token* squareBracket = tok;
                Token* roundBracket = squareBracket->link()->next();
                Token* curlyBracket = roundBracket->link()->next();
                while (Token::Match(curlyBracket, "%name%|.|::"))
                    curlyBracket = curlyBracket->next();
                if (Token::simpleMatch(curlyBracket, "{")) {
                    lambda = true;
                    squareBracket->astOperand1(roundBracket);
                    roundBracket->astOperand1(curlyBracket);
                    state.op.push(squareBracket);
                    tok = curlyBracket->link()->next();
                }
            }
            if (!lambda) {
                Token* tok2 = tok;
                if (tok->strAt(1) != "]")
                    compileBinOp(tok, state, compileExpression);
                else
                    compileUnaryOp(tok, state, compileExpression);
                tok = tok2->link()->next();
            }
        } else if (tok->str() == "(" && (!iscast(tok) || Token::Match(tok->previous(), "if|while|for|switch|catch"))) {
            Token* tok2 = tok;
            tok = tok->next();
            const bool opPrevTopSquare = !state.op.empty() && state.op.top() && state.op.top()->str() == "[";
            const std::size_t oldOpSize = state.op.size();
            compileExpression(tok, state);
            tok = tok2;
            if ((tok->previous() && tok->previous()->isName() && (tok->strAt(-1) != "return" && (!state.cpp || !Token::Match(tok->previous(), "throw|delete"))))
                || (tok->strAt(-1) == "]" && (!state.cpp || !Token::Match(tok->linkAt(-1)->previous(), "new|delete")))
                || (tok->strAt(-1) == ">" && tok->linkAt(-1))
                || (tok->strAt(-1) == ")" && !iscast(tok->linkAt(-1))) // Don't treat brackets to clarify precedence as function calls
                || (tok->strAt(-1) == "}" && opPrevTopSquare)) {
                const bool operandInside = oldOpSize < state.op.size();
                if (operandInside)
                    compileBinOp(tok, state, nullptr);
                else
                    compileUnaryOp(tok, state, nullptr);
            }
            tok = tok->link()->next();
        } else if (state.cpp && tok->str() == "{" && iscpp11init(tok)) {
            if (Token::simpleMatch(tok, "{ }"))
                compileUnaryOp(tok, state, compileExpression);
            else
                compileBinOp(tok, state, compileExpression);
            if (Token::simpleMatch(tok, "}"))
                tok = tok->next();
        } else break;
    }
}

static void compilePrecedence3(Token *&tok, AST_state& state)
{
    compilePrecedence2(tok, state);
    while (tok) {
        if ((Token::Match(tok, "[+-!~*&]") || tok->tokType() == Token::eIncDecOp) &&
            isPrefixUnary(tok, state.cpp)) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok->next();
                while (tok2->next() && tok2->str() == "*")
                    tok2 = tok2->next();
                if (Token::Match(tok2, "[>),]")) {
                    tok = tok2;
                    continue;
                }
            }
            compileUnaryOp(tok, state, compilePrecedence3);
        } else if (tok->str() == "(" && iscast(tok)) {
            Token* tok2 = tok;
            tok = tok->link()->next();
            compilePrecedence3(tok, state);
            compileUnaryOp(tok2, state, nullptr);
        } else if (state.cpp && Token::Match(tok, "new %name%|::|(")) {
            Token* newtok = tok;
            tok = tok->next();
            bool innertype = false;
            if (tok->str() == "(") {
                if (Token::Match(tok, "( &| %name%") && Token::Match(tok->link(), ") ( %type%") && Token::simpleMatch(tok->link()->linkAt(1), ") ("))
                    tok = tok->link()->next();
                if (Token::Match(tok->link(), ") ::| %type%"))
                    tok = tok->link()->next();
                else if (Token::Match(tok, "( %type%") && Token::Match(tok->link(), ") [();,[]")) {
                    tok = tok->next();
                    innertype = true;
                } else if (Token::Match(tok, "( &| %name%") && Token::simpleMatch(tok->link(), ") (")) {
                    tok = tok->next();
                    innertype = true;
                } else {
                    /* bad code */
                    continue;
                }
            }
            state.op.push(tok);
            while (Token::Match(tok, "%name%|*|&|<|::")) {
                if (tok->link())
                    tok = tok->link();
                tok = tok->next();
            }
            if (Token::Match(tok, "( const| %type% ) (")) {
                state.op.push(tok->next());
                tok = tok->link()->next();
                compileBinOp(tok, state, compilePrecedence2);
            } else if (tok && (tok->str() == "[" || tok->str() == "(" || tok->str() == "{"))
                compilePrecedence2(tok, state);
            else if (innertype && Token::simpleMatch(tok, ") [")) {
                tok = tok->next();
                compilePrecedence2(tok, state);
            }
            compileUnaryOp(newtok, state, nullptr);
            if (innertype && Token::simpleMatch(tok, ") ,"))
                tok = tok->next();
        } else if (state.cpp && Token::Match(tok, "delete %name%|*|&|::|(|[")) {
            Token* tok2 = tok;
            tok = tok->next();
            if (tok && tok->str() == "[")
                tok = tok->link()->next();
            compilePrecedence3(tok, state);
            compileUnaryOp(tok2, state, nullptr);
        }
        // TODO: Handle sizeof
        else break;
    }
}

static void compilePointerToElem(Token *&tok, AST_state& state)
{
    compilePrecedence3(tok, state);
    while (tok) {
        if (Token::simpleMatch(tok, ". *")) {
            compileBinOp(tok, state, compilePrecedence3);
        } else break;
    }
}

static void compileMulDiv(Token *&tok, AST_state& state)
{
    compilePointerToElem(tok, state);
    while (tok) {
        if (Token::Match(tok, "[/%]") || (tok->str() == "*" && !tok->astOperand1())) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok->next();
                while (tok2->next() && tok2->str() == "*")
                    tok2 = tok2->next();
                if (Token::Match(tok2, "[>),]")) {
                    tok = tok2;
                    break;
                }
            }
            compileBinOp(tok, state, compilePointerToElem);
        } else break;
    }
}

static void compileAddSub(Token *&tok, AST_state& state)
{
    compileMulDiv(tok, state);
    while (tok) {
        if (Token::Match(tok, "+|-") && !tok->astOperand1()) {
            compileBinOp(tok, state, compileMulDiv);
        } else break;
    }
}

static void compileShift(Token *&tok, AST_state& state)
{
    compileAddSub(tok, state);
    while (tok) {
        if (Token::Match(tok, "<<|>>")) {
            compileBinOp(tok, state, compileAddSub);
        } else break;
    }
}

static void compileRelComp(Token *&tok, AST_state& state)
{
    compileShift(tok, state);
    while (tok) {
        if (Token::Match(tok, "<|<=|>=|>") && !tok->link()) {
            compileBinOp(tok, state, compileShift);
        } else break;
    }
}

static void compileEqComp(Token *&tok, AST_state& state)
{
    compileRelComp(tok, state);
    while (tok) {
        if (Token::Match(tok, "==|!=")) {
            compileBinOp(tok, state, compileRelComp);
        } else break;
    }
}

static void compileAnd(Token *&tok, AST_state& state)
{
    compileEqComp(tok, state);
    while (tok) {
        if (tok->str() == "&" && !tok->astOperand1()) {
            Token* tok2 = tok->next();
            if (!tok2)
                break;
            if (tok2->str() == "&")
                tok2 = tok2->next();
            if (state.cpp && Token::Match(tok2, ",|)")) {
                tok = tok2;
                break; // rValue reference
            }
            compileBinOp(tok, state, compileEqComp);
        } else break;
    }
}

static void compileXor(Token *&tok, AST_state& state)
{
    compileAnd(tok, state);
    while (tok) {
        if (tok->str() == "^") {
            compileBinOp(tok, state, compileAnd);
        } else break;
    }
}

static void compileOr(Token *&tok, AST_state& state)
{
    compileXor(tok, state);
    while (tok) {
        if (tok->str() == "|") {
            compileBinOp(tok, state, compileXor);
        } else break;
    }
}

static void compileLogicAnd(Token *&tok, AST_state& state)
{
    compileOr(tok, state);
    while (tok) {
        if (tok->str() == "&&") {
            compileBinOp(tok, state, compileOr);
        } else break;
    }
}

static void compileLogicOr(Token *&tok, AST_state& state)
{
    compileLogicAnd(tok, state);
    while (tok) {
        if (tok->str() == "||") {
            compileBinOp(tok, state, compileLogicAnd);
        } else break;
    }
}

static void compileAssignTernary(Token *&tok, AST_state& state)
{
    compileLogicOr(tok, state);
    while (tok) {
        if (tok->isAssignmentOp()) {
            state.assign++;
            compileBinOp(tok, state, compileAssignTernary);
            if (state.assign > 0U)
                state.assign--;
        } else if (tok->str() == "?") {
            // http://en.cppreference.com/w/cpp/language/operator_precedence says about ternary operator:
            //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
            // Hence, we rely on Tokenizer::prepareTernaryOpForAST() to add such parentheses where necessary.
            if (tok->strAt(1) == ":") {
                state.op.push(0);
            }
            const unsigned int assign = state.assign;
            state.assign = 0U;
            compileBinOp(tok, state, compileAssignTernary);
            state.assign = assign;
        } else if (tok->str() == ":") {
            if (state.assign > 0U)
                break;
            compileBinOp(tok, state, compileAssignTernary);
        } else break;
    }
}

static void compileComma(Token *&tok, AST_state& state)
{
    compileAssignTernary(tok, state);
    while (tok) {
        if (tok->str() == ",") {
            compileBinOp(tok, state, compileAssignTernary);
        } else break;
    }
}

static void compileExpression(Token *&tok, AST_state& state)
{
    if (state.depth > AST_MAX_DEPTH)
        return; // ticket #5592
    if (tok)
        compileComma(tok, state);
}

static bool isLambdaCaptureList(const Token * tok)
{
    // a lambda expression '[x](y){}' is compiled as:
    // [
    // `-(
    //   `-{
    // see compilePrecedence2
    if (tok->str() != "[")
        return false;
    if (!tok->astOperand1() || tok->astOperand1()->str() != "(")
        return false;
    const Token * params = tok->astOperand1();
    if (!params || !params->astOperand1() || params->astOperand1()->str() != "{")
        return false;
    return true;
}

static Token * createAstAtToken(Token *tok, bool cpp);

// Compile inner expressions inside inner ({..}) and lambda bodies
static void createAstAtTokenInner(Token * const tok1, const Token *endToken, bool cpp)
{
    for (Token *tok = tok1; tok && tok != endToken; tok = tok ? tok->next() : nullptr) {
        if (tok->str() == "{") {
            if (Token::simpleMatch(tok->previous(), "( {"))
                ;
            else if (Token::simpleMatch(tok->astParent(), "(") &&
                     Token::simpleMatch(tok->astParent()->astParent(), "[") &&
                     tok->astParent()->astParent()->astOperand1() &&
                     tok == tok->astParent()->astParent()->astOperand1()->astOperand1())
                ;
            else
                continue;

            if (Token::simpleMatch(tok->previous(), "( { ."))
                break;

            const Token * const endToken2 = tok->link();
            for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                tok = createAstAtToken(tok, cpp);
        } else if (tok->str() == "[") {
            if (isLambdaCaptureList(tok)) {
                const Token * const endToken2 = tok->link();
                for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                    tok = createAstAtToken(tok, cpp);
            }
        }
    }
}

static Token * createAstAtToken(Token *tok, bool cpp)
{
    if (Token::simpleMatch(tok, "for (")) {
        Token *tok2 = skipDecl(tok->tokAt(2));
        Token *init1 = nullptr;
        Token * const endPar = tok->next()->link();
        while (tok2 && tok2 != endPar && tok2->str() != ";") {
            if (tok2->str() == "<" && tok2->link()) {
                tok2 = tok2->link();
                if (!tok2)
                    break;
            } else if (Token::Match(tok2, "%name% %op%|(|[|.|:|::") || Token::Match(tok2->previous(), "[(;{}] %cop%|(")) {
                init1 = tok2;
                AST_state state1(cpp);
                compileExpression(tok2, state1);
                if (Token::Match(tok2, ";|)"))
                    break;
                init1 = 0;
            }
            if (!tok2) // #7109 invalid code
                return nullptr;
            tok2 = tok2->next();
        }
        if (!tok2 || tok2->str() != ";") {
            if (tok2 == endPar && init1) {
                tok->next()->astOperand2(init1);
                tok->next()->astOperand1(tok);
            }
            return tok2;
        }

        Token * const init = init1 ? init1 : tok2;

        Token * const semicolon1 = tok2;
        tok2 = tok2->next();
        AST_state state2(cpp);
        compileExpression(tok2, state2);

        Token * const semicolon2 = tok2;
        if (!semicolon2)
            return nullptr; // invalid code #7235
        tok2 = tok2->next();
        AST_state state3(cpp);
        if (Token::simpleMatch(tok2, "( {")) {
            state3.op.push(tok2->next());
            tok2 = tok2->link()->next();
        }
        compileExpression(tok2, state3);

        if (init != semicolon1)
            semicolon1->astOperand1(const_cast<Token*>(init->astTop()));
        tok2 = semicolon1->next();
        while (tok2 != semicolon2 && !tok2->isName() && !tok2->isNumber())
            tok2 = tok2->next();
        if (tok2 != semicolon2)
            semicolon2->astOperand1(const_cast<Token*>(tok2->astTop()));
        tok2 = endPar;
        while (tok2 != semicolon2 && !tok2->isName() && !tok2->isNumber()) {
            if (Token::simpleMatch(tok2, "} )"))
                tok2 = tok2->link();
            tok2 = tok2->previous();
        }
        if (tok2 != semicolon2)
            semicolon2->astOperand2(const_cast<Token*>(tok2->astTop()));
        else if (!state3.op.empty())
            semicolon2->astOperand2(const_cast<Token*>(state3.op.top()));

        semicolon1->astOperand2(semicolon2);
        tok->next()->astOperand1(tok);
        tok->next()->astOperand2(semicolon1);

        createAstAtTokenInner(endPar->link(), endPar, cpp);

        return endPar;
    }

    if (Token::simpleMatch(tok, "( {"))
        return tok;

    if (Token::Match(tok, "%type% <") && !Token::Match(tok->linkAt(1), "> [({]"))
        return tok->linkAt(1);

    if (tok->str() == "return" || !tok->previous() || Token::Match(tok, "%name% %op%|(|[|.|::|<|?|;") || Token::Match(tok->previous(), "[;{}] %cop%|++|--|( !!{")) {
        if (cpp && (Token::Match(tok->tokAt(-2), "[;{}] new|delete %name%") || Token::Match(tok->tokAt(-3), "[;{}] :: new|delete %name%")))
            tok = tok->previous();

        Token * const tok1 = tok;
        AST_state state(cpp);
        compileExpression(tok, state);
        Token * const endToken = tok;
        if (endToken == tok1 || !endToken)
            return tok1;

        createAstAtTokenInner(tok1->next(), endToken, cpp);

        return endToken ? endToken->previous() : nullptr;
    }

    return tok;
}

void TokenList::createAst()
{
    for (Token *tok = _front; tok; tok = tok ? tok->next() : nullptr) {
        tok = createAstAtToken(tok, isCPP());
    }
}

void TokenList::validateAst() const
{
    // Check for some known issues in AST to avoid crash/hang later on
    std::set < const Token* > safeAstTokens; // list of "safe" AST tokens without endless recursion
    for (const Token *tok = _front; tok; tok = tok->next()) {
        // Syntax error if binary operator only has 1 operand
        if ((tok->isAssignmentOp() || tok->isComparisonOp() || Token::Match(tok,"[|^/%]")) && tok->astOperand1() && !tok->astOperand2())
            throw InternalError(tok, "Syntax Error: AST broken, binary operator has only one operand.", InternalError::SYNTAX);

        // Syntax error if we encounter "?" with operand2 that is not ":"
        if (tok->astOperand2() && tok->str() == "?" && tok->astOperand2()->str() != ":")
            throw InternalError(tok, "Syntax Error: AST broken, ternary operator lacks ':'.", InternalError::SYNTAX);

        // Check for endless recursion
        const Token* parent=tok->astParent();
        if (parent) {
            std::set < const Token* > astTokens; // list of anchestors
            astTokens.insert(tok);
            do {
                if (safeAstTokens.find(parent) != safeAstTokens.end())
                    break;
                if (astTokens.find(parent) != astTokens.end())
                    throw InternalError(tok, "AST broken: endless recursion from '" + tok->str() + "'", InternalError::SYNTAX);
                astTokens.insert(parent);
            } while ((parent = parent->astParent()) != nullptr);
            safeAstTokens.insert(astTokens.begin(), astTokens.end());
        } else
            safeAstTokens.insert(tok);
    }
}

const std::string& TokenList::file(const Token *tok) const
{
    return _files.at(tok->fileIndex());
}

std::string TokenList::fileLine(const Token *tok) const
{
    return ErrorLogger::ErrorMessage::FileLocation(tok, this).stringify();
}

bool TokenList::validateToken(const Token* tok) const
{
    if (!tok)
        return true;
    for (const Token *t = _front; t; t = t->next()) {
        if (tok==t)
            return true;
    }
    return false;
}
