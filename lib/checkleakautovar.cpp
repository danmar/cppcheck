/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "checkother.h"   // <- doubleFreeError

#include "tokenize.h"
#include "errorlogger.h"
#include "symboldatabase.h"

#include <fstream>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckLeakAutoVar instance;
}

//---------------------------------------------------------------------------

void VarInfo::print()
{
    std::cout << "size=" << alloctype.size() << std::endl;
    std::map<unsigned int, std::string>::const_iterator it;
    for (it = alloctype.begin(); it != alloctype.end(); ++it) {
        std::string strusage;
        std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(it->first);
        if (use != possibleUsage.end())
            strusage = use->second;

        std::cout << "alloctype='" << it->second << "' "
                  << "possibleUsage='" << strusage << "'" << std::endl;
    }
}

void VarInfo::possibleUsageAll(const std::string &functionName)
{
    possibleUsage.clear();
    std::map<unsigned int, std::string>::const_iterator it;
    for (it = alloctype.begin(); it != alloctype.end(); ++it)
        possibleUsage[it->first] = functionName;
}


void CheckLeakAutoVar::leakError(const Token *tok, const std::string &varname, const std::string &type)
{
    const CheckMemoryLeak checkmemleak(_tokenizer, _errorLogger, _settings->standards);
    if (type == "fopen")
        checkmemleak.resourceLeakError(tok, varname);
    else
        checkmemleak.memleakError(tok, varname);
    //reportError(tok, Severity::error, "newleak", "New memory leak: " + varname);
}

void CheckLeakAutoVar::mismatchError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(_tokenizer, _errorLogger, _settings->standards);
    std::list<const Token *> callstack(1, tok);
    c.mismatchAllocDealloc(callstack, varname);
    //reportError(tok, Severity::error, "newmismatch", "New mismatching allocation and deallocation: " + varname);
}

void CheckLeakAutoVar::deallocUseError(const Token *tok, const std::string &varname)
{
    const CheckMemoryLeak c(_tokenizer, _errorLogger, _settings->standards);
    c.deallocuseError(tok, varname);
    //reportError(tok, Severity::error, "newdeallocuse", "Using deallocated pointer " + varname);
}

void CheckLeakAutoVar::deallocReturnError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "deallocret", "Returning/dereferencing '" + varname + "' after it is deallocated / released");
}

void CheckLeakAutoVar::configurationInfo(const Token* tok, const std::string &functionName)
{
    if (((!cfgalloc.empty() || !cfgdealloc.empty()) && _settings->isEnabled("information")) || _settings->experimental) {
        reportError(tok,
                    Severity::information,
                    "leakconfiguration",
                    functionName + " configuration is needed to establish if there is a leak or not");
    }
}

void CheckLeakAutoVar::parseConfigurationFile(const std::string &filename)
{
    std::ifstream fin(filename.c_str());
    if (!fin.is_open())
        return;

    std::string line;
    while (std::getline(fin,line)) {
        if (line.compare(0,4,"MEM ",0,4) == 0) {
            std::string f1;
            enum {ALLOC, DEALLOC} type = ALLOC;
            std::string::size_type pos1 = line.find_first_not_of(" ", 4U);
            while (pos1 < line.size()) {
                const std::string::size_type pos2 = line.find(" ", pos1);
                std::string f;
                if (pos2 == std::string::npos)
                    f = line.substr(pos1);
                else
                    f = line.substr(pos1, pos2-pos1);
                if (f1.empty())
                    f1 = f;
                if (f == ":")
                    type = DEALLOC;
                else if (type == ALLOC)
                    cfgalloc[f] = f1;
                else if (type == DEALLOC)
                    cfgdealloc[f] = f1;
                pos1 = line.find_first_not_of(" ", pos2);
            }
        }

        else if (line.compare(0,7,"IGNORE ",0,7) == 0) {
            std::string::size_type pos1 = line.find_first_not_of(" ", 7U);
            while (pos1 < line.size()) {
                std::string::size_type pos2 = line.find_first_of(" ", pos1);
                std::string functionName;
                if (pos2 == std::string::npos)
                    functionName = line.substr(pos1);
                else
                    functionName = line.substr(pos1, pos2-pos1);
                cfgignore.insert(functionName);
                pos1 = line.find_first_not_of(" ", pos2);
            }
        }

        else if (line.compare(0,4,"USE ",0,4) == 0) {
            std::string::size_type pos1 = line.find_first_not_of(" ", 4U);
            while (pos1 < line.size()) {
                std::string::size_type pos2 = line.find_first_of(" ", pos1);
                std::string functionName;
                if (pos2 == std::string::npos)
                    functionName = line.substr(pos1);
                else
                    functionName = line.substr(pos1, pos2-pos1);
                cfguse.insert(functionName);
                pos1 = line.find_first_not_of(" ", pos2);
            }
        }

        else if (line.compare(0,9,"NORETURN ",0,9) == 0) {
            std::string::size_type pos1 = line.find_first_not_of(" ", 9U);
            while (pos1 < line.size()) {
                std::string::size_type pos2 = line.find_first_of(" ", pos1);
                std::string functionName;
                if (pos2 == std::string::npos)
                    functionName = line.substr(pos1);
                else
                    functionName = line.substr(pos1, pos2-pos1);
                cfgnoreturn.insert(functionName);
                pos1 = line.find_first_not_of(" ", pos2);
            }
        }
    }
}

void CheckLeakAutoVar::check()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Check function scopes
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        // Empty variable info
        VarInfo varInfo;

        // Local variables that are known to be non-zero.
        static const std::set<unsigned int> notzero;

        checkScope(scope->classStart, &varInfo, notzero);

        varInfo.conditionalAlloc.clear();

        // Clear reference arguments from varInfo..
        std::map<unsigned int, std::string>::iterator it = varInfo.alloctype.begin();
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
    std::map<unsigned int, std::string> &alloctype = varInfo->alloctype;
    std::map<unsigned int, std::string> &possibleUsage = varInfo->possibleUsage;
    const std::set<unsigned int> conditionalAlloc(varInfo->conditionalAlloc);

    // Allocation functions. key = function name, value = allocation type
    std::map<std::string, std::string> allocFunctions(cfgalloc);
    allocFunctions["malloc"] = "malloc";
    allocFunctions["strdup"] = "malloc";
    allocFunctions["fopen"] = "fopen";

    // Deallocation functions. key = function name, value = allocation type
    std::map<std::string, std::string> deallocFunctions(cfgdealloc);
    deallocFunctions["free"] = "malloc";
    deallocFunctions["fclose"] = "fopen";

    // Parse all tokens
    const Token * const endToken = startToken->link();
    for (const Token *tok = startToken; tok && tok != endToken; tok = tok->next()) {
        // Deallocation and then dereferencing pointer..
        if (tok->varId() > 0) {
            const std::map<unsigned int, std::string>::iterator var = alloctype.find(tok->varId());
            if (var != alloctype.end()) {
                if (var->second == "dealloc" && !Token::Match(tok->previous(), "[;{},=] %var% =")) {
                    deallocUseError(tok, tok->str());
                } else if (Token::simpleMatch(tok->tokAt(-2), "= &")) {
                    varInfo->erase(tok->varId());
                } else if (tok->strAt(-1) == "=") {
                    varInfo->erase(tok->varId());
                }
            } else if (Token::Match(tok->previous(), "& %var% = %var% ;")) {
                varInfo->referenced.insert(tok->tokAt(2)->varId());
            }
        }

        if (tok->str() == "(" && tok->previous()->isName()) {
            functionCall(tok->previous(), varInfo, "");
            tok = tok->link();
            continue;
        }

        // look for end of statement
        if (!Token::Match(tok, "[;{}]") || Token::Match(tok->next(), "[;{}]"))
            continue;
        tok = tok->next();
        if (!tok || tok == endToken)
            break;

        // parse statement

        // assignment..
        if (tok->varId() && Token::Match(tok, "%var% =")) {
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
            if (var && !var->isArgument() && !var->isLocal()) {
                continue;
            }

            // Don't check reference variables
            if (var && var->isReference())
                continue;

            // allocation?
            if (Token::Match(tok->tokAt(2), "%type% (")) {
                const std::map<std::string, std::string>::const_iterator it = allocFunctions.find(tok->strAt(2));
                if (it != allocFunctions.end()) {
                    alloctype[tok->varId()] = it->second;
                }
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
                    std::string dealloc;
                    {
                        const std::map<std::string, std::string>::iterator func = deallocFunctions.find(tok->str());
                        if (func != deallocFunctions.end()) {
                            dealloc = func->second;
                        }
                    }

                    functionCall(innerTok->previous(), varInfo, dealloc);
                    innerTok = innerTok->link();
                }
            }

            const Token *tok2 = tok->linkAt(1);
            if (Token::simpleMatch(tok2, ") {")) {
                VarInfo varInfo1(*varInfo);
                VarInfo varInfo2(*varInfo);

                if (Token::Match(tok->next(), "( %var% )")) {
                    varInfo2.erase(tok->tokAt(2)->varId());
                    if (notzero.find(tok->tokAt(2)->varId()) != notzero.end())
                        varInfo2.clear();
                } else if (Token::Match(tok->next(), "( ! %var% )|&&")) {
                    varInfo1.erase(tok->tokAt(3)->varId());
                } else if (Token::Match(tok->next(), "( %var% ( ! %var% ) )|&&")) {
                    varInfo1.erase(tok->tokAt(5)->varId());
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
                std::map<unsigned int, std::string>::const_iterator it;
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
                    if (it->second == "dealloc" && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
                        varInfo->conditionalAlloc.erase(it->first);
                        varInfo2.erase(it->first);
                    }
                }
                for (it = varInfo2.alloctype.begin(); it != varInfo2.alloctype.end(); ++it) {
                    if (it->second == "dealloc" && conditionalAlloc.find(it->first) != conditionalAlloc.end()) {
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

        // unknown control..
        else if (Token::Match(tok, "%type% (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            varInfo->clear();
            break;
        }

        // Function call..
        else if (Token::Match(tok, "%type% (") && tok->str() != "return") {
            std::string dealloc;
            {
                const std::map<std::string, std::string>::iterator func = deallocFunctions.find(tok->str());
                if (func != deallocFunctions.end()) {
                    dealloc = func->second;
                }
            }

            functionCall(tok, varInfo, dealloc);

            tok = tok->next()->link();

            // Handle scopes that might be noreturn
            if (dealloc.empty() && Token::simpleMatch(tok, ") ; }")) {
                const std::string &functionName(tok->link()->previous()->str());
                bool unknown = false;
                if (cfgignore.find(functionName) == cfgignore.end() &&
                    cfguse.find(functionName) == cfguse.end() &&
                    _tokenizer->IsScopeNoReturn(tok->tokAt(2), &unknown)) {
                    if (unknown) {
                        //const std::string &functionName(tok->link()->previous()->str());
                        varInfo->possibleUsageAll(functionName);
                    } else {
                        varInfo->clear();
                    }
                }
            }

            continue;
        }

        // return
        else if (tok->str() == "return") {
            ret(tok, *varInfo);
            varInfo->clear();
        }

        // goto => weird execution path
        else if (tok->str() == "goto") {
            varInfo->clear();
        }

        // throw
        // TODO: if the execution leave the function then treat it as return
        else if (tok->str() == "throw") {
            varInfo->clear();
        }
    }
}

void CheckLeakAutoVar::functionCall(const Token *tok, VarInfo *varInfo, const std::string &dealloc)
{
    std::map<unsigned int, std::string> &alloctype = varInfo->alloctype;
    std::map<unsigned int, std::string> &possibleUsage = varInfo->possibleUsage;

    // Ignore function call?
    const bool ignore = bool(cfgignore.find(tok->str()) != cfgignore.end());
    //const bool use = bool(cfguse.find(tok->str()) != cfguse.end());

    if (ignore)
        return;

    for (const Token *arg = tok->tokAt(2); arg; arg = arg->nextArgument()) {
        if ((Token::Match(arg, "%var% [-,)]") && arg->varId() > 0) ||
            (Token::Match(arg, "& %var%") && arg->next()->varId() > 0)) {

            // goto variable
            if (arg->str() == "&")
                arg = arg->next();

            // Is variable allocated?
            const std::map<unsigned int,std::string>::iterator var = alloctype.find(arg->varId());
            if (var != alloctype.end()) {
                if (dealloc.empty()) {
                    // possible usage
                    possibleUsage[arg->varId()] = tok->str();
                    if (var->second == "dealloc" && arg->previous()->str() == "&")
                        varInfo->erase(arg->varId());
                } else if (var->second == "dealloc") {
                    CheckOther checkOther(_tokenizer, _settings, _errorLogger);
                    checkOther.doubleFreeError(tok, arg->str());
                } else if (var->second != dealloc) {
                    // mismatching allocation and deallocation
                    mismatchError(tok, arg->str());
                    varInfo->erase(arg->varId());
                } else {
                    // deallocation
                    var->second = "dealloc";
                }
            } else if (!dealloc.empty()) {
                alloctype[arg->varId()] = "dealloc";
            }
        } else if (Token::Match(arg, "%var% (")) {
            functionCall(arg, varInfo, dealloc);
        }
    }
}


void CheckLeakAutoVar::leakIfAllocated(const Token *vartok,
                                       const VarInfo &varInfo)
{
    const std::map<unsigned int, std::string> &alloctype = varInfo.alloctype;
    const std::map<unsigned int, std::string> &possibleUsage = varInfo.possibleUsage;

    const std::map<unsigned int,std::string>::const_iterator var = alloctype.find(vartok->varId());
    if (var != alloctype.end() && var->second != "dealloc") {
        const std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(vartok->varId());
        if (use == possibleUsage.end()) {
            leakError(vartok, vartok->str(), var->second);
        } else {
            configurationInfo(vartok, use->second);
        }
    }
}

void CheckLeakAutoVar::ret(const Token *tok, const VarInfo &varInfo)
{
    const std::map<unsigned int, std::string> &alloctype = varInfo.alloctype;
    const std::map<unsigned int, std::string> &possibleUsage = varInfo.possibleUsage;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::map<unsigned int, std::string>::const_iterator it = alloctype.begin(); it != alloctype.end(); ++it) {
        // don't warn if variable is conditionally allocated
        if (it->second != "dealloc" && varInfo.conditionalAlloc.find(it->first) != varInfo.conditionalAlloc.end())
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
                if (Token::Match(tok2, "return|(|, & %varid% . %var% [);,]", varid)) {
                    used = true;
                    break;
                }
            }

            // return deallocated pointer
            if (used && it->second == "dealloc")
                deallocReturnError(tok, var->name());

            else if (!used && it->second != "dealloc") {

                const std::map<unsigned int, std::string>::const_iterator use = possibleUsage.find(varid);
                if (use == possibleUsage.end()) {
                    leakError(tok, var->name(), it->second);
                } else {
                    configurationInfo(tok, use->second);
                }
            }
        }
    }
}
