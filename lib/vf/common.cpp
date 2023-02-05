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

#include "common.h"

#include "astutils.h"
#include "calculate.h"
#include "mathlib.h"
#include "path.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueflow.h"

using namespace ValueFlow;

bool ValueFlow::getMinMaxValues(const ValueType *vt, const cppcheck::Platform &platform, MathLib::bigint *minValue, MathLib::bigint *maxValue)
{
    if (!vt || !vt->isIntegral() || vt->pointer)
        return false;

    int bits;
    switch (vt->type) {
    case ValueType::Type::BOOL:
        bits = 1;
        break;
    case ValueType::Type::CHAR:
        bits = platform.char_bit;
        break;
    case ValueType::Type::SHORT:
        bits = platform.short_bit;
        break;
    case ValueType::Type::INT:
        bits = platform.int_bit;
        break;
    case ValueType::Type::LONG:
        bits = platform.long_bit;
        break;
    case ValueType::Type::LONGLONG:
        bits = platform.long_long_bit;
        break;
    default:
        return false;
    }

    if (bits == 1) {
        *minValue = 0;
        *maxValue = 1;
    } else if (bits < 62) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            *minValue = 0;
            *maxValue = (1LL << bits) - 1;
        } else {
            *minValue = -(1LL << (bits - 1));
            *maxValue = (1LL << (bits - 1)) - 1;
        }
    } else if (bits == 64) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            *minValue = 0;
            *maxValue = LLONG_MAX; // todo max unsigned value
        } else {
            *minValue = LLONG_MIN;
            *maxValue = LLONG_MAX;
        }
    } else {
        return false;
    }

    return true;
}

static nonneg int getSizeOfType(const Token *typeTok, const Settings *settings)
{
    const ValueType &valueType = ValueType::parseDecl(typeTok, settings, true); // TODO: set isCpp
    if (valueType.pointer > 0)
        return settings->sizeof_pointer;
    if (valueType.type == ValueType::Type::BOOL || valueType.type == ValueType::Type::CHAR)
        return 1;
    if (valueType.type == ValueType::Type::SHORT)
        return settings->sizeof_short;
    if (valueType.type == ValueType::Type::INT)
        return settings->sizeof_int;
    if (valueType.type == ValueType::Type::LONG)
        return settings->sizeof_long;
    if (valueType.type == ValueType::Type::LONGLONG)
        return settings->sizeof_long_long;
    if (valueType.type == ValueType::Type::WCHAR_T)
        return settings->sizeof_wchar_t;

    return 0;
}

// Handle various constants..
Token * ValueFlow::valueFlowSetConstantValue(Token *tok, const Settings *settings, bool cpp)
{
    if ((tok->isNumber() && MathLib::isInt(tok->str())) || (tok->tokType() == Token::eChar)) {
        try {
            MathLib::bigint signedValue = MathLib::toLongNumber(tok->str());
            const ValueType* vt = tok->valueType();
            if (vt && vt->sign == ValueType::UNSIGNED && signedValue < 0 && getSizeOf(*vt, settings) < sizeof(MathLib::bigint)) {
                MathLib::bigint minValue{}, maxValue{};
                if (getMinMaxValues(tok->valueType(), *settings, &minValue, &maxValue))
                    signedValue += maxValue + 1;
            }
            Value value(signedValue);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, value, settings);
        } catch (const std::exception & /*e*/) {
            // Bad character literal
        }
    } else if (tok->isNumber() && MathLib::isFloat(tok->str())) {
        Value value;
        value.valueType = Value::ValueType::FLOAT;
        value.floatValue = MathLib::toDoubleNumber(tok->str());
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (tok->enumerator() && tok->enumerator()->value_known) {
        Value value(tok->enumerator()->value);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (tok->str() == "NULL" || (cpp && tok->str() == "nullptr")) {
        Value value(0);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (Token::simpleMatch(tok, "sizeof (")) {
        if (tok->next()->astOperand2() && !tok->next()->astOperand2()->isLiteral() && tok->next()->astOperand2()->valueType() &&
            (tok->next()->astOperand2()->valueType()->pointer == 0 || // <- TODO this is a bailout, abort when there are array->pointer conversions
             (tok->next()->astOperand2()->variable() && !tok->next()->astOperand2()->variable()->isArray())) &&
            !tok->next()->astOperand2()->valueType()->isEnum()) { // <- TODO this is a bailout, handle enum with non-int types
            const size_t sz = getSizeOf(*tok->next()->astOperand2()->valueType(), settings);
            if (sz) {
                Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), value, settings);
                return tok->linkAt(1);
            }
        }

        const Token *tok2 = tok->tokAt(2);
        // skip over tokens to find variable or type
        while (tok2 && !tok2->isStandardType() && Token::Match(tok2, "%name% ::|.|[")) {
            if (tok2->next()->str() == "[")
                tok2 = tok2->linkAt(1)->next();
            else
                tok2 = tok2->tokAt(2);
        }
        if (Token::simpleMatch(tok, "sizeof ( *")) {
            const ValueType *vt = tok->tokAt(2)->valueType();
            const size_t sz = vt ? getSizeOf(*vt, settings) : 0;
            if (sz > 0) {
                Value value(sz);
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        } else if (tok2->enumerator() && tok2->enumerator()->scope) {
            long long size = settings->sizeof_int;
            const Token * type = tok2->enumerator()->scope->enumType;
            if (type) {
                size = getSizeOfType(type, settings);
                if (size == 0)
                    tok->linkAt(1);
            }
            Value value(size);
            if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), value, settings);
        } else if (tok2->type() && tok2->type()->isEnumType()) {
            long long size = settings->sizeof_int;
            if (tok2->type()->classScope) {
                const Token * type = tok2->type()->classScope->enumType;
                if (type) {
                    size = getSizeOfType(type, settings);
                }
            }
            Value value(size);
            if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), value, settings);
        } else if (Token::Match(tok, "sizeof ( %var% ) /") && tok->next()->astParent() == tok->tokAt(4) &&
                   tok->tokAt(4)->astOperand2() && Token::simpleMatch(tok->tokAt(4)->astOperand2()->previous(), "sizeof (")) {
            // Get number of elements in array
            const Token *sz1 = tok->tokAt(2);
            const Token *sz2 = tok->tokAt(4)->astOperand2(); // left parenthesis of sizeof on rhs
            const nonneg int varid1 = sz1->varId();
            if (varid1 &&
                sz1->variable() &&
                sz1->variable()->isArray() &&
                !sz1->variable()->dimensions().empty() &&
                sz1->variable()->dimensionKnown(0) &&
                Token::Match(sz2->astOperand2(), "*|[") && Token::Match(sz2->astOperand2()->astOperand1(), "%varid%", varid1)) {
                Value value(sz1->variable()->dimension(0));
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->tokAt(4), value, settings);
            }
        } else if (Token::Match(tok2, "%var% )")) {
            const Variable *var = tok2->variable();
            // only look for single token types (no pointers or references yet)
            if (var && var->typeStartToken() == var->typeEndToken()) {
                // find the size of the type
                size_t size = 0;
                if (var->isEnumType()) {
                    size = settings->sizeof_int;
                    if (var->type()->classScope && var->type()->classScope->enumType)
                        size = getSizeOfType(var->type()->classScope->enumType, settings);
                } else if (var->valueType()) {
                    size = getSizeOf(*var->valueType(), settings);
                } else if (!var->type()) {
                    size = getSizeOfType(var->typeStartToken(), settings);
                }
                // find the number of elements
                size_t count = 1;
                for (size_t i = 0; i < var->dimensions().size(); ++i) {
                    if (var->dimensionKnown(i))
                        count *= var->dimension(i);
                    else
                        count = 0;
                }
                if (size && count > 0) {
                    Value value(count * size);
                    if (settings->platformType != cppcheck::Platform::Unspecified)
                        value.setKnown();
                    setTokenValue(tok, value, settings);
                    setTokenValue(tok->next(), value, settings);
                }
            }
        } else if (tok2->tokType() == Token::eString) {
            const size_t sz = Token::getStrSize(tok2, settings);
            if (sz > 0) {
                Value value(sz);
                value.setKnown();
                setTokenValue(const_cast<Token *>(tok->next()), value, settings);
            }
        } else if (tok2->tokType() == Token::eChar) {
            nonneg int sz = 0;
            if (cpp && settings->standards.cpp >= Standards::CPP20 && tok2->isUtf8())
                sz = 1;
            else if (tok2->isUtf16())
                sz = 2;
            else if (tok2->isUtf32())
                sz = 4;
            else if (tok2->isLong())
                sz = settings->sizeof_wchar_t;
            else if ((tok2->isCChar() && !cpp) || (tok2->isCMultiChar()))
                sz = settings->sizeof_int;
            else
                sz = 1;

            if (sz > 0) {
                Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        } else if (!tok2->type()) {
            const ValueType& vt = ValueType::parseDecl(tok2, settings, true); // TODO: set isCpp
            size_t sz = getSizeOf(vt, settings);
            const Token* brac = tok2->astParent();
            while (Token::simpleMatch(brac, "[")) {
                const Token* num = brac->astOperand2();
                if (num && ((num->isNumber() && MathLib::isInt(num->str())) || num->tokType() == Token::eChar)) {
                    try {
                        const MathLib::biguint dim = MathLib::toULongNumber(num->str());
                        sz *= dim;
                        brac = brac->astParent();
                        continue;
                    }
                    catch (const std::exception& /*e*/) {
                        // Bad integer literal
                    }
                }
                sz = 0;
                break;
            }
            if (sz > 0) {
                Value value(sz);
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        }
        // skip over enum
        tok = tok->linkAt(1);
    }
    return tok->next();
}

static Value truncateImplicitConversion(Token* parent, const Value& value, const Settings* settings)
{
    if (!value.isIntValue() && !value.isFloatValue())
        return value;
    if (!parent)
        return value;
    if (!parent->isBinaryOp())
        return value;
    if (!parent->isConstOp())
        return value;
    if (!astIsIntegral(parent->astOperand1(), false))
        return value;
    if (!astIsIntegral(parent->astOperand2(), false))
        return value;
    const ValueType* vt1 = parent->astOperand1()->valueType();
    const ValueType* vt2 = parent->astOperand2()->valueType();
    // If the sign is the same there is no truncation
    if (vt1->sign == vt2->sign)
        return value;
    const size_t n1 = getSizeOf(*vt1, settings);
    const size_t n2 = getSizeOf(*vt2, settings);
    ValueType::Sign sign = ValueType::Sign::UNSIGNED;
    if (n1 < n2)
        sign = vt2->sign;
    else if (n1 > n2)
        sign = vt1->sign;
    Value v = castValue(value, sign, std::max(n1, n2) * 8);
    v.wideintvalue = value.intvalue;
    return v;
}

Value ValueFlow::castValue(Value value, const ValueType::Sign sign, nonneg int bit)
{
    if (value.isFloatValue()) {
        value.valueType = Value::ValueType::INT;
        if (value.floatValue >= std::numeric_limits<int>::min() && value.floatValue <= std::numeric_limits<int>::max()) {
            value.intvalue = value.floatValue;
        } else { // don't perform UB
            value.intvalue = 0;
        }
    }
    if (bit < MathLib::bigint_bits) {
        const MathLib::biguint one = 1;
        value.intvalue &= (one << bit) - 1;
        if (sign == ValueType::Sign::SIGNED && value.intvalue & (one << (bit - 1))) {
            value.intvalue |= ~((one << bit) - 1ULL);
        }
    }
    return value;
}

std::string ValueFlow::debugString(const Value& v)
{
    std::string kind;
    switch (v.valueKind) {
    case Value::ValueKind::Impossible:
    case Value::ValueKind::Known:
        kind = "always";
        break;
    case Value::ValueKind::Inconclusive:
        kind = "inconclusive";
        break;
    case Value::ValueKind::Possible:
        kind = "possible";
        break;
    }
    return kind + " " + v.toString();
}

void ValueFlow::setSourceLocation(Value& v,
                                  SourceLocation ctx,
                                  const Token* tok,
                                  SourceLocation local)
{
    std::string file = ctx.file_name();
    if (file.empty())
        return;
    std::string s = Path::stripDirectoryPart(file) + ":" + MathLib::toString(ctx.line()) + ": " + ctx.function_name() +
                    " => " + local.function_name() + ": " + debugString(v);
    v.debugPath.emplace_back(tok, std::move(s));
}

static const Token *getCastTypeStartToken(const Token *parent, const Settings* settings)
{
    // TODO: This might be a generic utility function?
    if (!Token::Match(parent, "{|("))
        return nullptr;
    // Functional cast
    if (parent->isBinaryOp() && Token::Match(parent->astOperand1(), "%type% (|{") &&
        parent->astOperand1()->tokType() == Token::eType && astIsPrimitive(parent))
        return parent->astOperand1();
    if (parent->str() != "(")
        return nullptr;
    if (!parent->astOperand2() && Token::Match(parent, "( %name%") &&
        (parent->next()->isStandardType() || settings->library.isNotLibraryFunction(parent->next())))
        return parent->next();
    if (parent->astOperand2() && Token::Match(parent->astOperand1(), "const_cast|dynamic_cast|reinterpret_cast|static_cast <"))
        return parent->astOperand1()->tokAt(2);
    return nullptr;
}

static bool isNumeric(const Value& value) {
    return value.isIntValue() || value.isFloatValue();
}

/** Set token value for cast */
static void setTokenValueCast(Token *parent, const ValueType &valueType, const Value &value, const Settings *settings)
{
    if (valueType.pointer || value.isImpossible())
        setTokenValue(parent,value,settings);
    else if (valueType.type == ValueType::Type::CHAR)
        setTokenValue(parent, castValue(value, valueType.sign, settings->char_bit), settings);
    else if (valueType.type == ValueType::Type::SHORT)
        setTokenValue(parent, castValue(value, valueType.sign, settings->short_bit), settings);
    else if (valueType.type == ValueType::Type::INT)
        setTokenValue(parent, castValue(value, valueType.sign, settings->int_bit), settings);
    else if (valueType.type == ValueType::Type::LONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings->long_bit), settings);
    else if (valueType.type == ValueType::Type::LONGLONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings->long_long_bit), settings);
    else if (valueType.isFloat() && isNumeric(value)) {
        Value floatValue = value;
        floatValue.valueType = Value::ValueType::FLOAT;
        if (value.isIntValue())
            floatValue.floatValue = value.intvalue;
        setTokenValue(parent, floatValue, settings);
    } else if (value.isIntValue()) {
        const long long charMax = settings->signedCharMax();
        const long long charMin = settings->signedCharMin();
        if (charMin <= value.intvalue && value.intvalue <= charMax) {
            // unknown type, but value is small so there should be no truncation etc
            setTokenValue(parent,value,settings);
        }
    }
}

// does the operation cause a loss of information?
static bool isNonInvertibleOperation(const Token* tok)
{
    return !Token::Match(tok, "+|-");
}

static bool isComputableValue(const Token* parent, const Value& value)
{
    const bool noninvertible = isNonInvertibleOperation(parent);
    if (noninvertible && value.isImpossible())
        return false;
    if (!value.isIntValue() && !value.isFloatValue() && !value.isTokValue() && !value.isIteratorValue())
        return false;
    if (value.isIteratorValue() && !Token::Match(parent, "+|-"))
        return false;
    if (value.isTokValue() && (!parent->isComparisonOp() || value.tokvalue->tokType() != Token::eString))
        return false;
    return true;
}

static bool isCompatibleValueTypes(Value::ValueType x, Value::ValueType y)
{
    static const std::unordered_map<Value::ValueType,
                                    std::unordered_set<Value::ValueType, EnumClassHash>,
                                    EnumClassHash>
    compatibleTypes = {
        {Value::ValueType::INT,
         {Value::ValueType::FLOAT,
              Value::ValueType::SYMBOLIC,
              Value::ValueType::TOK}},
        {Value::ValueType::FLOAT, {Value::ValueType::INT}},
        {Value::ValueType::TOK, {Value::ValueType::INT}},
        {Value::ValueType::ITERATOR_START, {Value::ValueType::INT}},
        {Value::ValueType::ITERATOR_END, {Value::ValueType::INT}},
    };
    if (x == y)
        return true;
    auto it = compatibleTypes.find(x);
    if (it == compatibleTypes.end())
        return false;
    return it->second.count(y) > 0;
}

static bool isCompatibleValues(const Value& value1, const Value& value2)
{
    if (value1.isSymbolicValue() && value2.isSymbolicValue() && value1.tokvalue->exprId() != value2.tokvalue->exprId())
        return false;
    if (!isCompatibleValueTypes(value1.valueType, value2.valueType))
        return false;
    if (value1.isKnown() || value2.isKnown())
        return true;
    if (value1.isImpossible() || value2.isImpossible())
        return false;
    if (value1.varId == 0 || value2.varId == 0)
        return true;
    if (value1.varId == value2.varId && value1.varvalue == value2.varvalue && value1.isIntValue() && value2.isIntValue())
        return true;
    return false;
}

/** set ValueFlow value and perform calculations if possible */
void ValueFlow::setTokenValue(Token* tok,
                              Value value,
                              const Settings* settings,
                              SourceLocation loc)
{
    // Skip setting values that are too big since its ambiguous
    if (!value.isImpossible() && value.isIntValue() && value.intvalue < 0 && astIsUnsigned(tok) &&
        getSizeOf(*tok->valueType(), settings) >= sizeof(MathLib::bigint))
        return;

    if (!value.isImpossible() && value.isIntValue())
        value = truncateImplicitConversion(tok->astParent(), value, settings);

    if (settings->debugnormal)
        setSourceLocation(value, loc, tok);

    if (!tok->addValue(value))
        return;

    if (value.path < 0)
        return;

    Token *parent = tok->astParent();
    if (!parent)
        return;

    if (Token::simpleMatch(parent, ",") && astIsRHS(tok)) {
        const Token* callParent = findParent(parent, [](const Token* p) {
            return !Token::simpleMatch(p, ",");
        });
        // Ensure that the comma isn't a function call
        if (!callParent || (!Token::Match(callParent->previous(), "%name%|> (") && !Token::simpleMatch(callParent, "{") &&
                            (!Token::Match(callParent, "( %name%") || settings->library.isNotLibraryFunction(callParent->next())))) {
            setTokenValue(parent, std::move(value), settings);
            return;
        }
    }

    if (Token::simpleMatch(parent, "=") && astIsRHS(tok) && !value.isLifetimeValue()) {
        setTokenValue(parent, value, settings);
        return;
    }

    if (value.isContainerSizeValue() && astIsContainer(tok)) {
        // .empty, .size, +"abc", +'a'
        if (Token::Match(parent, "+|==|!=") && parent->astOperand1() && parent->astOperand2()) {
            for (const Value &value1 : parent->astOperand1()->values()) {
                if (value1.isImpossible())
                    continue;
                for (const Value &value2 : parent->astOperand2()->values()) {
                    if (value2.isImpossible())
                        continue;
                    if (value1.path != value2.path)
                        continue;
                    Value result;
                    if (Token::Match(parent, "%comp%"))
                        result.valueType = Value::ValueType::INT;
                    else
                        result.valueType = Value::ValueType::CONTAINER_SIZE;

                    if (value1.isContainerSizeValue() && value2.isContainerSizeValue())
                        result.intvalue = calculate(parent->str(), value1.intvalue, value2.intvalue);
                    else if (value1.isContainerSizeValue() && value2.isTokValue() && value2.tokvalue->tokType() == Token::eString)
                        result.intvalue = calculate(parent->str(), value1.intvalue, MathLib::bigint(Token::getStrLength(value2.tokvalue)));
                    else if (value2.isContainerSizeValue() && value1.isTokValue() && value1.tokvalue->tokType() == Token::eString)
                        result.intvalue = calculate(parent->str(), MathLib::bigint(Token::getStrLength(value1.tokvalue)), value2.intvalue);
                    else
                        continue;

                    combineValueProperties(value1, value2, &result);

                    if (Token::simpleMatch(parent, "==") && result.intvalue)
                        continue;
                    if (Token::simpleMatch(parent, "!=") && !result.intvalue)
                        continue;

                    setTokenValue(parent, result, settings);
                }
            }
        }

        else if (Token::Match(parent, ". %name% (") && parent->astParent() == parent->tokAt(2) &&
                 parent->astOperand1() && parent->astOperand1()->valueType()) {
            const Library::Container* c = getLibraryContainer(parent->astOperand1());
            const Library::Container::Yield yields = c ? c->getYield(parent->strAt(1)) : Library::Container::Yield::NO_YIELD;
            if (yields == Library::Container::Yield::SIZE) {
                Value v(value);
                v.valueType = Value::ValueType::INT;
                setTokenValue(parent->astParent(), v, settings);
            } else if (yields == Library::Container::Yield::EMPTY) {
                Value v(value);
                v.intvalue = !v.intvalue;
                v.valueType = Value::ValueType::INT;
                setTokenValue(parent->astParent(), v, settings);
            }
        } else if (Token::Match(parent->previous(), "%name% (")) {
            if (const Library::Function* f = settings->library.getFunction(parent->previous())) {
                if (f->containerYield == Library::Container::Yield::SIZE) {
                    Value v(value);
                    v.valueType = Value::ValueType::INT;
                    setTokenValue(parent, v, settings);
                } else if (f->containerYield == Library::Container::Yield::EMPTY) {
                    Value v(value);
                    v.intvalue = !v.intvalue;
                    v.valueType = Value::ValueType::INT;
                    setTokenValue(parent, v, settings);
                }
            }
        }

        return;
    }

    if (value.isLifetimeValue()) {
        if (!isLifetimeBorrowed(parent, settings))
            return;
        if (value.lifetimeKind == Value::LifetimeKind::Iterator && astIsIterator(parent)) {
            setTokenValue(parent,value,settings);
        } else if (astIsPointer(tok) && astIsPointer(parent) && !parent->isUnaryOp("*") &&
                   (parent->isArithmeticalOp() || parent->isCast())) {
            setTokenValue(parent,value,settings);
        }
        return;
    }

    if (value.isUninitValue()) {
        if (Token::Match(tok, ". %var%"))
            setTokenValue(tok->next(), value, settings);
        if (parent->isCast()) {
            setTokenValue(parent, value, settings);
            return;
        }
        Value pvalue = value;
        if (!value.subexpressions.empty() && Token::Match(parent, ". %var%")) {
            if (contains(value.subexpressions, parent->next()->str()))
                pvalue.subexpressions.clear();
            else
                return;
        }
        if (parent->isUnaryOp("&")) {
            pvalue.indirect++;
            setTokenValue(parent, pvalue, settings);
        } else if (Token::Match(parent, ". %var%") && parent->astOperand1() == tok && parent->astOperand2()) {
            if (parent->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astOperand2(), pvalue, settings);
        } else if (Token::Match(parent->astParent(), ". %var%") && parent->astParent()->astOperand1() == parent) {
            if (parent->astParent()->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astParent()->astOperand2(), pvalue, settings);
        } else if (parent->isUnaryOp("*") && pvalue.indirect > 0) {
            pvalue.indirect--;
            setTokenValue(parent, pvalue, settings);
        }
        return;
    }

    // cast..
    if (const Token *castType = getCastTypeStartToken(parent, settings)) {
        if (contains({Value::ValueType::INT, Value::ValueType::SYMBOLIC}, value.valueType) &&
            Token::simpleMatch(parent->astOperand1(), "dynamic_cast"))
            return;
        const ValueType &valueType = ValueType::parseDecl(castType, settings, true); // TODO: set isCpp
        if (value.isImpossible() && value.isIntValue() && value.intvalue < 0 && astIsUnsigned(tok) &&
            valueType.sign == ValueType::SIGNED && tok->valueType() &&
            getSizeOf(*tok->valueType(), settings) >= getSizeOf(valueType, settings))
            return;
        setTokenValueCast(parent, valueType, value, settings);
    }

    else if (parent->str() == ":") {
        setTokenValue(parent,value,settings);
    }

    else if (parent->str() == "?" && tok->str() == ":" && tok == parent->astOperand2() && parent->astOperand1()) {
        // is condition always true/false?
        if (parent->astOperand1()->hasKnownValue()) {
            const Value &condvalue = parent->astOperand1()->values().front();
            const bool cond(condvalue.isTokValue() || (condvalue.isIntValue() && condvalue.intvalue != 0));
            if (cond && !tok->astOperand1()) { // true condition, no second operator
                setTokenValue(parent, condvalue, settings);
            } else {
                const Token *op = cond ? tok->astOperand1() : tok->astOperand2();
                if (!op) // #7769 segmentation fault at setTokenValue()
                    return;
                const std::list<Value> &values = op->values();
                if (std::find(values.cbegin(), values.cend(), value) != values.cend())
                    setTokenValue(parent, value, settings);
            }
        } else if (!value.isImpossible()) {
            // is condition only depending on 1 variable?
            // cppcheck-suppress[variableScope] #8541
            nonneg int varId = 0;
            bool ret = false;
            visitAstNodes(parent->astOperand1(),
                          [&](const Token *t) {
                if (t->varId()) {
                    if (varId > 0 || value.varId != 0)
                        ret = true;
                    varId = t->varId();
                } else if (t->str() == "(" && Token::Match(t->previous(), "%name%"))
                    ret = true;               // function call
                return ret ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
            });
            if (ret)
                return;

            Value v(value);
            v.conditional = true;
            v.changeKnownToPossible();

            setTokenValue(parent, v, settings);
        }
    }

    else if (parent->str() == "?" && value.isIntValue() && tok == parent->astOperand1() && value.isKnown() &&
             parent->astOperand2() && parent->astOperand2()->astOperand1() && parent->astOperand2()->astOperand2()) {
        const std::list<Value> &values = (value.intvalue == 0
                                                     ? parent->astOperand2()->astOperand2()->values()
                                                     : parent->astOperand2()->astOperand1()->values());

        for (const Value &v : values)
            setTokenValue(parent, v, settings);
    }

    // Calculations..
    else if ((parent->isArithmeticalOp() || parent->isComparisonOp() || (parent->tokType() == Token::eBitOp) || (parent->tokType() == Token::eLogicalOp)) &&
             parent->astOperand1() &&
             parent->astOperand2()) {

        const bool noninvertible = isNonInvertibleOperation(parent);

        // Skip operators with impossible values that are not invertible
        if (noninvertible && value.isImpossible())
            return;

        // known result when a operand is 0.
        if (Token::Match(parent, "[&*]") && astIsIntegral(parent, true) && value.isKnown() && value.isIntValue() &&
            value.intvalue == 0) {
            setTokenValue(parent, value, settings);
            return;
        }

        // known result when a operand is true.
        if (Token::simpleMatch(parent, "&&") && value.isKnown() && value.isIntValue() && value.intvalue==0) {
            setTokenValue(parent, value, settings);
            return;
        }

        // known result when a operand is false.
        if (Token::simpleMatch(parent, "||") && value.isKnown() && value.isIntValue() && value.intvalue!=0) {
            setTokenValue(parent, value, settings);
            return;
        }

        for (const Value &value1 : parent->astOperand1()->values()) {
            if (!isComputableValue(parent, value1))
                continue;
            for (const Value &value2 : parent->astOperand2()->values()) {
                if (value1.path != value2.path)
                    continue;
                if (!isComputableValue(parent, value2))
                    continue;
                if (value1.isIteratorValue() && value2.isIteratorValue())
                    continue;
                if (!isCompatibleValues(value1, value2))
                    continue;
                Value result(0);
                combineValueProperties(value1, value2, &result);
                if (astIsFloat(parent, false)) {
                    if (!result.isIntValue() && !result.isFloatValue())
                        continue;
                    result.valueType = Value::ValueType::FLOAT;
                }
                const double floatValue1 = value1.isFloatValue() ? value1.floatValue : value1.intvalue;
                const double floatValue2 = value2.isFloatValue() ? value2.floatValue : value2.intvalue;
                const auto intValue1 = [&]() -> MathLib::bigint {
                    return value1.isFloatValue() ? static_cast<MathLib::bigint>(value1.floatValue) : value1.intvalue;
                };
                const auto intValue2 = [&]() -> MathLib::bigint {
                    return value2.isFloatValue() ? static_cast<MathLib::bigint>(value2.floatValue) : value2.intvalue;
                };
                if ((value1.isFloatValue() || value2.isFloatValue()) && Token::Match(parent, "&|^|%|<<|>>|==|!=|%or%"))
                    continue;
                if (Token::Match(parent, "==|!=")) {
                    if ((value1.isIntValue() && value2.isTokValue()) || (value1.isTokValue() && value2.isIntValue())) {
                        if (parent->str() == "==")
                            result.intvalue = 0;
                        else if (parent->str() == "!=")
                            result.intvalue = 1;
                    } else if (value1.isIntValue() && value2.isIntValue()) {
                        bool error = false;
                        result.intvalue = calculate(parent->str(), intValue1(), intValue2(), &error);
                        if (error)
                            continue;
                    } else {
                        continue;
                    }
                    setTokenValue(parent, result, settings);
                } else if (Token::Match(parent, "%op%")) {
                    if (Token::Match(parent, "%comp%")) {
                        if (!result.isFloatValue() && !value1.isIntValue() && !value2.isIntValue())
                            continue;
                    } else {
                        if (value1.isTokValue() || value2.isTokValue())
                            break;
                    }
                    bool error = false;
                    if (result.isFloatValue()) {
                        result.floatValue = calculate(parent->str(), floatValue1, floatValue2, &error);
                    } else {
                        result.intvalue = calculate(parent->str(), intValue1(), intValue2(), &error);
                    }
                    if (error)
                        continue;
                    // If the bound comes from the second value then invert the bound when subtracting
                    if (Token::simpleMatch(parent, "-") && value2.bound == result.bound &&
                        value2.bound != Value::Bound::Point)
                        result.invertBound();
                    setTokenValue(parent, result, settings);
                }
            }
        }
    }

    // !
    else if (parent->str() == "!") {
        for (const Value &val : tok->values()) {
            if (!val.isIntValue())
                continue;
            if (val.isImpossible() && val.intvalue != 0)
                continue;
            Value v(val);
            v.intvalue = !v.intvalue;
            setTokenValue(parent, v, settings);
        }
    }

    // ~
    else if (parent->str() == "~") {
        for (const Value &val : tok->values()) {
            if (!val.isIntValue())
                continue;
            Value v(val);
            v.intvalue = ~v.intvalue;
            int bits = 0;
            if (tok->valueType() &&
                tok->valueType()->sign == ValueType::Sign::UNSIGNED &&
                tok->valueType()->pointer == 0) {
                if (tok->valueType()->type == ValueType::Type::INT)
                    bits = settings->int_bit;
                else if (tok->valueType()->type == ValueType::Type::LONG)
                    bits = settings->long_bit;
            }
            if (bits > 0 && bits < MathLib::bigint_bits)
                v.intvalue &= (((MathLib::biguint)1)<<bits) - 1;
            setTokenValue(parent, v, settings);
        }
    }

    // unary minus
    else if (parent->isUnaryOp("-")) {
        for (const Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue())
                continue;
            Value v(val);
            if (v.isIntValue()) {
                if (v.intvalue == LLONG_MIN)
                    // Value can't be inverted
                    continue;
                v.intvalue = -v.intvalue;
            } else
                v.floatValue = -v.floatValue;
            v.invertBound();
            setTokenValue(parent, v, settings);
        }
    }

    // increment
    else if (parent->str() == "++") {
        for (const Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue() && !val.isSymbolicValue())
                continue;
            Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue() || v.isSymbolicValue())
                    v.intvalue = v.intvalue + 1;
                else
                    v.floatValue = v.floatValue + 1.0;
            }
            setTokenValue(parent, v, settings);
        }
    }

    // decrement
    else if (parent->str() == "--") {
        for (const Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue() && !val.isSymbolicValue())
                continue;
            Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue() || v.isSymbolicValue())
                    v.intvalue = v.intvalue - 1;
                else
                    v.floatValue = v.floatValue - 1.0;
            }
            setTokenValue(parent, v, settings);
        }
    }

    else if (Token::Match(parent, ":: %name%") && parent->astOperand2() == tok) {
        setTokenValue(parent, value, settings);
    }
    // Calling std::size or std::empty on an array
    else if (value.isTokValue() && Token::simpleMatch(value.tokvalue, "{") && tok->variable() &&
             tok->variable()->isArray() && Token::Match(parent->previous(), "%name% (") && astIsRHS(tok)) {
        std::vector<const Token*> args = getArguments(value.tokvalue);
        if (const Library::Function* f = settings->library.getFunction(parent->previous())) {
            if (f->containerYield == Library::Container::Yield::SIZE) {
                Value v(value);
                v.valueType = Value::ValueType::INT;
                v.intvalue = args.size();
                setTokenValue(parent, v, settings);
            } else if (f->containerYield == Library::Container::Yield::EMPTY) {
                Value v(value);
                v.intvalue = args.empty();
                v.valueType = Value::ValueType::INT;
                setTokenValue(parent, v, settings);
            }
        }
    }
}
