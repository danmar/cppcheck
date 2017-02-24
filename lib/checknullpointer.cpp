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


//---------------------------------------------------------------------------
#include "checknullpointer.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "utils.h"
#include <cctype>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckNullPointer instance;
}

static const CWE CWE476(476U);  // NULL Pointer Dereference
static const CWE CWE682(682U);  // Incorrect Calculation

//---------------------------------------------------------------------------

static bool checkNullpointerFunctionCallPlausibility(const Function* func, unsigned int arg)
{
    return !func || (func->argCount() >= arg && func->getArgumentVar(arg - 1) && func->getArgumentVar(arg - 1)->isPointer());
}

/**
 * @brief parse a function call and extract information about variable usage
 * @param tok first token
 * @param var variables that the function read / write.
 * @param library --library files data
 * @param value 0 => invalid with null pointers as parameter.
 *              1-.. => only invalid with uninitialized data.
 */
void CheckNullPointer::parseFunctionCall(const Token &tok, std::list<const Token *> &var, const Library *library)
{
    if (Token::Match(&tok, "%name% ( )") || !tok.tokAt(2))
        return;

    const Token* firstParam = tok.tokAt(2);
    const Token* secondParam = firstParam->nextArgument();

    // 1st parameter..
    if (Token::Match(&tok, "snprintf|vsnprintf|fnprintf|vfnprintf") && secondParam && secondParam->str() != "0") // Only if length (second parameter) is not zero
        var.push_back(firstParam);

    if (library || tok.function() != nullptr) {
        const Token *param = firstParam;
        int argnr = 1;
        while (param) {
            if (library && library->isnullargbad(&tok, argnr) && checkNullpointerFunctionCallPlausibility(tok.function(), argnr))
                var.push_back(param);
            else if (tok.function()) {
                const Variable* argVar = tok.function()->getArgumentVar(argnr-1);
                if (argVar && argVar->isStlStringType() && !argVar->isArrayOrPointer())
                    var.push_back(param);
            }
            param = param->nextArgument();
            argnr++;
        }
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
            if (formatStringTok && formatStringTok->tokType() == Token::eString) {
                argListTok = formatStringTok->nextArgument(); // Find third parameter (first argument of va_args)
                formatString = formatStringTok->strValue();
            }
        } else if (Token::Match(&tok, "snprintf|fnprintf|swprintf") && secondParam) {
            const Token* formatStringTok = secondParam->nextArgument(); // Find third parameter (format string)
            if (formatStringTok && formatStringTok->tokType() == Token::eString) {
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

                    if ((*i == 'n' || *i == 's' || scan)) {
                        var.push_back(argListTok);
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

namespace {
    const std::set<std::string> stl_stream = make_container< std::set<std::string> >() <<
            "fstream" << "ifstream" << "iostream" << "istream" <<
            "istringstream" << "ofstream" << "ostream" << "ostringstream" <<
            "stringstream" << "wistringstream" << "wostringstream" << "wstringstream";
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
    unknown = false;

    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    if (parent->str() == "." && parent->astOperand2() == tok)
        return isPointerDeRef(parent, unknown);
    const bool firstOperand = parent->astOperand1() == tok;
    while (parent->str() == "(" && (parent->astOperand2() == nullptr && parent->strAt(1) != ")")) { // Skip over casts
        parent = parent->astParent();
        if (!parent)
            return false;
    }

    // Dereferencing pointer..
    if (parent->str() == "*" && !parent->astOperand2() && !Token::Match(parent->tokAt(-2), "sizeof|decltype|typeof"))
        return true;

    // array access
    if (firstOperand && parent->str() == "[" && (!parent->astParent() || parent->astParent()->str() != "&"))
        return true;

    // address of member variable / array element
    const Token *parent2 = parent;
    while (Token::Match(parent2, "[|."))
        parent2 = parent2->astParent();
    if (parent2 != parent && parent2 && parent2->str() == "&" && !parent2->astOperand2())
        return false;

    // read/write member variable
    if (firstOperand && parent->str() == "." && (!parent->astParent() || parent->astParent()->str() != "&")) {
        if (!parent->astParent() || parent->astParent()->str() != "(" || parent->astParent() == tok->previous())
            return true;
        unknown = true;
        return false;
    }

    if (Token::Match(tok, "%name% ("))
        return true;

    if (Token::Match(tok, "%var% = %var% .") &&
        tok->varId() == tok->tokAt(2)->varId())
        return true;

    // std::string dereferences nullpointers
    if (Token::Match(parent->tokAt(-3), "std :: string|wstring (") && tok->strAt(1) == ")")
        return true;
    if (Token::Match(parent->previous(), "%name% (") && tok->strAt(1) == ")") {
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

    if (!_settings->isEnabled("warning"))
        return;

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
            if (Token::Match(tok2, "%var% . %name%")) {
                // Is this variable a pointer?
                const Variable *var = tok2->variable();
                if (!var || !var->isPointer())
                    continue;

                // Variable id for dereferenced variable
                const unsigned int varid(tok2->varId());

                if (Token::Match(tok2->tokAt(-2), "%varid% ?", varid))
                    continue;

                // Check usage of dereferenced variable in the loop..
                for (std::list<Scope*>::const_iterator j = i->nestedList.begin(); j != i->nestedList.end(); ++j) {
                    const Scope* const scope = *j;
                    if (scope->type != Scope::eWhile)
                        continue;

                    // TODO: are there false negatives for "while ( %varid% ||"
                    if (Token::Match(scope->classDef->next(), "( %varid% &&|)", varid)) {
                        // Make sure there is a "break" or "return" inside the loop.
                        // Without the "break" a null pointer could be dereferenced in the
                        // for statement.
                        for (const Token *tok4 = scope->classStart; tok4; tok4 = tok4->next()) {
                            if (tok4 == i->classEnd) {
                                nullPointerError(tok1, var->name(), scope->classDef, false);
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
    const bool printWarnings = _settings->isEnabled("warning");
    const bool printInconclusive = (_settings->inconclusive);

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof|decltype|typeid|typeof (")) {
            tok = tok->next()->link();
            continue;
        }

        const Variable *var = tok->variable();
        if (!var || !var->isPointer() || tok == var->nameToken())
            continue;

        // Can pointer be NULL?
        const ValueFlow::Value *value = tok->getValue(0);
        if (!value)
            continue;

        if (!printInconclusive && value->inconclusive)
            continue;

        // Is pointer used as function parameter?
        if (Token::Match(tok->previous(), "[(,] %name% [,)]")) {
            const Token *ftok = tok->previous();
            while (ftok && ftok->str() != "(") {
                if (ftok->str() == ")")
                    ftok = ftok->link();
                ftok = ftok->previous();
            }
            if (!ftok || !ftok->previous())
                continue;
            std::list<const Token *> varlist;
            parseFunctionCall(*ftok->previous(), varlist, &_settings->library);
            if (std::find(varlist.begin(), varlist.end(), tok) != varlist.end()) {
                if (value->condition == nullptr)
                    nullPointerError(tok, tok->str(), false, value->defaultArg, !value->isKnown());
                else if (printWarnings)
                    nullPointerError(tok, tok->str(), value->condition, value->inconclusive);
            }
            continue;
        }

        // Pointer dereference.
        bool unknown = false;
        if (!isPointerDeRef(tok,unknown)) {
            if (printInconclusive && unknown) {
                if (value->condition == nullptr)
                    nullPointerError(tok, tok->str(), true, value->defaultArg, !value->isKnown());
                else
                    nullPointerError(tok, tok->str(), value->condition, true);
            }
            continue;
        }

        if (value->condition == nullptr)
            nullPointerError(tok, tok->str(), value->inconclusive, value->defaultArg, !value->isKnown());
        else if (printWarnings)
            nullPointerError(tok, tok->str(), value->condition, value->inconclusive);
    }
}

void CheckNullPointer::nullPointer()
{
    nullPointerLinkedList();
    nullPointerByDeRefAndChec();
}

namespace {
    const std::set<std::string> stl_istream = make_container< std::set<std::string> >() <<
            "fstream" << "ifstream" << "iostream" << "istream" <<
            "istringstream" << "stringstream" << "wistringstream" << "wstringstream";
}

/** Dereferencing null constant (simplified token list) */
void CheckNullPointer::nullConstantDereference()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (scope->function == 0 || !scope->function->hasBody()) // We only look for functions with a body
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

            else if (Token::Match(tok->previous(), "!!. %name% (") && (tok->previous()->str() != "::" || tok->strAt(-2) == "std")) {
                if (Token::Match(tok->tokAt(2), "0|NULL|nullptr )") && tok->varId()) { // constructor call
                    const Variable *var = tok->variable();
                    if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
                        nullPointerError(tok);
                } else { // function call
                    std::list<const Token *> var;
                    parseFunctionCall(*tok, var, &_settings->library);

                    // is one of the var items a NULL pointer?
                    for (std::list<const Token *>::const_iterator it = var.begin(); it != var.end(); ++it) {
                        if (Token::Match(*it, "0|NULL|nullptr [,)]")) {
                            nullPointerError(*it);
                        }
                    }
                }
            } else if (Token::Match(tok, "std :: string|wstring ( 0|NULL|nullptr )"))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), ">> 0|NULL|nullptr")) { // Only checking input stream operations is safe here, because otherwise 0 can be an integer as well
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
                    if (var && var->isStlType(stl_istream))
                        nullPointerError(tok);
                }
            }

            const Variable *ovar = nullptr;
            const Token *tokNull = nullptr;
            if (Token::Match(tok, "0|NULL|nullptr ==|!=|>|>=|<|<= %var%")) {
                if (!Token::Match(tok->tokAt(3),".|[")) {
                    ovar = tok->tokAt(2)->variable();
                    tokNull = tok;
                }
            } else if (Token::Match(tok, "%var% ==|!=|>|>=|<|<= 0|NULL|nullptr") ||
                       Token::Match(tok, "%var% =|+ 0|NULL|nullptr )|]|,|;|+")) {
                ovar = tok->variable();
                tokNull = tok->tokAt(2);
            }
            if (ovar && !ovar->isPointer() && !ovar->isArray() && ovar->isStlStringType() && tokNull && tokNull->originalName() != "'\\0'")
                nullPointerError(tokNull);
        }
    }
}

void CheckNullPointer::nullPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "nullPointer", "Null pointer dereference", CWE476, false);
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, bool inconclusive, bool defaultArg, bool possible)
{
    if (defaultArg) {
        if (_settings->isEnabled("warning"))
            reportError(tok, Severity::warning, "nullPointerDefaultArg", "Possible null pointer dereference if the default parameter value is used: " + varname, CWE476, inconclusive);
    } else if (possible) {
        if (_settings->isEnabled("warning"))
            reportError(tok, Severity::warning, "nullPointer", "Possible null pointer dereference: " + varname, CWE476, inconclusive);
    } else
        reportError(tok, Severity::error, "nullPointer", "Null pointer dereference: " + varname, CWE476, inconclusive);
}

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, const Token* nullCheck, bool inconclusive)
{
    if (! _settings->isEnabled("warning"))
        return;
    std::list<const Token*> callstack;
    callstack.push_back(tok);
    callstack.push_back(nullCheck);
    const std::string errmsg(ValueFlow::eitherTheConditionIsRedundant(nullCheck) + " or there is possible null pointer dereference: " + varname + ".");
    reportError(callstack, Severity::warning, "nullPointerRedundantCheck", errmsg, CWE476, inconclusive);
}

void CheckNullPointer::arithmetic()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->astOperand2() || tok->str() != "-")
                continue;
            // pointer subtraction
            if (!tok->valueType() || !tok->valueType()->pointer)
                continue;
            // Can LHS be NULL?
            const ValueFlow::Value *value = tok->astOperand1()->getValue(0);
            if (!value)
                continue;
            if (!_settings->inconclusive && value->inconclusive)
                continue;
            if (value->condition && !_settings->isEnabled("warning"))
                continue;
            arithmeticError(tok,value);
        }
    }
}

void CheckNullPointer::arithmeticError(const Token *tok, const ValueFlow::Value *value)
{
    std::string errmsg;
    if (value && value->condition)
        errmsg = ValueFlow::eitherTheConditionIsRedundant(value->condition) + " or there is overflow in pointer subtraction.";
    else
        errmsg = "Overflow in pointer arithmetic, NULL pointer is subtracted.";

    std::list<const Token*> callstack;
    callstack.push_back(tok);
    if (value && value->condition)
        callstack.push_back(value->condition);

    reportError(callstack,
                (value && value->condition) ? Severity::warning : Severity::error,
                (value && value->condition) ? "nullPointerArithmeticRedundantCheck" : "nullPointerArithmetic",
                errmsg,
                CWE682, // unknown - pointer overflow
                value && value->inconclusive);
}

