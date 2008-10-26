
#include <sstream>


class TestFixture
{
private:
    static std::ostringstream errmsg;
    static unsigned int countTests;

protected:
    std::string classname;

    virtual void run()
    { }

    bool runTest(const char testname[]);
    void assertFail(const char *filename, int linenr);

public:
    TestFixture(const std::string &_name);
    virtual ~TestFixture() { }

    static void printTests();
    static void runTests(const char cmd[]);
};


#define TEST_CASE( NAME )  if ( runTest(#NAME) ) NAME ();
#define ASSERT_EQUALS( EXPECTED , ACTUAL )  if (EXPECTED!=ACTUAL) assertFail(__FILE__, __LINE__);
#define REGISTER_TEST( CLASSNAME ) namespace { CLASSNAME instance; }

