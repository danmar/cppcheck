/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "errortypes.h"
#include "mathlib.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

#include "json.h"
#include "simplecpp.h"

static const std::string AccessSpecDecl = "AccessSpecDecl";
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
static const std::string CXXFunctionalCastExpr = "CXXFunctionalCastExpr";
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
static const std::string DefaultStmt = "DefaultStmt";
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
static const std::string LinkageSpecDecl = "LinkageSpecDecl";
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
static const std::string TranslationUnitDecl = "TranslationUnitDecl";
static const std::string TypedefDecl = "TypedefDecl";
static const std::string UnaryOperator = "UnaryOperator";
static const std::string UnaryExprOrTypeTraitExpr = "UnaryExprOrTypeTraitExpr";
static const std::string UsingDirectiveDecl = "UsingDirectiveDecl";
static const std::string VarDecl = "VarDecl";
static const std::string WhileStmt = "WhileStmt";

static std::string unquote(const std::string &s)
{
    return (s[0] == '\'') ? s.substr(1, s.size() - 2) : s;
}


static std::vector<std::string> splitString(const std::string &line)
{
    std::vector<std::string> ret;

    if (!line.empty() && line[0] == '[') {
        std::istringstream i(line);
        simplecpp::OutputList outputList;
        std::vector<std::string> filenames{"test.cpp"};
        simplecpp::TokenList tokens(i, filenames, filenames[0], &outputList);
        for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next)
            ret.push_back(tok->str());
        return ret;
    }

    std::string::size_type pos1 = line.find_first_not_of(' ');
    while (pos1 < line.size()) {
        std::string::size_type pos2;
        if (std::strchr("*()", line[pos1])) {
            ret.push_back(line.substr(pos1,1));
            pos1 = line.find_first_not_of(' ', pos1 + 1);
            continue;
        }
        if (line[pos1] == '<')
            pos2 = line.find('>', pos1);
        else if (line[pos1] == '\"')
            pos2 = line.find('\"', pos1+1);
        else if (line[pos1] == '\'') {
            pos2 = line.find('\'', pos1+1);
            if (pos2 < static_cast<int>(line.size()) - 3 && line.compare(pos2, 3, "\':\'", 0, 3) == 0)
                pos2 = line.find('\'', pos2 + 3);
        } else {
            pos2 = pos1;
            while (pos2 < line.size() && (line[pos2] == '_' || line[pos2] == ':' || std::isalnum(static_cast<unsigned char>(line[pos2]))))
                ++pos2;
            if (pos2 > pos1 && pos2 < line.size() && line[pos2] == '<' && std::isalpha(line[pos1])) {
                int tlevel = 1;
                while (++pos2 < line.size() && tlevel > 0) {
                    if (line[pos2] == '<')
                        ++tlevel;
                    else if (line[pos2] == '>')
                        --tlevel;
                }
                if (tlevel == 0 && pos2 < line.size() && line[pos2] == ' ') {
                    ret.push_back(line.substr(pos1, pos2-pos1));
                    pos1 = pos2 + 1;
                    continue;
                }
            }

            pos2 = line.find(' ', pos1) - 1;
            if ((std::isalpha(line[pos1]) || line[pos1] == '_') &&
                line.find("::", pos1) < pos2 &&
                line.find("::", pos1) < line.find('<', pos1)) {
                pos2 = line.find("::", pos1);
                ret.push_back(line.substr(pos1, pos2-pos1));
                ret.emplace_back("::");
                pos1 = pos2 + 2;
                continue;
            }
            if ((std::isalpha(line[pos1]) || line[pos1] == '_') &&
                line.find('<', pos1) < pos2 &&
                line.find("<<",pos1) != line.find('<',pos1) &&
                line.find('>', pos1) != std::string::npos &&
                line.find('>', pos1) > pos2) {
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
                pos2 = line.find(' ', pos2);
                if (pos2 != std::string::npos)
                    --pos2;
            }
        }
        if (pos2 == std::string::npos) {
            ret.push_back(line.substr(pos1));
            break;
        }
        ret.push_back(line.substr(pos1, pos2+1-pos1));
        pos1 = line.find_first_not_of(' ', pos2 + 1);
    }
    return ret;
}

static std::string::size_type getQualTypeSplitPos(const std::string& qualType) {
    const auto pos1 = qualType.find_first_of("[(");
    if (pos1 == std::string::npos)
        return pos1;
    if (qualType[pos1] == '[')
        return pos1;
    const auto pos2 = qualType.find(")(");
    return (pos2 == std::string::npos) ? pos1 : std::string::npos;
}

static std::string getQualTypeBefore(const std::string& qualType) {
    const auto pos = getQualTypeSplitPos(qualType);
    return (pos == std::string::npos) ? qualType : qualType.substr(0, pos);
}

static std::string getQualTypeAfter(const std::string& qualType) {
    const auto pos = getQualTypeSplitPos(qualType);
    return (pos == std::string::npos) ? "" : qualType.substr(pos);
}

namespace clangimport {
    struct Data {
        struct Decl {
            explicit Decl(Scope *scope) : scope(scope) {}
            Decl(Token *def, Variable *var) : def(def), var(var) {}
            Decl(Token *def, Function *function) : def(def), function(function) {}
            Decl(Token *def, Enumerator *enumerator) : def(def), enumerator(enumerator) {}
            void ref(Token *tok) const {
                if (enumerator)
                    tok->enumerator(enumerator);
                if (function)
                    tok->function(function);
                if (var) {
                    tok->variable(var);
                    tok->varId(var->declarationId());
                }
            }
            Token* def{};
            Enumerator* enumerator{};
            Function* function{};
            Scope* scope{};
            Variable* var{};
        };

        Data(const Settings& settings, SymbolDatabase& symbolDatabase)
            : mSettings(settings)
            , mSymbolDatabase(symbolDatabase)
        {}

        const Settings &mSettings;
        SymbolDatabase &mSymbolDatabase;

        int enumValue = 0;

        void enumDecl(const std::string &addr, Token *nameToken, Enumerator *enumerator) {
            Decl decl(nameToken, enumerator);
            mDeclMap.emplace(addr, decl);
            nameToken->enumerator(enumerator);
            notFound(addr);
        }

        void funcDecl(const std::string &id, Token *nameToken, Function *function) {
            Decl decl(nameToken, function);
            mDeclMap.emplace(id, decl);
            nameToken->function(function);
            notFound(id);
        }

        void labelDecl(const std::string &addr, const Token* nameToken) {
            mLabelMap.emplace(addr, nameToken);
        }

        void scopeDecl(const std::string &addr, Scope *scope) {
            Decl decl(scope);
            mDeclMap.emplace(addr, decl);
        }

        void varDecl(const std::string &addr, Token *def, Variable *var) {
            Decl decl(def, var);
            mDeclMap.emplace(addr, decl);
            def->varId(++mVarId);
            def->variable(var);
            if (def->valueType())
                var->setValueType(*def->valueType());
            notFound(addr);
        }

        void replaceVarDecl(const Variable *from, Variable *to) {
            for (auto &it: mDeclMap) {
                Decl &decl = it.second;
                if (decl.var == from)
                    decl.var = to;
            }
        }

        void ref(const std::string &id, Token *tok) {
            auto it = mDeclMap.find(id);
            if (it != mDeclMap.end())
                it->second.ref(tok);
            else
                mNotFound[id].push_back(tok);
        }

        std::vector<const Variable *> getVariableList() const {
            std::vector<const Variable *> ret;
            ret.resize(mVarId + 1, nullptr);
            for (const auto& it: mDeclMap) {
                if (it.second.var)
                    ret[it.second.var->declarationId()] = it.second.var;
            }
            return ret;
        }

        bool hasDecl(const std::string &id) const {
            return mDeclMap.find(id) != mDeclMap.end();
        }

        const Token *getLabel(const std::string& id) const {
            auto it = mLabelMap.find(id);
            return (it == mLabelMap.end() ? nullptr : it->second);
        }

        const Scope *getScope(const std::string &id) const {
            auto it = mDeclMap.find(id);
            return (it == mDeclMap.end() ? nullptr : it->second.scope);
        }

        // "}" tokens that are not end-of-scope
        std::set<Token *> mNotScope;

        std::map<const Scope *, AccessControl> scopeAccessControl;
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
        std::map<std::string, const Token*> mLabelMap;
        std::map<std::string, std::vector<Token *>> mNotFound;
        int mVarId = 0;
    };

    class AstNode;
    using AstNodePtr = std::shared_ptr<AstNode>;

    class AstNode {
    public:
        AstNode(const picojson::object& jsonObject, Data &data)
            : mKind(jsonObject.at("kind").get<std::string>())
            , mJsonObject(jsonObject)
            , mData(data)
        {}
        const std::string mKind;
        std::vector<AstNodePtr> children;

        bool isPrologueTypedefDecl() const;
        void setLocations(TokenList &tokenList, int file, int line, int col);

        void dumpAst(int num = 0, int indent = 0) const;
        void createTokens1(TokenList &tokenList) {
            //dumpAst(); // TODO: reactivate or remove
            if (isPrologueTypedefDecl())
                return;
            if (!tokenList.back()) {
                setLocations(tokenList, 0, 1, 1);
            }
            else
                setLocations(tokenList, tokenList.back()->fileIndex(), tokenList.back()->linenr(), 1);
            createTokens(tokenList);
            if (mKind == VarDecl || mKind == RecordDecl || mKind == TypedefDecl)
                addtoken(tokenList, ";");
            mData.mNotScope.clear();
        }

        AstNodePtr getChild(int c) {
            if (c >= children.size()) {
                std::ostringstream err;
                err << "ClangImport: AstNodePtr::getChild(" << c << ") out of bounds. children.size=" << children.size() << " " << mKind;
                // JSON for (const std::string &s: mExtTokens)
                // JSON    err << " " << s;
                throw InternalError(nullptr, err.str());
            }
            return children[c];
        }

        const picojson::object& getJsonObject() const {
            return mJsonObject;
        }

    private:
        Token *createTokens(TokenList &tokenList);
        Token *addtoken(TokenList &tokenList, const std::string &str, bool valueType=true);
        const ::Type *addTypeTokens(TokenList &tokenList, const std::string &str, const Scope *scope = nullptr);
        void addFullScopeNameTokens(TokenList &tokenList, const Scope *recordScope);
        Scope *createScope(TokenList &tokenList, ScopeType scopeType, AstNodePtr astNode, const Token *def);
        Scope *createScope(TokenList &tokenList, ScopeType scopeType, const std::vector<AstNodePtr> &children2, const Token *def);
        RET_NONNULL Token *createTokensCall(TokenList &tokenList);
        void createTokensFunctionDecl(TokenList &tokenList);
        void createTokensForCXXRecord(TokenList &tokenList);
        Token *createTokensVarDecl(TokenList &tokenList);
        std::string getSpelling() const;
        std::string getQualType() const;
        std::string getStorageClass() const;
        bool isDefinition() const;
        std::string getTemplateParameters() const;
        const Scope *getNestedInScope(TokenList &tokenList);
        void setValueType(Token *tok);

        int mFile  = 0;
        int mLine  = 1;
        int mCol   = 1;
        const picojson::object mJsonObject;
        Data& mData;
    };
}

std::string clangimport::AstNode::getSpelling() const
{
    if (mJsonObject.count("opcode") > 0)
        return mJsonObject.at("opcode").get<std::string>();
    if (mJsonObject.count("name") > 0)
        return mJsonObject.at("name").get<std::string>();
    return {};
}

std::string clangimport::AstNode::getQualType() const
{
    if (mJsonObject.count("type") == 0)
        return "";
    const picojson::object &type = mJsonObject.at("type").get<picojson::object>();
    std::string ret;
    if (type.count("desugaredQualType") > 0)
        ret = type.at("desugaredQualType").get<std::string>();
    else if (type.count("qualType") > 0)
        ret = type.at("qualType").get<std::string>();
    if (startsWith(ret, "enum (unnamed enum at "))
        ret = "";
    return ret;
}

std::string clangimport::AstNode::getStorageClass() const
{
    return mJsonObject.count("storageClass") ? mJsonObject.at("storageClass").get<std::string>() : "";
}

bool clangimport::AstNode::isDefinition() const
{
    return mJsonObject.count("completeDefinition") && mJsonObject.at("completeDefinition").is<bool>();
}

std::string clangimport::AstNode::getTemplateParameters() const
{
    std::string ret;
    for (const AstNodePtr& child: children) {
        if (child->mKind == TemplateArgument)
            ret += (ret.empty() ? "<" : ",") + child->getQualType();
    }
    return ret.empty() ? "" : (ret + ">");
}

// cppcheck-suppress unusedFunction // only used in comment
void clangimport::AstNode::dumpAst(int num, int indent) const
{
    (void)num;
    std::cout << std::string(indent, ' ') << mKind << " " << mJsonObject.at("id").get<std::string>() << " " << getSpelling();
    std::cout << std::endl;
    for (int c = 0; c < children.size(); ++c) {
        if (children[c])
            children[c]->dumpAst(c, indent + 2);
        else
            std::cout << std::string(indent + 2, ' ') << "<<<<NULL>>>>>" << std::endl;
    }
}

bool clangimport::AstNode::isPrologueTypedefDecl() const
{
    // these TypedefDecl are included in *any* AST dump and we should ignore them as they should not be of interest to us
    // see https://github.com/llvm/llvm-project/issues/120228#issuecomment-2549212109 for an explanation
    if (mKind != TypedefDecl)
        return false;

    // TODO: use different values to indicate "<invalid sloc>"?
    if (mFile != 0 || mLine != 1 || mCol != 1)
        return false;

    if (!startsWith(getSpelling(), "__"))
        return false;

    // TODO: match without using children
    if (children.empty())
        return false;

    const auto& type = children[0].get()->getQualType();
    if (type == "__int128" ||
        type == "unsigned __int128" ||
        type == "struct __NSConstantString_tag" ||
        type == "char *" ||
        type == "struct __va_list_tag[1]")
    {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        return true;
    }

    return false;
}

void clangimport::AstNode::setLocations(TokenList &tokenList, int file, int line, int col)
{
    if (mJsonObject.count("loc") && mJsonObject.at("loc").is<picojson::object>())
    {
        const picojson::object &loc = mJsonObject.at("loc").get<picojson::object>();
        if (loc.count("file") && loc.at("file").is<std::string>())
            file = tokenList.appendFileIfNew(loc.at("file").get<std::string>());
        if (loc.count("line") && loc.at("line").is<int64_t>())
            line = loc.at("line").get<int64_t>();
        if (loc.count("col") && loc.at("col").is<int64_t>())
            col = loc.at("col").get<int64_t>();
    } else if (mJsonObject.count("range") && mJsonObject.at("range").is<picojson::object>()) {
        const picojson::object &range = mJsonObject.at("range").get<picojson::object>();
        if (range.count("begin") && range.at("begin").is<picojson::object>()) {
            const picojson::object &begin = range.at("begin").get<picojson::object>();
            if (begin.count("file") && begin.at("file").is<std::string>())
                file = tokenList.appendFileIfNew(begin.at("file").get<std::string>());
            if (begin.count("line") && begin.at("line").is<int64_t>())
                line = begin.at("line").get<int64_t>();
            if (begin.count("col") && begin.at("col").is<int64_t>())
                col = begin.at("col").get<int64_t>();
        }
    }
    mFile = file;
    mLine = line;
    mCol = col;
    for (const auto& child: children) {
        if (child)
            child->setLocations(tokenList, file, line, col);
    }
}

Token *clangimport::AstNode::addtoken(TokenList &tokenList, const std::string &str, bool valueType)
{
    const Scope *scope = getNestedInScope(tokenList);
    tokenList.addtoken(str, mLine, mCol, mFile);
    tokenList.back()->scope(scope);
    if (valueType)
        setValueType(tokenList.back());
    return tokenList.back();
}

const ::Type * clangimport::AstNode::addTypeTokens(TokenList &tokenList, const std::string &str, const Scope *scope)
{
    if (str.find("\':\'") != std::string::npos) {
        return addTypeTokens(tokenList, str.substr(0, str.find("\':\'") + 1), scope);
    }

    if (startsWith(str, "'enum (anonymous"))
        return nullptr;

    std::string type;
    if (str.find(" (") != std::string::npos) {
        if (str.find('<') != std::string::npos)
            type = str.substr(1, str.find('<')) + "...>";
        else
            type = str.substr(1,str.find(" (")-1);
    } else
        type = unquote(str);

    if (type.find("(*)(") != std::string::npos) {
        type.erase(type.find("(*)("));
        type += "*";
    }
    if (type.find('(') != std::string::npos)
        type.erase(type.find('('));

    // TODO: put in a helper?
    std::stack<Token *> lpar;
    for (const std::string &s: splitString(type)) {
        Token *tok = addtoken(tokenList, s, false);
        if (tok->str() == "(" || tok->str() == "[")
            lpar.push(tok);
        else if (tok->str() == ")" || tok->str() == "]") {
            Token::createMutualLinks(tok, lpar.top());
            lpar.pop();
        }
    }

    // Set Type
    if (!scope) {
        scope = tokenList.back() ? tokenList.back()->scope() : nullptr;
        if (!scope)
            return nullptr;
    }
    for (const Token *typeToken = tokenList.back(); Token::Match(typeToken, "&|*|%name%"); typeToken = typeToken->previous()) {
        if (!typeToken->isName())
            continue;
        const ::Type *recordType = scope->symdb.findVariableType(scope, typeToken);
        if (recordType) {
            const_cast<Token*>(typeToken)->type(recordType);
            return recordType;
        }
    }
    return nullptr;
}

void clangimport::AstNode::addFullScopeNameTokens(TokenList &tokenList, const Scope *recordScope)
{
    if (!recordScope)
        return;
    std::list<const Scope *> scopes;
    while (recordScope && recordScope != tokenList.back()->scope() && !recordScope->isExecutable()) {
        scopes.push_front(recordScope);
        recordScope = recordScope->nestedIn;
    }
    for (const Scope *s: scopes) {
        if (!s->className.empty()) {
            addtoken(tokenList, s->className);
            addtoken(tokenList, "::");
        }
    }
}

const Scope *clangimport::AstNode::getNestedInScope(TokenList &tokenList)
{
    if (!tokenList.back())
        return &mData.mSymbolDatabase.scopeList.front();
    if (tokenList.back()->str() == "}" && mData.mNotScope.find(tokenList.back()) == mData.mNotScope.end())
        return tokenList.back()->scope()->nestedIn;
    return tokenList.back()->scope();
}

void clangimport::AstNode::setValueType(Token *tok)
{
    const std::string &type = getQualTypeBefore(getQualType());

    if (type.find('<') != std::string::npos)
        // TODO
        return;

    TokenList decl(mData.mSettings, tok->isCpp() ? Standards::Language::CPP : Standards::Language::C);
    addTypeTokens(decl, type, tok->scope());
    if (!decl.front())
        return;

    ValueType valueType = ValueType::parseDecl(decl.front(), mData.mSettings);
    if (valueType.type != ValueType::Type::UNKNOWN_TYPE)
        tok->setValueType(new ValueType(std::move(valueType)));
}

Scope *clangimport::AstNode::createScope(TokenList &tokenList, ScopeType scopeType, AstNodePtr astNode, const Token *def)
{
    std::vector<AstNodePtr> children2{std::move(astNode)};
    return createScope(tokenList, scopeType, children2, def);
}

Scope *clangimport::AstNode::createScope(TokenList &tokenList, ScopeType scopeType, const std::vector<AstNodePtr> & children2, const Token *def)
{
    SymbolDatabase &symbolDatabase = mData.mSymbolDatabase;

    auto *nestedIn = const_cast<Scope *>(getNestedInScope(tokenList));

    symbolDatabase.scopeList.emplace_back(nestedIn->symdb, nullptr, nestedIn);
    Scope *scope = &symbolDatabase.scopeList.back();
    if (scopeType == ScopeType::eEnum)
        scope->enumeratorList.reserve(children2.size());
    nestedIn->nestedList.push_back(scope);
    scope->type = scopeType;
    scope->classDef = def; // TODO: pass into ctor
    if (Token::Match(def, "if|for|while (")) {
        std::map<const Variable *, const Variable *> replaceVar;
        for (const Token *vartok = def->tokAt(2); vartok; vartok = vartok->next()) {
            if (!vartok->variable())
                continue;
            if (vartok->variable()->nameToken() == vartok) {
                const Variable *from = vartok->variable();
                scope->varlist.emplace_back(*from, scope);
                Variable *to = &scope->varlist.back();
                replaceVar[from] = to;
                mData.replaceVarDecl(from, to);
            }
            if (replaceVar.find(vartok->variable()) != replaceVar.end())
                const_cast<Token *>(vartok)->variable(replaceVar[vartok->variable()]);
        }
        std::list<Variable> &varlist = const_cast<Scope *>(def->scope())->varlist;
        for (auto var = varlist.cbegin(); var != varlist.cend();) {
            if (replaceVar.find(&(*var)) != replaceVar.end())
                var = varlist.erase(var);
            else
                ++var;
        }
    }
    scope->bodyStart = addtoken(tokenList, "{");
    tokenList.back()->scope(scope);
    mData.scopeAccessControl[scope] = scope->defaultAccess();
    for (const AstNodePtr &astNode: children2) {
        if (astNode->mKind == "VisibilityAttr")
            continue;
        if (astNode->mKind == AccessSpecDecl) {
            const std::string access = astNode->mJsonObject.at("access").get<std::string>();
            astNode->addtoken(tokenList, access);
            astNode->addtoken(tokenList, ":");
            if (access == "private")
                mData.scopeAccessControl[scope] = AccessControl::Private;
            if (access == "protected")
                mData.scopeAccessControl[scope] = AccessControl::Protected;
            if (access == "public")
                mData.scopeAccessControl[scope] = AccessControl::Public;
            continue;
        }
        astNode->createTokens(tokenList);
        if (scopeType == ScopeType::eEnum)
            astNode->addtoken(tokenList, ",");
        else if (!Token::Match(tokenList.back(), "[;{}]"))
            astNode->addtoken(tokenList, ";");
    }
    scope->bodyEnd = addtoken(tokenList, "}");
    Token::createMutualLinks(const_cast<Token*>(scope->bodyStart), const_cast<Token*>(scope->bodyEnd));
    mData.scopeAccessControl.erase(scope);
    return scope;
}

Token *clangimport::AstNode::createTokens(TokenList &tokenList)
{
    if (mKind == ArraySubscriptExpr) {
        Token *array = getChild(0)->createTokens(tokenList);
        Token *bracket1 = addtoken(tokenList, "[");
        Token *index = children[1]->createTokens(tokenList);
        Token *bracket2 = addtoken(tokenList, "]");
        bracket1->astOperand1(array);
        bracket1->astOperand2(index);
        bracket1->link(bracket2);
        bracket2->link(bracket1);
        return bracket1;
    }
    if (mKind == BinaryOperator) {
        Token *tok1 = getChild(0)->createTokens(tokenList);
        Token *binop = addtoken(tokenList, getSpelling());
        Token *tok2 = children[1]->createTokens(tokenList);
        binop->astOperand1(tok1);
        binop->astOperand2(tok2);
        return binop;
    }
    if (mKind == BreakStmt)
        return addtoken(tokenList, "break");
    if (mKind == CharacterLiteral) {
        const int c = mJsonObject.at("value").get<int64_t>();
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
    if (mKind == CallExpr)
        return createTokensCall(tokenList);
    if (mKind == CaseStmt) {
        Token *caseToken = addtoken(tokenList, "case");
        Token *exprToken = getChild(0)->createTokens(tokenList);
        caseToken->astOperand1(exprToken);
        addtoken(tokenList, ":");
        children.back()->createTokens(tokenList);
        return nullptr;
    }
    if (mKind == ClassTemplateDecl) {
        for (const AstNodePtr& child: children) {
            if (child->mKind == ClassTemplateSpecializationDecl)
                child->createTokens(tokenList);
        }
        return nullptr;
    }
    if (mKind == ClassTemplateSpecializationDecl) {
        createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (mKind == ConditionalOperator) {
        Token *expr1 = getChild(0)->createTokens(tokenList);
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
    if (mKind == CompoundAssignOperator) {
        Token *lhs = getChild(0)->createTokens(tokenList);
        Token *assign = addtoken(tokenList, getSpelling());
        Token *rhs = children[1]->createTokens(tokenList);
        assign->astOperand1(lhs);
        assign->astOperand2(rhs);
        return assign;
    }
    if (mKind == CompoundStmt) {
        for (const AstNodePtr& child: children) {
            child->createTokens(tokenList);
            if (!Token::Match(tokenList.back(), "[;{}]"))
                child->addtoken(tokenList, ";");
        }
        return nullptr;
    }
    if (mKind == ConstantExpr)
        return children.back()->createTokens(tokenList);
    if (mKind == ContinueStmt)
        return addtoken(tokenList, "continue");
    if (mKind == CStyleCastExpr) {
        Token *par1 = addtoken(tokenList, "(");
        addTypeTokens(tokenList, '\'' + getQualType() + '\'');
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(getChild(0)->createTokens(tokenList));
        return par1;
    }
    if (mKind == CXXBindTemporaryExpr)
        return getChild(0)->createTokens(tokenList);
    if (mKind == CXXBoolLiteralExpr) {
        addtoken(tokenList, mJsonObject.at("value").get<bool>() ? "true" : "false");
        tokenList.back()->setValueType(new ValueType(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::BOOL, 0));
        return tokenList.back();
    }
    if (mKind == CXXConstructExpr) {
        if (!children.empty())
            return getChild(0)->createTokens(tokenList);
        addTypeTokens(tokenList, '\'' + getQualType() + '\'');
        Token *type = tokenList.back();
        Token *par1 = addtoken(tokenList, "(");
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(type);
        return par1;
    }
    if (mKind == CXXConstructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (mKind == CXXDeleteExpr) {
        addtoken(tokenList, "delete");
        getChild(0)->createTokens(tokenList);
        return nullptr;
    }
    if (mKind == CXXDestructorDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (mKind == CXXForRangeStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");

        const auto it1 = std::find_if(children.rbegin(), children.rend(), [&](const AstNodePtr& c) {
            return c->mKind == DeclStmt;
        });
        AstNodePtr varDecl = (*it1)->getChild(0);
        varDecl->children.clear();
        Token *expr1 = varDecl->createTokens(tokenList);
        Token *colon = addtoken(tokenList, ":");

        auto it = std::find_if(children.cbegin(), children.cbegin() + 2, [&](const AstNodePtr& c) {
            return c && c->mKind == DeclStmt && c->getChild(0)->mKind == VarDecl;
        });
        if (it == children.cbegin() + 2)
            throw InternalError(tokenList.back(), "Failed to import CXXForRangeStmt. Range?");
        AstNodePtr range = (*it)->getChild(0)->getChild(0);

        Token *expr2 = range->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");

        par1->link(par2);
        par2->link(par1);

        colon->astOperand1(expr1);
        colon->astOperand2(expr2);
        par1->astOperand1(forToken);
        par1->astOperand2(colon);

        createScope(tokenList, ScopeType::eFor, children.back(), forToken);
        return nullptr;
    }
    if (mKind == CXXMethodDecl) {
        (void)mData.hasDecl(""); // <- do not show warning that hasDecl is not used
        /* JSON
           for (std::size_t i = 0; i+1 < mExtTokens.size(); ++i) {
            if (mExtTokens[i] == "prev" && !mData.hasDecl(mExtTokens[i+1]))
                return nullptr;
           }
         */
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (mKind == CXXMemberCallExpr)
        return createTokensCall(tokenList);
    if (mKind == CXXNewExpr) {
        Token *newtok = addtoken(tokenList, "new");
        if (children.size() == 1 && getChild(0)->mKind == CXXConstructExpr) {
            newtok->astOperand1(getChild(0)->createTokens(tokenList));
            return newtok;
        }
        std::string type = getQualType();
        if (type.find('*') != std::string::npos)
            type = type.erase(type.rfind('*'));
        addTypeTokens(tokenList, type);
        if (!children.empty()) {
            Token *bracket1 = addtoken(tokenList, "[");
            getChild(0)->createTokens(tokenList);
            Token *bracket2 = addtoken(tokenList, "]");
            bracket1->link(bracket2);
            bracket2->link(bracket1);
        }
        return newtok;
    }
    if (mKind == CXXNullPtrLiteralExpr)
        return addtoken(tokenList, "nullptr");
    if (mKind == CXXOperatorCallExpr)
        return createTokensCall(tokenList);
    if (mKind == CXXRecordDecl) {
        createTokensForCXXRecord(tokenList);
        return nullptr;
    }
    if (mKind == CXXStaticCastExpr || mKind == CXXFunctionalCastExpr) {
        Token *cast = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = getChild(0)->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(cast);
        par1->astOperand2(expr);
        setValueType(par1);
        return par1;
    }
    if (mKind == CXXStdInitializerListExpr)
        return getChild(0)->createTokens(tokenList);
    if (mKind == CXXTemporaryObjectExpr && !children.empty())
        return getChild(0)->createTokens(tokenList);
    if (mKind == CXXThisExpr)
        return addtoken(tokenList, "this");
    if (mKind == CXXThrowExpr) {
        Token *t = addtoken(tokenList, "throw");
        t->astOperand1(getChild(0)->createTokens(tokenList));
        return t;
    }
    if (mKind == DeclRefExpr) {
        const picojson::value &referencedDeclValue = mJsonObject.at("referencedDecl");
        const picojson::object &referencedDecl = referencedDeclValue.get<picojson::object>();
        const std::string addr = referencedDecl.at("id").get<std::string>();
        std::string name = referencedDecl.count("name") ? referencedDecl.at("name").get<std::string>() : "";
        Token *reftok = addtoken(tokenList, name.empty() ? "<NoName>" : std::move(name));
        mData.ref(addr, reftok);
        return reftok;
    }
    if (mKind == DeclStmt)
        return getChild(0)->createTokens(tokenList);
    if (mKind == DefaultStmt) {
        addtoken(tokenList, "default");
        addtoken(tokenList, ":");
        children.back()->createTokens(tokenList);
        return nullptr;
    }
    if (mKind == DoStmt) {
        addtoken(tokenList, "do");
        createScope(tokenList, ScopeType::eDo, getChild(0), tokenList.back());
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
    if (mKind == EnumConstantDecl) {
        Token *nameToken = addtoken(tokenList, getSpelling());
        auto *scope = const_cast<Scope *>(nameToken->scope());
        scope->enumeratorList.emplace_back(nameToken->scope());
        Enumerator *e = &scope->enumeratorList.back();
        e->name = nameToken;
        e->value = mData.enumValue++;
        e->value_known = true;
        mData.enumDecl(mJsonObject.at("id").get<std::string>(), nameToken, e);
        return nameToken;
    }
    if (mKind == EnumDecl) {
        mData.enumValue = 0;
        Token *enumtok = addtoken(tokenList, "enum");
        const std::string& name = getSpelling();
        const Token *nametok = name.empty() ? nullptr : addtoken(tokenList, name);
        if (mJsonObject.count("fixedUnderlyingType")) {
            const picojson::object& obj = mJsonObject.at("fixedUnderlyingType").get<picojson::object>();
            const std::string& qualType = obj.at("qualType").get<std::string>();
            addtoken(tokenList, ":");
            addTypeTokens(tokenList, qualType);
        }

        Scope *enumscope = createScope(tokenList, ScopeType::eEnum, children, enumtok);
        if (nametok)
            enumscope->className = nametok->str();
        if (enumscope->bodyEnd && Token::simpleMatch(enumscope->bodyEnd->previous(), ", }"))
            const_cast<Token *>(enumscope->bodyEnd)->deletePrevious();

        // Create enum type
        mData.mSymbolDatabase.typeList.emplace_back(enumtok, enumscope, enumtok->scope());
        enumscope->definedType = &mData.mSymbolDatabase.typeList.back();
        if (nametok)
            const_cast<Scope *>(enumtok->scope())->definedTypesMap[nametok->str()] = enumscope->definedType;

        return nullptr;
    }
    if (mKind == ExprWithCleanups)
        return getChild(0)->createTokens(tokenList);
    if (mKind == FieldDecl)
        return createTokensVarDecl(tokenList);
    if (mKind == FloatingLiteral)
        return addtoken(tokenList, mJsonObject.at("value").get<std::string>());
    if (mKind == ForStmt) {
        Token *forToken = addtoken(tokenList, "for");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr1 = getChild(0) ? children[0]->createTokens(tokenList) : nullptr;
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
        createScope(tokenList, ScopeType::eFor, children[4], forToken);
        return nullptr;
    }
    if (mKind == FunctionDecl) {
        createTokensFunctionDecl(tokenList);
        return nullptr;
    }
    if (mKind == FunctionTemplateDecl) {
        bool first = true;
        for (const AstNodePtr& child: children) {
            if (child->mKind == FunctionDecl) {
                if (!first)
                    child->createTokens(tokenList);
                first = false;
            }
        }
        return nullptr;
    }
    if (mKind == GotoStmt) {
        addtoken(tokenList, "goto");
        const std::string& targetLabelDeclId = mJsonObject.at("targetLabelDeclId").get<std::string>();
        addtoken(tokenList, mData.getLabel(targetLabelDeclId)->str());
        addtoken(tokenList, ";");
        return nullptr;
    }
    if (mKind == IfStmt) {
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
        createScope(tokenList, ScopeType::eIf, std::move(thenCode), iftok);
        if (elseCode) {
            elseCode->addtoken(tokenList, "else");
            createScope(tokenList, ScopeType::eElse, std::move(elseCode), tokenList.back());
        }
        return nullptr;
    }
    if (mKind == ImplicitCastExpr) {
        Token *expr = getChild(0)->createTokens(tokenList);
        setValueType(expr);
        return expr;
    }
    if (mKind == InitListExpr) {
        const Scope *scope = tokenList.back()->scope();
        Token *start = addtoken(tokenList, "{");
        start->scope(scope);
        for (const AstNodePtr& child: children) {
            if (tokenList.back()->str() != "{")
                addtoken(tokenList, ",");
            child->createTokens(tokenList);
        }
        Token *end = addtoken(tokenList, "}");
        end->scope(scope);
        start->link(end);
        end->link(start);
        mData.mNotScope.insert(end);
        return start;
    }
    if (mKind == IntegerLiteral)
        return addtoken(tokenList, mJsonObject.at("value").get<std::string>());
    if (mKind == LabelStmt) {
        const std::string declId = mJsonObject.at("declId").get<std::string>();
        mData.labelDecl(declId, addtoken(tokenList, getSpelling()));
        addtoken(tokenList, ":");
        for (const auto& child: children)
            child->createTokens(tokenList);
        return nullptr;
    }
    if (mKind == LinkageSpecDecl)
        return nullptr;
    if (mKind == MaterializeTemporaryExpr)
        return getChild(0)->createTokens(tokenList);
    if (mKind == MemberExpr) {
        Token *s = getChild(0)->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        std::string memberName = getSpelling();
        if (startsWith(memberName, "->")) {
            dot->originalName("->");
            memberName = memberName.substr(2);
        } else if (startsWith(memberName, ".")) {
            memberName = memberName.substr(1);
        }
        if (memberName.empty())
            memberName = "<unknown>";
        Token *member = addtoken(tokenList, memberName);
        mData.ref(mJsonObject.at("referencedMemberDecl").get<std::string>(), member);
        dot->astOperand1(s);
        dot->astOperand2(member);
        return dot;
    }
    if (mKind == NamespaceDecl) {
        if (children.empty())
            return nullptr;
        const Token *defToken = addtoken(tokenList, "namespace");
        const std::string &name = getSpelling();
        const Token* nameToken = (!name.empty()) ? addtoken(tokenList, name) : nullptr;
        Scope *scope = createScope(tokenList, ScopeType::eNamespace, children, defToken);
        if (nameToken)
            scope->className = nameToken->str();
        return nullptr;
    }
    if (mKind == NullStmt)
        return addtoken(tokenList, ";");
    if (mKind == ParenExpr) {
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = getChild(0)->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return expr;
    }
    if (mKind == RecordDecl) {
        const Token *classDef = addtoken(tokenList, "struct");
        const std::string &recordName = getSpelling();
        if (!recordName.empty())
            addtoken(tokenList, getSpelling());
        if (!isDefinition()) {
            addtoken(tokenList, ";");
            return nullptr;
        }

        Scope *recordScope = createScope(tokenList, ScopeType::eStruct, children, classDef);
        mData.mSymbolDatabase.typeList.emplace_back(classDef, recordScope, classDef->scope());
        recordScope->definedType = &mData.mSymbolDatabase.typeList.back();
        if (!recordName.empty()) {
            recordScope->className = recordName;
            const_cast<Scope *>(classDef->scope())->definedTypesMap[recordName] = recordScope->definedType;
        }

        return nullptr;
    }
    if (mKind == ReturnStmt) {
        Token *tok1 = addtoken(tokenList, "return");
        if (!children.empty()) {
            getChild(0)->setValueType(tok1);
            tok1->astOperand1(getChild(0)->createTokens(tokenList));
        }
        return tok1;
    }
    if (mKind == StringLiteral) {
        Token* tok = addtoken(tokenList, mJsonObject.at("value").get<std::string>());
        setValueType(tok);
        return tok;
    }
    if (mKind == SwitchStmt) {
        Token *tok1 = addtoken(tokenList, "switch");
        Token *par1 = addtoken(tokenList, "(");
        Token *expr = children[children.size() - 2]->createTokens(tokenList);
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(expr);
        createScope(tokenList, ScopeType::eSwitch, children.back(), tok1);
        return nullptr;
    }
    if (mKind == TypedefDecl) {
        addtoken(tokenList, "typedef");
        addTypeTokens(tokenList, getQualType());
        return addtoken(tokenList, getSpelling());
    }
    if (mKind == UnaryOperator) {
        const std::string& spelling = getSpelling();
        const bool postfix = mJsonObject.count("isPostfix") > 0 && mJsonObject.at("isPostfix").get<bool>();
        if (postfix) {
            Token* tok = getChild(0)->createTokens(tokenList);
            Token *unaryOp = addtoken(tokenList, spelling);
            setValueType(unaryOp);
            unaryOp->astOperand1(tok);
            return unaryOp;
        }
        Token *unaryOp = addtoken(tokenList, spelling);
        setValueType(unaryOp);
        unaryOp->astOperand1(getChild(0)->createTokens(tokenList));
        return unaryOp;
    }
    if (mKind == UnaryExprOrTypeTraitExpr) {
        Token *tok1 = addtoken(tokenList, getSpelling());
        Token *par1 = addtoken(tokenList, "(");
        if (children.empty()) {
            if (mJsonObject.count("argType")) {
                const picojson::object& obj = mJsonObject.at("argType").get<picojson::object>();
                addTypeTokens(tokenList, obj.at("qualType").get<std::string>());
            }
        }
        else {
            AstNodePtr child = getChild(0);
            if (child && child->mKind == ParenExpr)
                child = child->getChild(0);
            Token *expr = child->createTokens(tokenList);
            child->setValueType(expr);
            par1->astOperand2(expr);
        }
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        par1->astOperand1(tok1);
        par1->astOperand2(par1->next());
        setValueType(par1);
        return par1;
    }
    if (mKind == UsingDirectiveDecl) {
        // Fixme
        return nullptr;
    }
    if (mKind == VarDecl || mKind == ParmVarDecl || mKind == FieldDecl)
        return createTokensVarDecl(tokenList);
    if (mKind == WhileStmt) {
        AstNodePtr cond = children[children.size() - 2];
        AstNodePtr body = children.back();
        Token *whiletok = addtoken(tokenList, "while");
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(whiletok);
        par1->astOperand2(cond->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        createScope(tokenList, ScopeType::eWhile, std::move(body), whiletok);
        return nullptr;
    }
    return addtoken(tokenList, "?" + mKind + "?");
}

Token * clangimport::AstNode::createTokensCall(TokenList &tokenList)
{
    int firstParam;
    Token *f;
    if (mKind == CXXOperatorCallExpr) {
        firstParam = 2;
        Token *obj = getChild(1)->createTokens(tokenList);
        Token *dot = addtoken(tokenList, ".");
        Token *op = getChild(0)->createTokens(tokenList);
        dot->astOperand1(obj);
        dot->astOperand2(op);
        f = dot;
    } else {
        firstParam = 1;
        f = getChild(0)->createTokens(tokenList);
    }
    f->setValueType(nullptr);
    Token *par1 = addtoken(tokenList, "(");
    par1->astOperand1(f);
    std::size_t args = 0;
    while (args < children.size() && children[args]->mKind != CXXDefaultArgExpr)
        args++;
    Token *child = nullptr;
    for (std::size_t c = firstParam; c < args; ++c) {
        if (child) {
            Token *comma = addtoken(tokenList, ",");
            comma->setValueType(nullptr);
            comma->astOperand1(child);
            comma->astOperand2(children[c]->createTokens(tokenList));
            child = comma;
        } else {
            child = children[c]->createTokens(tokenList);
        }
    }
    par1->astOperand2(child);
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);
    return par1;
}

void clangimport::AstNode::createTokensFunctionDecl(TokenList &tokenList)
{
    const std::string prev = mJsonObject.count("previousDecl") > 0 ? mJsonObject.at("previousDecl").get<std::string>() : "";
    const bool hasBody = !children.empty() && children.back()->mKind == CompoundStmt;
    const bool isStatic = getStorageClass() == "static";
    const bool isInline = mJsonObject.count("inline");
    const Scope* parentDeclContext = mJsonObject.count("parentDeclContextId") > 0 ?
                                     mData.getScope(mJsonObject.at("parentDeclContextId").get<std::string>())
                                     : nullptr;

    const Token *startToken = nullptr;

    SymbolDatabase &symbolDatabase = mData.mSymbolDatabase;
    if (mKind != CXXConstructorDecl && mKind != CXXDestructorDecl) {
        if (isStatic)
            addtoken(tokenList, "static");
        if (isInline)
            addtoken(tokenList, "inline");
        const Token * const before = tokenList.back();
        addTypeTokens(tokenList, '\'' + getQualType() + '\'');
        startToken = before ? before->next() : tokenList.front();
    }

    if (parentDeclContext)
        addFullScopeNameTokens(tokenList, parentDeclContext);

    Token *nameToken = addtoken(tokenList, getSpelling() + getTemplateParameters());
    auto *nestedIn = const_cast<Scope *>(nameToken->scope());

    if (!prev.empty())
        mData.ref(prev, nameToken);
    if (!nameToken->function()) {
        const std::string id = mJsonObject.at("id").get<std::string>();
        nestedIn->functionList.emplace_back(nameToken, getQualType());
        mData.funcDecl(id, nameToken, &nestedIn->functionList.back());
        if (mKind == CXXConstructorDecl)
            nestedIn->functionList.back().type = FunctionType::eConstructor;
        else if (mKind == CXXDestructorDecl)
            nestedIn->functionList.back().type = FunctionType::eDestructor;
        else
            nestedIn->functionList.back().retDef = startToken;
    }

    auto * const function = const_cast<Function*>(nameToken->function());

    if (prev.empty()) {
        auto accessControl = mData.scopeAccessControl.find(tokenList.back()->scope());
        if (accessControl != mData.scopeAccessControl.end())
            function->access = accessControl->second;
    }

    Scope *scope = nullptr;
    if (hasBody) {
        symbolDatabase.scopeList.emplace_back(symbolDatabase, nullptr, nestedIn);
        scope = &symbolDatabase.scopeList.back();
        scope->function = function;
        scope->classDef = nameToken; // TODO: pass into ctor
        scope->type = ScopeType::eFunction;
        scope->className = nameToken->str();
        scope->functionOf = parentDeclContext;
        nestedIn->nestedList.push_back(scope);
        function->hasBody(true);
        function->functionScope = scope;
    }

    Token *par1 = addtoken(tokenList, "(");
    if (!function->arg)
        function->arg = par1;
    function->token = nameToken;
    if (!function->nestedIn)
        function->nestedIn = nestedIn;
    function->argDef = par1;
    // Function arguments
    for (int i = 0; i < children.size(); ++i) {
        AstNodePtr child = children[i];
        if (child->mKind != ParmVarDecl)
            continue;
        if (tokenList.back() != par1)
            addtoken(tokenList, ",");
        const Type *recordType = addTypeTokens(tokenList, child->getQualType(), nestedIn);
        const Token *typeEndToken = tokenList.back();
        const std::string spelling = child->getSpelling();
        Token *vartok = nullptr;
        if (!spelling.empty())
            vartok = child->addtoken(tokenList, spelling);
        if (prev.empty()) {
            function->argumentList.emplace_back(vartok, child->getQualType(), nullptr, typeEndToken, i, AccessControl::Argument, recordType, scope);
            if (vartok) {
                const std::string id = child->mJsonObject.at("id").get<std::string>();
                mData.varDecl(id, vartok, &function->argumentList.back());
            }
        } else if (vartok) {
            const std::string id = child->mJsonObject.at("id").get<std::string>();
            mData.ref(id, vartok);
        }
    }
    Token *par2 = addtoken(tokenList, ")");
    par1->link(par2);
    par2->link(par1);

    if (function->isConst())
        addtoken(tokenList, "const");

    // Function body
    if (hasBody) {
        symbolDatabase.functionScopes.push_back(scope);
        Token *bodyStart = addtoken(tokenList, "{");
        bodyStart->scope(scope);
        children.back()->createTokens(tokenList);
        Token *bodyEnd = addtoken(tokenList, "}");
        scope->bodyStart = bodyStart;
        scope->bodyEnd = bodyEnd;
        bodyStart->link(bodyEnd);
        bodyEnd->link(bodyStart);
    } else {
        if (mJsonObject.count("explicitlyDefaulted") && mJsonObject.at("explicitlyDefaulted").get<std::string>() == "default") {
            addtoken(tokenList, "=");
            addtoken(tokenList, "default");
        }
        addtoken(tokenList, ";");
    }
}

void clangimport::AstNode::createTokensForCXXRecord(TokenList &tokenList)
{
    const std::string tagUsed = (mJsonObject.count("tagUsed") ? mJsonObject.at("tagUsed").get<std::string>() : "struct");
    Token * const classToken = addtoken(tokenList, tagUsed);
    std::string className = getSpelling();
    className += getTemplateParameters();
    addtoken(tokenList, className);
    // base classes
    /* JSON
       bool firstBase = true;
       for (const AstNodePtr &child: children) {
        if (child->mKind == "public" || child->mKind == "protected" || child->mKind == "private") {
            addtoken(tokenList, firstBase ? ":" : ",");
            addtoken(tokenList, child->mKind);
            addtoken(tokenList, unquote(child->mExtTokens.back()));
            firstBase = false;
        }
       }
     */
    // definition
    if (isDefinition()) {
        std::vector<AstNodePtr> children2;
        std::copy_if(children.cbegin(), children.cend(), std::back_inserter(children2), [](const AstNodePtr& child) {
            return child->mKind == CXXConstructorDecl ||
                   child->mKind == CXXDestructorDecl ||
                   child->mKind == CXXMethodDecl ||
                   child->mKind == FieldDecl ||
                   child->mKind == VarDecl ||
                   child->mKind == AccessSpecDecl ||
                   child->mKind == TypedefDecl;
        });
        Scope *scope = createScope(tokenList, (tagUsed == "struct") ? ScopeType::eStruct : ScopeType::eClass, children2, classToken);
        const std::string id = mJsonObject.at("id").get<std::string>();
        mData.scopeDecl(id, scope);
        scope->className = className;
        mData.mSymbolDatabase.typeList.emplace_back(classToken, scope, classToken->scope());
        scope->definedType = &mData.mSymbolDatabase.typeList.back();
        const_cast<Scope *>(classToken->scope())->definedTypesMap[className] = scope->definedType;
    }
    addtoken(tokenList, ";");
    tokenList.back()->scope(classToken->scope());
}

Token * clangimport::AstNode::createTokensVarDecl(TokenList &tokenList)
{
    const std::string id = mJsonObject.at("id").get<std::string>();
    if (getStorageClass() == "static")
        addtoken(tokenList, "static");
    const std::string qualType = getQualType();
    const std::string name = getSpelling();
    const Token *startToken = tokenList.back();
    const ::Type *recordType = addTypeTokens(tokenList, getQualTypeBefore(qualType));
    if (!startToken)
        startToken = tokenList.front();
    else if (startToken->str() != "static")
        startToken = startToken->next();
    const Token* const typeEndToken = tokenList.back();
    Token *vartok1 = name.empty() ? nullptr : addtoken(tokenList, name);
    if (vartok1)
        setValueType(vartok1);
    const std::string &qualTypeAfter = getQualTypeAfter(qualType);
    if (!qualTypeAfter.empty())
        addTypeTokens(tokenList, qualTypeAfter);
    auto *scope = const_cast<Scope *>(tokenList.back()->scope());
    scope->varlist.emplace_back(vartok1, unquote(qualType), startToken, typeEndToken, 0, scope->defaultAccess(), recordType, scope);
    mData.varDecl(id, vartok1, &scope->varlist.back());
    if (!children.empty() && mJsonObject.count("init") && mJsonObject.at("init").get<std::string>() == "c") {
        Token *eq = addtoken(tokenList, "=");
        eq->astOperand1(vartok1);
        eq->astOperand2(children.back()->createTokens(tokenList));
        return eq;
    }
    /* JSON
       if (mExtTokens.back() == "callinit") {
        Token *par1 = addtoken(tokenList, "(");
        par1->astOperand1(vartok1);
        par1->astOperand2(getChild(0)->createTokens(tokenList));
        Token *par2 = addtoken(tokenList, ")");
        par1->link(par2);
        par2->link(par1);
        return par1;
       }
       if (mExtTokens.back() == "listinit") {
        return getChild(0)->createTokens(tokenList);
       }
     */
    return vartok1;
}

static void setTypes(TokenList &tokenList)
{
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            for (Token *typeToken = tok->tokAt(2); typeToken->str() != ")"; typeToken = typeToken->next()) {
                if (typeToken->type())
                    continue;
                typeToken->type(typeToken->scope()->findType(typeToken->str()));
            }
        }
    }
}

static void setValues(const Tokenizer &tokenizer, const SymbolDatabase *symbolDatabase)
{
    const Settings & settings = tokenizer.getSettings();

    for (const Scope& scope : symbolDatabase->scopeList) {
        if (!scope.definedType)
            continue;

        MathLib::bigint typeSize = 0;
        for (const Variable &var: scope.varlist) {
            const int mul = std::accumulate(var.dimensions().cbegin(), var.dimensions().cend(), 1, [](int v, const Dimension& dim) {
                return v * dim.num;
            });
            if (var.valueType())
                typeSize += mul * var.valueType()->typeSize(settings.platform, true);
        }
        scope.definedType->sizeOf = typeSize;
    }

    for (auto *tok = const_cast<Token*>(tokenizer.tokens()); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof (")) {
            ValueType vt = ValueType::parseDecl(tok->tokAt(2), settings);
            const MathLib::bigint sz = vt.typeSize(settings.platform, true);
            if (sz <= 0)
                continue;
            long long mul = 1;
            for (const Token *arrtok = tok->linkAt(1)->previous(); arrtok; arrtok = arrtok->previous()) {
                const std::string &a = arrtok->str();
                if (a.size() > 2 && a[0] == '[' && a.back() == ']')
                    mul *= strToInt<long long>(a.substr(1));
                else
                    break;
            }
            ValueFlow::Value v(mul * sz);
            v.setKnown();
            tok->next()->addValue(v);
        }
    }
}

static void parseTree(clangimport::AstNode& astNode, clangimport::Data &data) {
    const picojson::object& obj = astNode.getJsonObject();
    if (obj.count("inner")) {
        for (const picojson::value& child: obj.at("inner").get<picojson::array>()) {
            const picojson::object& childObject = child.get<picojson::object>();
            if (childObject.count("kind") == 0) {
                astNode.children.push_back({});
                continue;
            }

            astNode.children.emplace_back(std::make_shared<clangimport::AstNode>(childObject, data));
            parseTree(*astNode.children.back(), data);
        }
    }
}

static void parseTranslationUnitDecl(TokenList& tokenList, clangimport::Data &data, const picojson::object& obj) {
    if (obj.count("kind") == 0)
        return;
    const std::string kind = obj.at("kind").get<std::string>();
    if (kind == TranslationUnitDecl && obj.count("inner")) {
        for (const picojson::value& child: obj.at("inner").get<picojson::array>()) {
            clangimport::AstNode astNode(child.get<picojson::object>(), data);
            parseTree(astNode, data);
            astNode.createTokens1(tokenList);
        }
    }
}

void clangimport::parseClangAstDump(Tokenizer &tokenizer, const std::string &json)
{
    TokenList &tokenList = tokenizer.list;

    tokenizer.createSymbolDatabase();
    auto *symbolDatabase = const_cast<SymbolDatabase *>(tokenizer.getSymbolDatabase());
    symbolDatabase->scopeList.emplace_back(*symbolDatabase, nullptr, nullptr);
    symbolDatabase->scopeList.back().type = ScopeType::eGlobal;

    picojson::value res;
    const std::string err = picojson::parse(res, json);
    if (!err.empty())
        throw InternalError(nullptr, "Failed to parse Clang AST. Bad JSON format: " + err);
    if (!res.is<picojson::object>())
        throw InternalError(nullptr, "Failed to parse Clang AST. Bad JSON format: Not an object");

    clangimport::Data data(tokenizer.getSettings(), *symbolDatabase);

    parseTranslationUnitDecl(tokenList, data, res.get<picojson::object>());

    // Validation
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "(|)|[|]|{|}") && !tok->link())
            throw InternalError(tok, "Token::link() is not set properly");
    }

    if (tokenList.front())
        tokenList.front()->assignIndexes();
    symbolDatabase->clangSetVariables(data.getVariableList());
    symbolDatabase->createSymbolDatabaseExprIds();
    tokenList.clangSetOrigFiles();
    setTypes(tokenList);
    setValues(tokenizer, symbolDatabase);
}

