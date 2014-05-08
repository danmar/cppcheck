/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
    _settings(settings)
{
}

TokenList::~TokenList()
{
    deallocateTokens();
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
    for (unsigned int i = 0; i < _files.size(); ++i)
        if (Path::sameFileName(_files[i], fileName))
            return i;

    // The "_files" vector remembers what files have been tokenized..
    _files.push_back(Path::simplifyPath(fileName));
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
    _files.push_back(file0);

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
            CurrentToken.length() > 0 &&
            std::isdigit(CurrentToken[0])) {
            // Don't separate doubles "5.4"
        } else if (std::strchr("+-", ch) &&
                   CurrentToken.length() > 0 &&
                   std::isdigit(CurrentToken[0]) &&
                   (CurrentToken[CurrentToken.length()-1] == 'e' ||
                    CurrentToken[CurrentToken.length()-1] == 'E') &&
                   !MathLib::isHex(CurrentToken)) {
            // Don't separate doubles "4.2e+10"
        } else if (CurrentToken.empty() && ch == '.' && std::isdigit(code.peek())) {
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
    _front->assignProgressValues();

    for (unsigned int i = 1; i < _files.size(); i++)
        _files[i] = Path::getRelativePath(_files[i], _settings->_basePaths);

    return true;
}

//---------------------------------------------------------------------------

static bool iscast(const Token *tok)
{
    if (!Token::Match(tok, "( %var%"))
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        if (tok2->str() == ")")
            return tok2->previous()->str() == "*" ||
                   (Token::Match(tok2, ") %any%") &&
                    (!tok2->next()->isOp() && !Token::Match(tok2->next(), "[[]);,?:.]")));
        if (!Token::Match(tok2, "%var%|*|&|::"))
            return false;
    }

    return false;
}

static void compileUnaryOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &, unsigned int depth), std::stack<Token*> &op, unsigned int depth)
{
    Token *unaryop = tok;
    tok = tok->next();
    f(tok,op, depth);

    if (!op.empty()) {
        unaryop->astOperand1(op.top());
        op.pop();
    }
    op.push(unaryop);
}

static void compileBinOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &, unsigned int depth), std::stack<Token*> &op, unsigned int depth)
{
    Token *binop = tok;
    tok = tok->next();
    if (tok)
        f(tok,op, depth);

    // Assignment operators are executed in right-to-left order
    if (binop->isAssignmentOp() && tok && tok->isAssignmentOp())
        compileBinOp(tok,f,op,depth);

    // TODO: Should we check if op is empty.
    // * Is it better to add assertion that it isn't?
    // * Write debug warning if it's empty?
    if (!op.empty()) {
        binop->astOperand2(op.top());
        op.pop();
    }
    if (!op.empty()) {
        binop->astOperand1(op.top());
        op.pop();
    }
    op.push(binop);
}

static void compileDot(Token *&tok, std::stack<Token*> &op, unsigned int depth);
static void compileExpression(Token *&tok, std::stack<Token*> &op, unsigned int depth);

static void compileTerm(Token *& tok, std::stack<Token*> &op, unsigned int depth)
{
    if (!tok)
        return;
    if (Token::Match(tok, "L %str%|%char%"))
        tok = tok->next();
    if (tok->isLiteral()) {
        op.push(tok);
        tok = tok->next();
    } else if (Token::Match(tok, "+|-|~|*|&|!")) {
        compileUnaryOp(tok, compileDot, op, depth);
    } else if (tok->str() == "return") {
        compileUnaryOp(tok, compileExpression, op, depth);
    } else if (tok->isName()) {
        const bool templatefunc = Token::Match(tok, "%var% <") && Token::simpleMatch(tok->linkAt(1), "> (");
        if (!Token::Match(tok->previous(), ".|::") && Token::Match(tok->next(), "++|--")) {  // post increment / decrement
            tok = tok->next();
            tok->astOperand1(tok->previous());
            op.push(tok);
            tok = tok->next();
        } else if (tok->next() && tok->next()->str() == "<" && tok->next()->link() && !templatefunc) {
            op.push(tok);
            tok = tok->next()->link()->next();
            if (!Token::simpleMatch(tok, "{"))
                compileTerm(tok,op, depth);
        } else if (!Token::Match(tok->next(), "(|[") && !templatefunc) {
            op.push(tok);
            tok = tok->next();
        } else if (Token::Match(tok->previous(), ".|:: %var% [")) {
            op.push(tok);
            tok = tok->next();
        } else {
            Token *name = tok;
            Token *par  = templatefunc ? tok->linkAt(1)->next() : tok->next();
            Token *prev = name;
            tok = par->next();
            if (Token::Match(tok, ")|]")) {
                par->astOperand1(name);
                tok = tok->next();
            } else {
                tok = tok->previous();
            }
            while (Token::Match(tok, "(|[")) {
                Token *tok1 = tok;
                tok = tok->next();
                while (Token::Match(tok,"%var% %var%")) // example: sizeof(struct S)
                    tok = tok->next();
                compileExpression(tok, op, depth);
                if (!op.empty()) {
                    tok1->astOperand2(op.top());
                    op.pop();
                }
                tok1->astOperand1(prev);
                prev = tok1;
                if (Token::Match(tok, "]|)")) {
                    tok = tok->next();
                    if (depth==1U && Token::Match(tok,"++|--") && op.empty()) {
                        tok->astOperand1(tok->previous()->link());
                        tok = tok->next();
                    }
                }
            }
            op.push(par);
        }
    } else if (Token::Match(tok, "++|--")) {
        bool pre = false;
        if (Token::Match(tok->next(), "%var%|("))
            pre = true;
        else if (!op.empty() && !Token::Match(tok->previous(), "(|[") && !op.top()->isOp())
            pre = false;
        else
            pre = true;

        if (pre) {
            // pre increment/decrement
            compileUnaryOp(tok, compileDot, op, depth);
        } else {
            // post increment/decrement
            tok->astOperand1(op.top());
            op.pop();
            op.push(tok);
            tok = tok->next();
        }
    } else if (tok->str() == "(") {
        if (iscast(tok)) {
            Token *unaryop = tok;
            tok = tok->link()->next();
            compileDot(tok,op, depth);

            if (!op.empty()) {
                unaryop->astOperand1(op.top());
                op.pop();
            }
            op.push(unaryop);
        } else if (Token::simpleMatch(tok->link(),") (")) {
            // Parenthesized sub-expression
            Token *nextpar = tok->link()->next();
            tok = tok->next();
            compileExpression(tok,op, depth);
            tok = nextpar;
            if (Token::simpleMatch(tok,"( )")) {
                if (!op.empty()) {
                    Token *f = op.top();
                    op.pop();
                    tok->astOperand1(f);
                    op.push(tok);
                }
                tok = tok->tokAt(2);
            } else {
                compileBinOp(tok, compileExpression, op, depth);
                tok = tok->next();
            }
        } else {
            // Parenthesized sub-expression
            tok = tok->next();
            compileExpression(tok,op, depth);
            tok = tok->next();
        }
    } else if (tok->str() == "{") {
        op.push(tok);
        tok = tok->link()->next();
    }
}

static void compileScope(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileTerm(tok,op, depth);
    while (tok) {
        if (tok->str() == "::") {
            if (tok->previous() && tok->previous()->isName())
                compileBinOp(tok, compileTerm, op, depth);
            else
                compileUnaryOp(tok, compileDot, op, depth);
            if (depth==1U && Token::Match(tok,"++|--"))
                compileTerm(tok,op,depth);
        } else break;
    }
}

static void compileDot(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileScope(tok,op, depth);
    while (tok) {
        if (tok->str() == ".") {
            compileBinOp(tok, compileScope, op, depth);
            if (depth==1U && Token::Match(tok,"++|--"))
                compileTerm(tok,op,depth);
        } else break;
    }
}

static void compileBrackets(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileDot(tok,op, depth);
    while (tok) {
        if (tok->str() == ".") { // compile dot and brackets from left to right. Example: "a.b[c]"
            compileBinOp(tok, compileScope, op, depth);
            if (depth==1U && Token::Match(tok,"++|--"))
                compileTerm(tok,op,depth);
        } else if (tok->str() == "[") {
            compileBinOp(tok, compileDot, op, depth);
            tok = tok->next();
        } else break;
    }
}

static void compileMulDiv(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileBrackets(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "[*/%]")) {
            if (Token::Match(tok, "* [*,)]")) {
                while (tok->next() && tok->str() == "*")
                    tok = tok->next();
                break;
            }
            compileBinOp(tok, compileBrackets, op, depth);
        } else break;
    }
}

static void compileAddSub(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileMulDiv(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "+|-")) {
            compileBinOp(tok, compileMulDiv, op, depth);
        } else break;
    }
}

static void compileShift(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAddSub(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "<<|>>")) {
            compileBinOp(tok, compileAddSub, op, depth);
        } else break;
    }
}

static void compileRelComp(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileShift(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "<|<=|>=|>")) {
            compileBinOp(tok, compileShift, op, depth);
        } else break;
    }
}

static void compileEqComp(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileRelComp(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "==|!=")) {
            compileBinOp(tok, compileRelComp, op, depth);
        } else break;
    }
}

static void compileAnd(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileEqComp(tok,op, depth);
    while (tok) {
        if (tok->str() == "&") {
            compileBinOp(tok, compileEqComp, op, depth);
        } else break;
    }
}

static void compileXor(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAnd(tok,op, depth);
    while (tok) {
        if (tok->str() == "^") {
            compileBinOp(tok, compileAnd, op, depth);
        } else break;
    }
}

static void compileOr(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileXor(tok,op, depth);
    while (tok) {
        if (tok->str() == "|") {
            compileBinOp(tok, compileXor, op, depth);
        } else break;
    }
}

static void compileLogicAnd(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileOr(tok,op, depth);
    while (tok) {
        if (tok->str() == "&&") {
            compileBinOp(tok, compileOr, op, depth);
        } else break;
    }
}

static void compileLogicOr(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileLogicAnd(tok,op, depth);
    while (tok) {
        if (tok->str() == "||") {
            compileBinOp(tok, compileLogicAnd, op, depth);
        } else break;
    }
}

static void compileTernaryOp(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileLogicOr(tok,op, depth);
    while (tok) {
        if (Token::Match(tok, "[?:]")) {
            compileBinOp(tok, compileLogicOr, op, depth);
        } else break;
    }
}

static void compileAssign(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileTernaryOp(tok,op, depth);
    while (tok) {
        if (tok->isAssignmentOp()) {
            compileBinOp(tok, compileTernaryOp, op, depth);
        } else break;
    }
}

static void compileComma(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAssign(tok,op, depth);
    while (tok) {
        if (tok->str() == ",") {
            compileBinOp(tok, compileAssign, op, depth);
        } else break;
    }
}

static void compileExpression(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    if (depth > AST_MAX_DEPTH)
        return; // ticket #5592
    if (tok)
        compileComma(tok,op, depth+1U);
}

static Token * createAstAtToken(Token *tok)
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
            } else if (Token::Match(tok2, "%var% %op%|(|[|.|=|:|::") || Token::Match(tok2->previous(), "[(;{}] %cop%|(")) {
                init1 = tok2;
                std::stack<Token *> operands;
                compileExpression(tok2, operands, 0U);
                if (tok2->str() == ";" || tok2->str() == ")")
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
        std::stack<Token *> operands2;
        compileExpression(tok2, operands2, 0U);

        Token * const semicolon2 = tok2;
        tok2 = tok2->next();
        std::stack<Token *> operands3;
        compileExpression(tok2, operands3, 0U);

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

    if (tok->str() == "return" || !tok->previous() || Token::Match(tok, "%var% %op%|(|[|.|=|::") || Token::Match(tok->previous(), "[;{}] %cop%|( !!{")) {
        std::stack<Token *> operands;
        Token * const tok1 = tok;
        compileExpression(tok, operands, 0U);
        Token * const endToken = tok;
        if (endToken == tok1)
            return tok1;

        // Compile inner expressions inside inner ({..})
        for (tok = tok1->next(); tok && tok != endToken; tok = tok ? tok->next() : NULL) {
            if (!Token::simpleMatch(tok, "( {"))
                continue;
            if (tok->next() == endToken)
                break;
            const Token * const endToken2 = tok->linkAt(1);
            for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : NULL)
                tok = createAstAtToken(tok);
        }

        return endToken ? endToken->previous() : NULL;
    }

    return tok;
}

void TokenList::createAst()
{
    for (Token *tok = _front; tok; tok = tok ? tok->next() : NULL) {
        tok = createAstAtToken(tok);
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
