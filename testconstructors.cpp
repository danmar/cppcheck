
#include "tokenize.h"
#include "CheckClass.h"
#include "testsuite.h"

#include <sstream>

extern std::ostringstream errout;

class TestConstructors : public TestFixture
{
public:
    TestConstructors() : TestFixture("TestConstructors")
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

        // Check for memory leaks..
        CheckConstructors();
    }

    void run()
    {
        TEST_CASE( simple1 );
        TEST_CASE( simple2 );
        TEST_CASE( simple3 );
        TEST_CASE( simple4 );
    }


    void simple1()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    int i;\n"
           "};\n" );
        std::string actual( errout.str() );
        std::string expected( "[test.cpp:1] The class 'Fred' has no constructor\n" );
        ASSERT_EQUALS( expected, actual );
    }


    void simple2()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred() { }\n"
           "    int i;\n"
           "};\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }


    void simple3()
    {
        check( "class Fred\n"
           "{\n"
           "public:\n"
           "    Fred();\n"
           "    int i;\n"
           "};\n"
           "Fred::Fred()\n"
           "{ }\n" );
        ASSERT_EQUALS( std::string("[test.cpp:7] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }


    void simple4()
    {
        check( "class Fred\n"
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
           "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8] Uninitialized member variable 'Fred::i'\n"), errout.str() );
    }

};

REGISTER_TEST( TestConstructors );
