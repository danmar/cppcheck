/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "checkclass.h"

#include "tokenize.h"
#include "token.h"
#include "errorlogger.h"
#include "symboldatabase.h"

#include <locale>

#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

//---------------------------------------------------------------------------

// Register CheckClass..
namespace
{
CheckClass instance;
}

//---------------------------------------------------------------------------

CheckClass::CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    : Check(myName(), tokenizer, settings, errorLogger),
      symbolDatabase(NULL)
{

}

void CheckClass::createSymbolDatabase()
{
    // Multiple calls => bail out
    if (symbolDatabase)
        return;

    symbolDatabase = _tokenizer->getSymbolDatabase();
}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    if (!_settings->isEnabled("style"))
        return;

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check classes and structures
        if (!scope->isClassOrStruct())
            continue;

        // There are no constructors.
        if (scope->numConstructors == 0)
        {
            // If there is a private variable, there should be a constructor..
            std::list<Variable>::const_iterator var;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
            {
                if (var->isPrivate() && !var->isClass() && !var->isStatic())
                {
                    noConstructorError(scope->classDef, scope->className, scope->classDef->str() == "struct");
                    break;
                }
            }
        }

        std::list<Function>::const_iterator func;
        std::vector<Usage> usage(scope->varlist.size());

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            if (!func->hasBody || !(func->type == Function::eConstructor ||
                                    func->type == Function::eCopyConstructor ||
                                    func->type == Function::eOperatorEqual))
                continue;

            // Mark all variables not used
            clearAllVar(usage);

            std::list<std::string> callstack;
            initializeVarList(*func, callstack, &(*scope), usage);

            // Check if any variables are uninitialized
            std::list<Variable>::const_iterator var;
            unsigned int count = 0;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count)
            {
                if (usage[count].assign || usage[count].init || var->isStatic())
                    continue;

                if (var->isConst() && var->nameToken()->previous()->str() != "*")
                    continue;

                // Check if this is a class constructor
                if (var->isClass() && func->type == Function::eConstructor)
                {
                    // Unknown type so assume it is initialized
                    if (!var->type())
                        continue;

                    // Known type that doesn't need initialization or
                    // known type that has member variables of an unknown type
                    else if (var->type()->needInitialization != Scope::True)
                        continue;
                }

                // Check if type can't be copied
                if (var->type() && canNotCopy(var->type()))
                    continue;

                // It's non-static and it's not initialized => error
                if (func->type == Function::eOperatorEqual)
                {
                    const Token *operStart = func->token->tokAt(1);

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next())
                    {
                        if (operTok->str() == scope->className)
                        {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed)
                        operatorEqVarError(func->token, scope->className, var->name());
                }
                else if (func->access != Private)
                    uninitVarError(func->token, scope->className, var->name());
            }
        }
    }
}

bool CheckClass::canNotCopy(const Scope *scope) const
{
    std::list<Function>::const_iterator func;
    bool privateAssign = false;
    bool privateCopy = false;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
    {
        if (func->type == Function::eCopyConstructor && func->access == Private)
            privateCopy = true;
        else if (func->type == Function::eOperatorEqual && func->access == Private)
            privateAssign = true;
    }

    return privateAssign && privateCopy;
}

void CheckClass::assignVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage)
{
    std::list<Variable>::const_iterator var;
    unsigned int count = 0;

    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count)
    {
        if (var->name() == varname)
        {
            usage[count].assign = true;
            return;
        }
    }
}

void CheckClass::initVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage)
{
    std::list<Variable>::const_iterator var;
    unsigned int count = 0;

    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var, ++count)
    {
        if (var->name() == varname)
        {
            usage[count].init = true;
            return;
        }
    }
}

void CheckClass::assignAllVar(std::vector<Usage> &usage)
{
    for (size_t i = 0; i < usage.size(); ++i)
        usage[i].assign = true;
}

void CheckClass::clearAllVar(std::vector<Usage> &usage)
{
    for (size_t i = 0; i < usage.size(); ++i)
    {
        usage[i].assign = false;
        usage[i].init = false;
    }
}

bool CheckClass::isBaseClassFunc(const Token *tok, const Scope *scope)
{
    // Iterate through each base class...
    for (size_t i = 0; i < scope->derivedFrom.size(); ++i)
    {
        const Scope *derivedFrom = scope->derivedFrom[i].scope;

        // Check if base class exists in database
        if (derivedFrom)
        {
            std::list<Function>::const_iterator func;

            for (func = derivedFrom->functionList.begin(); func != derivedFrom->functionList.end(); ++func)
            {
                if (func->tokenDef->str() == tok->str())
                    return true;
            }
        }

        // Base class not found so assume it is in it.
        else
            return true;
    }

    return false;
}

void CheckClass::initializeVarList(const Function &func, std::list<std::string> &callstack, const Scope *scope, std::vector<Usage> &usage)
{
    bool Assign = false;
    unsigned int indentlevel = 0;
    const Token *ftok = func.token;

    for (; ftok; ftok = ftok->next())
    {
        if (!ftok->next())
            break;

        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (indentlevel == 0)
        {
            if (Assign && Token::Match(ftok, "%var% ("))
            {
                initVar(ftok->str(), scope, usage);

                // assignment in the initializer..
                // : var(value = x)
                if (Token::Match(ftok->tokAt(2), "%var% ="))
                    assignVar(ftok->strAt(2), scope, usage);
            }

            Assign |= (ftok->str() == ":");
        }


        if (ftok->str() == "{")
        {
            ++indentlevel;
            Assign = false;
        }

        else if (ftok->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        if (indentlevel < 1)
            continue;

        // Variable getting value from stream?
        if (Token::Match(ftok, ">> %var%"))
        {
            assignVar(ftok->strAt(1), scope, usage);
        }

        // Before a new statement there is "[{};)=]"
        if (! Token::Match(ftok, "[{};()=]"))
            continue;

        if (Token::simpleMatch(ftok, "( !"))
            ftok = ftok->next();

        // Using the operator= function to initialize all variables..
        if (Token::Match(ftok->next(), "return| * this ="))
        {
            assignAllVar(usage);
            break;
        }

        // Calling member variable function?
        if (Token::Match(ftok->next(), "%var% . %var% ("))
        {
            std::list<Variable>::const_iterator var;
            for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
            {
                if (var->varId() == ftok->next()->varId())
                {
                    /** @todo false negative: we assume function changes variable state */
                    assignVar(ftok->next()->str(), scope, usage);
                    continue;
                }
            }

            ftok = ftok->tokAt(2);
        }

        if (!Token::Match(ftok->next(), "::| %var%") &&
            !Token::Match(ftok->next(), "this . %var%") &&
            !Token::Match(ftok->next(), "* %var% =") &&
            !Token::Match(ftok->next(), "( * this ) . %var%"))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next();

        // skip "return"
        if (ftok->str() == "return")
            ftok = ftok->next();

        // Skip "( * this )"
        if (Token::simpleMatch(ftok, "( * this ) ."))
        {
            ftok = ftok->tokAt(5);
        }

        // Skip "this->"
        if (Token::simpleMatch(ftok, "this ."))
            ftok = ftok->tokAt(2);

        // Skip "classname :: "
        if (Token::Match(ftok, "%var% ::"))
            ftok = ftok->tokAt(2);

        // Clearing all variables..
        if (Token::Match(ftok, "::| memset ( this ,"))
        {
            assignAllVar(usage);
            return;
        }

        // Clearing array..
        else if (Token::Match(ftok, "::| memset ( %var% ,"))
        {
            const int offset = ftok->str() == "::" ? 1 : 0;
            assignVar(ftok->strAt(2 + offset), scope, usage);
            ftok = ftok->tokAt(1 + offset)->link();
            continue;
        }

        // Calling member function?
        else if (Token::simpleMatch(ftok, "operator= (") &&
                 ftok->previous()->str() != "::")
        {
            // check if member function exists
            std::list<Function>::const_iterator it;
            for (it = scope->functionList.begin(); it != scope->functionList.end(); ++it)
            {
                if (ftok->str() == it->tokenDef->str() && it->type != Function::eConstructor)
                    break;
            }

            // member function found
            if (it != scope->functionList.end())
            {
                // member function has implementation
                if (it->hasBody)
                {
                    // initialize variable use list using member function
                    callstack.push_back(ftok->str());
                    initializeVarList(*it, callstack, scope, usage);
                    callstack.pop_back();
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else
                {
                    assignAllVar(usage);
                }
            }

            // using default operator =, assume everything initialized
            else
            {
                assignAllVar(usage);
            }
        }
        else if (Token::Match(ftok, "%var% (") && ftok->str() != "if")
        {
            // Passing "this" => assume that everything is initialized
            for (const Token *tok2 = ftok->next()->link(); tok2 && tok2 != ftok; tok2 = tok2->previous())
            {
                if (tok2->str() == "this")
                {
                    assignAllVar(usage);
                    return;
                }
            }

            // recursive call / calling overloaded function
            // assume that all variables are initialized
            if (std::find(callstack.begin(), callstack.end(), ftok->str()) != callstack.end())
            {
                assignAllVar(usage);
                return;
            }

            // check if member function
            std::list<Function>::const_iterator it;
            for (it = scope->functionList.begin(); it != scope->functionList.end(); ++it)
            {
                if (ftok->str() == it->tokenDef->str() && it->type != Function::eConstructor)
                    break;
            }

            // member function found
            if (it != scope->functionList.end())
            {
                // member function has implementation
                if (it->hasBody)
                {
                    // initialize variable use list using member function
                    callstack.push_back(ftok->str());
                    initializeVarList(*it, callstack, scope, usage);
                    callstack.pop_back();
                }

                // there is a called member function, but it has no implementation, so we assume it initializes everything
                else
                {
                    assignAllVar(usage);
                }
            }

            // not member function
            else
            {
                // could be a base class virtual function, so we assume it initializes everything
                if (func.type != Function::eConstructor && isBaseClassFunc(ftok, scope))
                {
                    /** @todo False Negative: we should look at the base class functions to see if they
                     *  call any derived class virtual functions that change the derived class state
                     */
                    assignAllVar(usage);
                }

                // has friends, so we assume it initializes everything
                if (!scope->friendList.empty())
                    assignAllVar(usage);

                // the function is external and it's neither friend nor inherited virtual function.
                // assume all variables that are passed to it are initialized..
                else
                {
                    unsigned int indentlevel2 = 0;
                    for (const Token *tok = ftok->tokAt(2); tok; tok = tok->next())
                    {
                        if (tok->str() == "(")
                            ++indentlevel2;
                        else if (tok->str() == ")")
                        {
                            if (indentlevel2 == 0)
                                break;
                            --indentlevel2;
                        }
                        if (tok->isName())
                        {
                            assignVar(tok->str(), scope, usage);
                        }
                    }
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%var% ="))
        {
            assignVar(ftok->str(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [") &&
                 Token::simpleMatch(ftok->next()->link(), "] ="))
        {
            assignVar(ftok->str(), scope, usage);
        }

        // Assignment of member of array item of member variable?
        else if (Token::Match(ftok, "%var% [ ") &&
                 (Token::Match(ftok->next()->link(), "] . %var%  =") ||
                  Token::Match(ftok->next()->link(), "] . %var% . %var%  =")))
        {
            assignVar(ftok->str(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [") &&
                 Token::simpleMatch(ftok->next()->link(), "] [") &&
                 Token::simpleMatch(ftok->next()->link()->next()->link(), "] ="))
        {
            assignVar(ftok->str(), scope, usage);
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "* %var% ="))
        {
            assignVar(ftok->next()->str(), scope, usage);
        }

        // Assignment of struct member of member variable?
        else if (Token::Match(ftok, "%var% . %any% ="))
        {
            assignVar(ftok->str(), scope, usage);
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%var% . clear|Clear ("))
        {
            assignVar(ftok->str(), scope, usage);
        }
    }
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // For performance reasons the constructor might be intentionally missing. Therefore this is not a "warning"
    reportError(tok, Severity::style, "noConstructor",
                "The " + std::string(isStruct ? "struct" : "class") + " '" + classname +
                "' does not have a constructor.\n"
                "The " + std::string(isStruct ? "struct" : "class") + " '" + classname +
                " 'does not have a constructor but it has attributes. "
                "The attributes are not initialized which may cause bugs or undefined behavior.");
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::warning, "uninitVar", "Member variable '" + classname + "::" + varname + "' is not initialized in the constructor.");
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::warning, "operatorEqVarError", "Member variable '" + classname + "::" + varname + "' is not assigned a value in '" + classname + "::operator=" + "'");
}

//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckClass::privateFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    // don't check code that contains templates. Templates that are
    // "unused" are removed from the code. #2067
    if (_tokenizer->codeWithTemplates())
        return;

    // dont check borland classes with properties..
    if (Token::findmatch(_tokenizer->tokens(), "; __property ;"))
        return;

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check classes and structures
        if (!scope->isClassOrStruct())
            continue;

        // don’t check derived classes
        if (!scope->derivedFrom.empty())
            continue;

        // skip checking if there are friends
        if (!scope->friendList.empty())
            continue;

        // Locate some class
        const Token *tok1 = scope->classDef;

        // check that the whole class implementation is seen
        bool whole = true;
        std::list<Function>::const_iterator func;
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            if (!func->hasBody)
            {
                // empty private copy constructors and assignment operators are OK
                if ((func->type == Function::eCopyConstructor ||
                     func->type == Function::eOperatorEqual) &&
                    func->access == Private)
                    continue;

                whole = false;
                break;
            }
        }

        if (!whole)
            continue;

        const std::string &classname = tok1->next()->str();

        std::list<const Token *> FuncList;
        /** @todo embedded class have access to private functions */
        if (!scope->getNestedNonFunctions())
        {
            for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
            {
                // Get private functions..
                if (func->type == Function::eFunction && func->access == Private)
                    FuncList.push_back(func->tokenDef);
            }
        }

        // Check that all private functions are used..
        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            const Token *ftok = func->start;
            if (ftok)
            {
                const Token *etok = ftok->link();

                for (; ftok != etok; ftok = ftok->next())
                {
                    if (Token::Match(ftok, "%var% ("))
                    {
                        // Remove function from FuncList
                        std::list<const Token *>::iterator it = FuncList.begin();
                        while (it != FuncList.end())
                        {
                            if (ftok->str() == (*it)->str())
                                FuncList.erase(it++);
                            else
                                ++it;
                        }
                    }
                }
            }
        }

        while (!FuncList.empty())
        {
            // Final check; check if the function pointer is used somewhere..
            const std::string _pattern("return|(|)|,|= &|" + FuncList.front()->str());

            // or if the function address is used somewhere...
            // eg. sigc::mem_fun(this, &className::classFunction)
            const std::string _pattern2("& " + classname + " :: " + FuncList.front()->str());
            const std::string methodAsArgument("(|, " + classname + " :: " + FuncList.front()->str() + " ,|)");
            if (!Token::findmatch(_tokenizer->tokens(), _pattern.c_str()) &&
                !Token::findmatch(_tokenizer->tokens(), _pattern2.c_str()) &&
                !Token::findmatch(_tokenizer->tokens(), methodAsArgument.c_str())
               )
            {
                unusedPrivateFunctionError(FuncList.front(), classname, FuncList.front()->str());
            }
            FuncList.pop_front();
        }
    }
}

void CheckClass::unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "unusedPrivateFunction", "Unused private function '" + classname + "::" + funcname + "'");
}

//---------------------------------------------------------------------------
// ClassCheck: Check that memset is not used on classes
//---------------------------------------------------------------------------

void CheckClass::noMemset()
{
    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            // only check functions with bodies
            if (!func->hasBody)
                continue;

            // Locate all 'memset' tokens..
            const Token *end = func->start->link();
            for (const Token *tok = func->start; tok && tok != end; tok = tok->next())
            {
                if (!Token::Match(tok, "memset|memcpy|memmove"))
                    continue;

                const Token *typeTok = 0;
                if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( %type% ) )"))
                    typeTok = tok->tokAt(8);
                else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( %type% ) )"))
                    typeTok = tok->tokAt(9);
                else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( %type% :: %type% ) )"))
                    typeTok = tok->tokAt(11);
                else if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( struct %type% ) )"))
                    typeTok = tok->tokAt(9);
                else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( struct %type% ) )"))
                    typeTok = tok->tokAt(10);
                else if (Token::Match(tok, "%type% ( %var% , %var% , sizeof ( %type% ) )"))
                    typeTok = tok->tokAt(8);
                else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( %var% ) )"))
                {
                    unsigned int varid = tok->tokAt(3)->varId();
                    const Variable *var = symbolDatabase->getVariableFromVarId(varid);
                    if (var && (var->typeStartToken() == var->typeEndToken() ||
                                Token::Match(var->typeStartToken(), "%type% :: %type%")))
                        typeTok = var->typeEndToken();
                }

                // No type defined => The tokens didn't match
                if (!typeTok)
                    continue;

                const Scope *type = symbolDatabase->findVariableType(&(*scope), typeTok);

                if (type)
                    checkMemsetType(&(*scope), tok, type);
            }
        }
    }
}

void CheckClass::checkMemsetType(const Scope *start, const Token *tok, const Scope *type)
{
    // recursively check all parent classes
    for (size_t i = 0; i < type->derivedFrom.size(); i++)
    {
        if (type->derivedFrom[i].scope)
            checkMemsetType(start, tok, type->derivedFrom[i].scope);
    }

    // Warn if type is a class that contains any virtual functions
    std::list<Function>::const_iterator func;

    for (func = type->functionList.begin(); func != type->functionList.end(); ++func)
    {
        if (func->isVirtual)
            memsetError(tok, tok->str(), "virtual method", type->classDef->str());
    }

    // Warn if type is a class or struct that contains any std::* variables
    std::list<Variable>::const_iterator var;

    for (var = type->varlist.begin(); var != type->varlist.end(); ++var)
    {
        // don't warn if variable static or const
        if (!var->isStatic() && !var->isConst())
        {
            const Token *tok1 = var->typeStartToken();

            // check for std:: type that is not a pointer or reference
            if (Token::simpleMatch(tok1, "std ::") && !Token::Match(var->nameToken()->previous(), "*|&"))
                memsetError(tok, tok->str(), "'std::" + tok1->strAt(2) + "'", type->classDef->str());

            // check for known type that is not a pointer or reference
            else if (var->type() && !Token::Match(var->nameToken()->previous(), "*|&"))
                checkMemsetType(start, tok, var->type());
        }
    }
}

void CheckClass::memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type)
{
    reportError(tok, Severity::error, "memsetClass", "Using '" + memfunc + "' on " + type + " that contains a " + classname);
}

//---------------------------------------------------------------------------
// ClassCheck: "void operator=(" and "const type & operator=("
//---------------------------------------------------------------------------

void CheckClass::operatorEq()
{
    if (!_settings->isEnabled("style"))
        return;

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        if (!scope->isClassOrStruct())
            continue;

        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            if (func->type == Function::eOperatorEqual && func->access != Private)
            {
                // use definition for check so we don't have to deal with qualification
                if (!(Token::Match(func->tokenDef->tokAt(-3), ";|}|{|public:|protected:|private: %type% &") &&
                      func->tokenDef->strAt(-2) == scope->className))
                {
                    // make sure we really have a copy assignment operator
                    if (Token::Match(func->tokenDef->tokAt(2), "const| %var% &"))
                    {
                        if (func->tokenDef->strAt(2) == "const" &&
                            func->tokenDef->strAt(3) == scope->className)
                            operatorEqReturnError(func->tokenDef->tokAt(-1), scope->className);
                        else if (func->tokenDef->strAt(2) == scope->className)
                            operatorEqReturnError(func->tokenDef->tokAt(-1), scope->className);
                    }
                }
            }
        }
    }
}

void CheckClass::operatorEqReturnError(const Token *tok, const std::string &className)
{
    reportError(tok, Severity::style, "operatorEq", "\'" + className + "::operator=' should return \'" + className + " &\'");
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::operatorEqRetRefThis()
{
    if (!_settings->isEnabled("style"))
        return;

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check classes and structures
        if (scope->isClassOrStruct())
        {
            std::list<Function>::const_iterator func;

            for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
            {
                if (func->type == Function::eOperatorEqual && func->hasBody)
                {
                    // make sure return signature is correct
                    if (Token::Match(func->tokenDef->tokAt(-3), ";|}|{|public:|protected:|private: %type% &") &&
                        func->tokenDef->strAt(-2) == scope->className)
                    {
                        // find the ')'
                        const Token *tok = func->token->next()->link();

                        checkReturnPtrThis(&(*scope), &(*func), tok->tokAt(2), tok->next()->link());
                    }
                }
            }
        }
    }
}

void CheckClass::checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last)
{
    bool foundReturn = false;

    for (; tok && tok != last; tok = tok->next())
    {
        // check for return of reference to this
        if (tok->str() == "return")
        {
            foundReturn = true;
            std::string cast("( " + scope->className + " & )");
            if (Token::Match(tok->next(), cast.c_str()))
                tok = tok->tokAt(4);

            // check if a function is called
            if (Token::Match(tok->tokAt(1), "%any% (") &&
                tok->tokAt(2)->link()->next()->str() == ";")
            {
                std::list<Function>::const_iterator it;

                // check if it is a member function
                for (it = scope->functionList.begin(); it != scope->functionList.end(); ++it)
                {
                    // check for a regular function with the same name and a body
                    if (it->type == Function::eFunction && it->hasBody &&
                        it->token->str() == tok->next()->str())
                    {
                        // check for the proper return type
                        if (it->tokenDef->previous()->str() == "&" &&
                            it->tokenDef->strAt(-2) == scope->className)
                        {
                            // make sure it's not a const function
                            if (it->arg->link()->next()->str() != "const")
                            {
                                /** @todo make sure argument types match */
                                // make sure it's not the same function
                                if (&*it != func)
                                    checkReturnPtrThis(scope, &*it, it->arg->link()->next(), it->arg->link()->next()->link());

                                // just bail for now
                                else
                                    return;
                            }
                        }
                    }
                }
            }

            // check if *this is returned
            else if (!(Token::Match(tok->tokAt(1), "(| * this ;|=") ||
                       Token::Match(tok->tokAt(1), "(| * this +=") ||
                       Token::simpleMatch(tok->tokAt(1), "operator= (") ||
                       Token::simpleMatch(tok->tokAt(1), "this . operator= (") ||
                       (Token::Match(tok->tokAt(1), "%type% :: operator= (") &&
                        tok->next()->str() == scope->className)))
                operatorEqRetRefThisError(func->token);
        }
    }
    if (!foundReturn)
        operatorEqRetRefThisError(func->token);
}

void CheckClass::operatorEqRetRefThisError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEqRetRefThis", "'operator=' should return reference to self");
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C& rhs) { if (this == &rhs) ... }"
// operator= should check for assignment to self
//
// For simple classes, an assignment to self check is only a potential optimization.
//
// For classes that allocate dynamic memory, assignment to self can be a real error
// if it is deallocated and allocated again without being checked for.
//
// This check is not valid for classes with multiple inheritance because a
// class can have multiple addresses so there is no trivial way to check for
// assignment to self.
//---------------------------------------------------------------------------

void CheckClass::operatorEqToSelf()
{
    if (!_settings->isEnabled("style"))
        return;

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        if (!scope->isClassOrStruct())
            continue;

        std::list<Function>::const_iterator func;

        // skip classes with multiple inheritance
        if (scope->derivedFrom.size() > 1)
            continue;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            if (func->type == Function::eOperatorEqual && func->hasBody)
            {
                // make sure return signature is correct
                if (Token::Match(func->tokenDef->tokAt(-3), ";|}|{|public:|protected:|private: %type% &") &&
                    func->tokenDef->strAt(-2) == scope->className)
                {
                    // find the parameter name
                    const Token *rhs = func->argumentList.begin()->nameToken();

                    if (!hasAssignSelf(&(*func), rhs))
                    {
                        if (hasDeallocation(&(*func)))
                            operatorEqToSelfError(func->token);
                    }
                }
            }
        }
    }
}

bool CheckClass::hasDeallocation(const Function *func)
{
    // This function is called when no simple check was found for assignment
    // to self.  We are currently looking for a specific sequence of:
    // deallocate member ; ... member = allocate
    // This check is far from ideal because it can cause false negatives.
    // Unfortunately, this is necessary to prevent false positives.
    // This check needs to do careful analysis someday to get this
    // correct with a high degree of certainty.
    const Token *last = func->start->link();
    for (const Token *tok = func->start; tok && (tok != last); tok = tok->next())
    {
        // check for deallocating memory
        if (Token::Match(tok, "{|;|, free ( %var%"))
        {
            const Token *var = tok->tokAt(3);

            // we should probably check that var is a pointer in this class

            const Token *tok1 = tok->tokAt(4);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% ="))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete [ ] %var%"))
        {
            const Token *var = tok->tokAt(4);

            // we should probably check that var is a pointer in this class

            const Token *tok1 = tok->tokAt(5);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% = new %type% ["))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
        else if (Token::Match(tok, "{|;|, delete %var%"))
        {
            const Token *var = tok->tokAt(2);

            // we should probably check that var is a pointer in this class

            const Token *tok1 = tok->tokAt(3);

            while (tok1 && (tok1 != last))
            {
                if (Token::Match(tok1, "%var% = new"))
                {
                    if (tok1->str() == var->str())
                        return true;
                }

                tok1 = tok1->next();
            }
        }
    }

    return false;
}

bool CheckClass::hasAssignSelf(const Function *func, const Token *rhs)
{
    const Token *last = func->start->link();
    for (const Token *tok = func->start; tok && tok != last; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "if ("))
        {
            const Token *tok1 = tok->tokAt(2);
            const Token *tok2 = tok->tokAt(1)->link();

            if (tok1 && tok2)
            {
                for (; tok1 && tok1 != tok2; tok1 = tok1->next())
                {
                    if (Token::Match(tok1, "this ==|!= & %var%"))
                    {
                        if (tok1->tokAt(3)->str() == rhs->str())
                            return true;
                    }
                    else if (Token::Match(tok1, "& %var% ==|!= this"))
                    {
                        if (tok1->tokAt(1)->str() == rhs->str())
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

void CheckClass::operatorEqToSelfError(const Token *tok)
{
    reportError(tok, Severity::warning, "operatorEqToSelf", "'operator=' should check for assignment to self");
}

//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    // This error should only be given if:
    // * base class doesn't have virtual destructor
    // * derived class has non-empty destructor
    // * base class is deleted

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // Skip base classes and namespaces
        if (scope->derivedFrom.empty())
            continue;

        // Find the destructor
        const Function *destructor = scope->getDestructor();

        // Check for destructor with implementation
        if (!destructor || !destructor->hasBody)
            continue;

        // Empty destructor
        if (destructor->token->tokAt(3)->link() == destructor->token->tokAt(4))
            continue;

        const Token *derived = scope->classDef;
        const Token *derivedClass = derived->tokAt(1);

        // Iterate through each base class...
        for (unsigned int j = 0; j < scope->derivedFrom.size(); ++j)
        {
            // Check if base class is public and exists in database
            if (scope->derivedFrom[j].access != Private && scope->derivedFrom[j].scope)
            {
                const Scope *derivedFrom = scope->derivedFrom[j].scope;

                // Name of base class..
                const std::string baseName = derivedFrom->className;

                // Check for this pattern:
                // 1. Base class pointer is given the address of derived class instance
                // 2. Base class pointer is deleted
                //
                // If this pattern is not seen then bailout the checking of these base/derived classes
                {
                    // pointer variables of type 'Base *'
                    std::set<unsigned int> basepointer;

                    // pointer variables of type 'Base *' that should not be deleted
                    std::set<unsigned int> dontDelete;

                    // No deletion of derived class instance through base class pointer found => the code is ok
                    bool ok = true;

                    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
                    {
                        // Declaring base class pointer
                        if (Token::simpleMatch(tok, baseName.c_str()))
                        {
                            if (Token::Match(tok->previous(), ("[;{}] " + baseName + " * %var% ;").c_str()))
                                basepointer.insert(tok->tokAt(2)->varId());
                        }

                        // Assign base class pointer with pointer to derived class instance
                        if (Token::Match(tok, "[;{}] %var% =") &&
                            tok->next()->varId() > 0 &&
                            basepointer.find(tok->next()->varId()) != basepointer.end())
                        {
                            // new derived class..
                            if (Token::simpleMatch(tok->tokAt(3), ("new " + derivedClass->str()).c_str()))
                            {
                                dontDelete.insert(tok->next()->varId());
                            }
                        }

                        // Delete base class pointer that might point at derived class
                        if (Token::Match(tok, "delete %var% ;") &&
                            tok->next()->varId() &&
                            dontDelete.find(tok->next()->varId()) != dontDelete.end())
                        {
                            ok = false;
                            break;
                        }
                    }

                    // No base class pointer that points at a derived class is deleted
                    if (ok)
                        continue;
                }

                // Find the destructor declaration for the base class.
                const Function *base_destructor = derivedFrom->getDestructor();
                const Token *base = 0;
                if (base_destructor)
                    base = base_destructor->token;

                // Check that there is a destructor..
                if (!base_destructor)
                {
                    if (derivedFrom->derivedFrom.empty())
                        virtualDestructorError(derivedFrom->classDef, baseName, derivedClass->str());
                }
                else if (!base_destructor->isVirtual)
                {
                    // TODO: This is just a temporary fix, better solution is needed.
                    // Skip situations where base class has base classes of its own, because
                    // some of the base classes might have virtual destructor.
                    // Proper solution is to check all of the base classes. If base class is not
                    // found or if one of the base classes has virtual destructor, error should not
                    // be printed. See TODO test case "virtualDestructorInherited"
                    if (derivedFrom->derivedFrom.empty())
                    {
                        // Make sure that the destructor is public (protected or private
                        // would not compile if inheritance is used in a way that would
                        // cause the bug we are trying to find here.)
                        if (base_destructor->access == Public)
                            virtualDestructorError(base, baseName, derivedClass->str());
                    }
                }
            }
        }
    }
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived)
{
    reportError(tok, Severity::error, "virtualDestructor", "Class " + Base + " which is inherited by class " + Derived + " does not have a virtual destructor");
}

//---------------------------------------------------------------------------
// warn for "this-x". The indented code may be "this->x"
//---------------------------------------------------------------------------

void CheckClass::thisSubtraction()
{
    if (!_settings->isEnabled("style"))
        return;

    const Token *tok = _tokenizer->tokens();
    for (;;)
    {
        tok = Token::findmatch(tok, "this - %var%");
        if (!tok)
            break;

        if (!Token::simpleMatch(tok->previous(), "*"))
            thisSubtractionError(tok);

        tok = tok->next();
    }
}

void CheckClass::thisSubtractionError(const Token *tok)
{
    reportError(tok, Severity::warning, "thisSubtraction", "Suspicious pointer subtraction");
}

//---------------------------------------------------------------------------
// can member function be const?
//---------------------------------------------------------------------------

void CheckClass::checkConst()
{
    if (!_settings->isEnabled("information") || _settings->ifcfg)
        return;

    // Don't check C# and JAVA classes
    if (_tokenizer->isJavaOrCSharp())
    {
        return;
    }

    createSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check classes and structures
        if (!scope->isClassOrStruct())
            continue;

        std::list<Function>::const_iterator func;

        for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
        {
            // does the function have a body?
            if (func->type == Function::eFunction && func->hasBody && !func->isFriend && !func->isStatic && !func->isConst && !func->isVirtual)
            {
                // get last token of return type
                const Token *previous = func->tokenDef->isName() ? func->token->previous() : func->token->tokAt(-2);
                while (previous && previous->str() == "::")
                    previous = previous->tokAt(-2);

                // does the function return a pointer or reference?
                if (Token::Match(previous, "*|&"))
                {
                    const Token *temp = func->token->previous();

                    while (!Token::Match(temp->previous(), ";|}|{|public:|protected:|private:"))
                        temp = temp->previous();

                    if (temp->str() != "const")
                        continue;
                }
                else if (Token::Match(previous->previous(), "*|& >"))
                {
                    const Token *temp = func->token->previous();

                    while (!Token::Match(temp->previous(), ";|}|{|public:|protected:|private:"))
                    {
                        temp = temp->previous();
                        if (temp->str() == "const")
                            break;
                    }

                    if (temp->str() != "const")
                        continue;
                }
                else
                {
                    // don't warn for unknown types..
                    // LPVOID, HDC, etc
                    if (previous->isName())
                    {
                        bool allupper = true;
                        const std::string s(previous->str());
                        for (std::string::size_type pos = 0; pos < s.size(); ++pos)
                        {
                            const char ch = s[pos];
                            if (!(ch == '_' || (ch >= 'A' && ch <= 'Z')))
                            {
                                allupper = false;
                                break;
                            }
                        }

                        if (allupper && previous->str().size() > 2)
                            continue;
                    }
                }

                const Token *paramEnd = func->arg->link();

                // check if base class function is virtual
                if (!scope->derivedFrom.empty())
                {
                    if (isVirtualFunc(&(*scope), func->tokenDef))
                        continue;
                }

                // if nothing non-const was found. write error..
                if (checkConstFunc(&(*scope), paramEnd))
                {
                    std::string classname = scope->className;
                    const Scope *nest = scope->nestedIn;
                    while (nest && nest->type != Scope::eGlobal)
                    {
                        classname = std::string(nest->className + "::" + classname);
                        nest = nest->nestedIn;
                    }

                    // get function name
                    std::string functionName((func->tokenDef->isName() ? "" : "operator") + func->tokenDef->str());

                    if (func->tokenDef->str() == "(")
                        functionName += ")";
                    else if (func->tokenDef->str() == "[")
                        functionName += "]";

                    if (func->isInline)
                        checkConstError(func->token, classname, functionName);
                    else // not inline
                        checkConstError2(func->token, func->tokenDef, classname, functionName);
                }
            }
        }
    }
}

bool CheckClass::isMemberVar(const Scope *scope, const Token *tok)
{
    bool again = false;

    // try to find the member variable
    do
    {
        again = false;

        if (tok->str() == "this")
        {
            return true;
        }
        else if (Token::Match(tok->tokAt(-3), "( * this )"))
        {
            return true;
        }
        else if (Token::Match(tok->tokAt(-2), "%var% . %var%"))
        {
            tok = tok->tokAt(-2);
            again = true;
        }
        else if (Token::Match(tok->tokAt(-2), "] . %var%"))
        {
            tok = tok->tokAt(-2)->link()->previous();
            again = true;
        }
        else if (tok->str() == "]")
        {
            tok = tok->link()->previous();
            again = true;
        }
    }
    while (again);

    std::list<Variable>::const_iterator var;
    for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
    {
        if (var->name() == tok->str())
        {
            if (tok->varId() == 0)
                symbolDatabase->debugMessage(tok, "CheckClass::isMemberVar found used member variable \'" + tok->str() + "\' with varid 0");

            return !var->isMutable();
        }
    }

    // not found in this class
    if (!scope->derivedFrom.empty())
    {
        // check each base class
        for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i)
        {
            // find the base class
            const Scope *derivedFrom = scope->derivedFrom[i].scope;

            // find the function in the base class
            if (derivedFrom)
            {
                if (isMemberVar(derivedFrom, tok))
                    return true;
            }
        }
    }

    return false;
}

static int countParameters(const Token *tok)
{
    if (Token::Match(tok->tokAt(2), "void| )"))
        return 0;

    int numpar = 1;
    int parlevel = 0;
    for (; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            ++parlevel;

        else if (tok->str() == ")")
        {
            if (parlevel <= 1)
                break;
            --parlevel;
        }

        else if (parlevel == 1 && tok->str() == ",")
        {
            ++numpar;
        }
    }

    return numpar;
}

bool CheckClass::isConstMemberFunc(const Scope *scope, const Token *tok)
{
    unsigned int args = countParameters(tok);

    std::list<Function>::const_iterator    func;
    unsigned int matches = 0;
    unsigned int consts = 0;

    for (func = scope->functionList.begin(); func != scope->functionList.end(); ++func)
    {
        /** @todo we need to look at the argument types when there are overloaded functions
          * with the same number of arguments */
        if (func->tokenDef->str() == tok->str() && func->argCount() == args)
        {
            matches++;
            if (func->isConst)
                consts++;
        }
    }

    // if there are multiple matches that are all const, return const
    if (matches > 0 && matches == consts)
        return true;

    // not found in this class
    if (!scope->derivedFrom.empty())
    {
        // check each base class
        for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i)
        {
            // find the base class
            const Scope *derivedFrom = scope->derivedFrom[i].scope;

            // find the function in the base class
            if (derivedFrom)
            {
                if (isConstMemberFunc(derivedFrom, tok))
                    return true;
            }
        }
    }

    return false;
}

bool CheckClass::checkConstFunc(const Scope *scope, const Token *tok)
{
    // if the function doesn't have any assignment nor function call,
    // it can be a const function..
    unsigned int indentlevel = 0;
    bool isconst = true;
    for (const Token *tok1 = tok; tok1; tok1 = tok1->next())
    {
        if (tok1->str() == "{")
            ++indentlevel;
        else if (tok1->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        // assignment.. = += |= ..
        else if (tok1->isAssignmentOp())
        {
            if (tok1->next()->str() == "this")
            {
                isconst = false;
                break;
            }
            else if (isMemberVar(scope, tok1->previous()))
            {
                isconst = false;
                break;
            }
        }

        // streaming: <<
        else if (tok1->str() == "<<" && isMemberVar(scope, tok1->previous()))
        {
            isconst = false;
            break;
        }
        else if (Token::simpleMatch(tok1->previous(), ") <<") &&
                 isMemberVar(scope, tok1->tokAt(-2)))
        {
            isconst = false;
            break;
        }

        // increment/decrement (member variable?)..
        else if (Token::Match(tok1, "++|--"))
        {
            // var++ and var--
            if (Token::Match(tok1->previous(), "%var%") &&
                tok1->previous()->str() != "return")
            {
                if (isMemberVar(scope, tok1->previous()))
                {
                    isconst = false;
                    break;
                }
            }

            // var[...]++ and var[...]--
            else if (tok1->previous()->str() == "]")
            {
                if (isMemberVar(scope, tok1->previous()->link()->previous()))
                {
                    isconst = false;
                    break;
                }
            }

            // ++var and --var
            else if (Token::Match(tok1->next(), "%var%"))
            {
                if (isMemberVar(scope, tok1->next()))
                {
                    isconst = false;
                    break;
                }
            }
        }

        // std::map variable member
        else if (Token::Match(tok1, "%var% [") && isMemberVar(scope, tok1))
        {
            const Variable *var = symbolDatabase->getVariableFromVarId(tok1->varId());

            if (var && (var->typeStartToken()->str() == "map" ||
                        Token::simpleMatch(var->typeStartToken(), "std :: map")))
            {
                isconst = false;
                break;
            }
        }

        // function call..
        else if (Token::Match(tok1, "%var% (") &&
                 !(Token::Match(tok1, "return|c_str|if|string") || tok1->isStandardType()))
        {
            if (!isConstMemberFunc(scope, tok1))
            {
                isconst = false;
                break;
            }
        }
        else if (Token::Match(tok1, "%var% < %any% > ("))
        {
            isconst = false;
            break;
        }
        else if (Token::Match(tok1, "%var% . size|empty ( )") && tok1->varId())
        {
            // assume all std::*::size() and std::*::empty() are const
            const Variable *var = symbolDatabase->getVariableFromVarId(tok1->varId());

            if (var && Token::simpleMatch(var->typeStartToken(), "std ::"))
                tok1 = tok1->tokAt(4);
        }

        // delete..
        else if (tok1->str() == "delete")
        {
            isconst = false;
            break;
        }
    }

    return isconst;
}

// check if this function is defined virtual in the base classes
bool CheckClass::isVirtualFunc(const Scope *scope, const Token *functionToken) const
{
    // check each base class
    for (unsigned int i = 0; i < scope->derivedFrom.size(); ++i)
    {
        // check if base class exists in database
        if (scope->derivedFrom[i].scope)
        {
            const Scope *derivedFrom = scope->derivedFrom[i].scope;

            std::list<Function>::const_iterator func;

            // check if function defined in base class
            for (func = derivedFrom->functionList.begin(); func != derivedFrom->functionList.end(); ++func)
            {
                if (func->isVirtual)
                {
                    const Token *tok = func->tokenDef;

                    if (tok->str() == functionToken->str())
                    {
                        const Token *temp1 = tok->previous();
                        const Token *temp2 = functionToken->previous();
                        bool returnMatch = true;

                        // check for matching return parameters
                        while (temp1->str() != "virtual")
                        {
                            if (temp1->str() != temp2->str())
                            {
                                returnMatch = false;
                                break;
                            }

                            temp1 = temp1->previous();
                            temp2 = temp2->previous();
                        }

                        // check for matching function parameters
                        if (returnMatch && symbolDatabase->argsMatch(scope, tok->tokAt(2), functionToken->tokAt(2), std::string(""), 0))
                        {
                            return true;
                        }
                    }
                }
            }

            if (!derivedFrom->derivedFrom.empty())
            {
                if (isVirtualFunc(derivedFrom, functionToken))
                    return true;
            }
        }
        else
        {
            // unable to find base class so assume it has a virtual function
            return true;
        }
    }

    return false;
}

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::information, "functionConst",
                "Technically the member function '" + classname + "::" + funcname + "' can be const.\n"
                "The member function '" + classname + "::" + funcname + "' can be made a const "
                "function. Making this function const function should not cause compiler errors. "
                "Even though the function can be made const function technically it may not make "
                "sense conceptually. Think about your design and task of the function first - is "
                "it a function that must not change object internal state?");
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    toks.push_back(tok2);
    reportError(toks, Severity::information, "functionConst",
                "Technically the member function '" + classname + "::" + funcname + "' can be const.\n"
                "The member function '" + classname + "::" + funcname + "' can be made a const "
                "function. Making this function const function should not cause compiler errors. "
                "Even though the function can be made const function technically it may not make "
                "sense conceptually. Think about your design and task of the function first - is "
                "it a function that must not change object internal state?");
}
