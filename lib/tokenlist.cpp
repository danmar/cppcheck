/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

void TokenList::addtoken(const char str[], const unsigned int lineno, const unsigned int fileno, bool split)
{
    if (str[0] == 0)
        return;

    // If token contains # characters, split it up
    if (split && std::strstr(str, "##")) {
        std::string temp;
        for (unsigned int i = 0; str[i]; ++i) {
            if (std::strncmp(&str[i], "##", 2) == 0) {
                addtoken(temp.c_str(), lineno, fileno, false);
                temp.clear();
                addtoken("##", lineno, fileno, false);
                ++i;
            } else
                temp += str[i];
        }
        addtoken(temp.c_str(), lineno, fileno, false);
        return;
    }

    // Replace hexadecimal value with decimal
    std::ostringstream str2;
    if (MathLib::isHex(str) || MathLib::isOct(str) || MathLib::isBin(str)) {
        str2 << MathLib::toLongNumber(str);
    } else if (std::strncmp(str, "_Bool", 5) == 0) {
        str2 << "bool";
    } else {
        str2 << str;
    }

    if (_back) {
        _back->insertToken(str2.str());
    } else {
        _front = new Token(&_back);
        _back = _front;
        _back->str(str2.str());
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
    _back->isUnsigned(tok->isUnsigned());
    _back->isSigned(tok->isSigned());
    _back->isLong(tok->isLong());
    _back->isAttributeUnused(tok->isAttributeUnused());
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
        dest->isUnsigned(src->isUnsigned());
        dest->isSigned(src->isSigned());
        dest->isPointerCompare(src->isPointerCompare());
        dest->isLong(src->isLong());
        dest->isAttributeUnused(src->isAttributeUnused());
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
    for (char ch = (char)code.get(); code.good(); ch = (char)code.get()) {
        if (ch == Preprocessor::macroChar) {
            while (code.peek() == Preprocessor::macroChar)
                code.get();
            ch = ' ';
            expandedMacro = true;
        } else if (ch == '\n') {
            expandedMacro = false;
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

                // Has this file been tokenized already?
                ++lineno;
                bool foundOurfile = false;
                fileIndexes.push(FileIndex);
                for (unsigned int i = 0; i < _files.size(); ++i) {
                    if (Path::sameFileName(_files[i], line)) {
                        // Use this index
                        foundOurfile = true;
                        FileIndex = i;
                    }
                }

                if (!foundOurfile) {
                    // The "_files" vector remembers what files have been tokenized..
                    _files.push_back(Path::simplifyPath(line.c_str()));
                    FileIndex = static_cast<unsigned int>(_files.size() - 1);
                }

                lineNumbers.push(lineno);
                lineno = 0;
            } else {
                // Add previous token
                addtoken(CurrentToken.c_str(), lineno, FileIndex);
                if (!CurrentToken.empty())
                    _back->isExpandedMacro(expandedMacro);

                // Add content of the string
                addtoken(line.c_str(), lineno, FileIndex);
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

                // Update the current line number
                unsigned int row;
                if (!(std::stringstream(line) >> row))
                    ++lineno;
                else
                    lineno = row;
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

            addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
            if (!CurrentToken.empty())
                _back->isExpandedMacro(expandedMacro);

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
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            _back->isExpandedMacro(expandedMacro);
            CurrentToken.clear();
            continue;
        }

        CurrentToken += ch;
    }
    addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
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

    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        if (!Token::Match(tok2, "%var%|*"))
            return Token::Match(tok2, ") %any%") && (!tok2->next()->isOp() && tok2->next()->str() != "[");
    }

    return false;
}

static void compileUnaryOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &), std::stack<Token*> &op)
{
    Token *unaryop = tok;
    tok = tok->next();
    f(tok,op);

    if (!op.empty()) {
        unaryop->astOperand1(op.top());
        op.pop();
    }
    op.push(unaryop);
}

static void compileBinOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &), std::stack<Token*> &op)
{
    Token *binop = tok;
    tok = tok->next();
    if (tok)
        f(tok,op);

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

static void compileDot(Token *&tok, std::stack<Token*> &op);
static void compileExpression(Token *&tok, std::stack<Token*> &op);

static void compileTerm(Token *& tok, std::stack<Token*> &op)
{
    if (!tok)
        return;
    if (Token::Match(tok, "L %str%|%char%"))
        tok = tok->next();
    if (tok->isLiteral()) {
        op.push(tok);
        tok = tok->next();
    } else if (Token::Match(tok, "+|-|~|*|&|!")) {
        compileUnaryOp(tok, compileDot, op);
    } else if (tok->str() == "return") {
        compileUnaryOp(tok, compileExpression, op);
    } else if (tok->isName()) {
        if (Token::Match(tok->next(), "++|--")) {  // post increment / decrement
            tok = tok->next();
            tok->astOperand1(tok->previous());
            op.push(tok);
            tok = tok->next();
        } else if (!Token::Match(tok->next(), "(|[")) {
            op.push(tok);
            tok = tok->next();
        } else {
            Token *name = tok;
            tok = tok->tokAt(2);
            if (Token::Match(tok, ")|]")) {
                name->next()->astOperand1(name);
                tok = tok->next();
            } else {
                Token *prev = name;
                tok = tok->previous();
                while (Token::Match(tok, "(|[")) {
                    Token *tok1 = tok;
                    tok = tok->next();
                    compileExpression(tok, op);
                    if (!op.empty()) {
                        tok1->astOperand2(op.top());
                        op.pop();
                    }
                    tok1->astOperand1(prev);
                    prev = tok1;
                    if (Token::Match(tok, "]|)"))
                        tok = tok->next();
                }
            }
            op.push(name->next());
        }
    } else if (Token::Match(tok, "++|--")) {
        if (!op.empty() && op.top()->isOp()) {
            // post increment/decrement
            tok->astOperand1(op.top());
            op.pop();
            op.push(tok);
            tok = tok->next();
        } else {
            // pre increment/decrement
            compileUnaryOp(tok, compileDot, op);
        }
    } else if (tok->str() == "(") {
        if (iscast(tok)) {
            Token *unaryop = tok;
            tok = tok->link()->next();
            compileDot(tok,op);

            if (!op.empty()) {
                unaryop->astOperand1(op.top());
                op.pop();
            }
            op.push(unaryop);
        } else if (Token::Match(tok,"( {")) {
            op.push(tok->next());
            tok = tok->link()->next();
        } else if (Token::simpleMatch(tok->link(),") (")) {
            // Parenthesized sub-expression
            Token *nextpar = tok->link()->next();
            tok = tok->next();
            compileExpression(tok,op);
            tok = nextpar;
            compileBinOp(tok, compileExpression, op);
            tok = tok->next();
        } else {
            // Parenthesized sub-expression
            tok = tok->next();
            compileExpression(tok,op);
            tok = tok->next();
        }
    }
}

static void compileScope(Token *&tok, std::stack<Token*> &op)
{
    compileTerm(tok,op);
    while (tok) {
        if (tok->str() == "::") {
            compileBinOp(tok, compileTerm, op);
        } else break;
    }
}

static void compileParAndBrackets(Token *&tok, std::stack<Token*> &op)
{
    compileScope(tok,op);
    while (tok) {
        if (tok->str() == "[") {
            compileBinOp(tok, compileScope, op);
        } else break;
    }
}

static void compileDot(Token *&tok, std::stack<Token*> &op)
{
    compileParAndBrackets(tok,op);
    while (tok) {
        if (tok->str() == ".") {
            compileBinOp(tok, compileParAndBrackets, op);
        } else break;
    }
}

static void compileMulDiv(Token *&tok, std::stack<Token*> &op)
{
    compileDot(tok,op);
    while (tok) {
        if (Token::Match(tok, "[*/%]")) {
            if (Token::Match(tok, "* [,)]"))
                break;
            compileBinOp(tok, compileDot, op);
        } else break;
    }
}

static void compileAddSub(Token *&tok, std::stack<Token*> &op)
{
    compileMulDiv(tok,op);
    while (tok) {
        if (Token::Match(tok, "+|-")) {
            compileBinOp(tok, compileMulDiv, op);
        } else break;
    }
}

static void compileShift(Token *&tok, std::stack<Token*> &op)
{
    compileAddSub(tok,op);
    while (tok) {
        if (Token::Match(tok, "<<|>>")) {
            compileBinOp(tok, compileAddSub, op);
        } else break;
    }
}

static void compileRelComp(Token *&tok, std::stack<Token*> &op)
{
    compileShift(tok,op);
    while (tok) {
        if (Token::Match(tok, "<|<=|>=|>")) {
            compileBinOp(tok, compileShift, op);
        } else break;
    }
}

static void compileEqComp(Token *&tok, std::stack<Token*> &op)
{
    compileRelComp(tok,op);
    while (tok) {
        if (Token::Match(tok, "==|!=")) {
            compileBinOp(tok, compileRelComp, op);
        } else break;
    }
}

static void compileAnd(Token *&tok, std::stack<Token*> &op)
{
    compileEqComp(tok,op);
    while (tok) {
        if (tok->str() == "&") {
            compileBinOp(tok, compileEqComp, op);
        } else break;
    }
}

static void compileXor(Token *&tok, std::stack<Token*> &op)
{
    compileAnd(tok,op);
    while (tok) {
        if (tok->str() == "^") {
            compileBinOp(tok, compileAnd, op);
        } else break;
    }
}

static void compileOr(Token *&tok, std::stack<Token*> &op)
{
    compileXor(tok,op);
    while (tok) {
        if (tok->str() == "|") {
            compileBinOp(tok, compileXor, op);
        } else break;
    }
}

static void compileLogicAnd(Token *&tok, std::stack<Token*> &op)
{
    compileOr(tok,op);
    while (tok) {
        if (tok->str() == "&&") {
            compileBinOp(tok, compileOr, op);
        } else break;
    }
}

static void compileLogicOr(Token *&tok, std::stack<Token*> &op)
{
    compileLogicAnd(tok,op);
    while (tok) {
        if (tok->str() == "||") {
            compileBinOp(tok, compileLogicAnd, op);
        } else break;
    }
}

static void compileTernaryOp(Token *&tok, std::stack<Token*> &op)
{
    compileLogicOr(tok,op);
    while (tok) {
        if (Token::Match(tok, "[?:]")) {
            compileBinOp(tok, compileLogicOr, op);
        } else break;
    }
}

static void compileAssign(Token *&tok, std::stack<Token*> &op)
{
    compileTernaryOp(tok,op);
    while (tok) {
        if (tok->str() == "=") {
            compileBinOp(tok, compileTernaryOp, op);
        } else break;
    }
}

static void compileComma(Token *&tok, std::stack<Token*> &op)
{
    compileAssign(tok,op);
    while (tok) {
        if (tok->str() == ",") {
            compileBinOp(tok, compileAssign, op);
        } else break;
    }
}

static void compileExpression(Token *&tok, std::stack<Token*> &op)
{
    if (tok)
        compileComma(tok,op);
}

void TokenList::createAst()
{
    for (Token *tok = _front; tok; tok = tok ? tok->next() : NULL) {
        if (tok->str() == "return" || !tok->previous() || Token::Match(tok, "%var% (|[|.|=") || Token::Match(tok->previous(), "[;{}] %cop%")) {
            std::stack<Token *> operands;
            compileExpression(tok, operands);
        }
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
