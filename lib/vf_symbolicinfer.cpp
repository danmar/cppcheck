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

#include "vf_symbolicinfer.h"

#include "astutils.h"
#include "infer.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <cassert>
#include <utility>
#include <vector>

namespace ValueFlow
{
    struct SymbolicInferModel : InferModel {
        const Token* expr;
        explicit SymbolicInferModel(const Token* tok) : expr(tok) {
            assert(expr->exprId() != 0);
        }
        bool match(const Value& value) const override
        {
            return value.isSymbolicValue() && value.tokvalue && value.tokvalue->exprId() == expr->exprId();
        }
        Value yield(MathLib::bigint value) const override
        {
            Value result(value);
            result.valueType = Value::ValueType::SYMBOLIC;
            result.tokvalue = expr;
            result.setKnown();
            return result;
        }
    };

    void valueFlowSymbolicInfer(const SymbolDatabase& symboldatabase, const Settings& settings)
    {
        for (const Scope* scope : symboldatabase.functionScopes) {
            for (auto* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (!Token::Match(tok, "-|%comp%"))
                    continue;
                if (tok->hasKnownIntValue())
                    continue;
                if (!tok->astOperand1())
                    continue;
                if (!tok->astOperand2())
                    continue;
                if (tok->astOperand1()->exprId() == 0)
                    continue;
                if (tok->astOperand2()->exprId() == 0)
                    continue;
                if (tok->astOperand1()->hasKnownIntValue())
                    continue;
                if (tok->astOperand2()->hasKnownIntValue())
                    continue;
                if (astIsFloat(tok->astOperand1(), false))
                    continue;
                if (astIsFloat(tok->astOperand2(), false))
                    continue;

                SymbolicInferModel leftModel{tok->astOperand1()};
                std::vector<Value> values = infer(leftModel, tok->str(), 0, tok->astOperand2()->values());
                if (values.empty()) {
                    SymbolicInferModel rightModel{tok->astOperand2()};
                    values = infer(rightModel, tok->str(), tok->astOperand1()->values(), 0);
                }
                for (Value& value : values) {
                    setTokenValue(tok, std::move(value), settings);
                }
            }
        }
    }
}
