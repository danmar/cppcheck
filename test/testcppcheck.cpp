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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "testsuite.h"
#include "path.h"

#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>

// use tinyxml with STL
#include "tinyxml/tinyxml.h"

extern std::ostringstream errout;
extern std::ostringstream output;

class TestCppcheck : public TestFixture
{
public:
    TestCppcheck() : TestFixture("TestCppcheck")
    { }

private:

    class ErrorLogger2 : public ErrorLogger
    {
    public:
        std::list<std::string> id;

        void reportOut(const std::string & /*outmsg*/)
        {
        }

        void reportErr(const ErrorLogger::ErrorMessage &msg)
        {
            id.push_back(msg._id);
        }

        void reportStatus(unsigned int /*index*/, unsigned int /*max*/)
        {
        }
    };

    void run()
    {
        TEST_CASE(getErrorMessages);
    }

    void getErrorMessages()
    {
        ErrorLogger2 errorLogger;
        CppCheck cppCheck(errorLogger);
        cppCheck.getErrorMessages();

        // TODO: check if there are duplicate error ids in errorLogger.id
    }
};

REGISTER_TEST(TestCppcheck)
