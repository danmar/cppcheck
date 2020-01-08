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
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"
#include "utils.h"

#include <memory>
#include <vector>
#include <iostream>

static const std::string ArraySubscriptExpr = "ArraySubscriptExpr";
static const std::string BinaryOperator = "BinaryOperator";
static const std::string CallExpr = "CallExpr";
static const std::string CompoundStmt = "CompoundStmt";
static const std::string DeclRefExpr = "DeclRefExpr";
static const std::string DeclStmt = "DeclStmt";
static const std::string FieldDecl = "FieldDecl";
static const std::string ForStmt = "ForStmt";
static const std::string FunctionDecl = "FunctionDecl";
static const std::string IfStmt = "IfStmt";
static const std::string ImplicitCastExpr = "ImplicitCastExpr";
static const std::string IntegerLiteral = "IntegerLiteral";
static const std::string MemberExpr = "MemberExpr";
static const std::string ParmVarDecl = "ParmVarDecl";
static const std::string RecordDecl = "RecordDecl";
static const std::string ReturnStmt = "ReturnStmt";
static const std::string UnaryOperator = "UnaryOperator";
static const std::string VarDecl = "VarDecl";

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
        else if (line[pos1] == '\'') {
            pos2 = line.find("\'", pos1+1);
            if (line.compare(pos2, 3, "\':\'", 0, 3) == 0)
                pos2 = line.find("\'", pos2 + 3);
        } else
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
        struct Decl {
            Decl(Token *def, Variable *var) : def(def), function(nullptr), var(var) {}
            Decl(Token *def, Function *function) : def(def), function(function), var(nullptr) {}
            void ref(Token *tok) {
                tok->function(function);
                tok->varId(var ? var->declarationId() : 0);
                tok->variable(var);
            }
            Token *def;
            Function *function;
            Variable *var;
        };

        SymbolDatabase *mSymbolDatabase = nullptr;

        void varDecl(const std::string &addr, Token *def, Variable *var) {
            Decl decl(def, var);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            def->varId(++mVarId);
            def->variable(var);
            var->setValueType(ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
        }

        void funcDecl(const std::string &addr, Token *nameToken, Function *function) {
            Decl decl(nameToken, function);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            nameToken->function(function);
        }

        void ref(const std::string &addr, Token *tok) {
            auto it = mDeclMap.find(addr);
            if (it != mDeclMap.end())
                it->second.ref(tok);
        }

        std::vector<const Variable *> getVariableList() const {
            std::vector<const Variable *> ret;
            ret.resize(mVarId + 1, nullptr);
            for (auto it: mDeclMap) {
                if (it.second.var)
                    ret[it.second.var->declarationId()] = it.second.var;
            }
            return ret;
        }

    private:
        std::map<std::string, Decl> mDeclMap;
        int mVarId = 0;
    };

    class AstNode;
    typedef std::shared_ptr<AstNode> AstNodePtr;

    class AstNode {
    public:
        AstNode(const std::string &nodeType, const std::string &ext, Data *data)
            : nodeType(nodeType), mExtTokens(splitString(ext)), mData(data)
        {}
        std::string nodeType;
        std::vector<AstNodePtr> children;

        void setLocations(TokenList *tokenList, int file, int line, int col);

        void dumpAst(int num = 0, int indent = 0) const;
        void createTokens1(TokenList *tokenList) {
            setLocations(tokenList, 0, 1, 1);
            createTokens(tokenList);
            if (nodeType == VarDecl || nodeType == RecordDecl)
                addtoken(tokenList, ";");
        }
    private:
        Token *createTokens(TokenList *tokenList);
        Token *addtoken(TokenList *tokenList, const std::string &str);
        void addTypeTokens(TokenList *tokenList, const std::string &str);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> &children);
        Token *createTokensVarDecl(TokenList *tokenList);
        std::string getSpelling() const;
        std::string getType() const;
        const Scope *getNestedInScope(TokenList *tokenList);

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        int mVarId = 0;
        std::vector<std::string> mExtTokens;
        Data *mData;
    };
}

std::string clangastdump::AstNode::getSpelling() const
{
    return mExtTokens[mExtTokens.size() - 2];
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
        else if (ext.compare(0,6,"<line:") == 0) {
            line = std::atoi(ext.substr(6).c_str());
            if (ext.find(", col:") != std::string::npos)
                col = std::atoi(ext.c_str() + ext.find(", col:") + 6);
        } else if (ext[0] == '<' && ext.find(":") != std::string::npos)
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
    const Scope *scope = getNestedInScope(tokenList);
    tokenList->addtoken(str, mLine, mFile);
    tokenList->back()->column(mCol);
    tokenList->back()->scope(scope);
    if (getType() == "int")
        tokenList->back()->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
    return tokenList->back();
}

void clangastdump::AstNode::addTypeTokens(TokenList *tokenList, const std::string &str)
{
    std::string type;
    if (str.find(" (") != std::string::npos)
        type = str.substr(1,str.find(" (")-1);
    else if (str.find("\':\'") != std::string::npos)
        type = str.substr(1, str.find("\':\'") - 1);
    else
        type = unquote(str);
    for (const std::string &s: splitString(type))
        addtoken(tokenList, s);
}

const Scope *clangastdump::AstNode::getNestedInScope(TokenList *tokenList)
{
    if (!tokenList->back())
        return &mData->mSymbolDatabase->scopeList.front();
    if (tokenList->back()->str() == "}")
        return tokenList->back()->scope()->nestedIn;
    return tokenList->back()->scope();
}

Scope *clangastdump::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode)
{
    std::vector<AstNodePtr> children{astNode};
    return createScope(tokenList, scopeType, children);
}

Scope *clangastdump::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> &children)
{
    SymbolDatabase *symbolDatabase = mData->mSymbolDatabase;

    const Scope *nestedIn = getNestedInScope(tokenList);

    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nestedIn));
    Scope *scope = &symbolDatabase->scopeList.back();
    scope->type = scopeType;
    Token *bodyStart = children[0]->addtoken(tokenList, "{");
    tokenList->back()->scope(scope);
    for (AstNodePtr astNode: children) {
        astNode->createTokens(tokenList);
        if (!Token::Match(tokenList->back(), "[;{}]"))
            astNode->addtoken(tokenList, ";");
    }
    Token *bodyEnd = children.back()->addtoken(tokenList, "}");
    bodyStart->link(bodyEnd);
    scope->bodyStart = bodyStart;
    scope->bodyEnd = bodyEnd;
    return scope;
}

Token *clangastdump::AstNode::createTokens(TokenList *tokenList)
{
    if (nodeType == ArraySubscriptExpr) {
        Token *array = children[0]->createTokens(tokenList);
        Token *bracket1 = addtoken(tokenList, "[");
        Token *index = children[1]->createTokens(tokenList);
        Token *bracket2 = addtoken(tokenList, "]");
        bracket1->astOperand1(array);
        bracket1->astOperand2(index);
        bracket1->link(bracket2);
        return bracket1;
    }
    if (nodeType == BinaryOperator) {
        Token *tok1 = children[0]->createTokens(tokenList);
        Token *binop = addtoken(tokenList, unquote(mExtTokens.back()));
        Token *tok2 = children[1]->createTokens(tokenList);
        binop->astOperand1(tok1);
        binop->astOperand2(tok2);
        return binop;
    }
    if (nodeType == CallExpr) {
        Token *f = children[0]->createTokens(tokenList);
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(f);
        Token *parent = par1;
        for (int c = 1; c < children.size(); ++c) {
            if (c + 1 < children.size()) {
                Token *child = children[c]->createTokens(tokenList);
                Token *comma = addtoken(tokenList, ",");
                comma->astOperand1(child);
                parent->astOperand2(comma);
                parent = comma;
            } else {
                parent->astOperand2(children[c]->createTokens(tokenList));
            }
        }
        par1->link(addtoken(tokenList, ")"));
        return par1;
    }
    if (nodeType == CompoundStmt) {
        for (AstNodePtr child: children) {
            child->createTokens(tokenList);
            child->addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (nodeType == DeclStmt)
        return children[0]->createTokens(tokenList);
    if (nodeType == DeclRefExpr) {
        const std::string addr = mExtTokens[mExtTokens.size() - 3];
        Token *reftok = addtoken(tokenList, unquote(mExtTokens[mExtTokens.size() - 2]));
        mData->ref(addr, reftok);
        return reftok;
    }
    if (nodeType == FieldDecl)
        return createTokensVarDecl(tokenList);
    if (nodeType == ForStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr1 = children[0]->createTokens(tokenList);
        Token *sep1 = addtoken(tokenList, ";");
        Token *expr2 = children[2]->createTokens(tokenList);
        Token *sep2 = addtoken(tokenList, ";");
        Token *expr3 = children[3]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par1->astOperand1(forToken);
        par1->astOperand2(sep1);
        sep1->astOperand1(expr1);
        sep1->astOperand2(sep2);
        sep2->astOperand1(expr2);
        sep2->astOperand2(expr3);
        createScope(tokenList, Scope::ScopeType::eFor, children[4]);
        return nullptr;
    }
    if (nodeType == FunctionDecl) {
        SymbolDatabase *symbolDatabase = mData->mSymbolDatabase;
        addTypeTokens(tokenList, mExtTokens.back());
        Token *nameToken = addtoken(tokenList, mExtTokens[mExtTokens.size() - 2]);
        Scope *nestedIn = const_cast<Scope *>(nameToken->scope());
        symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nestedIn));
        Scope &scope = symbolDatabase->scopeList.back();
        symbolDatabase->functionScopes.push_back(&scope);
        nestedIn->functionList.push_back(Function(nameToken));
        scope.function = &nestedIn->functionList.back();
        scope.type = Scope::ScopeType::eFunction;
        scope.className = nameToken->str();
        mData->funcDecl(mExtTokens.front(), nameToken, scope.function);
        Token *par1 = addtoken(tokenList, "(");
        // Function arguments
        for (AstNodePtr child: children) {
            if (child->nodeType != ParmVarDecl)
                continue;
            if (tokenList->back() != par1)
                addtoken(tokenList, ",");
            addTypeTokens(tokenList, child->mExtTokens.back());
            const std::string spelling = child->getSpelling();
            if (!spelling.empty()) {
                const std::string addr = child->mExtTokens[0];
                Token *vartok = addtoken(tokenList, spelling);
                scope.function->argumentList.push_back(Variable(vartok, nullptr, nullptr, 0, AccessControl::Argument, nullptr, &scope, nullptr));
                mData->varDecl(addr, vartok, &scope.function->argumentList.back());
            }
        }
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        // Function body
        if (!children.empty() && children.back()->nodeType == CompoundStmt) {
            Token *bodyStart = addtoken(tokenList, "{");
            bodyStart->scope(&scope);
            children.back()->createTokens(tokenList);
            Token *bodyEnd = addtoken(tokenList, "}");
            scope.bodyStart = bodyStart;
            scope.bodyEnd = bodyEnd;
            bodyStart->link(bodyEnd);
        } else {
            addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (nodeType == IfStmt) {
        AstNodePtr cond = children[2];
        AstNodePtr then = children[3];
        AstNodePtr else_ = children[4];
        Token *iftok = addtoken(tokenList, "if");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(iftok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        createScope(tokenList, Scope::ScopeType::eIf, then);
        if (else_) {
            else_->addtoken(tokenList, "else");
            createScope(tokenList, Scope::ScopeType::eElse, else_);
        }
        return nullptr;
    }
    if (nodeType == ImplicitCastExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == IntegerLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == MemberExpr) {
        Token *s = children[0]->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        Token *member = addtoken(tokenList, getSpelling().substr(1));
        mData->ref(mExtTokens.back(), member);
        dot->astOperand1(s);
        dot->astOperand2(member);
        return dot;
    }
    if (nodeType == RecordDecl) {
        const Token *classDef = addtoken(tokenList, "struct");
        const std::string &recordName = getSpelling();
        if (!recordName.empty())
            addtoken(tokenList, getSpelling());
        Scope *recordScope = createScope(tokenList, Scope::ScopeType::eStruct, children);
        mData->mSymbolDatabase->typeList.push_back(Type(classDef, recordScope, classDef->scope()));
        recordScope->definedType = &mData->mSymbolDatabase->typeList.back();
        return nullptr;
    }
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
    if (nodeType == VarDecl)
        return createTokensVarDecl(tokenList);
    return addtoken(tokenList, "?" + nodeType + "?");
}

Token * clangastdump::AstNode::createTokensVarDecl(TokenList *tokenList)
{
    bool isInit = mExtTokens.back() == "cinit";
    const std::string addr = mExtTokens.front();
    const std::string type = isInit ? mExtTokens[mExtTokens.size() - 2] : mExtTokens.back();
    const std::string name = isInit ? mExtTokens[mExtTokens.size() - 3] : mExtTokens[mExtTokens.size() - 2];
    addTypeTokens(tokenList, type);
    Token *vartok1 = addtoken(tokenList, name);
    Scope *scope = const_cast<Scope *>(tokenList->back()->scope());
    const AccessControl accessControl = (scope->type == Scope::ScopeType::eGlobal) ? (AccessControl::Global) : (AccessControl::Local);
    scope->varlist.push_back(Variable(vartok1, type, 0, accessControl, nullptr, scope));
    mData->varDecl(addr, vartok1, &scope->varlist.back());
    if (isInit) {
        Token *eq = addtoken(tokenList, "=");
        eq->astOperand1(vartok1);
        eq->astOperand2(children.back()->createTokens(tokenList));
        return eq;
    }
    return vartok1;
}

void clangastdump::parseClangAstDump(Tokenizer *tokenizer, std::istream &f)
{
    TokenList *tokenList = &tokenizer->list;

    tokenizer->createSymbolDatabase();
    SymbolDatabase *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer->getSymbolDatabase());
    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nullptr));
    symbolDatabase->scopeList.back().type = Scope::ScopeType::eGlobal;

    clangastdump::Data data;
    data.mSymbolDatabase = symbolDatabase;
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

        if (pos1 == 1 && endsWith(nodeType, "Decl", 4) && nodeType != "TypedefDecl") {
            if (!tree.empty())
                tree[0]->createTokens1(tokenList);
            tree.clear();
            tree.push_back(std::make_shared<AstNode>(nodeType, ext, &data));
            continue;
        }

        const int level = (pos1 - 1) / 2;
        if (level == 0 || tree.empty())
            continue;

        AstNodePtr newNode = std::make_shared<AstNode>(nodeType, ext, &data);
        tree[level - 1]->children.push_back(newNode);
        if (level >= tree.size())
            tree.push_back(newNode);
        else
            tree[level] = newNode;
    }

    if (!tree.empty())
        tree[0]->createTokens1(tokenList);

    symbolDatabase->clangSetVariables(data.getVariableList());
    tokenList->clangSetOrigFiles();
}

