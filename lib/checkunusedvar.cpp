/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "checkunusedvar.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckUnusedVar instance;
}

/**
 * @brief This class is used to capture the control flow within a function.
 */
class ScopeInfo
{
public:
    ScopeInfo() : _token(NULL), _parent(NULL) { }
    ScopeInfo(const Token *token, ScopeInfo *parent_) : _token(token), _parent(parent_) { }
    ~ScopeInfo();

    ScopeInfo *parent()
    {
        return _parent;
    }
    ScopeInfo *addChild(const Token *token);
    void remove(ScopeInfo *scope);

private:
    const Token *_token;
    ScopeInfo *_parent;
    std::list<ScopeInfo *> _children;
};

ScopeInfo::~ScopeInfo()
{
    while (!_children.empty())
    {
        delete *_children.begin();
        _children.pop_front();
    }
}

ScopeInfo *ScopeInfo::addChild(const Token *token)
{
    ScopeInfo *temp = new ScopeInfo(token, this);

    _children.push_back(temp);

    return temp;
}

void ScopeInfo::remove(ScopeInfo *scope)
{
    std::list<ScopeInfo *>::iterator it;

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
                      ScopeInfo *scope = NULL,
                      bool read = false,
                      bool write = false,
                      bool modified = false,
                      bool allocateMemory = false) :
            _name(name),
            _type(type),
            _scope(scope),
            _read(read),
            _write(write),
            _modified(modified),
            _allocateMemory(allocateMemory)
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
        ScopeInfo *_scope;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
        bool _allocateMemory;
        std::set<unsigned int> _aliases;
        std::set<ScopeInfo *> _assignments;
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
    void addVar(const Token *name, VariableType type, ScopeInfo *scope, bool write_);
    void allocateMemory(unsigned int varid);
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
                       ScopeInfo *scope,
                       bool write_)
{
    if (name->varId() > 0)
        _varUsage.insert(std::make_pair(name->varId(), VariableUsage(name, type, scope, false, write_, false)));
}

void Variables::allocateMemory(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_allocateMemory = true;
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

static int doAssignment(Variables &variables, const Token *tok, bool dereference, ScopeInfo *scope)
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

                            // not in same scope as declaration
                            else
                            {
                                std::set<ScopeInfo *>::iterator assignment;

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
                                    // replace when only one other assignment
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
                    // check if variable declaration is in this scope
                    if (var1->_scope == scope)
                        variables.clearAliases(varid1);
                    else
                    {
                        std::set<ScopeInfo *>::iterator assignment;

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

bool CheckUnusedVar::isRecordTypeWithoutSideEffects(const Token *tok)
{
    const Variable * var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->varId());

    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check base class for side effects */
    /** @todo false negative: check constructors for side effects */
    if (var && var->type() && var->type()->numConstructors == 0 &&
        (var->type()->varlist.empty() || var->type()->needInitialization == Scope::True) &&
        var->type()->derivedFrom.empty())
        return true;

    return false;
}

//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------
void CheckUnusedVar::checkFunctionVariableUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // First token for the current scope..
        const Token *const tok1 = scope->classStart;

        // varId, usage {read, write, modified}
        Variables variables;

        // scopes
        ScopeInfo scopes;
        ScopeInfo *info = &scopes;

        unsigned int indentlevel = 0;
        for (const Token *tok = tok1; tok; tok = tok->next())
        {
            if (tok->str() == "{")
            {
                // replace the head node when found
                if (indentlevel == 0)
                    scopes = ScopeInfo(tok, NULL);
                // add the new scope
                else
                    info = info->addChild(tok);
                ++indentlevel;
            }
            else if (tok->str() == "}")
            {
                --indentlevel;

                info = info->parent();

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
                !Token::Match(tok->next(), "return|throw"))
            {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->isStandardType() || isRecordTypeWithoutSideEffects(tok->next()))
                {
                    variables.addVar(tok->next(), Variables::standard, info,
                                     tok->tokAt(2)->str() == "=" || isStatic);
                }
                tok = tok->next();
            }

            // standard const type declaration
            // const int i = x;
            else if (Token::Match(tok, "[;{}] const %type% %var% ="))
            {
                tok = tok->next()->next();

                if (tok->isStandardType() || isRecordTypeWithoutSideEffects(tok->next()))
                    variables.addVar(tok->next(), Variables::standard, info, true);

                tok = tok->next();
            }

            // std::string declaration with possible initialization
            // std::string s; std::string s = "string";
            else if (Token::Match(tok, "[;{}] static| std :: string %var% ;|="))
            {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                tok = tok->tokAt(3);
                variables.addVar(tok, Variables::standard, info,
                                 tok->next()->str() == "=" || isStatic);
            }

            // standard struct type declaration with possible initialization
            // struct S s; struct S s = { 0 }; static struct S s;
            else if (Token::Match(tok, "[;{}] static| struct %type% %var% ;|=") &&
                     (isRecordTypeWithoutSideEffects(tok->strAt(1) == "static" ? tok->tokAt(4) : tok->tokAt(3))))
            {
                tok = tok->next();

                bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                tok = tok->next();

                variables.addVar(tok->next(), Variables::standard, info,
                                 tok->tokAt(2)->str() == "=" || isStatic);
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

                variables.addVar(tok->next(), Variables::standard, info, true);

                // check if a local variable is used to initialize this variable
                if (tok->tokAt(3)->varId() > 0)
                    variables.readAll(tok->tokAt(3)->varId());
                tok = tok->tokAt(4);
            }

            // standard type declaration of array of with possible initialization
            // int i[10]; int j[2] = { 0, 1 }; static int k[2] = { 2, 3 };
            else if (Token::Match(tok, "[;{}] static| const| %type% *| %var% [ %any% ] ;|=") &&
                     nextIsStandardType(tok))
            {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return" && tok->str() != "throw")
                {
                    bool isPointer = bool(tok->strAt(1) == "*");
                    const Token * const nametok = tok->tokAt(isPointer ? 2 : 1);

                    variables.addVar(nametok, isPointer ? Variables::pointerArray : Variables::array, info,
                                     nametok->tokAt(4)->str() == "=" || isStatic);

                    // check for reading array size from local variable
                    if (nametok->tokAt(2)->varId() != 0)
                        variables.read(nametok->tokAt(2)->varId());

                    // look at initializers
                    if (Token::simpleMatch(nametok->tokAt(4), "= {"))
                    {
                        tok = nametok->tokAt(6);
                        while (tok->str() != "}")
                        {
                            if (Token::Match(tok, "%var%"))
                                variables.read(tok->varId());
                            tok = tok->next();
                        }
                    }
                    else
                        tok = nametok->tokAt(3);
                }
            }

            // pointer or reference declaration with possible initialization
            // int * i; int * j = 0; static int * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% *|& %var% ;|="))
            {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->strAt(1) == "::")
                    tok = tok->tokAt(2);

                if (tok->str() != "return" && tok->str() != "throw")
                {
                    Variables::VariableType type;

                    if (tok->next()->str() == "*")
                        type = Variables::pointer;
                    else
                        type = Variables::reference;

                    bool written = tok->tokAt(3)->str() == "=";

                    variables.addVar(tok->tokAt(2), type, info, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(2), false, info);

                    tok = tok->tokAt(2 + offset);
                }
            }

            // pointer to pointer declaration with possible initialization
            // int ** i; int ** j = 0; static int ** k = 0;
            else if (Token::Match(tok, "[;{}] static| const| %type% * * %var% ;|="))
            {
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return")
                {
                    bool written = tok->tokAt(4)->str() == "=";

                    variables.addVar(tok->tokAt(3), Variables::pointerPointer, info, written || isStatic);

                    int offset = 0;

                    // check for assignment
                    if (written)
                        offset = doAssignment(variables, tok->tokAt(3), false, info);

                    tok = tok->tokAt(3 + offset);
                }
            }

            // pointer or reference of struct or union declaration with possible initialization
            // struct s * i; struct s * j = 0; static struct s * k = 0;
            else if (Token::Match(tok, "[;{}] static| const| struct|union %type% *|& %var% ;|="))
            {
                Variables::VariableType type;

                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->strAt(2) == "*")
                    type = Variables::pointer;
                else
                    type = Variables::reference;

                const bool written = tok->strAt(4) == "=";

                variables.addVar(tok->tokAt(3), type, info, written || isStatic);

                int offset = 0;

                // check for assignment
                if (written)
                    offset = doAssignment(variables, tok->tokAt(3), false, info);

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

                variables.addVar(tok->tokAt(2), type, info, true);

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
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                if (tok->str() != "return")
                {
                    variables.addVar(tok->tokAt(2),
                                     tok->next()->str() == "*" ? Variables::pointerArray : Variables::referenceArray, info,
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
                tok = tok->next();

                const bool isStatic = tok->str() == "static";
                if (isStatic)
                    tok = tok->next();

                if (tok->str() == "const")
                    tok = tok->next();

                variables.addVar(tok->tokAt(3),
                                 tok->tokAt(2)->str() == "*" ? Variables::pointerArray : Variables::referenceArray, info,
                                 tok->tokAt(7)->str() == "=" || isStatic);

                // check for reading array size from local variable
                if (tok->tokAt(5)->varId() != 0)
                    variables.read(tok->tokAt(5)->varId());

                tok = tok->tokAt(6);
            }

            // Freeing memory (not considered "using" the pointer if it was also allocated in this function)
            else if (Token::Match(tok, "free|g_free|kfree|vfree ( %var% )") ||
                     Token::Match(tok, "delete %var% ;") ||
                     Token::Match(tok, "delete [ ] %var% ;"))
            {
                unsigned int varid = 0;
                if (tok->str() != "delete")
                {
                    varid = tok->tokAt(2)->varId();
                    tok = tok->tokAt(3);
                }
                else if (tok->strAt(1) == "[")
                {
                    varid = tok->tokAt(3)->varId();
                    tok = tok->tokAt(4);
                }
                else
                {
                    varid = tok->next()->varId();
                    tok = tok->tokAt(2);
                }

                Variables::VariableUsage *var = variables.find(varid);
                if (var && !var->_allocateMemory)
                {
                    variables.readAll(varid);
                }
            }

            else if (Token::Match(tok, "return|throw %var%"))
                variables.readAll(tok->next()->varId());

            // assignment
            else if (!Token::Match(tok->tokAt(-2), "[;{}.] %var% (") &&
                     (Token::Match(tok, "*| (| ++|--| %var% ++|--| )| =") ||
                      Token::Match(tok, "*| ( const| %type% *| ) %var% =")))
            {
                bool dereference = false;
                bool pre = false;
                bool post = false;

                if (tok->str() == "*")
                {
                    dereference = true;
                    tok = tok->next();
                }

                if (Token::Match(tok, "( const| %type% *| ) %var% ="))
                    tok = tok->link()->next();

                else if (tok->str() == "(")
                    tok = tok->next();

                if (Token::Match(tok, "++|--"))
                {
                    pre = true;
                    tok = tok->next();
                }

                if (Token::Match(tok->next(), "++|--"))
                    post = true;

                const unsigned int varid1 = tok->varId();
                const Token *start = tok;

                tok = tok->tokAt(doAssignment(variables, tok, dereference, info));

                if (pre || post)
                    variables.use(varid1);

                if (dereference)
                {
                    Variables::VariableUsage *var = variables.find(varid1);
                    if (var && var->_type == Variables::array)
                        variables.write(varid1);
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
                    // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                    else if (var && var->_type == Variables::pointer &&
                             Token::Match(start, "%var% = new|malloc|calloc|g_malloc|kmalloc|vmalloc"))
                    {
                        bool allocate = true;

                        if (start->strAt(2) == "new")
                        {
                            const Token *type = start->tokAt(3);

                            // skip nothrow
                            if (Token::Match(type, "( nothrow )") ||
                                Token::Match(type, "( std :: nothrow )"))
                                type = type->link()->next();

                            // is it a user defined type?
                            if (!type->isStandardType())
                            {
                                if (!isRecordTypeWithoutSideEffects(start))
                                    allocate = false;
                            }
                        }

                        if (allocate)
                            variables.allocateMemory(varid1);
                        else
                            variables.write(varid1);
                    }
                    else if (varid1 && Token::Match(tok, "%varid% .", varid1))
                    {
                        variables.use(varid1);
                    }
                    else
                    {
                        variables.write(varid1);
                    }

                    Variables::VariableUsage *var2 = variables.find(tok->varId());
                    if (var2)
                    {
                        if (var2->_type == Variables::reference)
                        {
                            variables.writeAliases(tok->varId());
                            variables.read(tok->varId());
                        }
                        else if (tok->varId() != varid1 && Token::Match(tok, "%var% ."))
                            variables.read(tok->varId());
                        else if (tok->varId() != varid1 &&
                                 var2->_type == Variables::standard &&
                                 tok->strAt(-1) != "&")
                            variables.use(tok->varId());
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
            else if (Token::Match(tok, "%var% [") && Token::simpleMatch(tok->next()->link(), "] ="))
            {
                unsigned int varid = tok->varId();
                const Variables::VariableUsage *var = variables.find(varid);

                if (var)
                {
                    // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                    if (var->_type == Variables::pointer &&
                        Token::Match(tok->next()->link(), "] = new|malloc|calloc|g_malloc|kmalloc|vmalloc"))
                    {
                        variables.allocateMemory(varid);
                    }
                    else if (var->_type == Variables::pointer || var->_type == Variables::reference)
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
            else if (Token::Match(tok, "[;{}] %var% >>"))
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
            else if (Token::Match(tok, "%var% ("))
            {
                variables.read(tok->varId());
                if (Token::Match(tok->tokAt(2), "%var% ="))
                    variables.read(tok->tokAt(2)->varId());
            }

            else if (Token::Match(tok, "[{,] %var% [,}]"))
                variables.read(tok->next()->varId());

            else if (Token::Match(tok, "%var% ."))
                variables.use(tok->varId());   // use = read + write

            else if ((Token::Match(tok, "[(=&!]") || tok->isExtendedOp()) &&
                     (Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new")))
                variables.readAll(tok->next()->varId());

            else if (Token::Match(tok, "%var%") && (tok->next()->str() == ")" || tok->next()->isExtendedOp()))
                variables.readAll(tok->varId());

            else if (Token::Match(tok, "; %var% ;"))
                variables.readAll(tok->next()->varId());

            if (Token::Match(tok, "++|-- %var%"))
            {
                if (tok->strAt(-1) != ";")
                    variables.use(tok->next()->varId());
                else
                    variables.modified(tok->next()->varId());
            }

            else if (Token::Match(tok, "%var% ++|--"))
            {
                if (tok->strAt(-1) != ";")
                    variables.use(tok->varId());
                else
                    variables.modified(tok->varId());
            }

            else if (tok->isAssignmentOp())
            {
                for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next())
                {
                    if (tok2->varId())
                    {
                        variables.read(tok2->varId());
                        if (tok2->next()->isAssignmentOp())
                            variables.write(tok2->varId());
                    }
                }
            }
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

            // variable has had memory allocated for it, but hasn't done
            // anything with that memory other than, perhaps, freeing it
            if (usage.unused() && !usage._modified && usage._allocateMemory)
                allocatedButUnusedVariableError(usage._name, varname);

            // variable has not been written, read, or modified
            else if (usage.unused() && !usage._modified)
                unusedVariableError(usage._name, varname);

            // variable has not been written but has been modified
            else if (usage._modified & !usage._write)
                unassignedVariableError(usage._name, varname);

            // variable has been written but not read
            else if (!usage._read && !usage._modified)
                unreadVariableError(usage._name, varname);

            // variable has been read but not written
            else if (!usage._write && !usage._allocateMemory)
                unassignedVariableError(usage._name, varname);
        }
    }
}

void CheckUnusedVar::unusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedVariable", "Unused variable: " + varname);
}

void CheckUnusedVar::allocatedButUnusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedAllocatedMemory", "Variable '" + varname + "' is allocated memory that is never used");
}

void CheckUnusedVar::unreadVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unreadVariable", "Variable '" + varname + "' is assigned a value that is never used");
}

void CheckUnusedVar::unassignedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unassignedVariable", "Variable '" + varname + "' is not assigned a value");
}

//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------
void CheckUnusedVar::checkStructMemberUsage()
{
    if (!_settings->isEnabled("style"))
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

            // declaring a POD variable?
            if (!tok->next()->isStandardType())
                continue;

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

void CheckUnusedVar::unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedStructMember", "struct or union member '" + structname + "::" + varname + "' is never used");
}

