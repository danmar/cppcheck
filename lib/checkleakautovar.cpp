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
// Leaks when using auto variables
//---------------------------------------------------------------------------

#include "checkleakautovar.h"

#include "astutils.h"
#include "checkmemoryleak.h"  // <- CheckMemoryLeak::memoryLeak
#include "checknullpointer.h" // <- CheckNullPointer::isPointerDeRef
#include "errorlogger.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <stack>
#include <utility>

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

//---------------------------------------------------------------------------

void VarInfo::print()
{
    std::cout << "size=" << alloctype.size() << std::endl;
    for (std::map<unsigned int, AllocInfo>::const_iterator it = alloctype.begin(); it != alloctype.end(); ++it) {
        std::string strusage;
        const std::map<unsigned int, std::string>::const_iterator use =
            possibleUsage.find(it->first);
        if (use != possibleUsage.end())
            strusage = use->second;

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
        default:
            status = "?";
            break;
        };

        std::cout << "status=" << status << " "
                  << "alloctype='" << it->second.type << "' "
                  << "possibleUsage='" << strusage << "' "
                  << "conditionalAlloc=" << (conditionalAlloc.find(it->first) != conditionalAlloc.end() ? "yes" : "no") << " "
                  << "referenced=" << (referenced.find(it->first) != referenced.end() ? "yes" : "no") << " "
                  << std::endl;
    }
}

void VarInfo::possibleUsageAll(const std::string &functionName)
{
    possibleUsage.clear();
    for (std::map<unsigned int, AllocInfo>::const_iterator it = alloctype.begin(); it != alloctype.end(); ++it)
        possibleUsage[it->first] = functionName;
}


void CheckLeakAutoVar::leakError(const Token *tok, const std::string &varname, int type)
{
    const CheckMemoryLeak checkmemleak(mTokenizer, mErrorLogger, mSettings);
    if (mSettings->library.isresource(type))
        checkmemleak.resourceLeakError(tok, varname);
    else
        checkmemleak.memleakError(tok, varname);
}

void CheckLeakAutoVar::mismatchError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(mTokenizer, mErrorLogger, mSettings);
    const std::list<const Token *> callstack(1, tok);
    c.mismatchAllocDealloc(callstack, varname);
}

void CheckLeakAutoVar::deallocUseError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(mTokenizer, mErrorLogger, mSettings);
    c.deallocuseError(tok, varname);
}

void CheckLeakAutoVar::deallocReturnError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "deallocret", "$symbol:" + varname + "\nReturning/dereferencing '$symbol' after it is deallocated / released", CWE672, false);
}

void CheckLeakAutoVar::configurationInfo(const Token* tok, const std::string &functionName)
{
    if (mSettings->checkLibrary && mSettings->isEnabled(Settings::INFORMATION)) {
        reportError(tok,
                    Severity::information,
                    "checkLibraryUseIgnore",
                    "--check-library: Function " + functionName + "() should have <use>/<leak-ignore> configuration");
    }
}

void CheckLeakAutoVar::doubleFreeError(const Token *tok, const std::string &varname, int type)
{
    if (mSettings->library.isresource(type))
        reportError(tok, Severity::error, "doubleFree", "$symbol:" + varname + "\nResource handle '$symbol' freed twice.", CWE415, false);
    else
        reportError(tok, Severity::error, "doubleFree", "$symbol:" + varname + "\nMemory pointed to by '$symbol' is freed twice.", CWE415, false);
}


void CheckLeakAutoVar::check()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Local variables that are known to be non-zero.
    const std::set<unsigned int> notzero;

    // Check function scopes
    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->hasInlineOrLambdaFunction())
            continue;

        // Empty variable info
        VarInfo varInfo;

        checkScope(scope->bodyStart, &varInfo, notzero);

        varInfo.conditionalAlloc.clear();

        // Clear reference arguments from varInfo..
        std::map<unsigned int, VarInfo::AllocInfo>::iterator it = varInfo.alloctype.begin();
        while (it != varInfo.alloctype.end()) {
            const Variable *var = symbolDatabase->getVariableFromVarId(it->first);
            if (!var ||
                (var->isArgument() && var->isReference()) ||
                (!var->isArgument() && !var->isLocal()))
                varInfo.alloctype.erase(it++);
            else
                ++it;
        }

        ret(scope->bodyEnd, varInfo);
    }
}

static bool isVarUsedInTree(const Token *tok, unsigned int varid)
{
    if (!tok)
        return false;
    if (tok->varId() == varid)
        return true;
    return isVarUsedInTree(tok->astOperand1(), varid) || isVarUsedInTree(tok->astOperand2(), varid);
}

static bool isPointerReleased(const Token *startToken, const Token *endToken, unsigned int varid)
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

/** checks if nameToken is a name of a function in a function call:
*     func(arg)
* or
*     func<temp1_arg>(arg)
* @param nameToken Function name token
* @return opening parenthesis token or NULL if not a function call
*/

static const Token * isFunctionCall(const Token * nameToken)
{
    if (nameToken->isName()) {
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

void CheckLeakAutoVar::checkScope(const Token * const startToken,
                                  VarInfo *varInfo,
                                  std::set<unsigned int> notzero)
{
    std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo->alloctype;
    std::map<unsigned int, std::string> &possibleUsage = varInfo->possibleUsage;
    const std::set<unsigned int> conditionalAlloc(varInfo->conditionalAlloc);

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
        if (!Token::Match(tok, "[;{}]") || Token::Match(tok->next(), "[;{}]"))
            continue;

        tok = tok->next();
        if (!tok || tok == endToken)
            break;

        // parse statement, skip to last member
        const Token *varTok = tok;
        while (Token::Match(varTok, "%name% ::|. %name% !!("))
            varTok = varTok->tokAt(2);

        const Token *ftok = tok;
        if (ftok->str() == "::")
            ftok = ftok->next();
        while (Token::Match(ftok, "%name% :: %name%"))
            ftok = ftok->tokAt(2);

        // assignment..
        if (Token::Match(varTok, "%var% =")) {
            const Token* const tokAssignOp = varTok->next();

            // taking address of another variable..
            if (Token::Match(tokAssignOp, "= %var% [+;]")) {
                if (varTok->tokAt(2)->varId() != varTok->varId()) {
                    // If variable points at allocated memory => error
                    leakIfAllocated(varTok, *varInfo);

                    // no multivariable checking currently => bail out for rhs variables
                    for (const Token *tok2 = varTok; tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ";") {
                            break;
                        }
                        if (tok2->varId()) {
                            varInfo->erase(tok2->varId());
                        }
                    }
                }
            }

            // right ast part (after `=` operator)
            const Token* const tokRightAstOperand = tokAssignOp->astOperand2();

            // is variable used in rhs?
            if (isVarUsedInTree(tokRightAstOperand, varTok->varId()))
                continue;

            // Variable has already been allocated => error
            if (conditionalAlloc.find(varTok->varId()) == conditionalAlloc.end())
                leakIfAllocated(varTok, *varInfo);
            varInfo->erase(varTok->varId());

            // not a local variable nor argument?
            const Variable *var = varTok->variable();
            if (var && !var->isArgument() && (!var->isLocal() || var->isStatic()))
                continue;

            // Don't check reference variables
            if (var && var->isReference())
                continue;

            // non-pod variable
            if (mTokenizer->isCPP()) {
                if (!var)
                    continue;
                // Possibly automatically deallocated memory
                if (!var->typeStartToken()->isStandardType() && Token::Match(varTok, "%var% = new"))
                    continue;
                if (!var->isPointer() && !var->typeStartToken()->isStandardType())
                    continue;
            }

            // allocation?
            if (tokRightAstOperand && Token::Match(tokRightAstOperand->previous(), "%type% (")) {
                const Library::AllocFunc* f = mSettings->library.alloc(tokRightAstOperand->previous());
                if (f && f->arg == -1) {
                    VarInfo::AllocInfo& varAlloc = alloctype[varTok->varId()];
                    varAlloc.type = f->groupId;
                    varAlloc.status = VarInfo::ALLOC;
                }
            } else if (mTokenizer->isCPP() && Token::Match(varTok->tokAt(2), "new !!(")) {
                const Token* tok2 = varTok->tokAt(2)->astOperand1();
                const bool arrayNew = (tok2 && (tok2->str() == "[" || (tok2->str() == "(" && tok2->astOperand1() && tok2->astOperand1()->str() == "[")));
                VarInfo::AllocInfo& varAlloc = alloctype[varTok->varId()];
                varAlloc.type = arrayNew ? NEW_ARRAY : NEW;
                varAlloc.status = VarInfo::ALLOC;
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

                if (Token::Match(innerTok, "%var% =")) {
                    // allocation?
                    if (Token::Match(innerTok->tokAt(2), "%type% (")) {
                        const Library::AllocFunc* f = mSettings->library.alloc(innerTok->tokAt(2));
                        if (f && f->arg == -1) {
                            VarInfo::AllocInfo& varAlloc = alloctype[innerTok->varId()];
                            varAlloc.type = f->groupId;
                            varAlloc.status = VarInfo::ALLOC;
                        }
                    } else if (mTokenizer->isCPP() && Token::Match(innerTok->tokAt(2), "new !!(")) {
                        const Token* tok2 = innerTok->tokAt(2)->astOperand1();
                        const bool arrayNew = (tok2 && (tok2->str() == "[" || (tok2->str() == "(" && tok2->astOperand1() && tok2->astOperand1()->str() == "[")));
                        VarInfo::AllocInfo& varAlloc = alloctype[innerTok->varId()];
                        varAlloc.type = arrayNew ? NEW_ARRAY : NEW;
                        varAlloc.status = VarInfo::ALLOC;
                    }
                }

                // check for function call
                const Token * const openingPar = isFunctionCall(innerTok);
                if (openingPar) {
                    // innerTok is a function name
                    const VarInfo::AllocInfo allocation(0, VarInfo::NOALLOC);
                    functionCall(innerTok, openingPar, varInfo, allocation, nullptr);
                    innerTok = openingPar->link();
                }
            }

            if (Token::simpleMatch(closingParenthesis, ") {")) {
                VarInfo varInfo1(*varInfo);  // VarInfo for if code
                VarInfo varInfo2(*varInfo);  // VarInfo for else code

                // Recursively scan variable comparisons in condition
                std::stack<const Token *> tokens;
                tokens.push(tok->next()->astOperand2());
                while (!tokens.empty()) {
                    const Token *tok3 = tokens.top();
                    tokens.pop();
                    if (!tok3)
                        continue;
                    if (tok3->str() == "&&") {
                        tokens.push(tok3->astOperand1());
                        tokens.push(tok3->astOperand2());
                        continue;
                    }
                    if (tok3->str() == "(" && Token::Match(tok3->astOperand1(), "UNLIKELY|LIKELY")) {
                        tokens.push(tok3->astOperand2());
                        continue;
                    } else if (tok3->str() == "(" && Token::Match(tok3->previous(), "%name%")) {
                        const std::vector<const Token *> params = getArguments(tok3->previous());
                        for (const Token *par : params) {
                            if (!par->isComparisonOp())
                                continue;
                            const Token *vartok = nullptr;
                            if (astIsVariableComparison(par, "!=", "0", &vartok) ||
                                astIsVariableComparison(par, "==", "0", &vartok) ||
                                astIsVariableComparison(par, "<", "0", &vartok) ||
                                astIsVariableComparison(par, ">", "0", &vartok) ||
                                astIsVariableComparison(par, "==", "-1", &vartok) ||
                                astIsVariableComparison(par, "!=", "-1", &vartok)) {
                                varInfo1.erase(vartok->varId());
                                varInfo2.erase(vartok->varId());
                            }
                        }
                        continue;
                    }

                    const Token *vartok = nullptr;
                    if (astIsVariableComparison(tok3, "!=", "0", &vartok)) {
                        varInfo2.erase(vartok->varId());
                        if (notzero.find(vartok->varId()) != notzero.end())
                            varInfo2.clear();
                    } else if (astIsVariableComparison(tok3, "==", "0", &vartok)) {
                        varInfo1.erase(vartok->varId());
                    } else if (astIsVariableComparison(tok3, "<", "0", &vartok)) {
                        varInfo1.erase(vartok->varId());
                    } else if (astIsVariableComparison(tok3, ">", "0", &vartok)) {
                        varInfo2.erase(vartok->varId());
                    } else if (astIsVariableComparison(tok3, "==", "-1", &vartok)) {
                        varInfo1.erase(vartok->varId());
                    }
                }

                checkScope(closingParenthesis->next(), &varInfo1, notzero);
                closingParenthesis = closingParenthesis->linkAt(1);
                if (Token::simpleMatch(closingParenthesis, "} else {")) {
                    checkScope(closingParenthesis->tokAt(2), &varInfo2, notzero);
                    tok = closingParenthesis->linkAt(2)->previous();
                } else {
                    tok = closingParenthesis->previous();
                }

                VarInfo old;
                old.swap(*varInfo);

                std::map<unsigned int, VarInfo::AllocInfo>::const_iterator it;

                for (it = old.alloctype.begin(); it != old.alloctype.end(); ++it) {
                    const unsigned int varId = it->first;
                    if (old.conditionalAlloc.find(varId) == old.conditionalAlloc.end())
                        continue;
                    if (varInfo1.alloctype.find(varId) == varInfo1.alloctype.end() ||
                        varInfo2.alloctype.find(varId) == varInfo2.alloctype.end()) {
                        varInfo1.erase(varId);
                        varInfo2.erase(varId);
                    }
                }

                // Conditional allocation in varInfo1
                for (it = varInfo1.alloctype.begin(); it != varInfo1.alloctype.end(); ++it) {
                    if (varInfo2.alloctype.find(it->first) == varInfo2.alloctype.end() &&
                        old.alloctype.find(it->first) == old.alloctype.end()) {
                        varInfo->conditionalAlloc.insert(it->first);
                    }
                }

                // Conditional allocation in varInfo2
                for (it = varInfo2.alloctype.begin(); it != varInfo2.alloctype.end(); ++it) {
                    if (varInfo1.alloctype.find(it->first) == varInfo1.alloctype.end() &&
                        old.alloctype.find(it->first) == old.alloctype.end()) {
                        varInfo->conditionalAlloc.insert(it->first);
                    }
                }

                // Conditional allocation/deallocation
                for (it = varInfo1.alloctype.begin(); it != varInfo1.alloctype.end(); ++it) {
                    if (it->second.managed() && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo->conditionalAlloc.erase(it->first);
                        varInfo2.erase(it->first);
                    }
                }
                for (it = varInfo2.alloctype.begin(); it != varInfo2.alloctype.end(); ++it) {
                    if (it->second.managed() && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo->conditionalAlloc.erase(it->first);
                        varInfo1.erase(it->first);
                    }
                }

                alloctype.insert(varInfo1.alloctype.begin(), varInfo1.alloctype.end());
                alloctype.insert(varInfo2.alloctype.begin(), varInfo2.alloctype.end());

                possibleUsage.insert(varInfo1.possibleUsage.begin(), varInfo1.possibleUsage.end());
                possibleUsage.insert(varInfo2.possibleUsage.begin(), varInfo2.possibleUsage.end());
            }
        }

        // unknown control.. (TODO: handle loops)
        else if ((Token::Match(tok, "%type% (") && Token::simpleMatch(tok->linkAt(1), ") {")) || Token::simpleMatch(tok, "do {")) {
            varInfo->clear();
            break;
        }

        // return
        else if (tok->str() == "return") {
            ret(tok, *varInfo);
            varInfo->clear();
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
                ret(tok, *varInfo);
            varInfo->clear();
        }

        // Function call..
        else if (isFunctionCall(ftok)) {
            const Token * openingPar = isFunctionCall(ftok);
            const Library::AllocFunc* af = mSettings->library.dealloc(ftok);
            VarInfo::AllocInfo allocation(af ? af->groupId : 0, VarInfo::DEALLOC);
            if (allocation.type == 0)
                allocation.status = VarInfo::NOALLOC;
            functionCall(ftok, openingPar, varInfo, allocation, af);

            tok = ftok->next()->link();

            // Handle scopes that might be noreturn
            if (allocation.status == VarInfo::NOALLOC && Token::simpleMatch(tok, ") ; }")) {
                const std::string &functionName(tok->link()->previous()->str());
                bool unknown = false;
                if (mTokenizer->IsScopeNoReturn(tok->tokAt(2), &unknown)) {
                    if (!unknown)
                        varInfo->clear();
                    else if (!mSettings->library.isLeakIgnore(functionName) && !mSettings->library.isUse(functionName))
                        varInfo->possibleUsageAll(functionName);
                }
            }

            continue;
        }

        // delete
        else if (mTokenizer->isCPP() && tok->str() == "delete") {
            const bool arrayDelete = (tok->strAt(1) == "[");
            if (arrayDelete)
                tok = tok->tokAt(3);
            else
                tok = tok->next();
            while (Token::Match(tok, "%name% ::|."))
                tok = tok->tokAt(2);
            const bool isnull = tok->hasKnownIntValue() && tok->values().front().intvalue == 0;
            if (!isnull && tok->varId() && tok->strAt(1) != "[") {
                const VarInfo::AllocInfo allocation(arrayDelete ? NEW_ARRAY : NEW, VarInfo::DEALLOC);
                changeAllocStatus(varInfo, allocation, tok, tok);
            }
        }

        // goto => weird execution path
        else if (tok->str() == "goto") {
            varInfo->clear();
        }

        // continue/break
        else if (Token::Match(tok, "continue|break ;")) {
            varInfo->clear();
        }

        // Check smart pointer
        else if (Token::Match(ftok, "auto_ptr|unique_ptr|shared_ptr < %type%")) {


            const Token * typeEndTok = ftok->linkAt(1);
            if (!Token::Match(typeEndTok, "> %var% {|( %var% ,|)|}"))
                continue;

            tok = typeEndTok->linkAt(2);

            const unsigned varid = typeEndTok->next()->varId();
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
                    af = mSettings->library.dealloc(dtok->tokAt(1));
                } else {
                    const Token * tscopeStart = nullptr;
                    const Token * tscopeEnd = nullptr;
                    // If the deleter is a lambda, check if it calls the dealloc function
                    if (deleterToken->str() == "[" &&
                        Token::simpleMatch(deleterToken->link(), "] (") &&
                        // TODO: Check for mutable keyword
                        Token::simpleMatch(deleterToken->link()->linkAt(1), ") {")) {
                        tscopeStart = deleterToken->link()->linkAt(1)->tokAt(1);
                        tscopeEnd = tscopeStart->link();
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
                            af = mSettings->library.dealloc(tok2);
                            if (af)
                                break;
                        }
                    }
                }
            }

            const Token * vtok = typeEndTok->tokAt(3);
            const VarInfo::AllocInfo allocation(af ? af->groupId : (arrayDelete ? NEW_ARRAY : NEW), VarInfo::OWNED);
            changeAllocStatus(varInfo, allocation, vtok, vtok);
        }
    }
}


const Token * CheckLeakAutoVar::checkTokenInsideExpression(const Token * const tok, VarInfo *varInfo)
{
    // Deallocation and then dereferencing pointer..
    if (tok->varId() > 0) {
        // TODO : Write a separate checker for this that uses valueFlowForward.
        const std::map<unsigned int, VarInfo::AllocInfo>::const_iterator var = varInfo->alloctype.find(tok->varId());
        if (var != varInfo->alloctype.end()) {
            bool unknown = false;
            if (var->second.status == VarInfo::DEALLOC && CheckNullPointer::isPointerDeRef(tok, unknown) && !unknown) {
                deallocUseError(tok, tok->str());
            } else if (Token::simpleMatch(tok->tokAt(-2), "= &")) {
                varInfo->erase(tok->varId());
            } else if (tok->strAt(-1) == "=") {
                varInfo->erase(tok->varId());
            }
        } else if (Token::Match(tok->previous(), "& %name% = %var% ;")) {
            varInfo->referenced.insert(tok->tokAt(2)->varId());
        }
    }

    // check for function call
    const Token * const openingPar = isFunctionCall(tok);
    if (openingPar) {
        const Library::AllocFunc* allocFunc = mSettings->library.dealloc(tok);
        VarInfo::AllocInfo alloc(allocFunc ? allocFunc->groupId : 0, VarInfo::DEALLOC);
        if (alloc.type == 0)
            alloc.status = VarInfo::NOALLOC;
        functionCall(tok, openingPar, varInfo, alloc, nullptr);
        return openingPar->link();
    }

    return nullptr;
}


void CheckLeakAutoVar::changeAllocStatus(VarInfo *varInfo, const VarInfo::AllocInfo& allocation, const Token* tok, const Token* arg)
{
    std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo->alloctype;
    const std::map<unsigned int, VarInfo::AllocInfo>::iterator var = alloctype.find(arg->varId());
    if (var != alloctype.end()) {
        if (allocation.status == VarInfo::NOALLOC) {
            // possible usage
            varInfo->possibleUsage[arg->varId()] = tok->str();
            if (var->second.status == VarInfo::DEALLOC && arg->previous()->str() == "&")
                varInfo->erase(arg->varId());
        } else if (var->second.managed()) {
            doubleFreeError(tok, arg->str(), allocation.type);
        } else if (var->second.type != allocation.type) {
            // mismatching allocation and deallocation
            mismatchError(tok, arg->str());
            varInfo->erase(arg->varId());
        } else {
            // deallocation
            var->second.status = allocation.status;
            var->second.type = allocation.type;
        }
    } else if (allocation.status != VarInfo::NOALLOC) {
        alloctype[arg->varId()].status = VarInfo::DEALLOC;
    }
}

void CheckLeakAutoVar::functionCall(const Token *tokName, const Token *tokOpeningPar, VarInfo *varInfo, const VarInfo::AllocInfo& allocation, const Library::AllocFunc* af)
{
    // Ignore function call?
    if (mSettings->library.isLeakIgnore(tokName->str()))
        return;

    const Token * const tokFirstArg = tokOpeningPar->next();
    if (!tokFirstArg || tokFirstArg->str() == ")") {
        // no arguments
        return;
    }

    int argNr = 1;
    for (const Token *arg = tokFirstArg; arg; arg = arg->nextArgument()) {
        if (mTokenizer->isCPP() && arg->str() == "new") {
            arg = arg->next();
            if (Token::simpleMatch(arg, "( std :: nothrow )"))
                arg = arg->tokAt(5);
        }

        while (Token::Match(arg, "%name% .|:: %name%"))
            arg = arg->tokAt(2);

        if (Token::Match(arg, "%var% [-,)] !!.") || Token::Match(arg, "& %var%")) {
            // goto variable
            if (arg->str() == "&")
                arg = arg->next();

            const bool isnull = arg->hasKnownIntValue() && arg->values().front().intvalue == 0;

            // Is variable allocated?
            if (!isnull && (!af || af->arg == argNr))
                changeAllocStatus(varInfo, allocation, tokName, arg);
        }
        // Check smart pointer
        else if (Token::Match(arg, "auto_ptr|unique_ptr|shared_ptr < %type%")) {
            const Token * typeEndTok = arg->linkAt(1);
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
                    sp_af = mSettings->library.dealloc(dtok->tokAt(1));
                } else {
                    // If the deleter is a class, check if class calls the dealloc function
                    dtok = Token::findmatch(deleterToken, "%type%", endDeleterToken);
                    if (dtok && dtok->type()) {
                        const Scope * tscope = dtok->type()->classScope;
                        for (const Token *tok2 = tscope->bodyStart; tok2 != tscope->bodyEnd; tok2 = tok2->next()) {
                            sp_af = mSettings->library.dealloc(tok2);
                            if (sp_af)
                                break;
                        }
                    }
                }
            }

            const Token * vtok = typeEndTok->tokAt(2);
            const VarInfo::AllocInfo sp_allocation(sp_af ? sp_af->groupId : (arrayDelete ? NEW_ARRAY : NEW), VarInfo::OWNED);
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
    const std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const std::map<unsigned int, std::string> &possibleUsage = varInfo.possibleUsage;

    const std::map<unsigned int, VarInfo::AllocInfo>::const_iterator var = alloctype.find(vartok->varId());
    if (var != alloctype.end() && var->second.status == VarInfo::ALLOC) {
        const std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(vartok->varId());
        if (use == possibleUsage.end()) {
            leakError(vartok, vartok->str(), var->second.type);
        } else {
            configurationInfo(vartok, use->second);
        }
    }
}

void CheckLeakAutoVar::ret(const Token *tok, const VarInfo &varInfo)
{
    const std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const std::map<unsigned int, std::string> &possibleUsage = varInfo.possibleUsage;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (std::map<unsigned int, VarInfo::AllocInfo>::const_iterator it = alloctype.begin(); it != alloctype.end(); ++it) {
        // don't warn if variable is conditionally allocated
        if (!it->second.managed() && varInfo.conditionalAlloc.find(it->first) != varInfo.conditionalAlloc.end())
            continue;

        // don't warn if there is a reference of the variable
        if (varInfo.referenced.find(it->first) != varInfo.referenced.end())
            continue;

        const unsigned int varid = it->first;
        const Variable *var = symbolDatabase->getVariableFromVarId(varid);
        if (var) {
            bool used = false;
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";")
                    break;
                if (Token::Match(tok2, "return|(|, %varid% [);,]", varid)) {
                    used = true;
                    break;
                }
                if (Token::Match(tok2, "return|(|, & %varid% . %name% [);,]", varid)) {
                    used = true;
                    break;
                }
            }

            // return deallocated pointer
            if (used && it->second.status == VarInfo::DEALLOC)
                deallocReturnError(tok, var->name());

            else if (!used && !it->second.managed()) {
                const std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(varid);
                if (use == possibleUsage.end()) {
                    leakError(tok, var->name(), it->second.type);
                } else {
                    configurationInfo(tok, use->second);
                }
            }
        }
    }
}
