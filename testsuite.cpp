
#include "testsuite.h"

#include <iostream>
#include <list>



/**
 * TestRegistry
 **/

class TestRegistry
{
private:
    std::list<TestSuite *> _tests;

public:
    static TestRegistry &theInstance()
    {
        static TestRegistry testreg;
        return testreg;
    }

    void addTest( TestSuite *t )
    {
        _tests.push_back( t );
    }

    void removeTest( TestSuite *t )
    {
        _tests.remove( t );
    }

    const std::list<TestSuite *> &tests() const
    { return _tests; }
};




/**
 * TestSuite
 **/

TestSuite::TestSuite(const std::string &_name) : classname(_name)
{
    TestRegistry::theInstance().addTest(this);
}

TestSuite::~TestSuite()
{
    TestRegistry::theInstance().removeTest(this);
}

void TestSuite::printTests()
{
    const std::list<TestSuite *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestSuite *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        std::cout << (*it)->classname << std::endl;
    }
}

void TestSuite::runTests()
{
    const std::list<TestSuite *> &tests = TestRegistry::theInstance().tests();

    for ( std::list<TestSuite *>::const_iterator it = tests.begin(); it != tests.end(); ++it )
    {
        (*it)->run();
    }
}

