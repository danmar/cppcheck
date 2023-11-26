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
#ifndef checkinternalH
#define checkinternalH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <string>

class ErrorLogger;
class Settings;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief %Check Internal cppcheck API usage */
class CPPCHECKLIB CheckInternal : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckInternal() : Check("cppcheck internal API usage") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        // Don't include these checks on the WIKI where people can read what
        // checks there are. These checks are not intended for users.
        return "";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkinternalH
