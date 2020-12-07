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

//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

#include "checkbufferoverrun.h"

#include "astutils.h"
#include "errorlogger.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "utils.h"
#include "valueflow.h"

#include <tinyxml2.h>
#include <algorithm>
#include <cstdlib>
#include <numeric> // std::accumulate
#include <sstream>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

// CWE ids used:
static const CWE CWE131(131U);  // Incorrect Calculation of Buffer Size
static const CWE CWE170(170U);  // Improper Null Termination
static const CWE CWE_ARRAY_INDEX_THEN_CHECK(398U);  // Indicator of Poor Code Quality
static const CWE CWE682(682U);  // Incorrect Calculation
static const CWE CWE758(758U);  // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE_POINTER_ARITHMETIC_OVERFLOW(758U); // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const CWE CWE_BUFFER_UNDERRUN(786U);  // Access of Memory Location Before Start of Buffer
static const CWE CWE_BUFFER_OVERRUN(788U);   // Access of Memory Location After End of Buffer

//---------------------------------------------------------------------------

static const ValueFlow::Value *getBufferSizeValue(const Token *tok)
{
    const std::list<ValueFlow::Value> &tokenValues = tok->values();
    const auto it = std::find_if(tokenValues.begin(), tokenValues.end(), std::mem_fn(&ValueFlow::Value::isBufferSizeValue));
    return it == tokenValues.end() ? nullptr : &*it;
}

static int getMinFormatStringOutputLength(const std::vector<const Token*> &parameters, nonneg int formatStringArgNr)
{
    if (formatStringArgNr <= 0 || formatStringArgNr > parameters.size())
        return 0;
    if (parameters[formatStringArgNr - 1]->tokType() != Token::eString)
        return 0;
    const std::string &formatString = parameters[formatStringArgNr - 1]->str();
    bool percentCharFound = false;
    int outputStringSize = 0;
    bool handleNextParameter = false;
    std::string digits_string;
    bool i_d_x_f_found = false;
    int parameterLength = 0;
    int inputArgNr = formatStringArgNr;
    for (int i = 1; i + 1 < formatString.length(); ++i) {
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
            int tempDigits = std::abs(std::atoi(digits_string.c_str()));
            if (i_d_x_f_found)
                tempDigits = std::max(tempDigits, 1);

            if (digits_string.find('.') != std::string::npos) {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                const int maxLen = std::max(std::abs(std::atoi(endStr.c_str())), 1);

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

static bool getDimensionsEtc(const Token * const arrayToken, const Settings *settings, std::vector<Dimension> * const dimensions, ErrorPath * const errorPath, bool * const mightBeLarger, MathLib::bigint* path)
{
    const Token *array = arrayToken;
    while (Token::Match(array, ".|::"))
        array = array->astOperand2();

    if (array->variable() && array->variable()->isArray() && !array->variable()->dimensions().empty()) {
        *dimensions = array->variable()->dimensions();
        if (dimensions->size() >= 1 && ((*dimensions)[0].num <= 1 || !(*dimensions)[0].tok)) {
            visitAstNodes(arrayToken,
            [&](const Token *child) {
                if (child->originalName() == "->") {
                    *mightBeLarger = true;
                    return ChildrenToVisit::none;
                }
                return ChildrenToVisit::op1_and_op2;
            });
        }
    } else if (const Token *stringLiteral = array->getValueTokenMinStrSize(settings)) {
        Dimension dim;
        dim.tok = nullptr;
        dim.num = Token::getStrArraySize(stringLiteral);
        dim.known = array->hasKnownValue();
        dimensions->emplace_back(dim);
    } else if (array->valueType() && array->valueType()->pointer >= 1 && array->valueType()->isIntegral()) {
        const ValueFlow::Value *value = getBufferSizeValue(array);
        if (!value)
            return false;
        if (path)
            *path = value->path;
        *errorPath = value->errorPath;
        Dimension dim;
        dim.known = value->isKnown();
        dim.tok = nullptr;
        const int typeSize = array->valueType()->typeSize(*settings);
        if (typeSize == 0)
            return false;
        dim.num = value->intvalue / typeSize;
        dimensions->emplace_back(dim);
    }
    return !dimensions->empty();
}

static std::vector<const ValueFlow::Value *> getOverrunIndexValues(const Token *tok, const Token *arrayToken, const std::vector<Dimension> &dimensions, const std::vector<const Token *> &indexTokens, MathLib::bigint path)
{
    const Token *array = arrayToken;
    while (Token::Match(array, ".|::"))
        array = array->astOperand2();

    for (int cond = 0; cond < 2; cond++) {
        bool equal = false;
        bool overflow = false;
        bool allKnown = true;
        std::vector<const ValueFlow::Value *> indexValues;
        for (int i = 0; i < dimensions.size() && i < indexTokens.size(); ++i) {
            const ValueFlow::Value *value = indexTokens[i]->getMaxValue(cond == 1);
            indexValues.push_back(value);
            if (!value)
                continue;
            if (value->path != path)
                continue;
            if (!value->isKnown()) {
                if (!allKnown)
                    continue;
                allKnown = false;
            }
            if (array->variable() && array->variable()->isArray() && dimensions[i].num == 0)
                continue;
            if (value->intvalue == dimensions[i].num)
                equal = true;
            else if (value->intvalue > dimensions[i].num)
                overflow = true;
        }
        if (equal && tok->str() != "[")
            continue;
        if (!overflow && equal) {
            const Token *parent = tok;
            while (Token::simpleMatch(parent, "["))
                parent = parent->astParent();
            if (!parent || parent->isUnaryOp("&"))
                continue;
        }
        if (overflow || equal)
            return indexValues;
    }

    return std::vector<const ValueFlow::Value *>();
}

void CheckBufferOverrun::arrayIndex()
{
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "[")
            continue;
        const Token *array = tok->astOperand1();
        while (Token::Match(array, ".|::"))
            array = array->astOperand2();
        if (!array || ((!array->variable() || array->variable()->nameToken() == array) && array->tokType() != Token::eString))
            continue;
        if (!array->scope()->isExecutable()) {
            // LHS in non-executable scope => This is just a definition
            const Token *parent = tok;
            while (parent && !Token::simpleMatch(parent->astParent(), "="))
                parent = parent->astParent();
            if (!parent || parent == parent->astParent()->astOperand1())
                continue;
        }

        if (astIsContainer(array))
            continue;

        std::vector<const Token *> indexTokens;
        for (const Token *tok2 = tok; tok2 && tok2->str() == "["; tok2 = tok2->link()->next()) {
            if (!tok2->astOperand2()) {
                indexTokens.clear();
                break;
            }
            indexTokens.emplace_back(tok2->astOperand2());
        }
        if (indexTokens.empty())
            continue;

        std::vector<Dimension> dimensions;
        ErrorPath errorPath;
        bool mightBeLarger = false;
        MathLib::bigint path = 0;
        if (!getDimensionsEtc(tok->astOperand1(), mSettings, &dimensions, &errorPath, &mightBeLarger, &path))
            continue;

        // Positive index
        if (!mightBeLarger) { // TODO check arrays with dim 1 also
            const std::vector<const ValueFlow::Value *> &indexValues = getOverrunIndexValues(tok, tok->astOperand1(), dimensions, indexTokens, path);
            if (!indexValues.empty())
                arrayIndexError(tok, dimensions, indexValues);
        }

        // Negative index
        bool neg = false;
        std::vector<const ValueFlow::Value *> negativeIndexes;
        for (const Token * indexToken : indexTokens) {
            const ValueFlow::Value *negativeValue = indexToken->getValueLE(-1, mSettings);
            negativeIndexes.emplace_back(negativeValue);
            if (negativeValue)
                neg = true;
        }
        if (neg) {
            negativeIndexError(tok, dimensions, negativeIndexes);
        }
    }
}

static std::string stringifyIndexes(const std::string &array, const std::vector<const ValueFlow::Value *> &indexValues)
{
    if (indexValues.size() == 1)
        return MathLib::toString(indexValues[0]->intvalue);

    std::ostringstream ret;
    ret << array;
    for (const ValueFlow::Value *index : indexValues) {
        ret << "[";
        if (index)
            ret << index->intvalue;
        else
            ret << "*";
        ret << "]";
    }
    return ret.str();
}

static std::string arrayIndexMessage(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexValues, const Token *condition)
{
    auto add_dim = [](const std::string &s, const Dimension &dim) {
        return s + "[" + MathLib::toString(dim.num) + "]";
    };
    const std::string array = std::accumulate(dimensions.begin(), dimensions.end(), tok->astOperand1()->expressionString(), add_dim);

    std::ostringstream errmsg;
    if (condition)
        errmsg << ValueFlow::eitherTheConditionIsRedundant(condition)
               << " or the array '" + array + "' is accessed at index " << stringifyIndexes(tok->astOperand1()->expressionString(), indexValues) << ", which is out of bounds.";
    else
        errmsg << "Array '" << array << "' accessed at index " << stringifyIndexes(tok->astOperand1()->expressionString(), indexValues) <<  ", which is out of bounds.";

    return errmsg.str();
}

void CheckBufferOverrun::arrayIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes)
{
    if (!tok) {
        reportError(tok, Severity::error, "arrayIndexOutOfBounds", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE_BUFFER_OVERRUN, false);
        reportError(tok, Severity::warning, "arrayIndexOutOfBoundsCond", "Array 'arr[16]' accessed at index 16, which is out of bounds.", CWE_BUFFER_OVERRUN, false);
        return;
    }

    const Token *condition = nullptr;
    const ValueFlow::Value *index = nullptr;
    for (const ValueFlow::Value *indexValue: indexes) {
        if (!indexValue)
            continue;
        if (!indexValue->errorSeverity() && !mSettings->isEnabled(Settings::WARNING))
            return;
        if (indexValue->condition)
            condition = indexValue->condition;
        if (!index || !indexValue->errorPath.empty())
            index = indexValue;
    }

    reportError(getErrorPath(tok, index, "Array index out of bounds"),
                index->errorSeverity() ? Severity::error : Severity::warning,
                index->condition ? "arrayIndexOutOfBoundsCond" : "arrayIndexOutOfBounds",
                arrayIndexMessage(tok, dimensions, indexes, condition),
                CWE_BUFFER_OVERRUN,
                index->isInconclusive());
}

void CheckBufferOverrun::negativeIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes)
{
    if (!tok) {
        reportError(tok, Severity::error, "negativeIndex", "Negative array index", CWE_BUFFER_UNDERRUN, false);
        return;
    }

    const Token *condition = nullptr;
    const ValueFlow::Value *negativeValue = nullptr;
    for (const ValueFlow::Value *indexValue: indexes) {
        if (!indexValue)
            continue;
        if (!indexValue->errorSeverity() && !mSettings->isEnabled(Settings::WARNING))
            return;
        if (indexValue->condition)
            condition = indexValue->condition;
        if (!negativeValue || !indexValue->errorPath.empty())
            negativeValue = indexValue;
    }

    reportError(getErrorPath(tok, negativeValue, "Negative array index"),
                negativeValue->errorSeverity() ? Severity::error : Severity::warning,
                "negativeIndex",
                arrayIndexMessage(tok, dimensions, indexes, condition),
                CWE_BUFFER_UNDERRUN,
                negativeValue->isInconclusive());
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::pointerArithmetic()
{
    if (!mSettings->isEnabled(Settings::PORTABILITY))
        return;

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "+|-"))
            continue;
        if (!tok->valueType() || tok->valueType()->pointer == 0)
            continue;
        if (!tok->astOperand1() || !tok->astOperand2())
            continue;
        if (!tok->astOperand1()->valueType() || !tok->astOperand2()->valueType())
            continue;

        const Token *arrayToken, *indexToken;
        if (tok->astOperand1()->valueType()->pointer > 0) {
            arrayToken = tok->astOperand1();
            indexToken = tok->astOperand2();
        } else {
            arrayToken = tok->astOperand2();
            indexToken = tok->astOperand1();
        }

        if (!indexToken || !indexToken->valueType() || indexToken->valueType()->pointer > 0 || !indexToken->valueType()->isIntegral())
            continue;

        std::vector<Dimension> dimensions;
        ErrorPath errorPath;
        bool mightBeLarger = false;
        MathLib::bigint path = 0;
        if (!getDimensionsEtc(arrayToken, mSettings, &dimensions, &errorPath, &mightBeLarger, &path))
            continue;

        if (tok->str() == "+") {
            // Positive index
            if (!mightBeLarger) { // TODO check arrays with dim 1 also
                const std::vector<const Token *> indexTokens{indexToken};
                const std::vector<const ValueFlow::Value *> &indexValues = getOverrunIndexValues(tok, arrayToken, dimensions, indexTokens, path);
                if (!indexValues.empty())
                    pointerArithmeticError(tok, indexToken, indexValues.front());
            }

            if (const ValueFlow::Value *neg = indexToken->getValueLE(-1, mSettings))
                pointerArithmeticError(tok, indexToken, neg);
        } else if (tok->str() == "-") {
            // TODO
        }
    }
}

void CheckBufferOverrun::pointerArithmeticError(const Token *tok, const Token *indexToken, const ValueFlow::Value *indexValue)
{
    if (!tok) {
        reportError(tok, Severity::portability, "pointerOutOfBounds", "Pointer arithmetic overflow.", CWE_POINTER_ARITHMETIC_OVERFLOW, false);
        reportError(tok, Severity::portability, "pointerOutOfBoundsCond", "Pointer arithmetic overflow.", CWE_POINTER_ARITHMETIC_OVERFLOW, false);
        return;
    }

    std::string errmsg;
    if (indexValue->condition)
        errmsg = "Undefined behaviour, when '" + indexToken->expressionString() + "' is " + MathLib::toString(indexValue->intvalue) + " the pointer arithmetic '" + tok->expressionString() + "' is out of bounds.";
    else
        errmsg = "Undefined behaviour, pointer arithmetic '" + tok->expressionString() + "' is out of bounds.";

    reportError(getErrorPath(tok, indexValue, "Pointer arithmetic overflow"),
                Severity::portability,
                indexValue->condition ? "pointerOutOfBoundsCond" : "pointerOutOfBounds",
                errmsg,
                CWE_POINTER_ARITHMETIC_OVERFLOW,
                indexValue->isInconclusive());
}

//---------------------------------------------------------------------------

ValueFlow::Value CheckBufferOverrun::getBufferSize(const Token *bufTok) const
{
    if (!bufTok->valueType())
        return ValueFlow::Value(-1);
    const Variable *var = bufTok->variable();

    if (!var || var->dimensions().empty()) {
        const ValueFlow::Value *value = getBufferSizeValue(bufTok);
        if (value)
            return *value;
    }

    if (!var)
        return ValueFlow::Value(-1);

    MathLib::bigint dim = std::accumulate(var->dimensions().begin(), var->dimensions().end(), 1LL, [](MathLib::bigint i1, const Dimension &dim) {
        return i1 * dim.num;
    });

    ValueFlow::Value v;
    v.setKnown();
    v.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;

    if (var->isPointerArray())
        v.intvalue = dim * mSettings->sizeof_pointer;
    else if (var->isPointer())
        return ValueFlow::Value(-1);
    else {
        const MathLib::bigint typeSize = bufTok->valueType()->typeSize(*mSettings);
        v.intvalue = dim * typeSize;
    }

    return v;
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
    }
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
            for (int argnr = 0; argnr < args.size(); ++argnr) {
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
                if (argtok->valueType() && argtok->valueType()->pointer == 0)
                    continue;
                // TODO: strcpy(buf+10, "hello");
                const ValueFlow::Value bufferSize = getBufferSize(argtok);
                if (bufferSize.intvalue <= 1)
                    continue;
                bool error = std::none_of(minsizes->begin(), minsizes->end(), [=](const Library::ArgumentChecks::MinSize &minsize) {
                    return checkBufferSize(tok, minsize, args, bufferSize.intvalue, mSettings);
                });
                if (error)
                    bufferOverflowError(args[argnr], &bufferSize);
            }
        }
    }
}

void CheckBufferOverrun::bufferOverflowError(const Token *tok, const ValueFlow::Value *value)
{
    reportError(getErrorPath(tok, value, "Buffer overrun"), Severity::error, "bufferAccessOutOfBounds", "Buffer is accessed out of bounds: " + (tok ? tok->expressionString() : "buf"), CWE_BUFFER_OVERRUN, false);
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

                const int indexID = tok->next()->varId();
                const std::string& indexName(tok->strAt(1));

                // Iterate AST upwards
                const Token* tok2 = tok;
                const Token* tok3 = tok2;
                while (tok2->astParent() && tok2->tokType() != Token::eLogicalOp && tok2->str() != "?") {
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
                "not be accessed if the index is out of limits.", CWE_ARRAY_INDEX_THEN_CHECK, false);
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
            const ValueFlow::Value &bufferSize = getBufferSize(args[0]);
            if (bufferSize.intvalue < 0 || sizeToken->getKnownIntValue() < bufferSize.intvalue)
                continue;
            const Token *srcValue = args[1]->getValueTokenMaxStrLength();
            if (srcValue && Token::getStrLength(srcValue) < sizeToken->getKnownIntValue())
                continue;
            // Is the buffer zero terminated after the call?
            bool isZeroTerminated = false;
            for (const Token *tok2 = tok->next()->link(); tok2 != scope->bodyEnd; tok2 = tok2->next()) {
                if (!Token::simpleMatch(tok2, "] ="))
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



//---------------------------------------------------------------------------
// CTU..
//---------------------------------------------------------------------------

std::string CheckBufferOverrun::MyFileInfo::toString() const
{
    std::string xml;
    if (!unsafeArrayIndex.empty())
        xml = "    <array-index>\n" + CTU::toString(unsafeArrayIndex) + "    </array-index>\n";
    if (!unsafePointerArith.empty())
        xml += "    <pointer-arith>\n" + CTU::toString(unsafePointerArith) + "    </pointer-arith>\n";
    return xml;
}

bool CheckBufferOverrun::isCtuUnsafeBufferUsage(const Check *check, const Token *argtok, MathLib::bigint *offset, int type)
{
    const CheckBufferOverrun *c = dynamic_cast<const CheckBufferOverrun *>(check);
    if (!c)
        return false;
    if (!argtok->valueType() || argtok->valueType()->typeSize(*c->mSettings) == 0)
        return false;
    const Token *indexTok = nullptr;
    if (type == 1 && Token::Match(argtok, "%name% [") && argtok->astParent() == argtok->next() && !Token::simpleMatch(argtok->linkAt(1), "] ["))
        indexTok = argtok->next()->astOperand2();
    else if (type == 2 && Token::simpleMatch(argtok->astParent(), "+"))
        indexTok = (argtok == argtok->astParent()->astOperand1()) ?
                   argtok->astParent()->astOperand2() :
                   argtok->astParent()->astOperand1();
    if (!indexTok)
        return false;
    if (!indexTok->hasKnownIntValue())
        return false;
    if (!offset)
        return false;
    *offset = indexTok->getKnownIntValue() * argtok->valueType()->typeSize(*c->mSettings);
    return true;
}

bool CheckBufferOverrun::isCtuUnsafeArrayIndex(const Check *check, const Token *argtok, MathLib::bigint *offset)
{
    return CheckBufferOverrun::isCtuUnsafeBufferUsage(check, argtok, offset, 1);
}

bool CheckBufferOverrun::isCtuUnsafePointerArith(const Check *check, const Token *argtok, MathLib::bigint *offset)
{
    return CheckBufferOverrun::isCtuUnsafeBufferUsage(check, argtok, offset, 2);
}

/** @brief Parse current TU and extract file info */
Check::FileInfo *CheckBufferOverrun::getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const
{
    CheckBufferOverrun checkBufferOverrun(tokenizer, settings, nullptr);
    MyFileInfo *fileInfo = new MyFileInfo;
    fileInfo->unsafeArrayIndex = CTU::getUnsafeUsage(tokenizer, settings, &checkBufferOverrun, isCtuUnsafeArrayIndex);
    fileInfo->unsafePointerArith = CTU::getUnsafeUsage(tokenizer, settings, &checkBufferOverrun, isCtuUnsafePointerArith);
    if (fileInfo->unsafeArrayIndex.empty() && fileInfo->unsafePointerArith.empty()) {
        delete fileInfo;
        return nullptr;
    }
    return fileInfo;
}

Check::FileInfo * CheckBufferOverrun::loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const
{
    const std::string arrayIndex("array-index");
    const std::string pointerArith("pointer-arith");

    MyFileInfo *fileInfo = new MyFileInfo;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (e->Name() == arrayIndex)
            fileInfo->unsafeArrayIndex = CTU::loadUnsafeUsageListFromXml(e);
        else if (e->Name() == pointerArith)
            fileInfo->unsafePointerArith = CTU::loadUnsafeUsageListFromXml(e);
    }

    if (fileInfo->unsafeArrayIndex.empty() && fileInfo->unsafePointerArith.empty()) {
        delete fileInfo;
        return nullptr;
    }

    return fileInfo;
}

/** @brief Analyse all file infos for all TU */
bool CheckBufferOverrun::analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    if (!ctu)
        return false;
    bool foundErrors = false;
    (void)settings; // This argument is unused

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> callsMap = ctu->getCallsMap();

    for (Check::FileInfo *fi1 : fileInfo) {
        const MyFileInfo *fi = dynamic_cast<MyFileInfo*>(fi1);
        if (!fi)
            continue;
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafeArrayIndex)
            foundErrors |= analyseWholeProgram1(ctu, callsMap, unsafeUsage, 1, errorLogger);
        for (const CTU::FileInfo::UnsafeUsage &unsafeUsage : fi->unsafePointerArith)
            foundErrors |= analyseWholeProgram1(ctu, callsMap, unsafeUsage, 2, errorLogger);
    }
    return foundErrors;
}

bool CheckBufferOverrun::analyseWholeProgram1(const CTU::FileInfo *ctu, const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap, const CTU::FileInfo::UnsafeUsage &unsafeUsage, int type, ErrorLogger &errorLogger)
{
    const CTU::FileInfo::FunctionCall *functionCall = nullptr;

    const std::list<ErrorMessage::FileLocation> &locationList =
        ctu->getErrorPath(CTU::FileInfo::InvalidValueType::bufferOverflow,
                          unsafeUsage,
                          callsMap,
                          "Using argument ARG",
                          &functionCall,
                          false);
    if (locationList.empty())
        return false;

    const char *errorId = nullptr;
    std::string errmsg;
    CWE cwe(0);

    if (type == 1) {
        errorId = "ctuArrayIndex";
        if (unsafeUsage.value > 0)
            errmsg = "Array index out of bounds; '" + unsafeUsage.myArgumentName + "' buffer size is " + MathLib::toString(functionCall->callArgValue) + " and it is accessed at offset " + MathLib::toString(unsafeUsage.value) + ".";
        else
            errmsg = "Array index out of bounds; buffer '" + unsafeUsage.myArgumentName + "' is accessed at offset " + MathLib::toString(unsafeUsage.value) + ".";
        cwe = (unsafeUsage.value > 0) ? CWE_BUFFER_OVERRUN : CWE_BUFFER_UNDERRUN;
    } else {
        errorId = "ctuPointerArith";
        errmsg = "Pointer arithmetic overflow; '" + unsafeUsage.myArgumentName + "' buffer size is " + MathLib::toString(functionCall->callArgValue);
        cwe = CWE_POINTER_ARITHMETIC_OVERFLOW;
    }

    const ErrorMessage errorMessage(locationList,
                                    emptyString,
                                    Severity::error,
                                    errmsg,
                                    errorId,
                                    cwe, false);
    errorLogger.reportErr(errorMessage);

    return true;
}

void CheckBufferOverrun::objectIndex()
{
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "["))
                continue;
            const Token *obj = tok->astOperand1();
            const Token *idx = tok->astOperand2();
            if (!idx || !obj)
                continue;
            if (idx->hasKnownIntValue() && idx->getKnownIntValue() == 0)
                continue;

            ValueFlow::Value v = getLifetimeObjValue(obj);
            if (!v.isLocalLifetimeValue())
                continue;
            if (v.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                continue;
            const Variable *var = v.tokvalue->variable();
            if (var->isReference())
                continue;
            if (var->isRValueReference())
                continue;
            if (var->isArray())
                continue;
            if (var->isPointer())
                continue;
            objectIndexError(tok, &v, idx->hasKnownIntValue());
        }
    }
}

void CheckBufferOverrun::objectIndexError(const Token *tok, const ValueFlow::Value *v, bool known)
{
    ErrorPath errorPath;
    std::string name;
    if (v) {
        name = v->tokvalue->variable()->name();
        errorPath = v->errorPath;
    }
    errorPath.emplace_back(tok, "");
    std::string verb = known ? "is" : "might be";
    reportError(errorPath,
                known ? Severity::error : Severity::warning,
                "objectIndex",
                "The address of local variable '" + name + "' " + verb + " accessed at non-zero index.",
                CWE758,
                false);
}
