
#include "tokenize.h"
#include "CheckMemoryLeak.h"
#include "MiniCppUnit.h"

#include <sstream>

extern std::ostringstream errout;

class TestMemleak : public TestFixture<TestMemleak>
{
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

        // Check for memory leaks..
        CheckMemoryLeak();
    }

public:
    TEST_FIXTURE( TestMemleak )
    {
        TEST_CASE( simple1 );
    }

    void simple1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: a\n"), errout.str() );
    }
};

REGISTER_FIXTURE( TestMemleak )


