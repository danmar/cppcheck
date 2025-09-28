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

#include "checkunionzeroinit.h"

#include <sstream>
#include <unordered_map>
#include <vector>

#include "errortypes.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

static const CWE CWEXXX(0U); /* TODO */

static const std::string noname;

// Register this check class (by creating a static instance of it)
namespace {
    CheckUnionZeroInit instance;
}

struct UnionMember {
    UnionMember()
        : name(noname)
        , size(0) {}

    UnionMember(const std::string &name_, size_t size_)
        : name(name_)
        , size(size_) {}

    const std::string &name;
    size_t size;
};

struct Union {
    Union(const Scope *scope_, const std::string &name_)
        : scope(scope_)
        , name(name_) {}

    const Scope *scope;
    const std::string &name;
    std::vector<UnionMember> members;

    const UnionMember *largestMember() const {
        const UnionMember *largest = nullptr;
        for (const UnionMember &m : members) {
            if (m.size == 0)
                return nullptr;
            if (largest == nullptr || m.size > largest->size)
                largest = &m;
        }
        return largest;
    }

    bool isLargestMemberFirst() const {
        const UnionMember *largest = largestMember();
        return largest == nullptr || largest == &members[0];
    }
};

static UnionMember parseUnionMember(const Variable &var,
                                    const Settings &settings)
{
    const Token *nameToken = var.nameToken();
    if (nameToken == nullptr)
        return UnionMember();

    const ValueType *vt = nameToken->valueType();
    size_t size = 0;
    if (var.isArray()) {
        size = var.dimension(0);
    } else if (vt != nullptr) {
        size = ValueFlow::getSizeOf(*vt, settings,
                                    ValueFlow::Accuracy::ExactOrZero);
    }
    return UnionMember(nameToken->str(), size);
}

static std::vector<Union> parseUnions(const SymbolDatabase &symbolDatabase,
                                      const Settings &settings)
{
    std::vector<Union> unions;

    for (const Scope &scope : symbolDatabase.scopeList) {
        if (scope.type != ScopeType::eUnion)
            continue;

        Union u(&scope, scope.className);
        for (const Variable &var : scope.varlist) {
            u.members.push_back(parseUnionMember(var, settings));
        }
        unions.push_back(u);
    }

    return unions;
}

static bool isZeroInitializer(const Token *tok)
{
    return Token::simpleMatch(tok, "= { 0 } ;") ||
           Token::simpleMatch(tok, "= { } ;");
}

void CheckUnionZeroInit::check()
{
    if (!mSettings->severity.isEnabled(Severity::portability))
        return;

    logChecker("CheckUnionZeroInit::check"); // portability

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    std::unordered_map<const Scope *, Union> unionsByScopeId;
    const std::vector<Union> unions = parseUnions(*symbolDatabase, *mSettings);
    for (const Union &u : unions) {
        unionsByScopeId.insert({u.scope, u});
    }

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isName() || !isZeroInitializer(tok->next()))
            continue;

        const ValueType *vt = tok->valueType();
        if (vt == nullptr || vt->typeScope == nullptr)
            continue;
        auto it = unionsByScopeId.find(vt->typeScope);
        if (it == unionsByScopeId.end())
            continue;
        const Union &u = it->second;
        if (!u.isLargestMemberFirst()) {
            const UnionMember *largestMember = u.largestMember();
            assert(largestMember != nullptr);
            unionZeroInitError(tok, *largestMember);
        }
    }
}

void CheckUnionZeroInit::runChecks(const Tokenizer &tokenizer,
                                   ErrorLogger *errorLogger)
{
    CheckUnionZeroInit(&tokenizer, &tokenizer.getSettings(), errorLogger).check();
}

void CheckUnionZeroInit::unionZeroInitError(const Token *tok,
                                            const UnionMember& largestMember)
{
    reportError(tok, Severity::portability, "UnionZeroInit",
                "$symbol:" + (tok != nullptr ? tok->str() : "") + "\n"
                "Zero initializing union '$symbol' does not guarantee " +
                "its complete storage to be zero initialized as its largest member " +
                "is not declared as the first member. Consider making " +
                largestMember.name + " the first member or favor memset().",
                CWEXXX, Certainty::normal);
}

void CheckUnionZeroInit::getErrorMessages(ErrorLogger *errorLogger,
                                          const Settings *settings) const
{
    CheckUnionZeroInit c(nullptr, settings, errorLogger);
    c.unionZeroInitError(nullptr, UnionMember());
}

std::string CheckUnionZeroInit::generateTestMessage(const Tokenizer &tokenizer,
                                                    const Settings &settings)
{
    std::stringstream ss;

    const std::vector<Union> unions = parseUnions(*tokenizer.getSymbolDatabase(),
                                                  settings);
    for (const Union &u : unions) {
        ss << "Union{";
        ss << "name=\"" << u.name << "\", ";
        ss << "scope=" << u.scope << ", ";
        ss << "isLargestMemberFirst=" << u.isLargestMemberFirst();
        ss << "}" << std::endl;

        const UnionMember *largest = u.largestMember();
        for (const UnionMember &m : u.members) {
            ss << "  Member{";
            ss << "name=\"" << m.name << "\", ";
            ss << "size=" << m.size;
            ss << "}";
            if (&m == largest)
                ss << " (largest)";
            ss << std::endl;
        }
    }

    return ss.str();
}
