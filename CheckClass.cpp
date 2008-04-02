//---------------------------------------------------------------------------
#include "CheckClass.h"
#include "tokenize.h"
#include "CommonCheck.h"
#include <locale>
#include <list>
#include <string>
#include <sstream>


#ifdef __BORLANDC__
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

static struct VAR *ClassChecking_GetVarList(const char classname[])
{
    // Locate class..
    const char *pattern[] = {"class","","{",0};
    pattern[1] = classname;
    const TOKEN *tok1 = findtoken(tokens, pattern);

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
                varname = getstr(next, 2);
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

static const TOKEN * FindClassFunction( const TOKEN *_tokens, const char classname[], const char funcname[], unsigned int &indentlevel )
{
    const char *_classname[2] = {0,0};
    const char *_funcname[2] = {0,0};
    _classname[0] = classname;
    _funcname[0] = funcname;


    while ( _tokens )
    {
        if ( indentlevel > 0 )
        {
            if ( _tokens->str[0] == '{' )
                indentlevel++;
            else if ( _tokens->str[0] == '}' )
                indentlevel--;
            else if ( indentlevel == 1 )
            {
                // Member function is implemented in the class declaration..
                if ( Match( _tokens, "%var1% (", _funcname ) )
                {
                    const TOKEN *tok2 = _tokens;
                    while ( tok2 && tok2->str[0] != '{' && tok2->str[0] != ';' )
                        tok2 = tok2->next;
                    if ( tok2 && tok2->str[0] == '{' )
                        return _tokens;
                }
            }
        }

        else if ( Match(_tokens, "class %var1% {", _classname) )
        {
            indentlevel = 1;
            _tokens = gettok( _tokens, 2 );
        }

        else if ( Match(_tokens, "%var1% :: %var2% (", _classname, _funcname) )
        {
            return _tokens;
        }

        _tokens = _tokens->next;
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

static void ClassChecking_VarList_Initialize(const TOKEN *ftok, struct VAR *varlist, const char classname[])
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

        if (indentlevel>=1 && Match(ftok, "[{};] %var%"))
        {
            ftok = ftok->next;

            // Clearing all variables..
            if (Match(ftok,"memset ( this ,"))
            {
                for (struct VAR *var = varlist; var; var = var->next)
                    var->init = true;
            }

            // Calling member function?
            else if (Match(ftok, "%var% ("))
            {
                unsigned int i = 0;
                const TOKEN *ftok2 = FindClassFunction( tokens, classname, ftok->str, i );
                ClassChecking_VarList_Initialize(ftok2, varlist, classname);
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
}






//---------------------------------------------------------------------------
// ClassCheck: Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckConstructors()
{
    // Locate class
    const char *pattern_classname[] = {"class","","{",NULL};
    const TOKEN *tok1 = findtoken(tokens, pattern_classname);
    while (tok1)
    {
        const char *classname = tok1->next->str;
        if ( ! IsName(classname) )
        {
            tok1 = findtoken( tok1->next, pattern_classname );
            continue;
        }

        // Are there a class constructor?
        const char *constructor_pattern[] = {"","clKalle","(",NULL};
        constructor_pattern[1] = classname;
        const TOKEN *constructor_token = findtoken( tokens, constructor_pattern );
        while ( constructor_token && constructor_token->str[0] == '~' )
            constructor_token = findtoken( constructor_token->next, constructor_pattern );
        if ( ! constructor_token )
        {
            // There's no class constructor
            if ( CheckCodingStyle )
            {
                // Check that all member variables are initialized..
                struct VAR *varlist = ClassChecking_GetVarList(classname);
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

            tok1 = findtoken( tok1->next, pattern_classname );
            continue;
        }

        // Check that all member variables are initialized..
        struct VAR *varlist = ClassChecking_GetVarList(classname);

        unsigned int indentlevel = 0;
        constructor_token = FindClassFunction( tokens, classname, classname, indentlevel );
        ClassChecking_VarList_Initialize(constructor_token, varlist, classname);
        while ( constructor_token )
        {
            // Check if any variables are uninitialized
            for (struct VAR *var = varlist; var; var = var->next)
            {
                // Is it a static member variable?
                const char *pattern[] = {"","::","","=",NULL};
                pattern[0] = classname;
                pattern[2] = var->name;
                if (findtoken(tokens, pattern))
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
            ClassChecking_VarList_Initialize(constructor_token, varlist, classname);
        }

        // Delete the varlist..
        while (varlist)
        {
            struct VAR *nextvar = varlist->next;
            delete varlist;
            varlist = nextvar;
        }

        tok1 = findtoken( tok1->next, pattern_classname );
    }
}



//---------------------------------------------------------------------------
// ClassCheck: Unused private functions
//---------------------------------------------------------------------------

void CheckUnusedPrivateFunctions()
{
    // Locate some class
    const char *pattern_class[] = {"class","","{",NULL};
    for (const TOKEN *tok1 = findtoken(tokens, pattern_class); tok1; tok1 = findtoken(tok1->next, pattern_class))
    {
        const char *classname = tok1->next->str;

        // The class implementation must be available..
        const char *pattern_classconstructor[] = {"","::","",NULL};
        pattern_classconstructor[0] = classname;
        pattern_classconstructor[2] = classname;
        if (!findtoken(tokens,pattern_classconstructor))
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
                if (std::isalpha(tok->str[0]) &&
                    tok->next->str[0]=='(' &&
                    strcmp(tok->str,classname) != 0)
                {
                    FuncList.push_back(tok->str);
                }
            }
        }

        // Check that all private functions are used..
        const char *pattern_function[] = {"","::",NULL};
        pattern_function[0] = classname;
        bool HasFuncImpl = false;
        for (const TOKEN *ftok = findtoken(tokens, pattern_function); ftok; ftok = findtoken(ftok->next,pattern_function))
        {
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

            if (ftok->str[0] == ';')
                continue;

            if (numpar != 0)
                continue;

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

        while (HasFuncImpl && !FuncList.empty())
        {
            bool fp = false;

            // Final check; check if the function pointer is used somewhere..
            const char *_pattern[] = {"=","",NULL};
            _pattern[1] = FuncList.front().c_str();
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = "(";
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ")";
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ",";
            fp |= (findtoken(tokens, _pattern) != NULL);

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
        if (strcmp(tok->str,"memset")!=0)
            continue;

        const char *type = NULL;
        if (Match(tok, "memset ( %var% , %num% , sizeof ( %type% ) )"))
            type = getstr(tok, 8);
        else if (Match(tok, "memset ( & %var% , %num% , sizeof ( %type% ) )"))
            type = getstr(tok, 9);
        else if (Match(tok, "memset ( %var% , %num% , sizeof ( struct %type% ) )"))
            type = getstr(tok, 9);
        else if (Match(tok, "memset ( & %var% , %num% , sizeof ( struct %type% ) )"))
            type = getstr(tok, 10);

        // No type defined => The tokens didn't match
        if (!(type && type[0]))
            continue;

        // Warn if type is a class..
        const char *pattern1[] = {"class","",NULL};
        pattern1[1] = type;
        if (strcmp("this",getstr(tok,2))==0 || findtoken(tokens,pattern1))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Using 'memset' on class.";
            ReportErr(ostr.str());
            continue;
        }

        // Warn if type is a struct that contains any std::*
        const char *pattern2[] = {"struct","","{",NULL};
        pattern2[1] = type;
        for (const TOKEN *tstruct = findtoken(tokens, pattern2); tstruct; tstruct = tstruct->next)
        {
            if (tstruct->str[0] == '}')
                break;

            if (Match(tstruct, "std :: %type% %var% ;"))
            {
                std::ostringstream ostr;
                ostr << FileLine(tok) << ": Using 'memset' on struct that contains a 'std::" << getstr(tstruct,2) << "'";
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
    if (const TOKEN *tok = findtoken(tokens,pattern))
    {
        std::ostringstream ostr;
        ostr << FileLine(tok) << ": 'operator=' should return something";
        ReportErr(ostr.str());
    }
}


