/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "vfvalue.h"

#include "errortypes.h"
#include "token.h"

#include <sstream>
#include <string>

ValueFlow::Value::Value(const Token* c, long long val, Bound b)
    : valueType(ValueType::INT),
    bound(b),
    intvalue(val),
    tokvalue(nullptr),
    floatValue(0.0),
    varvalue(val),
    condition(c),
    varId(0),
    safe(false),
    conditional(false),
    macro(false),
    defaultArg(false),
    indirect(0),
    moveKind(MoveKind::NonMovedVariable),
    path(0),
    wideintvalue(0),
    subexpressions(),
    capturetok(nullptr),
    lifetimeKind(LifetimeKind::Object),
    lifetimeScope(LifetimeScope::Local),
    valueKind(ValueKind::Possible)
{
    errorPath.emplace_back(c, "Assuming that condition '" + c->expressionString() + "' is not redundant");
}

void ValueFlow::Value::assumeCondition(const Token* tok)
{
    condition = tok;
    errorPath.emplace_back(tok, "Assuming that condition '" + tok->expressionString() + "' is not redundant");
}

std::string ValueFlow::Value::toString() const
{
    std::stringstream ss;
    if (this->isImpossible())
        ss << "!";
    if (this->bound == ValueFlow::Value::Bound::Lower)
        ss << ">=";
    if (this->bound == ValueFlow::Value::Bound::Upper)
        ss << "<=";
    switch (this->valueType) {
    case ValueFlow::Value::ValueType::INT:
        ss << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::TOK:
        ss << this->tokvalue->str();
        break;
    case ValueFlow::Value::ValueType::FLOAT:
        ss << this->floatValue;
        break;
    case ValueFlow::Value::ValueType::MOVED:
        ss << ValueFlow::Value::toString(this->moveKind);
        break;
    case ValueFlow::Value::ValueType::UNINIT:
        ss << "Uninit";
        break;
    case ValueFlow::Value::ValueType::BUFFER_SIZE:
    case ValueFlow::Value::ValueType::CONTAINER_SIZE:
        ss << "size=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::ITERATOR_START:
        ss << "start=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::ITERATOR_END:
        ss << "end=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::LIFETIME:
        ss << "lifetime[" << ValueFlow::Value::toString(this->lifetimeKind) << "]=("
           << this->tokvalue->expressionString() << ")";
        break;
    case ValueFlow::Value::ValueType::SYMBOLIC:
        ss << "symbolic=(" << this->tokvalue->expressionString();
        if (this->intvalue > 0)
            ss << "+" << this->intvalue;
        else if (this->intvalue < 0)
            ss << "-" << -this->intvalue;
        ss << ")";
        break;
    }
    if (this->indirect > 0)
        for (int i = 0; i < this->indirect; i++)
            ss << "*";
    if (this->path > 0)
        ss << "@" << this->path;
    return ss.str();
}

std::string ValueFlow::Value::infoString() const
{
    switch (valueType) {
    case ValueType::INT:
        return MathLib::toString(intvalue);
    case ValueType::TOK:
        return tokvalue->str();
    case ValueType::FLOAT:
        return MathLib::toString(floatValue);
    case ValueType::MOVED:
        return "<Moved>";
    case ValueType::UNINIT:
        return "<Uninit>";
    case ValueType::BUFFER_SIZE:
    case ValueType::CONTAINER_SIZE:
        return "size=" + MathLib::toString(intvalue);
    case ValueType::ITERATOR_START:
        return "start=" + MathLib::toString(intvalue);
    case ValueType::ITERATOR_END:
        return "end=" + MathLib::toString(intvalue);
    case ValueType::LIFETIME:
        return "lifetime=" + tokvalue->str();
    case ValueType::SYMBOLIC:
        std::string result = "symbolic=" + tokvalue->expressionString();
        if (intvalue > 0)
            result += "+" + MathLib::toString(intvalue);
        else if (intvalue < 0)
            result += "-" + MathLib::toString(-intvalue);
        return result;
    }
    throw InternalError(nullptr, "Invalid ValueFlow Value type");
}

const char* ValueFlow::Value::toString(MoveKind moveKind)
{
    switch (moveKind) {
    case MoveKind::NonMovedVariable:
        return "NonMovedVariable";
    case MoveKind::MovedVariable:
        return "MovedVariable";
    case MoveKind::ForwardedVariable:
        return "ForwardedVariable";
    }
    return "";
}

const char* ValueFlow::Value::toString(LifetimeKind lifetimeKind)
{
    switch (lifetimeKind) {
    case LifetimeKind::Object:
        return "Object";
    case LifetimeKind::SubObject:
        return "SubObject";
    case LifetimeKind::Lambda:
        return "Lambda";
    case LifetimeKind::Iterator:
        return "Iterator";
    case LifetimeKind::Address:
        return "Address";
    }
    return "";
}

bool ValueFlow::Value::sameToken(const Token* tok1, const Token* tok2)
{
    if (tok1 == tok2)
        return true;
    if (!tok1)
        return false;
    if (tok1->exprId() == 0 || tok2->exprId() == 0)
        return false;
    return tok1->exprId() == tok2->exprId();
}

const char* ValueFlow::Value::toString(LifetimeScope lifetimeScope)
{
    switch (lifetimeScope) {
    case ValueFlow::Value::LifetimeScope::Local:
        return "Local";
    case ValueFlow::Value::LifetimeScope::Argument:
        return "Argument";
    case ValueFlow::Value::LifetimeScope::SubFunction:
        return "SubFunction";
    case ValueFlow::Value::LifetimeScope::ThisPointer:
        return "ThisPointer";
    case ValueFlow::Value::LifetimeScope::ThisValue:
        return "ThisValue";
    }
    return "";
}

const char* ValueFlow::Value::toString(Bound bound)
{
    switch (bound) {
    case ValueFlow::Value::Bound::Point:
        return "Point";
    case ValueFlow::Value::Bound::Upper:
        return "Upper";
    case ValueFlow::Value::Bound::Lower:
        return "Lower";
    }
    return "";
}

ValueFlow::Value ValueFlow::Value::unknown()
{
    Value v;
    v.valueType = Value::ValueType::UNINIT;
    return v;
}
