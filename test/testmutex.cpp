/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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



#include "tokenize.h"
#include "checkmutex.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestMutex : public TestFixture {
public:
    TestMutex() : TestFixture("TestMutex")
    { }

private:
    void run() {
        TEST_CASE(MutexSimplePass)
        TEST_CASE(MutexSimpleFail)
        TEST_CASE(MutexComplexPass)
        TEST_CASE(MutexComplexFail)
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("performance");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckMutex checkMutex;
        checkMutex.runChecks(&tokenizer, &settings, this);
    }

    void MutexSimplePass() {
        check("void f() {\n"
              "    pthread_mutex_lock(m);\n"
              "    functionCall() ;\n" 
              "    pthread_mutex_unlock(m) ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
    void MutexSimpleFail() {
        check("void f() {\n"
              "    pthread_mutex_lock(m);\n"
              "    functionCall() ;\n"
              "    return ;\n" 
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) A pthread_mutex_lock call doesn't have a related unlock call in function f.\n",
           errout.str());
    }
    void MutexComplexPass() {
        check("int f() {\n"
              "    pthread_mutex_lock(m);\n"
              "    if (n) {\n"
              "      functionCall() ;" 
              "      pthread_mutex_unlock(m) ;\n"
              "      return 10; }  \n"
              "    pthread_mutex_unlock(m)\n"
              "    return 23;  \n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
    void MutexComplexFail() {
        check("void f() {\n"
              "    pthread_mutex_lock(m);\n"
              "    if (m) { \n" 
              "       functionCall1() ; \n"
              "       pthread_mutex_unlock(m);\n"
              "       return ; \n"
              "    } else { \n" 
              "       functionCall2(); \n"
              "    }\n" 
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) A pthread_mutex_lock call doesn't have a related unlock call in function f.\n",
           errout.str());
    }

};

REGISTER_TEST(TestMutex)
