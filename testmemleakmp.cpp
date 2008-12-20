/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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



#define UNIT_TESTING
#include "tokenize.h"
#include "checkmemoryleak.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestMemleakMultiPass : public TestFixture
{
public:
    TestMemleakMultiPass() : TestFixture("TestMemleakMultiPass")
    { }

private:

    void run()
    {
        // TODO TEST_CASE( alloc1 );
    }

    // Check that base classes have virtual destructors
    std::string functionCode(const char code[], const char funcname[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize( istr, "test.cpp" );
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckMemoryLeakClass checkMemoryLeak( &tokenizer, settings, this );
        TOKEN *tok = checkMemoryLeak.functionCode(funcname);

        // Return tokens..
        std::ostringstream ret;
        for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
            ret << tok2->str() << " ";
        while ( tok )
        {
            TOKEN *tok_ = tok;
            tok = tok->next();
            delete tok_;
        }
        return ret.str();
    }

    void alloc1()
    {
        const char code[] = "char *f()\n"
                            "{\n"
                            "    return malloc(100);\n"
                            "}\n";
        ASSERT_EQUALS( "alloc ;", functionCode(code, "f") );
    }

};

REGISTER_TEST( TestMemleakMultiPass )
