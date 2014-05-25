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
    Token::assignProgressValues(_front);

    for (unsigned int i = 1; i < _files.size(); i++)
        _files[i] = Path::getRelativePath(_files[i], _settings->_basePaths);

    return true;
}

//---------------------------------------------------------------------------

static bool iscast(const Token *tok)
{
    if (!Token::Match(tok, "( %var%"))
        return false;

    if (tok->previous() && tok->previous()->isName())
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    bool type = false;
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        while (tok2->link() && Token::Match(tok2, "(|[|<"))
            tok2 = tok2->link()->next();

        if (tok2->str() == ")")
            return type || tok2->previous()->str() == "*" ||
                   (Token::Match(tok2, ") %any%") &&
                    (tok2->strAt(1) == "&" || (!tok2->next()->isOp() && !Token::Match(tok2->next(), "[[]);,?:.]"))));
        if (!Token::Match(tok2, "%var%|*|&|::"))
            return false;

        if (tok2->isStandardType())
            type = true;
    }

    return false;
}

static void compileUnaryOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &, unsigned int depth), std::stack<Token*> &op, unsigned int depth)
{
    Token *unaryop = tok;
    if (f) {
        tok = tok->next();
        f(tok, op, depth);
    }

    if (!op.empty()) {
        unaryop->astOperand1(op.top());
        op.pop();
    }
    op.push(unaryop);
}

static void compileBinOp(Token *&tok, void (*f)(Token *&, std::stack<Token*> &, unsigned int depth), std::stack<Token*> &op, unsigned int depth)
{
    Token *binop = tok;
    if (f) {
        tok = tok->next();
        if (tok)
            f(tok, op, depth);
    }

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
    } else if (tok->str() == "return") {
        compileUnaryOp(tok, compileExpression, op, depth);
        op.pop();
    } else if (tok->isName()) {
        while (tok->next() && tok->next()->isName())
            tok = tok->next();
        op.push(tok);
        if (tok->next() && tok->linkAt(1) && Token::Match(tok, "%var% <"))
            tok = tok->linkAt(1);
        tok = tok->next();
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
                compileUnaryOp(tok, compileTerm, op, depth);
        } else break;
    }
}

static bool isPrefixUnary(const Token* tok)
{
    if (!tok->previous()
        || (Token::Match(tok->previous(), "(|[|{|%op%|;|}|?|:|,|.|return|throw")
            && (tok->previous()->type() != Token::eIncDecOp || tok->type() == Token::eIncDecOp)))
        return true;

    return tok->strAt(-1) == ")" && iscast(tok->linkAt(-1));
}

static void compilePrecedence2(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileScope(tok, op, depth);
    while (tok) {
        if (tok->type() == Token::eIncDecOp && !isPrefixUnary(tok)) {
            compileUnaryOp(tok, compileScope, op, depth);
        } else if (tok->str() == "." && tok->strAt(1) != "*") {
            compileBinOp(tok, compileScope, op, depth);
        } else if (tok->str() == "[") {
            if (isPrefixUnary(tok) && tok->link()->strAt(1) == "(") { // Lambda
                // What we do here:
                // - Nest the round bracket under the square bracket.
                // - Nest what follows the lambda (if anything) with the lambda opening [
                // - Compile the content of the lambda function as separate tree
                Token* squareBracket = tok;
                Token* roundBracket = squareBracket->link()->next();
                Token* curlyBracket = Token::findsimplematch(roundBracket->link()->next(), "{");
                tok = curlyBracket->next();
                compileExpression(tok, op, depth);
                op.push(roundBracket);
                compileUnaryOp(squareBracket, 0, op, depth);
                tok = curlyBracket->link()->next();
            } else {
                Token* tok2 = tok;
                compileBinOp(tok, compileExpression, op, depth);
                tok = tok2->link()->next();
            }
        } else if (tok->str() == "(" && (!iscast(tok) || Token::Match(tok->previous(), "if|while|for|switch|catch"))) {
            Token* tok2 = tok;
            tok = tok->next();
            bool opPrevTopSquare = !op.empty() && op.top() && op.top()->str() == "[";
            compileExpression(tok, op, depth);
            tok = tok2;
            if ((tok->previous() && tok->previous()->isName() && !Token::Match(tok->previous(), "return|throw"))
                || tok->strAt(-1) == "]"
                || (tok->strAt(-1) == ">" && tok->linkAt(-1))
                || (tok->strAt(-1) == ")" && !iscast(tok->linkAt(-1))) // Don't treat brackets to clarify precedence as function calls
                || (tok->strAt(-1) == "}" && opPrevTopSquare)) {
                if (tok->strAt(1) != ")")
                    compileBinOp(tok, 0, op, depth);
                else
                    compileUnaryOp(tok, 0, op, depth);
            }
            tok = tok->link()->next();
        } else break;
    }
}

static void compilePrecedence3(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compilePrecedence2(tok, op, depth);
    while (tok) {
        if ((Token::Match(tok, "[+-!~*&]") || tok->type() == Token::eIncDecOp) &&
            isPrefixUnary(tok)) {
            compileUnaryOp(tok, compilePrecedence3, op, depth);
        } else if (tok->str() == "(" && iscast(tok)) {
            Token* tok2 = tok;
            tok = tok->link()->next();
            compilePrecedence3(tok, op, depth);
            compileUnaryOp(tok2, 0, op, depth);
        }

        // TODO: Handle sizeof, new and delete
        else break;
    }
}

static void compilePointerToElem(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compilePrecedence3(tok, op, depth);
    while (tok) {
        if (Token::simpleMatch(tok, ". *")) {
            compileBinOp(tok, compilePrecedence3, op, depth);
        } else break;
    }
}

static void compileMulDiv(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compilePointerToElem(tok, op, depth);
    while (tok) {
        if (Token::Match(tok, "[/%]") || (tok->str() == "*" && !tok->astOperand1())) {
            if (Token::Match(tok, "* [*,)]")) {
                Token* tok2 = tok;
                while (tok2->next() && tok2->str() == "*")
                    tok2 = tok2->next();
                if (Token::Match(tok2, "[,)]")) {
                    break;
                }
            }
            compileBinOp(tok, compilePointerToElem, op, depth);
        } else break;
    }
}

static void compileAddSub(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileMulDiv(tok, op, depth);
    while (tok) {
        if (Token::Match(tok, "+|-") && !tok->astOperand1()) {
            compileBinOp(tok, compileMulDiv, op, depth);
        } else break;
    }
}

static void compileShift(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAddSub(tok, op, depth);
    while (tok) {
        if (Token::Match(tok, "<<|>>")) {
            compileBinOp(tok, compileAddSub, op, depth);
        } else break;
    }
}

static void compileRelComp(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileShift(tok, op, depth);
    while (tok) {
        if (Token::Match(tok, "<|<=|>=|>")) {
            compileBinOp(tok, compileShift, op, depth);
        } else break;
    }
}

static void compileEqComp(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileRelComp(tok, op, depth);
    while (tok) {
        if (Token::Match(tok, "==|!=")) {
            compileBinOp(tok, compileRelComp, op, depth);
        } else break;
    }
}

static void compileAnd(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileEqComp(tok, op, depth);
    while (tok) {
        if (tok->str() == "&" && !tok->astOperand1()) {
            compileBinOp(tok, compileEqComp, op, depth);
        } else break;
    }
}

static void compileXor(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAnd(tok, op, depth);
    while (tok) {
        if (tok->str() == "^") {
            compileBinOp(tok, compileAnd, op, depth);
        } else break;
    }
}

static void compileOr(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileXor(tok, op, depth);
    while (tok) {
        if (tok->str() == "|") {
            compileBinOp(tok, compileXor, op, depth);
        } else break;
    }
}

static void compileLogicAnd(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileOr(tok, op, depth);
    while (tok) {
        if (tok->str() == "&&") {
            compileBinOp(tok, compileOr, op, depth);
        } else break;
    }
}

static void compileLogicOr(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileLogicAnd(tok, op, depth);
    while (tok) {
        if (tok->str() == "||") {
            compileBinOp(tok, compileLogicAnd, op, depth);
        } else break;
    }
}

static void compileAssignTernary(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileLogicOr(tok, op, depth);
    while (tok) {
        // TODO: http://en.cppreference.com/w/cpp/language/operator_precedence says:
        //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
        if (tok->isAssignmentOp() || Token::Match(tok, "[?:]")) {
            if (tok->str() == "?" && tok->strAt(1) == ":") {
                op.push(0);
            }
            compileBinOp(tok, compileAssignTernary, op, depth);
        } else break;
    }
}

static void compileComma(Token *&tok, std::stack<Token*> &op, unsigned int depth)
{
    compileAssignTernary(tok, op, depth);
    while (tok) {
        if (tok->str() == ",") {
            compileBinOp(tok, compileAssignTernary, op, depth);
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
            } else if (Token::Match(tok2, "%var% %op%|(|[|.|:|::") || Token::Match(tok2->previous(), "[(;{}] %cop%|(")) {
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

    if (tok->str() == "return" || !tok->previous() || Token::Match(tok, "%var% %op%|(|[|.|::") || Token::Match(tok->previous(), "[;{}] %cop%|( !!{")) {
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
