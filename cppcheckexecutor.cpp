#include "cppcheckexecutor.h"
#include "cppcheck.h"
#include <iostream>

CppCheckExecutor::CppCheckExecutor()
{
    //ctor
}

CppCheckExecutor::~CppCheckExecutor()
{
    //dtor
}

void CppCheckExecutor::check( int argc, char* argv[] )
{
    CppCheck cppCheck( *this );
    std::string result = cppCheck.parseFromArgs( argc, argv );
    if( result.length() == 0 )
        cppCheck.check();
    else
        std::cout << result;
}

void CppCheckExecutor::reportErr( const std::string &errmsg)
{
    std::cerr << errmsg << std::endl;
}

void CppCheckExecutor::reportOut( const std::string &outmsg)
{
    std::cout << outmsg << std::endl;
}
