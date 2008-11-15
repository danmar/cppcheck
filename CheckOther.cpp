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
#include "CheckOther.h"

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

CheckOther::CheckOther( Tokenizer *tokenizer )
{
    _tokenizer = tokenizer;
}

CheckOther::~CheckOther()
{

}

void CheckOther::WarningOldStylePointerCast()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Old style pointer casting..
        if (!Match(tok, "( %type% * ) %var%"))
            continue;

        // Is "type" a class?
        const char *pattern[] = {"class","",NULL};
        pattern[1] = Tokenizer::getstr(tok, 1);
        if (!Tokenizer::findtoken(tokens, pattern))
            continue;

        std::ostringstream ostr;
        ostr << FileLine(tok, _tokenizer) << ": C-style pointer casting";
        ReportErr(ostr.str());
    }
}




//---------------------------------------------------------------------------
// Use standard function "isdigit" instead
//---------------------------------------------------------------------------

void CheckOther::WarningIsDigit()
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
            ostr << FileLine(tok, _tokenizer) << ": The condition can be simplified; use 'isdigit'";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Use standard function "isalpha" instead
//---------------------------------------------------------------------------

void CheckOther::WarningIsAlpha()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( ! Match(tok, "(") )
            continue;

        bool err = false;

        err |= Match(tok, "( %var% >= 'A' && %var% <= 'Z' )");
        err |= Match(tok, "( * %var% >= 'A' && * %var% <= 'Z' )");
        err |= Match(tok, "( ( %var% >= 'A' ) && ( %var% <= 'Z' ) )");
        err |= Match(tok, "( ( * %var% >= 'A' ) && ( * %var% <= 'Z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": The condition can be simplified; use 'isupper'";
            ReportErr(ostr.str());
            continue;
        }

        err = false;
        err |= Match(tok, "( %var% >= 'a' && %var% <= 'z' )");
        err |= Match(tok, "( * %var% >= 'a' && * %var% <= 'z' )");
        err |= Match(tok, "( ( %var% >= 'a' ) && ( %var% <= 'z' ) )");
        err |= Match(tok, "( ( * %var% >= 'a' ) && ( * %var% <= 'z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": The condition can be simplified; use 'islower'";
            ReportErr(ostr.str());
            continue;
        }

        err = false;
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
            ostr << FileLine(tok, _tokenizer) << ": The condition can be simplified; use 'isalpha'";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Redundant code..
//---------------------------------------------------------------------------

void CheckOther::WarningRedundantCode()
{

    // if (p) delete p
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (!Match(tok,"if"))
            continue;

        const char *varname1 = NULL;
        const TOKEN *tok2 = NULL;

        if (Match(tok,"if ( %var% )"))
        {
            varname1 = Tokenizer::getstr(tok, 2);
            tok2 = Tokenizer::gettok(tok, 4);
        }
        else if (Match(tok,"if ( %var% != NULL )"))
        {
            varname1 = Tokenizer::getstr(tok, 2);
            tok2 = Tokenizer::gettok(tok, 6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        if ( Match(tok2, "{") )
            tok2 = tok2->next;

        bool err = false;
        if (Match(tok2,"delete %var% ;"))
            err = (strcmp(Tokenizer::getstr(tok2,1),varname1)==0);
        else if (Match(tok2,"delete [ ] %var% ;"))
            err = (strcmp(Tokenizer::getstr(tok2,1),varname1)==0);
        else if (Match(tok2,"free ( %var% )"))
            err = (strcmp(Tokenizer::getstr(tok2,2),varname1)==0);
        else if (Match(tok2,"kfree ( %var% )"))
            err = (strcmp(Tokenizer::getstr(tok2,2),varname1)==0);

        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": Redundant condition. It is safe to deallocate a NULL pointer";
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

void CheckOther::WarningIf()
{

    // Search for 'if (condition);'
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (Match(tok,"if"))
        {
            int parlevel = 0;
            for (const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
            {
                if (Match(tok2,"("))
                    parlevel++;
                else if (Match(tok2,")"))
                {
                    parlevel--;
                    if (parlevel<=0)
                    {
                        if (strcmp(Tokenizer::getstr(tok2,1), ";") == 0 &&
                            strcmp(Tokenizer::getstr(tok2,2), "else") != 0)
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(tok, _tokenizer) << ": Found \"if (condition);\"";
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
        if ( ! Match(tok, "[;{}]") )
            continue;
        tok = tok->next;
        if ( ! tok )
            break;

        if (!Match(tok,"%var% = %var% ; if ( %var%"))
            continue;

        if ( strcmp(Tokenizer::getstr(tok, 9), ")") != 0 )
            continue;

        // var1 = var2 ; if ( var3 cond var4 )
        const char *var1 = Tokenizer::getstr(tok, 0);
        const char *var2 = Tokenizer::getstr(tok, 2);
        const char *var3 = Tokenizer::getstr(tok, 6);
        const char *cond = Tokenizer::getstr(tok, 7);
        const char *var4 = Tokenizer::getstr(tok, 8);

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
        ostr << FileLine(Tokenizer::gettok(tok,4), _tokenizer) << ": The condition is always ";
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

void CheckOther::InvalidFunctionUsage()
{
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if (!Match(tok, "strtol") && !Match(tok, "strtoul"))
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
        {
            if ( Match(tok2, "(") )
                parlevel++;
            else if (Match(tok2, ")"))
                parlevel--;
            else if (parlevel == 1 && Match(tok2, ","))
            {
                param++;
                if (param==3)
                {
                    if ( Match(tok2, ", %num% )") )
                    {
                        int radix = atoi(Tokenizer::getstr(tok2, 1));
                        if (!(radix==0 || (radix>=2 && radix<=36)))
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(tok2, _tokenizer) << ": Invalid radix in call to strtol or strtoul. Must be 0 or 2-36";
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

void CheckOther::CheckIfAssignment()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (Match(tok, "if ( %var% = %num% )") ||
            Match(tok, "if ( %var% = %str% )") ||
            Match(tok, "if ( %var% = %var% )") )
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": Possible bug. Should it be '==' instead of '='?";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckOther::CheckUnsignedDivision()
{
    // Check for "ivar / uvar" and "uvar / ivar"
    std::map<std::string, char> varsign;
    for ( TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( Match(tok, "[{};(,] %type% %var% [;=,)]") )
        {
            const char *type = Tokenizer::getstr(tok, 1);
            if (strcmp(type,"char")==0 || strcmp(type,"short")==0 || strcmp(type,"int")==0)
                varsign[Tokenizer::getstr(tok,2)] = 's';
        }

        else if ( Match(tok, "[{};(,] unsigned %type% %var% [;=,)]") )
            varsign[Tokenizer::getstr(tok,3)] = 'u';

        else if (!Match(tok,"[).]") && Match(tok->next, "%var% / %var%"))
        {
            const char *varname1 = Tokenizer::getstr(tok,1);
            const char *varname2 = Tokenizer::getstr(tok,3);
            char sign1 = varsign[varname1];
            char sign2 = varsign[varname2];

            if ( sign1 && sign2 && sign1 != sign2 )
            {
                // One of the operands are signed, the other is unsigned..
                std::ostringstream ostr;
                ostr << FileLine(tok->next, _tokenizer) << ": Warning: Division with signed and unsigned operators";
                ReportErr(ostr.str());
            }
        }

        else if (!Match(tok,"[).]") && Match(tok->next, "%var% / - %num%"))
        {
            const char *varname1 = Tokenizer::getstr(tok,1);
            char sign1 = varsign[varname1];
            if ( sign1 == 'u' )
            {
                std::ostringstream ostr;
                ostr << FileLine(tok->next, _tokenizer) << ": Unsigned division. The result will be wrong.";
                ReportErr(ostr.str());
            }
        }

        else if (Match(tok, "[([=*/+-] - %num% / %var%"))
        {
            const char *varname2 = Tokenizer::getstr(tok,4);
            char sign2 = varsign[varname2];
            if ( sign2 == 'u' )
            {
                std::ostringstream ostr;
                ostr << FileLine(tok->next, _tokenizer) << ": Unsigned division. The result will be wrong.";
                ReportErr(ostr.str());
            }
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------


void CheckOther::CheckVariableScope()
{
    // Walk through all tokens..
    bool func = false;
    int indentlevel = 0;
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        // Skip class and struct declarations..
        if ( Match(tok, "class") || Match(tok, "struct") )
        {
            for (const TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if ( Match(tok2, "{") )
                {
                    int _indentlevel = 0;
                    tok = tok2;
                    for (tok = tok2; tok; tok = tok->next)
                    {
                        if ( Match(tok, "{") )
                        {
                            _indentlevel++;
                        }
                        if ( Match(tok, "}") )
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
                if (Match(tok2, "[,);]"))
                {
                    break;
                }
            }
            if ( ! tok )
                break;
        }

        if ( Match(tok, "{") )
        {
            indentlevel++;
        }
        if ( Match(tok, "}") )
        {
            indentlevel--;
            if ( indentlevel == 0 )
                func = false;
        }
        if ( indentlevel == 0 && Match(tok, ") {") )
        {
            func = true;
        }
        if ( indentlevel > 0 && func && Match(tok, "[{};]") )
        {
            // First token of statement..
            const TOKEN *tok1 = tok->next;
            if ( ! tok1 )
                continue;

            if (Match(tok1,"return") ||
                Match(tok1,"delete") ||
                Match(tok1,"goto") ||
                Match(tok1,"else"))
                continue;

            // Variable declaration?
            if (Match(tok1, "%var% %var% ;") ||
                Match(tok1, "%var% %var% =") )
            {
                CheckVariableScope_LookupVar( tok1, Tokenizer::getstr(tok1, 1) );
            }
        }
    }

}
//---------------------------------------------------------------------------

void CheckOther::CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] )
{
    const TOKEN *tok = tok1;

    // Skip the variable declaration..
    while (tok && !Match(tok,";"))
        tok = tok->next;

    // Check if the variable is used in this indentlevel..
    bool used = false, used1 = false;
    int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;
    while ( indentlevel >= 0 && tok )
    {
        if ( Match(tok, "{") )
        {
            indentlevel++;
        }

        else if ( Match(tok, "}") )
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

        else if ( Match(tok, "(") )
        {
            parlevel++;
        }

        else if ( Match(tok, ")") )
        {
            parlevel--;
        }


        else if ( strcmp(Tokenizer::getstr(tok, 0), varname) == 0 )
        {
            if ( indentlevel == 0 || used1 )
                return;
            used = true;
        }

        else if ( indentlevel==0 )
        {
            if ( Match(tok,"for") || Match(tok,"while") )
                for_or_while = true;
            if ( parlevel == 0 && Match(tok, ";") )
                for_or_while = false;
        }

        tok = tok->next;
    }

    // Warning if "used" is true
    std::ostringstream errmsg;
    errmsg << FileLine(tok1, _tokenizer) << " The scope of the variable '" << varname << "' can be limited";
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------

void CheckOther::CheckConstantFunctionParameter()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( Match(tok,"[,(] const std :: %type% %var% [,)]") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok, _tokenizer) << " " << Tokenizer::getstr(tok,5) << " is passed by value, it could be passed by reference/pointer instead";
            ReportErr( errmsg.str() );
        }

        else if ( Match(tok,"[,(] const %type% %var% [,)]") )
        {
            // Check if type is a struct or class.
            const char *pattern[3] = {"class","type",0};
            pattern[1] = Tokenizer::getstr(tok, 2);
            if ( Tokenizer::findtoken(tokens, pattern) )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok, _tokenizer) << " " << Tokenizer::getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                ReportErr( errmsg.str() );
            }
            pattern[0] = "struct";
            if ( Tokenizer::findtoken(tokens, pattern) )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok, _tokenizer) << " " << Tokenizer::getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                ReportErr( errmsg.str() );
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------

void CheckOther::CheckStructMemberUsage()
{
    const char *structname = 0;

    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->FileIndex != 0 )
            continue;
        if ( Match(tok,"}") )
            structname = 0;
        if ( Match(tok, "struct %type% {") )
            structname = Tokenizer::getstr(tok, 1);

        if (structname && Match(tok, "[{;]"))
        {
            const char *varname = 0;
            if (Match(tok->next, "%type% %var% [;[]"))
                varname = Tokenizer::getstr( tok, 2 );
            else if (Match(tok->next, "%type% %type% %var% [;[]"))
                varname = Tokenizer::getstr( tok, 2 );
            else if (Match(tok->next, "%type% * %var% [;[]"))
                varname = Tokenizer::getstr( tok, 3 );
            else if (Match(tok->next, "%type% %type% * %var% [;[]"))
                varname = Tokenizer::getstr( tok, 4 );
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
                    if ( strcmp("=", Tokenizer::getstr(tok2,2)) == 0 )
                        continue;
                    used = true;
                    break;
                }
            }

            if ( ! used )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok, _tokenizer) << ": struct member '" << structname << "::" << varname << "' is never read";
                ReportErr(errmsg.str());
            }
        }
    }
}





//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::CheckCharVariable()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Declaring the variable..
        if ( Match(tok, "[{};(,] char %var% [;=,)]") )
        {
            const char *varname[2] = {0};
            varname[0] = Tokenizer::getstr(tok, 2);

            // Check usage of char variable..
            int indentlevel = 0;
            for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
            {
                if ( Match(tok2, "{") )
                    ++indentlevel;

                else if ( Match(tok2, "}") )
                {
                    --indentlevel;
                    if ( indentlevel <= 0 )
                        break;
                }

                if (!Match(tok2,".") && Match(tok2->next, "%var% [ %var1% ]", varname))
                {
                    std::ostringstream errmsg;
                    errmsg << FileLine(tok2->next, _tokenizer) << ": Warning - using char variable as array index";
                    ReportErr(errmsg.str());
                    break;
                }

                if ( Match(tok2, "%var% [&|] %var1%", varname) || Match(tok2, "%var1% [&|]", varname) )
                {
                    std::ostringstream errmsg;
                    errmsg << FileLine(tok2, _tokenizer) << ": Warning - using char variable in bit operation";
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

void CheckOther::CheckIncompleteStatement()
{
    int parlevel = 0;

    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( Match(tok, "(") )
            ++parlevel;
        else if ( Match(tok, ")") )
            --parlevel;

        if ( parlevel != 0 )
            continue;

        if ( !Match(tok,"#") && Match(tok->next,"; %str%") && !Match(Tokenizer::gettok(tok,3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok->next, _tokenizer) << ": Redundant code: Found a statement that begins with string constant";
            ReportErr(errmsg.str());
        }

        if ( !Match(tok,"#") && Match(tok->next,"; %num%") && !Match(Tokenizer::gettok(tok,3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << FileLine(tok->next, _tokenizer) << ": Redundant code: Found a statement that begins with numeric constant";
            ReportErr(errmsg.str());
        }
    }
}

