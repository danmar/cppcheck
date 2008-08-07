
// Unit-testing cppcheck

//---------------------------------------------------------------------------

#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"

#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckMemoryLeak.h"
#include "CheckOther.h"

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------
bool ShowAll = true;
bool CheckCodingStyle = true;
bool Debug = false;
//---------------------------------------------------------------------------
static unsigned int FailCount, SuccessCount;
//---------------------------------------------------------------------------
static void buffer_overrun();
static void constructors();
static void operator_eq();
static void memleak_in_function();
static void memleak_in_class();
static void division();
static void variable_scope();
static void fpar_byvalue();
static void unused_struct_member();
//---------------------------------------------------------------------------

int main()
{
    // Provide a dummy filename for the error messages
    Files.push_back( std::string("test.cpp") );

    // Don't filter out duplicate error messages..
    OnlyReportUniqueErrors = false;

    // Check that buffer overruns are detected
    buffer_overrun();

    // Test the constructor-checks
    constructors();

    // Test the class operator= checking
    operator_eq();

    // Test that memory leaks in a function are detected
    memleak_in_function();

    // Test that memory leaks in a class are detected
    memleak_in_class();

    // Check for dangerous division.. such as "svar / uvar". Treating "svar" as unsigned data is not good
    division();

    // variable scope..
    variable_scope();

    fpar_byvalue();
    
    // unused struct member..
    unused_struct_member();

    // unused variable

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
    if ( chk != CheckUnsignedDivision )
        SimplifyTokenList();

    FillFunctionList(0);

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
    // test7: unknown string length
    // test8: struct member..

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

    check( CheckBufferOverrun, __LINE__, test7, "" );


    const char test8[] = "struct ABC\n"
                         "{\n"
                         "    char str[10];\n"
                         "};\n"
                         "\n"
                         "static void f()\n"
                         "{\n"
                         "    struct ABC abc;\n"
                         "    abc.str[10] = 0;\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test8, "[test.cpp:9]: Array index out of bounds\n" );


    const char test9[] = "const int SIZE = 10;\n"
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
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test9, "[test.cpp:11]: Array index out of bounds\n" );



    const char test10[] = "struct ABC\n"
                          "{\n"
                          "    char str[10];\n"
                          "};\n"
                          "\n"
                          "static void f(ABC *abc)\n"
                          "{\n"
                          "    abc->str[10] = 0;\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test10, "[test.cpp:8]: Array index out of bounds\n" );



    const char test11[] = "struct ABC\n"
                          "{\n"
                          "    char str[5];\n"
                          "};\n"
                          "\n"
                          "static void f(ABC *abc)\n"
                          "{\n"
                          "    strcpy( abc->str, \"abcdef\" );\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test11, "[test.cpp:8]: Buffer overrun\n" );



    const char test12[] = "static void memclr( char *data )\n"
                          "{\n"
                          "    data[10] = 0;\n"
                          "}\n"
                          "\n"
                          "static void f()\n"
                          "{\n"
                          "    char str[5];\n"
                          "    memclr( str );   // ERROR\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test12, "[test.cpp:9] -> [test.cpp:3]: Array index out of bounds\n" );


    const char test13[] = "struct ABC\n"
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
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test13, "[test.cpp:13] -> [test.cpp:8]: Array index out of bounds\n" );



    const char test14[] = "class ABC\n"
                          "{\n"
                          "public:\n"
                          "    ABC();\n"
                          "    char *str[10];\n"
                          "    struct ABC *next;"
                          "};\n"
                          "\n"
                          "static void f()\n"
                          "{\n"
                          "    for ( ABC *abc = abc1; abc; abc = abc->next )\n"
                          "    {\n"
                          "        abc->str[10] = 0;\n"
                          "    }\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test14, "[test.cpp:12]: Array index out of bounds\n" );



    const char test15[] = "static void f()\n"
                          "{\n"
                          "    char data[1];\n"
                          "    return abc.data[1];\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test15, "" );





    // TODO
    /*
    const char test11[] = "static void memclr( char *data, const int bytes )\n"
                          "{\n"
                          "    for (int i = 0; i < bytes; i++)\n"
                          "        data[i] = 0;\n"
                          "}\n"
                          "\n"
                          "static void f()\n"
                          "{\n"
                          "    char str[5];\n"
                          "    memclr( str, 5 );   // OK\n"
                          "    memclr( str+1, 5 );   // ERROR\n"
                          "    memclr( str, 6 );   // ERROR\n"
                          "}\n";
    check( CheckBufferOverrun, __LINE__, test11, "" );
    */


    // TODO
    /*
    const char test[] = "class Fred\n"
                         "{\n"
                         "private:\n"
                         "    char str[10];\n"
                         "public:\n"
                         "    Fred();\n"
                         "};\n"
                         "Fred::Fred()\n"
                         "{\n"
                         "    str[10] = 0;\n"
                         "}\n";
    check( CheckBufferOverrun, __LINE__, test, "[test.cpp:5]: Array index out of bounds\n" );
    */

}
//---------------------------------------------------------------------------

static void constructors()
{
    // Test1: No constructor
    // Test2: embedded constructor, uninitialized variable
    // Test3: Uninitialized variable
    // Test4: multiple constructors, uninitialized variable

    const char test1[] = "class Fred\n"
                         "{\n"
                         "public:\n"
                         "    int i;\n"
                         "};\n";
    check( CheckConstructors, __LINE__, test1, "[test.cpp:1] The class 'Fred' has no constructor\n" );




    const char test2[] = "class Fred\n"
                         "{\n"
                         "public:\n"
                         "    Fred() { }\n"
                         "    int i;\n"
                         "};\n";
    check( CheckConstructors, __LINE__, test2, "[test.cpp:4] Uninitialized member variable 'Fred::i'\n" );



    const char test3[] = "class Fred\n"
                         "{\n"
                         "public:\n"
                         "    Fred();\n"
                         "    int i;\n"
                         "};\n"
                         "Fred::Fred()\n"
                         "{ }\n";
    check( CheckConstructors, __LINE__, test3, "[test.cpp:7] Uninitialized member variable 'Fred::i'\n" );


    const char test4[] = "class Fred\n"
                         "{\n"
                         "public:\n"
                         "    Fred();\n"
                         "    Fred(int _i);\n"
                         "    int i;\n"
                         "};\n"
                         "Fred::Fred()\n"
                         "{ }\n"
                         "Fred::Fred(int _i)\n"
                         "{\n"
                         "    i = _i;\n"
                         "}\n";
    check( CheckConstructors, __LINE__, test4, "[test.cpp:8] Uninitialized member variable 'Fred::i'\n" );

}
//---------------------------------------------------------------------------

static void operator_eq()
{
    const char test1[] = "class Fred\n"
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
    check( CheckMemoryLeak, __LINE__, test1, "[test.cpp:3]: Memory leak: a\n" );




    const char test2[] = "Fred *NewFred()\n"
                         "{\n"
                         "    Fred *f = new Fred;\n"
                         "    return f;\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test2, "" );




    /* TODO
    const char test3[] = "void f()\n"
                         "{\n"
                         "    Fred *fred;\n"
                         "    if (somecondition)\n"
                         "    {\n"
                         "        fred = new Fred;\n"
                         "    }\n"
                         "    else\n"
                         "    {\n"
                         "        return;\n"
                         "    }\n"
                         "    delete fred;\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test3, "" );
    */


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
    check( CheckMemoryLeak, __LINE__, test4, "[test.cpp:7]: Memory leak: str\n" );





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
    check( CheckMemoryLeak, __LINE__, test6, "[test.cpp:6]: Memory leak: str\n" );



    const char test7[] = "void f()\n"
                         "{\n"
                         "    char *str = strdup(\"hello\");\n"
                         "    if (a==b)\n"
                         "    {\n"
                         "        free(str);\n"
                         "        return;\n"
                         "    }\n"
                         "}\n";
    check( CheckMemoryLeak, __LINE__, test7, "[test.cpp:3]: Memory leak: str\n" );



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
    check( CheckMemoryLeak, __LINE__, test9, "[test.cpp:3]: Mismatching allocation and deallocation: a\n" );



    const char test10[] = "static void f()\n"
                          "{\n"
                          "    struct acpi_object_list *obj_list;\n"
                          "    obj_list = kmalloc(sizeof(struct acpi_object_list), GFP_KERNEL);\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test10, "[test.cpp:3]: Memory leak: obj_list\n" );



    const char test11[] = "static char *f()\n"
                          "{\n"
                          "    char *s = new char[100];\n"
                          "    return s;\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test11, "" );




    const char test12[] = "static char *f()\n"
                          "{\n"
                          "    Fred *fred = new Fred;\n"
                          "    free( fred->Name );\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test12, "[test.cpp:3]: Memory leak: fred\n" );




    const char test13[] = "static char *f()\n"
                          "{\n"
                          "    Fred *fred = new Fred;\n"
                          "    // fred is deleted automaticly\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test13, "" );


    /* TODO
    const char test14[] = "struct Fred\n"
                          "{\n"
                          "    char *str;\n"
                          "}\n"
                          "\n"
                          "void f()\n"
                          "{\n"
                          "    Fred f;\n"
                          "    f.str = strdup(\"aa\");\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test14, "[test.cpp:9]: Memory leak: f.str\n" );
    */



    const char test15[] = "static char *f()\n"
                          "{\n"
                          "    char *s = new char[100];\n"
                          "    return (char *)s;\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test15, "" );


    const char test16[] = "static char *f()\n"
                          "{\n"
                          "    char *s = new char[100];\n"
                          "    if ( a == b )\n"
                          "    {\n"
                          "        return s;\n"
                          "    }\n"
                          "    return NULL;\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test16, "[test.cpp:8]: Memory leak: s\n" );


    const char test17[] = "static void f()\n"
                          "{\n"
                          "    char *str = strdup(\"hello\");\n"
                          "    char *str2 = (char *)str;\n"
                          "    free(str2);\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test17, "" );


    const char test18[] = "static void f()\n"
                          "{\n"
                          "    char *str;\n"
                          "    if ((str = (char *)malloc(123,33)) == NULL)\n"
                          "        return;\n"
                          "    free(str);\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test18, "" );


    const char test19[] = "struct abc\n"
                          "{\n"
                          "    int a;\n"
                          "    int b;\n"
                          "    int c;\n"
                          "}\n"
                          "\n"
                          "static void f()\n"
                          "{\n"
                          "    struct abc *abc1 = new abc;\n"
                          "    p = &abc1->a;\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test19, "" );
    
    
    const char test20[] = "static char *dmalloc()\n"
                          "{\n"
                          "    char *p = new char[100];\n"
                          "    return p;\n"
                          "}\n"
                          "static void f()\n"
                          "{\n"
                          "    char *p = dmalloc();\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test20, "[test.cpp:8]: Memory leak: p\n" );
    
    
    const char test21[] = "static char *dmalloc()\n"
                          "{\n"
                          "    char *p = new char[100];\n"
                          "    return p;\n"
                          "}\n"
                          "static void f()\n"
                          "{\n"
                          "    char *p = dmalloc();\n"
                          "    delete p;\n"
                          "}\n";
    check( CheckMemoryLeak, __LINE__, test21, "[test.cpp:8]: Mismatching allocation and deallocation: p\n" );
}
//---------------------------------------------------------------------------

static void memleak_in_class()
{

    const char test1[] = "class Fred\n"
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
                         "}\n";

    check( CheckMemoryLeak, __LINE__, test1, "[test.cpp:1]: Memory leak: Fred::str1\n" );




    const char test2[] = "class Fred\n"
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
                         "}\n";

    check( CheckMemoryLeak, __LINE__, test2, "[test.cpp:17]: Mismatching allocation and deallocation: Fred::str1\n" );




/*   TODO
    const char test3[] = "class Fred\n"
                         "{\n"
                         "private:\n"
                         "    char *str;\n"
                         "public:\n"
                         "    Fred();\n"
                         "    ~Fred();\n"
                         "    void SetStr(const char s[]);"
                         "};\n"
                         "\n"
                         "Fred::Fred()\n"
                         "{\n"
                         "    str = NULL;\n"
                         "}\n"
                         "\n"
                         "Fred::~Fred()\n"
                         "{\n"
                         "    free(str1);\n"
                         "}\n"
                         "\n"
                         "void Fred::SetStr(const char s[])\n"
                         "{\n"
                         "    str = strdup(s);\n"
                         "}\n";

    check( CheckMemoryLeak, __LINE__, test3, "Memory leak for 'Fred::str'\n" );
*/


}
//---------------------------------------------------------------------------

static void division()
{

    const char test1[] = "void f()\n"
                         "{\n"
                         "    int ivar = -2;\n"
                         "    unsigned int uvar = 2;\n"
                         "    return ivar / uvar;\n"
                         "}\n";
    check( CheckUnsignedDivision, __LINE__, test1, "[test.cpp:5]: If the result is negative it will be wrong because an operand is unsigned.\n" );


    const char test2[] = "void f()\n"
                         "{\n"
                         "    int ivar = -2;\n"
                         "    unsigned int uvar = 2;\n"
                         "    return uvar / ivar;\n"
                         "}\n";
    check( CheckUnsignedDivision, __LINE__, test2, "[test.cpp:5]: If the result is negative it will be wrong because an operand is unsigned.\n" );

}
//---------------------------------------------------------------------------

static void variable_scope()
{
/* TODO
    // Unused private member variable...
    const char test1[] = "class Fred\n"
                         "{\n"
                         "private:\n"
                         "    int i;\n"
                         "public:\n"
                         "    Fred();\n"
                         "};\n"
                         "Fred::Fred()\n"
                         "{\n"
                         "    i = 0;\n"
                         "}\n";
*/

    // Scope of variable..
    const char test2[] = "void f()\n"
                         "{\n"
                         "    int i;\n"
                         "    if (abc)\n"
                         "    {\n"
                         "        i = 1;\n"
                         "    }\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test2, "[test.cpp:3] The scope of the variable 'i' can be limited\n" );


    const char test3[] = "static void DeleteNextToken(TOKEN *tok)\n"
                         "{\n"
                         "    TOKEN *next = tok->next;\n"
                         "    tok->next = next->next;\n"
                         "    free(next->str);\n"
                         "    delete next;\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test3, "" );



    const char test4[] = "static void f()\n"
                         "{\n"
                         "    bool special = false;\n"
                         "    do\n"
                         "    {\n"
                         "        // Special sequence\n"
                         "        if (special)\n"
                         "            special = false;\n"
                         "        else\n"
                         "            special = (c == \'\\\');\n"
                         "    }\n"
                         "    while (special || c != \'\"\');\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test4, "" );



    const char test5[] = "static void f()\n"
                         "{\n"
                         "    int i = 0;\n"
                         "    {\n"
                         "        i+5;\n"
                         "    }\n"
                         "    {\n"
                         "        i+5;\n"
                         "    }\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test5, "" );



    const char test6[] = "static void f()\n"
                         "{\n"
                         "#define F1(x, y, z)     (z ^ (x & (y ^ z)))\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test6, "" );


    const char test7[] = "struct a\n"
                         "{\n"
                         "    int x;\n"
                         "    int y;\n"
                         "};\n";
    check( CheckVariableScope, __LINE__, test7, "" );


    const char test8[] = "static void f()\n"
                         "{\n"
                         "    struct\n"
                         "    {\n"
                         "        int x;\n"
                         "        int y;\n"
                         "    } fred;\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test8, "" );


    const char test9[] = "static void f()\n"
                         "{\n"
                         "    int i;\n"
                         "    while (abc)\n"
                         "    {\n"
                         "        if (cond1)\n"
                         "        {\n"
                         "            i = 2;\n"
                         "        }\n"
                         "        if (cond2)\n"
                         "        {\n"
                         "            f(i);\n"
                         "        }\n"
                         "    }\n"
                         "}\n";
    check( CheckVariableScope, __LINE__, test9, "" );




    const char test10[] = "static void f()\n"
                          "{\n"
                          "    TPoint p1;\n"
                          "    for (i=0;i<10;i++)\n"
                          "    {\n"
                          "        p1=point(i,i);\n"
                          "    }\n"
                          "}\n";
    check( CheckVariableScope, __LINE__, test10, "" );
}
//---------------------------------------------------------------------------

static void fpar_byvalue()
{
    check( CheckConstantFunctionParameter,
           __LINE__,
           "void f(const std::string str);",
           "[test.cpp:1] str is passed by value, it could be passed by reference/pointer instead\n" );

    check( CheckConstantFunctionParameter,
           __LINE__,
           "void f(const int a, const std::vector v, const int b);",
           "[test.cpp:1] v is passed by value, it could be passed by reference/pointer instead\n" );

    check( CheckConstantFunctionParameter,
           __LINE__,
           "class Fred;\n"
           "void f(const Fred f);",
           "[test.cpp:2] f is passed by value, it could be passed by reference/pointer instead\n" );
}
//---------------------------------------------------------------------------

static void unused_struct_member()
{
    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {int i;};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {unsigned int i;};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {int *i;};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {unsigned int *i;};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {int i[10];};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {unsigned int i[10];};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );


    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {int *i[10];};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );

    check( CheckStructMemberUsage,
           __LINE__,
           "struct abc {unsigned int *i[10];};",
           "[test.cpp:1]: struct member 'abc::i' is never read\n" );


}
//---------------------------------------------------------------------------



