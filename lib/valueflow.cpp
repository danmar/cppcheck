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
#include "errorlogger.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"

#include <iostream>


static void printvalues(const Token *tok)
{
    if (tok->values.empty())
        std::cout << "empty";
    for (std::list<ValueFlow::Value>::const_iterator it = tok->values.begin(); it != tok->values.end(); ++it)
        std::cout << " " << (it->intvalue);
    std::cout << std::endl;
}

static void bailout(TokenList *tokenlist, ErrorLogger *errorLogger, const Token *tok, const std::string &what)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
    callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(tok,tokenlist));
    ErrorLogger::ErrorMessage errmsg(callstack, Severity::debug, "ValueFlow bailout: " + what, "valueFlowBailout", false);
    errorLogger->reportErr(errmsg);
}

static void valueFlowBeforeCondition(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        unsigned int varid;
        MathLib::bigint num;
        const Variable *var;
        if (Token::Match(tok, "==|!=|>=|<=") && tok->astOperand1() && tok->astOperand2()) {
            if (tok->astOperand1()->isName() && tok->astOperand2()->isNumber()) {
                varid = tok->astOperand1()->varId();
                var = tok->astOperand1()->variable();
                num = MathLib::toLongNumber(tok->astOperand2()->str());
            } else if (tok->astOperand1()->isNumber() && tok->astOperand2()->isName()) {
                varid = tok->astOperand2()->varId();
                var = tok->astOperand2()->variable();
                num = MathLib::toLongNumber(tok->astOperand1()->str());
            } else {
                continue;
            }
        } else if (Token::Match(tok->previous(), "if|while ( %var% %oror%|&&|)") ||
                   Token::Match(tok, "%oror%|&& %var% %oror%|&&|)")) {
            varid = tok->next()->varId();
            var = tok->next()->variable();
            num = 0;
        } else if (tok->str() == "!" && tok->astOperand1() && tok->astOperand1()->isName()) {
            varid = tok->astOperand1()->varId();
            var = tok->astOperand1()->variable();
            num = 0;
        } else {
            continue;
        }

        if (varid == 0U)
            continue;

        // bailout: global variables
        if (var && var->isGlobal()) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "global variable " + var->nameToken()->str());
            continue;
        }

        struct ValueFlow::Value val;
        val.condition = tok;
        val.intvalue = num;

        for (Token *tok2 = tok->previous(); ; tok2 = tok2->previous()) {
            if (!tok2) {
                if (settings->debugwarnings) {
                    std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
                    callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(tok,tokenlist));
                    ErrorLogger::ErrorMessage errmsg(callstack, Severity::debug, "iterated too far", "debugValueFlowBeforeCondition", false);
                    errorLogger->reportErr(errmsg);
                }
                break;
            }

            if (tok2->varId() == varid) {
                // bailout: assignment
                if (Token::Match(tok2, "%var% =")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                    break;
                }

                tok2->values.push_back(val);
                if (var && tok2 == var->nameToken())
                    break;
            }

            if (tok2->str() == "}") {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " stopping on }");
                break;
            }
        }
    }
}

static void valueFlowSubFunction(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    std::list<ValueFlow::Value> argvalues;
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[(,] %var% [,)]") && !tok->next()->values.empty())
            argvalues = tok->next()->values;
        else if (Token::Match(tok, "[(,] %num% [,)]")) {
            ValueFlow::Value val;
            val.condition = 0;
            val.intvalue = MathLib::toLongNumber(tok->next()->str());
            argvalues.clear();
            argvalues.push_back(val);
        } else {
            continue;
        }

        const Token * const argumentToken = tok->next();

        // is this a function call?
        const Token *ftok = tok;
        while (ftok && ftok->str() != "(")
            ftok = ftok->astParent();
        if (!ftok || !ftok->astOperand1() || !ftok->astOperand2() || !ftok->astOperand1()->function())
            continue;

        // Get argument nr
        unsigned int argnr = 0;
        for (const Token *argtok = ftok->next(); argtok && argtok != argumentToken; argtok = argtok->nextArgument())
            ++ argnr;

        // Get function argument, and check if parameter is passed by value
        const Function * const function = ftok->astOperand1()->function();
        const Variable * const arg = function ? function->getArgumentVar(argnr) : NULL;
        if (!Token::Match(arg ? arg->typeStartToken() : NULL, "const| %type% %var% ,|)"))
            continue;

        // Function scope..
        const Scope * const functionScope = function ? function->functionScope : NULL;
        if (!functionScope)
            continue;

        // Set value in function scope..
        const unsigned int varid2 = arg->nameToken()->varId();
        for (const Token *tok2 = functionScope->classStart->next(); tok2 != functionScope->classEnd; tok2 = tok2->next()) {
            if (Token::Match(tok2, "%cop%|return %varid%", varid2)) {
                tok2 = tok2->next();
                std::list<ValueFlow::Value> &values = const_cast<Token*>(tok2)->values;
                values.insert(values.begin(), argvalues.begin(), argvalues.end());
            } else if (tok2->varId() == varid2 || tok2->str() == "{") {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "parameter " + arg->nameToken()->str());
                continue;
            }
        }
    }
}

void ValueFlow::setValues(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next())
        tok->values.clear();

    valueFlowBeforeCondition(tokenlist, errorLogger, settings);
    valueFlowSubFunction(tokenlist, errorLogger, settings);
}
