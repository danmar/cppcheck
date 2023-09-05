/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
// Leaks when using auto variables
//---------------------------------------------------------------------------

#include "checkleakautovar.h"

#include "astutils.h"
#include "checkmemoryleak.h"  // <- CheckMemoryLeak::memoryLeak
#include "checknullpointer.h" // <- CheckNullPointer::isPointerDeRef
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "errortypes.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "vfvalue.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckLeakAutoVar instance;
}

static const CWE CWE672(672U);
static const CWE CWE415(415U);

// Hardcoded allocation types (not from library)
static const int NEW_ARRAY = -2;
static const int NEW = -1;

static const std::array<std::pair<std::string, std::string>, 4> alloc_failed_conds {{{"==", "0"}, {"<", "0"}, {"==", "-1"}, {"<=", "-1"}}};
static const std::array<std::pair<std::string, std::string>, 4> alloc_success_conds {{{"!=", "0"}, {">", "0"}, {"!=", "-1"}, {">=", "0"}}};

/**
 * @brief Is variable type some class with automatic deallocation?
 * @param var variable token
 * @return true unless it can be seen there is no automatic deallocation
 */
static bool isAutoDealloc(const Variable *var)
{
    if (var->valueType() && var->valueType()->type != ValueType::Type::RECORD && var->valueType()->type != ValueType::Type::UNKNOWN_TYPE)
        return false;

    // return false if the type is a simple record type without side effects
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check base class for side effects */
    /** @todo false negative: check constructors for side effects */
    if (var->typeScope() && var->typeScope()->numConstructors == 0 &&
        (var->typeScope()->varlist.empty() || var->type()->needInitialization == Type::NeedInitialization::True) &&
        var->type()->derivedFrom.empty())
        return false;

    return true;
}

template<std::size_t N>
static bool isVarTokComparison(const Token * tok, const Token ** vartok,
                               const std::array<std::pair<std::string, std::string>, N>& ops)
{
    return std::any_of(ops.cbegin(), ops.cend(), [&](const std::pair<std::string, std::string>& op) {
        return astIsVariableComparison(tok, op.first, op.second, vartok);
    });
}

//---------------------------------------------------------------------------

void VarInfo::print()
{
    std::cout << "size=" << alloctype.size() << std::endl;
    for (std::map<int, AllocInfo>::const_iterator it = alloctype.cbegin(); it != alloctype.cend(); ++it) {
        std::string strusage;
        const auto use = possibleUsage.find(it->first);
        if (use != possibleUsage.end())
            strusage = use->second.first;

        std::string status;
        switch (it->second.status) {
        case OWNED:
            status = "owned";
            break;
        case DEALLOC:
            status = "dealloc";
            break;
        case ALLOC:
            status = "alloc";
            break;
        case NOALLOC:
            status = "noalloc";
            break;
        case REALLOC:
            status = "realloc";
            break;
        default:
            status = "?";
            break;
        }

        std::cout << "status=" << status << " "
                  << "alloctype='" << it->second.type << "' "
                  << "possibleUsage='" << strusage << "' "
                  << "conditionalAlloc=" << (conditionalAlloc.find(it->first) != conditionalAlloc.end() ? "yes" : "no") << " "
                  << "referenced=" << (referenced.find(it->first) != referenced.end() ? "yes" : "no") << " "
                  << "reallocedFrom=" << it->second.reallocedFromType
                  << std::endl;
    }
}

void VarInfo::possibleUsageAll(const std::pair<std::string, Usage>& functionUsage)
{
    possibleUsage.clear();
    for (std::map<int, AllocInfo>::const_iterator it = alloctype.cbegin(); it != alloctype.cend(); ++it)
        possibleUsage[it->first] = functionUsage;
}


void CheckLeakAutoVar::leakError(const Token *tok, const std::string &varname, int type) const
{
    const CheckMemoryLeak checkmemleak(mTokenizer, mErrorLogger, mSettings);
    if (Library::isresource(type))
        checkmemleak.resourceLeakError(tok, varname);
    else
        checkmemleak.memleakError(tok, varname);
}

void CheckLeakAutoVar::mismatchError(const Token *deallocTok, const Token *allocTok, const std::string &varname) const
{
    const CheckMemoryLeak c(mTokenizer, mErrorLogger, mSettings);
    const std::list<const Token *> callstack = { allocTok, deallocTok };
    c.mismatchAllocDealloc(callstack, varname);
}

void CheckLeakAutoVar::deallocUseError(const Token *tok, const std::string &varname) const
{
    const CheckMemoryLeak c(mTokenizer, mErrorLogger, mSettings);
    c.deallocuseError(tok, varname);
}

void CheckLeakAutoVar::deallocReturnError(const Token *tok, const Token *deallocTok, const std::string &varname)
{
    const std::list<const Token *> locations = { deallocTok, tok };
    reportError(locations, Severity::error, "deallocret", "$symbol:" + varname + "\nReturning/dereferencing '$symbol' after it is deallocated / released", CWE672, Certainty::normal);
}

void CheckLeakAutoVar::configurationInfo(const Token* tok, const std::pair<std::string, VarInfo::Usage>& functionUsage)
{
    if (mSettings->checkLibrary && functionUsage.second == VarInfo::USED) {
        reportError(tok,
                    Severity::information,
                    "checkLibraryUseIgnore",
                    "--check-library: Function " + functionUsage.first + "() should have <use>/<leak-ignore> configuration");
    }
}

void CheckLeakAutoVar::doubleFreeError(const Token *tok, const Token *prevFreeTok, const std::string &varname, int type)
{
    const std::list<const Token *> locations = { prevFreeTok, tok };

    if (Library::isresource(type))
        reportError(locations, Severity::error, "doubleFree", "$symbol:" + varname + "\nResource handle '$symbol' freed twice.", CWE415, Certainty::normal);
    else
        reportError(locations, Severity::error, "doubleFree", "$symbol:" + varname + "\nMemory pointed to by '$symbol' is freed twice.", CWE415, Certainty::normal);
}


void CheckLeakAutoVar::check()
{
    if (mSettings->clang)
        return;

    logChecker("CheckLeakAutoVar::check"); // notclang

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Local variables that are known to be non-zero.
    const std::set<int> notzero;

    // Check function scopes
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->hasInlineOrLambdaFunction())
            continue;

        // Empty variable info
        VarInfo varInfo;

        checkScope(scope->bodyStart, varInfo, notzero, 0);
    }
}

static bool isVarUsedInTree(const Token *tok, nonneg int varid)
{
    if (!tok)
        return false;
    if (tok->varId() == varid)
        return true;
    if (tok->str() == "(" && Token::simpleMatch(tok->astOperand1(), "sizeof"))
        return false;
    return isVarUsedInTree(tok->astOperand1(), varid) || isVarUsedInTree(tok->astOperand2(), varid);
}

static bool isPointerReleased(const Token *startToken, const Token *endToken, nonneg int varid)
{
    for (const Token *tok = startToken; tok && tok != endToken; tok = tok->next()) {
        if (tok->varId() != varid)
            continue;
        if (Token::Match(tok, "%var% . release ( )"))
            return true;
        if (Token::Match(tok, "%var% ="))
            return false;
    }
    return false;
}

static bool isLocalVarNoAutoDealloc(const Token *varTok, const bool isCpp)
{
    // not a local variable nor argument?
    const Variable *var = varTok->variable();
    if (!var)
        return true;
    if (!var->isArgument() && (!var->isLocal() || var->isStatic()))
        return false;

    // Don't check reference variables
    if (var->isReference() && !var->isArgument())
        return false;

    // non-pod variable
    if (isCpp) {
        // Possibly automatically deallocated memory
        if (isAutoDealloc(var) && Token::Match(varTok, "%var% = new"))
            return false;
        if (!var->isPointer() && !var->typeStartToken()->isStandardType())
            return false;
    }
    return true;
}

/** checks if nameToken is a name of a function in a function call:
 *     func(arg)
 * or
 *     func<temp1_arg>(arg)
 * @param nameToken Function name token
 * @return opening parenthesis token or NULL if not a function call
 */

static const Token * isFunctionCall(const Token * nameToken)
{
    if (!nameToken->isStandardType() && nameToken->isName()) {
        nameToken = nameToken->next();
        // check if function is a template
        if (nameToken && nameToken->link() && nameToken->str() == "<") {
            // skip template arguments
            nameToken = nameToken->link()->next();
        }
        // check for '('
        if (nameToken && nameToken->link() && nameToken->str() == "(") {
            // returning opening parenthesis pointer
            return nameToken;
        }
    }
    return nullptr;
}

bool CheckLeakAutoVar::checkScope(const Token * const startToken,
                                  VarInfo &varInfo,
                                  std::set<int> notzero,
                                  nonneg int recursiveCount)
{
#if ASAN
    static const nonneg int recursiveLimit = 300;
#elif defined(__MINGW32__)
    // testrunner crashes with stack overflow in CI
    static const nonneg int recursiveLimit = 600;
#else
    static const nonneg int recursiveLimit = 1000;
#endif
    if (++recursiveCount > recursiveLimit)    // maximum number of "else if ()"
        throw InternalError(startToken, "Internal limit: CheckLeakAutoVar::checkScope() Maximum recursive count of 1000 reached.", InternalError::LIMIT);

    std::map<int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    auto& possibleUsage = varInfo.possibleUsage;
    const std::set<int> conditionalAlloc(varInfo.conditionalAlloc);

    // Parse all tokens
    const Token * const endToken = startToken->link();
    for (const Token *tok = startToken; tok && tok != endToken; tok = tok->next()) {
        if (!tok->scope()->isExecutable()) {
            tok = tok->scope()->bodyEnd;
            if (!tok) // Ticket #6666 (crash upon invalid code)
                break;
        }

        // check each token
        {
            const Token * nextTok = checkTokenInsideExpression(tok, varInfo);
            if (nextTok) {
                tok = nextTok;
                continue;
            }
        }


        // look for end of statement
        if (!Token::Match(tok, "[;{},]") || Token::Match(tok->next(), "[;{},]"))
            continue;

        if (Token::Match(tok, "[;{},] %var% ["))
            continue;

        tok = tok->next();
        if (!tok || tok == endToken)
            break;

        if (Token::Match(tok, "const %type%"))
            tok = tok->tokAt(2);

        while (tok->str() == "(")
            tok = tok->next();
        while (tok->isUnaryOp("*") && tok->astOperand1()->isUnaryOp("&"))
            tok = tok->astOperand1()->astOperand1();

        // parse statement, skip to last member
        const Token *varTok = tok;
        while (Token::Match(varTok, "%name% ::|. %name% !!("))
            varTok = varTok->tokAt(2);

        const Token *ftok = tok;
        if (ftok->str() == "::")
            ftok = ftok->next();
        while (Token::Match(ftok, "%name% :: %name%"))
            ftok = ftok->tokAt(2);

        auto isAssignment = [](const Token* varTok) -> const Token* {
            if (varTok->varId()) {
                const Token* top = varTok;
                while (top->astParent()) {
                    top = top->astParent();
                    if (!Token::Match(top, "(|*|&|."))
                        break;
                }
                if (top->str() == "=" && succeeds(top, varTok))
                    return top;
            }
            return nullptr;
        };

        // assignment..
        if (const Token* const tokAssignOp = isAssignment(varTok)) {

            if (Token::simpleMatch(tokAssignOp->astOperand1(), "."))
                continue;
            // taking address of another variable..
            if (Token::Match(tokAssignOp, "= %var% [+;]")) {
                if (varTok->tokAt(2)->varId() != varTok->varId()) {
                    // If variable points at allocated memory => error
                    leakIfAllocated(varTok, varInfo);

                    // no multivariable checking currently => bail out for rhs variables
                    for (const Token *tok2 = varTok; tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ";") {
                            break;
                        }
                        if (tok2->varId()) {
                            varInfo.erase(tok2->varId());
                        }
                    }
                }
            }

            // right ast part (after `=` operator)
            const Token* tokRightAstOperand = tokAssignOp->astOperand2();
            while (tokRightAstOperand && tokRightAstOperand->isCast())
                tokRightAstOperand = tokRightAstOperand->astOperand2() ? tokRightAstOperand->astOperand2() : tokRightAstOperand->astOperand1();

            // is variable used in rhs?
            if (isVarUsedInTree(tokRightAstOperand, varTok->varId()))
                continue;

            // Variable has already been allocated => error
            if (conditionalAlloc.find(varTok->varId()) == conditionalAlloc.end())
                leakIfAllocated(varTok, varInfo);
            varInfo.erase(varTok->varId());

            if (!isLocalVarNoAutoDealloc(varTok, mTokenizer->isCPP()))
                continue;

            // allocation?
            const Token *const fTok = tokRightAstOperand ? tokRightAstOperand->previous() : nullptr;
            if (Token::Match(fTok, "%type% (")) {
                const Library::AllocFunc* f = mSettings->library.getAllocFuncInfo(fTok);
                if (f && f->arg == -1) {
                    VarInfo::AllocInfo& varAlloc = alloctype[varTok->varId()];
                    varAlloc.type = f->groupId;
                    varAlloc.status = VarInfo::ALLOC;
                    varAlloc.allocTok = fTok;
                }

                changeAllocStatusIfRealloc(alloctype, fTok, varTok);
            } else if (mTokenizer->isCPP() && Token::Match(varTok->tokAt(2), "new !!(")) {
                const Token* tok2 = varTok->tokAt(2)->astOperand1();
                const bool arrayNew = (tok2 && (tok2->str() == "[" || (Token::Match(tok2, "(|{") && tok2->astOperand1() && tok2->astOperand1()->str() == "[")));
                VarInfo::AllocInfo& varAlloc = alloctype[varTok->varId()];
                varAlloc.type = arrayNew ? NEW_ARRAY : NEW;
                varAlloc.status = VarInfo::ALLOC;
                varAlloc.allocTok = varTok->tokAt(2);
            }

            // Assigning non-zero value variable. It might be used to
            // track the execution for a later if condition.
            if (Token::Match(varTok->tokAt(2), "%num% ;") && MathLib::toLongNumber(varTok->strAt(2)) != 0)
                notzero.insert(varTok->varId());
            else if (Token::Match(varTok->tokAt(2), "- %type% ;") && varTok->tokAt(3)->isUpperCaseName())
                notzero.insert(varTok->varId());
            else
                notzero.erase(varTok->varId());
        }

        // if/else
        else if (Token::simpleMatch(tok, "if (")) {
            // Parse function calls inside the condition

            const Token * closingParenthesis = tok->linkAt(1);
            for (const Token *innerTok = tok->tokAt(2); innerTok && innerTok != closingParenthesis; innerTok = innerTok->next()) {
                // TODO: replace with checkTokenInsideExpression()

                if (!isLocalVarNoAutoDealloc(innerTok, mTokenizer->isCPP()))
                    continue;

                // Check assignments in the if-statement. Skip multiple assignments since we don't track those
                if (Token::Match(innerTok, "%var% =") && innerTok->astParent() == innerTok->next() &&
                    !(innerTok->next()->astParent() && innerTok->next()->astParent()->isAssignmentOp())) {
                    // allocation?
                    // right ast part (after `=` operator)
                    const Token* tokRightAstOperand = innerTok->next()->astOperand2();
                    while (tokRightAstOperand && tokRightAstOperand->isCast())
                        tokRightAstOperand = tokRightAstOperand->astOperand2() ? tokRightAstOperand->astOperand2() : tokRightAstOperand->astOperand1();
                    if (tokRightAstOperand && Token::Match(tokRightAstOperand->previous(), "%type% (")) {
                        const Token * fTok = tokRightAstOperand->previous();
                        const Library::AllocFunc* f = mSettings->library.getAllocFuncInfo(fTok);
                        if (f && f->arg == -1) {
                            VarInfo::AllocInfo& varAlloc = alloctype[innerTok->varId()];
                            varAlloc.type = f->groupId;
                            varAlloc.status = VarInfo::ALLOC;
                            varAlloc.allocTok = fTok;
                        } else {
                            // Fixme: warn about leak
                            alloctype.erase(innerTok->varId());
                        }
                        changeAllocStatusIfRealloc(alloctype, fTok, varTok);
                    } else if (mTokenizer->isCPP() && Token::Match(innerTok->tokAt(2), "new !!(")) {
                        const Token* tok2 = innerTok->tokAt(2)->astOperand1();
                        const bool arrayNew = (tok2 && (tok2->str() == "[" || (tok2->str() == "(" && tok2->astOperand1() && tok2->astOperand1()->str() == "[")));
                        VarInfo::AllocInfo& varAlloc = alloctype[innerTok->varId()];
                        varAlloc.type = arrayNew ? NEW_ARRAY : NEW;
                        varAlloc.status = VarInfo::ALLOC;
                        varAlloc.allocTok = innerTok->tokAt(2);
                    }
                }

                // check for function call
                const Token * const openingPar = isFunctionCall(innerTok);
                if (openingPar) {
                    const Library::AllocFunc* allocFunc = mSettings->library.getDeallocFuncInfo(innerTok);
                    // innerTok is a function name
                    const VarInfo::AllocInfo allocation(0, VarInfo::NOALLOC);
                    functionCall(innerTok, openingPar, varInfo, allocation, allocFunc);
                    innerTok = openingPar->link();
                }
            }

            if (Token::simpleMatch(closingParenthesis, ") {")) {
                VarInfo varInfo1(varInfo);  // VarInfo for if code
                VarInfo varInfo2(varInfo);  // VarInfo for else code

                // Skip expressions before commas
                const Token * astOperand2AfterCommas = tok->next()->astOperand2();
                while (Token::simpleMatch(astOperand2AfterCommas, ","))
                    astOperand2AfterCommas = astOperand2AfterCommas->astOperand2();

                // Recursively scan variable comparisons in condition
                visitAstNodes(astOperand2AfterCommas, [&](const Token *tok3) {
                    if (!tok3)
                        return ChildrenToVisit::none;
                    if (tok3->str() == "&&" || tok3->str() == "||") {
                        // FIXME: handle && ! || better
                        return ChildrenToVisit::op1_and_op2;
                    }
                    if (tok3->str() == "(" && Token::Match(tok3->astOperand1(), "UNLIKELY|LIKELY")) {
                        return ChildrenToVisit::op2;
                    }
                    if (tok3->str() == "(" && tok3->previous()->isName()) {
                        const std::vector<const Token *> params = getArguments(tok3->previous());
                        for (const Token *par : params) {
                            if (!par->isComparisonOp())
                                continue;
                            const Token *vartok = nullptr;
                            if (isVarTokComparison(par, &vartok, alloc_success_conds) ||
                                (isVarTokComparison(par, &vartok, alloc_failed_conds))) {
                                varInfo1.erase(vartok->varId());
                                varInfo2.erase(vartok->varId());
                            }
                        }
                        return ChildrenToVisit::none;
                    }

                    const Token *vartok = nullptr;
                    if (isVarTokComparison(tok3, &vartok, alloc_success_conds)) {
                        varInfo2.reallocToAlloc(vartok->varId());
                        varInfo2.erase(vartok->varId());
                        if (astIsVariableComparison(tok3, "!=", "0", &vartok) &&
                            (notzero.find(vartok->varId()) != notzero.end()))
                            varInfo2.clear();
                    } else if (isVarTokComparison(tok3, &vartok, alloc_failed_conds)) {
                        varInfo1.reallocToAlloc(vartok->varId());
                        varInfo1.erase(vartok->varId());
                    }
                    return ChildrenToVisit::none;
                });

                if (!checkScope(closingParenthesis->next(), varInfo1, notzero, recursiveCount)) {
                    varInfo.clear();
                    continue;
                }
                closingParenthesis = closingParenthesis->linkAt(1);
                if (Token::simpleMatch(closingParenthesis, "} else {")) {
                    if (!checkScope(closingParenthesis->tokAt(2), varInfo2, notzero, recursiveCount)) {
                        varInfo.clear();
                        return false;
                    }
                    tok = closingParenthesis->linkAt(2)->previous();
                } else {
                    tok = closingParenthesis->previous();
                }

                VarInfo old;
                old.swap(varInfo);

                std::map<int, VarInfo::AllocInfo>::const_iterator it;

                for (it = old.alloctype.cbegin(); it != old.alloctype.cend(); ++it) {
                    const int varId = it->first;
                    if (old.conditionalAlloc.find(varId) == old.conditionalAlloc.end())
                        continue;
                    if (varInfo1.alloctype.find(varId) == varInfo1.alloctype.end() ||
                        varInfo2.alloctype.find(varId) == varInfo2.alloctype.end()) {
                        varInfo1.erase(varId);
                        varInfo2.erase(varId);
                    }
                }

                // Conditional allocation in varInfo1
                for (it = varInfo1.alloctype.cbegin(); it != varInfo1.alloctype.cend(); ++it) {
                    if (varInfo2.alloctype.find(it->first) == varInfo2.alloctype.end() &&
                        old.alloctype.find(it->first) == old.alloctype.end()) {
                        varInfo.conditionalAlloc.insert(it->first);
                    }
                }

                // Conditional allocation in varInfo2
                for (it = varInfo2.alloctype.cbegin(); it != varInfo2.alloctype.cend(); ++it) {
                    if (varInfo1.alloctype.find(it->first) == varInfo1.alloctype.end() &&
                        old.alloctype.find(it->first) == old.alloctype.end()) {
                        varInfo.conditionalAlloc.insert(it->first);
                    }
                }

                // Conditional allocation/deallocation
                for (it = varInfo1.alloctype.cbegin(); it != varInfo1.alloctype.cend(); ++it) {
                    if (it->second.managed() && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo.conditionalAlloc.erase(it->first);
                        varInfo2.erase(it->first);
                    }
                }
                for (it = varInfo2.alloctype.cbegin(); it != varInfo2.alloctype.cend(); ++it) {
                    if (it->second.managed() && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo.conditionalAlloc.erase(it->first);
                        varInfo1.erase(it->first);
                    }
                }

                alloctype.insert(varInfo1.alloctype.cbegin(), varInfo1.alloctype.cend());
                alloctype.insert(varInfo2.alloctype.cbegin(), varInfo2.alloctype.cend());

                possibleUsage.insert(varInfo1.possibleUsage.cbegin(), varInfo1.possibleUsage.cend());
                possibleUsage.insert(varInfo2.possibleUsage.cbegin(), varInfo2.possibleUsage.cend());
            }
        }

        // unknown control.. (TODO: handle loops)
        else if ((Token::Match(tok, "%type% (") && Token::simpleMatch(tok->linkAt(1), ") {")) || Token::simpleMatch(tok, "do {")) {
            varInfo.clear();
            return false;
        }

        // return
        else if (tok->str() == "return") {
            ret(tok, varInfo);
            varInfo.clear();
        }

        // throw
        else if (mTokenizer->isCPP() && tok->str() == "throw") {
            bool tryFound = false;
            const Scope* scope = tok->scope();
            while (scope && scope->isExecutable()) {
                if (scope->type == Scope::eTry)
                    tryFound = true;
                scope = scope->nestedIn;
            }
            // If the execution leaves the function then treat it as return
            if (!tryFound)
                ret(tok, varInfo);
            varInfo.clear();
        }

        // delete
        else if (mTokenizer->isCPP() && tok->str() == "delete") {
            const Token * delTok = tok;
            if (Token::simpleMatch(delTok->astOperand1(), "."))
                continue;
            const bool arrayDelete = Token::simpleMatch(tok->next(), "[ ]");
            if (arrayDelete)
                tok = tok->tokAt(3);
            else
                tok = tok->next();
            if (tok->str() == "(")
                tok = tok->next();
            while (Token::Match(tok, "%name% ::|."))
                tok = tok->tokAt(2);
            const bool isnull = tok->hasKnownIntValue() && tok->values().front().intvalue == 0;
            if (!isnull && tok->varId() && tok->strAt(1) != "[") {
                const VarInfo::AllocInfo allocation(arrayDelete ? NEW_ARRAY : NEW, VarInfo::DEALLOC, delTok);
                changeAllocStatus(varInfo, allocation, tok, tok);
            }
        }

        // Function call..
        else if (const Token* openingPar = isFunctionCall(ftok)) {
            const Library::AllocFunc* af = mSettings->library.getDeallocFuncInfo(ftok);
            VarInfo::AllocInfo allocation(af ? af->groupId : 0, VarInfo::DEALLOC, ftok);
            if (allocation.type == 0)
                allocation.status = VarInfo::NOALLOC;
            if (Token::simpleMatch(ftok->astParent(), "(") && Token::simpleMatch(ftok->astParent()->astOperand2(), "."))
                continue;
            functionCall(ftok, openingPar, varInfo, allocation, af);

            tok = ftok->next()->link();

            // Handle scopes that might be noreturn
            if (allocation.status == VarInfo::NOALLOC && Token::simpleMatch(tok, ") ; }")) {
                if (ftok->isKeyword())
                    continue;
                const std::string functionName(mSettings->library.getFunctionName(ftok));
                bool unknown = false;
                if (mTokenizer->isScopeNoReturn(tok->tokAt(2), &unknown)) {
                    if (!unknown)
                        varInfo.clear();
                    else {
                        if (ftok->function() && !ftok->function()->isAttributeNoreturn() &&
                            !(ftok->function()->functionScope && mTokenizer->isScopeNoReturn(ftok->function()->functionScope->bodyEnd))) // check function scope
                            continue;
                        if (!mSettings->library.isLeakIgnore(functionName) && !mSettings->library.isUse(functionName)) {
                            const VarInfo::Usage usage = Token::simpleMatch(openingPar, "( )") ? VarInfo::NORET : VarInfo::USED; // TODO: check parameters passed to function
                            varInfo.possibleUsageAll({ functionName, usage });
                        }
                    }
                }
            }

            continue;
        }

        // goto => weird execution path
        else if (tok->str() == "goto") {
            varInfo.clear();
            return false;
        }

        // continue/break
        else if (Token::Match(tok, "continue|break ;")) {
            varInfo.clear();
        }

        // Check smart pointer
        else if (Token::Match(ftok, "%name% <") && mSettings->library.isSmartPointer(tok)) {
            const Token * typeEndTok = ftok->linkAt(1);
            if (!Token::Match(typeEndTok, "> %var% {|( %var% ,|)|}"))
                continue;

            tok = typeEndTok->linkAt(2);

            const int varid = typeEndTok->next()->varId();
            if (isPointerReleased(typeEndTok->tokAt(2), endToken, varid))
                continue;

            bool arrayDelete = false;
            if (Token::findsimplematch(ftok->next(), "[ ]", typeEndTok))
                arrayDelete = true;

            // Check deleter
            const Token * deleterToken = nullptr;
            const Token * endDeleterToken = nullptr;
            const Library::AllocFunc* af = nullptr;
            if (Token::Match(ftok, "unique_ptr < %type% ,")) {
                deleterToken = ftok->tokAt(4);
                endDeleterToken = typeEndTok;
            } else if (Token::Match(typeEndTok, "> %var% {|( %var% ,")) {
                deleterToken = typeEndTok->tokAt(5);
                endDeleterToken = typeEndTok->linkAt(2);
            }
            if (deleterToken) {
                // Skip the decaying plus in expressions like +[](T*){}
                if (deleterToken->str() == "+") {
                    deleterToken = deleterToken->next();
                }
                // Check if its a pointer to a function
                const Token * dtok = Token::findmatch(deleterToken, "& %name%", endDeleterToken);
                if (dtok) {
                    dtok = dtok->next();
                    af = mSettings->library.getDeallocFuncInfo(dtok);
                }
                if (!dtok || !af) {
                    const Token * tscopeStart = nullptr;
                    const Token * tscopeEnd = nullptr;
                    // If the deleter is a lambda, check if it calls the dealloc function
                    if (deleterToken->str() == "[" &&
                        Token::simpleMatch(deleterToken->link(), "] (") &&
                        // TODO: Check for mutable keyword
                        Token::simpleMatch(deleterToken->link()->linkAt(1), ") {")) {
                        tscopeStart = deleterToken->link()->linkAt(1)->tokAt(1);
                        tscopeEnd = tscopeStart->link();
                        // check user-defined deleter function
                    } else if (dtok && dtok->function()) {
                        const Scope* tscope = dtok->function()->functionScope;
                        if (tscope) {
                            tscopeStart = tscope->bodyStart;
                            tscopeEnd = tscope->bodyEnd;
                        }
                        // If the deleter is a class, check if class calls the dealloc function
                    } else if ((dtok = Token::findmatch(deleterToken, "%type%", endDeleterToken)) && dtok->type()) {
                        const Scope * tscope = dtok->type()->classScope;
                        if (tscope) {
                            tscopeStart = tscope->bodyStart;
                            tscopeEnd = tscope->bodyEnd;
                        }
                    }

                    if (tscopeStart && tscopeEnd) {
                        for (const Token *tok2 = tscopeStart; tok2 != tscopeEnd; tok2 = tok2->next()) {
                            af = mSettings->library.getDeallocFuncInfo(tok2);
                            if (af)
                                break;
                        }
                    } else { // there is a deleter, but we can't check it -> assume that it deallocates correctly
                        varInfo.clear();
                        continue;
                    }
                }
            }

            const Token * vtok = typeEndTok->tokAt(3);
            const VarInfo::AllocInfo allocation(af ? af->groupId : (arrayDelete ? NEW_ARRAY : NEW), VarInfo::OWNED, ftok);
            changeAllocStatus(varInfo, allocation, vtok, vtok);
        }
    }
    ret(endToken, varInfo, true);
    return true;
}


const Token * CheckLeakAutoVar::checkTokenInsideExpression(const Token * const tok, VarInfo &varInfo)
{
    // Deallocation and then dereferencing pointer..
    if (tok->varId() > 0) {
        // TODO : Write a separate checker for this that uses valueFlowForward.
        const std::map<int, VarInfo::AllocInfo>::const_iterator var = varInfo.alloctype.find(tok->varId());
        if (var != varInfo.alloctype.end()) {
            bool unknown = false;
            if (var->second.status == VarInfo::DEALLOC && CheckNullPointer::isPointerDeRef(tok, unknown, mSettings) && !unknown) {
                deallocUseError(tok, tok->str());
            } else if (Token::simpleMatch(tok->tokAt(-2), "= &")) {
                varInfo.erase(tok->varId());
            } else {
                // check if tok is assigned into another variable
                const Token *rhs = tok;
                while (rhs->astParent()) {
                    if (rhs->astParent()->str() == "=")
                        break;
                    rhs = rhs->astParent();
                }
                while (rhs->isCast()) {
                    rhs = rhs->astOperand2() ? rhs->astOperand2() : rhs->astOperand1();
                }
                if (rhs->varId() == tok->varId()) {
                    // simple assignment
                    varInfo.erase(tok->varId());
                } else if (rhs->str() == "(" && !mSettings->library.returnValue(rhs->astOperand1()).empty()) {
                    // #9298, assignment through return value of a function
                    const std::string &returnValue = mSettings->library.returnValue(rhs->astOperand1());
                    if (returnValue.compare(0, 3, "arg") == 0) {
                        int argn;
                        const Token *func = getTokenArgumentFunction(tok, argn);
                        if (func) {
                            const std::string arg = "arg" + std::to_string(argn + 1);
                            if (returnValue == arg) {
                                varInfo.erase(tok->varId());
                            }
                        }
                    }
                }
            }
        } else if (Token::Match(tok->previous(), "& %name% = %var% ;")) {
            varInfo.referenced.insert(tok->tokAt(2)->varId());
        }
    }

    // check for function call
    const Token * const openingPar = isFunctionCall(tok);
    if (openingPar) {
        const Library::AllocFunc* allocFunc = mSettings->library.getDeallocFuncInfo(tok);
        VarInfo::AllocInfo alloc(allocFunc ? allocFunc->groupId : 0, VarInfo::DEALLOC, tok);
        if (alloc.type == 0)
            alloc.status = VarInfo::NOALLOC;
        functionCall(tok, openingPar, varInfo, alloc, nullptr);
        const std::string &returnValue = mSettings->library.returnValue(tok);
        if (returnValue.compare(0, 3, "arg") == 0)
            // the function returns one of its argument, we need to process a potential assignment
            return openingPar;
        return isCPPCast(tok->astParent()) ? openingPar : openingPar->link();
    }

    return nullptr;
}


void CheckLeakAutoVar::changeAllocStatusIfRealloc(std::map<int, VarInfo::AllocInfo> &alloctype, const Token *fTok, const Token *retTok) const
{
    const Library::AllocFunc* f = mSettings->library.getReallocFuncInfo(fTok);
    if (f && f->arg == -1 && f->reallocArg > 0 && f->reallocArg <= numberOfArguments(fTok)) {
        const Token* argTok = getArguments(fTok).at(f->reallocArg - 1);
        if (alloctype.find(argTok->varId()) != alloctype.end()) {
            VarInfo::AllocInfo& argAlloc = alloctype[argTok->varId()];
            if (argAlloc.type != 0 && argAlloc.type != f->groupId)
                mismatchError(fTok, argAlloc.allocTok, argTok->str());
            argAlloc.status = VarInfo::REALLOC;
            argAlloc.allocTok = fTok;
        }
        VarInfo::AllocInfo& retAlloc = alloctype[retTok->varId()];
        retAlloc.type = f->groupId;
        retAlloc.status = VarInfo::ALLOC;
        retAlloc.allocTok = fTok;
        retAlloc.reallocedFromType = argTok->varId();
    }
}


void CheckLeakAutoVar::changeAllocStatus(VarInfo &varInfo, const VarInfo::AllocInfo& allocation, const Token* tok, const Token* arg)
{
    std::map<int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const std::map<int, VarInfo::AllocInfo>::iterator var = alloctype.find(arg->varId());
    if (var != alloctype.end()) {
        if (allocation.status == VarInfo::NOALLOC) {
            // possible usage
            varInfo.possibleUsage[arg->varId()] = { tok->str(), VarInfo::USED };
            if (var->second.status == VarInfo::DEALLOC && arg->previous()->str() == "&")
                varInfo.erase(arg->varId());
        } else if (var->second.managed()) {
            doubleFreeError(tok, var->second.allocTok, arg->str(), allocation.type);
            var->second.status = allocation.status;
        } else if (var->second.type != allocation.type && var->second.type != 0) {
            // mismatching allocation and deallocation
            mismatchError(tok, var->second.allocTok, arg->str());
            varInfo.erase(arg->varId());
        } else {
            // deallocation
            var->second.status = allocation.status;
            var->second.type = allocation.type;
            var->second.allocTok = allocation.allocTok;
        }
    } else if (allocation.status != VarInfo::NOALLOC && allocation.status != VarInfo::OWNED && !Token::simpleMatch(tok->astTop(), "return")) {
        alloctype[arg->varId()].status = VarInfo::DEALLOC;
        alloctype[arg->varId()].allocTok = tok;
    }
}

void CheckLeakAutoVar::functionCall(const Token *tokName, const Token *tokOpeningPar, VarInfo &varInfo, const VarInfo::AllocInfo& allocation, const Library::AllocFunc* af)
{
    // Ignore function call?
    if (mSettings->library.isLeakIgnore(mSettings->library.getFunctionName(tokName)))
        return;
    if (mSettings->library.getReallocFuncInfo(tokName))
        return;

    const Token * const tokFirstArg = tokOpeningPar->next();
    if (!tokFirstArg || tokFirstArg->str() == ")") {
        // no arguments
        return;
    }

    int argNr = 1;
    for (const Token *funcArg = tokFirstArg; funcArg; funcArg = funcArg->nextArgument()) {
        const Token* arg = funcArg;
        if (mTokenizer->isCPP()) {
            int tokAdvance = 0;
            if (arg->str() == "new")
                tokAdvance = 1;
            else if (Token::simpleMatch(arg, "* new"))
                tokAdvance = 2;
            if (tokAdvance > 0) {
                arg = arg->tokAt(tokAdvance);
                if (Token::simpleMatch(arg, "( std :: nothrow )"))
                    arg = arg->tokAt(5);
            }
        }

        // Skip casts
        if (arg->isKeyword() && arg->astParent() && arg->astParent()->isCast())
            arg = arg->astParent();
        while (arg && arg->isCast())
            arg = arg->astOperand2() ? arg->astOperand2() : arg->astOperand1();
        const Token * const argTypeStartTok = arg;

        while (Token::Match(arg, "%name% .|:: %name%"))
            arg = arg->tokAt(2);

        if (Token::Match(arg, "%var% [-,)] !!.") || Token::Match(arg, "& %var%")) {
            // goto variable
            if (arg->str() == "&")
                arg = arg->next();

            const bool isnull = arg->hasKnownIntValue() && arg->values().front().intvalue == 0;

            // Is variable allocated?
            if (!isnull && (!af || af->arg == argNr)) {
                const Library::AllocFunc* deallocFunc = mSettings->library.getDeallocFuncInfo(tokName);
                VarInfo::AllocInfo dealloc(deallocFunc ? deallocFunc->groupId : 0, VarInfo::DEALLOC, tokName);
                if (dealloc.type == 0)
                    changeAllocStatus(varInfo, allocation, tokName, arg);
                else
                    changeAllocStatus(varInfo, dealloc, tokName, arg);
            }
        }
        // Check smart pointer
        else if (Token::Match(arg, "%name% < %type%") && mSettings->library.isSmartPointer(argTypeStartTok)) {
            const Token * typeEndTok = arg->linkAt(1);
            const Token * allocTok = nullptr;
            if (!Token::Match(typeEndTok, "> {|( %var% ,|)|}"))
                continue;

            bool arrayDelete = false;
            if (Token::findsimplematch(arg->next(), "[ ]", typeEndTok))
                arrayDelete = true;

            // Check deleter
            const Token * deleterToken = nullptr;
            const Token * endDeleterToken = nullptr;
            const Library::AllocFunc* sp_af = nullptr;
            if (Token::Match(arg, "unique_ptr < %type% ,")) {
                deleterToken = arg->tokAt(4);
                endDeleterToken = typeEndTok;
            } else if (Token::Match(typeEndTok, "> {|( %var% ,")) {
                deleterToken = typeEndTok->tokAt(4);
                endDeleterToken = typeEndTok->linkAt(1);
            }
            if (deleterToken) {
                // Check if its a pointer to a function
                const Token * dtok = Token::findmatch(deleterToken, "& %name%", endDeleterToken);
                if (dtok) {
                    sp_af = mSettings->library.getDeallocFuncInfo(dtok->tokAt(1));
                } else {
                    // If the deleter is a class, check if class calls the dealloc function
                    dtok = Token::findmatch(deleterToken, "%type%", endDeleterToken);
                    if (dtok && dtok->type()) {
                        const Scope * tscope = dtok->type()->classScope;
                        for (const Token *tok2 = tscope->bodyStart; tok2 != tscope->bodyEnd; tok2 = tok2->next()) {
                            sp_af = mSettings->library.getDeallocFuncInfo(tok2);
                            if (sp_af) {
                                allocTok = tok2;
                                break;
                            }
                        }
                    }
                }
            }

            const Token * vtok = typeEndTok->tokAt(2);
            const VarInfo::AllocInfo sp_allocation(sp_af ? sp_af->groupId : (arrayDelete ? NEW_ARRAY : NEW), VarInfo::OWNED, allocTok);
            changeAllocStatus(varInfo, sp_allocation, vtok, vtok);
        } else {
            checkTokenInsideExpression(arg, varInfo);
        }
        // TODO: check each token in argument expression (could contain multiple variables)
        argNr++;
    }
}


void CheckLeakAutoVar::leakIfAllocated(const Token *vartok,
                                       const VarInfo &varInfo)
{
    const std::map<int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const auto& possibleUsage = varInfo.possibleUsage;

    const std::map<int, VarInfo::AllocInfo>::const_iterator var = alloctype.find(vartok->varId());
    if (var != alloctype.cend() && var->second.status == VarInfo::ALLOC) {
        const auto use = possibleUsage.find(vartok->varId());
        if (use == possibleUsage.end()) {
            leakError(vartok, vartok->str(), var->second.type);
        } else {
            configurationInfo(vartok, use->second);
        }
    }
}

void CheckLeakAutoVar::ret(const Token *tok, VarInfo &varInfo, const bool isEndOfScope)
{
    const std::map<int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const auto& possibleUsage = varInfo.possibleUsage;
    std::vector<int> toRemove;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (std::map<int, VarInfo::AllocInfo>::const_iterator it = alloctype.cbegin(); it != alloctype.cend(); ++it) {
        // don't warn if variable is conditionally allocated, unless it leaves the scope
        if (!isEndOfScope && !it->second.managed() && varInfo.conditionalAlloc.find(it->first) != varInfo.conditionalAlloc.end())
            continue;

        // don't warn if there is a reference of the variable
        if (varInfo.referenced.find(it->first) != varInfo.referenced.end())
            continue;

        const int varid = it->first;
        const Variable *var = symbolDatabase->getVariableFromVarId(varid);
        if (var) {
            // don't warn if we leave an inner scope
            if (isEndOfScope && var->scope() && tok != var->scope()->bodyEnd)
                continue;
            enum class PtrUsage { NONE, DEREF, PTR } used = PtrUsage::NONE;
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";")
                    break;
                if (!Token::Match(tok2, "return|(|{|,"))
                    continue;

                const Token* tok3 = tok2->next();
                while (tok3 && tok3->isCast() && tok3->valueType() &&
                       (tok3->valueType()->pointer ||
                        (tok3->valueType()->typeSize(mSettings->platform) == 0) ||
                        (tok3->valueType()->typeSize(mSettings->platform) >= mSettings->platform.sizeof_pointer)))
                    tok3 = tok3->astOperand2() ? tok3->astOperand2() : tok3->astOperand1();
                if (tok3 && tok3->varId() == varid)
                    tok2 = tok3->next();
                else if (Token::Match(tok3, "& %varid% . %name%", varid))
                    tok2 = tok3->tokAt(4);
                else if (Token::simpleMatch(tok3, "*") && tok3->next()->varId() == varid)
                    tok2 = tok3;
                else
                    continue;
                if (Token::Match(tok2, "[});,+]")) {
                    used = PtrUsage::PTR;
                    break;
                }
                if (Token::Match(tok2, "[|.|*")) {
                    used = PtrUsage::DEREF;
                    break;
                }
            }

            // return deallocated pointer
            if (used != PtrUsage::NONE && it->second.status == VarInfo::DEALLOC)
                deallocReturnError(tok, it->second.allocTok, var->name());

            else if (used != PtrUsage::PTR && !it->second.managed() && !var->isReference()) {
                const auto use = possibleUsage.find(varid);
                if (use == possibleUsage.end()) {
                    leakError(tok, var->name(), it->second.type);
                } else {
                    configurationInfo(tok, use->second);
                }
            }
            toRemove.push_back(varid);
        }
    }
    for (const int varId : toRemove)
        varInfo.erase(varId);
}
