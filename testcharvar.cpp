


#include "tokenize.h"
#include "CommonCheck.h"
#include "CheckOther.h"
#include "MiniCppUnit.h"

#include <sstream>

extern std::ostringstream errout;
extern bool ShowAll;

class TestCharVar : public TestFixture<TestCharVar>
{
private:
    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        TokenizeCode( istr );

        // Fill function list
        FillFunctionList(0);

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        ShowAll = true;
        CheckCharVariable();
    }

public:
    TEST_FIXTURE( TestCharVar )
    {
        TEST_CASE( array_index );
        TEST_CASE( bitop );
    }


    void array_index()
    {
        check( "void foo()\n"
               "{\n"
               "    unsigned char ch = 0x80;\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );

        check( "void foo()\n"
               "{\n"
               "    char ch = 0x80;\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Warning - using char variable as array index\n"), errout.str() );

        check( "void foo(char ch)\n"
               "{\n"
               "    buf[ch] = 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:3]: Warning - using char variable as array index\n"), errout.str() );
    }


    void bitop()
    {
        check( "void foo()\n"
               "{\n"
               "    char ch;\n"
               "    result = a | ch;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Warning - using char variable in bit operation\n"), errout.str() );
    }
};

REGISTER_FIXTURE( TestCharVar )

