
#include <iostream>
#include <string>


class TestSuite
{
protected:
    std::string classname;

    virtual void run() = 0;

public:
    TestSuite(const std::string &_name);
    ~TestSuite();

    static void printTests();
    static void runTests();
};


#define TEST_CASE( NAME )  std::cout << classname << "::" << #NAME << std::endl; NAME ();

/*
#define ASSERT_EQUALS( EXPECTED , ACTUAL )
*/

