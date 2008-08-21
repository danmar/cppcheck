
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
    // There are 3 sections..
    // 1. No errors
    // 2. Array index out of bounds
    // 3. Buffer overrun


    const char *code;


    ////////////////////////////////////////////////
    // NO ERRORS
    ////////////////////////////////////////////////

    code = "void f()\n"
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
    check( CheckBufferOverrun, __LINE__, code, "" );


    code = "void f1(char *str)\n"
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

    check( CheckBufferOverrun, __LINE__, code, "" );




    code = "static void f()\n"
           "{\n"
           "    char data[1];\n"
           "    return abc.data[1];\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "" );


    // TODO
    /*
    code = "static void memclr( char *data, const int bytes )\n"
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
    check( CheckBufferOverrun, __LINE__, code, "" );
    */




    ////////////////////////////////////////////////
    // Array index out of bounds
    ////////////////////////////////////////////////

    code = "void f()\n"
           "{\n"
           "    char str[0x10];\n"
           "    str[15] = 0;\n"
           "    str[16] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:5]: Array index out of bounds\n" );


    code = "void f()\n"
           "{\n"
           "    char *str = new char[0x10];\n"
           "    str[15] = 0;\n"
           "    str[16] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:5]: Array index out of bounds\n" );


    code = "void f()\n"
           "{\n"
           "    int val[50];\n"
           "    for (i = 0; i < 100; i++)\n"
           "        sum += val[i];\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:5]: Buffer overrun\n" );


    code = "const int SIZE = 10;\n"
           "void f()\n"
           "{\n"
           "    int i[SIZE];\n"
           "    i[SIZE] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:5]: Array index out of bounds\n" );


    code = "void f()\n"
           "{\n"
           "    int i[10];\n"
           "    i[ sizeof(i) - 1 ] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:4]: Array index out of bounds\n" );



    code = "struct ABC\n"
           "{\n"
           "    char str[10];\n"
           "};\n"
           "\n"
           "static void f()\n"
           "{\n"
           "    struct ABC abc;\n"
           "    abc.str[10] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:9]: Array index out of bounds\n" );



    code = "struct ABC\n"
           "{\n"
           "    char str[10];\n"
           "};\n"
           "\n"
           "static void f(ABC *abc)\n"
           "{\n"
           "    abc->str[10] = 0;\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:8]: Array index out of bounds\n" );


    code = "const int SIZE = 10;\n"
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
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:11]: Array index out of bounds\n" );




    code = "static void memclr( char *data )\n"
           "{\n"
           "    data[10] = 0;\n"
           "}\n"
           "\n"
           "static void f()\n"
           "{\n"
           "    char str[5];\n"
           "    memclr( str );   // ERROR\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:9] -> [test.cpp:3]: Array index out of bounds\n" );


    code = "struct ABC\n"
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
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:13] -> [test.cpp:8]: Array index out of bounds\n" );



    code = "class ABC\n"
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
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:12]: Array index out of bounds\n" );



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



    ////////////////////////////////////////////////
    // Buffer overrun
    ////////////////////////////////////////////////


    code = "void f()\n"
           "{\n"
           "    char str[3];\n"
           "    strcpy(str, \"abc\");\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:4]: Buffer overrun\n" );


    code = "struct ABC\n"
           "{\n"
           "    char str[5];\n"
           "};\n"
           "\n"
           "static void f(ABC *abc)\n"
           "{\n"
           "    strcpy( abc->str, \"abcdef\" );\n"
           "}\n";
    check( CheckBufferOverrun, __LINE__, code, "[test.cpp:8]: Buffer overrun\n" );


}
//---------------------------------------------------------------------------

static void constructors()
{
    // Test1: No constructor
    // Test2: embedded constructor, uninitialized variable
    // Test3: Uninitialized variable
    // Test4: multiple constructors, uninitialized variable

    const char *code;

    code = "class Fred\n"
           "{\n"
           "public:\n"
           "    int i;\n"
           "};\n";
    check( CheckConstructors, __LINE__, code, "[test.cpp:1] The class 'Fred' has no constructor\n" );




    code = "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred() { }\n"
           "    int i;\n"
           "};\n";
    check( CheckConstructors, __LINE__, code, "[test.cpp:4] Uninitialized member variable 'Fred::i'\n" );



    code = "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred();\n"
           "    int i;\n"
           "};\n"
           "Fred::Fred()\n"
           "{ }\n";
    check( CheckConstructors, __LINE__, code, "[test.cpp:7] Uninitialized member variable 'Fred::i'\n" );


    code = "class Fred\n"
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
    check( CheckConstructors, __LINE__, code, "[test.cpp:8] Uninitialized member variable 'Fred::i'\n" );

}
//---------------------------------------------------------------------------

static void operator_eq()
{
    const char *code;

    code = "class Fred\n"
           "{\n"
           "public:\n"
           "    void operator=(const int &value);\n"
           "};\n";
    check( CheckOperatorEq1, __LINE__, code, "[test.cpp:4]: 'operator=' should return something\n" );

}
//---------------------------------------------------------------------------

static void check_(void (chk)(),
                   const unsigned int line,
                   const char code[],
                   const char msg[])
{
    ShowAll = false;
    check( chk, line, code, msg );
    ShowAll = true;
    check( chk, line, code, msg );
}

static void memleak_in_function()
{
    // There are 2 sections:
    // * Simple testcases
    // * if else
    // * for/while
    // * switch
    // * mismatching allocation and deallocation
    // * garbage collection
    // * arrays
    // * struct members
    // * function calls




    ////////////////////////////////////////////////
    // for/while
    ////////////////////////////////////////////////


    code = "void f()\n"
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
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );


    code = "void f()\n"
           "{\n"
           "    for (int i = 0; i < j; i++)\n"
           "    {\n"
           "        char *str = strdup(\"hello\");\n"
           "        if (condition)\n"
           "            continue;\n"
           "        free(str);\n"
           "    }\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "[test.cpp:7]: Memory leak: str\n" );







    ////////////////////////////////////////////////
    // switch
    ////////////////////////////////////////////////

    code = "void f()\n"
           "{\n"
           "    char *str = new char[10];\n"
           "    switch (abc)\n"
           "    {\n"
           "        case 1:\n"
           "            break;\n"
           "    };\n"
           "    delete [] str;\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );






    ////////////////////////////////////////////////
    // mismatching allocation and deallocation
    ////////////////////////////////////////////////

    code = "void f()\n"
           "{\n"
           "    int *a = new int[10];\n"
           "    free(a);\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "[test.cpp:4]: Mismatching allocation and deallocation: a\n" );







    ////////////////////////////////////////////////
    // Garbage collection
    ////////////////////////////////////////////////


    code = "static char *f()\n"
           "{\n"
           "    Fred *fred = new Fred;\n"
           "    // fred is deleted automaticly\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );


    code = "struct abc\n"
           "{\n"
           "    int a;\n"
           "    int b;\n"
           "    int c;\n"
           "}\n"
           "\n"
           "static void f()\n"
           "{\n"
           "    struct abc *abc1 = new abc;\n"
           "    p = &abc1->a;\n"       // p may be part of a garbage collector
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );






    ////////////////////////////////////////////////
    // arrays
    ////////////////////////////////////////////////


    /*  TODO
    code = "static void f()\n"
           "{\n"
           "    char *str[10];\n"
           "    str[0] = strdup(\"hello\");\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "[test.cpp:3]: Memory leak: str[0]\n" );
    */



    ////////////////////////////////////////////////
    // struct members
    ////////////////////////////////////////////////


    code = "static char *f()\n"
           "{\n"
           "    Fred *fred = new Fred;\n"
           "    free( fred->Name );\n"
           "}\n";
    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:5]: Memory leak: fred\n" );


    /* TODO
    code = "struct Fred\n"
           "{\n"
           "    char *str;\n"
           "}\n"
           "\n"
           "void f()\n"
           "{\n"
           "    Fred f;\n"
           "    f.str = strdup(\"aa\");\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "[test.cpp:9]: Memory leak: f.str\n" );
    */


    code = "static struct fib6_table *fib6_alloc_table(struct net *net, u32 id)\n"
           "{\n"
           "	struct fib6_table *table;\n"
           "\n"
           "	table = kzalloc(sizeof(*table), GFP_ATOMIC);\n"
           "	if (table != NULL) {\n"
           "		table->tb6_id = id;\n"
           "		table->tb6_root.leaf = net->ipv6.ip6_null_entry;\n"
           "		table->tb6_root.fn_flags = RTN_ROOT | RTN_TL_ROOT | RTN_RTINFO;\n"
           "	}\n"
           "\n"
           "	return table;\n"
           "}\n";
    ShowAll = false;
    check( CheckMemoryLeak, __LINE__, code, "" );
    ShowAll = true;




    ////////////////////////////////////////////////
    // function calls
    ////////////////////////////////////////////////


    code = "static char *dmalloc()\n"
           "{\n"
           "    char *p = new char[100];\n"
           "    return p;\n"
           "}\n"
           "static void f()\n"
           "{\n"
           "    char *p = dmalloc();\n"
           "}\n";
    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:9]: Memory leak: p\n" );


    code = "static char *dmalloc()\n"
           "{\n"
           "    char *p = new char[100];\n"
           "    return p;\n"
           "}\n"
           "static void f()\n"
           "{\n"
           "    char *p = dmalloc();\n"
           "    delete p;\n"
           "}\n";
    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:9]: Mismatching allocation and deallocation: p\n" );


    code = "static void foo(const char *str)\n"
           "{ }\n"
           "\n"
           "static void f()\n"
           "{\n"
           "    char *p = new char[100];\n"
           "    foo(p);\n"
           "}\n";
    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:8]: Memory leak: p\n" );


    code = "static void f()\n"
           "{\n"
           "    char *p = new char[100];\n"
           "    foo(p);\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );


    code = "static void f()\n"
           "{\n"
           "    char *p = new char[100];\n"
           "    foo.add(p);\n"
           "}\n";
    check_( CheckMemoryLeak, __LINE__, code, "" );

}
//---------------------------------------------------------------------------

static void memleak_in_class()
{


    const char *code;

    code = "class Fred\n"
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

    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:1]: Memory leak: Fred::str1\n" );




    code = "class Fred\n"
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

    check( CheckMemoryLeak, __LINE__, code, "[test.cpp:17]: Mismatching allocation and deallocation: Fred::str1\n" );




/*   TODO
    code = "class Fred\n"
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

    check( CheckMemoryLeak, __LINE__, code, "Memory leak for 'Fred::str'\n" );
*/


}
//---------------------------------------------------------------------------

static void division()
{

    const char *code;

    code = "void f()\n"
           "{\n"
           "    int ivar = -2;\n"
           "    unsigned int uvar = 2;\n"
           "    return ivar / uvar;\n"
           "}\n";
    check( CheckUnsignedDivision, __LINE__, code, "[test.cpp:5]: If the result is negative it will be wrong because an operand is unsigned.\n" );


    code = "void f()\n"
           "{\n"
           "    int ivar = -2;\n"
           "    unsigned int uvar = 2;\n"
           "    return uvar / ivar;\n"
           "}\n";
    check( CheckUnsignedDivision, __LINE__, code, "[test.cpp:5]: If the result is negative it will be wrong because an operand is unsigned.\n" );

}
//---------------------------------------------------------------------------

static void variable_scope()
{

    const char *code;

/* TODO
    // Unused private member variable...
    code = "class Fred\n"
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
    code = "void f()\n"
           "{\n"
           "    int i;\n"
           "    if (abc)\n"
           "    {\n"
           "        i = 1;\n"
           "    }\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "[test.cpp:3] The scope of the variable 'i' can be limited\n" );


    code = "static void DeleteNextToken(TOKEN *tok)\n"
           "{\n"
           "    TOKEN *next = tok->next;\n"
           "    tok->next = next->next;\n"
           "    free(next->str);\n"
           "    delete next;\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "" );



    code = "static void f()\n"
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
    check( CheckVariableScope, __LINE__, code, "" );



    code = "static void f()\n"
           "{\n"
           "    int i = 0;\n"
           "    {\n"
           "        i+5;\n"
           "    }\n"
           "    {\n"
           "        i+5;\n"
           "    }\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "" );



    code = "static void f()\n"
           "{\n"
           "#define F1(x, y, z)     (z ^ (x & (y ^ z)))\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "" );


    code = "struct a\n"
           "{\n"
           "    int x;\n"
           "    int y;\n"
           "};\n";
    check( CheckVariableScope, __LINE__, code, "" );


    code = "static void f()\n"
           "{\n"
           "    struct\n"
           "    {\n"
           "        int x;\n"
           "        int y;\n"
           "    } fred;\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "" );


    code = "static void f()\n"
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
    check( CheckVariableScope, __LINE__, code, "" );




    code = "static void f()\n"
           "{\n"
           "    TPoint p1;\n"
           "    for (i=0;i<10;i++)\n"
           "    {\n"
           "        p1=point(i,i);\n"
           "    }\n"
           "}\n";
    check( CheckVariableScope, __LINE__, code, "" );
}
//---------------------------------------------------------------------------

static void fpar_byvalue()
{
    check( CheckConstantFunctionParameter,
           __LINE__,
           "void f(const std::string str);",
           "[test.cpp:1] str is passed by value, it could be passed by reference/pointer instead\n" );

/*  TODO
    check( CheckConstantFunctionParameter,
           __LINE__,
           "void f(const int a, const std::vector<int> v, const int b);",
           "[test.cpp:1] v is passed by value, it could be passed by reference/pointer instead\n" );
*/

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



