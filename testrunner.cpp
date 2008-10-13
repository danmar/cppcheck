
#include "testsuite.h"
#include <string>
#include <vector>


bool ShowAll = false;
bool CheckCodingStyle = true;
extern std::vector<std::string> Files;


int main()
{
    Files.push_back( "test.cpp" );
    TestFixture::runTests();
    return 0;
}

