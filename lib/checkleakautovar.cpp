/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "checkmemoryleak.h"  // <- CheckMemoryLeak::memoryLeak
#include "tokenize.h"
#include "symboldatabase.h"
#include "astutils.h"

#include <iostream>
#include <stack>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckLeakAutoVar instance;
}

static const CWE CWE672(672U);
static const CWE CWE415(415U);
//---------------------------------------------------------------------------

void VarInfo::print()
{
    std::cout << "size=" << alloctype.size() << std::endl;
    std::map<unsigned int, AllocInfo>::const_iterator it;
    for (it = alloctype.begin(); it != alloctype.end(); ++it) {
        std::string strusage;
        std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(it->first);
        if (use != possibleUsage.end())
            strusage = use->second;

        std::string status;
        switch (it->second.status) {
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
    std::map<unsigned int, AllocInfo>::const_iterator it;
    for (it = alloctype.begin(); it != alloctype.end(); ++it)
        possibleUsage[it->first] = functionName;
}


void CheckLeakAutoVar::leakError(const Token *tok, const std::string &varname, int type)
{
    const CheckMemoryLeak checkmemleak(_tokenizer, _errorLogger, _settings);
    if (_settings->library.isresource(type))
        checkmemleak.resourceLeakError(tok, varname);
    else
        checkmemleak.memleakError(tok, varname);
}

void CheckLeakAutoVar::mismatchError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(_tokenizer, _errorLogger, _settings);
    std::list<const Token *> callstack(1, tok);
    c.mismatchAllocDealloc(callstack, varname);
}

void CheckLeakAutoVar::deallocUseError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(_tokenizer, _errorLogger, _settings);
    c.deallocuseError(tok, varname);
}

void CheckLeakAutoVar::deallocReturnError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "deallocret", "Returning/dereferencing '" + varname + "' after it is deallocated / released", CWE672, false);
}

void CheckLeakAutoVar::configurationInfo(const Token* tok, const std::string &functionName)
{
    if (_settings->checkLibrary && _settings->isEnabled("information")) {
        reportError(tok,
                    Severity::information,
                    "checkLibraryUseIgnore",
                    "--check-library: Function " + functionName + "() should have <use>/<leak-ignore> configuration");
    }
}

void CheckLeakAutoVar::doubleFreeError(const Token *tok, const std::string &varname, int type)
{
    if (_settings->library.isresource(type))
        reportError(tok, Severity::error, "doubleFree", "Resource handle '" + varname + "' freed twice.", CWE415, false);
    else
        reportError(tok, Severity::error, "doubleFree", "Memory pointed to by '" + varname + "' is freed twice.", CWE415, false);
}


void CheckLeakAutoVar::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Local variables that are known to be non-zero.
    const std::set<unsigned int> notzero;

    // Check function scopes
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        // Empty variable info
        VarInfo varInfo;

        checkScope(scope->classStart, &varInfo, notzero);

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

        ret(scope->classEnd, varInfo);
    }
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
            tok = tok->scope()->classEnd;
            if (!tok) // Ticket #6666 (crash upon invalid code)
                break;
        }

        // Deallocation and then dereferencing pointer..
        if (tok->varId() > 0) {
            const std::map<unsigned int, VarInfo::AllocInfo>::const_iterator var = alloctype.find(tok->varId());
            if (var != alloctype.end()) {
                if (var->second.status == VarInfo::DEALLOC && tok->strAt(-1) != "&" && (!Token::Match(tok, "%name% =") || tok->strAt(-1) == "*")) {
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

        if (tok->str() == "(" && tok->previous()->isName()) {
            VarInfo::AllocInfo allocation(0, VarInfo::NOALLOC);
            functionCall(tok->previous(), varInfo, allocation);
            tok = tok->link();
            continue;
        }

        // look for end of statement
        if (!Token::Match(tok, "[;{}]") || Token::Match(tok->next(), "[;{}]"))
            continue;

        tok = tok->next();
        if (!tok || tok == endToken)
            break;

        // parse statement, skip to last member
        while (Token::Match(tok, "%name% ::|. %name% !!("))
            tok = tok->tokAt(2);

        // assignment..
        if (Token::Match(tok, "%var% =")) {
            // taking address of another variable..
            if (Token::Match(tok->next(), "= %var% [+;]")) {
                if (tok->tokAt(2)->varId() != tok->varId()) {
                    // If variable points at allocated memory => error
                    leakIfAllocated(tok, *varInfo);

                    // no multivariable checking currently => bail out for rhs variables
                    for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ";") {
                            break;
                        }
                        if (tok2->varId()) {
                            varInfo->erase(tok2->varId());
                        }
                    }
                }
            }

            // is variable used in rhs?
            bool used_in_rhs = false;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    break;
                }
                if (tok->varId() == tok2->varId()) {
                    used_in_rhs = true;
                    break;
                }
            }
            // TODO: Better checking how the pointer is used in rhs?
            if (used_in_rhs)
                continue;

            // Variable has already been allocated => error
            if (conditionalAlloc.find(tok->varId()) == conditionalAlloc.end())
                leakIfAllocated(tok, *varInfo);
            varInfo->erase(tok->varId());

            // not a local variable nor argument?
            const Variable *var = tok->variable();
            if (var && !var->isArgument() && (!var->isLocal() || var->isStatic()))
                continue;

            // Don't check reference variables
            if (var && var->isReference())
                continue;

            // non-pod variable
            if (_tokenizer->isCPP()) {
                if (!var)
                    continue;
                // Possibly automatically deallocated memory
                if (!var->typeStartToken()->isStandardType() && Token::Match(tok, "%var% = new"))
                    continue;
            }

            // allocation?
            if (tok->next()->astOperand2() && Token::Match(tok->next()->astOperand2()->previous(), "%type% (")) {
                int i = _settings->library.alloc(tok->next()->astOperand2()->previous());
                if (i > 0) {
                    alloctype[tok->varId()].type = i;
                    alloctype[tok->varId()].status = VarInfo::ALLOC;
                }
            } else if (_tokenizer->isCPP() && tok->strAt(2) == "new") {
                alloctype[tok->varId()].type = -1;
                alloctype[tok->varId()].status = VarInfo::ALLOC;
            }

            // Assigning non-zero value variable. It might be used to
            // track the execution for a later if condition.
            if (Token::Match(tok->tokAt(2), "%num% ;") && MathLib::toLongNumber(tok->strAt(2)) != 0)
                notzero.insert(tok->varId());
            else if (Token::Match(tok->tokAt(2), "- %type% ;") && tok->tokAt(3)->isUpperCaseName())
                notzero.insert(tok->varId());
            else
                notzero.erase(tok->varId());
        }

        // if/else
        else if (Token::simpleMatch(tok, "if (")) {
            // Parse function calls inside the condition
            for (const Token *innerTok = tok->tokAt(2); innerTok; innerTok = innerTok->next()) {
                if (innerTok->str() == ")")
                    break;
                if (innerTok->str() == "(" && innerTok->previous()->isName()) {
                    VarInfo::AllocInfo allocation(_settings->library.dealloc(tok), VarInfo::DEALLOC);
                    if (allocation.type == 0)
                        allocation.status = VarInfo::NOALLOC;
                    functionCall(innerTok->previous(), varInfo, allocation);
                    innerTok = innerTok->link();
                }
            }

            const Token *tok2 = tok->linkAt(1);
            if (Token::simpleMatch(tok2, ") {")) {
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

                checkScope(tok2->next(), &varInfo1, notzero);
                tok2 = tok2->linkAt(1);
                if (Token::simpleMatch(tok2, "} else {")) {
                    checkScope(tok2->tokAt(2), &varInfo2, notzero);
                    tok = tok2->linkAt(2)->previous();
                } else {
                    tok = tok2->previous();
                }

                VarInfo old;
                old.swap(*varInfo);

                // Conditional allocation in varInfo1
                std::map<unsigned int, VarInfo::AllocInfo>::const_iterator it;
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
                    if (it->second.status == VarInfo::DEALLOC && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo->conditionalAlloc.erase(it->first);
                        varInfo2.erase(it->first);
                    }
                }
                for (it = varInfo2.alloctype.begin(); it != varInfo2.alloctype.end(); ++it) {
                    if (it->second.status == VarInfo::DEALLOC && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
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
        else if (_tokenizer->isCPP() && tok->str() == "throw") {
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
        else if (Token::Match(tok, "%type% (")) {
            VarInfo::AllocInfo allocation(_settings->library.dealloc(tok), VarInfo::DEALLOC);
            if (allocation.type == 0)
                allocation.status = VarInfo::NOALLOC;
            functionCall(tok, varInfo, allocation);

            tok = tok->next()->link();

            // Handle scopes that might be noreturn
            if (allocation.status == VarInfo::NOALLOC && Token::simpleMatch(tok, ") ; }")) {
                const std::string &functionName(tok->link()->previous()->str());
                bool unknown = false;
                if (_tokenizer->IsScopeNoReturn(tok->tokAt(2), &unknown)) {
                    if (!unknown)
                        varInfo->clear();
                    else if (_settings->library.leakignore.find(functionName) == _settings->library.leakignore.end() &&
                             _settings->library.use.find(functionName) == _settings->library.use.end())
                        varInfo->possibleUsageAll(functionName);
                }
            }

            continue;
        }

        // delete
        else if (_tokenizer->isCPP() && tok->str() == "delete") {
            if (tok->strAt(1) == "[")
                tok = tok->tokAt(3);
            else
                tok = tok->next();
            while (Token::Match(tok, "%name% ::|."))
                tok = tok->tokAt(2);
            if (tok->varId() && tok->strAt(1) != "[") {
                VarInfo::AllocInfo allocation(-1, VarInfo::DEALLOC);
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
    }
}

void CheckLeakAutoVar::changeAllocStatus(VarInfo *varInfo, const VarInfo::AllocInfo& allocation, const Token* tok, const Token* arg)
{
    std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo->alloctype;
    std::map<unsigned int, std::string> &possibleUsage = varInfo->possibleUsage;
    const std::map<unsigned int, VarInfo::AllocInfo>::iterator var = alloctype.find(arg->varId());
    if (var != alloctype.end()) {
        if (allocation.status == VarInfo::NOALLOC) {
            // possible usage
            possibleUsage[arg->varId()] = tok->str();
            if (var->second.status == VarInfo::DEALLOC && arg->previous()->str() == "&")
                varInfo->erase(arg->varId());
        } else if (var->second.status == VarInfo::DEALLOC) {
            doubleFreeError(tok, arg->str(), allocation.type);
        } else if (var->second.type != allocation.type) {
            // mismatching allocation and deallocation
            mismatchError(tok, arg->str());
            varInfo->erase(arg->varId());
        } else {
            // deallocation
            var->second.status = VarInfo::DEALLOC;
            var->second.type = allocation.type;
        }
    } else if (allocation.status != VarInfo::NOALLOC) {
        alloctype[arg->varId()].status = VarInfo::DEALLOC;
    }
}

void CheckLeakAutoVar::functionCall(const Token *tok, VarInfo *varInfo, const VarInfo::AllocInfo& allocation)
{
    // Ignore function call?
    const bool ignore = bool(_settings->library.leakignore.find(tok->str()) != _settings->library.leakignore.end());
    if (ignore)
        return;

    for (const Token *arg = tok->tokAt(2); arg; arg = arg->nextArgument()) {
        if (_tokenizer->isCPP() && arg->str() == "new")
            arg = arg->next();

        if (Token::Match(arg, "%var% [-,)]") || Token::Match(arg, "& %var%")) {

            // goto variable
            if (arg->str() == "&")
                arg = arg->next();

            // Is variable allocated?
            changeAllocStatus(varInfo, allocation, tok, arg);
        } else if (Token::Match(arg, "%name% (")) {
            functionCall(arg, varInfo, allocation);
        }
    }
}


void CheckLeakAutoVar::leakIfAllocated(const Token *vartok,
                                       const VarInfo &varInfo)
{
    const std::map<unsigned int, VarInfo::AllocInfo> &alloctype = varInfo.alloctype;
    const std::map<unsigned int, std::string> &possibleUsage = varInfo.possibleUsage;

    const std::map<unsigned int, VarInfo::AllocInfo>::const_iterator var = alloctype.find(vartok->varId());
    if (var != alloctype.end() && var->second.status != VarInfo::DEALLOC) {
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

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::map<unsigned int, VarInfo::AllocInfo>::const_iterator it = alloctype.begin(); it != alloctype.end(); ++it) {
        // don't warn if variable is conditionally allocated
        if (it->second.status != VarInfo::DEALLOC && varInfo.conditionalAlloc.find(it->first) != varInfo.conditionalAlloc.end())
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

            else if (!used && it->second.status != VarInfo::DEALLOC) {
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
