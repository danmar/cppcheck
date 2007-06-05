//---------------------------------------------------------------------------
#include "CheckOther.h"
#include "Tokenize.h"
#include "CommonCheck.h"
#include <sstream>
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
// if (condition);
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

