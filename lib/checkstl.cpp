/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "symboldatabase.h"
#include "checknullpointer.h"
#include "utils.h"
#include <sstream>

// Register this check class (by creating a static instance of it)
namespace {
    CheckStl instance;
}

// CWE IDs used:
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality
static const struct CWE CWE597(597U);   // Use of Wrong Operator in String Comparison
static const struct CWE CWE628(628U);   // Function Call with Incorrectly Specified Arguments
static const struct CWE CWE664(664U);   // Improper Control of a Resource Through its Lifetime
static const struct CWE CWE704(704U);   // Incorrect Type Conversion or Cast
static const struct CWE CWE762(762U);   // Mismatched Memory Management Routines
static const struct CWE CWE788(788U);   // Access of Memory Location After End of Buffer
static const struct CWE CWE825(825U);   // Expired Pointer Dereference
static const struct CWE CWE834(834U);   // Excessive Iteration

// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &iteratorName)
{
    reportError(tok, Severity::error, "invalidIterator1", "Invalid iterator: " + iteratorName, CWE664, false);
}

void CheckStl::iteratorsError(const Token *tok, const std::string &container1, const std::string &container2)
{
    reportError(tok, Severity::error, "iterators", "Same iterator is used with different containers '" + container1 + "' and '" + container2 + "'.", CWE664, false);
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
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, false);
    } else {
        reportError(deref, Severity::error, "eraseDereference",
                    "Invalid iterator '" + itername + "' used.\n"
                    "The iterator '" + itername + "' is invalid before being assigned. "
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, false);
    }
}

static const Token *skipMembers(const Token *tok)
{
    while (Token::Match(tok, "%name% ."))
        tok = tok->tokAt(2);
    return tok;
}

bool CheckStl::isIterator(const Variable *var) const
{
    // Check that its an iterator
    if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator|auto"))
        return false;

    if (var->typeEndToken()->str() == "auto") {
        if (Token::Match(var->typeEndToken(), "auto %name% ; %name% = %var% . %name% ( )")) {
            const Token* containertok = var->typeEndToken()->tokAt(5);
            if (!containertok->variable())
                return false;

            const Library::Container* container = _settings->library.detectContainer(containertok->variable()->typeStartToken());
            if (!container)
                return false;

            Library::Container::Yield yield = container->getYield(containertok->strAt(2));
            if (yield != Library::Container::END_ITERATOR && yield != Library::Container::START_ITERATOR && yield != Library::Container::ITERATOR)
                return false;
        } else
            return false;
    }

    if (var->type()) { // If it is defined, ensure that it is defined like an iterator
        // look for operator* and operator++
        const Function* end = var->type()->getFunction("operator*");
        const Function* incOperator = var->type()->getFunction("operator++");
        if (!end || end->argCount() > 0 || !incOperator)
            return false;
    }

    return true;
}

void CheckStl::iterators()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (unsigned int iteratorId = 1; iteratorId < symbolDatabase->getVariableListSize(); iteratorId++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(iteratorId);

        if (!isIterator(var))
            continue;

        // the validIterator flag says if the iterator has a valid value or not
        bool validIterator = Token::Match(var->nameToken()->next(), "[(=:]");
        const Scope* invalidationScope = 0;

        // The container this iterator can be used with
        const Variable* container = 0;
        const Scope* containerAssignScope = 0;

        // When "validatingToken" is reached the validIterator is set to true
        const Token* validatingToken = 0;

        const Token* eraseToken = 0;

        // Scan through the rest of the code and see if the iterator is
        // used against other containers.
        for (const Token *tok2 = var->nameToken(); tok2 && tok2 != var->scope()->classEnd; tok2 = tok2->next()) {
            if (invalidationScope && tok2 == invalidationScope->classEnd)
                validIterator = true; // Assume that the iterator becomes valid again
            if (containerAssignScope && tok2 == containerAssignScope->classEnd)
                container = 0; // We don't know which containers might be used with the iterator

            if (tok2 == validatingToken)
                validIterator = true;

            // Is iterator compared against different container?
            if (Token::Match(tok2, "%varid% !=|== %name% . end|rend|cend|crend ( )", iteratorId) && container && tok2->tokAt(2)->varId() != container->declarationId()) {
                iteratorsError(tok2, container->name(), tok2->strAt(2));
                tok2 = tok2->tokAt(6);
            }

            // Is the iterator used in a insert/erase operation?
            else if (Token::Match(tok2, "%name% . insert|erase ( *| %varid% )|,", iteratorId)) {
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
                    const Token *decltok = variableInfo ? variableInfo->typeStartToken() : nullptr;

                    if (Token::simpleMatch(decltok, "std :: set"))
                        continue; // No warning

                    // skip error message if the iterator is erased/inserted by value
                    if (itTok->previous()->str() == "*")
                        continue;

                    // inserting iterator range..
                    if (tok2->strAt(2) == "insert") {
                        const Token *par2 = itTok->nextArgument();
                        if (!par2 || par2->nextArgument())
                            continue;
                        while (par2->str() != ")") {
                            if (par2->varId() == container->declarationId())
                                break;
                            if (isIterator(par2->variable()))
                                break;  // TODO: check if iterator points at same container
                            if (par2->str() == "(")
                                par2 = par2->link();
                            par2 = par2->next();
                        }
                        if (par2->str() != ")")
                            continue;
                    }

                    // Show error message, mismatching iterator is used.
                    iteratorsError(tok2, container->name(), tok2->str());
                }

                // invalidate the iterator if it is erased
                else if (tok2->strAt(2) == "erase" && (tok2->strAt(4) != "*" || (container && tok2->varId() == container->declarationId()))) {
                    validIterator = false;
                    eraseToken = tok2;
                    invalidationScope = tok2->scope();
                }

                // skip the operation
                tok2 = itTok->next();
            }

            // it = foo.erase(..
            // taking the result of an erase is ok
            else if (Token::Match(tok2, "%varid% = %name% .", iteratorId) &&
                     Token::simpleMatch(skipMembers(tok2->tokAt(2)), "erase (")) {
                // the returned iterator is valid
                validatingToken = tok2->linkAt(5);
                tok2 = tok2->tokAt(5);
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %name% . begin|rbegin|cbegin|crbegin|find (", iteratorId)) {
                validatingToken = tok2->linkAt(5);
                container = tok2->tokAt(2)->variable();
                containerAssignScope = tok2->scope();

                // skip ahead
                tok2 = tok2->tokAt(5);
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% =", iteratorId)) {
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
            } else if (!validIterator && Token::Match(tok2, "%varid% . %name%", iteratorId)) {
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
            else if (tok2 && tok2->str() == "else") {
                validIterator = true;
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::mismatchingContainersError(const Token *tok)
{
    reportError(tok, Severity::error, "mismatchingContainers", "Iterators of different containers are used together.", CWE664, false);
}

static const std::set<std::string> algorithm2 = make_container< std::set<std::string> >() // func(begin1, end1
        << "binary_search" << "copy" << "copy_if" << "equal_range"
        << "generate" << "is_heap" << "is_heap_until" << "is_partitioned"
        << "is_permutation" << "is_sorted" << "is_sorted_until" << "lower_bound" << "make_heap" << "max_element" << "minmax_element"
        << "min_element" << "mismatch" << "move" << "move_backward" << "next_permutation" << "partition" << "partition_copy"
        << "partition_point" << "pop_heap" << "prev_permutation" << "push_heap" << "random_shuffle" << "remove" << "remove_copy"
        << "remove_copy_if" << "remove_if" << "replace" << "replace_copy" << "replace_copy_if" << "replace_if" << "reverse" << "reverse_copy"
        << "shuffle" << "sort" << "sort_heap" << "stable_partition" << "stable_sort" << "swap_ranges" << "transform" << "unique"
        << "unique_copy" << "upper_bound" << "string" << "wstring" << "u16string" << "u32string";
static const std::set<std::string> algorithm22 = make_container< std::set<std::string> >() // func(begin1 << end1 << begin2 << end2
        << "includes" << "lexicographical_compare" << "merge" << "partial_sort_copy"
        << "set_difference" << "set_intersection" << "set_symmetric_difference" << "set_union";
static const std::set<std::string> algorithm1x1 = make_container< std::set<std::string> >()  // func(begin1 << x << end1
        << "nth_element" << "partial_sort" << "rotate" << "rotate_copy";

static const std::string iteratorBeginFuncPattern = "begin|cbegin|rbegin|crbegin";
static const std::string iteratorEndFuncPattern = "end|cend|rend|crend";

static const std::string pattern1x1_1 = "%name% . " + iteratorBeginFuncPattern + " ( ) , ";
static const std::string pattern1x1_2 = "%name% . " + iteratorEndFuncPattern + " ( ) ,|)";
static const std::string pattern2 = pattern1x1_1 + pattern1x1_2;

static const Variable *getContainer(const Token *argtok)
{
    while (argtok && argtok->astOperand1())
        argtok = argtok->astOperand1();
    if (!Token::Match(argtok, "%var% . begin|end|rbegin|rend ( )")) // TODO: use Library yield
        return nullptr;
    const Variable *var = argtok->variable();
    if (var && Token::simpleMatch(var->typeStartToken(), "std ::"))
        return var;
    return nullptr;
}

void CheckStl::mismatchingContainers()
{
    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t ii = 0; ii < functions; ++ii) {
        const Scope * scope = symbolDatabase->functionScopes[ii];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const ftok = tok;
            const Token * const arg1 = tok->tokAt(2);

            int argnr = 1;
            std::map<const Variable *, unsigned int> containerNr;
            for (const Token *argTok = arg1; argTok; argTok = argTok->nextArgument()) {
                const Library::ArgumentChecks::IteratorInfo *i = _settings->library.getArgIteratorInfo(ftok,argnr++);
                if (!i)
                    continue;
                const Variable *c = getContainer(argTok);
                if (!c)
                    continue;
                std::map<const Variable *, unsigned int>::const_iterator it = containerNr.find(c);
                if (it == containerNr.end()) {
                    for (it = containerNr.begin(); it != containerNr.end(); ++it) {
                        if (it->second == i->container) {
                            mismatchingContainersError(argTok);
                            break;
                        }
                    }
                    containerNr[c] = i->container;
                } else if (it->second != i->container) {
                    mismatchingContainersError(argTok);
                }
            }
            int ret = _settings->library.returnValueContainer(ftok);
            if (ret != -1 && Token::Match(ftok->next()->astParent(), "==|!=")) {
                const Token *parent = ftok->next()->astParent();
                const Token *other = (parent->astOperand1() == ftok->next()) ? parent->astOperand2() : parent->astOperand1();
                const Variable *c = getContainer(other);
                if (c) {
                    std::map<const Variable *, unsigned int>::const_iterator it = containerNr.find(c);
                    if (it == containerNr.end() || it->second != ret)
                        mismatchingContainersError(other);
                }
            }
        }
    }
    for (unsigned int varid = 0; varid < symbolDatabase->getVariableListSize(); varid++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(varid);
        if (var && var->isStlStringType() && Token::Match(var->nameToken(), "%var% (") && Token::Match(var->nameToken()->tokAt(2), pattern2.c_str())) {
            if (var->nameToken()->strAt(2) != var->nameToken()->strAt(8)) {
                mismatchingContainersError(var->nameToken());
            }
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
        if ((i->type != Scope::eFor && i->type != Scope::eWhile && i->type != Scope::eIf && i->type != Scope::eDo) || !tok)
            continue;

        if (i->type == Scope::eFor)
            tok = Token::findsimplematch(tok->tokAt(2), ";");
        else if (i->type == Scope::eDo) {
            tok = tok->linkAt(1)->tokAt(2);
        } else
            tok = tok->next();

        if (!tok)
            continue;
        tok = tok->next();

        // check if the for loop condition is wrong
        if (Token::Match(tok, "%var% <= %var% . %name% ( ) ;|)|%oror%")) {
            // Is it a vector?
            const Variable *var = tok->tokAt(2)->variable();
            if (!var)
                continue;

            const Library::Container* container = _settings->library.detectContainer(var->typeStartToken());
            if (!container)
                continue;

            if (container->getYield(tok->strAt(4)) != Library::Container::SIZE)
                continue;

            // variable id for loop variable.
            const unsigned int numId = tok->varId();

            // variable id for the container variable
            const unsigned int declarationId = var->declarationId();

            for (const Token *tok3 = i->classStart; tok3 && tok3 != i->classEnd; tok3 = tok3->next()) {
                if (tok3->varId() == declarationId) {
                    tok3 = tok3->next();
                    if (Token::Match(tok3, ". %name% ( )")) {
                        if (container->getYield(tok3->strAt(1)) == Library::Container::SIZE)
                            break;
                    } else if (container->arrayLike_indexOp && Token::Match(tok3, "[ %varid% ]", numId))
                        stlOutOfBoundsError(tok3, tok3->strAt(1), var->name(), false);
                    else if (Token::Match(tok3, ". %name% ( %varid% )", numId)) {
                        Library::Container::Yield yield = container->getYield(tok3->strAt(1));
                        if (yield == Library::Container::AT_INDEX)
                            stlOutOfBoundsError(tok3, tok3->strAt(3), var->name(), true);
                    }
                }
            }
            continue;
        }
    }
}

void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var, bool at)
{
    if (at)
        reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + ".at(" + num + ") is out of bounds.", CWE788, false);
    else
        reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + "[" + num + "] is out of bounds.", CWE788, false);
}

void CheckStl::erase()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type == Scope::eFor && Token::simpleMatch(i->classDef, "for (")) {
            const Token *tok = i->classDef->linkAt(1);
            if (!Token::Match(tok->tokAt(-3), "; ++| %var% ++| ) {"))
                continue;
            tok = tok->previous();
            if (!tok->isName())
                tok = tok->previous();
            eraseCheckLoopVar(*i, tok->variable());
        } else if (i->type == Scope::eWhile && Token::Match(i->classDef, "while ( %var% !=")) {
            eraseCheckLoopVar(*i, i->classDef->tokAt(2)->variable());
        }
    }
}

void CheckStl::eraseCheckLoopVar(const Scope &scope, const Variable *var)
{
    if (!var || !Token::simpleMatch(var->typeEndToken(), "iterator"))
        return;
    for (const Token *tok = scope.classStart; tok != scope.classEnd; tok = tok->next()) {
        if (tok->str() != "(")
            continue;
        if (!Token::Match(tok->tokAt(-2), ". erase ( ++| %varid% )", var->declarationId()))
            continue;
        if (Token::simpleMatch(tok->astParent(), "="))
            continue;
        // Iterator is invalid..
        unsigned int indentlevel = 0U;
        const Token *tok2 = tok->link();
        for (; tok2 != scope.classEnd; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                ++indentlevel;
                continue;
            }
            if (tok2->str() == "}") {
                if (indentlevel > 0U)
                    --indentlevel;
                else if (Token::simpleMatch(tok2, "} else {"))
                    tok2 = tok2->linkAt(2);
                continue;
            }
            if (tok2->varId() == var->declarationId()) {
                if (Token::simpleMatch(tok2->next(), "="))
                    break;
                dereferenceErasedError(tok, tok2, tok2->str());
                break;
            }
            if (indentlevel == 0U && Token::Match(tok2, "break|return|goto"))
                break;
        }
        if (tok2 == scope.classEnd)
            dereferenceErasedError(tok, scope.classDef, var->nameToken()->str());
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
                // Skip it directly if it is a pointer or an array
                const Token* containerTok = tok->tokAt(3);
                if (containerTok->variable() && containerTok->variable()->isArrayOrPointer())
                    continue;

                // Variable id for pointer
                const unsigned int pointerId(tok->varId());

                bool invalidPointer = false;
                const Token* function = nullptr;
                const Token* end2 = tok->scope()->classEnd;
                for (const Token *tok2 = tok; tok2 != end2; tok2 = tok2->next()) {
                    // push_back on vector..
                    if (Token::Match(tok2, "%varid% . push_front|push_back|insert|reserve|resize|clear", containerTok->varId())) {
                        invalidPointer = true;
                        function = tok2->tokAt(2);
                    }

                    // Using invalid pointer..
                    if (invalidPointer && tok2->varId() == pointerId) {
                        bool unknown = false;
                        if (CheckNullPointer::isPointerDeRef(tok2, unknown))
                            invalidPointerError(tok2, function->str(), tok2->str());
                        break;
                    }
                }
            }
        }
    }

    // Iterator becomes invalid after reserve, resize, insert, push_back or push_front..
    for (unsigned int iteratorId = 1; iteratorId < symbolDatabase->getVariableListSize(); iteratorId++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(iteratorId);

        // Check that its an iterator
        if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator"))
            continue;

        // ... on std::vector
        if (!Token::Match(var->typeStartToken(), "std| ::| vector <"))
            continue;

        // the variable id for the vector
        unsigned int vectorid = 0;

        const Token* validatingToken = 0;

        std::string invalidIterator;
        const Token* end2 = var->scope()->classEnd;
        for (const Token *tok2 = var->nameToken(); tok2 != end2; tok2 = tok2->next()) {

            if (validatingToken == tok2) {
                invalidIterator.clear();
                validatingToken = 0;
            }

            // Using push_back or push_front inside a loop..
            if (Token::simpleMatch(tok2, "for (")) {
                tok2 = tok2->tokAt(2);
            }

            if (Token::Match(tok2, "%varid% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %varid% != %var% . end|rend|cend|crend ( ) ; ++| %varid% ++| ) {", iteratorId)) {
                // variable id for the loop iterator
                const unsigned int varId(tok2->tokAt(2)->varId());

                const Token *pushbackTok = nullptr;

                // Count { and } for tok3
                const Token *tok3 = tok2->tokAt(20);
                for (const Token* const end3 = tok3->linkAt(-1); tok3 != end3; tok3 = tok3->next()) {
                    if (tok3->str() == "break" || tok3->str() == "return") {
                        pushbackTok = 0;
                        break;
                    } else if (Token::Match(tok3, "%varid% . push_front|push_back|insert|reserve|resize|clear|erase (", varId) && !tok3->previous()->isAssignmentOp()) {
                        if (tok3->strAt(2) != "erase" || (tok3->tokAt(4)->varId() != iteratorId && tok3->tokAt(5)->varId() != iteratorId)) // This case is handled in: CheckStl::iterators()
                            pushbackTok = tok3->tokAt(2);
                    }
                }

                if (pushbackTok)
                    invalidIteratorError(pushbackTok, pushbackTok->str(), tok2->str());
            }

            // Assigning iterator..
            if (Token::Match(tok2, "%varid% =", iteratorId)) {
                if (Token::Match(tok2->tokAt(2), "%var% . begin|end|rbegin|rend|cbegin|cend|crbegin|crend|insert|erase|find (")) {
                    if (!invalidIterator.empty() && Token::Match(tok2->tokAt(4), "insert|erase ( *| %varid% )|,", iteratorId)) {
                        invalidIteratorError(tok2, invalidIterator, var->name());
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
            if (vectorid > 0 && Token::Match(tok2, "%varid% . push_front|push_back|insert|reserve|resize|clear|erase (", vectorid)) {
                if (!invalidIterator.empty() && Token::Match(tok2->tokAt(2), "insert|erase ( *| %varid% ,|)", iteratorId)) {
                    invalidIteratorError(tok2, invalidIterator, var->name());
                    break;
                }

                if (tok2->strAt(2) != "erase" || (tok2->tokAt(4)->varId() != iteratorId && tok2->tokAt(5)->varId() != iteratorId)) // This case is handled in: CheckStl::iterators()
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
                if (Token::Match(tok2, "++|--|*|+|-|(|,|=|!= %varid%", iteratorId))
                    invalidIteratorError(tok2, invalidIterator, tok2->strAt(1));
                if (Token::Match(tok2, "%varid% ++|--|+|-|.", iteratorId))
                    invalidIteratorError(tok2, invalidIterator, tok2->str());
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name)
{
    reportError(tok, Severity::error, "invalidIterator2", "After " + func + "(), the iterator '" + iterator_name + "' may be invalid.", CWE664, false);
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &func, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "invalidPointer", "Invalid pointer '" + pointer_name + "' after " + func + "().", CWE664, false);
}


void CheckStl::stlBoundaries()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int iteratorId = 1; iteratorId < symbolDatabase->getVariableListSize(); iteratorId++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(iteratorId);
        if (!var || !var->scope() || !var->scope()->isExecutable())
            continue;

        const Library::Container* container = _settings->library.detectContainer(var->typeStartToken(), true);
        if (!container || container->opLessAllowed)
            continue;

        const Token* const end = var->scope()->classEnd;
        for (const Token *tok = var->nameToken(); tok != end; tok = tok->next()) {
            if (Token::Match(tok, "!!* %varid% <", var->declarationId())) {
                stlBoundariesError(tok);
            } else if (Token::Match(tok, "> %varid% !!.", var->declarationId())) {
                stlBoundariesError(tok);
            }
        }
    }
}

// Error message for bad boundary usage..
void CheckStl::stlBoundariesError(const Token *tok)
{
    reportError(tok, Severity::error, "stlBoundaries",
                "Dangerous comparison using operator< on iterator.\n"
                "Iterator compared with operator<. This is dangerous since the order of items in the "
                "container is not guaranteed. One should use operator!= instead to compare iterators.", CWE664, false);
}

static bool if_findCompare(const Token * const tokBack)
{
    const Token *tok = tokBack->astParent();
    if (!tok)
        return true;
    if (tok->isComparisonOp())
        return (!tok->astOperand1()->isNumber() && !tok->astOperand2()->isNumber());
    if (tok->isArithmeticalOp()) // result is used in some calculation
        return true;  // TODO: check if there is a comparison of the result somewhere
    if (tok->str() == ".")
        return true; // Dereferencing is OK, the programmer might know that the element exists - TODO: An inconclusive warning might be appropriate
    if (tok->isAssignmentOp())
        return if_findCompare(tok); // Go one step upwards in the AST
    return false;
}

void CheckStl::if_find()
{
    const bool printWarning = _settings->isEnabled("warning");
    const bool printPerformance = _settings->isEnabled("performance");
    if (!printWarning && !printPerformance)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if ((i->type != Scope::eIf && i->type != Scope::eWhile) || !i->classDef)
            continue;

        for (const Token *tok = i->classDef->next(); tok->str() != "{"; tok = tok->next()) {
            const Token* funcTok = nullptr;
            const Library::Container* container = nullptr;

            if (Token::Match(tok, "%name% ("))
                tok = tok->linkAt(1);

            else if (tok->variable() && Token::Match(tok, "%var% . %name% (")) {
                container = _settings->library.detectContainer(tok->variable()->typeStartToken());
                funcTok = tok->tokAt(2);
            }

            // check also for vector-like or pointer containers
            else if (tok->variable() && tok->astParent() && (tok->astParent()->str() == "*" || tok->astParent()->str() == "[")) {
                const Token *tok2 = tok->astParent();

                if (!Token::Match(tok2->astParent(), ". %name% ("))
                    continue;

                funcTok = tok2->astParent()->next();

                if (tok->variable()->isArrayOrPointer())
                    container = _settings->library.detectContainer(tok->variable()->typeStartToken());
                else { // Container of container - find the inner container
                    container = _settings->library.detectContainer(tok->variable()->typeStartToken()); // outer container
                    tok2 = Token::findsimplematch(tok->variable()->typeStartToken(), "<", tok->variable()->typeEndToken());
                    if (container && container->type_templateArgNo >= 0 && tok2) {
                        tok2 = tok2->next();
                        for (int j = 0; j < container->type_templateArgNo; j++)
                            tok2 = tok2->nextTemplateArgument();

                        container = _settings->library.detectContainer(tok2); // innner container
                    } else
                        container = nullptr;
                }
            }

            if (container && container->getAction(funcTok->str()) == Library::Container::FIND) {
                if (if_findCompare(funcTok->next()))
                    continue;

                if (printWarning && container->getYield(funcTok->str()) == Library::Container::ITERATOR)
                    if_findError(tok, false);
                else if (printPerformance && container->stdStringLike && funcTok->str() == "find")
                    if_findError(tok, true);
            } else if (printWarning && Token::Match(tok, "std :: find|find_if (")) {
                // check that result is checked properly
                if (!if_findCompare(tok->tokAt(3))) {
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
                    "you should compare with std::string::npos.", CWE597, false);
    else
        reportError(tok, Severity::warning, "stlIfFind", "Suspicious condition. The result of find() is an iterator, but it is not properly checked.", CWE398, false);
}


/**
 * Is container.size() slow?
 */
static bool isCpp03ContainerSizeSlow(const Token *tok)
{
    if (!tok)
        return false;
    const Variable* var = tok->variable();
    return var && var->isStlType("list");
}

void CheckStl::size()
{
    if (!_settings->isEnabled("performance"))
        return;

    if (_settings->standards.cpp == Standards::CPP11)
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% . size ( )") ||
                Token::Match(tok, "%name% . %var% . size ( )")) {
                // get the variable
                const Token *varTok = tok;
                if (tok->strAt(2) != "size")
                    varTok = varTok->tokAt(2);

                const Token* const end = varTok->tokAt(5);

                // check for comparison to zero
                if ((tok->previous() && !tok->previous()->isArithmeticalOp() && Token::Match(end, "==|<=|!=|> 0")) ||
                    (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "0 ==|>=|!=|<"))) {
                    if (isCpp03ContainerSizeSlow(varTok)) {
                        sizeError(varTok);
                        continue;
                    }
                }

                // check for comparison to one
                if ((tok->previous() && !tok->previous()->isArithmeticalOp() && Token::Match(end, ">=|< 1") && !end->tokAt(2)->isArithmeticalOp()) ||
                    (end->next() && !end->next()->isArithmeticalOp() && Token::Match(tok->tokAt(-2), "1 <=|>") && !tok->tokAt(-3)->isArithmeticalOp())) {
                    if (isCpp03ContainerSizeSlow(varTok))
                        sizeError(varTok);
                }

                // check for using as boolean expression
                else if ((Token::Match(tok->tokAt(-2), "if|while (") && end->str() == ")") ||
                         (tok->previous()->tokType() == Token::eLogicalOp && Token::Match(end, "&&|)|,|;|%oror%"))) {
                    if (isCpp03ContainerSizeSlow(varTok))
                        sizeError(varTok);
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
                "guaranteed to take constant time.", CWE398, false);
}

void CheckStl::redundantCondition()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eIf)
            continue;

        const Token* tok = i->classDef->tokAt(2);
        if (!Token::Match(tok, "%name% . find ( %any% ) != %name% . end|rend|cend|crend ( ) ) { %name% . remove|erase ( %any% ) ;"))
            continue;

        // Get tokens for the fields %name% and %any%
        const Token *var1 = tok;
        const Token *any1 = var1->tokAt(4);
        const Token *var2 = any1->tokAt(3);
        const Token *var3 = var2->tokAt(7);
        const Token *any2 = var3->tokAt(4);

        // Check if all the "%name%" fields are the same and if all the "%any%" are the same..
        if (var1->str() == var2->str() &&
            var2->str() == var3->str() &&
            any1->str() == any2->str()) {
            redundantIfRemoveError(tok);
        }
    }
}

void CheckStl::redundantIfRemoveError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfRemove",
                "Redundant checking of STL container element existence before removing it.\n"
                "Redundant checking of STL container element existence before removing it. "
                "It is safe to call the remove method on a non-existing element.", CWE398, false);
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

            if (!Token::Match(tok2, "%var% = %name% . begin|rbegin|cbegin|crbegin ( ) ; %name% != %name% . end|rend|cend|crend ( ) ; ++| %name% ++| ) {"))
                continue;

            // same container
            if (tok2->strAt(2) != tok2->strAt(10))
                break;

            const unsigned int iteratorId(tok2->varId());

            // same iterator
            if (iteratorId == tok2->tokAt(10)->varId())
                break;

            // increment iterator
            if (!Token::Match(tok2->tokAt(16), "++ %varid% )", iteratorId) &&
                !Token::Match(tok2->tokAt(16), "%varid% ++ )", iteratorId)) {
                break;
            }

            const Token *incrementToken = nullptr;

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
                else if (Token::Match(tok3, "%varid% = %name% . insert ( ++| %varid% ++| ,", iteratorId)) {
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

    reportError(callstack, Severity::warning, "StlMissingComparison", errmsg.str(), CWE834, false);
}


static bool isLocal(const Token *tok)
{
    const Variable *var = tok->variable();
    return var && !var->isStatic() && var->isLocal();
}

namespace {
    const std::set<std::string> stl_string_stream = make_container< std::set<std::string> >() <<
            "istringstream" <<  "ostringstream" <<  "stringstream" <<  "wstringstream" ;
}

void CheckStl::string_c_str()
{
    const bool printInconclusive = _settings->inconclusive;
    const bool printPerformance = _settings->isEnabled("performance");

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    // Find all functions that take std::string as argument
    std::multimap<std::string, unsigned int> c_strFuncParam;
    if (printPerformance) {
        for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
            for (std::list<Function>::const_iterator func = scope->functionList.begin(); func != scope->functionList.end(); ++func) {
                if (c_strFuncParam.erase(func->tokenDef->str()) != 0) { // Check if function with this name was already found
                    c_strFuncParam.insert(std::make_pair(func->tokenDef->str(), 0)); // Disable, because there are overloads. TODO: Handle overloads
                    continue;
                }

                unsigned int numpar = 0;
                c_strFuncParam.insert(std::make_pair(func->tokenDef->str(), numpar)); // Insert function as dummy, to indicate that there is at least one function with that name
                for (std::list<Variable>::const_iterator var = func->argumentList.cbegin(); var != func->argumentList.cend(); ++var) {
                    numpar++;
                    if (var->isStlStringType() && (!var->isReference() || var->isConst()))
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
            if (Token::Match(tok, "throw %var% . c_str|data ( ) ;") && isLocal(tok->next()) &&
                tok->next()->variable() && tok->next()->variable()->isStlStringType()) {
                string_c_strThrowError(tok);
            } else if (Token::Match(tok, "[;{}] %name% = %var% . str ( ) . c_str|data ( ) ;")) {
                const Variable* var = tok->next()->variable();
                const Variable* var2 = tok->tokAt(3)->variable();
                if (var && var->isPointer() && var2 && var2->isStlType(stl_string_stream))
                    string_c_strError(tok);
            } else if (Token::Match(tok, "[;{}] %var% = %name% (") &&
                       Token::Match(tok->linkAt(4), ") . c_str|data ( ) ;") &&
                       tok->tokAt(3)->function() && Token::Match(tok->tokAt(3)->function()->retDef, "std :: string|wstring %name%")) {
                const Variable* var = tok->next()->variable();
                if (var && var->isPointer())
                    string_c_strError(tok);
            } else if (printPerformance && Token::Match(tok, "%name% ( !!)") && c_strFuncParam.find(tok->str()) != c_strFuncParam.end() &&
                       !Token::Match(tok->previous(), "::|.") && tok->varId() == 0 && tok->str() != scope->className) { // calling function. TODO: Add support for member functions
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
                    if (tok2 && Token::Match(tok2->tokAt(-4), ". c_str|data ( )")) {
                        const Variable* var = tok2->tokAt(-5)->variable();
                        if (var && var->isStlStringType()) {
                            string_c_strParam(tok, i->second);
                        } else if (Token::Match(tok2->tokAt(-9), "%name% . str ( )")) { // Check ss.str().c_str() as parameter
                            const Variable* ssVar = tok2->tokAt(-9)->variable();
                            if (ssVar && ssVar->isStlType(stl_string_stream))
                                string_c_strParam(tok, i->second);
                        }

                    }
                }
            }

            // Using c_str() to get the return value is only dangerous if the function returns a char*
            if ((returnType == charPtr || (printPerformance && (returnType == stdString || returnType == stdStringConstRef))) && tok->str() == "return") {
                bool err = false;

                const Token* tok2 = tok->next();
                if (Token::Match(tok2, "std :: string|wstring (") &&
                    Token::Match(tok2->linkAt(3), ") . c_str|data ( ) ;")) {
                    err = true;
                } else if (Token::simpleMatch(tok2, "(") &&
                           Token::Match(tok2->link(), ") . c_str|data ( ) ;")) {
                    // Check for "+ localvar" or "+ std::string(" inside the bracket
                    bool is_implicit_std_string = printInconclusive;
                    const Token *search_end = tok2->link();
                    for (const Token *search_tok = tok2->next(); search_tok != search_end; search_tok = search_tok->next()) {
                        if (Token::Match(search_tok, "+ %var%") && isLocal(search_tok->next()) &&
                            search_tok->next()->variable() && search_tok->next()->variable()->isStlStringType()) {
                            is_implicit_std_string = true;
                            break;
                        } else if (Token::Match(search_tok, "+ std :: string|wstring (")) {
                            is_implicit_std_string = true;
                            break;
                        }
                    }

                    if (is_implicit_std_string)
                        err = true;
                }

                bool local = false;
                bool ptrOrRef = false;
                const Variable* lastVar = nullptr;
                const Function* lastFunc = nullptr;
                bool funcStr = false;
                if (Token::Match(tok2, "%var% .")) {
                    local = isLocal(tok2);
                    bool refToNonLocal = false;
                    if (tok2->variable() && tok2->variable()->isReference()) {
                        const Token *refTok = tok2->variable()->nameToken();
                        refToNonLocal = true; // safe assumption is default to avoid FPs
                        if (Token::Match(refTok, "%var% = %var% .|;|["))
                            refToNonLocal = !isLocal(refTok->tokAt(2));
                    }
                    ptrOrRef = refToNonLocal || (tok2->variable() && tok2->variable()->isPointer());
                }
                while (tok2) {
                    if (Token::Match(tok2, "%var% .|::")) {
                        if (ptrOrRef)
                            local = false;
                        lastVar = tok2->variable();
                        tok2 = tok2->tokAt(2);
                    } else if (Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") .")) {
                        lastFunc = tok2->function();
                        local = false;
                        funcStr = tok2->str() == "str";
                        tok2 = tok2->linkAt(1)->tokAt(2);
                    } else
                        break;
                }

                if (Token::Match(tok2, "c_str|data ( ) ;")) {
                    if ((local || returnType != charPtr) && lastVar && lastVar->isStlStringType())
                        err = true;
                    else if (funcStr && lastVar && lastVar->isStlType(stl_string_stream))
                        err = true;
                    else if (lastFunc && Token::Match(lastFunc->tokenDef->tokAt(-3), "std :: string|wstring"))
                        err = true;
                }

                if (err) {
                    if (returnType == charPtr)
                        string_c_strError(tok);
                    else
                        string_c_strReturn(tok);
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
                "Dangerous usage of c_str(). The c_str() return value is only valid until its string is deleted.", CWE664, false);
}

void CheckStl::string_c_strReturn(const Token* tok)
{
    reportError(tok, Severity::performance, "stlcstrReturn", "Returning the result of c_str() in a function that returns std::string is slow and redundant.\n"
                "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly returning the string.", CWE704, false);
}

void CheckStl::string_c_strParam(const Token* tok, unsigned int number)
{
    std::ostringstream oss;
    oss << "Passing the result of c_str() to a function that takes std::string as argument no. " << number << " is slow and redundant.\n"
        "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrParam", oss.str(), CWE704, false);
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
    std::map<unsigned int, const std::string> mallocVarId; // variables allocated by the malloc-like function
    const char STL_CONTAINER_LIST[] = "array|bitset|deque|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|vector|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset|basic_string";
    const int malloc = _settings->library.allocId("malloc"); // allocation function, which are not compatible with auto_ptr
    const bool printStyle = _settings->isEnabled("style");

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "auto_ptr <")) {
            if ((tok->strAt(-1) == "<" && Token::Match(tok->tokAt(-2), STL_CONTAINER_LIST)) ||
                (Token::simpleMatch(tok->tokAt(-3), "< std :: auto_ptr") && Token::Match(tok->tokAt(-4), STL_CONTAINER_LIST))) {
                autoPointerContainerError(tok);
            } else {
                const Token *tok2 = tok->linkAt(1);

                if (Token::Match(tok2, "> %name%")) {
                    const Token *tok3 = tok2->tokAt(2);
                    if (Token::Match(tok3, "( new %type%") && hasArrayEndParen(tok3)) {
                        autoPointerArrayError(tok2->next());
                    }
                    if (Token::Match(tok3, "( %name% (") && malloc && _settings->library.alloc(tok3->next(), -1) == malloc) {
                        // malloc-like function allocated memory passed to the auto_ptr constructor -> error
                        autoPointerMallocError(tok2->next(), tok3->next()->str());
                    }
                    if (Token::Match(tok3, "( %var%")) {
                        std::map<unsigned int, const std::string>::const_iterator it = mallocVarId.find(tok3->next()->varId());
                        if (it != mallocVarId.cend()) {
                            // pointer on the memory allocated by malloc used in the auto pointer constructor -> error
                            autoPointerMallocError(tok2->next(), it->second);
                        }
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
            if (Token::Match(tok, "%name% = %var% ;")) {
                if (printStyle) {
                    std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->tokAt(2)->varId());
                    if (iter != autoPtrVarId.end()) {
                        autoPointerError(tok->tokAt(2));
                    }
                }
            } else if ((Token::Match(tok, "%var% = new %type%") && hasArrayEnd(tok)) ||
                       (Token::Match(tok, "%var% . reset ( new %type%") && hasArrayEndParen(tok))) {
                std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerArrayError(tok);
                }
            } else if (Token::Match(tok, "%var% = %name% (") && malloc && _settings->library.alloc(tok->tokAt(2), -1) == malloc) {
                // C library function like 'malloc' used together with auto pointer -> error
                std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerMallocError(tok, tok->strAt(2));
                } else if (tok->varId()) {
                    // it is not an auto pointer variable and it is allocated by malloc like function.
                    mallocVarId.insert(std::make_pair(tok->varId(), tok->strAt(2)));
                }
            } else if (Token::Match(tok, "%var% . reset ( %name% (") && malloc && _settings->library.alloc(tok->tokAt(4), -1) == malloc) {
                // C library function like 'malloc' used when resetting auto pointer -> error
                std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerMallocError(tok, tok->strAt(4));
                }
            }
        }
    }
}


void CheckStl::autoPointerError(const Token *tok)
{
    reportError(tok, Severity::style, "useAutoPointerCopy",
                "Copying 'auto_ptr' pointer to another does not create two equal objects since one has lost its ownership of the pointer.\n"
                "'std::auto_ptr' has semantics of strict ownership, meaning that the 'auto_ptr' instance is the sole entity responsible for the object's lifetime. If an 'auto_ptr' is copied, the source looses the reference.",
                CWE398, false);
}

void CheckStl::autoPointerContainerError(const Token *tok)
{
    reportError(tok, Severity::error, "useAutoPointerContainer",
                "You can randomly lose access to pointers if you store 'auto_ptr' pointers in an STL container.\n"
                "An element of container must be able to be copied but 'auto_ptr' does not fulfill this requirement. You should consider to use 'shared_ptr' or 'unique_ptr'. It is suitable for use in containers, because they no longer copy their values, they move them.", CWE664, false
               );
}

void CheckStl::autoPointerArrayError(const Token *tok)
{
    reportError(tok, Severity::error, "useAutoPointerArray",
                "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n"
                "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. This means that you should only use 'auto_ptr' for pointers obtained with operator 'new'. This excludes arrays, which are allocated by operator 'new[]' and must be deallocated by operator 'delete[]'.", CWE664, false
               );
}

void CheckStl::autoPointerMallocError(const Token *tok, const std::string& allocFunction)
{
    const std::string summary = "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function '" + allocFunction + "'.";
    const std::string verbose = summary + " This means that you should only use 'auto_ptr' for pointers obtained with operator 'new'. This excludes use C library allocation functions (for example '" + allocFunction + "'), which must be deallocated by the appropriate C library function.";
    reportError(tok, Severity::error, "useAutoPointerMalloc", summary + "\n" + verbose, CWE762, false);
}

namespace {
    const std::set<std::string> stl_containers_with_empty_and_clear = make_container< std::set<std::string> >() <<
            "deque" <<  "forward_list" <<  "list" <<
            "map" <<  "multimap" <<  "multiset" <<  "set" <<  "string" <<
            "unordered_map" <<  "unordered_multimap" <<  "unordered_multiset" <<
            "unordered_set" <<  "vector" <<  "wstring";

}

void CheckStl::uselessCalls()
{
    const bool printPerformance = _settings->isEnabled("performance");
    const bool printWarning = _settings->isEnabled("warning");
    if (!printPerformance && !printWarning)
        return;


    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (printWarning && Token::Match(tok, "%var% . compare|find|rfind|find_first_not_of|find_first_of|find_last_not_of|find_last_of ( %name% [,)]") &&
                tok->varId() == tok->tokAt(4)->varId()) {
                const Variable* var = tok->variable();
                if (!var || !var->isStlType())
                    continue;
                uselessCallsReturnValueError(tok->tokAt(4), tok->str(), tok->strAt(2));
            } else if (printPerformance && Token::Match(tok, "%var% . swap ( %name% )") &&
                       tok->varId() == tok->tokAt(4)->varId()) {
                const Variable* var = tok->variable();
                if (!var || !var->isStlType())
                    continue;
                uselessCallsSwapError(tok, tok->str());
            } else if (printPerformance && Token::Match(tok, "%var% . substr (") &&
                       tok->variable() && tok->variable()->isStlStringType()) {
                if (Token::Match(tok->tokAt(4), "0| )")) {
                    uselessCallsSubstrError(tok, false);
                } else if (tok->strAt(4) == "0" && tok->linkAt(3)->strAt(-1) == "npos") {
                    if (!tok->linkAt(3)->previous()->variable()) // Make sure that its no variable
                        uselessCallsSubstrError(tok, false);
                } else if (Token::simpleMatch(tok->linkAt(3)->tokAt(-2), ", 0 )"))
                    uselessCallsSubstrError(tok, true);
            } else if (printWarning && Token::Match(tok, "[{};] %var% . empty ( ) ;") &&
                       tok->next()->variable() && tok->next()->variable()->isStlType(stl_containers_with_empty_and_clear))
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
    reportError(tok, Severity::warning, "uselessCallsCompare", errmsg.str(), CWE628, false);
}

void CheckStl::uselessCallsSwapError(const Token *tok, const std::string &varname)
{
    std::ostringstream errmsg;
    errmsg << "It is inefficient to swap a object with itself by calling '" << varname << ".swap(" << varname << ")'\n"
           << "The 'swap()' function has no logical effect when given itself as parameter "
           << "(" << varname << ".swap(" << varname << ")). As it is currently the "
           << "code is inefficient. Is the object or the parameter wrong here?";
    reportError(tok, Severity::performance, "uselessCallsSwap", errmsg.str(), CWE628, false);
}

void CheckStl::uselessCallsSubstrError(const Token *tok, bool empty)
{
    if (empty)
        reportError(tok, Severity::performance, "uselessCallsSubstr", "Ineffective call of function 'substr' because it returns an empty string.", CWE398, false);
    else
        reportError(tok, Severity::performance, "uselessCallsSubstr", "Ineffective call of function 'substr' because it returns a copy of the object. Use operator= instead.", CWE398, false);
}

void CheckStl::uselessCallsEmptyError(const Token *tok)
{
    reportError(tok, Severity::warning, "uselessCallsEmpty", "Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead?", CWE398, false);
}

void CheckStl::uselessCallsRemoveError(const Token *tok, const std::string& function)
{
    reportError(tok, Severity::warning, "uselessCallsRemove", "Return value of std::" + function + "() ignored. Elements remain in container.\n"
                "The return value of std::" + function + "() is ignored. This function returns an iterator to the end of the range containing those elements that should be kept. "
                "Elements past new end remain valid but with unspecified values. Use the erase method of the container to delete them.", CWE762, false);
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
        if (i->type == Scope::eIf || i->type == Scope::eDo || i->type == Scope::eWhile || i->type == Scope::eFor) {

            const Token* const tok = i->classDef;
            const Token* startOfCondition = tok->next();
            if (i->type == Scope::eDo)
                startOfCondition = startOfCondition->link()->tokAt(2);
            if (!startOfCondition) // ticket #6626 invalid code
                continue;
            const Token* endOfCondition = startOfCondition->link();
            if (!endOfCondition)
                continue;

            // For "for" loops, only search between the two semicolons
            if (i->type == Scope::eFor) {
                startOfCondition = Token::findsimplematch(tok->tokAt(2), ";", endOfCondition);
                if (!startOfCondition)
                    continue;
                endOfCondition = Token::findsimplematch(startOfCondition->next(), ";", endOfCondition);
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
                    Token::findmatch(startOfCondition, "&& %var% != %name% . end|rend|cend|crend ( )", endOfCondition);
            } else if (isOrExpression && !isAndExpression) {
                validityCheckTok =
                    Token::findmatch(startOfCondition, "%oror% %var% == %name% . end|rend|cend|crend ( )", endOfCondition);
            }

            if (!validityCheckTok)
                continue;
            const unsigned int iteratorVarId = validityCheckTok->next()->varId();

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
                "Make sure to check that the iterator is valid before dereferencing it - not after.", CWE825, false);
}



void CheckStl::readingEmptyStlContainer_parseUsage(const Token* tok, const Library::Container* container, std::map<unsigned int, const Library::Container*>& empty, bool noerror)
{
    // Check for various conditions for the way stl containers and variables can be used
    if (tok->strAt(1) == "=" || (tok->strAt(1) == "[" && Token::simpleMatch(tok->linkAt(1), "] ="))) {
        // Assignment (LHS)
        empty.erase(tok->varId());
    } else if (Token::Match(tok, "%name% [")) {
        // Access through operator[]
        if (!container->arrayLike_indexOp) { // operator[] inserts an element if used on a std::map
            if (!noerror && tok->strAt(-1) == "=")
                readingEmptyStlContainerError(tok);
            empty.erase(tok->varId());
        } else if (!noerror)
            readingEmptyStlContainerError(tok);
    } else if (Token::Match(tok, "%name% . %type% (")) {
        // Member function call
        const Library::Container::Action action = container->getAction(tok->strAt(2));
        if ((action == Library::Container::FIND || action == Library::Container::ERASE || action == Library::Container::POP || action == Library::Container::CLEAR) && !noerror) {
            readingEmptyStlContainerError(tok);
            return;
        }

        const Token* parent = tok->tokAt(3)->astParent();
        const Library::Container::Yield yield = container->getYield(tok->strAt(2));
        bool yieldsIterator = (yield == Library::Container::ITERATOR || yield == Library::Container::START_ITERATOR || yield == Library::Container::END_ITERATOR);
        if (yield != Library::Container::NO_YIELD &&
            (!parent || Token::Match(parent, "%cop%|*") || parent->isAssignmentOp() || !yieldsIterator)) { // These functions read from the container
            if (!noerror && (!yieldsIterator || !parent || !parent->isAssignmentOp()))
                readingEmptyStlContainerError(tok);
        } else
            empty.erase(tok->varId());
    } else if (tok->strAt(-1) == "=") {
        // Assignment (RHS)
        if (!noerror)
            readingEmptyStlContainerError(tok);
    } else {
        // Unknown usage. Assume it is initialized.
        empty.erase(tok->varId());
    }
}

void CheckStl::readingEmptyStlContainer()
{
    if (!_settings->isEnabled("style"))
        return;

    if (!_settings->inconclusive)
        return;

    std::map<unsigned int, const Library::Container*> emptyContainer;

    const std::list<Scope>& scopeList = _tokenizer->getSymbolDatabase()->scopeList;

    for (std::list<Scope>::const_iterator i = scopeList.begin(); i != scopeList.end(); ++i) {
        if (i->type != Scope::eFunction)
            continue;

        for (const Token *tok = i->classStart->next(); tok != i->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "for|while")) { // Loops and end of scope clear the sets.
                const Token* tok2 = tok->linkAt(1);
                if (!tok2)
                    continue;
                tok2 = tok2->next();
                for (const Token* end2 = tok2->link(); tok2 && tok2 != end2; tok2 = tok2->next()) {
                    if (!tok2->varId())
                        continue;

                    const std::map<unsigned int, const Library::Container*>::const_iterator container = emptyContainer.find(tok2->varId());
                    if (container == emptyContainer.end())
                        continue;

                    readingEmptyStlContainer_parseUsage(tok2, container->second, emptyContainer, true);
                }
            } else if (Token::Match(tok, "do|}|break|case")) {
                emptyContainer.clear();
            }

            if (!tok->varId())
                continue;

            // Check whether a variable should be marked as "empty"
            const Variable* var = tok->variable();
            if (var && !var->isArrayOrPointer() && !var->typeStartToken()->isStandardType()) {
                bool insert = false;
                if (var->nameToken() == tok && var->isLocal() && !var->isStatic()) { // Local variable declared
                    insert = !Token::Match(tok->next(), "[(=]"); // Only if not initialized
                } else if (Token::Match(tok, "%var% . clear ( ) ;")) {
                    insert = true;
                }

                if (insert) {
                    const Library::Container* container = _settings->library.detectContainer(var->typeStartToken());
                    if (container)
                        emptyContainer[var->declarationId()] = container;
                    continue;
                }
            }

            const std::map<unsigned int, const Library::Container*>::const_iterator container = emptyContainer.find(tok->varId());
            if (container == emptyContainer.end())
                continue;

            readingEmptyStlContainer_parseUsage(tok, container->second, emptyContainer, false);
        }
        emptyContainer.clear();
    }
}

void CheckStl::readingEmptyStlContainerError(const Token *tok)
{
    reportError(tok, Severity::style, "reademptycontainer", "Reading from empty STL container '" + (tok ? tok->str() : std::string("var")) + "'", CWE398, true);
}
