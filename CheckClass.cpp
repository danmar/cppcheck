/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

//---------------------------------------------------------------------------
#include "CheckClass.h"
#include "tokenize.h"
#include "CommonCheck.h"
#include <locale>
#include <list>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>


#ifdef __BORLANDC__
#include <ctype.h>
#include <mem.h>
#endif
//---------------------------------------------------------------------------

extern bool CheckCodingStyle;

struct VAR
{
    const char *name;
    bool        init;
    struct VAR *next;
};
//---------------------------------------------------------------------------

static struct VAR *ClassChecking_GetVarList(const TOKEN *tok1)
{
    // Get variable list..
    struct VAR *varlist = NULL;
    unsigned int indentlevel = 0;
    for (const TOKEN *tok = tok1; tok; tok = tok->next)
    {
        if (!tok->next)
            break;

        if (tok->str[0] == '{')
            indentlevel++;
        if (tok->str[0] == '}')
        {
            if (indentlevel <= 1)
                break;
            indentlevel--;
        }


        if (indentlevel==1 && (strchr(";{}", tok->str[0]) || (tok->str[0]!=':' && strchr(tok->str, ':'))))
        {
            const TOKEN *next = tok->next;

            const char *varname = 0;

            // Is it a variable declaration?
            if ( Match(next,"%type% %var% ;") )
            {
                const char *types[] = {"bool", "char", "int", "short", "long", "float", "double", 0};
                for ( int type = 0; types[type]; type++ )
                {
                    if ( strcmp(next->str, types[type]) == 0)
                    {
                        varname = next->next->str;
                        break;
                    }
                }
            }

            // Pointer?
            else if ( Match(next, "%type% * %var% ;") )
            {
                varname = Tokenizer::getstr(next, 2);
            }

            if (varname)
            {
                struct VAR *var = new VAR;
                memset(var, 0, sizeof(struct VAR));
                var->name = varname;
                var->init = false;
                var->next = varlist;
                varlist   = var;
            }
        }
    }

    return varlist;
}
//---------------------------------------------------------------------------

static const TOKEN * FindClassFunction( const TOKEN *tok, const char classname[], const char funcname[], int &indentlevel )
{
    const char *_classname[2] = {0,0};
    const char *_funcname[2] = {0,0};
    _classname[0] = classname;
    _funcname[0] = funcname;

    if ( indentlevel < 0 || tok == NULL )
        return NULL;

    for ( ;tok; tok = tok->next )
    {
        if ( indentlevel == 0 &&
             ( Match(tok, "class %var1% {", _classname) ||
               Match(tok, "class %var1% : %type% {", _classname) ) )
        {
            if ( Match(tok, "class %var% {") )
                tok = Tokenizer::gettok(tok, 3);
            else
                tok = Tokenizer::gettok(tok, 5);
            indentlevel = 1;
        }

        if ( tok->str[0] == '{' )
        {
            // If indentlevel==0 don't go to indentlevel 1. Skip the block.
            if ( indentlevel > 0 )
                ++indentlevel;

            else
            {
                for ( ; tok; tok = tok->next )
                {
                    if ( tok->str[0] == '{' )
                        ++indentlevel;
                    else if ( tok->str[0] == '}' )
                    {
                        --indentlevel;
                        if ( indentlevel <= 0 )
                            break;
                    }
                }
                if ( tok == NULL )
                    return NULL;

                continue;
            }
        }

        if ( tok->str[0] == '}' )
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return NULL;
        }

        if ( indentlevel == 1 )
        {
            // Member function implemented in the class declaration?
            if ( Match( tok, "%var1% (", _funcname ) )
            {
                const TOKEN *tok2 = tok;
                while ( tok2 && tok2->str[0] != '{' && tok2->str[0] != ';' )
                    tok2 = tok2->next;
                if ( tok2 && tok2->str[0] == '{' )
                    return tok;
            }
        }

        else if ( indentlevel == 0 && Match(tok, "%var1% :: %var2% (", _classname, _funcname) )
        {
            return tok;
        }
    }

    // Not found
    return NULL;
}
//---------------------------------------------------------------------------

static void InitVar(struct VAR *varlist, const char varname[])
{
    for (struct VAR *var = varlist; var; var = var->next)
    {
        if ( strcmp(var->name, varname) == 0 )
        {
            var->init = true;
            break;
        }
    }
}
//---------------------------------------------------------------------------

static void ClassChecking_VarList_Initialize(const TOKEN *tok1, const TOKEN *ftok, struct VAR *varlist, const char classname[], std::list<std::string> &callstack)
{
    bool Assign = false;
    unsigned int indentlevel = 0;

    for (; ftok; ftok = ftok->next)
    {
        if (!ftok->next)
            break;

        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (indentlevel==0)
        {
            if (Assign && Match(ftok, "%var% ("))
            {
                InitVar( varlist, ftok->str );
            }

            Assign |= (ftok->str[0] == ':');
        }


        if (ftok->str[0] == '{')
        {
            indentlevel++;
            Assign = false;
        }

        if (ftok->str[0] == '}')
        {
            if (indentlevel <= 1)
                break;
            indentlevel--;
        }

        if ( indentlevel < 1 )
            continue;

        // Before a new statement there is "[{};)]" or "else"
        if ( ! Match(ftok, "[{};)]") && ! Match(ftok, "else") )
            continue;

        // Using the operator= function to initialize all variables..
        if ( Match(ftok->next, "* this = ") )
        {
            for (struct VAR *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        if (!Match(ftok->next, "%var%") && !Match(ftok->next, "this . %var%"))
            continue;

        // Goto the first token in this statement..
        ftok = ftok->next;

        // Skip "this->"
        if ( Match(ftok, "this .") )
            ftok = Tokenizer::gettok(ftok, 2);

        // Clearing all variables..
        if (Match(ftok,"memset ( this ,"))
        {
            for (struct VAR *var = varlist; var; var = var->next)
                var->init = true;
            break;
        }

        // Calling member function?
        else if (Match(ftok, "%var% ("))
        {
            // No recursive calls!
            if ( std::find(callstack.begin(),callstack.end(),ftok->str) == callstack.end() )
            {
                callstack.push_back( ftok->str );
                int i = 0;
                const TOKEN *ftok2 = FindClassFunction( tok1, classname, ftok->str, i );
                ClassChecking_VarList_Initialize(tok1, ftok2, varlist, classname, callstack);
            }
        }

        // Assignment of member variable?
        else if (Match(ftok, "%var% ="))
        {
            InitVar( varlist, ftok->str );
        }

        // The functions 'clear' and 'Clear' are supposed to initialize variable.
        if (Match(ftok,"%var% . clear (") || Match(ftok,"%var% . Clear ("))
        {
            InitVar( varlist, ftok->str );
        }
    }
}






//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckConstructors()
{
    // Locate class
    const char *pattern_classname[] = {"class","","{",NULL};
    const TOKEN *tok1 = Tokenizer::findtoken(tokens, pattern_classname);
    while (tok1)
    {
        const char *classname = tok1->next->str;
        if ( ! IsName(classname) )
        {
            tok1 = Tokenizer::findtoken( tok1->next, pattern_classname );
            continue;
        }

        // Are there a class constructor?
        const char *constructor_pattern[] = {"","clKalle","(",NULL};
        constructor_pattern[1] = classname;
        const TOKEN *constructor_token = Tokenizer::findtoken( tokens, constructor_pattern );
        while ( constructor_token && constructor_token->str[0] == '~' )
            constructor_token = Tokenizer::findtoken( constructor_token->next, constructor_pattern );
        if ( ! constructor_token )
        {
            // There's no class constructor
            if ( CheckCodingStyle )
            {
                // Check that all member variables are initialized..
                struct VAR *varlist = ClassChecking_GetVarList(tok1);
                if ( varlist )
                {
                    std::ostringstream ostr;
                    ostr << FileLine(tok1);
                    ostr << " The class '" << classname << "' has no constructor";
                    ReportErr(ostr.str());
                }
                // Delete the varlist..
                while (varlist)
                {
                    struct VAR *nextvar = varlist->next;
                    delete varlist;
                    varlist = nextvar;
                }
            }

            tok1 = Tokenizer::findtoken( tok1->next, pattern_classname );
            continue;
        }

        // Check that all member variables are initialized..
        struct VAR *varlist = ClassChecking_GetVarList(tok1);

        int indentlevel = 0;
        constructor_token = FindClassFunction( tok1, classname, classname, indentlevel );
        std::list<std::string> callstack;
        ClassChecking_VarList_Initialize(tok1, constructor_token, varlist, classname, callstack);
        while ( constructor_token )
        {
            // Check if any variables are uninitialized
            for (struct VAR *var = varlist; var; var = var->next)
            {
                // Is it a static member variable?
                const char *pattern[] = {"","::","","=",NULL};
                pattern[0] = classname;
                pattern[2] = var->name;
                if (Tokenizer::findtoken(tokens, pattern))
                    continue;

                if (!var->init)
                {
                    std::ostringstream ostr;
                    ostr << FileLine(constructor_token);
                    ostr << " Uninitialized member variable '" << classname << "::" << var->name << "'";
                    ReportErr(ostr.str());
                }
            }

            for ( struct VAR *var = varlist; var; var = var->next )
                var->init = false;

            constructor_token = FindClassFunction( constructor_token->next, classname, classname, indentlevel );
            ClassChecking_VarList_Initialize(tok1, constructor_token, varlist, classname, callstack);
        }

        // Delete the varlist..
        while (varlist)
        {
            struct VAR *nextvar = varlist->next;
            delete varlist;
            varlist = nextvar;
        }

        tok1 = Tokenizer::findtoken( tok1->next, pattern_classname );
    }
}



//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckUnusedPrivateFunctions()
{
    // Locate some class
    const char *pattern_class[] = {"class","","{",NULL};
    for (const TOKEN *tok1 = Tokenizer::findtoken(tokens, pattern_class); tok1; tok1 = Tokenizer::findtoken(tok1->next, pattern_class))
    {
        const char *classname = tok1->next->str;

        // The class implementation must be available..
        const char *pattern_classconstructor[] = {"","::","",NULL};
        pattern_classconstructor[0] = classname;
        pattern_classconstructor[2] = classname;
        if (!Tokenizer::findtoken(tokens,pattern_classconstructor))
            continue;

        // Get private functions..
        std::list<std::string> FuncList;
        FuncList.clear();
        bool priv = false;
        unsigned int indent_level = 0;
        for (const TOKEN *tok = tok1; tok; tok = tok->next)
        {
            if (Match(tok,"friend %var%"))
            {
                // Todo: Handle friend classes
                FuncList.clear();
                break;
            }

            if (tok->str[0] == '{')
                indent_level++;
            if (tok->str[0] == '}')
            {
                if (indent_level <= 1)
                    break;
                indent_level--;
            }
            if (strcmp(tok->str,"};") == 0)
                break;
            if (strcmp(tok->str,"private:") == 0)
                priv = true;
            else if (strcmp(tok->str,"public:") == 0)
                priv = false;
            else if (strcmp(tok->str,"protected:") == 0)
                priv = false;
            else if (priv && indent_level == 1)
            {
                if ( Match(tok, "typedef %type% (") )
                    tok = Tokenizer::gettok(tok, 2);

                if (Match(tok, "%var% (") &&
                    !Match(tok,classname))
                {
                    FuncList.push_back(tok->str);
                }
            }
        }

        // Check that all private functions are used..
        const char *pattern_function[] = {"","::",NULL};
        pattern_function[0] = classname;
        bool HasFuncImpl = false;
        const TOKEN *ftok = tokens;
        while (ftok)
        {
            ftok = Tokenizer::findtoken(ftok,pattern_function);
            int numpar = 0;
            while (ftok && ftok->str[0]!=';' && ftok->str[0]!='{')
            {
                if (ftok->str[0] == '(')
                    numpar++;
                else if (ftok->str[0] == ')')
                    numpar--;
                ftok = ftok->next;
            }

            if (!ftok)
                break;

            if (ftok->str[0] != ';' && numpar == 0)
            {
                HasFuncImpl = true;

                indent_level = 0;
                while (ftok)
                {
                    if (ftok->str[0] == '{')
                        indent_level++;
                    if (ftok->str[0] == '}')
                    {
                        if (indent_level<=1)
                            break;
                        indent_level--;
                    }
                    if (ftok->next && ftok->next->str[0] == '(')
                        FuncList.remove(ftok->str);
                    ftok = ftok->next;
                }
            }

            if (ftok)
                ftok = ftok->next;
        }

        while (HasFuncImpl && !FuncList.empty())
        {
            bool fp = false;

            // Final check; check if the function pointer is used somewhere..
            const char *_pattern[] = {"=","",NULL};
            _pattern[1] = FuncList.front().c_str();
            fp |= (Tokenizer::findtoken(tokens, _pattern) != NULL);
            _pattern[0] = "(";
            fp |= (Tokenizer::findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ")";
            fp |= (Tokenizer::findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ",";
            fp |= (Tokenizer::findtoken(tokens, _pattern) != NULL);

            if (!fp)
            {
                std::ostringstream ostr;
                ostr << "Class '" << classname << "', unused private function: '" << FuncList.front() << "'";
                ReportErr(ostr.str());
            }
            FuncList.pop_front();
        }
    }
}

//---------------------------------------------------------------------------
// ClassCheck: Check that memset is not used on classes
//---------------------------------------------------------------------------

void CheckMemset()
{
    // Locate all 'memset' tokens..
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (!Match(tok,"memset") && !Match(tok,"memcpy") && !Match(tok,"memmove"))
            continue;

        // Todo: Handle memcpy and memmove
        const char *type = NULL;
        if (Match(tok, "memset ( %var% , %num% , sizeof ( %type% ) )"))
            type = Tokenizer::getstr(tok, 8);
        else if (Match(tok, "memset ( & %var% , %num% , sizeof ( %type% ) )"))
            type = Tokenizer::getstr(tok, 9);
        else if (Match(tok, "memset ( %var% , %num% , sizeof ( struct %type% ) )"))
            type = Tokenizer::getstr(tok, 9);
        else if (Match(tok, "memset ( & %var% , %num% , sizeof ( struct %type% ) )"))
            type = Tokenizer::getstr(tok, 10);
        else if (Match(tok, "%type% ( %var% , %var% , sizeof ( %type% ) )"))
            type = Tokenizer::getstr(tok, 8);

        // No type defined => The tokens didn't match
        if (!(type && type[0]))
            continue;

        // Warn if type is a class..
        const char *pattern1[] = {"class","",NULL};
        pattern1[1] = type;
        if (Tokenizer::findtoken(tokens,pattern1))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Using '" << tok->str << "' on class.";
            ReportErr(ostr.str());
            continue;
        }

        // Warn if type is a struct that contains any std::*
        const char *pattern2[] = {"struct","","{",NULL};
        pattern2[1] = type;
        for (const TOKEN *tstruct = Tokenizer::findtoken(tokens, pattern2); tstruct; tstruct = tstruct->next)
        {
            if (tstruct->str[0] == '}')
                break;

            if (Match(tstruct, "std :: %type% %var% ;"))
            {
                std::ostringstream ostr;
                ostr << FileLine(tok) << ": Using '" << tok->str << "' on struct that contains a 'std::" << Tokenizer::getstr(tstruct,2) << "'";
                ReportErr(ostr.str());
                break;
            }
        }
    }
}




//---------------------------------------------------------------------------
// ClassCheck: "void operator=("
//---------------------------------------------------------------------------

void CheckOperatorEq1()
{
    const char *pattern[] = {"void", "operator", "=", "(", NULL};
    if (const TOKEN *tok = Tokenizer::findtoken(tokens,pattern))
    {
        std::ostringstream ostr;
        ostr << FileLine(tok) << ": 'operator=' should return something";
        ReportErr(ostr.str());
    }
}


