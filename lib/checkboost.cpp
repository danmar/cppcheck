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

#include "checkboost.h"

#include "errortypes.h"
#include "symboldatabase.h"
#include "token.h"

#include <vector>

// Register this check class (by creating a static instance of it)
namespace {
    CheckBoost instance;
}

static const CWE CWE664(664);

void CheckBoost::checkBoostForeachModification()
{
    logChecker("CheckBoost::checkBoostForeachModification");
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok && tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "BOOST_FOREACH ("))
                continue;

            const Token *containerTok = tok->next()->link()->previous();
            if (!Token::Match(containerTok, "%var% ) {"))
                continue;

            const Token *tok2 = containerTok->tokAt(2);
            const Token *end = tok2->link();
            for (; tok2 != end; tok2 = tok2->next()) {
                if (Token::Match(tok2, "%varid% . insert|erase|push_back|push_front|pop_front|pop_back|clear|swap|resize|assign|merge|remove|remove_if|reverse|sort|splice|unique|pop|push", containerTok->varId())) {
                    const Token* nextStatement = Token::findsimplematch(tok2->linkAt(3), ";", end);
                    if (!Token::Match(nextStatement, "; break|return|throw"))
                        boostForeachError(tok2);
                    break;
                }
            }
        }
    }
}

void CheckBoost::boostForeachError(const Token *tok)
{
    reportError(tok, Severity::error, "boostForeachError",
                "BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container inside.", CWE664, Certainty::normal
                );
}
