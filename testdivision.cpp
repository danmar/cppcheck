
// Check for dangerous division..
// such as "svar / uvar". Treating "svar" as unsigned data is not good


#include "tokenize.h"
#include "CheckOther.h"
#include "MiniCppUnit.h"

#include <sstream>

extern std::ostringstream errout;
extern bool ShowAll;

class TestDivision : public TestFixture<TestDivision>
{
private:
    void check( const char code[] )
    {
        // Tokenize..
        tokens = tokens_back = NULL;
        std::istringstream istr(code);
        TokenizeCode( istr );
        //SimplifyTokenList();  <- this can't be used as it removes 'unsigned'

        // Clear the error buffer..
        errout.str("");

        // Check for memory leaks..
        ShowAll = true;
        CheckUnsignedDivision();
    }

public:
    TEST_FIXTURE( TestDivision )
    {
        TEST_CASE( division1 );
        TEST_CASE( division2 );
        TEST_CASE( division3 );
    }

    void division1()
    {
        check( "void f()\n"
               "{\n"
               "    int ivar = -2;\n"
               "    unsigned int uvar = 2;\n"
               "    return ivar / uvar;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:5]: Warning: Division with signed and unsigned operators\n"), errout.str() );
    }

    void division2()
    {
        check( "void f()\n"
               "{\n"
               "    int ivar = -2;\n"
               "    unsigned int uvar = 2;\n"
               "    return uvar / ivar;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:5]: Warning: Division with signed and unsigned operators\n"), errout.str() );
    }

    void division3()
    {
        check( "typedef int s32;\n"
               "typedef unsigned int u32;\n"
               "void f()\n"
               "{\n"
               "    s32 ivar = -2;\n"
               "    u32 uvar = 2;\n"
               "    return uvar / ivar;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:7]: Warning: Division with signed and unsigned operators\n"), errout.str() );
    }
};

REGISTER_FIXTURE( TestDivision )


