/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "astutils.h"
#include "check.h"
#include "checknullpointer.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "pathanalysis.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "valueflow.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
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
static const struct CWE CWE667(667U);   // Improper Locking
static const struct CWE CWE704(704U);   // Incorrect Type Conversion or Cast
static const struct CWE CWE762(762U);   // Mismatched Memory Management Routines
static const struct CWE CWE786(786U);   // Access of Memory Location Before Start of Buffer
static const struct CWE CWE788(788U);   // Access of Memory Location After End of Buffer
static const struct CWE CWE825(825U);   // Expired Pointer Dereference
static const struct CWE CWE833(833U);   // Deadlock
static const struct CWE CWE834(834U);   // Excessive Iteration

static bool isElementAccessYield(const Library::Container::Yield& yield)
{
    return yield == Library::Container::Yield::ITEM || yield == Library::Container::Yield::AT_INDEX;
}

void CheckStl::outOfBounds()
{
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            const Library::Container *container = getLibraryContainer(tok);
            if (!container)
                continue;
            const Token * parent = astParentSkipParens(tok);
            for (const ValueFlow::Value &value : tok->values()) {
                if (!value.isContainerSizeValue())
                    continue;
                if (value.isImpossible())
                    continue;
                if (value.isInconclusive() && !mSettings->inconclusive)
                    continue;
                if (!value.errorSeverity() && !mSettings->isEnabled(Settings::WARNING))
                    continue;
                if (Token::Match(parent, ". %name% (") && isElementAccessYield(container->getYield(parent->strAt(1)))) {
                    if (value.intvalue == 0) {
                        outOfBoundsError(parent->tokAt(2), tok->expressionString(), &value, parent->strAt(1), nullptr);
                        continue;
                    }
                    const Token* indexTok = parent->tokAt(2)->astOperand2();
                    if (!indexTok)
                        continue;
                    const ValueFlow::Value* indexValue = indexTok->getMaxValue(false);
                    if (indexValue && indexValue->intvalue >= value.intvalue) {
                        outOfBoundsError(
                            parent, tok->expressionString(), &value, indexTok->expressionString(), indexValue);
                        continue;
                    }
                    if (mSettings->isEnabled(Settings::WARNING)) {
                        indexValue = indexTok->getMaxValue(true);
                        if (indexValue && indexValue->intvalue >= value.intvalue) {
                            outOfBoundsError(
                                parent, tok->expressionString(), &value, indexTok->expressionString(), indexValue);
                            continue;
                        }
                    }
                }
                if (Token::Match(tok, "%name% . %name% (") && container->getYield(tok->strAt(2)) == Library::Container::Yield::START_ITERATOR) {
                    const Token *fparent = tok->tokAt(3)->astParent();
                    const Token *other = nullptr;
                    if (Token::simpleMatch(fparent, "+") && fparent->astOperand1() == tok->tokAt(3))
                        other = fparent->astOperand2();
                    else if (Token::simpleMatch(fparent, "+") && fparent->astOperand2() == tok->tokAt(3))
                        other = fparent->astOperand1();
                    if (other && other->hasKnownIntValue() && other->getKnownIntValue() > value.intvalue) {
                        outOfBoundsError(fparent, tok->expressionString(), &value, other->expressionString(), &other->values().back());
                        continue;
                    } else if (other && !other->hasKnownIntValue() && value.isKnown() && value.intvalue==0) {
                        outOfBoundsError(fparent, tok->expressionString(), &value, other->expressionString(), nullptr);
                        continue;
                    }
                }
                if (!container->arrayLike_indexOp && !container->stdStringLike)
                    continue;
                if (value.intvalue == 0 && Token::Match(parent, "[") && tok == parent->astOperand1()) {
                    outOfBoundsError(parent, tok->expressionString(), &value, "", nullptr);
                    continue;
                }
                if (container->arrayLike_indexOp && Token::Match(parent, "[")) {
                    const ValueFlow::Value *indexValue = parent->astOperand2() ? parent->astOperand2()->getMaxValue(false) : nullptr;
                    if (indexValue && indexValue->intvalue >= value.intvalue) {
                        outOfBoundsError(parent, tok->expressionString(), &value, parent->astOperand2()->expressionString(), indexValue);
                        continue;
                    }
                    if (mSettings->isEnabled(Settings::WARNING)) {
                        indexValue = parent->astOperand2() ? parent->astOperand2()->getMaxValue(true) : nullptr;
                        if (indexValue && indexValue->intvalue >= value.intvalue) {
                            outOfBoundsError(parent, tok->expressionString(), &value, parent->astOperand2()->expressionString(), indexValue);
                            continue;
                        }
                    }
                }
            }
        }
    }
}

static std::string indexValueString(const ValueFlow::Value& indexValue)
{
    if (indexValue.isIteratorStartValue())
        return "at position " + MathLib::toString(indexValue.intvalue) + " from the beginning";
    if (indexValue.isIteratorEndValue())
        return "at position " + MathLib::toString(-indexValue.intvalue) + " from the end";
    return MathLib::toString(indexValue.intvalue);
}

void CheckStl::outOfBoundsError(const Token *tok, const std::string &containerName, const ValueFlow::Value *containerSize, const std::string &index, const ValueFlow::Value *indexValue)
{
    // Do not warn if both the container size and index value are possible
    if (containerSize && indexValue && containerSize->isPossible() && indexValue->isPossible())
        return;

    const std::string expression = tok ? tok->expressionString() : (containerName+"[x]");

    std::string errmsg;
    if (!containerSize)
        errmsg = "Out of bounds access in expression '" + expression + "'";
    else if (containerSize->intvalue == 0) {
        if (containerSize->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(containerSize->condition) + " or expression '" + expression + "' cause access out of bounds.";
        else if (indexValue == nullptr && !index.empty())
            errmsg = "Out of bounds access in expression '" + expression + "' because '$symbol' is empty and '" + index + "' may be non-zero.";
        else
            errmsg = "Out of bounds access in expression '" + expression + "' because '$symbol' is empty.";
    } else if (indexValue) {
        if (containerSize->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(containerSize->condition) + " or $symbol size can be " + MathLib::toString(containerSize->intvalue) + ". Expression '" + expression + "' cause access out of bounds.";
        else if (indexValue->condition)
            errmsg = ValueFlow::eitherTheConditionIsRedundant(indexValue->condition) + " or '" + index + "' can have the value " + indexValueString(*indexValue) + ". Expression '" + expression + "' cause access out of bounds.";
        else
            errmsg = "Out of bounds access in '" + expression + "', if '$symbol' size is " + MathLib::toString(containerSize->intvalue) + " and '" + index + "' is " + indexValueString(*indexValue);
    } else {
        // should not happen
        return;
    }

    ErrorPath errorPath;
    if (!indexValue)
        errorPath = getErrorPath(tok, containerSize, "Access out of bounds");
    else {
        ErrorPath errorPath1 = getErrorPath(tok, containerSize, "Access out of bounds");
        ErrorPath errorPath2 = getErrorPath(tok, indexValue, "Access out of bounds");
        if (errorPath1.size() <= 1)
            errorPath = errorPath2;
        else if (errorPath2.size() <= 1)
            errorPath = errorPath1;
        else {
            errorPath = errorPath1;
            errorPath.splice(errorPath.end(), errorPath2);
        }
    }

    reportError(errorPath,
                (containerSize && !containerSize->errorSeverity()) || (indexValue && !indexValue->errorSeverity()) ? Severity::warning : Severity::error,
                "containerOutOfBounds",
                "$symbol:" + containerName +"\n" + errmsg,
                CWE398,
                (containerSize && containerSize->isInconclusive()) || (indexValue && indexValue->isInconclusive()));
}

bool CheckStl::isContainerSize(const Token *containerToken, const Token *expr) const
{
    if (!Token::simpleMatch(expr, "( )"))
        return false;
    if (!Token::Match(expr->astOperand1(), ". %name% ("))
        return false;
    if (!isSameExpression(mTokenizer->isCPP(), false, containerToken, expr->astOperand1()->astOperand1(), mSettings->library, false, false))
        return false;
    return containerToken->valueType()->container->getYield(expr->previous()->str()) == Library::Container::Yield::SIZE;
}

bool CheckStl::isContainerSizeGE(const Token * containerToken, const Token *expr) const
{
    if (!expr)
        return false;
    if (isContainerSize(containerToken, expr))
        return true;
    if (expr->str() == "*") {
        const Token *mul;
        if (isContainerSize(containerToken, expr->astOperand1()))
            mul = expr->astOperand2();
        else if (isContainerSize(containerToken, expr->astOperand2()))
            mul = expr->astOperand1();
        else
            return false;
        return mul && (!mul->hasKnownIntValue() || mul->values().front().intvalue != 0);
    }
    if (expr->str() == "+") {
        const Token *op;
        if (isContainerSize(containerToken, expr->astOperand1()))
            op = expr->astOperand2();
        else if (isContainerSize(containerToken, expr->astOperand2()))
            op = expr->astOperand1();
        else
            return false;
        return op && op->getValueGE(0, mSettings);
    }
    return false;
}

void CheckStl::outOfBoundsIndexExpression()
{
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            if (!tok->isName() || !tok->valueType())
                continue;
            const Library::Container *container = tok->valueType()->container;
            if (!container)
                continue;
            if (!container->arrayLike_indexOp && !container->stdStringLike)
                continue;
            if (!Token::Match(tok, "%name% ["))
                continue;
            if (isContainerSizeGE(tok, tok->next()->astOperand2()))
                outOfBoundsIndexExpressionError(tok, tok->next()->astOperand2());
        }
    }
}

void CheckStl::outOfBoundsIndexExpressionError(const Token *tok, const Token *index)
{
    const std::string varname = tok ? tok->str() : std::string("var");
    const std::string i = index ? index->expressionString() : std::string(varname + ".size()");

    std::string errmsg = "Out of bounds access of $symbol, index '" + i + "' is out of bounds.";

    reportError(tok,
                Severity::error,
                "containerOutOfBoundsIndexExpression",
                "$symbol:" + varname +"\n" + errmsg,
                CWE398,
                false);
}



// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &iteratorName)
{
    reportError(tok, Severity::error, "invalidIterator1", "$symbol:"+iteratorName+"\nInvalid iterator: $symbol", CWE664, false);
}

void CheckStl::iteratorsError(const Token* tok, const std::string& containerName1, const std::string& containerName2)
{
    reportError(tok, Severity::error, "iterators1",
                "$symbol:" + containerName1 + "\n"
                "$symbol:" + containerName2 + "\n"
                "Same iterator is used with different containers '" + containerName1 + "' and '" + containerName2 + "'.", CWE664, false);
}

void CheckStl::iteratorsError(const Token* tok, const Token* containerTok, const std::string& containerName1, const std::string& containerName2)
{
    std::list<const Token*> callstack = { tok, containerTok };
    reportError(callstack, Severity::error, "iterators2",
                "$symbol:" + containerName1 + "\n"
                "$symbol:" + containerName2 + "\n"
                "Same iterator is used with different containers '" + containerName1 + "' and '" + containerName2 + "'.", CWE664, false);
}

void CheckStl::iteratorsError(const Token* tok, const Token* containerTok, const std::string& containerName)
{
    std::list<const Token*> callstack = { tok, containerTok };
    reportError(callstack, Severity::error, "iterators3",
                "$symbol:" + containerName + "\n"
                "Same iterator is used with containers '" + containerName + "' that are defined in different scopes.", CWE664, false);
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

enum OperandPosition {
    Left,
    Right
};

static bool isVector(const Token* tok)
{
    if (!tok)
        return false;
    const Variable *var = tok->variable();
    const Token *decltok = var ? var->typeStartToken() : nullptr;
    return Token::simpleMatch(decltok, "std :: vector");
}

void CheckStl::iterators()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    // Filling map of iterators id and their scope begin
    std::map<int, const Token*> iteratorScopeBeginInfo;
    for (const Variable* var : symbolDatabase->variableList()) {
        bool inconclusiveType=false;
        if (!isIterator(var, inconclusiveType))
            continue;
        const int iteratorId = var->declarationId();
        if (iteratorId != 0)
            iteratorScopeBeginInfo[iteratorId] = var->nameToken();
    }

    for (const Variable* var : symbolDatabase->variableList()) {
        bool inconclusiveType=false;
        if (!isIterator(var, inconclusiveType))
            continue;
        if (inconclusiveType && !mSettings->inconclusive)
            continue;

        const int iteratorId = var->declarationId();

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

            // Is the iterator used in a insert/erase operation?
            if (Token::Match(tok2, "%name% . insert|erase ( *| %varid% )|,", iteratorId) && !isVector(tok2)) {
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

                    // Not different containers if a reference is used..
                    if (containerToken && containerToken->variable() && containerToken->variable()->isReference()) {
                        const Token *nameToken = containerToken->variable()->nameToken();
                        if (Token::Match(nameToken, "%name% =")) {
                            const Token *name1 = nameToken->tokAt(2);
                            const Token *name2 = tok2;
                            while (Token::Match(name1, "%name%|.|::") && name2 && name1->str() == name2->str()) {
                                name1 = name1->next();
                                name2 = name2->next();
                            }
                            if (!Token::simpleMatch(name1, ";") || !Token::Match(name2, "[;,()=]"))
                                continue;
                        }
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
                break;
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

void CheckStl::mismatchingContainerIteratorError(const Token* tok, const Token* iterTok)
{
    const std::string container(tok ? tok->expressionString() : std::string("v1"));
    const std::string iter(iterTok ? iterTok->expressionString() : std::string("it"));
    reportError(tok,
                Severity::error,
                "mismatchingContainerIterator",
                "Iterator '" + iter + "' from different container '" + container + "' are used together.",
                CWE664,
                false);
}

// Error message for bad iterator usage..
void CheckStl::mismatchingContainersError(const Token* tok1, const Token* tok2)
{
    const std::string expr1(tok1 ? tok1->expressionString() : std::string("v1"));
    const std::string expr2(tok2 ? tok2->expressionString() : std::string("v2"));
    reportError(tok1,
                Severity::error,
                "mismatchingContainers",
                "Iterators of different containers '" + expr1 + "' and '" + expr2 + "' are used together.",
                CWE664,
                false);
}

void CheckStl::mismatchingContainerExpressionError(const Token *tok1, const Token *tok2)
{
    const std::string expr1(tok1 ? tok1->expressionString() : std::string("v1"));
    const std::string expr2(tok2 ? tok2->expressionString() : std::string("v2"));
    reportError(tok1, Severity::warning, "mismatchingContainerExpression",
                "Iterators to containers from different expressions '" +
                expr1 + "' and '" + expr2 + "' are used together.", CWE664, false);
}

void CheckStl::sameIteratorExpressionError(const Token *tok)
{
    reportError(tok, Severity::style, "sameIteratorExpression", "Same iterators expression are used for algorithm.", CWE664, false);
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

static const Token * getIteratorExpression(const Token * tok)
{
    if (!tok)
        return nullptr;
    if (tok->isUnaryOp("*"))
        return nullptr;
    if (!tok->isName()) {
        const Token *iter1 = getIteratorExpression(tok->astOperand1());
        if (iter1)
            return iter1;
        if (tok->str() == "(")
            return nullptr;
        const Token *iter2 = getIteratorExpression(tok->astOperand2());
        if (iter2)
            return iter2;
    } else if (Token::Match(tok, "begin|cbegin|rbegin|crbegin|end|cend|rend|crend (")) {
        if (Token::Match(tok->previous(), ". %name% ( ) !!."))
            return tok->previous()->astOperand1();
        if (!Token::simpleMatch(tok->previous(), ".") && Token::Match(tok, "%name% ( !!)") && !Token::simpleMatch(tok->linkAt(1), ") ."))
            return tok->next()->astOperand2();
    }
    return nullptr;
}

bool CheckStl::checkIteratorPair(const Token* tok1, const Token* tok2)
{
    if (!tok1)
        return false;
    if (!tok2)
        return false;
    ValueFlow::Value val1 = getLifetimeObjValue(tok1);
    ValueFlow::Value val2 = getLifetimeObjValue(tok2);
    if (val1.tokvalue && val2.tokvalue && val1.lifetimeKind == val2.lifetimeKind) {
        if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Lambda)
            return false;
        if (tok1->astParent() == tok2->astParent() && Token::Match(tok1->astParent(), "%comp%|-")) {
            if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Address)
                return false;
            if (val1.lifetimeKind == ValueFlow::Value::LifetimeKind::Object &&
                (!astIsContainer(val1.tokvalue) || !astIsContainer(val2.tokvalue)))
                return false;
        }
        if (isSameExpression(true, false, val1.tokvalue, val2.tokvalue, mSettings->library, false, false))
            return false;
        if (val1.tokvalue->expressionString() == val2.tokvalue->expressionString())
            iteratorsError(tok1, val1.tokvalue, val1.tokvalue->expressionString());
        else
            mismatchingContainersError(val1.tokvalue, val2.tokvalue);
        return true;
    }

    const Token* iter1 = getIteratorExpression(tok1);
    const Token* iter2 = getIteratorExpression(tok2);
    if (iter1 && iter2 && !isSameExpression(true, false, iter1, iter2, mSettings->library, false, false)) {
        mismatchingContainerExpressionError(iter1, iter2);
        return true;
    }
    return false;
}

struct ArgIteratorInfo {
    const Token* tok;
    const Library::ArgumentChecks::IteratorInfo* info;
};

void CheckStl::mismatchingContainers()
{
    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%comp%|-")) {
                if (checkIteratorPair(tok->astOperand1(), tok->astOperand2()))
                    continue;
            }
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const ftok = tok;

            const std::vector<const Token *> args = getArguments(ftok);
            if (args.size() < 2)
                continue;

            // Group args together by container
            std::map<int, std::vector<ArgIteratorInfo>> containers;
            for (int argnr = 1; argnr <= args.size(); ++argnr) {
                const Library::ArgumentChecks::IteratorInfo *i = mSettings->library.getArgIteratorInfo(ftok, argnr);
                if (!i)
                    continue;
                const Token * const argTok = args[argnr - 1];
                containers[i->container].push_back({argTok, i});
            }

            // Lambda is used to escape the nested loops
            [&] {
                for (const auto& p : containers)
                {
                    const std::vector<ArgIteratorInfo>& cargs = p.second;
                    for (ArgIteratorInfo iter1 : cargs) {
                        for (ArgIteratorInfo iter2 : cargs) {
                            if (iter1.tok == iter2.tok)
                                continue;
                            if (iter1.info->first && iter2.info->last &&
                                isSameExpression(true, false, iter1.tok, iter2.tok, mSettings->library, false, false))
                                sameIteratorExpressionError(iter1.tok);
                            if (checkIteratorPair(iter1.tok, iter2.tok))
                                return;
                        }
                    }
                }
            }();
        }
    }
    for (const Variable *var : symbolDatabase->variableList()) {
        if (var && var->isStlStringType() && Token::Match(var->nameToken(), "%var% (") && Token::Match(var->nameToken()->tokAt(2), pattern2.c_str())) {
            if (var->nameToken()->strAt(2) != var->nameToken()->strAt(8)) {
                mismatchingContainersError(var->nameToken(), var->nameToken()->tokAt(2));
            }
        }
    }
}

void CheckStl::mismatchingContainerIterator()
{
    // Check if different containers are used in various calls of standard functions
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!astIsContainer(tok))
                continue;
            if (!Token::Match(tok, "%var% . %name% ( !!)"))
                continue;
            const Token * const ftok = tok->tokAt(2);
            const std::vector<const Token *> args = getArguments(ftok);

            const Library::Container * c = tok->valueType()->container;
            Library::Container::Action action = c->getAction(tok->strAt(2));
            const Token* iterTok = nullptr;
            if (action == Library::Container::Action::INSERT && args.size() == 2) {
                // Skip if iterator pair
                if (astIsIterator(args.back()))
                    continue;
                if (!astIsIterator(args.front()))
                    continue;
                iterTok = args.front();
            } else if (action == Library::Container::Action::ERASE) {
                if (!astIsIterator(args.front()))
                    continue;
                iterTok = args.front();
            } else {
                continue;
            }

            ValueFlow::Value val = getLifetimeObjValue(iterTok);
            if (!val.tokvalue)
                continue;
            if (val.lifetimeKind != ValueFlow::Value::LifetimeKind::Iterator)
                continue;
            for (const LifetimeToken& lt:getLifetimeTokens(tok)) {
                if (lt.inconclusive)
                    continue;
                const Token* contTok = lt.token;
                if (isSameExpression(true, false, contTok, val.tokvalue, mSettings->library, false, false))
                    continue;
                mismatchingContainerIteratorError(tok, iterTok);
            }

        }
    }
}

static bool isInvalidMethod(const Token * tok)
{
    if (Token::Match(tok->next(), ". assign|clear|swap"))
        return true;
    if (Token::Match(tok->next(), "%assign%"))
        return true;
    if (const Library::Container * c = tok->valueType()->container) {
        Library::Container::Action action = c->getAction(tok->strAt(2));
        if (c->unstableErase) {
            if (action == Library::Container::Action::ERASE)
                return true;
        }
        if (c->unstableInsert) {
            if (action == Library::Container::Action::RESIZE)
                return true;
            if (action == Library::Container::Action::CLEAR)
                return true;
            if (action == Library::Container::Action::PUSH)
                return true;
            if (action == Library::Container::Action::POP)
                return true;
            if (action == Library::Container::Action::INSERT)
                return true;
            if (action == Library::Container::Action::CHANGE)
                return true;
            if (action == Library::Container::Action::CHANGE_INTERNAL)
                return true;
            if (Token::Match(tok->next(), ". insert|emplace"))
                return true;
        }
    }
    return false;
}

struct InvalidContainerAnalyzer {
    struct Info {
        struct Reference {
            const Token* tok;
            ErrorPath errorPath;
        };
        std::unordered_map<int, Reference> expressions;
        ErrorPath errorPath;
        void add(const std::vector<Reference>& refs) {
            for (const Reference& r : refs) {
                add(r);
            }
        }
        void add(const Reference& r) {
            if (!r.tok)
                return;
            expressions.insert(std::make_pair(r.tok->exprId(), r));
        }

        std::vector<Reference> invalidTokens() const {
            std::vector<Reference> result;
            std::transform(expressions.begin(), expressions.end(), std::back_inserter(result), SelectMapValues{});
            return result;
        }
    };
    std::unordered_map<const Function*, Info> invalidMethods;

    std::vector<Info::Reference> invalidatesContainer(const Token* tok) const {
        std::vector<Info::Reference> result;
        if (Token::Match(tok, "%name% (")) {
            const Function* f = tok->function();
            if (!f)
                return result;
            ErrorPathItem epi = std::make_pair(tok, "Calling function " + tok->str());
            const bool dependsOnThis = exprDependsOnThis(tok->next());
            auto it = invalidMethods.find(f);
            if (it != invalidMethods.end()) {
                std::vector<Info::Reference> refs = it->second.invalidTokens();
                std::copy_if(refs.begin(), refs.end(), std::back_inserter(result), [&](const Info::Reference& r) {
                    const Variable* var = r.tok->variable();
                    if (!var)
                        return false;
                    if (dependsOnThis && !var->isLocal() && !var->isGlobal() && !var->isStatic())
                        return true;
                    if (!var->isArgument())
                        return false;
                    if (!var->isReference())
                        return false;
                    return true;
                });
                std::vector<const Token*> args = getArguments(tok);
                for (Info::Reference& r : result) {
                    r.errorPath.push_front(epi);
                    const Variable* var = r.tok->variable();
                    if (!var)
                        continue;
                    if (var->isArgument()) {
                        int n = getArgumentPos(var, f);
                        const Token* tok2 = nullptr;
                        if (n >= 0 && n < args.size())
                            tok2 = args[n];
                        r.tok = tok2;
                    }
                }
            }
        } else if (astIsContainer(tok)) {
            if (isInvalidMethod(tok)) {
                ErrorPath ep;
                ep.emplace_front(tok,
                                 "After calling '" + tok->strAt(2) +
                                 "', iterators or references to the container's data may be invalid .");
                result.push_back(Info::Reference{tok, ep});
            }
        }
        return result;
    }

    void analyze(const SymbolDatabase* symboldatabase) {
        for (const Scope* scope : symboldatabase->functionScopes) {
            const Function* f = scope->function;
            if (!f)
                continue;
            for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
                if (Token::Match(tok, "if|while|for|goto|return"))
                    break;
                std::vector<Info::Reference> c = invalidatesContainer(tok);
                if (c.empty())
                    continue;
                invalidMethods[f].add(c);
            }
        }
    }
};

static bool isVariableDecl(const Token* tok)
{
    if (!tok)
        return false;
    const Variable* var = tok->variable();
    if (!var)
        return false;
    if (var->nameToken() == tok)
        return true;
    if (Token::Match(var->declEndToken(), "; %var%") && var->declEndToken()->next() == tok)
        return true;
    return false;
}

void CheckStl::invalidContainer()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    const Library& library = mSettings->library;
    InvalidContainerAnalyzer analyzer;
    analyzer.analyze(symbolDatabase);
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            for (const InvalidContainerAnalyzer::Info::Reference& r : analyzer.invalidatesContainer(tok)) {
                if (!astIsContainer(r.tok))
                    continue;
                std::set<nonneg int> skipVarIds;
                // Skip if the variable is assigned to
                const Token* assignExpr = tok;
                while (assignExpr->astParent()) {
                    bool isRHS = astIsRHS(assignExpr);
                    assignExpr = assignExpr->astParent();
                    if (Token::Match(assignExpr, "%assign%")) {
                        if (!isRHS)
                            assignExpr = nullptr;
                        break;
                    }
                }
                if (Token::Match(assignExpr, "%assign%") && Token::Match(assignExpr->astOperand1(), "%var%"))
                    skipVarIds.insert(assignExpr->astOperand1()->varId());
                const Token* endToken = nextAfterAstRightmostLeaf(tok->next()->astParent());
                if (!endToken)
                    endToken = tok->next();
                const ValueFlow::Value* v = nullptr;
                ErrorPath errorPath;
                PathAnalysis::Info info =
                PathAnalysis{endToken, library} .forwardFind([&](const PathAnalysis::Info& info) {
                    if (!info.tok->variable())
                        return false;
                    if (info.tok->varId() == 0)
                        return false;
                    if (skipVarIds.count(info.tok->varId()) > 0)
                        return false;
                    // if (Token::simpleMatch(info.tok->next(), "."))
                    // return false;
                    if (Token::Match(info.tok->astParent(), "%assign%") && astIsLHS(info.tok))
                        skipVarIds.insert(info.tok->varId());
                    if (info.tok->variable()->isReference() && !isVariableDecl(info.tok) &&
                        reaches(info.tok->variable()->nameToken(), tok, library, nullptr)) {

                        ErrorPath ep;
                        bool addressOf = false;
                        const Variable* var = getLifetimeVariable(info.tok, ep, &addressOf);
                        // Check the reference is created before the change
                        if (var && var->declarationId() == r.tok->varId() && !addressOf) {
                            // An argument always reaches
                            if (var->isArgument() ||
                                (!var->isReference() && !var->isRValueReference() && !isVariableDecl(tok) &&
                                 reaches(var->nameToken(), tok, library, &ep))) {
                                errorPath = ep;
                                return true;
                            }
                        }
                    }
                    for (const ValueFlow::Value& val : info.tok->values()) {
                        if (!val.isLocalLifetimeValue())
                            continue;
                        if (val.lifetimeKind == ValueFlow::Value::LifetimeKind::Address)
                            continue;
                        if (val.lifetimeKind == ValueFlow::Value::LifetimeKind::SubObject)
                            continue;
                        if (!val.tokvalue->variable())
                            continue;
                        if (val.tokvalue->varId() != r.tok->varId())
                            continue;
                        ErrorPath ep;
                        // Check the iterator is created before the change
                        if (val.tokvalue != tok && reaches(val.tokvalue, tok, library, &ep)) {
                            v = &val;
                            errorPath = ep;
                            return true;
                        }
                    }
                    return false;
                });
                if (!info.tok)
                    continue;
                errorPath.insert(errorPath.end(), info.errorPath.begin(), info.errorPath.end());
                errorPath.insert(errorPath.end(), r.errorPath.begin(), r.errorPath.end());
                if (v) {
                    invalidContainerError(info.tok, r.tok, v, errorPath);
                } else {
                    invalidContainerReferenceError(info.tok, r.tok, errorPath);
                }
            }
        }
    }
}

static const Token* getLoopContainer(const Token* tok)
{
    if (!Token::simpleMatch(tok, "for ("))
        return nullptr;
    const Token * sepTok = tok->next()->astOperand2();
    if (!Token::simpleMatch(sepTok, ":"))
        return nullptr;
    return sepTok->astOperand2();
}

void CheckStl::invalidContainerLoop()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            const Token* contTok = getLoopContainer(tok);
            if (!contTok)
                continue;
            const Token * blockStart = tok->next()->link()->next();
            const Token * blockEnd = blockStart->link();
            if (!Token::Match(contTok, "%var%"))
                continue;
            if (contTok->varId() == 0)
                continue;
            if (!astIsContainer(contTok))
                continue;
            nonneg int varid = contTok->varId();
            for (const Token* tok2 = blockStart; tok2 != blockEnd; tok2 = tok2->next()) {
                if (tok2->varId() != varid)
                    continue;
                if (!Token::Match(tok2->next(), ". %name% ("))
                    continue;
                if (!isInvalidMethod(tok2))
                    continue;
                const Scope* s = tok2->scope();
                if (!s)
                    continue;
                if (isReturnScope(s->bodyEnd, &mSettings->library))
                    continue;
                invalidContainerLoopError(tok2, tok);
                break;
            }
        }
    }
}

void CheckStl::invalidContainerLoopError(const Token *tok, const Token * loopTok)
{
    ErrorPath errorPath;
    const std::string method = tok ? tok->strAt(2) : "erase";
    errorPath.emplace_back(loopTok, "Iterating container here.");

    const std::string msg = "Calling '" + method + "' while iterating the container is invalid.";
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "invalidContainerLoop", msg, CWE664, false);
}

void CheckStl::invalidContainerError(const Token *tok, const Token * contTok, const ValueFlow::Value *val, ErrorPath errorPath)
{
    const bool inconclusive = val ? val->isInconclusive() : false;
    if (val)
        errorPath.insert(errorPath.begin(), val->errorPath.begin(), val->errorPath.end());
    std::string msg = "Using " + lifetimeMessage(tok, val, errorPath);
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "invalidContainer", msg + " that may be invalid.", CWE664, inconclusive);
}

void CheckStl::invalidContainerReferenceError(const Token* tok, const Token* contTok, ErrorPath errorPath)
{
    std::string name = contTok ? contTok->expressionString() : "x";
    std::string msg = "Reference to " + name;
    errorPath.emplace_back(tok, "");
    reportError(errorPath, Severity::error, "invalidContainerReference", msg + " that may be invalid.", CWE664, false);
}

void CheckStl::stlOutOfBounds()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Scan through all scopes..
    for (const Scope &scope : symbolDatabase->scopeList) {
        const Token* tok = scope.classDef;
        // only interested in conditions
        if ((!scope.isLoopScope() && scope.type != Scope::eIf) || !tok)
            continue;

        const Token *condition = nullptr;
        if (scope.type == Scope::eFor) {
            if (Token::simpleMatch(tok->next()->astOperand2(), ";") && Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
                condition = tok->next()->astOperand2()->astOperand2()->astOperand1();
        } else if (Token::simpleMatch(tok, "do {") && Token::simpleMatch(tok->linkAt(1), "} while ("))
            condition = tok->linkAt(1)->tokAt(2)->astOperand2();
        else
            condition = tok->next()->astOperand2();

        if (!condition)
            continue;

        std::vector<const Token *> conds;

        visitAstNodes(condition,
        [&](const Token *cond) {
            if (Token::Match(cond, "%oror%|&&"))
                return ChildrenToVisit::op1_and_op2;
            if (cond->isComparisonOp())
                conds.emplace_back(cond);
            return ChildrenToVisit::none;
        });

        for (const Token *cond : conds) {
            const Token *vartok;
            const Token *containerToken;
            // check in the ast that cond is of the form "%var% <= %var% . %name% ( )"
            if (cond->str() == "<=" && Token::Match(cond->astOperand1(), "%var%") &&
                cond->astOperand2()->str() == "(" && cond->astOperand2()->astOperand1()->str() == "." &&
                Token::Match(cond->astOperand2()->astOperand1()->astOperand1(), "%var%") &&
                Token::Match(cond->astOperand2()->astOperand1()->astOperand2(), "%name%")) {
                vartok = cond->astOperand1();
                containerToken = cond->next();
            } else {
                continue;
            }

            // Is it a array like container?
            const Library::Container* container = containerToken->valueType() ? containerToken->valueType()->container : nullptr;
            if (!container)
                continue;
            if (container->getYield(containerToken->strAt(2)) != Library::Container::Yield::SIZE)
                continue;

            // variable id for loop variable.
            const int numId = vartok->varId();

            // variable id for the container variable
            const int declarationId = containerToken->varId();
            const std::string &containerName = containerToken->str();

            for (const Token *tok3 = scope.bodyStart; tok3 && tok3 != scope.bodyEnd; tok3 = tok3->next()) {
                if (tok3->varId() == declarationId) {
                    tok3 = tok3->next();
                    if (Token::Match(tok3, ". %name% ( )")) {
                        if (container->getYield(tok3->strAt(1)) == Library::Container::Yield::SIZE)
                            break;
                    } else if (container->arrayLike_indexOp && Token::Match(tok3, "[ %varid% ]", numId))
                        stlOutOfBoundsError(tok3, tok3->strAt(1), containerName, false);
                    else if (Token::Match(tok3, ". %name% ( %varid% )", numId)) {
                        const Library::Container::Yield yield = container->getYield(tok3->strAt(1));
                        if (yield == Library::Container::Yield::AT_INDEX)
                            stlOutOfBoundsError(tok3, tok3->strAt(3), containerName, true);
                    }
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
    for (const Scope * scope : symbolDatabase->functionScopes) {
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

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type == Scope::eFor && Token::simpleMatch(scope.classDef, "for (")) {
            const Token *tok = scope.classDef->linkAt(1);
            if (!Token::Match(tok->tokAt(-3), "; ++| %var% ++| ) {"))
                continue;
            tok = tok->previous();
            if (!tok->isName())
                tok = tok->previous();
            eraseCheckLoopVar(scope, tok->variable());
        } else if (scope.type == Scope::eWhile && Token::Match(scope.classDef, "while ( %var% !=")) {
            eraseCheckLoopVar(scope, scope.classDef->tokAt(2)->variable());
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
        // Vector erases are handled by invalidContainer check
        if (isVector(tok->tokAt(-3)))
            continue;
        if (Token::simpleMatch(tok->astParent(), "="))
            continue;
        // Iterator is invalid..
        int indentlevel = 0U;
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

    for (const Scope &scope : symbolDatabase->scopeList) {
        if ((scope.type != Scope::eIf && scope.type != Scope::eWhile) || !scope.classDef)
            continue;

        const Token *conditionStart = scope.classDef->next();
        if (conditionStart && Token::simpleMatch(conditionStart->astOperand2(), ";"))
            conditionStart = conditionStart->astOperand2();

        for (const Token *tok = conditionStart; tok->str() != "{"; tok = tok->next()) {
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

            if (container && container->getAction(funcTok->str()) == Library::Container::Action::FIND) {
                if (if_findCompare(funcTok->next()))
                    continue;

                if (printWarning && container->getYield(funcTok->str()) == Library::Container::Yield::ITERATOR)
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
    if (str && mSettings->standards.cpp >= Standards::CPP20)
        reportError(tok, Severity::performance, "stlIfStrFind",
                    "Inefficient usage of string::find() in condition; string::starts_with() could be faster.\n"
                    "Either inefficient or wrong usage of string::find(). string::starts_with() will be faster if "
                    "string::find's result is compared with 0, because it will not scan the whole "
                    "string. If your intention is to check that there are no findings in the string, "
                    "you should compare with std::string::npos.", CWE597, false);
    if (!str)
        reportError(tok, Severity::warning, "stlIfFind", "Suspicious condition. The result of find() is an iterator, but it is not properly checked.", CWE398, false);
}

static std::pair<const Token *, const Token *> isMapFind(const Token *tok)
{
    if (!Token::simpleMatch(tok, "("))
        return {};
    if (!Token::simpleMatch(tok->astOperand1(), "."))
        return {};
    if (!astIsContainer(tok->astOperand1()->astOperand1()))
        return {};
    const Token * contTok = tok->astOperand1()->astOperand1();
    const Library::Container * container = contTok->valueType()->container;
    if (!container)
        return {};
    if (!container->stdAssociativeLike)
        return {};
    if (!Token::Match(tok->astOperand1(), ". find|count ("))
        return {};
    if (!tok->astOperand2())
        return {};
    return {contTok, tok->astOperand2()};
}

static const Token *skipLocalVars(const Token *tok)
{
    if (!tok)
        return tok;
    if (Token::simpleMatch(tok, "{"))
        return skipLocalVars(tok->next());
    const Scope *scope = tok->scope();

    const Token *top = tok->astTop();
    if (!top) {
        const Token *semi = Token::findsimplematch(tok, ";");
        if (!semi)
            return tok;
        if (!Token::Match(semi->previous(), "%var% ;"))
            return tok;
        const Token *varTok = semi->previous();
        const Variable *var = varTok->variable();
        if (!var)
            return tok;
        if (var->nameToken() != varTok)
            return tok;
        return skipLocalVars(semi->next());
    }
    if (Token::Match(top, "%assign%")) {
        const Token *varTok = top->astOperand1();
        if (!Token::Match(varTok, "%var%"))
            return tok;
        const Variable *var = varTok->variable();
        if (!var)
            return tok;
        if (var->scope() != scope)
            return tok;
        const Token *endTok = nextAfterAstRightmostLeaf(top);
        if (!endTok)
            return tok;
        return skipLocalVars(endTok->next());
    }
    return tok;
}

static const Token *findInsertValue(const Token *tok, const Token *containerTok, const Token *keyTok, const Library &library)
{
    const Token *startTok = skipLocalVars(tok);
    const Token *top = startTok->astTop();

    const Token *icontainerTok = nullptr;
    const Token *ikeyTok = nullptr;
    const Token *ivalueTok = nullptr;
    if (Token::simpleMatch(top, "=") && Token::simpleMatch(top->astOperand1(), "[")) {
        icontainerTok = top->astOperand1()->astOperand1();
        ikeyTok = top->astOperand1()->astOperand2();
        ivalueTok = top->astOperand2();
    }
    if (Token::simpleMatch(top, "(") && Token::Match(top->astOperand1(), ". insert|emplace (") && !astIsIterator(top->astOperand1()->tokAt(2))) {
        icontainerTok = top->astOperand1()->astOperand1();
        const Token *itok = top->astOperand1()->tokAt(2)->astOperand2();
        if (Token::simpleMatch(itok, ",")) {
            ikeyTok = itok->astOperand1();
            ivalueTok = itok->astOperand2();
        } else {
            ikeyTok = itok;
        }
    }
    if (!ikeyTok || !icontainerTok)
        return nullptr;
    if (isSameExpression(true, true, containerTok, icontainerTok, library, true, false) &&
        isSameExpression(true, true, keyTok, ikeyTok, library, true, true)) {
        if (ivalueTok)
            return ivalueTok;
        else
            return ikeyTok;
    }
    return nullptr;
}

void CheckStl::checkFindInsert()
{
    if (!mSettings->isEnabled(Settings::PERFORMANCE))
        return;

    const SymbolDatabase *const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "if ("))
                continue;
            if (!Token::simpleMatch(tok->next()->link(), ") {"))
                continue;
            if (!Token::Match(tok->next()->astOperand2(), "%comp%"))
                continue;
            const Token *condTok = tok->next()->astOperand2();
            const Token *containerTok;
            const Token *keyTok;
            std::tie(containerTok, keyTok) = isMapFind(condTok->astOperand1());
            if (!containerTok)
                continue;
            // In < C++17 we only warn for small simple types
            if (mSettings->standards.cpp < Standards::CPP17 && !(keyTok && keyTok->valueType() && (keyTok->valueType()->isIntegral() || keyTok->valueType()->pointer > 0)))
                continue;

            const Token *thenTok = tok->next()->link()->next();
            const Token *valueTok = findInsertValue(thenTok, containerTok, keyTok, mSettings->library);
            if (!valueTok)
                continue;

            if (Token::simpleMatch(thenTok->link(), "} else {")) {
                const Token *valueTok2 =
                    findInsertValue(thenTok->link()->tokAt(2), containerTok, keyTok, mSettings->library);
                if (!valueTok2)
                    continue;
                if (isSameExpression(true, true, valueTok, valueTok2, mSettings->library, true, true)) {
                    checkFindInsertError(valueTok);
                }
            } else {
                checkFindInsertError(valueTok);
            }
        }
    }
}

void CheckStl::checkFindInsertError(const Token *tok)
{
    reportError(
        tok, Severity::performance, "stlFindInsert", "Searching before insertion is not necessary.", CWE398, false);
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

    if (mSettings->standards.cpp >= Standards::CPP11)
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
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

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != Scope::eIf)
            continue;

        const Token* tok = scope.classDef->tokAt(2);
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

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != Scope::eFor || !scope.classDef)
            continue;

        for (const Token *tok2 = scope.classDef->tokAt(2); tok2 != scope.bodyStart; tok2 = tok2->next()) {
            if (tok2->str() == ";")
                break;

            if (!Token::Match(tok2, "%var% = %name% . begin|rbegin|cbegin|crbegin ( ) ; %name% != %name% . end|rend|cend|crend ( ) ; ++| %name% ++| ) {"))
                continue;

            // same container
            if (tok2->strAt(2) != tok2->strAt(10))
                break;

            const int iteratorId(tok2->varId());

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
            for (const Token *tok3 = scope.bodyStart; tok3 != scope.bodyEnd; tok3 = tok3->next()) {
                if (tok3->varId() == iteratorId) {
                    if (Token::Match(tok3, "%varid% = %name% . insert ( ++| %varid% ++| ,", iteratorId)) {
                        // skip insertion..
                        tok3 = tok3->linkAt(6);
                        if (!tok3)
                            break;
                    } else if (Token::simpleMatch(tok3->astParent(), "++"))
                            incrementToken = tok3;
                    else if (Token::simpleMatch(tok3->astParent(), "+")) {
                        if (Token::Match(tok3->astSibling(), "%num%")) {
                            const Token* tokenGrandParent = tok3->astParent()->astParent();
                            if (Token::Match(tokenGrandParent, "==|!="))
                                break;
                        }
                    } else if (Token::Match(tok3->astParent(), "==|!="))
                        incrementToken = nullptr;
                } else if (tok3->str() == "break" || tok3->str() == "return")
                    incrementToken = nullptr;
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
    std::multimap<std::string, int> c_strFuncParam;
    if (printPerformance) {
        for (const Scope &scope : symbolDatabase->scopeList) {
            for (const Function &func : scope.functionList) {
                if (c_strFuncParam.erase(func.tokenDef->str()) != 0) { // Check if function with this name was already found
                    c_strFuncParam.insert(std::make_pair(func.tokenDef->str(), 0)); // Disable, because there are overloads. TODO: Handle overloads
                    continue;
                }

                int numpar = 0;
                c_strFuncParam.insert(std::make_pair(func.tokenDef->str(), numpar)); // Insert function as dummy, to indicate that there is at least one function with that name
                for (const Variable &var : func.argumentList) {
                    numpar++;
                    if (var.isStlStringType() && (!var.isReference() || var.isConst()))
                        c_strFuncParam.insert(std::make_pair(func.tokenDef->str(), numpar));
                }
            }
        }
    }

    // Try to detect common problems when using string::c_str()
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != Scope::eFunction || !scope.function)
            continue;

        enum {charPtr, stdString, stdStringConstRef, Other} returnType = Other;
        if (Token::Match(scope.function->tokenDef->tokAt(-2), "char|wchar_t *"))
            returnType = charPtr;
        else if (Token::Match(scope.function->tokenDef->tokAt(-5), "const std :: string|wstring &"))
            returnType = stdStringConstRef;
        else if (Token::Match(scope.function->tokenDef->tokAt(-3), "std :: string|wstring !!&"))
            returnType = stdString;

        for (const Token *tok = scope.bodyStart; tok && tok != scope.bodyEnd; tok = tok->next()) {
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
                       !Token::Match(tok->previous(), "::|.") && tok->varId() == 0 && tok->str() != scope.className) { // calling function. TODO: Add support for member functions
                const std::pair<std::multimap<std::string, int>::const_iterator, std::multimap<std::string, int>::const_iterator> range = c_strFuncParam.equal_range(tok->str());
                for (std::multimap<std::string, int>::const_iterator i = range.first; i != range.second; ++i) {
                    if (i->second == 0)
                        continue;

                    const Token* tok2 = tok->tokAt(2);
                    int j;
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

void CheckStl::string_c_strParam(const Token* tok, nonneg int number)
{
    std::ostringstream oss;
    oss << "Passing the result of c_str() to a function that takes std::string as argument no. " << number << " is slow and redundant.\n"
        "The conversion from const char* as returned by c_str() to std::string creates an unnecessary string copy. Solve that by directly passing the string.";
    reportError(tok, Severity::performance, "stlcstrParam", oss.str(), CWE704, false);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

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
    for (const Scope * scope : symbolDatabase->functionScopes) {
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
    for (const Scope &scope : mTokenizer->getSymbolDatabase()->scopeList) {
        if (!(scope.type == Scope::eIf || scope.isLoopScope()))
            continue;

        const Token* const tok = scope.classDef;
        const Token* startOfCondition = tok->next();
        if (scope.type == Scope::eDo)
            startOfCondition = startOfCondition->link()->tokAt(2);
        if (!startOfCondition) // ticket #6626 invalid code
            continue;
        const Token* endOfCondition = startOfCondition->link();
        if (!endOfCondition)
            continue;

        // For "for" loops, only search between the two semicolons
        if (scope.type == Scope::eFor) {
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
        const int iteratorVarId = validityCheckTok->next()->varId();

        // If the iterator dereference is to the left of the check for
        // the iterator's validity, report an error.
        const Token* const dereferenceTok =
            Token::findmatch(startOfCondition, "* %varid%", validityCheckTok, iteratorVarId);
        if (dereferenceTok)
            dereferenceInvalidIteratorError(dereferenceTok, dereferenceTok->strAt(1));
    }
}


void CheckStl::checkDereferenceInvalidIterator2()
{
    const bool printInconclusive = (mSettings->inconclusive);

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof|decltype|typeid|typeof (")) {
            tok = tok->next()->link();
            continue;
        }

        std::vector<ValueFlow::Value> contValues;
        std::copy_if(tok->values().begin(), tok->values().end(), std::back_inserter(contValues), [&](const ValueFlow::Value& value) {
            if (value.isImpossible())
                return false;
            if (!printInconclusive && value.isInconclusive())
                return false;
            return value.isContainerSizeValue();
        });


        // Can iterator point to END or before START?
        for (const ValueFlow::Value& value:tok->values()) {
            if (value.isImpossible())
                continue;
            if (!printInconclusive && value.isInconclusive())
                continue;
            if (!value.isIteratorValue())
                continue;
            const bool isInvalidIterator = (value.isIteratorEndValue() && value.intvalue >= 0) || (value.isIteratorStartValue() && value.intvalue < 0);
            const ValueFlow::Value* cValue = nullptr;
            if (!isInvalidIterator) {
                auto it = std::find_if(contValues.begin(), contValues.end(), [&](const ValueFlow::Value& c) {
                    if (value.isIteratorStartValue() && value.intvalue >= c.intvalue)
                        return true;
                    if (value.isIteratorEndValue() && -value.intvalue > c.intvalue)
                        return true;
                    return false;
                });
                if (it == contValues.end())
                    continue;
                cValue = &*it;
            }
            bool inconclusive = false;
            bool unknown = false;
            if (!CheckNullPointer::isPointerDeRef(tok, unknown, mSettings)) {
                if (!unknown)
                    continue;
                inconclusive = true;
            }
            if (cValue) {
                const ValueFlow::Value& lValue = getLifetimeObjValue(tok, true);
                outOfBoundsError(tok, lValue.tokvalue->expressionString(), cValue, tok->expressionString(), &value);
            } else {
                dereferenceInvalidIteratorError(tok, &value, inconclusive);
            }
        }
    }
}

void CheckStl::dereferenceInvalidIteratorError(const Token* tok, const ValueFlow::Value *value, bool inconclusive)
{
    const std::string& varname = tok ? tok->expressionString() : "var";
    const std::string errmsgcond("$symbol:" + varname + '\n' + ValueFlow::eitherTheConditionIsRedundant(value ? value->condition : nullptr) + " or there is possible dereference of an invalid iterator: $symbol.");
    if (!tok || !value) {
        reportError(tok, Severity::error, "derefInvalidIterator", "Dereference of an invalid iterator", CWE825, false);
        reportError(tok, Severity::warning, "derefInvalidIteratorRedundantCheck", errmsgcond, CWE825, false);
        return;
    }
    if (!mSettings->isEnabled(value, inconclusive))
        return;

    const ErrorPath errorPath = getErrorPath(tok, value, "Dereference of an invalid iterator");

    if (value->condition) {
        reportError(errorPath, Severity::warning, "derefInvalidIteratorRedundantCheck", errmsgcond, CWE825, inconclusive || value->isInconclusive());
    } else {
        std::string errmsg;
        errmsg = std::string(value->isKnown() ? "Dereference" : "Possible dereference") + " of an invalid iterator";
        if (!varname.empty())
            errmsg = "$symbol:" + varname + '\n' + errmsg + ": $symbol";

        reportError(errorPath,
                    value->isKnown() ? Severity::error : Severity::warning,
                    "derefInvalidIterator",
                    errmsg,
                    CWE825, inconclusive || value->isInconclusive());
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


void CheckStl::readingEmptyStlContainer2()
{
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            if (!tok->isName() || !tok->valueType())
                continue;
            const Library::Container *container = tok->valueType()->container;
            if (!container)
                continue;
            const ValueFlow::Value *value = tok->getContainerSizeValue(0);
            if (!value)
                continue;
            if (value->isInconclusive() && !mSettings->inconclusive)
                continue;
            if (!value->errorSeverity() && !mSettings->isEnabled(Settings::WARNING))
                continue;
            if (Token::Match(tok, "%name% . %name% (")) {
                if (container->getYield(tok->strAt(2)) == Library::Container::Yield::ITEM)
                    readingEmptyStlContainerError(tok,value);
            }
        }
    }
}

void CheckStl::readingEmptyStlContainerError(const Token *tok, const ValueFlow::Value *value)
{
    const std::string varname = tok ? tok->str() : std::string("var");

    std::string errmsg;
    if (value && value->condition)
        errmsg = "Reading from container '$symbol'. " + ValueFlow::eitherTheConditionIsRedundant(value->condition) + " or '$symbol' can be empty.";
    else
        errmsg = "Reading from empty STL container '$symbol'";

    const ErrorPath errorPath = getErrorPath(tok, value, "Reading from empty container");

    reportError(errorPath, value ? (value->errorSeverity() ? Severity::error : Severity::warning) : Severity::style, "reademptycontainer", "$symbol:" + varname +"\n" + errmsg, CWE398, !value);
}

void CheckStl::useStlAlgorithmError(const Token *tok, const std::string &algoName)
{
    reportError(tok, Severity::style, "useStlAlgorithm",
                "Consider using " + algoName + " algorithm instead of a raw loop.", CWE398, false);
}

static bool isEarlyExit(const Token *start)
{
    if (start->str() != "{")
        return false;
    const Token *endToken = start->link();
    const Token *tok = Token::findmatch(start, "return|throw|break", endToken);
    if (!tok)
        return false;
    const Token *endStatement = Token::findsimplematch(tok, "; }", endToken);
    if (!endStatement)
        return false;
    if (endStatement->next() != endToken)
        return false;
    return true;
}

static const Token *singleStatement(const Token *start)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    const Token *endStatement = Token::findsimplematch(start->next(), ";");
    if (!Token::simpleMatch(endStatement, "; }"))
        return nullptr;
    if (endStatement->next() != endToken)
        return nullptr;
    return endStatement;
}

static const Token *singleAssignInScope(const Token *start, nonneg int varid, bool &input)
{
    const Token *endStatement = singleStatement(start);
    if (!endStatement)
        return nullptr;
    if (!Token::Match(start->next(), "%var% %assign%"))
        return nullptr;
    const Token *assignTok = start->tokAt(2);
    if (isVariableChanged(assignTok->next(), endStatement, assignTok->astOperand1()->varId(), false, nullptr, true))
        return nullptr;
    if (isVariableChanged(assignTok->next(), endStatement, varid, false, nullptr, true))
        return nullptr;
    input = Token::findmatch(assignTok->next(), "%varid%", endStatement, varid) || !Token::Match(start->next(), "%var% =");
    return assignTok;
}

static const Token *singleMemberCallInScope(const Token *start, nonneg int varid, bool &input)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    if (!Token::Match(start->next(), "%var% . %name% ("))
        return nullptr;
    if (!Token::simpleMatch(start->linkAt(4), ") ; }"))
        return nullptr;
    const Token *endStatement = start->linkAt(4)->next();
    if (endStatement->next() != endToken)
        return nullptr;

    const Token *dotTok = start->tokAt(2);
    if (!Token::findmatch(dotTok->tokAt(2), "%varid%", endStatement, varid))
        return nullptr;
    input = Token::Match(start->next(), "%var% . %name% ( %varid% )", varid);
    if (isVariableChanged(dotTok->next(), endStatement, dotTok->astOperand1()->varId(), false, nullptr, true))
        return nullptr;
    return dotTok;
}

static const Token *singleIncrementInScope(const Token *start, nonneg int varid, bool &input)
{
    if (start->str() != "{")
        return nullptr;
    const Token *varTok = nullptr;
    if (Token::Match(start->next(), "++ %var% ; }"))
        varTok = start->tokAt(2);
    else if (Token::Match(start->next(), "%var% ++ ; }"))
        varTok = start->tokAt(1);
    if (!varTok)
        return nullptr;
    input = varTok->varId() == varid;
    return varTok;
}

static const Token *singleConditionalInScope(const Token *start, nonneg int varid)
{
    if (start->str() != "{")
        return nullptr;
    const Token *endToken = start->link();
    if (!Token::simpleMatch(start->next(), "if ("))
        return nullptr;
    if (!Token::simpleMatch(start->linkAt(2), ") {"))
        return nullptr;
    const Token *bodyTok = start->linkAt(2)->next();
    const Token *endBodyTok = bodyTok->link();
    if (!Token::simpleMatch(endBodyTok, "} }"))
        return nullptr;
    if (endBodyTok->next() != endToken)
        return nullptr;
    if (!Token::findmatch(start, "%varid%", bodyTok, varid))
        return nullptr;
    if (isVariableChanged(start, bodyTok, varid, false, nullptr, true))
        return nullptr;
    return bodyTok;
}

static bool addByOne(const Token *tok, nonneg int varid)
{
    if (Token::Match(tok, "+= %any% ;") &&
        tok->tokAt(1)->hasKnownIntValue() &&
        tok->tokAt(1)->getValue(1)) {
        return true;
    }
    if (Token::Match(tok, "= %varid% + %any% ;", varid) &&
        tok->tokAt(3)->hasKnownIntValue() &&
        tok->tokAt(3)->getValue(1)) {
        return true;
    }
    return false;
}

static bool accumulateBoolLiteral(const Token *tok, nonneg int varid)
{
    if (Token::Match(tok, "%assign% %bool% ;") &&
        tok->tokAt(1)->hasKnownIntValue()) {
        return true;
    }
    if (Token::Match(tok, "= %varid% %oror%|%or%|&&|& %bool% ;", varid) &&
        tok->tokAt(3)->hasKnownIntValue()) {
        return true;
    }
    return false;
}

static bool accumulateBool(const Token *tok, nonneg int varid)
{
    // Missing %oreq% so we have to check both manually
    if (Token::simpleMatch(tok, "&=") || Token::simpleMatch(tok, "|=")) {
        return true;
    }
    if (Token::Match(tok, "= %varid% %oror%|%or%|&&|&", varid)) {
        return true;
    }
    return false;
}

static bool hasVarIds(const Token *tok, nonneg int var1, nonneg int var2)
{
    if (tok->astOperand1()->varId() == tok->astOperand2()->varId())
        return false;
    if (tok->astOperand1()->varId() == var1 || tok->astOperand1()->varId() == var2) {
        if (tok->astOperand2()->varId() == var1 || tok->astOperand2()->varId() == var2) {
            return true;
        }
    }
    return false;
}

static std::string flipMinMax(const std::string &algo)
{
    if (algo == "std::max_element")
        return "std::min_element";
    if (algo == "std::min_element")
        return "std::max_element";
    return algo;
}

static std::string minmaxCompare(const Token *condTok, nonneg int loopVar, nonneg int assignVar, bool invert = false)
{
    if (!Token::Match(condTok, "<|<=|>=|>"))
        return "std::accumulate";
    if (!hasVarIds(condTok, loopVar, assignVar))
        return "std::accumulate";
    std::string algo = "std::max_element";
    if (Token::Match(condTok, "<|<="))
        algo = "std::min_element";
    if (condTok->astOperand1()->varId() == assignVar)
        algo = flipMinMax(algo);
    if (invert)
        algo = flipMinMax(algo);
    return algo;
}

void CheckStl::useStlAlgorithm()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            // Parse range-based for loop
            if (!Token::simpleMatch(tok, "for ("))
                continue;
            if (!Token::simpleMatch(tok->next()->link(), ") {"))
                continue;
            const Token *bodyTok = tok->next()->link()->next();
            const Token *splitTok = tok->next()->astOperand2();
            if (!Token::simpleMatch(splitTok, ":"))
                continue;
            const Token *loopVar = splitTok->previous();
            if (!Token::Match(loopVar, "%var%"))
                continue;

            // Check for single assignment
            bool useLoopVarInAssign;
            const Token *assignTok = singleAssignInScope(bodyTok, loopVar->varId(), useLoopVarInAssign);
            if (assignTok) {
                int assignVarId = assignTok->astOperand1()->varId();
                std::string algo;
                if (assignVarId == loopVar->varId()) {
                    if (useLoopVarInAssign)
                        algo = "std::transform";
                    else if (Token::Match(assignTok->next(), "%var%|%bool%|%num%|%char% ;"))
                        algo = "std::fill";
                    else if (Token::Match(assignTok->next(), "%name% ( )"))
                        algo = "std::generate";
                    else
                        algo = "std::fill or std::generate";
                } else {
                    if (addByOne(assignTok, assignVarId))
                        algo = "std::distance";
                    else if (accumulateBool(assignTok, assignVarId))
                        algo = "std::any_of, std::all_of, std::none_of, or std::accumulate";
                    else if (Token::Match(assignTok, "= %var% <|<=|>=|> %var% ? %var% : %var%") && hasVarIds(assignTok->tokAt(6), loopVar->varId(), assignVarId))
                        algo = minmaxCompare(assignTok->tokAt(2), loopVar->varId(), assignVarId, assignTok->tokAt(5)->varId() == assignVarId);
                    else
                        algo = "std::accumulate";
                }
                useStlAlgorithmError(assignTok, algo);
                continue;
            }
            // Check for container calls
            bool useLoopVarInMemCall;
            const Token *memberAccessTok = singleMemberCallInScope(bodyTok, loopVar->varId(), useLoopVarInMemCall);
            if (memberAccessTok) {
                const Token *memberCallTok = memberAccessTok->astOperand2();
                const int contVarId = memberAccessTok->astOperand1()->varId();
                if (contVarId == loopVar->varId())
                    continue;
                if (memberCallTok->str() == "push_back" ||
                    memberCallTok->str() == "push_front" ||
                    memberCallTok->str() == "emplace_back") {
                    std::string algo;
                    if (useLoopVarInMemCall)
                        algo = "std::copy";
                    else
                        algo = "std::transform";
                    useStlAlgorithmError(memberCallTok, algo);
                }
                continue;
            }

            // Check for increment in loop
            bool useLoopVarInIncrement;
            const Token *incrementTok = singleIncrementInScope(bodyTok, loopVar->varId(), useLoopVarInIncrement);
            if (incrementTok) {
                std::string algo;
                if (useLoopVarInIncrement)
                    algo = "std::transform";
                else
                    algo = "std::distance";
                useStlAlgorithmError(incrementTok, algo);
                continue;
            }

            // Check for conditionals
            const Token *condBodyTok = singleConditionalInScope(bodyTok, loopVar->varId());
            if (condBodyTok) {
                // Check for single assign
                assignTok = singleAssignInScope(condBodyTok, loopVar->varId(), useLoopVarInAssign);
                if (assignTok) {
                    const int assignVarId = assignTok->astOperand1()->varId();
                    std::string algo;
                    if (assignVarId == loopVar->varId()) {
                        if (useLoopVarInAssign)
                            algo = "std::transform";
                        else
                            algo = "std::replace_if";
                    } else {
                        if (addByOne(assignTok, assignVarId))
                            algo = "std::count_if";
                        else if (accumulateBoolLiteral(assignTok, assignVarId))
                            algo = "std::any_of, std::all_of, std::none_of, or std::accumulate";
                        else
                            algo = "std::accumulate";
                    }
                    useStlAlgorithmError(assignTok, algo);
                    continue;
                }

                // Check for container call
                memberAccessTok = singleMemberCallInScope(condBodyTok, loopVar->varId(), useLoopVarInMemCall);
                if (memberAccessTok) {
                    const Token *memberCallTok = memberAccessTok->astOperand2();
                    const int contVarId = memberAccessTok->astOperand1()->varId();
                    if (contVarId == loopVar->varId())
                        continue;
                    if (memberCallTok->str() == "push_back" ||
                        memberCallTok->str() == "push_front" ||
                        memberCallTok->str() == "emplace_back") {
                        if (useLoopVarInMemCall)
                            useStlAlgorithmError(memberAccessTok, "std::copy_if");
                        // There is no transform_if to suggest
                    }
                    continue;
                }

                // Check for increment in loop
                incrementTok = singleIncrementInScope(condBodyTok, loopVar->varId(), useLoopVarInIncrement);
                if (incrementTok) {
                    std::string algo;
                    if (useLoopVarInIncrement)
                        algo = "std::transform";
                    else
                        algo = "std::count_if";
                    useStlAlgorithmError(incrementTok, algo);
                    continue;
                }

                // Check early return
                if (isEarlyExit(condBodyTok)) {
                    const Token *loopVar2 = Token::findmatch(condBodyTok, "%varid%", condBodyTok->link(), loopVar->varId());
                    std::string algo;
                    if (loopVar2)
                        algo = "std::find_if";
                    else
                        algo = "std::any_of";
                    useStlAlgorithmError(condBodyTok, algo);
                    continue;
                }
            }
        }
    }
}

void CheckStl::knownEmptyContainerError(const Token *tok, const std::string& algo)
{
    const std::string var = tok ? tok->expressionString() : std::string("var");

    std::string msg;
    if (astIsIterator(tok)) {
        msg = "Using " + algo + " with iterator '" + var + "' that is always empty.";
    } else {
        msg = "Iterating over container '" + var + "' that is always empty.";
    }

    reportError(tok, Severity::style,
                "knownEmptyContainer",
                msg, CWE398, false);
}

static bool isKnownEmptyContainer(const Token* tok)
{
    if (!tok)
        return false;
    for (const ValueFlow::Value& v:tok->values()) {
        if (!v.isKnown())
            continue;
        if (!v.isContainerSizeValue())
            continue;
        if (v.intvalue != 0)
            continue;
        return true;
    }
    return false;
}

void CheckStl::knownEmptyContainer()
{
    if (!mSettings->isEnabled(Settings::STYLE))
        return;
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {

            if (!Token::Match(tok, "%name% ( !!)"))
                continue;

            // Parse range-based for loop
            if (tok->str() == "for") {
                if (!Token::simpleMatch(tok->next()->link(), ") {"))
                    continue;
                const Token *splitTok = tok->next()->astOperand2();
                if (!Token::simpleMatch(splitTok, ":"))
                    continue;
                const Token* contTok = splitTok->astOperand2();
                if (!isKnownEmptyContainer(contTok))
                    continue;
                knownEmptyContainerError(contTok, "");
            } else {
                const std::vector<const Token *> args = getArguments(tok);
                if (args.empty())
                    continue;

                for (int argnr = 1; argnr <= args.size(); ++argnr) {
                    const Library::ArgumentChecks::IteratorInfo *i = mSettings->library.getArgIteratorInfo(tok, argnr);
                    if (!i)
                        continue;
                    const Token * const argTok = args[argnr - 1];
                    if (!isKnownEmptyContainer(argTok))
                        continue;
                    knownEmptyContainerError(argTok, tok->str());
                    break;

                }
            }
        }
    }
}

static bool isMutex(const Variable* var)
{
    const Token* tok = Token::typeDecl(var->nameToken()).first;
    return Token::Match(tok, "std :: mutex|recursive_mutex|timed_mutex|recursive_timed_mutex|shared_mutex");
}

static bool isLockGuard(const Variable* var)
{
    const Token* tok = Token::typeDecl(var->nameToken()).first;
    return Token::Match(tok, "std :: lock_guard|unique_lock|scoped_lock");
}

static bool isLocalMutex(const Variable* var, const Scope* scope)
{
    if (!var)
        return false;
    return !var->isReference() && !var->isRValueReference() && !var->isStatic() && var->scope() == scope;
}

void CheckStl::globalLockGuardError(const Token* tok)
{
    reportError(tok, Severity::warning,
                "globalLockGuard",
                "Lock guard is defined globally. Lock guards are intended to be local. A global lock guard could lead to a deadlock since it won't unlock until the end of the program.", CWE833, false);
}

void CheckStl::localMutexError(const Token* tok)
{
    reportError(tok, Severity::warning,
                "localMutex",
                "The lock is ineffective because the mutex is locked at the same scope as the mutex itself.", CWE667, false);
}

void CheckStl::checkMutexes()
{
    for (const Scope *function : mTokenizer->getSymbolDatabase()->functionScopes) {
        std::set<nonneg int> checkedVars;
        for (const Token *tok = function->bodyStart; tok != function->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%var%"))
                continue;
            const Variable* var = tok->variable();
            if (!var)
                continue;
            if (Token::Match(tok, "%var% . lock ( )")) {
                if (!isMutex(var))
                    continue;
                if (!checkedVars.insert(var->declarationId()).second)
                    continue;
                if (isLocalMutex(var, tok->scope()))
                    localMutexError(tok);
            } else if (Token::Match(tok, "%var% (|{ %var% )|}|,")) {
                if (!isLockGuard(var))
                    continue;
                const Variable* mvar = tok->tokAt(2)->variable();
                if (!mvar)
                    continue;
                if (!checkedVars.insert(mvar->declarationId()).second)
                    continue;
                if (var->isStatic() || var->isGlobal())
                    globalLockGuardError(tok);
                else if (isLocalMutex(mvar, tok->scope()))
                    localMutexError(tok);
            }
        }
    }
}

