/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

#include "clangastdump.h"
#include "symboldatabase.h"
#include "tokenize.h"

#include <memory>
#include <vector>
#include <iostream>

static const std::string BinaryOperator = "BinaryOperator";
static const std::string CallExpr = "CallExpr";
static const std::string CompoundStmt = "CompoundStmt";
static const std::string DeclRefExpr = "DeclRefExpr";
static const std::string FunctionDecl = "FunctionDecl";
static const std::string IfStmt = "IfStmt";
static const std::string ImplicitCastExpr = "ImplicitCastExpr";
static const std::string IntegerLiteral = "IntegerLiteral";
static const std::string ParmVarDecl = "ParmVarDecl";
static const std::string ReturnStmt = "ReturnStmt";
static const std::string UnaryOperator = "UnaryOperator";

static std::string unquote(const std::string &s)
{
    return (s[0] == '\'') ? s.substr(1, s.size() - 2) : s;
}

static std::vector<std::string> splitString(const std::string &line)
{
    std::vector<std::string> ret;
    std::string::size_type pos1 = line.find_first_not_of(" ");
    while (pos1 != std::string::npos) {
        std::string::size_type pos2;
        if (line[pos1] == '<')
            pos2 = line.find(">", pos1);
        else if (line[pos1] == '\'')
            pos2 = line.find("\'", pos1+1);
        else
            pos2 = line.find(" ", pos1) - 1;
        ret.push_back(line.substr(pos1, pos2+1-pos1));
        if (pos2 == std::string::npos)
            break;
        pos1 = line.find_first_not_of(" ", pos2 + 1);
    }
    return ret;
}

namespace clangastdump {
    struct Data {
        std::map<std::string, int> varId;
        std::map<std::string, Variable *> variableMap;
    };

    class AstNode {
    public:
        AstNode(const std::string &nodeType, const std::string &ext, Data *data, SymbolDatabase *symbolDatabase)
            : nodeType(nodeType), mExtTokens(splitString(ext)), mData(data), mSymbolDatabase(symbolDatabase)
        {}
        std::string nodeType;
        std::vector<std::shared_ptr<AstNode>> children;

        void setLocations(TokenList *tokenList, int file, int line, int col);

        void dumpAst(int num = 0, int indent = 0) const;
        Token *createTokens(TokenList *tokenList);
    private:
        Token *addtoken(TokenList *tokenList, const std::string &str);
        Token *addTypeTokens(TokenList *tokenList, const std::string &str);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNode *astNode);
        std::string getSpelling() const;
        std::string getType() const;

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        int mVarId = 0;
        std::vector<std::string> mExtTokens;
        Data *mData;
        SymbolDatabase *mSymbolDatabase;
    };

    typedef std::shared_ptr<AstNode> AstNodePtr;
}

std::string clangastdump::AstNode::getSpelling() const
{
    if (nodeType == ParmVarDecl)
        return mExtTokens[mExtTokens.size() - 2];
    return "";
}

std::string clangastdump::AstNode::getType() const
{
    if (nodeType == DeclRefExpr)
        return unquote(mExtTokens.back());
    if (nodeType == BinaryOperator)
        return unquote(mExtTokens[mExtTokens.size() - 2]);
    if (nodeType == IntegerLiteral)
        return unquote(mExtTokens[mExtTokens.size() - 2]);
    return "";
}

void clangastdump::AstNode::dumpAst(int num, int indent) const
{
    (void)num;
    std::cout << std::string(indent, ' ') << nodeType;
    for (auto tok: mExtTokens)
        std::cout << " " << tok;
    std::cout << std::endl;
    for (int c = 0; c < children.size(); ++c)
        children[c]->dumpAst(c, indent + 2);
}

void clangastdump::AstNode::setLocations(TokenList *tokenList, int file, int line, int col)
{
    for (const std::string &ext: mExtTokens) {
        if (ext.compare(0,5,"<col:") == 0)
            col = std::atoi(ext.substr(5).c_str());
        else if (ext.compare(0,6,"<line:") == 0)
            line = std::atoi(ext.substr(6).c_str());
        else if (ext[0] == '<' && ext.find(":") != std::string::npos)
            file = tokenList->appendFileIfNew(ext.substr(1,ext.find(":") - 1));
    }
    mFile = file;
    mLine = line;
    mCol = col;
    for (auto child: children) {
        if (child)
            child->setLocations(tokenList, file, line, col);
    }
}

Token *clangastdump::AstNode::addtoken(TokenList *tokenList, const std::string &str)
{
    const Scope *scope;
    if (!tokenList->back())
        scope = &mSymbolDatabase->scopeList.front();
    else
        scope = tokenList->back()->scope();
    tokenList->addtoken(str, mLine, mFile);
    tokenList->back()->scope(scope);
    if (getType() == "int")
        tokenList->back()->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
    return tokenList->back();
}

Token *clangastdump::AstNode::addTypeTokens(TokenList *tokenList, const std::string &str)
{
    if (str.find(" (") == std::string::npos)
        return addtoken(tokenList, unquote(str));
    return addtoken(tokenList, str.substr(1,str.find(" (")-1));
}

Scope *clangastdump::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNode *astNode)
{
    const Scope *nestedIn;
    if (!tokenList->back())
        nestedIn = &mSymbolDatabase->scopeList.front();
    else if (tokenList->back()->str() == "}")
        nestedIn = tokenList->back()->link()->previous()->scope();
    else
        nestedIn = tokenList->back()->scope();
    mSymbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nestedIn));
    Scope *scope = &mSymbolDatabase->scopeList.back();
    scope->type = scopeType;
    Token *bodyStart = addtoken(tokenList, "{");
    tokenList->back()->scope(scope);
    astNode->createTokens(tokenList);
    addtoken(tokenList, ";");
    Token *bodyEnd = addtoken(tokenList, "}");
    bodyStart->link(bodyEnd);
    scope->bodyStart = bodyStart;
    scope->bodyEnd = bodyEnd;
    return scope;
}

Token *clangastdump::AstNode::createTokens(TokenList *tokenList)
{
    if (nodeType == BinaryOperator) {
        Token *tok1 = children[0]->createTokens(tokenList);
        Token *binop = addtoken(tokenList, unquote(mExtTokens.back()));
        Token *tok2 = children[1]->createTokens(tokenList);
        binop->astOperand1(tok1);
        binop->astOperand2(tok2);
        return binop;
    }
    if (nodeType == CallExpr) {
        Token *op1 = children[0]->createTokens(tokenList);
        Token *call = addtoken(tokenList, "(");
        call->astOperand1(op1);
        for (int c = 1; c < children.size(); ++c)
            call->astOperand2(children[c]->createTokens(tokenList));
        call->link(addtoken(tokenList, ")"));
        return call;
    }
    if (nodeType == CompoundStmt) {
        bool first = true;
        for (AstNodePtr child: children) {
            if (!first)
                child->addtoken(tokenList, ";");
            first = false;
            child->createTokens(tokenList);
        }
        return nullptr;
    }
    if (nodeType == DeclRefExpr) {
        Token *vartok = addtoken(tokenList, unquote(mExtTokens[mExtTokens.size() - 2]));
        std::string addr = mExtTokens[mExtTokens.size() - 3];
        vartok->varId(mData->varId[addr]);
        vartok->variable(mData->variableMap[addr]);
        return vartok;
    }
    if (nodeType == FunctionDecl) {
        addTypeTokens(tokenList, mExtTokens.back());
        Token *nameToken = addtoken(tokenList, mExtTokens[mExtTokens.size() - 2]);
        Scope &globalScope = mSymbolDatabase->scopeList.front();
        mSymbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, &globalScope));
        Scope &scope = mSymbolDatabase->scopeList.back();
        mSymbolDatabase->functionScopes.push_back(&scope);
        globalScope.functionList.push_back(Function(nameToken));
        scope.function = &globalScope.functionList.back();
        scope.type = Scope::ScopeType::eFunction;
        Token *par1 = addtoken(tokenList, "(");
        for (AstNodePtr child: children) {
            if (child->nodeType != ParmVarDecl)
                continue;
            if (tokenList->back() != par1)
                addtoken(tokenList, ",");
            addTypeTokens(tokenList, child->mExtTokens.back());
            const std::string spelling = child->getSpelling();
            if (!spelling.empty()) {
                Token *vartok = addtoken(tokenList, spelling);
                std::string addr = child->mExtTokens[0];
                int varId = mData->varId.size() + 1;
                mData->varId[addr] = varId;
                vartok->varId(varId);
                scope.function->argumentList.push_back(Variable(vartok, nullptr, nullptr, varId, AccessControl::Argument, nullptr, nullptr, nullptr));
                Variable *var = &scope.function->argumentList.back();
                mData->variableMap[addr] = var;
                vartok->variable(var);
                var->setValueType(ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
            }
        }
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        Token *bodyStart = addtoken(tokenList, "{");
        bodyStart->scope(&scope);
        children.back()->createTokens(tokenList);
        Token *bodyEnd = addtoken(tokenList, "}");
        scope.bodyStart = bodyStart;
        scope.bodyEnd = bodyEnd;
        bodyStart->link(bodyEnd);
        return nullptr;
    }
    if (nodeType == IfStmt) {
        AstNode *cond = children[2].get();
        AstNode *then = children[3].get();
        Token *iftok = addtoken(tokenList, "if");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(iftok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        createScope(tokenList, Scope::ScopeType::eIf, then);
        return nullptr;
    }
    if (nodeType == ImplicitCastExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == IntegerLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == ReturnStmt) {
        Token *tok1 = addtoken(tokenList, "return");
        if (!children.empty())
            tok1->astOperand1(children[0]->createTokens(tokenList));
        return tok1;
    }
    if (nodeType == UnaryOperator) {
        Token *unop = addtoken(tokenList, unquote(mExtTokens.back()));
        unop->astOperand1(children[0]->createTokens(tokenList));
        return unop;
    }
    return addtoken(tokenList, "?" + nodeType + "?");
}

void clangastdump::parseClangAstDump(Tokenizer *tokenizer, std::istream &f)
{
    TokenList *tokenList = &tokenizer->list;
    clangastdump::Data data;

    tokenizer->createSymbolDatabase();
    SymbolDatabase *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer->getSymbolDatabase());
    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nullptr));
    symbolDatabase->scopeList.back().type = Scope::ScopeType::eGlobal;

    std::string line;
    std::vector<AstNodePtr> tree;
    while (std::getline(f,line)) {
        const std::string::size_type pos1 = line.find("-");
        if (pos1 == std::string::npos)
            continue;
        if (!tree.empty() && line.substr(pos1) == "-<<<NULL>>>") {
            const int level = (pos1 - 1) / 2;
            tree[level - 1]->children.push_back(nullptr);
            continue;
        }
        const std::string::size_type pos2 = line.find(" ", pos1);
        if (pos2 < pos1 + 4 || pos2 == std::string::npos)
            continue;
        const std::string nodeType = line.substr(pos1+1, pos2 - pos1 - 1);
        const std::string ext = line.substr(pos2);

        if (nodeType == FunctionDecl) {
            if (!tree.empty()) {
                tree[0]->setLocations(tokenList, 0, 1, 1);
                tree[0]->createTokens(tokenList);
            }
            tree.clear();
            tree.push_back(std::make_shared<AstNode>(nodeType, ext, &data, symbolDatabase));
            continue;
        }

        const int level = (pos1 - 1) / 2;
        if (level == 0 || tree.empty())
            continue;

        AstNodePtr newNode = std::make_shared<AstNode>(nodeType, ext, &data, symbolDatabase);
        tree[level - 1]->children.push_back(newNode);
        if (level >= tree.size())
            tree.push_back(newNode);
        else
            tree[level] = newNode;
    }

    if (!tree.empty()) {
        tree[0]->setLocations(tokenList, 0, 1, 1);
        tree[0]->createTokens(tokenList);
    }

    symbolDatabase->clangSetVariables(data.variableMap);
    tokenList->clangSetOrigFiles();
}

