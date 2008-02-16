
// Unit-testing cppcheck

//---------------------------------------------------------------------------

#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"

#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckMemoryLeak.h"

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------
bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;
//---------------------------------------------------------------------------
static unsigned int FailCount, SuccessCount;
//---------------------------------------------------------------------------
static void buffer_overrun();
static void constructors();
static void operator_eq();
static void mismatching_allocation_deallocation();
static void memleak_in_function();
//---------------------------------------------------------------------------

int main()
{
    Files.push_back( std::string("test.cpp") );
    buffer_overrun();
    constructors();
    operator_eq();
    memleak_in_function();
    mismatching_allocation_deallocation();
    std::cout << "Success Rate: " 
              << SuccessCount 
              << " / " 
              << (SuccessCount + FailCount) 
              << std::endl;
    return 0;
}
//---------------------------------------------------------------------------

static void check(void (chk)(),
                  const unsigned int line,
                  const char code[],
                  const char msg[])
{
    // Tokenize..
    tokens = tokens_back = NULL;
    std::istringstream istr(code);
    TokenizeCode( istr );
    SimplifyTokenList();

    // Check for buffer overruns..
    errout.str("");
    chk();

    // Check the error messages..
    std::string err( errout.str() );
    if ( err == msg )
    {
        SuccessCount++;
    }
    else
    {
        FailCount++;
        std::cerr << "Failed at line " << line << std::endl
                  << "Unexpected Result:" << std::endl
                  << err << std::endl;
    }

    // Cleanup..
    DeallocateTokens();
}
//---------------------------------------------------------------------------

static void buffer_overrun()
{
    // test1: numeric array index
    // test2: variable array index (for-loop)
    // test3: creating several arrays with the same names.
    // test4: using strcpy -> check string length
    // test5: constant array index
    // test6: calculated array index that is out of bounds

    const char test1[] = "void f()\n"
                         "{\n"
                         "    char str[0x10];\n"
                         "    str[15] = 0;\n"
                         "    str[16] = 0;\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test1, "[test.cpp:5]: Array index out of bounds\n" );



    const char test2[] = "void f()\n"
                         "{\n"
                         "    int val[50];\n"
                         "    for (i = 0; i < 100; i++)\n"
                         "        sum += val[i];\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test2, "[test.cpp:5]: Buffer overrun\n" );



    const char test3[] = "void f()\n"
                         "{\n"
                         "    if (ab)\n"
                         "    {\n"
                         "        char str[50];\n"
                         "    }\n"
                         "    if (ab)\n"
                         "    {\n"
                         "        char str[50];\n"
                         "    }\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test3, "" );



    const char test4[] = "void f()\n"
                         "{\n"
                         "    char str[3];\n"
                         "    strcpy(str, \"abc\");\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test4, "[test.cpp:4]: Buffer overrun\n" );



    const char test5[] = "const int SIZE = 10;\n"
                         "void f()\n"
                         "{\n"
                         "    int i[SIZE];\n"
                         "    i[SIZE] = 0;\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test5, "[test.cpp:5]: Array index out of bounds\n" );




    const char test6[] = "void f()\n"
                         "{\n"
                         "    int i[10];\n"
                         "    i[ sizeof(i) - 1 ] = 0;\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test6, "[test.cpp:4]: Array index out of bounds\n" );




    const char test7[] = "void f1(char *str)\n"
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
                         "}\n";
    const char err7[] = 
        "[test.cpp:3]: A string with unknown length is copied to buffer.\n"
        "[test.cpp:7]: A string with unknown length is copied to buffer.\n"
        "[test.cpp:11]: A string with unknown length is copied to buffer.\n"
        "[test.cpp:15]: A string with unknown length is copied to buffer.\n";

    check( CheckBufferOverrun, __LINE__, test7, err7 );
}
//---------------------------------------------------------------------------

static void constructors()
{
    // Test1: No constructor => Uninitialized variable (TODO)
    // Test2: embedded constructor, uninitialized variable (TODO)
    // Test3: Uninitialized variable
    // Test4: multiple constructors, uninitialized variable

    const char test1[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    int i;\n"
                         "};\n";
    check( CheckConstructors, __LINE__, test1, "" );




    const char test2[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    clKalle() { }\n"
                         "    int i;\n"
                         "};\n";
    check( CheckConstructors, __LINE__, test2, "" );



    const char test3[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    clKalle();\n"
                         "    int i;\n"
                         "};\n"
                         "clKalle::clKalle()\n"
                         "{ }\n";
    check( CheckConstructors, __LINE__, test3, "[test.cpp:8] Uninitialized member variable 'clKalle::i'\n" );


    const char test4[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    clKalle();\n"
                         "    clKalle(int _i);\n"
                         "    int i;\n"
                         "};\n"
                         "clKalle::clKalle()\n"
                         "{ }\n"
                         "clKalle::clKalle(int _i)\n"
                         "{\n"
                         "    i = _i;\n"
                         "}\n";
    check( CheckConstructors, __LINE__, test4, "[test.cpp:9] Uninitialized member variable 'clKalle::i'\n" );

}
//---------------------------------------------------------------------------

void operator_eq()
{
    const char test1[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    void operator=(const int &value);\n"
                         "};\n";
    check( CheckOperatorEq1, __LINE__, test1, "[test.cpp:4]: 'operator=' should return something\n" );

}
//---------------------------------------------------------------------------

static void mismatching_allocation_deallocation()
{
    // TODO: This check must be created as I can't find it anywhere

/*
    const char test1[] = "void f()\n"
                         "{\n"
                         "    int *a = new int[10];\n"
                         "    free(a);\n"
                         "}\n";
    check( CheckMismatchingAllocationDeallocation, __LINE__, test1, "[test.cpp:4]: Mismatching allocation / deallocation\n" );
*/
}
//---------------------------------------------------------------------------

static void memleak_in_function()
{
    // test1: 'new' but not 'delete'
    // test2: Return allocated memory
    // test3: check all execution paths
    // test4: check all execution paths
    // test5: check all execution paths

    const char test1[] = "void f()\n"
                         "{\n"
                         "    int *a = new int[10];\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test1, "[test.cpp:4]: Memory leak:a\n" );




    const char test2[] = "Fred *NewFred()\n"
                         "{\n"
                         "    Fred *f = new Fred;\n"
                         "    return f;\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test2, "" );





    const char test3[] = "void f()\n"
                         "{\n"
                         "    Kalle *kalle;\n"
                         "    if (somecondition)\n"
                         "    {\n"
                         "        kalle = new Kalle;\n"
                         "    }\n"
                         "    else\n"
                         "    {\n"
                         "        return;\n"
                         "    }\n"
                         "    delete kalle;\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test3, "" );



    const char test4[] = "void f()\n"
                         "{\n"
                         "    for (int i = 0; i < j; i++)\n"
                         "    {\n"
                         "        char *str = strdup(\"hello\");\n"
                         "        if (condition)\n"
                         "            continue;\n"
                         "        free(str);\n"
                         "    }\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test4, "[test.cpp:7]: Memory leak:str\n" );





    const char test5[] = "void f()\n"
                         "{\n"
                         "     char *str = strdup(\"hello\");\n"
                         "    while (condition)\n"
                         "    {\n"
                         "        if (condition)\n"
                         "            break;\n"
                         "    }\n"
                         "    free(str);\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test5, "" );

}
//---------------------------------------------------------------------------

