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

#include <list>
#include <map>
#include <sstream>
#include <stdlib.h>     // <- atoi
#include <cstring>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Warning on C-Style casts.. p = (kalle *)foo;
//---------------------------------------------------------------------------

CheckOther::CheckOther( const Tokenizer *tokenizer, ErrorLogger *errorLogger )
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckOther::~CheckOther()
{

}

void CheckOther::WarningOldStylePointerCast()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        // Old style pointer casting..
        if (!TOKEN::Match(tok, "( %type% * ) %var%"))
            continue;

        // Is "type" a class?
        const char *pattern[] = {"class","",NULL};
        pattern[1] = TOKEN::getstr(tok, 1);
        if (!TOKEN::findtoken(_tokenizer->tokens(), pattern))
            continue;

        std::ostringstream ostr;
        ostr << _tokenizer->fileLine(tok) << ": C-style pointer casting";
        _errorLogger->reportErr(ostr.str());
    }
}




//---------------------------------------------------------------------------
// Use standard function "isdigit" instead
//---------------------------------------------------------------------------

void CheckOther::WarningIsDigit()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        bool err = false;
        err |= TOKEN::Match(tok, "%var% >= '0' && %var% <= '9'");
        err |= TOKEN::Match(tok, "* %var% >= '0' && * %var% <= '9'");
        err |= TOKEN::Match(tok, "( %var% >= '0' ) && ( %var% <= '9' )");
        err |= TOKEN::Match(tok, "( * %var% >= '0' ) && ( * %var% <= '9' )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": The condition can be simplified; use 'isdigit'";
            _errorLogger->reportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Use standard function "isalpha" instead
//---------------------------------------------------------------------------

void CheckOther::WarningIsAlpha()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if ( ! TOKEN::Match(tok, "(") )
            continue;

        bool err = false;

        err |= TOKEN::Match(tok, "( %var% >= 'A' && %var% <= 'Z' )");
        err |= TOKEN::Match(tok, "( * %var% >= 'A' && * %var% <= 'Z' )");
        err |= TOKEN::Match(tok, "( ( %var% >= 'A' ) && ( %var% <= 'Z' ) )");
        err |= TOKEN::Match(tok, "( ( * %var% >= 'A' ) && ( * %var% <= 'Z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": The condition can be simplified; use 'isupper'";
            _errorLogger->reportErr(ostr.str());
            continue;
        }

        err = false;
        err |= TOKEN::Match(tok, "( %var% >= 'a' && %var% <= 'z' )");
        err |= TOKEN::Match(tok, "( * %var% >= 'a' && * %var% <= 'z' )");
        err |= TOKEN::Match(tok, "( ( %var% >= 'a' ) && ( %var% <= 'z' ) )");
        err |= TOKEN::Match(tok, "( ( * %var% >= 'a' ) && ( * %var% <= 'z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": The condition can be simplified; use 'islower'";
            _errorLogger->reportErr(ostr.str());
            continue;
        }

        err = false;
        err |= TOKEN::Match(tok, "( %var% >= 'A' && %var% <= 'Z' ) || ( %var% >= 'a' && %var% <= 'z' )");
        err |= TOKEN::Match(tok, "( %var% >= 'a' && %var% <= 'z' ) || ( %var% >= 'A' && %var% <= 'Z' )");
        err |= TOKEN::Match(tok, "( * %var% >= 'A' && * %var% <= 'Z' ) || ( * %var% >= 'a' && * %var% <= 'z' )");
        err |= TOKEN::Match(tok, "( * %var% >= 'a' && * %var% <= 'z' ) || ( * %var% >= 'A' && * %var% <= 'Z' )");
        err |= TOKEN::Match(tok, "( ( %var% >= 'A' ) && ( %var% <= 'Z' ) ) || ( ( %var% >= 'a' ) && ( %var% <= 'z' ) )");
        err |= TOKEN::Match(tok, "( ( %var% >= 'a' ) && ( %var% <= 'z' ) ) || ( ( %var% >= 'A' ) && ( %var% <= 'Z' ) )");
        err |= TOKEN::Match(tok, "( ( * %var% >= 'A' ) && ( * %var% <= 'Z' ) ) || ( ( * var >= 'a' ) && ( * %var% <= 'z' ) )");
        err |= TOKEN::Match(tok, "( ( * %var% >= 'a' ) && ( * %var% <= 'z' ) ) || ( ( * var >= 'A' ) && ( * %var% <= 'Z' ) )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": The condition can be simplified; use 'isalpha'";
            _errorLogger->reportErr(ostr.str());
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
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if (!TOKEN::Match(tok,"if"))
            continue;

        const char *varname1 = NULL;
        const TOKEN *tok2 = NULL;

        if (TOKEN::Match(tok,"if ( %var% )"))
        {
            varname1 = TOKEN::getstr(tok, 2);
            tok2 = tok->at(4);
        }
        else if (TOKEN::Match(tok,"if ( %var% != NULL )"))
        {
            varname1 = TOKEN::getstr(tok, 2);
            tok2 = tok->at(6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        if ( TOKEN::Match(tok2, "{") )
            tok2 = tok2->next;

        bool err = false;
        if (TOKEN::Match(tok2,"delete %var% ;"))
            err = (strcmp(TOKEN::getstr(tok2,1),varname1)==0);
        else if (TOKEN::Match(tok2,"delete [ ] %var% ;"))
            err = (strcmp(TOKEN::getstr(tok2,1),varname1)==0);
        else if (TOKEN::Match(tok2,"free ( %var% )"))
            err = (strcmp(TOKEN::getstr(tok2,2),varname1)==0);
        else if (TOKEN::Match(tok2,"kfree ( %var% )"))
            err = (strcmp(TOKEN::getstr(tok2,2),varname1)==0);

        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Redundant condition. It is safe to deallocate a NULL pointer";
            _errorLogger->reportErr(ostr.str());
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
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if (TOKEN::Match(tok,"if"))
        {
            int parlevel = 0;
            for (const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
            {
                if (TOKEN::Match(tok2,"("))
                    parlevel++;
                else if (TOKEN::Match(tok2,")"))
                {
                    parlevel--;
                    if (parlevel<=0)
                    {
                        if (strcmp(TOKEN::getstr(tok2,1), ";") == 0 &&
                            strcmp(TOKEN::getstr(tok2,2), "else") != 0)
                        {
                            std::ostringstream ostr;
                            ostr << _tokenizer->fileLine(tok) << ": Found \"if (condition);\"";
                            _errorLogger->reportErr(ostr.str());
                        }
                        break;
                    }
                }
            }
        }
    }

    // Search for 'a=b; if (a==b)'
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        // Begin statement?
        if ( ! TOKEN::Match(tok, "[;{}]") )
            continue;
        tok = tok->next;
        if ( ! tok )
            break;

        if (!TOKEN::Match(tok,"%var% = %var% ; if ( %var%"))
            continue;

        if ( strcmp(TOKEN::getstr(tok, 9), ")") != 0 )
            continue;

        // var1 = var2 ; if ( var3 cond var4 )
        const char *var1 = TOKEN::getstr(tok, 0);
        const char *var2 = TOKEN::getstr(tok, 2);
        const char *var3 = TOKEN::getstr(tok, 6);
        const char *cond = TOKEN::getstr(tok, 7);
        const char *var4 = TOKEN::getstr(tok, 8);

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
        ostr << _tokenizer->fileLine(tok->at(4)) << ": The condition is always ";
        for (int i = 0; i < 6; i++)
        {
            if (strcmp(cond, p[i]) == 0)
                ostr << (i < 3 ? "True" : "False");
        }
        _errorLogger->reportErr(ostr.str());
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// strtol(str, 0, radix)  <- radix must be 0 or 2-36
//---------------------------------------------------------------------------

void CheckOther::InvalidFunctionUsage()
{
    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next )
    {
        if (!TOKEN::Match(tok, "strtol") && !TOKEN::Match(tok, "strtoul"))
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
        {
            if ( TOKEN::Match(tok2, "(") )
                parlevel++;
            else if (TOKEN::Match(tok2, ")"))
                parlevel--;
            else if (parlevel == 1 && TOKEN::Match(tok2, ","))
            {
                param++;
                if (param==3)
                {
                    if ( TOKEN::Match(tok2, ", %num% )") )
                    {
                        int radix = atoi(TOKEN::getstr(tok2, 1));
                        if (!(radix==0 || (radix>=2 && radix<=36)))
                        {
                            std::ostringstream ostr;
                            ostr << _tokenizer->fileLine(tok2) << ": Invalid radix in call to strtol or strtoul. Must be 0 or 2-36";
                            _errorLogger->reportErr(ostr.str());
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
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if (TOKEN::Match(tok, "if ( %var% = %num% )") ||
            TOKEN::Match(tok, "if ( %var% = %str% )") ||
            TOKEN::Match(tok, "if ( %var% = %var% )") )
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Possible bug. Should it be '==' instead of '='?";
            _errorLogger->reportErr(ostr.str());
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
    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next )
    {
        if ( TOKEN::Match(tok, "[{};(,] %type% %var% [;=,)]") )
        {
            const char *type = TOKEN::getstr(tok, 1);
            if (strcmp(type,"char")==0 || strcmp(type,"short")==0 || strcmp(type,"int")==0)
                varsign[TOKEN::getstr(tok,2)] = 's';
        }

        else if ( TOKEN::Match(tok, "[{};(,] unsigned %type% %var% [;=,)]") )
            varsign[TOKEN::getstr(tok,3)] = 'u';

        else if (!TOKEN::Match(tok,"[).]") && TOKEN::Match(tok->next, "%var% / %var%"))
        {
            const char *varname1 = TOKEN::getstr(tok,1);
            const char *varname2 = TOKEN::getstr(tok,3);
            char sign1 = varsign[varname1];
            char sign2 = varsign[varname2];

            if ( sign1 && sign2 && sign1 != sign2 )
            {
                // One of the operands are signed, the other is unsigned..
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next) << ": Warning: Division with signed and unsigned operators";
                _errorLogger->reportErr(ostr.str());
            }
        }

        else if (!TOKEN::Match(tok,"[).]") && TOKEN::Match(tok->next, "%var% / - %num%"))
        {
            const char *varname1 = TOKEN::getstr(tok,1);
            char sign1 = varsign[varname1];
            if ( sign1 == 'u' )
            {
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next) << ": Unsigned division. The result will be wrong.";
                _errorLogger->reportErr(ostr.str());
            }
        }

        else if (TOKEN::Match(tok, "[([=*/+-] - %num% / %var%"))
        {
            const char *varname2 = TOKEN::getstr(tok,4);
            char sign2 = varsign[varname2];
            if ( sign2 == 'u' )
            {
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next) << ": Unsigned division. The result will be wrong.";
                _errorLogger->reportErr(ostr.str());
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
    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next )
    {
        // Skip class and struct declarations..
        if ( TOKEN::Match(tok, "class") || TOKEN::Match(tok, "struct") )
        {
            for (const TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if ( TOKEN::Match(tok2, "{") )
                {
                    int _indentlevel = 0;
                    tok = tok2;
                    for (tok = tok2; tok; tok = tok->next)
                    {
                        if ( TOKEN::Match(tok, "{") )
                        {
                            _indentlevel++;
                        }
                        if ( TOKEN::Match(tok, "}") )
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
                if (TOKEN::Match(tok2, "[,);]"))
                {
                    break;
                }
            }
            if ( ! tok )
                break;
        }

        if ( TOKEN::Match(tok, "{") )
        {
            indentlevel++;
        }
        if ( TOKEN::Match(tok, "}") )
        {
            indentlevel--;
            if ( indentlevel == 0 )
                func = false;
        }
        if ( indentlevel == 0 && TOKEN::Match(tok, ") {") )
        {
            func = true;
        }
        if ( indentlevel > 0 && func && TOKEN::Match(tok, "[{};]") )
        {
            // First token of statement..
            const TOKEN *tok1 = tok->next;
            if ( ! tok1 )
                continue;

            if (TOKEN::Match(tok1,"return") ||
                TOKEN::Match(tok1,"delete") ||
                TOKEN::Match(tok1,"goto") ||
                TOKEN::Match(tok1,"else"))
                continue;

            // Variable declaration?
            if (TOKEN::Match(tok1, "%var% %var% ;") ||
                TOKEN::Match(tok1, "%var% %var% =") )
            {
                CheckVariableScope_LookupVar( tok1, TOKEN::getstr(tok1, 1) );
            }
        }
    }

}
//---------------------------------------------------------------------------

void CheckOther::CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] )
{
    const TOKEN *tok = tok1;

    // Skip the variable declaration..
    while (tok && !TOKEN::Match(tok,";"))
        tok = tok->next;

    // Check if the variable is used in this indentlevel..
    bool used = false, used1 = false;
    int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;
    while ( indentlevel >= 0 && tok )
    {
        if ( TOKEN::Match(tok, "{") )
        {
            indentlevel++;
        }

        else if ( TOKEN::Match(tok, "}") )
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

        else if ( TOKEN::Match(tok, "(") )
        {
            parlevel++;
        }

        else if ( TOKEN::Match(tok, ")") )
        {
            parlevel--;
        }


        else if ( strcmp(TOKEN::getstr(tok, 0), varname) == 0 )
        {
            if ( indentlevel == 0 || used1 )
                return;
            used = true;
        }

        else if ( indentlevel==0 )
        {
            if ( TOKEN::Match(tok,"for") || TOKEN::Match(tok,"while") )
                for_or_while = true;
            if ( parlevel == 0 && TOKEN::Match(tok, ";") )
                for_or_while = false;
        }

        tok = tok->next;
    }

    // Warning if "used" is true
    std::ostringstream errmsg;
    errmsg << _tokenizer->fileLine(tok1) << " The scope of the variable '" << varname << "' can be limited";
    _errorLogger->reportErr( errmsg.str() );
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------

void CheckOther::CheckConstantFunctionParameter()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if ( TOKEN::Match(tok,"[,(] const std :: %type% %var% [,)]") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok) << " " << TOKEN::getstr(tok,5) << " is passed by value, it could be passed by reference/pointer instead";
            _errorLogger->reportErr( errmsg.str() );
        }

        else if ( TOKEN::Match(tok,"[,(] const %type% %var% [,)]") )
        {
            // Check if type is a struct or class.
            const char *pattern[3] = {"class","type",0};
            pattern[1] = TOKEN::getstr(tok, 2);
            if ( TOKEN::findtoken(_tokenizer->tokens(), pattern) )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok) << " " << TOKEN::getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                _errorLogger->reportErr( errmsg.str() );
            }
            pattern[0] = "struct";
            if ( TOKEN::findtoken(_tokenizer->tokens(), pattern) )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok) << " " << TOKEN::getstr(tok,3) << " is passed by value, it could be passed by reference/pointer instead";
                _errorLogger->reportErr( errmsg.str() );
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

    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next )
    {
        if ( tok->FileIndex != 0 )
            continue;
        if ( TOKEN::Match(tok,"}") )
            structname = 0;
        if ( TOKEN::Match(tok, "struct %type% {") )
            structname = TOKEN::getstr(tok, 1);

        if (structname && TOKEN::Match(tok, "[{;]"))
        {
            const char *varname = 0;
            if (TOKEN::Match(tok->next, "%type% %var% [;[]"))
                varname = TOKEN::getstr( tok, 2 );
            else if (TOKEN::Match(tok->next, "%type% %type% %var% [;[]"))
                varname = TOKEN::getstr( tok, 2 );
            else if (TOKEN::Match(tok->next, "%type% * %var% [;[]"))
                varname = TOKEN::getstr( tok, 3 );
            else if (TOKEN::Match(tok->next, "%type% %type% * %var% [;[]"))
                varname = TOKEN::getstr( tok, 4 );
            else
                continue;

            const char *varnames[2];
            varnames[0] = varname;
            varnames[1] = 0;
            bool used = false;
            for ( const TOKEN *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next )
            {
                if ( tok->FileIndex != 0 )
                    continue;

                if (TOKEN::Match(tok2, ". %var%", varnames))
                {
                    if ( strcmp("=", TOKEN::getstr(tok2,2)) == 0 )
                        continue;
                    used = true;
                    break;
                }
            }

            if ( ! used )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok) << ": struct member '" << structname << "::" << varname << "' is never read";
                _errorLogger->reportErr(errmsg.str());
            }
        }
    }
}





//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::CheckCharVariable()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        // Declaring the variable..
        if ( TOKEN::Match(tok, "[{};(,] char %var% [;=,)]") )
        {
            const char *varname[2] = {0};
            varname[0] = TOKEN::getstr(tok, 2);

            // Check usage of char variable..
            int indentlevel = 0;
            for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
            {
                if ( TOKEN::Match(tok2, "{") )
                    ++indentlevel;

                else if ( TOKEN::Match(tok2, "}") )
                {
                    --indentlevel;
                    if ( indentlevel <= 0 )
                        break;
                }

                if (!TOKEN::Match(tok2,".") && TOKEN::Match(tok2->next, "%var% [ %var1% ]", varname))
                {
                    std::ostringstream errmsg;
                    errmsg << _tokenizer->fileLine(tok2->next) << ": Warning - using char variable as array index";
                    _errorLogger->reportErr(errmsg.str());
                    break;
                }

                if ( TOKEN::Match(tok2, "%var% [&|] %var1%", varname) || TOKEN::Match(tok2, "%var1% [&|]", varname) )
                {
                    std::ostringstream errmsg;
                    errmsg << _tokenizer->fileLine(tok2) << ": Warning - using char variable in bit operation";
                    _errorLogger->reportErr(errmsg.str());
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

    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next )
    {
        if ( TOKEN::Match(tok, "(") )
            ++parlevel;
        else if ( TOKEN::Match(tok, ")") )
            --parlevel;

        if ( parlevel != 0 )
            continue;

        if ( !TOKEN::Match(tok,"#") && TOKEN::Match(tok->next,"; %str%") && !TOKEN::Match(tok->at(3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok->next) << ": Redundant code: Found a statement that begins with string constant";
            _errorLogger->reportErr(errmsg.str());
        }

        if ( !TOKEN::Match(tok,"#") && TOKEN::Match(tok->next,"; %num%") && !TOKEN::Match(tok->at(3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok->next) << ": Redundant code: Found a statement that begins with numeric constant";
            _errorLogger->reportErr(errmsg.str());
        }
    }
}

