/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#include "checkother.h"

#include <list>
#include <map>
#include <sstream>
#include <stdlib.h>     // <- atoi
#include <cstring>
#include <cctype>
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Old style pointer casting..
        if (!Token::Match(tok, "( %type% * ) %var%"))
            continue;

        // Is "type" a class?
        const std::string pattern("class " + tok->next()->str());
        if (!Token::findmatch(_tokenizer->tokens(), pattern.c_str()))
            continue;

        std::ostringstream ostr;
        ostr << _tokenizer->fileLine(tok) << ": C-style pointer casting";
        _errorLogger->reportErr(ostr.str());
    }
}




//---------------------------------------------------------------------------
// Redundant code..
//---------------------------------------------------------------------------

void CheckOther::WarningRedundantCode()
{

    // if (p) delete p
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if ( tok->str() != "if" )
            continue;

        const char *varname1 = NULL;
        const Token *tok2 = NULL;

        if (Token::Match(tok,"if ( %var% )"))
        {
            varname1 = tok->strAt( 2);
            tok2 = tok->tokAt(4);
        }
        else if (Token::Match(tok,"if ( %var% != NULL )"))
        {
            varname1 = tok->strAt( 2);
            tok2 = tok->tokAt(6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        bool err = false;
        if ( tok2->str() == "{" )
        {
            tok2 = tok2->next();

            if (Token::Match(tok2,"delete %var% ; }"))
            {
                err = (strcmp(tok2->strAt(1),varname1)==0);
            }
            else if (Token::Match(tok2,"delete [ ] %var% ; }"))
            {
                err = (strcmp(tok2->strAt(1),varname1)==0);
            }
            else if (Token::Match(tok2,"free ( %var% ) ; }"))
            {
                err = (strcmp(tok2->strAt(2),varname1)==0);
            }
            else if (Token::Match(tok2,"kfree ( %var% ) ; }"))
            {
                err = (strcmp(tok2->strAt(2),varname1)==0);
            }
        }
        else
        {
            if (Token::Match(tok2,"delete %var% ;"))
            {
                err = (strcmp(tok2->strAt(1),varname1)==0);
            }
            else if (Token::Match(tok2,"delete [ ] %var% ;"))
            {
                err = (strcmp(tok2->strAt(1),varname1)==0);
            }
            else if (Token::Match(tok2,"free ( %var% ) ;"))
            {
                err = (strcmp(tok2->strAt(2),varname1)==0);
            }
            else if (Token::Match(tok2,"kfree ( %var% ) ;"))
            {
                err = (strcmp(tok2->strAt(2),varname1)==0);
            }
        }

        if (err)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Redundant condition. It is safe to deallocate a NULL pointer";
            _errorLogger->reportErr(ostr.str());
        }
    }



    // Redundant condition
    // if (haystack.find(needle) != haystack.end())
    //    haystack.remove(needle);
    redundantCondition2();
}
//---------------------------------------------------------------------------

void CheckOther::redundantCondition2()
{
    const char pattern[] = "if ( %var% . find ( %any% ) != %var% . end ( ) ) "
                           "{|{|"
                           "    %var% . remove ( %any% ) ; "
                           "}|}|";
    const Token *tok = Token::findmatch( _tokenizer->tokens(), pattern );
    while ( tok )
    {
        bool b = Token::Match( tok->tokAt(15), "{" );

        // Get tokens for the fields %var% and %any%
        const Token *var1 = tok->tokAt(2);
        const Token *any1 = tok->tokAt(6);
        const Token *var2 = tok->tokAt(9);
        const Token *var3 = tok->tokAt(b ? 16 : 15);
        const Token *any2 = tok->tokAt(b ? 20 : 19);

        // Check if all the "%var%" fields are the same and if all the "%any%" are the same..
        if (var1->str() == var2->str() &&
            var2->str() == var3->str() &&
            any1->str() == any2->str() )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok)
                   << ": Redundant condition found. The remove function in the STL will not do anything if element doesn't exist";
            _errorLogger->reportErr(errmsg.str());
        }

        tok = Token::findmatch( tok->next(), pattern );
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// if (condition) ....
//---------------------------------------------------------------------------

void CheckOther::WarningIf()
{
    // Search for 'if (condition);'
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (!Token::simpleMatch(tok, "if ("))
            continue;

        // Search for the end paranthesis for the condition..
        int parlevel = 0;
        for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(")
                ++parlevel;
            else if (tok2->str() == ")")
            {
                --parlevel;
                if ( parlevel <= 0 )
                {
                    if ( Token::Match(tok2, ") ; !!else") )
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

    // Search for 'a=b; if (a==b)'
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Begin statement?
        if ( ! Token::Match(tok, "[;{}]") )
            continue;
        tok = tok->next();
        if ( ! tok )
            break;

        if (!Token::Match(tok,"%var% = %var% ; if ( %var%"))
            continue;

        if ( strcmp(tok->strAt( 9), ")") != 0 )
            continue;

        // var1 = var2 ; if ( var3 cond var4 )
        const char *var1 = tok->strAt( 0);
        const char *var2 = tok->strAt( 2);
        const char *var3 = tok->strAt( 6);
        const char *cond = tok->strAt( 7);
        const char *var4 = tok->strAt( 8);

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
        {
            if (strcmp(cond, p[i]) == 0)
            {
                iscond = true;
                break;
            }
        }
        if (!iscond)
            break;

        // we found the error. Report.
        std::ostringstream ostr;
        ostr << _tokenizer->fileLine(tok->tokAt(4)) << ": The condition is always ";
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
    for ( const Token *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ((tok->str() != "strtol") && (tok->str() != "strtoul"))
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for ( const Token *tok2 = tok->next(); tok2; tok2 = tok2->next() )
        {
            if ( Token::Match(tok2, "(") )
                ++parlevel;
            else if (Token::Match(tok2, ")"))
                --parlevel;
            else if (parlevel == 1 && Token::Match(tok2, ","))
            {
                ++param;
                if (param==3)
                {
                    if ( Token::Match(tok2, ", %num% )") )
                    {
                        int radix = atoi(tok2->strAt( 1));
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "if ( %var% = %num% )") ||
            Token::Match(tok, "if ( %var% = %str% )") ||
            Token::Match(tok, "if ( %var% = %var% )") )
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
    for ( const Token *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ( Token::Match(tok, "[{};(,] %type% %var% [;=,)]") )
        {
            const char *type = tok->strAt( 1);
            if (strcmp(type,"char")==0 || strcmp(type,"short")==0 || strcmp(type,"int")==0)
                varsign[tok->strAt(2)] = 's';
        }

        else if ( Token::Match(tok, "[{};(,] unsigned %type% %var% [;=,)]") )
            varsign[tok->strAt(3)] = 'u';

        else if (!Token::Match(tok,"[).]") && Token::Match(tok->next(), "%var% / %var%"))
        {
            const char *varname1 = tok->strAt(1);
            const char *varname2 = tok->strAt(3);
            char sign1 = varsign[varname1];
            char sign2 = varsign[varname2];

            if ( sign1 && sign2 && sign1 != sign2 )
            {
                // One of the operands are signed, the other is unsigned..
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next()) << ": Warning: Division with signed and unsigned operators";
                _errorLogger->reportErr(ostr.str());
            }
        }

        else if (!Token::Match(tok,"[).]") && Token::Match(tok->next(), "%var% / - %num%"))
        {
            const char *varname1 = tok->strAt(1);
            char sign1 = varsign[varname1];
            if ( sign1 == 'u' )
            {
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next()) << ": Unsigned division. The result will be wrong.";
                _errorLogger->reportErr(ostr.str());
            }
        }

        else if (Token::Match(tok, "[([=*/+-] - %num% / %var%"))
        {
            const char *varname2 = tok->strAt(4);
            char sign2 = varsign[varname2];
            if ( sign2 == 'u' )
            {
                std::ostringstream ostr;
                ostr << _tokenizer->fileLine(tok->next()) << ": Unsigned division. The result will be wrong.";
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
    for ( const Token *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        // Skip class and struct declarations..
        if ( (tok->str() == "class") || (tok->str() == "struct") )
        {
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if ( tok2->str() == "{" )
                {
                    int indentlevel2 = 0;
                    for (tok = tok2; tok; tok = tok->next())
                    {
                        if ( tok->str() == "{" )
                        {
                            ++indentlevel2;
                        }
                        if ( tok->str() == "}" )
                        {
                            --indentlevel2;
                            if ( indentlevel2 <= 0 )
                            {
                                tok = tok->next();
                                break;
                            }
                        }
                    }
                    break;
                }
                if (Token::Match(tok2, "[,);]"))
                {
                    break;
                }
            }
            if ( ! tok )
                break;
        }

        if ( tok->str() == "{" )
        {
            ++indentlevel;
        }
        if ( tok->str() == "}" )
        {
            --indentlevel;
            if ( indentlevel == 0 )
                func = false;
        }
        if ( indentlevel == 0 && Token::Match(tok, ") {") )
        {
            func = true;
        }
        if ( indentlevel > 0 && func && Token::Match(tok, "[{};]") )
        {
            // First token of statement..
            const Token *tok1 = tok->next();
            if ( ! tok1 )
                continue;

            if ((tok1->str() == "return") ||
                (tok1->str() == "delete") ||
                (tok1->str() == "goto") ||
                (tok1->str() == "else"))
                continue;

            // Variable declaration?
            if (Token::Match(tok1, "%var% %var% ;") ||
                Token::Match(tok1, "%var% %var% =") )
            {
                CheckVariableScope_LookupVar( tok1, tok1->strAt( 1) );
            }
        }
    }

}
//---------------------------------------------------------------------------

void CheckOther::CheckVariableScope_LookupVar( const Token *tok1, const char varname[] )
{
    const Token *tok = tok1;

    // Skip the variable declaration..
    while (tok && !Token::Match(tok,";"))
        tok = tok->next();

    // Check if the variable is used in this indentlevel..
    bool used = false, used1 = false;
    int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;
    while ( indentlevel >= 0 && tok )
    {
        if ( tok->str() == "{" )
        {
            ++indentlevel;
        }

        else if ( tok->str() == "}" )
        {
            --indentlevel;
            if ( indentlevel == 0 )
            {
                if ( for_or_while && used )
                    return;
                used1 = used;
                used = false;
            }
        }

        else if ( tok->str() == "(" )
        {
            ++parlevel;
        }

        else if ( tok->str() == ")" )
        {
            --parlevel;
        }


        else if ( tok->str() == varname )
        {
            if ( indentlevel == 0 || used1 )
                return;
            used = true;
        }

        else if ( indentlevel==0 )
        {
            if ( (tok->str() == "for") || (tok->str() == "while") )
                for_or_while = true;
            if ( parlevel == 0 && (tok->str() == ";") )
                for_or_while = false;
        }

        tok = tok->next();
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if ( Token::Match(tok,"[,(] const std :: %type% %var% [,)]") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok) << " " << tok->strAt(5) << " is passed by value, it could be passed by reference/pointer instead";
            _errorLogger->reportErr( errmsg.str() );
        }

        else if ( Token::Match(tok,"[,(] const %type% %var% [,)]") )
        {
            // Check if type is a struct or class.
            const std::string pattern(std::string("class|struct ") + tok->strAt(2));
            if ( Token::findmatch(_tokenizer->tokens(), pattern.c_str()) )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok) << " " << tok->strAt(3) << " is passed by value, it could be passed by reference/pointer instead";
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

    for ( const Token *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ( tok->fileIndex() != 0 )
            continue;
        if ( tok->str() == "}" )
            structname = 0;
        if ( Token::Match(tok, "struct|union %type% {") )
            structname = tok->strAt( 1);

        if (structname && Token::Match(tok, "[{;]"))
        {
            const char *varname = 0;
            if (Token::Match(tok->next(), "%type% %var% [;[]"))
                varname = tok->strAt( 2 );
            else if (Token::Match(tok->next(), "%type% %type% %var% [;[]"))
                varname = tok->strAt( 2 );
            else if (Token::Match(tok->next(), "%type% * %var% [;[]"))
                varname = tok->strAt( 3 );
            else if (Token::Match(tok->next(), "%type% %type% * %var% [;[]"))
                varname = tok->strAt( 4 );
            else
                continue;

            const std::string usagePattern( ". " + std::string(varname) );
            bool used = false;
            for ( const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next() )
            {
                if (Token::simpleMatch(tok2, usagePattern.c_str() ))
                {
                    used = true;
                    break;
                }
            }

            if ( ! used )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok->next()) << ": struct or union member '" << structname << "::" << varname << "' is never used";
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Declaring the variable..
        if ( Token::Match(tok, "[{};(,] char %var% [;=,)]") )
        {
            // Set tok to point to the variable name
            tok = tok->tokAt( 2 );

            // Check usage of char variable..
            int indentlevel = 0;
            for ( const Token *tok2 = tok->next(); tok2; tok2 = tok2->next() )
            {
                if ( tok2->str() == "{" )
                    ++indentlevel;

                else if ( tok2->str() == "}" )
                {
                    --indentlevel;
                    if ( indentlevel <= 0 )
                        break;
                }

                if ((tok2->str() != ".") && Token::Match(tok2->next(), "%var% [ %varid% ]", tok->varId()))
                {
                    std::ostringstream errmsg;
                    errmsg << _tokenizer->fileLine(tok2->next()) << ": Warning - using char variable as array index";
                    _errorLogger->reportErr(errmsg.str());
                    break;
                }

                if ( Token::Match(tok2, "%var% [&|] %varid%", tok->varId()) || Token::Match(tok2, "%varid% [&|]", tok->varId()) )
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

    for ( const Token *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ( tok->str() == "(" )
            ++parlevel;
        else if ( tok->str() == ")" )
            --parlevel;

        if ( parlevel != 0 )
            continue;

        if ( (tok->str() != "#") && Token::Match(tok->next(),"; %str%") && !Token::Match(tok->tokAt(3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok->next()) << ": Redundant code: Found a statement that begins with string constant";
            _errorLogger->reportErr(errmsg.str());
        }

        if ( !Token::Match(tok,"#") && Token::Match(tok->next(),"; %num%") && !Token::Match(tok->tokAt(3), ",") )
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok->next()) << ": Redundant code: Found a statement that begins with numeric constant";
            _errorLogger->reportErr(errmsg.str());
        }
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Unreachable code below a 'return'
//---------------------------------------------------------------------------

void CheckOther::unreachableCode()
{
    const Token *tok = Token::findmatch( _tokenizer->tokens(), "[;{}] return" );
    while ( tok )
    {
        // Goto the 'return' token
        tok = tok->next();

        // Locate the end of the 'return' statement
        while ( tok && ! Token::Match(tok, ";") )
            tok = tok->next();
        while ( tok && Token::Match(tok->next(), ";") )
            tok = tok->next();

        // If there is a statement below the return it is unreachable
        if (!Token::Match(tok, "; case|default|}|#") && !Token::Match(tok, "; %var% :"))
        {
            std::ostringstream errmsg;
            errmsg << _tokenizer->fileLine(tok->next()) << ": Unreachable code below a 'return'";
            _errorLogger->reportErr(errmsg.str());
        }

        // Find the next 'return' statement
        tok = Token::findmatch( tok, "[;{}] return" );
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------

static bool isOp(const Token *tok)
{
    return bool(tok &&
                (tok->str() == "&&" ||
                 tok->str() == "||" ||
                 tok->str() == "==" ||
                 tok->str() == "!=" ||
                 tok->str() == "<" ||
                 tok->str() == "<=" ||
                 tok->str() == ">" ||
                 tok->str() == ">=" ||
                 tok->str() == "<<" ||
                 Token::Match(tok, "[+-*/%&!~|,[])?:]")));
}

void CheckOther::functionVariableUsage()
{
    // Parse all executing scopes..
    const Token *tok1 = _tokenizer->tokens();
    if (!tok1)
        return;

    while ((tok1 = Token::findmatch( tok1->next(), ") const| {" )) != NULL)
    {
        // Varname, usage {1=declare, 2=read, 4=write}
        std::map<std::string, unsigned int> varUsage;
        static const unsigned int USAGE_DECLARE = 1;
        static const unsigned int USAGE_READ    = 2;
        static const unsigned int USAGE_WRITE   = 4;

        int indentlevel = 0;
        for ( const Token *tok = tok1; tok; tok = tok->next() )
        {
            if ( tok->str() == "{" )
                ++indentlevel;
            else if ( tok->str() == "}" )
            {
                --indentlevel;
                if ( indentlevel <= 0 )
                    break;
            }
            else if ( Token::Match(tok, "struct|union|class {") ||
                      Token::Match(tok, "struct|union|class %type% {") )
            {
                while ( tok && tok->str() != "}" )
                    tok = tok->next();
                if ( tok )
                    continue;
                break;
            }

            if ( Token::Match(tok, "[;{}] bool|char|short|int|long|float|double %var% ;|=") )
                varUsage[ tok->strAt(2) ] = USAGE_DECLARE;

            if ( Token::Match(tok, "[;{}] bool|char|short|int|long|float|double * %var% ;|=") )
                varUsage[ tok->strAt(3) ] = USAGE_DECLARE;

            if ( Token::Match(tok, "delete|return %var%") )
                varUsage[ tok->strAt(1) ] |= USAGE_READ;

            if ( Token::Match(tok, "%var% =") )
                varUsage[ tok->str() ] |= USAGE_WRITE;

            if ( Token::Match(tok, "else %var% =") )
                varUsage[ tok->strAt(1) ] |= USAGE_WRITE;

            if ( Token::Match(tok, ">>|& %var%") )
                varUsage[ tok->strAt(1) ] |= USAGE_WRITE;

            if ((Token::Match(tok,"[(=&!]") || isOp(tok)) && Token::Match(tok->next(), "%var%"))
                varUsage[ tok->strAt(1) ] |= USAGE_READ;

            if (Token::Match(tok, "-=|+=|*=|/=|&=|^= %var%") || Token::Match(tok, "|= %var%"))
                varUsage[ tok->strAt(1) ] |= USAGE_READ;

            if (Token::Match(tok, "%var%") && (tok->next()->str()==")" || isOp(tok->next())))
                varUsage[ tok->str() ] |= USAGE_READ;

            if ( Token::Match(tok, "[(,] %var% [,)]") )
                varUsage[ tok->strAt(1) ] |= USAGE_WRITE;
        }

        // Check usage of all variables in the current scope..
        for ( std::map<std::string, unsigned int>::const_iterator it = varUsage.begin(); it != varUsage.end(); ++it )
        {
            std::string varname = it->first;
            unsigned int usage = it->second;

            if (!std::isalpha(varname[0]))
                continue;

            if ( ! ( usage & USAGE_DECLARE ) )
                continue;

            if ( usage == USAGE_DECLARE )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok1->next()) << ": Unused variable '" << varname << "'";
                _errorLogger->reportErr(errmsg.str());
            }

            else if ( ! (usage & USAGE_READ) )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok1->next()) << ": Variable '" << varname << "' is assigned a value that is never used";
                _errorLogger->reportErr(errmsg.str());
            }

            else if ( ! (usage & USAGE_WRITE) )
            {
                std::ostringstream errmsg;
                errmsg << _tokenizer->fileLine(tok1->next()) << ": Variable '" << varname << "' is not assigned a value";
                _errorLogger->reportErr(errmsg.str());
            }
        }
    }
}
