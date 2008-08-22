
#include "tokenize.h"
#include "CheckMemoryLeak.h"
#include "MiniCppUnit.h"

#include <sstream>

extern std::ostringstream errout;
extern bool ShowAll;

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
        ShowAll = false;
        CheckMemoryLeak();
    }

public:
    TEST_FIXTURE( TestMemleak )
    {
        TEST_CASE( simple1 );
        TEST_CASE( simple2 );
        TEST_CASE( simple3 );
        TEST_CASE( simple4 );
        TEST_CASE( simple5 );
        TEST_CASE( simple6 );
        TEST_CASE( simple7 );

        TEST_CASE( ifelse1 );
        TEST_CASE( ifelse2 );
        TEST_CASE( ifelse3 );
        TEST_CASE( ifelse4 );
        TEST_CASE( ifelse5 );

        TEST_CASE( forwhile1 );
        TEST_CASE( forwhile2 );


        TEST_CASE( switch1 );
        // TODO: TEST_CASE( switch2 );

        TEST_CASE( mismatch1 );

    }

    void simple1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: a\n"), errout.str() );
    }

    void simple2()
    {
        check( "Fred *NewFred()\n"
               "{\n"
               "    Fred *f = new Fred;\n"
               "    return f;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }

    void simple3()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    return (char *)s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple4()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    return 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:4]: Memory leak: s\n"), errout.str() );
    }


    void simple5()
    {
        check( "static char *f()\n"
               "{\n"
               "    struct *str = new strlist;\n"
               "    return &str->s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple6()
    {
        check( "static void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    char *str2 = (char *)str;\n"
               "    free(str2);\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void simple7()
    {
        // A garbage collector may delete f automaticly
        check( "class Fred;\n"
               "void foo()\n"
               "{\n"
               "    Fred *f = new Fred;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }





    void ifelse1()
    {
        check( "void f()\n"
               "{\n"
               "    int *a = new int[10];\n"
               "    if (a)\n"
               "    {\n"
               "        delete [] a;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse2()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    if (somecondition)\n"
               "    {\n"
               "        return;\n"
               "    }\n"
               "    free(str);\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:6]: Memory leak: str\n"), errout.str() );
    }


    void ifelse3()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = strdup(\"hello\");\n"
               "    if (a==b)\n"
               "    {\n"
               "        free(str);\n"
               "        return;\n"
               "    }\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:9]: Memory leak: str\n"), errout.str() );
    }


    void ifelse4()
    {
        check( "void f()\n"
               "{\n"
               "    char *str = new char[10];\n"
               "    if (a==b)\n"
               "    {\n"
               "        delete [] str;\n"
               "        return;\n"
               "    }\n"
               "    delete [] str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse5()
    {
        check( "void f()\n"
               "{\n"
               "    char *str;\n"
               "    if (somecondition)\n"
               "    {\n"
               "        str = new char[100];\n"
               "    }\n"
               "    else\n"
               "    {\n"
               "        return;\n"
               "    }\n"
               "    delete [] str;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse6()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[100];\n"
               "    if ( a == b )\n"
               "    {\n"
               "        return s;\n"
               "    }\n"
               "    return NULL;\n"
               "}\n" );
        ASSERT_EQUALS( std::string("[test.cpp:8]: Memory leak: s\n"), errout.str() );
    }


    void ifelse7()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s;\n"
               "    if ( abc )\n"
               "    {\n"
               "        s = new char[10];\n"
               "    }\n"
               "    return s;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void ifelse8()
    {
        check( "static char *f()\n"
               "{\n"
               "    char *s = new char[10];\n"
               "    if ( s )\n"
               "    {\n"
               "        return s;\n"
               "    }\n"
               "    return 0;\n"
               "}\n" );
        ASSERT_EQUALS( std::string(""), errout.str() );
    }





    void forwhile1()
    {
        check("void f()\n"
              "{\n"
              "    char *str = strdup(\"hello\");\n"
              "    while (condition)\n"
              "    {\n"
              "        if (condition)\n"
              "        {\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "    free(str);\n"
              "}\n");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void forwhile2()
    {
        check("void f()\n"
              "{\n"
              "    for (int i = 0; i < j; i++)\n"
              "    {\n"
              "        char *str = strdup(\"hello\");\n"
              "        if (condition)\n"
              "            continue;\n"
              "        free(str);\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:7]: Memory leak: str\n"), errout.str() );
    }








    void switch1()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    switch (abc)\n"
              "    {\n"
              "        case 1:\n"
              "            break;\n"
              "    };\n"
              "    delete [] str;\n"
              "}\n");
        ASSERT_EQUALS( std::string(""), errout.str() );
    }


    void switch2()
    {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    switch (abc)\n"
              "    {\n"
              "        case 1:\n"
              "            delete [] str;\n"
              "            break;\n"
              "        default:\n"
              "            break;\n"
              "    };\n"
              "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:12]: Memory leak"), errout.str() );
    }







    void mismatch1()
    {
        check( "void f()\n"
           "{\n"
           "    int *a = new int[10];\n"
           "    free(a);\n"
           "}\n");
        ASSERT_EQUALS( std::string("[test.cpp:4]: Mismatching allocation and deallocation: a\n"), errout.str() );
    }



};

REGISTER_FIXTURE( TestMemleak )


