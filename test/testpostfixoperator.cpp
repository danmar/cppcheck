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


#include "tokenize.h"
#include "checkpostfixoperator.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestPostfixOperator : public TestFixture
{
public:
    TestPostfixOperator() : TestFixture("TestPostfixOperator")
    { }

private:



    void check(const char code[])
    {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("performance");
        //settings.inconclusive = true;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Assign variable ids
        tokenizer.setVarId();

        // Fill function list
        tokenizer.fillFunctionList();

        // Check for postfix operators..
        CheckPostfixOperator checkPostfixOperator(&tokenizer, &settings, this);
        checkPostfixOperator.postfixOperator();
    }

    void run()
    {
        TEST_CASE(testsimple);
        TEST_CASE(testfor);
        TEST_CASE(teststream);
        TEST_CASE(testvolatile);
        TEST_CASE(testiterator);
        TEST_CASE(test2168);
        TEST_CASE(pointer);   // #2321 - postincrement of pointer is OK
        TEST_CASE(testHangWithInvalidCode); // #2847 - cppcheck hangs with 100% cpu load
    }

    void testHangWithInvalidCode()
    {
        check("a,b--\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testsimple()
    {
        check("\n"
              "#include <iostream>\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    unsigned int k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        k++;\n"
              "    }\n"
              "    std::cout << k << std::endl;\n"
              "    k--;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "struct K {};"
              "void foo()\n"
              "{\n"
              "    K k(0);\n"
              "    k++;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "union K {};"
              "void foo()\n"
              "{\n"
              "    K k(0);\n"
              "    k++;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(1);\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        k++;\n"
              "    }\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(1);\n"
              "    std::cout << k << std::endl;\n"
              "    if(k) {\n"
              "        ++k;\n"
              "    }\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());


        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k--;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    ++k;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    --k;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

    }

    void testfor()
    {
        check("\n"
              "#include <iostream>\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( unsigned int i=0; i <= 10; i++) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( K i(0); i <= 10; i++) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( K i(0); i <= 10; ++i) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( K i(10); i > 1; i--) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    for ( K i=10; i > 1; --i) {\n"
              "         std::cout << i << std::endl;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());


    }

    void teststream()
    {
        check("\n"
              "#include <iostream>\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    int k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    std::cout << k++ << std::endl;\n"
              "    std::cout << k-- << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    std::cout << k-- << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    std::cout << ++k << std::endl;\n"
              "    std::cout << --k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testvolatile()
    {
        check("\n"
              "#include <iostream>\n"
              "class K {};\n"
              "int main(int argc, char *argv[])\n"
              "{\n"
              "    volatile K k(0);\n"
              "    std::cout << k << std::endl;\n"
              "    k++;\n"
              "    std::cout << k << std::endl;\n"
              "    return 0;\n"
              "}\n");
        TODO_ASSERT_EQUALS("",
                           "[test.cpp:8]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());
    }

    void testiterator()
    {
        check("\n"
              "#include <vector>\n"
              "class Base {};\n"
              "int main() {\n"
              "    std::vector<Base*> v;\n"
              "    v.push_back(new Base());\n"
              "    v.push_back(new Base());\n"
              "    for (std::vector<Base*>::iterator i=v.begin(); i!=v.end(); i++) {\n"
              "        ;;\n"
              "    }\n"
              "    v.clear();\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "#include <vector>\n"
              "int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    it = v.begin();\n"
              "    while( it != v.end() ) {\n"
              "       total += *it;\n"
              "       it++;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "#include <vector>\n"
              "int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    it = v.begin();\n"
              "    while( it != v.end() ) {\n"
              "       it++;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

        check("\n"
              "#include <iostream>\n"
              "#include <vector>\n"
              "int main() {\n"
              "    std::vector<int> v;\n"
              "    std::vector<int>::iterator it;\n"
              "    for( int i=0; i < 10; ++i ) v.push_back(i);\n"
              "        unsigned int total = 0;\n"
              "    std::vector<int>::reverse_iterator rit;\n"
              "    rit= v.rend();\n"
              "    while( rit != v.rbegin() ) {\n"
              "       rit--;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (performance) Prefer prefix ++/-- operators for non-primitive types.\n", errout.str());

    }

    void test2168()
    {
        check("--> declare allocator lock here\n"
              "int main(){}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void pointer()
    {
        check("static struct class * ab;\n"
              "int * p;\n"
              "p++;\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestPostfixOperator)


