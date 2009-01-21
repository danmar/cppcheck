/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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




#define UNIT_TESTING
#include "../src/tokenize.h"
#include "../src/checkmemoryleak.h"
#include "testsuite.h"

#include <iostream>
#include <sstream>

extern std::ostringstream errout;

class TestMemleak : public TestFixture
{
public:
    TestMemleak() : TestFixture("TestMemleak")
    { }

private:
    void check(const char code[], bool showAll = false)
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        Settings settings;
        settings._debug = true;
        settings._showAll = showAll;
        tokenizer.fillFunctionList();
        CheckMemoryLeakClass checkMemoryLeak(&tokenizer, settings, this);
        checkMemoryLeak.CheckMemoryLeak();
    }

    void run()
    {
        TEST_CASE(simple1);
        TEST_CASE(simple2);
        TEST_CASE(simple3);
        TEST_CASE(simple4);
        TEST_CASE(simple5);
        TEST_CASE(simple6);
        TEST_CASE(simple7);
        TEST_CASE(simple8);
        TEST_CASE(simple9);     // Bug 2435468 - member function "free"
        TEST_CASE(simple10);    // fclose in a if condition

        TEST_CASE(use1);
        TEST_CASE(use2);

        TEST_CASE(ifelse1);
        TEST_CASE(ifelse2);
        TEST_CASE(ifelse3);
        TEST_CASE(ifelse4);
        TEST_CASE(ifelse5);
        TEST_CASE(ifelse6);
        TEST_CASE(ifelse7);
        TEST_CASE(ifelse8);
        TEST_CASE(ifelse9);

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);
        TEST_CASE(if6);     // Bug 2432631
        TEST_CASE(if7);     // Bug 2401436
        TEST_CASE(if8);     // Bug 2458532
        // TODO TEST_CASE( if9 );   // if (realloc)
        TEST_CASE(if10);            // else if (realloc)

        TEST_CASE(forwhile1);
        TEST_CASE(forwhile2);
        TEST_CASE(forwhile3);
        TEST_CASE(forwhile4);
        TEST_CASE(forwhile5);
        TEST_CASE(forwhile6);
        TEST_CASE(forwhile7);
        TEST_CASE(forwhile8);       // Bug 2429936
        TEST_CASE(forwhile9);
        TEST_CASE(forwhile10);

        TEST_CASE(dowhile1);

        TEST_CASE(switch1);
        TEST_CASE(switch2);
        TEST_CASE(switch3);

        TEST_CASE(ret1);
        TEST_CASE(ret2);
        TEST_CASE(ret3);
        TEST_CASE(ret4);
        TEST_CASE(ret5);        // Bug 2458436 - return use

        TEST_CASE(mismatch1);

        TEST_CASE(func1);
        TEST_CASE(func2);
        TEST_CASE(func3);
        TEST_CASE(func4);
        TEST_CASE(func5);
        TEST_CASE(func6);
        // TODO TEST_CASE( func7 );
        TEST_CASE(func8);       // Using callback
        TEST_CASE(func9);       // Embedding the function call in a if-condition
        TEST_CASE(func10);      // Bug 2458510 - Function pointer
        TEST_CASE(func11);      // Bug 2458510 - Function pointer
        TEST_CASE(func12);

        TEST_CASE(class1);
        TEST_CASE(class2);
        // TODO TEST_CASE( class3 );
        TEST_CASE(class4);
        TEST_CASE(class5);
        TEST_CASE(class6);

        TEST_CASE(throw1);
        TEST_CASE(throw2);

        TEST_CASE(linux_list_1);

        TEST_CASE(sizeof1);

        TEST_CASE(realloc1);
        TEST_CASE(realloc2);

        TEST_CASE(assign);

        // TODO TEST_CASE( varid );

        TEST_CASE(cast1);
        TEST_CASE(cast2);
        TEST_CASE(cast3);


        // TODO TEST_CASE( structmember1 );

        TEST_CASE(dealloc_use_1);       // Deallocate and then use memory
        TEST_CASE(dealloc_use_2);       // Deallocate and then use memory. No error if "use" is &var
        TEST_CASE(dealloc_use_3);       // Deallocate and then use memory. No error
        TEST_CASE(dealloc_use_4);
        TEST_CASE(dealloc_use_5);
        TEST_CASE(dealloc_use_6);

        // free a free'd pointer
        TEST_CASE(freefree1);
        TEST_CASE(freefree2);
    }



    void simple1()
    {
        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Memory leak: a\n"), errout.str());
    }

    void simple2()
    {
        check("Fred *NewFred()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "    return f;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void simple3()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    return (char *)s;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void simple4()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Memory leak: s\n"), errout.str());
    }


    void simple5()
    {
        check("static char *f()\n"
              "{\n"
              "    struct *str = new strlist;\n"
              "    return &str->s;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void simple6()
    {
        check("static void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    char *str2 = (char *)str;\n"
              "    free(str2);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void simple7()
    {
        // A garbage collector may delete f automaticly
        check("class Fred;\n"
              "void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void simple8()
    {
        check("char * foo ()\n"
              "{\n"
              "    char *str = strdup(\"abc\");\n"
              "    if (somecondition)\n"
              "        for (i = 0; i < 2; i++)\n"
              "        { }\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void simple9()
    {
        check("void foo()\n"
              "{\n"
              "    MyClass *c = new MyClass();\n"
              "    c->free(c);\n"
              "    delete c;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void simple10()
    {
        check("void foo()\n"
              "{\n"
              "    FILE * f = fopen(fname, str);\n"
              "    if ( fclose(f) != NULL );\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }







    void use1()
    {
        check("void foo()\n"
              "{\n"
              "    char *str;\n"
              "    if (somecondition)\n"
              "        str = strdup(\"abc\");\n"
              "    if (somecondition)\n"
              "        DeleteString(str);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void use2()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = strdup(\"abc\");\n"
              "    if ( abc ) { memset(str, 0, 3); }\n"
              "    *somestr = str;\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }






    void ifelse1()
    {
        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "    if (a)\n"
              "    {\n"
              "        delete [] a;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ifelse2()
    {
        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    if (somecondition)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:6]: Memory leak: str\n"), errout.str());
    }


    void ifelse3()
    {
        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    if (a==b)\n"
              "    {\n"
              "        free(str);\n"
              "        return;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());

        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    if (a==b)\n"
              "    {\n"
              "        free(str);\n"
              "        return;\n"
              "    }\n"
              "}\n", true);
        ASSERT_EQUALS(std::string("[test.cpp:9]: Memory leak: str\n"), errout.str());
    }


    void ifelse4()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    if (a==b)\n"
              "    {\n"
              "        delete [] str;\n"
              "        return;\n"
              "    }\n"
              "    delete [] str;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ifelse5()
    {
        check("void f()\n"
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
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ifelse6()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    if ( a == b )\n"
              "    {\n"
              "        return s;\n"
              "    }\n"
              "    return NULL;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: Memory leak: s\n"), errout.str());
    }


    void ifelse7()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s;\n"
              "    if ( abc )\n"
              "    {\n"
              "        s = new char[10];\n"
              "    }\n"
              "    return s;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ifelse8()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    if ( s )\n"
              "    {\n"
              "        return s;\n"
              "    }\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ifelse9()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    if ( ghfgf )\n"
              "    {\n"
              "        delete [] s;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }




    void if1()
    {
        check("void f()\n"
              "{\n"
              "    struct abc *p = new abc;\n"
              "    p->a = new char[100];\n"
              "    if ( ! p->a )\n"
              "        return;\n"
              "    foo(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:6]: Memory leak: p\n"), errout.str());
    }

    void if2()
    {
        check("void f()\n"
              "{\n"
              "    struct smp_alt_module *smp;\n"
              "    smp = kzalloc(sizeof(*smp), GFP_KERNEL);\n"
              "    if (NULL == smp)\n"
              "        return;\n"
              "    kfree( smp );\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if3()
    {
        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    if (0 != s)\n"
              "        foo(s);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if4()
    {
        check("void f()\n"
              "{\n"
              "    char *s;\n"
              "    bool b = true;\n"
              "    if (b && (s = malloc(256)))\n"
              "        ;\n"
              "    if (b)\n"
              "        free(s);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if5()
    {
        check("void f()\n"
              "{\n"
              "    char *p = malloc(256);\n"
              "    if (somecondition && !p)\n"
              "        return;\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if6()
    {
        check("void f()\n"
              "{\n"
              "    FILE *a = 0;\n"
              "    a = fopen(\"test.txt\", \"rw\");\n"
              "    if( a == 0 )\n"
              "    {\n"
              "        a = fopen(\"test.txt\", \"r\");\n"
              "    }\n"
              "\n"
              "    fclose( a );\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if7()
    {
        check("void f( bool b )\n"
              "{\n"
              "    int *a=0;\n"
              "    if( b )\n"
              "    {\n"
              "        a = new int[10];\n"
              "    }\n"
              "\n"
              "    if( b )\n"
              "        delete [] a;\n"
              "    else {}\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if8()
    {
        check("static void f(int i)\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    if (i == 1)\n"
              "    {\n"
              "        free(c);\n"
              "        return;\n"
              "    }\n"
              "    if (i == 2)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    free(c);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:11]: Memory leak: c\n"), errout.str());
    }

    void if9()
    {
        check("static void f()\n"
              "{\n"
              "	char *buf = NULL, *tmp;\n"
              "	if (!(tmp = realloc(buf, 50)))\n"
              "	{\n"
              "		free(buf);\n"
              "		return NULL;\n"
              "	}\n"
              "	buf = tmp;\n"
              "	return buf;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void if10()
    {
        check("static void f()\n"
              "{\n"
              "	char *buf = malloc(10);\n"
              "	if (aa)\n"
              "	    ;\n"
              "    else if (buf = realloc(buf, 100))\n"
              "		;\n"
              "    free(buf);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
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
        ASSERT_EQUALS(std::string(""), errout.str());
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
        ASSERT_EQUALS(std::string("[test.cpp:7]: Memory leak: str\n"), errout.str());
    }


    void forwhile3()
    {
        check("void f()\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10; i++)\n"
              "    {\n"
              "        str = strdup(\"hello\");\n"
              "    }\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Memory leak: str\n"), errout.str());
    }


    void forwhile4()
    {
        check("void f()\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10; i++)\n"
              "    {\n"
              "        str = strdup(\"hello\");\n"
              "        if (str) { }\n"
              "    }\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Memory leak: str\n"), errout.str());
    }


    void forwhile5()
    {
        check("void f(const char **a)\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10 && !str; ++i)\n"
              "    {\n"
              "        str = strdup(a[i]);\n"
              "    }\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void forwhile6()
    {
        check("void f(const char **a)\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10 && !str; ++i)\n"
              "    {\n"
              "        str = strdup(a[i]);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: Memory leak: str\n"), errout.str());
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
        ASSERT_EQUALS(std::string("[test.cpp:7]: Memory leak: str\n"), errout.str());
    }


    void forwhile8()
    {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for( ;; )\n"
              "    {\n"
              "    ++i;\n"
              "    a = realloc( a, i );\n"
              "    if( !a )\n"
              "        return 0;\n"
              "\n"
              "    if( i > 10 )\n"
              "        break;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void forwhile9()
    {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for( ;; )\n"
              "    {\n"
              "    if(i>=0)\n"
              "        continue;\n"
              "    a = realloc( a, i );\n"
              "    if(i>=0)\n"
              "        continue;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", true);
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void forwhile10()
    {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for( ;; )\n"
              "    {\n"
              "    if(i>=0)\n"
              "        continue;\n"
              "    a = realloc( a, i );\n"
              "    if(i>=0)\n"
              "        return;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", true);
        ASSERT_EQUALS(std::string("[test.cpp:11]: Memory leak: a\n"), errout.str());
    }





    void dowhile1()
    {
        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"abc\");\n"
              "    do\n"
              "    {\n"
              "        str = strdup(\"def\");\n"
              "    }\n"
              "    while (!str);\n"
              "    return str;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: Memory leak: str\n"), errout.str());
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
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void switch2()
    {
        const std::string code("void f()\n"
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
        check(code.c_str(), false);
        ASSERT_EQUALS("", errout.str());
        check(code.c_str(), true);
        ASSERT_EQUALS("[test.cpp:12]: Memory leak: str\n", errout.str());
    }

    void switch3()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    while (abc)\n"
              "    {\n"
              "        switch (def)\n"
              "        {\n"
              "            default:\n"
              "                return;\n"
              "        }\n"
              "    }\n"
              "    delete [] str;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9]: Memory leak: str\n", errout.str());
    }




    void ret1()
    {
        check("char *f( char **str )\n"
              "{\n"
              "    char *ret = malloc( 10 );\n"
              "    return *str = ret;\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void ret2()
    {
        check("void foo()\n"
              "{\n"
              "    struct ABC *abc = new ABC;\n"
              "    abc->a = new char[10];\n"
              "    if ( ! abc->a )\n"
              "        return;\n"
              "    delete [] abc->a;\n"
              "    delete abc;\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:6]: Memory leak: abc\n"), errout.str());
    }

    void ret3()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *filep = fopen(\"myfile.txt\",\"w\");\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:4]: Resource leak: filep\n"), errout.str());
    }

    void ret4()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *p = popen( \"ls -l\", \"r\");\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:4]: Resource leak: p\n"), errout.str());
    }

    void ret5()
    {
        check("static char * f()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return (c ? c : NULL);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }




    void mismatch1()
    {
        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "    free(a);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:4]: Mismatching allocation and deallocation: a\n"), errout.str());
    }





    ////////////////////////////////////////////////
    // function calls
    ////////////////////////////////////////////////


    void func1()
    {
        check("static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void func2()
    {
        check("static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo.add(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void func3()
    {
        check("static void foo(const char *str)\n"
              "{ }\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: Memory leak: p\n"), errout.str());
    }


    void func4()
    {
        check("static void foo(char *str)\n"
              "{\n"
              "    delete [] str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void func5()
    {
        check("static void foo(char *str)\n"
              "{\n"
              "    delete str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        std::string err(errout.str());
        ASSERT_EQUALS(std::string("[test.cpp:9] -> [test.cpp:3]: Mismatching allocation and deallocation: str\n"), err);
    }


    void func6()
    {
        check("static void foo(char *str)\n"
              "{\n"
              "    goto abc;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        std::string err(errout.str());
        ASSERT_EQUALS(std::string("[test.cpp:10]: Memory leak: p\n"), err);
    }


    void func7()
    {
        check("static void foo(char *str)\n"
              "{\n"
              "    if (abc)\n"
              "        return;"
              "    delete [] str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n");
        std::string err(errout.str());
        ASSERT_EQUALS(std::string("[test.cpp:11]: Memory leak: p\n"), err);
    }


    void func8()
    {
        check("static void foo()\n"
              "{\n"
              "    char *str = new char[100];"
              "    (*release)(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void func9()
    {
        check("int b()\n"
              "{\n"
              "    return 0;\n"
              "}\n"
              "\n"
              "void a()\n"
              "{\n"
              "    char *a = new char[10];\n"
              "    if (b())\n"
              "        return;\n"
              "    delete [] a;\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void func10()
    {
        check("static void f(void (*fnc)(char*))\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (fnc)(c);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void func11()
    {
        check("static void f(struct1 *s1)\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (s1->fnc)(c);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void func12()
    {
        check("void add_list(struct mmtimer *n)\n"
              "{\n"
              "    rb_link_node(&n->list, parent, link);\n"
              "}\n"
              "\n"
              "int foo()\n"
              "{\n"
              "    struct mmtimer *base;\n"
              "\n"
              "    base = kmalloc(sizeof(struct mmtimer), GFP_KERNEL);\n"
              "    if (base == NULL)\n"
              "        return -ENOMEM;\n"
              "\n"
              "    add_list(base);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
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
    */





    void class1()
    {
        check("class Fred\n"
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
              "}\n", true);

        ASSERT_EQUALS(std::string("[test.cpp:1]: Memory leak: Fred::str1\n"), errout.str());
    }


    void class2()
    {
        check("class Fred\n"
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
              "}\n", true);

        ASSERT_EQUALS(std::string("[test.cpp:17]: Mismatching allocation and deallocation: Fred::str1\n"), errout.str());
    }

    void class3()
    {
        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "    void deleteTokens(Token *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "    _tokens = new Token;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(Token *tok)\n"
              "{\n"
              "    while (tok)\n"
              "    {\n"
              "        Token *next = tok->next();\n"
              "        delete tok;\n"
              "        tok = next;\n"
              "    }\n"
              "}\n", true);

        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void class4()
    {
        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC *abc);\n"
              "public:\n"
              "    void click();\n"
              "};\n"
              "\n"
              "void Fred::addAbc(ABC* abc)\n"
              "{\n"
              "    AbcPosts->Add(abc);\n"
              "}\n"
              "\n"
              "void Fred::click()\n"
              "{\n"
              "    ABC *p = new ABC;\n"
              "    addAbc( p );\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void class5()
    {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *str = new char[100];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10]: Memory leak: str\n", errout.str());
    }

    void class6()
    {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *str = new char[100];\n"
              "    delete [] str;\n"
              "    hello();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }




    void throw1()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    if ( ! abc )\n"
              "        throw 123;\n"
              "    delete [] str;\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:5]: Memory leak: str\n"), errout.str());
    }

    void throw2()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    try\n"
              "    {\n"
              "        str = new char[100];\n"
              "        if ( somecondition )\n"
              "            throw exception;\n"
              "        delete [] str;\n"
              "    }\n"
              "    catch ( ... )\n"
              "    {\n"
              "        delete [] str;\n"
              "    }\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }




    void linux_list_1()
    {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    struct AB *ab = new AB;\n"
              "    func(&ab->a);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }



    void sizeof1()
    {
        check("void f()\n"
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
              "}\n");

        std::string err(errout.str());

        ASSERT_EQUALS(std::string("[test.cpp:12]: Memory leak: s2\n"), err);
    }


    void realloc1()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = realloc(a, 100);\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:5]: Memory leak: a\n"), errout.str());
    }

    void realloc2()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (char *)realloc(a, 100);\n"
              "    free(a);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void assign()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = 0;\n"
              "    free(a);\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:3]: Memory leak: a\n"), errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a;\n"
              "    free(p);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a + 1;\n"
              "    free(p);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a += 10;\n"
              "    free(a - 10);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (void *)a + 10;\n"
              "    free(a - 10);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = new char[100];\n"
              "    list += a;\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void varid()
    {
        check("void foo()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    {\n"
              "        char *p = 0;\n"
              "        delete p;\n"
              "    }\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void cast1()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = reinterpret_cast<char *>(malloc(10));\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:4]: Memory leak: a\n"), errout.str());
    }

    void cast2()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = malloc(10);\n"
              "    free((void *)a);\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void cast3()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = malloc(10);\n"
              "    free(reinterpret_cast<void *>(a));\n"
              "}\n");

        ASSERT_EQUALS(std::string(""), errout.str());
    }





    void structmember1()
    {
        check("void f()\n"
              "{\n"
              "    struct ABC *abc = new ABC;\n"
              "    abc->a = new char[100];\n"
              "    delete abc;\n"
              "}\n");

        ASSERT_EQUALS(std::string("[test.cpp:5]: Memory leak: abc.a\n"), errout.str());
    }




    void dealloc_use_1()
    {
        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    delete [] s;\n"
              "    p = s;\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: Using \"s\" after it has been deallocated / released\n"), errout.str());
    }

    void dealloc_use_2()
    {
        check("void f()\n"
              "{\n"
              "    char *str;\n"
              "    free(str);\n"
              "    foo(&str);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void dealloc_use_3()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    free(str);\n"
              "    f1(&str);\n"
              "    f2(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void dealloc_use_4()
    {
        check("static void ReadDir(DIR *d)\n"
              "{\n"
              "    DIR *subdir = OpenDir();\n"
              "    ReadDir( subdir );\n"
              "    closedir(subdir);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

    void dealloc_use_5()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    free(str);\n"
              "    char c = str[10];\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: Using \"str\" after it has been deallocated / released\n"), errout.str());
    }

    void dealloc_use_6()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    free(str);\n"
              "    printf(\"free %x\", str);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void freefree1()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(100);\n"
              "    free(str);\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:5]: Deallocating a deallocated pointer: str\n"), errout.str());
    }

    void freefree2()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *fd = fopen(\"test.txt\", \"wb\");\n"
              "    fprintf(fd, \"test\");\n"
              "    fclose(fd);\n"
              "}\n");
        ASSERT_EQUALS(std::string(""), errout.str());
    }

};

REGISTER_TEST(TestMemleak)


