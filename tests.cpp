
// Unit-testing cppcheck

#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"
#include "CheckBufferOverrun.h"

#include <iostream>
#include <sstream>

#define assert_equal(A,B) if (A!=B) { std::cerr << "Failed at line " << __LINE__ << "\n"; FailCount++; } else { SuccessCount++; }

bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;

static unsigned int FailCount, SuccessCount;

static void buffer_overrun_1();

int main()
{
    Files.push_back( std::string("test.cpp") );
    buffer_overrun_1();
    std::cout << "Success Rate: " 
              << SuccessCount 
              << " / " 
              << (SuccessCount + FailCount) 
              << std::endl;
    return 0;
}

static void buffer_overrun_1()
{
    const char code[] = "void f()\n"
                        "{\n"
                        "    char str[0x10];\n"
                        "    str[15] = 0;\n"
                        "    str[16] = 0;\n"
                        "}\n";

    // Tokenize..
    tokens = tokens_back = NULL;
    std::istringstream istr(code);
    TokenizeCode( istr );
    SimplifyTokenList();

    errout.str("");
    CheckBufferOverrun();

    std::string err = errout.str();
    assert_equal( errout.str(), "[test.cpp:5]: Array index out of bounds\n" );
}



