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


#include "../src/tokenize.h"
#include "../src/checkbufferoverrun.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestBufferOverrun : public TestFixture
{
public:
    TestBufferOverrun() : TestFixture("TestBufferOverrun")
    { }

private:



    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Assign variable ids
        tokenizer.setVarId();
        tokenizer.simplifyTokenList();

        // Fill function list
        tokenizer.fillFunctionList();

        // Clear the error buffer..
        errout.str("");

        // Check for buffer overruns..
        Settings settings;
        settings._showAll = true;
        CheckBufferOverrun checkBufferOverrun(&tokenizer, &settings, this);
        checkBufferOverrun.bufferOverrun();
    }

    void run()
    {
        TEST_CASE(noerr1);
        TEST_CASE(noerr2);
        TEST_CASE(noerr3);
        TEST_CASE(noerr4);

        TEST_CASE(sizeof1);
        TEST_CASE(sizeof2);
        TEST_CASE(sizeof3);

        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(array_index_3);
        TEST_CASE(array_index_4);
        TEST_CASE(array_index_5);
        TEST_CASE(array_index_6);
        TEST_CASE(array_index_7);
        TEST_CASE(array_index_8);
        TEST_CASE(array_index_9);
        TEST_CASE(array_index_10);
        TEST_CASE(array_index_11);
        TEST_CASE(array_index_12);
        TEST_CASE(array_index_13);
        TEST_CASE(array_index_14);
        TEST_CASE(array_index_15);
        TEST_CASE(array_index_16);
        TEST_CASE(array_index_17);
        TEST_CASE(array_index_18);
        TEST_CASE(array_index_19);

        TEST_CASE(buffer_overrun_1);
        TEST_CASE(buffer_overrun_2);
        TEST_CASE(buffer_overrun_3);
        TEST_CASE(buffer_overrun_4);
        TEST_CASE(buffer_overrun_5);
        TEST_CASE(buffer_overrun_6);
        TEST_CASE(buffer_overrun_7);
        TEST_CASE(buffer_overrun_8);
        TEST_CASE(buffer_overrun_9);
        TEST_CASE(buffer_overrun_10);
        TEST_CASE(buffer_overrun_11);

        TEST_CASE(sprintf1);
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);
        TEST_CASE(sprintf5);
        TEST_CASE(sprintf6);

        TEST_CASE(snprintf1);
        TEST_CASE(snprintf2);
        TEST_CASE(snprintf3);
        TEST_CASE(snprintf4);

        TEST_CASE(strncat1);
        TEST_CASE(strncat2);

        TEST_CASE(cin1);

        TEST_CASE(varid1);
        TEST_CASE(varid2);

        TEST_CASE(assign1);

        TEST_CASE(alloc);    // Buffer allocated with new

        TEST_CASE(memset1);
        TEST_CASE(counter_test);
        TEST_CASE(strncpy1);
    }



    void noerr1()
    {
        check("void f()\n"
              "{\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr2()
    {
        check("void f1(char *str)\n"
              "{\n"
              "    strcpy(buf,str);\n"
              "}\n"
              "void f2(char *str)\n"
              "{\n"
              "    strcat(buf,str);\n"
              "}\n"
              "void f3(char *str)\n"
              "{\n"
              "    sprintf(buf,\"%s\",str);\n"
              "}\n"
              "void f4(const char str[])\n"
              "{\n"
              "    strcpy(buf, str);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr3()
    {
        check("static void f()\n"
              "{\n"
              "    char data[1];\n"
              "    return abc.data[1];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr4()
    {
        // The memory isn't read or written and therefore there is no error.
        check("static void f()\n"
              "{\n"
              "    char data[100];\n"
              "    const char *p = &data[100];\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }




    void sizeof1()
    {
        check("static void f()\n"
              "{\n"
              "    char data[10];\n"
              "    data[ sizeof(*data) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof2()
    {
        check("static void f()\n"
              "{\n"
              "    char data[10];\n"
              "    data[ sizeof(data[0]) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("static void f()\n"
              "{\n"
              "    int data[2];\n"
              "    data[ sizeof(data[0]) ] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Array index out of bounds\n", errout.str());
    }

    void sizeof3()
    {
        check("void f()\n"
              "{\n"
              "    char group[32];\n"
              "    snprintf(group, sizeof(group), \"%u\", 0);\n"
              "    struct group *gr;\n"
              "    snprintf(group, sizeof(group), \"%u\", gr->gr_gid);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_1()
    {
        check("void f()\n"
              "{\n"
              "    char str[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_2()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_3()
    {
        {
            check("void f()\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i;\n"
                  "    for (i = 0; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:6]: (possible error) Buffer overrun\n", errout.str());
        }

        {
            check("void f()\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i;\n"
                  "    for (i = 1; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:6]: (possible error) Buffer overrun\n", errout.str());
        }


        {
            check("void f(int a)\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i;\n"
                  "    for (i = a; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:6]: (possible error) Buffer overrun\n", errout.str());
        }
    }


    void array_index_4()
    {
        check("const int SIZE = 10;\n"
              "void f()\n"
              "{\n"
              "    int i[SIZE];\n"
              "    i[SIZE] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_5()
    {
        check("void f()\n"
              "{\n"
              "    int i[10];\n"
              "    i[ sizeof(i) - 1 ] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_6()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_7()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    abc->str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_8()
    {
        check("const int SIZE = 10;\n"
              "\n"
              "struct ABC\n"
              "{\n"
              "    char str[SIZE];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[SIZE] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:11]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_9()
    {
        check("static void memclr( char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( str );   // ERROR\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_10()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void memclr( char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    memclr(abc->str);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:13] -> [test.cpp:8]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_11()
    {
        check("class ABC\n"
              "{\n"
              "public:\n"
              "    ABC();\n"
              "    char *str[10];\n"
              "    struct ABC *next;"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    for ( ABC *abc = abc1; abc; abc = abc->next() )\n"
              "    {\n"
              "        abc->str[10] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:12]: (possible error) Array index out of bounds\n", errout.str());
    }


    void array_index_12()
    {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char str[10];\n"
              "public:\n"
              "    Fred();\n"
              "};\n"
              "Fred::Fred()\n"
              "{\n"
              "    str[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_13()
    {
        check("void f()\n"
              "{\n"
              "    char buf[10];\n"
              "    for (int i = 0; i < 100; i++)\n"
              "    {\n"
              "        if (i < 10)\n"
              "            int x = buf[i];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_14()
    {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+10] = i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_15()
    {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[10+i] = i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_16()
    {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+1] = i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_17()
    {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i*2] = i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+5] = i;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+6] = i;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }

    void array_index_18()
    {
        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        ++i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i=4;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[6];\n"
              "    for (int i = 0; i < 7; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (possible error) Buffer overrun\n", errout.str());
    }

    void array_index_19()
    {
        // "One Past the End" is legal, as long as pointer is not dereferenced.
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[2]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Getting more than one past the end is not legal
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[3]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Array index out of bounds\n", errout.str());
    }

    void buffer_overrun_1()
    {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    strcpy(str, \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }


    void buffer_overrun_2()
    {
        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(ABC *abc)\n"
              "{\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:8]: (possible error) Buffer overrun\n", errout.str());
    }


    void buffer_overrun_3()
    {
        check("int a[10];\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int i;\n"
              "    for (i = 0; i <= 10; ++i)\n"
              "        a[i] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (possible error) Buffer overrun\n", errout.str());
    }


    void buffer_overrun_4()
    {
        check("void foo()\n"
              "{\n"
              "    const char *p[2];\n"
              "    for (int i = 0; i < 8; ++i)\n"
              "        p[i] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Buffer overrun\n", errout.str());

        // No false positive
        check("void foo(int x, int y)\n"
              "{\n"
              "    const char *p[2];\n"
              "    x = y * p[1];\n"
              "    p[1] = 0;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_5()
    {
        check("void f()\n"
              "{\n"
              "    char n[5];\n"
              "    sprintf(n, \"d\");\n"
              "    printf(\"hello!\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_6()
    {
        check("void f()\n"
              "{\n"
              "   char n[5];\n"
              "   strcat(n, \"abc\");\n"
              "   strcat(n, \"def\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Buffer overrun\n", errout.str());

        check("void f()\n"
              "{\n"
              "   char n[5];\n"
              "   strcat(strcat(n, \"abc\"), \"def\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }

    void buffer_overrun_7()
    {
        // ticket #731
        check("void f()\n"
              "{\n"
              "    char a[2];\n"
              "    strcpy(a, \"a\\0\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_8()
    {
        // ticket #714
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i+= 100)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = i + 100)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = 100 + i)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_9()
    {
        // ticket #738
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; )\n"
              "    {\n"
              "        a[i] = 0;\n"
              "        i += 100;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_10()
    {
        // ticket #740
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (int i = 0; i < 4; i++)\n"
              "    {\n"
              "        char b = a[i];\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_11()
    {
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i+=0.1)\n"
              "    {\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=i+0.1)\n"
              "    {\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=0.1+i)\n"
              "    {\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf1()
    {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    sprintf(str, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char * c = new char[10];\n"
              "    sprintf(c, \"%s\", \"/usr/LongLongLongLongUserName/bin/LongLongApplicationName\");\n"
              "    delete [] c;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }

    void sprintf2()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    sprintf(str, \"%d: %s\", getnumber(), \"abcde\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }

    void sprintf3()
    {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    sprintf(str, \"test\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    sprintf(str, \"test%s\", \"\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf4()
    {
        // ticket #690
        check("void f()\n"
              "{\n"
              "    char a[3];\n"
              "    sprintf(a, \"%02ld\", 99);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf5()
    {
        // ticket #729
        check("void f(bool condition)\n"
              "{\n"
              "    char buf[3];\n"
              "    sprintf(buf, \"%s\", condition ? \"11\" : \"22\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf6()
    {
        check("void f(bool condition)\n"
              "{\n"
              "    char buf[3];\n"
              "    sprintf(buf, \"%s\", condition ? \"11\" : \"222\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }

    void snprintf1()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 10, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) snprintf size is out of bounds\n", errout.str());
    }

    void snprintf2()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 5, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf3()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, sizeof str, \"%s\", \"abc\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf4()
    {
        check("void f(int x)\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 8 - x, \"abcdefghijkl\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }




    void strncat1()
    {
        check("void f()\n"
              "{\n"
              "    char str[16];\n"
              "    strncpy(str, a, 10);\n"
              "    strncat(str, b, 10);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Dangerous usage of strncat. Tip: the 3rd parameter means maximum number of characters to append\n", errout.str());
    }

    void strncat2()
    {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    strncat(str, a, 5);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Dangerous usage of strncat. Tip: the 3rd parameter means maximum number of characters to append\n", errout.str());
    }



    void cin1()
    {
        check("void f()\n"
              "{\n"
              "    char str[10];\n"
              "    cin >> str;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Dangerous usage of std::cin, possible buffer overrun\n", errout.str());
    }



    void varid1()
    {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        str[30] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }


    void varid2()
    {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        memset(str,0,50);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void assign1()
    {
        check("char str[3] = {'a', 'b', 'c'};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    str[3] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible error) Array index out of bounds\n", errout.str());
    }



    void alloc()
    {
        check("void foo()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    s[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Array index out of bounds\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *s = malloc(10);\n"
              "    s[10] = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Array index out of bounds\n", errout.str());
    }


    void memset1()
    {
        check("void foo()\n"
              "{\n"
              "    char s[10];\n"
              "    memset(s, 5, '*');\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) The size argument is given as a char constant\n", errout.str());
    }

    void counter_test()
    {
        std::list<const Token*> unknownParameter;
        unknownParameter.push_back(0);

        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("Hello", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("s", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("i", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%d", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%1d", unknownParameter));
        ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%2.2d", unknownParameter));
        ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("%s", unknownParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("\\\"", unknownParameter));
        ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("Hello \0Text", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%%", unknownParameter));
        ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%d%d", unknownParameter));
        ASSERT_EQUALS(10, CheckBufferOverrun::countSprintfLength("\\\\\\\\Hello%d \0Text\\\\\\\\", unknownParameter));
        ASSERT_EQUALS(4, CheckBufferOverrun::countSprintfLength("%%%%%d", unknownParameter));

        Token strTok;
        strTok.str("\"12345\"");
        std::list<const Token*> stringAsParameter;
        stringAsParameter.push_back(&strTok);
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("str%s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-4s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", stringAsParameter));
        ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("%-6s", stringAsParameter));

        std::list<const Token*> intAsParameter;
        Token numTok;
        numTok.str("12345");
        stringAsParameter.push_back(&numTok);
        TODO_ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%02ld", intAsParameter));
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%08ld", intAsParameter));

        std::list<const Token*> multipleParams;
        multipleParams.push_back(&strTok);
        multipleParams.push_back(0);
        multipleParams.push_back(&numTok);
        TODO_ASSERT_EQUALS(15, CheckBufferOverrun::countSprintfLength("str%s%d%d", multipleParams));
        ASSERT_EQUALS(26, CheckBufferOverrun::countSprintfLength("str%-6s%08ld%08ld", multipleParams));

    }

    void strncpy1()
    {
        check("void f()\n"
              "{\n"
              " char a[6];\n"
              " char c[7];\n"
              " strcpy(a,\"hello\");\n"
              " strncpy(c,a,sizeof(c));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              " char a[6];\n"
              " char c[6];\n"
              " strcpy(a,\"hello\");\n"
              " strncpy(c,a,sizeof(c));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              " char a[6];\n"
              " char c[5];\n"
              " strcpy(a,\"hello\");\n"
              " strncpy(c,a,sizeof(c)+1);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (possible error) Buffer overrun\n", errout.str());

        check("void f()\n"
              "{\n"
              " char c[6];\n"
              " strncpy(c,\"hello!\",sizeof(c));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              " char c[6];\n"
              " strncpy(c,\"hello!\",sizeof(c)+1);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (possible error) Buffer overrun\n", errout.str());
    }
};

REGISTER_TEST(TestBufferOverrun)


