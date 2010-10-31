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

#include <cctype> // std::isupper
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
    const char breakPattern[] = "break|continue|return|exit|goto";
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
//    int x = 1;
//    x = x;            // <- redundant assignment to self
//
//    int y = y;        // <- redundant initialization to self
//---------------------------------------------------------------------------
void CheckOther::checkSelfAssignment()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char selfAssignmentPattern[] = "%var% = %var% ;|=|)";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), selfAssignmentPattern);
    while (tok)
    {
        if (tok->varId() && tok->varId() == tok->tokAt(2)->varId())
        {
            selfAssignmentError(tok, tok->str());
        }

        tok = Token::findmatch(tok->next(), selfAssignmentPattern);
    }
}

//---------------------------------------------------------------------------
//    int a = 1;
//    assert(a = 2);            // <- assert should not have a side-effect
//---------------------------------------------------------------------------
void CheckOther::checkAssignmentInAssert()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char assertPattern[] = "assert ( %any%";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), assertPattern);
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok)
    {
        const Token* varTok = Token::findmatch(tok->tokAt(2), "%var% --|++|+=|-=|*=|/=|&=|^=|=", endTok);
        if (varTok)
        {
            assignmentInAssertError(tok, varTok->str());
        }
        else if (NULL != (varTok = Token::findmatch(tok->tokAt(2), "--|++ %var%", endTok)))
        {
            assignmentInAssertError(tok, varTok->strAt(1));
        }

        tok = Token::findmatch(endTok->next(), assertPattern);
        endTok = tok ? tok->next()->link() : NULL;
    }
}

//---------------------------------------------------------------------------
//    if ((x != 1) || (x != 3))            // <- always true
//---------------------------------------------------------------------------
void CheckOther::checkIncorrectLogicOperator()
{
    if (!_settings->_checkCodingStyle)
        return;

    const char conditionPattern[] = "if|while (";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), conditionPattern);
    const Token *endTok = tok ? tok->next()->link() : NULL;

    while (tok && endTok)
    {
        // Find a pair of OR'd terms, with or without parenthesis
        const Token *logicTok = NULL, *term1Tok = NULL, *term2Tok = NULL;
        if ((logicTok = Token::findmatch(tok, "( %any% != %any% ) %oror% ( %any% != %any% ) !!&&", endTok)))
        {
            term1Tok = logicTok->next();
            term2Tok = logicTok->tokAt(7);
        }
        else if ((logicTok = Token::findmatch(tok, "%any% != %any% %oror% %any% != %any% !!&&", endTok)))
        {
            term1Tok = logicTok;
            term2Tok = logicTok->tokAt(4);
        }

        // If both terms reference a common variable and are not AND'd with anything, this is an error
        if (logicTok && (logicTok->strAt(-1) != "&&"))
        {
            if (Token::Match(term1Tok, "%var%") &&
                ((term1Tok->str() == term2Tok->str()) ||
                 (term1Tok->str() == term2Tok->strAt(2))))
            {
                incorrectLogicOperatorError(term1Tok);
            }
            else if (Token::Match(term1Tok->tokAt(2), "%var%") &&
                     ((term1Tok->strAt(2) == term2Tok->str()) ||
                      (term1Tok->strAt(2) == term2Tok->strAt(2))))
            {
                incorrectLogicOperatorError(term1Tok->tokAt(2));
            }
        }

        tok = Token::findmatch(endTok->next(), conditionPattern);
        endTok = tok ? tok->next()->link() : NULL;
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
    reportError(tok, Severity::warning,
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

    // a = a + b;
    if (Token::Match(tok, "%var% = %var% !!;") && tok->str() == tok->strAt(2))
    {
        return 2;
    }

    // check for aliased variable
    const unsigned int varid1 = tok->varId();
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

                    Variables::VariableUsage *var2 = variables.find(tok->varId());
                    if (var2 && var2->_type == Variables::reference)
                    {
                        variables.writeAliases(tok->varId());
                        variables.read(tok->varId());
                    }
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
                const Variables::VariableUsage *var = variables.find(varid);

                if (var)
                {
                    if (var->_type == Variables::pointer || var->_type == Variables::reference)
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

            // possible unexpanded macro hiding for/while..
            if (Token::Match(tok->previous(), "[;{}] %type% {"))
            {
                bool upper = true;
                for (unsigned int i = 0; i < tok->str().length(); ++i)
                {
                    if (!std::isupper(tok->str()[i]))
                        upper = false;
                }
                for_or_while |= upper;
            }

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
                    structname.clear();
                    break;
                }

                if (tok2->str() == "}")
                    break;
            }

            // bail out if struct is inherited
            if (!structname.empty() && Token::findmatch(tok, (",|private|protected|public " + structname).c_str()))
                structname.clear();

            // Bail out if some data is casted to struct..
            const std::string s("( struct| " + tok->next()->str() + " * ) & %var% [");
            if (Token::findmatch(tok, s.c_str()))
                structname.clear();

            // Try to prevent false positives when struct members are not used directly.
            if (Token::findmatch(tok, (structname + " *").c_str()))
                structname.clear();
            else if (Token::findmatch(tok, (structname + " %type% *").c_str()))
                structname = "";
        }

        if (tok->str() == "}")
            structname.clear();

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

            // Check if the struct variable is used anywhere in the file
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
    // Don't use this check for Java and C# programs..
    if (_tokenizer->isJavaOrCSharp())
    {
        return;
    }

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


bool CheckOther::isIdentifierObjectType(const Token * const tok)
{
    const std::string identifier = tok->tokAt(1)->str();

    const std::map<std::string, bool>::const_iterator found = isClassResults.find(identifier);
    if (found != isClassResults.end())
    {
        return found->second;
    }

    const std::string classDefnOrDecl = std::string("class|struct ") + identifier + " [{:;]";
    const bool result = Token::findmatch(_tokenizer->tokens(), classDefnOrDecl.c_str()) != NULL;
    isClassResults.insert(std::make_pair(identifier, result));
    return result;
}


void CheckOther::checkMisusedScopedObject()
{
    bool withinFunction = false;
    unsigned int depth = 0;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        withinFunction |= Token::Match(tok, ") const| {");
        if (withinFunction)
        {
            if (tok->str() == "{")
            {
                ++depth;
            }
            else if (tok->str() == "}")
            {
                --depth;
                withinFunction &= depth > 0;
            }

            if (withinFunction
                && Token::Match(tok, "[;{}] %var% (")
                && Token::Match(tok->tokAt(2)->link(), ") ;")
                && isIdentifierObjectType(tok)
               )
            {
                tok = tok->next();
                misusedScopeObjectError(tok, tok->str());
                tok = tok->next();
            }
        }
    }
}

void CheckOther::cstyleCastError(const Token *tok)
{
    reportError(tok, Severity::style, "cstyleCast", "C-style pointer casting");
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
    reportError(tok, Severity::performance, "passedByValue", "Function parameter '" + parname + "' is passed by value. It could be passed by reference instead, to make it faster.");
}

void CheckOther::constStatementError(const Token *tok, const std::string &type)
{
    reportError(tok, Severity::warning, "constStatement", "Redundant code: Found a statement that begins with " + type + " constant");
}

void CheckOther::charArrayIndexError(const Token *tok)
{
    reportError(tok, Severity::warning, "charArrayIndex", "Warning - using char variable as array index");
}

void CheckOther::charBitOpError(const Token *tok)
{
    reportError(tok, Severity::warning, "charBitOp", "Warning - using char variable in bit operation");
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

void CheckOther::emptyStringTestError(const Token *tok, const std::string &var_name, const bool isTestForEmpty)
{
    if (isTestForEmpty)
    {
        reportError(tok, Severity::performance,
                    "emptyStringTest", "Empty string test can be simplified to \"*" + var_name + " == '\\0'\"");
    }
    else
    {
        reportError(tok, Severity::performance,
                    "emptyStringTest", "Non-empty string test can be simplified to \"*" + var_name + " != '\\0'\"");
    }
}

void CheckOther::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream \"" + varname + "\" may result in undefined behaviour");
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
    reportError(tok, Severity::warning,
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
    reportError(tok, Severity::warning,
                "sizeofCalculation", "Found calculation inside sizeof()");
}

void CheckOther::redundantAssignmentInSwitchError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "redundantAssignInSwitch", "Redundant assignment of \"" + varname + "\" in switch");
}

void CheckOther::selfAssignmentError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "selfAssignment", "Redundant assignment of \"" + varname + "\" to itself");
}

void CheckOther::assignmentInAssertError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning,
                "assignmentInAssert", "Assert statement modifies '" + varname + "'. If the modification is needed in release builds there is a bug.");
}

void CheckOther::incorrectLogicOperatorError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "incorrectLogicOperator", "Mutual exclusion over || always evaluates to true. Did you intend to use && instead?");
}

void CheckOther::misusedScopeObjectError(const Token *tok, const std::string& varname)
{
    reportError(tok, Severity::error,
                "unusedScopedObject", "instance of \"" + varname + "\" object destroyed immediately");
}
