
#include "tokenize.h"
#include "CheckClass.h"
#include "MiniCppUnit.h"

#include <sstream>

extern std::ostringstream errout;

class TestConstructors : public TestFixture<TestConstructors>
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
        CheckConstructors();
    }

public:
    TEST_FIXTURE( TestConstructors )
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
        ASSERT_EQUALS( std::string("[test.cpp:1] The class 'Fred' has no constructor\n"), errout.str() );
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

REGISTER_FIXTURE( TestConstructors )
