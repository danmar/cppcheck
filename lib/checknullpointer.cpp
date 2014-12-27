/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#include "checknullpointer.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include <cctype>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckNullPointer instance;
}

//---------------------------------------------------------------------------

/**
 * @brief parse a function call and extract information about variable usage
 * @param tok first token
 * @param var variables that the function read / write.
 * @param library --library files data
 * @param value 0 => invalid with null pointers as parameter.
 *              1-.. => only invalid with uninitialized data.
 */
void CheckNullPointer::parseFunctionCall(const Token &tok, std::list<const Token *> &var, const Library *library, unsigned char value)
{
    if (Token::Match(&tok, "%var% ( )") || !tok.tokAt(2))
        return;

    const Token* firstParam = tok.tokAt(2);
    const Token* secondParam = firstParam->nextArgument();

    // 1st parameter..
    if ((Token::Match(firstParam, "%var% ,|)") && firstParam->varId() > 0) ||
        (value == 0 && Token::Match(firstParam, "0|NULL ,|)"))) {
        if (value == 0 && Token::Match(&tok, "snprintf|vsnprintf|fnprintf|vfnprintf") && secondParam && secondParam->str() != "0") // Only if length (second parameter) is not zero
            var.push_back(firstParam);
        else if (value == 0 && library != nullptr && library->isnullargbad(tok.str(),1))
            var.push_back(firstParam);
        else if (value == 1 && library != nullptr && library->isuninitargbad(tok.str(),1))
            var.push_back(firstParam);
    }

    // 2nd parameter..
    if ((value == 0 && Token::Match(secondParam, "0|NULL ,|)")) || (secondParam && secondParam->varId() > 0 && Token::Match(secondParam->next(),"[,)]"))) {
        if (value == 0 && library != nullptr && library->isnullargbad(tok.str(),2))
            var.push_back(secondParam);
        else if (value == 1 && library != nullptr && library->isuninitargbad(tok.str(),2))
            var.push_back(secondParam);
    }

    if (Token::Match(&tok, "printf|sprintf|snprintf|fprintf|fnprintf|scanf|sscanf|fscanf|wprintf|swprintf|fwprintf|wscanf|swscanf|fwscanf")) {
        const Token* argListTok = 0; // Points to first va_list argument
        std::string formatString;
        bool scan = Token::Match(&tok, "scanf|sscanf|fscanf|wscanf|swscanf|fwscanf");

        if (Token::Match(&tok, "printf|scanf|wprintf|wscanf ( %str%")) {
            formatString = firstParam->strValue();
            argListTok = secondParam;
        } else if (Token::Match(&tok, "sprintf|fprintf|sscanf|fscanf|fwprintf|fwscanf|swscanf")) {
            const Token* formatStringTok = secondParam; // Find second parameter (format string)
            if (formatStringTok && formatStringTok->type() == Token::eString) {
                argListTok = formatStringTok->nextArgument(); // Find third parameter (first argument of va_args)
                formatString = formatStringTok->strValue();
            }
        } else if (Token::Match(&tok, "snprintf|fnprintf|swprintf") && secondParam) {
            const Token* formatStringTok = secondParam->nextArgument(); // Find third parameter (format string)
            if (formatStringTok && formatStringTok->type() == Token::eString) {
                argListTok = formatStringTok->nextArgument(); // Find fourth parameter (first argument of va_args)
                formatString = formatStringTok->strValue();
            }
        }

        if (argListTok) {
            bool percent = false;
            for (std::string::iterator i = formatString.begin(); i != formatString.end(); ++i) {
                if (*i == '%') {
                    percent = !percent;
                } else if (percent) {
                    percent = false;

                    bool _continue = false;
                    while (!std::isalpha((unsigned char)*i)) {
                        if (*i == '*') {
                            if (scan)
                                _continue = true;
                            else
                                argListTok = argListTok->nextArgument();
                        }
                        ++i;
                        if (!argListTok || i == formatString.end())
                            return;
                    }
                    if (_continue)
                        continue;

                    if ((*i == 'n' || *i == 's' || scan) && (!scan || value == 0)) {
                        if ((value == 0 && argListTok->str() == "0") || (argListTok->varId() > 0 && Token::Match(argListTok,"%var% [,)]"))) {
                            var.push_back(argListTok);
                        }
                    }

                    if (*i != 'm') // %m is a non-standard glibc extension that requires no parameter
                        argListTok = argListTok->nextArgument(); // Find next argument
                    if (!argListTok)
                        break;
                }
            }
        }
    }
}


/**
 * Is there a pointer dereference? Everything that should result in
 * a nullpointer dereference error message will result in a true
 * return value. If it's unknown if the pointer is dereferenced false
 * is returned.
 * @param tok token for the pointer
 * @param unknown it is not known if there is a pointer dereference (could be reported as a debug message)
 * @return true => there is a dereference
 */
bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown)
{
    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_stream [] = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "ofstream", "ostream", "ostringstream",
        "stringstream", "wistringstream", "wostringstream", "wstringstream"
    };

    unknown = false;

    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    bool firstOperand = parent->astOperand1() == tok;
    while (parent->str() == "(" && (parent->astOperand2() == nullptr && parent->strAt(1) != ")")) { // Skip over casts
        parent = parent->astParent();
        if (!parent)
            return false;
    }

    // Dereferencing pointer..
    if (parent->str() == "*" && !parent->astOperand2() && !Token::Match(parent->tokAt(-2), "sizeof|decltype|typeof"))
        return true;

    // array access
    if (parent->str() == "[" && (!parent->astParent() || parent->astParent()->str() != "&"))
        return true;

    // read/write member variable
    if (firstOperand && parent->str() == "." && (!parent->astParent() || parent->astParent()->str() != "&")) {
        if (!parent->astParent() || parent->astParent()->str() != "(" || parent->astParent() == tok->previous())
            return true;
        unknown = true;
        return false;
    }

    if (Token::Match(tok, "%var% ("))
        return true;

    if (Token::Match(tok, "%var% = %var% .") &&
        tok->varId() > 0 &&
        tok->varId() == tok->tokAt(2)->varId())
        return true;

    // std::string dereferences nullpointers
    if (Token::Match(parent->tokAt(-3), "std :: string|wstring (") && tok->strAt(1) == ")")
        return true;
    if (Token::Match(parent->previous(), "%var% (") && tok->strAt(1) == ")") {
        const Variable* var = tok->tokAt(-2)->variable();
        if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
            return true;
    }

    // streams dereference nullpointers
    if (Token::Match(parent, "<<|>>") && !firstOperand) {
        const Variable* var = tok->variable();
        if (var && var->isPointer() && Token::Match(var->typeStartToken(), "char|wchar_t")) { // Only outputting or reading to char* can cause problems
            const Token* tok2 = parent; // Find start of statement
            for (; tok2; tok2 = tok2->previous()) {
                if (Token::Match(tok2->previous(), ";|{|}|:"))
                    break;
            }
            if (Token::Match(tok2, "std :: cout|cin|cerr"))
                return true;
            if (tok2 && tok2->varId() != 0) {
                const Variable* var2 = tok2->variable();
                if (var2 && var2->isStlType(stl_stream))
                    return true;
            }
        }
    }

    const Variable *ovar = nullptr;
    if (Token::Match(parent, "+|==|!=") || (parent->str() == "=" && !firstOperand)) {
        if (parent->astOperand1() == tok && parent->astOperand2())
            ovar = parent->astOperand2()->variable();
        else if (parent->astOperand1() && parent->astOperand2() == tok)
            ovar = parent->astOperand1()->variable();
    }
    if (ovar && !ovar->isPointer() && !ovar->isArray() && ovar->isStlStringType())
        return true;

    // assume that it's not a dereference (no false positives)
    return false;
}


void CheckNullPointer::nullPointerLinkedList()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // looping through items in a linked list in a inner loop.
    // Here is an example:
    //    for (const Token *tok = tokens; tok; tok = tok->next) {
    //        if (tok->str() == "hello")
    //            tok = tok->next;   // <- tok might become a null pointer!
    //    }
    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        const Token* const tok1 = i->classDef;
        // search for a "for" scope..
        if (i->type != Scope::eFor || !tok1)
            continue;

        // is there any dereferencing occurring in the for statement
        const Token* end2 = tok1->linkAt(1);
        for (const Token *tok2 = tok1->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
            // Dereferencing a variable inside the "for" parentheses..
            if (Token::Match(tok2, "%var% . %var%")) {
                // Is this variable a pointer?
                const Variable *var = tok2->variable();
                if (!var || !var->isPointer())
                    continue;

                // Variable id for dereferenced variable
                const unsigned int varid(tok2->varId());

                // We don't support variables without a varid
                if (varid == 0)
                    continue;

                if (Token::Match(tok2->tokAt(-2), "%varid% ?", varid))
                    continue;

                // Check usage of dereferenced variable in the loop..
                for (std::list<Scope*>::const_iterator j = i->nestedList.begin(); j != i->nestedList.end(); ++j) {
                    Scope* scope = *j;
                    if (scope->type != Scope::eWhile)
                        continue;

                    // TODO: are there false negatives for "while ( %varid% ||"
                    if (Token::Match(scope->classDef->next(), "( %varid% &&|)", varid)) {
                        // Make sure there is a "break" or "return" inside the loop.
                        // Without the "break" a null pointer could be dereferenced in the
                        // for statement.
                        for (const Token *tok4 = scope->classStart; tok4; tok4 = tok4->next()) {
                            if (tok4 == i->classEnd) {
                                nullPointerError(tok1, var->name(), scope->classDef);
                                break;
                            }

                            // There is a "break" or "return" inside the loop.
                            // TODO: there can be false negatives. There could still be
                            //       execution paths that are not properly terminated
                            else if (tok4->str() == "break" || tok4->str() == "return")
                                break;
                        }
                    }
                }
            }
        }
    }
}

void CheckNullPointer::nullPointerByDeRefAndChec()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        const Variable *var = tok->variable();
        if (!var || !var->isPointer() || tok == var->nameToken())
            continue;

        // Can pointer be NULL?
        const ValueFlow::Value *value = tok->getValue(0);
        if (!value)
            continue;

        if (!_settings->inconclusive && value->inconclusive)
            continue;

        // Is pointer used as function parameter?
        if (Token::Match(tok->previous(), "[(,] %var% [,)]")) {
            const Token *ftok = tok->previous();
            while (ftok && ftok->str() != "(") {
                if (ftok->str() == ")")
                    ftok = ftok->link();
                ftok = ftok->previous();
            }
            if (!ftok || !ftok->previous())
                continue;
            std::list<const Token *> varlist;
            parseFunctionCall(*ftok->previous(), varlist, &_settings->library, 0);
            if (std::find(varlist.begin(), varlist.end(), tok) != varlist.end()) {
                if (value->condition == nullptr)
                    nullPointerError(tok, tok->str());
                else if (_settings->isEnabled("warning"))
                    nullPointerError(tok, tok->str(), value->condition, value->inconclusive);
            }
            continue;
        }

        // Pointer dereference.
        bool unknown = false;
        if (!isPointerDeRef(tok,unknown)) {
            if (_settings->inconclusive && unknown) {
                if (value->condition == nullptr)
                    nullPointerError(tok, tok->str(), true);
                else
                    nullPointerError(tok, tok->str(), value->condition, true);
            }
            continue;
        }

        if (value->condition == nullptr)
            nullPointerError(tok, tok->str(), value->inconclusive);
        else if (_settings->isEnabled("warning"))
            nullPointerError(tok, tok->str(), value->condition, value->inconclusive);
    }
}

void CheckNullPointer::nullPointer()
{
    nullPointerLinkedList();
    nullPointerByDeRefAndChec();
    nullPointerDefaultArgument();
}

/** Dereferencing null constant (simplified token list) */
void CheckNullPointer::nullConstantDereference()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_stream[] = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "stringstream", "wistringstream", "wstringstream"
    };

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (scope->function == 0 || !scope->function->hasBody) // We only look for functions with a body
            continue;

        const Token *tok = scope->classStart;

        if (scope->function && scope->function->isConstructor())
            tok = scope->function->token; // Check initialization list

        for (; tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "sizeof|decltype|typeid|typeof ("))
                tok = tok->next()->link();

            else if (Token::simpleMatch(tok, "* 0")) {
                if (Token::Match(tok->previous(), "return|throw|;|{|}|:|[|(|,") || tok->previous()->isOp()) {
                    nullPointerError(tok);
                }
            }

            else if (Token::Match(tok, "0 [") && (tok->previous()->str() != "&" || !Token::Match(tok->next()->link()->next(), "[.(]")))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), "!!. %var% (") && (tok->previous()->str() != "::" || tok->strAt(-2) == "std")) {
                if (Token::simpleMatch(tok->tokAt(2), "0 )") && tok->varId()) { // constructor call
                    const Variable *var = tok->variable();
                    if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
                        nullPointerError(tok);
                } else { // function call
                    std::list<const Token *> var;
                    parseFunctionCall(*tok, var, &_settings->library, 0);

                    // is one of the var items a NULL pointer?
                    for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it) {
                        if (Token::Match(*it, "0|NULL [,)]")) {
                            nullPointerError(*it);
                        }
                    }
                }
            } else if (Token::Match(tok, "std :: string|wstring ( 0 )"))
                nullPointerError(tok);

            else if (Token::simpleMatch(tok->previous(), ">> 0")) { // Only checking input stream operations is safe here, because otherwise 0 can be an integer as well
                const Token* tok2 = tok->previous(); // Find start of statement
                for (; tok2; tok2 = tok2->previous()) {
                    if (Token::Match(tok2->previous(), ";|{|}|:|("))
                        break;
                }
                if (tok2 && tok2->previous() && tok2->previous()->str()=="(")
                    continue;
                if (Token::simpleMatch(tok2, "std :: cin"))
                    nullPointerError(tok);
                if (tok2 && tok2->varId() != 0) {
                    const Variable *var = tok2->variable();
                    if (var && var->isStlType(stl_stream))
                        nullPointerError(tok);
                }
            }

            const Variable *ovar = nullptr;
            if (Token::Match(tok, "0 ==|!=|>|>=|<|<= %var% !!."))
                ovar = tok->tokAt(2)->variable();
            else if (Token::Match(tok, "%var% ==|!=|>|>=|<|<= 0"))
                ovar = tok->variable();
            else if (Token::Match(tok, "%var% =|+ 0 )|]|,|;|+"))
                ovar = tok->variable();
            if (ovar && !ovar->isPointer() && !ovar->isArray() && ovar->isStlStringType() && tok->tokAt(2)->originalName() != "'\\0'")
                nullPointerError(tok);
        }
    }
}

/**
* @brief If tok is a function call that passes in a pointer such that
*         the pointer may be modified, this function will remove that
*         pointer from pointerArgs.
*/
void CheckNullPointer::removeAssignedVarFromSet(const Token* tok, std::set<unsigned int>& pointerArgs)
{
    // If a pointer's address is passed into a function, stop considering it
    if (Token::Match(tok->previous(), "[;{}] %var% (")) {
        // Common functions that are known NOT to modify their pointer argument
        const char safeFunctions[] = "printf|sprintf|fprintf|vprintf";

        const Token* endParen = tok->next()->link();
        for (const Token* tok2 = tok->next(); tok2 != endParen; tok2 = tok2->next()) {
            if (tok2->isName() && tok2->varId() > 0 && !Token::Match(tok, safeFunctions)) {
                pointerArgs.erase(tok2->varId());
            }
        }
    }
}

/**
* @brief Does one part of the check for nullPointer().
* -# default argument that sets a pointer to 0
* -# dereference pointer
*/
void CheckNullPointer::nullPointerDefaultArgument()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (scope->function == 0 || !scope->function->hasBody) // We only look for functions with a body
            continue;

        // Scan the argument list for default arguments that are pointers and
        // which default to a NULL pointer if no argument is specified.
        std::set<unsigned int> pointerArgs;
        for (const Token *tok = scope->function->arg; tok != scope->function->arg->link(); tok = tok->next()) {

            if (Token::Match(tok, "%var% = 0 ,|)") && tok->varId() != 0) {
                const Variable *var = tok->variable();
                if (var && var->isPointer())
                    pointerArgs.insert(tok->varId());
            }
        }

        // Report an error if any of the default-NULL arguments are dereferenced
        if (!pointerArgs.empty()) {
            for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {

                // If we encounter a possible NULL-pointer check, skip over its body
                if (tok->str() == "?") { // TODO: Skip this if the condition is unrelated to the variables
                    // Find end of statement
                    tok = tok->astOperand2();
                    while (tok && !Token::Match(tok, ")|;")) {
                        if (tok->link() && Token::Match(tok, "(|[|<|{"))
                            tok = tok->link();
                        tok = tok->next();
                    }
                    if (!tok)
                        break;
                } else if (Token::simpleMatch(tok, "if ("))  {
                    bool dependsOnPointer = false;
                    const Token *endOfCondition = tok->next()->link();
                    if (!endOfCondition)
                        continue;

                    const Token *startOfIfBlock =
                        Token::simpleMatch(endOfCondition, ") {") ? endOfCondition->next() : nullptr;
                    if (!startOfIfBlock)
                        continue;

                    // If this if() statement may return, it may be a null
                    // pointer check for the pointers referenced in its condition
                    const Token *endOfIf = startOfIfBlock->link();
                    bool isExitOrReturn =
                        Token::findmatch(startOfIfBlock, "exit|return|throw", endOfIf) != nullptr;

                    if (Token::Match(tok, "if ( %var% == 0 )")) {
                        const unsigned int var = tok->tokAt(2)->varId();
                        if (var > 0 && pointerArgs.count(var) > 0) {
                            if (isExitOrReturn)
                                pointerArgs.erase(var);
                            else
                                dependsOnPointer = true;
                        }
                    } else {
                        for (const Token *tok2 = tok->next(); tok2 != endOfCondition; tok2 = tok2->next()) {
                            if (tok2->isName() && tok2->varId() > 0 &&
                                pointerArgs.count(tok2->varId()) > 0) {

                                // If the if() depends on a pointer and may return, stop
                                // considering that pointer because it may be a NULL-pointer
                                // check that returns if the pointer is NULL.
                                if (isExitOrReturn)
                                    pointerArgs.erase(tok2->varId());
                                else
                                    dependsOnPointer = true;
                            }
                        }
                    }

                    if (dependsOnPointer && endOfIf) {
                        for (; tok != endOfIf; tok = tok->next()) {
                            // If a pointer is assigned a new value, stop considering it.
                            if (Token::Match(tok, "%var% ="))
                                pointerArgs.erase(tok->varId());
                            else
                                removeAssignedVarFromSet(tok, pointerArgs);
                        }
                        continue;
                    }
                }

                // If there is a noreturn function (e.g. exit()), stop considering the rest of
                // this function.
                bool unknown = false;
                if (Token::Match(tok, "return|throw|exit") ||
                    (_tokenizer->IsScopeNoReturn(tok, &unknown) && !unknown))
                    break;

                removeAssignedVarFromSet(tok, pointerArgs);

                if (tok->varId() == 0 || pointerArgs.count(tok->varId()) == 0)
                    continue;

                // If a pointer is assigned a new value, stop considering it.
                if (Token::Match(tok, "%var% ="))
                    pointerArgs.erase(tok->varId());

                // If a pointer dereference is preceded by an && or ||,
                // they serve as a sequence point so the dereference
                // may not be executed.
                if (isPointerDeRef(tok, unknown) && !unknown &&
                    tok->strAt(-1) != "&&" && tok->strAt(-1) != "||" &&
                    tok->strAt(-2) != "&&" && tok->strAt(-2) != "||")
                    nullPointerDefaultArgError(tok, tok->str());
            }
        }
    }
}

void CheckNullPointer::nullPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "nullPointer", "Null pointer dereference");
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, bool inconclusive)
{
    reportError(tok, Severity::error, "nullPointer", "Possible null pointer dereference: " + varname, inconclusive);
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, const Token* nullCheck, bool inconclusive)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok);
    callstack.push_back(nullCheck);
    const std::string errmsg("Possible null pointer dereference: " + varname + " - otherwise it is redundant to check it against null.");
    reportError(callstack, Severity::warning, "nullPointer", errmsg, inconclusive);
}

void CheckNullPointer::nullPointerDefaultArgError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "nullPointer", "Possible null pointer dereference if the default parameter value is used: " + varname);
}
