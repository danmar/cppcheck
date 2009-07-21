/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
        TEST_CASE(zeroDiv1);
        TEST_CASE(zeroDiv2);

        TEST_CASE(delete1);
        TEST_CASE(delete2);

        TEST_CASE(unreachable1);

        TEST_CASE(sprintf1);        // Dangerous usage of sprintf
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);        // struct member

        TEST_CASE(strPlusChar1);     // "/usr" + '/'
        TEST_CASE(strPlusChar2);     // "/usr" + ch
        TEST_CASE(strPlusChar3);     // ok: path + "/sub" + '/'

        TEST_CASE(varScope1);
        TEST_CASE(varScope2);
        TEST_CASE(varScope3);
        TEST_CASE(varScope4);

        TEST_CASE(nullpointer1);
        TEST_CASE(nullpointer2);

        TEST_CASE(invalidpointer);

        TEST_CASE(oldStylePointerCast);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Simplify token list..
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.warningRedundantCode();
        checkOther.checkZeroDivision();
    }




    void zeroDiv1()
    {
        check("void foo()\n"
              "{\n"
              "    int a = 0;\n"
              "    double b = 1.;\n"
              "    cout<<b/a;\n"
              "}");


        ASSERT_EQUALS("[test.cpp:5]: (error) Division by zero\n", errout.str());
    }

    void zeroDiv2()
    {
        check("void foo()\n"
              "{\n"
              "    int sum = 0;\n"
              "    int n = 100;\n"
              "    for(int i = 0; i < n; i ++)\n"
              "    {\n"
              "        sum += i; \n"
              "    }\n"
              "    cout<<b/sum;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    int a = 0 ? (2/0) : 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p != NULL)\n"
              "        delete p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (p)\n"
              "        delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    if (0 != this->p)\n"
              "        delete this->p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());

        check("void Foo::deleteInstance()\n"
              "{\n"
              "    if (Foo::instance != NULL)\n"
              "        delete Foo::instance;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant condition. It is safe to deallocate a NULL pointer\n", errout.str());
    }

    void unreachable1()
    {
        check("void foo()\n"
              "{\n"
              "    switch (p)\n"
              "    {\n"
              "    default:\n"
              "        return 0;\n"
              "        break;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.invalidFunctionUsage();
    }

    void sprintf1()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%s\",buf);\n"
                     "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Overlapping data buffer buf\n", errout.str());
    }

    void sprintf2()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf3()
    {
        sprintfUsage("void foo()\n"
                     "{\n"
                     "    char buf[100];\n"
                     "    sprintf(buf,\"%i\",sizeof(buf));\n"
                     "    if (buf[0]);\n"
                     "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.strPlusChar();
    }

    void strPlusChar1()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    const char *p = \"/usr\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Unusual pointer arithmetic\n", errout.str());
    }

    void strPlusChar2()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    char ch = '/';\n"
                    "    const char *p = \"/usr\" + ch;\n"
                    "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Unusual pointer arithmetic\n", errout.str());
    }

    void strPlusChar3()
    {
        // Strange looking pointer arithmetic..
        strPlusChar("void foo()\n"
                    "{\n"
                    "    std::string temp = \"/tmp\";\n"
                    "    std::string path = temp + '/' + \"sub\" + '/';\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void varScope(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkVariableScope();
    }

    void varScope1()
    {
        varScope("unsigned short foo()\n"
                 "{\n"
                 "    test_client CClient;\n"
                 "    try\n"
                 "    {\n"
                 "        if (CClient.Open())\n"
                 "        {\n"
                 "            return 0;\n"
                 "        }\n"
                 "    }\n"
                 "    catch (...)\n"
                 "    {\n"
                 "        return 2;\n"
                 "    }\n"
                 "\n"
                 "    try\n"
                 "    {\n"
                 "        CClient.Close();\n"
                 "    }\n"
                 "    catch (...)\n"
                 "    {\n"
                 "        return 2;\n"
                 "    }\n"
                 "\n"
                 "    return 1;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope2()
    {
        varScope("int foo()\n"
                 "{\n"
                 "    Error e;\n"
                 "    e.SetValue(12);\n"
                 "    throw e;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope3()
    {
        varScope("void foo()\n"
                 "{\n"
                 "    int i;\n"
                 "    int *p = 0;\n"
                 "    if (abc)\n"
                 "    {\n"
                 "        p = &i;\n"
                 "    }\n"
                 "    *p = 1;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void varScope4()
    {
        varScope("void foo()\n"
                 "{\n"
                 "    int i;\n"
                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }




    void checkNullPointer(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.nullPointer();
    }


    void nullpointer1()
    {
        checkNullPointer("int foo(const Token *tok)\n"
                         "{\n"
                         "    while (tok);\n"
                         "    tok = tok->next();\n"
                         "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible null pointer dereference\n", errout.str());
    }

    void nullpointer2()
    {
        // Null pointer dereference can only happen with pointers
        checkNullPointer("void foo()\n"
                         "{\n"
                         "    Fred fred;\n"
                         "    while (fred);\n"
                         "    fred.hello();\n"
                         "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void checkInvalidPointer(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.invalidPointer();
    }

    void invalidpointer()
    {
        // errors..
        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    int *a = abc->a;\n"
                            "    *a;\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Possible invalid pointer dereference\n", errout.str());

        // ok dereferencing in a condition
        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    if (abc && abc->a);\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("", errout.str());

        // ok to use a linked list..
        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    abc = abc->next;\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("", errout.str());

        // reassign struct..
        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    a = abc->a;\n"
                            "    *a;\n"
                            "    abc = abc->next;\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("", errout.str());

        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    a = abc->a;\n"
                            "    *a;\n"
                            "    f(&abc);\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("", errout.str());

        // goto..
        checkInvalidPointer("void foo(struct ABC *abc)\n"
                            "{\n"
                            "    if (!abc)\n"
                            "        goto out;"
                            "    a = abc->a;\n"
                            "    *a;\n"
                            "    return;\n"
                            "out:\n"
                            "    if (!abc)\n"
                            "        ;\n"
                            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkOldStylePointerCast(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check for redundant code..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.warningOldStylePointerCast();
    }

    void oldStylePointerCast()
    {
        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (Base *) derived;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class Base;\n"
                                 "void foo()\n"
                                 "{\n"
                                 "    Base * b = (const Base *) derived;\n"
                                 "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) C-style pointer casting\n", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(B *) const = 0;\n"
                                 "}\n");
        ASSERT_EQUALS("", errout.str());

        checkOldStylePointerCast("class B;\n"
                                 "class A\n"
                                 "{\n"
                                 "  virtual void abc(const B *) const = 0;\n"
                                 "}\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestOther)

