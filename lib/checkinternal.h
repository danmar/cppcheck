/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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


//---------------------------------------------------------------------------
#ifndef CHECKINTERNAL_H
#define CHECKINTERNAL_H
//---------------------------------------------------------------------------

#include "check.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check Internal cppcheck API usage */
class CheckInternal : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckInternal() : Check(myName())
    { }

    /** This constructor is used when running checks. */
    CheckInternal(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** Simplified checks. The token list is simplified. */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (!settings->isEnabled("internal"))
            return;

        CheckInternal checkInternal(tokenizer, settings, errorLogger);

        checkInternal.checkTokenMatchPatterns();
        checkInternal.checkTokenSimpleMatchPatterns();
        checkInternal.checkMissingPercentCharacter();
    }

    /** @brief %Check if a simple pattern is used inside Token::Match or Token::findmatch */
    void checkTokenMatchPatterns();

    /** @brief %Check if a complex pattern is used inside Token::simpleMatch or Token::findsimplematch */
    void checkTokenSimpleMatchPatterns();

    /** @brief %Check for missing % end character in Token::Match pattern */
    void checkMissingPercentCharacter();

private:
    void simplePatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void complexPatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void missingPercentCharacterError(const Token *tok, const std::string &pattern, const std::string &funcname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) {
        CheckInternal c(0, settings, errorLogger);
        c.simplePatternError(0, "class {", "Match");
        c.complexPatternError(0, "%type% ( )", "Match");
        c.missingPercentCharacterError(0, "%num", "Match");
    }

    std::string myName() const {
        return "cppcheck internal API usage";
    }

    std::string classInfo() const {
        return "Check for wrong or unsuitable internal API usage:\n"
               "* Found simple pattern inside Token::Match() call: \"class {\"\n"
               "* Found complex pattern inside Token::simpleMatch() call: \"%type%\"\n"
               "* Missing percent end character in Token::Match pattern: \"%num\"\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif
