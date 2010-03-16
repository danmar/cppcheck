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
#include "checkother.h"
#include "mathlib.h"
#include "tokenize.h"
#include "executionpath.h"

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <cstring>
#include <cctype>
#include <memory>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckOther instance;
}

//---------------------------------------------------------------------------



void CheckOther::warningOldStylePointerCast()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Old style pointer casting..
        if (!Token::Match(tok, "( const| %type% * ) %var%") &&
            !Token::Match(tok, "( const| %type% * ) (| new"))
            continue;

        int addToIndex = 0;
        if (tok->tokAt(1)->str() == "const")
            addToIndex = 1;

        if (tok->tokAt(4 + addToIndex)->str() == "const")
            continue;

        // Is "type" a class?
        const std::string pattern("class " + tok->tokAt(1 + addToIndex)->str());
        if (!Token::findmatch(_tokenizer->tokens(), pattern.c_str()))
            continue;

        cstyleCastError(tok);
    }
}




//---------------------------------------------------------------------------
// Redundant code..
//---------------------------------------------------------------------------

void CheckOther::warningRedundantCode()
{

    // if (p) delete p
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (! Token::simpleMatch(tok, "if ("))
            continue;

        std::string varname;
        const Token *tok2 = tok->tokAt(2);

        /*
         * Possible if-constructions:
         *
         *   if (var)
         *   if (this->var)
         *   if (Foo::var)
         *
         **/

        while (Token::Match(tok2, "%var% .|::"))
        {
            varname.append(tok2->str());
            varname.append(tok2->next()->str());
            tok2 = tok2->tokAt(2);
        }

        if (!Token::Match(tok2, "%var% ) {"))
            continue;

        varname.append(tok2->str());
        tok2 = tok2->tokAt(3);

        /*
         * Possible constructions:
         *
         * - delete %var%
         * - delete [] %var%
         * - free ( %var )
         * - kfree ( %var% )
         *
         * Where %var% may be:
         * - just variable name (var)
         * - class member (this->var)
         * - static member (Class::var)
         *
         **/

        bool funcHasBracket = false;
        if (Token::Match(tok2, "free|kfree ("))
        {
            tok2 = tok2->tokAt(2);
            funcHasBracket = true;

        }
        else if (tok2->str() == "delete")
        {

            tok2 = tok2->next();

            if (Token::simpleMatch(tok2, "[ ]"))
            {
                tok2 = tok2->tokAt(2);
            }
        }

        std::string varname2;
        while (Token::Match(tok2, "%var% ::|."))
        {
            varname2.append(tok2->str());
            varname2.append(tok2->next()->str());
            tok2 = tok2->tokAt(2);
        }

        varname2.append(tok2->str());
        if (Token::Match(tok2, "%var%") && varname == varname2)
            tok2 = tok2->next();
        else
            continue;

        if (funcHasBracket)
        {
            if (tok2->str() != ")")
            {
                continue;
            }
            else
            {
                tok2 = tok2->next();
            }
        }

        if (!Token::Match(tok2, "; } !!else"))
        {
            continue;
        }

        redundantIfDelete0Error(tok);
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
    const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern);
    while (tok)
    {
        bool b(tok->tokAt(15)->str() == "{");

        // Get tokens for the fields %var% and %any%
        const Token *var1 = tok->tokAt(2);
        const Token *any1 = tok->tokAt(6);
        const Token *var2 = tok->tokAt(9);
        const Token *var3 = tok->tokAt(b ? 16 : 15);
        const Token *any2 = tok->tokAt(b ? 20 : 19);

        // Check if all the "%var%" fields are the same and if all the "%any%" are the same..
        if (var1->str() == var2->str() &&
            var2->str() == var3->str() &&
            any1->str() == any2->str())
        {
            redundantIfRemoveError(tok);
        }

        tok = Token::findmatch(tok->next(), pattern);
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// strtol(str, 0, radix)  <- radix must be 0 or 2-36
//---------------------------------------------------------------------------

void CheckOther::invalidFunctionUsage()
{
    // strtol and strtoul..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if ((tok->str() != "strtol") && (tok->str() != "strtoul"))
            continue;

        // Locate the third parameter of the function call..
        int parlevel = 0;
        int param = 1;
        for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(")
                ++parlevel;
            else if (tok2->str() == ")")
                --parlevel;
            else if (parlevel == 1 && tok2->str() == ",")
            {
                ++param;
                if (param == 3)
                {
                    if (Token::Match(tok2, ", %num% )"))
                    {
                        int radix = MathLib::toLongNumber(tok2->next()->str());
                        if (!(radix == 0 || (radix >= 2 && radix <= 36)))
                        {
                            dangerousUsageStrtolError(tok2);
                        }
                    }
                    break;
                }
            }
        }
    }

    // sprintf|snprintf overlapping data
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Get variable id of target buffer..
        unsigned int varid = 0;

        if (Token::Match(tok, "sprintf|snprintf ( %var% ,"))
            varid = tok->tokAt(2)->varId();

        else if (Token::Match(tok, "sprintf|snprintf ( %var% . %var% ,"))
            varid = tok->tokAt(4)->varId();

        if (varid == 0)
            continue;

        // goto ","
        const Token *tok2 = tok->tokAt(3);
        while (tok2 && tok2->str() != ",")
            tok2 = tok2->next();

        // is any source buffer overlapping the target buffer?
        int parlevel = 0;
        while ((tok2 = tok2->next()) != NULL)
        {
            if (tok2->str() == "(")
                ++parlevel;
            else if (tok2->str() == ")")
            {
                --parlevel;
                if (parlevel < 0)
                    break;
            }
            else if (parlevel == 0 && Token::Match(tok2, ", %varid% [,)]", varid))
            {
                sprintfOverlappingDataError(tok2->next(), tok2->next()->str());
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckOther::checkUnsignedDivision()
{
    if (!_settings->_showAll || !_settings->_checkCodingStyle)
        return;

    // Check for "ivar / uvar" and "uvar / ivar"
    std::map<unsigned int, char> varsign;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "[{};(,] %type% %var% [;=,)]"))
        {
            const std::string type = tok->strAt(1);
            if (type == "char" || type == "short" || type == "int")
                varsign[tok->tokAt(2)->varId()] = 's';
        }

        else if (Token::Match(tok, "[{};(,] unsigned %type% %var% [;=,)]"))
            varsign[tok->tokAt(3)->varId()] = 'u';

        else if (!Token::Match(tok, "[).]") &&
                 Token::Match(tok->next(), "%var% / %var%") &&
                 tok->tokAt(1)->varId() != 0 &&
                 tok->tokAt(3)->varId() != 0)
        {
            if (ErrorLogger::udivWarning(*_settings))
            {
                char sign1 = varsign[tok->tokAt(1)->varId()];
                char sign2 = varsign[tok->tokAt(3)->varId()];

                if (sign1 && sign2 && sign1 != sign2)
                {
                    // One of the operands are signed, the other is unsigned..
                    udivWarning(tok->next());
                }
            }
        }

        else if (!Token::Match(tok, "[).]") && Token::Match(tok->next(), "%var% / %num%"))
        {
            if (tok->strAt(3)[0] == '-' && ErrorLogger::udivError())
            {
                char sign1 = varsign[tok->tokAt(1)->varId()];
                if (sign1 == 'u')
                {
                    udivError(tok->next());
                }
            }
        }

        else if (Token::Match(tok, "[([=*/+-,] %num% / %var%"))
        {
            if (tok->strAt(1)[0] == '-' && ErrorLogger::udivError())
            {
                char sign2 = varsign[tok->tokAt(3)->varId()];
                if (sign2 == 'u')
                {
                    udivError(tok->next());
                }
            }
        }
    }
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Unreachable code below a 'return'
//---------------------------------------------------------------------------

void CheckOther::unreachableCode()
{
    const Token *tok = _tokenizer->tokens();
    while ((tok = Token::findmatch(tok, "[;{}] return")))
    {
        // Goto the 'return' token
        tok = tok->next();

        // Locate the end of the 'return' statement
        while (tok && tok->str() != ";")
            tok = tok->next();
        while (tok && tok->next() && tok->next()->str() == ";")
            tok = tok->next();

        if (!tok)
            break;

        // If there is a statement below the return it is unreachable
        /* original:
                if (!Token::Match(tok, "; case|default|}|#") &&
                    !Token::Match(tok, "; %var% :") &&
                    !Token::simpleMatch(tok, "; break"))
        */
        if (Token::simpleMatch(tok, "; break"))
        {
            unreachableCodeError(tok->next());
        }
    }
}

void CheckOther::unreachableCodeError(const Token *tok)
{
    reportError(tok, Severity::style, "unreachableCode", "Unreachable code below a 'return'");
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
                 Token::Match(tok, "[+-*/%&!~|^,[])?:]")));
}

/** Store information about variable usage */
class VariableUsage
{
public:
    VariableUsage()
    {
        declare = false;
        read = false;
        write = false;
    }

    /** variable is used.. set both read+write */
    void use()
    {
        read = true;
        write = true;
    }

    /** is variable unused? */
    bool unused() const
    {
        return (read == false && write == false);
    }

    bool declare;
    bool read;
    bool write;
};

void CheckOther::functionVariableUsage()
{
    // Parse all executing scopes..
    for (const Token *token = Token::findmatch(_tokenizer->tokens(), ") const| {"); token;)
    {
        // goto "{"
        while (token->str() != "{")
            token = token->next();

        // First token for the current scope..
        const Token * const tok1 = token;

        // Find next scope that will be checked next time..
        token = Token::findmatch(token->link(), ") const| {");

        // Varname, usage {1=declare, 2=read, 4=write}
        std::map<std::string, VariableUsage> varUsage;

        unsigned int indentlevel = 0;
        for (const Token *tok = tok1; tok; tok = tok->next())
        {
            if (tok->str() == "{")
                ++indentlevel;
            else if (tok->str() == "}")
            {
                if (indentlevel <= 1)
                    break;
                --indentlevel;
            }
            else if (Token::Match(tok, "struct|union|class {") ||
                     Token::Match(tok, "struct|union|class %type% {"))
            {
                while (tok->str() != "{")
                    tok = tok->next();
                tok = tok->link();
                if (! tok)
                    break;
            }

            if (Token::Match(tok, "[;{}] %type% %var% ;|=") && tok->next()->isStandardType())
                varUsage[tok->strAt(2)].declare = true;

            else if (Token::Match(tok, "[;{}] %type% * %var% ;|=") && tok->next()->isStandardType())
                varUsage[tok->strAt(3)].declare = true;

            else if (Token::Match(tok, "delete|return %var%"))
                varUsage[tok->strAt(1)].read = true;

            else if (Token::Match(tok, "%var% ="))
                varUsage[tok->str()].write = true;

            else if (Token::Match(tok, "else %var% ="))
                varUsage[ tok->strAt(1)].write = true;

            else if (Token::Match(tok, ">>|& %var%"))
                varUsage[ tok->strAt(1)].use();    // use = read + write

            else if (Token::Match(tok, "[(,] %var% [,)]"))
                varUsage[ tok->strAt(1)].use();   // use = read + write

            else if ((Token::Match(tok, "[(=&!]") || isOp(tok)) && Token::Match(tok->next(), "%var%"))
                varUsage[ tok->strAt(1)].read = true;

            else if (Token::Match(tok, "-=|+=|*=|/=|&=|^= %var%") || Token::Match(tok, "|= %var%"))
                varUsage[ tok->strAt(1)].read = true;

            else if (Token::Match(tok, "%var%") && (tok->next()->str() == ")" || isOp(tok->next())))
                varUsage[ tok->str()].read = true;

            else if (Token::Match(tok, "; %var% ;"))
                varUsage[ tok->strAt(1)].read = true;

            else if (Token::Match(tok, "++|-- %var%") ||
                     Token::Match(tok, "%var% ++|--"))
                varUsage[tok->strAt(1)].use();
        }

        // Check usage of all variables in the current scope..
        for (std::map<std::string, VariableUsage>::const_iterator it = varUsage.begin(); it != varUsage.end(); ++it)
        {
            const std::string &varname = it->first;
            const VariableUsage &usage = it->second;

            if (!std::isalpha(varname[0]))
                continue;

            if (!(usage.declare))
                continue;

            if (usage.unused())
            {
                unusedVariableError(tok1, varname);
            }

            else if (!(usage.read))
            {
                unreadVariableError(tok1, varname);
            }

            else if (!(usage.write))
            {
                unassignedVariableError(tok1, varname);
            }
        }
    }
}

void CheckOther::unusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedVariable", "Unused variable: " + varname);
}

void CheckOther::unreadVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unreadVariable", "Variable '" + varname + "' is assigned a value that is never used");
}

void CheckOther::unassignedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unassignedVariable", "Variable '" + varname + "' is not assigned a value");
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Check scope of variables..
//---------------------------------------------------------------------------

void CheckOther::checkVariableScope()
{
    // Walk through all tokens..
    bool func = false;
    int indentlevel = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Skip class and struct declarations..
        if ((tok->str() == "class") || (tok->str() == "struct"))
        {
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                {
                    tok = tok2->link();
                    break;
                }
                if (Token::Match(tok2, "[,);]"))
                {
                    break;
                }
            }
            if (! tok)
                break;
        }

        else if (tok->str() == "{")
        {
            ++indentlevel;
        }
        else if (tok->str() == "}")
        {
            --indentlevel;
            if (indentlevel == 0)
                func = false;
        }
        if (indentlevel == 0 && Token::simpleMatch(tok, ") {"))
        {
            func = true;
        }
        if (indentlevel > 0 && func && Token::Match(tok, "[{};]"))
        {
            // First token of statement..
            const Token *tok1 = tok->next();
            if (! tok1)
                continue;

            if ((tok1->str() == "return") ||
                (tok1->str() == "throw") ||
                (tok1->str() == "delete") ||
                (tok1->str() == "goto") ||
                (tok1->str() == "else"))
                continue;

            // Variable declaration?
            if (Token::Match(tok1, "%type% %var% ; %var% = %num% ;"))
            {
                // Tokenizer modify "int i = 0;" to "int i; i = 0;",
                // so to handle this situation we just skip
                // initialization (see ticket #272).
                const unsigned int firstVarId = tok1->next()->varId();
                const unsigned int secondVarId = tok1->tokAt(3)->varId();
                if (firstVarId > 0 && firstVarId == secondVarId)
                {
                    lookupVar(tok1->tokAt(6), tok1->strAt(1));
                }
            }
            else if (Token::Match(tok1, "%type% %var% [;=]"))
            {
                lookupVar(tok1, tok1->strAt(1));
            }
        }
    }

}
//---------------------------------------------------------------------------

void CheckOther::lookupVar(const Token *tok1, const std::string &varname)
{
    const Token *tok = tok1;

    // Skip the variable declaration..
    while (tok && tok->str() != ";")
        tok = tok->next();

    // Check if the variable is used in this indentlevel..
    bool used1 = false;   // used in one sub-scope -> reducable
    bool used2 = false;   // used in more sub-scopes -> not reducable
    int indentlevel = 0;
    int parlevel = 0;
    bool for_or_while = false;  // is sub-scope a "for/while/etc". anything that is not "if"
    while (tok)
    {
        if (tok->str() == "{")
        {
            ++indentlevel;
        }

        else if (tok->str() == "}")
        {
            if (indentlevel == 0)
                break;
            --indentlevel;
            if (indentlevel == 0)
            {
                if (for_or_while && used2)
                    return;
                used2 |= used1;
                used1 = false;
            }
        }

        else if (tok->str() == "(")
        {
            ++parlevel;
        }

        else if (tok->str() == ")")
        {
            --parlevel;
        }

        // Bail out if references are used
        else if (Token::simpleMatch(tok, (std::string("& ") + varname).c_str()))
        {
            return;
        }

        else if (tok->str() == varname)
        {
            if (indentlevel == 0)
                return;
            used1 = true;
            if (for_or_while && !Token::simpleMatch(tok->next(), "="))
                used2 = true;
            if (used1 && used2)
                return;
        }

        else if (indentlevel == 0)
        {
            // %unknown% ( %any% ) {
            // If %unknown% is anything except if, we assume
            // that it is a for or while loop or a macro hiding either one
            if (Token::simpleMatch(tok->next(), "(") &&
                Token::simpleMatch(tok->next()->link(), ") {"))
            {
                if (tok->str() != "if")
                    for_or_while = true;
            }

            if (Token::simpleMatch(tok, "do {"))
                for_or_while = true;

            if (parlevel == 0 && (tok->str() == ";"))
                for_or_while = false;
        }

        tok = tok->next();
    }

    // Warning if this variable:
    // * not used in this indentlevel
    // * used in lower indentlevel
    if (used1 || used2)
        variableScopeError(tok1, varname);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check for constant function parameters
//---------------------------------------------------------------------------

void CheckOther::checkConstantFunctionParameter()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "[,(] const std :: %type% %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(5));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(8));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(10));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% , std :: %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(14));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% , std :: %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(12));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < std :: %type% , %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(12));
        }

        else if (Token::Match(tok, "[,(] const std :: %type% < %type% , %type% > %var% [,)]"))
        {
            passedByValueError(tok, tok->strAt(10));
        }

        else if (Token::Match(tok, "[,(] const %type% %var% [,)]"))
        {
            // Check if type is a struct or class.
            const std::string pattern(std::string("class|struct ") + tok->strAt(2));
            if (Token::findmatch(_tokenizer->tokens(), pattern.c_str()))
            {
                passedByValueError(tok, tok->strAt(3));
            }
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------

void CheckOther::checkStructMemberUsage()
{
    std::string structname;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "struct|union %type% {"))
        {
            structname = tok->strAt(1);

            // Bail out if struct/union contain any functions
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                {
                    structname = "";
                    break;
                }

                if (tok2->str() == "}")
                    break;
            }

            // Bail out if some data is casted to struct..
            const std::string s("( struct| " + tok->next()->str() + " * ) & %var% [");
            if (Token::findmatch(tok, s.c_str()))
                structname = "";
        }

        if (tok->str() == "}")
            structname = "";

        if (!structname.empty() && Token::Match(tok, "[{;]"))
        {
            std::string varname;
            if (Token::Match(tok->next(), "%type% %var% [;[]"))
                varname = tok->strAt(2);
            else if (Token::Match(tok->next(), "%type% %type% %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% * %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% %type% * %var% [;[]"))
                varname = tok->strAt(4);
            else
                continue;

            const std::string usagePattern(". " + varname);
            bool used = false;
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next())
            {
                if (Token::simpleMatch(tok2, usagePattern.c_str()))
                {
                    used = true;
                    break;
                }
            }

            if (! used)
            {
                unusedStructMemberError(tok->next(), structname, varname);
            }
        }
    }
}





//---------------------------------------------------------------------------
// Check usage of char variables..
//---------------------------------------------------------------------------

void CheckOther::checkCharVariable()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Declaring the variable..
        if (Token::Match(tok, "[{};(,] signed| char %var% [;=,)]"))
        {
            // Set tok to point to the variable name
            tok = tok->tokAt(2);
            if (tok->str() == "char")
                tok = tok->next();

            // Check usage of char variable..
            int indentlevel = 0;
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;

                else if (tok2->str() == "}")
                {
                    --indentlevel;
                    if (indentlevel <= 0)
                        break;
                }

                else if (tok2->str() == "return")
                    continue;

                std::string temp = "%var% [ " + tok->str() + " ]";
                if ((tok2->str() != ".") && Token::Match(tok2->next(), temp.c_str()))
                {
                    charArrayIndexError(tok2->next());
                    break;
                }

                if (Token::Match(tok2, "[;{}] %var% = %any% [&|] %any% ;"))
                {
                    // is the char variable used in the calculation?
                    if (tok2->tokAt(3)->varId() != tok->varId() && tok2->tokAt(5)->varId() != tok->varId())
                        continue;

                    // it's ok with a bitwise and where the other operand is 0xff or less..
                    if (std::string(tok2->strAt(4)) == "&")
                    {
                        if (tok2->tokAt(3)->isNumber() && MathLib::isGreater("0x100", tok2->strAt(3)))
                            continue;
                        if (tok2->tokAt(5)->isNumber() && MathLib::isGreater("0x100", tok2->strAt(5)))
                            continue;
                    }

                    // is the result stored in a short|int|long?
                    if (!Token::findmatch(_tokenizer->tokens(), "short|int|long %varid%", tok2->next()->varId()))
                        continue;

                    // This is an error..
                    charBitOpError(tok2);
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

void CheckOther::checkIncompleteStatement()
{
    int parlevel = 0;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
            --parlevel;

        if (parlevel != 0)
            continue;

        if (Token::simpleMatch(tok, "= {"))
        {
            /* We are in an assignment, so it's not a statement.
             * Skip until ";" */

            while (tok->str() != ";")
            {
                int level = 0;
                do
                {
                    if (tok->str() == "(" || tok->str() == "{")
                        ++level;
                    else if (tok->str() == ")" || tok->str() == "}")
                        --level;

                    tok = tok->next();

                    if (tok == NULL)
                        return;
                }
                while (level > 0);
            }

            continue;
        }

        if (Token::Match(tok, "[;{}] %str%") && !Token::Match(tok->tokAt(2), "[,}]"))
        {
            constStatementError(tok->next(), "string");
        }

        if (Token::Match(tok, "[;{}] %num%") && !Token::Match(tok->tokAt(2), "[,}]"))
        {
            constStatementError(tok->next(), "numeric");
        }
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// str plus char
//---------------------------------------------------------------------------

void CheckOther::strPlusChar()
{
    bool charVars[10000] = {0};

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Declaring char variable..
        if (Token::Match(tok, "char|int|short %var% [;=]"))
        {
            unsigned int varid = tok->next()->varId();
            if (varid > 0 && varid < 10000)
                charVars[varid] = true;
        }

        //
        else if (Token::Match(tok, "[=(] %str% + %any%"))
        {
            // char constant..
            const std::string s = tok->strAt(3);
            if (s[0] == '\'')
                strPlusChar(tok->next());

            // char variable..
            unsigned int varid = tok->tokAt(3)->varId();
            if (varid > 0 && varid < 10000 && charVars[varid])
                strPlusChar(tok->next());
        }
    }
}


void CheckOther::nullPointerAfterLoop()
{
    // Locate insufficient null-pointer handling after loop
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (! Token::Match(tok, "while ( %var% )"))
            continue;

        const unsigned int varid(tok->tokAt(2)->varId());
        if (varid == 0)
            continue;

        const std::string varname(tok->strAt(2));

        // Locate the end of the while loop..
        const Token *tok2 = tok->tokAt(4);
        if (tok2->str() == "{")
            tok2 = tok2->link();
        else
        {
            while (tok2 && tok2->str() != ";")
                tok2 = tok2->next();
        }

        // Goto next token
        if (tok2)
            tok2 = tok2->next();

        // Check if the variable is dereferenced..
        while (tok2)
        {
            if (tok2->str() == "{" || tok2->str() == "}" || tok2->str() == "break")
                break;

            if (tok2->varId() == varid)
            {
                if (tok2->next()->str() == "." || Token::Match(tok2->next(), "= %varid% .", varid))
                {
                    // Is this variable a pointer?
                    const Token *tok3 = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid);
                    if (!tok3)
                        break;

                    if (!tok3->previous() ||
                        Token::Match(tok3->previous(), "[({};]") ||
                        tok3->previous()->isName())
                    {
                        nullPointerError(tok2, varname);
                    }
                }
                break;
            }

            tok2 = tok2->next();
        }
    }
}

void CheckOther::nullPointerLinkedList()
{
    // looping through items in a linked list in a inner loop..
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        // search for a "for" token..
        if (!Token::simpleMatch(tok1, "for ("))
            continue;

        if (!Token::simpleMatch(tok1->next()->link(), ") {"))
            continue;

        // is there any dereferencing occuring in the for statement..
        unsigned int parlevel2 = 1;
        for (const Token *tok2 = tok1->tokAt(2); tok2; tok2 = tok2->next())
        {
            // Parantheses..
            if (tok2->str() == "(")
                ++parlevel2;
            else if (tok2->str() == ")")
            {
                if (parlevel2 <= 1)
                    break;
                --parlevel2;
            }

            // Dereferencing a variable inside the "for" parantheses..
            else if (Token::Match(tok2, "%var% . %var%"))
            {
                const unsigned int varid(tok2->varId());
                if (varid == 0)
                    continue;

                if (Token::Match(tok2->tokAt(-2), "%varid% ?", varid))
                    continue;

                const std::string varname(tok2->str());

                // Check usage of dereferenced variable in the loop..
                unsigned int indentlevel3 = 0;
                for (const Token *tok3 = tok1->next()->link(); tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "{")
                        ++indentlevel3;
                    else if (tok3->str() == "}")
                    {
                        if (indentlevel3 <= 1)
                            break;
                        --indentlevel3;
                    }
                    else if (Token::Match(tok3, "while ( %varid% &&|)", varid))
                    {
                        // Make sure there is a "break" to prevent segmentation faults..
                        unsigned int indentlevel4 = indentlevel3;
                        for (const Token *tok4 = tok3->next()->link(); tok4; tok4 = tok4->next())
                        {
                            if (tok4->str() == "{")
                                ++indentlevel4;
                            else if (tok4->str() == "}")
                            {
                                if (indentlevel4 <= 1)
                                {
                                    // Is this variable a pointer?
                                    const Token *tempTok = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid);
                                    if (tempTok)
                                        nullPointerError(tok1, varname, tok3->linenr());

                                    break;
                                }
                                --indentlevel4;
                            }
                            else if (tok4->str() == "break")
                                break;
                        }
                    }
                }
            }
        }
    }
}

void CheckOther::nullPointerStructByDeRefAndChec()
{

    // Dereferencing a struct pointer and then checking if it's NULL..
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        if (Token::Match(tok1, "[{};] %var% = %var% . %var%"))
        {
            if (std::string(tok1->strAt(1)) == tok1->strAt(3))
                continue;

            tok1 = tok1->tokAt(3);
            const unsigned int varid1(tok1->varId());
            if (varid1 == 0)
                continue;

            const std::string varname(tok1->str());

            // Checking if the struct pointer is non-null before the assignment..
            {
                const Token *tok2 = _tokenizer->tokens();
                while (tok2)
                {
                    if (tok2 == tok1)
                        break;
                    if (Token::Match(tok2, "if|while ( !| %varid% )", varid1))
                        break;
                    tok2 = tok2->next();
                }
                if (tok2 != tok1)
                    continue;
            }

            unsigned int indentlevel2 = 0;
            for (const Token *tok2 = tok1->tokAt(3); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel2;

                else if (tok2->str() == "}")
                {
                    if (indentlevel2 == 0)
                        break;
                    --indentlevel2;
                }

                // goto destination..
                else if (tok2->isName() && Token::simpleMatch(tok2->next(), ":"))
                    break;

                // Reassignment of the struct
                else if (tok2->varId() == varid1)
                {
                    if (tok2->next()->str() == "=")
                        break;
                    if (Token::Match(tok2->tokAt(-2), "[,(] &"))
                        break;
                }

                // Loop..
                /** @todo don't bail out if the variable is not used in the loop */
                else if (tok2->str() == "do")
                    break;

                // return at base level => stop checking
                else if (indentlevel2 == 0 && tok2->str() == "return")
                    break;

                else if (Token::Match(tok2, "if ( !| %varid% )", varid1))
                {
                    // Is this variable a pointer?
                    const Token *tempTok = Token::findmatch(_tokenizer->tokens(), "%type% * %varid% [;)=]", varid1);
                    if (tempTok)
                        nullPointerError(tok1, varname, tok2->linenr());
                    break;
                }
            }
        }
    }

}

void CheckOther::nullPointerByDeRefAndChec()
{
    // Dereferencing a pointer and then checking if it's NULL..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "if" && Token::Match(tok->previous(), "; if ( ! %var% )"))
        {
            const unsigned int varid(tok->tokAt(3)->varId());
            if (varid == 0)
                continue;

            const std::string varname(tok->strAt(3));

            const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid%", varid);
            if (!Token::Match(decltok->tokAt(-3), "[;,(] %var% *"))
                continue;

            for (const Token *tok1 = tok->previous(); tok1 && tok1 != decltok; tok1 = tok1->previous())
            {
                if (tok1->varId() == varid)
                {
                    if (Token::Match(tok1->tokAt(-2), "[=;{}] *"))
                    {
                        nullPointerError(tok1, varname);
                        break;
                    }
                    else if (tok1->previous() && tok1->previous()->str() == "&")
                    {
                        break;
                    }
                    else if (tok1->next() && tok1->next()->str() == "=")
                    {
                        break;
                    }
                }

                else if (tok1->str() == "{" ||
                         tok1->str() == "}")
                    break;

                // goto destination..
                else if (tok1->isName() && Token::simpleMatch(tok1->next(), ":"))
                    break;
            }
        }
    }
}


void CheckOther::nullPointer()
{
    nullPointerAfterLoop();
    nullPointerLinkedList();
    nullPointerStructByDeRefAndChec();
    nullPointerByDeRefAndChec();
}



/**
 * \brief parse a function call and extract information about variable usage
 * \param tok first token
 * \param var variables that the function read / write.
 * \param value 0 => invalid with null pointers as parameter.
 *              1-.. => invalid with uninitialized data.
 */
static void parseFunctionCall(const Token &tok, std::list<const Token *> &var, unsigned char value)
{
    // standard functions that dereference first parameter..
    // both uninitialized data and null pointers are invalid.
    static std::set<std::string> functionNames1;
    if (functionNames1.empty())
    {
        functionNames1.insert("memchr");
        functionNames1.insert("memcmp");
        functionNames1.insert("strcat");
        functionNames1.insert("strncat");
        functionNames1.insert("strchr");
        functionNames1.insert("strrchr");
        functionNames1.insert("strcmp");
        functionNames1.insert("strncmp");
        functionNames1.insert("strdup");
        functionNames1.insert("strlen");
        functionNames1.insert("strstr");
    }

    // standard functions that dereference second parameter..
    // both uninitialized data and null pointers are invalid.
    static std::set<std::string> functionNames2;
    if (functionNames2.empty())
    {
        functionNames2.insert("memcmp");
        functionNames2.insert("memcpy");
        functionNames2.insert("memmove");
        functionNames2.insert("strcat");
        functionNames2.insert("strncat");
        functionNames2.insert("strcmp");
        functionNames2.insert("strncmp");
        functionNames2.insert("strcpy");
        functionNames2.insert("strncpy");
        functionNames2.insert("strstr");
    }

    // 1st parameter..
    if (Token::Match(&tok, "%var% ( %var% ,|)") && tok.tokAt(2)->varId() > 0)
    {
        if (functionNames1.find(tok.str()) != functionNames1.end())
            var.push_back(tok.tokAt(2));
        else if (value == 0 && Token::Match(&tok, "memchr|memcmp|memcpy|memmove|memset|strcpy|printf|sprintf|snprintf"))
            var.push_back(tok.tokAt(2));
        else if (Token::simpleMatch(&tok, "fflush"))
            var.push_back(tok.tokAt(2));
    }

    // 2nd parameter..
    if (Token::Match(&tok, "%var% ( %any% , %var% ,|)") && tok.tokAt(4)->varId() > 0)
    {
        if (functionNames2.find(tok.str()) != functionNames2.end())
            var.push_back(tok.tokAt(4));
    }
}


/// @addtogroup Checks
/// @{


/**
 * @brief %Check for null pointer usage (using ExecutionPath)
 */

class CheckNullpointer : public ExecutionPath
{
public:
    /** Startup constructor */
    CheckNullpointer(Check *c) : ExecutionPath(c, 0), null(false)
    {
    }

private:
    /** Create checking of specific variable: */
    CheckNullpointer(Check *c, const unsigned int id, const std::string &name)
            : ExecutionPath(c, id),
            varname(name),
            null(false)
    {
    }

    /** Copy this check */
    ExecutionPath *copy()
    {
        return new CheckNullpointer(*this);
    }

    /** no implementation => compiler error if used by accident */
    void operator=(const CheckNullpointer &);

    /** variable name for this check (empty => dummy check) */
    const std::string varname;

    /** is this variable null? */
    bool null;

    /** variable is set to null */
    static void setnull(std::list<ExecutionPath *> &checks, const unsigned int varid)
    {
        std::list<ExecutionPath *>::iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckNullpointer *c = dynamic_cast<CheckNullpointer *>(*it);
            if (c && c->varId == varid)
                c->null = true;
        }
    }

    /** variable is dereferenced. If the variable is null there's an error */
    static void dereference(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());

        std::list<ExecutionPath *>::iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckNullpointer *c = dynamic_cast<CheckNullpointer *>(*it);
            if (c && c->varId == varid && c->null)
            {
                foundError = true;
                CheckOther *checkOther = dynamic_cast<CheckOther *>(c->owner);
                if (checkOther)
                {
                    checkOther->nullPointerError(tok, c->varname);
                    break;
                }
            }
        }
    }

    /** parse tokens */
    const Token *parse(const Token &tok, bool &foundError, std::list<ExecutionPath *> &checks) const
    {
        if (Token::Match(tok.previous(), "[;{}] %type% * %var% ;"))
        {
            const Token * vartok = tok.tokAt(2);
            if (vartok->varId() != 0)
                checks.push_back(new CheckNullpointer(owner, vartok->varId(), vartok->str()));
            return vartok->next();
        }

        // Template pointer variable..
        if (Token::Match(tok.previous(), "[;{}] %type% ::|<"))
        {
            const Token * vartok = &tok;
            while (Token::Match(vartok, "%type% ::"))
                vartok = vartok->tokAt(2);
            if (Token::Match(vartok, "%type% < %type%"))
            {
                vartok = vartok->tokAt(3);
                while (vartok && (vartok->str() == "*" || vartok->isName()))
                    vartok = vartok->next();
                if (Token::Match(vartok, "> * %var% ;|="))
                {
                    vartok = vartok->tokAt(2);
                    checks.push_back(new CheckNullpointer(owner, vartok->varId(), vartok->str()));
                    if (Token::simpleMatch(vartok->next(), "= 0 ;"))
                        setnull(checks, vartok->varId());
                    return vartok->next();
                }
            }
        }


        if (Token::Match(&tok, "%var% ("))
        {
            if (tok.str() == "sizeof")
                return tok.next()->link();

            // parse usage..
            std::list<const Token *> var;
            parseFunctionCall(tok, var, 0);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                dereference(foundError, checks, *it);
        }

        if (tok.varId() != 0)
        {
            if (Token::Match(tok.previous(), "[;{}=] %var% = 0 ;"))
                setnull(checks, tok.varId());
            else if (Token::Match(tok.tokAt(-2), "[;{}=+-/(,] * %var%"))
                dereference(foundError, checks, &tok);
            else if (Token::Match(tok.tokAt(-2), "return * %var%"))
                dereference(foundError, checks, &tok);
            else if (!Token::simpleMatch(tok.tokAt(-2), "& (") && Token::Match(tok.next(), ". %var%"))
                dereference(foundError, checks, &tok);
            else if (Token::Match(tok.previous(), "[;{}=+-/(,] %var% [ %any% ]"))
                dereference(foundError, checks, &tok);
            else if (Token::Match(tok.previous(), "return %var% [ %any% ]"))
                dereference(foundError, checks, &tok);
            else if (Token::Match(&tok, "%var% ("))
                dereference(foundError, checks, &tok);
            else
                bailOutVar(checks, tok.varId());
        }

        if (Token::simpleMatch(&tok, "* 0"))
        {
            if (Token::Match(tok.previous(), "[;{}=+-/(,]") ||
                Token::Match(tok.previous(), "return|<<"))
            {
                CheckOther *checkOther = dynamic_cast<CheckOther *>(owner);
                if (checkOther)
                {
                    checkOther->nullPointerError(&tok);
                    foundError = true;
                }
            }
        }

        return &tok;
    }

    /** parse condition. @sa ExecutionPath::parseCondition */
    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        if (Token::Match(&tok, "!| %var% ("))
        {
            bool foundError = false;
            std::list<const Token *> var;
            parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 0);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                dereference(foundError, checks, *it);
        }

        return ExecutionPath::parseCondition(tok, checks);
    }
};


/**
 * @brief %Check that uninitialized variables aren't used (using ExecutionPath)
 * */
class CheckUninitVar : public ExecutionPath
{
public:
    /** Startup constructor */
    CheckUninitVar(Check *c)
            : ExecutionPath(c, 0), pointer(false), array(false), alloc(false), strncpy_(false)
    {
    }

private:
    /** Create a copy of this check */
    ExecutionPath *copy()
    {
        return new CheckUninitVar(*this);
    }

    /** no implementation => compiler error if used */
    void operator=(const CheckUninitVar &);

    /** internal constructor for creating extra checks */
    CheckUninitVar(Check *c, unsigned int v, const std::string &name, bool p, bool a)
            : ExecutionPath(c, v), varname(name), pointer(p), array(a), alloc(false), strncpy_(false)
    {
    }

    /** variable name for this check */
    const std::string varname;

    /** is this variable a pointer? */
    const bool pointer;

    /** is this variable an array? */
    const bool array;

    /** is this variable allocated? */
    bool  alloc;

    /** is this variable initialized with strncpy (not always zero-terminated) */
    bool  strncpy_;

    /** allocating pointer. For example : p = malloc(10); */
    static void alloc_pointer(std::list<ExecutionPath *> &checks, unsigned int varid)
    {
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid)
                c->alloc = true;
        }
    }

    /** Initializing a pointer value. For example: *p = 0; */
    static void init_pointer(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::iterator it = checks.begin();
        while (it != checks.end())
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid)
            {
                if (c->alloc || c->array)
                {
                    delete c;
                    checks.erase(it++);
                    continue;
                }
                else
                {
                    use_pointer(foundError, checks, tok);
                }
            }

            ++it;
        }
    }

    /** Deallocate a pointer. For example: free(p); */
    static void dealloc_pointer(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid)
            {
                if (c->pointer && !c->alloc)
                {
                    CheckOther *checkOther = dynamic_cast<CheckOther *>(c->owner);
                    if (checkOther)
                    {
                        foundError = true;
                        checkOther->uninitvarError(tok, c->varname);
                        break;
                    }
                }
                c->alloc = false;
            }
        }
    }

    /**
     * Pointer assignment:  p = x;
     * if p is a pointer and x is an array/pointer then bail out
     * \param checks all available checks
     * \param tok1 the "p" token
     * \param tok2 the "x" token
     */
    static void pointer_assignment(std::list<ExecutionPath *> &checks, const Token *tok1, const Token *tok2)
    {
        const unsigned int varid1(tok1->varId());
        if (varid1 == 0)
            return;

        const unsigned int varid2(tok2->varId());
        if (varid2 == 0)
            return;

        std::list<ExecutionPath *>::const_iterator it;

        // bail out if first variable is a pointer
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid1 && c->pointer)
            {
                bailOutVar(checks, varid1);
                break;
            }
        }

        // bail out if second variable is a array/pointer
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid2 && (c->pointer || c->array))
            {
                bailOutVar(checks, varid2);
                break;
            }
        }
    }


    /** Initialize an array with strncpy.. */
    static void init_strncpy(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());
        if (!varid)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid)
            {
                c->strncpy_ = true;
            }
        }
    }



    /**
     * use - called from the use* functions below.
     * @param foundError this is set to true if an error is found
     * @param checks all available checks
     * @param tok variable token
     * @param mode specific behaviour
     */
    static void use(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok, const int mode)
    {
        const unsigned int varid(tok->varId());
        if (varid == 0)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
            if (c && c->varId == varid)
            {
                // mode 0 : the variable is used "directly"
                // example: .. = var;
                // it is ok to read the address of an uninitialized array.
                // it is ok to read the address of an allocated pointer
                if (mode == 0 && (c->array || (c->pointer && c->alloc)))
                    continue;

                // mode 2 : bad usage of pointer. if it's not a pointer then the usage is ok.
                // example: ptr->foo();
                if (mode == 2 && !c->pointer)
                    continue;

                // mode 3 : using dead pointer is invalid.
                if (mode == 3 && (!c->pointer || c->alloc))
                    continue;

                CheckOther *checkOther = dynamic_cast<CheckOther *>(c->owner);
                if (checkOther)
                {
                    if (c->strncpy_)
                        checkOther->uninitstringError(tok, c->varname);
                    else if (c->pointer && c->alloc)
                        checkOther->uninitdataError(tok, c->varname);
                    else
                        checkOther->uninitvarError(tok, c->varname);
                    foundError = true;
                    break;
                }
            }
        }
    }

    /**
     * Reading variable. Use this function in situations when it is
     * invalid to read the data of the variable but not the address.
     * @param foundError this is set to true if an error is found
     * @param checks all available checks
     * @param tok variable token
     */
    static void use(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(foundError, checks, tok, 0);
    }

    /**
     * Reading array elements. If the variable is not an array then the usage is ok.
     * @param foundError this is set to true if an error is found
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_array(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(foundError, checks, tok, 1);
    }

    /**
     * Bad pointer usage. If the variable is not a pointer then the usage is ok.
     * @param foundError this is set to true if an error is found
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_pointer(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(foundError, checks, tok, 2);
    }

    /**
     * Using variable.. if it's a dead pointer the usage is invalid.
     * @param foundError this is set to true if an error is found
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_dead_pointer(bool &foundError, std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(foundError, checks, tok, 3);
    }

    /** declaring a variable */
    void declare(std::list<ExecutionPath *> &checks, const Token *vartok, const Token &tok, const bool p, const bool a) const
    {
        if (vartok->varId() == 0)
            return;

        bool isenum = false;
        if (!tok.isStandardType())
        {
            const std::string pattern("enum " + tok.str());
            for (const Token *tok2 = tok.previous(); tok2; tok2 = tok2->previous())
            {
                if (tok2->str() != "{")
                    continue;
                if (Token::simpleMatch(tok2->tokAt(-2), pattern.c_str()))
                {
                    isenum = true;
                    break;
                }
            }
        }

        // Suppress warnings if variable in inner scope has same name as variable in outer scope
        if (!tok.isStandardType() && !isenum)
        {
            std::set<unsigned int> dup;
            for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it)
            {
                CheckUninitVar *c = dynamic_cast<CheckUninitVar *>(*it);
                if (c && c->varname == vartok->str() && c->varId != vartok->varId())
                    dup.insert(c->varId);
            }
            if (!dup.empty())
            {
                for (std::set<unsigned int>::const_iterator it = dup.begin(); it != dup.end(); ++it)
                    bailOutVar(checks, *it);
                return;
            }
        }

        if (a || p || tok.isStandardType() || isenum)
            checks.push_back(new CheckUninitVar(owner, vartok->varId(), vartok->str(), p, a));
    }

    /** parse tokens. @sa ExecutionPath::parse */
    const Token *parse(const Token &tok, bool &foundError, std::list<ExecutionPath *> &checks) const
    {
        // Variable declaration..
        if (tok.str() != "return")
        {
            if (Token::Match(&tok, "enum %type% {"))
                return tok.tokAt(2)->link();

            if (Token::Match(tok.previous(), "[;{}] %type% *| %var% ;"))
            {
                const Token * vartok = tok.next();
                const bool p(vartok->str() == "*");
                if (p)
                    vartok = vartok->next();
                declare(checks, vartok, tok, p, false);
                return vartok->next();
            }

            // Variable declaration for array..
            if (Token::Match(tok.previous(), "[;{}] %type% %var% [ %num% ] ;"))
            {
                const Token * vartok = tok.next();
                declare(checks, vartok, tok, false, true);
                return vartok->next()->link()->next();
            }

            // Template pointer variable..
            if (Token::Match(tok.previous(), "[;{}] %type% ::|<"))
            {
                const Token * vartok = &tok;
                while (Token::Match(vartok, "%type% ::"))
                    vartok = vartok->tokAt(2);
                if (Token::Match(vartok, "%type% < %type%"))
                {
                    vartok = vartok->tokAt(3);
                    while (vartok && (vartok->str() == "*" || vartok->isName()))
                        vartok = vartok->next();
                    if (Token::Match(vartok, "> * %var% ;"))
                    {
                        declare(checks, vartok->tokAt(2), tok, true, false);
                        return vartok->tokAt(2);
                    }
                }
            }
        }

        if (tok.varId())
        {
            // Used..
            if (Token::Match(tok.previous(), "[[(,+-*/] %var% []),+-*/]"))
            {
                use(foundError, checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "++|--") || Token::Match(tok.next(), "++|--"))
            {
                use(foundError, checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "[;{}] %var% ="))
            {
                // using same variable rhs?
                for (const Token *tok2 = tok.tokAt(2); tok2; tok2 = tok2->next())
                {
                    if (Token::Match(tok2, ";|)|="))
                        break;
                    if (Token::Match(tok2, "%var% ("))
                        break;
                    if (tok2->varId() &&
                        !Token::Match(tok2->previous(), "&|::") &&
                        !Token::simpleMatch(tok2->next(), "="))
                        use(foundError, checks, tok2);
                }

                // pointer aliasing?
                if (Token::Match(tok.tokAt(2), "%var% ;"))
                {
                    pointer_assignment(checks, &tok, tok.tokAt(2));
                }
            }

            if (Token::simpleMatch(tok.next(), "("))
            {
                use_pointer(foundError, checks, &tok);
            }

            if (Token::Match(tok.tokAt(-2), "[;{}] *"))
            {
                if (Token::simpleMatch(tok.next(), "="))
                    init_pointer(foundError, checks, &tok);
                else
                    use_pointer(foundError, checks, &tok);
                return &tok;
            }

            // += etc
            if (Token::Match(tok.previous(), "[;{}]") || Token::Match(tok.tokAt(-2), "[;{}] *"))
            {
                // goto the equal..
                const Token *eq = tok.next();
                if (eq && eq->str() == "[" && eq->link() && eq->link()->next())
                    eq = eq->link()->next();

                // is it X=
                if (Token::Match(eq, "+=|-=|*=|/=|&=|^=") || eq->str() == "|=")
                {
                    if (tok.previous()->str() == "*")
                        use_pointer(foundError, checks, &tok);
                    else if (tok.next()->str() == "[")
                        use_array(foundError, checks, &tok);
                    else
                        use(foundError, checks, &tok);
                }
            }

            if (Token::Match(tok.next(), "= malloc|kmalloc") || Token::simpleMatch(tok.next(), "= new char ["))
            {
                alloc_pointer(checks, tok.varId());
                if (tok.tokAt(3)->str() == "(")
                    return tok.tokAt(3)->link();
            }

            else if (Token::simpleMatch(tok.previous(), ">>") || Token::simpleMatch(tok.next(), "="))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::simpleMatch(tok.next(), "["))
            {
                const Token *tok2 = tok.next()->link();
                if (Token::simpleMatch(tok2 ? tok2->next() : 0, "="))
                {
                    ExecutionPath::bailOutVar(checks, tok.varId());
                    return &tok;
                }
            }

            if (Token::simpleMatch(tok.previous(), "delete") ||
                Token::simpleMatch(tok.tokAt(-3), "delete [ ]"))
            {
                dealloc_pointer(foundError, checks, &tok);
                return &tok;
            }
        }

        if (Token::Match(&tok, "%var% (") && uvarFunctions.find(tok.str()) == uvarFunctions.end())
        {
            if (Token::simpleMatch(&tok, "sizeof ("))
                return tok.next()->link();

            // deallocate pointer
            if (Token::Match(&tok, "free|kfree|fclose ( %var% )"))
            {
                dealloc_pointer(foundError, checks, tok.tokAt(2));
                return tok.tokAt(3);
            }

            // parse usage..
            {
                std::list<const Token *> var;
                parseFunctionCall(tok, var, 1);
                for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                    use_array(foundError, checks, *it);
            }

            // strncpy doesn't 0-terminate first parameter
            if (Token::Match(&tok, "strncpy ( %var% ,"))
            {
                init_strncpy(checks, tok.tokAt(2));
                return tok.next()->link();
            }

            if (Token::Match(&tok, "asm ( )"))
            {
                ExecutionPath::bailOut(checks);
                return &tok;
            }

            // is the variable passed as a parameter to some function?
            unsigned int parlevel = 0;
            std::set<unsigned int> bailouts;
            for (const Token *tok2 = tok.next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }

                else if (Token::simpleMatch(tok2, "sizeof ("))
                {
                    tok2 = tok2->next()->link();
                    if (!tok2)
                        break;
                }

                else if (tok2->varId())
                {
                    if (Token::Match(tok2->tokAt(-2), "[(,] *") || Token::Match(tok2->next(), ". %var%"))
                        use_dead_pointer(foundError, checks, tok2);

                    // it is possible that the variable is initialized here
                    bailouts.insert(tok2->varId());
                }
            }

            for (std::set<unsigned int>::const_iterator it = bailouts.begin(); it != bailouts.end(); ++it)
                ExecutionPath::bailOutVar(checks, *it);
        }

        // function call via function pointer
        if (Token::Match(&tok, "( * %var% ) ("))
        {
            // is the variable passed as a parameter to some function?
            unsigned int parlevel = 0;
            for (const Token *tok2 = tok.link()->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }

                else if (tok2->varId())
                {
                    // it is possible that the variable is initialized here
                    ExecutionPath::bailOutVar(checks, tok2->varId());
                }
            }
        }

        if (tok.str() == "return")
        {
            // Todo: if (!array && ..
            if (Token::Match(tok.next(), "%var% ;"))
            {
                use(foundError, checks, tok.next());
            }
        }

        if (tok.varId())
        {
            if (Token::simpleMatch(tok.previous(), "="))
            {
                if (Token::Match(tok.tokAt(-3), "& %var% ="))
                {
                    bailOutVar(checks, tok.varId());
                    return &tok;
                }

                if (!Token::Match(tok.tokAt(-3), ". %var% ="))
                {
                    if (!Token::Match(tok.tokAt(-3), "[;{}] %var% ="))
                    {
                        use(foundError, checks, &tok);
                        return &tok;
                    }

                    const unsigned int varid2 = tok.tokAt(-2)->varId();
                    if (varid2)
                    {
                        /*
                        const Token *tok2 = Token::findmatch(owner->_tokenizer->tokens(), "%varid%", varid2);
                        if (tok2 && !Token::simpleMatch(tok2->previous(), "*"))
                        */
                        {
                            use(foundError, checks, &tok);
                            return &tok;
                        }
                    }
                }
            }

            if (Token::simpleMatch(tok.next(), "."))
            {
                const Token *tok2 = tok.next();
                while (Token::Match(tok2, ". %var%"))
                    tok2 = tok2->tokAt(2);
                if (tok2 && tok2->str() != "=")
                    use_pointer(foundError, checks, &tok);
                else
                    bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::simpleMatch(tok.next(), "["))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
                return &tok;
            }

            if (Token::Match(tok.tokAt(-2), "[,(=] *"))
            {
                use_pointer(foundError, checks, &tok);
                return &tok;
            }

            if (Token::simpleMatch(tok.previous(), "&"))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
            }
        }
        return &tok;
    }

    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        bool foundError = false;

        if (tok.varId() && Token::Match(&tok, "%var% <|<=|==|!=|)|["))
            use(foundError, checks, &tok);

        else if (Token::Match(&tok, "!| %var% ("))
        {
            std::list<const Token *> var;
            parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 1);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                use_array(foundError, checks, *it);
        }

        else if (Token::Match(&tok, "! %var% )"))
        {
            use(foundError, checks, &tok);
            return false;
        }

        return ExecutionPath::parseCondition(tok, checks);
    }

public:

    /** Functions that don't handle uninitialized variables well */
    static std::set<std::string> uvarFunctions;

    static void analyseFunctions(const Token * const tokens, std::set<std::string> &func)
    {
        for (const Token *tok = tokens; tok; tok = tok->next())
        {
            if (tok->str() == "{")
            {
                tok = tok->link();
                continue;
            }
            if (tok->str() != "::" && Token::Match(tok->next(), "%var% ( %type%"))
            {
                if (!Token::simpleMatch(tok->tokAt(2)->link(), ") {"))
                    continue;
                const Token *tok2 = tok->tokAt(3);
                while (tok2 && tok2->str() != ")")
                {
                    if (tok2->str() == ",")
                        tok2 = tok2->next();

                    if (Token::Match(tok2, "%type% %var% ,|)") && tok2->isStandardType())
                    {
                        tok2 = tok2->tokAt(2);
                        continue;
                    }

                    if (Token::Match(tok2, "const %type% %var% ,|)") && tok2->next()->isStandardType())
                    {
                        tok2 = tok2->tokAt(2);
                        continue;
                    }

                    break;
                }

                // found simple function..
                if (tok2->link() == tok->tokAt(2))
                    func.insert(tok->next()->str());
            }
        }
    }
};

/** Functions that don't handle uninitialized variables well */
std::set<std::string> CheckUninitVar::uvarFunctions;


/// @}


void CheckOther::analyseFunctions(const Token * const tokens, std::set<std::string> &func)
{
    CheckUninitVar::analyseFunctions(tokens, func);
}



void CheckOther::executionPaths()
{
    // Check for null pointer errors..
    {
        CheckNullpointer c(this);
        checkExecutionPaths(_tokenizer->tokens(), &c);
    }

    // check if variable is accessed uninitialized..
    {
        // no writing if multiple threads are used (TODO: thread safe analysis?)
        if (_settings->_jobs == 1)
            CheckUninitVar::analyseFunctions(_tokenizer->tokens(), CheckUninitVar::uvarFunctions);

        CheckUninitVar c(this);
        checkExecutionPaths(_tokenizer->tokens(), &c);
    }
}

void CheckOther::checkZeroDivision()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {

        if (Token::Match(tok, "/ %num%") &&
            MathLib::isInt(tok->next()->str()) &&
            MathLib::toLongNumber(tok->next()->str()) == 0L)
        {
            zerodivError(tok);
        }
        else if (Token::Match(tok, "div|ldiv|lldiv|imaxdiv ( %num% , %num% )") &&
                 MathLib::isInt(tok->tokAt(4)->str()) &&
                 MathLib::toLongNumber(tok->tokAt(4)->str()) == 0L)
        {
            zerodivError(tok);
        }
    }
}



void CheckOther::postIncrement()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "for ("))
        {
            const Token *tok2 = tok->next()->link();
            if (tok2)
                tok2 = tok2->tokAt(-3);
            if (Token::Match(tok2, "; %var% ++|-- )"))
            {
                if (tok2->next()->varId() == 0)
                    continue;

                // Take a look at the variable declaration
                const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid%", tok2->tokAt(1)->varId());
                const std::string classDef = std::string("class ") + std::string(decltok->previous()->strAt(0));

                // Is the variable an iterator?
                if (decltok && Token::Match(decltok->previous(), "iterator|const_iterator"))
                    postIncrementError(tok2, tok2->strAt(1), (std::string("++") == tok2->strAt(2)));
                // Is the variable a class?
                else if (Token::findmatch(_tokenizer->tokens(), classDef.c_str()))
                    postIncrementError(tok2, tok2->strAt(1), (std::string("++") == tok2->strAt(2)));
            }
        }
    }
}




void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast", "C-style pointer casting");
}

void CheckOther::redundantIfDelete0Error(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfDelete0", "Redundant condition. It is safe to deallocate a NULL pointer");
}

void CheckOther::redundantIfRemoveError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfRemove", "Redundant condition. The remove function in the STL will not do anything if element doesn't exist");
}

void CheckOther::dangerousUsageStrtolError(const Token *tok)
{
    reportError(tok, Severity::error, "dangerousUsageStrtol", "Invalid radix in call to strtol or strtoul. Must be 0 or 2-36");
}

void CheckOther::sprintfOverlappingDataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "sprintfOverlappingData", "Undefined behaviour: " + varname + " is used wrong in call to sprintf or snprintf. Quote: If copying takes place between objects that overlap as a result of a call to sprintf() or snprintf(), the results are undefined.");
}

void CheckOther::udivError(const Token *tok)
{
    reportError(tok, Severity::error, "udivError", "Unsigned division. The result will be wrong.");
}

void CheckOther::udivWarning(const Token *tok)
{
    reportError(tok, Severity::possibleStyle, "udivWarning", "Division with signed and unsigned operators");
}

void CheckOther::unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedStructMember", "struct or union member '" + structname + "::" + varname + "' is never used");
}

void CheckOther::passedByValueError(const Token *tok, const std::string &parname)
{
    reportError(tok, Severity::style, "passedByValue", "Function parameter '" + parname + "' is passed by value. It could be passed by reference instead.");
}

void CheckOther::constStatementError(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::style, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant");
}

void CheckOther::charArrayIndexError(const Token *tok)
{
    reportError(tok, Severity::style, "charArrayIndex", "Warning - using char variable as array index");
}

void CheckOther::charBitOpError(const Token *tok)
{
    reportError(tok, Severity::style, "charBitOp", "Warning - using char variable in bit operation");
}

void CheckOther::variableScopeError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "variableScope", "The scope of the variable " + varname + " can be reduced");
}

void CheckOther::conditionAlwaysTrueFalse(const Token *tok, const std::string &truefalse)
{
    reportError(tok, Severity::style, "conditionAlwaysTrueFalse", "Condition is always " + truefalse);
}

void CheckOther::strPlusChar(const Token *tok)
{
    reportError(tok, Severity::error, "strPlusChar", "Unusual pointer arithmetic");
}

void CheckOther::nullPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "nullPointer", "Null pointer dereference");
}

void CheckOther::nullPointerError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname);
}

void CheckOther::nullPointerError(const Token *tok, const std::string &varname, const int line)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname + " - otherwise it is redundant to check if " + varname + " is null at line " + MathLib::toString<long>(line));
}

void CheckOther::uninitstringError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitstring", "Dangerous usage of '" + varname + "' (strncpy doesn't always 0-terminate it)");
}

void CheckOther::uninitdataError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitdata", "Data is allocated but not initialized: " + varname);
}

void CheckOther::uninitvarError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error, "uninitvar", "Uninitialized variable: " + varname);
}

void CheckOther::zerodivError(const Token *tok)
{
    reportError(tok, Severity::error, "zerodiv", "Division by zero");
}

void CheckOther::postIncrementError(const Token *tok, const std::string &var_name, const bool isIncrement)
{
    std::string type = (isIncrement ? "Incrementing" : "Decrementing");
    reportError(tok, Severity::possibleStyle, "postIncrementDecrement", ("Pre-" + type + " variable '" + var_name + "' is preferred to Post-" + type));
}
