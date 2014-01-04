/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include "valueflow.h"
#include "token.h"
#include "mathlib.h"

static void valueFlowBeforeCondition(Token *tokens)
{
    for (Token *tok = tokens; tok; tok = tok->next()) {
        unsigned int varid;
        MathLib::bigint num;
        if (Token::Match(tok, "==|!=|>=|<=") && tok->astOperand1() && tok->astOperand2()) {
            if (tok->astOperand1()->isName() && tok->astOperand2()->isNumber()) {
                varid = tok->astOperand1()->varId();
                num = MathLib::toLongNumber(tok->astOperand2()->str());
            } else if (tok->astOperand1()->isNumber() && tok->astOperand2()->isName()) {
                varid = tok->astOperand2()->varId();
                num = MathLib::toLongNumber(tok->astOperand1()->str());
            } else {
                continue;
            }
        } else if (Token::Match(tok->previous(), "if|while ( %var% %oror%|&&|)") ||
                   Token::Match(tok, "%oror%|&& %var% %oror%|&&|)")) {
            varid = tok->next()->varId();
            num = 0;
        } else if (tok->str() == "!" && tok->astOperand1() && tok->astOperand1()->isName()) {
            varid = tok->astOperand1()->varId();
            num = 0;
        } else {
            continue;
        }

        if (varid == 0U)
            continue;

        struct ValueFlow::Value val;
        val.link = tok;
        val.intvalue = num;

        for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous()) {
            if (tok2->varId() == varid)
                tok2->values.push_back(val);
            if (tok2->str() == "{") {
                if (!Token::simpleMatch(tok2->previous(), ") {"))
                    break;
                if (!Token::simpleMatch(tok2->previous()->link()->previous(), "if ("))
                    break;
            }
        }
    }
}

void ValueFlow::setValues(Token *tokens)
{
    for (Token *tok = tokens; tok; tok = tok->next())
        tok->values.clear();

    valueFlowBeforeCondition(tokens);
}
