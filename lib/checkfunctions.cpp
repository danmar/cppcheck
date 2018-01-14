/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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
// Check functions
//---------------------------------------------------------------------------

#include "checkfunctions.h"

#include "astutils.h"
#include "mathlib.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <cmath>
#include <cstddef>
#include <ostream>
#include <vector>

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckFunctions instance;
}

static const CWE CWE252(252U);  // Unchecked Return Value
static const CWE CWE477(477U);  // Use of Obsolete Functions
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE628(628U);  // Function Call with Incorrectly Specified Arguments
static const CWE CWE686(686U);  // Function Call With Incorrect Argument Type
static const CWE CWE687(687U);  // Function Call With Incorrectly Specified Argument Value
static const CWE CWE688(688U);  // Function Call With Incorrect Variable or Reference as Argument

void CheckFunctions::checkProhibitedFunctions()
{
    const bool checkAlloca = _settings->isEnabled(Settings::WARNING) && ((_settings->standards.c >= Standards::C99 && _tokenizer->isC()) || _settings->standards.cpp >= Standards::CPP11);

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% (") && tok->varId() == 0)
                continue;
            // alloca() is special as it depends on the code being C or C++, so it is not in Library
            if (checkAlloca && Token::simpleMatch(tok, "alloca (") && (!tok->function() || tok->function()->nestedIn->type == Scope::eGlobal)) {
                if (_tokenizer->isC()) {
                    if (_settings->standards.c > Standards::C89)
                        reportError(tok, Severity::warning, "allocaCalled",
                                    "Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n"
                                    "The obsolete function 'alloca' is called. In C99 and later it is recommended to use a variable length array or "
                                    "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                    "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
                } else
                    reportError(tok, Severity::warning, "allocaCalled",
                                "Obsolete function 'alloca' called.\n"
                                "The obsolete function 'alloca' is called. In C++11 and later it is recommended to use std::array<> or "
                                "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
            } else {
                if (tok->function() && tok->function()->hasBody())
                    continue;

                const Library::WarnInfo* wi = _settings->library.getWarnInfo(tok);
                if (wi) {
                    if (_settings->isEnabled(wi->severity) && _settings->standards.c >= wi->standards.c && _settings->standards.cpp >= wi->standards.cpp) {
                        reportError(tok, wi->severity, tok->str() + "Called", wi->message, CWE477, false);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// Check <valid> and <not-bool>
//---------------------------------------------------------------------------
void CheckFunctions::invalidFunctionUsage()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% ( !!)"))
                continue;
            const Token * const functionToken = tok;
            const std::vector<const Token *> arguments = getArguments(tok);
            for (unsigned int argnr = 1; argnr <= arguments.size(); ++argnr) {
                const Token * const argtok = arguments[argnr-1];

                // check <valid>...</valid>
                const ValueFlow::Value *invalidValue = argtok->getInvalidValue(functionToken,argnr,_settings);
                if (invalidValue) {
                    invalidFunctionArgError(argtok, functionToken->next()->astOperand1()->expressionString(), argnr, invalidValue, _settings->library.validarg(functionToken, argnr));
                }

                if (astIsBool(argtok)) {
                    // check <not-bool>
                    if (_settings->library.isboolargbad(functionToken, argnr))
                        invalidFunctionArgBoolError(argtok, functionToken->str(), argnr);

                    // Are the values 0 and 1 valid?
                    else if (!_settings->library.isargvalid(functionToken, argnr, 0))
                        invalidFunctionArgError(argtok, functionToken->str(), argnr, nullptr, _settings->library.validarg(functionToken, argnr));
                    else if (!_settings->library.isargvalid(functionToken, argnr, 1))
                        invalidFunctionArgError(argtok, functionToken->str(), argnr, nullptr, _settings->library.validarg(functionToken, argnr));
                }
            }
        }
    }
}

void CheckFunctions::invalidFunctionArgError(const Token *tok, const std::string &functionName, int argnr, const ValueFlow::Value *invalidValue, const std::string &validstr)
{
    std::ostringstream errmsg;
    if (invalidValue && invalidValue->condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(invalidValue->condition)
               << " or " << functionName << "() argument nr " << argnr
               << " can have invalid value.";
    else
        errmsg << "Invalid " << functionName << "() argument nr " << argnr << '.';
    if (invalidValue)
        errmsg << " The value is " << invalidValue->intvalue << " but the valid values are '" << validstr << "'.";
    else
        errmsg << " The value is 0 or 1 (boolean) but the valid values are '" << validstr << "'.";
    if (invalidValue)
        reportError(getErrorPath(tok, invalidValue, "Invalid argument"),
                    invalidValue->errorSeverity() ? Severity::error : Severity::warning,
                    "invalidFunctionArg",
                    errmsg.str(),
                    CWE628,
                    invalidValue->isInconclusive());
    else
        reportError(tok,
                    Severity::error,
                    "invalidFunctionArg",
                    errmsg.str(),
                    CWE628,
                    false);
}

void CheckFunctions::invalidFunctionArgBoolError(const Token *tok, const std::string &functionName, int argnr)
{
    std::ostringstream errmsg;
    errmsg << "Invalid " << functionName << "() argument nr " << argnr << ". A non-boolean value is required.";
    reportError(tok, Severity::error, "invalidFunctionArgBool", errmsg.str(), CWE628, false);
}

//---------------------------------------------------------------------------
// Check for ignored return values.
//---------------------------------------------------------------------------
void CheckFunctions::checkIgnoredReturnValue()
{
    if (!_settings->isEnabled(Settings::WARNING))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // skip c++11 initialization, ({...})
            if (Token::Match(tok, "%var%|(|, {"))
                tok = tok->linkAt(1);
            else if (Token::Match(tok, "[(<]") && tok->link())
                tok = tok->link();

            if (tok->varId() || !Token::Match(tok, "%name% ("))
                continue;

            if (tok->next()->astParent())
                continue;

            if (!tok->scope()->isExecutable()) {
                tok = tok->scope()->classEnd;
                continue;
            }

            if ((!tok->function() || !Token::Match(tok->function()->retDef, "void %name%")) && _settings->library.isUseRetVal(tok) && !WRONG_DATA(!tok->next()->astOperand1(), tok))
                ignoredReturnValueError(tok, tok->next()->astOperand1()->expressionString());
        }
    }
}

void CheckFunctions::ignoredReturnValueError(const Token* tok, const std::string& function)
{
    reportError(tok, Severity::warning, "ignoredReturnValue",
                "Return value of function " + function + "() is not used.", CWE252, false);
}


//---------------------------------------------------------------------------
// Detect passing wrong values to <cmath> functions like atan(0, x);
//---------------------------------------------------------------------------
void CheckFunctions::checkMathFunctions()
{
    const bool styleC99 = _settings->isEnabled(Settings::STYLE) && _settings->standards.c != Standards::C89 && _settings->standards.cpp != Standards::CPP03;
    const bool printWarnings = _settings->isEnabled(Settings::WARNING);

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->varId())
                continue;
            if (printWarnings && Token::Match(tok, "%name% ( !!)")) {
                if (tok->strAt(-1) != "."
                    && Token::Match(tok, "log|logf|logl|log10|log10f|log10l ( %num% )")) {
                    const std::string& number = tok->strAt(2);
                    bool isNegative = MathLib::isNegative(number);
                    bool isInt = MathLib::isInt(number);
                    bool isFloat = MathLib::isFloat(number);
                    if (isNegative && isInt && MathLib::toLongNumber(number) <= 0) {
                        mathfunctionCallWarning(tok); // case log(-2)
                    } else if (isNegative && isFloat && MathLib::toDoubleNumber(number) <= 0.) {
                        mathfunctionCallWarning(tok); // case log(-2.0)
                    } else if (!isNegative && isFloat && MathLib::toDoubleNumber(number) <= 0.) {
                        mathfunctionCallWarning(tok); // case log(0.0)
                    } else if (!isNegative && isInt && MathLib::toLongNumber(number) <= 0) {
                        mathfunctionCallWarning(tok); // case log(0)
                    }
                }

                // acos( x ), asin( x )  where x is defined for interval [-1,+1], but not beyond
                else if (Token::Match(tok, "acos|acosl|acosf|asin|asinf|asinl ( %num% )")) {
                    if (std::fabs(MathLib::toDoubleNumber(tok->strAt(2))) > 1.0)
                        mathfunctionCallWarning(tok);
                }
                // sqrt( x ): if x is negative the result is undefined
                else if (Token::Match(tok, "sqrt|sqrtf|sqrtl ( %num% )")) {
                    if (MathLib::isNegative(tok->strAt(2)))
                        mathfunctionCallWarning(tok);
                }
                // atan2 ( x , y): x and y can not be zero, because this is mathematically not defined
                else if (Token::Match(tok, "atan2|atan2f|atan2l ( %num% , %num% )")) {
                    if (MathLib::isNullValue(tok->strAt(2)) && MathLib::isNullValue(tok->strAt(4)))
                        mathfunctionCallWarning(tok, 2);
                }
                // fmod ( x , y) If y is zero, then either a range error will occur or the function will return zero (implementation-defined).
                else if (Token::Match(tok, "fmod|fmodf|fmodl (")) {
                    const Token* nextArg = tok->tokAt(2)->nextArgument();
                    if (nextArg && nextArg->isNumber() && MathLib::isNullValue(nextArg->str()))
                        mathfunctionCallWarning(tok, 2);
                }
                // pow ( x , y) If x is zero, and y is negative --> division by zero
                else if (Token::Match(tok, "pow|powf|powl ( %num% , %num% )")) {
                    if (MathLib::isNullValue(tok->strAt(2)) && MathLib::isNegative(tok->strAt(4)))
                        mathfunctionCallWarning(tok, 2);
                }
            }

            if (styleC99) {
                if (Token::Match(tok, "%num% - erf (") && Tokenizer::isOneNumber(tok->str()) && tok->next()->astOperand2() == tok->tokAt(3)) {
                    mathfunctionCallWarning(tok, "1 - erf(x)", "erfc(x)");
                } else if (Token::simpleMatch(tok, "exp (") && Token::Match(tok->linkAt(1), ") - %num%") && Tokenizer::isOneNumber(tok->linkAt(1)->strAt(2)) && tok->linkAt(1)->next()->astOperand1() == tok->next()) {
                    mathfunctionCallWarning(tok, "exp(x) - 1", "expm1(x)");
                } else if (Token::simpleMatch(tok, "log (") && tok->next()->astOperand2()) {
                    const Token* plus = tok->next()->astOperand2();
                    if (plus->str() == "+" && ((plus->astOperand1() && Tokenizer::isOneNumber(plus->astOperand1()->str())) || (plus->astOperand2() && Tokenizer::isOneNumber(plus->astOperand2()->str()))))
                        mathfunctionCallWarning(tok, "log(1 + x)", "log1p(x)");
                }
            }
        }
    }
}

void CheckFunctions::mathfunctionCallWarning(const Token *tok, const unsigned int numParam)
{
    if (tok) {
        if (numParam == 1)
            reportError(tok, Severity::warning, "wrongmathcall", "Passing value " + tok->strAt(2) + " to " + tok->str() + "() leads to implementation-defined result.", CWE758, false);
        else if (numParam == 2)
            reportError(tok, Severity::warning, "wrongmathcall", "Passing values " + tok->strAt(2) + " and " + tok->strAt(4) + " to " + tok->str() + "() leads to implementation-defined result.", CWE758, false);
    } else
        reportError(tok, Severity::warning, "wrongmathcall", "Passing value '#' to #() leads to implementation-defined result.", CWE758, false);
}

void CheckFunctions::mathfunctionCallWarning(const Token *tok, const std::string& oldexp, const std::string& newexp)
{
    reportError(tok, Severity::style, "unpreciseMathCall", "Expression '" + oldexp + "' can be replaced by '" + newexp + "' to avoid loss of precision.", CWE758, false);
}

//---------------------------------------------------------------------------
// memset(p, y, 0 /* bytes to fill */) <- 2nd and 3rd arguments inverted
//---------------------------------------------------------------------------
void CheckFunctions::memsetZeroBytes()
{
// FIXME:
//  Replace this with library configuration.
//  For instance:
//     <arg nr="3">
//       <warn knownIntValue="0" severity="warning" msg="..."/>
//     </arg>

    if (!_settings->isEnabled(Settings::WARNING))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "memset|wmemset (") && (numberOfArguments(tok)==3)) {
                const std::vector<const Token *> &arguments = getArguments(tok);
                if (WRONG_DATA(arguments.size() != 3U, tok))
                    continue;
                const Token* lastParamTok = arguments[2];
                if (lastParamTok->str() == "0")
                    memsetZeroBytesError(tok);
            }
        }
    }
}

void CheckFunctions::memsetZeroBytesError(const Token *tok)
{
    const std::string summary("memset() called to fill 0 bytes.");
    const std::string verbose(summary + " The second and third arguments might be inverted."
                              " The function memset ( void * ptr, int value, size_t num ) sets the"
                              " first num bytes of the block of memory pointed by ptr to the specified value.");
    reportError(tok, Severity::warning, "memsetZeroBytes", summary + "\n" + verbose, CWE687, false);
}

void CheckFunctions::memsetInvalid2ndParam()
{
// FIXME:
//  Replace this with library configuration.
//  For instance:
//     <arg nr="2">
//       <not-float/>
//       <warn possibleIntValue=":-129,256:" severity="warning" msg="..."/>
//     </arg>

    const bool printPortability = _settings->isEnabled(Settings::PORTABILITY);
    const bool printWarning = _settings->isEnabled(Settings::WARNING);
    if (!printWarning && !printPortability)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok && (tok != scope->classEnd); tok = tok->next()) {
            if (!Token::simpleMatch(tok, "memset ("))
                continue;

            const std::vector<const Token *> args = getArguments(tok);
            if (args.size() != 3)
                continue;

            // Second parameter is zero literal, i.e. 0.0f
            const Token * const secondParamTok = args[1];
            if (Token::Match(secondParamTok, "%num% ,") && MathLib::isNullValue(secondParamTok->str()))
                continue;

            // Check if second parameter is a float variable or a float literal != 0.0f
            if (printPortability && astIsFloat(secondParamTok,false)) {
                memsetFloatError(secondParamTok, secondParamTok->expressionString());
            }

            if (printWarning && secondParamTok->isNumber()) { // Check if the second parameter is a literal and is out of range
                const long long int value = MathLib::toLongNumber(secondParamTok->str());
                const long long sCharMin = _settings->signedCharMin();
                const long long uCharMax = _settings->unsignedCharMax();
                if (value < sCharMin || value > uCharMax)
                    memsetValueOutOfRangeError(secondParamTok, secondParamTok->str());
            }
        }
    }
}

void CheckFunctions::memsetFloatError(const Token *tok, const std::string &var_value)
{
    const std::string message("The 2nd memset() argument '" + var_value +
                              "' is a float, its representation is implementation defined.");
    const std::string verbose(message + " memset() is used to set each byte of a block of memory to a specific value and"
                              " the actual representation of a floating-point value is implementation defined.");
    reportError(tok, Severity::portability, "memsetFloat", message + "\n" + verbose, CWE688, false);
}

void CheckFunctions::memsetValueOutOfRangeError(const Token *tok, const std::string &value)
{
    const std::string message("The 2nd memset() argument '" + value + "' doesn't fit into an 'unsigned char'.");
    const std::string verbose(message + " The 2nd parameter is passed as an 'int', but the function fills the block of memory using the 'unsigned char' conversion of this value.");
    reportError(tok, Severity::warning, "memsetValueOutOfRange", message + "\n" + verbose, CWE686, false);
}

//---------------------------------------------------------------------------
// --check-library => warn for unconfigured functions
//---------------------------------------------------------------------------

void CheckFunctions::checkLibraryMatchFunctions()
{
    if (!_settings->checkLibrary || !_settings->isEnabled(Settings::INFORMATION))
        return;

    bool New = false;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->scope() || !tok->scope()->isExecutable())
            continue;

        if (tok->str() == "new")
            New = true;
        else if (tok->str() == ";")
            New = false;
        else if (New)
            continue;

        if (!Token::Match(tok, "%name% (") || Token::Match(tok, "asm|sizeof|catch"))
            continue;

        if (tok->varId() != 0 || tok->type() || tok->isStandardType() || tok->isControlFlowKeyword())
            continue;

        if (tok->linkAt(1)->strAt(1) == "(")
            continue;

        if (!_settings->library.isNotLibraryFunction(tok))
            continue;

        const std::string &functionName = _settings->library.getFunctionName(tok);
        if (functionName.empty() || _settings->library.functions.find(functionName) != _settings->library.functions.end())
            continue;

        reportError(tok,
                    Severity::information,
                    "checkLibraryFunction",
                    "--check-library: There is no matching configuration for function " + functionName + "()");
    }
}
