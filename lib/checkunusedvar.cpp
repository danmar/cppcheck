/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include <algorithm>
#include <cctype>
#include <utility>
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
        explicit VariableUsage(const Variable *var = nullptr,
                               VariableType type = standard,
                               bool read = false,
                               bool write = false,
                               bool modified = false,
                               bool allocateMemory = false) :
            _var(var),
            _lastAccess(var?var->nameToken():0),
            _type(type),
            _read(read),
            _write(write),
            _modified(modified),
            _allocateMemory(allocateMemory) {
        }

        /** variable is used.. set both read+write */
        void use(std::list<std::set<unsigned int> > & varReadInScope) {
            varReadInScope.back().insert(_var->declarationId());
            _read = true;
            _write = true;
        }

        /** is variable unused? */
        bool unused() const {
            return (_read == false && _write == false);
        }

        std::set<unsigned int> _aliases;
        std::set<const Scope*> _assignments;

        const Variable* _var;
        const Token* _lastAccess;
        VariableType _type;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
        bool _allocateMemory;
    };

    class ScopeGuard {
    public:
        ScopeGuard(Variables & guarded,
                   bool insideLoop)
            :_guarded(guarded),
             _insideLoop(insideLoop) {
            _guarded.enterScope();
        }

        ~ScopeGuard() {
            _guarded.leaveScope(_insideLoop);
        }

    private:
        /** No implementation */
        ScopeGuard& operator=(const ScopeGuard &);

        Variables & _guarded;
        bool _insideLoop;
    };

    void clear() {
        _varUsage.clear();
    }
    const std::map<unsigned int, VariableUsage> &varUsage() const {
        return _varUsage;
    }
    void addVar(const Variable *var, VariableType type, bool write_);
    void allocateMemory(unsigned int varid, const Token* tok);
    void read(unsigned int varid, const Token* tok);
    void readAliases(unsigned int varid, const Token* tok);
    void readAll(unsigned int varid, const Token* tok);
    void write(unsigned int varid, const Token* tok);
    void writeAliases(unsigned int varid, const Token* tok);
    void writeAll(unsigned int varid, const Token* tok);
    void use(unsigned int varid, const Token* tok);
    void modified(unsigned int varid, const Token* tok);
    VariableUsage *find(unsigned int varid);
    void alias(unsigned int varid1, unsigned int varid2, bool replace);
    void erase(unsigned int varid) {
        _varUsage.erase(varid);
    }
    void eraseAliases(unsigned int varid);
    void eraseAll(unsigned int varid);
    void clearAliases(unsigned int varid);

    ScopeGuard newScope(bool insideLoop) {
        return ScopeGuard(*this, insideLoop);
    }

private:
    void enterScope();
    void leaveScope(bool insideLoop);

    std::map<unsigned int, VariableUsage> _varUsage;
    std::list<std::set<unsigned int> > _varAddedInScope;
    std::list<std::set<unsigned int> > _varReadInScope;
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

    if (!var1 || !var2)
        return;

    // alias to self
    if (varid1 == varid2) {
        var1->use(_varReadInScope);
        return;
    }

    if (replace) {
        // remove var1 from all aliases
        for (std::set<unsigned int>::iterator i = var1->_aliases.begin(); i != var1->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(var1->_var->declarationId());
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

    if (var2->_type == Variables::pointer) {
        _varReadInScope.back().insert(varid2);
        var2->_read = true;
    }
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
                temp->_aliases.erase(usage->_var->declarationId());
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

void Variables::addVar(const Variable *var,
                       VariableType type,
                       bool write_)
{
    if (var->declarationId() > 0) {
        _varAddedInScope.back().insert(var->declarationId());
        _varUsage.insert(std::make_pair(var->declarationId(), VariableUsage(var, type, false, write_, false)));
    }
}

void Variables::allocateMemory(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_allocateMemory = true;
        usage->_lastAccess = tok;
    }
}

void Variables::read(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        _varReadInScope.back().insert(varid);
        usage->_read = true;
        if (tok)
            usage->_lastAccess = tok;
    }
}

void Variables::readAliases(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased) {
                _varReadInScope.back().insert(*aliases);
                aliased->_read = true;
                aliased->_lastAccess = tok;
            }
        }
    }
}

void Variables::readAll(unsigned int varid, const Token* tok)
{
    read(varid, tok);
    readAliases(varid, tok);
}

void Variables::write(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_write = true;
        if (!usage->_var->isStatic() && !Token::simpleMatch(tok->next(), "= 0 ;"))
            usage->_read = false;
        usage->_lastAccess = tok;
    }
}

void Variables::writeAliases(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased) {
                aliased->_write = true;
                aliased->_lastAccess = tok;
            }
        }
    }
}

void Variables::writeAll(unsigned int varid, const Token* tok)
{
    write(varid, tok);
    writeAliases(varid, tok);
}

void Variables::use(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->use(_varReadInScope);
        usage->_lastAccess = tok;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased) {
                aliased->use(_varReadInScope);
                aliased->_lastAccess = tok;
            }
        }
    }
}

void Variables::modified(unsigned int varid, const Token* tok)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_modified = true;
        usage->_lastAccess = tok;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased) {
                aliased->_modified = true;
                aliased->_lastAccess = tok;
            }
        }
    }
}

Variables::VariableUsage *Variables::find(unsigned int varid)
{
    if (varid) {
        std::map<unsigned int, VariableUsage>::iterator i = _varUsage.find(varid);
        if (i != _varUsage.end())
            return &i->second;
    }
    return 0;
}

void Variables::enterScope()
{
    _varAddedInScope.push_back(std::set<unsigned int>());
    _varReadInScope.push_back(std::set<unsigned int>());
}

void Variables::leaveScope(bool insideLoop)
{
    if (insideLoop) {
        // read variables are read again in subsequent run through loop
        std::set<unsigned int> const & currentVarReadInScope = _varReadInScope.back();
        for (std::set<unsigned int>::const_iterator readIter = currentVarReadInScope.begin();
             readIter != currentVarReadInScope.end();
             ++readIter) {
            read(*readIter, nullptr);
        }
    }

    std::list<std::set<unsigned int> >::reverse_iterator reverseReadIter = _varReadInScope.rbegin();
    ++reverseReadIter;
    if (reverseReadIter != _varReadInScope.rend()) {
        // Transfer read variables into previous scope

        std::set<unsigned int> const & currentVarAddedInScope = _varAddedInScope.back();
        std::set<unsigned int>  & currentVarReadInScope = _varReadInScope.back();
        for (std::set<unsigned int>::const_iterator addedIter = currentVarAddedInScope.begin();
             addedIter != currentVarAddedInScope.end();
             ++addedIter) {
            currentVarReadInScope.erase(*addedIter);
        }
        std::set<unsigned int> & previousVarReadInScope = *reverseReadIter;
        previousVarReadInScope.insert(currentVarReadInScope.begin(),
                                      currentVarReadInScope.end());
    }
    _varReadInScope.pop_back();
    _varAddedInScope.pop_back();
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
                variables.read(tok->varId(), tok);
            tok = tok->next();
        }
        tok = tok->next();

        if (Token::Match(tok, "( const| struct|union| %type% * ) ( ("))
            tok = tok->link()->next();

        if (Token::Match(tok, "( [(<] const| struct|union| %type% *| [>)]"))
            tok = tok->next();

        if (Token::Match(tok, "(| &| %name%") ||
            Token::Match(tok->next(), "< const| struct|union| %type% *| > ( &| %name%")) {
            bool addressOf = false;

            if (Token::Match(tok, "%var% ."))
                variables.use(tok->varId(), tok);   // use = read + write

            // check for C style cast
            if (tok->str() == "(") {
                tok = tok->next();
                if (tok->str() == "const")
                    tok = tok->next();

                if (Token::Match(tok, "struct|union"))
                    tok = tok->next();

                while ((tok->isName() && tok->varId() == 0) || (tok->str() == "*") || (tok->str() == ")"))
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
                } else if (Token::Match(tok, "%cop% %var%")) {
                    variables.read(tok->next()->varId(), tok);
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
            else if (!Token::Match(tok, "%name% ?")) {
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
                        variables.read(varid2, tok);
                    else {
                        if (addressOf ||
                            var2->_type == Variables::array ||
                            var2->_type == Variables::pointer) {
                            bool replace = true;

                            // pointerArray => don't replace
                            if (var1->_type == Variables::pointerArray)
                                replace = false;

                            // check if variable declared in same scope
                            else if (scope == var1->_var->scope())
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
                                variables.readAliases(varid2, tok);
                            else
                                variables.read(varid2, tok);
                        } else {
                            variables.readAll(varid2, tok);
                        }
                    }
                } else if (var1->_type == Variables::reference) {
                    variables.alias(varid1, varid2, true);
                } else {
                    if ((var2->_type == Variables::pointer || var2->_type == Variables::pointerArray) && tok->strAt(1) == "[")
                        variables.readAliases(varid2, tok);

                    variables.read(varid2, tok);
                }
            } else { // not a local variable (or an unsupported local variable)
                if (var1->_type == Variables::pointer && !dereference) {
                    // check if variable declaration is in this scope
                    if (var1->_var->scope() == scope)
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
    else if (Token::Match(tok->tokAt(-2), "%name% .")) {
        if (tok->tokAt(2)->varId()) {
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

    // Possible pointer alias
    else if (Token::Match(tok, "%name% = %name% ;")) {
        const unsigned int varid2 = tok->tokAt(2)->varId();
        Variables::VariableUsage *var2 = variables.find(varid2);
        if (var2 && (var2->_type == Variables::array ||
                     var2->_type == Variables::pointer)) {
            variables.use(varid2,tok);
        }
    }

    return tok;
}

static bool isPartOfClassStructUnion(const Token* tok)
{
    for (; tok; tok = tok->previous()) {
        if (tok->str() == "}" || tok->str() == ")")
            tok = tok->link();
        else if (tok->str() == "(")
            return (false);
        else if (tok->str() == "{") {
            return (tok->strAt(-1) == "struct" || tok->strAt(-2) == "struct" || tok->strAt(-1) == "class" || tok->strAt(-2) == "class" || tok->strAt(-1) == "union" || tok->strAt(-2) == "union");
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
        else if (Token::Match(tok, ". %name%"))
            tok = tok->tokAt(2);
        else
            break;
    }
    return tok;
}


//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------
void CheckUnusedVar::checkFunctionVariableUsage_iterateScopes(const Scope* const scope, Variables& variables, bool insideLoop)
{
    Variables::ScopeGuard scopeGuard=variables.newScope(insideLoop);

    // Find declarations if the scope is executable..
    if (scope->isExecutable()) {
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
            else if (_tokenizer->isC() ||
                     i->typeEndToken()->isStandardType() ||
                     isRecordTypeWithoutSideEffects(i->type()) ||
                     (i->isStlType() &&
                      !Token::Match(i->typeStartToken()->tokAt(2), "lock_guard|unique_lock|shared_ptr|unique_ptr|auto_ptr|shared_lock")))
                type = Variables::standard;
            if (type == Variables::none || isPartOfClassStructUnion(i->typeStartToken()))
                continue;
            const Token* defValTok = i->nameToken()->next();
            for (; defValTok; defValTok = defValTok->next()) {
                if (defValTok->str() == "[")
                    defValTok = defValTok->link();
                else if (defValTok->str() == "(" || defValTok->str() == "{" || defValTok->str() == "=") {
                    variables.addVar(&*i, type, true);
                    break;
                } else if (defValTok->str() == ";" || defValTok->str() == "," || defValTok->str() == ")") {
                    variables.addVar(&*i, type, i->isStatic());
                    break;
                }
            }
            if (i->isArray() && i->isClass()) // Array of class/struct members. Initialized by ctor.
                variables.write(i->declarationId(), i->nameToken());
            if (i->isArray() && Token::Match(i->nameToken(), "%name% [ %var% ]")) // Array index variable read.
                variables.read(i->nameToken()->tokAt(2)->varId(), i->nameToken());

            if (defValTok && defValTok->str() == "=") {
                if (defValTok->next() && defValTok->next()->str() == "{") {
                    for (const Token* tok = defValTok; tok && tok != defValTok->linkAt(1); tok = tok->next())
                        if (tok->varId()) // Variables used to initialize the array read.
                            variables.read(tok->varId(), i->nameToken());
                } else
                    doAssignment(variables, i->nameToken(), false, scope);
            }
        }
    }

    // Check variable usage
    const Token *tok;
    if (scope->type == Scope::eFunction)
        tok = scope->classStart->next();
    else
        tok = scope->classDef->next();
    for (; tok && tok != scope->classEnd; tok = tok->next()) {
        if (tok->str() == "for" || tok->str() == "while" || tok->str() == "do") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classDef == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(*i, variables, true); // Scan child scope
                    tok = (*i)->classStart->link();
                    break;
                }
            }
            if (!tok)
                break;
        }
        if (tok->str() == "{" && tok != scope->classStart && !tok->previous()->varId()) {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classStart == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(*i, variables, false); // Scan child scope
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
        if (Token::simpleMatch(tok, "goto")) { // https://sourceforge.net/apps/trac/cppcheck/ticket/4447
            variables.clear();
            break;
        }


        // bailout when for_each is used
        if (Token::Match(tok, "%name% (") && Token::simpleMatch(tok->linkAt(1), ") {") && !Token::Match(tok, "if|for|while|switch")) {
            // does the name contain "for_each" or "foreach"?
            std::string nameTok;
            nameTok.resize(tok->str().size());
            std::transform(tok->str().begin(), tok->str().end(), nameTok.begin(), ::tolower);
            if (nameTok.find("foreach") != std::string::npos || nameTok.find("for_each") != std::string::npos) {
                // bailout all variables in the body that are used more than once.
                // TODO: there is no need to bailout if variable is only read or only written
                std::set<unsigned int> varid;
                const Token * const endTok = tok->linkAt(1)->linkAt(1);
                for (const Token *tok2 = endTok->link(); tok2 && tok2 != endTok; tok2 = tok2->next()) {
                    if (tok2->varId()) {
                        if (varid.find(tok2->varId()) == varid.end())
                            varid.insert(tok2->varId());
                        else
                            variables.erase(tok2->varId());
                    }
                }
            }
        }

        // C++11 std::for_each
        // No warning should be written if a variable is first read and
        // then written in the body.
        else if (Token::simpleMatch(tok, "for_each (") && Token::simpleMatch(tok->linkAt(1), ") ;")) {
            const Token *end = tok->linkAt(1);
            if (end->previous()->str() == "}") {
                std::set<unsigned int> readvar;
                for (const Token *body = end->linkAt(-1); body != end; body = body->next()) {
                    if (body->varId() == 0U)
                        continue;
                    if (!Token::simpleMatch(body->next(),"="))
                        readvar.insert(body->varId());
                    else if (readvar.find(body->varId()) != readvar.end())
                        variables.erase(body->varId());
                }
            }
        }

        else if (Token::Match(tok->previous(), "[;{}]")) {
            for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    const Variable *var = tok2->variable();
                    if (var && var->nameToken() == tok2) { // Declaration: Skip
                        tok = tok2->next();
                        if (Token::Match(tok, "( %name% )")) // Simple initialization through copy ctor
                            tok = tok->next();
                        else if (Token::Match(tok, "= %var% ;")) { // Simple initialization
                            tok = tok->next();
                            if (!var->isReference())
                                variables.read(tok->varId(), tok);
                        } else if (var->typeEndToken()->str() == ">") // Be careful with types like std::vector
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
                variables.readAll(varid, tok);
            }
        }

        else if (Token::Match(tok, "return|throw")) {
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId())
                    variables.readAll(tok2->varId(), tok);
                else if (tok2->str() == ";")
                    break;
            }
        }

        else if (Token::Match(tok->tokAt(-2), "while|if") && tok->strAt(1) == "=" && tok->varId() && tok->varId() == tok->tokAt(2)->varId()) {
            variables.use(tok->tokAt(2)->varId(), tok);
        }
        // assignment
        else if (Token::Match(tok, "*| ++|--| %name% ++|--| =") ||
                 Token::Match(tok, "*| ( const| %type% *| ) %name% =")) {
            bool dereference = false;
            bool pre = false;
            bool post = false;

            if (tok->str() == "*") {
                dereference = true;
                tok = tok->next();
            }

            if (Token::Match(tok, "( const| %type% *| ) %name% ="))
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
            const Token * const start = tok;

            tok = doAssignment(variables, tok, dereference, scope);

            if (pre || post)
                variables.use(varid1, tok);

            if (dereference) {
                Variables::VariableUsage *var = variables.find(varid1);
                if (var && var->_type == Variables::array)
                    variables.write(varid1, tok);
                variables.writeAliases(varid1, tok);
                variables.read(varid1, tok);
            } else {
                Variables::VariableUsage *var = variables.find(varid1);
                if (var && start->strAt(-1) == ",") {
                    variables.use(varid1, tok);
                } else if (var && var->_type == Variables::reference) {
                    variables.writeAliases(varid1, tok);
                    variables.read(varid1, tok);
                }
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                else if (var && var->_type == Variables::pointer &&
                         Token::Match(start, "%name% = new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    bool allocate = true;

                    if (start->strAt(2) == "new") {
                        const Token *type = start->tokAt(3);

                        // skip nothrow
                        if (Token::simpleMatch(type, "( nothrow )") ||
                            Token::simpleMatch(type, "( std :: nothrow )"))
                            type = type->link()->next();

                        // is it a user defined type?
                        if (!type->isStandardType()) {
                            const Variable *variable = start->variable();
                            if (!variable || !isRecordTypeWithoutSideEffects(variable->type()))
                                allocate = false;
                        }
                    }

                    if (allocate)
                        variables.allocateMemory(varid1, tok);
                    else
                        variables.write(varid1, tok);
                } else if (varid1 && Token::Match(tok, "%varid% .", varid1)) {
                    variables.use(varid1, tok);
                } else {
                    variables.write(varid1, tok);
                }

                Variables::VariableUsage *var2 = variables.find(tok->varId());
                if (var2) {
                    if (var2->_type == Variables::reference) {
                        variables.writeAliases(tok->varId(), tok);
                        variables.read(tok->varId(), tok);
                    } else if (tok->varId() != varid1 && Token::Match(tok, "%name% ."))
                        variables.read(tok->varId(), tok);
                    else if (tok->varId() != varid1 &&
                             var2->_type == Variables::standard &&
                             tok->strAt(-1) != "&")
                        variables.use(tok->varId(), tok);
                }
            }

            const Token * const equal = skipBracketsAndMembers(tok->next());

            // checked for chained assignments
            if (tok != start && equal && equal->str() == "=") {
                unsigned int varId = tok->varId();
                Variables::VariableUsage *var = variables.find(varId);

                if (var && var->_type != Variables::reference) {
                    variables.read(varId,tok);
                }

                tok = tok->previous();
            }
        }

        // assignment
        else if ((Token::Match(tok, "%name% [") && Token::simpleMatch(skipBracketsAndMembers(tok->next()), "=")) ||
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
                    variables.allocateMemory(varid, tok);
                } else if (var->_type == Variables::pointer || var->_type == Variables::reference) {
                    variables.read(varid, tok);
                    variables.writeAliases(varid, tok);
                } else if (var->_type == Variables::pointerArray) {
                    tok = doAssignment(variables, tok, false, scope);
                } else
                    variables.writeAll(varid, tok);
            }
        }

        else if (_tokenizer->isCPP() && Token::Match(tok, "[;{}] %var% <<")) {
            variables.erase(tok->next()->varId());
        }

        else if (Token::Match(tok, "& %var%")) {
            if (tok->astOperand2()) { // bitop
                variables.read(tok->next()->varId(), tok);
            } else // addressof
                variables.use(tok->next()->varId(), tok); // use = read + write
        } else if (Token::Match(tok, ">>|>>= %name%")) {
            if (_tokenizer->isC() || (tok->previous()->variable() && tok->previous()->variable()->typeEndToken()->isStandardType() && tok->astOperand1() && tok->astOperand1()->str() != ">>"))
                variables.read(tok->next()->varId(), tok);
            else
                variables.use(tok->next()->varId(), tok); // use = read + write
        } else if (Token::Match(tok, "%var% >>|&") && Token::Match(tok->previous(), "[{};:]")) {
            variables.read(tok->varId(), tok);
        }

        // function parameter
        else if (Token::Match(tok, "[(,] %var% [")) {
            variables.use(tok->next()->varId(), tok);   // use = read + write
        } else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*") {
            variables.use(tok->next()->varId(), tok);   // use = read + write
        } else if (Token::Match(tok, "[(,] (") &&
                   Token::Match(tok->next()->link(), ") %var% [,)]")) {
            variables.use(tok->next()->link()->next()->varId(), tok);   // use = read + write
        }

        // function
        else if (Token::Match(tok, "%var% (")) {
            variables.read(tok->varId(), tok);
        }

        else if (Token::Match(tok->previous(), "[{,] %var% [,}]")) {
            variables.read(tok->varId(), tok);
        }

        else if (tok->varId() && Token::Match(tok, "%var% .")) {
            variables.use(tok->varId(), tok);   // use = read + write
        }

        else if (tok->isExtendedOp() && tok->next() && tok->next()->varId() && tok->strAt(2) != "=") {
            variables.readAll(tok->next()->varId(), tok);
        }

        else if (tok->varId() && tok->next() && (tok->next()->str() == ")" || tok->next()->isExtendedOp())) {
            variables.readAll(tok->varId(), tok);
        }

        else if (Token::Match(tok, "%var% ;") && Token::Match(tok->previous(), "[;{}:]")) {
            variables.readAll(tok->varId(), tok);
        }

        // ++|--
        else if (tok->next() && tok->next()->type() == Token::eIncDecOp && tok->next()->astOperand1() && tok->next()->astOperand1()->varId()) {
            if (tok->next()->astParent())
                variables.use(tok->next()->astOperand1()->varId(), tok);
            else
                variables.modified(tok->next()->astOperand1()->varId(), tok);
        }

        else if (tok->isAssignmentOp()) {
            for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    if (tok2->strAt(1) == "=")
                        variables.write(tok2->varId(), tok);
                    else if (tok2->next()->isAssignmentOp())
                        variables.use(tok2->varId(), tok);
                    else
                        variables.read(tok2->varId(), tok);
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

    // only check functions
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        // varId, usage {read, write, modified}
        Variables variables;

        checkFunctionVariableUsage_iterateScopes(scope, variables, false);


        // Check usage of all variables in the current scope..
        for (std::map<unsigned int, Variables::VariableUsage>::const_iterator it = variables.varUsage().begin();
             it != variables.varUsage().end();
             ++it) {
            const Variables::VariableUsage &usage = it->second;

            // variable has been marked as unused so ignore it
            if (usage._var->nameToken()->isAttributeUnused() || usage._var->nameToken()->isAttributeUsed())
                continue;

            // skip things that are only partially implemented to prevent false positives
            if (usage._type == Variables::pointerPointer ||
                usage._type == Variables::pointerArray ||
                usage._type == Variables::referenceArray)
                continue;

            const std::string &varname = usage._var->name();
            const Variable* var = symbolDatabase->getVariableFromVarId(it->first);

            // variable has had memory allocated for it, but hasn't done
            // anything with that memory other than, perhaps, freeing it
            if (usage.unused() && !usage._modified && usage._allocateMemory)
                allocatedButUnusedVariableError(usage._lastAccess, varname);

            // variable has not been written, read, or modified
            else if (usage.unused() && !usage._modified)
                unusedVariableError(usage._var->nameToken(), varname);

            // variable has not been written but has been modified
            else if (usage._modified && !usage._write && !usage._allocateMemory && var && !var->isStlType())
                unassignedVariableError(usage._var->nameToken(), varname);

            // variable has been written but not read
            else if (!usage._read && !usage._modified)
                unreadVariableError(usage._lastAccess, varname);

            // variable has been read but not written
            else if (!usage._write && !usage._allocateMemory && var && !var->isStlType() && !isEmptyType(var->type()))
                unassignedVariableError(usage._var->nameToken(), varname);
        }
    }
}

void CheckUnusedVar::unusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedVariable", "Unused variable: " + varname);
}

void CheckUnusedVar::allocatedButUnusedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unusedAllocatedMemory", "Variable '" + varname + "' is allocated memory that is never used.");
}

void CheckUnusedVar::unreadVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unreadVariable", "Variable '" + varname + "' is assigned a value that is never used.");
}

void CheckUnusedVar::unassignedVariableError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::style, "unassignedVariable", "Variable '" + varname + "' is not assigned a value.");
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
            const std::string s("( struct| " + tok->next()->str() + " * ) & %name% [");
            if (Token::findmatch(tok, s.c_str()))
                structname.clear();

            // Bail out if instance is initialized with {}..
            if (!structname.empty()) {
                const std::string pattern1(structname + " %name% ;");
                const Token *tok2 = tok;
                while (nullptr != (tok2 = Token::findmatch(tok2->next(), pattern1.c_str()))) {
                    if (Token::simpleMatch(tok2->tokAt(3), (tok2->strAt(1) + " = {").c_str())) {
                        structname.clear();
                        break;
                    }
                }
            }

            // bail out for extern/global struct
            for (const Token *tok2 = Token::findmatch(tok, (structname + " %name%").c_str());
                 tok2 && tok2->next();
                 tok2 = Token::findmatch(tok2->next(), (structname + " %name%").c_str())) {

                const Variable *var = tok2->next()->variable();
                if (var && (var->isExtern() || (var->isGlobal() && !var->isStatic()))) {
                    structname.clear();
                    break;
                }
            }
            if (structname.empty())
                continue;

            // Try to prevent false positives when struct members are not used directly.
            if (Token::findmatch(tok, (structname + " %type%| *").c_str()))
                structname.clear();
        }

        if (tok->str() == "}")
            structname.clear();

        if (!structname.empty() && Token::Match(tok, "[{;]")) {
            // Declaring struct variable..
            std::string varname;

            // declaring a POD variable?
            if (!tok->next()->isStandardType())
                continue;

            if (Token::Match(tok->next(), "%type% %name% [;[]"))
                varname = tok->strAt(2);
            else if (Token::Match(tok->next(), "%type% %type%|* %name% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% %type% * %name% [;[]"))
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
    reportError(tok, Severity::style, "unusedStructMember", "struct or union member '" + structname + "::" + varname + "' is never used.");
}

bool CheckUnusedVar::isRecordTypeWithoutSideEffects(const Type* type)
{
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check constructors for side effects */

    std::pair<std::map<const Type *,bool>::iterator,bool> found=isRecordTypeWithoutSideEffectsMap.insert(
                std::pair<const Type *,bool>(type,false)); //Initialize with side effects for possible recursions
    bool & withoutSideEffects=found.first->second;
    if (!found.second)
        return withoutSideEffects;

    if (type && type->classScope && type->classScope->numConstructors == 0 &&
        (type->classScope->varlist.empty() || type->needInitialization == Type::True)) {
        for (std::vector<Type::BaseInfo>::const_iterator i = type->derivedFrom.begin(); i != type->derivedFrom.end(); ++i) {
            if (!isRecordTypeWithoutSideEffects(i->type)) {
                withoutSideEffects=false;
                return withoutSideEffects;
            }
        }
        withoutSideEffects=true;
        return withoutSideEffects;
    }

    withoutSideEffects=false;   // unknown types are assumed to have side effects
    return withoutSideEffects;
}

bool CheckUnusedVar::isEmptyType(const Type* type)
{
    // a type that has no variables and no constructor

    std::pair<std::map<const Type *,bool>::iterator,bool> found=isEmptyTypeMap.insert(
                std::pair<const Type *,bool>(type,false));
    bool & emptyType=found.first->second;
    if (!found.second)
        return emptyType;

    if (type && type->classScope && type->classScope->numConstructors == 0 &&
        (type->classScope->varlist.empty())) {
        for (std::vector<Type::BaseInfo>::const_iterator i = type->derivedFrom.begin(); i != type->derivedFrom.end(); ++i) {
            if (!isEmptyType(i->type)) {
                emptyType=false;
                return emptyType;
            }
        }
        emptyType=true;
        return emptyType;
    }

    emptyType=false;   // unknown types are assumed to be nonempty
    return emptyType;
}
