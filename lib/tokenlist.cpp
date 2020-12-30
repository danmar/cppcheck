/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include "library.h"
#include "path.h"
#include "settings.h"
#include "standards.h"
#include "token.h"

#include <simplecpp.h>
#include <cctype>
#include <cstring>
#include <set>
#include <stack>

// How many compileExpression recursions are allowed?
// For practical code this could be endless. But in some special torture test
// there needs to be a limit.
static const int AST_MAX_DEPTH = 50;


TokenList::TokenList(const Settings* settings) :
    mTokensFrontBack(),
    mSettings(settings),
    mIsC(false),
    mIsCpp(false)
{
    mTokensFrontBack.list = this;
    mKeywords.insert("auto");
    mKeywords.insert("break");
    mKeywords.insert("case");
    //mKeywords.insert("char"); // type
    mKeywords.insert("const");
    mKeywords.insert("continue");
    mKeywords.insert("default");
    mKeywords.insert("do");
    //mKeywords.insert("double"); // type
    mKeywords.insert("else");
    mKeywords.insert("enum");
    mKeywords.insert("extern");
    //mKeywords.insert("float"); // type
    mKeywords.insert("for");
    mKeywords.insert("goto");
    mKeywords.insert("if");
    mKeywords.insert("inline");
    //mKeywords.insert("int"); // type
    //mKeywords.insert("long"); // type
    mKeywords.insert("register");
    mKeywords.insert("restrict");
    mKeywords.insert("return");
    //mKeywords.insert("short"); // type
    mKeywords.insert("signed");
    mKeywords.insert("sizeof");
    mKeywords.insert("static");
    mKeywords.insert("struct");
    mKeywords.insert("switch");
    mKeywords.insert("typedef");
    mKeywords.insert("union");
    mKeywords.insert("unsigned");
    mKeywords.insert("void");
    mKeywords.insert("volatile");
    mKeywords.insert("while");
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

void TokenList::determineCppC()
{
    if (!mSettings) {
        mIsC = Path::isC(getSourceFilePath());
        mIsCpp = Path::isCPP(getSourceFilePath());
    } else {
        mIsC = mSettings->enforcedLang == Settings::C || (mSettings->enforcedLang == Settings::None && Path::isC(getSourceFilePath()));
        mIsCpp = mSettings->enforcedLang == Settings::CPP || (mSettings->enforcedLang == Settings::None && Path::isCPP(getSourceFilePath()));
    }

    if (mIsCpp) {
        //mKeywords.insert("bool"); // type
        mKeywords.insert("catch");
        mKeywords.insert("class");
        mKeywords.insert("const_cast");
        mKeywords.insert("decltype");
        mKeywords.insert("delete");
        mKeywords.insert("dynamic_cast");
        mKeywords.insert("explicit");
        mKeywords.insert("export");
        //mKeywords.insert("false"); // literal
        mKeywords.insert("friend");
        mKeywords.insert("mutable");
        mKeywords.insert("namespace");
        mKeywords.insert("new");
        mKeywords.insert("operator");
        mKeywords.insert("private");
        mKeywords.insert("protected");
        mKeywords.insert("public");
        mKeywords.insert("reinterpret_cast");
        mKeywords.insert("static_cast");
        mKeywords.insert("template");
        mKeywords.insert("this");
        mKeywords.insert("throw");
        //mKeywords.insert("true"); // literal
        mKeywords.insert("try");
        mKeywords.insert("typeid");
        mKeywords.insert("typename");
        mKeywords.insert("typeof");
        mKeywords.insert("using");
        mKeywords.insert("virtual");
        //mKeywords.insert("wchar_t"); // type
    }
}

int TokenList::appendFileIfNew(const std::string &fileName)
{
    // Has this file been tokenized already?
    for (int i = 0; i < mFiles.size(); ++i)
        if (Path::sameFileName(mFiles[i], fileName))
            return i;

    // The "mFiles" vector remembers what files have been tokenized..
    mFiles.push_back(fileName);

    // Update mIsC and mIsCpp properties
    if (mFiles.size() == 1) { // Update only useful if first file added to _files
        determineCppC();
    }
    return mFiles.size() - 1;
}

void TokenList::clangSetOrigFiles()
{
    mOrigFiles = mFiles;
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

void TokenList::addtoken(std::string str, const nonneg int lineno, const nonneg int column, const nonneg int fileno, bool split)
{
    if (str.empty())
        return;

    // If token contains # characters, split it up
    if (split) {
        size_t begin = 0;
        size_t end = 0;
        while ((end = str.find("##", begin)) != std::string::npos) {
            addtoken(str.substr(begin, end - begin), lineno, fileno, false);
            addtoken("##", lineno, column, fileno, false);
            begin = end+2;
        }
        if (begin != 0) {
            addtoken(str.substr(begin), lineno, column, fileno, false);
            return;
        }
    }

    if (mTokensFrontBack.back) {
        mTokensFrontBack.back->insertToken(str);
    } else {
        mTokensFrontBack.front = new Token(&mTokensFrontBack);
        mTokensFrontBack.back = mTokensFrontBack.front;
        mTokensFrontBack.back->str(str);
    }

    mTokensFrontBack.back->linenr(lineno);
    mTokensFrontBack.back->column(column);
    mTokensFrontBack.back->fileIndex(fileno);
}

void TokenList::addtoken(std::string str, const Token *locationTok)
{
    if (str.empty())
        return;

    if (mTokensFrontBack.back) {
        mTokensFrontBack.back->insertToken(str);
    } else {
        mTokensFrontBack.front = new Token(&mTokensFrontBack);
        mTokensFrontBack.back = mTokensFrontBack.front;
        mTokensFrontBack.back->str(str);
    }

    mTokensFrontBack.back->linenr(locationTok->linenr());
    mTokensFrontBack.back->column(locationTok->column());
    mTokensFrontBack.back->fileIndex(locationTok->fileIndex());
}

void TokenList::addtoken(const Token * tok, const nonneg int lineno, const nonneg int column, const nonneg int fileno)
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
    mTokensFrontBack.back->column(column);
    mTokensFrontBack.back->fileIndex(fileno);
    mTokensFrontBack.back->flags(tok->flags());
}

void TokenList::addtoken(const Token *tok, const Token *locationTok)
{
    if (tok == nullptr || locationTok == nullptr)
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

    mTokensFrontBack.back->flags(tok->flags());
    mTokensFrontBack.back->linenr(locationTok->linenr());
    mTokensFrontBack.back->column(locationTok->column());
    mTokensFrontBack.back->fileIndex(locationTok->fileIndex());
}

void TokenList::addtoken(const Token *tok)
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

    mTokensFrontBack.back->flags(tok->flags());
    mTokensFrontBack.back->linenr(tok->linenr());
    mTokensFrontBack.back->column(tok->column());
    mTokensFrontBack.back->fileIndex(tok->fileIndex());
}


//---------------------------------------------------------------------------
// copyTokens - Copy and insert tokens
//---------------------------------------------------------------------------

Token *TokenList::copyTokens(Token *dest, const Token *first, const Token *last, bool one_line)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    int linenr = dest->linenr();
    const int commonFileIndex = dest->fileIndex();
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(commonFileIndex);
        tok2->linenr(linenr);
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
            linenr += tok->next()->linenr() - tok->linenr();
    }
    return tok2;
}

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void TokenList::insertTokens(Token *dest, const Token *src, nonneg int n)
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
        dest->column(src->column());
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

    createTokens(std::move(tokens));

    return outputList.empty();
}

//---------------------------------------------------------------------------

void TokenList::createTokens(simplecpp::TokenList&& tokenList)
{
    if (tokenList.cfront())
        mOrigFiles = mFiles = tokenList.cfront()->location.files;
    else
        mFiles.clear();

    determineCppC();

    for (const simplecpp::Token *tok = tokenList.cfront(); tok;) {

        std::string str = tok->str();

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

        mTokensFrontBack.back->fileIndex(tok->location.fileIndex);
        mTokensFrontBack.back->linenr(tok->location.line);
        mTokensFrontBack.back->column(tok->location.col);
        mTokensFrontBack.back->isExpandedMacro(!tok->macro.empty());

        tok = tok->next;
        if (tok)
            tokenList.deleteToken(tok->previous);
    }

    if (mSettings && mSettings->relativePaths) {
        for (std::string & mFile : mFiles)
            mFile = Path::getRelativePath(mFile, mSettings->basePaths);
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
        for (char i : tok->str())
            subchecksum2 += (unsigned int)i;
        if (!tok->originalName().empty()) {
            for (char i : tok->originalName())
                subchecksum2 += (unsigned int) i;
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
    int depth;
    int inArrayAssignment;
    bool cpp;
    int assign;
    bool inCase; // true from case to :
    bool stopAtColon; // help to properly parse ternary operators
    const Token *functionCallEndPar;
    explicit AST_state(bool cpp) : depth(0), inArrayAssignment(0), cpp(cpp), assign(0), inCase(false),stopAtColon(false), functionCallEndPar(nullptr) {}
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
        } else if (Token::Match(vartok, "%var% [:=(]")) {
            return vartok;
        } else if (Token::simpleMatch(vartok, "decltype (")) {
            return vartok->linkAt(1)->next();
        }
        vartok = vartok->next();
    }
    return tok;
}

static bool iscast(const Token *tok, bool cpp)
{
    if (!Token::Match(tok, "( ::| %name%"))
        return false;

    if (Token::simpleMatch(tok->link(), ") ( )"))
        return false;

    if (tok->previous() && tok->previous()->isName() && tok->previous()->str() != "return" &&
        (!cpp || tok->previous()->str() != "throw"))
        return false;

    if (Token::simpleMatch(tok->previous(), ">") && tok->previous()->link())
        return false;

    if (Token::Match(tok, "( (| typeof (") && Token::Match(tok->link(), ") %num%"))
        return true;

    if (Token::Match(tok->link(), ") }|)|]|;"))
        return false;

    if (Token::Match(tok->link(), ") %cop%") && !Token::Match(tok->link(), ") [&*+-~!]"))
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
            if (Token::simpleMatch(tok2, ") {") && !type) {
                const Token *tok3 = tok2->linkAt(1);
                while (tok3 != tok2 && Token::Match(tok3, "[{}]"))
                    tok3 = tok3->previous();
                return tok3 != tok2 && tok3->str() != ";";
            }
            return type || tok2->strAt(-1) == "*" || Token::simpleMatch(tok2, ") ~") ||
                   (Token::Match(tok2, ") %any%") &&
                    !tok2->next()->isOp() &&
                    !Token::Match(tok2->next(), "[[]);,?:.]"));
        }

        if (Token::Match(tok2, "&|&& )"))
            return true;

        if (!Token::Match(tok2, "%name%|*|::"))
            return false;

        if (tok2->isStandardType() && (tok2->next()->str() != "(" || Token::Match(tok2->next(), "( * *| )")))
            type = true;
    }

    return false;
}

static Token* findTypeEnd(Token* tok)
{
    while (Token::Match(tok, "%name%|.|::|*|&|&&|<|(|template|decltype|sizeof")) {
        if (Token::Match(tok, "(|<"))
            tok = tok->link();
        if (!tok)
            return nullptr;
        tok = tok->next();
    }
    return tok;
}

static const Token* findTypeEnd(const Token* tok)
{
    return findTypeEnd(const_cast<Token*>(tok));
}

static const Token * findLambdaEndScope(const Token *tok)
{
    if (!Token::simpleMatch(tok, "["))
        return nullptr;
    tok = tok->link();
    if (!Token::Match(tok, "] (|{"))
        return nullptr;
    tok = tok->linkAt(1);
    if (Token::simpleMatch(tok, "}"))
        return tok;
    if (Token::simpleMatch(tok, ") {"))
        return tok->linkAt(1);
    if (!Token::simpleMatch(tok, ")"))
        return nullptr;
    tok = tok->next();
    while (Token::Match(tok, "mutable|constexpr|constval|noexcept|.")) {
        if (Token::simpleMatch(tok, "noexcept ("))
            tok = tok->linkAt(1);
        if (Token::simpleMatch(tok, ".")) {
            tok = findTypeEnd(tok);
            break;
        }
        tok = tok->next();
    }
    if (Token::simpleMatch(tok, "{"))
        return tok->link();
    return nullptr;
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
static bool iscpp11init_impl(const Token * const tok);
static bool iscpp11init(const Token * const tok)
{
    if (tok->isCpp11init() == TokenImpl::Cpp11init::UNKNOWN)
        tok->setCpp11init(iscpp11init_impl(tok));
    return tok->isCpp11init() == TokenImpl::Cpp11init::CPP11INIT;
}

static bool iscpp11init_impl(const Token * const tok)
{
    if (Token::simpleMatch(tok, "{") && Token::simpleMatch(tok->link()->previous(), "; }"))
        return false;
    const Token *nameToken = tok;
    while (nameToken && nameToken->str() == "{") {
        if (nameToken->isCpp11init() != TokenImpl::Cpp11init::UNKNOWN)
            return nameToken->isCpp11init() == TokenImpl::Cpp11init::CPP11INIT;
        nameToken = nameToken->previous();
        if (nameToken && nameToken->str() == "," && Token::simpleMatch(nameToken->previous(), "} ,"))
            nameToken = nameToken->linkAt(-1);
    }
    if (!nameToken)
        return false;
    if (nameToken->str() == ">" && nameToken->link())
        nameToken = nameToken->link()->previous();

    const Token *endtok = nullptr;
    if (Token::Match(nameToken, "%name%|return {") && (!Token::simpleMatch(nameToken->tokAt(2), "[") || findLambdaEndScope(nameToken->tokAt(2))))
        endtok = nameToken->linkAt(1);
    else if (Token::Match(nameToken,"%name% <") && Token::simpleMatch(nameToken->linkAt(1),"> {"))
        endtok = nameToken->linkAt(1)->linkAt(1);
    else if (Token::Match(nameToken->previous(), "%name% ( {"))
        endtok = nameToken->linkAt(1);
    else
        return false;
    if (Token::Match(nameToken, "else|try|do|const|override|volatile|&|&&"))
        return false;
    if (Token::simpleMatch(nameToken->previous(), "namespace"))
        return false;
    if (Token::Match(nameToken, "%any% {")) {
        // If there is semicolon between {..} this is not a initlist
        for (const Token *tok2 = nameToken->next(); tok2 != endtok; tok2 = tok2->next()) {
            if (tok2->str() == ";")
                return false;
            const Token * lambdaEnd = findLambdaEndScope(tok2);
            if (lambdaEnd)
                tok2 = lambdaEnd;
        }
    }
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

static bool isQualifier(const Token* tok)
{
    while (Token::Match(tok, "&|&&|*"))
        tok = tok->next();
    if (!Token::Match(tok, "{|;"))
        return false;
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
            const bool tokIsReturn = tok->str() == "return";
            const bool stopAtColon = state.stopAtColon;
            state.stopAtColon=true;
            compileUnaryOp(tok, state, compileExpression);
            state.stopAtColon=stopAtColon;
            if (tokIsReturn)
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

            if (Token::Match(tok, "{ . %name% =|{")) {
                const int inArrayAssignment = state.inArrayAssignment;
                state.inArrayAssignment = 1;
                compileBinOp(tok, state, compileExpression);
                state.inArrayAssignment = inArrayAssignment;
            } else if (Token::simpleMatch(tok, "{ }")) {
                tok->astOperand1(state.op.top());
                state.op.pop();
                state.op.push(tok);
                tok = tok->tokAt(2);
            }
        } else if (!state.cpp || !Token::Match(tok, "new|delete %name%|*|&|::|(|[")) {
            tok = skipDecl(tok);
            bool repeat = true;
            while (repeat) {
                repeat = false;
                if (Token::Match(tok->next(), "%name%")) {
                    tok = tok->next();
                    repeat = true;
                }
                if (Token::simpleMatch(tok->next(), "<") && Token::Match(tok->linkAt(1), "> %name%")) {
                    tok = tok->next()->link()->next();
                    repeat = true;
                }
            }
            state.op.push(tok);
            if (Token::Match(tok, "%name% <") && tok->linkAt(1))
                tok = tok->linkAt(1);
            else if (Token::Match(tok, "%name% ..."))
                tok = tok->next();
            tok = tok->next();
            if (Token::Match(tok, "%str%")) {
                while (Token::Match(tok, "%name%|%str%"))
                    tok = tok->next();
            }
            if (Token::Match(tok, "%name% %assign%"))
                tok = tok->next();
        }
    } else if (tok->str() == "{") {
        const Token *prev = tok->previous();
        if (Token::simpleMatch(prev, ") {") && iscast(prev->link(), state.cpp))
            prev = prev->link()->previous();
        if (Token::simpleMatch(tok->link(),"} [")) {
            tok = tok->next();
        } else if (state.cpp && iscpp11init(tok)) {
            if (state.op.empty() || Token::Match(tok->previous(), "[{,]") || Token::Match(tok->tokAt(-2), "%name% (")) {
                if (Token::Match(tok, "{ !!}")) {
                    Token *end = tok->link();
                    compileUnaryOp(tok, state, compileExpression);
                    tok = end;
                } else {
                    state.op.push(tok);
                    tok = tok->tokAt(2);
                }
            } else
                compileBinOp(tok, state, compileExpression);
            if (Token::Match(tok, "} ,|:"))
                tok = tok->next();
        } else if (state.cpp && Token::Match(tok->tokAt(-2), "%name% ( {") && !Token::findsimplematch(tok, ";", tok->link())) {
            if (Token::simpleMatch(tok, "{ }"))
                tok = tok->tokAt(2);
            else {
                Token *tok1 = tok;
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                state.inArrayAssignment--;
                tok = tok1->link()->next();
            }
        } else if (!state.inArrayAssignment && !Token::simpleMatch(prev, "=")) {
            state.op.push(tok);
            tok = tok->link()->next();
        } else {
            if (tok->link() != tok->next()) {
                state.inArrayAssignment++;
                compileUnaryOp(tok, state, compileExpression);
                if (Token::Match(tok, "} [,};]") && state.inArrayAssignment > 0) {
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

    return tok->strAt(-1) == ")" && iscast(tok->linkAt(-1), cpp);
}

static void compilePrecedence2(Token *&tok, AST_state& state)
{
    compileScope(tok, state);
    while (tok) {
        if (tok->tokType() == Token::eIncDecOp && !isPrefixUnary(tok, state.cpp)) {
            compileUnaryOp(tok, state, compileScope);
        } else if (tok->str() == "...") {
            state.op.push(tok);
            tok = tok->next();
            break;
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
                // Parse arguments in the capture list
                if (tok->strAt(1) != "]") {
                    Token* tok2 = tok->next();
                    AST_state state2(state.cpp);
                    compileExpression(tok2, state2);
                    if (!state2.op.empty()) {
                        squareBracket->astOperand2(state2.op.top());
                    }
                }

                if (Token::simpleMatch(squareBracket->link(), "] (")) {
                    Token* const roundBracket = squareBracket->link()->next();
                    Token* curlyBracket = roundBracket->link()->next();
                    if (Token::Match(curlyBracket, "mutable|const|noexcept"))
                        curlyBracket = curlyBracket->next();
                    if (curlyBracket && curlyBracket->originalName() == "->")
                        curlyBracket = findTypeEnd(curlyBracket->next());
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
        } else if (tok->str() == "(" && (!iscast(tok, state.cpp) || Token::Match(tok->previous(), "if|while|for|switch|catch"))) {
            Token* tok2 = tok;
            tok = tok->next();
            const bool opPrevTopSquare = !state.op.empty() && state.op.top() && state.op.top()->str() == "[";
            const std::size_t oldOpSize = state.op.size();
            compileExpression(tok, state);
            tok = tok2;
            if ((oldOpSize > 0 && Token::simpleMatch(tok->previous(), "} ("))
                || (tok->previous() && tok->previous()->isName() && !Token::Match(tok->previous(), "return|case") && (!state.cpp || !Token::Match(tok->previous(), "throw|delete")))
                || (tok->strAt(-1) == "]" && (!state.cpp || !Token::Match(tok->linkAt(-1)->previous(), "new|delete")))
                || (tok->strAt(-1) == ">" && tok->linkAt(-1))
                || (tok->strAt(-1) == ")" && !iscast(tok->linkAt(-1), state.cpp)) // Don't treat brackets to clarify precedence as function calls
                || (tok->strAt(-1) == "}" && opPrevTopSquare)) {
                const bool operandInside = oldOpSize < state.op.size();
                if (operandInside)
                    compileBinOp(tok, state, nullptr);
                else
                    compileUnaryOp(tok, state, nullptr);
            }
            tok = tok->link()->next();
        } else if (iscast(tok, state.cpp) && Token::simpleMatch(tok->link(), ") {") && Token::simpleMatch(tok->link()->linkAt(1), "} [")) {
            Token *cast = tok;
            tok = tok->link()->next();
            Token *tok1 = tok;
            compileUnaryOp(tok, state, compileExpression);
            cast->astOperand1(tok1);
            tok = tok1->link()->next();
        } else if (state.cpp && tok->str() == "{" && iscpp11init(tok)) {
            if (Token::simpleMatch(tok, "{ }"))
                compileUnaryOp(tok, state, compileExpression);
            else
                compileBinOp(tok, state, compileExpression);
            while (Token::simpleMatch(tok, "}"))
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
        } else if (tok->str() == "(" && iscast(tok, state.cpp)) {
            Token* castTok = tok;
            castTok->isCast(true);
            tok = tok->link()->next();
            const int inArrayAssignment = state.inArrayAssignment;
            if (tok && tok->str() == "{")
                state.inArrayAssignment = 1;
            compilePrecedence3(tok, state);
            state.inArrayAssignment = inArrayAssignment;
            compileUnaryOp(castTok, state, nullptr);
        } else if (state.cpp && Token::Match(tok, "new %name%|::|(")) {
            Token* newtok = tok;
            tok = tok->next();
            bool innertype = false;
            if (tok->str() == "(") {
                if (Token::Match(tok, "( &| %name%") && Token::Match(tok->link(), ") ( %type%") && Token::simpleMatch(tok->link()->linkAt(1), ") ("))
                    tok = tok->link()->next();
                if (Token::Match(tok->link(), ") ::| %type%")) {
                    if (Token::Match(tok, "( !!)")) {
                        Token *innerTok = tok->next();
                        AST_state innerState(true);
                        compileExpression(innerTok, innerState);
                    }
                    tok = tok->link()->next();
                } else if (Token::Match(tok, "( %type%") && Token::Match(tok->link(), ") [();,[]")) {
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

            Token* leftToken = tok;
            while (Token::Match(tok->next(), ":: %name%")) {
                Token* scopeToken = tok->next(); //The ::
                scopeToken->astOperand1(leftToken);
                scopeToken->astOperand2(scopeToken->next());
                leftToken = scopeToken;
                tok = scopeToken->next();
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
        if (Token::Match(tok, "[/%]") || (tok->str() == "*" && !tok->astOperand1() && !isQualifier(tok))) {
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
        if (tok->str() == "&" && !tok->astOperand1() && !isQualifier(tok)) {
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
        if (tok->str() == "&&" && !isQualifier(tok)) {
            if (!tok->astOperand1()) {
                Token* tok2 = tok->next();
                if (!tok2)
                    break;
                if (state.cpp && Token::Match(tok2, ",|)")) {
                    tok = tok2;
                    break; // rValue reference
                }
            }
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
            if (state.assign > 0)
                state.assign--;
        } else if (tok->str() == "?") {
            // http://en.cppreference.com/w/cpp/language/operator_precedence says about ternary operator:
            //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
            // Hence, we rely on Tokenizer::prepareTernaryOpForAST() to add such parentheses where necessary.
            const bool stopAtColon = state.stopAtColon;
            state.stopAtColon = false;
            if (tok->strAt(1) == ":") {
                state.op.push(nullptr);
            }
            const int assign = state.assign;
            state.assign = 0;
            compileBinOp(tok, state, compileAssignTernary);
            state.assign = assign;
            state.stopAtColon = stopAtColon;
        } else if (tok->str() == ":") {
            if (state.depth == 1U && state.inCase) {
                state.inCase = false;
                tok = tok->next();
                break;
            }
            if (state.stopAtColon)
                break;
            if (state.assign > 0)
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
        } else if (tok->str() == ";" && state.functionCallEndPar && tok->index() < state.functionCallEndPar->index()) {
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
    if (!params->astOperand1() || params->astOperand1()->str() != "{")
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
        } else if (cpp && tok->str() == "[") {
            if (isLambdaCaptureList(tok)) {
                tok = tok->astOperand1();
                if (tok->str() == "(")
                    tok = tok->astOperand1();
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
        if (tok->astParent() || tok->astOperand1() || tok->astOperand2()) {
            while (tok->astParent() && tok->astParent()->index() >= tok1->index() && tok->astParent()->index() <= tok2->index())
                tok = tok->astParent();
            return tok;
        }
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
        if (cpp && Token::Match(tok, "for ( const| auto &|&&| [")) {
            Token *decl = Token::findsimplematch(tok, "[");
            if (Token::simpleMatch(decl->link(), "] :")) {
                AST_state state1(cpp);
                while (decl->str() != "]") {
                    if (Token::Match(decl, "%name% ,|]")) {
                        state1.op.push(decl);
                    } else if (decl->str() == ",") {
                        if (!state1.op.empty()) {
                            decl->astOperand1(state1.op.top());
                            state1.op.pop();
                        }
                        if (!state1.op.empty()) {
                            state1.op.top()->astOperand2(decl);
                            state1.op.pop();
                        }
                        state1.op.push(decl);
                    }
                    decl = decl->next();
                }
                if (state1.op.size() > 1) {
                    Token *lastName = state1.op.top();
                    state1.op.pop();
                    state1.op.top()->astOperand2(lastName);
                }
                decl = decl->next();

                Token *colon = decl;
                compileExpression(decl, state1);

                tok->next()->astOperand1(tok);
                tok->next()->astOperand2(colon);

                return decl;
            }
        }

        Token *tok2 = skipDecl(tok->tokAt(2));
        Token *init1 = nullptr;
        Token * const endPar = tok->next()->link();
        if (tok2 == tok->tokAt(2) && Token::Match(tok2, "%op%|(")) {
            init1 = tok2;
            AST_state state1(cpp);
            compileExpression(tok2, state1);
            if (Token::Match(init1, "( !!{")) {
                for (Token *tok3 = init1; tok3 != tok3->link(); tok3 = tok3->next()) {
                    if (tok3->astParent()) {
                        while (tok3->astParent())
                            tok3 = tok3->astParent();
                        init1 = tok3;
                        break;
                    }
                    if (!Token::Match(tok3, "%op%|(|["))
                        init1 = tok3;
                }
            }
        } else {
            while (tok2 && tok2 != endPar && tok2->str() != ";") {
                if (tok2->str() == "<" && tok2->link()) {
                    tok2 = tok2->link();
                } else if (Token::Match(tok2, "%name% %op%|(|[|.|:|::") || Token::Match(tok2->previous(), "[(;{}] %cop%|(")) {
                    init1 = tok2;
                    AST_state state1(cpp);
                    compileExpression(tok2, state1);
                    if (Token::Match(tok2, ";|)"))
                        break;
                    init1 = nullptr; // cppcheck-suppress redundantAssignment ; FALSE POSITIVE
                }
                if (!tok2) // #7109 invalid code
                    return nullptr;
                tok2 = tok2->next();
            }
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
            semicolon1->astOperand1(init->astTop());
        tok2 = findAstTop(semicolon1->next(), semicolon2);
        if (tok2)
            semicolon2->astOperand1(tok2);
        tok2 = findAstTop(semicolon2->next(), endPar);
        if (tok2)
            semicolon2->astOperand2(tok2);
        else if (!state3.op.empty())
            semicolon2->astOperand2(state3.op.top());

        semicolon1->astOperand2(semicolon2);
        tok->next()->astOperand1(tok);
        tok->next()->astOperand2(semicolon1);

        createAstAtTokenInner(endPar->link(), endPar, cpp);

        return endPar;
    }

    if (cpp && Token::Match(tok, "if|switch (")) {
        Token *semicolon = nullptr;
        Token *tok2;
        for (tok2 = tok->tokAt(2); tok2 && tok2->str() != ")"; tok2 = tok2->next()) {
            if (tok2->str() == ";") {
                if (semicolon)
                    break;
                semicolon = tok2;
            }
            if (tok2->str() == "(")
                tok2 = tok2->link();
        }
        if (semicolon && tok2 == tok->linkAt(1)) {
            tok2 = skipDecl(tok->tokAt(2));
            Token *init1 = tok2;
            AST_state state1(cpp);
            compileExpression(tok2, state1);

            tok2 = semicolon->next();
            Token *expr1 = tok2;
            AST_state state2(cpp);
            compileExpression(tok2, state2);

            semicolon->astOperand1(findAstTop(init1, semicolon->previous()));
            semicolon->astOperand2(findAstTop(expr1, tok2));
            tok->next()->astOperand1(tok);
            tok->next()->astOperand2(semicolon);
        }
    }

    if (Token::simpleMatch(tok, "( {"))
        return tok;

    if (Token::Match(tok, "%type% <") && tok->linkAt(1) && !Token::Match(tok->linkAt(1), "> [({]"))
        return tok->linkAt(1);

    if (Token::Match(tok, "%type% %name%|*|&|::") && tok->str() != "return") {
        int typecount = 0;
        Token *typetok = tok;
        while (Token::Match(typetok, "%type%|::|*|&")) {
            if (typetok->isName() && !Token::simpleMatch(typetok->previous(), "::"))
                typecount++;
            typetok = typetok->next();
        }
        if (Token::Match(typetok, "%var% =") && typetok->varId())
            tok = typetok;

        // Do not create AST for function declaration
        if (typetok &&
            typecount >= 2 &&
            !Token::Match(tok, "return|throw") &&
            Token::Match(typetok->previous(), "%name% (") &&
            typetok->previous()->varId() == 0 &&
            !typetok->previous()->isKeyword() &&
            Token::Match(typetok->link(), ") const|;|{"))
            return typetok;
    }

    if (Token::Match(tok, "return|case") ||
        (cpp && tok->str() == "throw") ||
        !tok->previous() ||
        Token::Match(tok, "%name% %op%|(|[|.|::|<|?|;") ||
        Token::Match(tok->previous(), "[;{}] %cop%|++|--|( !!{") ||
        Token::Match(tok->previous(), "[;{}] %num%|%str%|%char%")) {
        if (cpp && (Token::Match(tok->tokAt(-2), "[;{}] new|delete %name%") || Token::Match(tok->tokAt(-3), "[;{}] :: new|delete %name%")))
            tok = tok->previous();

        Token * const tok1 = tok;
        AST_state state(cpp);
        if (Token::Match(tok, "%name% ("))
            state.functionCallEndPar = tok->linkAt(1);
        compileExpression(tok, state);
        const Token * const endToken = tok;
        if (endToken == tok1 || !endToken)
            return tok1;

        createAstAtTokenInner(tok1->next(), endToken, cpp);

        return endToken->previous();
    }

    if (cpp && tok->str() == "{" && iscpp11init(tok)) {
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
        if (tok->str() == "?") {
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok, "AST broken, ternary operator missing operand(s)", InternalError::AST);
            else if (tok->astOperand2()->str() != ":")
                throw InternalError(tok, "Syntax Error: AST broken, ternary operator lacks ':'.", InternalError::AST);
        }

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

        // Don't check templates
        if (tok->str() == "<" && tok->link()) {
            tok = tok->link();
            continue;
        }

        // Check binary operators
        if (Token::Match(tok, "%or%|%oror%|%assign%|%comp%")) {
            // Skip lambda captures
            if (Token::Match(tok, "= ,|]"))
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

        // Check control blocks
        if (Token::Match(tok->previous(), "if|while|for|switch (")) {
            if (!tok->astOperand1() || !tok->astOperand2())
                throw InternalError(tok,
                                    "Syntax Error: AST broken, '" + tok->previous()->str() +
                                    "' doesn't have two operands.",
                                    InternalError::AST);
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
    return ErrorMessage::FileLocation(tok, this).stringify();
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

void TokenList::simplifyPlatformTypes()
{
    const bool isCPP11  = mSettings->standards.cpp >= Standards::CPP11;

    enum { isLongLong, isLong, isInt } type;

    /** @todo This assumes a flat address space. Not true for segmented address space (FAR *). */

    if (mSettings->sizeof_size_t == mSettings->sizeof_long)
        type = isLong;
    else if (mSettings->sizeof_size_t == mSettings->sizeof_long_long)
        type = isLongLong;
    else if (mSettings->sizeof_size_t == mSettings->sizeof_int)
        type = isInt;
    else
        return;

    for (Token *tok = front(); tok; tok = tok->next()) {
        // pre-check to reduce unneeded match calls
        if (!Token::Match(tok, "std| ::| %type%"))
            continue;
        bool isUnsigned;
        if (Token::Match(tok, "std| ::| size_t|uintptr_t|uintmax_t")) {
            if (isCPP11 && tok->strAt(-1) == "using" && tok->strAt(1) == "=")
                continue;
            isUnsigned = true;
        } else if (Token::Match(tok, "std| ::| ssize_t|ptrdiff_t|intptr_t|intmax_t")) {
            if (isCPP11 && tok->strAt(-1) == "using" && tok->strAt(1) == "=")
                continue;
            isUnsigned = false;
        } else
            continue;

        bool inStd = false;
        if (tok->str() == "::") {
            tok->deleteThis();
        } else if (tok->str() == "std") {
            if (tok->next()->str() != "::")
                continue;
            inStd = true;
            tok->deleteNext();
            tok->deleteThis();
        }

        if (inStd)
            tok->originalName("std::" + tok->str());
        else
            tok->originalName(tok->str());
        if (isUnsigned)
            tok->isUnsigned(true);

        switch (type) {
        case isLongLong:
            tok->isLong(true);
            tok->str("long");
            break;
        case isLong:
            tok->str("long");
            break;
        case isInt:
            tok->str("int");
            break;
        }
    }

    const std::string platform_type(mSettings->platformString());

    for (Token *tok = front(); tok; tok = tok->next()) {
        if (tok->tokType() != Token::eType && tok->tokType() != Token::eName)
            continue;

        const Library::PlatformType * const platformtype = mSettings->library.platform_type(tok->str(), platform_type);

        if (platformtype) {
            // check for namespace
            if (tok->strAt(-1) == "::") {
                const Token * tok1 = tok->tokAt(-2);
                // skip when non-global namespace defined
                if (tok1 && tok1->tokType() == Token::eName)
                    continue;
                tok = tok->previous();
                tok->deleteThis();
            }
            Token *typeToken;
            if (platformtype->mConstPtr) {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken(platformtype->mType);
                typeToken = tok;
            } else if (platformtype->mPointer) {
                tok->str(platformtype->mType);
                typeToken = tok;
                tok->insertToken("*");
            } else if (platformtype->mPtrPtr) {
                tok->str(platformtype->mType);
                typeToken = tok;
                tok->insertToken("*");
                tok->insertToken("*");
            } else {
                tok->originalName(tok->str());
                tok->str(platformtype->mType);
                typeToken = tok;
            }
            if (platformtype->mSigned)
                typeToken->isSigned(true);
            if (platformtype->mUnsigned)
                typeToken->isUnsigned(true);
            if (platformtype->mLong)
                typeToken->isLong(true);
        }
    }
}

void TokenList::simplifyStdType()
{
    for (Token *tok = front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "char|short|int|long|unsigned|signed|double|float") || (mSettings->standards.c >= Standards::C99 && Token::Match(tok, "complex|_Complex"))) {
            bool isFloat= false;
            bool isSigned = false;
            bool isUnsigned = false;
            bool isComplex = false;
            int countLong = 0;
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

bool TokenList::isKeyword(const std::string &str) const
{
    return mKeywords.find(str) != mKeywords.end();
}
