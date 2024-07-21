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

#include "vf_arrayelement.h"

#include "astutils.h"
#include "library.h"
#include "mathlib.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <list>
#include <string>
#include <utility>
#include <vector>

namespace ValueFlow
{
    void analyzeArrayElement(TokenList& tokenlist, const Settings& settings)
    {
        for (Token* tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->hasKnownIntValue())
                continue;
            const Token* indexTok = nullptr;
            const Token* arrayTok = nullptr;
            if (Token::simpleMatch(tok, "[") && tok->isBinaryOp()) {
                indexTok = tok->astOperand2();
                arrayTok = tok->astOperand1();
            } else if (Token::Match(tok->tokAt(-2), ". %name% (") && astIsContainer(tok->tokAt(-2)->astOperand1())) {
                arrayTok = tok->tokAt(-2)->astOperand1();
                const Library::Container* container = getLibraryContainer(arrayTok);
                if (!container || container->stdAssociativeLike)
                    continue;
                const Library::Container::Yield yield = container->getYield(tok->strAt(-1));
                if (yield != Library::Container::Yield::AT_INDEX)
                    continue;
                indexTok = tok->astOperand2();
            }

            if (!indexTok || !arrayTok)
                continue;

            for (const Value& arrayValue : arrayTok->values()) {
                if (!arrayValue.isTokValue())
                    continue;
                if (arrayValue.isImpossible())
                    continue;
                for (const Value& indexValue : indexTok->values()) {
                    if (!indexValue.isIntValue())
                        continue;
                    if (indexValue.isImpossible())
                        continue;
                    if (!arrayValue.isKnown() && !indexValue.isKnown() && arrayValue.varId != 0 && indexValue.varId != 0 &&
                        !(arrayValue.varId == indexValue.varId && arrayValue.varvalue == indexValue.varvalue))
                        continue;

                    Value result(0);
                    result.condition = arrayValue.condition ? arrayValue.condition : indexValue.condition;
                    result.setInconclusive(arrayValue.isInconclusive() || indexValue.isInconclusive());
                    result.varId = (arrayValue.varId != 0) ? arrayValue.varId : indexValue.varId;
                    result.varvalue = (result.varId == arrayValue.varId) ? arrayValue.intvalue : indexValue.intvalue;
                    if (arrayValue.valueKind == indexValue.valueKind)
                        result.valueKind = arrayValue.valueKind;

                    result.errorPath.insert(result.errorPath.end(), arrayValue.errorPath.cbegin(), arrayValue.errorPath.cend());
                    result.errorPath.insert(result.errorPath.end(), indexValue.errorPath.cbegin(), indexValue.errorPath.cend());

                    const MathLib::bigint index = indexValue.intvalue;

                    if (arrayValue.tokvalue->tokType() == Token::eString) {
                        const std::string s = arrayValue.tokvalue->strValue();
                        if (index == s.size()) {
                            result.intvalue = 0;
                            setTokenValue(tok, std::move(result), settings);
                        } else if (index >= 0 && index < s.size()) {
                            result.intvalue = s[index];
                            setTokenValue(tok, std::move(result), settings);
                        }
                    } else if (Token::simpleMatch(arrayValue.tokvalue, "{")) {
                        std::vector<const Token*> args = getArguments(arrayValue.tokvalue);
                        if (index < 0 || index >= args.size())
                            continue;
                        const Token* arg = args[index];
                        if (!arg->hasKnownIntValue())
                            continue;
                        const Value& v = arg->values().front();
                        result.intvalue = v.intvalue;
                        result.errorPath.insert(result.errorPath.end(), v.errorPath.cbegin(), v.errorPath.cend());
                        setTokenValue(tok, std::move(result), settings);
                    }
                }
            }
        }
    }
}
