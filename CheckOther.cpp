//---------------------------------------------------------------------------
#include "CheckOther.h"
#include "tokenize.h"
#include "CommonCheck.h"
#include <list>
#include <map>
#include <sstream>
#include <stdlib.h>     // <- atoi
#include <cstring>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Warning on C-Style casts.. p = (kalle *)foo;
//---------------------------------------------------------------------------

void WarningOldStylePointerCast()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Old style pointer casting..
        if (!Match(tok, "( %type% * ) %var%"))
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
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        bool err = false;
        err |= Match(tok, "%var% >= '0' && %var% <= '9'");
        err |= Match(tok, "* %var% >= '0' && * %var% <= '9'");
        err |= Match(tok, "( %var% >= '0' ) && ( %var% <= '9' )");
        err |= Match(tok, "( * %var% >= '0' ) && ( * %var% <= '9' )");
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
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        bool err = false;

        if ( tok->str[0] != '(' )
            continue;

        err |= Match(tok, "( %var% >= 'A' && %var% <= 'Z' ) || ( %var% >= 'a' && %var% <= 'z' )");
        err |= Match(tok, "( %var% >= 'a' && %var% <= 'z' ) || ( %var% >= 'A' && %var% <= 'Z' )");
        err |= Match(tok, "( * %var% >= 'A' && * %var% <= 'Z' ) || ( * %var% >= 'a' && * %var% <= 'z' )");
        err |= Match(tok, "( * %var% >= 'a' && * %var% <= 'z' ) || ( * %var% >= 'A' && * %var% <= 'Z' )");
        err |= Match(tok, "( ( %var% >= 'A' ) && ( %var% <= 'Z' ) ) || ( ( %var% >= 'a' ) && ( %var% <= 'z' ) )");
        err |= Match(tok, "( ( %var% >= 'a' ) && ( %var% <= 'z' ) ) || ( ( %var% >= 'A' ) && ( %var% <= 'Z' ) )");
        err |= Match(tok, "( ( * %var% >= 'A' ) && ( * %var% <= 'Z' ) ) || ( ( * var >= 'a' ) && ( * %var% <= 'z' ) )");
        err |= Match(tok, "( ( * %var% >= 'a' ) && ( * %var% <= 'z' ) ) || ( ( * var >= 'A' ) && ( * %var% <= 'Z' ) )");
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
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if"))
            continue;

        const char *varname1 = NULL;
        const TOKEN *tok2 = NULL;

        if (Match(tok,"if ( %var% )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 4);
        }
        else if (Match(tok,"if ( %var% != NULL )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        if ( tok2->str[0] == '{' )
            tok2 = tok2->next;

        bool err = false;
        if (Match(tok2,"delete %var% ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (Match(tok2,"delete [ ] %var% ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (Match(tok2,"free ( %var% )"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (Match(tok2,"kfree ( %var% )"))
            err = (strcmp(getstr(tok2,2),varname1)==0);

        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Redundant condition. It is safe to deallocate a NULL pointer";
            ReportErr(ostr.str());
        }
    }


    // TODO: Redundant condition
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
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if")==0)
        {
            int parlevel = 0;
            for (const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
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
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Begin statement?
        if ( ! strchr(";{}", tok->str[0]) )
            continue;
        tok = tok->next;
        if ( ! tok )
            break;

        if (!Match(tok,"%var% = %var% ; if ( %var%"))
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
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( strcmp(tok->str, "strtol") && strcmp(tok->str, "strtoul") )
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
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
                    if ( Match(tok2, ", %num% )") )
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
// Assignment in condition
//---------------------------------------------------------------------------

void CheckIfAssignment()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (Match(tok, "if ( %var% = %num% )") ||
            Match(tok, "if ( %var% = %str% )") ||
            Match(tok, "if ( %var% = %var% )") )
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Possible bug. Should it be '==' instead of '='?";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckUnsignedDivision()
{
    // Check for "ivar / uvar" and "uvar / ivar"
    std::map<std::string, char> varsign;
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( Match(tok, "[{};(,] %type% %var% [;=,)]") )
        {
            const char *type = getstr(tok, 1);
            if (strcmp(type,"char")==0 || strcmp(type,"short")==0 || strcmp(type,"int")==0)
                varsign[getstr(tok,2)] = 's';
        }

        else if ( Match(tok, "[{};(,] unsigned %type% %var% [;=,)]") )
            varsign[getstr(tok,3)] = 'u';

        else if (!Match(tok,"[).]") && Match(tok->next, "%var% / %var%"))
        {
            const char *varname1 = getstr(tok,1);
            const char *varname2 = getstr(tok,3);
            char sign1 = varsign[varname1];
            char sign2 = varsign[varname2];

            if ( sign1 && sign2 && sign1 != sign2 )
            {
                // One of the operands are signed, the other is unsigned..
                std::ostringstream ostr;
                ostr << FileLine(tok->next) << ": Warning: Division with signed and unsigned operators";
                ReportErr(ostr.str());
            }
        }
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
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        // Skip class and struct declarations..
        if ( strcmp(tok->str, "class") == 0 || strcmp(tok->str, "struct") == 0 )
        {
            for (const TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if ( tok2->str[0] == '{' )
                {
                    int _indentlevel = 0;
                    tok = tok2;
                    for (tok = tok2; tok; tok = tok->next)
                    {
                        if ( tok->str[0] == '{' )
                        {
                            _indentlevel++;
                        }
                        if ( tok->str[0] == '}' )
                        {
                            _indentlevel--;
                            if ( _indentlevel <= 0 )
                            {
                                tok = tok->next;
                                break;
                            }
                        }
                    }
                    break;
                }
                if (strchr(",);", tok2->str[0]))
                {
                    break;
                }
            }
            if ( ! tok )
                break;
        }

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
        if ( indentlevel == 0 && Match(tok, ") {") )
        {
            func = true;
        }
        if ( indentlevel > 0 && func && strchr("{};", tok->str[0]) )
        {
            // First token of statement..
            const TOKEN *tok1 = tok->next;
            if ( ! tok1 )
                continue;

            if (strcmp(tok1->str,"return")==0 ||
                strcmp(tok1->str,"delete")==0 ||
                strcmp(tok1->str,"goto")==0 ||
                strcmp(tok1->str,"else")==0)
                continue;

            // Variable declaration?
            if (Match(tok1, "%var% %var% ;") ||
                Match(tok1, "%var% %var% =") )
            {
                CheckVariableScope_LookupVar( tok1, getstr(tok1, 1) );
            }
        }
    }

}
//---------------------------------------------------------------------------

static void CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] )
{
    const TOKEN *tok = tok1;

    // Skip the variable declaration..
    while ( tok->str[0] != ';' )
        tok = tok->next;

    // Check if the variable is used in this indentlevel..
    bool used = false, used1 = false;
    int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;
    while ( indentlevel >= 0 && tok )
    {
        if ( tok->str[0] == '{' )
        {
            indentlevel++;
        }

        else if ( tok->str[0] == '}' )
        {
            indentlevel--;
            if ( indentlevel == 0 )
            {
                if ( for_or_while && used )
                    return;
                used1 = used;
                used = false;
            }
        }

        else if ( tok->str[0] == '(' )
        {
            parlevel++;
        }

        else if ( tok->str[0] == ')' )
        {
            parlevel--;
        }


        else if ( strcmp(tok->str, varname) == 0 )
        {
            if ( indentlevel == 0 || used1 )
                return;
            used = true;
        }

        else if ( indentlevel==0 )
        {
            if ( strcmp(tok->str,"for")==0 || strcmp(tok->str,"while")==0 )
                for_or_while = true;
            if ( parlevel == 0 && tok->str[0] == ';' )
                for_or_while = false;
        }

        tok = tok->next;
    }

    // Warning if "used" is true
    std::ostringstream errmsg;
    errmsg << FileLine(tok1) << " The scope of the variable '" << varname << "' can be limited";
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------

void CheckConstantFunctionParameter()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( Match(tok,"[,(] const std :: %type% %var% [,)]") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok) << " " << getstr(tok,5) << " is passed by value, it could be passed by reference/pointer instead";
            ReportErr( errmsg.str() );
        }

        else if ( Match(tok,"[,(] const %type% %var% [,)]") )
        {
            // Check if type is a struct or class.
            const char *pattern[3] = {"class","type",0};
            pattern[1] = getstr(tok, 2);
            if ( findtoken(tokens, pattern) )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok) << " " << getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                ReportErr( errmsg.str() );
            }
            pattern[0] = "struct";
            if ( findtoken(tokens, pattern) )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok) << " " << getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                ReportErr( errmsg.str() );
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------

void CheckStructMemberUsage()
{
    const char *structname = 0;

    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->FileIndex != 0 )
            continue;
        if ( tok->str[0] == '}' )
            structname = 0;
        if ( Match(tok, "struct %type% {") )
            structname = getstr(tok, 1);

        if (structname && Match(tok, "[{;]"))
        {
            const char *varname = 0;
            if (Match(tok->next, "%type% %var% [;[]"))
                varname = getstr( tok, 2 );
            else if (Match(tok->next, "%type% %type% %var% [;[]"))
                varname = getstr( tok, 2 );
            else if (Match(tok->next, "%type% * %var% [;[]"))
                varname = getstr( tok, 3 );
            else if (Match(tok->next, "%type% %type% * %var% [;[]"))
                varname = getstr( tok, 4 );
            else
                continue;

            const char *varnames[2];
            varnames[0] = varname;
            varnames[1] = 0;
            bool used = false;
            for ( const TOKEN *tok2 = tokens; tok2; tok2 = tok2->next )
            {
                if ( tok->FileIndex != 0 )
                    continue;

                if (Match(tok2, ". %var%", varnames))
                {
                    if ( strcmp("=", getstr(tok2,2)) == 0 )
                        continue;
                    used = true;
                    break;
                }
            }

            if ( ! used )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok) << ": struct member '" << structname << "::" << varname << "' is never read";
                ReportErr(errmsg.str());
            }
        }
    }
}





//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckCharVariable()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Declaring the variable..
        if ( Match(tok, "[{};(,] char %var% [;=,)]") )
        {
            const char *varname[2] = {0};
            varname[0] = getstr(tok, 2);

            // Check usage of char variable..
            int indentlevel = 0;
            for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
            {
                if ( tok2->str[0] == '{' )
                    ++indentlevel;

                else if ( tok2->str[0] == '}' )
                {
                    --indentlevel;
                    if ( indentlevel <= 0 )
                        break;
                }

                else if ( Match(tok2, "%var% [ %var1% ]", varname) )
                {
                    std::ostringstream errmsg;
                    errmsg << FileLine(tok2) << ": Warning - using char variable as array index";
                    ReportErr(errmsg.str());
                    break;
                }

                else if ( Match(tok2, "[&|] %var1%", varname) || Match(tok2, "%var1% [&|]", varname) )
                {
                    std::ostringstream errmsg;
                    errmsg << FileLine(tok2) << ": Warning - using char variable in bit operation";
                    ReportErr(errmsg.str());
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Incomplete statement..
//---------------------------------------------------------------------------

void CheckIncompleteStatement()
{
    int parlevel = 0;

    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '(' )
            ++parlevel;
        else if ( tok->str[0] == ')' )
            --parlevel;

        if ( parlevel != 0 )
            continue;

        if ( Match(tok,"; %str%") && !Match(gettok(tok,2), ",") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok->next) << ": Redundant code: Found a statement that begins with string constant";
            ReportErr(errmsg.str());
        }

        if ( Match(tok,"; %num%") && !Match(gettok(tok,2), ",") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok->next) << ": Redundant code: Found a statement that begins with numeric constant";
            ReportErr(errmsg.str());
        }
    }
}

