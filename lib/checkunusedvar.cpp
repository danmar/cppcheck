/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
namespace {
    CheckUnusedVar instance;
}

/**
 * @brief This class is used create a list of variables within a function.
 */
class Variables {
public:
    enum VariableType { standard, array, pointer, reference, pointerArray, referenceArray, pointerPointer, none };

    /** Store information about variable usage */
    class VariableUsage {
    public:
        VariableUsage(const Token *name = 0,
                      VariableType type = standard,
                      const Scope *scope = NULL,
                      bool read = false,
                      bool write = false,
                      bool modified = false,
                      bool allocateMemory = false) :
            _name(name),
            _scope(scope),
            _type(type),
            _read(read),
            _write(write),
            _modified(modified),
            _allocateMemory(allocateMemory) {
        }

        /** variable is used.. set both read+write */
        void use() {
            _read = true;
            _write = true;
        }

        /** is variable unused? */
        bool unused() const {
            return (_read == false && _write == false);
        }

        std::set<unsigned int> _aliases;
        std::set<const Scope*> _assignments;

        const Token *_name;
        const Scope *_scope;
        VariableType _type;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
        bool _allocateMemory;
    };

    typedef std::map<unsigned int, VariableUsage> VariableMap;

    void clear() {
        _varUsage.clear();
    }
    const VariableMap &varUsage() const {
        return _varUsage;
    }
    void addVar(const Token *name, VariableType type, const Scope *scope, bool write_);
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
    void erase(unsigned int varid) {
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
    if (varid1 == varid2) {
        if (var1)
            var1->use();
        return;
    }

    if (replace) {
        // remove var1 from all aliases
        for (std::set<unsigned int>::iterator i = var1->_aliases.begin(); i != var1->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(var1->_name->varId());
        }

        // remove all aliases from var1
        var1->_aliases.clear();
    }

    // var1 gets all var2s aliases
    for (std::set<unsigned int>::iterator i = var2->_aliases.begin(); i != var2->_aliases.end(); ++i) {
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

    if (usage) {
        // remove usage from all aliases
        std::set<unsigned int>::iterator i;

        for (i = usage->_aliases.begin(); i != usage->_aliases.end(); ++i) {
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

    if (usage) {
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
                       const Scope *scope,
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

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_read = true;
        }
    }
}

void Variables::readAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_read = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
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

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::writeAll(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_write = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_write = true;
        }
    }
}

void Variables::use(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->use();

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->use();
        }
    }
}

void Variables::modified(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_modified = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
                aliased->_modified = true;
        }
    }
}

Variables::VariableUsage *Variables::find(unsigned int varid)
{
    if (varid) {
        VariableMap::iterator i = _varUsage.find(varid);
        if (i != _varUsage.end())
            return &i->second;
    }
    return 0;
}

static const Token* doAssignment(Variables &variables, const Token *tok, bool dereference, const Scope *scope)
{
    // a = a + b;
    if (Token::Match(tok, "%var% = %var% !!;") && tok->varId() == tok->tokAt(2)->varId()) {
        return tok->tokAt(2);
    }

    const Token* const tokOld = tok;

    // check for aliased variable
    const unsigned int varid1 = tok->varId();
    Variables::VariableUsage *var1 = variables.find(varid1);

    if (var1) {
        // jump behind '='
        tok = tok->next();
        while (tok->str() != "=") {
            if (tok->varId())
                variables.read(tok->varId());
            tok = tok->next();
        }
        tok = tok->next();

        if (Token::Match(tok, "&| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) &| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) ( &| %var%") ||
            Token::Match(tok->next(), "< const| struct|union| %type% *| > ( &| %var%")) {
            bool addressOf = false;

            if (Token::Match(tok, "%var% ."))
                variables.use(tok->varId());   // use = read + write

            // check for C style cast
            if (tok->str() == "(") {
                tok = tok->next();
                if (tok->str() == "const")
                    tok = tok->next();

                if (Token::Match(tok, "struct|union"))
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "*")
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                } else if (tok->str() == "(") {
                    tok = tok->next();
                    if (tok->str() == "&") {
                        addressOf = true;
                        tok = tok->next();
                    }
                }
            }

            // check for C++ style cast
            else if (tok->str().find("cast") != std::string::npos &&
                     tok->strAt(1) == "<") {
                tok = tok->tokAt(2);
                if (tok->str() == "const")
                    tok = tok->next();

                if (Token::Match(tok, "struct|union"))
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "*")
                    tok = tok->next();

                tok = tok->tokAt(2);
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                }
            }

            // no cast, no ?
            else if (!Token::Match(tok, "%var% ?")) {
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                } else if (tok->str() == "new")
                    return tokOld;
            }

            // check if variable is local
            unsigned int varid2 = tok->varId();
            Variables::VariableUsage* var2 = variables.find(varid2);

            if (var2) { // local variable (alias or read it)
                if (var1->_type == Variables::pointer || var1->_type == Variables::pointerArray) {
                    if (dereference)
                        variables.read(varid2);
                    else {
                        if (addressOf ||
                            var2->_type == Variables::array ||
                            var2->_type == Variables::pointer) {
                            bool replace = true;

                            // pointerArray => don't replace
                            if (var1->_type == Variables::pointerArray)
                                replace = false;

                            // check if variable declared in same scope
                            else if (scope == var1->_scope)
                                replace = true;

                            // not in same scope as declaration
                            else {
                                // no other assignment in this scope
                                if (var1->_assignments.find(scope) == var1->_assignments.end() ||
                                    scope->type == Scope::eSwitch) {
                                    // nothing to replace
                                    if (var1->_assignments.empty())
                                        replace = false;

                                    // this variable has previous assignments
                                    else {
                                        /**
                                         * @todo determine if existing aliases should be replaced or merged
                                         */

                                        replace = false;
                                    }
                                }

                                // assignment in this scope
                                else {
                                    // replace when only one other assignment, merge them otherwise
                                    replace = (var1->_assignments.size() == 1);
                                }
                            }

                            variables.alias(varid1, varid2, replace);
                        } else if (tok->strAt(1) == "?") {
                            if (var2->_type == Variables::reference)
                                variables.readAliases(varid2);
                            else
                                variables.read(varid2);
                        } else {
                            variables.read(varid2);
                        }
                    }
                } else if (var1->_type == Variables::reference) {
                    variables.alias(varid1, varid2, true);
                } else {
                    if (var2->_type == Variables::pointer && tok->strAt(1) == "[")
                        variables.readAliases(varid2);

                    variables.read(varid2);
                }
            } else { // not a local variable (or an unsupported local variable)
                if (var1->_type == Variables::pointer && !dereference) {
                    // check if variable declaration is in this scope
                    if (var1->_scope == scope)
                        variables.clearAliases(varid1);
                    else {
                        // no other assignment in this scope
                        if (var1->_assignments.find(scope) == var1->_assignments.end()) {
                            /**
                             * @todo determine if existing aliases should be discarded
                             */
                        }

                        // this assignment replaces the last assignment in this scope
                        else {
                            // aliased variables in a larger scope are not supported
                            // remove all aliases
                            variables.clearAliases(varid1);
                        }
                    }
                }
            }
        } else
            tok = tokOld;

        var1->_assignments.insert(scope);
    }

    // check for alias to struct member
    // char c[10]; a.b = c;
    else if (Token::Match(tok->tokAt(-2), "%var% .")) {
        if (Token::Match(tok->tokAt(2), "%var%")) {
            unsigned int varid2 = tok->tokAt(2)->varId();
            Variables::VariableUsage *var2 = variables.find(varid2);

            // struct member aliased to local variable
            if (var2 && (var2->_type == Variables::array ||
                         var2->_type == Variables::pointer)) {
                // erase aliased variable and all variables that alias it
                // to prevent false positives
                variables.eraseAll(varid2);
            }
        }
    }

    return tok;
}

static bool isRecordTypeWithoutSideEffects(const Scope* type)
{
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check constructors for side effects */
    if (type && type->numConstructors == 0 &&
        (type->varlist.empty() || type->needInitialization == Scope::True)) {
        bool yes = true;
        for (std::vector<Scope::BaseInfo>::const_iterator i = type->derivedFrom.begin(); yes && i != type->derivedFrom.end(); ++i)
            yes = isRecordTypeWithoutSideEffects(i->scope);
        return yes;
    }
    return false;
}

static bool isPartOfClassStructUnion(const Token* tok)
{
    for (; tok; tok = tok->previous()) {
        if (tok->str() == "}" || tok->str() == ")")
            tok = tok->link();
        else if (tok->str() == "(")
            return(false);
        else if (tok->str() == "{") {
            return(tok->strAt(-1) == "struct" || tok->strAt(-2) == "struct" || tok->strAt(-1) == "class" || tok->strAt(-2) == "class" || tok->strAt(-1) == "union" || tok->strAt(-2) == "union");
        }
    }
    return false;
}

// Skip [ .. ]
static const Token * skipBrackets(const Token *tok)
{
    while (tok && tok->str() == "[")
        tok = tok->link()->next();
    return tok;
}


// Skip [ .. ] . x
static const Token * skipBracketsAndMembers(const Token *tok)
{
    while (tok) {
        if (tok->str() == "[")
            tok = tok->link()->next();
        else if (Token::Match(tok, ". %var%"))
            tok = tok->tokAt(2);
        else
            break;
    }
    return tok;
}


//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------
void CheckUnusedVar::checkFunctionVariableUsage_iterateScopes(const Scope* const scope, Variables& variables)
{
    // Find declarations if the scope is executable..
    if (scope->type != Scope::eClass && scope->type != Scope::eUnion && scope->type != Scope::eStruct) {
        // Find declarations
        for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
            if (i->isThrow() || i->isExtern())
                continue;
            Variables::VariableType type = Variables::none;
            if (i->isArray() && (i->nameToken()->previous()->str() == "*" || i->nameToken()->strAt(-2) == "*"))
                type = Variables::pointerArray;
            else if (i->isArray() && i->nameToken()->previous()->str() == "&")
                type = Variables::referenceArray;
            else if (i->isArray())
                type = Variables::array;
            else if (i->isReference())
                type = Variables::reference;
            else if (i->nameToken()->previous()->str() == "*" && i->nameToken()->strAt(-2) == "*")
                type = Variables::pointerPointer;
            else if (i->isPointer())
                type = Variables::pointer;
            else if (_tokenizer->isC() || i->typeEndToken()->isStandardType() || isRecordTypeWithoutSideEffects(i->type()) || Token::simpleMatch(i->typeStartToken(), "std ::"))
                type = Variables::standard;
            if (type == Variables::none || isPartOfClassStructUnion(i->typeStartToken()))
                continue;
            const Token* defValTok = i->nameToken()->next();
            for (; defValTok; defValTok = defValTok->next()) {
                if (defValTok->str() == "[")
                    defValTok = defValTok->link();
                else if (defValTok->str() == "(" || defValTok->str() == "=") {
                    variables.addVar(i->nameToken(), type, scope, true);
                    break;
                } else if (defValTok->str() == ";" || defValTok->str() == "," || defValTok->str() == ")") {
                    variables.addVar(i->nameToken(), type, scope, i->isStatic());
                    break;
                }
            }
            if (i->isArray() && i->isClass()) // Array of class/struct members. Initialized by ctor.
                variables.write(i->varId());
            if (i->isArray() && Token::Match(i->nameToken(), "%var% [ %var% ]")) // Array index variable read.
                variables.read(i->nameToken()->tokAt(2)->varId());

            if (defValTok && defValTok->str() == "=") {
                if (defValTok->next() && defValTok->next()->str() == "{") {
                    for (const Token* tok = defValTok; tok && tok != defValTok->linkAt(1); tok = tok->next())
                        if (Token::Match(tok, "%var%")) // Variables used to initialize the array read.
                            variables.read(tok->varId());
                } else
                    doAssignment(variables, i->nameToken(), false, scope);
            } else if (Token::Match(defValTok, "( %var% )")) // Variables used to initialize the variable read.
                variables.readAll(defValTok->next()->varId()); // ReadAll?
        }
    }

    // Check variable usage
    for (const Token *tok = scope->classDef->next(); tok && tok != scope->classEnd; tok = tok->next()) {
        if (tok->str() == "for") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classDef == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(*i, variables); // Scan child scope
                    tok = (*i)->classStart->link();
                    break;
                }
            }
            if (!tok)
                break;
        }
        if (tok->str() == "{") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classStart == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(*i, variables); // Scan child scope
                    tok = tok->link();
                    break;
                }
            }
            if (!tok)
                break;
        }

        if (Token::Match(tok, "asm ( %str% )")) {
            variables.clear();
            break;
        }

        if (Token::Match(tok->previous(), "[;{}]")) {
            for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    const Variable* var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok2->varId());
                    if (var && var->nameToken() == tok2) { // Declaration: Skip
                        tok = tok2->next();
                        if (Token::Match(tok, "( %var% )")) // Simple initialization through copy ctor
                            tok = tok->next();
                        else if (Token::Match(tok, "= %var% ;")) // Simple initialization
                            tok = tok->next();
                        else if (var->typeEndToken()->str() == ">") // Be careful with types like std::vector
                            tok = tok->previous();
                        break;
                    }
                } else if (Token::Match(tok2, "[;({=]"))
                    break;
            }
        }
        // Freeing memory (not considered "using" the pointer if it was also allocated in this function)
        if (Token::Match(tok, "free|g_free|kfree|vfree ( %var% )") ||
            Token::Match(tok, "delete %var% ;") ||
            Token::Match(tok, "delete [ ] %var% ;")) {
            unsigned int varid = 0;
            if (tok->str() != "delete") {
                varid = tok->tokAt(2)->varId();
                tok = tok->tokAt(3);
            } else if (tok->strAt(1) == "[") {
                varid = tok->tokAt(3)->varId();
                tok = tok->tokAt(3);
            } else {
                varid = tok->next()->varId();
                tok = tok->next();
            }

            Variables::VariableUsage *var = variables.find(varid);
            if (var && !var->_allocateMemory) {
                variables.readAll(varid);
            }
        }

        else if (Token::Match(tok, "return|throw")) {
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId())
                    variables.readAll(tok2->varId());
                else if (tok2->str() == ";")
                    break;
            }
        }

        // assignment
        else if (!Token::Match(tok->tokAt(-2), "[;{}.] %var% (") &&
                 (Token::Match(tok, "*| (| ++|--| %var% ++|--| )| =") ||
                  Token::Match(tok, "*| ( const| %type% *| ) %var% ="))) {
            bool dereference = false;
            bool pre = false;
            bool post = false;

            if (tok->str() == "*") {
                dereference = true;
                tok = tok->next();
            }

            if (Token::Match(tok, "( const| %type% *| ) %var% ="))
                tok = tok->link()->next();

            else if (tok->str() == "(")
                tok = tok->next();

            if (tok->type() == Token::eIncDecOp) {
                pre = true;
                tok = tok->next();
            }

            if (tok->next()->type() == Token::eIncDecOp)
                post = true;

            const unsigned int varid1 = tok->varId();
            const Token *start = tok;

            tok = doAssignment(variables, tok, dereference, scope);

            if (pre || post)
                variables.use(varid1);

            if (dereference) {
                Variables::VariableUsage *var = variables.find(varid1);
                if (var && var->_type == Variables::array)
                    variables.write(varid1);
                variables.writeAliases(varid1);
                variables.read(varid1);
            } else {
                Variables::VariableUsage *var = variables.find(varid1);
                if (var && var->_type == Variables::reference) {
                    variables.writeAliases(varid1);
                    variables.read(varid1);
                }
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                else if (var && var->_type == Variables::pointer &&
                         Token::Match(start, "%var% = new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    bool allocate = true;

                    if (start->strAt(2) == "new") {
                        const Token *type = start->tokAt(3);

                        // skip nothrow
                        if (Token::simpleMatch(type, "( nothrow )") ||
                            Token::simpleMatch(type, "( std :: nothrow )"))
                            type = type->link()->next();

                        // is it a user defined type?
                        if (!type->isStandardType()) {
                            const Variable* variable = _tokenizer->getSymbolDatabase()->getVariableFromVarId(start->varId());
                            if (!variable || !isRecordTypeWithoutSideEffects(variable->type()))
                                allocate = false;
                        }
                    }

                    if (allocate)
                        variables.allocateMemory(varid1);
                    else
                        variables.write(varid1);
                } else if (varid1 && Token::Match(tok, "%varid% .", varid1)) {
                    variables.use(varid1);
                } else {
                    variables.write(varid1);
                }

                Variables::VariableUsage *var2 = variables.find(tok->varId());
                if (var2) {
                    if (var2->_type == Variables::reference) {
                        variables.writeAliases(tok->varId());
                        variables.read(tok->varId());
                    } else if (tok->varId() != varid1 && Token::Match(tok, "%var% ."))
                        variables.read(tok->varId());
                    else if (tok->varId() != varid1 &&
                             var2->_type == Variables::standard &&
                             tok->strAt(-1) != "&")
                        variables.use(tok->varId());
                }
            }

            const Token * const equal = skipBracketsAndMembers(tok->next());

            // checked for chained assignments
            if (tok != start && equal && equal->str() == "=") {
                Variables::VariableUsage *var = variables.find(tok->varId());

                if (var && var->_type != Variables::reference)
                    var->_read = true;

                tok = tok->previous();
            }
        }

        // assignment
        else if ((Token::Match(tok, "%var% [") && Token::simpleMatch(skipBracketsAndMembers(tok->next()), "=")) ||
                 (Token::simpleMatch(tok, "* (") && Token::simpleMatch(tok->next()->link(), ") ="))) {
            if (tok->str() == "*") {
                tok = tok->tokAt(2);
                if (tok->str() == "(")
                    tok = tok->link()->next();
            }

            unsigned int varid = tok->varId();
            const Variables::VariableUsage *var = variables.find(varid);

            if (var) {
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                if (var->_type == Variables::pointer &&
                    Token::Match(skipBrackets(tok->next()), "= new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    variables.allocateMemory(varid);
                } else if (var->_type == Variables::pointer || var->_type == Variables::reference) {
                    variables.read(varid);
                    variables.writeAliases(varid);
                } else if (var->_type == Variables::pointerArray) {
                    tok = doAssignment(variables, tok, false, scope);
                } else
                    variables.writeAll(varid);
            }
        }

        else if (Token::Match(tok, "& %var%")) {
            if (tok->previous()->isName() || tok->previous()->isNumber()) { // bitop
                variables.read(tok->next()->varId());
            } else // addressof
                variables.use(tok->next()->varId()); // use = read + write
        } else if (Token::Match(tok, ">> %var%"))
            variables.use(tok->next()->varId()); // use = read + write
        else if (Token::Match(tok, "%var% >>|&") && Token::Match(tok->previous(), "[{};:]"))
            variables.read(tok->varId());

        // function parameter
        else if (Token::Match(tok, "[(,] %var% ["))
            variables.use(tok->next()->varId());   // use = read + write
        else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*") {
            variables.use(tok->next()->varId());   // use = read + write
        } else if (Token::Match(tok, "[(,] (") &&
                   Token::Match(tok->next()->link(), ") %var% [,)]"))
            variables.use(tok->next()->link()->next()->varId());   // use = read + write

        // function
        else if (Token::Match(tok, "%var% (")) {
            variables.read(tok->varId());
            if (Token::Match(tok->tokAt(2), "%var% ="))
                variables.read(tok->tokAt(2)->varId());
        }

        else if (Token::Match(tok, "[{,] %var% [,}]"))
            variables.read(tok->next()->varId());

        else if (Token::Match(tok, "%var% ."))
            variables.use(tok->varId());   // use = read + write

        else if (tok->isExtendedOp() &&
                 Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new") && tok->strAt(2) != "=")
            variables.readAll(tok->next()->varId());

        else if (Token::Match(tok, "%var%") && tok->next() && (tok->next()->str() == ")" || tok->next()->isExtendedOp()))
            variables.readAll(tok->varId());

        else if (Token::Match(tok, "%var% ;") && Token::Match(tok->previous(), "[;{}:]"))
            variables.readAll(tok->varId());

        else if (Token::Match(tok, "++|-- %var%")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variables.use(tok->next()->varId());
            else
                variables.modified(tok->next()->varId());
        }

        else if (Token::Match(tok, "%var% ++|--")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variables.use(tok->varId());
            else
                variables.modified(tok->varId());
        }

        else if (tok->isAssignmentOp()) {
            for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    if (tok2->next()->isAssignmentOp())
                        variables.write(tok2->varId());
                    else
                        variables.read(tok2->varId());
                }
            }
        }
    }
}

void CheckUnusedVar::checkFunctionVariableUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

        // varId, usage {read, write, modified}
        Variables variables;

        checkFunctionVariableUsage_iterateScopes(&*scope, variables);


        // Check usage of all variables in the current scope..
        for (Variables::VariableMap::const_iterator it = variables.varUsage().begin(); it != variables.varUsage().end(); ++it) {
            const Variables::VariableUsage &usage = it->second;
            const std::string &varname = usage._name->str();
            const Variable* var = symbolDatabase->getVariableFromVarId(it->first);

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
            else if (usage._modified && !usage._write && !usage._allocateMemory && !Token::simpleMatch(var->typeStartToken(), "std ::"))
                unassignedVariableError(usage._name, varname);

            // variable has been written but not read
            else if (!usage._read && !usage._modified)
                unreadVariableError(usage._name, varname);

            // variable has been read but not written
            else if (!usage._write && !usage._allocateMemory && !Token::simpleMatch(var->typeStartToken(), "std ::"))
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "struct|union %type% {")) {
            structname.clear();
            if (tok->strAt(-1) == "extern")
                continue;
            if ((!tok->previous() || tok->previous()->str() == ";") && Token::Match(tok->linkAt(2), ("} ; " + tok->strAt(1) + " %var% ;").c_str()))
                continue;

            structname = tok->strAt(1);

            // Bail out if struct/union contain any functions
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(") {
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

        if (!structname.empty() && Token::Match(tok, "[{;]")) {
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
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, usagePattern.c_str())) {
                    used = true;
                    break;
                }
            }

            if (! used) {
                unusedStructMemberError(tok->next(), structname, varname);
            }
        }
    }
}

void CheckUnusedVar::unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedStructMember", "struct or union member '" + structname + "::" + varname + "' is never used");
}
