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

#include "clangimport.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"
#include "utils.h"

#include <memory>
#include <vector>
#include <iostream>

static const std::string ArraySubscriptExpr = "ArraySubscriptExpr";
static const std::string BinaryOperator = "BinaryOperator";
static const std::string BreakStmt = "BreakStmt";
static const std::string CallExpr = "CallExpr";
static const std::string CaseStmt = "CaseStmt";
static const std::string CharacterLiteral = "CharacterLiteral";
static const std::string ClassTemplateDecl = "ClassTemplateDecl";
static const std::string ClassTemplateSpecializationDecl = "ClassTemplateSpecializationDecl";
static const std::string ConditionalOperator = "ConditionalOperator";
static const std::string ConstantExpr = "ConstantExpr";
static const std::string CompoundAssignOperator = "CompoundAssignOperator";
static const std::string CompoundStmt = "CompoundStmt";
static const std::string ContinueStmt = "ContinueStmt";
static const std::string CStyleCastExpr = "CStyleCastExpr";
static const std::string CXXBindTemporaryExpr = "CXXBindTemporaryExpr";
static const std::string CXXBoolLiteralExpr = "CXXBoolLiteralExpr";
static const std::string CXXConstructorDecl = "CXXConstructorDecl";
static const std::string CXXConstructExpr = "CXXConstructExpr";
static const std::string CXXDefaultArgExpr = "CXXDefaultArgExpr";
static const std::string CXXDeleteExpr = "CXXDeleteExpr";
static const std::string CXXDestructorDecl = "CXXDestructorDecl";
static const std::string CXXForRangeStmt = "CXXForRangeStmt";
static const std::string CXXMemberCallExpr = "CXXMemberCallExpr";
static const std::string CXXMethodDecl = "CXXMethodDecl";
static const std::string CXXNewExpr = "CXXNewExpr";
static const std::string CXXNullPtrLiteralExpr = "CXXNullPtrLiteralExpr";
static const std::string CXXOperatorCallExpr = "CXXOperatorCallExpr";
static const std::string CXXRecordDecl = "CXXRecordDecl";
static const std::string CXXStaticCastExpr = "CXXStaticCastExpr";
static const std::string CXXStdInitializerListExpr = "CXXStdInitializerListExpr";
static const std::string CXXTemporaryObjectExpr = "CXXTemporaryObjectExpr";
static const std::string CXXThisExpr = "CXXThisExpr";
static const std::string CXXThrowExpr = "CXXThrowExpr";
static const std::string DeclRefExpr = "DeclRefExpr";
static const std::string DeclStmt = "DeclStmt";
static const std::string DoStmt = "DoStmt";
static const std::string EnumConstantDecl = "EnumConstantDecl";
static const std::string EnumDecl = "EnumDecl";
static const std::string ExprWithCleanups = "ExprWithCleanups";
static const std::string FieldDecl = "FieldDecl";
static const std::string FloatingLiteral = "FloatingLiteral";
static const std::string ForStmt = "ForStmt";
static const std::string FunctionDecl = "FunctionDecl";
static const std::string FunctionTemplateDecl = "FunctionTemplateDecl";
static const std::string GotoStmt = "GotoStmt";
static const std::string IfStmt = "IfStmt";
static const std::string ImplicitCastExpr = "ImplicitCastExpr";
static const std::string InitListExpr = "InitListExpr";
static const std::string IntegerLiteral = "IntegerLiteral";
static const std::string LabelStmt = "LabelStmt";
static const std::string MaterializeTemporaryExpr = "MaterializeTemporaryExpr";
static const std::string MemberExpr = "MemberExpr";
static const std::string NamespaceDecl = "NamespaceDecl";
static const std::string NullStmt = "NullStmt";
static const std::string ParenExpr = "ParenExpr";
static const std::string ParmVarDecl = "ParmVarDecl";
static const std::string RecordDecl = "RecordDecl";
static const std::string ReturnStmt = "ReturnStmt";
static const std::string StringLiteral = "StringLiteral";
static const std::string SwitchStmt = "SwitchStmt";
static const std::string TemplateArgument = "TemplateArgument";
static const std::string TypedefDecl = "TypedefDecl";
static const std::string UnaryOperator = "UnaryOperator";
static const std::string UnaryExprOrTypeTraitExpr = "UnaryExprOrTypeTraitExpr";
static const std::string VarDecl = "VarDecl";
static const std::string WhileStmt = "WhileStmt";

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
        else if (line[pos1] == '\"')
            pos2 = line.find("\"", pos1+1);
        else if (line[pos1] == '\'') {
            pos2 = line.find("\'", pos1+1);
            if (pos2 < (int)line.size() - 3 && line.compare(pos2, 3, "\':\'", 0, 3) == 0)
                pos2 = line.find("\'", pos2 + 3);
        } else {
            pos2 = line.find(" ", pos1) - 1;
            if (std::isalpha(line[pos1]) &&
                line.find("<", pos1) < pos2 &&
                line.find("<<",pos1) != line.find("<",pos1) &&
                line.find(">", pos1) != std::string::npos &&
                line.find(">", pos1) > pos2) {
                int level = 0;
                for (pos2 = pos1; pos2 < line.size(); ++pos2) {
                    if (line[pos2] == '<')
                        ++level;
                    else if (line[pos2] == '>') {
                        if (level <= 1)
                            break;
                        --level;
                    }
                }
                if (level > 1 && pos2 + 1 >= line.size())
                    return std::vector<std::string> {};
                pos2 = line.find(" ", pos2);
                if (pos2 != std::string::npos)
                    --pos2;
            }
        }
        if (pos2 == std::string::npos) {
            ret.push_back(line.substr(pos1));
            break;
        }
        ret.push_back(line.substr(pos1, pos2+1-pos1));
        pos1 = line.find_first_not_of(" ", pos2 + 1);
    }
    return ret;
}

namespace clangimport {
    struct Data {
        struct Decl {
            Decl(Token *def, Variable *var) : def(def), enumerator(nullptr), function(nullptr), var(var) {}
            Decl(Token *def, Function *function) : def(def), enumerator(nullptr), function(function), var(nullptr) {}
            Decl(Token *def, Enumerator *enumerator) : def(def), enumerator(enumerator), function(nullptr), var(nullptr) {}
            void ref(Token *tok) {
                if (enumerator)
                    tok->enumerator(enumerator);
                if (function)
                    tok->function(function);
                if (var) {
                    tok->variable(var);
                    tok->varId(var->declarationId());
                }
            }
            Token *def;
            Enumerator *enumerator;
            Function *function;
            Variable *var;
        };

        const Settings *mSettings = nullptr;
        SymbolDatabase *mSymbolDatabase = nullptr;

        int enumValue = 0;

        void enumDecl(const std::string &addr, Token *nameToken, Enumerator *enumerator) {
            Decl decl(nameToken, enumerator);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            nameToken->enumerator(enumerator);
            notFound(addr);
        }

        void funcDecl(const std::string &addr, Token *nameToken, Function *function) {
            Decl decl(nameToken, function);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            nameToken->function(function);
            notFound(addr);
        }

        void varDecl(const std::string &addr, Token *def, Variable *var) {
            Decl decl(def, var);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            def->varId(++mVarId);
            def->variable(var);
            if (def->valueType())
                var->setValueType(*def->valueType());
            notFound(addr);
        }

        void ref(const std::string &addr, Token *tok) {
            auto it = mDeclMap.find(addr);
            if (it != mDeclMap.end())
                it->second.ref(tok);
            else
                mNotFound[addr].push_back(tok);
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

        bool hasDecl(const std::string &addr) const {
            return mDeclMap.find(addr) != mDeclMap.end();
        }

        // "}" tokens that are not end-of-scope
        std::set<Token *> mNotScope;
    private:
        void notFound(const std::string &addr) {
            auto it = mNotFound.find(addr);
            if (it != mNotFound.end()) {
                for (Token *reftok: it->second)
                    ref(addr, reftok);
                mNotFound.erase(it);
            }
        }

        std::map<std::string, Decl> mDeclMap;
        std::map<std::string, std::vector<Token *>> mNotFound;
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
            //dumpAst();
            if (!tokenList->back())
                setLocations(tokenList, 0, 1, 1);
            else
                setLocations(tokenList, tokenList->back()->fileIndex(), tokenList->back()->linenr(), 1);
            createTokens(tokenList);
            if (nodeType == VarDecl || nodeType == RecordDecl || nodeType == TypedefDecl)
                addtoken(tokenList, ";");
            mData->mNotScope.clear();
        }
    private:
        Token *createTokens(TokenList *tokenList);
        Token *addtoken(TokenList *tokenList, const std::string &str, bool valueType=true);
        void addTypeTokens(TokenList *tokenList, const std::string &str);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode, const Token *def);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> &children, const Token *def);
        Token *createTokensCall(TokenList *tokenList);
        void createTokensFunctionDecl(TokenList *tokenList);
        void createTokensForCXXRecord(TokenList *tokenList);
        Token *createTokensVarDecl(TokenList *tokenList);
        std::string getSpelling() const;
        std::string getType(int index = 0) const;
        std::string getTemplateParameters() const;
        const Scope *getNestedInScope(TokenList *tokenList);
        void setValueType(Token *tok);

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        std::vector<std::string> mExtTokens;
        Data *mData;
    };
}

std::string clangimport::AstNode::getSpelling() const
{
    if (nodeType == CompoundAssignOperator) {
        int typeIndex = 1;
        while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
            typeIndex++;
        // name is next quoted token
        int nameIndex = typeIndex + 1;
        while (nameIndex < mExtTokens.size() && mExtTokens[nameIndex][0] != '\'')
            nameIndex++;
        return (nameIndex < mExtTokens.size()) ? unquote(mExtTokens[nameIndex]) : "";
    }

    if (nodeType == UnaryExprOrTypeTraitExpr) {
        int typeIndex = 1;
        while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
            typeIndex++;
        int nameIndex = typeIndex + 1;
        return (nameIndex < mExtTokens.size()) ? unquote(mExtTokens[nameIndex]) : "";
    }

    int typeIndex = mExtTokens.size() - 1;
    if (nodeType == FunctionDecl) {
        while (typeIndex >= 0 && mExtTokens[typeIndex][0] != '\'')
            typeIndex--;
        if (typeIndex <= 0)
            return "";
    }
    const std::string &str = mExtTokens[typeIndex - 1];
    if (str.compare(0,4,"col:") == 0)
        return "";
    if (str.compare(0,8,"<invalid") == 0)
        return "";
    return str;
}

std::string clangimport::AstNode::getType(int index) const
{
    int typeIndex = 1;
    while (typeIndex < mExtTokens.size() && mExtTokens[typeIndex][0] != '\'')
        typeIndex++;
    if (typeIndex >= mExtTokens.size())
        return "";
    std::string type = mExtTokens[typeIndex];
    if (type.find("\':\'") != std::string::npos) {
        if (index == 0)
            type.erase(type.find("\':\'") + 1);
        else
            type.erase(0, type.find("\':\'") + 2);
    }
    if (type.find(" (") != std::string::npos) {
        std::string::size_type pos = type.find(" (");
        type[pos] = '\'';
        type.erase(pos+1);
    }
    if (type.find(" *(") != std::string::npos) {
        std::string::size_type pos = type.find(" *(") + 2;
        type[pos] = '\'';
        type.erase(pos+1);
    }
    if (type.find(" &(") != std::string::npos) {
        std::string::size_type pos = type.find(" &(") + 2;
        type[pos] = '\'';
        type.erase(pos+1);
    }
    return unquote(type);
}

std::string clangimport::AstNode::getTemplateParameters() const
{
    if (children.empty() || children[0]->nodeType != TemplateArgument)
        return "";
    std::string templateParameters;
    for (AstNodePtr child: children) {
        if (child->nodeType == TemplateArgument) {
            if (templateParameters.empty())
                templateParameters = "<";
            else
                templateParameters += ",";
            templateParameters += unquote(child->mExtTokens.back());
        }
    }
    return templateParameters + ">";
}

void clangimport::AstNode::dumpAst(int num, int indent) const
{
    (void)num;
    std::cout << std::string(indent, ' ') << nodeType;
    for (auto tok: mExtTokens)
        std::cout << " " << tok;
    std::cout << std::endl;
    for (int c = 0; c < children.size(); ++c) {
        if (children[c])
            children[c]->dumpAst(c, indent + 2);
        else
            std::cout << std::string(indent + 2, ' ') << "<<<<NULL>>>>>" << std::endl;
    }
}

void clangimport::AstNode::setLocations(TokenList *tokenList, int file, int line, int col)
{
    for (const std::string &ext: mExtTokens) {
        if (ext.compare(0,5,"<col:") == 0)
            col = std::atoi(ext.substr(5).c_str());
        else if (ext.compare(0,6,"<line:") == 0) {
            line = std::atoi(ext.substr(6).c_str());
            if (ext.find(", col:") != std::string::npos)
                col = std::atoi(ext.c_str() + ext.find(", col:") + 6);
        } else if (ext[0] == '<' && ext.find(":") != std::string::npos) {
            std::string::size_type sep1 = ext.find(":");
            std::string::size_type sep2 = ext.find(":", sep1+1);
            file = tokenList->appendFileIfNew(ext.substr(1, sep1 - 1));
            line = MathLib::toLongNumber(ext.substr(sep1+1, sep2-sep1));
        }
    }
    mFile = file;
    mLine = line;
    mCol = col;
    for (auto child: children) {
        if (child)
            child->setLocations(tokenList, file, line, col);
    }
}

Token *clangimport::AstNode::addtoken(TokenList *tokenList, const std::string &str, bool valueType)
{
    const Scope *scope = getNestedInScope(tokenList);
    tokenList->addtoken(str, mLine, mFile);
    tokenList->back()->column(mCol);
    tokenList->back()->scope(scope);
    if (valueType)
        setValueType(tokenList->back());
    return tokenList->back();
}

void clangimport::AstNode::addTypeTokens(TokenList *tokenList, const std::string &str)
{
    if (str.find("\':\'") != std::string::npos) {
        addTypeTokens(tokenList, str.substr(0, str.find("\':\'") + 1));
        return;
    }

    std::string type;
    if (str.find(" (") != std::string::npos) {
        if (str.find("<") != std::string::npos)
            type = str.substr(1, str.find("<")) + "...>";
        else
            type = str.substr(1,str.find(" (")-1);
    } else
        type = unquote(str);

    for (const std::string &s: splitString(type))
        addtoken(tokenList, s, false);
}

const Scope *clangimport::AstNode::getNestedInScope(TokenList *tokenList)
{
    if (!tokenList->back())
        return &mData->mSymbolDatabase->scopeList.front();
    if (tokenList->back()->str() == "}" && mData->mNotScope.find(tokenList->back()) == mData->mNotScope.end())
        return tokenList->back()->scope()->nestedIn;
    return tokenList->back()->scope();
}

void clangimport::AstNode::setValueType(Token *tok)
{
    for (int i = 0; i < 2; i++) {
        const std::string &type = getType(i);

        if (type.find("<") != std::string::npos)
            // TODO
            continue;

        TokenList decl(nullptr);
        addTypeTokens(&decl, type);
        if (!decl.front())
            break;

        ValueType valueType = ValueType::parseDecl(decl.front(), mData->mSettings);
        if (valueType.type != ValueType::Type::UNKNOWN_TYPE) {
            tok->setValueType(new ValueType(valueType));
            break;
        }
    }
    return;
}

Scope *clangimport::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode, const Token *def)
{
    std::vector<AstNodePtr> children2{astNode};
    return createScope(tokenList, scopeType, children2, def);
}

Scope *clangimport::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> & children2, const Token *def)
{
    SymbolDatabase *symbolDatabase = mData->mSymbolDatabase;

    Scope *nestedIn = const_cast<Scope *>(getNestedInScope(tokenList));

    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nestedIn));
    Scope *scope = &symbolDatabase->scopeList.back();
    if (scopeType == Scope::ScopeType::eEnum)
        scope->enumeratorList.reserve(children2.size());
    nestedIn->nestedList.push_back(scope);
    scope->type = scopeType;
    scope->classDef = def;
    scope->check = nestedIn->check;
    if (!children2.empty()) {
        Token *bodyStart = children2[0]->addtoken(tokenList, "{");
        tokenList->back()->scope(scope);
        for (AstNodePtr astNode: children2) {
            astNode->createTokens(tokenList);
            if (scopeType == Scope::ScopeType::eEnum)
                astNode->addtoken(tokenList, ",");
            else if (!Token::Match(tokenList->back(), "[;{}]"))
                astNode->addtoken(tokenList, ";");
        }
        Token *bodyEnd = children2.back()->addtoken(tokenList, "}");
        bodyStart->link(bodyEnd);
        bodyEnd->link(bodyStart);
        scope->bodyStart = bodyStart;
        scope->bodyEnd = bodyEnd;
    }
    return scope;
}

Token *clangimport::AstNode::createTokens(TokenList *tokenList)
{
    if (nodeType == ArraySubscriptExpr) {
        Token *array = children[0]->createTokens(tokenList);
        Token *bracket1 = addtoken(tokenList, "[");
        Token *index = children[1]->createTokens(tokenList);
        Token *bracket2 = addtoken(tokenList, "]");
        bracket1->astOperand1(array);
        bracket1->astOperand2(index);
        bracket1->link(bracket2);
        bracket2->link(bracket1);
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
    if (nodeType == BreakStmt)
        return addtoken(tokenList, "break");
    if (nodeType == CharacterLiteral) {
        int c = MathLib::toLongNumber(mExtTokens.back());
        if (c == 0)
            return addtoken(tokenList, "\'\\0\'");
        if (c == '\r')
            return addtoken(tokenList, "\'\\r\'");
        if (c == '\n')
            return addtoken(tokenList, "\'\\n\'");
        if (c == '\t')
            return addtoken(tokenList, "\'\\t\'");
        if (c == '\\')
            return addtoken(tokenList, "\'\\\\\'");
        if (c < ' ' || c >= 0x80) {
            std::ostringstream hex;
            hex << std::hex << ((c>>4) & 0xf) << (c&0xf);
            return addtoken(tokenList, "\'\\x" + hex.str() + "\'");
        }
        return addtoken(tokenList, std::string("\'") + char(c) + std::string("\'"));
    }
    if (nodeType == CallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CaseStmt) {
        addtoken(tokenList, "case");
        children[0]->createTokens(tokenList);
        addtoken(tokenList, ":");
        children.back()->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == ClassTemplateDecl) {
        for (AstNodePtr child: children) {
            if (child->nodeType == ClassTemplateSpecializationDecl)
                child->createTokens(tokenList);
        }
        return nullptr;
    }
    if (nodeType == ClassTemplateSpecializationDecl) {
        createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (nodeType == ConditionalOperator) {
        Token *expr1 = children[0]->createTokens(tokenList);
        Token *tok1 = addtoken(tokenList, "?");
        Token *expr2 = children[1]->createTokens(tokenList);
        Token *tok2 = addtoken(tokenList, ":");
        Token *expr3 = children[2]->createTokens(tokenList);
        tok2->astOperand1(expr2);
        tok2->astOperand2(expr3);
        tok1->astOperand1(expr1);
        tok1->astOperand2(tok2);
        return tok1;
    }
    if (nodeType == CompoundAssignOperator) {
        Token *lhs = children[0]->createTokens(tokenList);
        Token *assign = addtoken(tokenList, getSpelling());
        Token *rhs = children[1]->createTokens(tokenList);
        assign->astOperand1(lhs);
        assign->astOperand2(rhs);
        return assign;
    }
    if (nodeType == CompoundStmt) {
        for (AstNodePtr child: children) {
            child->createTokens(tokenList);
            if (!Token::Match(tokenList->back(), "[;{}]"))
                child->addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (nodeType == ConstantExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == ContinueStmt)
        return addtoken(tokenList, "continue");
    if (nodeType == CStyleCastExpr) {
        Token *par1 = addtoken(tokenList, "(");
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(children[0]->createTokens(tokenList));
        return par1;
    }
    if (nodeType == CXXBindTemporaryExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == CXXBoolLiteralExpr) {
        addtoken(tokenList, mExtTokens.back());
        tokenList->back()->setValueType(new ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0));
        return tokenList->back();
    }
    if (nodeType == CXXConstructExpr) {
        if (!children.empty())
            return children[0]->createTokens(tokenList);
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        Token *par1 = addtoken(tokenList, "(");
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return par1;
    }
    if (nodeType == CXXConstructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXDeleteExpr) {
        addtoken(tokenList, "delete");
        children[0]->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == CXXDestructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXForRangeStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        AstNodePtr varDecl;
        if (children[6]->nodeType == DeclStmt)
            varDecl = children[6]->children[0];
        else
            varDecl = children[5]->children[0];
        varDecl->mExtTokens.pop_back();
        varDecl->children.clear();
        Token *expr1 = varDecl->createTokens(tokenList);
        Token *colon = addtoken(tokenList, ":");
        AstNodePtr range;
        for (int i = 0; i < 2; i++) {
            if (children[i] && children[i]->nodeType == DeclStmt && children[i]->children[0]->nodeType == VarDecl) {
                range = children[i]->children[0]->children[0];
                break;
            }
        }
        Token *expr2 = range->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");

        par1->link(par2);
        par2->link(par1);

        colon->astOperand1(expr1);
        colon->astOperand2(expr2);
        par1->astOperand1(forToken);
        par1->astOperand2(colon);

        createScope(tokenList, Scope::ScopeType::eFor, children.back(), forToken);
        return nullptr;
    }
    if (nodeType == CXXMethodDecl) {
        for (int i = 0; i+1 < mExtTokens.size(); ++i) {
            if (mExtTokens[i] == "prev" && !mData->hasDecl(mExtTokens[i+1]))
                return nullptr;
        }
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXMemberCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXNewExpr) {
        Token *newtok = addtoken(tokenList, "new");
        std::string type = getType();
        if (type.find("*") != std::string::npos)
            type = type.erase(type.rfind("*"));
        addTypeTokens(tokenList, type);
        if (!children.empty()) {
            Token *bracket1 = addtoken(tokenList, "[");
            children[0]->createTokens(tokenList);
            Token *bracket2 = addtoken(tokenList, "]");
            bracket1->link(bracket2);
            bracket2->link(bracket1);
        }
        return newtok;
    }
    if (nodeType == CXXNullPtrLiteralExpr)
        return addtoken(tokenList, "nullptr");
    if (nodeType == CXXOperatorCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXRecordDecl) {
        if (!children.empty())
            createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (nodeType == CXXStaticCastExpr) {
        Token *cast = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[0]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(cast);
        par1->astOperand2(expr);
        setValueType(par1);
        return par1;
    }
    if (nodeType == CXXStdInitializerListExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == CXXTemporaryObjectExpr && !children.empty())
        return children[0]->createTokens(tokenList);
    if (nodeType == CXXThisExpr)
        return addtoken(tokenList, "this");
    if (nodeType == CXXThrowExpr) {
        Token *t = addtoken(tokenList, "throw");
        t->astOperand1(children[0]->createTokens(tokenList));
        return t;
    }
    if (nodeType == DeclRefExpr) {
        const std::string addr = mExtTokens[mExtTokens.size() - 3];
        std::string name = unquote(getSpelling());
        Token *reftok = addtoken(tokenList, name.empty() ? "<NoName>" : name);
        mData->ref(addr, reftok);
        return reftok;
    }
    if (nodeType == DeclStmt)
        return children[0]->createTokens(tokenList);
    if (nodeType == DoStmt) {
        addtoken(tokenList, "do");
        createScope(tokenList, Scope::ScopeType::eDo, children[0], tokenList->back());
        Token *tok1 = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[1]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(expr);
        return nullptr;
    }
    if (nodeType == EnumConstantDecl) {
        Token *nameToken = addtoken(tokenList, getSpelling());
        Scope *scope = const_cast<Scope *>(nameToken->scope());
        scope->enumeratorList.push_back(Enumerator(nameToken->scope()));
        Enumerator *e = &scope->enumeratorList.back();
        e->name = nameToken;
        e->value = mData->enumValue++;
        e->value_known = true;
        mData->enumDecl(mExtTokens.front(), nameToken, e);
        return nameToken;
    }
    if (nodeType == EnumDecl) {
        mData->enumValue = 0;
        Token *enumtok = addtoken(tokenList, "enum");
        Token *nametok = nullptr;
        if (mExtTokens[mExtTokens.size() - 3].compare(0,4,"col:") == 0)
            nametok = addtoken(tokenList, mExtTokens.back());
        Scope *enumscope = createScope(tokenList, Scope::ScopeType::eEnum, children, enumtok);
        if (nametok)
            enumscope->className = nametok->str();
        if (enumscope->bodyEnd && Token::simpleMatch(enumscope->bodyEnd->previous(), ", }"))
            const_cast<Token *>(enumscope->bodyEnd)->deletePrevious();

        // Create enum type
        mData->mSymbolDatabase->typeList.push_back(Type(enumtok, enumscope, enumtok->scope()));
        enumscope->definedType = &mData->mSymbolDatabase->typeList.back();
        if (nametok)
            const_cast<Scope *>(enumtok->scope())->definedTypesMap[nametok->str()] = enumscope->definedType;

        return nullptr;
    }
    if (nodeType == ExprWithCleanups)
        return children[0]->createTokens(tokenList);
    if (nodeType == FieldDecl)
        return createTokensVarDecl(tokenList);
    if (nodeType == FloatingLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == ForStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr1 = children[0] ? children[0]->createTokens(tokenList) : nullptr;
        Token *sep1 = addtoken(tokenList, ";");
        Token *expr2 = children[2] ? children[2]->createTokens(tokenList) : nullptr;
        Token *sep2 = addtoken(tokenList, ";");
        Token *expr3 = children[3] ? children[3]->createTokens(tokenList) : nullptr;
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(forToken);
        par1->astOperand2(sep1);
        sep1->astOperand1(expr1);
        sep1->astOperand2(sep2);
        sep2->astOperand1(expr2);
        sep2->astOperand2(expr3);
        createScope(tokenList, Scope::ScopeType::eFor, children[4], forToken);
        return nullptr;
    }
    if (nodeType == FunctionDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == FunctionTemplateDecl) {
        bool first = true;
        for (AstNodePtr child: children) {
            if (child->nodeType == FunctionDecl) {
                if (!first)
                    child->createTokens(tokenList);
                first = false;
            }
        }
        return nullptr;
    }
    if (nodeType == GotoStmt) {
        addtoken(tokenList, "goto");
        addtoken(tokenList, unquote(mExtTokens[mExtTokens.size() - 2]));
        addtoken(tokenList, ";");
        return nullptr;
    }
    if (nodeType == IfStmt) {
        AstNodePtr cond;
        AstNodePtr thenCode;
        AstNodePtr elseCode;
        if (children.size() == 2) {
            cond = children[children.size() - 2];
            thenCode = children[children.size() - 1];
        } else {
            cond = children[children.size() - 3];
            thenCode = children[children.size() - 2];
            elseCode = children[children.size() - 1];
        }

        Token *iftok = addtoken(tokenList, "if");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(iftok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, Scope::ScopeType::eIf, thenCode, iftok);
        if (elseCode) {
            elseCode->addtoken(tokenList, "else");
            createScope(tokenList, Scope::ScopeType::eElse, elseCode, tokenList->back());
        }
        return nullptr;
    }
    if (nodeType == ImplicitCastExpr) {
        Token *expr = children[0]->createTokens(tokenList);
        if (!expr->valueType())
            setValueType(expr);
        return expr;
    }
    if (nodeType == InitListExpr) {
        const Scope *scope = tokenList->back()->scope();
        Token *start = addtoken(tokenList, "{");
        start->scope(scope);
        for (AstNodePtr child: children) {
            if (tokenList->back()->str() != "{")
                addtoken(tokenList, ",");
            child->createTokens(tokenList);
        }
        Token *end = addtoken(tokenList, "}");
        end->scope(scope);
        start->link(end);
        end->link(start);
        mData->mNotScope.insert(end);
        return start;
    }
    if (nodeType == IntegerLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == LabelStmt) {
        addtoken(tokenList, unquote(mExtTokens.back()));
        addtoken(tokenList, ":");
        for (auto child: children)
            child->createTokens(tokenList);
        return nullptr;
    }
    if (nodeType == MaterializeTemporaryExpr)
        return children[0]->createTokens(tokenList);
    if (nodeType == MemberExpr) {
        Token *s = children[0]->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        std::string memberName = getSpelling();
        if (memberName.compare(0, 2, "->") == 0) {
            dot->originalName("->");
            memberName = memberName.substr(2);
        } else if (memberName.compare(0, 1, ".") == 0) {
            memberName = memberName.substr(1);
        }
        if (memberName.empty())
            memberName = "<unknown>";
        Token *member = addtoken(tokenList, memberName);
        mData->ref(mExtTokens.back(), member);
        dot->astOperand1(s);
        dot->astOperand2(member);
        return dot;
    }
    if (nodeType == NamespaceDecl) {
        if (children.empty())
            return nullptr;
        Token *defToken = addtoken(tokenList, "namespace");
        const std::string &s = mExtTokens[mExtTokens.size() - 2];
        Token *nameToken = (s.compare(0,4,"col:")==0 || s.compare(0,5,"line:")==0) ?
                           addtoken(tokenList, mExtTokens.back()) : nullptr;
        Scope *scope = createScope(tokenList, Scope::ScopeType::eNamespace, children, defToken);
        if (nameToken)
            scope->className = nameToken->str();
        return nullptr;
    }
    if (nodeType == NullStmt)
        return addtoken(tokenList, ";");
    if (nodeType == ParenExpr) {
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[0]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return expr;
    }
    if (nodeType == RecordDecl) {
        const Token *classDef = addtoken(tokenList, "struct");
        const std::string &recordName = getSpelling();
        if (!recordName.empty())
            addtoken(tokenList, getSpelling());
        if (children.empty())
            addtoken(tokenList, ";");
        else {
            Scope *recordScope = createScope(tokenList, Scope::ScopeType::eStruct, children, classDef);
            mData->mSymbolDatabase->typeList.push_back(Type(classDef, recordScope, classDef->scope()));
            recordScope->definedType = &mData->mSymbolDatabase->typeList.back();
            if (!recordName.empty())
                const_cast<Scope *>(classDef->scope())->definedTypesMap[recordName] = recordScope->definedType;
        }
        return nullptr;
    }
    if (nodeType == ReturnStmt) {
        Token *tok1 = addtoken(tokenList, "return");
        if (!children.empty())
            tok1->astOperand1(children[0]->createTokens(tokenList));
        return tok1;
    }
    if (nodeType == StringLiteral)
        return addtoken(tokenList, mExtTokens.back());
    if (nodeType == SwitchStmt) {
        Token *tok1 = addtoken(tokenList, "switch");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[children.size() - 2]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(expr);
        createScope(tokenList, Scope::ScopeType::eSwitch, children.back(), tok1);
        return nullptr;
    }
    if (nodeType == TypedefDecl) {
        addtoken(tokenList, "typedef");
        addTypeTokens(tokenList, getType());
        return addtoken(tokenList, getSpelling());
    }
    if (nodeType == UnaryOperator) {
        int index = (int)mExtTokens.size() - 1;
        while (index > 0 && mExtTokens[index][0] != '\'')
            --index;
        Token *unop = addtoken(tokenList, unquote(mExtTokens[index]));
        unop->astOperand1(children[0]->createTokens(tokenList));
        return unop;
    }
    if (nodeType == UnaryExprOrTypeTraitExpr) {
        Token *tok1 = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        if (children.empty())
            addTypeTokens(tokenList, mExtTokens.back());
        else
            addTypeTokens(tokenList, children[0]->getType());
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(par1->next());
        setValueType(par1);
        return par1;
    }
    if (nodeType == VarDecl)
        return createTokensVarDecl(tokenList);
    if (nodeType == WhileStmt) {
        AstNodePtr cond = children[children.size() - 2];
        AstNodePtr body = children.back();
        Token *whiletok = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(whiletok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, Scope::ScopeType::eWhile, body, whiletok);
        return nullptr;
    }
    return addtoken(tokenList, "?" + nodeType + "?");
}

Token * clangimport::AstNode::createTokensCall(TokenList *tokenList)
{
    int firstParam;
    Token *f;
    if (nodeType == CXXOperatorCallExpr) {
        firstParam = 2;
        Token *obj = children[1]->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        Token *op = children[0]->createTokens(tokenList);
        dot->astOperand1(obj);
        dot->astOperand2(op);
        f = dot;
    } else {
        firstParam = 1;
        f = children[0]->createTokens(tokenList);
    }
    Token *par1 = addtoken(tokenList, "(");
    par1->astOperand1(f);
    Token *parent = par1;
    int args = 0;
    while (args < children.size() && children[args]->nodeType != CXXDefaultArgExpr)
        args++;
    for (int c = firstParam; c < args; ++c) {
        if (c + 1 < args) {
            Token *child = children[c]->createTokens(tokenList);
            Token *comma = addtoken(tokenList, ",");
            comma->astOperand1(child);
            parent->astOperand2(comma);
            parent = comma;
        } else {
            parent->astOperand2(children[c]->createTokens(tokenList));
        }
    }
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);
    return par1;
}

void clangimport::AstNode::createTokensFunctionDecl(TokenList *tokenList)
{
    const bool prev = (std::find(mExtTokens.begin(), mExtTokens.end(), "prev") != mExtTokens.end());
    const bool hasBody = mFile == 0 && !children.empty() && children.back()->nodeType == CompoundStmt;

    SymbolDatabase *symbolDatabase = mData->mSymbolDatabase;
    if (nodeType != CXXConstructorDecl && nodeType != CXXDestructorDecl)
        addTypeTokens(tokenList, '\'' + getType() + '\'');
    Token *nameToken = addtoken(tokenList, getSpelling() + getTemplateParameters());
    Scope *nestedIn = const_cast<Scope *>(nameToken->scope());

    if (prev) {
        const std::string addr = *(std::find(mExtTokens.begin(), mExtTokens.end(), "prev") + 1);
        mData->ref(addr, nameToken);
    }
    if (!nameToken->function()) {
        nestedIn->functionList.push_back(Function(nameToken));
        mData->funcDecl(mExtTokens.front(), nameToken, &nestedIn->functionList.back());
        if (nodeType == CXXConstructorDecl)
            nestedIn->functionList.back().type = Function::Type::eConstructor;
        else if (nodeType == CXXDestructorDecl)
            nestedIn->functionList.back().type = Function::Type::eDestructor;
    }

    Function * const function = const_cast<Function*>(nameToken->function());

    Scope *scope = nullptr;
    if (hasBody) {
        symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nestedIn));
        scope = &symbolDatabase->scopeList.back();
        scope->function = function;
        scope->type = Scope::ScopeType::eFunction;
        scope->className = nameToken->str();
        nestedIn->nestedList.push_back(scope);
        function->hasBody(true);
    }

    Token *par1 = addtoken(tokenList, "(");
    if (!function->arg)
        function->arg = par1;
    function->token = nameToken;
    // Function arguments
    for (int i = 0; i < children.size(); ++i) {
        AstNodePtr child = children[i];
        if (child->nodeType != ParmVarDecl)
            continue;
        if (tokenList->back() != par1)
            addtoken(tokenList, ",");
        addTypeTokens(tokenList, child->mExtTokens.back());
        const std::string spelling = child->getSpelling();
        Token *vartok = nullptr;
        if (!spelling.empty())
            vartok = child->addtoken(tokenList, spelling);
        if (!prev) {
            function->argumentList.push_back(Variable(vartok, child->getType(), nullptr, i, AccessControl::Argument, nullptr, scope));
            if (vartok) {
                const std::string addr = child->mExtTokens[0];
                mData->varDecl(addr, vartok, &function->argumentList.back());
            }
        } else if (vartok) {
            const std::string addr = child->mExtTokens[0];
            mData->ref(addr, vartok);
        }
    }
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);

    // Function body
    if (hasBody) {
        symbolDatabase->functionScopes.push_back(scope);
        Token *bodyStart = addtoken(tokenList, "{");
        bodyStart->scope(scope);
        children.back()->createTokens(tokenList);
        Token *bodyEnd = addtoken(tokenList, "}");
        scope->bodyStart = bodyStart;
        scope->bodyEnd = bodyEnd;
        bodyStart->link(bodyEnd);
        bodyEnd->link(bodyStart);
    } else {
        addtoken(tokenList, ";");
    }
}

void clangimport::AstNode::createTokensForCXXRecord(TokenList *tokenList)
{
    Token *classToken = addtoken(tokenList, "class");
    const std::string className = mExtTokens[mExtTokens.size() - 2] + getTemplateParameters();
    /*Token *nameToken =*/ addtoken(tokenList, className);
    std::vector<AstNodePtr> children2;
    for (AstNodePtr child: children) {
        if (child->nodeType == CXXConstructorDecl ||
            child->nodeType == CXXDestructorDecl ||
            child->nodeType == CXXMethodDecl ||
            child->nodeType == FieldDecl)
            children2.push_back(child);
    }
    if (children2.empty()) {
        addtoken(tokenList, ";");
        return;
    }
    Scope *scope = createScope(tokenList, Scope::ScopeType::eClass, children2, classToken);
    scope->className = className;
    mData->mSymbolDatabase->typeList.push_back(Type(classToken, scope, classToken->scope()));
    scope->definedType = &mData->mSymbolDatabase->typeList.back();
}

Token * clangimport::AstNode::createTokensVarDecl(TokenList *tokenList)
{
    const std::string addr = mExtTokens.front();
    const Token *startToken = nullptr;
    if (std::find(mExtTokens.cbegin(), mExtTokens.cend(), "static") != mExtTokens.cend())
        startToken = addtoken(tokenList, "static");
    int typeIndex = mExtTokens.size() - 1;
    while (typeIndex > 1 && std::isalpha(mExtTokens[typeIndex][0]))
        typeIndex--;
    const std::string type = mExtTokens[typeIndex];
    const std::string name = mExtTokens[typeIndex - 1];
    addTypeTokens(tokenList, type);
    if (!startToken && tokenList->back()) {
        startToken = tokenList->back();
        while (Token::Match(startToken->previous(), "%type%|*|&|&&"))
            startToken = startToken->previous();
    }
    Token *vartok1 = addtoken(tokenList, name);
    Scope *scope = const_cast<Scope *>(tokenList->back()->scope());
    const AccessControl accessControl = (scope->type == Scope::ScopeType::eGlobal) ? (AccessControl::Global) : (AccessControl::Local);
    scope->varlist.push_back(Variable(vartok1, type, startToken, 0, accessControl, nullptr, scope));
    mData->varDecl(addr, vartok1, &scope->varlist.back());
    if (mExtTokens.back() == "cinit" && !children.empty()) {
        Token *eq = addtoken(tokenList, "=");
        eq->astOperand1(vartok1);
        eq->astOperand2(children.back()->createTokens(tokenList));
        return eq;
    } else if (mExtTokens.back() == "callinit") {
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(vartok1);
        par1->astOperand2(children[0]->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return par1;
    } else if (mExtTokens.back() == "listinit") {
        return children[0]->createTokens(tokenList);
    }
    return vartok1;
}

static void setTypes(TokenList *tokenList)
{
    for (Token *tok = tokenList->front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            for (Token *typeToken = tok->tokAt(2); typeToken->str() != ")"; typeToken = typeToken->next()) {
                if (typeToken->type())
                    continue;
                typeToken->type(typeToken->scope()->findType(typeToken->str()));
            }
        }
    }
}

static void setValues(Tokenizer *tokenizer, SymbolDatabase *symbolDatabase)
{
    const Settings * const settings = tokenizer->getSettings();

    for (Scope &scope: symbolDatabase->scopeList) {
        if (!scope.definedType)
            continue;

        int typeSize = 0;
        for (const Variable &var: scope.varlist) {
            int mul = 1;
            for (const auto &dim: var.dimensions()) {
                mul *= dim.num;
            }
            if (var.valueType())
                typeSize += mul * var.valueType()->typeSize(*settings, true);
        }
        scope.definedType->sizeOf = typeSize;
    }

    for (Token *tok = const_cast<Token*>(tokenizer->tokens()); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            ValueType vt = ValueType::parseDecl(tok->tokAt(2), settings);
            int sz = vt.typeSize(*settings, true);
            if (sz <= 0)
                continue;
            int mul = 1;
            for (Token *arrtok = tok->linkAt(1)->previous(); arrtok; arrtok = arrtok->previous()) {
                const std::string &a = arrtok->str();
                if (a.size() > 2 && a[0] == '[' && a.back() == ']')
                    mul *= std::atoi(a.substr(1).c_str());
                else
                    break;
            }
            ValueFlow::Value v(mul * sz);
            v.setKnown();
            tok->next()->addValue(v);
        }
    }
}

void clangimport::parseClangAstDump(Tokenizer *tokenizer, std::istream &f)
{
    TokenList *tokenList = &tokenizer->list;

    tokenizer->createSymbolDatabase();
    SymbolDatabase *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer->getSymbolDatabase());
    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nullptr));
    symbolDatabase->scopeList.back().type = Scope::ScopeType::eGlobal;
    symbolDatabase->scopeList.back().check = symbolDatabase;

    clangimport::Data data;
    data.mSettings = tokenizer->getSettings();
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

        if (pos1 == 1 && endsWith(nodeType, "Decl", 4)) {
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
    setTypes(tokenList);
    setValues(tokenizer, symbolDatabase);
}

