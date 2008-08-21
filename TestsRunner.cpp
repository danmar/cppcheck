#include "MiniCppUnit.h"
#include "tokenize.h"

bool ShowAll = true;
bool CheckCodingStyle = true;
bool Debug = false;

int main()
{
    // Provide a dummy filename for the error messages
    Files.push_back( std::string("test.cpp") );

	return TestFixtureFactory::theInstance().runTests() ? 0 : -1;
}	
