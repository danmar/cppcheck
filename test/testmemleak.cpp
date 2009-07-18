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




#define UNIT_TESTING
#include "../src/tokenize.h"
#include "../src/checkmemoryleak.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;


class TestMemleak : private TestFixture
{
public:
    TestMemleak() : TestFixture("TestMemleak")
    { }

private:
    void run()
    {
        TEST_CASE(testFunctionReturnType);
    }

    CheckMemoryLeak::AllocType functionReturnType(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        return ((const CheckMemoryLeak *)0)->functionReturnType(tokenizer.tokens());
    }

    void testFunctionReturnType()
    {
        {
            const char code[] = "const char *foo()\n"
                                "{ return 0; }";
            ASSERT_EQUALS(CheckMemoryLeak::No, functionReturnType(code));
        }

        {
            const char code[] = "Fred *newFred()\n"
                                "{ return new Fred; }";
            ASSERT_EQUALS(CheckMemoryLeak::New, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{ return new char[100]; }";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{\n"
                                "    char *p = new char[100];\n"
                                "    return p;\n"
                                "}";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }
    }
};

static TestMemleak testMemleak;





class TestMemleakInFunction : public TestFixture
{
public:
    TestMemleakInFunction() : TestFixture("TestMemleakInFunction")
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
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.check();
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
        TEST_CASE(simple11);
        TEST_CASE(simple12);
        TEST_CASE(new_nothrow);

        TEST_CASE(staticvar);

        TEST_CASE(alloc_alloc_1);

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
        TEST_CASE(ifelse10);

        TEST_CASE(if1);
        TEST_CASE(if2);
        TEST_CASE(if3);
        TEST_CASE(if4);
        TEST_CASE(if5);
        TEST_CASE(if6);     // Bug 2432631
        TEST_CASE(if7);     // Bug 2401436
        TEST_CASE(if8);     // Bug 2458532
        TEST_CASE(if9);     // if (realloc)
        TEST_CASE(if10);    // else if (realloc)
        TEST_CASE(if11);

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
        TEST_CASE(forwhile11);

        TEST_CASE(dowhile1);

        TEST_CASE(switch1);
        TEST_CASE(switch2);
        TEST_CASE(switch3);

        TEST_CASE(ret1);
        TEST_CASE(ret2);
        TEST_CASE(ret3);
        TEST_CASE(ret4);
        TEST_CASE(ret5);        // Bug 2458436 - return use
        TEST_CASE(ret6);
        TEST_CASE(ret7);
        TEST_CASE(ret8);

        TEST_CASE(mismatch1);
        TEST_CASE(mismatch2);
        TEST_CASE(mismatch3);
        TEST_CASE(mismatch4);

        TEST_CASE(func1);
        TEST_CASE(func2);
        TEST_CASE(func3);
        TEST_CASE(func4);
        TEST_CASE(func5);
        TEST_CASE(func6);
        TEST_CASE(func7);
        TEST_CASE(func8);       // Using callback
        TEST_CASE(func9);       // Embedding the function call in a if-condition
        TEST_CASE(func10);      // Bug 2458510 - Function pointer
        TEST_CASE(func11);      // Bug 2458510 - Function pointer
        TEST_CASE(func12);
        TEST_CASE(func13);
        TEST_CASE(func14);
        TEST_CASE(func15);

        TEST_CASE(allocfunc1);

        TEST_CASE(throw1);
        TEST_CASE(throw2);

        TEST_CASE(linux_list_1);

        TEST_CASE(sizeof1);

        TEST_CASE(realloc1);
        TEST_CASE(realloc2);
        TEST_CASE(realloc3);
        TEST_CASE(realloc4);

        TEST_CASE(assign);

        TEST_CASE(varid);

        TEST_CASE(cast1);
        TEST_CASE(cast2);
        TEST_CASE(cast3);


        TEST_CASE(structmember1);

        TEST_CASE(dealloc_use_1);       // Deallocate and then use memory
        TEST_CASE(dealloc_use_2);       // Deallocate and then use memory. No error if "use" is &var
        TEST_CASE(dealloc_use_3);       // Deallocate and then use memory. No error
        TEST_CASE(dealloc_use_4);
        TEST_CASE(dealloc_use_5);
        TEST_CASE(dealloc_use_6);
        TEST_CASE(dealloc_use_7);

        // free a free'd pointer
        TEST_CASE(freefree1);
        TEST_CASE(freefree2);
        TEST_CASE(strcpy_result_assignment);
        TEST_CASE(strcat_result_assignment);

        TEST_CASE(all1);                // Extra checking when --all is given

        TEST_CASE(malloc_constant_1);     // Check that the malloc constant matches the type

        // Calls to unknown functions.. they may throw exception, quit the program, etc
        TEST_CASE(unknownFunction1);
        TEST_CASE(unknownFunction2);
        TEST_CASE(unknownFunction3);
        TEST_CASE(unknownFunction4);
        TEST_CASE(unknownFunction5);

        // VCL..
        TEST_CASE(vcl1);
        TEST_CASE(vcl2);

        TEST_CASE(autoptr1);
        TEST_CASE(if_with_and);
        TEST_CASE(assign_pclose);

        // Using the function "exit"
        TEST_CASE(exit1);
        TEST_CASE(exit2);
        TEST_CASE(stdstring);

        TEST_CASE(strndup_function);
        TEST_CASE(tmpfile_function);
        TEST_CASE(fcloseall_function);
        TEST_CASE(file_functions);
        TEST_CASE(getc_function);

        TEST_CASE(open_function);
        TEST_CASE(creat_function);
        TEST_CASE(close_function);
        TEST_CASE(fd_functions);

        TEST_CASE(opendir_function);
        TEST_CASE(fdopendir_function);
        TEST_CASE(closedir_function);
        TEST_CASE(dir_functions);

        TEST_CASE(pointer_to_pointer);
        TEST_CASE(dealloc_and_alloc_in_func);
    }



    void simple1()
    {
        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());

        // ticket #346
        check("void f()\n"
              "{\n"
              "    int * const a = new int[10];\n"
              "    const int * const b = new int[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n[test.cpp:5]: (error) Memory leak: b\n",
                      errout.str());
    }

    void simple2()
    {
        check("Fred *NewFred()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "    return f;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simple3()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    return (char *)s;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void simple4()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: s\n", errout.str());
    }


    void simple5()
    {
        check("static char *f()\n"
              "{\n"
              "    struct *str = new strlist;\n"
              "    return &str->s;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void simple6()
    {
        check("static void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    char *str2 = (char *)str;\n"
              "    free(str2);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void simple7()
    {
        // A garbage collector may delete f automaticly
        check("class Fred;\n"
              "void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }


    void simple9()
    {
        check("void foo()\n"
              "{\n"
              "    MyClass *c = new MyClass();\n"
              "    c->free(c);\n"
              "    delete c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simple10()
    {
        check("void foo()\n"
              "{\n"
              "    FILE * f = fopen(fname, str);\n"
              "    if ( fclose(f) != NULL );\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simple11()
    {
        check("void Fred::aaa()\n"
              "{ }\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *s = NULL;\n"
              "    if (a)\n"
              "        s = malloc(10);\n"
              "    else if (b)\n"
              "        s = malloc(10);\n"
              "    else\n"
              "        f();\n"
              "    g(s);\n"
              "    if (c)\n"
              "        h(s);\n"
              "    free(s);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void simple12()
    {
        check("void f()\n"
              "{\n"
              "    char *s;\n"
              "    foo(s);\n"
              "    s = malloc(100);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: s\n", errout.str());
    }



    void new_nothrow()
    {
        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    using std::nothrow;\n"
              "    int *p = new(nothrow) int;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    using namespace std;\n"
              "    int *p = new(nothrow) int[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int;\n"
              "    delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int[10];\n"
              "    delete p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: p\n", errout.str());
    }


    void staticvar()
    {
        check("int f()\n"
              "{\n"
              "    static char *s = 0;\n"
              "    free(s);\n"
              "    s = malloc(100);\n"
              "    return 123;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void alloc_alloc_1()
    {
        check("void foo()\n"
              "{\n"
              "    char *str;\n"
              "    str = new char[10];\n"
              "    str = new char[20];\n"
              "    delete [] str;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());
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

        ASSERT_EQUALS("", errout.str());
    }


    void use2()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = strdup(\"abc\");\n"
              "    if ( abc ) { memset(str, 0, 3); }\n"
              "    *somestr = str;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    if (a==b)\n"
              "    {\n"
              "        free(str);\n"
              "        return;\n"
              "    }\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:9]: (possible error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: s\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }


    void ifelse10()
    {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    if ( ghfgf )\n"
              "    {\n"
              "        str[0] = s;\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        str[0] = s;\n"
              "    }\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: p\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }

    void if3()
    {
        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    if (0 != s)\n"
              "        foo(s);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: c\n", errout.str());
    }

    void if9()
    {
        check("static void f()\n"
              "{\n"
              "    char *buf = NULL, *tmp;\n"
              "    if (!(tmp = realloc(buf, 50)))\n"
              "    {\n"
              "        free(buf);\n"
              "        return NULL;\n"
              "    }\n"
              "    buf = tmp;\n"
              "    return buf;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void if10()
    {
        check("static void f()\n"
              "{\n"
              "    char *buf = malloc(10);\n"
              "    if (aa)\n"
              "        ;\n"
              "    else if (buf = realloc(buf, 100))\n"
              "        ;\n"
              "    free(buf);\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: buf\n", errout.str());
    }

    void if11()
    {
        check("void foo()\n"
              "{\n"
              "    int *x = new int[10];\n"
              "    if (x == 0 || aa)\n"
              "    {\n"
              "        return 1;\n"
              "    }\n"
              "    delete [] x;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: x\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: str\n", errout.str());
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
        TODO_ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: a\n", errout.str());
    }


    void forwhile9()
    {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for(i = 0 ;i < 50 ; i++)\n"
              "    {\n"
              "        if(func1(i))\n"
              "            continue;\n"
              "        a = realloc( a, i );\n"
              "        if(func2(i))\n"
              "            continue;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }


    void forwhile10()
    {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for(i = 0; i < 50; i++)\n"
              "    {\n"
              "        if(func1(i))\n"
              "            continue;\n"
              "        a = realloc( a, i );\n"
              "        if(func2(i))\n"
              "            return;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: a\n", errout.str());
    }


    void forwhile11()
    {
        check("int main()\n"
              "{\n"
              "    FILE *stream=NULL;\n"
              "    while((stream = fopen(name,\"r\")) == NULL)\n"
              "    { }\n"
              "    if(stream!=NULL) fclose(stream);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:12]: (possible error) Memory leak: str\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:9]: (error) Memory leak: str\n", errout.str());
    }




    void ret1()
    {
        check("char *f( char **str )\n"
              "{\n"
              "    char *ret = malloc( 10 );\n"
              "    return *str = ret;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
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

        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: abc\n", errout.str());
    }

    void ret3()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *filep = fopen(\"myfile.txt\",\"w\");\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: filep\n", errout.str());
    }

    void ret4()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *p = popen( \"ls -l\", \"r\");\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: p\n", errout.str());
    }

    void ret5()
    {
        check("static char * f()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return (c ? c : NULL);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void ret6()
    {
        check("void foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return strcpy(c, \"foo\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void ret7()
    {
        check("void foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return memcpy(c, \"foo\",4);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void ret8()
    {
        check("char *foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return ((char *)(c+1));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void mismatch1()
    {
        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "    free(a);\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: a\n", errout.str());
    }

    void mismatch2()
    {
        check("void f()\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "    fp = fopen();\n"
              "    fclose(fp);\n"
              "\n"
              "    fp = popen();\n"
              "    pclose(fp);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch3()
    {
        check("void f()\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "    if (abc) fp = fopen();\n"
              "    else fp = popen();\n"
              "\n"
              "    if (abc) fclose(fp);\n"
              "    else pclose(fp);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch4()
    {
        check("void f()\n"
              "{\n"
              "    char *p = 0;\n"
              "    for (i = 0; i < 10; ++i)\n"
              "    {\n"
              "        delete p;\n"
              "        p = new char[100];\n"
              "    }\n"
              "    delete [] p;\n"
              "}\n", false);
        ASSERT_EQUALS("[test.cpp:7]: (error) Mismatching allocation and deallocation: p\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }


    void func2()
    {
        check("static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo.add(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: p\n", errout.str());
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
        ASSERT_EQUALS("", errout.str());
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
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: str\n",
                      errout.str());
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
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: p\n", errout.str());
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
        TODO_ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: p\n", errout.str());
    }


    void func8()
    {
        check("static void foo()\n"
              "{\n"
              "    char *str = new char[100];"
              "    (*release)(str);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }

    void func10()
    {
        check("static void f(void (*fnc)(char*))\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (fnc)(c);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void func11()
    {
        check("static void f(struct1 *s1)\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (s1->fnc)(c);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }

    void func13()
    {
        check("static void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    foo(&p);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void func14()
    {
        // It is not known what the "foo" that only takes one parameter does..
        check("static void foo(char *a, char *b)\n"
              "{\n"
              "    free(a);\n"
              "    free(b);\n"
              "}\n"
              "static void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    foo(p);\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void func15()
    {
        check("static void a()\n"
              "{ return true; }\n"
              "\n"
              "static void b()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    if (a()) return;\n"    // <- memory leak
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: p\n", errout.str());
    }



    void allocfunc1()
    {
        check("static char *a()\n"
              "{\n"
              "    return new char[100];\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a();\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (error) Memory leak: p\n"), errout.str());
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

        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());
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

        ASSERT_EQUALS("", errout.str());
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

        ASSERT_EQUALS("", errout.str());
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

        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: s2\n", errout.str());
    }


    void realloc1()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = realloc(a, 100);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n", errout.str());
    }

    void realloc2()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (char *)realloc(a, 100);\n"
              "    free(a);\n"
              "}\n");

        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n", errout.str());
    }

    void realloc3()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void realloc4()
    {
        check("void foo()\n"
              "{\n"
              "    static char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}\n");

        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n", errout.str());
    }


    void assign()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = 0;\n"
              "    free(a);\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a;\n"
              "    free(p);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a + 1;\n"
              "    free(p);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a += 10;\n"
              "    free(a - 10);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (void *)a + 10;\n"
              "    free(a - 10);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = new char[100];\n"
              "    list += a;\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
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
        TODO_ASSERT_EQUALS("", errout.str());
    }

    void cast1()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = reinterpret_cast<char *>(malloc(10));\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());
    }

    void cast2()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = malloc(10);\n"
              "    free((void *)a);\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }

    void cast3()
    {
        check("void foo()\n"
              "{\n"
              "    char *a = malloc(10);\n"
              "    free(reinterpret_cast<void *>(a));\n"
              "}\n");

        ASSERT_EQUALS("", errout.str());
    }





    void structmember1()
    {
        check("void f()\n"
              "{\n"
              "    struct ABC *abc = new ABC;\n"
              "    abc->a = new char[100];\n"
              "    delete abc;\n"
              "}\n");

        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());
    }




    void dealloc_use_1()
    {
        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    delete [] s;\n"
              "    p = s;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Using 's' after it is deallocated / released\n", errout.str());
    }

    void dealloc_use_2()
    {
        check("void f()\n"
              "{\n"
              "    char *str;\n"
              "    free(str);\n"
              "    foo(&str);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_use_4()
    {
        check("static void ReadDir(DIR *d)\n"
              "{\n"
              "    DIR *subdir = OpenDir();\n"
              "    ReadDir( subdir );\n"
              "    closedir(subdir);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_use_5()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    char c = str[10];\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Using 'str' after it is deallocated / released\n", errout.str());
    }

    void dealloc_use_6()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    free(str);\n"
              "    printf(\"free %x\", str);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_use_7()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    delete [] str;\n"
              "    str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Using 'str' after it is deallocated / released\n", errout.str());
    }


    void freefree1()
    {
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(100);\n"
              "    free(str);\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Deallocating a deallocated pointer: str\n", errout.str());
    }

    void freefree2()
    {
        check("void foo()\n"
              "{\n"
              "    FILE *fd = fopen(\"test.txt\", \"wb\");\n"
              "    fprintf(fd, \"test\");\n"
              "    fclose(fd);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void strcpy_result_assignment()
    {
        check("void foo()\n"
              "{\n"
              "    char *p1 = malloc(10);\n"
              "    char *p2 = strcpy(p1, \"a\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void strcat_result_assignment()
    {
        check("void foo()\n"
              "{\n"
              "    char *p = malloc(10);\n"
              "    p[0] = 0;\n"
              "    p = strcat( p, \"a\" );\n"
              "    free( p );\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }



    void all1()
    {
        check("void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}\n", false);
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Memory leak: f\n", errout.str());
    }



    void malloc_constant_1()
    {
        check("void foo()\n"
              "{\n"
              "    int *p = malloc(3);\n"
              "    free(p);\n"
              "}\n", false);
        ASSERT_EQUALS("[test.cpp:3]: (error) The given size 3 is mismatching\n", errout.str());
    }



    void unknownFunction1()
    {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    if (abc)\n"
              "    {\n"
              "        delete [] p;\n"
              "        ThrowException();\n"
              "    }\n"
              "    delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void unknownFunction2()
    {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    if (abc)\n"
              "    {\n"
              "        delete [] p;\n"
              "        ThrowException();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9]: (error) Memory leak: p\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    p = g();\n"
              "    delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());
    }

    void unknownFunction3()
    {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    ThrowException();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void unknownFunction4()
    {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    a();\n"
              "    if (b) return;\n"
              "    delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void unknownFunction5()
    {
        check("static void foo()\n"
              "{\n"
              "    char *p = NULL;\n"
              "\n"
              "    if( a )\n"
              "        p = malloc(100);\n"
              "\n"
              "    if( a )\n"
              "    {\n"
              "        FREENULL(p);\n"
              "        FREENULL();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void checkvcl(const char code[], const char _autoDealloc[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        {
            std::istringstream istr(code);
            tokenizer.tokenize(istr, "test.cpp");
        }
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        Settings settings;
        settings._debug = true;
        settings._showAll = true;

        {
            std::istringstream istr(_autoDealloc);
            settings.autoDealloc(istr);
        }

        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.check();
    }



    void vcl1()
    {
        checkvcl("void Form1::foo()\n"
                 "{\n"
                 "    TEdit *Edit1 = new TEdit(this);\n"
                 "}\n", "TEdit\n");
        ASSERT_EQUALS("", errout.str());
    }


    void vcl2()
    {
        checkvcl("class Fred\n"
                 "{\n"
                 "private:\n"
                 "    TButton *button;\n"
                 "public:\n"
                 "    Fred();\n"
                 "};\n"
                 "\n"
                 "Fred::Fred()\n"
                 "{\n"
                 "    button = new TButton(this);\n"
                 "}\n", "TButton\n");
        ASSERT_EQUALS("", errout.str());
    }


    void autoptr1()
    {
        check("std::auto_ptr<int> foo()\n"
              "{\n"
              "    int *i = new int;\n"
              "    return std::auto_ptr<int>(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if_with_and()
    {
        check("void f()\n"
              "{\n"
              "  char *a = new char[10];\n"
              "  if (!a && b() )\n"
              "    return;\n"
              "\n"
              "  delete [] a;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char *a = new char[10];\n"
              "  if (b() && !a )\n"
              "    return;\n"
              "\n"
              "  delete [] a;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void assign_pclose()
    {
        check("void f()\n"
              "{\n"
              "  FILE *f = popen (\"test\", \"w\");\n"
              "  int a = pclose(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void exit1()
    {
        // Ticket #297
        check("void f()\n"
              "{\n"
              "    char *out = new char[100];\n"
              "    if (c())\n"
              "    {\n"
              "        delete [] out;\n"
              "        exit(0);\n"
              "    }\n"
              "    delete [] out;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void exit2()
    {
        check("void f()\n"
              "{\n"
              "    char *out = new char[100];\n"
              "    exit(0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *out = new char[100];\n"
              "    if( out ) {}\n"
              "    exit(0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void stdstring()
    {
        check("void f(std::string foo)\n"
              "{\n"
              "    char *out = new char[11];\n"
              "    memset(&(out[0]), 0, 1);\n"
              "}\n");

        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: out\n", errout.str());
    }

    void strndup_function()
    {
        check("void f()\n"
              "{\n"
              "    char *out = strndup(\"text\", 3);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: out\n", errout.str());
    }

    void tmpfile_function()
    {
        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: f\n", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    if (!f)\n"
              "        return;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: f\n", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    fclose(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    if (!f)\n"
              "        return;\n"
              "    fclose(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("FILE *f()\n"
              "{\n"
              "    return tmpfile();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void fcloseall_function()
    {
        check("void f()\n"
              "{\n"
              "    FILE *f = fopen(fname, str);\n"
              "    fcloseall();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    fcloseall();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void open_function()
    {
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: fd\n", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    if (fd == -1)\n"
              "       return;\n"
              "    close(fd);\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    if (fd < 0)\n"
              "       return;\n"
              "    close(fd);\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void creat_function()
    {
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: fd\n", errout.str());
    }

    void close_function()
    {
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    close(fd);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "    close(fd);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "    if (close(fd) < 0) {\n"
              "        perror(\"close\");\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void fd_functions()
    {
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    read(fd, buf, count);\n"
              "    readv(fd, iov, iovcnt);\n"
              "    readahead(fd, offset, count);\n"
              "    pread(fd, buf, count, offset);\n"
              "    write(fd, buf, count);\n"
              "    writev(fd, iov, iovcnt);\n"
              "    pwrite(fd, buf, count, offset);\n"
              "    ioctl(fd, request);\n"
              "    posix_fallocate(fd, offset, len);\n"
              "    posix_fadvise(fd, offset, len, advise);\n"
              "    fsync(fd);\n"
              "    fdatasync(fd);\n"
              "    sync_file_range(fd, offset, nbytes, flags);\n"
              "    lseek(fd, offset, whence);\n"
              "    fcntl(fd, cmd);\n"
              "    flock(fd, op);\n"
              "    lockf(fd, cmd, len);\n"
              "    ftruncate(fd, len);\n"
              "    fstat(fd, buf);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:23]: (error) Resource leak: fd\n", errout.str());
    }

    void opendir_function()
    {
        check("void f()\n"
              "{\n"
              "    DIR *f = opendir(\".\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: f\n", errout.str());
    }

    void fdopendir_function()
    {
        check("void f(int fd)\n"
              "{\n"
              "    DIR *f = fdopendir(fd);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: f\n", errout.str());
    }

    void closedir_function()
    {
        check("void f()\n"
              "{\n"
              "    DIR *f = opendir(\".\");\n"
              "    closedir(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f(int fd)\n"
              "{\n"
              "    DIR *f = fdopendir(fd);\n"
              "    closedir(f);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    DIR * f = opendir(dirname);\n"
              "    if (closedir(f));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dir_functions()
    {
        check("void f()\n"
              "{\n"
              "    DIR *f = opendir(dir);\n"
              "    readdir(f);\n;"
              "    readdir_r(f, entry, res);\n;"
              "    rewinddir(f);\n;"
              "    telldir(f);\n;"
              "    seekdir(f, 2)\n;"
              "    scandir(f, namelist, filter, comp);\n;"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Resource leak: f\n", errout.str());
    }

    void file_functions()
    {
        check("void f()\n"
              "{\n"
              "FILE *f = fopen(fname, str);\n"
              "feof(f);\n"
              "clearerr(in);\n"
              "ferror(in);\n"
              "fread(ptr, 10, 1, in);\n"
              "fwrite(ptr, 10, 1, in);\n"
              "fflush(in);\n"
              "setbuf(in, buf);\n"
              "setbuffer(in, buf, 100);\n"
              "setlinebuf(in);\n"
              "setvbuf(in, buf, _IOLBF, 0);\n"
              "fseek(in, 10, SEEK_SET);\n"
              "fseeko(in, 10, SEEK_SET);\n"
              "ftell(in);\n"
              "ftello(in);\n"
              "rewind(in);\n"
              "fsetpos(in, 0);\n"
              "fgetpos(in, 10);\n"
              "fprintf(in, \"text\\n\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:22]: (error) Resource leak: f\n", errout.str());
    }

    void getc_function() {
        {
            check("void f()\n"
                  "{"
                  "    int c;\n"
                  "    FILE *fin1a = fopen (\"FILE.txt\", \"r\");\n"
                  "    while ( (c = getc (fin1a)) != EOF)\n"
                  "    { }\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: fin1a\n", errout.str());
        }

        {
            check("void f()\n"
                  "{\n"
                  "    int c;\n"
                  "    FILE *fin1b = fopen(\"FILE.txt\", \"r\");\n"
                  "    c = getc(fin1b);\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: fin1b\n", errout.str());
        }
    }

    void pointer_to_pointer()
    {
        check("void f(char **data)\n"
              "{\n"
              "    char *c = new char[12];\n"
              "    *c = 0;\n"
              "    *data = c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_and_alloc_in_func()
    {
        check("char *f( const char *x )\n"
              "{\n"
              "  delete [] x;\n"
              "  return new char[10];\n"
              "}\n"
              "\n"
              "int main()\n"
              "{\n"
              "  char *a=0;\n"
              "  a = f( a );\n"
              "  a[0] = 1;\n"
              "  delete [] a;\n"
              "  return 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

static TestMemleakInFunction testMemleakInFunction;
















class TestMemleakInClass : public TestFixture
{
public:
    TestMemleakInClass() : TestFixture("TestMemleakInClass")
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
        CheckMemoryLeakInClass checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.check();
    }

    void run()
    {
        TEST_CASE(class1);
        TEST_CASE(class2);
        TEST_CASE(class3);
        TEST_CASE(class4);
        TEST_CASE(class6);
        TEST_CASE(class7);
        TEST_CASE(class8);
        TEST_CASE(class9);
        TEST_CASE(class10);
        TEST_CASE(class11);

        TEST_CASE(free_member_in_sub_func);
    }


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

        ASSERT_EQUALS("[test.cpp:4]: (possible error) Memory leak: Fred::str1\n", errout.str());
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

        ASSERT_EQUALS("[test.cpp:17]: (error) Mismatching allocation and deallocation: Fred::str1\n", errout.str());
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

        TODO_ASSERT_EQUALS("", errout.str());
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

    void class7()
    {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    this->i = new int;\n"
              "}\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete this->i;\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void class8()
    {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a();\n"
              "    void doNothing() { }\n"
              "};\n"
              "\n"
              "void A::a()\n"
              "{\n"
              "    int* c = new int(1);\n"
              "    delete c;\n"
              "    doNothing(c);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void class9()
    {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "    ~A();\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int; }\n"
              "\n"
              "A::~A()\n"
              "{ delete (p); }\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void class10()
    {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() { p = new int; }\n"
              "};\n", true);
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Memory leak: A::p\n", errout.str());
    }

    void class11()
    {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A() : p(new int[10])\n"
              "{ }", true);
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Memory leak: A::p\n", errout.str());
    }


    void free_member_in_sub_func()
    {
        // Member function
        check("class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "    static void deleteTokens(int *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}\n", true);
        TODO_ASSERT_EQUALS("", errout.str());

        // Global function
        check("void deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}\n"
              "class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}\n", true);
        TODO_ASSERT_EQUALS("", errout.str());
    }
};

static TestMemleakInClass testMemleakInClass;







class TestMemleakStructMember : public TestFixture
{
public:
    TestMemleakStructMember() : TestFixture("TestMemleakStructMember")
    { }

private:
    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        Settings settings;
        CheckMemoryLeakStructMember checkMemoryLeakStructMember(&tokenizer, &settings, this);
        checkMemoryLeakStructMember.check();
    }

    void run()
    {
        TEST_CASE(test1);
    }

    void test1()
    {
        check("static void foo()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.a = malloc(10);\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());
    }
};


static TestMemleakStructMember testMemleakStructMember;

