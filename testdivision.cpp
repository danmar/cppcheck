
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
        TEST_CASE( division4 );
        TEST_CASE( division5 );
        TEST_CASE( division6 );
        TEST_CASE( division7 );
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

    void division4()
    {
        check( "void f1()\n"
               "{\n"
               "    int i1;\n"
               "}\n"
               "\n"
               "void f2(unsigned int i1)\n"
               "{\n"
               "    unsigned int i2;\n"
               "    result = i2 / i1;\n"
               );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void division5()
    {
        check( "#define USER_HASH (16)\n"
               "void foo()\n"
               "{\n"
               "    unsigned int val = 32;\n"
               "    val = val / USER_HASH;\n"
               );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void division6()
    {
        check( "void foo()\n"
               "{\n"
               "    unsigned int val = 32;\n"
               "    int i = val / -2;\n"
               );
        ASSERT_EQUALS( std::string("[test.cpp:4]: The division result will be wrong\n"), errout.str() );
    }

    void division7()
    {
        check( "void foo()\n"
               "{\n"
               "    unsigned int val = 32;\n"
               "    int i = -96 / val;\n"
               );
        ASSERT_EQUALS( std::string("[test.cpp:4]: The division result will be wrong\n"), errout.str() );
    }
};

REGISTER_FIXTURE( TestDivision )


