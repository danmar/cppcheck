/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki and Reijo Tomperi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

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
