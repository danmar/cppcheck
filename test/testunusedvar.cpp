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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



// Check for unused variables..

#include "testsuite.h"
#include "tokenize.h"
#include "checkother.h"

#include <sstream>
extern std::ostringstream errout;

class TestUnusedVar : public TestFixture
{
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
    { }

private:
    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkStructMemberUsage();
    }

    void run()
    {
        TEST_CASE(structmember1);
        TEST_CASE(structmember2);
        TEST_CASE(structmember3);
        TEST_CASE(structmember4);
        TEST_CASE(structmember5);
        TEST_CASE(structmember6);

        TEST_CASE(localvar1);
        TEST_CASE(localvar2);
        TEST_CASE(localvar3);
        TEST_CASE(localvar4);
        TEST_CASE(localvar5);

        // Don't give false positives for variables in structs/unions
        TEST_CASE(localvarStruct1);
        TEST_CASE(localvarStruct2);
        TEST_CASE(localvarStruct3);
        TEST_CASE(localvarStruct4); // Ticket #31: sigsegv on incomplete struct

        TEST_CASE(localvarOp);          // Usage with arithmetic operators
        TEST_CASE(localvarInvert);      // Usage with inverted variable
        TEST_CASE(localvarIf);          // Usage in if
        TEST_CASE(localvarIfElse);      // return tmp1 ? tmp2 : tmp3;
        TEST_CASE(localvarOpAssign);    // a |= b;
        TEST_CASE(localvarFor);         // for ( ; var; )
        TEST_CASE(localvarShift);       // 1 >> var
    }

    void structmember1()
    {
        check("struct abc\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n");
        ASSERT_EQUALS(std::string("[test.cpp:3]: (style) struct or union member 'abc::a' is never used\n"
                                  "[test.cpp:4]: (style) struct or union member 'abc::b' is never used\n"
                                  "[test.cpp:5]: (style) struct or union member 'abc::c' is never used\n"), errout.str());
    }

    void structmember2()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    int a = abc.a;\n"
              "    int b = abc.b;\n"
              "    int c = abc.c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void structmember3()
    {
        check("struct ABC\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    int c;\n"
              "};\n"
              "\n"
              "static struct ABC abc[] = { {1, 2, 3} };\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int a = abc[0].a;\n"
              "    int b = abc[0].b;\n"
              "    int c = abc[0].c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember4()
    {
        check("struct ABC\n"
              "{\n"
              "    const int a;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    ABC abc;\n"
              "    if (abc.a == 2);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember5()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "    void reset()\n"
              "    {\n"
              "        a = 1;\n"
              "        b = 2;\n"
              "    }\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct AB ab;\n"
              "    ab.reset();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void structmember6()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (struct AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "\n"
              "void foo(char *buf)\n"
              "{\n"
              "    struct AB *ab = (AB *)&buf[10];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }









    void functionVariableUsage(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        Settings settings;
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.functionVariableUsage();
    }

    void localvar1()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Variable 'i' is assigned a value that is never used\n", errout.str());
    }

    void localvar2()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    return i;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:2]: (style) Variable 'i' is not assigned a value\n"), errout.str());
    }

    void localvar3()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i;\n"
                              "    if ( abc )\n"
                              "        ;\n"
                              "    else i = 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:2]: (style) Variable 'i' is assigned a value that is never used\n"), errout.str());
    }

    void localvar4()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int i = 0;\n"
                              "    f(i);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvar5()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 0;\n"
                              "    b = (char)a;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }



    void localvarStruct1()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    static const struct{ int x, y, w, h; } bounds = {1,2,3,4};\n"
                              "    return bounds.x + bounds.y + bounds.w + bounds.h;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarStruct2()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    struct ABC { int a, b, c; };\n"
                              "    struct ABC abc = { 1, 2, 3 };\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarStruct3()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 10;\n"
                              "    union { struct { unsigned char x; }; unsigned char z; };\n"
                              "    do {\n"
                              "        func();\n"
                              "    } while(a--);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarStruct4()
    {
        /* This must not SIGSEGV: */
        // FIXME!!
        //functionVariableUsage("void foo()\n"
        //                      "{\n"
        //                      "    struct { \n");
    }



    void localvarOp()
    {
        const char op[] = "+-*/%&|^";
        for (const char *p = op; *p; ++p)
        {
            std::string code("int main()\n"
                             "{\n"
                             "    int tmp = 10;\n"
                             "    return 123 " + std::string(1, *p) + " tmp;\n"
                             "}\n");
            functionVariableUsage(code.c_str());
            ASSERT_EQUALS(std::string(""), errout.str());
        }
    }

    void localvarInvert()
    {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    return ~tmp;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarIf()
    {
        functionVariableUsage("int main()\n"
                              "{\n"
                              "    int tmp = 10;\n"
                              "    if ( tmp )\n"
                              "        return 1;\n"
                              "    return 0;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarIfElse()
    {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int tmp1 = 1;\n"
                              "    int tmp2 = 2;\n"
                              "    int tmp3 = 3;\n"
                              "    return tmp1 ? tmp2 : tmp3;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarOpAssign()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    int b = 2;\n"
                              "    a |= b;\n"
                              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:2]: (style) Variable 'a' is assigned a value that is never used\n"), errout.str());
    }

    void localvarFor()
    {
        functionVariableUsage("void foo()\n"
                              "{\n"
                              "    int a = 1;\n"
                              "    for (;a;);\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void localvarShift()
    {
        functionVariableUsage("int foo()\n"
                              "{\n"
                              "    int var = 1;\n"
                              "    return 1 >> var;\n"
                              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

};

REGISTER_TEST(TestUnusedVar)


