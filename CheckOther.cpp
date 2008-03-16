//---------------------------------------------------------------------------
#include "CheckOther.h"
#include "tokenize.h"
#include "CommonCheck.h"
#include <list>
#include <sstream>
#include <stdlib.h>     // <- atoi
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Warning on C-Style casts.. p = (kalle *)foo;
//---------------------------------------------------------------------------

void WarningOldStylePointerCast()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Old style pointer casting..
        if (!match(tok, "( type * ) var"))
            continue;

        // Is "type" a class?
        const char *pattern[] = {"class","",NULL};
        pattern[1] = getstr(tok, 1);
        if (!findtoken(tokens, pattern))
            continue;

        std::ostringstream ostr;
        ostr << FileLine(tok) << ": C-style pointer casting";
        ReportErr(ostr.str());
    }
}




//---------------------------------------------------------------------------
// Use standard function "isdigit" instead
//---------------------------------------------------------------------------

void WarningIsDigit()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        bool err = false;
        err |= match(tok, "var >= '0' && var <= '9'");
        err |= match(tok, "* var >= '0' && * var <= '9'");
        err |= match(tok, "( var >= '0' ) && ( var <= '9' )");
        err |= match(tok, "( * var >= '0' ) && ( * var <= '9' )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": The condition can be simplified; use 'isdigit'";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Use standard function "isalpha" instead
//---------------------------------------------------------------------------

void WarningIsAlpha()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        bool err = false;

        if ( tok->str[0] != '(' )
            continue;

        err |= match(tok, "( var >= 'A' && var <= 'Z' ) || ( var >= 'a' && var <= 'z' )");
        err |= match(tok, "( var >= 'a' && var <= 'z' ) || ( var >= 'A' && var <= 'Z' )");
        err |= match(tok, "( * var >= 'A' && * var <= 'Z' ) || ( * var >= 'a' && * var <= 'z' )");
        err |= match(tok, "( * var >= 'a' && * var <= 'z' ) || ( * var >= 'A' && * var <= 'Z' )");
        err |= match(tok, "( ( var >= 'A' ) && ( var <= 'Z' ) ) || ( ( var >= 'a' ) && ( var <= 'z' ) )");
        err |= match(tok, "( ( var >= 'a' ) && ( var <= 'z' ) ) || ( ( var >= 'A' ) && ( var <= 'Z' ) )");
        err |= match(tok, "( ( * var >= 'A' ) && ( * var <= 'Z' ) ) || ( ( * var >= 'a' ) && ( * var <= 'z' ) )");
        err |= match(tok, "( ( * var >= 'a' ) && ( * var <= 'z' ) ) || ( ( * var >= 'A' ) && ( * var <= 'Z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": The condition can be simplified; use 'isalpha'";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Redundant code..
//---------------------------------------------------------------------------

void WarningRedundantCode()
{

    // if (p) delete p
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if"))
            continue;

        const char *varname1 = NULL;
        TOKEN *tok2 = NULL;

        if (match(tok,"if ( var )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 4);
        }
        else if (match(tok,"if ( var != NULL )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        bool err = false;
        if (match(tok2,"delete var ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (match(tok2,"{ delete var ; }"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"delete [ ] var ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (match(tok2,"{ delete [ ] var ; }"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"free ( var )"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"{ free ( var ) ; }"))
            err = (strcmp(getstr(tok2,3),varname1)==0);

        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Redundant condition. It is safe to deallocate a NULL pointer";
            ReportErr(ostr.str());
        }
    }


    // TODO
    // if (haystack.find(needle) != haystack.end())
    //    haystack.remove(needle);

}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// if (condition) ....
//---------------------------------------------------------------------------

void WarningIf()
{

    // Search for 'if (condition);'
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if")==0)
        {
            int parlevel = 0;
            for (TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
            {
                if (tok2->str[0]=='(')
                    parlevel++;
                else if (tok2->str[0]==')')
                {
                    parlevel--;
                    if (parlevel<=0)
                    {
                        if (strcmp(getstr(tok2,1), ";") == 0 &&
                            strcmp(getstr(tok2,2), "else") != 0)
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(tok) << ": Found \"if (condition);\"";
                            ReportErr(ostr.str());
                        }
                        break;
                    }
                }
            }
        }
    }

    // Search for 'a=b; if (a==b)'
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Begin statement?
        if ( ! strchr(";{}", tok->str[0]) )
            continue;
        tok = tok->next;
        if ( ! tok )
            break;

        if (!match(tok,"var = var ; if ( var"))
            continue;

        if ( strcmp(getstr(tok, 9), ")") != 0 )
            continue;

        // var1 = var2 ; if ( var3 cond var4 )
        const char *var1 = tok->str;
        const char *var2 = getstr(tok, 2);
        const char *var3 = getstr(tok, 6);
        const char *cond = getstr(tok, 7);
        const char *var4 = getstr(tok, 8);

        // Check that var3 is equal with either var1 or var2
        if (strcmp(var1,var3) && strcmp(var2,var3))
            continue;

        // Check that var4 is equal with either var1 or var2
        if (strcmp(var1,var4) && strcmp(var2,var4))
            continue;

        // Check that there is a condition..
        const char *p[6] = {"==","<=",">=","!=","<",">"};
        bool iscond = false;
        for (int i = 0; i < 6; i++)
            iscond |= (strcmp(cond, p[i]) == 0);
        if (!iscond)
            break;

        // we found the error. Report.
        std::ostringstream ostr;
        ostr << FileLine(gettok(tok,4)) << ": The condition is always ";
        for (int i = 0; i < 6; i++)
        {
            if (strcmp(cond, p[i]) == 0)
                ostr << (i < 3 ? "True" : "False");
        }
        ReportErr(ostr.str());
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// strtol(str, 0, radix)  <- radix must be 0 or 2-36
//---------------------------------------------------------------------------

void InvalidFunctionUsage()
{
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( strcmp(tok->str, "strtol") && strcmp(tok->str, "strtoul") )
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for ( TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
        {
            if ( tok2->str[0] == '(' )
                parlevel++;
            else if (tok2->str[0] == ')')
                parlevel--;
            else if (parlevel == 1 && tok2->str[0] == ',')
            {
                param++;
                if (param==3)
                {
                    if ( match(tok2, ", num )") )
                    {
                        int radix = atoi(tok2->next->str);
                        if (!(radix==0 || (radix>=2 && radix<=36)))
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(tok2) << ": Invalid radix in call to strtol or strtoul. Must be 0 or 2-36";
                            ReportErr(ostr.str());
                        }
                    }
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Dangerous usage of 'strtok'
//---------------------------------------------------------------------------

static TOKEN *GetFunction( TOKEN *content )
{
    TOKEN *func = NULL;

    int indentlevel = 0;
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
        {
            indentlevel--;
            if (indentlevel == 0)
                func = NULL;
        }

        else if (indentlevel == 0)
        {
            if (tok->str[0] == ';')
                func = NULL;

            else if ( match(tok, "var :: var (") )
                func = tok->next->next;

            else if ( match(tok, "type var (") )
                func = tok->next;
        }

        else if (indentlevel>0 && func)
        {
            if ( tok == content )
                return func;
        }
    }
    return NULL;
}

void WarningStrTok()
{
    std::list<TOKEN *> funclist;

    // Which functions contain the 'strtok'?
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"strtok")!=0)
            continue;

        TOKEN *func = GetFunction(tok);
        if (!func)
            continue;

        funclist.push_back( func );
    }

    // No functions in list => No errors
    if ( funclist.empty() )
        return;

    // Take closer look at the strtok usage.
    std::list<TOKEN *>::const_iterator it1;
    for (it1 = funclist.begin(); it1 != funclist.end(); it1++)
    {
        // Search this function to check that it doesn't call any other of
        // the functions in the funclist.
        int indentlevel = 0;
        for ( TOKEN *tok = *it1; tok; tok = tok->next )
        {
            if ( tok->str[0] == '{' )
                indentlevel++;

            else if ( tok->str[0] == '}' )
            {
                if ( indentlevel <= 1 )
                    break;
                indentlevel--;
            }

            else if ( indentlevel >= 1 )
            {
                // Only interested in function calls..
                if (!(IsName(tok->str) && strcmp(getstr(tok,1), "(") == 0))
                    continue;

                // Check if function name is in funclist..
                std::list<TOKEN *>::const_iterator it2;
                for (it2 = funclist.begin(); it2 != funclist.end(); it2++)
                {
                    if ( strcmp( tok->str, (*it2)->str ) )
                        continue;

                    std::ostringstream ostr;
                    ostr << FileLine(tok) << ": Possible bug. Both '" << (*it1)->str << "' and '" << (*it2)->str << "' uses strtok.";
                    ReportErr(ostr.str());
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Assignment in condition
//---------------------------------------------------------------------------

void CheckIfAssignment()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (match(tok,"if ( a = b )"))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Possible bug. Should it be '==' instead of '='?";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Check for case without break
//---------------------------------------------------------------------------

void CheckCaseWithoutBreak()
{
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( strcmp(tok->str,"case")!=0 )
            continue;

        // Found a case, check that there's a break..
        int indentlevel = 0;
        for (TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
        {
            if (tok2->str[0] == '{')
                indentlevel++;
            else if (tok2->str[0] == '}')
            {
                indentlevel--;
                if (indentlevel < 0)
                {
                    std::ostringstream ostr;
                    ostr << FileLine(tok) << ": 'case' without 'break'.";
                    ReportErr(ostr.str());
                }
            }
            if (indentlevel==0)
            {
                if (strcmp(tok2->str,"break")==0)
                    break;
                if (strcmp(tok2->str,"return")==0)
                    break;
                if (strcmp(tok2->str,"case")==0)
                {
                    std::ostringstream ostr;
                    ostr << FileLine(tok) << ": Found 'case' without 'break'.";
                    ReportErr(ostr.str());
                    break;
                }
            }
        }
    }

}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckUnsignedDivision()
{
    // TODO: Check that the scope is the same

    const char *pattern_declvar[] = { "unsigned", "int", "", NULL };
    TOKEN *declvar = findtoken(tokens, pattern_declvar);
    while (  declvar )
    {
        const char *varname = getstr( declvar, 2 );
        if ( ! IsName(varname) )
        {
            declvar = findtoken( declvar->next, pattern_declvar );
            continue;
        }

        const char *pattern_div1[] = { "/", "", NULL };
        pattern_div1[1] = varname;
        TOKEN *tokdiv = findtoken(declvar, pattern_div1);
        while ( tokdiv )
        {
            std::ostringstream ostr;
            ostr << FileLine(tokdiv) << ": If the result is negative it will be wrong because an operand is unsigned.";
            ReportErr(ostr.str());
            tokdiv = findtoken(tokdiv->next, pattern_div1);
        }

        const char *pattern_div2[] = { "", "", "/", NULL };
        pattern_div2[1] = varname;
        tokdiv = findtoken(declvar, pattern_div2);
        while ( tokdiv )
        {
            if (!IsNumber(getstr(tokdiv,3)) && tokdiv->str[0]!=')')    // The ')' may indicate a cast
            {
                std::ostringstream ostr;
                ostr << FileLine(tokdiv) << ": If the result is negative it will be wrong because an operand is unsigned.";
                ReportErr(ostr.str());
            }
            tokdiv = findtoken(tokdiv->next, pattern_div2);
        }

        declvar = findtoken(declvar->next, pattern_declvar);
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------

static void CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] );

void CheckVariableScope()
{
    // Walk through all tokens..
    bool func = false;
    int indentlevel = 0;
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
        {
            indentlevel++;
        }
        if ( tok->str[0] == '}' )
        {
            indentlevel--;
            if ( indentlevel == 0 )
                func = false;
        }
        if ( indentlevel == 0 && match(tok, ") {") )
        {
            func = true;
        }
        if ( indentlevel > 0 && func && strchr("{};", tok->str[0]) )
        {
            // Variable declaration?
            if (match(tok->next, "var var ;"))
                CheckVariableScope_LookupVar( tok->next, getstr(tok, 2) );

            // Variable declaration?
            else if (match(tok->next, "var var ="))
                CheckVariableScope_LookupVar( tok->next, getstr(tok, 2) );
        }
    }

}
//---------------------------------------------------------------------------

static void CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] )
{
    const TOKEN *tok = tok1;

    // Skip the variable declaration..
    tok = tok->next;
    while ( tok->str[0] != ';' )
        tok = tok->next;

    // Check if the variable is used in this indentlevel..
    bool used = false;
    int indentlevel = 0;
    while ( indentlevel >= 0 && tok )
    {
        if ( tok->str[0] == '{' )
        {
            indentlevel++;
        }

        else if ( tok->str[0] == '}' )
        {
            indentlevel--;
        }

        else if ( strcmp(tok->str, varname) == 0 )
        {
            if ( indentlevel == 0 )
                return;
            used = true;
        }

        tok = tok->next;
    }

    // Warning if "used" is true
    std::ostringstream errmsg;
    errmsg << FileLine(tok1) << " The scope of the variable '" << varname << "' can be limited";
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------



