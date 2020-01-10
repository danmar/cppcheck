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
static const std::string CharacterLiteral = "CharacterLiteral";
static const std::string ClassTemplateDecl = "ClassTemplateDecl";
static const std::string ClassTemplateSpecializationDecl = "ClassTemplateSpecializationDecl";
static const std::string CompoundStmt = "CompoundStmt";
static const std::string ContinueStmt = "ContinueStmt";
static const std::string CStyleCastExpr = "CStyleCastExpr";
static const std::string CXXBoolLiteralExpr = "CXXBoolLiteralExpr";
static const std::string CXXConstructorDecl = "CXXConstructorDecl";
static const std::string CXXMemberCallExpr = "CXXMemberCallExpr";
static const std::string CXXMethodDecl = "CXXMethodDecl";
static const std::string CXXOperatorCallExpr = "CXXOperatorCallExpr";
static const std::string CXXRecordDecl = "CXXRecordDecl";
static const std::string CXXStaticCastExpr = "CXXStaticCastExpr";
static const std::string CXXThisExpr = "CXXThisExpr";
static const std::string DeclRefExpr = "DeclRefExpr";
static const std::string DeclStmt = "DeclStmt";
static const std::string FieldDecl = "FieldDecl";
static const std::string ForStmt = "ForStmt";
static const std::string FunctionDecl = "FunctionDecl";
static const std::string FunctionTemplateDecl = "FunctionTemplateDecl";
static const std::string IfStmt = "IfStmt";
static const std::string ImplicitCastExpr = "ImplicitCastExpr";
static const std::string IntegerLiteral = "IntegerLiteral";
static const std::string MemberExpr = "MemberExpr";
static const std::string NamespaceDecl = "NamespaceDecl";
static const std::string NullStmt = "NullStmt";
static const std::string ParenExpr = "ParenExpr";
static const std::string ParmVarDecl = "ParmVarDecl";
static const std::string RecordDecl = "RecordDecl";
static const std::string ReturnStmt = "ReturnStmt";
static const std::string StringLiteral = "StringLiteral";
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
            notFound(addr);
        }

        void funcDecl(const std::string &addr, Token *nameToken, Function *function) {
            Decl decl(nameToken, function);
            mDeclMap.insert(std::pair<std::string, Decl>(addr, decl));
            nameToken->function(function);
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
        }
    private:
        Token *createTokens(TokenList *tokenList);
        Token *addtoken(TokenList *tokenList, const std::string &str, bool valueType=true);
        void addTypeTokens(TokenList *tokenList, const std::string &str);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode);
        Scope *createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> &children);
        Token *createTokensCall(TokenList *tokenList);
        void createTokensFunctionDecl(TokenList *tokenList);
        void createTokensForCXXRecord(TokenList *tokenList);
        Token *createTokensVarDecl(TokenList *tokenList);
        std::string getSpelling() const;
        std::string getType() const;
        std::string getTemplateParameters() const;
        const Scope *getNestedInScope(TokenList *tokenList);
        void setValueType(Token *tok);

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        int mVarId = 0;
        std::vector<std::string> mExtTokens;
        Data *mData;
    };
}

std::string clangimport::AstNode::getSpelling() const
{
    int retTypeIndex = mExtTokens.size() - 1;
    if (nodeType == FunctionDecl) {
        while (mExtTokens[retTypeIndex][0] != '\'')
            retTypeIndex--;
    }
    const std::string &str = mExtTokens[retTypeIndex - 1];
    if (str.compare(0,4,"col:") == 0)
        return "";
    if (str.compare(0,8,"<invalid") == 0)
        return "";
    return str;
}

std::string clangimport::AstNode::getType() const
{
    if (nodeType == BinaryOperator)
        return unquote(mExtTokens[mExtTokens.size() - 2]);
    if (nodeType == CStyleCastExpr)
        return unquote((mExtTokens.back() == "<NoOp>") ?
                       mExtTokens[mExtTokens.size() - 2] :
                       mExtTokens.back());
    if (nodeType == CXXStaticCastExpr)
        return unquote(mExtTokens[mExtTokens.size() - 3]);
    if (nodeType == DeclRefExpr)
        return unquote(mExtTokens.back());
    if (nodeType == FunctionDecl) {
        int retTypeIndex = mExtTokens.size() - 1;
        while (mExtTokens[retTypeIndex][0] != '\'')
            retTypeIndex--;
        return unquote(mExtTokens[retTypeIndex]);
    }
    if (nodeType == IntegerLiteral)
        return unquote(mExtTokens[mExtTokens.size() - 2]);
    if (nodeType == TypedefDecl)
        return unquote(mExtTokens.back());
    if (nodeType == UnaryExprOrTypeTraitExpr)
        return unquote(mExtTokens[mExtTokens.size() - 3]);
    return "";
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
    if (tokenList->back()->str() == "}")
        return tokenList->back()->scope()->nestedIn;
    return tokenList->back()->scope();
}

void clangimport::AstNode::setValueType(Token *tok)
{
    const std::string &type = getType();

    if (type.find("<") != std::string::npos) {
        // TODO
        tok->setValueType(new ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::NONSTD, 0));
        return;
    }

    TokenList decl(nullptr);
    addTypeTokens(&decl, type);

    if (Token::simpleMatch(decl.front(), "bool"))
        tok->setValueType(new ValueType(ValueType::Sign::UNSIGNED, ValueType::Type::BOOL, 0));
    else if (Token::simpleMatch(decl.front(), "int"))
        tok->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
    else if (Token::simpleMatch(decl.front(), "unsigned long"))
        tok->setValueType(new ValueType(ValueType::Sign::UNSIGNED, ValueType::Type::LONG, 0));
    else if (Token::simpleMatch(decl.front(), "__int128"))
        tok->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::UNKNOWN_INT, 0));
    else if (tok->isNumber())
        tok->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::INT, 0));
    else if (tok->tokType() == Token::Type::eChar)
        // TODO
        tok->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::CHAR, 0));
    else if (tok->tokType() == Token::Type::eString)
        // TODO
        tok->setValueType(new ValueType(ValueType::Sign::SIGNED, ValueType::Type::CHAR, 1));
    else {
        //decl.front()->printOut("");
    }
}

Scope *clangimport::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, AstNodePtr astNode)
{
    std::vector<AstNodePtr> children{astNode};
    return createScope(tokenList, scopeType, children);
}

Scope *clangimport::AstNode::createScope(TokenList *tokenList, Scope::ScopeType scopeType, const std::vector<AstNodePtr> &children)
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
    bodyEnd->link(bodyStart);
    scope->bodyStart = bodyStart;
    scope->bodyEnd = bodyEnd;
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
    if (nodeType == CompoundStmt) {
        for (AstNodePtr child: children) {
            child->createTokens(tokenList);
            if (!Token::Match(tokenList->back(), "[;{}]"))
                child->addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (nodeType == ContinueStmt)
        return addtoken(tokenList, "continue");
    if (nodeType == CXXConstructorDecl) {
        bool hasBody = false;
        for (AstNodePtr child: children) {
            if (child->nodeType == CompoundStmt && !child->children.empty()) {
                hasBody = true;
                break;
            }
        }
        if (hasBody)
            createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CStyleCastExpr) {
        Token *par1 = addtoken(tokenList, "(");
        addTypeTokens(tokenList, '\'' + getType() + '\'');
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(children[0]->createTokens(tokenList));
        return par1;
    }
    if (nodeType == CXXBoolLiteralExpr) {
        addtoken(tokenList, mExtTokens.back());
        tokenList->back()->setValueType(new ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0));
        return tokenList->back();
    }
    if (nodeType == CXXMethodDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (nodeType == CXXMemberCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXOperatorCallExpr)
        return createTokensCall(tokenList);
    if (nodeType == CXXRecordDecl) {
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
    if (nodeType == CXXThisExpr)
        return addtoken(tokenList, "this");
    if (nodeType == DeclStmt)
        return children[0]->createTokens(tokenList);
    if (nodeType == DeclRefExpr) {
        const std::string addr = mExtTokens[mExtTokens.size() - 3];
        std::string name = unquote(getSpelling());
        Token *reftok = addtoken(tokenList, name.empty() ? "<NoName>" : name);
        mData->ref(addr, reftok);
        return reftok;
    }
    if (nodeType == FieldDecl)
        return createTokensVarDecl(tokenList);
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
        Scope *scope = createScope(tokenList, Scope::ScopeType::eFor, children[4]);
        scope->classDef = forToken;
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
        par2->link(par1);
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
    if (nodeType == NullStmt)
        return addtoken(tokenList, ";");
    if (nodeType == NamespaceDecl) {
        if (children.empty())
            return nullptr;
        Token *defToken = addtoken(tokenList, "namespace");
        Token *nameToken = (mExtTokens[mExtTokens.size() - 2].compare(0,4,"col:") == 0) ?
                           addtoken(tokenList, mExtTokens.back()) : nullptr;
        Scope *scope = createScope(tokenList, Scope::ScopeType::eNamespace, children);
        scope->classDef = defToken;
        if (nameToken)
            scope->className = nameToken->str();
        return nullptr;
    }
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
            Scope *recordScope = createScope(tokenList, Scope::ScopeType::eStruct, children);
            mData->mSymbolDatabase->typeList.push_back(Type(classDef, recordScope, classDef->scope()));
            recordScope->definedType = &mData->mSymbolDatabase->typeList.back();
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
        addTypeTokens(tokenList, mExtTokens.back());
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
        AstNodePtr cond = children[1];
        AstNodePtr body = children[2];
        Token *whiletok = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(whiletok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, Scope::ScopeType::eWhile, body);
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
    for (int c = firstParam; c < children.size(); ++c) {
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
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);
    return par1;
}

void clangimport::AstNode::createTokensFunctionDecl(TokenList *tokenList)
{
    SymbolDatabase *symbolDatabase = mData->mSymbolDatabase;
    addTypeTokens(tokenList, '\'' + getType() + '\'');
    Token *nameToken = addtoken(tokenList, getSpelling() + getTemplateParameters());
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
    par2->link(par1);
    // Function body
    if (mFile == 0 && !children.empty() && children.back()->nodeType == CompoundStmt) {
        Token *bodyStart = addtoken(tokenList, "{");
        bodyStart->scope(&scope);
        children.back()->createTokens(tokenList);
        Token *bodyEnd = addtoken(tokenList, "}");
        scope.bodyStart = bodyStart;
        scope.bodyEnd = bodyEnd;
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
            child->nodeType == CXXMethodDecl ||
            child->nodeType == FieldDecl)
            children2.push_back(child);
    }
    if (children2.empty()) {
        addtoken(tokenList, ";");
        return;
    }
    Scope *scope = createScope(tokenList, Scope::ScopeType::eClass, children2);
    scope->classDef = classToken;
    scope->className = className;
    mData->mSymbolDatabase->typeList.push_back(Type(classToken, scope, classToken->scope()));
    scope->definedType = &mData->mSymbolDatabase->typeList.back();
}

Token * clangimport::AstNode::createTokensVarDecl(TokenList *tokenList)
{
    const std::string addr = mExtTokens.front();
    int typeIndex = mExtTokens.size() - 1;
    while (typeIndex > 1 && std::isalpha(mExtTokens[typeIndex][0]))
        typeIndex--;
    const std::string type = mExtTokens[typeIndex];
    const std::string name = mExtTokens[typeIndex - 1];
    addTypeTokens(tokenList, type);
    Token *vartok1 = addtoken(tokenList, name);
    Scope *scope = const_cast<Scope *>(tokenList->back()->scope());
    const AccessControl accessControl = (scope->type == Scope::ScopeType::eGlobal) ? (AccessControl::Global) : (AccessControl::Local);
    scope->varlist.push_back(Variable(vartok1, type, 0, accessControl, nullptr, scope));
    mData->varDecl(addr, vartok1, &scope->varlist.back());
    if (mExtTokens.back() == "cinit") {
        Token *eq = addtoken(tokenList, "=");
        eq->astOperand1(vartok1);
        eq->astOperand2(children.back()->createTokens(tokenList));
        return eq;
    }
    return vartok1;
}

void clangimport::parseClangAstDump(Tokenizer *tokenizer, std::istream &f)
{
    TokenList *tokenList = &tokenizer->list;

    tokenizer->createSymbolDatabase();
    SymbolDatabase *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer->getSymbolDatabase());
    symbolDatabase->scopeList.push_back(Scope(nullptr, nullptr, nullptr));
    symbolDatabase->scopeList.back().type = Scope::ScopeType::eGlobal;

    clangimport::Data data;
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
}

