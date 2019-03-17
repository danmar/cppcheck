/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
// Buffer overrun..
//---------------------------------------------------------------------------

#include "checkbufferoverrun.h"

#include "astutils.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"

#include <tinyxml2.h>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stack>
#include <utility>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE131(131U);  // Incorrect Calculation of Buffer Size
static const CWE CWE170(170U);  // Improper Null Termination
static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE682(682U);  // Incorrect Calculation
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE786(786U);  // Access of Memory Location Before Start of Buffer
static const CWE CWE788(788U);  // Access of Memory Location After End of Buffer

//---------------------------------------------------------------------------

static std::vector<Dimension> getDynamicDimensions(const Token *tok, MathLib::bigint typeSize)
{
    if (typeSize == 0) {
        const std::vector<Dimension> dimensions;
        return dimensions;
    }
    for (const ValueFlow::Value &value : tok->values()) {
        if (!value.isBufferSizeValue())
            continue;
        Dimension dim;
        dim.tok = nullptr;
        dim.num = value.intvalue / typeSize;
        dim.known = value.isKnown();
        const std::vector<Dimension> dimensions{dim};
        return dimensions;
    }

    const std::vector<Dimension> dimensions;
    return dimensions;
}

static size_t getMinFormatStringOutputLength(const std::vector<const Token*> &parameters, unsigned int formatStringArgNr)
{
    if (formatStringArgNr == 0 || formatStringArgNr > parameters.size())
        return 0;
    if (parameters[formatStringArgNr - 1]->tokType() != Token::eString)
        return 0;
    const std::string &formatString = parameters[formatStringArgNr - 1]->str();
    bool percentCharFound = false;
    std::size_t outputStringSize = 0;
    bool handleNextParameter = false;
    std::string digits_string;
    bool i_d_x_f_found = false;
    std::size_t parameterLength = 0;
    unsigned int inputArgNr = formatStringArgNr;
    for (std::string::size_type i = 1; i + 1 < formatString.length(); ++i) {
        if (formatString[i] == '\\') {
            if (i < formatString.length() - 1 && formatString[i + 1] == '0')
                break;

            ++outputStringSize;
            ++i;
            continue;
        }

        if (percentCharFound) {
            switch (formatString[i]) {
            case 'f':
            case 'x':
            case 'X':
            case 'i':
                i_d_x_f_found = true;
                handleNextParameter = true;
                parameterLength = 1; // TODO
                break;
            case 'c':
            case 'e':
            case 'E':
            case 'g':
            case 'o':
            case 'u':
            case 'p':
            case 'n':
                handleNextParameter = true;
                parameterLength = 1; // TODO
                break;
            case 'd':
                i_d_x_f_found = true;
                parameterLength = 1;
                if (inputArgNr < parameters.size() && parameters[inputArgNr]->hasKnownIntValue())
                    parameterLength = MathLib::toString(parameters[inputArgNr]->getKnownIntValue()).length();

                handleNextParameter = true;
                break;
            case 's':
                parameterLength = 0;
                if (inputArgNr < parameters.size() && parameters[inputArgNr]->tokType() == Token::eString)
                    parameterLength = Token::getStrLength(parameters[inputArgNr]);

                handleNextParameter = true;
                break;
            }
        }

        if (formatString[i] == '%')
            percentCharFound = !percentCharFound;
        else if (percentCharFound) {
            digits_string.append(1, formatString[i]);
        }

        if (!percentCharFound)
            outputStringSize++;

        if (handleNextParameter) {
            unsigned int tempDigits = static_cast<unsigned int>(std::abs(std::atoi(digits_string.c_str())));
            if (i_d_x_f_found)
                tempDigits = std::max(static_cast<unsigned int>(tempDigits), 1U);

            if (digits_string.find('.') != std::string::npos) {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                const unsigned int maxLen = std::max(static_cast<unsigned int>(std::abs(std::atoi(endStr.c_str()))), 1U);

                if (formatString[i] == 's') {
                    // For strings, the length after the dot "%.2s" will limit
                    // the length of the string.
                    if (parameterLength > maxLen)
                        parameterLength = maxLen;
                } else {
                    // For integers, the length after the dot "%.2d" can
                    // increase required length
                    if (tempDigits < maxLen)
                        tempDigits = maxLen;
                }
            }

            if (tempDigits < parameterLength)
                outputStringSize += parameterLength;
            else
                outputStringSize += tempDigits;

            parameterLength = 0;
            digits_string.clear();
            i_d_x_f_found = false;
            percentCharFound = false;
            handleNextParameter = false;
            ++inputArgNr;
        }
    }

    return outputStringSize;
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::arrayIndex()
{
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "[")
            continue;
        const Token *array = tok->astOperand1();
        while (Token::Match(array, ".|::"))
            array = array->astOperand2();
        if (!array|| !array->variable() || array->variable()->nameToken() == array)
            continue;
        if (!array->scope()->isExecutable()) {
            // LHS in non-executable scope => This is just a definition
            const Token *parent = tok;
            while (parent && !Token::simpleMatch(parent->astParent(), "="))
                parent = parent->astParent();
            if (!parent || parent == parent->astParent()->astOperand1())
                continue;
        }

        const Token *indexToken = tok->astOperand2();
        if (!indexToken)
            continue;

        std::vector<Dimension> dimensions;

        bool mightBeLarger;

        if (array->variable()->isArray() && !array->variable()->dimensions().empty()) {
            dimensions = array->variable()->dimensions();
            mightBeLarger = (dimensions.size() >= 1 && (dimensions[0].num <= 1 || !dimensions[0].tok));
        } else if (const Token *stringLiteral = array->getValueTokenMinStrSize()) {
            Dimension dim;
            dim.tok = nullptr;
            dim.num = Token::getStrSize(stringLiteral);
            dim.known = array->hasKnownValue();
            dimensions.emplace_back(dim);
            mightBeLarger = false;
        } else if (array->valueType() && array->valueType()->pointer >= 1 && array->valueType()->isIntegral()) {
            dimensions = getDynamicDimensions(array, array->valueType()->typeSize(*mSettings));
            mightBeLarger = false;
        }

        if (dimensions.empty())
            continue;

        const MathLib::bigint dim = dimensions[0].num;

        // Positive index
        if (!mightBeLarger) { // TODO check arrays with dim 1 also
            for (int cond = 0; cond < 2; cond++) {
                const ValueFlow::Value *value = indexToken->getMaxValue(cond == 1);
                if (!value)
                    continue;
                const MathLib::bigint index = value->intvalue;
                if (index < dim)
                    continue;
                if (index == dim) {
                    const Token *parent = tok;
                    while (Token::simpleMatch(parent, "["))
                        parent = parent->astParent();
                    if (parent->isUnaryOp("&"))
                        continue;
                }
                arrayIndexError(tok, dimensions, value);
            }
        }

        // Negative index
        const ValueFlow::Value *negativeValue = indexToken->getValueLE(-1, mSettings);
        if (negativeValue) {
            negativeIndexError(tok, dimensions, negativeValue);
        }
    }
}

static std::string arrayIndexMessage(const Token *tok, const std::vector<Dimension> &dimensions, const ValueFlow::Value *index)
{
    std::string array = tok->astOperand1()->expressionString();
    for (const Dimension &dim : dimensions)
        array += "[" + MathLib::toString(dim.num) + "]";

    std::ostringstream errmsg;
    if (index->condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(index->condition)
               << " or the array '" + array + "' is accessed at index " << index->intvalue << ", which is out of bounds.";
    else
        errmsg << "Array '" << array << "' accessed at index " << index->intvalue <<  ", which is out of bounds.";

    return errmsg.str();
}

void CheckBufferOverrun::arrayIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const ValueFlow::Value *index)
{
    if (!tok) {
        reportError(tok, Severity::error, "arrayIndexOutOfBounds", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE788, false);
        reportError(tok, Severity::warning, "arrayIndexOutOfBoundsCond", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE788, false);
        return;
    }

    reportError(getErrorPath(tok, index, "Array index out of bounds"),
                index->errorSeverity() ? Severity::error : Severity::warning,
                index->condition ? "arrayIndexOutOfBoundsCond" : "arrayIndexOutOfBounds",
                arrayIndexMessage(tok, dimensions, index),
                CWE788,
                index->isInconclusive());
}

void CheckBufferOverrun::negativeIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const ValueFlow::Value *negativeValue)
{
    if (!negativeValue) {
        reportError(tok, Severity::error, "negativeIndex", "Negative array index", CWE786, false);
        return;
    }

    if (!negativeValue->errorSeverity() && !mSettings->isEnabled(Settings::WARNING))
        return;

    reportError(getErrorPath(tok, negativeValue, "Negative array index"),
                negativeValue->errorSeverity() ? Severity::error : Severity::warning,
                "negativeIndex",
                arrayIndexMessage(tok, dimensions, negativeValue),
                CWE786,
                negativeValue->isInconclusive());
}

//---------------------------------------------------------------------------

size_t CheckBufferOverrun::getBufferSize(const Token *bufTok) const
{
    if (!bufTok->valueType())
        return 0;
    const Variable *var = bufTok->variable();
    if (!var)
        return 0;
    const MathLib::bigint typeSize = bufTok->valueType()->typeSize(*mSettings);
    std::vector<Dimension> dimensions;
    if (!var->dimensions().empty())
        dimensions = var->dimensions();
    else
        dimensions = getDynamicDimensions(bufTok, typeSize);
    if (dimensions.empty())
        return 0;

    MathLib::bigint dim = 1;
    for (const Dimension &d : dimensions)
        dim *= d.num;
    if (var->isPointerArray())
        return dim * mSettings->sizeof_pointer;
    return dim * typeSize;
}
//---------------------------------------------------------------------------

static bool checkBufferSize(const Token *ftok, const Library::ArgumentChecks::MinSize &minsize, const std::vector<const Token *> &args, const MathLib::bigint bufferSize, const Settings *settings)
{
    const Token * const arg = (minsize.arg > 0 && minsize.arg - 1 < args.size()) ? args[minsize.arg - 1] : nullptr;
    const Token * const arg2 = (minsize.arg2 > 0 && minsize.arg2 - 1 < args.size()) ? args[minsize.arg2 - 1] : nullptr;

    switch (minsize.type) {
    case Library::ArgumentChecks::MinSize::Type::STRLEN:
        if (settings->library.isargformatstr(ftok, minsize.arg)) {
            return getMinFormatStringOutputLength(args, minsize.arg) < bufferSize;
        } else if (arg) {
            const Token *strtoken = arg->getValueTokenMaxStrLength();
            if (strtoken)
                return Token::getStrLength(strtoken) < bufferSize;
        }
        break;
    case Library::ArgumentChecks::MinSize::Type::ARGVALUE:
        if (arg && arg->hasKnownIntValue())
            return arg->getKnownIntValue() <= bufferSize;
        break;
    case Library::ArgumentChecks::MinSize::Type::SIZEOF:
        // TODO
        break;
    case Library::ArgumentChecks::MinSize::Type::MUL:
        if (arg && arg2 && arg->hasKnownIntValue() && arg2->hasKnownIntValue())
            return (arg->getKnownIntValue() * arg2->getKnownIntValue()) <= bufferSize;
        break;
    case Library::ArgumentChecks::MinSize::Type::VALUE:
        return minsize.value <= bufferSize;
    case Library::ArgumentChecks::MinSize::Type::NONE:
        break;
    };
    return true;
}


void CheckBufferOverrun::bufferOverflow()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% (") || Token::simpleMatch(tok, ") {"))
                continue;
            if (!mSettings->library.hasminsize(tok))
                continue;
            const std::vector<const Token *> args = getArguments(tok);
            for (unsigned int argnr = 0; argnr < args.size(); ++argnr) {
                if (!args[argnr]->valueType() || args[argnr]->valueType()->pointer == 0)
                    continue;
                const std::vector<Library::ArgumentChecks::MinSize> *minsizes = mSettings->library.argminsizes(tok, argnr + 1);
                if (!minsizes || minsizes->empty())
                    continue;
                // Get buffer size..
                const Token *argtok = args[argnr];
                while (argtok && argtok->isCast())
                    argtok = argtok->astOperand2() ? argtok->astOperand2() : argtok->astOperand1();
                while (Token::Match(argtok, ".|::"))
                    argtok = argtok->astOperand2();
                if (!argtok || !argtok->variable())
                    continue;
                // TODO: strcpy(buf+10, "hello");
                const size_t bufferSize = getBufferSize(argtok);
                if (bufferSize <= 1)
                    continue;
                bool error = true;
                for (const Library::ArgumentChecks::MinSize &minsize : *minsizes) {
                    if (checkBufferSize(tok, minsize, args, bufferSize, mSettings)) {
                        error = false;
                        break;
                    }
                }
                if (error)
                    bufferOverflowError(args[argnr]);
            }
        }
    }
}

void CheckBufferOverrun::bufferOverflowError(const Token *tok)
{
    reportError(tok, Severity::error, "bufferAccessOutOfBounds", "Buffer is accessed out of bounds: " + (tok ? tok->expressionString() : "buf"), CWE788, false);
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::arrayIndexThenCheck()
{
    if (!mSettings->isEnabled(Settings::PORTABILITY))
        return;

    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * const scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "sizeof (")) {
                tok = tok->linkAt(1);
                continue;
            }

            if (Token::Match(tok, "%name% [ %var% ]")) {
                tok = tok->next();

                const unsigned int indexID = tok->next()->varId();
                const std::string& indexName(tok->strAt(1));

                // Iterate AST upwards
                const Token* tok2 = tok;
                const Token* tok3 = tok2;
                while (tok2->astParent() && tok2->tokType() != Token::eLogicalOp) {
                    tok3 = tok2;
                    tok2 = tok2->astParent();
                }

                // Ensure that we ended at a logical operator and that we came from its left side
                if (tok2->tokType() != Token::eLogicalOp || tok2->astOperand1() != tok3)
                    continue;

                // check if array index is ok
                // statement can be closed in parentheses, so "(| " is using
                if (Token::Match(tok2, "&& (| %varid% <|<=", indexID))
                    arrayIndexThenCheckError(tok, indexName);
                else if (Token::Match(tok2, "&& (| %any% >|>= %varid% !!+", indexID))
                    arrayIndexThenCheckError(tok, indexName);
            }
        }
    }
}

void CheckBufferOverrun::arrayIndexThenCheckError(const Token *tok, const std::string &indexName)
{
    reportError(tok, Severity::style, "arrayIndexThenCheck",
                "$symbol:" + indexName + "\n"
                "Array index '$symbol' is used before limits check.\n"
                "Defensive programming: The variable '$symbol' is used as an array index before it "
                "is checked that is within limits. This can mean that the array might be accessed out of bounds. "
                "Reorder conditions such as '(a[i] && i < 10)' to '(i < 10 && a[i])'. That way the array will "
                "not be accessed if the index is out of limits.", CWE398, false);
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::stringNotZeroTerminated()
{
    // this is currently 'inconclusive'. See TestBufferOverrun::terminateStrncpy3
    if (!mSettings->isEnabled(Settings::WARNING) || !mSettings->inconclusive)
        return;
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * const scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart; tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "strncpy ("))
                continue;
            const std::vector<const Token *> args = getArguments(tok);
            if (args.size() != 3)
                continue;
            const Token *sizeToken = args[2];
            if (!sizeToken->hasKnownIntValue())
                continue;
            const size_t bufferSize = getBufferSize(args[0]);
            if (bufferSize == 0 || sizeToken->getKnownIntValue() < bufferSize)
                continue;
            const Token *srcValue = args[1]->getValueTokenMaxStrLength();
            if (srcValue && Token::getStrLength(srcValue) < sizeToken->getKnownIntValue())
                continue;
            // Is the buffer zero terminated after the call?
            bool isZeroTerminated = false;
            for (const Token *tok2 = tok->next()->link(); tok2 != scope->bodyEnd; tok2 = tok2->next()) {
                if (!Token::Match(tok2, "] ="))
                    continue;
                const Token *rhs = tok2->next()->astOperand2();
                if (!rhs || !rhs->hasKnownIntValue() || rhs->getKnownIntValue() != 0)
                    continue;
                if (isSameExpression(mTokenizer->isCPP(), false, args[0], tok2->link()->astOperand1(), mSettings->library, false, false))
                    isZeroTerminated = true;
            }
            if (isZeroTerminated)
                continue;
            // TODO: Locate unsafe string usage..
            terminateStrncpyError(tok, args[0]->expressionString());
        }
    }
}

void CheckBufferOverrun::terminateStrncpyError(const Token *tok, const std::string &varname)
{
    const std::string shortMessage = "The buffer '$symbol' may not be null-terminated after the call to strncpy().";
    reportError(tok, Severity::warning, "terminateStrncpy",
                "$symbol:" + varname + '\n' +
                shortMessage + '\n' +
                shortMessage + ' ' +
                "If the source string's size fits or exceeds the given size, strncpy() does not add a "
                "zero at the end of the buffer. This causes bugs later in the code if the code "
                "assumes buffer is null-terminated.", CWE170, true);
}

void CheckBufferOverrun::bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function)
{
    const std::string errmsg = "$symbol:" + varname + '\n' +
                               "$symbol:" + function + '\n' +
                               "The buffer '" + varname + "' is not null-terminated after the call to " + function + "().\n"
                               "The buffer '" + varname + "' is not null-terminated after the call to " + function + "(). "
                               "This will cause bugs later in the code if the code assumes the buffer is null-terminated.";

    reportError(tok, Severity::warning, "bufferNotZeroTerminated", errmsg, CWE170, true);
}


