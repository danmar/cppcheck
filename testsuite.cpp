
#include "testsuite.h"

#include <iostream>
#include <list>



/**
 * TestRegistry
 **/

class TestRegistry
{
private:
    std::list<TestFixture *> _tests;

public:
    static TestRegistry &theInstance()
    {
        static TestRegistry testreg;
        return testreg;
    }

    void addTest( TestFixture *t )
    {
        _tests.push_back( t );
    }

    void removeTest( TestFixture *t )
    {
        _tests.remove( t );
    }

    const std::list<TestFixture *> &tests() const
    {
        return _tests;
    }
};




/**
 * TestFixture
 **/

std::ostringstream TestFixture::errmsg;
unsigned int       TestFixture::countTests;

TestFixture::TestFixture(const std::string &_name) : classname(_name)
{
    TestRegistry::theInstance().addTest(this);
}

TestFixture::~TestFixture()
{
    TestRegistry::theInstance().removeTest(this);
}

bool TestFixture::runTest(const char testname[])
{
    countTests++;
    std::cout << classname << "::" << testname << "\n";
    return true;
}

void TestFixture::assertFail(const char *filename, int linenr)
{
    errmsg << "Assertion failed in " << filename << " at line " << linenr << std::endl;
}

void TestFixture::printTests()
{
    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        std::cout << (*it)->classname << std::endl;
    }
}

void TestFixture::runTests()
{
    countTests = 0;
    errmsg.str("");

    const std::list<TestFixture *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestFixture *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        (*it)->run();
    }

    std::cout << "\n\nTesting Complete\nNumber of tests: " << countTests << "\n";

    std::cerr << errmsg.str();
}



