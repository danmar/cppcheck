/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "checkbufferoverrun.h"
#include "testsuite.h"

#include <sstream>
#include <climits>

extern std::ostringstream errout;

class TestBufferOverrun : public TestFixture {
public:
    TestBufferOverrun() : TestFixture("TestBufferOverrun") {
    }

private:

    void check(const char code[], bool experimental = true, const char filename[] = "test.cpp", bool verify = true) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.inconclusive = true;
        settings.standards.posix = true;
        settings.experimental = experimental;
        settings.addEnabled("warning");
        settings.addEnabled("style");
        settings.addEnabled("portability");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        const std::string str1(tokenizer.tokens()->stringifyList(0,true));

        // Assign variable ids
        tokenizer.simplifyTokenList2();

        const std::string str2(tokenizer.tokens()->stringifyList(0,true));

        // Ensure that the test case is not bad.
        if (verify && str1 != str2) {
            warnUnsimplified(str1, str2);
        }

        // Check for buffer overruns..
        CheckBufferOverrun checkBufferOverrun(&tokenizer, &settings, this);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.bufferOverrun2();
        checkBufferOverrun.arrayIndexThenCheck();
        checkBufferOverrun.writeOutsideBufferSize();
    }

    void checkstd(const char code[], const char filename[] = "test.cpp") {
        static bool init;
        static Settings settings;
        if (!init) {
            init = true;
            LOAD_LIB_2(settings.library, "std.cfg");
            settings.addEnabled("warning");
        }

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        // Clear the error buffer..
        errout.str("");

        // Check for buffer overruns..
        CheckBufferOverrun checkBufferOverrun(&tokenizer, &settings, this);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.bufferOverrun2();
        checkBufferOverrun.arrayIndexThenCheck();
        checkBufferOverrun.writeOutsideBufferSize();
    }

    void checkposix(const char code[], const char filename[] = "test.cpp") {
        static bool init;
        static Settings settings;
        if (!init) {
            init = true;
            LOAD_LIB_2(settings.library, "posix.cfg");
            settings.addEnabled("warning");
        }

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Clear the error buffer..
        errout.str("");

        // Check for buffer overruns..
        CheckBufferOverrun checkBufferOverrun(&tokenizer, &settings, this);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.bufferOverrun2();
        checkBufferOverrun.arrayIndexThenCheck();
        checkBufferOverrun.writeOutsideBufferSize();
    }


    void run() {
        TEST_CASE(noerr1);
        TEST_CASE(noerr2);
        TEST_CASE(noerr3);
        TEST_CASE(noerr4);

        TEST_CASE(sizeof3);

        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(array_index_3);
        TEST_CASE(array_index_6);
        TEST_CASE(array_index_7);
        TEST_CASE(array_index_9);
        TEST_CASE(array_index_11);
        TEST_CASE(array_index_12);
        TEST_CASE(array_index_13);
        TEST_CASE(array_index_14);
        TEST_CASE(array_index_15);
        TEST_CASE(array_index_16);
        TEST_CASE(array_index_17);
        TEST_CASE(array_index_18);
        TEST_CASE(array_index_19);
        TEST_CASE(array_index_20);
        TEST_CASE(array_index_21);
        TEST_CASE(array_index_22);
        TEST_CASE(array_index_23);
        TEST_CASE(array_index_24); // ticket #1492 and #1539
        TEST_CASE(array_index_25); // ticket #1536
        TEST_CASE(array_index_26);
        TEST_CASE(array_index_27);
        TEST_CASE(array_index_28); // ticket #1418
        TEST_CASE(array_index_29); // ticket #1734
        TEST_CASE(array_index_30); // ticket #2086 - out of bounds when type is unknown
        TEST_CASE(array_index_31); // ticket #2120 - out of bounds in subfunction when type is unknown
        TEST_CASE(array_index_32);
        TEST_CASE(array_index_33); // ticket #3044
        TEST_CASE(array_index_34); // ticket #3063
        TEST_CASE(array_index_35); // ticket #2889
        TEST_CASE(array_index_36); // ticket #2960
        TEST_CASE(array_index_37);
        TEST_CASE(array_index_38); // ticket #3273
        TEST_CASE(array_index_39);
        TEST_CASE(array_index_40); // loop variable calculation, taking address
        TEST_CASE(array_index_41); // structs with the same name
        TEST_CASE(array_index_42);
        TEST_CASE(array_index_43); // struct with array
        TEST_CASE(array_index_44); // #3979
        TEST_CASE(array_index_45); // #4207 - calling function with variable number of parameters (...)
        TEST_CASE(array_index_46); // #4840 - two-statement for loop
        TEST_CASE(array_index_47); // #5849
        TEST_CASE(array_index_multidim);
        TEST_CASE(array_index_switch_in_for);
        TEST_CASE(array_index_for_in_for);   // FP: #2634
        TEST_CASE(array_index_calculation);
        TEST_CASE(array_index_negative1);
        TEST_CASE(array_index_negative2);    // ticket #3063
        TEST_CASE(array_index_for_decr);
        TEST_CASE(array_index_varnames);     // FP: struct member. #1576
        TEST_CASE(array_index_for_continue); // for,continue
        TEST_CASE(array_index_for);          // FN: for,if
        TEST_CASE(array_index_for_neq);      // #2211: Using != in condition
        TEST_CASE(array_index_for_question); // #2561: for, ?:
        TEST_CASE(array_index_for_andand_oror);  // FN: using && or || in the for loop condition
        TEST_CASE(array_index_for_varid0);   // #4228: No varid for counter variable
        TEST_CASE(array_index_vla_for);      // #3221: access VLA inside for
        TEST_CASE(array_index_extern);       // FP when using 'extern'. #1684
        TEST_CASE(array_index_cast);         // FP after cast. #2841
        TEST_CASE(array_index_string_literal);
        TEST_CASE(array_index_same_struct_and_var_name); // #4751 - not handled well when struct name and var name is same
        TEST_CASE(array_index_valueflow);
        TEST_CASE(array_index_function_parameter);

        TEST_CASE(buffer_overrun_1_standard_functions);
        TEST_CASE(buffer_overrun_1_posix_functions);
        TEST_CASE(buffer_overrun_2_struct);
        TEST_CASE(buffer_overrun_3);
        TEST_CASE(buffer_overrun_4);
        TEST_CASE(buffer_overrun_5);
        TEST_CASE(buffer_overrun_6);
        TEST_CASE(buffer_overrun_7);
        TEST_CASE(buffer_overrun_8);
        TEST_CASE(buffer_overrun_9);
        TEST_CASE(buffer_overrun_10);
        TEST_CASE(buffer_overrun_11);
        TEST_CASE(buffer_overrun_12);
        TEST_CASE(buffer_overrun_13);
        TEST_CASE(buffer_overrun_14);
        TEST_CASE(buffer_overrun_15); // ticket #1787
        TEST_CASE(buffer_overrun_16);
        TEST_CASE(buffer_overrun_17); // ticket #2548
        TEST_CASE(buffer_overrun_18); // ticket #2576 - for, calculation with loop variable
        TEST_CASE(buffer_overrun_19); // #2597 - class member with unknown type
        TEST_CASE(buffer_overrun_20); // #2986 (segmentation fault)
        TEST_CASE(buffer_overrun_21);
        TEST_CASE(buffer_overrun_22); // #3124
        TEST_CASE(buffer_overrun_23); // #3153
        TEST_CASE(buffer_overrun_24); // index variable is changed in for-loop
        TEST_CASE(buffer_overrun_26); // #4432 (segmentation fault)
        TEST_CASE(buffer_overrun_27); // #4444 (segmentation fault)
        TEST_CASE(buffer_overrun_28); // Out of bound char array access
        TEST_CASE(buffer_overrun_bailoutIfSwitch);  // ticket #2378 : bailoutIfSwitch
        TEST_CASE(buffer_overrun_function_array_argument);
        TEST_CASE(possible_buffer_overrun_1); // #3035
        TEST_CASE(buffer_overrun_readSizeFromCfg);

        TEST_CASE(valueflow_string); // using ValueFlow string values in checking

        // It is undefined behaviour to point out of bounds of an array
        // the address beyond the last element is in bounds
        // char a[10];
        // char *p1 = a + 10;  // OK
        // char *p2 = a + 11   // UB
        TEST_CASE(pointer_out_of_bounds_1);
        TEST_CASE(pointer_out_of_bounds_2);
        TEST_CASE(pointer_out_of_bounds_sub);

        TEST_CASE(sprintf1);
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);
        TEST_CASE(sprintf5);
        TEST_CASE(sprintf6);
        TEST_CASE(sprintf7);
        TEST_CASE(sprintf8);
        TEST_CASE(sprintf9);
        TEST_CASE(sprintf10);

        TEST_CASE(snprintf1);
        TEST_CASE(snprintf2);
        TEST_CASE(snprintf4);
        TEST_CASE(snprintf5);
        TEST_CASE(snprintf6);
        TEST_CASE(snprintf7);

        TEST_CASE(strncat1);
        TEST_CASE(strncat2);
        TEST_CASE(strncat3);
        TEST_CASE(strncat4);

        TEST_CASE(strcat1);
        TEST_CASE(strcat2);
        TEST_CASE(strcat3);

        TEST_CASE(memfunc1);  // memchr/memset/memcpy
        TEST_CASE(memfunc2);
        TEST_CASE(memfunc3);  // ticket #1659

        TEST_CASE(varid1);
        TEST_CASE(varid2);
        TEST_CASE(varid3);  // ticket #4764

        TEST_CASE(assign1);

        TEST_CASE(alloc1);    // Buffer allocated with new
        TEST_CASE(alloc2);    // Buffer allocated with malloc
        TEST_CASE(alloc3);    // statically allocated buffer
        TEST_CASE(alloc4);    // Buffer allocated with alloca
        TEST_CASE(malloc_memset);  // using memset on buffer allocated with malloc

        TEST_CASE(memset1);
        TEST_CASE(memset2);
        TEST_CASE(counter_test);
        TEST_CASE(strncpy1);
        TEST_CASE(unknownType);

        TEST_CASE(terminateStrncpy1);
        TEST_CASE(terminateStrncpy2);
        TEST_CASE(terminateStrncpy3);
        TEST_CASE(recursive_long_time);

        TEST_CASE(crash1);  // Ticket #1587 - crash
        TEST_CASE(crash2);  // Ticket #3034 - crash
        TEST_CASE(crash3);  // Ticket #5426 - crash

        TEST_CASE(executionPaths1);
        TEST_CASE(executionPaths2);
        TEST_CASE(executionPaths3);   // no FP for function parameter
        TEST_CASE(executionPaths5);   // Ticket #2920 - False positive when size is unknown
        TEST_CASE(executionPaths6);   // unknown types

        TEST_CASE(cmdLineArgs1);

        TEST_CASE(scope);   // handling different scopes

        TEST_CASE(getErrorMessages);

        TEST_CASE(unknownMacroNoDecl);    // #2638 - not variable declaration: 'AAA a[0] = 0;'

        // Access array and then check if the used index is within bounds
        TEST_CASE(arrayIndexThenCheck);

        TEST_CASE(bufferNotZeroTerminated);
        TEST_CASE(readlink);
        TEST_CASE(readlinkat);

        TEST_CASE(writeOutsideBufferSize)

        TEST_CASE(negativeMemoryAllocationSizeError) // #389

        TEST_CASE(garbage1) // #6303
    }



    void noerr1() {
        check("extern int ab;\n"
              "void f()\n"
              "{\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "    if (ab)\n"
              "    {\n"
              "        char str[50];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr2() {
        check("static char buf[2];\n"
              "void f1(char *str)\n"
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
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr3() {
        check("struct { char data[10]; } abc;\n"
              "static char f()\n"
              "{\n"
              "    char data[1];\n"
              "    return abc.data[1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void noerr4() {
        // The memory isn't read or written and therefore there is no error.
        check("static void f() {\n"
              "    char data[100];\n"
              "    const char *p = data + 100;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sizeof3() {
        check("struct group { int gr_gid; };\n"
              "void f()\n"
              "{\n"
              "    char group[32];\n"
              "    snprintf(group, 32, \"%u\", 0);\n"
              "    struct group *gr;\n"
              "    snprintf(group, 32, \"%u\", gr->gr_gid);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_1() {
        check("void f()\n"
              "{\n"
              "    char str[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'str[16]' accessed at index 16, which is out of bounds.\n", errout.str());

        check("char f()\n"
              "{\n"
              "    char str[16];\n"
              "    return str[16];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'str[16]' accessed at index 16, which is out of bounds.\n", errout.str());

        // test stack array
        check("int f()\n"
              "{\n"
              "   int x[ 3 ] = { 0, 1, 2 };\n"
              "   int y;\n"
              "   y = x[ 4 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'x[3]' accessed at index 4, which is out of bounds.\n", errout.str());

        check("int f()\n"
              "{\n"
              "   int x[ 3 ] = { 0, 1, 2 };\n"
              "   int y;\n"
              "   y = x[ 2 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void array_index_2() {
        check("void f()\n"
              "{\n"
              "    char *str = new char[0x10];\n"
              "    str[15] = 0;\n"
              "    str[16] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'str[16]' accessed at index 16, which is out of bounds.\n", errout.str());

        check("void a(int i)\n" // valueflow
              "{\n"
              "    char *str = new char[0x10];\n"
              "    str[i] = 0;\n"
              "}\n"
              "void b() { a(16); }");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'str[16]' accessed at index 16, which is out of bounds.\n", errout.str());
    }


    void array_index_3() {
        {
            check("void f()\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i, sum=0;\n"
                  "    for (i = 0; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6]: (error) Array 'val[50]' accessed at index 99, which is out of bounds.\n", errout.str());
        }

        {
            check("void f()\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i, sum=0;\n"
                  "    for (i = 1; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6]: (error) Array 'val[50]' accessed at index 99, which is out of bounds.\n", errout.str());
        }


        {
            check("void f(int a)\n"
                  "{\n"
                  "    int val[50];\n"
                  "    int i, sum=0;\n"
                  "    for (i = a; i < 100; i++)\n"
                  "        sum += val[i];\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6]: (error) Array 'val[50]' accessed at index 99, which is out of bounds.\n", errout.str());
        }

        {
            check("typedef struct g g2[3];\n"
                  "void foo(char *a)\n"
                  "{\n"
                  "  for (int i = 0; i < 4; i++)\n"
                  "  {\n"
                  "    a[i]=0;\n"
                  "  }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }

        {
            check("void foo(int argc)\n"
                  "{\n"
                  "  char a[2];\n"
                  "  for (int i = 4; i < argc; i++)\n"
                  "  {\n"
                  "  }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }

        {
            check("void foo(int a[10]) {\n"
                  "    for (int i=0;i<50;++i) {\n"
                  "        a[i] = 0;\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:3]: (error) Array 'a[10]' accessed at index 49, which is out of bounds.\n", errout.str());
        }
    }

    void array_index_6() {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    abc.str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static char f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    return abc.str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        // This is not out of bounds because it is a variable length array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[1] = 0;"
              "}");
        ASSERT_EQUALS("", errout.str());

        // This is not out of bounds because it is not a variable length array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "    int x;\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[1] = 0;"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'x.str[1]' accessed at index 1, which is out of bounds.\n", errout.str());

        // This is not out of bounds because it is a variable length array
        // and the index is within the memory allocated.
        /** @todo this works by accident because we ignore any access to this array */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[10] = 0;"
              "}");
        ASSERT_EQUALS("", errout.str());

        // This is out of bounds because it is outside the memory allocated.
        /** @todo this doesn't work because of a bug in sizeof(struct)  */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC) + 10);\n"
              "    x->str[11] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (error) Array 'str[1]' accessed at index 11, which is out of bounds.\n", "", errout.str());

        // This is out of bounds if 'sizeof(ABC)' is 1 (No padding)
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(ABC) + 10);\n"
              "    x->str[11] = 0;"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        // This is out of bounds because it is outside the memory allocated
        /** @todo this doesn't work because of a bug in sizeof(struct) */
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(struct ABC));\n"
              "    x->str[1] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (error) Array 'str[1]' accessed at index 1, which is out of bounds.\n", "", errout.str());

        // This is out of bounds because it is outside the memory allocated
        // But only if 'sizeof(ABC)' is 1 (No padding)
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC* x = malloc(sizeof(ABC));\n"
              "    x->str[1] = 0;"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        // This is out of bounds because it is not a variable array
        check("struct ABC\n"
              "{\n"
              "    char str[1];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC x;\n"
              "    x.str[1] = 0;"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'x.str[1]' accessed at index 1, which is out of bounds.\n", errout.str());

        check("struct foo\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "void x()\n"
              "{\n"
              "    foo f;\n"
              "    for ( unsigned int i = 0; i < 64; ++i )\n"
              "        f.str[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'f.str[10]' accessed at index 63, which is out of bounds.\n", errout.str());

        check("struct AB { char a[NUM]; char b[NUM]; }\n"
              "void f(struct AB *ab) {\n"
              "    ab->a[0] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void array_index_7() {
        check("struct ABC\n"
              "{\n"
              "    char str[10];\n"
              "};\n"
              "\n"
              "static void f(struct ABC *abc)\n"
              "{\n"
              "    abc->str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_9() {
        check("static void memclr( char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( str );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (error) Array 'str[5]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("static void memclr( int i, char *data )\n"
              "{\n"
              "    data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( 0, str );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (error) Array 'str[5]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("static void memclr( int i, char *data )\n"
              "{\n"
              "    data[i] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( 10, str );\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (possible error) Array index out of bounds.\n",
                           "", errout.str());

        // This is not an error
        check("static void memclr( char *data, int size )\n"
              "{\n"
              "    if( size > 10 )"
              "      data[10] = 0;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char str[5];\n"
              "    memclr( str, 5 );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2097..
        check("void foo(int *p)\n"
              "{\n"
              "    --p;\n"
              "    p[2] = 0;\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "    int p[3];\n"
              "    foo(p+1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3168
        checkstd("void a(char *p) { memset(p,0,100); }\n"
                 "void b() {\n"
                 "    char buf[10];\n"
                 "    a(buf);"
                 "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:1]: (error) Buffer is accessed out of bounds: buf\n", errout.str());
    }


    void array_index_11() {
        check("class ABC\n"
              "{\n"
              "public:\n"
              "    ABC();\n"
              "    char *str[10];\n"
              "    struct ABC *next();"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    ABC *abc1;\n"
              "    for ( ABC *abc = abc1; abc; abc = abc->next() )\n"
              "    {\n"
              "        abc->str[10] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13]: (error) Array 'abc.str[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }


    void array_index_12() {
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
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'str[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char str[10];\n"
              "public:\n"
              "    char c();\n"
              "};\n"
              "char Fred::c()\n"
              "{\n"
              "    return str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'str[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_13() {
        check("void f()\n"
              "{\n"
              "    char buf[10];\n"
              "    for (int i = 0; i < 100; i++)\n"
              "    {\n"
              "        if (i < 10)\n"
              "            int x = buf[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_14() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+10] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[10]' accessed at index 19, which is out of bounds.\n", errout.str());
    }

    void array_index_15() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[10+i] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[10]' accessed at index 19, which is out of bounds.\n", errout.str());
    }

    void array_index_16() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i+1] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_17() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i*2] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[10]' accessed at index 18, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+5] = i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[12];\n"
              "    for (int i = 0; i < 12; i+=6)\n"
              "        a[i+6] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[12]' accessed at index 12, which is out of bounds.\n", errout.str());

        check("void f() {\n"  // #4398
              "    int a[2];\n"
              "    for (int i = 0; i < 4; i+=2)\n"
              "        a[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2]' accessed at index 2, which is out of bounds.\n", errout.str());

        check("void f() {\n"  // #4398
              "    int a[2];\n"
              "    for (int i = 0; i < 4; i+=2)\n"
              "        do_stuff(a+i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_18() {
        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i++;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        ++i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 6; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i=4;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[6];\n"
              "    for (int i = 0; i < 7; i++)\n"
              "    {\n"
              "        a[i] = i;\n"
              "        i+=1;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Buffer overrun\n", "", errout.str());
    }

    void array_index_19() {
        // "One Past the End" is legal, as long as pointer is not dereferenced.
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[2]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Getting more than one past the end is not legal
        check("void f()\n"
              "{\n"
              "  char a[2];\n"
              "  char *end = &(a[3]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2]' accessed at index 3, which is out of bounds.\n", errout.str());
    }

    void array_index_20() {
        check("void f()\n"
              "{\n"
              " char a[8];\n"
              " int b[10];\n"
              " for ( int i = 0; i < 9; i++ )\n"
              "  b[i] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_21() {
        check("class A {\n"
              " int indices[2];\n"
              " void foo(int indices[3]);\n"
              "};\n"
              "\n"
              "void A::foo(int indices[3]) {\n"
              " for(int j=0; j<3; ++j) {\n"
              "  int b = indices[j];\n"
              " }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_22() {
        check("int main() {\n"
              "  size_t indices[2];\n"
              "  int b = indices[2];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'indices[2]' accessed at index 2, which is out of bounds.\n", errout.str());
    }

    void array_index_23() {
        check("void foo()\n"
              "{\n"
              "    char c[10];\n"
              "    c[1<<23]='a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'c[10]' accessed at index 8388608, which is out of bounds.\n", errout.str());
    }

    void array_index_24() {
        // ticket #1492 and #1539
        // CHAR_MAX can be equal to SCHAR_MAX or UCHAR_MAX depending on the environment.
        // This test should work for both environments.
        std::ostringstream charMaxPlusOne;
        charMaxPlusOne << (CHAR_MAX+1);
        check(("void f(char n) {\n"
               "    int a[n];\n"     // n <= CHAR_MAX
               "    a[-1] = 0;\n"    // negative index
               "    a[" + charMaxPlusOne.str() + "] = 0;\n"   // 128/256 > CHAR_MAX
               "}\n").c_str());
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a["+charMaxPlusOne.str()+"]' accessed at index "+charMaxPlusOne.str()+", which is out of bounds.\n", errout.str());

        check("void f(signed char n) {\n"
              "    int a[n];\n"     // n <= SCHAR_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[128] = 0;\n"   // 128 > SCHAR_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a[128]' accessed at index 128, which is out of bounds.\n", errout.str());

        check("void f(unsigned char n) {\n"
              "    int a[n];\n"     // n <= UCHAR_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[256] = 0;\n"   // 256 > UCHAR_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a[256]' accessed at index 256, which is out of bounds.\n", errout.str());

        check("void f(short n) {\n"
              "    int a[n];\n"     // n <= SHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[32768] = 0;\n" // 32768 > SHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a[32768]' accessed at index 32768, which is out of bounds.\n", errout.str());

        check("void f(unsigned short n) {\n"
              "    int a[n];\n"     // n <= USHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[65536] = 0;\n" // 65536 > USHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a[65536]' accessed at index 65536, which is out of bounds.\n", errout.str());

        check("void f(signed short n) {\n"
              "    int a[n];\n"     // n <= SHRT_MAX
              "    a[-1] = 0;\n"    // negative index
              "    a[32768] = 0;\n" // 32768 > SHRT_MAX
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'a[32768]' accessed at index 32768, which is out of bounds.\n", errout.str());

        check("void f(int n) {\n"
              "    int a[n];\n"     // n <= INT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n", errout.str());

        check("void f(unsigned int n) {\n"
              "    int a[n];\n"     // n <= UINT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n", errout.str());

        check("void f(signed int n) {\n"
              "    int a[n];\n"     // n <= INT_MAX
              "    a[-1] = 0;\n"    // negative index
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index -1 is out of bounds.\n", errout.str());
    }

    void array_index_25() {
        // ticket #1536
        check("void foo()\n"
              "{\n"
              "   long l[SOME_SIZE];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_26() {
        check("void f()\n"
              "{\n"
              "    int a[3];\n"
              "    for (int i = 3; 0 <= i; i--)\n"
              "        a[i] = i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'a[3]' accessed at index 3, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int a[4];\n"
              "    for (int i = 3; 0 <= i; i--)\n"
              "        a[i] = i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_27() {
        check("void f()\n"
              "{\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 10; i++)\n"
              "        a[i-1] = a[i];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array index -1 is out of bounds.\n", errout.str());
    }

    void array_index_28() {
        // ticket #1418
        check("void f()\n"
              "{\n"
              "    int i[2];\n"
              "    int *ip = i + 1;\n"
              "    ip[-10] = 1;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Array ip[-10] out of bounds.\n", "", errout.str());
    }

    void array_index_29() {
        // ticket #1724
        check("void f()\n"
              "{\n"
              "    int iBuf[10];"
              "    int *i = iBuf + 9;"
              "    int *ii = i + -5;"
              "    ii[10] = 0;"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Array ii[10] out of bounds.\n", "", errout.str());
    }

    void array_index_30() {
        // ticket #2086 - unknown type
        check("void f() {\n"
              "    UINT8 x[2];\n"
              "    x[5] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'x[2]' accessed at index 5, which is out of bounds.\n", errout.str());
    }

    void array_index_31() {
        // ticket #2120 - sub function, unknown type
        check("struct s1 {\n"
              "    unknown_type_t delay[3];\n"
              "};\n"
              "\n"
              "void x(unknown_type_t *delay, const int *net) {\n"
              "    delay[0] = 0;\n"
              "}\n"
              "\n"
              "void y() {\n"
              "    struct s1 obj;\n"
              "    x(obj.delay, 123);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct s1 {\n"
              "    unknown_type_t delay[3];\n"
              "};\n"
              "\n"
              "void x(unknown_type_t *delay, const int *net) {\n"
              "    delay[4] = 0;\n"
              "}\n"
              "\n"
              "void y() {\n"
              "    struct s1 obj;\n"
              "    x(obj.delay, 123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11] -> [test.cpp:6]: (error) Array 'obj.delay[3]' accessed at index 4, which is out of bounds.\n", errout.str());

        check("struct s1 {\n"
              "    float a[0];\n"
              "};\n"
              "\n"
              "void f() {\n"
              "    struct s1 *obj;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_32() {
        check("class X\n"
              "{\n"
              "    public:\n"
              "    X()\n"
              "    {\n"
              "       m_x[0] = 0;\n"
              "       m_x[1] = 0;\n"
              "    }\n"
              "    int m_x[1];\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'm_x[1]' accessed at index 1, which is out of bounds.\n", errout.str());
    }

    void array_index_33() {
        check("void foo(char bar[][4]) {\n"
              "    baz(bar[5]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_34() { // ticket #3063
        check("void foo() {\n"
              "    int y[2][2][2];\n"
              "    y[0][2][0] = 0;\n"
              "    y[0][0][2] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'y[2][2][2]' index y[0][2][0] out of bounds.\n"
                      "[test.cpp:4]: (error) Array 'y[2][2][2]' index y[0][0][2] out of bounds.\n", errout.str());

        check("struct TEST\n"
              "{\n"
              "    char a[10];\n"
              "    char b[10][5];\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    TEST test;\n"
              "    test.a[10] = 3;\n"
              "    test.b[10][2] = 4;\n"
              "    test.b[0][19] = 4;\n"
              "    TEST *ptest;\n"
              "    ptest = &test;\n"
              "    ptest->a[10] = 3;\n"
              "    ptest->b[10][2] = 4;\n"
              "    ptest->b[0][19] = 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'test.a[10]' accessed at index 10, which is out of bounds.\n"
                      "[test.cpp:14]: (error) Array 'ptest.a[10]' accessed at index 10, which is out of bounds.\n"
                      "[test.cpp:10]: (error) Array 'test.b[10][5]' index test.b[10][2] out of bounds.\n"
                      "[test.cpp:11]: (error) Array 'test.b[10][5]' index test.b[0][19] out of bounds.\n"
                      "[test.cpp:15]: (error) Array 'ptest.b[10][5]' index ptest.b[10][2] out of bounds.\n"
                      "[test.cpp:16]: (error) Array 'ptest.b[10][5]' index ptest.b[0][19] out of bounds.\n", errout.str());

        check("struct TEST\n"
              "{\n"
              "    char a[10][5];\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    TEST test;\n"
              "    test.a[9][5] = 4;\n"
              "    test.a[0][50] = 4;\n"
              "    TEST *ptest;\n"
              "    ptest = &test;\n"
              "    ptest->a[9][5] = 4;\n"
              "    ptest->a[0][50] = 4;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Array 'test.a[10][5]' index test.a[9][5] out of bounds.\n"
                      "[test.cpp:9]: (error) Array 'test.a[10][5]' index test.a[0][50] out of bounds.\n"
                      "[test.cpp:12]: (error) Array 'ptest.a[10][5]' index ptest.a[9][5] out of bounds.\n"
                      "[test.cpp:13]: (error) Array 'ptest.a[10][5]' index ptest.a[0][50] out of bounds.\n", errout.str());
    }

    void array_index_35() { // ticket #2889
        check("void f() {\n"
              "    struct Struct { unsigned m_Var[1]; } s;\n"
              "    s.m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 's.m_Var[1]' accessed at index 1, which is out of bounds.\n", errout.str());

        check("struct Struct { unsigned m_Var[1]; };\n"
              "void f() {\n"
              "    struct Struct s;\n"
              "    s.m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's.m_Var[1]' accessed at index 1, which is out of bounds.\n", errout.str());

        check("struct Struct { unsigned m_Var[1]; };\n"
              "void f() {\n"
              "    struct Struct * s = calloc(40);\n"
              "    s->m_Var[1] = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_36() { // ticket #2960
        check("class Fred {\n"
              "    Fred(const Fred &);\n"
              "private:\n"
              "    bool m_b[2];\n"
              "};\n"
              "Fred::Fred(const Fred & rhs) {\n"
              "    m_b[2] = rhs.m_b[2];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'm_b[2]' accessed at index 2, which is out of bounds.\n"
                      "[test.cpp:7]: (error) Array 'rhs.m_b[2]' accessed at index 2, which is out of bounds.\n", errout.str());
    }

    void array_index_37() {
        check("class Fred {\n"
              "    char x[X];\n"
              "    Fred() {\n"
              "        for (unsigned int i = 0; i < 15; i++)\n"
              "            i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_38() { //ticket #3273
        check("void aFunction() {\n"
              "    double aDoubleArray[ 10 ];\n"
              "    unsigned int i; i = 0;\n"
              "    for( i = 0; i < 6; i++ )\n"
              "    {\n"
              "        unsigned int j; j = 0;\n"
              "        for( j = 0; j < 5; j++ )\n"
              "        {\n"
              "            unsigned int x; x = 0;\n"
              "            for( x = 0; x < 4; x++ )\n"
              "            {\n"
              "            }\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_39() { // ticket 3387
        check("void aFunction()\n"
              "{\n"
              "    char a[10];\n"
              "    a[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_40() {
        check("void f() {\n"
              "    char a[10];\n"
              "    for (int i = 0; i < 10; ++i)\n"
              "        f2(&a[i + 1]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_41() {
        // Don't generate false positives when structs have the same name
        check("void a() {\n"
              "    struct Fred { char data[6]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- no error
              "}\n"
              "\n"
              "void b() {\n"
              "    struct Fred { char data[3]; } fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void a() {\n"
              "    struct Fred { char data[6]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- no error
              "}\n"
              "\n"
              "void b() {\n"
              "    struct Fred { char data[3]; } fred;\n"
              "    fred.data[4] = 0;\n"  // <- error
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Array 'fred.data[3]' accessed at index 4, which is out of bounds.\n", errout.str());
    }

    void array_index_42() { // ticket #3569

        check("void f()\n"
              "{\n"
              "  char *p; p = malloc(10);\n"
              "  p[10] = 7;\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'p[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char *p; p = malloc(10);\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char *p; p = new char[10];\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char *p(new char[10]);\n"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char *p = NULL;"
              "  try{\n"
              "  p = new char[10];\n"
              "  }\n"
              "  catch(...){\n"
              "  return;\n"
              "  }"
              "  p[0] = 0;\n"
              "  p[9] = 9;\n"
              "  delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_43() { // #3838

        check("int f( )\n"
              "{\n"
              "  struct {\n"
              "    int arr[ 3 ];\n"
              "  } var[ 1 ];\n"
              "   int y;\n"
              "   var[ 0 ].arr[ 0 ] = 0;\n"
              "   var[ 0 ].arr[ 1 ] = 1;\n"
              "   var[ 0 ].arr[ 2 ] = 2;\n"
              "   y = var[ 0 ].arr[ 3 ];\n" // <-- array access out of bounds
              "   return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'var.arr[3]' accessed at index 3, which is out of bounds.\n"
                      "[test.cpp:10]: (error) Array 'var[0].arr[3]' accessed at index 3, which is out of bounds.\n", errout.str());

        check("int f( )\n"
              "{\n"
              "  struct {\n"
              "    int arr[ 3 ];\n"
              "  } var[ 1 ];\n"
              "   int y=1;\n"
              "   var[ 0 ].arr[ 0 ] = 0;\n"
              "   var[ 0 ].arr[ 1 ] = 1;\n"
              "   var[ 0 ].arr[ 2 ] = 2;\n"
              "   y = var[ 0 ].arr[ 2 ];\n"
              "   return y;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        check("int f( ){ \n"
              "struct Struct{\n"
              "    int arr[ 3 ];\n"
              "};\n"
              "int y;\n"
              "Struct var;\n"
              "var.arr[ 0 ] = 0;\n"
              "var.arr[ 1 ] = 1;\n"
              "var.arr[ 2 ] = 2;\n"
              "var.arr[ 3 ] = 3;\n" // <-- array access out of bounds
              "y=var.arr[ 3 ];\n"   // <-- array access out of bounds
              "return y;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Array 'var.arr[3]' accessed at index 3, which is out of bounds.\n"
                      "[test.cpp:11]: (error) Array 'var.arr[3]' accessed at index 3, which is out of bounds.\n", errout.str());


        check("void f( ) {\n"
              "struct S{\n"
              "    int var[ 3 ];\n"
              "} ;\n"
              "S var[2];\n"
              "var[0].var[ 0 ] = 0;\n"
              "var[0].var[ 1 ] = 1;\n"
              "var[0].var[ 2 ] = 2;\n"
              "var[0].var[ 4 ] = 4;\n" // <-- array access out of bounds
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'var.var[3]' accessed at index 4, which is out of bounds.\n"
                      "[test.cpp:9]: (error) Array 'var[0].var[3]' accessed at index 4, which is out of bounds.\n", errout.str());

        check("void f( ) {\n"
              "struct S{\n"
              "    int var[ 3 ];\n"
              "} ;\n"
              "S var[2];\n"
              "var[0].var[ 0 ] = 0;\n"
              "var[0].var[ 1 ] = 1;\n"
              "var[0].var[ 2 ] = 2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // avoid FPs (modified examples taken from #3838)
        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab;\n"
              "    int * p = &ab.a[10]; \n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab[1];\n"
              "    int * p = &ab[0].a[10]; \n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct AB { int a[10]; int b[10]; };\n"
              "int main() {\n"
              "    struct AB ab[1];\n"
              "    int * p = &ab[10].a[0]; \n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'ab[1]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_44() { // #3979 (false positive)

        check("void f()\n"
              "{\n"
              "    char buf[2];\n"
              "    int i;\n"
              "    for (i = 2; --i >= 0; )\n"
              "    {\n"
              "        buf[i] = 1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    double buf[2];\n"
              "    for (int i = 2; i--; )\n"
              "    {\n"
              "        buf[i] = 2.;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_45() { // #4207 - handling of function with variable number of parameters / unnamed arguments
        // Variable number of arguments
        check("void f(const char *format, ...) {\n"
              "    va_args args;\n"
              "    va_start(args, format);\n"
              "}\n"
              "void test() {\n"
              "    CHAR buffer[1024];\n"
              "    f(\"%s\", buffer);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Unnamed argument
        check("void f(char *) {\n"
              "    dostuff();\n"
              "}\n"
              "void test() {\n"
              "    char buffer[1024];\n"
              "    f(buffer);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Two statement for-loop
    void array_index_46() {
        // #4840
        check("void bufferAccessOutOfBounds2() {\n"
              "    char *buffer[]={\"a\",\"b\",\"c\"};\n"
              "    for(int i=3; i--;) {\n"
              "        printf(\"files(%i): %s\n\", 3-i, buffer[3-i]);\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Array 'buffer[3]' accessed at index 3, which is out of bounds.\n", "", errout.str());

        check("void f() {\n"
              "    int buffer[9];\n"
              "    long int i;\n"
              "    for(i=10; i--;) {\n"
              "        buffer[i] = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'buffer[9]' accessed at index 9, which is out of bounds.\n", errout.str());

        // Correct access limits -> i from 9 to 0
        check("void f() {\n"
              "    int buffer[10];\n"
              "    for(unsigned long int i=10; i--;) {\n"
              "        buffer[i] = i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_47() {
        // #5849
        check("int s[4];\n"
              "void f() {\n"
              "    for (int i = 2; i < 0; i++)\n"
              "        s[i] = 5; \n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_multidim() {
        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[2][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2]' index a[2][1] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2];\n"
              "  a[1][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2]' index a[1][2] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[2][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2][2]' index a[2][1][1] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][2][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2][2]' index a[1][2][1] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2][2];\n"
              "  a[1][2][1][1] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2][2][2]' index a[1][2][1][1] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[2][2][2];\n"
              "  a[1][1][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[2][2][2]' index a[1][1][2] out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "  char a[10][10][10];\n"
              "  a[2*3][4*3][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[10][10][10]' index a[6][12][2] out of bounds.\n", errout.str());

        check("void f() {\n"
              "  char a[10][10][10];\n"
              "  a[6][40][10] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'a[10][10][10]' index a[6][40][10] out of bounds.\n", errout.str());

        check("void f() {\n"
              "  char a[1][1][1];\n"
              "  a[2][2][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'a[1][1][1]' index a[2][2][2] out of bounds.\n", errout.str());

        check("void f() {\n"
              "  char a[6][6][6];\n"
              "  a[6][6][2] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'a[6][6][6]' index a[6][6][2] out of bounds.\n", errout.str());

        check("void f() {\n"
              "  int a[2][2];\n"
              "  p = &a[2][0];\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // unknown dim..
        check("void f()\n"
              "{\n"
              "  int a[2][countof(x)] = {{1,2},{3,4}};\n"
              "  a[0][0] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_switch_in_for() {
        check("void f()\n"
              "{\n"
              " int ar[10];\n"
              " for (int i = 0; i < 10; ++i)\n"
              " {\n"
              "  switch(i)\n"
              "  {\n"
              "   case 9:\n"
              "    ar[i] = 0;\n"
              "    break;\n"
              "   default:\n"
              "    ar[i] = ar[i+1];\n"
              "    break;\n"
              "  };\n"
              " }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              " int ar[10];\n"
              " for (int i = 0; i < 10; ++i)\n"
              " {\n"
              "  switch(i)\n"
              "  {\n"
              "   case 8:\n"
              "    ar[i] = 0;\n"
              "    break;\n"
              "   default:\n"
              "    ar[i] = ar[i+1];\n"
              "    break;\n"
              "  };\n"
              " }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:12]: (error) Array index out of bounds.\n", "", errout.str());
    }

    void array_index_for_in_for() {
        check("void f() {\n"
              "    int a[5];\n"
              "    for (int i = 0; i < 10; ++i) {\n"
              "        for (int j = i; j < 5; ++j) {\n"
              "            a[i] = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_calculation() {
        // #1193 - false negative: array out of bounds in loop when there is calculation
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    for (int i = 19; i < 36; ++i) {\n"
              "        data[i/2] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[8]' accessed at index 17, which is out of bounds.\n", errout.str());

        // #2199 - false negative: array out of bounds in loop when there is calculation
        check("void f()\n"
              "{\n"
              "    char arr[5];\n"
              "    for (int i = 0; i < 5; ++i) {\n"
              "        arr[i + 7] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'arr[5]' accessed at index 11, which is out of bounds.\n", errout.str());
    }

    void array_index_negative1() {
        // #948 - array index out of bound not detected 'a[-1] = 0'
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    data[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array index -1 is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char data[8][4];\n"
              "    data[5][-1] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array index -1 is out of bounds.\n", errout.str());

        // #1614 - negative index is ok for pointers
        check("void foo(char *p)\n"
              "{\n"
              "    p[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char s[] = \"abc\";\n"
              "    char *p = s + strlen(s);\n"
              "    if (p[-1]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #1850
        check("int f(const std::map<int, std::map<int,int> > &m)\n"
              "{\n"
              "    return m[0][-1];\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_negative2() { // ticket #3063
        check("struct TEST { char a[10]; };\n"
              "void foo() {\n"
              "    TEST test;\n"
              "    test.a[-1] = 3;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'test.a[10]' accessed at index -1, which is out of bounds.\n", errout.str());
    }

    void array_index_for_decr() {
        check("void f()\n"
              "{\n"
              "    char data[8];\n"
              "    for (int i = 10; i > 0; --i) {\n"
              "        data[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[8]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char val[5];\n"
              "    for (unsigned int i = 3; i < 5; --i) {\n"
              "        val[i+1] = val[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char val[5];\n"
              "    for (int i = 3; i < 5; --i) {\n"
              "        val[i+1] = val[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array index -9994 is out of bounds.\n"
                      "[test.cpp:5]: (error) Array index -9995 is out of bounds.\n", errout.str());
    }


    void array_index_varnames() {
        check("struct A {\n"
              "    char data[4];\n"
              "    struct B { char data[3]; };\n"
              "    B b;\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    A a;\n"
              "    a.data[3] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_for_andand_oror() {  // #3907 - using && or ||
        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x < 10 && y; x++) {\n"
              "        data[x] = 0;\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[2]' accessed at index 9, which is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x < 10 || y; x++) {\n"
              "        data[x] = 0;\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[2]' accessed at index 9, which is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; x <= 10 && y; x++) {\n"
              "        data[x] = 0;\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[2]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char data[2];\n"
              "    int x;\n"
              "    for (x = 0; y && x <= 10; x++) {\n"
              "        data[x] = 0;\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'data[2]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_for_continue() {
        // #3913
        check("void f() {\n"
              "    int a[2];\n"
              "    for (int i = 0; i < 2; ++i) {\n"
              "        if (i == 0) {\n"
              "            continue;\n"
              "        }\n"
              "        a[i - 1] = 0;\n"
              "    }\n"
              "}", true, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a[2];\n"
              "    for (int i = 0; i < 2; ++i) {\n"
              "        if (somecondition) {\n"
              "            continue;\n"
              "        }\n"
              "        a[i - 1] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array index -1 is out of bounds.\n", errout.str());
    }

    void array_index_for() {
        // Ticket #2370 - No false negative when there is no "break"
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 20; ++i) {\n"
              "        if (i==1) {\n"
              "        }\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Array 'a[10]' accessed at index 19, which is out of bounds.\n", errout.str());

        // Ticket #2385 - No false positive
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i < 20; ++i) {\n"
              "        if (i<10) {\n"
              "        } else {\n"
              "            a[i-10] = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #3893 - start value out of bounds
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 10; somecondition; dosomething) {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void array_index_for_neq() {
        // Ticket #2211 - for loop using != in the condition
        check("void f() {\n"
              "    int a[5];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[5]' accessed at index 9, which is out of bounds.\n",
                      errout.str());
    }

    void array_index_for_question() {
        // Ticket #2561 - using ?: inside for loop
        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        i == 0 ? 0 : a[i-1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        some_condition ? 0 : a[i-1];\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Array index -1 is out of bounds.\n", "", errout.str());

        check("void f() {\n"
              "    int a[10];\n"
              "    for (int i = 0; i != 10; ++i) {\n"
              "        i==0 ? 0 : a[i-1];\n"
              "        a[i-1] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array index -1 is out of bounds.\n", errout.str());
    }

    void array_index_for_varid0() { // #4228: No varid for counter variable
        check("void f() {\n"
              "   char a[10];\n"
              "   for (i=0; i<10; i++);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_vla_for() {
        // #3221 - access VLA inside for
        check("void f(int len) {\n"
              "    char a[len];\n"
              "    for (int i=0; i<7; ++i) {\n"
              "        a[0] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_extern() {
        // Ticket #1684. FP when using 'extern'.
        check("extern char arr[15];\n"
              "char arr[15] = \"abc\";");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_cast() {
        // Ticket #2841. FP when using cast.

        // Different types => no error
        check("void f1(char *buf) {\n"
              "    buf[4] = 0;\n"
              "}\n"
              "void f2() {\n"
              "    int x[2];\n"
              "    f1(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Same type => error
        check("void f1(const char buf[]) {\n"
              "    char c = buf[4];\n"
              "}\n"
              "void f2() {\n"
              "    char x[2];\n"
              "    f1(x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:2]: (error) Array 'x[2]' accessed at index 4, which is out of bounds.\n", errout.str());
    }

    void array_index_string_literal() {
        check("void f() {\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[10]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'str[4]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[4]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'str[4]' accessed at index 4, which is out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"abc\";\n"
              "    bar(str[3]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    const char *str = \"a\tc\";\n"
              "    bar(str[4]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'str[4]' accessed at index 4, which is out of bounds.\n", errout.str());

    }

    void array_index_same_struct_and_var_name() {
        // don't throw internal error
        check("struct tt {\n"
              "    char name[21];\n"
              "} ;\n"
              "void doswitch(struct tt *x)\n"
              "{\n"
              "    struct tt *tt=x;\n"
              "    tt->name;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // detect error
        check("struct tt {\n"
              "    char name[21];\n"
              "} ;\n"
              "void doswitch(struct tt *x)\n"
              "{\n"
              "    struct tt *tt=x;\n"
              "    tt->name[22] = 123;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'tt.name[21]' accessed at index 22, which is out of bounds.\n", errout.str());
    }

    void array_index_valueflow() {
        check("void f(int i) {\n"
              "    char str[3];\n"
              "    str[i] = 0;\n"
              "    if (i==10) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:4]: (warning) Array 'str[3]' accessed at index 10, which is out of bounds. Otherwise condition 'i==10' is redundant.\n", errout.str());

        check("void f() {\n"
              "    char str[3];\n"
              "    str[((unsigned char)3) - 1] = 0;\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"  // #5416 FP
              "    char *str[3];\n"
              "    do_something(&str[0][5]);\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());

        check("class X { static const int x[100]; };\n" // #6070
              "const int X::x[100] = {0};", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());

        check("namespace { class X { static const int x[100]; };\n" // #6232
              "const int X::x[100] = {0}; }", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_function_parameter() {
        check("void f(char a[10]) {\n"
              "  a[20] = 0;\n" // <- cppcheck warn here even though it's not a definite access out of bounds
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Array 'a[10]' accessed at index 20, which is out of bounds.\n", errout.str());

        check("void f(char a[10]) {\n" // #6353 - reassign 'a'
              "  a += 4;\n"
              "  a[-1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_1_posix_functions() {
        checkposix("void f(int fd)\n"
                   "{\n"
                   "    char str[3];\n"
                   "    read(fd, str, 3);\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f(int fd)\n"
                   "{\n"
                   "    char str[3];\n"
                   "    read(fd, str, 4);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkposix("void f(int fd)\n"
                   "{\n"
                   "    char str[3];\n"
                   "    write(fd, str, 3);\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f(int fd)\n"
                   "{\n"
                   "    char str[3];\n"
                   "    write(fd, str, 4);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkposix("void f()\n"
                   "{\n"
                   "    long bb[2];\n"
                   "    write(stdin, bb, sizeof(bb));\n"
                   "}");
        ASSERT_EQUALS("", errout.str());

        checkposix("void f()\n"
                   "{\n"
                   "char str[3];\n"
                   "recv(s, str, 4, 0);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkposix("void f()\n"
                   "{\n"
                   "char str[3];\n"
                   "recvfrom(s, str, 4, 0, 0x0, 0x0);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkposix("void f()\n"
                   "{\n"
                   "char str[3];\n"
                   "send(s, str, 4, 0);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkposix("void f()\n"
                   "{\n"
                   "char str[3];\n"
                   "sendto(s, str, 4, 0, 0x0, 0x0);\n"
                   "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());
    }

    void buffer_overrun_1_standard_functions() {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    strcpy(str, \"abc\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkstd("void f() {\n"
                 "    char str[3];\n"
                 "    fgets(str, 3, stdin);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f() {\n"
                 "    char str[3];\n"
                 "    fgets(str, 4, stdin);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        // fread
        checkstd("void f(FILE* fd) {\n"
                 "  char str[3];\n"
                 "  fread(str,1,4,fd);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkstd("void f(FILE* fd) {\n"
                 "  char str[3];\n"
                 "  fread(str,1,3,fd);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        // fwrite
        checkstd("void f(FILE* fd) {\n"
                 "  char str[3];\n"
                 "  fwrite(str,1,4,fd);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkstd("void f(FILE* fd) {\n"
                 "  char str[3];\n"
                 "  fwrite(str,1,3,fd);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        // #4968 - not standard function
        checkstd("void f() {\n"
                 "    char str[3];\n"
                 "    foo.memset(str, 0, 100);\n"
                 "    foo::memset(str, 0, 100);\n"
                 "    std::memset(str, 0, 100);\n"
                 "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds: str\n", "", errout.str());

        // #5257 - check strings
        checkstd("void f() {\n"
                 "  memcpy(temp, \"hello world\", 20);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void f() {\n"
                 "  memcpy(temp, \"abc\", 4);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }


    void buffer_overrun_2_struct() {
        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(struct ABC *abc)\n"
              "{\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Buffer is accessed out of bounds: abc.str\n", errout.str());

        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    struct ABC abc;\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Buffer is accessed out of bounds: abc.str\n", errout.str());

        check("struct ABC\n"
              "{\n"
              "    char str[5];\n"
              "};\n"
              "\n"
              "static void f(struct ABC &abc)\n"
              "{\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Buffer is accessed out of bounds: abc.str\n", errout.str());

        check("static void f()\n"
              "{\n"
              "    struct ABC\n"
              "    {\n"
              "        char str[5];\n"
              "    } abc;\n"
              "    strcpy( abc.str, \"abcdef\" );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Buffer is accessed out of bounds: abc.str\n", errout.str());

        check("static void f()\n"
              "{\n"
              "    struct ABC\n"
              "    {\n"
              "        char str[5];\n"
              "    };\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    strcpy( abc->str, \"abcdef\" );\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Buffer is accessed out of bounds: abc.str\n", errout.str());
    }


    void buffer_overrun_3() {
        check("int a[10];\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    int i;\n"
              "    for (i = 0; i <= 10; ++i)\n"
              "        a[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'a[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }


    void buffer_overrun_4() {
        check("void foo()\n"
              "{\n"
              "    const char *p[2];\n"
              "    for (int i = 0; i < 8; ++i)\n"
              "        p[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'p[2]' accessed at index 7, which is out of bounds.\n", errout.str());

        // No false positive
        check("void foo(int x, int y)\n"
              "{\n"
              "    const char *p[2];\n"
              "    const char *s = y + p[1];\n"
              "    p[1] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // There is no error here
        check("void f1(char *s,int size)\n"
              "{\n"
              "  if( size > 10 ) strcpy(s,\"abc\");\n"
              "}\n"
              "void f2()\n"
              "{\n"
              "  char s[3];\n"
              "  f1(s,20);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (error) Buffer is accessed out of bounds.\n", "", errout.str());

        check("void f1(char *s,int size)\n"
              "{\n"
              "  if( size > 10 ) strcpy(s,\"abc\");\n"
              "}\n"
              "void f2()\n"
              "{\n"
              "  char s[3];\n"
              "  f1(s,3);\n"
              "}\n", false);
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_5() {
        check("void f()\n"
              "{\n"
              "    char n[5];\n"
              "    sprintf(n, \"d\");\n"
              "    printf(\"hello!\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_6() {
        check("void f()\n"
              "{\n"
              "   char n[5];\n"
              "   strcat(n, \"abc\");\n"
              "   strcat(n, \"def\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds: n\n", errout.str());
    }

    void buffer_overrun_7() {
        // ticket #731
        check("void f()\n"
              "{\n"
              "    char a[2];\n"
              "    strcpy(a, \"a\\0\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_8() {
        // ticket #714
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = i + 100)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; i = 100 + i)\n"
              "    {\n"
              "        a[i] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_9() {
        // ticket #738
        check("void f()\n"
              "{\n"
              "    char a[5];\n"
              "    for (int i = 0; i < 20; )\n"
              "    {\n"
              "        a[i] = 0;\n"
              "        i += 100;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_10() {
        // ticket #740
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (int i = 0; i < 4; i++)\n"
              "    {\n"
              "        char b = a[i];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_11() {
        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=i+0.1)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char a[4];\n"
              "    for (float i=0; i<10.0;i=0.1+i)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_12() {
        // ticket #900
        check("void f() {\n"
              "  char *a = new char(30);\n"
              "  sprintf(a, \"%s\", \"b\");\n"
              "  delete a;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void buffer_overrun_13() {
        // ticket #836
        checkstd("void f() {\n"
                 "  char a[10];\n"
                 "  memset(a+5, 0, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: a\n", errout.str());

        checkstd("void f() {\n"
                 "  char a[10];\n"
                 "  memmove(a, a+5, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: a\n", errout.str());
    }

    void buffer_overrun_14() {
        checkstd("void f(char *a) {\n"
                 "  char *b = new char[strlen(a)];\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = new char[strlen(a) + 1];\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = new char[strlen(a)];\n"
                 "  a[0] = '\\0';\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = malloc(strlen(a));\n"
                 "  b = realloc(b, 10000);\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = malloc(strlen(a));\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = malloc(strlen(a));\n"
                 "  {\n"
                 "    strcpy(b, a);\n"
                 "  }\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = malloc(strlen(a) + 1);\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f(char *a, char *c) {\n"
                 "  char *b = realloc(c, strlen(a));\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void f(char *a, char *c) {\n"
                 "  char *b = realloc(c, strlen(a) + 1);\n"
                 "  strcpy(b, a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f(char *a) {\n"
                 "  char *b = malloc(strlen(a));\n"
                 "  sprintf(b, \"%s\", a);\n"
                 "  return b;\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void buffer_overrun_15() { // ticket #1787
        check("class A : public B {\n"
              "    char val[12];\n"
              "    void f(int i, int ii);\n"
              "};\n"
              "void A::f(int i, int ii)\n"
              "{\n"
              "    sprintf(val, \"drive_%d_partition_%d_size\", i, ii) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void buffer_overrun_16() {
        // unknown types
        check("void f() {\n"
              "    struct Foo foo[5];\n"
              "    memset(foo, 0, sizeof(foo));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"  // ticket #2093
              "    gchar x[3];\n"
              "    strcpy(x, \"12\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("extern char a[10];\n"
              "void f() {\n"
              "    char b[25] = {0};\n"
              "    std::memcpy(b, a, 10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_17() { // ticket #2548
        check("void f() {\n"
              "    char t[8];\n"
              "    sprintf(t, \"%s\", \"foo     bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void buffer_overrun_18() { // ticket #2576
        check("class A {\n"
              "    void foo();\n"
              "    bool b[7];\n"
              "};\n"
              "\n"
              "void A::foo() {\n"
              "    for (int i=0; i<6; i++) {\n"
              "        b[i] = b[i+1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"
              "    void foo();\n"
              "    bool b[7];\n"
              "};\n"
              "\n"
              "void A::foo() {\n"
              "    for (int i=0; i<7; i++) {\n"
              "        b[i] = b[i+1];\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Array 'b[7]' accessed at index 7, which is out of bounds.\n", errout.str());
    }

    void buffer_overrun_19() { // #2597 - class member with unknown type
        check("class A {\n"
              "public:\n"
              "    u8 buf[10];\n"
              "    A();"
              "};\n"
              "\n"
              "A::A() {\n"
              "    memset(buf, 0, 10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_20() { // #2986(segmentation fault)
        check("x[y]\n");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_21() {
        check("void foo()\n"
              "{ { {\n"
              "    char dst[4];\n"
              "    const char *src = \"AAAAAAAAAAAAAAAAAAAAA\";\n"
              "    for (size_t i = 0; i <= 4; i++)\n"
              "        dst[i] = src[i];\n"
              "} } }\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Array 'dst[4]' accessed at index 4, which is out of bounds.\n", errout.str());
    }

    void buffer_overrun_22() { // ticket #3124
        checkstd("class A {\n"
                 "public:\n"
                 "    char b[5][6];\n"
                 "};\n"
                 "int main() {\n"
                 "    A a;\n"
                 "    memset(a.b, 0, 5 * 6);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("class A {\n"
                 "public:\n"
                 "    char b[5][6];\n"
                 "};\n"
                 "int main() {\n"
                 "    A a;\n"
                 "    memset(a.b, 0, 6 * 6);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Buffer is accessed out of bounds: a.b\n", errout.str());
    }

    void buffer_overrun_23() { // ticket #3153
        checkstd("void foo() {\n"
                 "    double dest = 23.0;\n"
                 "    char* const source = (char*) malloc(sizeof(dest));\n"
                 "    memcpy(&dest, source + sizeof(double), sizeof(dest));\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());

        checkstd("void foo() {\n"
                 "    double dest = 23.0;\n"
                 "    char* const source = (char*) malloc(2 * sizeof(dest));\n"
                 "    memcpy(&dest, source + sizeof(double), sizeof(dest));\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }


    void buffer_overrun_24() { // index variable is changed in for-loop
        // ticket #4106
        check("void main() {\n"
              "   int array[] = {1,2};\n"
              "   int x = 0;\n"
              "   for( int i = 0; i<6; ) {\n"
              "      x += array[i];\n"
              "       i++;  }\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        // ticket #4096
        check("void main() {\n"
              "   int array[] = {1,2};\n"
              "   int x = 0;\n"
              "   for( int i = 0; i<6; ) {\n"
              "      x += array[i++];\n"
              "   }\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());
    }

    void buffer_overrun_26() { // ticket #4432 (segmentation fault)
        check("extern int split();\n"
              "void regress() {\n"
              "    char inbuf[1000];\n"
              "    char *f[10];\n"
              "    split(inbuf, f, 10, \"\t\t\");\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_27() { // ticket #4444 (segmentation fault)
        check("void abc(struct foobar[5]);\n"
              "void main() {\n"
              "struct foobar x[5];\n"
              "abc(x);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_28() {
        check("char c = \"abc\"[4];");
        ASSERT_EQUALS("[test.cpp:1]: (error) Buffer is accessed out of bounds: \"abc\"\n", errout.str());
    }

    void buffer_overrun_bailoutIfSwitch() {
        // No false positive
        check("void f1(char *s) {\n"
              "    if (x) s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        ASSERT_EQUALS("", errout.str());

        // No false positive
        check("void f1(char *s) {\n"
              "    if (x) return;\n"
              "    s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        ASSERT_EQUALS("", errout.str());

        // No false negative
        check("void f1(char *s) {\n"
              "    if (x) { }\n"
              "    s[100] = 0;\n"
              "}\n"
              "\n"
              "void f2() {\n"
              "    char a[10];\n"
              "    f1(a);"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:3]: (error) Array 'a[10]' accessed at index 100, which is out of bounds.\n", errout.str());
    }

    void buffer_overrun_function_array_argument() {
        check("void f(char a[10]);\n"
              "void g() {\n"
              "    char a[2];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) The array 'a' is too small, the function 'f' expects a bigger one.\n", errout.str());

        check("void f(float a[10][20]);\n"
              "void g() {\n"
              "    float a[2][3];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) The array 'a' is too small, the function 'f' expects a bigger one.\n", errout.str());

        check("void f(char a[20]);\n"
              "void g() {\n"
              "    int a[2];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) The array 'a' is too small, the function 'f' expects a bigger one.\n", errout.str());

        check("void f(char a[20]);\n"
              "void g() {\n"
              "    int a[5];\n"
              "    f(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int a[]) {\n"
              "  switch (2) {\n"
              "    case 1:\n"
              "      a[1] = 1;\n"
              "    }\n"
              "}\n"
              "int a[1];\n"
              "f(a);\n"
              "");
        ASSERT_EQUALS("", errout.str());
    }

    void possible_buffer_overrun_1() { // #3035
        check("void foo() {\n"
              "    char * data = alloca(50);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible buffer overflow if strlen(src) is larger than sizeof(data)-strlen(data).\n", errout.str());

        check("void foo() {\n"
              "    char * data = alloca(100);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char src[100]) {\n"
              "    char * data = alloca(50);\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Possible buffer overflow if strlen(src) is larger than sizeof(data)-strlen(data).\n", errout.str());

        check("void foo(char src[100]) {\n"
              "    char * data = alloca(100);\n"
              "    strcat(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    char * data = alloca(50);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible buffer overflow if strlen(src) is larger than or equal to sizeof(data).\n", errout.str());

        check("void foo() {\n"
              "    char * data = alloca(100);\n"
              "    char src[100];\n"
              "    memset(src, 'C', 99);\n"
              "    src[99] = '\\0';\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char src[100]) {\n"
              "    char * data = alloca(50);\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Possible buffer overflow if strlen(src) is larger than or equal to sizeof(data).\n", errout.str());

        check("void foo(char src[100]) {\n"
              "    char * data = alloca(100);\n"
              "    strcpy(data, src);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void buffer_overrun_readSizeFromCfg() {
        // Attempt to get size from Cfg files, no false positives if size is not specified
        checkstd("void f() {\n"
                 "  uint8_t str[256];\n"
                 "  str[0] = 0;\n"
                 "  strcat(str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f() {\n"
                 "  uint8_t str[2];\n"
                 "  str[0] = 0;\n"
                 "  strcat(str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkstd("void f() {\n"
                 "  int_fast8_t str[256];\n"
                 "  str[0] = 0;\n"
                 "  strcat(str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        // The same for structs, where the message comes from a different check
        checkstd("typedef struct mystruct_s {\n"
                 "    uint8_t str[256];\n"
                 "} mystruct_t;\n"
                 "void f() {\n"
                 "    mystruct_t ms;\n"
                 "    ms.str[0] = 0;\n"
                 "    strcat((char*)ms.str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("typedef struct mystruct_s {\n"
                 "    uint8_t str[2];\n"
                 "} mystruct_t;\n"
                 "void f() {\n"
                 "    mystruct_t ms;\n"
                 "    ms.str[0] = 0;\n"
                 "    strcat((char*)ms.str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Buffer is accessed out of bounds: ms.str\n", errout.str());

        checkstd("typedef struct mystruct_s {\n"
                 "    int_fast8_t str[256];\n"
                 "} mystruct_t;\n"
                 "void f() {\n"
                 "    mystruct_t ms;\n"
                 "    ms.str[0] = 0;\n"
                 "    strcat((char*)ms.str, \"toto\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void valueflow_string() { // using ValueFlow string values in checking
        checkstd("void f() {\n"
                 "  char buf[3];\n"
                 "  const char *x = s;\n"
                 "  if (cond) x = \"abcde\";\n"
                 "  strcpy(buf,x);\n" // <- buffer overflow when x is "abcde"
                 "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds: buf\n", errout.str());

        checkstd("void f() {\n"
                 "  const char *x = s;\n"
                 "  if (cond) x = \"abcde\";\n"
                 "  memcpy(buf,x,20);\n" // <- buffer overflow when x is "abcde"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());

        check("char f() {\n"
              "  const char *x = s;\n"
              "  if (cond) x = \"abcde\";\n"
              "  return x[20];\n" // <- array index out of bounds when x is "abcde"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'x[6]' accessed at index 20, which is out of bounds.\n", errout.str());
    }

    void pointer_out_of_bounds_1() {
        check("void f() {\n"
              "    char a[10];\n"
              "    char *p = a + 100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Undefined behaviour, pointer arithmetic 'a+100' is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char a[10];\n"
              "    return a + 100;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Undefined behaviour, pointer arithmetic 'a+100' is out of bounds.\n", errout.str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == 123) {}\n"
              "    dostuff(x+i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) Undefined behaviour, when 'i' is 123 the pointer arithmetic 'x+i' is out of bounds.\n", errout.str());

        check("void f() {\n" // #6350 - fp when there is cast of buffer
              "  wchar_t buf[64];\n"
              "  p = (unsigned char *) buf + sizeof (buf);\n"
              "}", false, "6350.c", false);
        ASSERT_EQUALS("", errout.str());
    }

    void pointer_out_of_bounds_2() {
        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 100;\n"
              "    free(p);"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Undefined behaviour, pointer arithmetic 'p+100' is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) p is out of bounds.\n", errout.str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    p -= 10;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char *p = malloc(10);\n"
              "    p += 10;\n"
              "    p = p - 1;\n"
              "    *p = 0;\n"
              "    free(p);"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pointer_out_of_bounds_sub() {
        check("void f() {\n"
              "    char x[10];\n"
              "    return x-1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Undefined behaviour, pointer arithmetic 'x-1' is out of bounds.\n", errout.str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == 123) {}\n"
              "    dostuff(x-i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) Undefined behaviour, when 'i' is 123 the pointer arithmetic 'x-i' is out of bounds.\n", errout.str());

        check("void f(int i) {\n"
              "    char x[10];\n"
              "    if (i == -20) {}\n"
              "    dostuff(x-i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) Undefined behaviour, when 'i' is -20 the pointer arithmetic 'x-i' is out of bounds.\n", errout.str());
    }

    void sprintf1() {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    sprintf(str, \"%s\", \"abc\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char * c = new char[10];\n"
              "    sprintf(c, \"%s\", \"/usr/LongLongLongLongUserName/bin/LongLongApplicationName\");\n"
              "    delete [] c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void sprintf2() {
        check("int getnumber();\n"
              "void f()\n"
              "{\n"
              "    char str[5];\n"
              "    sprintf(str, \"%d: %s\", getnumber(), \"abcde\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void sprintf3() {
        check("void f()\n"
              "{\n"
              "    char str[3];\n"
              "    sprintf(str, \"test\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    sprintf(str, \"test%s\", \"\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf4() {
        // ticket #690
        check("void f()\n"
              "{\n"
              "    char a[3];\n"
              "    sprintf(a, \"%02ld\", 99);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf5() {
        // ticket #729
        check("void f(int condition)\n"
              "{\n"
              "    char buf[3];\n"
              "    sprintf(buf, \"%s\", condition ? \"11\" : \"22\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf6() {
        check("void f(int condition)\n"
              "{\n"
              "    char buf[3];\n"
              "    sprintf(buf, \"%s\", condition ? \"11\" : \"222\");\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds.\n", "", errout.str());
    }

    void sprintf7() {
        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo x;\n"
              "  sprintf(x.a, \"aa\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds.\n", errout.str());

        // This is out of bounds if 'sizeof(ABC)' is 1 (No padding)
        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo *x = malloc(sizeof(Foo));\n"
              "  sprintf(x.a, \"aa\");\n"
              "  free(x);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo *x = malloc(sizeof(Foo) + 10);\n"
              "  sprintf(x.a, \"aa\");\n"
              "  free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf8() {
        check("struct Foo { char a[3]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo x;\n"
              "  sprintf(x.a, \"aa\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf9() {
        check("void f()\n"
              "{\n"
              "  gchar str[3];\n"
              "  sprintf(str, \"1\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf10() {
        check("void f()\n"
              "{\n"
              "  TString str = \"\";\n"
              "  sprintf(str, \"1\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf1() {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 10, \"%s\", \"abc\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) snprintf size is out of bounds: Supplied size 10 is larger than actual size 5.\n", errout.str());
    }

    void snprintf2() {
        check("void f()\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 5, \"%s\", \"abc\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf4() {
        check("void f(int x)\n"
              "{\n"
              "    char str[5];\n"
              "    snprintf(str, 8 - x, \"abcdefghijkl\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf5() {
        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo x;\n"
              "  snprintf(x.a, 2, \"aa\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) snprintf size is out of bounds: Supplied size 2 is larger than actual size 1.\n", errout.str());

        // This is out of bounds if 'sizeof(ABC)' is 1 (No padding)
        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo *x = malloc(sizeof(Foo));\n"
              "  snprintf(x.a, 2, \"aa\");\n"
              "  free(x);\n"
              "}");
        TODO_ASSERT_EQUALS("error", "", errout.str());

        check("struct Foo { char a[1]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo *x = malloc(sizeof(Foo) + 10);\n"
              "  snprintf(x.a, 2, \"aa\");\n"
              "  free(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf6() {
        check("struct Foo { char a[3]; };\n"
              "void f()\n"
              "{\n"
              "  struct Foo x;\n"
              "  snprintf(x.a, 2, \"aa\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void snprintf7() {
        check("void x() {\n"
              "    sal_Char pString[1024];\n"
              "    snprintf(pString, 1024, \"ab\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6141 FP: Unknown type is assumed to have size 0
        check("typedef struct {\n"
              "    CHAR s[42];\n"
              "} sct_t;\n"
              "void foo() {\n"
              "    sct_t p;\n"
              "    snprintf(p.s, 42, \"abcdef\");\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

    }

    void strncat1() {
        checkstd("void f(char *a, char *b) {\n"
                 "    char str[16];\n"
                 "    strncpy(str, a, 10);\n"
                 "    strncat(str, b, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Dangerous usage of strncat - 3rd parameter is the maximum number of characters to append.\n", errout.str());
    }

    void strncat2() {
        checkstd("void f(char *a) {\n"
                 "    char str[5];\n"
                 "    strncat(str, a, 5);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Dangerous usage of strncat - 3rd parameter is the maximum number of characters to append.\n", errout.str());
    }

    void strncat3() {
        checkstd("struct Foo { char a[4]; };\n"
                 "void f(char *a) {\n"
                 "  struct Foo x;\n"
                 "  strncat(x.a, a, 5);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: x.a\n", errout.str());
    }

    void strncat4() {
        checkstd("void f(char *a) {\n"
                 "    char str[5];\n"
                 "    strncat(str, \"foobar\", 5);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Dangerous usage of strncat - 3rd parameter is the maximum number of characters to append.\n", errout.str());
    }


    void strcat1() {
        checkstd("struct Foo { char a[4]; };\n"
                 "void f() {\n"
                 "  struct Foo x;\n"
                 "  strcat(x.a, \"aa\");\n"
                 "  strcat(x.a, \"aa\");\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void strcat2() {
        checkstd("struct Foo { char a[5]; };\n"
                 "void f() {\n"
                 "  struct Foo x;\n"
                 "  strcat(x.a, \"aa\");\n"
                 "  strcat(x.a, \"aa\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void strcat3() {
        checkstd("void f() {\n"
                 "  INT str[10];\n"
                 "  strcat(str, \"aa\");\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }


    // memchr/memset/memcpy/etc
    void memfunc1() {
        checkstd("struct S {\n"
                 "    char a[5];\n"
                 "};\n"
                 "void f() {\n"
                 "    S s;\n"
                 "    memset(s.a, 0, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Buffer is accessed out of bounds: s.a\n", errout.str());

        checkstd("void f() {\n"
                 "    char str[5];\n"
                 "    memset(str, 0, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: str\n", errout.str());

        checkstd("void f() {\n"
                 "    char a[5], b[50];\n"
                 "    memcpy(a, b, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: a\n", errout.str());

        checkstd("void f() {\n"
                 "    char a[5], b[50];\n"
                 "    memmove(a, b, 10);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: a\n", errout.str());

        // Ticket #909
        checkstd("void f() {\n"
                 "    char * pch;\n"
                 "    char str[] = \"Example string\";\n"
                 "    pch = memchr (str, 'p', 16);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str\n", errout.str());
    }

    // ticket #2121 - buffer access out of bounds when using uint32_t
    void memfunc2() {
        checkstd("void f() {\n"
                 "    unknown_type_t buf[4];\n"
                 "    memset(buf, 0, 100);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    // ticket #1659 - overflowing variable when using memcpy
    void memfunc3() {
        checkstd("void f() { \n"
                 "  char str1[]=\"Sample string\";\n"
                 "  char str2;\n"
                 "  memcpy (&str2,str1,13);\n" // <-- strlen(str1)+1 = 13
                 "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str1\n","", errout.str());

        checkstd("void f() {\n"
                 "    char a[10];\n"
                 "    char str1[] = \"abcdef\";\n"
                 "    memset(a, 0, 11);\n" // <-- strlen(str1) + 5 = 11
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: a\n", errout.str());

        checkstd("void f() { \n"
                 "char str1[]=\"Sample string\";\n"
                 "char str2;\n"
                 "memcpy (&str2,str1,15);\n" // <-- strlen(str1) + 1 = 15
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer is accessed out of bounds: str1\n", errout.str());

        checkstd("void f() { \n"
                 "  char str[5];\n"
                 "  memcpy (str, \"\\0\\0\\0\\0\\0\", 5);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varid1() {
        check("void foo()\n"
              "{\n"
              "    char str[10];\n"
              "    if (str[0])\n"
              "    {\n"
              "        char str[50];\n"
              "        str[30] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void varid2() {
        checkstd("void foo()\n"
                 "{\n"
                 "    char str[10];\n"
                 "    if (str[0])\n"
                 "    {\n"
                 "        char str[50];\n"
                 "        memset(str,0,50);\n"
                 "    }\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varid3() { // #4764
        check("struct foo {\n"
              "  void bar() { return; }\n"
              "  type<> member[1];\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void assign1() {
        check("char str[3] = {'a', 'b', 'c'};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    str[3] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'str[3]' accessed at index 3, which is out of bounds.\n", errout.str());
    }

    void alloc1() {
        check("void foo()\n"
              "{\n"
              "    char *s; s = new char[10];\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        // ticket #1670 - false negative when using return
        check("char f()\n"
              "{\n"
              "    char *s; s = new int[10];\n"
              "    return s[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("struct Fred { char c[10]; };\n"
              "char f()\n"
              "{\n"
              "    Fred *f; f = new Fred;\n"
              "    return f->c[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Array 'f.c[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("static const size_t MAX_SIZE = UNAVAILABLE_TO_CPPCHECK;\n"
              "struct Thing { char data[MAX_SIZE]; };\n"
              "char f4(const Thing& t) { return !t.data[0]; }");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "char * buf; buf = new char[8];\n"
              "buf[7] = 0;\n"
              "delete [] buf;\n"
              "buf = new char[9];\n"
              "buf[8] = 0;\n"
              "delete [] buf;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "char * buf; buf = new char[8];\n"
              "buf[7] = 0;\n"
              "delete [] buf;\n"
              "buf = new char[9];\n"
              "buf[9] = 0;\n"
              "delete [] buf;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'buf[9]' accessed at index 9, which is out of bounds.\n", errout.str());

        check("void f() {\n"
              "  int *tab4; tab4 = malloc(20 * sizeof(int));\n"
              "  tab4[19] = 0;\n"
              "  free(tab4);\n"
              "  tab4 = malloc(21 * sizeof(int));\n"
              "  tab4[20] = 0;\n"
              "  free(tab4);\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  int *tab4 = malloc(20 * sizeof(int));\n"
              "  tab4[19] = 0;\n"
              "  tab4 = realloc(tab4,21 * sizeof(int));\n"
              "  tab4[20] = 0;\n"
              "  free(tab4);\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("", errout.str());
    }

    // data is allocated with malloc
    void alloc2() {
        check("void foo()\n"
              "{\n"
              "    char *s; s = malloc(10);\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[10]' accessed at index 10, which is out of bounds.\n", errout.str());

        // ticket #842
        check("void f() {\n"
              "    int *tab4 = malloc(20 * sizeof(int));\n"
              "    tab4[20] = 0;\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("[test.cpp:3]: (error) Array 'tab4[20]' accessed at index 20, which is out of bounds.\n", errout.str());

        // ticket #1134
        check("void f() {\n"
              "    int *x, i;\n"
              "    x = malloc(10 * sizeof(int));\n"
              "    x[10] = 0;\n"
              "}", false, "test.cpp", false);
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'x[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    // statically allocated buffer
    void alloc3() {
        check("void foo()\n"
              "{\n"
              "    const char *s = \"123\";\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[4]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *s; s = \"\";\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[1]' accessed at index 10, which is out of bounds.\n", errout.str());

        check("void foo() {\n"
              "    const char *s = \"\";\n"
              "    s = y();\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // data is allocated with alloca
    void alloc4() {
        check("void foo()\n"
              "{\n"
              "    char *s = alloca(10);\n"
              "    s[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 's[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void malloc_memset() {
        checkstd("void f() {\n"
                 "    char *p = malloc(10);\n"
                 "    memset(p,0,100);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds.\n", errout.str());
    }

    void memset1() {
        checkstd("void foo()\n"
                 "{\n"
                 "    char s[10];\n"
                 "    memset(s, 5, '*');\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) The size argument is given as a char constant.\n", errout.str());

        checkstd("void foo()\n"
                 "{\n"
                 "    int* x[5];\n"
                 "    memset(x, 0, sizeof(x));\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void memset2() {
        check("class X {\n"
              "    char* array[2];\n"
              "    X();\n"
              "};\n"
              "X::X() {\n"
              "    memset(array, 0, sizeof(array));\n"
              "}",false,"test.cpp",false);
        ASSERT_EQUALS("", errout.str());
    }

    void counter_test() const {
        std::list<const Token*> unknownParameter;
        unknownParameter.push_back(0);

        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("Hello", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("s", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("i", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%d", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%1d", unknownParameter));
        ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%2.2d", unknownParameter));
        ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("%s", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("f%s", unknownParameter));
        ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("%-s", unknownParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("\\\"", unknownParameter));
        ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("Hello \\0Text", unknownParameter));
        ASSERT_EQUALS(1, CheckBufferOverrun::countSprintfLength("\\0", unknownParameter));
        ASSERT_EQUALS(2, CheckBufferOverrun::countSprintfLength("%%", unknownParameter));
        ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("%d%d", unknownParameter));
        ASSERT_EQUALS(3, CheckBufferOverrun::countSprintfLength("\\\\a%s\\0a", unknownParameter));
        ASSERT_EQUALS(10, CheckBufferOverrun::countSprintfLength("\\\\\\\\Hello%d \\0Text\\\\\\\\", unknownParameter));
        ASSERT_EQUALS(4, CheckBufferOverrun::countSprintfLength("%%%%%d", unknownParameter));

        Token strTok(0);
        strTok.str("\"12345\"");
        std::list<const Token*> stringAsParameter;
        stringAsParameter.push_back(&strTok);
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("str%s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-4s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%-5s", stringAsParameter));
        ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("%-6s", stringAsParameter));
        ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%.4s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.5s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.6s", stringAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5.6s", stringAsParameter));
        ASSERT_EQUALS(7, CheckBufferOverrun::countSprintfLength("%6.6s", stringAsParameter));

        std::list<const Token*> intAsParameter;
        Token numTok(0);
        numTok.str("12345");
        intAsParameter.push_back(&numTok);
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%02ld", intAsParameter));
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%08ld", intAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.2d", intAsParameter));
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%08.2d", intAsParameter));
        TODO_ASSERT_EQUALS(5, 2, CheckBufferOverrun::countSprintfLength("%x", intAsParameter));
        ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%4x", intAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5x", intAsParameter));
        ASSERT_EQUALS(5, CheckBufferOverrun::countSprintfLength("%.4x", intAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%.5x", intAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%1.5x", intAsParameter));
        ASSERT_EQUALS(6, CheckBufferOverrun::countSprintfLength("%5.1x", intAsParameter));

        std::list<const Token*> floatAsParameter;
        Token floatTok(0);
        floatTok.str("1.12345f");
        floatAsParameter.push_back(&floatTok);
        TODO_ASSERT_EQUALS(5, 3, CheckBufferOverrun::countSprintfLength("%.2f", floatAsParameter));
        ASSERT_EQUALS(9, CheckBufferOverrun::countSprintfLength("%8.2f", floatAsParameter));
        TODO_ASSERT_EQUALS(5, 3, CheckBufferOverrun::countSprintfLength("%2.2f", floatAsParameter));

        std::list<const Token*> floatAsParameter2;
        Token floatTok2(0);
        floatTok2.str("100.12345f");
        floatAsParameter2.push_back(&floatTok2);
        TODO_ASSERT_EQUALS(7, 3, CheckBufferOverrun::countSprintfLength("%2.2f", floatAsParameter2));
        TODO_ASSERT_EQUALS(7, 3, CheckBufferOverrun::countSprintfLength("%.2f", floatAsParameter));
        TODO_ASSERT_EQUALS(7, 5, CheckBufferOverrun::countSprintfLength("%4.2f", floatAsParameter));

        std::list<const Token*> multipleParams;
        multipleParams.push_back(&strTok);
        multipleParams.push_back(0);
        multipleParams.push_back(&numTok);
        ASSERT_EQUALS(15, CheckBufferOverrun::countSprintfLength("str%s%d%d", multipleParams));
        ASSERT_EQUALS(26, CheckBufferOverrun::countSprintfLength("str%-6s%08ld%08ld", multipleParams));

    }

    void strncpy1() {
        checkstd("void f() {\n"
                 "    char c[7];\n"
                 "    strncpy(c, \"hello\", 7);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f() {\n"
                 " char c[6];\n"
                 " strncpy(c,\"hello\",6);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());

        checkstd("void f() {\n"
                 " char c[5];\n"
                 " strncpy(c,\"hello\",6);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: c\n", errout.str());

        checkstd("void f() {\n"
                 "    char c[6];\n"
                 "    strncpy(c,\"hello!\",7);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer is accessed out of bounds: c\n", errout.str());

        checkstd("struct AB { char a[10]; };\n"
                 "void foo(AB *ab) {\n"
                 "    strncpy(x, ab->a, 100);\n"
                 "}");
        ASSERT_EQUALS("", errout.str());
    }

    void unknownType() {
        check("void f()\n"
              "{\n"
              " UnknownType *a = malloc(4);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void terminateStrncpy1() {
        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[99] = 0;\n"
              "    strncpy(baz, bar, 100);\n"
              "    baz[99] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'baz' may not be null-terminated after the call to strncpy().\n", errout.str());

        // Test with invalid code that there is no segfault
        check("char baz[100];\n"
              "strncpy(baz, \"var\", 100)\n");
        ASSERT_EQUALS("", errout.str());

        // Test that there are no duplicate error messages
        check("void foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    foo(baz);\n"
              "    foo(baz);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'baz' may not be null-terminated after the call to strncpy().\n", errout.str());
    }

    void terminateStrncpy2() {
        check("char *foo ( char *bar ) {\n"
              "    char baz[100];\n"
              "    strncpy(baz, bar, 100);\n"
              "    bar[99] = 0;\n"
              "    return baz;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'baz' may not be null-terminated after the call to strncpy().\n", errout.str());
    }

    void terminateStrncpy3() {
        // Ticket #2170 - false positive
        // The function bar is risky. But it might work that way intentionally.
        check("char str[100];\n"
              "\n"
              "void foo(char *a) {\n"
              "    strncpy(str, a, 100);\n"
              "}\n"
              "\n"
              "void bar(char *p) {\n"
              "    strncpy(p, str, 100);\n"
              "}\n", false);
        ASSERT_EQUALS("[test.cpp:4]: (warning, inconclusive) The buffer 'str' may not be null-terminated after the call to strncpy().\n", errout.str());
    }

    void recursive_long_time() {
        // Just test that recursive check doesn't take long time
        check("char *f2 ( char *b )\n"
              "{\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "    f2( b );\n"
              "}\n"
              "void f()\n"
              "{\n"
              "    char a[10];\n"
              "    f2(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    // Ticket #1587 - crash
    void crash1() {
        check("struct struct A\n"
              "{\n"
              "    int alloclen;\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    struct A *str;\n"
              "    str = malloc(4);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void crash2() {
        check("void a(char *p) {\n"
              "    f( { if(finally_arg); } );\n"
              "}\n"
              "\n"
              "void b() {\n"
              "    char arr[64];\n"
              "    a(arr);\n"
              "}");
    }

    void crash3() {
        check("struct b { unknown v[0]; };\n"
              "void d() { struct b *f; f = malloc(108); }");
    }

    void epcheck(const char code[], const char filename[] = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        // Check for buffer overruns..
        CheckBufferOverrun checkBufferOverrun(&tokenizer, &settings, this);
        checkBufferOverrun.bufferOverrun();
    }


    void executionPaths1() {
        epcheck("void f(int a)\n"
                "{\n"
                "    int buf[10];\n"
                "    int i = 5;\n"
                "    if (a == 1)\n"
                "        i = 1000;\n"
                "    buf[i] = 0;\n"
                "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'buf[10]' accessed at index 1000, which is out of bounds.\n", errout.str());

        epcheck("void f(int a)\n"
                "{\n"
                "    int buf[10][5];\n"
                "    int i = 5;\n"
                "    if (a == 1)\n"
                "        i = 1000;\n"
                "    buf[i][0] = 0;\n"
                "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array 'buf[10][5]' index buf[1000][0] out of bounds.\n", errout.str());
    }

    void executionPaths2() {
        epcheck("void foo()\n"
                "{\n"
                "    char a[64];\n"
                "    int sz = sizeof(a);\n"
                "    bar(&sz);\n"
                "    a[sz] = 0;\n"
                "}");
        ASSERT_EQUALS("", errout.str());
    }

    void executionPaths3() {
        epcheck("void f(char *VLtext)\n"
                "{\n"
                "    if ( x ) {\n"
                "        return VLtext[0];\n"
                "    } else {\n"
                "        int wordlen = ab();\n"
                "        VLtext[wordlen] = 0;\n"
                "    }\n"
                "}");
        ASSERT_EQUALS("", errout.str());
    }

    void executionPaths5() {
        // No false positive
        epcheck("class A {\n"
                "    void foo() {\n"
                "        int j = g();\n"
                "        arr[j]=0;\n"
                "    }\n"
                "\n"
                "    int arr[2*BSize + 2];\n"
                "};");
        ASSERT_EQUALS("", errout.str());
    }

    void executionPaths6() {  // handling unknown type
        const char code[] = "void f() {\n"
                            "    u32 a[10];"
                            "    u32 i = 0;\n"
                            "    if (x) { i = 1000; }\n"
                            "    a[i] = 0;\n"
                            "}";
        epcheck(code);
        ASSERT_EQUALS("[test.cpp:4]: (error) Array 'a[10]' accessed at index 1000, which is out of bounds.\n", errout.str());
    }

    void cmdLineArgs1() {
        check("int main(int argc, char* argv[])\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char* argv[])\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char* argv[])\n"
              "{\n"
              "    char prog[10];\n"
              "    sprintf(prog, \"%s\", argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    sprintf(prog, \"%s\", argv[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10];\n"
              "    strcpy(prog, options[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    strcat(prog, options[0]);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **options)\n"
              "{\n"
              "    char prog[10];\n"
              "    sprintf(prog, \"%s\", *options);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    if (strlen(argv[0]) < 10)\n"
              "        strcpy(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10] = {'\\0'};\n"
              "    if (10 > strlen(argv[0]))\n"
              "        strcat(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    sprintf(prog, \"%p\", argv[0]);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("int main(int argc, char **argv, char **envp)\n"
              "{\n"
              "    char prog[10];\n"
              "    argv[0][0] = '\\0';\n"
              "    strcpy(prog, argv[0]);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        // #5835
        checkstd("int main(int argc, char* argv[]) {\n"
                 "    char prog[10];\n"
                 "    sprintf(prog, \"%s\", argv[0]);\n"
                 "    sprintf(prog, \"%s\", argv[0]);\n"
                 "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Buffer overrun possible for long command line arguments.\n"
                      "[test.cpp:4]: (error) Buffer overrun possible for long command line arguments.\n", errout.str());
    }

    void scope() {
        check("class A {\n"
              "private:\n"
              "    struct X { char buf[10]; };\n"
              "}\n"
              "\n"
              "void f()\n"
              "{\n"
              "    X x;\n"
              "    x.buf[10] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A {\n"
              "public:\n"
              "    struct X { char buf[10]; };\n"
              "}\n"
              "\n"
              "void f()\n"
              "{\n"
              "    A::X x;\n"
              "    x.buf[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array 'x.buf[10]' accessed at index 10, which is out of bounds.\n", errout.str());
    }

    void getErrorMessages() {
        // Ticket #2292: segmentation fault when using --errorlist
        CheckBufferOverrun c;
        c.getErrorMessages(this, 0);
    }

    void unknownMacroNoDecl() {
        check("void f() {\n"
              "    int a[10];\n"
              "    AAA a[0] = 0;\n"   // <- not a valid array declaration
              "    a[1] = 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void arrayIndexThenCheck() {
        check("void f(const char s[]) {\n"
              "    if (s[i] == 'x' && i < y) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("", errout.str()); // No message because i is unknown and thus gets no varid. Avoid an internalError here.

        check("void f(const char s[], int i) {\n"
              "    if (s[i] == 'x' && i < y) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Array index 'i' is used before limits check.\n", errout.str());

        check("void f(const char s[]) {\n"
              "    for (int i = 0; s[i] == 'x' && i < y; ++i) {\n"
              "    }"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Array index 'i' is used before limits check.\n", errout.str());

        check("void f(const int a[], unsigned i) {\n"
              "    if((a[i] < 2) && (i <= 42)) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Array index 'i' is used before limits check.\n", errout.str());

        check("void f(const int a[], unsigned i) {\n"
              "    if((a[i] < 2) && (42 >= i)) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Array index 'i' is used before limits check.\n", errout.str());

        check("void f(char* e, int y) {\n"
              "    if (e[y] == '/' && elen > y + 1 && e[y + 1] == '?') {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // this one doesn't work for now, hopefully in the future
        check("void f(const int a[], unsigned i) {\n"
              "    if(a[i] < func(i) && i <= 42) {\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (style) Array index 'i' is used before limits check.\n", "", errout.str());
    }

    void bufferNotZeroTerminated() {
        check("void f() {\n"
              "    char c[6];\n"
              "    strncpy(c,\"hello!\",6);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'c' is not null-terminated after the call to strncpy().\n", errout.str());

        check("void f() {\n"
              "    char c[6];\n"
              "    memcpy(c,\"hello!\",6);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'c' is not null-terminated after the call to memcpy().\n", errout.str());

        check("void f() {\n"
              "    char c[6];\n"
              "    memmove(c,\"hello!\",6);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'c' is not null-terminated after the call to memmove().\n", errout.str());
    }

    void readlink() {
        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlink(path, buf, 254);\n"
              "    printf(\"%s\n\", buf);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'buf' is not null-terminated after the call to readlink().\n", "", errout.str());

        // C only: Primitive pointer simplification
        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlink(path, &buf[0], 254);\n"
              "    printf(\"%s\n\", buf);\n"
              "}\n", true, "test.c");
        TODO_ASSERT_EQUALS("[test.c:3]: (warning, inconclusive) The buffer 'buf' is not null-terminated after the call to readlink().\n", "", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlink(path, buf, 254);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char buf[10];\n"
              "    ssize_t len = readlink(path, buf, 255);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) readlink() buf size is out of bounds: Supplied size 255 is larger than actual size 10.\n", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlink(path, buf, 255);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) readlink() might return the full size of 'buf'. Lower the supplied size by one.\n", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlink(path, buf, 254);\n"
              "    if (len == -1) {\n"
              "        return;\n"
              "    }\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char buf[255] = {0};\n"
              "    readlink(path, buf, 254);\n"  // <- doesn't write whole buf
              "    puts(buf);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void readlinkat() {
        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlinkat(42, path, buf, 254);\n"
              "    printf(\"%s\n\", buf);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) The buffer 'buf' is not null-terminated after the call to readlinkat().\n", "", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlinkat(42, path, buf, 254);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    char buf[10];\n"
              "    ssize_t len = readlinkat(42, path, buf, 255);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) readlinkat() buf size is out of bounds: Supplied size 255 is larger than actual size 10.\n", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlinkat(42, path, buf, 255);\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning, inconclusive) readlinkat() might return the full size of 'buf'. Lower the supplied size by one.\n", errout.str());

        check("void f() {\n"
              "    char buf[255];\n"
              "    ssize_t len = readlinkat(42, path, buf, 254);\n"
              "    if (len == -1) {\n"
              "        return;\n"
              "    }\n"
              "    buf[len] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void writeOutsideBufferSize() {
        check("void f(void){\n"
              "write(1, \"Dump string \\n\", 100);\n"
              "}");                       // ^ number of bytes too big
        ASSERT_EQUALS("[test.cpp:2]: (error) Writing 86 bytes outside buffer size.\n", errout.str());

        check("void f(void){\n"
              "write(1, \"Dump string \\n\", 10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4706 avoid crashing when a struct member is used as first argument
        check("static struct {\n"
              "    int i[2];\n"
              "} p;\n"
              "void foo()\n"
              "{\n"
              "    write(p.i[1], \"\", 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static struct {\n"
              "    int i[2];\n"
              "} p;\n"
              "void foo()\n"
              "{\n"
              "    write(p.i[1], \"\", 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Writing 1 bytes outside buffer size.\n", errout.str());
        // #4969
        check("void foo()\n"
              "{\n"
              "    write(1, \"\\0\", 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        // that is documented to be ok
        check("void foo()\n"
              "{\n"
              "    write(1, 0, 0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        // ... that is not ok
        check("void foo()\n"
              "{\n"
              "    write(1, 0, 1);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Writing 1 bytes outside buffer size.\n", "", errout.str());
    }

    void negativeMemoryAllocationSizeError() { // #389
        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = new int[-1];\n"
              "   delete [] a;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory allocation size is negative.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = malloc( -10 );\n"
              "   free(a);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory allocation size is negative.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = malloc( -10);\n"
              "   free(a);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory allocation size is negative.\n", errout.str());

        check("void f()\n"
              "{\n"
              "   int *a;\n"
              "   a = alloca( -10 );\n"
              "   free(a);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory allocation size is negative.\n", errout.str());
    }

    void garbage1() {
        check("void foo() {\n"
              "char *a = malloc(10);\n"
              "a[0]\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestBufferOverrun)
