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
#ifndef checkmemoryleakH
#define checkmemoryleakH
//---------------------------------------------------------------------------

/**
 * @file
 *
 * %Check for memory leaks
 *
 * The checking is split up into three specialized classes.
 * - CheckMemoryLeakInFunction can detect when a function variable is allocated but not deallocated properly.
 * - CheckMemoryLeakInClass can detect when a class variable is allocated but not deallocated properly.
 * - CheckMemoryLeakStructMember checks allocation/deallocation of structs and struct members
 */

#include "check.h"
#include "config.h"

#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;

/// @addtogroup Checks
/// @{


/**
 * @brief %CheckMemoryLeakInFunction detects when a function variable is allocated but not deallocated properly.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * -# locate a function variable
 * -# create a simple token list that describes the usage of the function variable.
 * -# simplify the token list.
 * -# finally, check if the simplified token list contain any leaks.
 */

class CPPCHECKLIB CheckMemoryLeakInFunction : public Check {
    friend class TestMemleakInFunction;

public:
    /** @brief This constructor is used when registering this class */
    CheckMemoryLeakInFunction() : Check("Memory leaks (function variables)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** Report all possible errors (for the --errorlist) */
    void getErrorMessages(ErrorLogger *e, const Settings *settings) const override;

    /**
     * Get class information (--doc)
     * @return Wiki formatted information about this class
     */
    std::string classInfo() const override {
        return "Is there any allocated memory when a function goes out of scope\n";
    }
};


/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClass : public Check {
    friend class TestMemleakInClass;

public:
    CheckMemoryLeakInClass() : Check("Memory leaks (class variables)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const override;

    std::string classInfo() const override {
        return "If the constructor allocate memory then the destructor must deallocate it.\n";
    }
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMember : public Check {
    friend class TestMemleakStructMember;

public:
    CheckMemoryLeakStructMember() : Check("Memory leaks (struct members)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger * /*errorLogger*/, const Settings * /*settings*/) const override {}

    std::string classInfo() const override {
        return "Don't forget to deallocate struct members\n";
    }
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVar : public Check {
    friend class TestMemleakNoVar;

public:
    CheckMemoryLeakNoVar() : Check("Memory leaks (address not taken)") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Not taking the address to allocated memory\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkmemoryleakH
