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

#include "vf_impossiblevalues.h"

#include "astutils.h"
#include "calculate.h"
#include "library.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace ValueFlow
{
    static std::vector<MathLib::bigint> minUnsignedValue(const Token* tok, int depth = 8)
    {
        std::vector<MathLib::bigint> result;
        if (!tok)
            return result;
        if (depth < 0)
            return result;
        if (tok->hasKnownIntValue()) {
            result = {tok->values().front().intvalue};
        } else if (!Token::Match(tok, "-|%|&|^") && tok->isConstOp() && tok->astOperand1() && tok->astOperand2()) {
            std::vector<MathLib::bigint> op1 = minUnsignedValue(tok->astOperand1(), depth - 1);
            std::vector<MathLib::bigint> op2 = minUnsignedValue(tok->astOperand2(), depth - 1);
            if (!op1.empty() && !op2.empty()) {
                result = calculate<std::vector<MathLib::bigint>>(tok->str(), op1.front(), op2.front());
            }
        }
        if (result.empty() && astIsUnsigned(tok))
            result = {0};
        return result;
    }

    static bool isSameToken(const Token* tok1, const Token* tok2)
    {
        if (!tok1 || !tok2)
            return false;
        if (tok1->exprId() != 0 && tok1->exprId() == tok2->exprId())
            return true;
        if (tok1->hasKnownIntValue() && tok2->hasKnownIntValue())
            return tok1->values().front().intvalue == tok2->values().front().intvalue;
        return false;
    }

    static bool isConvertedToIntegral(const Token* tok, const Settings& settings)
    {
        if (!tok)
            return false;
        std::vector<ValueType> parentTypes = getParentValueTypes(tok, settings);
        if (parentTypes.empty())
            return false;
        const ValueType& vt = parentTypes.front();
        return vt.type != ValueType::UNKNOWN_INT && vt.isIntegral();
    }

    void analyzeImpossibleValues(TokenList& tokenList, const Settings& settings)
    {
        for (Token* tok = tokenList.front(); tok; tok = tok->next()) {
            if (tok->hasKnownIntValue())
                continue;
            if (Token::Match(tok, "true|false"))
                continue;
            if (astIsBool(tok) || Token::Match(tok, "%comp%")) {
                Value lower{-1};
                lower.bound = Value::Bound::Upper;
                lower.setImpossible();
                setTokenValue(tok, std::move(lower), settings);

                Value upper{2};
                upper.bound = Value::Bound::Lower;
                upper.setImpossible();
                setTokenValue(tok, std::move(upper), settings);
            } else if (astIsUnsigned(tok) && !astIsPointer(tok)) {
                std::vector<MathLib::bigint> minvalue = minUnsignedValue(tok);
                if (minvalue.empty())
                    continue;
                Value value{std::max<MathLib::bigint>(0, minvalue.front()) - 1};
                value.bound = Value::Bound::Upper;
                value.setImpossible();
                setTokenValue(tok, std::move(value), settings);
            }
            if (Token::simpleMatch(tok, "?") && Token::Match(tok->astOperand1(), "<|<=|>|>=")) {
                const Token* condTok = tok->astOperand1();
                const Token* branchesTok = tok->astOperand2();

                auto tokens = makeArray(condTok->astOperand1(), condTok->astOperand2());
                auto branches = makeArray(branchesTok->astOperand1(), branchesTok->astOperand2());
                bool flipped = false;
                if (std::equal(tokens.cbegin(), tokens.cend(), branches.crbegin(), &isSameToken))
                    flipped = true;
                else if (!std::equal(tokens.cbegin(), tokens.cend(), branches.cbegin(), &isSameToken))
                    continue;
                const bool isMin = Token::Match(condTok, "<|<=") ^ flipped;
                std::vector<Value> values;
                for (const Token* tok2 : tokens) {
                    if (tok2->hasKnownIntValue()) {
                        values.emplace_back(tok2->values().front());
                    } else {
                        Value symValue{};
                        symValue.valueType = Value::ValueType::SYMBOLIC;
                        symValue.tokvalue = tok2;
                        values.push_back(symValue);
                        std::copy_if(tok2->values().cbegin(),
                                     tok2->values().cend(),
                                     std::back_inserter(values),
                                     [](const Value& v) {
                            if (!v.isKnown())
                                return false;
                            return v.isSymbolicValue();
                        });
                    }
                }
                for (Value& value : values) {
                    value.setImpossible();
                    if (isMin) {
                        value.intvalue++;
                        value.bound = Value::Bound::Lower;
                    } else {
                        value.intvalue--;
                        value.bound = Value::Bound::Upper;
                    }
                    setTokenValue(tok, std::move(value), settings);
                }

            } else if (Token::simpleMatch(tok, "%") && tok->astOperand2() && tok->astOperand2()->hasKnownIntValue()) {
                Value value{tok->astOperand2()->values().front()};
                value.bound = Value::Bound::Lower;
                value.setImpossible();
                setTokenValue(tok, std::move(value), settings);
            } else if (Token::Match(tok, "abs|labs|llabs|fabs|fabsf|fabsl (")) {
                Value value{-1};
                value.bound = Value::Bound::Upper;
                value.setImpossible();
                setTokenValue(tok->next(), std::move(value), settings);
            } else if (Token::Match(tok, ". data|c_str (") && astIsContainerOwned(tok->astOperand1())) {
                const Library::Container* container = getLibraryContainer(tok->astOperand1());
                if (!container)
                    continue;
                if (!container->stdStringLike)
                    continue;
                if (container->view)
                    continue;
                Value value{0};
                value.setImpossible();
                setTokenValue(tok->tokAt(2), std::move(value), settings);
            } else if (Token::Match(tok, "make_shared|make_unique <") && Token::simpleMatch(tok->linkAt(1), "> (")) {
                Value value{0};
                value.setImpossible();
                setTokenValue(tok->linkAt(1)->next(), std::move(value), settings);
            } else if ((tokenList.isCPP() && Token::simpleMatch(tok, "this")) || tok->isUnaryOp("&")) {
                Value value{0};
                value.setImpossible();
                setTokenValue(tok, std::move(value), settings);
            } else if (tok->variable() && tok->variable()->isArray() && !tok->variable()->isArgument() &&
                       !tok->variable()->isStlType()) {
                Value value{0};
                value.setImpossible();
                setTokenValue(tok, std::move(value), settings);
            } else if (tok->isIncompleteVar() && tok->astParent() && tok->astParent()->isUnaryOp("-") &&
                       isConvertedToIntegral(tok->astParent(), settings)) {
                Value value{0};
                value.setImpossible();
                setTokenValue(tok, std::move(value), settings);
            }
        }
    }
}
