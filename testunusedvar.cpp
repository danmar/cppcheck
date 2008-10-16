
// Check for unused variables..

#include "testsuite.h"
#include "tokenize.h"
#include "CheckOther.h"

#include <sstream>

extern std::ostringstream errout;

class TestUnusedVar : public TestFixture
{
public:
    TestUnusedVar() : TestFixture("TestUnusedVar")
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
        CheckStructMemberUsage();
    }

    void run()
    {
        TEST_CASE( structmember1 );
    }

    void structmember1()
    {
        check( "struct abc\n"
               "{\n"
               "    int a;\n"
               "    int b;\n"
               "    int c;\n"
               "};\n" );
        ASSERT_EQUALS( std::string("[test.cpp:2]: struct member 'abc::a' is never read\n"
                                   "[test.cpp:3]: struct member 'abc::b' is never read\n"
                                   "[test.cpp:4]: struct member 'abc::c' is never read\n"), errout.str() );
    }

};

REGISTER_TEST( TestUnusedVar )


