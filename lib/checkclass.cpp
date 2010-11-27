/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
    : Check(tokenizer, settings, errorLogger),
      symbolDatabase(NULL), ownSymbolDatabase(false)
{

}

CheckClass::~CheckClass()
{
    if (ownSymbolDatabase)
        delete symbolDatabase;
}

void CheckClass::createSymbolDatabase()
{
    // Multiple calls => bail out
    if (symbolDatabase)
        return;

    if (_tokenizer->_symbolDatabase)
        symbolDatabase = _tokenizer->_symbolDatabase;
    else
    {
        symbolDatabase = new SymbolDatabase(_tokenizer, _settings, _errorLogger);
        ownSymbolDatabase = true;
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        SymbolDatabase::SpaceInfo *info = *i;

        // only check classes and structures
        if (!info->isClassOrStruct())
            continue;

        // There are no constructors.
        if (info->numConstructors == 0)
        {
            // If there is a private variable, there should be a constructor..
            std::list<SymbolDatabase::Var>::const_iterator var;
            for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
            {
                if (var->access == SymbolDatabase::Private && !var->isClass && !var->isStatic)
                {
                    noConstructorError(info->classDef, info->className, info->classDef->str() == "struct");
                    break;
                }
            }
        }

        std::list<SymbolDatabase::Func>::const_iterator func;

        for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
        {
            if (!func->hasBody || !(func->type == SymbolDatabase::Func::Constructor || func->type == SymbolDatabase::Func::CopyConstructor || func->type == SymbolDatabase::Func::OperatorEqual))
                continue;

            // Mark all variables not used
            info->clearAllVar();

            std::list<std::string> callstack;
            info->initializeVarList(*func, callstack);

            // Check if any variables are uninitialized
            std::list<SymbolDatabase::Var>::const_iterator var;
            for (var = info->varlist.begin(); var != info->varlist.end(); ++var)
            {
                // skip classes for regular constructor
                if (var->isClass && func->type == SymbolDatabase::Func::Constructor)
                    continue;

                if (var->assign || var->init || var->isStatic)
                    continue;

                if (var->isConst && var->token->previous()->str() != "*")
                    continue;

                // It's non-static and it's not initialized => error
                if (func->type == SymbolDatabase::Func::OperatorEqual)
                {
                    const Token *operStart = 0;
                    if (func->token->str() == "=")
                        operStart = func->token->tokAt(1);
                    else
                        operStart = func->token->tokAt(3);

                    bool classNameUsed = false;
                    for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next())
                    {
                        if (operTok->str() == info->className)
                        {
                            classNameUsed = true;
                            break;
                        }
                    }

                    if (classNameUsed)
                        operatorEqVarError(func->token, info->className, var->token->str());
                }
                else if (func->access != SymbolDatabase::Private)
                    uninitVarError(func->token, info->className, var->token->str());
            }
        }
    }
}

void CheckClass::noConstructorError(const Token *tok, const std::string &classname, bool isStruct)
{
    // For performance reasons the constructor might be intentionally missing. Therefore this is not a "warning"
    reportError(tok, Severity::style, "noConstructor",
                "The " + std::string(isStruct ? "struct" : "class") + " '" + classname +
                "' does not have a constructor.\n"
                "The class 'classname' does not have a constructor but it has attributes. "
                "The attributes are not initialized which may cause bugs or undefined behavior.");
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::warning, "uninitVar", "Member variable not initialized in the constructor '" + classname + "::" + varname + "'");
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
    if (!_settings->_checkCodingStyle)
        return;

    // don't check code that contains templates. Templates that are
    // "unused" are removed from the code. #2067
    if (_tokenizer->codeWithTemplates())
        return;

    // dont check borland classes with properties..
    if (Token::findmatch(_tokenizer->tokens(), "; __property ;"))
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        SymbolDatabase::SpaceInfo *info = *i;

        // only check classes and structures
        if (!info->isClassOrStruct())
            continue;

        // dont check derived classes
        if (!info->derivedFrom.empty())
            continue;

        // Locate some class
        const Token *tok1 = info->classDef;

        // check that the whole class implementation is seen
        bool whole = true;
        std::list<SymbolDatabase::Func>::const_iterator func;
        for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
        {
            if (!func->hasBody)
            {
                // empty private copy constructors and assignment operators are OK
                if ((func->type == SymbolDatabase::Func::CopyConstructor || func->type == SymbolDatabase::Func::OperatorEqual) && func->access == SymbolDatabase::Private)
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
        if (!info->getNestedNonFunctions())
        {
            for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
            {
                // Get private functions..
                if (func->type == SymbolDatabase::Func::Function &&
                    func->access == SymbolDatabase::Private && func->hasBody)
                    FuncList.push_back(func->tokenDef);
            }
        }

        // Check that all private functions are used..
        bool HasFuncImpl = false;
        bool inclass = false;
        int indent_level = 0;
        for (const Token *ftok = _tokenizer->tokens(); ftok; ftok = ftok->next())
        {
            if (ftok->str() == "{")
                ++indent_level;
            else if (ftok->str() == "}")
            {
                if (indent_level > 0)
                    --indent_level;
                if (indent_level == 0)
                    inclass = false;
            }

            else if (ftok->str() == "class" &&
                     ftok->next()->str() == classname &&
                     Token::Match(ftok->tokAt(2), ":|{"))
            {
                indent_level = 0;
                inclass = true;
            }

            // Check member class functions to see what functions are used..
            else if ((inclass && indent_level == 1 && Token::Match(ftok, "%var% (")) ||
                     (ftok->str() == classname && Token::Match(ftok->next(), ":: ~| %var% (")))
            {
                while (ftok && ftok->str() != ")")
                    ftok = ftok->next();
                if (!ftok)
                    break;
                if (Token::Match(ftok, ") : %var% ("))
                {
                    while (!Token::Match(ftok->next(), "[{};]"))
                    {
                        if (Token::Match(ftok, "::|,|( %var% ,|)"))
                        {
                            // Remove function from FuncList
                            std::list<const Token *>::iterator it = FuncList.begin();
                            while (it != FuncList.end())
                            {
                                if (ftok->next()->str() == (*it)->str())
                                    FuncList.erase(it++);
                                else
                                    ++it;
                            }
                        }
                        ftok = ftok->next();
                    }
                }
                if (!Token::Match(ftok, ") const| {"))
                    continue;

                if (ftok->fileIndex() == 0)
                    HasFuncImpl = true;

                // Parse function..
                int indentlevel2 = 0;
                for (const Token *tok2 = ftok; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "{")
                        ++indentlevel2;
                    else if (tok2->str() == "}")
                    {
                        --indentlevel2;
                        if (indentlevel2 < 1)
                            break;
                    }
                    else if (Token::Match(tok2, "%var% ("))
                    {
                        // Remove function from FuncList
                        std::list<const Token *>::iterator it = FuncList.begin();
                        while (it != FuncList.end())
                        {
                            if (tok2->str() == (*it)->str())
                                FuncList.erase(it++);
                            else
                                ++it;
                        }
                    }
                }
            }
        }

        while (HasFuncImpl && !FuncList.empty())
        {
            // Final check; check if the function pointer is used somewhere..
            const std::string _pattern("return|(|)|,|= " + FuncList.front()->str());

            // or if the function address is used somewhere...
            // eg. sigc::mem_fun(this, &className::classFunction)
            const std::string _pattern2("& " + classname + " :: " + FuncList.front()->str());
            if (!Token::findmatch(_tokenizer->tokens(), _pattern.c_str()) &&
                !Token::findmatch(_tokenizer->tokens(), _pattern2.c_str()))
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
    // Locate all 'memset' tokens..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::Match(tok, "memset|memcpy|memmove"))
            continue;

        std::string type;
        if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( %type% ) )"))
            type = tok->strAt(8);
        else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( %type% ) )"))
            type = tok->strAt(9);
        else if (Token::Match(tok, "memset ( %var% , %num% , sizeof ( struct %type% ) )"))
            type = tok->strAt(9);
        else if (Token::Match(tok, "memset ( & %var% , %num% , sizeof ( struct %type% ) )"))
            type = tok->strAt(10);
        else if (Token::Match(tok, "%type% ( %var% , %var% , sizeof ( %type% ) )"))
            type = tok->strAt(8);

        // No type defined => The tokens didn't match
        if (type.empty())
            continue;

        // Warn if type is a class or struct that contains any std::* variables
        const std::string pattern2(std::string("struct|class ") + type + " {");
        for (const Token *tstruct = Token::findmatch(_tokenizer->tokens(), pattern2.c_str()); tstruct; tstruct = tstruct->next())
        {
            if (tstruct->str() == "}")
                break;

            // struct with function? skip function body..
            if (Token::simpleMatch(tstruct, ") {"))
            {
                tstruct = tstruct->next()->link();
                if (!tstruct)
                    break;
            }

            // before a statement there must be either:
            // * private:|protected:|public:
            // * { } ;
            if (Token::Match(tstruct, "[;{}]") ||
                tstruct->str().find(":") != std::string::npos)
            {
                if (Token::Match(tstruct->next(), "std :: %type% %var% ;"))
                    memsetStructError(tok, tok->str(), tstruct->strAt(3));

                else if (Token::Match(tstruct->next(), "std :: %type% < "))
                {
                    // backup the type
                    const std::string typestr(tstruct->strAt(3));

                    // check if it's a pointer variable..
                    unsigned int level = 0;
                    while (0 != (tstruct = tstruct->next()))
                    {
                        if (tstruct->str() == "<")
                            ++level;
                        else if (tstruct->str() == ">")
                        {
                            if (level <= 1)
                                break;
                            --level;
                        }
                        else if (tstruct->str() == "(")
                            tstruct = tstruct->link();
                    }

                    if (!tstruct)
                        break;

                    // found error => report
                    if (Token::Match(tstruct, "> %var% ;"))
                        memsetStructError(tok, tok->str(), typestr);
                }
            }
        }
    }
}

void CheckClass::memsetClassError(const Token *tok, const std::string &memfunc)
{
    reportError(tok, Severity::error, "memsetClass", "Using '" + memfunc + "' on class");
}

void CheckClass::memsetStructError(const Token *tok, const std::string &memfunc, const std::string &classname)
{
    reportError(tok, Severity::error, "memsetStruct", "Using '" + memfunc + "' on struct that contains a 'std::" + classname + "'");
}

//---------------------------------------------------------------------------
// ClassCheck: "void operator=(" and "const type & operator=("
//---------------------------------------------------------------------------

void CheckClass::operatorEq()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::const_iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        std::list<SymbolDatabase::Func>::const_iterator it;

        for (it = (*i)->functionList.begin(); it != (*i)->functionList.end(); ++it)
        {
            if (it->type == SymbolDatabase::Func::OperatorEqual && it->access != SymbolDatabase::Private)
            {
                if (it->token->strAt(-2) == "void")
                    operatorEqReturnError(it->token->tokAt(-2));
            }
        }
    }
}

void CheckClass::operatorEqReturnError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEq", "'operator=' should return something");
}

//---------------------------------------------------------------------------
// ClassCheck: "C& operator=(const C&) { ... return *this; }"
// operator= should return a reference to *this
//---------------------------------------------------------------------------

void CheckClass::checkReturnPtrThis(const SymbolDatabase::SpaceInfo *info, const SymbolDatabase::Func *func, const Token *tok, const Token *last)
{
    bool foundReturn = false;

    for (; tok && tok != last; tok = tok->next())
    {
        // check for return of reference to this
        if (tok->str() == "return")
        {
            foundReturn = true;
            std::string cast("( " + info->className + " & )");
            if (Token::Match(tok->next(), cast.c_str()))
                tok = tok->tokAt(4);

            // check if a function is called
            if (Token::Match(tok->tokAt(1), "%any% (") &&
                tok->tokAt(2)->link()->next()->str() == ";")
            {
                std::list<SymbolDatabase::Func>::const_iterator it;

                // check if it is a member function
                for (it = info->functionList.begin(); it != info->functionList.end(); ++it)
                {
                    // check for a regular function with the same name and a bofy
                    if (it->type == SymbolDatabase::Func::Function && it->hasBody &&
                        it->token->str() == tok->next()->str())
                    {
                        // check for the proper return type
                        if (it->tokenDef->previous()->str() == "&" &&
                            it->tokenDef->strAt(-2) == info->className)
                        {
                            // make sure it's not a const function
                            if (it->arg->link()->next()->str() != "const")
                                checkReturnPtrThis(info, &*it, it->arg->link()->next(), it->arg->link()->next()->link());
                        }
                    }
                }
            }

            // check of *this is returned
            else if (!(Token::Match(tok->tokAt(1), "(| * this ;|=") ||
                       Token::Match(tok->tokAt(1), "(| * this +=") ||
                       Token::Match(tok->tokAt(1), "operator = (")))
                operatorEqRetRefThisError(func->token);
        }
    }
    if (!foundReturn)
        operatorEqRetRefThisError(func->token);
}

void CheckClass::operatorEqRetRefThis()
{
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::const_iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        const SymbolDatabase::SpaceInfo *info = *i;

        // only check classes and structures
        if (info->isClassOrStruct())
        {
            std::list<SymbolDatabase::Func>::const_iterator func;

            for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
            {
                if (func->type == SymbolDatabase::Func::OperatorEqual && func->hasBody)
                {
                    // make sure return signature is correct
                    if (Token::Match(func->tokenDef->tokAt(-4), ";|}|{|public:|protected:|private: %type% &") &&
                        func->tokenDef->strAt(-3) == info->className)
                    {
                        // find the ')'
                        const Token *tok = func->token->next()->link();

                        checkReturnPtrThis(info, &(*func), tok->tokAt(2), tok->next()->link());
                    }
                }
            }
        }
    }
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
    if (!_settings->_checkCodingStyle)
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::const_iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        const SymbolDatabase::SpaceInfo *info = *i;
        std::list<SymbolDatabase::Func>::const_iterator it;

        // skip classes with multiple inheritance
        if (info->derivedFrom.size() > 1)
            continue;

        for (it = info->functionList.begin(); it != info->functionList.end(); ++it)
        {
            if (it->type == SymbolDatabase::Func::OperatorEqual && it->hasBody)
            {
                // make sure return signature is correct
                if (Token::Match(it->tokenDef->tokAt(-4), ";|}|{|public:|protected:|private: %type% &") &&
                    it->tokenDef->strAt(-3) == info->className)
                {
                    // check for proper function parameter signature
                    if ((Token::Match(it->tokenDef->next(), "( const %var% & )") ||
                         Token::Match(it->tokenDef->next(), "( const %var% & %var% )")) &&
                        it->tokenDef->strAt(3) == info->className)
                    {
                        // find the parameter name
                        const Token *rhs = it->token;
                        while (rhs->str() != "&")
                            rhs = rhs->next();
                        rhs = rhs->next();

                        // find the ')'
                        const Token *tok = it->token->next()->link();
                        const Token *tok1 = tok;

                        if (tok1 && tok1->tokAt(1) && tok1->tokAt(1)->str() == "{" && tok1->tokAt(1)->link())
                        {
                            const Token *first = tok1->tokAt(1);
                            const Token *last = first->link();

                            if (!hasAssignSelf(first, last, rhs))
                            {
                                if (hasDeallocation(first, last))
                                    operatorEqToSelfError(tok);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool CheckClass::hasDeallocation(const Token *first, const Token *last)
{
    // This function is called when no simple check was found for assignment
    // to self.  We are currently looking for a specific sequence of:
    // deallocate member ; ... member = allocate
    // This check is far from ideal because it can cause false negatives.
    // Unfortunately, this is necessary to prevent false positives.
    // This check needs to do careful analysis someday to get this
    // correct with a high degree of certainty.
    for (const Token *tok = first; tok && (tok != last); tok = tok->next())
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

bool CheckClass::hasAssignSelf(const Token *first, const Token *last, const Token *rhs)
{
    for (const Token *tok = first; tok && tok != last; tok = tok->next())
    {
        if (Token::Match(tok, "if ("))
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
    if (!_settings->inconclusive)
        return;

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::const_iterator i;

    for (i = symbolDatabase->spaceInfoList.begin(); i != symbolDatabase->spaceInfoList.end(); ++i)
    {
        const SymbolDatabase::SpaceInfo *info = *i;

        // Skip base classes and namespaces
        if (info->derivedFrom.empty())
            continue;

        // Find the destructor
        const SymbolDatabase::Func *destructor = info->getDestructor();

        // Check for destructor with implementation
        if (!destructor || !destructor->hasBody)
            continue;

        // Empty destructor
        if (destructor->token->tokAt(3)->link() == destructor->token->tokAt(4))
            continue;

        const Token *derived = info->classDef;
        const Token *derivedClass = derived->tokAt(1);

        // Iterate through each base class...
        for (unsigned int j = 0; j < info->derivedFrom.size(); ++j)
        {
            // Check if base class is public and exists in database
            if (info->derivedFrom[j].access == SymbolDatabase::Public && info->derivedFrom[j].spaceInfo)
            {
                const SymbolDatabase::SpaceInfo *spaceInfo = info->derivedFrom[j].spaceInfo;

                // Name of base class..
                const std::string baseName = spaceInfo->className;

                // Find the destructor declaration for the base class.
                const SymbolDatabase::Func *base_destructor = spaceInfo->getDestructor();
                const Token *base = 0;
                if (base_destructor)
                    base = base_destructor->token;

                // Check that there is a destructor..
                if (!base_destructor)
                {
                    if (spaceInfo->derivedFrom.empty())
                        virtualDestructorError(spaceInfo->classDef, baseName, derivedClass->str());
                }
                else if (!base_destructor->isVirtual)
                {
                    // TODO: This is just a temporary fix, better solution is needed.
                    // Skip situations where base class has base classes of its own, because
                    // some of the base classes might have virtual destructor.
                    // Proper solution is to check all of the base classes. If base class is not
                    // found or if one of the base classes has virtual destructor, error should not
                    // be printed. See TODO test case "virtualDestructorInherited"
                    if (spaceInfo->derivedFrom.empty())
                    {
                        // Make sure that the destructor is public (protected or private
                        // would not compile if inheritance is used in a way that would
                        // cause the bug we are trying to find here.)
                        if (base_destructor->access == SymbolDatabase::Public)
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
    if (!_settings->_checkCodingStyle)
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
    if (!_settings->_checkCodingStyle || _settings->ifcfg)
        return;

    // Don't check C# and JAVA classes
    if (_tokenizer->isJavaOrCSharp())
    {
        return;
    }

    createSymbolDatabase();

    std::list<SymbolDatabase::SpaceInfo *>::iterator it;

    for (it = symbolDatabase->spaceInfoList.begin(); it != symbolDatabase->spaceInfoList.end(); ++it)
    {
        SymbolDatabase::SpaceInfo *info = *it;

        // only check classes and structures
        if (!info->isClassOrStruct())
            continue;

        std::list<SymbolDatabase::Func>::const_iterator func;

        for (func = info->functionList.begin(); func != info->functionList.end(); ++func)
        {
            // does the function have a body?
            if (func->type == SymbolDatabase::Func::Function && func->hasBody && !func->isFriend && !func->isStatic && !func->isConst && !func->isVirtual)
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

                const Token *paramEnd = func->token->next()->link();

                // check if base class function is virtual
                if (!info->derivedFrom.empty())
                {
                    if (symbolDatabase->isVirtualFunc(info, func->tokenDef))
                        continue;
                }

                // if nothing non-const was found. write error..
                if (symbolDatabase->checkConstFunc(info, paramEnd))
                {
                    std::string classname = info->className;
                    SymbolDatabase::SpaceInfo *nest = info->nestedIn;
                    while (nest && nest->type != SymbolDatabase::SpaceInfo::Global)
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

void CheckClass::checkConstError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "functionConst", "The function '" + classname + "::" + funcname + "' can be const");
}

void CheckClass::checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname)
{
    std::list<const Token *> toks;
    toks.push_back(tok1);
    toks.push_back(tok2);
    reportError(toks, Severity::style, "functionConst", "The function '" + classname + "::" + funcname + "' can be const");
}
