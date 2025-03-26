/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "checkoperatorcomma.h"
#include "tokenize.h"
#include "errortypes.h"
#include "settings.h"
#include "token.h"

// CWE ids used
static const CWE CWE398(398U);   // Indicator of Poor Code Quality

namespace {
    CheckOpComma instance;
}

void CheckOpComma::runChecks(const Tokenizer &tokenizer, ErrorLogger * errorLogger) {
    CheckOpComma checkOpComma(&tokenizer, &tokenizer.getSettings(), errorLogger);
    checkOpComma.assertWithSuspiciousComma();
}

void CheckOpComma::getErrorMessages(ErrorLogger * errorLogger, const Settings * settings) const {
    CheckOpComma c(nullptr, settings, errorLogger);
    c.assertWithSuspiciousCommaError(nullptr);
}

void CheckOpComma::assertWithSuspiciousComma() {
    if (!mSettings->severity.isEnabled(Severity::style)) {
        return;
    }

    logChecker("CheckOpComma::assertWithSuspiciousComma");

    for (const Token* tok = mTokenizer->list.front(); tok; tok = tok->next()) {
        if (tok->str() == "," && tok->isBinaryOp()) {
            const Token * parent = tok->astParent();
            if (parent && (Token::simpleMatch(parent->previous(), "if (") ||
                           Token::simpleMatch(parent->previous(), "while ("))) {
                assertWithSuspiciousCommaError(tok);
                continue;
            }
            if (parent && parent->str() == ";") {
                parent = parent->astParent();
                if (parent && parent->str() == ";") {
                    parent = parent->astParent();
                    if (Token::simpleMatch(parent->previous(), "for (")) {
                        if (tok->index() < tok->astParent()->index()) {
                            assertWithSuspiciousCommaError(tok);
                            continue;
                        }
                    }
                }
            }
            if (parent && parent->isBinaryOp() && (parent->str() == "?" ||
                                                   parent->str() == "&&" ||
                                                   parent->str() == "||")) {
                assertWithSuspiciousCommaError(tok);
            }
        }
    }
}

void CheckOpComma::assertWithSuspiciousCommaError(const Token * tok) {
    reportError(tok, Severity::style,
                "assertWithSuspiciousComma",
                "There is an suspicious comma expression used as a condition.",
                CWE398, Certainty::normal);
}
