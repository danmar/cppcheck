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
#ifndef checkclassH
#define checkclassH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "symboldatabase.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class ErrorLogger;
class Settings;
class Tokenizer;

namespace CTU {
    class FileInfo;
}

namespace tinyxml2 {
    class XMLElement;
}

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CPPCHECKLIB CheckClass : public Check {
    friend class TestClass;
    friend class TestConstructors;
    friend class TestUnusedPrivateFunction;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check("Class") {}

    /** @brief Set of the STL types whose operator[] is not const */
    static const std::set<std::string> stl_containers_not_const;

private:
    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override ;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const override;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Check the code for each class.\n"
               "- Missing constructors and copy constructors\n"
               //"- Missing allocation of memory in copy constructor\n"
               "- Constructors which should be explicit\n"
               "- Are all variables initialized by the constructors?\n"
               "- Are all variables assigned by 'operator='?\n"
               "- Warn if memset, memcpy etc are used on a class\n"
               "- Warn if memory for classes is allocated with malloc()\n"
               "- If it's a base class, check that the destructor is virtual\n"
               "- Are there unused private functions?\n"
               "- 'operator=' should check for assignment to self\n"
               "- Constness for member functions\n"
               "- Order of initializations\n"
               "- Suggest usage of initialization list\n"
               "- Initialization of a member with itself\n"
               "- Suspicious subtraction from 'this'\n"
               "- Call of pure virtual function in constructor/destructor\n"
               "- Duplicated inherited data members\n"
               // disabled for now "- If 'copy constructor' defined, 'operator=' also should be defined and vice versa\n"
               "- Check that arbitrary usage of public interface does not result in division by zero\n"
               "- Delete \"self pointer\" and then access 'this'\n"
               "- Check that the 'override' keyword is used when overriding virtual functions\n"
               "- Check that the 'one definition rule' is not violated\n";
    }

    // for testing
    static void checkCopyCtorAndEqOperator(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkExplicitConstructors(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkDuplInheritedMembers(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void copyconstructors(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void operatorEqRetRefThis(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void operatorEqToSelf(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void virtualDestructor(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkMemset(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void thisSubtraction(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkConst(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void initializerListOrder(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void initializationListUsage(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkSelfInitialization(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkVirtualFunctionCallInConstructor(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkOverride(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkUselessOverride(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkUnsafeClassRefMember(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkThisUseAfterFree(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void constructors(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void privateFunctions(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkclassH
