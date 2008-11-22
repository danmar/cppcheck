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

class TestMemleak : public TestFixture
{
public:
    TestMemleak() : TestFixture("TestMemleak")
    { }

private:
    void check( const char code[] )
    {
        // Tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(code);
        tokenizer.TokenizeCode( istr );
        tokenizer.SimplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        Settings settings;
        settings._checkCodingStyle = true;
        settings._showAll = false;
        tokenizer.settings( settings );
        tokenizer.FillFunctionList(0);
        CheckMemoryLeakClass checkMemoryLeak( &tokenizer, settings, this );
        checkMemoryLeak.CheckMemoryLeak();
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

        TEST_CASE( use1 );
        TEST_CASE( use2 );

        TEST_CASE( ifelse1 );
        TEST_CASE( ifelse2 );
        TEST_CASE( ifelse3 );
        TEST_CASE( ifelse4 );
        TEST_CASE( ifelse5 );
        TEST_CASE( ifelse6 );
        TEST_CASE( ifelse7 );
        TEST_CASE( ifelse8 );
        TEST_CASE( ifelse9 );

        TEST_CASE( if1 );
        TEST_CASE( if2 );
        TEST_CASE( if3 );
        TEST_CASE( if4 );
        TEST_CASE( if5 );

        TEST_CASE( forwhile1 );
        TEST_CASE( forwhile2 );
        TEST_CASE( forwhile3 );
        TEST_CASE( forwhile4 );
        TEST_CASE( forwhile5 );
        TEST_CASE( forwhile6 );
        TEST_CASE( forwhile7 );

        TEST_CASE( dowhile1 );

        TEST_CASE( switch1 );
        TEST_CASE( switch2 );

        TEST_CASE( ret1 );
        TEST_CASE( ret2 );

        TEST_CASE( mismatch1 );

        TEST_CASE( func1 );
        TEST_CASE( func2 );
        TEST_CASE( func3 );
        TEST_CASE( func4 );
        TEST_CASE( func5 );
        TEST_CASE( func6 );

        TEST_CASE( class1 );
        TEST_CASE( class2 );

        TEST_CASE( throw1 );

        TEST_CASE( linux_list_1 );

        TEST_CASE( sizeof1 );
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





    void use1()
    {
        check( "void foo()\n"
               "{\n"
               "    char *str;\n"
               "    if (somecondition)\n"
               "        str = strdup(\"abc\");\n"
               "    if (somecondition)\n"
               "        DeleteString(str);\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void use2()
    {
        check( "void foo()\n"
               "{\n"
               "    char *str = strdup(\"abc\");\n"
	           "    if ( abc ) { memset(str, 0, 3); }\n"
               "    *somestr = str;\n"
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


    void ifelse9()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[10];\n"
               "    if ( ghfgf )\n"
               "    {\n"
               "        delete [] s;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }




    void if1()
    {
        check( "void f()\n"
               "{\n"
               "    struct abc *p = new abc;\n"
               "    p->a = new char[100];\n"
               "    if ( ! p->a )\n"
               "        return;\n"
               "    foo(p);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:6]: Memory leak: p\n"), errout.str() );
    }

    void if2()
    {
        check( "void f()\n"
               "{\n"
               "    struct smp_alt_module *smp;\n"
               "    smp = kzalloc(sizeof(*smp), GFP_KERNEL);\n"
               "    if (NULL == smp)\n"
               "        return;\n"
               "    kfree( smp );\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void if3()
    {
        check( "void f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    if (0 != s)\n"
               "        foo(s);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void if4()
    {
        check( "void f()\n"
               "{\n"
               "    char *s;\n"
               "    bool b = true;\n"
               "    if (b && (s = malloc(256)))\n"
               "        ;\n"
               "    if (b)\n"
               "        free(s);\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), err );
    }

    void if5()
    {
        check( "void f()\n"
               "{\n"
               "    char *p = malloc(256);\n"
               "    if (somecondition && !p)\n"
               "        return;\n"
               "    free(p);\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), err );
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


    void forwhile5()
    {
        check( "void f(const char **a)\n"
               "{\n"
               "    char *str = 0;\n"
               "    for (int i = 0; i < 10 && !str; ++i)\n"
               "    {\n"
               "        str = strdup(a[i]);\n"
               "    }\n"
               "    return str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void forwhile6()
    {
        check( "void f(const char **a)\n"
               "{\n"
               "    char *str = 0;\n"
               "    for (int i = 0; i < 10 && !str; ++i)\n"
               "    {\n"
               "        str = strdup(a[i]);\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8]: Memory leak: str\n"), errout.str() );
    }


    void forwhile7()
    {
        check("void f()\n"
              "{\n"
              "    for (int i = 0; i < j; i++)\n"
              "    {\n"
              "        char *str = strdup(\"hello\");\n"
              "        if (condition)\n"
              "            break;\n"
              "        free(str);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:7]: Memory leak: str\n"), errout.str() );
    }






    void dowhile1()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = strdup(\"abc\");\n"
               "    do\n"
               "    {\n"
               "        str = strdup(\"def\");\n"
               "    }\n"
               "    while (!str);\n"
               "    return str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:5]: Memory leak: str\n"), errout.str() );
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





    void ret1()
    {
        check( "char *f( char **str )\n"
               "{\n"
               "    char *ret = malloc( 10 );\n"
               "    return *str = ret;\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ret2()
    {
        check( "void foo()\n"
               "{\n"
               "    struct ABC *abc = new ABC;\n"
               "    abc->a = new char[10];\n"
               "    if ( ! abc->a )\n"
               "        return;\n"
               "    delete [] abc->a;\n"
               "    delete abc;\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:6]: Memory leak: abc\n"), errout.str() );
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


    void func3()
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


    void func4()
    {
        check( "static void foo(char *str)\n"
               "{\n"
               "    delete [] str;\n"
               "}\n"
               "\n"
               "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo(p);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void func5()
    {
        check( "static void foo(char *str)\n"
               "{\n"
               "    delete str;\n"
               "}\n"
               "\n"
               "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo(p);\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string("[test.cpp:9] -> [test.cpp:3]: Mismatching allocation and deallocation: str\n"), err );
    }


    void func6()
    {
        check( "static void foo(char *str)\n"
               "{\n"
               "    goto abc;\n"
               "}\n"
               "\n"
               "static void f()\n"
               "{\n"
               "    char *p = new char[100];\n"
               "    foo(p);\n"
               "}\n" );
        std::string err( errout.str() );
        ASSERT_EQUALS( std::string(""), err );
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




    void throw1()
    {
        check( "void foo()\n"
               "{\n"
               "    char *str = new char[10];\n"
               "    if ( ! abc )\n"
               "        throw 123;\n"
               "    delete [] str;\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:5]: Memory leak: str\n"), errout.str() );
    }




    void linux_list_1()
    {
        check( "struct AB\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "};\n"
               "void foo()\n"
               "{\n"
               "    struct AB *ab = new AB;\n"
               "    func(&ab->a);\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }



    void sizeof1()
    {
        check( "void f()\n"
               "{\n"
               "    struct s_t s1;\n"
               "    struct s_t cont *p = &s1;\n"
               "    struct s_t *s2;\n"
               "\n"
               "    memset(p, 0, sizeof(*p));\n"
               "\n"
               "    s2 = (struct s_t *) malloc(sizeof(*s2));\n"
               "\n"
               "    if (s2->value != 0)\n"
               "        return;\n"
               "\n"
               "    free(s2);\n"
               "\n"
               "    return;\n"
               "}\n" );

        std::string err( errout.str() );

        ASSERT_EQUALS( std::string("[test.cpp:12]: Memory leak: s2\n"), err );
    }

};

REGISTER_TEST( TestMemleak )


