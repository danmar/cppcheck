/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include "executionpath.h"

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <cstring>
#include <cctype>
#include <memory>
#include <cmath> // fabs()
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckOther instance;
}

//---------------------------------------------------------------------------



void CheckOther::warningOldStylePointerCast()
{
    if (!_settings->_checkCodingStyle ||
        (_tokenizer->tokens() && _tokenizer->fileLine(_tokenizer->tokens()).find(".cpp") == std::string::npos))
        return;

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
    if (!_settings->_checkCodingStyle)
        return;

    // if (p) delete p
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (! Token::simpleMatch(tok, "if ("))
            continue;

        const Token *tok2 = tok->tokAt(2);

        /*
         * Possible if-constructions:
         *
         *   if (var)
         *   if (this->var)
         *   if (Foo::var)
         *
         **/
        std::string varname = concatNames(&tok2);

        if (!Token::Match(tok2, "%var% ) {"))
            continue;

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

        std::string varname2 = concatNames(&tok2);

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

        /*
         * Possible constructions:
         *
         * - if (%var%) { delete %var%; }
         * - if (%var%) { delete %var%; %var% = 0; }
         *
         **/
        if (Token::Match(tok2, "; } !!else"))
        {
            redundantIfDelete0Error(tok);
        }
        else if (Token::Match(tok2, "; %var%"))
        {
            tok2 = tok2->next();
            std::string varname3 = concatNames(&tok2);
            if (Token::Match(tok2, "%var% = 0 ; } !!else") && varname2 == varname3)
            {
                redundantIfDelete0Error(tok);
            }
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
// "if (strlen(s))" can be rewritten as "if (*s != '\0')"
//---------------------------------------------------------------------------
void CheckOther::checkEmptyStringTest()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Non-empty string tests
        if (Token::Match(tok, "if ( strlen ( %any% ) )"))
        {
            emptyStringTestError(tok, tok->strAt(4), false);
        }
        else if (Token::Match(tok, "strlen ( %any% ) !=|> 0"))
        {
            emptyStringTestError(tok, tok->strAt(2), false);
        }
        else if (Token::Match(tok, "0 < strlen ( %any% )"))
        {
            emptyStringTestError(tok, tok->strAt(4), false);
        }

        // Empty string tests
        else if (Token::Match(tok, "! strlen ( %any% )"))
        {
            emptyStringTestError(tok, tok->strAt(3), true);
        }
        else if (Token::Match(tok, "strlen ( %any% ) == 0"))
        {
            emptyStringTestError(tok, tok->strAt(2), true);
        }
    }
}
//---------------------------------------------------------------------------
// fflush(stdin) <- fflush only applies to output streams in ANSI C
//---------------------------------------------------------------------------
void CheckOther::checkFflushOnInputStream()
{
    const Token *tok = _tokenizer->tokens();
    while (tok && ((tok = Token::findmatch(tok, "fflush ( stdin )")) != NULL))
    {
        fflushOnInputStreamError(tok, tok->strAt(2));
        tok = tok->tokAt(4);
    }
}

//---------------------------------------------------------------------------
//    switch (x)
//    {
//        case 2:
//            y = a;        // <- this assignment is redundant
//        case 3:
//            y = b;        // <- case 2 falls through and sets y twice
//    }
//---------------------------------------------------------------------------
void CheckOther::checkRedundantAssignmentInSwitch()
{
    const char switchPattern[] = "switch ( %any% ) { case";
    const char breakPattern[] = "break|return|exit|goto";
    const char functionPattern[] = "%var% (";

    // Find the beginning of a switch. E.g.:
    //   switch (var) { ...
    const Token *tok = Token::findmatch(_tokenizer->tokens(), switchPattern);
    while (tok)
    {

        // Check the contents of the switch statement
        std::map<unsigned int, const Token*> varsAssigned;
        int indentLevel = 0;
        for (const Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
            {
                // Inside a conditional or loop. Don't mark variable accesses as being redundant. E.g.:
                //   case 3: b = 1;
                //   case 4: if (a) { b = 2; }    // Doesn't make the b=1 redundant because it's conditional
                if (Token::Match(tok2->previous(), ")|else {") && tok2->link())
                {
                    const Token* endOfConditional = tok2->link();
                    for (const Token* tok3 = tok2; tok3 != endOfConditional; tok3 = tok3->next())
                    {
                        if (tok3->varId() != 0)
                            varsAssigned.erase(tok3->varId());
                        else if (Token::Match(tok3, functionPattern) || Token::Match(tok3, breakPattern))
                            varsAssigned.clear();
                    }
                    tok2 = endOfConditional;
                }
                else
                    ++ indentLevel;
            }
            else if (tok2->str() == "}")
            {
                -- indentLevel;

                // End of the switch block
                if (indentLevel < 0)
                    break;
            }

            // Variable assignment. Report an error if it's assigned to twice before a break. E.g.:
            //    case 3: b = 1;    // <== redundant
            //    case 4: b = 2;
            if (Token::Match(tok2->previous(), ";|{|}|: %var% = %any% ;") && tok2->varId() != 0)
            {
                std::map<unsigned int, const Token*>::iterator i = varsAssigned.find(tok2->varId());
                if (i == varsAssigned.end())
                    varsAssigned[tok2->varId()] = tok2;
                else
                    redundantAssignmentInSwitchError(i->second, i->second->str());
            }
            // Not a simple assignment so there may be good reason if this variable is assigned to twice. E.g.:
            //    case 3: b = 1;
            //    case 4: b++;
            else if (tok2->varId() != 0)
                varsAssigned.erase(tok2->varId());

            // Reset our record of assignments if there is a break or function call. E.g.:
            //    case 3: b = 1; break;
            if (Token::Match(tok2, functionPattern) || Token::Match(tok2, breakPattern))
                varsAssigned.clear();

        }

        tok = Token::findmatch(tok->next(), switchPattern);
    }
}

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

void CheckOther::invalidScanf()
{
    if (!_settings->_checkCodingStyle)
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        const Token *formatToken = 0;
        if (Token::Match(tok, "scanf|vscanf ( %str% ,"))
            formatToken = tok->tokAt(2);
        else if (Token::Match(tok, "fscanf|vfscanf ( %var% , %str% ,"))
            formatToken = tok->tokAt(4);
        else
            continue;

        bool format = false;

        // scan the string backwards, so we dont need to keep states
        const std::string &formatstr(formatToken->str());
        for (unsigned int i = 1; i < formatstr.length(); i++)
        {
            if (formatstr[i] == '%')
                format = !format;

            else if (!format)
                continue;

            else if (std::isdigit(formatstr[i]))
            {
                format = false;
            }

            else if (std::isalpha(formatstr[i]))
            {
                invalidScanfError(tok);
                format = false;
            }
        }
    }
}

void CheckOther::invalidScanfError(const Token *tok)
{
    reportError(tok, Severity::style,
                "invalidscanf", "scanf without field width limits can crash with huge input data\n"
                "To fix this error message add a field width specifier:\n"
                "    %s => %20s\n"
                "    %i => %3i\n"
                "\n"
                "Sample program that can crash:\n"
                "\n"
                "#include <stdio.h>\n"
                "int main()\n"
                "{\n"
                "    int a;\n"
                "    scanf(\"%i\", &a);\n"
                "    return 0;\n"
                "}\n"
                "\n"
                "To make it crash:\n"
                "perl -e 'print \"5\"x2100000' | ./a.out");
}

//---------------------------------------------------------------------------
// Check for unsigned divisions
//---------------------------------------------------------------------------

void CheckOther::checkUnsignedDivision()
{
    if (!_settings->_checkCodingStyle)
        return;

    // Check for "ivar / uvar" and "uvar / ivar"
    std::map<unsigned int, char> varsign;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "[{};(,] %type% %var% [;=,)]"))
        {
            if (tok->tokAt(1)->isUnsigned())
                varsign[tok->tokAt(2)->varId()] = 'u';
            else
                varsign[tok->tokAt(2)->varId()] = 's';
        }

        else if (!Token::Match(tok, "[).]") && Token::Match(tok->next(), "%var% / %num%"))
        {
            if (tok->strAt(3)[0] == '-')
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
            if (tok->strAt(1)[0] == '-')
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

/**
 * @brief This class is used to capture the control flow within a function.
 */
class Scope
{
public:
    Scope() : _token(NULL), _parent(NULL) { }
    Scope(const Token *token, Scope *parent_) : _token(token), _parent(parent_) { }
    ~Scope();

    Scope *parent()
    {
        return _parent;
    }
    Scope *addChild(const Token *token);
    void remove(Scope *scope);

private:
    const Token *_token;
    Scope *_parent;
    std::list<Scope *> _children;
};

Scope::~Scope()
{
    while (!_children.empty())
    {
        delete *_children.begin();
        _children.pop_front();
    }
}

Scope *Scope::addChild(const Token *token)
{
    Scope *temp = new Scope(token, this);

    _children.push_back(temp);

    return temp;
}

void Scope::remove(Scope *scope)
{
    std::list<Scope *>::iterator it;

    for (it = _children.begin(); it != _children.end(); ++it)
    {
        if (*it == scope)
        {
            delete *it;
            _children.erase(it);
            break;
        }
    }
}

/**
 * @brief This class is used create a list of variables within a function.
 */
class Variables
{
public:
    enum VariableType { standard, array, pointer, reference, pointerArray, referenceArray, pointerPointer };

    /** Store information about variable usage */
    class VariableUsage
    {
    public:
        VariableUsage(const Token *name = 0,
                      VariableType type = standard,
                      Scope *scope = NULL,
                      bool read = false,
                      bool write = false,
                      bool modified = false) :
            _name(name),
            _type(type),
            _scope(scope),
            _read(read),
            _write(write),
            _modified(modified)
        {
        }

        /** variable is used.. set both read+write */
        void use()
        {
            _read = true;
            _write = true;
        }

        /** is variable unused? */
        bool unused() const
        {
            return (_read == false && _write == false);
        }

        const Token *_name;
        VariableType _type;
        Scope *_scope;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
        std::set<unsigned int> _aliases;
        std::set<Scope *> _assignments;
    };

    typedef std::map<unsigned int, VariableUsage> VariableMap;

    void clear()
    {
        _varUsage.clear();
    }
    VariableMap &varUsage()
    {
        return _varUsage;
    }
    void addVar(const Token *name, VariableType type, Scope *scope, bool write_);
    void read(unsigned int varid);
    void readAliases(unsigned int varid);
    void readAll(unsigned int varid);
    void write(unsigned int varid);
    void writeAliases(unsigned int varid);
    void writeAll(unsigned int varid);
    void use(unsigned int varid);
    void modified(unsigned int varid);
    VariableUsage *find(unsigned int varid);
    void alias(unsigned int varid1, unsigned int varid2, bool replace);
    void erase(unsigned int varid)
    {
        _varUsage.erase(varid);
    }
    void eraseAliases(unsigned int varid);
    void eraseAll(unsigned int varid);
    void clearAliases(unsigned int varid);

private:
    VariableMap _varUsage;
};

/**
 * Alias the 2 given variables. Either replace the existing aliases if
 * they exist or merge them.  You would replace an existing alias when this
 * assignment is in the same scope as the previous assignment.  You might
 * merge the aliases when this assignment is in a different scope from the
 * previous assignment depending on the relationship of the 2 scopes.
 */
void Variables::alias(unsigned int varid1, unsigned int varid2, bool replace)
{
    VariableUsage *var1 = find(varid1);
    VariableUsage *var2 = find(varid2);

    // alias to self
    if (varid1 == varid2)
    {
        if (var1)
            var1->use();
        return;
    }

    std::set<unsigned int>::iterator i;

    if (replace)
    {
        // remove var1 from all aliases
        for (i = var1->_aliases.begin(); i != var1->_aliases.end(); ++i)
        {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(var1->_name->varId());
        }

        // remove all aliases from var1
        var1->_aliases.clear();
    }

    // var1 gets all var2s aliases
    for (i = var2->_aliases.begin(); i != var2->_aliases.end(); ++i)
    {
        if (*i != varid1)
            var1->_aliases.insert(*i);
    }

    // var2 is an alias of var1
    var2->_aliases.insert(varid1);
    var1->_aliases.insert(varid2);

    if (var2->_type == Variables::pointer)
        var2->_read = true;
}

void Variables::clearAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        // remove usage from all aliases
        std::set<unsigned int>::iterator i;

        for (i = usage->_aliases.begin(); i != usage->_aliases.end(); ++i)
        {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(usage->_name->varId());
        }

        // remove all aliases from usage
        usage->_aliases.clear();
    }
}

void Variables::eraseAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
            erase(*aliases);
    }
}

void Variables::eraseAll(unsigned int varid)
{
    eraseAliases(varid);
    erase(varid);
}

void Variables::addVar(const Token *name,
                       VariableType type,
                       Scope *scope,
                       bool write_)
{
    if (name->varId() > 0)
        _varUsage.insert(std::make_pair(name->varId(), VariableUsage(name, type, scope, false, write_, false)));
}

void Variables::read(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_read = true;
}

void Variables::readAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_read = true;
        }
    }
}

void Variables::readAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        usage->_read = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_read = true;
        }
    }
}

void Variables::write(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_write = true;
}

void Variables::writeAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::writeAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        usage->_write = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::use(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        usage->use();

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->use();
        }
    }
}

void Variables::modified(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
    {
        usage->_modified = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
        {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_modified = true;
        }
    }
}

Variables::VariableUsage *Variables::find(unsigned int varid)
{
    if (varid)
    {
        VariableMap::iterator i = _varUsage.find(varid);
        if (i != _varUsage.end())
            return &i->second;
    }
    return 0;
}

static int doAssignment(Variables &variables, const Token *tok, bool dereference, Scope *scope)
{
    int next = 0;

    // check for aliased variable
    unsigned int varid1 = tok->varId();
    Variables::VariableUsage *var1 = variables.find(varid1);

    if (var1)
    {
        Variables::VariableUsage *var2 = 0;
        int start = 1;

        // search for '='
        while (tok->tokAt(start)->str() != "=")
            start++;

        start++;

        if (Token::Match(tok->tokAt(start), "&| %var%") ||
            Token::Match(tok->tokAt(start), "( const| struct|union| %type% *| ) &| %var%") ||
            Token::Match(tok->tokAt(start), "( const| struct|union| %type% *| ) ( &| %var%") ||
            Token::Match(tok->tokAt(start), "%any% < const| struct|union| %type% *| > ( &| %var%"))
        {
            unsigned char offset = 0;
            unsigned int varid2;
            bool addressOf = false;

            if (Token::Match(tok->tokAt(start), "%var% ."))
                variables.use(tok->tokAt(start)->varId());   // use = read + write

            // check for C style cast
            if (tok->tokAt(start)->str() == "(")
            {
                if (tok->tokAt(start + 1)->str() == "const")
                    offset++;

                if (Token::Match(tok->tokAt(start + 1 + offset), "struct|union"))
                    offset++;

                if (tok->tokAt(start + 2 + offset)->str() == "*")
                    offset++;

                if (tok->tokAt(start + 3 + offset)->str() == "&")
                {
                    addressOf = true;
                    next = start + 4 + offset;
                }
                else if (tok->tokAt(start + 3 + offset)->str() == "(")
                {
                    if (tok->tokAt(start + 4 + offset)->str() == "&")
                    {
                        addressOf = true;
                        next = start + 5 + offset;
                    }
                    else
                        next = start + 4 + offset;
                }
                else
                    next = start + 3 + offset;
            }

            // check for C++ style cast
            else if (tok->tokAt(start)->str().find("cast") != std::string::npos &&
                     tok->tokAt(start + 1)->str() == "<")
            {
                if (tok->tokAt(start + 2)->str() == "const")
                    offset++;

                if (Token::Match(tok->tokAt(start + 2 + offset), "struct|union"))
                    offset++;

                if (tok->tokAt(start + 3 + offset)->str() == "*")
                    offset++;

                if (tok->tokAt(start + 5 + offset)->str() == "&")
                {
                    addressOf = true;
                    next = start + 6 + offset;
                }
                else
                    next = start + 5 + offset;
            }

            // check for var ? ...
            else if (Token::Match(tok->tokAt(start), "%var% ?"))
            {
                next = start;
            }

            // no cast
            else
            {
                if (tok->tokAt(start)->str() == "&")
                {
                    addressOf = true;
                    next = start + 1;
                }
                else if (tok->tokAt(start)->str() == "new")
                    return 0;
                else
                    next = start;
            }

            // check if variable is local
            varid2 = tok->tokAt(next)->varId();
            var2 = variables.find(varid2);

            if (var2) // local variable (alias or read it)
            {
                if (var1->_type == Variables::pointer)
                {
                    if (dereference)
                        variables.read(varid2);
                    else
                    {
                        if (addressOf ||
                            var2->_type == Variables::array ||
                            var2->_type == Variables::pointer)
                        {
                            bool    replace = true;

                            // check if variable declared in same scope
                            if (scope == var1->_scope)
                                replace = true;

                            // not in same scope as decelaration
                            else
                            {
                                std::set<Scope *>::iterator assignment;

                                // check for an assignment in this scope
                                assignment = var1->_assignments.find(scope);

                                // no other assignment in this scope
                                if (assignment == var1->_assignments.end())
                                {
                                    // nothing to replace
                                    if (var1->_assignments.empty())
                                        replace = false;

                                    // this variable has previous assignments
                                    else
                                    {
                                        /**
                                         * @todo determine if existing aliases should be replaced or merged
                                         */

                                        replace = false;
                                    }
                                }

                                // assignment in this scope
                                else
                                {
                                    // replace when only one other assingnment
                                    if (var1->_assignments.size() == 1)
                                        replace = true;

                                    // otherwise, merge them
                                    else
                                        replace = false;
                                }
                            }

                            variables.alias(varid1, varid2, replace);
                        }
                        else if (tok->tokAt(next + 1)->str() == "?")
                        {
                            if (var2->_type == Variables::reference)
                                variables.readAliases(varid2);
                            else
                                variables.read(varid2);
                        }
                    }
                }
                else if (var1->_type == Variables::reference)
                {
                    variables.alias(varid1, varid2, true);
                }
                else
                {
                    if (var2->_type == Variables::pointer && tok->tokAt(next + 1)->str() == "[")
                        variables.readAliases(varid2);

                    variables.read(varid2);
                }
            }
            else // not a local variable (or an unsupported local variable)
            {
                if (var1->_type == Variables::pointer && !dereference)
                {
                    // check if variable decelaration is in this scope
                    if (var1->_scope == scope)
                        variables.clearAliases(varid1);
                    else
                    {
                        std::set<Scope *>::iterator assignment;

                        // check for an assignment in this scope
                        assignment = var1->_assignments.find(scope);

                        // no other assignment in this scope
                        if (assignment == var1->_assignments.end())
                        {
                            /**
                             * @todo determine if existing aliases should be discarded
                             */
                        }

                        // this assignment replaces the last assignment in this scope
                        else
                        {
                            // aliased variables in a larger scope are not supported
                            // remove all aliases
                            variables.clearAliases(varid1);
                        }
                    }
                }
            }
        }

        var1->_assignments.insert(scope);
    }

    // check for alias to struct member
    // char c[10]; a.b = c;
    else if (Token::Match(tok->tokAt(-2), "%var% ."))
    {
        if (Token::Match(tok->tokAt(2), "%var%"))
        {
            unsigned int varid2 = tok->tokAt(2)->varId();
            Variables::VariableUsage *var2 = variables.find(varid2);

            // struct member aliased to local variable
            if (var2 && (var2->_type == Variables::array ||
                         var2->_type == Variables::pointer))
            {
                // erase aliased variable and all variables that alias it
                // to prevent false positives
                variables.eraseAll(varid2);
            }
        }
    }

    return next;
}

static bool nextIsStandardType(const Token *tok)
{
    tok = tok->next();

    if (tok->str() == "static")
        tok = tok->next();

    return tok->isStandardType();
}

static bool nextIsStandardTypeOrVoid(const Token *tok)
{
    tok = tok->next();

    if (tok->str() == "static")
        tok = tok->next();

    if (tok->str() == "const")
        tok = tok->next();

    return tok->isStandardType() || tok->str() == "void";
}

void CheckOther::functionVariableUsage()
{
    if (!_settings->_checkCodingStyle)
        return;

    // Parse all executing scopes..
    for (const Token *token = Token::findmatch(_tokenizer->tokens(), ") const| {"); token;)
    {
        // goto "{"
        while (token->str() != "{")
            token = token->next();

        // First token for the current scope..
        const Token *const tok1 = token;

        // Find next scope that will be checked next time..
        token = Token::findmatch(token->link(), ") const| {");

        // varId, usage {read, write, modified}
        Variables variables;

        // scopes
        Scope scopes;
        Scope *scope = &scopes;

        unsigned int indentlevel = 0;
        for (const Token *tok = tok1; tok; tok = tok->next())
        {
            if (tok->str() == "{")
            {
                // replace the head node when found
                if (indentlevel == 0)
                    scopes = Scope(tok, NULL);
                // add the new scope
                else
                    scope = scope->addChild(tok);
                ++indentlevel;
            }
            else if (tok->str() == "}")
            {
                --indentlevel;

                scope = scope->parent();

                if (indentlevel == 0)
                    break;
            }
            else if (Token::Match(tok, "struct|union|class {") ||
                     Token::Match(tok, "struct|union|class %type% {|:"))
            {
                while (tok->str() != "{")
                    tok = tok->next();
                tok = tok->link();
                if (! tok)
                    break;
            }

            if (Token::Match(tok, "[;{}] asm ( ) ;"))
            {
                variables.clear();
                break;
            }

            // standard type declaration with possible initialization
            // int i; int j = 0; static int k;
            if (Token::Match(tok, "[;{}] static| %type% %var% ;|=") &&
                nextIsStandardType(tok))
            {
                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                variables.addVar(tok->next(), Variables::standard, scope,
                                 tok->tokAt(2)->str() == "=" ||
                                 tok->previous()->str() == "static");
                tok = tok->next();
            }

            // standard type declaration and initialization using constructor
            // int i(0); static int j(0);
            else if (Token::Match(tok, "[;{}] static| %type% %var% ( %any% ) ;") &&
                     nextIsStandardType(tok))
            {
                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                variables.addVar(tok->next(), Variables::standard, scope, true);

                // check if a local variable is used to initialize this variable
                if (tok->tokAt(3)->varId() > 0)
                    variables.readAll(tok->tokAt(3)->varId());
                tok = tok->tokAt(4);
            }

            // standard type declaration of array of with possible initialization
            // int i[10]; int j[2] = { 0, 1 }; static int k[2] = { 2, 3 };
            else if (Token::Match(tok, "[;{}] static| %type% %var% [ %any% ] ;|=") &&
                     nextIsStandardType(tok))
            {
                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                variables.addVar(tok->tokAt(1), Variables::array, scope,
                                 tok->tokAt(5)->str() == "=" || tok->str() == "static");

                // check for reading array size from local variable
                if (tok->tokAt(3)->varId() != 0)
                    variables.read(tok->tokAt(3)->varId());

                // look at initializers
                if (Token::Match(tok->tokAt(5), "= {"))
                {
                    tok = tok->tokAt(7);
                    while (tok->str() != "}")
                    {
                        if (Token::Match(tok, "%var%"))
                            variables.read(tok->varId());
                        tok = tok->next();
                    }
                }
                else
                    tok = tok->tokAt(4);
            }

            // pointer or reference declaration with possible initialization
            // int * i; int * j = 0; static int * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% *|& %var% ;|="))
            {
                bool isStatic = false;

                tok = tok->next();

                if (tok->str() == "static")
                {
                    tok = tok->next();
                    isStatic = true;
                }

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return" && tok->str() != "throw")
                {
                    Variables::VariableType type;

                    if (tok->next()->str() == "*")
                        type = Variables::pointer;
                    else
                        type = Variables::reference;

                    bool written = tok->tokAt(3)->str() == "=";

                    variables.addVar(tok->tokAt(2), type, scope, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(2), false, scope);

                    tok = tok->tokAt(2 + offset);
                }
            }

            // pointer to pointer declaration with possible initialization
            // int ** i; int ** j = 0; static int ** k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% * * %var% ;|="))
            {
                bool isStatic = false;

                tok = tok->next();

                if (tok->str() == "static")
                {
                    tok = tok->next();
                    isStatic = true;
                }

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return")
                {
                    bool written = tok->tokAt(4)->str() == "=";

                    variables.addVar(tok->tokAt(3), Variables::pointerPointer, scope, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(3), false, scope);

                    tok = tok->tokAt(3 + offset);
                }
            }

            // pointer or reference of struct or union declaration with possible initialization
            // struct s * i; struct s * j = 0; static struct s * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| struct|union %type% *|& %var% ;|="))
            {
                Variables::VariableType type;
                bool isStatic = false;

                tok = tok->next();

                if (tok->str() == "static")
                {
                    tok = tok->next();
                    isStatic = true;
                }

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->strAt(2) == "*")
                    type = Variables::pointer;
                else
                    type = Variables::reference;

                const bool written = tok->strAt(4) == "=";

                variables.addVar(tok->tokAt(3), type, scope, written || isStatic);

                int offset = 0;

                // check for assignment
                if (written)
                    offset = doAssignment(variables, tok->tokAt(3), false, scope);

                tok = tok->tokAt(3 + offset);
            }

            // pointer or reference declaration with initialization using constructor
            // int * i(j); int * k(i); static int * l(i);
            else if (Token::Match(tok, "[;{}] static| const| %type% &|* %var% ( %any% ) ;") &&
                     nextIsStandardTypeOrVoid(tok))
            {
                Variables::VariableType type;

                tok = tok->next();

                if (tok->str() == "static")
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->next()->str() == "*")
                    type = Variables::pointer;
                else
                    type = Variables::reference;

                unsigned int varid = 0;

                // check for aliased variable
                if (Token::Match(tok->tokAt(4), "%var%"))
                    varid = tok->tokAt(4)->varId();

                variables.addVar(tok->tokAt(2), type, scope, true);

                // check if a local variable is used to initialize this variable
                if (varid > 0)
                {
                    Variables::VariableUsage	*var = variables.find(varid);

                    if (type == Variables::pointer)
                    {
                        variables.use(tok->tokAt(4)->varId());

                        if (var && (var->_type == Variables::array ||
                                    var->_type == Variables::pointer))
                            var->_aliases.insert(tok->varId());
                    }
                    else
                    {
                        variables.readAll(tok->tokAt(4)->varId());
                        if (var)
                            var->_aliases.insert(tok->varId());
                    }
                }
                tok = tok->tokAt(5);
            }

            // array of pointer or reference declaration with possible initialization
            // int * p[10]; int * q[10] = { 0 }; static int * * r[10] = { 0 };
            else if (Token::Match(tok, "[;{}] static| const| %type% *|& %var% [ %any% ] ;|="))
            {
                bool isStatic = false;

                tok = tok->next();

                if (tok->str() == "static")
                {
                    tok = tok->next();
                    isStatic = true;
                }

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return")
                {
                    variables.addVar(tok->tokAt(2),
                                     tok->next()->str() == "*" ? Variables::pointerArray : Variables::referenceArray, scope,
                                     tok->tokAt(6)->str() == "=" || isStatic);

                    // check for reading array size from local variable
                    if (tok->tokAt(4)->varId() != 0)
                        variables.read(tok->tokAt(4)->varId());

                    tok = tok->tokAt(5);
                }
            }

            // array of pointer or reference of struct or union declaration with possible initialization
            // struct S * p[10]; struct T * q[10] = { 0 }; static struct S * r[10] = { 0 };
            else if (Token::Match(tok, "[;{}] static| const| struct|union %type% *|& %var% [ %any% ] ;|="))
            {
                bool isStatic = false;

                tok = tok->next();

                if (tok->str() == "static")
                {
                    tok = tok->next();
                    isStatic = true;
                }

                if (tok->str() == "const")
                    tok = tok->next();

                variables.addVar(tok->tokAt(3),
                                 tok->tokAt(2)->str() == "*" ? Variables::pointerArray : Variables::referenceArray, scope,
                                 tok->tokAt(7)->str() == "=" || isStatic);

                // check for reading array size from local variable
                if (tok->tokAt(5)->varId() != 0)
                    variables.read(tok->tokAt(5)->varId());

                tok = tok->tokAt(6);
            }

            else if (Token::Match(tok, "delete|return|throw %var%"))
                variables.readAll(tok->next()->varId());

            // assignment
            else if (Token::Match(tok, "*| (| ++|--| %var% ++|--| )| ="))
            {
                bool dereference = false;
                bool pre = false;
                bool post = false;

                if (tok->str() == "*")
                {
                    dereference = true;
                    tok = tok->next();
                }

                if (tok->str() == "(")
                    tok = tok->next();

                if (Token::Match(tok, "++|--"))
                {
                    pre = true;
                    tok = tok->next();
                }

                if (Token::Match(tok->next(), "++|--"))
                    post = true;

                unsigned int varid1 = tok->varId();
                const Token *start = tok;

                tok = tok->tokAt(doAssignment(variables, tok, dereference, scope));

                if (pre || post)
                    variables.use(varid1);

                if (dereference)
                {
                    variables.writeAliases(varid1);
                    variables.read(varid1);
                }
                else
                {
                    Variables::VariableUsage *var = variables.find(varid1);

                    if (var && var->_type == Variables::reference)
                    {
                        variables.writeAliases(varid1);
                        variables.read(varid1);
                    }
                    else
                        variables.write(varid1);
                }

                const Token *equal = tok->next();

                if (Token::Match(tok->next(), "[ %any% ]"))
                    equal = tok->tokAt(4);

                // checked for chained assignments
                if (tok != start && equal->str() == "=")
                {
                    Variables::VariableUsage *var = variables.find(tok->varId());

                    if (var && var->_type != Variables::reference)
                        var->_read = true;

                    tok = tok->previous();
                }
            }

            // assignment
            else if (Token::Match(tok, "%var% [") && Token::Match(tok->next()->link(), "] ="))
            {
                unsigned int varid = tok->varId();
                Variables::VariableUsage *var = variables.find(varid);

                if (var)
                {
                    if (var->_type == Variables::pointer)
                    {
                        variables.read(varid);
                        variables.writeAliases(varid);
                    }
                    else
                        variables.writeAll(varid);
                }
            }

            else if (Token::Match(tok, ">>|& %var%"))
                variables.use(tok->next()->varId());    // use = read + write

            // function parameter
            else if (Token::Match(tok, "[(,] %var% ["))
                variables.use(tok->next()->varId());   // use = read + write
            else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*")
                variables.use(tok->next()->varId());   // use = read + write
            else if (Token::Match(tok, "[(,] (") &&
                     Token::Match(tok->next()->link(), ") %var% [,)]"))
                variables.use(tok->next()->link()->next()->varId());   // use = read + write

            // function
            else if (Token::Match(tok, " %var% ("))
                variables.read(tok->varId());

            else if (Token::Match(tok, " %var% ."))
                variables.use(tok->varId());   // use = read + write

            else if ((Token::Match(tok, "[(=&!]") || isOp(tok)) &&
                     (Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new")))
                variables.readAll(tok->next()->varId());

            else if (Token::Match(tok, "-=|+=|*=|/=|&=|^= %var%") || Token::Match(tok, "|= %var%"))
                variables.modified(tok->next()->varId());

            else if (Token::Match(tok, "%var%") && (tok->next()->str() == ")" || isOp(tok->next())))
                variables.readAll(tok->varId());

            else if (Token::Match(tok, "; %var% ;"))
                variables.readAll(tok->next()->varId());

            else if (Token::Match(tok, "++|-- %var%"))
                variables.modified(tok->next()->varId());

            else if (Token::Match(tok, "%var% ++|--"))
                variables.modified(tok->varId());
        }

        // Check usage of all variables in the current scope..
        Variables::VariableMap::const_iterator it;
        for (it = variables.varUsage().begin(); it != variables.varUsage().end(); ++it)
        {
            const Variables::VariableUsage &usage = it->second;
            const std::string &varname = usage._name->str();

            // variable has been marked as unused so ignore it
            if (usage._name->isUnused())
                continue;

            // skip things that are only partially implemented to prevent false positives
            if (usage._type == Variables::pointerPointer ||
                usage._type == Variables::pointerArray ||
                usage._type == Variables::referenceArray)
                continue;

            // variable has not been written, read, or modified
            if (usage.unused() && !usage._modified)
                unusedVariableError(usage._name, varname);

            // variable has not been written but has been modified
            else if (usage._modified & !usage._write)
                unassignedVariableError(usage._name, varname);

            // variable has been written but not read
            else if (!usage._read && !usage._modified)
                unreadVariableError(usage._name, varname);

            // variable has been read but not written
            else if (!usage._write)
                unassignedVariableError(usage._name, varname);
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
    if (!_settings->_checkCodingStyle)
        return;

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
            else if (tok1->isStandardType() && Token::Match(tok1, "%type% %var% [;=]"))
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
    if (!_settings->_checkCodingStyle)
        return;

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
    if (!_settings->_checkCodingStyle)
        return;

    std::string structname;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "struct|union %type% {"))
        {
            structname.clear();
            if (Token::simpleMatch(tok->previous(), "extern"))
                continue;
            if ((!tok->previous() || Token::simpleMatch(tok->previous(), ";")) && Token::Match(tok->tokAt(2)->link(), ("} ; " + tok->strAt(1) + " %var% ;").c_str()))
                continue;

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

            // Try to prevent false positives when struct members are not used directly.
            if (Token::findmatch(tok, (structname + " *").c_str()))
                structname = "";
            else if (Token::findmatch(tok, (structname + " %type% *").c_str()))
                structname = "";
        }

        if (tok->str() == "}")
            structname = "";

        if (!structname.empty() && Token::Match(tok, "[{;]"))
        {
            // Declaring struct variable..
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

            // Check if the struct variable is anywhere in the file
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
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Declaring the variable..
        if (Token::Match(tok, "[{};(,] char %var% [;=,)]"))
        {
            // Check for unsigned char
            if (tok->tokAt(1)->isUnsigned())
                continue;

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
    if (!_settings->_checkCodingStyle)
        return;

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
    // don't check vars that has been tested against null already
    std::set<unsigned int> skipvar;
    skipvar.insert(0);

    // Dereferencing a struct pointer and then checking if it's NULL..
    for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
    {
        if (Token::Match(tok1, "if|while ( !| %var% )"))
        {
            tok1 = tok1->tokAt(2);
            if (tok1->str() == "!")
                tok1 = tok1->next();
            skipvar.insert(tok1->varId());
            continue;
        }

        // dereference in assignment
        if (Token::Match(tok1, "[{};] %var% = %var% . %var%"))
        {
            if (std::string(tok1->strAt(1)) == tok1->strAt(3))
                continue;
            tok1 = tok1->tokAt(3);
        }

        // dereference in function call
        else if (Token::Match(tok1->tokAt(-2), "%var% ( %var% . %var%") ||
                 Token::Match(tok1->previous(), ", %var% . %var%"))
        {

        }

        // Goto next token
        else
        {
            continue;
        }

        // struct dereference was found - investigate if it is later
        // checked that it is not NULL
        const unsigned int varid1(tok1->varId());
        if (skipvar.find(varid1) != skipvar.end())
            continue;

        const std::string varname(tok1->str());

        unsigned int indentlevel2 = 0;
        for (const Token *tok2 = tok1->tokAt(3); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
                ++indentlevel2;

            else if (tok2->str() == "}")
            {
                if (indentlevel2 <= 1)
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

            // Check that variable is a pointer..
            const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid%", varid);
            if (!Token::Match(decltok->tokAt(-3), "[;,(] %type% *"))
                continue;

            for (const Token *tok1 = tok->previous(); tok1 && tok1 != decltok; tok1 = tok1->previous())
            {
                if (tok1->varId() == varid)
                {
                    if (Token::Match(tok1->tokAt(-2), "[=;{}] *"))
                    {
                        nullPointerError(tok1, varname, tok->linenr());
                        break;
                    }
                    else if (Token::simpleMatch(tok1->previous(), "&"))
                    {
                        break;
                    }
                    else if (Token::simpleMatch(tok1->next(), "="))
                    {
                        break;
                    }
                    // dereference in function call
                    else if (Token::Match(tok1->tokAt(-2), "[(,] *"))
                    {
                        nullPointerError(tok1, varname, tok->linenr());
                    }
                }

                else if (tok1->str() == "{" ||
                         tok1->str() == "}")
                    break;

                // label..
                else if (Token::Match(tok1, "%type% :"))
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

/** Derefencing null constant (simplified token list) */
void CheckOther::nullConstantDereference()
{
    // this is kept at 0 for all scopes that are not executing
    unsigned int indentlevel = 0;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // start of executable scope..
        if (indentlevel == 0 && Token::Match(tok, ") const| {"))
            indentlevel = 1;

        else if (indentlevel >= 1)
        {
            if (tok->str() == "{")
                ++indentlevel;

            else if (tok->str() == "}")
            {
                if (indentlevel <= 2)
                    indentlevel = 0;
                else
                    --indentlevel;
            }

            if (tok->str() == "(" && Token::simpleMatch(tok->previous(), "sizeof"))
                tok = tok->link();

            else if (Token::simpleMatch(tok, "exit ( )"))
            {
                // Goto end of scope
                while (tok && tok->str() != "}")
                {
                    if (tok->str() == "{")
                        tok = tok->link();
                    tok = tok->next();
                }
                if (!tok)
                    break;
            }

            else if (Token::simpleMatch(tok, "* 0"))
            {
                if (Token::Match(tok->previous(), "[<>;{}=+-*/(,]") ||
                    Token::Match(tok->previous(), "return|<<"))
                {
                    nullPointerError(tok);
                }
            }
        }
    }
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
        functionNames1.insert("strndup");
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

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const CheckNullpointer *c = static_cast<const CheckNullpointer *>(e);
        return (varname == c->varname && null == c->null);
    }

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

    /**
     * Dereferencing variable. Check if it is safe (if the variable is null there's an error)
     * @param checks Checks
     * @param tok token where dereferencing happens
     */
    static void dereference(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        const unsigned int varid(tok->varId());

        std::list<ExecutionPath *>::iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            CheckNullpointer *c = dynamic_cast<CheckNullpointer *>(*it);
            if (c && c->varId == varid && c->null)
            {
                CheckOther *checkOther = dynamic_cast<CheckOther *>(c->owner);
                if (checkOther)
                {
                    checkOther->nullPointerError(tok, c->varname);
                    return;
                }
            }
        }
    }

    /** parse tokens */
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const
    {
        if (Token::Match(tok.previous(), "[;{}] const| %type% * %var% ;"))
        {
            const Token * vartok = tok.tokAt(2);

            if (tok.str() == "const")
                vartok = vartok->next();

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

        if (Token::simpleMatch(&tok, "try {"))
        {
            // Bail out all used variables
            unsigned int indentlevel = 0;
            for (const Token *tok2 = &tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}")
                {
                    if (indentlevel == 0)
                        break;
                    if (indentlevel == 1 && !Token::simpleMatch(tok2,"} catch ("))
                        return tok2;
                    --indentlevel;
                }
                else if (tok2->varId())
                    bailOutVar(checks,tok2->varId());
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
                dereference(checks, *it);
        }

        if (tok.varId() != 0)
        {
            if (Token::Match(tok.previous(), "[;{}=] %var% = 0 ;"))
                setnull(checks, tok.varId());
            else if (Token::Match(tok.tokAt(-2), "[;{}=+-/(,] * %var%"))
                dereference(checks, &tok);
            else if (Token::Match(tok.tokAt(-2), "return * %var%"))
                dereference(checks, &tok);
            else if (!Token::simpleMatch(tok.tokAt(-2), "& (") && Token::Match(tok.next(), ". %var%"))
                dereference(checks, &tok);
            else if (Token::Match(tok.previous(), "[;{}=+-/(,] %var% [ %any% ]"))
                dereference(checks, &tok);
            else if (Token::Match(tok.previous(), "return %var% [ %any% ]"))
                dereference(checks, &tok);
            else if (Token::Match(&tok, "%var% ("))
                dereference(checks, &tok);
            else
                bailOutVar(checks, tok.varId());
        }

        else if (tok.str() == "delete")
        {
            const Token *ret = tok.next();
            if (Token::simpleMatch(ret, "[ ]"))
                ret = ret->tokAt(2);
            if (Token::Match(ret, "%var% ;"))
                return ret->next();
        }

        return &tok;
    }

    /** parse condition. @sa ExecutionPath::parseCondition */
    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        for (const Token *tok2 = &tok; tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(" || tok2->str() == ")")
                break;
            if (Token::Match(tok2, "[<>=] * %var%"))
                dereference(checks, tok2->tokAt(2));
        }

        if (Token::Match(&tok, "!| %var% ("))
        {
            std::list<const Token *> var;
            parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 0);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                dereference(checks, *it);
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

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const CheckUninitVar *c = static_cast<const CheckUninitVar *>(e);
        return (varname == c->varname && pointer == c->pointer && array == c->array && alloc == c->alloc && strncpy_ == c->strncpy_);
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
    static void init_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
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
                    use_pointer(checks, tok);
                }
            }

            ++it;
        }
    }

    /** Deallocate a pointer. For example: free(p); */
    static void dealloc_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
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


    /** Initialize an array with strncpy. */
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
     * @param checks all available checks
     * @param tok variable token
     * @param mode specific behaviour
     * @return if error is found, true is returned
     */
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok, const int mode)
    {
        const unsigned int varid(tok->varId());
        if (varid == 0)
            return false;

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

                // mode 4 : reading uninitialized array or pointer is invalid.
                if (mode == 4 && (!c->array && !c->pointer))
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
                    return true;
                }
            }
        }

        // No error found
        return false;
    }

    /**
     * Reading variable. Use this function in situations when it is
     * invalid to read the data of the variable but not the address.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 0);
    }

    /**
     * Reading array elements. If the variable is not an array then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     */
    static void use_array(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        use(checks, tok, 1);
    }

    /**
     * Bad pointer usage. If the variable is not a pointer then the usage is ok.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 2);
    }

    /**
     * Using variable.. if it's a dead pointer the usage is invalid.
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_dead_pointer(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 3);
    }

    /**
     * Using variable.. reading from uninitialized array or pointer data is invalid.
     * Example: = x[0];
     * @param checks all available checks
     * @param tok variable token
     * @return if error is found, true is returned
     */
    static bool use_array_or_pointer_data(std::list<ExecutionPath *> &checks, const Token *tok)
    {
        return use(checks, tok, 4);
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
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const
    {
        // Variable declaration..
        if (Token::Match(tok.previous(), "[;{}] %var%") && tok.str() != "return")
        {
            if (Token::Match(&tok, "enum %type% {"))
                return tok.tokAt(2)->link();

            const Token * vartok = &tok;
            while (Token::Match(vartok, "const|struct"))
                vartok = vartok->next();

            if (Token::Match(vartok, "%type% *| %var% ;"))
            {
                vartok = vartok->next();
                const bool p(vartok->str() == "*");
                if (p)
                    vartok = vartok->next();
                declare(checks, vartok, tok, p, false);
                return vartok;
            }

            // Variable declaration for array..
            if (Token::Match(vartok, "%type% %var% [ %num% ] ;"))
            {
                vartok = vartok->next();
                declare(checks, vartok, tok, false, true);
                return vartok->next()->link();
            }

            // Template pointer variable..
            if (Token::Match(vartok, "%type% ::|<"))
            {
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
                use(checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "++|--") || Token::Match(tok.next(), "++|--"))
            {
                use(checks, &tok);
                return &tok;
            }

            if (Token::Match(tok.previous(), "[;{}] %var% =|[|."))
            {
                if (tok.next()->str() == ".")
                {
                    if (use_dead_pointer(checks, &tok))
                    {
                        return &tok;
                    }
                }
                else
                {
                    // check variable usages in rhs/index
                    for (const Token *tok2 = tok.tokAt(2); tok2; tok2 = tok2->next())
                    {
                        if (Token::Match(tok2, ";|)|=|?"))
                            break;
                        if (Token::Match(tok2, "%var% ("))
                            break;
                        if (tok2->varId() &&
                            !Token::Match(tok2->previous(), "&|::") &&
                            !Token::simpleMatch(tok2->next(), "="))
                        {
                            // Multiple assignments..
                            if (Token::simpleMatch(tok2->next(), "["))
                            {
                                const Token * tok3 = tok2;
                                while (Token::simpleMatch(tok3->next(), "["))
                                    tok3 = tok3->next()->link();
                                if (Token::simpleMatch(tok3, "] ="))
                                    continue;
                            }
                            bool foundError;
                            if (tok2->previous()->str() == "*" || tok2->next()->str() == "[")
                                foundError = use_array_or_pointer_data(checks, tok2);
                            else
                                foundError = use(checks, tok2);

                            // prevent duplicate error messages
                            if (foundError)
                            {
                                bailOutVar(checks, tok2->varId());
                            }
                        }
                    }
                }

                // pointer aliasing?
                if (Token::Match(tok.tokAt(2), "%var% ;"))
                {
                    pointer_assignment(checks, &tok, tok.tokAt(2));
                }
            }

            if (Token::simpleMatch(tok.next(), "("))
            {
                use_pointer(checks, &tok);
            }

            if (Token::Match(tok.tokAt(-2), "[;{}] *"))
            {
                if (Token::simpleMatch(tok.next(), "="))
                    init_pointer(checks, &tok);
                else
                    use_pointer(checks, &tok);
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
                        use_pointer(checks, &tok);
                    else if (tok.next()->str() == "[")
                        use_array(checks, &tok);
                    else
                        use(checks, &tok);
                }
            }

            if (Token::Match(tok.next(), "= malloc|kmalloc") || Token::simpleMatch(tok.next(), "= new char ["))
            {
                alloc_pointer(checks, tok.varId());
                if (tok.tokAt(3)->str() == "(")
                    return tok.tokAt(3);
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
                dealloc_pointer(checks, &tok);
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
                dealloc_pointer(checks, tok.tokAt(2));
                return tok.tokAt(3);
            }

            // parse usage..
            {
                std::list<const Token *> var;
                parseFunctionCall(tok, var, 1);
                for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                    use_array(checks, *it);

                // Using uninitialized pointer is bad if using null pointer is bad
                std::list<const Token *> var2;
                parseFunctionCall(tok, var2, 0);
                for (std::list<const Token *>::const_iterator it = var2.begin(); it != var2.end(); ++it)
                {
                    if (std::find(var.begin(), var.end(), *it) == var.end())
                        use_dead_pointer(checks, *it);
                }
            }

            // strncpy doesn't 0-terminate first parameter
            if (Token::Match(&tok, "strncpy ( %var% ,"))
            {
                if (Token::Match(tok.tokAt(4), "%str% ,"))
                {
                    if (Token::Match(tok.tokAt(6), "%num% )"))
                    {
                        const unsigned int len = Token::getStrLength(tok.tokAt(4));
                        const long sz = MathLib::toLongNumber(tok.strAt(6));
                        if (sz >= 0 && len >= static_cast<unsigned long>(sz))
                        {
                            init_strncpy(checks, tok.tokAt(2));
                            return tok.next()->link();
                        }
                    }
                }
                else
                {
                    init_strncpy(checks, tok.tokAt(2));
                    return tok.next()->link();
                }
            }

            if (Token::simpleMatch(&tok, "asm ( )"))
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
                    {
                        if (use_dead_pointer(checks, tok2))
                            ExecutionPath::bailOutVar(checks, tok2->varId());
                    }

                    // it is possible that the variable is initialized here
                    if (Token::Match(tok2->previous(), "[(,] %var% [,)]"))
                        bailouts.insert(tok2->varId());

                    // array initialization..
                    if (Token::Match(tok2->previous(), "[,(] %var% +"))
                    {
                        // if var is array, bailout
                        for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it)
                        {
                            if ((*it)->varId == tok2->varId())
                            {
                                const CheckUninitVar *c = dynamic_cast<const CheckUninitVar *>(*it);
                                if (c && c->array)
                                    bailouts.insert(tok2->varId());
                                break;
                            }
                        }
                    }
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
                use(checks, tok.next());
            }
            else if (Token::Match(tok.next(), "%var% ["))
            {
                use_array_or_pointer_data(checks, tok.next());
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
                        use(checks, &tok);
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
                            use(checks, &tok);
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
                    use_pointer(checks, &tok);
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
                use_pointer(checks, &tok);
                return &tok;
            }

            if (Token::simpleMatch(tok.previous(), "&"))
            {
                ExecutionPath::bailOutVar(checks, tok.varId());
            }
        }

        // Parse "for"
        if (Token::Match(&tok, "[;{}] for ("))
        {
            // initialized variables
            std::set<unsigned int> varid1;
            varid1.insert(0);

            // Parse token
            const Token *tok2;

            // parse setup
            for (tok2 = tok.tokAt(3); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == ";")
                    break;
                if (tok2->varId())
                    varid1.insert(tok2->varId());
            }

            // parse condition
            if (Token::Match(tok2, "; %var% <|<=|>=|> %num% ;"))
            {
                // If the variable hasn't been initialized then call "use"
                if (varid1.find(tok2->next()->varId()) == varid1.end())
                    use(checks, tok2->next());
            }

            // goto stepcode
            tok2 = tok2->next();
            while (tok2 && tok2->str() != ";")
                tok2 = tok2->next();

            // parse the stepcode
            if (Token::Match(tok2, "; ++|-- %var% ) {") ||
                Token::Match(tok2, "; %var% ++|-- ) {"))
            {
                // get id of variable..
                unsigned int varid = tok2->next()->varId();
                if (!varid)
                    varid = tok2->tokAt(2)->varId();

                // Check that the variable hasn't been initialized and
                // that it isn't initialized in the body..
                if (varid1.find(varid) == varid1.end())
                {
                    unsigned int indentlevel = 0;
                    for (const Token *tok3 = tok2->tokAt(5); tok3; tok3 = tok3->next())
                    {
                        if (tok3->str() == "{")
                            ++indentlevel;
                        else if (tok3->str() == "}")
                        {
                            if (indentlevel == 0)
                                break;
                            --indentlevel;
                        }
                        if (tok3->varId() == varid)
                        {
                            varid = 0;  // variable is used.. maybe it's initialized. clear the variable id.
                            break;
                        }
                    }

                    // If the variable isn't initialized in the body call "use"
                    if (varid != 0)
                    {
                        // goto variable
                        tok2 = tok2->next();
                        if (!tok2->varId())
                            tok2 = tok2->next();

                        // call "use"
                        use(checks, tok2);
                    }
                }
            }
        }

        return &tok;
    }

    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks)
    {
        if (tok.varId() && Token::Match(&tok, "%var% <|<=|==|!=|)"))
            use(checks, &tok);

        else if (Token::Match(&tok, "!| %var% ["))
            use_array_or_pointer_data(checks, tok.str() == "!" ? tok.next() : &tok);

        else if (Token::Match(&tok, "!| %var% ("))
        {
            std::list<const Token *> var;
            parseFunctionCall(tok.str() == "!" ? *tok.next() : tok, var, 1);
            for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it)
                use_array(checks, *it);
        }

        else if (Token::Match(&tok, "! %var% )"))
        {
            use(checks, &tok);
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

                    if (tok2->isStandardType() && Token::Match(tok2, "%type% & %var% ,|)"))
                    {
                        const unsigned int varid(tok2->tokAt(2)->varId());

                        // flags for read/write
                        bool r = false, w = false;

                        // check how the variable is used in the function
                        unsigned int indentlevel = 0;
                        for (const Token *tok3 = tok2; tok3; tok3 = tok3->next())
                        {
                            if (tok3->str() == "{")
                                ++indentlevel;
                            else if (tok3->str() == "}")
                            {
                                if (indentlevel <= 1)
                                    break;
                                --indentlevel;
                            }
                            else if (indentlevel == 0 && tok3->str() == ";")
                                break;
                            else if (indentlevel >= 1 && tok3->varId() == varid)
                            {
                                if (Token::Match(tok3->previous(), "++|--") ||
                                    Token::Match(tok3->next(), "++|--"))
                                {
                                    r = true;
                                }

                                else
                                {
                                    w = true;
                                    break;
                                }
                            }
                        }

                        if (!r || w)
                            break;

                        tok2 = tok2->tokAt(3);
                        continue;
                    }

                    if (Token::Match(tok2, "const %type% &|*| const| %var% ,|)") && tok2->next()->isStandardType())
                    {
                        tok2 = tok2->tokAt(3);
                        while (tok2->isName())
                            tok2 = tok2->next();
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


void CheckOther::analyse(const Token * const tokens, std::set<std::string> &func) const
{
    CheckUninitVar::analyseFunctions(tokens, func);
}

void CheckOther::saveAnalysisData(const std::set<std::string> &data) const
{
    CheckUninitVar::uvarFunctions.insert(data.begin(), data.end());
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


void CheckOther::checkMathFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // case log(-2)
        if (tok->varId() == 0 &&
            Token::Match(tok, "log|log10 ( %num% )") &&
            MathLib::isNegative(tok->tokAt(2)->str()) &&
            MathLib::isInt(tok->tokAt(2)->str()) &&
            MathLib::toLongNumber(tok->tokAt(2)->str()) <= 0)
        {
            mathfunctionCallError(tok);
        }
        // case log(-2.0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isFloat(tok->tokAt(2)->str()) &&
                 MathLib::toDoubleNumber(tok->tokAt(2)->str()) <= 0.)
        {
            mathfunctionCallError(tok);
        }

        // case log(0.0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 !MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isFloat(tok->tokAt(2)->str()) &&
                 MathLib::toDoubleNumber(tok->tokAt(2)->str()) <= 0.)
        {
            mathfunctionCallError(tok);
        }

        // case log(0)
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "log|log10 ( %num% )") &&
                 !MathLib::isNegative(tok->tokAt(2)->str()) &&
                 MathLib::isInt(tok->tokAt(2)->str()) &&
                 MathLib::toLongNumber(tok->tokAt(2)->str()) <= 0)
        {
            mathfunctionCallError(tok);
        }
        // acos( x ), asin( x )  where x is defined for intervall [-1,+1], but not beyound
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "acos|asin ( %num% )") &&
                 std::fabs(MathLib::toDoubleNumber(tok->tokAt(2)->str())) > 1.0)
        {
            mathfunctionCallError(tok);
        }
        // sqrt( x ): if x is negative the result is undefined
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "sqrt ( %num% )") &&
                 MathLib::isNegative(tok->tokAt(2)->str()))
        {
            mathfunctionCallError(tok);
        }
        // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "atan2 ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(2)->str()) &&
                 MathLib::isNullValue(tok->tokAt(4)->str()))
        {
            mathfunctionCallError(tok, 2);
        }
        // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "fmod ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(4)->str()))
        {
            mathfunctionCallError(tok, 2);
        }
        // pow ( x , y) If x is zero, and y is negative --> division by zero
        else if (tok->varId() == 0 &&
                 Token::Match(tok, "pow ( %num% , %num% )") &&
                 MathLib::isNullValue(tok->tokAt(2)->str())  &&
                 MathLib::isNegative(tok->tokAt(4)->str()))
        {
            mathfunctionCallError(tok, 2);
        }

    }
}



void CheckOther::postIncrement()
{
    if (!_settings->_checkCodingStyle || !_settings->inconclusive)
        return;

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

void CheckOther::checkEmptyCatchBlock()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char pattern[] = "} catch (";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern); tok;
         tok = Token::findmatch(tok, pattern))
    {
        tok = tok->tokAt(2);

        if (Token::simpleMatch(tok->link(), ") { }"))
        {
            emptyCatchBlockError(tok);
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
    reportError(tok,
                Severity::style,
                "variableScope",
                "The scope of the variable " + varname + " can be reduced\n"
                "Warning: It can be unsafe to fix this message. Be careful. Especially when there are inner loops.\n"
                "Here is an example where cppcheck will write that the scope for 'i' can be reduced:\n"
                "void f(int x)\n"
                "{\n"
                "    int i = 0;\n"
                "    if (x) {\n"
                "        // it's safe to move 'int i = 0' here\n"
                "        for (int n = 0; n < 10; ++n) {\n"
                "            // it is possible but not safe to move 'int i = 0' here\n"
                "            do_something(&i);\n"
                "        }\n"
                "    }\n"
                "}\n"
                "\n"
                "When you see this message it is always safe to reduce the variable scope 1 level.");
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

void CheckOther::nullPointerError(const Token *tok, const std::string &varname, const unsigned int line)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname + " - otherwise it is redundant to check if " + varname + " is null at line " + MathLib::toString<unsigned int>(line));
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

void CheckOther::mathfunctionCallError(const Token *tok, const unsigned int numParam)
{
    if (tok)
    {
        if (numParam == 1)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->tokAt(2)->str() + " to " + tok->str() + "() leads to undefined result");
        else if (numParam == 2)
            reportError(tok, Severity::error, "wrongmathcall", "Passing value " + tok->tokAt(2)->str() + " and " + tok->tokAt(4)->str() + " to " + tok->str() + "() leads to undefined result");
    }
    else
        reportError(tok, Severity::error, "wrongmathcall", "Passing value " " to " "() leads to undefined result");
}

void CheckOther::postIncrementError(const Token *tok, const std::string &var_name, const bool isIncrement)
{
    std::string type = (isIncrement ? "Incrementing" : "Decrementing");
    reportError(tok, Severity::style, "postIncrementDecrement", ("Pre-" + type + " variable '" + var_name + "' is preferred to Post-" + type));
}

void CheckOther::emptyStringTestError(const Token *tok, const std::string &var_name, const bool isTestForEmpty)
{
    if (isTestForEmpty)
    {
        reportError(tok, Severity::style,
                    "emptyStringTest", "Empty string test can be simplified to \"*" + var_name + " == '\\0'\"");
    }
    else
    {
        reportError(tok, Severity::style,
                    "emptyStringTest", "Non-empty string test can be simplified to \"*" + var_name + " != '\\0'\"");
    }
}

void CheckOther::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream \"" + varname + "\" may result in undefined behaviour");
}

void CheckOther::emptyCatchBlockError(const Token *tok)
{
    reportError(tok, Severity::style, "emptyCatchBlock", "Empty catch block");
}

void CheckOther::sizeofsizeof()
{
    if (!_settings->_checkCodingStyle)
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "sizeof sizeof"))
            sizeofsizeofError(tok);
    }
}

void CheckOther::sizeofsizeofError(const Token *tok)
{
    reportError(tok, Severity::style,
                "sizeofsizeof", "Suspicious code 'sizeof sizeof ..', most likely there should only be one sizeof. The current code is equivalent to 'sizeof(size_t)'.");
}

void CheckOther::sizeofCalculation()
{
    if (!_settings->_checkCodingStyle)
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "sizeof ("))
        {
            unsigned int parlevel = 0;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }
                else if (Token::Match(tok2, "+|/"))
                {
                    sizeofCalculationError(tok2);
                    break;
                }
            }
        }
    }
}

void CheckOther::sizeofCalculationError(const Token *tok)
{
    reportError(tok, Severity::style,
                "sizeofCalculation", "Found calculation inside sizeof()");
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style,
                "redundantAssignInSwitch", "Redundant assignment of \"" + varname + "\" in switch");
}
