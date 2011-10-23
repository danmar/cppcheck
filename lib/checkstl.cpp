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
    reportError(tok, Severity::error, "iterators", "Same iterator is used with both " + container1 + " and " + container2);
}

// Error message used when dereferencing an iterator that has been erased..
void CheckStl::dereferenceErasedError(const Token *tok, const std::string &itername)
{
    reportError(tok, Severity::error, "eraseDereference", "Dereferenced iterator '" + itername + "' has been erased");
}

void CheckStl::iterators()
{
    // Using same iterator against different containers.
    // for (it = foo.begin(); it != bar.end(); ++it)
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Locate an iterator..
        if (!Token::Match(tok, "%var% = %var% . begin|rbegin|cbegin|crbegin ( ) ;|+"))
            continue;

        // Get variable ids for both the iterator and container
        const unsigned int iteratorId(tok->varId());
        const unsigned int containerId(tok->tokAt(2)->varId());
        if (iteratorId == 0 || containerId == 0)
            continue;

        // the validIterator flag says if the iterator has a valid value or not
        bool validIterator = true;

        // counter for { and }
        unsigned int indent = 0;

        // Scan through the rest of the code and see if the iterator is
        // used against other containers.
        for (const Token *tok2 = tok->tokAt(7); tok2; tok2 = tok2->next()) {
            // If a { is found then count it and continue
            if (tok2->str() == "{" && ++indent)
                continue;

            // If a } is found then count it. break if indentlevel becomes 0.
            if (tok2->str() == "}" && --indent == 0)
                break;

            // Is iterator compared against different container?
            if (Token::Match(tok2, "%varid% != %var% . end|rend|cend|crend ( )", iteratorId) && tok2->tokAt(2)->varId() != containerId) {
                iteratorsError(tok2, tok->strAt(2), tok2->strAt(2));
                tok2 = tok2->tokAt(6);
            }

            // Is the iterator used in a insert/erase operation?
            else if (Token::Match(tok2, "%var% . insert|erase ( %varid% )|,", iteratorId)) {
                // It is bad to insert/erase an invalid iterator
                if (!validIterator)
                    invalidIteratorError(tok2, tok2->strAt(4));

                // If insert/erase is used on different container then
                // report an error
                if (tok2->varId() != containerId && tok2->tokAt(5)->str() != ".") {
                    // skip error message if container is a set..
                    if (tok2->varId() > 0) {
                        const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
                        const Variable *variableInfo = symbolDatabase->getVariableFromVarId(tok2->varId());
                        const Token *decltok = variableInfo ? variableInfo->typeStartToken() : NULL;

                        if (Token::Match(decltok, "const| std :: set"))
                            continue; // No warning
                    }

                    // Show error message, mismatching iterator is used.
                    iteratorsError(tok2, tok->strAt(2), tok2->str());
                }

                // invalidate the iterator if it is erased
                else if (tok2->strAt(2) == std::string("erase"))
                    validIterator = false;

                // skip the operation
                tok2 = tok2->tokAt(4);
            }

            // it = foo.erase(..
            // taking the result of an erase is ok
            else if (Token::Match(tok2, "%varid% = %var% . erase (", iteratorId)) {
                // the returned iterator is valid
                validIterator = true;

                // skip the operation
                tok2 = tok2->tokAt(5)->link();
                if (!tok2)
                    break;
            }

            // Reassign the iterator
            else if (Token::Match(tok2, "%varid% = %var% ;", iteratorId)) {
                // Assume that the iterator becomes valid.
                // TODO: add checking that checks if the iterator becomes valid or not
                validIterator = true;

                // skip ahead
                tok2 = tok2->tokAt(2);
            }

            // Dereferencing invalid iterator?
            else if (!validIterator && Token::Match(tok2, "* %varid%", iteratorId)) {
                dereferenceErasedError(tok2, tok2->strAt(1));
                tok2 = tok2->next();
            } else if (!validIterator && Token::Match(tok2, "%varid% . %var%", iteratorId)) {
                dereferenceErasedError(tok2, tok2->strAt(0));
                tok2 = tok2->tokAt(2);
            } else if (Token::Match(tok2, "%var% . erase ( * %varid%", iteratorId) && tok2->varId() == containerId) {
//                eraseByValueError(tok2, tok2->strAt(0), tok2->strAt(5));
            }

            // bailout handling. Assume that the iterator becomes valid if we see return/break.
            // TODO: better handling
            else if (Token::Match(tok2, "return|break")) {
                validIterator = true;
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
    reportError(tok, Severity::error, "mismatchingContainers", "mismatching containers");
}

void CheckStl::mismatchingContainers()
{
    static const char* const algorithm2_strings[] = { // func(begin1, end1
        "adjacent_find", "binary_search", "count", "count_if", "equal", "equal_range", "find", "find_if", "for_each", "generate", "lower_bound", "make_heap",
        "max_element", "min_element", "mismatch", "next_permutation", "partition", "pop_heap", "prev_permutation", "push_heap", "random_shuffle", "remove",
        "remove_copy", "remove_copy_if", "remove_if", "replace", "replace_copy", "replace_copy_if", "replace_if", "reverse", "reverse_copy", "search_n",
        "sort", "sort_heap", "stable_partition", "stable_sort",    "swap_ranges", "transform", "unique", "unique_copy", "upper_bound"
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

    static const std::string pattern2 = "std :: %type% ( %var% . " + iteratorBeginFuncPattern + " ( ) , %var% . " + iteratorEndFuncPattern + " ( ) ,|)";
    static const std::string pattern22 = "std :: %type% ( %var% . " + iteratorBeginFuncPattern + " ( ) , %var% . " + iteratorEndFuncPattern + " ( ) , %var% . " + iteratorBeginFuncPattern + " ( ) , %var% . " + iteratorEndFuncPattern + " ( ) ,|)";
    static const std::string pattern1x1_1 = "std :: %type% ( %var% . " + iteratorBeginFuncPattern + " ( ) , ";
    static const std::string pattern1x1_2 = ", %var% . " + iteratorEndFuncPattern + " ( ) ,|)";

    // Check if different containers are used in various calls of standard functions
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "std")
            continue;

        // TODO: If iterator variables are used instead then there are false negatives.
        if (Token::Match(tok, pattern2.c_str()) && algorithm2.find(tok->strAt(2)) != algorithm2.end()) {
            if (tok->tokAt(4)->str() != tok->tokAt(10)->str()) {
                mismatchingContainersError(tok);
            }
            tok = tok->tokAt(15);
        } else if (Token::Match(tok, pattern22.c_str()) && algorithm22.find(tok->strAt(2)) != algorithm22.end()) {
            if (tok->tokAt(4)->str() != tok->tokAt(10)->str() || tok->tokAt(16)->str() != tok->tokAt(22)->str()) {
                mismatchingContainersError(tok);
            }
            tok = tok->tokAt(27);
        } else if (Token::Match(tok, pattern1x1_1.c_str()) && algorithm1x1.find(tok->strAt(2)) != algorithm1x1.end()) {
            // Find third parameter
            const Token *tok2 = tok->tokAt(10);
            int bracket = 0;
            for (; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    bracket++;
                else if (tok2->str() == ")")
                    bracket--;
                else if (tok2->str() == "," && bracket == 0)
                    break;
            }
            if (tok2 && Token::Match(tok2, pattern1x1_2.c_str())) {
                if (tok->tokAt(4)->str() != tok2->tokAt(1)->str()) {
                    mismatchingContainersError(tok);
                }
                tok = tok2->tokAt(6);
            } else
                tok = tok->tokAt(9);
        }
    }
}


void CheckStl::stlOutOfBounds()
{
    // Scan through all tokens..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // only interested in "for" loops
        if (!Token::simpleMatch(tok, "for ("))
            continue;

        // check if the for loop condition is wrong
        unsigned int indent = 0;
        for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                ++indent;

            else if (tok2->str() == ")") {
                if (indent == 0)
                    break;
                --indent;
            }

            if (Token::Match(tok2, "; %var% <= %var% . size ( ) ;")) {
                // Count { and } for tok3
                unsigned int indent3 = 0;

                // variable id for loop variable.
                unsigned int numId = tok2->tokAt(1)->varId();

                // variable id for the container variable
                unsigned int varId = tok2->tokAt(3)->varId();

                for (const Token *tok3 = tok2->tokAt(8); tok3; tok3 = tok3->next()) {
                    if (tok3->str() == "{")
                        ++indent3;
                    else if (tok3->str() == "}") {
                        if (indent3 <= 1)
                            break;
                        --indent3;
                    } else if (tok3->varId() == varId) {
                        if (Token::simpleMatch(tok3->next(), ". size ( )"))
                            break;
                        else if (Token::Match(tok3->next(), "[ %varid% ]", numId))
                            stlOutOfBoundsError(tok3, tok3->tokAt(2)->str(), tok3->str());
                    }
                }
                break;
            }
        }
    }
}

// Error message for bad iterator usage..
void CheckStl::stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var)
{
    reportError(tok, Severity::error, "stlOutOfBounds", "When " + num + "==" + var + ".size(), " + var + "[" + num + "] is out of bounds");
}


/**
 * @brief %Check for invalid iterator usage after erase/insert/etc
 */
class EraseCheckLoop : public ExecutionPath {
public:
    static void checkScope(CheckStl *checkStl, const Token *it) {
        const Token *tok = it;

        // Search for the start of the loop body..
        int indentlevel = 1;
        while (indentlevel > 0 && 0 != (tok = tok->next())) {
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

        EraseCheckLoop c(checkStl, it->varId());
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
    EraseCheckLoop(Check *o, unsigned int varid)
        : ExecutionPath(o, varid), eraseToken(0) {
    }

    /** @brief token where iterator is erased (non-zero => the iterator is invalid) */
    const Token *eraseToken;

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
                    checkStl->eraseError(c->eraseToken);
                }
            }
        }
    }
};


void CheckStl::erase()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "for (")) {
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    if (Token::Match(tok2, "; %var% !=")) {
                        // Get declaration token for var..
                        const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
                        const Variable *variableInfo = symbolDatabase->getVariableFromVarId(tok2->next()->varId());
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
                    tok2->str() == tok2->tokAt(8)->str() &&
                    tok2->tokAt(2)->str() == tok2->tokAt(10)->str()) {
                    EraseCheckLoop::checkScope(this, tok2);
                    break;
                }
            }
        }

        if (Token::Match(tok, "while ( %var% !=")) {
            const unsigned int varid = tok->tokAt(2)->varId();
            if (varid > 0 && Token::findmatch(_tokenizer->tokens(), "> :: iterator %varid%", varid))
                EraseCheckLoop::checkScope(this, tok->tokAt(2));
        }
    }
}

// Error message for bad iterator usage..
void CheckStl::eraseError(const Token *tok)
{
    reportError(tok, Severity::error, "erase",
                "Dangerous iterator usage after erase()-method.\n"
                "The iterator is invalid after it has been used in erase() function. "
                "Dereferencing or comparing it with another iterator is invalid operation.");
}

void CheckStl::pushback()
{
    // Pointer can become invalid after push_back or push_front..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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
                if (Token::Match(tok2, "%varid% . push_front|push_back", containerId))
                    invalidPointer = true;

                // Using invalid pointer..
                if (invalidPointer && tok2->varId() == pointerId) {
                    if (tok2->previous()->str() == "*")
                        invalidPointerError(tok2, tok2->str());
                    else if (tok2->next()->str() == ".")
                        invalidPointerError(tok2, tok2->str());
                    break;
                }
            }
        }
    }

    // Iterator becomes invalid after reserve, push_back or push_front..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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

        if (iteratorDeclaredInsideLoop && tok->tokAt(4)->str() == "=") {
            // skip "> :: iterator|const_iterator"
            tok = tok->tokAt(3);
        }

        // the variable id for the vector
        unsigned int vectorid = 0;

        // count { , } and parentheses for tok2
        int indent = 0;

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

            // Using push_back or push_front inside a loop..
            if (Token::simpleMatch(tok2, "for (")) {
                tok2 = tok2->tokAt(2);
            }

            if (Token::Match(tok2, "%varid% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %varid% != %var% . end|rend|cend|crend ( ) ; ++| %varid% ++| ) {", iteratorid)) {
                // variable id for the loop iterator
                const unsigned int varId(tok2->tokAt(2)->varId());
                if (varId == 0)
                    continue;

                const Token *pushbackTok = 0;

                // Count { and } for tok3
                unsigned int indent3 = 0;
                for (const Token *tok3 = tok2->tokAt(20); tok3; tok3 = tok3->next()) {
                    if (tok3->str() == "{")
                        ++indent3;
                    else if (tok3->str() == "}") {
                        if (indent3 <= 1)
                            break;
                        --indent3;
                    } else if (tok3->str() == "break" || tok3->str() == "return") {
                        pushbackTok = 0;
                        break;
                    } else if (Token::Match(tok3, "%varid% . push_front|push_back|insert|reserve (", varId)) {
                        pushbackTok = tok3->tokAt(2);
                    }
                }

                if (pushbackTok)
                    invalidIteratorError(pushbackTok, pushbackTok->str(), tok2->strAt(0));
            }

            // Assigning iterator..
            if (Token::Match(tok2, "%varid% =", iteratorid)) {
                if (Token::Match(tok2->tokAt(2), "%var% . begin|end|rbegin|rend|cbegin|cend|crbegin|crend ( )")) {
                    vectorid = tok2->tokAt(2)->varId();
                    tok2 = tok2->tokAt(6);
                } else {
                    vectorid = 0;
                }
                invalidIterator = "";
            }

            // push_back on vector..
            if (vectorid > 0 && Token::Match(tok2, "%varid% . push_front|push_back|insert|reserve (", vectorid)) {
                if (!invalidIterator.empty() && Token::Match(tok2->tokAt(2), "insert ( %varid% ,", iteratorid)) {
                    invalidIteratorError(tok2, invalidIterator, tok2->strAt(4));
                    break;
                }

                invalidIterator = tok2->strAt(2);
                tok2 = tok2->tokAt(3)->link();
            }

            // TODO: instead of bail out for 'else' try to check all execution paths.
            else if (tok2->str() == "return" || tok2->str() == "break" || tok2->str() == "else") {
                invalidIterator.clear();
            }

            // Using invalid iterator..
            if (!invalidIterator.empty()) {
                if (Token::Match(tok2, "++|--|*|+|-|(|,|=|!= %varid%", iteratorid))
                    invalidIteratorError(tok2, invalidIterator, tok2->strAt(1));
                if (Token::Match(tok2, "%varid% ++|--|+|-", iteratorid))
                    invalidIteratorError(tok2, invalidIterator, tok2->str());
            }
        }
    }
}


// Error message for bad iterator usage..
void CheckStl::invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name)
{
    reportError(tok, Severity::error, "invalidIterator2", "After " + func + ", the iterator '" + iterator_name + "' may be invalid");
}


// Error message for bad iterator usage..
void CheckStl::invalidPointerError(const Token *tok, const std::string &pointer_name)
{
    reportError(tok, Severity::error, "invalidPointer", "Invalid pointer '" + pointer_name + "' after push_back / push_front");
}




void CheckStl::stlBoundries()
{
    // containers (not the vector)..
    static const char STL_CONTAINER_LIST[] = "bitset|deque|list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set";

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Declaring iterator..
        if (tok->str() == "<" && Token::Match(tok->previous(), STL_CONTAINER_LIST)) {
            const std::string container_name(tok->strAt(-1));
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;

            if (Token::Match(tok, "> :: iterator|const_iterator %var% =|;")) {
                const unsigned int iteratorid(tok->tokAt(3)->varId());
                if (iteratorid == 0)
                    continue;

                // Using "iterator < ..." is not allowed
                unsigned int indentlevel = 0;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}") {
                        if (indentlevel == 0)
                            break;
                        --indentlevel;
                    } else if (Token::Match(tok2, "!!* %varid% <", iteratorid)) {
                        stlBoundriesError(tok2, container_name);
                    }
                }
            }
        }
    }
}

// Error message for bad boundry usage..
void CheckStl::stlBoundriesError(const Token *tok, const std::string &container_name)
{
    reportError(tok, Severity::error, "stlBoundries",
                "Dangerous container iterator compare using < operator for " + container_name + "\n"
                "Container '" + container_name + "' iterator compared with < operator. "
                "Using < operator with container type iterators is dangerous since the order of "
                "the items is not guaranteed. One should use != operator instead when comparing "
                "iterators in the container.");
}

void CheckStl::if_find()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if ( !| %var% . find ( %any% ) )")) {
            // goto %var%
            tok = tok->tokAt(2);
            if (!tok->isName())
                tok = tok->next();

            const unsigned int varid = tok->varId();
            const Variable *var = symbolDatabase->getVariableFromVarId(varid);
            if (var) {
                // Is the variable a std::string or STL container?
                const Token * decl = var->nameToken();
                while (decl && !Token::Match(decl, "[;{}(,]"))
                    decl = decl->previous();

                if (decl)
                    decl = decl->next();

                // stl container
                if (Token::Match(decl, "const| std :: %var% < %type% > &|*| %varid%", varid))
                    if_findError(tok, false);
                else if (Token::Match(decl, "const| std :: string &|*| %varid%", varid))
                    if_findError(tok, true);
            }
        }

        if (Token::Match(tok, "if ( !| std :: find|find_if (")) {
            // goto '(' for the find
            tok = tok->tokAt(4);
            if (tok->isName())
                tok = tok->next();

            // check that result is checked properly
            if (Token::simpleMatch(tok->link(), ") )")) {
                if_findError(tok, false);
            }
        }
    }
}


void CheckStl::if_findError(const Token *tok, bool str)
{
    if (str)
        reportError(tok, Severity::warning, "stlIfStrFind",
                    "Suspicious checking of string::find() return value.\n"
                    "Checking of string::find() return value looks Suspicious. "
                    "string::find will return 0 if the string is found at position 0. "
                    "If that is wanted to check then string::compare is a faster alternative "
                    "because it doesn't scan through the string.");
    else
        reportError(tok, Severity::warning, "stlIfFind", "Suspicious condition. The result of find is an iterator, but it is not properly checked.");
}



bool CheckStl::isStlContainer(unsigned int varid)
{
    // check if this token is defined
    if (varid) {
        // find where this token is defined
        const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(varid);

        if (!var)
            return false;

        // find where this tokens type starts
        const Token *type = var->typeStartToken();

        // ignore "const"
        if (type->str() == "const")
            type = type->next();

        // discard namespace if supplied
        if (Token::simpleMatch(type, "std ::"))
            type = type->next()->next();

        // all possible stl containers
        static const char STL_CONTAINER_LIST[] = "bitset|deque|list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|vector";

        // container template string
        const std::string checkStr(std::string(STL_CONTAINER_LIST) + " <");

        // check if it's an stl template
        if (Token::Match(type, checkStr.c_str()))
            return true;
    }

    return false;
}

void CheckStl::size()
{
    if (!_settings->isEnabled("performance"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% . size ( )") ||
            Token::Match(tok, "%var% . %var% . size ( )")) {
            int offset = 5;
            const Token *tok1 = tok;
            unsigned int varid = 0;

            // get the variable id
            if (tok->strAt(2) != "size") {
                offset = 7;
                tok1 = tok1->tokAt(2);

                // found a.b.size(), lookup class/struct variable
                const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->varId());
                if (var && var->type()) {
                    // get class/struct variable type
                    const Scope *type = var->type();

                    // lookup variable member
                    std::list<Variable>::const_iterator it;
                    for (it = type->varlist.begin(); it != type->varlist.end(); ++it) {
                        if (it->name() == tok1->str()) {
                            // found member variable, save varid
                            varid = it->varId();
                            break;
                        }
                    }
                }
            } else
                varid = tok1->varId();

            if (varid) {
                // check for comparison to zero
                if (Token::Match(tok->tokAt(offset), "==|!=|> 0") ||
                    Token::Match(tok->tokAt(-2), "0 ==|!=|<")) {
                    if (isStlContainer(varid))
                        sizeError(tok1);
                }

                // check for using as boolean expression
                else if ((Token::Match(tok->tokAt(-2), "if|while (") ||
                          Token::Match(tok->tokAt(-3), "if|while ( !")) &&
                         tok->strAt(offset) == ")") {
                    if (isStlContainer(varid))
                        sizeError(tok1);
                }
            }
        }
    }
}

void CheckStl::sizeError(const Token *tok)
{
    const std::string varname(tok ? tok->str().c_str() : "list");
    reportError(tok, Severity::performance, "stlSize",
                "Possible inefficient checking for '" + varname + "' emptiness.\n"
                "Checking for '" + varname + "' emptiness might be inefficient. "
                "Using " + varname + ".empty() instead of " + varname + ".size() can be faster. " +
                varname + ".size() can take linear time but " + varname + ".empty() is "
                "guaranteed to take constant time.");
}

void CheckStl::redundantCondition()
{
    const char pattern[] = "if ( %var% . find ( %any% ) != %var% . end|rend ( ) ) "
                           "{|{|"
                           "    %var% . remove ( %any% ) ; "
                           "}|}|";
    const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern);
    while (tok) {
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
            any1->str() == any2->str()) {
            redundantIfRemoveError(tok);
        }

        tok = Token::findmatch(tok->next(), pattern);
    }
}

void CheckStl::redundantIfRemoveError(const Token *tok)
{
    reportError(tok, Severity::style, "redundantIfRemove",
                "Redundant checking of STL container element.\n"
                "Redundant checking of STL container element existence before removing it. "
                "The remove method in the STL will not do anything if element doesn't exist");
}

void CheckStl::missingComparison()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "for (")) {
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";")
                    break;

                if (!Token::Match(tok2, "%var% = %var% . begin|rbegin|cbegin|crbegin ( ) ; %var% != %var% . end|rend|cend|crend ( ) ; ++| %var% ++| ) {"))
                    continue;

                // same iterator name
                if (tok2->str() != tok2->strAt(8))
                    continue;

                // same container
                if (tok2->strAt(2) != tok2->strAt(10))
                    continue;

                // increment iterator
                if (!Token::simpleMatch(tok2->tokAt(16), ("++ " + tok2->str() + " )").c_str()) &&
                    !Token::simpleMatch(tok2->tokAt(16), (tok2->str() + " ++ )").c_str())) {
                    continue;
                }

                const unsigned int &iteratorId(tok2->varId());
                if (iteratorId == 0)
                    continue;

                const Token *incrementToken = 0;

                // Count { and } for tok3
                unsigned int indentlevel = 0;

                // Parse loop..
                for (const Token *tok3 = tok2->tokAt(20); tok3; tok3 = tok3->next()) {
                    if (tok3->str() == "{")
                        ++indentlevel;
                    else if (tok3->str() == "}") {
                        if (indentlevel == 0)
                            break;
                        --indentlevel;
                    } else if (Token::Match(tok3, "%varid% ++", iteratorId))
                        incrementToken = tok3;
                    else if (Token::Match(tok3->previous(), "++ %varid% !!.", iteratorId))
                        incrementToken = tok3;
                    else if (Token::Match(tok3, "%varid% !=|==", iteratorId))
                        incrementToken = 0;
                    else if (tok3->str() == "break" || tok3->str() == "return")
                        incrementToken = 0;
                    else if (Token::Match(tok3, "%varid% = %var% . insert ( ++| %varid% ++| ,", iteratorId)) {
                        // skip insertion..
                        tok3 = tok3->tokAt(6)->link();
                        if (!tok3)
                            break;
                    }
                }
                if (incrementToken)
                    missingComparisonError(incrementToken, tok2->tokAt(16));
            }
        }
    }
}

void CheckStl::missingComparisonError(const Token *incrementToken1, const Token *incrementToken2)
{
    std::ostringstream errmsg;
    errmsg << "Missing bounds check for extra iterator increment in loop.\n"
           << "The iterator incrementing is suspicious - it is incremented at line "
           << incrementToken1->linenr() << " and then at line " << incrementToken2->linenr()
           << " The loop might unintentionally skip an element in the container. "
           << "There is no comparison between these increments to prevent that the iterator is "
           << "incremented beyond the end.";

    reportError(incrementToken1, Severity::warning, "StlMissingComparison", errmsg.str());
}


void CheckStl::string_c_str()
{
    // Try to detect common problems when using string::c_str()
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Locate executable scopes:
        if (Token::Match(tok, ") const| {")) {
            std::set<unsigned int> localvar;
            std::set<unsigned int> pointers;

            // scan through this executable scope:
            unsigned int indentlevel = 0;
            while (NULL != (tok = tok->next())) {
                if (tok->str() == "{")
                    ++indentlevel;
                else if (tok->str() == "}") {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }

                // Variable declarations..
                else if (Token::Match(tok->previous(), "[;{}] std :: %type% %var% ;"))
                    localvar.insert(tok->tokAt(3)->varId());
                else if (Token::Match(tok->previous(), "[;{}] %type% %var% ;"))
                    localvar.insert(tok->next()->varId());
                else if (Token::Match(tok->previous(), "[;{}] %type% * %var% ;"))
                    pointers.insert(tok->tokAt(2)->varId());
                else if (Token::Match(tok->previous(), "[;{}] %type% %type% * %var% ;"))
                    pointers.insert(tok->tokAt(3)->varId());

                // Invalid usage..
                else if (Token::Match(tok, "throw %var% . c_str ( ) ;") &&
                         tok->next()->varId() > 0 &&
                         localvar.find(tok->next()->varId()) != localvar.end()) {
                    string_c_strError(tok);
                } else if (Token::Match(tok, "[;{}] %var% = %var% . str ( ) . c_str ( ) ;") &&
                           tok->next()->varId() > 0 &&
                           pointers.find(tok->next()->varId()) != pointers.end()) {
                    string_c_strError(tok);
                } else if (Token::Match(tok, "[;{}] %var% = %var% (") &&
                           Token::simpleMatch(tok->tokAt(4)->link(), ") . c_str ( ) ;") &&
                           tok->next()->varId() > 0 &&
                           pointers.find(tok->next()->varId()) != pointers.end() &&
                           Token::findmatch(_tokenizer->tokens(), ("std :: string " + tok->strAt(3) + " (").c_str())) {
                    string_c_strError(tok);
                }
            }
        }
    }
}

void CheckStl::string_c_strError(const Token *tok)
{
    reportError(tok, Severity::error, "stlcstr", "Dangerous usage of c_str()");
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CheckStl::checkAutoPointer()
{
    std::set<int> autoPtrVarId;
    std::set<int>::const_iterator iter;
    static const char STL_CONTAINER_LIST[] = "bitset|deque|list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|vector";

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "auto_ptr <")) {
            if ((tok->previous() && tok->previous()->str() == "<" && Token::Match(tok->tokAt(-2), STL_CONTAINER_LIST)) ||
                (Token::Match(tok->tokAt(-3), "< std :: auto_ptr") && Token::Match(tok->tokAt(-4), STL_CONTAINER_LIST))) {
                autoPointerContainerError(tok);
            } else {
                const Token *tok2 = tok->next()->next();
                while (tok2) {
                    if (Token::Match(tok2, "> %var%")) {
                        const Token *tok3 = tok2->next()->next();
                        while (tok3 && tok3->str() != ";") {
                            tok3 = tok3->next();
                        }
                        if (tok3) {
                            tok3 = tok3->previous()->previous();
                            if (Token::simpleMatch(tok3->previous(), "[ ] )")) {
                                autoPointerArrayError(tok2->next());
                            } else if (tok3->varId()) {
                                const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid% = new %type% [", tok3->varId());
                                if (decltok) {
                                    autoPointerArrayError(tok2->next());
                                }
                            }
                            if (tok2->next()->varId()) {
                                autoPtrVarId.insert(tok2->next()->varId());
                            }
                            break;
                        }
                    }
                    tok2 = tok2->next();
                }
            }
        } else {
            if (Token::Match(tok, "%var% = %var% ;")) {
                if (_settings->isEnabled("style")) {
                    iter = autoPtrVarId.find(tok->next()->next()->varId());
                    if (iter != autoPtrVarId.end()) {
                        autoPointerError(tok->next()->next());
                    }
                }
            } else if (Token::Match(tok, "%var% = new %type% [") || Token::Match(tok, "%var% . reset ( new %type% [")) {
                iter = autoPtrVarId.find(tok->varId());
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
                "Copy 'auto_ptr' pointer to another do not create two equal objects since one has lost its ownership of the pointer.\n"
                "The auto_ptr has semantics of strict ownership, meaning that the auto_ptr instance is the sole entity responsible for the object's lifetime. If an auto_ptr is copied, the source loses the reference."
               );
}

void CheckStl::autoPointerContainerError(const Token *tok)
{
    reportError(tok, Severity::error, "useAutoPointerContainer",
                "You can randomly lose access to pointers if you store 'auto_ptr' pointers in a container because the copy-semantics of 'auto_ptr' are not compatible with containers.\n"
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

