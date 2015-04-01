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
#include <cassert>

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
    return static_cast<unsigned int>(_files.size() - 1);
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

void TokenList::addtoken(const std::string & str, const unsigned int lineno, const unsigned int fileno, bool split)
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
    std::string str2;
    if (MathLib::isHex(str) || MathLib::isOct(str) || MathLib::isBin(str)) {
        std::ostringstream str2stream;
        str2stream << MathLib::toLongNumber(str);
        str2 = str2stream.str();
    } else if (str.compare(0, 5, "_Bool") == 0) {
        str2 = "bool";
    } else {
        str2 = str;
    }

    if (_back) {
        _back->insertToken(str2);
    } else {
        _front = new Token(&_back);
        _back = _front;
        _back->str(str2);
    }

    if (isCPP() && str == "delete")
        _back->isKeyword(true);
    _back->linenr(lineno);
    _back->fileIndex(fileno);
}

void TokenList::addtoken(const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    if (tok == 0)
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
        dest->type(src->type());
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
            }
            CurrentToken.clear();
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
                   (CurrentToken[CurrentToken.length()-1] == 'e' ||
                    CurrentToken[CurrentToken.length()-1] == 'E') &&
                   !MathLib::isHex(CurrentToken)) {
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

                unsigned int row;
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
            }

            CurrentToken.clear();

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
    Token::assignProgressValues(_front);

    for (std::size_t i = 1; i < _files.size(); i++)
        _files[i] = Path::getRelativePath(_files[i], _settings->_basePaths);

    return true;
}

//---------------------------------------------------------------------------

unsigned long long TokenList::calculateChecksum() const
{
    unsigned long long checksum = 0;
    for (const Token* tok = front(); tok; tok = tok->next()) {
        unsigned int subchecksum1 = tok->flags() + tok->varId() + static_cast<unsigned int>(tok->type());
        unsigned int subchecksum2 = 0;
        for (std::size_t i = 0; i < tok->str().size(); i++)
            subchecksum2 += (unsigned int)tok->str()[i];
        if (!tok->originalName().empty()) {
            for (std::size_t i = 0; i < tok->originalName().size(); i++)
                subchecksum2 += (unsigned int) tok->originalName()[i];
        }

        checksum ^= ((static_cast<unsigned long long>(subchecksum1) << 32) | subchecksum2);

        bool bit1 = (checksum & 1) != 0;
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
    explicit AST_state(bool cpp_) : depth(0), inArrayAssignment(0), cpp(cpp_) {}
};

static bool iscast(const Token *tok)
{
    if (!Token::Match(tok, "( %name%"))
        return false;

    if (tok->previous() && tok->previous()->isName() && tok->previous()->str() != "return")
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    bool type = false;
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        while (tok2->link() && Token::Match(tok2, "(|[|<"))
            tok2 = tok2->link()->next();

        if (tok2->str() == ")")
            return type || tok2->strAt(-1) == "*" ||
                   (Token::Match(tok2, ") %any%") &&
                    (tok2->strAt(1) == "&" || (!tok2->next()->isOp() && !Token::Match(tok2->next(), "[[]);,?:.]"))));
        if (!Token::Match(tok2, "%name%|*|&|::"))
            return false;

        if (tok2->isStandardType() && tok2->next()->str() != "(")
            type = true;
    }

    return false;
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
        tok = tok->next();
    } else if (tok->isName() && tok->str() != "case") {
        if (tok->str() == "return") {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (Token::Match(tok, "sizeof !!(")) {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (!state.cpp || !Token::Match(tok, "new|delete %name%|*|&|::|(|[")) {
            while (tok->next() && tok->next()->isName())
                tok = tok->next();
            state.op.push(tok);
            if (tok->next() && tok->linkAt(1) && Token::Match(tok, "%name% <"))
                tok = tok->linkAt(1);
            tok = tok->next();
        }
    } else if (tok->str() == "{") {
        if (!state.inArrayAssignment && tok->strAt(-1) != "=") {
            state.op.push(tok);
            tok = tok->link()->next();
        } else {
            if (tok->link() != tok->next()) {
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                state.inArrayAssignment--;
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
            && (tok->previous()->type() != Token::eIncDecOp || tok->type() == Token::eIncDecOp)))
        return true;

    return tok->strAt(-1) == ")" && iscast(tok->linkAt(-1));
}

static void compilePrecedence2(Token *&tok, AST_state& state)
{
    compileScope(tok, state);
    while (tok) {
        if (tok->type() == Token::eIncDecOp && !isPrefixUnary(tok, state.cpp)) {
            compileUnaryOp(tok, state, compileScope);
        } else if (tok->str() == "." && tok->strAt(1) != "*") {
            if (tok->strAt(1) == ".") {
                state.op.push(tok);
                tok = tok->tokAt(3);
                break;
            } else
                compileBinOp(tok, state, compileScope);
        } else if (tok->str() == "[") {
            if (state.cpp && isPrefixUnary(tok, state.cpp) && tok->link()->strAt(1) == "(") { // Lambda
                // What we do here:
                // - Nest the round bracket under the square bracket.
                // - Nest what follows the lambda (if anything) with the lambda opening [
                // - Compile the content of the lambda function as separate tree
                Token* squareBracket = tok;
                Token* roundBracket = squareBracket->link()->next();
                Token* curlyBracket = Token::findsimplematch(roundBracket->link()->next(), "{");
                if (!curlyBracket)
                    break;
                tok = curlyBracket->next();
                compileExpression(tok, state);
                state.op.push(roundBracket);
                compileUnaryOp(squareBracket, state, nullptr);
                tok = curlyBracket->link()->next();
            } else {
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
            bool opPrevTopSquare = !state.op.empty() && state.op.top() && state.op.top()->str() == "[";
            std::size_t oldOpSize = state.op.size();
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
        } else break;
    }
}

static void compilePrecedence3(Token *&tok, AST_state& state)
{
    compilePrecedence2(tok, state);
    while (tok) {
        if ((Token::Match(tok, "[+-!~*&]") || tok->type() == Token::eIncDecOp) &&
            isPrefixUnary(tok, state.cpp)) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok;
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
            } else if (tok->str() == "[" || tok->str() == "(")
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
            if (tok->str() == "[")
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
                Token* tok2 = tok;
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
        // TODO: http://en.cppreference.com/w/cpp/language/operator_precedence says:
        //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
        if (tok->isAssignmentOp() || Token::Match(tok, "[?:]")) {
            if (tok->str() == "?" && tok->strAt(1) == ":") {
                state.op.push(0);
            }
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

static Token * createAstAtToken(Token *tok, bool cpp)
{
    if (Token::simpleMatch(tok,"for (")) {
        Token *tok2 = tok->tokAt(2);
        Token *init1 = nullptr;
        const Token * const endPar = tok->next()->link();
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
        tok2 = tok2->next();
        AST_state state3(cpp);
        compileExpression(tok2, state3);

        if (init != semicolon1)
            semicolon1->astOperand1(const_cast<Token*>(init->astTop()));
        tok2 = semicolon1->next();
        while (tok2 != semicolon2 && !tok2->isName() && !tok2->isNumber())
            tok2 = tok2->next();
        if (tok2 != semicolon2)
            semicolon2->astOperand1(const_cast<Token*>(tok2->astTop()));
        tok2 = tok->linkAt(1);
        while (tok2 != semicolon2 && !tok2->isName() && !tok2->isNumber())
            tok2 = tok2->previous();
        if (tok2 != semicolon2)
            semicolon2->astOperand2(const_cast<Token*>(tok2->astTop()));

        semicolon1->astOperand2(semicolon2);
        tok->next()->astOperand1(tok);
        tok->next()->astOperand2(semicolon1);

        return tok->linkAt(1);
    }

    if (Token::simpleMatch(tok, "( {"))
        return tok;

    if (Token::Match(tok, "%type% <") && Token::Match(tok->linkAt(1), "> !!("))
        return tok->linkAt(1);

    if (tok->str() == "return" || !tok->previous() || Token::Match(tok, "%name% %op%|(|[|.|::|<|?") || Token::Match(tok->previous(), "[;{}] %cop%|++|--|( !!{")) {
        Token * const tok1 = tok;
        AST_state state(cpp);
        compileExpression(tok, state);
        Token * const endToken = tok;
        if (endToken == tok1)
            return tok1;

        // Compile inner expressions inside inner ({..})
        for (tok = tok1->next(); tok && tok != endToken; tok = tok ? tok->next() : NULL) {
            if (!Token::simpleMatch(tok, "( {"))
                continue;
            if (tok->next() == endToken)
                break;
            if (Token::simpleMatch(tok, "( { ."))
                break;
            const Token * const endToken2 = tok->linkAt(1);
            for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : NULL)
                tok = createAstAtToken(tok, cpp);
        }

        return endToken ? endToken->previous() : NULL;
    }

    return tok;
}

void TokenList::createAst()
{
    for (Token *tok = _front; tok; tok = tok ? tok->next() : NULL) {
        tok = createAstAtToken(tok, isCPP());
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
