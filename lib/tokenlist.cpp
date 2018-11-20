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

//---------------------------------------------------------------------------
#include "tokenlist.h"

#include "errorlogger.h"
#include "mathlib.h"
#include "path.h"
#include "settings.h"
#include "token.h"

#include <simplecpp.h>
#include <cctype>
#include <cstring>
#include <set>
#include <stack>

// How many compileExpression recursions are allowed?
// For practical code this could be endless. But in some special torture test
// there needs to be a limit.
static const unsigned int AST_MAX_DEPTH = 50U;


TokenList::TokenList(const Settings* settings) :
    mTokensFrontBack(),
    mSettings(settings),
    mIsC(false),
    mIsCpp(false)
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
    deleteTokens(mTokensFrontBack.front);
    mTokensFrontBack.front = nullptr;
    mTokensFrontBack.back = nullptr;
    mFiles.clear();
}

unsigned int TokenList::appendFileIfNew(const std::string &fileName)
{
    // Has this file been tokenized already?
    for (std::size_t i = 0; i < mFiles.size(); ++i)
        if (Path::sameFileName(mFiles[i], fileName))
            return (unsigned int)i;

    // The "mFiles" vector remembers what files have been tokenized..
    mFiles.push_back(Path::simplifyPath(fileName));

    // Update mIsC and mIsCpp properties
    if (mFiles.size() == 1) { // Update only useful if first file added to _files
        if (!mSettings) {
            mIsC = Path::isC(getSourceFilePath());
            mIsCpp = Path::isCPP(getSourceFilePath());
        } else {
            mIsC = mSettings->enforcedLang == Settings::C || (mSettings->enforcedLang == Settings::None && Path::isC(getSourceFilePath()));
            mIsCpp = mSettings->enforcedLang == Settings::CPP || (mSettings->enforcedLang == Settings::None && Path::isCPP(getSourceFilePath()));
        }
    }
    return mFiles.size() - 1U;
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
    const bool isHex = MathLib::isIntHex(str) ;
    if (isHex || MathLib::isOct(str) || MathLib::isBin(str)) {
        // TODO: It would be better if TokenList didn't simplify hexadecimal numbers
        std::string suffix;
        if (isHex &&
            str.size() == (2 + mSettings->int_bit / 4) &&
            (str[2] >= '8') &&  // includes A-F and a-f
            MathLib::getSuffix(str).empty()
           )
            suffix = "U";
        str = MathLib::value(str).str() + suffix;
    }

    if (mTokensFrontBack.back) {
        mTokensFrontBack.back->insertToken(str);
    } else {
        mTokensFrontBack.front = new Token(&mTokensFrontBack);
        mTokensFrontBack.back = mTokensFrontBack.front;
        mTokensFrontBack.back->str(str);
    }

    if (isCPP() && str == "delete")
        mTokensFrontBack.back->isKeyword(true);
    mTokensFrontBack.back->linenr(lineno);
    mTokensFrontBack.back->fileIndex(fileno);
}

void TokenList::addtoken(const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    if (tok == nullptr)
        return;

    if (mTokensFrontBack.back) {
        mTokensFrontBack.back->insertToken(tok->str(), tok->originalName());
    } else {
        mTokensFrontBack.front = new Token(&mTokensFrontBack);
        mTokensFrontBack.back = mTokensFrontBack.front;
        mTokensFrontBack.back->str(tok->str());
        if (!tok->originalName().empty())
            mTokensFrontBack.back->originalName(tok->originalName());
    }

    mTokensFrontBack.back->linenr(lineno);
    mTokensFrontBack.back->fileIndex(fileno);
    mTokensFrontBack.back->flags(tok->flags());
}


//---------------------------------------------------------------------------
// copyTokens - Copy and insert tokens
//---------------------------------------------------------------------------

Token *TokenList::copyTokens(Token *dest, const Token *first, const Token *last, bool one_line)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    unsigned int linenrs = dest->linenr();
    const unsigned int commonFileIndex = dest->fileIndex();
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(commonFileIndex);
        tok2->linenr(linenrs);
        tok2->tokType(tok->tokType());
        tok2->flags(tok->flags());
        tok2->varId(tok->varId());

        // Check for links and fix them up
        if (Token::Match(tok2, "(|[|{"))
            links.push(tok2);
        else if (Token::Match(tok2, ")|]|}")) {
            if (links.empty())
                return tok2;

            Token * link = links.top();

            tok2->link(link);
            link->link(tok2);

            links.pop();
        }
        if (!one_line && tok->next())
            linenrs += tok->next()->linenr() - tok->linenr();
    }
    return tok2;
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

    simplecpp::OutputList outputList;
    simplecpp::TokenList tokens(code, mFiles, file0, &outputList);

    createTokens(&tokens);

    return outputList.empty();
}

//---------------------------------------------------------------------------

void TokenList::createTokens(const simplecpp::TokenList *tokenList)
{
    if (tokenList->cfront())
        mOrigFiles = mFiles = tokenList->cfront()->location.files;
    else
        mFiles.clear();

    mIsC = mIsCpp = false;
    if (!mFiles.empty()) {
        mIsC = Path::isC(getSourceFilePath());
        mIsCpp = Path::isCPP(getSourceFilePath());
    }
    if (mSettings && mSettings->enforcedLang != Settings::None) {
        mIsC = (mSettings->enforcedLang == Settings::C);
        mIsCpp = (mSettings->enforcedLang == Settings::CPP);
    }

    for (const simplecpp::Token *tok = tokenList->cfront(); tok; tok = tok->next) {

        std::string str = tok->str();

        // Replace hexadecimal value with decimal
        // TODO: Remove this
        const bool isHex = MathLib::isIntHex(str) ;
        if (isHex || MathLib::isOct(str) || MathLib::isBin(str)) {
            // TODO: It would be better if TokenList didn't simplify hexadecimal numbers
            std::string suffix;
            if (isHex &&
                mSettings &&
                str.size() == (2 + mSettings->int_bit / 4) &&
                (str[2] >= '8') &&  // includes A-F and a-f
                MathLib::getSuffix(str).empty()
               )
                suffix = "U";
            str = MathLib::value(str).str() + suffix;
        }

        // Float literal
        if (str.size() > 1 && str[0] == '.' && std::isdigit(str[1]))
            str = '0' + str;

        if (mTokensFrontBack.back) {
            mTokensFrontBack.back->insertToken(str);
        } else {
            mTokensFrontBack.front = new Token(&mTokensFrontBack);
            mTokensFrontBack.back = mTokensFrontBack.front;
            mTokensFrontBack.back->str(str);
        }

        if (isCPP() && mTokensFrontBack.back->str() == "delete")
            mTokensFrontBack.back->isKeyword(true);
        mTokensFrontBack.back->fileIndex(tok->location.fileIndex);
        mTokensFrontBack.back->linenr(tok->location.line);
        mTokensFrontBack.back->col(tok->location.col);
        mTokensFrontBack.back->isExpandedMacro(!tok->macro.empty());
    }

    if (mSettings && mSettings->relativePaths) {
        for (std::size_t i = 0; i < mFiles.size(); i++)
            mFiles[i] = Path::getRelativePath(mFiles[i], mSettings->basePaths);
    }

    Token::assignProgressValues(mTokensFrontBack.front);
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
    bool inCase; // true from case to :
    explicit AST_state(bool cpp_) : depth(0), inArrayAssignment(0), cpp(cpp_), assign(0U), inCase(false) {}
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

    if (Token::simpleMatch(tok->link(), ") ( )"))
        return false;

    if (tok->previous() && tok->previous()->isName() && tok->previous()->str() != "return")
        return false;

    if (Token::simpleMatch(tok->previous(), ">") && tok->previous()->link())
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    if (Token::Match(tok->link(), ") }|)|]"))
        return false;

    if (Token::Match(tok->link(), ") %cop%") && !Token::Match(tok->link(), ") [&*+-~]"))
        return false;

    if (Token::Match(tok->previous(), "= ( %name% ) {") && tok->next()->varId() == 0)
        return true;

    bool type = false;
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
        if (tok2->varId() != 0)
            return false;

        while (tok2->link() && Token::Match(tok2, "(|[|<"))
            tok2 = tok2->link()->next();

        if (tok2->str() == ")") {
            if (Token::simpleMatch(tok2, ") (") && Token::simpleMatch(tok2->linkAt(1), ") ."))
                return true;
            return type || tok2->strAt(-1) == "*" || Token::simpleMatch(tok2, ") ~") ||
                   (Token::Match(tok2, ") %any%") &&
                    !tok2->next()->isOp() &&
                    !Token::Match(tok2->next(), "[[]);,?:.]"));
        }
        if (!Token::Match(tok2, "%name%|*|&|::"))
            return false;

        if (tok2->isStandardType() && (tok2->next()->str() != "(" || Token::Match(tok2->next(), "( * *| )")))
            type = true;
    }

    return false;
}

// int(1), int*(2), ..
static Token * findCppTypeInitPar(Token *tok)
{
    if (!tok || !Token::Match(tok->previous(), "[,()] %name%"))
        return nullptr;
    bool istype = false;
    while (Token::Match(tok, "%name%|::|<")) {
        if (tok->str() == "<") {
            tok = tok->link();
            if (!tok)
                return nullptr;
        }
        istype |= tok->isStandardType();
        tok = tok->next();
    }
    if (!istype)
        return nullptr;
    if (!Token::Match(tok, "[*&]"))
        return nullptr;
    while (Token::Match(tok, "[*&]"))
        tok = tok->next();
    return (tok && tok->str() == "(") ? tok : nullptr;
}

// X{} X<Y>{} etc
static bool iscpp11init(const Token * const tok)
{
    const Token *nameToken = tok;
    while (nameToken && nameToken->str() == "{") {
        nameToken = nameToken->previous();
        if (nameToken && nameToken->str() == "," && Token::simpleMatch(nameToken->previous(), "} ,"))
            nameToken = nameToken->linkAt(-1);
    }
    if (!nameToken)
        return false;
    if (nameToken->str() == ">" && nameToken->link())
        nameToken = nameToken->link()->previous();

    const Token *endtok = nullptr;
    if (Token::Match(nameToken, "%name% { !!["))
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
    if (state.inArrayAssignment && Token::Match(tok->previous(), "[{,] . %name%")) { // Jump over . in C style struct initialization
        state.op.push(tok);
        tok->astOperand1(tok->next());
        tok = tok->tokAt(2);
    }
    if (state.inArrayAssignment && Token::Match(tok->previous(), "[{,] [ %num%|%name% ]")) {
        state.op.push(tok);
        tok->astOperand1(tok->next());
        tok = tok->tokAt(3);
    }
    if (tok->isLiteral()) {
        state.op.push(tok);
        do {
            tok = tok->next();
        } while (Token::Match(tok, "%name%|%str%"));
    } else if (tok->isName()) {
        if (Token::Match(tok, "return|case") || (state.cpp && tok->str() == "throw")) {
            if (tok->str() == "case")
                state.inCase = true;
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
            if (state.inCase && Token::simpleMatch(tok, ": ;")) {
                state.inCase = false;
                tok = tok->next();
            }
        } else if (Token::Match(tok, "sizeof !!(")) {
            compileUnaryOp(tok, state, compileExpression);
            state.op.pop();
        } else if (state.cpp && findCppTypeInitPar(tok))  { // int(0), int*(123), ..
            tok = findCppTypeInitPar(tok);
            state.op.push(tok);
            tok = tok->tokAt(2);
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
        const Token *prev = tok->previous();
        if (Token::simpleMatch(prev, ") {") && iscast(prev->link()))
            prev = prev->link()->previous();
        if (Token::simpleMatch(tok->link(),"} [")) {
            tok = tok->next();
        } else if (state.cpp && iscpp11init(tok)) {
            if (state.op.empty() || Token::Match(tok->previous(), "[{,]"))
                compileUnaryOp(tok, state, compileExpression);
            else
                compileBinOp(tok, state, compileExpression);
            if (Token::Match(tok, "} ,|:")) {
                tok = tok->next();
            }
        } else if (!state.inArrayAssignment && !Token::simpleMatch(prev, "=")) {
            state.op.push(tok);
            tok = tok->link()->next();
        } else {
            if (tok->link() != tok->next()) {
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                while (Token::Match(tok, "} [,};]") && state.inArrayAssignment > 0U) {
                    tok = tok->next();
                    state.inArrayAssignment--;
                }
            } else {
                state.op.push(tok);
                tok = tok->tokAt(2);
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

    if (tok->str() == "*" && tok->previous()->tokType() == Token::eIncDecOp && isPrefixUnary(tok->previous(), cpp))
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
            if (state.cpp && isPrefixUnary(tok, state.cpp) && Token::Match(tok->link(), "] (|{")) { // Lambda
                // What we do here:
                // - Nest the round bracket under the square bracket.
                // - Nest what follows the lambda (if anything) with the lambda opening [
                // - Compile the content of the lambda function as separate tree (this is done later)
                // this must be consistent with isLambdaCaptureList
                Token* const squareBracket = tok;
                if (Token::simpleMatch(squareBracket->link(), "] (")) {
                    Token* const roundBracket = squareBracket->link()->next();
                    Token* curlyBracket = roundBracket->link()->next();
                    if (Token::Match(curlyBracket, "mutable|const"))
                        curlyBracket = curlyBracket->next();
                    if (curlyBracket && curlyBracket->originalName() == "->") {
                        while (Token::Match(curlyBracket, "%name%|.|::|&|*"))
                            curlyBracket = curlyBracket->next();
                    }
                    if (curlyBracket && curlyBracket->str() == "{") {
                        squareBracket->astOperand1(roundBracket);
                        roundBracket->astOperand1(curlyBracket);
                        state.op.push(squareBracket);
                        tok = curlyBracket->link()->next();
                        continue;
                    }
                } else {
                    Token* const curlyBracket = squareBracket->link()->next();
                    squareBracket->astOperand1(curlyBracket);
                    state.op.push(squareBracket);
                    tok = curlyBracket->link()->next();
                    continue;
                }
            }

            const Token* const tok2 = tok;
            if (tok->strAt(1) != "]")
                compileBinOp(tok, state, compileExpression);
            else
                compileUnaryOp(tok, state, compileExpression);
            tok = tok2->link()->next();
        } else if (tok->str() == "(" && (!iscast(tok) || Token::Match(tok->previous(), "if|while|for|switch|catch"))) {
            Token* tok2 = tok;
            tok = tok->next();
            const bool opPrevTopSquare = !state.op.empty() && state.op.top() && state.op.top()->str() == "[";
            const std::size_t oldOpSize = state.op.size();
            compileExpression(tok, state);
            tok = tok2;
            if ((tok->previous() && tok->previous()->isName() && (!Token::Match(tok->previous(), "return|case") && (!state.cpp || !Token::Match(tok->previous(), "throw|delete"))))
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
            Token* castTok = tok;
            tok = tok->link()->next();
            compilePrecedence3(tok, state);
            compileUnaryOp(castTok, state, nullptr);
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
                state.op.push(nullptr);
            }
            const unsigned int assign = state.assign;
            state.assign = 0U;
            compileBinOp(tok, state, compileAssignTernary);
            state.assign = assign;
        } else if (tok->str() == ":") {
            if (state.depth == 1U && state.inCase) {
                state.inCase = false;
                tok = tok->next();
                break;
            }
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
            if (Token::simpleMatch(tok, ", }"))
                tok = tok->next();
            else
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
    // `-(  <<-- optional
    //   `-{
    // see compilePrecedence2
    if (tok->str() != "[")
        return false;
    if (!Token::Match(tok->link(), "] (|{"))
        return false;
    if (Token::simpleMatch(tok->astOperand1(), "{") && tok->astOperand1() == tok->link()->next())
        return true;
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
        if (tok->str() == "{" && !iscpp11init(tok)) {
            if (Token::simpleMatch(tok->astOperand1(), ","))
                continue;
            if (Token::simpleMatch(tok->previous(), "( {"))
                ;
            // struct assignment
            else if (Token::simpleMatch(tok->previous(), ") {") && Token::simpleMatch(tok->linkAt(-1), "( struct"))
                continue;
            // Lambda function
            else if (Token::simpleMatch(tok->astParent(), "(") &&
                     Token::simpleMatch(tok->astParent()->astParent(), "[") &&
                     tok->astParent()->astParent()->astOperand1() &&
                     tok == tok->astParent()->astParent()->astOperand1()->astOperand1())
                ;
            else {
                // function argument is initializer list?
                const Token *parent = tok->astParent();
                while (Token::simpleMatch(parent, ","))
                    parent = parent->astParent();
                if (!parent || !Token::Match(parent->previous(), "%name% ("))
                    // not function argument..
                    continue;
            }

            if (Token::simpleMatch(tok->previous(), "( { ."))
                break;

            const Token * const endToken2 = tok->link();
            for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                tok = createAstAtToken(tok, cpp);
        } else if (tok->str() == "[") {
            if (isLambdaCaptureList(tok)) {
                tok = const_cast<Token *>(tok->astOperand1());
                if (tok->str() == "(")
                    tok = const_cast<Token *>(tok->astOperand1());
                const Token * const endToken2 = tok->link();
                for (; tok && tok != endToken && tok != endToken2; tok = tok ? tok->next() : nullptr)
                    tok = createAstAtToken(tok, cpp);
            }
        }
    }
}

static Token * findAstTop(Token *tok1, Token *tok2)
{
    for (Token *tok = tok1; tok && (tok != tok2); tok = tok->next()) {
        if (tok->astParent() || tok->astOperand1() || tok->astOperand2())
            return const_cast<Token *>(tok->astTop());
        if (Token::simpleMatch(tok, "( {"))
            tok = tok->link();
    }
    for (Token *tok = tok1; tok && (tok != tok2); tok = tok->next()) {
        if (tok->isName() || tok->isNumber())
            return tok;
        if (Token::simpleMatch(tok, "( {"))
            tok = tok->link();
    }
    return nullptr;
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
                init1 = nullptr;
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
        tok2 = findAstTop(semicolon1->next(), semicolon2);
        if (tok2)
            semicolon2->astOperand1(tok2);
        tok2 = findAstTop(semicolon2->next(), endPar);
        if (tok2)
            semicolon2->astOperand2(tok2);
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

    if (Token::Match(tok, "return|case") || (cpp && tok->str() == "throw") || !tok->previous() || Token::Match(tok, "%name% %op%|(|[|.|::|<|?|;") || Token::Match(tok->previous(), "[;{}] %cop%|++|--|( !!{")) {
        if (cpp && (Token::Match(tok->tokAt(-2), "[;{}] new|delete %name%") || Token::Match(tok->tokAt(-3), "[;{}] :: new|delete %name%")))
            tok = tok->previous();

        Token * const tok1 = tok;
        AST_state state(cpp);
        compileExpression(tok, state);
        const Token * const endToken = tok;
        if (endToken == tok1 || !endToken)
            return tok1;

        createAstAtTokenInner(tok1->next(), endToken, cpp);

        return endToken->previous();
    }

    return tok;
}

void TokenList::createAst()
{
    for (Token *tok = mTokensFrontBack.front; tok; tok = tok ? tok->next() : nullptr) {
        tok = createAstAtToken(tok, isCPP());
    }
}

void TokenList::validateAst() const
{
    // Check for some known issues in AST to avoid crash/hang later on
    std::set < const Token* > safeAstTokens; // list of "safe" AST tokens without endless recursion
    for (const Token *tok = mTokensFrontBack.front; tok; tok = tok->next()) {
        // Syntax error if binary operator only has 1 operand
        if ((tok->isAssignmentOp() || tok->isComparisonOp() || Token::Match(tok,"[|^/%]")) && tok->astOperand1() && !tok->astOperand2())
            throw InternalError(tok, "Syntax Error: AST broken, binary operator has only one operand.", InternalError::AST);

        // Syntax error if we encounter "?" with operand2 that is not ":"
        if (tok->astOperand2() && tok->str() == "?" && tok->astOperand2()->str() != ":")
            throw InternalError(tok, "Syntax Error: AST broken, ternary operator lacks ':'.", InternalError::AST);

        // Check for endless recursion
        const Token* parent = tok->astParent();
        if (parent) {
            std::set < const Token* > astTokens; // list of anchestors
            astTokens.insert(tok);
            do {
                if (safeAstTokens.find(parent) != safeAstTokens.end())
                    break;
                if (astTokens.find(parent) != astTokens.end())
                    throw InternalError(tok, "AST broken: endless recursion from '" + tok->str() + "'", InternalError::AST);
                astTokens.insert(parent);
            } while ((parent = parent->astParent()) != nullptr);
            safeAstTokens.insert(astTokens.begin(), astTokens.end());
        } else if (tok->str() == ";") {
            safeAstTokens.clear();
        } else {
            safeAstTokens.insert(tok);
        }

        // Check binary operators
        if (Token::Match(tok, "%or%|%oror%|%assign%|%comp%")) {
            // Skip lambda captures
            if (Token::Match(tok, "= ,|]"))
                continue;
            // Don't check templates
            if (tok->link())
                continue;
            // Skip pure virtual functions
            if (Token::simpleMatch(tok->previous(), ") = 0"))
                continue;
            // Skip operator definitions
            if (Token::simpleMatch(tok->previous(), "operator"))
                continue;
            // Skip incomplete code
            if (!tok->astOperand1() && !tok->astOperand2() && !tok->astParent())
                continue;
            // Skip lambda assignment and/or initializer
            if (Token::Match(tok, "= {|^|["))
                continue;
            // FIXME: Workaround broken AST assignment in type aliases
            if (Token::Match(tok->previous(), "%name% = %name%"))
                continue;
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok, "Syntax Error: AST broken, binary operator '" + tok->str() + "' doesn't have two operands.", InternalError::AST);
        }
    }
}

std::string TokenList::getOrigFile(const Token *tok) const
{
    return mOrigFiles.at(tok->fileIndex());
}

const std::string& TokenList::file(const Token *tok) const
{
    return mFiles.at(tok->fileIndex());
}

std::string TokenList::fileLine(const Token *tok) const
{
    return ErrorLogger::ErrorMessage::FileLocation(tok, this).stringify();
}

bool TokenList::validateToken(const Token* tok) const
{
    if (!tok)
        return true;
    for (const Token *t = mTokensFrontBack.front; t; t = t->next()) {
        if (tok==t)
            return true;
    }
    return false;
}

void TokenList::simplifyStdType()
{
    for (Token *tok = front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "char|short|int|long|unsigned|signed|double|float") || (mSettings->standards.c >= Standards::C99 && Token::Match(tok, "complex|_Complex"))) {
            bool isFloat= false;
            bool isSigned = false;
            bool isUnsigned = false;
            bool isComplex = false;
            unsigned int countLong = 0;
            Token* typeSpec = nullptr;

            Token* tok2 = tok;
            for (; tok2->next(); tok2 = tok2->next()) {
                if (tok2->str() == "long") {
                    countLong++;
                    if (!isFloat)
                        typeSpec = tok2;
                } else if (tok2->str() == "short") {
                    typeSpec = tok2;
                } else if (tok2->str() == "unsigned")
                    isUnsigned = true;
                else if (tok2->str() == "signed")
                    isSigned = true;
                else if (Token::Match(tok2, "float|double")) {
                    isFloat = true;
                    typeSpec = tok2;
                } else if (mSettings->standards.c >= Standards::C99 && Token::Match(tok2, "complex|_Complex"))
                    isComplex = !isFloat || tok2->str() == "_Complex" || Token::Match(tok2->next(), "*|&|%name%"); // Ensure that "complex" is not the variables name
                else if (Token::Match(tok2, "char|int")) {
                    if (!typeSpec)
                        typeSpec = tok2;
                } else
                    break;
            }

            if (!typeSpec) { // unsigned i; or similar declaration
                if (!isComplex) { // Ensure that "complex" is not the variables name
                    tok->str("int");
                    tok->isSigned(isSigned);
                    tok->isUnsigned(isUnsigned);
                }
            } else {
                typeSpec->isLong(typeSpec->isLong() || (isFloat && countLong == 1) || countLong > 1);
                typeSpec->isComplex(typeSpec->isComplex() || (isFloat && isComplex));
                typeSpec->isSigned(typeSpec->isSigned() || isSigned);
                typeSpec->isUnsigned(typeSpec->isUnsigned() || isUnsigned);

                // Remove specifiers
                const Token* tok3 = tok->previous();
                tok2 = tok2->previous();
                while (tok3 != tok2) {
                    if (tok2 != typeSpec &&
                        (isComplex || !Token::Match(tok2, "complex|_Complex")))  // Ensure that "complex" is not the variables name
                        tok2->deleteThis();
                    tok2 = tok2->previous();
                }
            }
        }
    }
}

