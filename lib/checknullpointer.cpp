/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "astutils.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "valueflow.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <vector>

//---------------------------------------------------------------------------

// CWE ids used:
static const struct CWE CWE_NULL_POINTER_DEREFERENCE(476U);
static const struct CWE CWE_INCORRECT_CALCULATION(682U);

// Register this check class (by creating a static instance of it)
namespace {
    CheckNullPointer instance;
}

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
 */
void CheckNullPointer::parseFunctionCall(const Token &tok, std::list<const Token *> &var, const Library *library)
{
    if (Token::Match(&tok, "%name% ( )") || !tok.tokAt(2))
        return;

    const std::vector<const Token *> args = getArguments(&tok);

    if (library || tok.function() != nullptr) {
        for (int argnr = 1; argnr <= args.size(); ++argnr) {
            const Token *param = args[argnr - 1];
            if (library && library->isnullargbad(&tok, argnr) && checkNullpointerFunctionCallPlausibility(tok.function(), argnr))
                var.push_back(param);
            else if (tok.function()) {
                const Variable* argVar = tok.function()->getArgumentVar(argnr-1);
                if (argVar && argVar->isStlStringType() && !argVar->isArrayOrPointer())
                    var.push_back(param);
            }
        }
    }

    if (library && library->formatstr_function(&tok)) {
        const int formatStringArgNr = library->formatstr_argno(&tok);
        if (formatStringArgNr < 0 || formatStringArgNr >= args.size())
            return;

        // 1st parameter..
        if (Token::Match(&tok, "snprintf|vsnprintf|fnprintf|vfnprintf") && args.size() > 1 && !(args[1] && args[1]->hasKnownIntValue() && args[1]->getKnownIntValue() == 0)) // Only if length (second parameter) is not zero
            var.push_back(args[0]);

        if (args[formatStringArgNr]->tokType() != Token::eString)
            return;
        const std::string &formatString = args[formatStringArgNr]->strValue();
        int argnr = formatStringArgNr + 1;
        const bool scan = library->formatstr_scan(&tok);

        bool percent = false;
        for (std::string::const_iterator i = formatString.cbegin(); i != formatString.cend(); ++i) {
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
                            argnr++;
                    }
                    ++i;
                    if (i == formatString.end())
                        return;
                }
                if (_continue)
                    continue;

                if (argnr < args.size() && (*i == 'n' || *i == 's' || scan))
                    var.push_back(args[argnr]);

                if (*i != 'm') // %m is a non-standard glibc extension that requires no parameter
                    argnr++;
            }
        }
    }
}

namespace {
    const std::set<std::string> stl_stream = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "ofstream", "ostream", "ostringstream",
        "stringstream", "wistringstream", "wostringstream", "wstringstream"
    };
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
bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown) const
{
    return isPointerDeRef(tok, unknown, mSettings);
}

bool CheckNullPointer::isPointerDeRef(const Token *tok, bool &unknown, const Settings *settings)
{
    unknown = false;

    // Is pointer used as function parameter?
    if (Token::Match(tok->previous(), "[(,] %name% [,)]") && settings) {
        const Token *ftok = tok->previous();
        while (ftok && ftok->str() != "(") {
            if (ftok->str() == ")")
                ftok = ftok->link();
            ftok = ftok->previous();
        }
        if (ftok && ftok->previous()) {
            std::list<const Token *> varlist;
            parseFunctionCall(*ftok->previous(), varlist, &settings->library);
            if (std::find(varlist.cbegin(), varlist.cend(), tok) != varlist.cend()) {
                return true;
            }
        }
    }

    if (tok->str() == "(" && !tok->scope()->isExecutable())
        return false;

    const Token* parent = tok->astParent();
    if (!parent)
        return false;
    const bool addressOf = parent->astParent() && parent->astParent()->str() == "&";
    if (parent->str() == "." && astIsRHS(tok))
        return isPointerDeRef(parent, unknown, settings);
    const bool firstOperand = parent->astOperand1() == tok;
    parent = astParentSkipParens(tok);
    if (!parent)
        return false;

    // Dereferencing pointer..
    const Token* grandParent = parent->astParent();
    if (parent->isUnaryOp("*") && !(grandParent && isUnevaluated(grandParent->previous()))) {
        // declaration of function pointer
        if (tok->variable() && tok->variable()->nameToken() == tok)
            return false;
        if (!addressOf)
            return true;
    }

    // array access
    if (firstOperand && parent->str() == "[" && !addressOf)
        return true;

    // address of member variable / array element
    const Token *parent2 = parent;
    while (Token::Match(parent2, "[|."))
        parent2 = parent2->astParent();
    if (parent2 != parent && parent2 && parent2->isUnaryOp("&"))
        return false;

    // read/write member variable
    if (firstOperand && parent->originalName() == "->" && !addressOf)
        return true;

    // If its a function pointer then check if its called
    if (tok->variable() && tok->variable()->isPointer() && Token::Match(tok->variable()->nameToken(), "%name% ) (") &&
        Token::Match(tok, "%name% ("))
        return true;

    if (Token::Match(tok, "%var% = %var% .") &&
        tok->varId() == tok->tokAt(2)->varId())
        return true;

    // std::string dereferences nullpointers
    if (Token::Match(parent->tokAt(-3), "std :: string|wstring (|{ %name% )|}"))
        return true;
    if (Token::Match(parent->previous(), "%name% (|{ %name% )|}")) {
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


static bool isNullablePointer(const Token* tok, const Settings* settings)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "new") && tok->varId() == 0)
        return false;
    if (astIsPointer(tok))
        return true;
    if (astIsSmartPointer(tok))
        return true;
    if (Token::simpleMatch(tok, "."))
        return isNullablePointer(tok->astOperand2(), settings);
    if (const Variable* var = tok->variable()) {
        return (var->isPointer() || var->isSmartPointer());
    }
    return false;
}

void CheckNullPointer::nullPointerByDeRefAndChec()
{
    const bool printInconclusive = (mSettings->certainty.isEnabled(Certainty::inconclusive));

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (isUnevaluated(tok)) {
            tok = tok->next()->link();
            continue;
        }

        if (Token::Match(tok, "%num%|%char%|%str%"))
            continue;

        if (!isNullablePointer(tok, mSettings) ||
            (tok->str() == "." && isNullablePointer(tok->astOperand2(), mSettings) && tok->astOperand2()->getValue(0))) // avoid duplicate warning
            continue;

        // Can pointer be NULL?
        const ValueFlow::Value *value = tok->getValue(0);
        if (!value)
            continue;

        if (!printInconclusive && value->isInconclusive())
            continue;

        // Pointer dereference.
        bool unknown = false;
        if (!isPointerDeRef(tok,unknown)) {
            if (unknown)
                nullPointerError(tok, tok->expressionString(), value, true);
            continue;
        }

        nullPointerError(tok, tok->expressionString(), value, value->isInconclusive());
    }
}

void CheckNullPointer::nullPointer()
{
    logChecker("CheckNullPointer::nullPointer");
    nullPointerByDeRefAndChec();
}

namespace {
    const std::set<std::string> stl_istream = {
        "fstream", "ifstream", "iostream", "istream",
        "istringstream", "stringstream", "wistringstream", "wstringstream"
    };
}

/** Dereferencing null constant (simplified token list) */
void CheckNullPointer::nullConstantDereference()
{
    logChecker("CheckNullPointer::nullConstantDereference");

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        if (scope->function == nullptr || !scope->function->hasBody()) // We only look for functions with a body
            continue;

        const Token *tok = scope->bodyStart;

        if (scope->function->isConstructor())
            tok = scope->function->token; // Check initialization list

        for (; tok != scope->bodyEnd; tok = tok->next()) {
            if (isUnevaluated(tok))
                tok = tok->next()->link();

            else if (Token::simpleMatch(tok, "* 0")) {
                if (Token::Match(tok->previous(), "return|throw|;|{|}|:|[|(|,") || tok->previous()->isOp()) {
                    nullPointerError(tok);
                }
            }

            else if (Token::Match(tok, "0 [") && (tok->previous()->str() != "&" || !Token::Match(tok->next()->link()->next(), "[.(]")))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), "!!. %name% (|{") && (tok->previous()->str() != "::" || tok->strAt(-2) == "std")) {
                if (Token::Match(tok->tokAt(2), "0|NULL|nullptr )|}") && tok->varId()) { // constructor call
                    const Variable *var = tok->variable();
                    if (var && !var->isPointer() && !var->isArray() && var->isStlStringType())
                        nullPointerError(tok);
                } else { // function call
                    std::list<const Token *> var;
                    parseFunctionCall(*tok, var, &mSettings->library);

                    // is one of the var items a NULL pointer?
                    for (const Token *vartok : var) {
                        if (vartok->hasKnownIntValue() && vartok->getKnownIntValue() == 0)
                            nullPointerError(vartok);
                    }
                }
            } else if (Token::Match(tok, "std :: string|wstring ( 0|NULL|nullptr )"))
                nullPointerError(tok);

            else if (Token::Match(tok->previous(), "::|. %name% (")) {
                const std::vector<const Token *> &args = getArguments(tok);
                for (int argnr = 0; argnr < args.size(); ++argnr) {
                    const Token *argtok = args[argnr];
                    if (!argtok->hasKnownIntValue())
                        continue;
                    if (argtok->values().front().intvalue != 0)
                        continue;
                    if (mSettings->library.isnullargbad(tok, argnr+1))
                        nullPointerError(argtok);
                }
            }

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

void CheckNullPointer::nullPointerError(const Token *tok, const std::string &varname, const ValueFlow::Value *value, bool inconclusive)
{
    const std::string errmsgcond("$symbol:" + varname + '\n' + ValueFlow::eitherTheConditionIsRedundant(value ? value->condition : nullptr) + " or there is possible null pointer dereference: $symbol.");
    const std::string errmsgdefarg("$symbol:" + varname + "\nPossible null pointer dereference if the default parameter value is used: $symbol");

    if (!tok) {
        reportError(tok, Severity::error, "nullPointer", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerDefaultArg", errmsgdefarg, CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        reportError(tok, Severity::warning, "nullPointerRedundantCheck", errmsgcond, CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
        return;
    }

    if (!value) {
        reportError(tok, Severity::error, "nullPointer", "Null pointer dereference", CWE_NULL_POINTER_DEREFERENCE, inconclusive ? Certainty::inconclusive : Certainty::normal);
        return;
    }

    if (!mSettings->isEnabled(value, inconclusive))
        return;

    const ErrorPath errorPath = getErrorPath(tok, value, "Null pointer dereference");

    if (value->condition) {
        reportError(errorPath, Severity::warning, "nullPointerRedundantCheck", errmsgcond, CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    } else if (value->defaultArg) {
        reportError(errorPath, Severity::warning, "nullPointerDefaultArg", errmsgdefarg, CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    } else {
        std::string errmsg;
        errmsg = std::string(value->isKnown() ? "Null" : "Possible null") + " pointer dereference";
        if (!varname.empty())
            errmsg = "$symbol:" + varname + '\n' + errmsg + ": $symbol";

        reportError(errorPath,
                    value->isKnown() ? Severity::error : Severity::warning,
                    "nullPointer",
                    errmsg,
                    CWE_NULL_POINTER_DEREFERENCE, inconclusive || value->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    }
}

void CheckNullPointer::arithmetic()
{
    logChecker("CheckNullPointer::aithmetic");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "-|+|+=|-=|++|--"))
                continue;
            const Token *pointerOperand;
            const Token *numericOperand;
            if (tok->astOperand1() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->pointer != 0) {
                pointerOperand = tok->astOperand1();
                numericOperand = tok->astOperand2();
            } else if (tok->astOperand2() && tok->astOperand2()->valueType() && tok->astOperand2()->valueType()->pointer != 0) {
                pointerOperand = tok->astOperand2();
                numericOperand = tok->astOperand1();
            } else
                continue;
            if (numericOperand && numericOperand->valueType() && !numericOperand->valueType()->isIntegral())
                continue;
            const ValueFlow::Value* numValue = numericOperand ? numericOperand->getValue(0) : nullptr;
            if (numValue && numValue->intvalue == 0) // don't warn for arithmetic with 0
                continue;
            const ValueFlow::Value* value = pointerOperand->getValue(0);
            if (!value)
                continue;
            if (!mSettings->certainty.isEnabled(Certainty::inconclusive) && value->isInconclusive())
                continue;
            if (value->condition && !mSettings->severity.isEnabled(Severity::warning))
                continue;
            if (value->condition)
                redundantConditionWarning(tok, value, value->condition, value->isInconclusive());
            else
                pointerArithmeticError(tok, value, value->isInconclusive());
        }
    }
}

static std::string arithmeticTypeString(const Token *tok)
{
    if (tok && tok->str()[0] == '-')
        return "subtraction";
    if (tok && tok->str()[0] == '+')
        return "addition";
    return "arithmetic";
}

void CheckNullPointer::pointerArithmeticError(const Token* tok, const ValueFlow::Value *value, bool inconclusive)
{
    std::string arithmetic = arithmeticTypeString(tok);
    std::string errmsg;
    if (tok && tok->str()[0] == '-') {
        errmsg = "Overflow in pointer arithmetic, NULL pointer is subtracted.";
    } else {
        errmsg = "Pointer " + arithmetic + " with NULL pointer.";
    }
    const ErrorPath errorPath = getErrorPath(tok, value, "Null pointer " + arithmetic);
    reportError(errorPath,
                Severity::error,
                "nullPointerArithmetic",
                errmsg,
                CWE_INCORRECT_CALCULATION,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

void CheckNullPointer::redundantConditionWarning(const Token* tok, const ValueFlow::Value *value, const Token *condition, bool inconclusive)
{
    std::string arithmetic = arithmeticTypeString(tok);
    std::string errmsg;
    if (tok && tok->str()[0] == '-') {
        errmsg = ValueFlow::eitherTheConditionIsRedundant(condition) + " or there is overflow in pointer " + arithmetic + ".";
    } else {
        errmsg = ValueFlow::eitherTheConditionIsRedundant(condition) + " or there is pointer arithmetic with NULL pointer.";
    }
    const ErrorPath errorPath = getErrorPath(tok, value, "Null pointer " + arithmetic);
    reportError(errorPath,
                Severity::warning,
                "nullPointerArithmeticRedundantCheck",
                errmsg,
                CWE_INCORRECT_CALCULATION,
                inconclusive ? Certainty::inconclusive : Certainty::normal);
}

std::string CheckNullPointer::MyFileInfo::toString() const
{
    return CTU::toString(unsafeUsage);
}

// NOLINTNEXTLINE(readability-non-const-parameter) - used as callback so we need to preserve the signature
static bool isUnsafeUsage(const Check *check, const Token *vartok, MathLib::bigint *value)
{
    (void)value;
    const CheckNullPointer *checkNullPointer = dynamic_cast<const CheckNullPointer *>(check);
    bool unknown = false;
    return checkNullPointer && checkNullPointer->isPointerDeRef(vartok, unknown);
}

Check::FileInfo *CheckNullPointer::getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const
{
    CheckNullPointer check(tokenizer, settings, nullptr);
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::getUnsafeUsage(tokenizer, settings, &check, ::isUnsafeUsage);
    if (unsafeUsage.empty())
        return nullptr;

    MyFileInfo *fileInfo = new MyFileInfo;
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

Check::FileInfo * CheckNullPointer::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage = CTU::loadUnsafeUsageListFromXml(xmlElement);
    if (unsafeUsage.empty())
        return nullptr;

    MyFileInfo *fileInfo = new MyFileInfo;
    fileInfo->unsafeUsage = unsafeUsage;
    return fileInfo;
}

bool CheckNullPointer::analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    if (!ctu)
        return false;
    bool foundErrors = false;
    (void)settings; // This argument is unused

    CheckNullPointer dummy(nullptr, &settings, &errorLogger);
    dummy.
    logChecker("CheckNullPointer::analyseWholeProgram"); // unusedfunctions

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> callsMap = ctu->getCallsMap();

    for (const Check::FileInfo* fi1 : fileInfo) {
        const MyFileInfo *fi = dynamic_cast<const MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafeUsage) {
            for (int warning = 0; warning <= 1; warning++) {
                if (warning == 1 && !settings.severity.isEnabled(Severity::warning))
                    break;

                const std::list<ErrorMessage::FileLocation> &locationList =
                    CTU::FileInfo::getErrorPath(CTU::FileInfo::InvalidValueType::null,
                                                unsafeUsage,
                                                callsMap,
                                                "Dereferencing argument ARG that is null",
                                                nullptr,
                                                warning);
                if (locationList.empty())
                    continue;

                const ErrorMessage errmsg(locationList,
                                          emptyString,
                                          warning ? Severity::warning : Severity::error,
                                          "Null pointer dereference: " + unsafeUsage.myArgumentName,
                                          "ctunullpointer",
                                          CWE_NULL_POINTER_DEREFERENCE, Certainty::normal);
                errorLogger.reportErr(errmsg);

                foundErrors = true;
                break;
            }
        }
    }

    return foundErrors;
}
