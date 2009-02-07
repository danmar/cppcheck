/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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


#include "../src/tokenize.h"
#include "../src/checkmemoryleak.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestMemleakMultiPass : public TestFixture
{
public:
    TestMemleakMultiPass() : TestFixture("TestMemleakMultiPass")
    { }

    class OurCheckMemoryLeakClass : public CheckMemoryLeakClass
    {
    public:
        OurCheckMemoryLeakClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
                : CheckMemoryLeakClass(tokenizer, settings, errorLogger)
        {
        }

        Token *functionParameterCode(const Token *ftok, int parameter)
        {
            return CheckMemoryLeakClass::functionParameterCode(ftok, parameter);
        }
    };

private:

    void run()
    {
        TEST_CASE(param1);
    }

    void param1()
    {
        const char code[] = "void f(char *s)\n"
                            "{\n"
                            "    ;\n"
                            "}\n";

        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        OurCheckMemoryLeakClass checkMemoryLeak(&tokenizer, settings, this);
        Token *tok = checkMemoryLeak.functionParameterCode(tokenizer.tokens(), 1);

        // Compare tokens..
        std::string s;
        for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            s += tok2->str() + " ";
        ASSERT_EQUALS("; } ", s);
    }

};

REGISTER_TEST(TestMemleakMultiPass)
