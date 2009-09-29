/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

CheckClass::Var *CheckClass::getVarList(const Token *tok1, bool withClasses)
{
    // Get variable list..
    Var *varlist = NULL;
    unsigned int indentlevel = 0;
    for (const Token *tok = tok1; tok; tok = tok->next())
    {
        if (!tok->next())
            break;

        if (tok->str() == "{")
            ++indentlevel;
        else if (tok->str() == "}")
        {
            if (indentlevel <= 1)
                break;
            --indentlevel;
        }

        if (indentlevel != 1)
            continue;

        if (tok->str() == "__published:")
        {
            for (; tok; tok = tok->next())
            {
                if (tok->str() == "{")
                    ++indentlevel;
                else if (tok->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }
                if (indentlevel == 1 && Token::Match(tok->next(), "private:|protected:|public:"))
                    break;
            }
            if (tok)
                continue;
            else
                break;
        }

        // "private:" "public:" "protected:" etc
        const bool b((*tok->strAt(0) != ':') && strchr(tok->strAt(0), ':') != 0);

        // Search for start of statement..
        if (! Token::Match(tok, "[;{}]") && ! b)
            continue;

        // This is the start of a statement
        const Token *next = tok->next();
        const char *varname = 0;

        // If next token contains a ":".. it is not part of a variable declaration
        if (next->str().find(":") != std::string::npos)
            continue;

        // Variable declarations that start with "static" shall be ignored..
        if (next->str() == "static")
            continue;

        // Type definitions shall be ignored..
        if (next->str() == "typedef")
            continue;

        // Is it a variable declaration?
        if (Token::Match(next, "%type% %var% ;"))
        {
            if (withClasses)
                varname = next->strAt(1);
            else if (next->isStandardType())
                varname = next->strAt(1);
            else if (Token::findmatch(_tokenizer->tokens(), ("enum " + next->str()).c_str()))
                varname = next->strAt(1);
        }

        // Pointer?
        else if (Token::Match(next, "%type% * %var% ;"))
        {
            varname = next->strAt(2);
        }

        // Pointer?
        else if (Token::Match(next, "%type% %type% * %var% ;"))
        {
            varname = next->strAt(3);
        }

        else if (Token::Match(next, "%type% %var% [") && next->next()->str() != "operator")
        {
            varname = next->strAt(1);
        }

        // std::string..
        else if (withClasses && Token::Match(next, "std :: string %var% ;"))
        {
            varname = next->strAt(3);
        }

        // Container..
        else if (withClasses && Token::Match(next, "std :: %type% <"))
        {
            while (next && next->str() != ">")
                next = next->next();
            if (Token::Match(next, "> %var% ;"))
                varname = next->strAt(1);
        }

        // If the varname was set in one of the two if-block above, create a entry for this variable..
        if (varname)
        {
            Var *var = new Var(varname, false, varlist);
            varlist   = var;
        }
    }

    return varlist;
}
//---------------------------------------------------------------------------

void CheckClass::initVar(Var *varlist, const char varname[])
{
    for (Var *var = varlist; var; var = var->next)
    {
        if (strcmp(var->name, varname) == 0)
        {
            var->init = true;
            break;
        }
    }
}
//---------------------------------------------------------------------------

void CheckClass::initializeVarList(const Token *tok1, const Token *ftok, Var *varlist, const char classname[], std::list<std::string> &callstack)
{
    bool Assign = false;
    unsigned int indentlevel = 0;

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
                initVar(varlist, ftok->strAt(0));
            }

            Assign |= (ftok->str() == ":");
        }


        if (ftok->str() == "{")
        {
            ++indentlevel;
            Assign = false;
        }

        if (ftok->str() == "}")
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
            initVar(varlist, ftok->strAt(1));
        }

        // Before a new statement there is "[{};)=]" or "else"
        if (! Token::Match(ftok, "[{};)=]") && ftok->str() != "else")
            continue;

        // Using the operator= function to initialize all variables..
        if (Token::simpleMatch(ftok->next(), "* this = "))
        {
            for (Var *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        if (!Token::Match(ftok->next(), "%var%") && !Token::Match(ftok->next(), "this . %var%"))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next();

        // Skip "this->"
        if (Token::simpleMatch(ftok, "this ."))
            ftok = ftok->tokAt(2);

        // Skip "classname :: "
        if (Token::Match(ftok, "%var% ::"))
            ftok = ftok->tokAt(2);

        // Clearing all variables..
        if (Token::simpleMatch(ftok, "memset ( this ,"))
        {
            for (Var *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        // Calling member function?
        else if (Token::Match(ftok, "%var% ("))
        {
            // No recursive calls!
            if (std::find(callstack.begin(), callstack.end(), ftok->str()) == callstack.end())
            {
                callstack.push_back(ftok->str());
                int i = 0;
                const Token *ftok2 = Tokenizer::findClassFunction(tok1, classname, ftok->strAt(0), i);
                if (ftok2)
                {
                    initializeVarList(tok1, ftok2, varlist, classname, callstack);
                }
                else  // there is a called member function, but it is not defined where we can find it, so we assume it initializes everything
                {
                    for (Var *var = varlist; var; var = var->next)
                        var->init = true;
                    break;

                    // we don't report this, as somewhere along the line we hope that the class and member function
                    // are checked together. It is possible that this will not be the case (where there are enough
                    // nested functions defined in different files), but that isn't really likely.
                }
            }
        }

        // Assignment of member variable?
        else if (Token::Match(ftok, "%var% ="))
        {
            initVar(varlist, ftok->strAt(0));
        }

        // Assignment of array item of member variable?
        else if (Token::Match(ftok, "%var% [ %any% ] ="))
        {
            initVar(varlist, ftok->strAt(0));
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Token::Match(ftok, "%var% . clear|Clear ("))
        {
            initVar(varlist, ftok->strAt(0));
        }
    }
}






//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckClass::constructors()
{
    const char pattern_class[] = "class %var% [{:]";

    // Locate class
    const Token *tok1 = Token::findmatch(_tokenizer->tokens(), pattern_class);
    while (tok1)
    {
        const char *className[2];
        className[0] = tok1->strAt(1);
        className[1] = 0;
        const Token *classNameToken = tok1->tokAt(1);

        /** @todo handling of private constructors should be improved */
        bool hasPrivateConstructor = false;
        {
            int indentlevel = 0;
            bool isPrivate = true;
            for (const Token *tok = tok1; tok; tok = tok->next())
            {
                // Indentation
                if (tok->str() == "{")
                    ++indentlevel;

                else if (tok->str() == "}")
                {
                    --indentlevel;
                    if (indentlevel <= 0)
                        break;
                }

                // Parse class contents (indentlevel == 1)..
                if (indentlevel == 1)
                {
                    // What section are we in.. private/non-private
                    if (tok->str() == "private:")
                        isPrivate = true;
                    else if (tok->str() == "protected:" || tok->str() == "public:")
                        isPrivate = false;

                    // Is there a private constructor?
                    else if (isPrivate && Token::simpleMatch(tok, (classNameToken->str() + " (").c_str()))
                    {
                        hasPrivateConstructor = true;
                        break;
                    }
                }
            }
        }

        if (hasPrivateConstructor)
        {
            /** @todo Handle private constructors. Right now to avoid
             * false positives we just bail out */
            tok1 = Token::findmatch(tok1->next(), pattern_class);
            continue;
        }

        // Are there a class constructor?
        std::string tempPattern = "%any% " + classNameToken->str() + " (";
        const Token *constructor_token = Token::findmatch(tok1, tempPattern.c_str());
        while (constructor_token && constructor_token->str() == "~")
            constructor_token = Token::findmatch(constructor_token->next(), tempPattern.c_str());

        // There are no constructor.
        if (! constructor_token)
        {
            // If "--style" has been given, give a warning
            if (ErrorLogger::noConstructor(*_settings))
            {
                // If the class has member variables there should be an constructor
                Var *varlist = getVarList(tok1, false);
                if (varlist)
                {
                    noConstructorError(tok1, classNameToken->str());
                }
                // Delete the varlist..
                while (varlist)
                {
                    Var *nextvar = varlist->next;
                    delete varlist;
                    varlist = nextvar;
                }
            }

            tok1 = Token::findmatch(tok1->next(), pattern_class);
            continue;
        }

        // Check constructors
        checkConstructors(tok1, className[0]);

        // Check assignment operators
        checkConstructors(tok1, "operator =");

        tok1 = Token::findmatch(tok1->next(), pattern_class);
    }
}

void CheckClass::checkConstructors(const Token *tok1, const char funcname[])
{
    const char * const className = tok1->strAt(1);

    // Check that all member variables are initialized..
    bool withClasses = bool(_settings->_showAll && std::string(funcname) == "operator =");
    Var *varlist = getVarList(tok1, withClasses);

    int indentlevel = 0;
    const Token *constructor_token = Tokenizer::findClassFunction(tok1, className, funcname, indentlevel);
    std::list<std::string> callstack;
    initializeVarList(tok1, constructor_token, varlist, className, callstack);
    while (constructor_token)
    {
        // Check if any variables are uninitialized
        for (Var *var = varlist; var; var = var->next)
        {
            if (var->init)
                continue;

            // Is it a static member variable?
            std::ostringstream pattern;
            pattern << className << " :: " << var->name << " =";
            if (Token::findmatch(_tokenizer->tokens(), pattern.str().c_str()))
                continue;

            // It's non-static and it's not initialized => error
            if (Token::simpleMatch(constructor_token, "operator = (") ||
                Token::simpleMatch(constructor_token->tokAt(2), "operator = ("))
            {
                const Token *operStart = 0;
                if (Token::simpleMatch(constructor_token, "operator = ("))
                    operStart = constructor_token->tokAt(2);
                else
                    operStart = constructor_token->tokAt(4);

                bool classNameUsed = false;
                for (const Token *operTok = operStart; operTok != operStart->link(); operTok = operTok->next())
                {
                    if (operTok->str() == className)
                    {
                        classNameUsed = true;
                        break;
                    }
                }

                if (classNameUsed)
                    operatorEqVarError(constructor_token, className, var->name);
            }
            else
                uninitVarError(constructor_token, className, var->name);
        }

        for (Var *var = varlist; var; var = var->next)
            var->init = false;

        constructor_token = Tokenizer::findClassFunction(constructor_token->next(), className, funcname, indentlevel);
        callstack.clear();
        initializeVarList(tok1, constructor_token, varlist, className, callstack);
    }

    // Delete the varlist..
    while (varlist)
    {
        Var *nextvar = varlist->next;
        delete varlist;
        varlist = nextvar;
    }
}


//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckClass::privateFunctions()
{
    // Locate some class
    for (const Token *tok1 = Token::findmatch(_tokenizer->tokens(), "class %var% {"); tok1; tok1 = Token::findmatch(tok1->next(), "class %var% {"))
    {
        /** @todo check that the whole class implementation is seen */
        // until the todo above is fixed we only check classes that are
        // declared in the source file
        if (tok1->fileIndex() != 0)
            continue;

        const std::string &classname = tok1->next()->str();

        // Get private functions..
        std::list<const Token *> FuncList;
        FuncList.clear();
        bool priv = false;
        unsigned int indent_level = 0;
        for (const Token *tok = tok1; tok; tok = tok->next())
        {
            if (Token::Match(tok, "friend %var%"))
            {
                /** @todo Handle friend classes */
                FuncList.clear();
                break;
            }

            if (tok->str() == "{")
                ++indent_level;
            else if (tok->str() == "}")
            {
                if (indent_level <= 1)
                    break;
                --indent_level;
            }

            else if (indent_level != 1)
                continue;
            else if (tok->str() == "private:")
                priv = true;
            else if (tok->str() == "public:")
                priv = false;
            else if (tok->str() == "protected:")
                priv = false;
            else if (priv)
            {
                if (Token::Match(tok, "typedef %type% ("))
                    tok = tok->tokAt(2)->link();

                else if (Token::Match(tok, "[:,] %var% ("))
                    tok = tok->tokAt(2)->link();

                else if (Token::Match(tok, "%var% (") &&
                         !Token::Match(tok, classname.c_str()))
                {
                    FuncList.push_back(tok);
                }
            }

            /** @todo embedded class have access to private functions */
            if (tok->str() == "class")
            {
                FuncList.clear();
                break;
            }
        }

        // Check that all private functions are used..
        bool HasFuncImpl = false;
        bool inclass = false;
        indent_level = 0;
        const std::string pattern_function(classname + " ::");
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

            if (Token::Match(ftok, ("class " + classname + " :|{").c_str()))
            {
                indent_level = 0;
                inclass = true;
            }

            // Check member class functions to see what functions are used..
            if ((inclass && indent_level == 1 && Token::Match(ftok, ") const| {")) ||
                (Token::Match(ftok, (classname + " :: ~| %var% (").c_str())))
            {
                while (ftok && ftok->str() != ")")
                    ftok = ftok->next();
                if (!ftok)
                    break;
                if (Token::Match(ftok, ") : %var% ("))
                {
                    while (!Token::Match(ftok->next(), "[{};]"))
                        ftok = ftok->next();
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
                                it++;
                        }
                    }
                }
            }
        }

        while (HasFuncImpl && !FuncList.empty())
        {
            // Final check; check if the function pointer is used somewhere..
            const std::string _pattern("return|(|)|,|= " + FuncList.front()->str());
            if (!Token::findmatch(_tokenizer->tokens(), _pattern.c_str()))
            {
                unusedPrivateFunctionError(FuncList.front(), classname, FuncList.front()->str());
            }
            FuncList.pop_front();
        }
    }
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

        const char *type = NULL;
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
        if (!(type && type[0]))
            continue;

        // Warn if type is a class..
        const std::string pattern1(std::string("class ") + type);
        if (Token::findmatch(_tokenizer->tokens(), pattern1.c_str()))
        {
            memsetClassError(tok, tok->str());
            continue;
        }

        // Warn if type is a struct that contains any std::*
        const std::string pattern2(std::string("struct ") + type + " {");
        for (const Token *tstruct = Token::findmatch(_tokenizer->tokens(), pattern2.c_str()); tstruct; tstruct = tstruct->next())
        {
            if (tstruct->str() == "}")
                break;

            if (Token::Match(tstruct, "std :: %type% %var% ;"))
            {
                memsetStructError(tok, tok->str(), tstruct->strAt(2));
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// ClassCheck: "void operator=("
//---------------------------------------------------------------------------

void CheckClass::operatorEq()
{
    const Token *tok = Token::findmatch(_tokenizer->tokens(), "void operator = (");
    if (tok)
    {
        const Token *tok1 = tok;
        while (tok1 && !Token::Match(tok1, "class %var%"))
        {
            if (tok1->str() == "public:")
            {
                operatorEqReturnError(tok);
                break;
            }
            if (tok1->str() == "private:" || tok1->str() == "protected:")
                break;
            tok1 = tok1->previous();
        }
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// A destructor in a base class should be virtual
//---------------------------------------------------------------------------

void CheckClass::virtualDestructor()
{
    const char pattern_classdecl[] = "class %var% : %var%";

    const Token *derived = _tokenizer->tokens();
    while ((derived = Token::findmatch(derived, pattern_classdecl)) != NULL)
    {
        // Check that the derived class has a non empty destructor..
        {
            std::ostringstream destructorPattern;
            destructorPattern << "~ " << derived->strAt(1) << " ( ) {";
            const Token *derived_destructor = Token::findmatch(_tokenizer->tokens(), destructorPattern.str().c_str());

            // No destructor..
            if (! derived_destructor)
            {
                derived = derived->next();
                continue;
            }

            // Empty destructor..
            if (Token::Match(derived_destructor, "~ %var% ( ) { }"))
            {
                derived = derived->next();
                continue;
            }
        }

        const Token *derivedClass = derived->tokAt(1);

        // Iterate through each base class...
        derived = derived->tokAt(3);
        while (Token::Match(derived, "%var%"))
        {
            bool isPublic(derived->str() == "public");

            // What kind of inheritance is it.. public|protected|private
            if (Token::Match(derived, "public|protected|private"))
                derived = derived->next();

            // Name of base class..
            const char *baseName[2];
            baseName[0] = derived->strAt(0);
            baseName[1] = 0;

            // Update derived so it's ready for the next loop.
            do
            {
                if (derived->str() == "{")
                    break;

                if (derived->str() == ",")
                {
                    derived = derived->next();
                    break;
                }

                derived = derived->next();
            }
            while (derived);

            // If not public inheritance, skip checking of this base class..
            if (! isPublic)
                continue;

            // Find the destructor declaration for the base class.
            const Token *base = Token::findmatch(_tokenizer->tokens(), (std::string("%any% ~ ") + baseName[0] + " (").c_str());
            while (base && base->str() == "::")
                base = Token::findmatch(base->next(), (std::string("%any% ~ ") + baseName[0] + " (").c_str());

            const Token *reverseTok = base;
            while (Token::Match(base, "%var%") && base->str() != "virtual")
                base = base->previous();

            // Check that there is a destructor..
            if (! base)
            {
                // Is the class declaration available?
                base = Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName[0] + " {").c_str());
                if (base)
                {
                    virtualDestructorError(base, baseName[0], derivedClass->str());
                }

                continue;
            }

            // There is a destructor. Check that it's virtual..
            else if (base->str() == "virtual")
                continue;

            // TODO: This is just a temporary fix, better solution is needed.
            // Skip situations where base class has base classes of its own, because
            // some of the base classes might have virtual destructor.
            // Proper solution is to check all of the base classes. If base class is not
            // found or if one of the base classes has virtual destructor, error should not
            // be printed. See TODO test case "virtualDestructorInherited"
            if (!Token::findmatch(_tokenizer->tokens(), (std::string("class ") + baseName[0] + " {").c_str()))
                continue;

            // Make sure that the destructor is public (protected or private
            // would not compile if inheritance is used in a way that would
            // cause the bug we are trying to find here.)
            int indent = 0;
            while (reverseTok)
            {
                if (reverseTok->str() == "public:")
                {
                    virtualDestructorError(base, baseName[0], derivedClass->str());
                    break;
                }
                else if (reverseTok->str() == "protected:" ||
                         reverseTok->str() == "private:")
                {
                    // No bug, protected/private destructor is allowed
                    break;
                }
                else if (reverseTok->str() == "{")
                {
                    indent++;
                    if (indent >= 1)
                    {
                        // We have found the start of the class without any sign
                        // of "public :" so we can assume that the destructor is not
                        // public and there is no bug in the code we are checking.
                        break;
                    }
                }
                else if (reverseTok->str() == "}")
                    indent--;

                reverseTok = reverseTok->previous();
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckClass::thisSubtractionError(const Token *tok)
{
    reportError(tok, Severity::possibleStyle, "thisSubtraction", "Suspicious pointer subtraction");
}

void CheckClass::thisSubtraction()
{
    const Token *tok = Token::findmatch(_tokenizer->tokens(), "this - %var%");
    if (tok)
    {
        thisSubtractionError(tok);
    }
}




void CheckClass::noConstructorError(const Token *tok, const std::string &classname)
{
    reportError(tok, Severity::style, "noConstructor", "The class '" + classname + "' has no constructor. Member variables not initialized.");
}

void CheckClass::uninitVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::style, "uninitVar", "Member variable not initialized in the constructor '" + classname + "::" + varname + "'");
}

void CheckClass::operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname)
{
    reportError(tok, Severity::possibleStyle, "operatorEqVarError", "Member variable '" + classname + "::" + varname + "' is not assigned a value in '" + classname + "::operator=" + "'");
}

void CheckClass::unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname)
{
    reportError(tok, Severity::style, "unusedPrivateFunction", "Unused private function '" + classname + "::" + funcname + "'");
}

void CheckClass::memsetClassError(const Token *tok, const std::string &memfunc)
{
    reportError(tok, Severity::error, "memsetClass", "Using '" + memfunc + "' on class");
}

void CheckClass::memsetStructError(const Token *tok, const std::string &memfunc, const std::string &classname)
{
    reportError(tok, Severity::error, "memsetStruct", "Using '" + memfunc + "' on struct that contains a 'std::" + classname + "'");
}

void CheckClass::operatorEqReturnError(const Token *tok)
{
    reportError(tok, Severity::style, "operatorEq", "'operator=' should return something");
}

void CheckClass::virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived)
{
    reportError(tok, Severity::error, "virtualDestructor", "Class " + Base + " which is inherited by class " + Derived + " does not have a virtual destructor");
}


