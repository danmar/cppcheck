/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "tokenize.h"
#include "checkexceptionsafety.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestExceptionSafety : public TestFixture {
public:
    TestExceptionSafety() : TestFixture("TestExceptionSafety") {
    }

private:

    void run() {
        TEST_CASE(destructors);
        TEST_CASE(deallocThrow1);
        TEST_CASE(deallocThrow2);
        TEST_CASE(deallocThrow3);
        TEST_CASE(rethrowCopy1);
        TEST_CASE(rethrowCopy2);
        TEST_CASE(rethrowCopy3);
        TEST_CASE(rethrowCopy4);
        TEST_CASE(rethrowCopy5);
        TEST_CASE(catchExceptionByValue);
    }

    void check(const char code[], bool inconclusive = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("all");
        settings.inconclusive = inconclusive;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check char variable usage..
        CheckExceptionSafety checkExceptionSafety(&tokenizer, &settings, this);
        checkExceptionSafety.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void destructors() {
        check("class x {\n"
              "    ~x() {\n"
              "        throw e;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (error) Exception thrown in destructor.\n", errout.str());

        check("class x {\n"
              "    ~x();\n"
              "};\n"
              "x::~x() {\n"
              "    throw e;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Exception thrown in destructor.\n", errout.str());

        check("x::~x() {\n"
              "    throw e;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Exception thrown in destructor.\n", "", errout.str());

        // #3858 - throwing exception in try block in destructor.
        check("class x {\n"
              "    ~x() {\n"
              "        try {\n"
              "            throw e;\n"
              "        } catch (...) {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void deallocThrow1() {
        check("int * p;\n"
              "void f(int x) {\n"
              "    delete p;\n"
              "    if (x)\n"
              "        throw 123;\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory.\n", errout.str());

        check("void f() {\n"
              "    static int* p = foo;\n"
              "    delete p;\n"
              "    if (foo)\n"
              "        throw 1;\n"
              "    p = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory.\n", errout.str());
    }

    void deallocThrow2() {
        check("void f() {\n"
              "    int* p = 0;\n"
              "    delete p;\n"
              "    if (foo)\n"
              "        throw 1;\n"
              "    p = new int;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    reset(p);\n"
              "    throw 1;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }

    void deallocThrow3() {
        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    throw 1;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    static int* p = 0;\n"
              "    delete p;\n"
              "    throw 1;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:4]: (warning) Exception thrown in invalid state, 'p' points at deallocated memory.\n", errout.str());
    }

    void rethrowCopy1() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception.\n", errout.str());
    }

    void rethrowCopy2() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(exception& err)\n"
              "    {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception.\n", errout.str());
    }

    void rethrowCopy3() {
        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(std::runtime_error& err) {\n"
              "        throw err;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception.\n", errout.str());
    }

    void rethrowCopy4() {
        check("void f() {\n"
              "    try\n"
              "    {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& err)\n"
              "    {\n"
              "        exception err2;\n"
              "        throw err2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void rethrowCopy5() {
        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& outer) {\n"
              "        try {\n"
              "           foo(outer);\n"
              "        }\n"
              "        catch(const exception& inner) {\n"
              "            throw inner;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (style) Throwing a copy of the caught exception instead of rethrowing the original exception.\n", errout.str());

        check("void f() {\n"
              "    try {\n"
              "       foo();\n"
              "    }\n"
              "    catch(const exception& outer) {\n"
              "        try {\n"
              "           foo(outer);\n"
              "        }\n"
              "        catch(const exception& inner) {\n"
              "            throw outer;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void catchExceptionByValue() {
        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch( ::std::exception err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Exception should be caught by reference.\n", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(const exception err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Exception should be caught by reference.\n", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch( ::std::exception& err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(exception* err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(const exception& err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(int err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    try {\n"
              "        bar();\n"
              "    }\n"
              "    catch(exception* const err) {\n"
              "        foo(err);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestExceptionSafety)
