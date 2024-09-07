/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "vf_infercondition.h"

#include "astutils.h"
#include "infer.h"
#include "mathlib.h"
#include "token.h"
#include "tokenlist.h"
#include "valueflow.h"
#include "valueptr.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <array>
#include <list>
#include <utility>
#include <vector>

namespace ValueFlow
{
    struct IteratorInferModel : InferModel {
        virtual Value::ValueType getType() const = 0;
        bool match(const Value& value) const override {
            return value.valueType == getType();
        }
        Value yield(MathLib::bigint value) const override
        {
            Value result(value);
            result.valueType = getType();
            result.setKnown();
            return result;
        }
    };

    struct EndIteratorInferModel : IteratorInferModel {
        Value::ValueType getType() const override {
            return Value::ValueType::ITERATOR_END;
        }
    };

    struct StartIteratorInferModel : IteratorInferModel {
        Value::ValueType getType() const override {
            return Value::ValueType::ITERATOR_END;
        }
    };

    static bool isIntegralOnlyOperator(const Token* tok) {
        return Token::Match(tok, "%|<<|>>|&|^|~|%or%");
    }

    static bool isIntegralOrPointer(const Token* tok)
    {
        if (!tok)
            return false;
        if (astIsIntegral(tok, false))
            return true;
        if (astIsPointer(tok))
            return true;
        if (Token::Match(tok, "NULL|nullptr"))
            return true;
        if (tok->valueType())
            return false;
        // These operators only work on integers
        if (isIntegralOnlyOperator(tok))
            return true;
        if (isIntegralOnlyOperator(tok->astParent()))
            return true;
        if (Token::Match(tok, "+|-|*|/") && tok->isBinaryOp())
            return isIntegralOrPointer(tok->astOperand1()) && isIntegralOrPointer(tok->astOperand2());
        return false;
    }

    void analyzeInferCondition(TokenList& tokenlist, const Settings& settings)
    {
        for (Token* tok = tokenlist.front(); tok; tok = tok->next()) {
            if (!tok->astParent())
                continue;
            if (tok->hasKnownIntValue())
                continue;
            if (Token::Match(tok, "%comp%|-") && tok->astOperand1() && tok->astOperand2()) {
                if (astIsIterator(tok->astOperand1()) || astIsIterator(tok->astOperand2())) {
                    static const std::array<ValuePtr<InferModel>, 2> iteratorModels = {EndIteratorInferModel{},
                                                                                       StartIteratorInferModel{}};
                    for (const ValuePtr<InferModel>& model : iteratorModels) {
                        std::vector<Value> result =
                            infer(model, tok->str(), tok->astOperand1()->values(), tok->astOperand2()->values());
                        for (Value value : result) {
                            value.valueType = Value::ValueType::INT;
                            setTokenValue(tok, std::move(value), settings);
                        }
                    }
                } else if (isIntegralOrPointer(tok->astOperand1()) && isIntegralOrPointer(tok->astOperand2())) {
                    std::vector<Value> result =
                        infer(makeIntegralInferModel(), tok->str(), tok->astOperand1()->values(), tok->astOperand2()->values());
                    for (Value& value : result) {
                        setTokenValue(tok, std::move(value), settings);
                    }
                }
            } else if (Token::Match(tok->astParent(), "?|&&|!|%oror%") ||
                       Token::Match(tok->astParent()->previous(), "if|while (") ||
                       (astIsPointer(tok) && isUsedAsBool(tok, settings))) {
                std::vector<Value> result = infer(makeIntegralInferModel(), "!=", tok->values(), 0);
                if (result.size() != 1)
                    continue;
                Value value = result.front();
                setTokenValue(tok, std::move(value), settings);
            }
        }
    }
}
