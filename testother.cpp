
// Check for unused variables..

#include "testsuite.h"
#include "tokenize.h"
#include "CheckOther.h"

#include <sstream>

extern std::ostringstream errout;

class TestOther : public TestFixture
{
public:
    TestOther() : TestFixture("TestOther")
    { }

private:
    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        TokenizeCode( istr );
        SimplifyTokenList();

        // Clear the error buffer..
        errout.str("");

        // Check for unused variables..
        CheckIncompleteStatement();
    }

    void run()
    {
        TEST_CASE( test1 );
        TEST_CASE( test2 );
    }

    void test1()
    {
        check( "void foo()\n"
               "{\n"
               "    const char def[] =\n"
               "#ifdef ABC\n"
               "    \"abc\";\n"
               "#else\n"
               "    \"not abc\";\n"
               "#endif\n"
               "}\n" );

        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void test2()
    {
        // Todo: remove the ';' before the string

        check( "void foo()\n"
               "{\n"
               "    ;\"abc\";\n"
               "}\n" );

        ASSERT_EQUALS( std::string("[test.cpp:3]: Redundant code: Found a statement that begins with string constant\n"), errout.str() );
    }
};

REGISTER_TEST( TestOther )


