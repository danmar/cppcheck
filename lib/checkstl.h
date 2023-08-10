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


//---------------------------------------------------------------------------
#ifndef checkstlH
#define checkstlH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief %Check STL usage (invalidation of iterators, mismatching containers, etc) */
class CPPCHECKLIB CheckStl : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckStl() : Check("STL usage") {}

private:
    /** run checks, the token list is not simplified */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const override;

    std::string classInfo() const override {
        return "Check for invalid usage of STL:\n"
               "- out of bounds errors\n"
               "- misuse of iterators when iterating through a container\n"
               "- mismatching containers in calls\n"
               "- same iterators in calls\n"
               "- dereferencing an erased iterator\n"
               "- for vectors: using iterator/pointer after push_back has been used\n"
               "- optimisation: use empty() instead of size() to guarantee fast code\n"
               "- suspicious condition when using find\n"
               "- unnecessary searching in associative containers\n"
               "- redundant condition\n"
               "- common mistakes when using string::c_str()\n"
               "- useless calls of string and STL functions\n"
               "- dereferencing an invalid iterator\n"
               "- reading from empty STL container\n"
               "- iterating over an empty STL container\n"
               "- consider using an STL algorithm instead of raw loop\n"
               "- incorrect locking with mutex\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstlH
