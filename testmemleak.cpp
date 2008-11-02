/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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




#include "tokenize.h"
#include "CheckMemoryLeak.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;
extern bool ShowAll;

class TestMemleak : public TestFixture
{
public:
    TestMemleak() : TestFixture("TestMemleak")
    { }

private:
    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        TokenizeCode( istr );
        SimplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        ShowAll = false;
        CheckMemoryLeak();
    }

    void run()
    {
        TEST_CASE( simple1 );
        TEST_CASE( simple2 );
        TEST_CASE( simple3 );
        TEST_CASE( simple4 );
        TEST_CASE( simple5 );
        TEST_CASE( simple6 );
        TEST_CASE( simple7 );
        TEST_CASE( simple8 );

        TEST_CASE( ifelse1 );
        TEST_CASE( ifelse2 );
        TEST_CASE( ifelse3 );
        TEST_CASE( ifelse4 );
        TEST_CASE( ifelse5 );

        TEST_CASE( forwhile1 );
        TEST_CASE( forwhile2 );
        TEST_CASE( forwhile3 );
        TEST_CASE( forwhile4 );

        TEST_CASE( switch1 );
        TEST_CASE( switch2 );

        TEST_CASE( ret );

        TEST_CASE( mismatch1 );

        TEST_CASE( func1 );
        TEST_CASE( func2 );

        TEST_CASE( class1 );
        TEST_CASE( class2 );
    }

    void simple1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: a\n"), errout.str() );
    }

    void simple2()
    {
        check( "Fred *NewFred()\n"
               "{\n"
               "    Fred *f = new Fred;\n"
               "    return f;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void simple3()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    return (char *)s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple4()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    return 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: s\n"), errout.str() );
    }


    void simple5()
    {
        check( "static char *f()\n"
               "{\n"
               "    struct *str = new strlist;\n"
               "    return &str->s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple6()
    {
        check( "static void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    char *str2 = (char *)str;\n"
               "    free(str2);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple7()
    {
        // A garbage collector may delete f automaticly
        check( "class Fred;\n"
               "void foo()\n"
               "{\n"
               "    Fred *f = new Fred;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple8()
    {
        check( "char * foo ()\n"
               "{\n"
               "    char *str = strdup(\"abc\");\n"
               "    if (somecondition)\n"
               "        for (i = 0; i < 2; i++)\n"
               "        { }\n"
               "    return str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }





    void ifelse1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "    if (a)\n"
               "    {\n"
               "        delete [] a;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse2()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    if (somecondition)\n"
               "    {\n"
               "        return;\n"
               "    }\n"
               "    free(str);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:6]: Memory leak: str\n"), errout.str() );
    }


    void ifelse3()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    if (a==b)\n"
               "    {\n"
               "        free(str);\n"
               "        return;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:9]: Memory leak: str\n"), errout.str() );
    }


    void ifelse4()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = new char[10];\n"
               "    if (a==b)\n"
               "    {\n"
               "        delete [] str;\n"
               "        return;\n"
               "    }\n"
               "    delete [] str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse5()
    {
        check( "void f()\n"
               "{\n"
               "    char *str;\n"
               "    if (somecondition)\n"
               "    {\n"
               "        str = new char[100];\n"
               "    }\n"
               "    else\n"
               "    {\n"
               "        return;\n"
               "    }\n"
               "    delete [] str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse6()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    if ( a == b )\n"
               "    {\n"
               "        return s;\n"
               "    }\n"
               "    return NULL;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8]: Memory leak: s\n"), errout.str() );
    }


    void ifelse7()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s;\n"
               "    if ( abc )\n"
               "    {\n"
               "        s = new char[10];\n"
               "    }\n"
               "    return s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse8()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[10];\n"
               "    if ( s )\n"
               "    {\n"
               "        return s;\n"
               "    }\n"
               "    return 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }





    void forwhile1()
    {
        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    while (condition)\n"
              "    {\n"
              "        if (condition)\n"
              "        {\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void forwhile2()
    {
        check("void f()\n"
              "{\n"
              "    for (int i = 0; i < j; i++)\n"
              "    {\n"
              "        char *str = strdup(\"hello\");\n"
              "        if (condition)\n"
              "            continue;\n"
              "        free(str);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:7]: Memory leak: str\n"), errout.str() );
    }


    void forwhile3()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = 0;\n"
               "    for (int i = 0; i < 10; i++)\n"
               "    {\n"
               "        str = strdup(\"hello\");\n"
               "    }\n"
               "    free(str);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: str\n"), errout.str() );
    }


    void forwhile4()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = 0;\n"
               "    for (int i = 0; i < 10; i++)\n"
               "    {\n"
               "        str = strdup(\"hello\");\n"
               "        if (str) { }\n"
               "    }\n"
               "    free(str);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: str\n"), errout.str() );
    }





    void switch1()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    switch (abc)\n"
              "    {\n"
              "        case 1:\n"
              "            break;\n"
              "    };\n"
              "    delete [] str;\n"
              "}\n");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void switch2()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    switch (abc)\n"
              "    {\n"
              "        case 1:\n"
              "            delete [] str;\n"
              "            break;\n"
              "        default:\n"
              "            break;\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:12]: Memory leak: str\n"), errout.str() );
    }





    void ret()
    {
        check( "char *f( char **str )\n"
               "{\n"
               "    char *ret = malloc( 10 );\n"
               "    return *str = ret;\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }






    void mismatch1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "    free(a);\n"
               "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:4]: Mismatching allocation and deallocation: a\n"), errout.str() );
    }





    ////////////////////////////////////////////////
    // function calls
    ////////////////////////////////////////////////


    void func1()
    {
        check( "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo(p);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void func2()
    {
        check( "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo.add(p);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


/*
    void func3()
    {
        check( "static char *dmalloc()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    return p;\n"
               "}\n"
               "static void f()\n"
               "{\n"
               "    char *p = dmalloc();\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:9]: Memory leak: p\n"), errout.str() );
    }


    void func4()
    {
        check( "static char *dmalloc()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    return p;\n"
               "}\n"
               "static void f()\n"
               "{\n"
               "    char *p = dmalloc();\n"
               "    delete p;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:9]: Mismatching allocation and deallocation: p\n"), errout.str() );
    }


    void func5()
    {
        check( "static void foo(const char *str)\n"
               "{ }\n"
               "\n"
               "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo(p);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8]: Memory leak: p\n"), errout.str() );
    }


    void func6()
    {
        check( "struct ab\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "};\n"
               "\n"
               "static void f()\n"
               "{\n"
               "    struct ab *p = malloc(sizeof(struct ab));\n"
               "    foo(&p->b);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:11]: Memory leak: p\n"), errout.str() );
    }
*/





    void class1()
    {
        check( "class Fred\n"
               "{\n"
               "private:\n"
               "    char *str1;\n"
               "    char *str2;\n"
               "public:\n"
               "    Fred();\n"
               "    ~Fred();\n"
               "};\n"
               "\n"
               "Fred::Fred()\n"
               "{\n"
               "    str1 = new char[10];\n"
               "    str2 = new char[10];\n"
               "}\n"
               "\n"
               "Fred::~Fred()\n"
               "{\n"
               "    delete [] str2;\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:1]: Memory leak: Fred::str1\n"), errout.str() );
    }


    void class2()
    {
        check( "class Fred\n"
               "{\n"
               "private:\n"
               "    char *str1;\n"
               "public:\n"
               "    Fred();\n"
               "    ~Fred();\n"
               "};\n"
               "\n"
               "Fred::Fred()\n"
               "{\n"
               "    str1 = new char[10];\n"
               "}\n"
               "\n"
               "Fred::~Fred()\n"
               "{\n"
               "    free(str1);\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:17]: Mismatching allocation and deallocation: Fred::str1\n"), errout.str() );
    }





};

REGISTER_TEST( TestMemleak )


