/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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


#include "checkexceptionsafety.h"
#include "errortypes.h"
#include "settings.h"
#include "fixture.h"
#include "tokenize.h"

#include <sstream> // IWYU pragma: keep

class TestExceptionSafety : public TestFixture {
public:
    TestExceptionSafety() : TestFixture("TestExceptionSafety") {}

private:
    Settings settings;

    void run() override {
        settings.severity.fill();

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
        TEST_CASE(noexceptThrow);
        TEST_CASE(nothrowThrow);
        TEST_CASE(unhandledExceptionSpecification1); // #4800
        TEST_CASE(unhandledExceptionSpecification2);
        TEST_CASE(unhandledExceptionSpecification3);
        TEST_CASE(nothrowAttributeThrow);
        TEST_CASE(nothrowAttributeThrow2); // #5703
        TEST_CASE(nothrowDeclspecThrow);
        TEST_CASE(rethrowNoCurrentException1);
        TEST_CASE(rethrowNoCurrentException2);
        TEST_CASE(rethrowNoCurrentException3);
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], bool inconclusive = false, const Settings *s = nullptr) {
        // Clear the error buffer..
        errout.str("");

        const Settings settings1 = settingsBuilder(s ? *s : settings).certainty(Certainty::inconclusive, inconclusive).build();

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

        // Check char variable usage..
        runChecks<CheckExceptionSafety>(tokenizer, this);
    }

    void destructors() {
        check("class x {\n"
              "    ~x() {\n"
              "        throw e;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Class x is not safe, destructor throws exception\n", errout.str());

        check("class x {\n"
              "    ~x();\n"
              "};\n"
              "x::~x() {\n"
              "    throw e;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Class x is not safe, destructor throws exception\n", errout.str());

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

        check("class x {\n"
              "    ~x() {\n"
              "        if(!std::uncaught_exception()) {\n"
              "            throw e;\n"
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

    void noexceptThrow() {
        check("void func1() noexcept(false) { try {} catch(...) {;} throw 1; }\n"
              "void func2() noexcept { throw 1; }\n"
              "void func3() noexcept(true) { throw 1; }\n"
              "void func4() noexcept(false) { throw 1; }\n"
              "void func5() noexcept(true) { func1(); }\n"
              "void func6() noexcept(false) { func1(); }");
        ASSERT_EQUALS("[test.cpp:2]: (error) Exception thrown in function declared not to throw exceptions.\n"
                      "[test.cpp:3]: (error) Exception thrown in function declared not to throw exceptions.\n"
                      "[test.cpp:5]: (error) Exception thrown in function declared not to throw exceptions.\n", errout.str());

        // avoid false positives
        check("const char *func() noexcept { return 0; }\n"
              "const char *func1() noexcept { try { throw 1; } catch(...) {} return 0; }");
        ASSERT_EQUALS("", errout.str());
    }

    void nothrowThrow() {
        check("void func1() throw(int) { try {;} catch(...) { throw 1; } ; }\n"
              "void func2() throw() { throw 1; }\n"
              "void func3() throw(int) { throw 1; }\n"
              "void func4() throw() { func1(); }\n"
              "void func5() throw(int) { func1(); }");
        ASSERT_EQUALS("[test.cpp:2]: (error) Exception thrown in function declared not to throw exceptions.\n"
                      "[test.cpp:4]: (error) Exception thrown in function declared not to throw exceptions.\n", errout.str());

        // avoid false positives
        check("const char *func() throw() { return 0; }");
        ASSERT_EQUALS("", errout.str());
    }

    void unhandledExceptionSpecification1() { // #4800
        check("void myThrowingFoo() throw(MyException) {\n"
              "  throw MyException();\n"
              "}\n"
              "void myNonCatchingFoo() {\n"
              "  myThrowingFoo();\n"
              "}\n"
              "void myCatchingFoo() {\n"
              "  try {\n"
              "    myThrowingFoo();\n"
              "  } catch(MyException &) {}\n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:1]: (style, inconclusive) Unhandled exception specification when calling function myThrowingFoo().\n", errout.str());
    }

    void unhandledExceptionSpecification2() {
        check("void f() const throw (std::runtime_error);\n"
              "int main()\n"
              "{\n"
              "    f();\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void unhandledExceptionSpecification3() {
        const char code[] = "void f() const throw (std::runtime_error);\n"
                            "int _init() {\n"
                            "    f();\n"
                            "}\n"
                            "int _fini() {\n"
                            "    f();\n"
                            "}\n"
                            "int main()\n"
                            "{\n"
                            "    f();\n"
                            "}\n";

        check(code, true);
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:1]: (style, inconclusive) Unhandled exception specification when calling function f().\n"
                      "[test.cpp:6] -> [test.cpp:1]: (style, inconclusive) Unhandled exception specification when calling function f().\n", errout.str());

        const Settings s = settingsBuilder().library("gnu.cfg").build();
        check(code, true, &s);
        ASSERT_EQUALS("", errout.str());
    }

    void nothrowAttributeThrow() {
        check("void func1() throw(int) { throw 1; }\n"
              "void func2() __attribute((nothrow)); void func2() { throw 1; }\n"
              "void func3() __attribute((nothrow)); void func3() { func1(); }");
        ASSERT_EQUALS("[test.cpp:2]: (error) Exception thrown in function declared not to throw exceptions.\n"
                      "[test.cpp:3]: (error) Exception thrown in function declared not to throw exceptions.\n", errout.str());

        // avoid false positives
        check("const char *func() __attribute((nothrow)); void func1() { return 0; }");
        ASSERT_EQUALS("", errout.str());
    }

    void nothrowAttributeThrow2() {
        check("class foo {\n"
              "  void copyMemberValues() throw () {\n"
              "      copyMemberValues();\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void nothrowDeclspecThrow() {
        check("void func1() throw(int) { throw 1; }\n"
              "void __declspec(nothrow) func2() { throw 1; }\n"
              "void __declspec(nothrow) func3() { func1(); }");
        ASSERT_EQUALS("[test.cpp:2]: (error) Exception thrown in function declared not to throw exceptions.\n"
                      "[test.cpp:3]: (error) Exception thrown in function declared not to throw exceptions.\n", errout.str());

        // avoid false positives
        check("const char *func() __attribute((nothrow)); void func1() { return 0; }");
        ASSERT_EQUALS("", errout.str());
    }

    void rethrowNoCurrentException1() {
        check("void func1(const bool flag) { try{ if(!flag) throw; } catch (int&) { ; } }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow."
                      " If there is no current exception this calls std::terminate(). More: https://isocpp.org/wiki/faq/exceptions#throw-without-an-object\n", errout.str());
    }

    void rethrowNoCurrentException2() {
        check("void func1() { try{ ; } catch (...) { ; } throw; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow."
                      " If there is no current exception this calls std::terminate(). More: https://isocpp.org/wiki/faq/exceptions#throw-without-an-object\n", errout.str());
    }

    void rethrowNoCurrentException3() {
        check("void on_error() { try { throw; } catch (const int &) { ; } catch (...) { ; } }\n"      // exception dispatcher idiom
              "void func2() { try{ ; } catch (const int&) { throw; } ; }\n"
              "void func3() { throw 0; }");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestExceptionSafety)
