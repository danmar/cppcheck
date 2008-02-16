
// Unit-testing cppcheck

//---------------------------------------------------------------------------

#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"
#include "CheckBufferOverrun.h"

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------

#define assert_equal(A,B) if (A!=B) { std::cerr << "Failed at line " << line << "\n"; FailCount++; } else { SuccessCount++; }
//---------------------------------------------------------------------------
bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;
//---------------------------------------------------------------------------
static unsigned int FailCount, SuccessCount;
//---------------------------------------------------------------------------
static void buffer_overrun();
//---------------------------------------------------------------------------

int main()
{
    Files.push_back( std::string("test.cpp") );
    buffer_overrun();
    std::cout << "Success Rate: " 
              << SuccessCount 
              << " / " 
              << (SuccessCount + FailCount) 
              << std::endl;
    return 0;
}
//---------------------------------------------------------------------------

static void buffer_overrun_check(const unsigned int line,
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
    CheckBufferOverrun();

    // Check the error messages..
    assert_equal(errout.str(), msg);

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
    buffer_overrun_check( __LINE__, test1, "[test.cpp:5]: Array index out of bounds\n" );



    const char test2[] = "void f()\n"
                         "{\n"
                         "    int val[50];\n"
                         "    for (i = 0; i < 100; i++)\n"
                         "        sum += val[i];\n"
                         "}\n";
    buffer_overrun_check( __LINE__, test2, "[test.cpp:5]: Buffer overrun\n" );



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
    buffer_overrun_check( __LINE__, test3, "" );



    const char test4[] = "void f()\n"
                         "{\n"
                         "    char str[3];\n"
                         "    strcpy(str, \"abc\");\n"
                         "}\n";
    buffer_overrun_check( __LINE__, test4, "[test.cpp:4]: Buffer overrun\n" );



    const char test5[] = "const int SIZE = 10;\n"
                         "void f()\n"
                         "{\n"
                         "    int i[SIZE];\n"
                         "    i[SIZE] = 0;\n"
                         "}\n";
    buffer_overrun_check( __LINE__, test5, "[test.cpp:5]: Array index out of bounds\n" );




    const char test6[] = "void f()\n"
                         "{\n"
                         "    int i[10];\n"
                         "    i[ sizeof(i) - 1 ] = 0;\n"
                         "}\n";
    buffer_overrun_check( __LINE__, test6, "[test.cpp:4]: Array index out of bounds\n" );




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

    buffer_overrun_check( __LINE__, test7, err7 );

}
//---------------------------------------------------------------------------
