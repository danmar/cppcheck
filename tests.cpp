
// Unit-testing cppcheck

//---------------------------------------------------------------------------

#include "tokenize.h"   // <- Tokenizer
#include "Statements.h"
#include "CommonCheck.h"

#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckMemoryLeak.h"

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------
bool ShowAll = true;
bool Debug = false;
//---------------------------------------------------------------------------
static unsigned int FailCount, SuccessCount;
//---------------------------------------------------------------------------
static void internal_statementlist();
static void buffer_overrun();
static void constructors();
static void operator_eq();
static void memleak_in_function();
static void memleak_in_class();
//---------------------------------------------------------------------------

int main()
{
    Files.push_back( std::string("test.cpp") );
    internal_statementlist();
    buffer_overrun();
    constructors();
    operator_eq();
    memleak_in_function();
    memleak_in_class();
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

static void statementlist()
{
    CreateStatementList();
    OutputStatementList( errout );
}

static void internal_statementlist()
{
    const char code1[] = "void f()\n"
                         "{\n"
                         "    a = 1;\n"
                         "    b[2] = 3;\n"
                         "    c[4][5].min = 6;\n"
                         "    d.min = 7;\n"
                         "}\n";

    const char sl1[] = "{\n"
                       "assign a\n"
                       "assign b[2]\n"
                       "assign c[4][5].min\n"
                       "assign d.min\n"
                       "}\n";

    check( statementlist, __LINE__, code1, sl1 );





    const char code2[] = "void f()\n"
                         "{\n"
                         "    int a;\n"
                         "    int b = 2, c, *d = NULL;\n"
                         "    int e = g(p1,p2);\n"
                         "    char str[10];\n"
                         "    return a;\n"
                         "    delete a;\n"
                         "}\n";

    const char sl2[] = "{\n"
                       "decl a\n"
                       "decl b\n"
                       "assign b\n"
                       "decl c\n"
                       "decl d\n"
                       "assign d\n"
                       "use NULL\n"
                       "decl e\n"
                       "assign e\n"
                       "use p1\n"
                       "use p2\n"
                       "decl str\n"
                       "return a\n"
                       "delete a\n"
                       "}\n";

    check( statementlist, __LINE__, code2, sl2 );






    const char code3[] = "void f()\n"
                         "{\n"
                         "    if (ab)\n"
                         "    {\n"
                         "    }\n"
                         "    else if (cd)\n"
                         "    {\n"
                         "    }\n"
                         "    else\n"
                         "    {\n"
                         "    }\n"
                         "}\n";

    const char sl3[] = "{\n"
                       "if\n"
                       "use ab\n"
                       "endif\n"
                       "{\n"
                       "}\n"
                       "elseif\n"
                       "use cd\n"
                       "endif\n"
                       "{\n"
                       "}\n"
                       "else\n"
                       "endif\n"
                       "{\n"
                       "}\n"
                       "}\n";

    check( statementlist, __LINE__, code3, sl3 );






    const char code4[] = "void f()\n"
                         "{\n"
                         "    for (int i = 0; i < j; i++)\n"
                         "    {\n"
                         "        if (condition)\n"
                         "            continue;\n"
                         "        break;\n"
                         "    }\n"
                         "}\n";

    const char sl4[] = "{\n"
                       "loop\n"
                       "assign i\n"
                       "use i\n"
                       "use i\n"
                       "use j\n"
                       "use i\n"
                       "endloop\n"
                       "{\n"
                       "if\n"
                       "use condition\n"
                       "continue\n"
                       "endif\n"
                       "break\n"
                       "}\n"
                       "}\n";

    check( statementlist, __LINE__, code4, sl4 );





    const char code5[] = "void f()\n"
                         "{\n"
                         "    a = new char[10];\n"
                         "    fred = new Fred;\n"
                         "    fred = new Fred();\n"
                         "}\n";

    const char sl5[] = "{\n"
                       "new[] a\n"
                       "use char[10]\n"
                       "new fred\n"
                       "use Fred\n"
                       "new fred\n"
                       "}\n";

    check( statementlist, __LINE__, code5, sl5 );



    const char code6[] = "void f()\n"
                         "{\n"
                         "    a = b;\n"
                         "    c = func(d,e);\n"
                         "}\n";

    const char sl6[] = "{\n"
                       "assign a\n"
                       "use b\n"
                       "assign c\n"
                       "use d\n"
                       "use e\n"
                       "}\n";

    check( statementlist, __LINE__, code6, sl6 );

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
    // test7: unknown string length

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
    // Test1: No constructor
    // Test2: embedded constructor, uninitialized variable (TODO)
    // Test3: Uninitialized variable
    // Test4: multiple constructors, uninitialized variable

    const char test1[] = "class clKalle\n"
                         "{\n"
                         "public:\n"
                         "    int i;\n"
                         "};\n";
    check( CheckConstructors, __LINE__, test1, "[test.cpp:1] The class 'clKalle' has no constructor\n" );




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

static void memleak_in_function()
{
    // test1: 'new' but not 'delete'
    // test2: Return allocated memory
    // test3: check all execution paths
    // test4: check all execution paths
    // test5: check all execution paths
    // test6: check all execution paths
    // test7: check all execution paths
    // test8: check all execution paths
    // test9: mismatching allocation / deallocation

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
                         "    char *str = strdup(\"hello\");\n"
                         "    while (condition)\n"
                         "    {\n"
                         "        if (condition)\n"
                         "            break;\n"
                         "    }\n"
                         "    free(str);\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test5, "" );




    const char test6[] = "void f()\n"
                         "{\n"
                         "    char *str = strdup(\"hello\");\n"
                         "    if (a==b)\n"
                         "    {\n"
                         "        return;\n"
                         "    }\n"
                         "    free(str);\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test6, "[test.cpp:6]: Memory leak:str\n" );




    const char test7[] = "void f()\n"
                         "{\n"
                         "    char *str = strdup(\"hello\");\n"
                         "    if (a==b)\n"
                         "    {\n"
                         "        free(str);\n"
                         "        return;\n"
                         "    }\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test7, "[test.cpp:9]: Memory leak:str\n" );




    const char test8[] = "void f()\n"
                         "{\n"
                         "    char *str = new char[10];\n"
                         "    if (a==b)\n"
                         "    {\n"
                         "        delete [] str;\n"
                         "        return;\n"
                         "    }\n"
                         "    delete [] str;\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test8, "" );




    const char test9[] = "void f()\n"
                         "{\n"
                         "    int *a = new int[10];\n"
                         "    free(a);\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test9, "[test.cpp:4]: Mismatching allocation and deallocation 'a'\n" );

}
//---------------------------------------------------------------------------

static void memleak_in_class()
{


    const char test1[] = "class clKalle\n"
                         "{\n"
                         "private:\n"
                         "    char *str1;\n"
                         "    char *str2;\n"
                         "public:\n"
                         "    clKalle();\n"
                         "    ~clKalle();\n"
                         "};\n"
                         "\n"
                         "clKalle::clKalle()\n"
                         "{\n"
                         "    str1 = new char[10];\n"
                         "    str2 = new char[10];\n"
                         "}\n"
                         "\n"
                         "clKalle::~clKalle()\n"
                         "{\n"
                         "    delete [] str2;\n"
                         "}\n";

    check( CheckMemoryLeak, __LINE__, test1, "Memory leak for 'clKalle::str1'\n" );




    const char test2[] = "class clKalle\n"
                         "{\n"
                         "private:\n"
                         "    char *str1;\n"
                         "public:\n"
                         "    clKalle();\n"
                         "    ~clKalle();\n"
                         "};\n"
                         "\n"
                         "clKalle::clKalle()\n"
                         "{\n"
                         "    str1 = new char[10];\n"
                         "}\n"
                         "\n"
                         "clKalle::~clKalle()\n"
                         "{\n"
                         "    free(str1);\n"
                         "}\n";

    check( CheckMemoryLeak, __LINE__, test2, "[test.cpp:17]: Mismatching deallocation for 'clKalle::str1'\n" );



}
//---------------------------------------------------------------------------

