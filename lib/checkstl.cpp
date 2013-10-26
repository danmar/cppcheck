/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include "checkstl.h"
#include "executionpath.h"
#include "symboldatabase.h"
#include <sstream>

// Register this check class (by creating a static instance of it)
namespace {
    CheckStl instance;
}


// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &iteratorName)
{
    reportError(tok, Severity::error, "invalidIterator1", "Invalid iterator: " + iteratorName);
}

void CheckStl::iteratorsError(const Token *tok, const std::string &container1, const std::string &container2)
{
    reportError(tok, Severity::error, "iterators", "Same iterator is used with different containers '" + container1 + "' and '" + container2 + "'.");
}

// Error message used when dereferencing an iterator that has been erased..
void CheckStl::dereferenceErasedError(const Token *erased, const Token* deref, const std::string &itername)
{
    if (erased) {
        std::list<const Token*> callstack;
        callstack.push_back(deref);
        callstack.push_back(erased);
        reportError(callstack, Severity::error, "eraseDereference",
                    "Iterator '" + itername + "' used after element has been erased.\n"
                    "The iterator '" + itername + "' is invalid after the element it pointed to has been erased. "
                    "Dereferencing or comparing it with another iterator is invalid operation.");
    } else {
        reportError(deref, Severity::error, "eraseDereference",
                    "Invalid iterator '" + itername + "' used.\n"
                    "The iterator '" + itername + "' is invalid before being assigned. "
                    "Dereferencing or comparing it with another iterator is invalid operation.");
    }
}

static const Token *skipMembers(const Token *tok)
{
    while (Token::Match(tok, "%var% ."))
        tok = tok->tokAt(2);
    return tok;
}

void CheckStl::iterators()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Using same iterator against different containers.
    // for (it = foo.begin(); it != bar.end(); ++it)
    for (unsigned int iteratorId = 1; iteratorId < symbolDatabase->getVariableListSize(); iteratorId++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(iteratorId);

        // Check that its an iterator
        if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator|auto"))
            continue;

        if (var->type()) { // If it is defined, ensure that it is defined like an iterator
            // look for operator* and operator++
            const Function* end = var->type()->getFunction("operator*");
            const Function* incOperator = var->type()->getFunction("operator++");
            if (!end || end->argCount() > 0 || !incOperator)
                continue;
        }

        // the validIterator flag says if the iterator has a valid value or not
        bool validIterator = Token::Match(var->nameToken()->next(), "[(=]");
        const Scope* invalidationScope = 0;

        // The container this iterator can be used with
        const Variable* container = 0;
        const Scope* containerAssignScope = 0;

        // When "validatingToken" is reached the validIterator is set to true
        const Token* validatingToken = 0;

        const Token* eraseToken = 0;

        // Scan through the rest of the code and see if the iterator is
        // used against other containers.
        for (const Token *tok2 = var->nameToken(); tok2 != var->scope()->classEnd; tok2 = tok2->next()) {
            if (invalidationScope && tok2 == invalidationScope->classEnd)
                validIterator = true; // Assume that the iterator becomes valid again
            if (containerAssignScope && tok2 == containerAssignScope->classEnd)
                container = 0; // We don't know which containers might be used with the iterator

            if (tok2 == validatingToken)
                validIterator = true;

            // Is iterator compared against different container?
            if (Token::Match(tok2, "%varid% !=|== %var% . end|rend|cend|crend ( )", iteratorId) && container && tok2->tokAt(2)->varId() != container->declarationId()) {
                iteratorsError(tok2, container->name(), tok2->strAt(2));
                tok2 = tok2->tokAt(6);
            }

            // Is the iterator used in a insert/erase operation?
            else if (Token::Match(tok2, "%var% . insert|erase ( *| %varid% )|,", iteratorId)) {
                const Token* itTok = tok2->tokAt(4);
                if (itTok->str() == "*") {
                    if (tok2->strAt(2) == "insert")
                        continue;

                    itTok = itTok->next();
                }
                // It is bad to insert/erase an invalid iterator
                if (!validIterator)
                    invalidIteratorError(tok2, itTok->str());

                // If insert/erase is used on different container then
                // report an error
                if (container && tok2->varId() != container->declarationId()) {
                    // skip error message if container is a set..
                    const Variable *variableInfo = tok2->variable();
                    const Token *decltok = variableInfo ? variableInfo->typeStartToken() : NULL;

                    if (Token::simpleMatch(decltok, "std :: set"))
                        continue; // No warning

                    // skip error message if the iterator is erased/inserted by value
                    if (itTok->previous()->str() == "*")
                        continue;

                    // Show error message, mismatching iterator is used.
                    iteratorsError(tok2, container->name(), tok2->str());
                }

                // invalidate the iterator if it is erased
                else if (tok2->strAt(2) == std::string("erase")) {
                    validIterator = false;
                    eraseToken = tok2;
                    invalidationScope = tok2->scope();
                }

                // skip the operation
                tok2 = itTok->next();
            }

            // it = foo.erase(..
            // taking the result of an erase is ok
            else if (Token::Match(tok2, "%varid% = %var% .", iteratorId) &&
                     Token::simpleMatch(skipMembers(tok2->tokAt(2)), "erase (")) {
                // the returned iterator is valid
                validatingToken = tok2->linkAt(5);
                tok2 = tok2->tokAt(5);
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %var% . begin|rbegin|cbegin|crbegin|find (", iteratorId)) {
                validatingToken = tok2->linkAt(5);
                container = tok2->tokAt(2)->variable();
                containerAssignScope = tok2->scope();

                // skip ahead
                tok2 = tok2->tokAt(5);
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %any%", iteratorId)) {
                // Assume that the iterator becomes valid.
                // TODO: add checking that checks if the iterator becomes valid or not
                validatingToken = Token::findmatch(tok2->tokAt(2), "[;)]");

                // skip ahead
                tok2 = tok2->tokAt(2);
            }

            // Passing iterator to function. Iterator might be initialized
            else if (Token::Match(tok2, "%varid% ,|)", iteratorId)) {
                validIterator = true;
            }

            // Dereferencing invalid iterator?
            else if (!validIterator && Token::Match(tok2, "* %varid%", iteratorId)) {
                dereferenceErasedError(eraseToken, tok2, tok2->strAt(1));
                tok2 = tok2->next();
            } else if (!validIterator && Token::Match(tok2, "%varid% . %var%", iteratorId)) {
                dereferenceErasedError(eraseToken, tok2, tok2->str());
                tok2 = tok2->tokAt(2);
            }

            // bailout handling. Assume that the iterator becomes valid if we see return/break.
            // TODO: better handling
            else if (Token::Match(tok2, "return|break")) {
                validatingToken = Token::findsimplematch(tok2->next(), ";");
            }

            // bailout handling. Assume that the iterator becomes valid if we see else.
            // TODO: better handling
            else if (tok2->str() == "else") {
                validIterator = true;
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::mismatchingContainersError(const Token *tok)
{
    reportError(tok, Severity::error, "mismatchingContainers", "Iterators of different containers are used together.");
}

void CheckStl::mismatchingContainers()
{
    static const char* const algorithm2_strings[] = { // func(begin1, end1
        "adjacent_find", "all_of", "any_of", "binary_search", "copy", "copy_if", "count", "count_if", "equal", "equal_range",
        "find", "find_if", "find_if_not", "for_each", "generate", "is_heap", "is_heap_until", "is_partitioned",
        "is_permutation", "is_sorted", "is_sorted_until", "lower_bound", "make_heap", "max_element", "minmax_element",
        "min_element", "mismatch", "move", "move_backward", "next_permutation", "none_of", "partition", "partition_copy",
        "partition_point", "pop_heap", "prev_permutation", "push_heap", "random_shuffle", "remove", "remove_copy",
        "remove_copy_if", "remove_if", "replace", "replace_copy", "replace_copy_if", "replace_if", "reverse", "reverse_copy",
        "search_n", "shuffle", "sort", "sort_heap", "stable_partition", "stable_sort", "swap_ranges", "transform", "unique",
        "unique_copy", "upper_bound"
    };
    static const char* const algorithm22_strings[] = { // func(begin1, end1, begin2, end2
        "find_end", "find_first_of", "includes", "lexicographical_compare", "merge", "partial_sort_copy",
        "search", "set_difference", "set_intersection", "set_symmetric_difference", "set_union"
    };
    static const char* const algorithm1x1_strings[] = { // func(begin1, x, end1
        "inplace_merge", "nth_element", "partial_sort", "rotate", "rotate_copy"
    };

    static const std::set<std::string> algorithm2(algorithm2_strings, &algorithm2_strings[sizeof(algorithm2_strings) / sizeof(*algorithm2_strings)]);
    static const std::set<std::string> algorithm22(algorithm22_strings, &algorithm22_strings[sizeof(algorithm22_strings) / sizeof(*algorithm22_strings)]);
    static const std::set<std::string> algorithm1x1(algorithm1x1_strings, &algorithm1x1_strings[sizeof(algorithm1x1_strings) / sizeof(*algorithm1x1_strings)]);

    static const std::string iteratorBeginFuncPattern = "begin|cbegin|rbegin|crbegin";
    static const std::string iteratorEndFuncPattern = "end|cend|rend|crend";

    static const std::string pattern1x1_1 = "%var% . " + iteratorBeginFuncPattern + " ( ) , ";
    static const std::string pattern1x1_2 = "%var% . " + iteratorEndFuncPattern + " ( ) ,|)";
    static const std::string pattern2 = pattern1x1_1 + pattern1x1_2;

    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t ii = 0; ii < functions; ++ii) {
        const Scope * scope = symbolDatabase->functionScopes[ii];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "std :: %type% ( !!)"))
                continue;
            const Token* arg1 = tok->tokAt(4);

            // TODO: If iterator variables are used instead then there are false negatives.
            if (Token::Match(arg1, pattern2.c_str()) && algorithm2.find(tok->strAt(2)) != algorithm2.end()) {
                if (arg1->str() != arg1->strAt(6)) {
                    mismatchingContainersError(arg1);
                }
            } else if (algorithm22.find(tok->strAt(2)) != algorithm22.end()) {
                if (Token::Match(arg1, pattern2.c_str()) && arg1->str() != arg1->strAt(6))
                    mismatchingContainersError(arg1);
                // Find third parameter
                const Token* arg3 = arg1;
                for (unsigned int i = 0; i < 2 && arg3; i++)
                    arg3 = arg3->nextArgument();
                if (Token::Match(arg3, pattern2.c_str()) && arg3->str() != arg3->strAt(6))
                    mismatchingContainersError(arg3);
            } else if (Token::Match(arg1, pattern1x1_1.c_str()) && algorithm1x1.find(tok->strAt(2)) != algorithm1x1.end()) {
                // Find third parameter
                const Token *arg3 = arg1->tokAt(6)->nextArgument();
                if (Token::Match(arg3, pattern1x1_2.c_str())) {
                    if (arg1->str() != arg3->str()) {
                        mismatchingContainersError(arg1);
                    }
                }
            }
            tok = arg1->linkAt(-1);
        }
    }
}


void CheckStl::stlOutOfBounds()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Scan through all scopes..
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        const Token* tok = i->classDef;
        // only interested in conditions
        if ((i->type != Scope::eFor && i->type != Scope::eWhile && i->type != Scope::eIf && i->type != Scope::eElseIf) || !tok)
            continue;

        if (i->type == Scope::eElseIf)
            tok = tok->tokAt(2);
        else if (i->type == Scope::eFor)
            tok = Token::findsimplematch(tok->tokAt(2), ";");
        else
            tok = tok->next();

        // check if the for loop condition is wrong
        if (Token::Match(tok, ";|( %var% <= %var% . size|length ( ) ;|)|%oror%")) {
            // Is it a vector?
            const Variable *container = tok->tokAt(3)->variable();
            if (!container)
                continue;
            if (!Token::Match(container->typeStartToken(), "std :: vector|deque|array|string|wstring|basic_string"))
                continue;

            // variable id for loop variable.
            const unsigned int numId = tok->next()->varId();

            // variable id for the container variable
            const unsigned int declarationId = container->declarationId();

            for (const Token *tok3 = tok->tokAt(8); tok3 && tok3 != i->classEnd; tok3 = tok3->next()) {
                if (tok3->varId() == declarationId) {
                    if (Token::Match(tok3->next(), ". size|length ( )"))
                        break;
                    else if (Token::Match(tok3->next(), "[ %varid% ]", numId))
                        stlOutOfBoundsError(tok3, tok3->strAt(2), tok3->str(), false);
                    else if (Token::Match(tok3->next(), ". at ( %varid% )", numId))
                        stlOutOfBoundsError(tok3, tok3->strAt(4), tok3->str(), true);
                }
            }
            break;
        }
    }
}

// Error message for bad iterator usage..
void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var, bool at)
{
    if (at)
        reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + ".at(" + num + ") is out of bounds.");
    else
        reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + "[" + num + "] is out of bounds.");
}


/**
 * @brief %Check for invalid iterator usage after erase/insert/etc
 */
class EraseCheckLoop : public ExecutionPath {
public:
    static void checkScope(CheckStl *checkStl, const Token *it) {
        const Token *tok = it;

        // Search for the start of the loop body..
        while (0 != (tok = tok->next())) {
            if (tok->str() == "(")
                tok = tok->link();
            else if (tok->str() == ")")
                break;

            // reassigning iterator in loop head
            else if (Token::Match(tok, "%var% =") && tok->str() == it->str())
                break;
        }

        if (! Token::simpleMatch(tok, ") {"))
            return;

        EraseCheckLoop c(checkStl, it->varId(), it);
        std::list<ExecutionPath *> checks;
        checks.push_back(c.copy());
        ExecutionPath::checkScope(tok->tokAt(2), checks);

        c.end(checks, tok->link());

        while (!checks.empty()) {
            delete checks.back();
            checks.pop_back();
        }
    }

private:
    /** Startup constructor */
    EraseCheckLoop(Check *o, unsigned int varid, const Token* usetoken)
        : ExecutionPath(o, varid), eraseToken(0), useToken(usetoken) {
    }

    /** @brief token where iterator is erased (non-zero => the iterator is invalid) */
    const Token *eraseToken;

    /** @brief name of the iterator */
    const Token* useToken;

    /** @brief Copy this check. Called from the ExecutionPath baseclass. */
    ExecutionPath *copy() {
        return new EraseCheckLoop(*this);
    }

    /** @brief is another execution path equal? */
    bool is_equal(const ExecutionPath *e) const {
        const EraseCheckLoop *c = static_cast<const EraseCheckLoop *>(e);
        return (eraseToken == c->eraseToken);
    }

    /** @brief no implementation => compiler error if used by accident */
    void operator=(const EraseCheckLoop &);

    /** @brief parse tokens */
    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const {
        // bail out if there are assignments. We don't check the assignments properly.
        if (Token::Match(&tok, "[;{}] %var% =") || Token::Match(&tok, "= %var% ;")) {
            ExecutionPath::bailOutVar(checks, tok.next()->varId());
        }

        // the loop stops here. Bail out all execution checks that reach
        // this statement
        if (Token::Match(&tok, "[;{}] break ;")) {
            ExecutionPath::bailOut(checks);
        }

        // erasing iterator => it is invalidated
        if (Token::Match(&tok, "erase ( ++|--| %var% )")) {
            // check if there is a "it = ints.erase(it);" pattern. if so
            // the it is not invalidated.
            const Token *token = &tok;
            while (NULL != (token = token ? token->previous() : 0)) {
                if (Token::Match(token, "[;{}]"))
                    break;
                else if (token->str() == "=")
                    token = 0;
            }

            // the it is invalidated by the erase..
            if (token) {
                // get variable id for the iterator
                unsigned int iteratorId = 0;
                if (tok.tokAt(2)->isName())
                    iteratorId = tok.tokAt(2)->varId();
                else
                    iteratorId = tok.tokAt(3)->varId();

                // invalidate this iterator in the corresponding checks
                for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it) {
                    EraseCheckLoop *c = dynamic_cast<EraseCheckLoop *>(*it);
                    if (c && c->varId == iteratorId) {
                        c->eraseToken = &tok;
                    }
                }
            }
        }

        // don't skip any tokens. return the token that we received.
        return &tok;
    }

    /**
     * Parse condition. @sa ExecutionPath::parseCondition
     * @param tok first token in condition.
     * @param checks The execution paths. All execution paths in the list are executed in the current scope
     * @return true => bail out all checking
     **/
    bool parseCondition(const Token &tok, std::list<ExecutionPath *> &checks) {
        // no checking of conditions.
        (void)tok;
        (void)checks;
        return false;
    }

    /** @brief going out of scope - all execution paths end */
    void end(const std::list<ExecutionPath *> &checks, const Token * /*tok*/) const {
        // check if there are any invalid iterators. If so there is an error.
        for (std::list<ExecutionPath *>::const_iterator it = checks.begin(); it != checks.end(); ++it) {
            EraseCheckLoop *c = dynamic_cast<EraseCheckLoop *>(*it);
            if (c && c->eraseToken) {
                CheckStl *checkStl = dynamic_cast<CheckStl *>(c->owner);
                if (checkStl) {
                    checkStl->dereferenceErasedError(c->eraseToken, c->useToken, c->useToken->str());
                }
            }
        }
    }
};


void CheckStl::erase()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        const Token* const tok = i->classDef;

        if (tok && i->type == Scope::eFor) {
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    if (Token::Match(tok2, "; %var% !=")) {
                        // Get declaration token for var..
                        const Variable *variableInfo = tok2->next()->variable();
                        const Token *decltok = variableInfo ? variableInfo->typeEndToken() : NULL;

                        // Is variable an iterator?
                        bool isIterator = false;
                        if (decltok && Token::Match(decltok->tokAt(-2), "> :: iterator %varid%", tok2->next()->varId()))
                            isIterator = true;

                        // If tok2->next() is an iterator, check scope
                        if (isIterator)
                            EraseCheckLoop::checkScope(this, tok2->next());
                    }
                    break;
                }

                if (Token::Match(tok2, "%var% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %var% != %var% . end|rend|cend|crend ( )") &&
                    tok2->str() == tok2->strAt(8) &&
                    tok2->strAt(2) == tok2->strAt(10)) {
                    EraseCheckLoop::checkScope(this, tok2);
                    break;
                }
            }
        }

        else if (i->type == Scope::eWhile && Token::Match(tok, "while ( %var% !=")) {
            const unsigned int varid = tok->tokAt(2)->varId();
            if (varid > 0 && Token::findmatch(_tokenizer->tokens(), "> :: iterator %varid%", varid))
                EraseCheckLoop::checkScope(this, tok->tokAt(2));
        }
    }
}


void CheckStl::pushback()
{
    // Pointer can become invalid after push_back, push_front, reserve or resize..
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% = & %var% [")) {
                // Variable id for pointer
                const unsigned int pointerId(tok->varId());

                // Variable id for the container variable
                const unsigned int containerId(tok->tokAt(3)->varId());

                if (pointerId == 0 || containerId == 0)
                    continue;

                // Count { , } and parentheses for tok2
                int indent = 0;
                bool invalidPointer = false;
                std::string function;
                for (const Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{" || tok2->str() == "(")
                        ++indent;
                    else if (tok2->str() == "}" || tok2->str() == ")") {
                        if (indent == 0 && Token::simpleMatch(tok2, ") {"))
                            tok2 = tok2->next();
                        else
                            --indent;
                    }

                    // push_back on vector..
                    if (Token::Match(tok2, "%varid% . push_front|push_back|insert|reserve|resize|clear", containerId)) {
                        invalidPointer = true;
                        function = tok2->strAt(2);
                    }

                    // Using invalid pointer..
                    if (invalidPointer && tok2->varId() == pointerId) {
                        if (tok2->previous()->str() == "*")
                            invalidPointerError(tok2, function, tok2->str());
                        else if (tok2->next()->str() == ".")
                            invalidPointerError(tok2, function, tok2->str());
                        break;
                    }
                }
            }
        }
    }

    // Iterator becomes invalid after reserve, resize, insert, push_back or push_front..
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok && tok != scope->classEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "vector <"))
                continue;

            // if iterator declaration inside for() loop
            bool iteratorDeclaredInsideLoop = false;
            if ((tok->tokAt(-2) && Token::simpleMatch(tok->tokAt(-2), "for (")) ||
                (tok->tokAt(-4) && Token::simpleMatch(tok->tokAt(-4), "for ( std ::"))) {
                iteratorDeclaredInsideLoop = true;
            }

            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;
            if (!Token::Match(tok, "> :: iterator|const_iterator %var% =|;"))
                continue;

            const unsigned int iteratorid(tok->tokAt(3)->varId());
            if (iteratorid == 0)
                continue;

            if (iteratorDeclaredInsideLoop && tok->strAt(4) == "=") {
                // skip "> :: iterator|const_iterator"
                tok = tok->tokAt(3);
            }

            // the variable id for the vector
            unsigned int vectorid = 0;

            // count { , } and parentheses for tok2
            int indent = 0;

            const Token* validatingToken = 0;

            std::string invalidIterator;
            for (const Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{" || tok2->str() == "(")
                    ++indent;
                else if (tok2->str() == "}" || tok2->str() == ")") {
                    if (indent == 0 && Token::simpleMatch(tok2, ") {"))
                        tok2 = tok2->next();
                    else
                        --indent;
                }

                if (validatingToken == tok2) {
                    invalidIterator.clear();
                    validatingToken = 0;
                }

                // Using push_back or push_front inside a loop..
                if (Token::simpleMatch(tok2, "for (")) {
                    tok2 = tok2->tokAt(2);
                    ++indent;
                }

                if (Token::Match(tok2, "%varid% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %varid% != %var% . end|rend|cend|crend ( ) ; ++| %varid% ++| ) {", iteratorid)) {
                    // variable id for the loop iterator
                    const unsigned int varId(tok2->tokAt(2)->varId());
                    if (varId == 0)
                        continue;

                    const Token *pushbackTok = 0;

                    // Count { and } for tok3
                    const Token *tok3 = tok2->tokAt(20);
                    for (const Token* const end3 = tok3->linkAt(-1); tok3 != end3; tok3 = tok3->next()) {
                        if (tok3->str() == "break" || tok3->str() == "return") {
                            pushbackTok = 0;
                            break;
                        } else if (Token::Match(tok3, "%varid% . push_front|push_back|insert|reserve|resize|clear (", varId) && !tok3->previous()->isAssignmentOp()) {
                            pushbackTok = tok3->tokAt(2);
                        }
                    }

                    if (pushbackTok)
                        invalidIteratorError(pushbackTok, pushbackTok->str(), tok2->str());
                }

                // Assigning iterator..
                if (Token::Match(tok2, "%varid% =", iteratorid)) {
                    if (Token::Match(tok2->tokAt(2), "%var% . begin|end|rbegin|rend|cbegin|cend|crbegin|crend|insert (")) {
                        if (!invalidIterator.empty() && Token::Match(tok2->tokAt(4), "insert ( %varid% ,", iteratorid)) {
                            invalidIteratorError(tok2, invalidIterator, tok2->strAt(6));
                            break;
                        }
                        vectorid = tok2->tokAt(2)->varId();
                        tok2 = tok2->linkAt(5);
                    } else {
                        vectorid = 0;
                    }
                    invalidIterator = "";
                }

                // push_back on vector..
                if (vectorid > 0 && Token::Match(tok2, "%varid% . push_front|push_back|insert|reserve|resize|clear (", vectorid)) {
                    if (!invalidIterator.empty() && Token::Match(tok2->tokAt(2), "insert ( %varid% ,", iteratorid)) {
                        invalidIteratorError(tok2, invalidIterator, tok2->strAt(4));
                        break;
                    }

                    invalidIterator = tok2->strAt(2);
                    tok2 = tok2->linkAt(3);
                }

                else if (tok2->str() == "return" || tok2->str() == "throw")
                    validatingToken = Token::findsimplematch(tok2->next(), ";");

                // TODO: instead of bail out for 'else' try to check all execution paths.
                else if (tok2->str() == "break" || tok2->str() == "else")
                    invalidIterator.clear();

                // Using invalid iterator..
                if (!invalidIterator.empty()) {
                    if (Token::Match(tok2, "++|--|*|+|-|(|,|=|!= %varid%", iteratorid))
                        invalidIteratorError(tok2, invalidIterator, tok2->strAt(1));
                    if (Token::Match(tok2, "%varid% ++|--|+|-|.", iteratorid))
                        invalidIteratorError(tok2, invalidIterator, tok2->str());
                }
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name)
{
    reportError(tok, Severity::error, "invalidIterator2", "After " + func + "(), the iterator '" + iterator_name + "' may be invalid.");
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &func, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "invalidPointer", "Invalid pointer '" + pointer_name + "' after " + func + "().");
}




void CheckStl::stlBoundaries()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // Declaring iterator..
            if (tok->str() == "<" && Token::Match(tok->previous(), "bitset|deque|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset")) {
                const std::string& container_name(tok->strAt(-1));
                if (tok->link())
                    tok = tok->link();
                else
                    continue;

                if (Token::Match(tok, "> :: iterator|const_iterator %var% =|;")) {
                    const unsigned int iteratorid(tok->tokAt(3)->varId());
                    if (iteratorid == 0)
                        continue;

                    // Using "iterator < ..." is not allowed
                    const Token* const end = tok->scope()->classEnd;
                    for (const Token *tok2 = tok; tok2 != end; tok2 = tok2->next()) {
                        if (Token::Match(tok2, "!!* %varid% <", iteratorid)) {
                            stlBoundariesError(tok2, container_name);
                        } else if (Token::Match(tok2, "> %varid% !!.", iteratorid)) {
                            stlBoundariesError(tok2, container_name);
                        }
                    }
                }
            }
        }
    }
}

// Error message for bad boundary usage..
void CheckStl::stlBoundariesError(const Token *tok, const std::string &container_name)
{
    reportError(tok, Severity::error, "stlBoundaries",
                "Dangerous iterator comparison using operator< on 'std::" + container_name + "'.\n"
                "Iterator of container 'std::" + container_name + "' compared with operator<. "
                "This is dangerous since the order of items in the container is not guaranteed. "
                "One should use operator!= instead to compare iterators.");
}

static bool if_findCompare(const Token * const tokBack, bool str)
{
    const Token *tok = tokBack;
    while (tok && tok->str() == ")") {
        tok = tok->next();

        if (Token::Match(tok, ") !!{") &&
            tok->link()->previous() &&
            (Token::Match(tok->link()->previous(),",|==|!=") ||
             tok->link()->previous()->isName()))
            return true;
    }

    if (Token::Match(tok,",|==|!="))
        return true;
    if (str && tok->isComparisonOp())
        return true;
    if (tok->isArithmeticalOp()) // result is used in some calculation
        return true;  // TODO: check if there is a comparison of the result somewhere
    return false;
}

void CheckStl::if_find()
{
    bool warning = _settings->isEnabled("warning");
    bool performance = _settings->isEnabled("performance");
    if (!warning && !performance)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if ((i->type != Scope::eIf && i->type != Scope::eElseIf && i->type != Scope::eWhile) || !i->classDef)
            continue;

        const Token* tok = i->classDef->next();
        if (tok->str() == "if")
            tok = tok->next();

        for (const Token* const end = tok->link(); tok != end; tok = (tok == end) ? end : tok->next()) {
            if (Token::Match(tok, "&&|(|%oror%"))
                tok = tok->next();
            else
                continue;

            while (tok->str() == "(")
                tok = tok->next();

            if (tok->str() == "!")
                tok = tok->next();

            if (Token::Match(tok, "%var% . find (")) {
                const Variable *var = tok->variable();
                if (var) {
                    // Is the variable a std::string or STL container?
                    const Token * decl = var->typeStartToken();
                    const unsigned int varid = tok->varId();

                    bool str = Token::Match(decl, "std :: string|wstring &| %varid%", varid);
                    if (if_findCompare(tok->linkAt(3), str))
                        continue;

                    // stl container
                    if (Token::Match(decl, "std :: %var% < %type% > &| %varid%", varid) && warning)
                        if_findError(tok, false);
                    else if (str && performance)
                        if_findError(tok, true);
                }
            }

            //check also for vector-like or pointer containers
            else if (Token::Match(tok, "* %var%") || Token::Match(tok, "%var% [")) {
                // goto %var%
                if (tok->str() == "*")
                    tok = tok->next();

                const Token *tok2 = tok->next();
                if (tok2->str() == "[")
                    tok2 = tok2->link()->next();

                if (!Token::simpleMatch(tok2, ". find ("))
                    continue;
                if (if_findCompare(tok2->linkAt(2), false))
                    continue;

                const Variable *var = tok->variable();
                if (var) {
                    // Is the variable a std::string or STL container?
                    const Token * decl = var->typeStartToken();
                    const unsigned int varid = tok->varId();

                    //pretty bad limitation.. but it is there in order to avoid
                    //own implementations of 'find' or any container
                    if (!Token::simpleMatch(decl, "std ::"))
                        continue;

                    decl = decl->tokAt(2);

                    if (Token::Match(decl, "%var% <")) {
                        decl = decl->tokAt(2);
                        //stl-like
                        if (Token::Match(decl, "std :: %var% < %type% > > &| %varid%", varid) && warning)
                            if_findError(tok, false);
                        //not stl-like, then let's hope it's a pointer or an array
                        else if (Token::Match(decl, "%type% >")) {
                            decl = decl->tokAt(2);
                            if ((Token::Match(decl, "* &| %varid%", varid) ||
                                 Token::Match(decl, "&| %varid% [ ]| %any% ]| ", varid)) && warning)
                                if_findError(tok, false);
                        }

                        else if (Token::Match(decl, "std :: string|wstring > &| %varid%", varid) && performance)
                            if_findError(tok, true);
                    }

                    else if (decl && decl->str() == "string") {
                        decl = decl->next();
                        if ((Token::Match(decl, "* &| %varid%", varid) ||
                             Token::Match(decl, "&| %varid% [ ]| %any% ]| ", varid)) && performance)
                            if_findError(tok, true);
                    }
                }
            }

            else if (Token::Match(tok, "std :: find|find_if (")) {
                // check that result is checked properly
                if (!if_findCompare(tok->linkAt(3), false) && warning) {
                    if_findError(tok, false);
                }
            }
        }
    }
}


void CheckStl::if_findError(const Token *tok, bool str)
{
    if (str)
        reportError(tok, Severity::performance, "stlIfStrFind",
                    "Inefficient usage of string::find() in condition; string::compare() would be faster.\n"
                    "Either inefficient or wrong usage of string::find(). string::compare() will be faster if "
                    "string::find's result is compared with 0, because it will not scan the whole "
                    "string. If your intention is to check that there are no findings in the string, "
                    "you should compare with std::string::npos.");
    else
        reportError(tok, Severity::warning, "stlIfFind", "Suspicious condition. The result of find() is an iterator, but it is not properly checked.");
}


/**
 * Is container.size() slow?
 */
static bool isContainerSizeSlow(const Token *tok)
{
    // find where this token is defined
    const Variable *var = tok->variable();

    if (!var)
        return false;

    // find where this tokens type starts
    const Token *type = var->typeStartToken();

    // discard namespace if supplied
    if (Token::simpleMatch(type, "std ::"))
        type = type->tokAt(2);

    // check if it's an stl template
    if (Token::Match(type, "array|bitset|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset|basic_string"))
        return true;

    return false;
}

void CheckStl::size()
{
    if (!_settings->isEnabled("performance"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% . size ( )") ||
                Token::Match(tok, "%var% . %var% . size ( )")) {
                const Token *tok1 = tok;

                // get the variable
                if (tok->strAt(2) != "size")
                    tok1 = tok1->tokAt(2);

                const Token* const end = tok1->tokAt(5);

                if (tok1->varId()) {
                    // check for comparison to zero
                    if ((tok->previous() && !tok->previous()->isArithmeticalOp() && Token::Match(end, "==|<=|!=|> 0")) ||
                        (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "0 ==|>=|!=|<"))) {
                        if (isContainerSizeSlow(tok1))
                            sizeError(tok1);
                    }

                    // check for comparison to one
                    if ((tok->previous() && !tok->previous()->isArithmeticalOp() && Token::Match(end, ">=|< 1") && !end->tokAt(2)->isArithmeticalOp()) ||
                        (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "1 <=|>") && !tok->tokAt(-3)->isArithmeticalOp())) {
                        if (isContainerSizeSlow(tok1))
                            sizeError(tok1);
                    }

                    // check for using as boolean expression
                    else if ((Token::Match(tok->tokAt(-2), "if|while (") && end->str() == ")") ||
                             (tok->previous()->type() == Token::eLogicalOp && Token::Match(end, "&&|)|,|;|%oror%"))) {
                        if (isContainerSizeSlow(tok1))
                            sizeError(tok1);
                    }
                }
            }
        }
    }
}

void CheckStl::sizeError(const Token *tok)
{
    const std::string varname = tok ? tok->str() : std::string("list");
    reportError(tok, Severity::performance, "stlSize",
                "Possible inefficient checking for '" + varname + "' emptiness.\n"
                "Checking for '" + varname + "' emptiness might be inefficient. "
                "Using " + varname + ".empty() instead of " + varname + ".size() can be faster. " +
                varname + ".size() can take linear time but " + varname + ".empty() is "
                "guaranteed to take constant time.");
}

static inline const Token *findRedundantCondition(const Token *start)
{
    return Token::findmatch(start, "if ( %var% . find ( %any% ) != %var% . end|rend|cend|crend ( ) ) { %var% . remove ( %any% ) ;");
}

void CheckStl::redundantCondition()
{
    const Token *tok = findRedundantCondition(_tokenizer->tokens());
    while (tok) {
        // Get tokens for the fields %var% and %any%
        const Token *var1 = tok->tokAt(2);
        const Token *any1 = var1->tokAt(4);
        const Token *var2 = any1->tokAt(3);
        const Token *var3 = var2->tokAt(7);
        const Token *any2 = var3->tokAt(4);

        // Check if all the "%var%" fields are the same and if all the "%any%" are the same..
        if (var1->str() == var2->str() &&
            var2->str() == var3->str() &&
            any1->str() == any2->str()) {
            redundantIfRemoveError(tok);
        }

        tok = findRedundantCondition(tok->next());
    }
}

void CheckStl::redundantIfRemoveError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfRemove",
                "Redundant checking of STL container element existence before removing it.\n"
                "Redundant checking of STL container element existence before removing it. "
                "It is safe to call the remove method on a non-existing element.");
}

void CheckStl::missingComparison()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eFor || !i->classDef)
            continue;

        for (const Token *tok2 = i->classDef->tokAt(2); tok2 != i->classStart; tok2 = tok2->next()) {
            if (tok2->str() == ";")
                break;

            if (!Token::Match(tok2, "%var% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %var% != %var% . end|rend|cend|crend ( ) ; ++| %var% ++| ) {"))
                continue;

            // same container
            if (tok2->strAt(2) != tok2->strAt(10))
                break;

            const unsigned int iteratorId(tok2->varId());
            if (iteratorId == 0)
                break;

            // same iterator
            if (iteratorId == tok2->tokAt(10)->varId())
                break;

            // increment iterator
            if (!Token::Match(tok2->tokAt(16), "++ %varid% )", iteratorId) &&
                !Token::Match(tok2->tokAt(16), "%varid% ++ )", iteratorId)) {
                break;
            }

            const Token *incrementToken = 0;

            // Parse loop..
            for (const Token *tok3 = i->classStart; tok3 != i->classEnd; tok3 = tok3->next()) {
                if (Token::Match(tok3, "%varid% ++", iteratorId))
                    incrementToken = tok3;
                else if (Token::Match(tok3->previous(), "++ %varid% !!.", iteratorId))
                    incrementToken = tok3;
                else if (Token::Match(tok3, "%varid% !=|==", iteratorId))
                    incrementToken = 0;
                else if (tok3->str() == "break" || tok3->str() == "return")
                    incrementToken = 0;
                else if (Token::Match(tok3, "%varid% = %var% . insert ( ++| %varid% ++| ,", iteratorId)) {
                    // skip insertion..
                    tok3 = tok3->linkAt(6);
                    if (!tok3)
                        break;
                }
            }
            if (incrementToken)
                missingComparisonError(incrementToken, tok2->tokAt(16));
        }
    }
}

void CheckStl::missingComparisonError(const Token *incrementToken1, const Token *incrementToken2)
{
    std::list<const Token*> callstack;
    callstack.push_back(incrementToken1);
    callstack.push_back(incrementToken2);

    std::ostringstream errmsg;
    errmsg << "Missing bounds check for extra iterator increment in loop.\n"
           << "The iterator incrementing is suspicious - it is incremented at line ";
    if (incrementToken1)
        errmsg << incrementToken1->linenr();
    errmsg << " and then at line ";
    if (incrementToken2)
        errmsg << incrementToken2->linenr();
    errmsg << ". The loop might unintentionally skip an element in the container. "
           << "There is no comparison between these increments to prevent that the iterator is "
           << "incremented beyond the end.";

    reportError(callstack, Severity::warning, "StlMissingComparison", errmsg.str());
}


static bool isLocal(const Token *tok)
{
    const Variable *var = tok->variable();
    return var && !var->isStatic() && var->isLocal();
}

void CheckStl::string_c_str()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    // Find all functions that take std::string as argument
    std::multimap<std::string, unsigned int> c_strFuncParam;
    if (_settings->isEnabled("performance")) {
        for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
            for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                if (c_strFuncParam.erase(func->tokenDef->str()) != 0) { // Check if function with this name was already found
                    c_strFuncParam.insert(std::make_pair(func->tokenDef->str(), 0)); // Disable, because there are overloads. TODO: Handle overloads
                    continue;
                }

                unsigned int numpar = 0;
                c_strFuncParam.insert(std::make_pair(func->tokenDef->str(), numpar)); // Insert function as dummy, to indicate that there is at least one function with that name
                for (const Token* tok = func->argDef->next(); tok != 0; tok = tok->nextArgument()) {
                    numpar++;
                    if (Token::Match(tok, "std :: string|wstring !!&") || Token::Match(tok, "const std :: string|wstring"))
                        c_strFuncParam.insert(std::make_pair(func->tokenDef->str(), numpar));
                }
            }
        }
    }

    // Try to detect common problems when using string::c_str()
    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eFunction || !scope->function)
            continue;

        enum {charPtr, stdString, stdStringConstRef, Other} returnType = Other;
        if (Token::Match(scope->function->tokenDef->tokAt(-2), "char|wchar_t *"))
            returnType = charPtr;
        else if (Token::Match(scope->function->tokenDef->tokAt(-5), "const std :: string|wstring &"))
            returnType = stdStringConstRef;
        else if (Token::Match(scope->function->tokenDef->tokAt(-3), "std :: string|wstring !!&"))
            returnType = stdString;

        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            // Invalid usage..
            if (Token::Match(tok, "throw %var% . c_str ( ) ;") && isLocal(tok->next())) {
                string_c_strThrowError(tok);
            } else if (Token::Match(tok, "[;{}] %var% = %var% . str ( ) . c_str ( ) ;")) {
                const Variable* var = tok->next()->variable();
                if (var && var->isPointer())
                    string_c_strError(tok);
            } else if (Token::Match(tok, "[;{}] %var% = %var% (") &&
                       Token::simpleMatch(tok->linkAt(4), ") . c_str ( ) ;") &&
                       Token::findmatch(_tokenizer->tokens(), ("std :: string|wstring " + tok->strAt(3) + " (").c_str())) {
                const Variable* var = tok->next()->variable();
                if (var && var->isPointer())
                    string_c_strError(tok);
            } else if (Token::Match(tok, "%var% ( !!)") && c_strFuncParam.find(tok->str()) != c_strFuncParam.end() &&
                       _settings->isEnabled("performance") && !Token::Match(tok->previous(), "::|.") && tok->varId() == 0 && tok->str() != scope->className) { // calling function. TODO: Add support for member functions
                std::pair<std::multimap<std::string, unsigned int>::const_iterator, std::multimap<std::string, unsigned int>::const_iterator> range = c_strFuncParam.equal_range(tok->str());
                for (std::multimap<std::string, unsigned int>::const_iterator i = range.first; i != range.second; ++i) {
                    if (i->second == 0)
                        continue;

                    const Token* tok2 = tok->tokAt(2);
                    unsigned int j;
                    for (j = 0; tok2 && j < i->second-1; j++)
                        tok2 = tok2->nextArgument();
                    if (tok2)
                        tok2 = tok2->nextArgument();
                    else
                        break;
                    if (!tok2 && j == i->second-1)
                        tok2 = tok->next()->link();
                    else
                        tok2 = tok2->previous();
                    if (tok2 && Token::simpleMatch(tok2->tokAt(-4), ". c_str ( )")) {
                        const Variable* var = tok2->tokAt(-5)->variable();
                        if (var && Token::simpleMatch(var->typeStartToken(), "std ::"))
                            string_c_strParam(tok, i->second);
                    }
                }
            }

            // Using c_str() to get the return value is only dangerous if the function returns a char*
            if (returnType == charPtr) {
                if (Token::Match(tok, "return %var% . c_str ( ) ;") && isLocal(tok->next())) {
                    string_c_strError(tok);
                } else if (Token::Match(tok, "return %var% . str ( ) . c_str ( ) ;") && isLocal(tok->next())) {
                    string_c_strError(tok);
                } else if (Token::Match(tok, "return std :: string|wstring (") &&
                           Token::simpleMatch(tok->linkAt(4), ") . c_str ( ) ;")) {
                    string_c_strError(tok);
                } else if (Token::Match(tok, "return %var% (") && Token::simpleMatch(tok->linkAt(2), ") . c_str ( ) ;")) {
                    const Function* func = tok->next()->function();
                    if (func && Token::Match(func->tokenDef->tokAt(-3), "std :: string|wstring"))
                        string_c_strError(tok);
                } else if (Token::simpleMatch(tok, "return (") &&
                           Token::simpleMatch(tok->next()->link(), ") . c_str ( ) ;")) {
                    // Check for "+ localvar" or "+ std::string(" inside the bracket
                    bool is_implicit_std_string = _settings->inconclusive;
                    const Token *search_end = tok->next()->link();
                    for (const Token *search_tok = tok->tokAt(2); search_tok != search_end; search_tok = search_tok->next()) {
                        if (Token::Match(search_tok, "+ %var%") && isLocal(search_tok->next())) {
                            is_implicit_std_string = true;
                            break;
                        } else if (Token::Match(search_tok, "+ std :: string|wstring (")) {
                            is_implicit_std_string = true;
                            break;
                        }
                    }

                    if (is_implicit_std_string)
                        string_c_strError(tok);
                }
            }
            // Using c_str() to get the return value is redundant if the function returns std::string or const std::string&.
            else if ((returnType == stdString || returnType == stdStringConstRef) && _settings->isEnabled("performance")) {
                if (tok->str() == "return") {
                    const Token* tok2 = Token::findsimplematch(tok->next(), ";");
                    if (Token::simpleMatch(tok2->tokAt(-4), ". c_str ( )")) {
                        tok2 = tok2->tokAt(-5);
                        if (tok2->isName()) {  // return var.c_str(); => check if var is a std type
                            const Variable *var = tok2->variable();
                            if (var && Token::simpleMatch(var->typeStartToken(), "std ::"))
                                string_c_strReturn(tok);
                        } else {
                            // TODO: determine if a error should be written or not
                            string_c_strReturn(tok);
                        }
                    }
                }
            }
        }
    }
}

void CheckStl::string_c_strThrowError(const Token* tok)
{
    reportError(tok, Severity::error, "stlcstrthrow", "Dangerous usage of c_str(). The value returned by c_str() is invalid after throwing exception.\n"
                "Dangerous usage of c_str(). The string is destroyed after the c_str() call so the thrown pointer is invalid.");
}

void CheckStl::string_c_strError(const Token* tok)
{
    reportError(tok, Severity::error, "stlcstr", "Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n"
                "Dangerous usage of c_str(). The c_str() return value is only valid until its string is deleted.");
}

void CheckStl::string_c_strReturn(const Token* tok)
{
    reportError(tok, Severity::performance, "stlcstrReturn", "Returning the result of c_str() in a function that returns std::string is slow and redundant.\n"
                "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly returning the string.");
}

void CheckStl::string_c_strParam(const Token* tok, unsigned int number)
{
    std::ostringstream oss;
    oss << "Passing the result of c_str() to a function that takes std::string as argument no. " << number << " is slow and redundant.\n"
        "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrParam", oss.str());
}

static bool hasArrayEnd(const Token *tok1)
{
    const Token *end = Token::findsimplematch(tok1, ";");
    return (end && Token::simpleMatch(end->previous(), "] ;"));
}

static bool hasArrayEndParen(const Token *tok1)
{
    const Token *end = Token::findsimplematch(tok1, ";");
    return (end && end->previous() &&
            Token::simpleMatch(end->tokAt(-2), "] ) ;"));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CheckStl::checkAutoPointer()
{
    std::set<unsigned int> autoPtrVarId;
    static const char STL_CONTAINER_LIST[] = "array|bitset|deque|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|vector|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset|basic_string";

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "auto_ptr <")) {
            if ((tok->strAt(-1) == "<" && Token::Match(tok->tokAt(-2), STL_CONTAINER_LIST)) ||
                (Token::simpleMatch(tok->tokAt(-3), "< std :: auto_ptr") && Token::Match(tok->tokAt(-4), STL_CONTAINER_LIST))) {
                autoPointerContainerError(tok);
            } else {
                const Token *tok2 = tok->linkAt(1);

                if (Token::Match(tok2, "> %var%")) {
                    const Token *tok3 = tok2->tokAt(2);
                    if (Token::Match(tok3, "( new %type%") && hasArrayEndParen(tok3)) {
                        autoPointerArrayError(tok2->next());
                    }
                    while (tok3 && tok3->str() != ";") {
                        tok3 = tok3->next();
                    }
                    if (tok3) {
                        tok3 = tok3->tokAt(-2);
                        if (Token::simpleMatch(tok3->previous(), "[ ] )")) {
                            autoPointerArrayError(tok2->next());
                        } else if (tok3->varId()) {
                            const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid% = new %type%", tok3->varId());
                            if (decltok && hasArrayEnd(decltok)) {
                                autoPointerArrayError(tok2->next());
                            }
                        }
                        if (tok2->next()->varId()) {
                            autoPtrVarId.insert(tok2->next()->varId());
                        }
                    }
                }
            }
        } else {
            if (Token::Match(tok, "%var% = %var% ;")) {
                if (_settings->isEnabled("style")) {
                    std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->tokAt(2)->varId());
                    if (iter != autoPtrVarId.end()) {
                        autoPointerError(tok->tokAt(2));
                    }
                }
            } else if ((Token::Match(tok, "%var% = new %type% ") && hasArrayEnd(tok)) ||
                       (Token::Match(tok, "%var% . reset ( new %type% ") && hasArrayEndParen(tok))) {
                std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerArrayError(tok);
                }
            }
        }
    }
}


void CheckStl::autoPointerError(const Token *tok)
{
    reportError(tok, Severity::style, "useAutoPointerCopy",
                "Copying 'auto_ptr' pointer to another does not create two equal objects since one has lost its ownership of the pointer.\n"
                "'std::auto_ptr' has semantics of strict ownership, meaning that the 'auto_ptr' instance is the sole entity responsible for the object's lifetime. If an 'auto_ptr' is copied, the source looses the reference."
               );
}

void CheckStl::autoPointerContainerError(const Token *tok)
{
    reportError(tok, Severity::error, "useAutoPointerContainer",
                "You can randomly lose access to pointers if you store 'auto_ptr' pointers in an STL container.\n"
                "An element of container must be able to be copied but 'auto_ptr' does not fulfill this requirement. You should consider to use 'shared_ptr' or 'unique_ptr'. It is suitable for use in containers, because they no longer copy their values, they move them."
               );
}

void CheckStl::autoPointerArrayError(const Token *tok)
{
    reportError(tok, Severity::error, "useAutoPointerArray",
                "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n"
                "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. This means that you should only use 'auto_ptr' for pointers obtained with operator 'new'. This excludes arrays, which are allocated by operator 'new[]' and must be deallocated by operator 'delete[]'."
               );
}

void CheckStl::uselessCalls()
{
    bool performance = _settings->isEnabled("performance");
    bool warning = _settings->isEnabled("warning");
    if (!performance && !warning)
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->varId() && Token::Match(tok, "%var% . compare|find|rfind|find_first_not_of|find_first_of|find_last_not_of|find_last_of ( %var% [,)]") &&
                tok->varId() == tok->tokAt(4)->varId() && warning) {
                uselessCallsReturnValueError(tok->tokAt(4), tok->str(), tok->strAt(2));
            } else if (tok->varId() && Token::Match(tok, "%var% . swap ( %var% )") &&
                       tok->varId() == tok->tokAt(4)->varId() && performance) {
                uselessCallsSwapError(tok, tok->str());
            } else if (Token::simpleMatch(tok, ". substr (") && performance) {
                if (Token::Match(tok->tokAt(3), "0| )"))
                    uselessCallsSubstrError(tok, false);
                else if (tok->strAt(3) == "0" && tok->linkAt(2)->strAt(-1) == "npos") {
                    if (!tok->linkAt(2)->previous()->variable()) // Make sure that its no variable
                        uselessCallsSubstrError(tok, false);
                } else if (Token::simpleMatch(tok->linkAt(2)->tokAt(-2), ", 0 )"))
                    uselessCallsSubstrError(tok, true);
            } else if (Token::Match(tok, "[{};] %var% . empty ( ) ;") && warning)
                uselessCallsEmptyError(tok->next());
            else if (Token::Match(tok, "[{};] std :: remove|remove_if|unique (") && tok->tokAt(5)->nextArgument())
                uselessCallsRemoveError(tok->next(), tok->strAt(3));
        }
    }
}


void CheckStl::uselessCallsReturnValueError(const Token *tok, const std::string &varname, const std::string &function)
{
    std::ostringstream errmsg;
    errmsg << "It is inefficient to call '" << varname << "." << function << "(" << varname << ")' as it always returns 0.\n"
           << "'std::string::" << function << "()' returns zero when given itself as parameter "
           << "(" << varname << "." << function << "(" << varname << ")). As it is currently the "
           << "code is inefficient. It is possible either the string searched ('"
           << varname << "') or searched for ('" << varname << "') is wrong.";
    reportError(tok, Severity::warning, "uselessCallsCompare", errmsg.str());
}

void CheckStl::uselessCallsSwapError(const Token *tok, const std::string &varname)
{
    std::ostringstream errmsg;
    errmsg << "It is inefficient to swap a object with itself by calling '" << varname << ".swap(" << varname << ")'\n"
           << "The 'swap()' function has no logical effect when given itself as parameter "
           << "(" << varname << ".swap(" << varname << ")). As it is currently the "
           << "code is inefficient. Is the object or the parameter wrong here?";
    reportError(tok, Severity::performance, "uselessCallsSwap", errmsg.str());
}

void CheckStl::uselessCallsSubstrError(const Token *tok, bool empty)
{
    if (empty)
        reportError(tok, Severity::performance, "uselessCallsSubstr", "Ineffective call of function 'substr' because it returns an empty string.");
    else
        reportError(tok, Severity::performance, "uselessCallsSubstr", "Ineffective call of function 'substr' because it returns a copy of the object. Use operator= instead.");
}

void CheckStl::uselessCallsEmptyError(const Token *tok)
{
    reportError(tok, Severity::warning, "uselessCallsEmpty", "Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead?");
}

void CheckStl::uselessCallsRemoveError(const Token *tok, const std::string& function)
{
    reportError(tok, Severity::warning, "uselessCallsRemove", "Return value of std::" + function + "() ignored. Elements remain in container.\n"
                "The return value of std::" + function + "() is ignored. This function returns an iterator to the end of the range containing those elements that should be kept. "
                "Elements past new end remain valid but with unspecified values. Use the erase method of the container to delete them.");
}

// Check for iterators being dereferenced before being checked for validity.
// E.g.  if (*i && i != str.end()) { }
void CheckStl::checkDereferenceInvalidIterator()
{
    if (!_settings->isEnabled("warning"))
        return;

    // Iterate over "if", "while", and "for" conditions where there may
    // be an iterator that is dereferenced before being checked for validity.
    const std::list<Scope>& scopeList = _tokenizer->getSymbolDatabase()->scopeList;
    for (std::list<Scope>::const_iterator i = scopeList.begin(); i != scopeList.end(); ++i) {
        if (i->type == Scope::eIf || i->type == Scope::eWhile || i->type == Scope::eFor) {

            const Token* const tok = i->classDef;
            const Token* startOfCondition = tok->next();
            const Token* endOfCondition = startOfCondition->link();
            if (!endOfCondition)
                continue;

            // For "for" loops, only search between the two semicolons
            if (i->type == Scope::eFor) {
                startOfCondition = Token::findmatch(tok->tokAt(2), ";", endOfCondition);
                if (!startOfCondition)
                    continue;
                endOfCondition = Token::findmatch(startOfCondition->next(), ";", endOfCondition);
                if (!endOfCondition)
                    continue;
            }

            // Only consider conditions composed of all "&&" terms and
            // conditions composed of all "||" terms
            const bool isOrExpression =
                Token::findsimplematch(startOfCondition, "||", endOfCondition) != 0;
            const bool isAndExpression =
                Token::findsimplematch(startOfCondition, "&&", endOfCondition) != 0;

            // Look for a check of the validity of an iterator
            const Token* validityCheckTok = 0;
            if (!isOrExpression && isAndExpression) {
                validityCheckTok =
                    Token::findmatch(startOfCondition, "&& %var% != %var% . end|rend|cend|crend ( )", endOfCondition);
            } else if (isOrExpression && !isAndExpression) {
                validityCheckTok =
                    Token::findmatch(startOfCondition, "%oror% %var% == %var% . end|rend|cend|crend ( )", endOfCondition);
            }

            if (!validityCheckTok)
                continue;
            const unsigned int iteratorVarId = validityCheckTok->next()->varId();
            if (!iteratorVarId)
                continue;

            // If the iterator dereference is to the left of the check for
            // the iterator's validity, report an error.
            const Token* const dereferenceTok =
                Token::findmatch(startOfCondition, "* %varid%", validityCheckTok, iteratorVarId);
            if (dereferenceTok)
                dereferenceInvalidIteratorError(dereferenceTok, dereferenceTok->strAt(1));
        }
    }
}

void CheckStl::dereferenceInvalidIteratorError(const Token* deref, const std::string &iterName)
{
    reportError(deref, Severity::warning,
                "derefInvalidIterator", "Possible dereference of an invalid iterator: " + iterName + "\n" +
                "Make sure to check that the iterator is valid before dereferencing it - not after.");
}
