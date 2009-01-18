/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include "../src/tokenize.h"
#include "../src/checkother.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestOther : public TestFixture
{
public:
    TestOther() : TestFixture("TestOther")
    { }

private:


    void run()
    {
        TEST_CASE(delete1);
        TEST_CASE(delete2);

        TEST_CASE(sprintf1);        // Dangerous usage of sprintf
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);        // struct member

        TEST_CASE(strPlusChar1);     // "/usr" + '/'
        TEST_CASE(strPlusChar2);     // "/usr" + ch
        TEST_CASE(strPlusChar3);     // ok: path + "/sub" + '/'
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, Settings(), this);
        checkOther.WarningRedundantCode();
    }

    void delete1()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "        p = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void delete2()
    {
        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "    {\n"
              "        delete p;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: Redundant condition. It is safe to deallocate a NULL pointer\n"), errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: Redundant condition. It is safe to deallocate a NULL pointer\n"), errout.str());
    }



    void sprintfUsage(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        //tokenizer.tokens()->printOut( "tokens" );

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, Settings(), this);
        checkOther.InvalidFunctionUsage();
    }

    void sprintf1()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%s\",buf);\n"
                     "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Overlapping data buffer buf\n"), errout.str());
    }

    void sprintf2()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void sprintf3()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "    if (buf[0]);\n"
                     "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void sprintf4()
    {
        sprintfUsage("struct A\n"
                     "{\n"
                     "    char filename[128];\n"
                     "};\n"
                     "\n"
                     "void foo()\n"
                     "{\n"
                     "    const char* filename = \"hello\";\n"
                     "    struct A a;\n"
                     "    snprintf(a.filename, 128, \"%s\", filename);\n"
                     "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }





    void strPlusChar(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        CheckOther checkOther(&tokenizer, Settings(), this);
        checkOther.strPlusChar();
    }

    void strPlusChar1()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    const char *p = \"/usr\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: Unusual pointer arithmetic\n"), errout.str());
    }

    void strPlusChar2()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    char ch = '/';\n"
                    "    const char *p = \"/usr\" + ch;\n"
                    "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Unusual pointer arithmetic\n"), errout.str());
    }

    void strPlusChar3()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    std::string temp = \"/tmp\";\n"
                    "    std::string path = temp + '/' + \"sub\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

};

REGISTER_TEST(TestOther)

