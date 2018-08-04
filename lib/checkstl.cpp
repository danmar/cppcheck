/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "checknullpointer.h"
#include "errorlogger.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "astutils.h"

#include <cstddef>
#include <list>
#include <set>
#include <sstream>
#include <utility>

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
static const struct CWE CWE786(786U);   // Access of Memory Location Before Start of Buffer
static const struct CWE CWE788(788U);   // Access of Memory Location After End of Buffer
static const struct CWE CWE825(825U);   // Expired Pointer Dereference
static const struct CWE CWE834(834U);   // Excessive Iteration

// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &iteratorName)
{
    reportError(tok, Severity::error, "invalidIterator1", "$symbol:"+iteratorName+"\nInvalid iterator: $symbol", CWE664, false);
}

void CheckStl::iteratorsError(const Token *tok, const std::string &container1, const std::string &container2)
{
    reportError(tok, Severity::error, "iterators",
                "$symbol:" + container1 + "\n"
                "$symbol:" + container2 + "\n"
                "Same iterator is used with different containers '" + container1 + "' and '" + container2 + "'.", CWE664, false);
}

// Error message used when dereferencing an iterator that has been erased..
void CheckStl::dereferenceErasedError(const Token *erased, const Token* deref, const std::string &itername, bool inconclusive)
{
    if (erased) {
        std::list<const Token*> callstack = { deref, erased };
        reportError(callstack, Severity::error, "eraseDereference",
                    "$symbol:" + itername + "\n"
                    "Iterator '$symbol' used after element has been erased.\n"
                    "The iterator '$symbol' is invalid after the element it pointed to has been erased. "
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, inconclusive);
    } else {
        reportError(deref, Severity::error, "eraseDereference",
                    "$symbol:" + itername + "\n"
                    "Invalid iterator '$symbol' used.\n"
                    "The iterator '$symbol' is invalid before being assigned. "
                    "Dereferencing or comparing it with another iterator is invalid operation.", CWE664, inconclusive);
    }
}

static const Token *skipMembers(const Token *tok)
{
    while (Token::Match(tok, "%name% ."))
        tok = tok->tokAt(2);
    return tok;
}

static bool isIterator(const Variable *var, bool& inconclusiveType)
{
    // Check that its an iterator
    if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator|auto"))
        return false;

    inconclusiveType = false;
    if (var->typeEndToken()->str() == "auto")
        return (var->nameToken()->valueType() && var->nameToken()->valueType()->type == ValueType::Type::ITERATOR);

    if (var->type()) { // If it is defined, ensure that it is defined like an iterator
        // look for operator* and operator++
        const Function* end = var->type()->getFunction("operator*");
        const Function* incOperator = var->type()->getFunction("operator++");
        if (!end || end->argCount() > 0 || !incOperator) {
            return false;
        } else {
            inconclusiveType = true; // heuristics only
        }
    }

    return true;
}

static std::string getContainerName(const Token *containerToken)
{
    if (!containerToken)
        return std::string();
    std::string ret(containerToken->str());
    for (const Token *nametok = containerToken; nametok; nametok = nametok->tokAt(-2)) {
        if (!Token::Match(nametok->tokAt(-2), "%name% ."))
            break;
        ret = nametok->strAt(-2) + '.' + ret;
    }
    return ret;
}

void CheckStl::iterators()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Variable* var : symbolDatabase->variableList()) {
        bool inconclusiveType=false;
        if (!isIterator(var, inconclusiveType))
            continue;

        const unsigned int iteratorId = var->declarationId();

        // the validIterator flag says if the iterator has a valid value or not
        bool validIterator = Token::Match(var->nameToken()->next(), "[(=:{]");
        const Scope* invalidationScope = nullptr;

        // The container this iterator can be used with
        const Token* containerToken = nullptr;
        const Scope* containerAssignScope = nullptr;

        // When "validatingToken" is reached the validIterator is set to true
        const Token* validatingToken = nullptr;

        const Token* eraseToken = nullptr;

        // Scan through the rest of the code and see if the iterator is
        // used against other containers.
        for (const Token *tok2 = var->nameToken(); tok2 && tok2 != var->scope()->bodyEnd; tok2 = tok2->next()) {
            if (invalidationScope && tok2 == invalidationScope->bodyEnd)
                validIterator = true; // Assume that the iterator becomes valid again
            if (containerAssignScope && tok2 == containerAssignScope->bodyEnd)
                containerToken = nullptr; // We don't know which containers might be used with the iterator

            if (tok2 == validatingToken) {
                validIterator = true;
                eraseToken = nullptr;
                invalidationScope = nullptr;
            }

            // Is iterator compared against different container?
            if (tok2->isComparisonOp() && containerToken && tok2->astOperand1() && tok2->astOperand2()) {
                const Token *other = nullptr;
                if (tok2->astOperand1()->varId() == iteratorId)
                    other = tok2->astOperand2()->tokAt(-3);
                else if (tok2->astOperand2()->varId() == iteratorId)
                    other = tok2->astOperand1()->tokAt(-3);
                if (Token::Match(other, "%name% . end|rend|cend|crend ( )") && other->varId() != containerToken->varId())
                    iteratorsError(tok2, getContainerName(containerToken), getContainerName(other));
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
                if (containerToken && tok2->varId() != containerToken->varId()) {
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
                            if (par2->varId() == containerToken->varId())
                                break;
                            bool inconclusiveType2=false;
                            if (isIterator(par2->variable(), inconclusiveType2))
                                break;  // TODO: check if iterator points at same container
                            if (par2->str() == "(")
                                par2 = par2->link();
                            par2 = par2->next();
                        }
                        if (par2->str() != ")")
                            continue;
                    }

                    // Show error message, mismatching iterator is used.
                    iteratorsError(tok2, getContainerName(containerToken), getContainerName(tok2));
                }

                // invalidate the iterator if it is erased
                else if (tok2->strAt(2) == "erase" && (tok2->strAt(4) != "*" || (containerToken && tok2->varId() == containerToken->varId()))) {
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
                validatingToken = skipMembers(tok2->tokAt(2))->linkAt(1);
                tok2 = validatingToken->link();
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %name% .", iteratorId) &&
                     Token::Match(skipMembers(tok2->tokAt(2)), "begin|rbegin|cbegin|crbegin|find (")) {
                validatingToken = skipMembers(tok2->tokAt(2))->linkAt(1);
                containerToken = skipMembers(tok2->tokAt(2))->tokAt(-2);
                if (containerToken->varId() == 0 || Token::simpleMatch(validatingToken, ") ."))
                    containerToken = nullptr;
                containerAssignScope = tok2->scope();

                // skip ahead
                tok2 = validatingToken->link();
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
                dereferenceErasedError(eraseToken, tok2, tok2->strAt(1), inconclusiveType);
                tok2 = tok2->next();
            } else if (!validIterator && Token::Match(tok2, "%varid% . %name%", iteratorId)) {
                dereferenceErasedError(eraseToken, tok2, tok2->str(), inconclusiveType);
                tok2 = tok2->tokAt(2);
            }

            // bailout handling. Assume that the iterator becomes valid if we see return/break.
            // TODO: better handling
            else if (tok2->scope() == invalidationScope && Token::Match(tok2, "return|break|continue")) {
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
    reportError(tok, Severity::error, "mismatchingContainers", "Iterators of different containers are used together.", CWE664, false);
}

void CheckStl::mismatchingContainerExpressionError(const Token *tok1, const Token *tok2)
{
    const std::string expr1(tok1 ? tok1->expressionString() : std::string("v1"));
    const std::string expr2(tok2 ? tok2->expressionString() : std::string("v2"));
    reportError(tok1, Severity::warning, "mismatchingContainerExpression",
                "Iterators to containers from different expressions '" +
                expr1 + "' and '" + expr2 + "' are used together.", CWE664, false);
}

static const std::set<std::string> algorithm2 = { // func(begin1, end1
    "binary_search", "copy", "copy_if", "equal_range"
    , "generate", "is_heap", "is_heap_until", "is_partitioned"
    , "is_permutation", "is_sorted", "is_sorted_until", "lower_bound", "make_heap", "max_element", "minmax_element"
    , "min_element", "mismatch", "move", "move_backward", "next_permutation", "partition", "partition_copy"
    , "partition_point", "pop_heap", "prev_permutation", "push_heap", "random_shuffle", "remove", "remove_copy"
    , "remove_copy_if", "remove_if", "replace", "replace_copy", "replace_copy_if", "replace_if", "reverse", "reverse_copy"
    , "shuffle", "sort", "sort_heap", "stable_partition", "stable_sort", "swap_ranges", "transform", "unique"
    , "unique_copy", "upper_bound", "string", "wstring", "u16string", "u32string"
};
static const std::set<std::string> algorithm22 = { // func(begin1, end1, begin2, end2
    "includes", "lexicographical_compare", "merge", "partial_sort_copy"
    , "set_difference", "set_intersection", "set_symmetric_difference", "set_union"
};
static const std::set<std::string> algorithm1x1 = {  // func(begin1, x, end1
    "nth_element", "partial_sort", "rotate", "rotate_copy"
};

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

static const Token * getIteratorExpression(const Token * tok)
{
    if (!tok)
        return nullptr;
    if (!tok->isName()) {
        const Token *iter1 = getIteratorExpression(tok->astOperand1());
        if (iter1)
            return iter1;
        const Token *iter2 = getIteratorExpression(tok->astOperand2());
        if (iter2)
            return iter2;
    } else if (Token::Match(tok, "begin|cbegin|rbegin|crbegin|end|cend|rend|crend (")) {
        if (Token::Match(tok->previous(), ". %name% ( )"))
            return tok->previous()->astOperand1();
        if (Token::Match(tok, "%name% ( !!)"))
            return tok->next()->astOperand2();
    }
    return nullptr;
}

void CheckStl::mismatchingContainers()
{
    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const ftok = tok;
            const Token * firstArg = nullptr;

            const std::vector<const Token *> args = getArguments(ftok);
            if (args.size() < 2)
                continue;

            std::map<const Variable *, unsigned int> containerNr;
            for (unsigned int argnr = 1; argnr <= args.size(); ++argnr) {
                const Library::ArgumentChecks::IteratorInfo *i = mSettings->library.getArgIteratorInfo(ftok, argnr);
                if (!i)
                    continue;
                const Token * const argTok = args[argnr - 1];
                const Variable *c = getContainer(argTok);
                if (c) {
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
                } else {
                    if (i->first) {
                        firstArg = argTok;
                    } else if (i->last && firstArg && argTok) {
                        const Token * iter1 = getIteratorExpression(firstArg);
                        const Token * iter2 = getIteratorExpression(argTok);
                        if (iter1 && iter2 && !isSameExpression(true, false, iter1, iter2, mSettings->library, false)) {
                            mismatchingContainerExpressionError(iter1, iter2);
                        }
                    }
                }
            }
            const int ret = mSettings->library.returnValueContainer(ftok);
            if (ret != -1 && Token::Match(ftok->next()->astParent(), "==|!=")) {
                const Token *parent = ftok->next()->astParent();
                const Token *other = (parent->astOperand1() == ftok->next()) ? parent->astOperand2() : parent->astOperand1();
                const Variable *c = getContainer(other);
                if (c) {
                    const std::map<const Variable *, unsigned int>::const_iterator it = containerNr.find(c);
                    if (it == containerNr.end() || it->second != ret)
                        mismatchingContainersError(other);
                }
            }
        }
    }
    for (const Variable *var : symbolDatabase->variableList()) {
        if (var && var->isStlStringType() && Token::Match(var->nameToken(), "%var% (") && Token::Match(var->nameToken()->tokAt(2), pattern2.c_str())) {
            if (var->nameToken()->strAt(2) != var->nameToken()->strAt(8)) {
                mismatchingContainersError(var->nameToken());
            }
        }
    }
}


void CheckStl::stlOutOfBounds()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Scan through all scopes..
    for (const Scope &scope : symbolDatabase->scopeList) {
        const Token* tok = scope.classDef;
        // only interested in conditions
        if ((scope.type != Scope::eFor && scope.type != Scope::eWhile && scope.type != Scope::eIf && scope.type != Scope::eDo) || !tok)
            continue;

        if (scope.type == Scope::eFor)
            tok = Token::findsimplematch(tok->tokAt(2), ";");
        else if (scope.type == Scope::eDo) {
            tok = tok->linkAt(1)->tokAt(2);
        } else
            tok = tok->next();

        if (!tok)
            continue;
        tok = tok->next();

        // check if the for loop condition is wrong
        if (!Token::Match(tok, "%var% <= %var% . %name% ( ) ;|)|%oror%"))
            continue;
        // Is it a vector?
        const Variable *var = tok->tokAt(2)->variable();
        if (!var)
            continue;

        const Library::Container* container = mSettings->library.detectContainer(var->typeStartToken());
        if (!container)
            continue;

        if (container->getYield(tok->strAt(4)) != Library::Container::SIZE)
            continue;

        // variable id for loop variable.
        const unsigned int numId = tok->varId();

        // variable id for the container variable
        const unsigned int declarationId = var->declarationId();

        for (const Token *tok3 = scope.bodyStart; tok3 && tok3 != scope.bodyEnd; tok3 = tok3->next()) {
            if (tok3->varId() == declarationId) {
                tok3 = tok3->next();
                if (Token::Match(tok3, ". %name% ( )")) {
                    if (container->getYield(tok3->strAt(1)) == Library::Container::SIZE)
                        break;
                } else if (container->arrayLike_indexOp && Token::Match(tok3, "[ %varid% ]", numId))
                    stlOutOfBoundsError(tok3, tok3->strAt(1), var->name(), false);
                else if (Token::Match(tok3, ". %name% ( %varid% )", numId)) {
                    const Library::Container::Yield yield = container->getYield(tok3->strAt(1));
                    if (yield == Library::Container::AT_INDEX)
                        stlOutOfBoundsError(tok3, tok3->strAt(3), var->name(), true);
                }
            }
        }
    }
}

void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var, bool at)
{
    if (at)
        reportError(tok, Severity::error, "stlOutOfBounds", "$symbol:" + var + "\nWhen " + num + "==$symbol.size(), $symbol.at(" + num + ") is out of bounds.", CWE788, false);
    else
        reportError(tok, Severity::error, "stlOutOfBounds", "$symbol:" + var + "\nWhen " + num + "==$symbol.size(), $symbol[" + num + "] is out of bounds.", CWE788, false);
}

void CheckStl::negativeIndex()
{
    // Negative index is out of bounds..
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t ii = 0; ii < functions; ++ii) {
        const Scope * scope = symbolDatabase->functionScopes[ii];
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%var% [") || WRONG_DATA(!tok->next()->astOperand2(), tok))
                continue;
            const Variable * const var = tok->variable();
            if (!var || tok == var->nameToken())
                continue;
            const Library::Container * const container = mSettings->library.detectContainer(var->typeStartToken());
            if (!container || !container->arrayLike_indexOp)
                continue;
            const ValueFlow::Value *index = tok->next()->astOperand2()->getValueLE(-1, mSettings);
            if (!index)
                continue;
            negativeIndexError(tok, *index);
        }
    }
}

void CheckStl::negativeIndexError(const Token *tok, const ValueFlow::Value &index)
{
    const ErrorPath errorPath = getErrorPath(tok, &index, "Negative array index");
    std::ostringstream errmsg;
    if (index.condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(index.condition)
               << ", otherwise there is negative array index " << index.intvalue << ".";
    else
        errmsg << "Array index " << index.intvalue << " is out of bounds.";
    reportError(errorPath, index.errorSeverity() ? Severity::error : Severity::warning, "negativeContainerIndex", errmsg.str(), CWE786, index.isInconclusive());
}

void CheckStl::erase()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

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
    bool inconclusiveType=false;
    if (!isIterator(var, inconclusiveType))
        return;
    for (const Token *tok = scope.bodyStart; tok != scope.bodyEnd; tok = tok->next()) {
        if (tok->str() != "(")
            continue;
        if (!Token::Match(tok->tokAt(-2), ". erase ( ++| %varid% )", var->declarationId()))
            continue;
        if (Token::simpleMatch(tok->astParent(), "="))
            continue;
        // Iterator is invalid..
        unsigned int indentlevel = 0U;
        const Token *tok2 = tok->link();
        for (; tok2 != scope.bodyEnd; tok2 = tok2->next()) {
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
                dereferenceErasedError(tok, tok2, tok2->str(), inconclusiveType);
                break;
            }
            if (indentlevel == 0U && Token::Match(tok2, "break|return|goto"))
                break;
        }
        if (tok2 == scope.bodyEnd)
            dereferenceErasedError(tok, scope.classDef, var->nameToken()->str(), inconclusiveType);
    }
}

void CheckStl::pushback()
{
    // Pointer can become invalid after push_back, push_front, reserve or resize..
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% = & %var% [")) {
                // Skip it directly if it is a pointer or an array
                const Token* containerTok = tok->tokAt(3);
                if (containerTok->variable() && containerTok->variable()->isArrayOrPointer())
                    continue;

                // Variable id for pointer
                const unsigned int pointerId(tok->varId());

                bool invalidPointer = false;
                const Token* function = nullptr;
                const Token* end2 = tok->scope()->bodyEnd;
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
    for (const Variable* var : symbolDatabase->variableList()) {
        // Check that its an iterator
        if (!var || !var->isLocal() || !Token::Match(var->typeEndToken(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator"))
            continue;

        const unsigned int iteratorId = var->declarationId();

        // ... on std::vector
        if (!Token::Match(var->typeStartToken(), "std| ::| vector <"))
            continue;

        // the variable id for the vector
        unsigned int vectorid = 0;

        const Token* validatingToken = nullptr;

        std::string invalidIterator;
        const Token* end2 = var->scope()->bodyEnd;
        for (const Token *tok2 = var->nameToken(); tok2 != end2; tok2 = tok2->next()) {

            if (validatingToken == tok2) {
                invalidIterator.clear();
                validatingToken = nullptr;
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
                        pushbackTok = nullptr;
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
                invalidIterator.clear();
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
    reportError(tok, Severity::error, "invalidIterator2",
                "$symbol:" + func + "\n"
                "$symbol:" + iterator_name + "\n"
                "After " + func + "(), the iterator '" + iterator_name + "' may be invalid.", CWE664, false);
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &func, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "invalidPointer",
                "$symbol:" + func + "\n"
                "$symbol:" + pointer_name + "\n"
                "Invalid pointer '" + pointer_name + "' after " + func + "().", CWE664, false);
}


void CheckStl::stlBoundaries()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Variable* var : symbolDatabase->variableList()) {
        if (!var || !var->scope() || !var->scope()->isExecutable())
            continue;

        const Library::Container* container = mSettings->library.detectContainer(var->typeStartToken(), true);
        if (!container || container->opLessAllowed)
            continue;

        const Token* const end = var->scope()->bodyEnd;
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
    const bool printWarning = mSettings->isEnabled(Settings::WARNING);
    const bool printPerformance = mSettings->isEnabled(Settings::PERFORMANCE);
    if (!printWarning && !printPerformance)
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if ((i->type != Scope::eIf && i->type != Scope::eWhile) || !i->classDef)
            continue;

        for (const Token *tok = i->classDef->next(); tok->str() != "{"; tok = tok->next()) {
            const Token* funcTok = nullptr;
            const Library::Container* container = nullptr;

            if (Token::Match(tok, "%name% ("))
                tok = tok->linkAt(1);

            else if (tok->variable() && Token::Match(tok, "%var% . %name% (")) {
                container = mSettings->library.detectContainer(tok->variable()->typeStartToken());
                funcTok = tok->tokAt(2);
            }

            // check also for vector-like or pointer containers
            else if (tok->variable() && tok->astParent() && (tok->astParent()->str() == "*" || tok->astParent()->str() == "[")) {
                const Token *tok2 = tok->astParent();

                if (!Token::Match(tok2->astParent(), ". %name% ("))
                    continue;

                funcTok = tok2->astParent()->next();

                if (tok->variable()->isArrayOrPointer())
                    container = mSettings->library.detectContainer(tok->variable()->typeStartToken());
                else { // Container of container - find the inner container
                    container = mSettings->library.detectContainer(tok->variable()->typeStartToken()); // outer container
                    tok2 = Token::findsimplematch(tok->variable()->typeStartToken(), "<", tok->variable()->typeEndToken());
                    if (container && container->type_templateArgNo >= 0 && tok2) {
                        tok2 = tok2->next();
                        for (int j = 0; j < container->type_templateArgNo; j++)
                            tok2 = tok2->nextTemplateArgument();

                        container = mSettings->library.detectContainer(tok2); // innner container
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
    if (!mSettings->isEnabled(Settings::PERFORMANCE))
        return;

    if (mSettings->standards.cpp == Standards::CPP11)
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
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
                "$symbol:" + varname + "\n"
                "Possible inefficient checking for '$symbol' emptiness.\n"
                "Checking for '$symbol' emptiness might be inefficient. "
                "Using $symbol.empty() instead of $symbol.size() can be faster. "
                "$symbol.size() can take linear time but $symbol.empty() is "
                "guaranteed to take constant time.", CWE398, false);
}

void CheckStl::redundantCondition()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

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
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eFor || !i->classDef)
            continue;

        for (const Token *tok2 = i->classDef->tokAt(2); tok2 != i->bodyStart; tok2 = tok2->next()) {
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
            for (const Token *tok3 = i->bodyStart; tok3 != i->bodyEnd; tok3 = tok3->next()) {
                if (Token::Match(tok3, "%varid% ++", iteratorId))
                    incrementToken = tok3;
                else if (Token::Match(tok3->previous(), "++ %varid% !!.", iteratorId))
                    incrementToken = tok3;
                else if (Token::Match(tok3, "%varid% !=|==", iteratorId))
                    incrementToken = nullptr;
                else if (tok3->str() == "break" || tok3->str() == "return")
                    incrementToken = nullptr;
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
    std::list<const Token*> callstack = { incrementToken1,incrementToken2 };

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
    const std::set<std::string> stl_string_stream = {
        "istringstream", "ostringstream", "stringstream", "wstringstream"
    };
}

void CheckStl::string_c_str()
{
    const bool printInconclusive = mSettings->inconclusive;
    const bool printPerformance = mSettings->isEnabled(Settings::PERFORMANCE);

    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();

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

        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
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
                const std::pair<std::multimap<std::string, unsigned int>::const_iterator, std::multimap<std::string, unsigned int>::const_iterator> range = c_strFuncParam.equal_range(tok->str());
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
                    else if (tok2)
                        tok2 = tok2->previous();
                    else
                        break;
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
    const int malloc = mSettings->library.allocId("malloc"); // allocation function, which are not compatible with auto_ptr
    const bool printStyle = mSettings->isEnabled(Settings::STYLE);

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
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
                    if (Token::Match(tok3, "( %name% (") && malloc && mSettings->library.alloc(tok3->next(), -1) == malloc) {
                        // malloc-like function allocated memory passed to the auto_ptr constructor -> error
                        autoPointerMallocError(tok2->next(), tok3->next()->str());
                    }
                    if (Token::Match(tok3, "( %var%")) {
                        const std::map<unsigned int, const std::string>::const_iterator it = mallocVarId.find(tok3->next()->varId());
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
                            const Token *decltok = Token::findmatch(mTokenizer->tokens(), "%varid% = new %type%", tok3->varId());
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
                    const std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->tokAt(2)->varId());
                    if (iter != autoPtrVarId.end()) {
                        autoPointerError(tok->tokAt(2));
                    }
                }
            } else if ((Token::Match(tok, "%var% = new %type%") && hasArrayEnd(tok)) ||
                       (Token::Match(tok, "%var% . reset ( new %type%") && hasArrayEndParen(tok))) {
                const std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerArrayError(tok);
                }
            } else if (Token::Match(tok, "%var% = %name% (") && malloc && mSettings->library.alloc(tok->tokAt(2), -1) == malloc) {
                // C library function like 'malloc' used together with auto pointer -> error
                const std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
                if (iter != autoPtrVarId.end()) {
                    autoPointerMallocError(tok, tok->strAt(2));
                } else if (tok->varId()) {
                    // it is not an auto pointer variable and it is allocated by malloc like function.
                    mallocVarId.insert(std::make_pair(tok->varId(), tok->strAt(2)));
                }
            } else if (Token::Match(tok, "%var% . reset ( %name% (") && malloc && mSettings->library.alloc(tok->tokAt(4), -1) == malloc) {
                // C library function like 'malloc' used when resetting auto pointer -> error
                const std::set<unsigned int>::const_iterator iter = autoPtrVarId.find(tok->varId());
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
    const std::string summary = "Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function '$symbol'.";
    const std::string verbose = summary + " This means that you should only use 'auto_ptr' for pointers obtained with operator 'new'. This excludes use C library allocation functions (for example '$symbol'), which must be deallocated by the appropriate C library function.";
    reportError(tok, Severity::error, "useAutoPointerMalloc", "$symbol:" + allocFunction + '\n' + summary + '\n' + verbose, CWE762, false);
}

namespace {
    const std::set<std::string> stl_containers_with_empty_and_clear = {
        "deque",  "forward_list",  "list",
        "map",  "multimap",  "multiset",  "set",  "string",
        "unordered_map",  "unordered_multimap",  "unordered_multiset",
        "unordered_set",  "vector",  "wstring"
    };

}

void CheckStl::uselessCalls()
{
    const bool printPerformance = mSettings->isEnabled(Settings::PERFORMANCE);
    const bool printWarning = mSettings->isEnabled(Settings::WARNING);
    if (!printPerformance && !printWarning)
        return;


    const SymbolDatabase* symbolDatabase = mTokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
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
                       !tok->tokAt(4)->astParent() &&
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
    errmsg << "$symbol:" << varname << '\n';
    errmsg << "$symbol:" << function << '\n';
    errmsg << "It is inefficient to call '" << varname << "." << function << "(" << varname << ")' as it always returns 0.\n"
           << "'std::string::" << function << "()' returns zero when given itself as parameter "
           << "(" << varname << "." << function << "(" << varname << ")). As it is currently the "
           << "code is inefficient. It is possible either the string searched ('"
           << varname << "') or searched for ('" << varname << "') is wrong.";
    reportError(tok, Severity::warning, "uselessCallsCompare", errmsg.str(), CWE628, false);
}

void CheckStl::uselessCallsSwapError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::performance, "uselessCallsSwap",
                "$symbol:" + varname + "\n"
                "It is inefficient to swap a object with itself by calling '$symbol.swap($symbol)'\n"
                "The 'swap()' function has no logical effect when given itself as parameter "
                "($symbol.swap($symbol)). As it is currently the "
                "code is inefficient. Is the object or the parameter wrong here?", CWE628, false);
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
    reportError(tok, Severity::warning, "uselessCallsRemove",
                "$symbol:" + function + "\n"
                "Return value of std::$symbol() ignored. Elements remain in container.\n"
                "The return value of std::$symbol() is ignored. This function returns an iterator to the end of the range containing those elements that should be kept. "
                "Elements past new end remain valid but with unspecified values. Use the erase method of the container to delete them.", CWE762, false);
}

// Check for iterators being dereferenced before being checked for validity.
// E.g.  if (*i && i != str.end()) { }
void CheckStl::checkDereferenceInvalidIterator()
{
    if (!mSettings->isEnabled(Settings::WARNING))
        return;

    // Iterate over "if", "while", and "for" conditions where there may
    // be an iterator that is dereferenced before being checked for validity.
    const std::list<Scope>& scopeList = mTokenizer->getSymbolDatabase()->scopeList;
    for (std::list<Scope>::const_iterator i = scopeList.begin(); i != scopeList.end(); ++i) {
        if (!(i->type == Scope::eIf || i->type == Scope::eDo || i->type == Scope::eWhile || i->type == Scope::eFor))
            continue;

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
            Token::findsimplematch(startOfCondition, "||", endOfCondition) != nullptr;
        const bool isAndExpression =
            Token::findsimplematch(startOfCondition, "&&", endOfCondition) != nullptr;

        // Look for a check of the validity of an iterator
        const Token* validityCheckTok = nullptr;
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

void CheckStl::dereferenceInvalidIteratorError(const Token* deref, const std::string &iterName)
{
    reportError(deref, Severity::warning,
                "derefInvalidIterator",
                "$symbol:" + iterName + "\n"
                "Possible dereference of an invalid iterator: $symbol\n"
                "Possible dereference of an invalid iterator: $symbol. Make sure to check that the iterator is valid before dereferencing it - not after.", CWE825, false);
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
        const bool yieldsIterator = (yield == Library::Container::ITERATOR || yield == Library::Container::START_ITERATOR || yield == Library::Container::END_ITERATOR);
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
    if (!mSettings->isEnabled(Settings::STYLE))
        return;

    if (!mSettings->inconclusive)
        return;

    std::map<unsigned int, const Library::Container*> emptyContainer;

    const std::list<Scope>& scopeList = mTokenizer->getSymbolDatabase()->scopeList;

    for (std::list<Scope>::const_iterator i = scopeList.begin(); i != scopeList.end(); ++i) {
        if (i->type != Scope::eFunction)
            continue;

        for (const Token *tok = i->bodyStart->next(); tok != i->bodyEnd; tok = tok->next()) {
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
            } else if (tok->str() == "{" && tok->next()->scope()->type == Scope::eLambda)
                tok = tok->link();

            // function call
            if (Token::Match(tok, "!!. %name% (") && !Token::simpleMatch(tok->linkAt(2), ") {")) {
                for (std::map<unsigned int, const Library::Container*>::iterator it = emptyContainer.begin(); it != emptyContainer.end();) {
                    const Variable *var = mTokenizer->getSymbolDatabase()->getVariableFromVarId(it->first);
                    if (var && (var->isLocal() || var->isArgument()))
                        ++it;
                    else
                        emptyContainer.erase(it++);
                }
            }

            if (!tok->varId())
                continue;

            // Check whether a variable should be marked as "empty"
            const Variable* var = tok->variable();
            if (var && !var->isArrayOrPointer() && !var->typeStartToken()->isStandardType()) {
                bool insert = false;
                if (var->nameToken() == tok && var->isLocal() && !var->isStatic()) { // Local variable declared
                    insert = !Token::Match(tok->next(), "[(=:]"); // Only if not initialized
                } else if (Token::Match(tok, "%var% . clear ( ) ;")) {
                    insert = true;
                }

                if (insert) {
                    const Library::Container* container = mSettings->library.detectContainer(var->typeStartToken());
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
    const std::string varname = tok ? tok->str() : std::string("var");
    reportError(tok, Severity::style, "reademptycontainer", "$symbol:" + varname +"\nReading from empty STL container '$symbol'", CWE398, true);
}
