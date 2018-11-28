/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "checkstl.h"
#include "settings.h"
#include "standards.h"
#include "testsuite.h"
#include "tokenize.h"

#include <cstddef>
#include <string>


class TestStl : public TestFixture {
public:
    TestStl() : TestFixture("TestStl") {
    }

private:
    Settings settings;

    void run() override {
        settings.addEnabled("warning");
        settings.addEnabled("style");
        settings.addEnabled("performance");
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(outOfBounds);

        TEST_CASE(iterator1);
        TEST_CASE(iterator2);
        TEST_CASE(iterator3);
        TEST_CASE(iterator4);
        TEST_CASE(iterator5);
        TEST_CASE(iterator6);
        TEST_CASE(iterator7);
        TEST_CASE(iterator8);
        TEST_CASE(iterator9);
        TEST_CASE(iterator10);
        TEST_CASE(iterator11);
        TEST_CASE(iterator12);
        TEST_CASE(iterator13);
        TEST_CASE(iterator14); // #8191
        TEST_CASE(iterator15); // #8341
        TEST_CASE(iterator16);
        TEST_CASE(iterator17);
        TEST_CASE(iterator18);
        TEST_CASE(iterator19);
        TEST_CASE(iterator20);
        TEST_CASE(iterator21);
        TEST_CASE(iteratorExpression);
        TEST_CASE(iteratorSameExpression);

        TEST_CASE(dereference);
        TEST_CASE(dereference_break);  // #3644 - handle "break"
        TEST_CASE(dereference_member);

        TEST_CASE(STLSize);
        TEST_CASE(STLSizeNoErr);
        TEST_CASE(negativeIndex);
        TEST_CASE(erase1);
        TEST_CASE(erase2);
        TEST_CASE(erase3);
        TEST_CASE(erase4);
        TEST_CASE(erase5);
        TEST_CASE(erase6);
        TEST_CASE(eraseBreak);
        TEST_CASE(eraseContinue);
        TEST_CASE(eraseReturn1);
        TEST_CASE(eraseReturn2);
        TEST_CASE(eraseReturn3);
        TEST_CASE(eraseGoto);
        TEST_CASE(eraseAssign1);
        TEST_CASE(eraseAssign2);
        TEST_CASE(eraseAssign3);
        TEST_CASE(eraseAssign4);
        TEST_CASE(eraseAssignByFunctionCall);
        TEST_CASE(eraseErase);
        TEST_CASE(eraseByValue);
        TEST_CASE(eraseIf);
        TEST_CASE(eraseOnVector);

        TEST_CASE(pushback1);
        TEST_CASE(pushback2);
        TEST_CASE(pushback3);
        TEST_CASE(pushback4);
        TEST_CASE(pushback5);
        TEST_CASE(pushback6);
        TEST_CASE(pushback7);
        TEST_CASE(pushback8);
        TEST_CASE(pushback9);
        TEST_CASE(pushback10);
        TEST_CASE(pushback11);
        TEST_CASE(pushback12);
        TEST_CASE(pushback13);
        TEST_CASE(insert1);
        TEST_CASE(insert2);

        TEST_CASE(stlBoundaries1);
        TEST_CASE(stlBoundaries2);
        TEST_CASE(stlBoundaries3);
        TEST_CASE(stlBoundaries4); // #4364
        TEST_CASE(stlBoundaries5); // #4352
        TEST_CASE(stlBoundaries6); // #7106

        // if (str.find("ab"))
        TEST_CASE(if_find);
        TEST_CASE(if_str_find);

        TEST_CASE(size1);
        TEST_CASE(size2);
        TEST_CASE(size3);
        TEST_CASE(size4); // #2652 - don't warn about vector/deque

        // Redundant conditions..
        // if (ints.find(123) != ints.end()) ints.remove(123);
        TEST_CASE(redundantCondition1);

        // missing inner comparison when incrementing iterator inside loop
        TEST_CASE(missingInnerComparison1);
        TEST_CASE(missingInnerComparison2);     // no FP when there is comparison
        TEST_CASE(missingInnerComparison3);     // no FP when there is iterator shadowing
        TEST_CASE(missingInnerComparison4);     // no FP when "break;" is used
        TEST_CASE(missingInnerComparison5);     // Ticket #2154 - FP
        TEST_CASE(missingInnerComparison6);     // #2643 - 'it=foo.insert(++it,0);'

        // catch common problems when using the string::c_str() function
        TEST_CASE(cstr);

        TEST_CASE(autoPointer);

        TEST_CASE(uselessCalls);
        TEST_CASE(stabilityOfChecks); // #4684 cppcheck crash in template function call

        TEST_CASE(dereferenceInvalidIterator);
        TEST_CASE(dereferenceInvalidIterator2); // #6572
        TEST_CASE(dereference_auto);

        TEST_CASE(loopAlgoElementAssign);
        TEST_CASE(loopAlgoAccumulateAssign);
        TEST_CASE(loopAlgoContainerInsert);
        TEST_CASE(loopAlgoIncrement);
        TEST_CASE(loopAlgoConditional);
        TEST_CASE(loopAlgoMinMax);
    }

    void check(const char code[], const bool inconclusive=false, const Standards::cppstd_t cppstandard=Standards::CPP11) {
        // Clear the error buffer..
        errout.str("");

        settings.inconclusive = inconclusive;
        settings.standards.cpp = cppstandard;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check..
        CheckStl checkStl(&tokenizer, &settings, this);
        checkStl.runSimplifiedChecks(&tokenizer, &settings, this);
    }
    void check(const std::string &code, const bool inconclusive=false) {
        check(code.c_str(), inconclusive);
    }

    void checkNormal(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckStl checkStl(&tokenizer, &settings, this);
        checkStl.runChecks(&tokenizer, &settings, this);
    }

    void outOfBounds() {
        setMultiline();

        checkNormal("void f() {\n"
                    "  std::string s;\n"
                    "  s[10] = 1;\n"
                    "}");
        ASSERT_EQUALS("test.cpp:3:error:Accessing an item in container 's' that is empty.\n", errout.str());

        checkNormal("void f() {\n"
                    "  std::string s = \"abcd\";\n"
                    "  s[10] = 1;\n"
                    "}");
        ASSERT_EQUALS("test.cpp:3:error:Accessing s[10] is out of bounds when s size is 4.\n", errout.str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    v.front();\n"
                    "    if (v.empty()) {}\n"
                    "}\n");
        ASSERT_EQUALS("test.cpp:2:warning:Accessing an item in container 'v'. Either the condition 'v.empty()' is redundant or 'v' can be empty.\n"
                      "test.cpp:3:note:condition 'v.empty()'\n"
                      "test.cpp:2:note:Access out of bounds\n", errout.str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    if (v.size() == 3) {}\n"
                    "    v[16] = 0;\n"
                    "}\n");
        ASSERT_EQUALS("test.cpp:3:warning:Either the condition 'v.size()==3' is redundant or v size can be 3. Accessing v[16] is out of bounds when v size is 3.\n"
                      "test.cpp:2:note:condition 'v.size()==3'\n"
                      "test.cpp:3:note:Access out of bounds\n", errout.str());

        checkNormal("void f(std::vector<int> v) {\n"
                    "    int i = 16;\n"
                    "    if (v.size() == 3) {\n"
                    "        v[i] = 0;\n"
                    "    }\n"
                    "}\n");
        ASSERT_EQUALS("test.cpp:4:warning:Either the condition 'v.size()==3' is redundant or v size can be 3. Accessing v[16] is out of bounds when v size is 3.\n"
                      "test.cpp:3:note:condition 'v.size()==3'\n"
                      "test.cpp:4:note:Access out of bounds\n", errout.str());

        checkNormal("void f(std::vector<int> v, int i) {\n"
                    "    if (v.size() == 3 || i == 16) {}\n"
                    "    v[i] = 0;\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNormal("void f(std::map<int,int> x) {\n"
                    "    if (x.empty()) { x[1] = 2; }\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNormal("void f(std::string s) {\n"
                    "    if (s.size() == 1) {\n"
                    "        s[2] = 0;\n"
                    "    }\n"
                    "}\n");
        ASSERT_EQUALS("test.cpp:3:warning:Either the condition 's.size()==1' is redundant or s size can be 1. Accessing s[2] is out of bounds when s size is 1.\n"
                      "test.cpp:2:note:condition 's.size()==1'\n"
                      "test.cpp:3:note:Access out of bounds\n", errout.str());

        // Do not crash
        checkNormal("void a() {\n"
                    "  std::string b[];\n"
                    "  for (auto c : b)\n"
                    "    c.data();\n"
                    "}\n");
        ASSERT_EQUALS("", errout.str());


    }

    void iterator1() {
        check("void f()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    for (list<int>::iterator it = l1.begin(); it != l2.end(); ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:5]: (error) Same iterator is used with different containers 'l1' and 'l2'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    for (list<int>::iterator it = l1.begin(); l2.end() != it; ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:5]: (error) Same iterator is used with different containers 'l2' and 'l1'.\n", errout.str());

        check("struct C { std::list<int> l1; void func(); };\n"
              "void C::func() {\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = l1.begin(); it != l1.end(); ++it) { }\n"
              "    C c;\n"
              "    for (it = c.l1.begin(); it != c.l1.end(); ++it) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Same check with reverse iterator
        check("void f()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    for (list<int>::const_reverse_iterator it = l1.rbegin(); it != l2.rend(); ++it)\n"
              "    { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:5]: (error) Same iterator is used with different containers 'l1' and 'l2'.\n", errout.str());
    }

    void iterator2() {
        check("void foo()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    while (it != l2.end())\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:5]: (error) Same iterator is used with different containers 'l1' and 'l2'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    while (l2.end() != it)\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:5]: (error) Same iterator is used with different containers 'l2' and 'l1'.\n", errout.str());
    }

    void iterator3() {
        check("void foo()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it, 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Same iterator is used with different containers 'l1' and 'l2'.\n", errout.str());

        check("void foo() {\n" // #5803
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it, l1.end());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n" // #7658
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    list<int>::iterator end = l1.end();\n"
              "    l2.insert(it, end);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // only warn for insert when there are preciself 2 arguments.
        check("void foo() {\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo() {\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it,0,1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void iterator4() {
        check("void foo(std::vector<std::string> &test)\n"
              "{\n"
              "    std::set<int> result;\n"
              "    for (std::vector<std::string>::const_iterator cit = test.begin();\n"
              "        cit != test.end();\n"
              "        ++cit)\n"
              "    {\n"
              "        result.insert(cit->size());\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator5() {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::find(ints1.begin(), ints2.end(), 22);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());
    }

    void iterator6() {
        // Ticket #1357
        check("void foo(const std::set<int> &ints1)\n"
              "{\n"
              "    std::set<int> ints2;\n"
              "    std::set<int>::iterator it1 = ints1.begin();\n"
              "    std::set<int>::iterator it2 = ints1.end();\n"
              "    ints2.insert(it1, it2);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(const std::set<int> &ints1)\n"
              "{\n"
              "    std::set<int> ints2;\n"
              "    std::set<int>::iterator it1 = ints1.begin();\n"
              "    std::set<int>::iterator it2 = ints2.end();\n"
              "    ints2.insert(it1, it2);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Iterators of different containers are used together.\n", "", errout.str());
    }

    void iterator7() {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::inplace_merge(ints1.begin(), std::advance(ints1.rbegin(), 5), ints2.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::inplace_merge(ints1.begin(), std::advance(ints2.rbegin(), 5), ints1.end());\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator8() {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints2.end(), ints1.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints1.end(), ints2.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::find_first_of(foo.bar.begin(), foo.bar.end()-6, ints2.begin(), ints1.end());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> ints1;\n"
              "    std::vector<int> ints2;\n"
              "    std::vector<int>::iterator it = std::find_first_of(ints1.begin(), ints1.end(), ints2.begin(), ints2.end());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6839
        check("void f(const std::wstring& a, const std::wstring& b) {\n"
              "    const std::string tp1 = std::string(a.begin(), b.end());\n"
              "    const std::wstring tp2 = std::string(b.begin(), a.end());\n"
              "    const std::u16string tp3(a.begin(), b.end());\n"
              "    const std::u32string tp4(b.begin(), a.end());\n"
              "    const std::string fp1 = std::string(a.begin(), a.end());\n"
              "    const std::string tp2(a.begin(), a.end());\n"
              "}");
        ASSERT_EQUALS(// TODO "[test.cpp:2]: (error) Iterators of different containers are used together.\n"
            // TODO "[test.cpp:3]: (error) Iterators of different containers are used together.\n"
            "[test.cpp:4]: (error) Iterators of different containers are used together.\n"
            "[test.cpp:5]: (error) Iterators of different containers are used together.\n", errout.str());
    }

    void iterator9() {
        // Ticket #1600
        check("void foo(std::vector<int> &r)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            return;\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #2481
        check("void foo(std::vector<int> &r)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            break;\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Execution path checking..
        check("void foo(std::vector<int> &r, int c)\n"
              "{\n"
              "    std::vector<int>::iterator aI = r.begin();\n"
              "    while(aI != r.end())\n"
              "    {\n"
              "        if (*aI == 0)\n"
              "        {\n"
              "            r.insert(aI, 42);\n"
              "            if (c)\n"
              "            {\n"
              "                return;\n"
              "            }\n"
              "        }\n"
              "        ++aI;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:14] (error) After insert(), the iterator 'aI' may be invalid.", "", errout.str());
    }

    void iterator10() {
        // Ticket #1679
        check("void foo()\n"
              "{\n"
              "    std::set<int> s1;\n"
              "    std::set<int> s2;\n"
              "    for (std::set<int>::iterator it = s1.begin(); it != s1.end(); ++it)\n"
              "    {\n"
              "        if (true) { }\n"
              "        if (it != s2.end()) continue;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:5]: (error) Same iterator is used with different containers 's1' and 's2'.\n", errout.str());
    }

    void iterator11() {
        // Ticket #3433
        check("int main() {\n"
              "    map<int, int> myMap;\n"
              "    vector<string> myVector;\n"
              "    for(vector<int>::iterator x = myVector.begin(); x != myVector.end(); x++)\n"
              "        myMap.erase(*x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator12() {
        // Ticket #3201
        check("void f() {\n"
              "    std::map<int, int> map1;\n"
              "    std::map<int, int> map2;\n"
              "    std::map<int, int>::const_iterator it = map1.find(123);\n"
              "    if (it == map2.end()) { }"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (error) Same iterator is used with different containers 'map1' and 'map2'.\n", errout.str());

        check("void f() {\n"
              "    std::map<int, int> map1;\n"
              "    std::map<int, int> map2;\n"
              "    std::map<int, int>::const_iterator it = map1.find(123);\n"
              "    if (map2.end() == it) { }"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:4]: (error) Same iterator is used with different containers 'map2' and 'map1'.\n", errout.str());

        check("void f(std::string &s) {\n"
              "    int pos = s.find(x);\n"
              "    s.erase(pos);\n"
              "    s.erase(pos);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator13() {
        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    it = a.begin();\n"
              "    while (it!=a.end())\n"
              "        ++it;\n"
              "    it = t.begin();\n"
              "    while (it!=a.end())\n"
              "        ++it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:8]: (error) Same iterator is used with different containers 't' and 'a'.\n", errout.str());

        // #4062
        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    it = a.begin();\n"
              "    while (it!=a.end())\n"
              "        v++it;\n"
              "    it = t.begin();\n"
              "    while (it!=t.end())\n"
              "        ++it;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::vector<int> t;\n"
              "    std::vector<int>::const_iterator it;\n"
              "    if(z)\n"
              "        it = a.begin();\n"
              "    else\n"
              "        it = t.begin();\n"
              "    while (z && it!=a.end())\n"
              "        v++it;\n"
              "    while (!z && it!=t.end())\n"
              "        v++it;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator14() {
        check("void f() {\n"
              "    std::map<int,Foo> x;\n"
              "    std::map<int,Foo>::const_iterator it;\n"
              "    for (it = x.find(0)->second.begin(); it != x.find(0)->second.end(); ++it) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator15() {
        check("void f(C1* x, std::list<int> a) {\n"
              "  std::list<int>::iterator pos = a.begin();\n"
              "  for(pos = x[0]->plist.begin(); pos != x[0]->plist.end(); ++pos) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void iterator16() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.end();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:5] -> [test.cpp:6]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    while (it2 != it1)\n"
              "    {\n"
              "        ++it2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l2' and 'l1'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it2 = l2.end();\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2(10, 4);\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.find(4);\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());

    }

    void iterator17() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:5] -> [test.cpp:7]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "    it2 = l2.end();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    it1 = l2.end();\n"
              "    it1 = l1.end();\n"
              "    if (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    it2 = l1.end();\n"
              "    { it2 = l2.end(); }\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10] -> [test.cpp:5] -> [test.cpp:9]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());
    }

    void iterator18() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (++it1 != --it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    while (it1++ != --it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l1.end();\n"
              "    if (--it2 > it1++)\n"
              "    {\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:7]: (error) Dangerous comparison using operator< on iterator.\n", errout.str());
    }

    void iterator19() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        if (it1 != l1.end())\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (error) Same iterator is used with containers 'l1' that are defined in different scopes.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        if (l1.end() > it1)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:4]: (error) Same iterator is used with containers 'l1' that are defined in different scopes.\n",
                           "[test.cpp:7] -> [test.cpp:4]: (error) Same iterator is used with containers 'l1' that are defined in different scopes.\n[test.cpp:7]: (error) Dangerous comparison using operator< on iterator.\n",
                           errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        std::list<int>::iterator it2 = l1.begin();\n"
              "        if (it1 != it2)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:4] -> [test.cpp:7]: (error) Comparison of iterators from containers 'l1' that are defined in different scopes.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    {\n"
              "        std::list<int> l1;\n"
              "        std::list<int>::iterator it2 = l1.begin();\n"
              "        if (it2 != it1)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:4] -> [test.cpp:7]: (error) Comparison of iterators from containers 'l1' that are defined in different scopes.\n", errout.str());

    }

    void iterator20() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    it1 = it2;\n"
              "    while (it1 != l1.end())\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8] -> [test.cpp:7]: (error) Same iterator is used with different containers 'l2' and 'l1'.\n", "", errout.str());

        check("std::list<int> l3;\n"
              "std::list<int>::iterator bar()\n"
              "{\n"
              "    return l3.end();\n"
              "}\n"
              "void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.begin();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    it1 = bar();\n"
              "    while (it1 != it2)\n"
              "    {\n"
              "        ++it1;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:13] -> [test.cpp:10] -> [test.cpp:11]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", "", errout.str());

    }

    void iterator21() {
        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    if (it1 != it2)\n"
              "    {\n"
              "    }\n"
              "    if (it2 != it1)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n"
                      "[test.cpp:10] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l2' and 'l1'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::list<int> l1;\n"
              "    std::list<int> l2;\n"
              "    std::list<int>::iterator it1 = l1.end();\n"
              "    std::list<int>::iterator it2 = l2.begin();\n"
              "    if (it1 != it2 && it1 != it2)\n"
              "    {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6] -> [test.cpp:5]: (error) Comparison of iterators from containers 'l1' and 'l2'.\n", errout.str());

    }

    void iteratorExpression() {
        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    (void)std::find(f().begin(), g().end(), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Iterators to containers from different expressions 'f()' and 'g()' are used together.\n", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    if(f().begin() == g().end()) {}\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Iterators to containers from different expressions 'f()' and 'g()' are used together.\n", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    auto size = f().end() - g().begin();\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Iterators to containers from different expressions 'f()' and 'g()' are used together.\n", errout.str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A().f().begin(), A().g().end(), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Iterators to containers from different expressions 'A().f()' and 'A().g()' are used together.\n", errout.str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A{} .f().begin(), A{} .g().end(), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Iterators to containers from different expressions 'A{}.f()' and 'A{}.g()' are used together.\n", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    (void)std::find(begin(f()), end(g()), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Iterators to containers from different expressions 'f()' and 'g()' are used together.\n", errout.str());

        check("struct A {\n"
              "    std::vector<int>& f();\n"
              "    std::vector<int>& g();\n"
              "};\n"
              "void foo() {\n"
              "    (void)std::find(A().f().begin(), A().f().end(), 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    if(bar(f().begin()) == g().end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    auto it = f().end();\n"
              "    f().begin() - it\n"
              "    f().begin()+1 - it\n"
              "    f().begin() - (it + 1)\n"
              "    f().begin() - f().end()\n"
              "    f().begin()+1 - f().end()\n"
              "    f().begin() - (f().end() + 1)\n"
              "    (void)std::find(f().begin(), it, 0);\n"
              "    (void)std::find(f().begin(), it + 1, 0);\n"
              "    (void)std::find(f().begin() + 1, it + 1, 0);\n"
              "    (void)std::find(f().begin() + 1, it, 0);\n"
              "    (void)std::find(f().begin(), f().end(), 0);\n"
              "    (void)std::find(f().begin() + 1, f().end(), 0);\n"
              "    (void)std::find(f().begin(), f().end() - 1, 0);\n"
              "    (void)std::find(f().begin() + 1, f().end() - 1, 0);\n"
              "    (void)std::find(begin(f()), end(f()));\n"
              "    (void)std::find(begin(f()) + 1, end(f()), 0);\n"
              "    (void)std::find(begin(f()), end(f()) - 1, 0);\n"
              "    (void)std::find(begin(f()) + 1, end(f()) - 1, 0);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int>& f();\n"
              "std::vector<int>& g();\n"
              "void foo() {\n"
              "    if(f().begin() == f().end()) {}\n"
              "    if(f().begin() == f().end()+1) {}\n"
              "    if(f().begin()+1 == f().end()) {}\n"
              "    if(f().begin()+1 == f().end()+1) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("template<int N>\n"
              "std::vector<int>& f();\n"
              "void foo() {\n"
              "    if(f<1>().begin() == f<1>().end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  if (a.begin().x == b.begin().x) {}\n"
              "  if (begin(a).x == begin(b).x) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  std::list<int*> a;\n"
              "  std::list<int*> b;\n"
              "  if (*a.begin() == *b.begin()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if(f().begin(1) == f().end()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void iteratorSameExpression() {
        check("void f(std::vector<int> v) {\n"
              "    std::for_each(v.begin(), v.begin(), [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same iterators expression are used for algorithm.\n", errout.str());

        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    std::for_each(g().begin(), g().begin(), [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Same iterators expression are used for algorithm.\n", errout.str());

        check("void f(std::vector<int> v) {\n"
              "    std::for_each(v.end(), v.end(), [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same iterators expression are used for algorithm.\n", errout.str());

        check("std::vector<int>& g();\n"
              "void f() {\n"
              "    std::for_each(g().end(), g().end(), [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Same iterators expression are used for algorithm.\n", errout.str());

        check("std::vector<int>::iterator g();\n"
              "void f(std::vector<int> v) {\n"
              "    std::for_each(g(), g(), [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) Same iterators expression are used for algorithm.\n", errout.str());

        check("void f(std::vector<int>::iterator it) {\n"
              "    std::for_each(it, it, [](int){});\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (style) Same iterators expression are used for algorithm.\n", errout.str());
    }

    // Dereferencing invalid pointer
    void dereference() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator iter;\n"
              "    iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    std::cout << (*iter) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6]: (error) Iterator 'iter' used after element has been erased.\n", errout.str());

        // #6554 "False positive eraseDereference - erase in while() loop"
        check("typedef std::map<Packet> packetMap;\n"
              "packetMap waitingPackets;\n"
              "void ProcessRawPacket() {\n"
              "    packetMap::iterator wpi;\n"
              "    while ((wpi = waitingPackets.find(lastInOrder + 1)) != waitingPackets.end()) {\n"
              "        waitingPackets.erase(wpi);\n"
              "        for (unsigned pos = 0; pos < buf.size(); ) {     }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #8509 Uniform initialization ignored for iterator
        check("void f() {\n"
              "  std::vector<int> ints;\n"
              "  std::vector<int>::const_iterator iter {ints.cbegin()};\n"
              "  std::cout << (*iter) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void dereference_break() {  // #3644
        check("void f(std::vector<int> &ints) {\n"
              "    std::vector<int>::iterator iter;\n"
              "    for (iter=ints.begin();iter!=ints.end();++iter) {\n"
              "        if (*iter == 2) {\n"
              "            ints.erase(iter);\n"
              "            break;\n"
              "        }\n"
              "        if (*iter == 3) {\n"
              "            ints.erase(iter);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void dereference_member() {
        check("void f()\n"
              "{\n"
              "    std::map<int, int> ints;\n"
              "    std::map<int, int>::iterator iter;\n"
              "    iter = ints.begin();\n"
              "    ints.erase(iter);\n"
              "    std::cout << iter->first << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6]: (error) Iterator 'iter' used after element has been erased.\n", errout.str());

        // Reverse iterator
        check("void f()\n"
              "{\n"
              "    std::map<int, int> ints;\n"
              "    std::map<int, int>::reverse_iterator iter;\n"
              "    iter = ints.rbegin();\n"
              "    ints.erase(iter);\n"
              "    std::cout << iter->first << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7] -> [test.cpp:6]: (error) Iterator 'iter' used after element has been erased.\n", errout.str());
    }

    void dereference_auto() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    auto iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    std::cout << (*iter) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6] -> [test.cpp:5]: (error) Iterator 'iter' used after element has been erased.\n", errout.str());

        check("void f() {\n"
              "    auto x = *myList.begin();\n"
              "    myList.erase(x);\n"
              "    auto b = x.first;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("const CXXRecordDecl *CXXRecordDecl::getTemplateInstantiationPattern() const {\n"
              "    if (auto *TD = dyn_cast<ClassTemplateSpecializationDecl>(this)) {\n"
              "        auto From = TD->getInstantiatedFrom();\n"
              "    }\n"
              "    return nullptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void STLSize() {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "       foo[ii] = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) When ii==foo.size(), foo[ii] is out of bounds.\n", errout.str());

        check("void foo(std::vector<int> foo) {\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii) {\n"
              "       foo.at(ii) = 0;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) When ii==foo.size(), foo.at(ii) is out of bounds.\n", errout.str());

        check("void foo(const std::string& foo) {\n"
              "    for (unsigned int ii = 0; ii <= foo.length(); ++ii) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) When ii==foo.size(), foo[ii] is out of bounds.\n", errout.str());

        check("void foo(const std::string& foo, unsigned int ii) {\n"
              "    if (ii <= foo.length()) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) When ii==foo.size(), foo[ii] is out of bounds.\n", errout.str());

        check("void foo(const std::string& foo, unsigned int ii) {\n"
              "    do {\n"
              "       foo[ii] = 'x';\n"
              "       ++i;\n"
              "    } while(ii <= foo.length());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) When ii==foo.size(), foo[ii] is out of bounds.\n", errout.str());

        check("void foo(const std::string& foo, unsigned int ii) {\n"
              "    if (anything()) {\n"
              "    } else if (ii <= foo.length()) {\n"
              "       foo[ii] = 'x';\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) When ii==foo.size(), foo[ii] is out of bounds.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(1);\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "    }\n"
              "    int ii = 0;\n"
              "    foo[ii] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    for (B b : D()) {}\n" // Don't crash on range-based for-loop
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void STLSizeNoErr() {
        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii < foo.size(); ++ii)\n"
                  "    {\n"
                  "       foo[ii] = 0;\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "        if (ii == foo.size())\n"
                  "        {\n"
                  "        }\n"
                  "        else\n"
                  "        {\n"
                  "            foo[ii] = 0;\n"
                  "        }\n"
                  "    }\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }

        {
            check("void f(const std::map<int,int> &data) {\n"
                  "    int i = x;"
                  "    for (int i = 5; i <= data.size(); i++)\n"
                  "        data[i] = 0;\n"
                  "}");
            ASSERT_EQUALS("", errout.str());
        }
    }

    void negativeIndex() {
        check("void f(const std::vector<int> &v) {\n"
              "  v[-11] = 123;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Array index -11 is out of bounds.\n", errout.str());
    }



    void erase1() {
        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it) {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "    for (it = foo.begin(); it != foo.end(); ++it) {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:5]: (error) Iterator 'it' used after element has been erased.\n"
                      "[test.cpp:7] -> [test.cpp:8]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f(std::list<int> &ints)\n"
              "{\n"
              "    std::list<int>::iterator i = ints.begin();\n"
              "    i = ints.erase(i);\n"
              "    *i = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator i;\n"
              "    while (i != x.y.end())\n"
              "        i = x.y.erase(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #2101
        check("void f(vector< list<int> > &ints, unsigned int i)\n"
              "{\n"
              "    list<int>::iterator it;\n"
              "    for(it = ints[i].begin(); it != ints[i].end(); it++) {\n"
              "        if (*it % 2)\n"
              "            it = ints[i].erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void erase2() {
        check("static void f()\n"
              "{\n"
              "    for (iterator it = foo.begin(); it != foo.end(); it = next)\n"
              "    {\n"
              "        next = it;\n"
              "        next++;\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void erase3() {
        check("static void f(std::list<abc> &foo)\n"
              "{\n"
              "    std::list<abc>::iterator it = foo.begin();\n"
              "    foo.erase(it->a);\n"
              "    if (it->b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void erase4() {
        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it, it2;\n"
              "    for (it = foo.begin(); it != i2; ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    for (; it != i2; ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    while (it != i2)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::list<int>::iterator it = foo.begin();\n"
              "    while (it != i2)\n"
              "    {\n"
              "        foo.erase(++it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:6]: (error) Iterator 'it' used after element has been erased.\n", errout.str());
    }

    void erase5() {
        check("void f()\n"
              "{\n"
              "    std::list<int> foo;\n"
              "    std::list<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (*it == 123)\n"
              "            foo.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:8]: (error) Iterator 'it' used after element has been erased.\n", errout.str());
    }

    void erase6() {
        check("void f() {\n"
              "    std::vector<int> vec(3);\n"
              "    std::vector<int>::iterator it;\n"
              "    std::vector<int>::iterator itEnd = vec.end();\n"
              "    for (it = vec.begin(); it != itEnd; it = vec.begin(), itEnd = vec.end())\n"
              "    {\n"
              "        vec.erase(it);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseBreak() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        if (x)"
              "            break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (x) {\n"
              "            foo.erase(it);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x)\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        if (x)"
              "            return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:5]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

    }

    void eraseContinue() {
        check("void f(std::vector<int> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator it;\n"
              "    std::vector<int>::iterator jt = ints.begin();\n"
              "    for (it = ints.begin(); it != ints.end(); it = jt) {\n"
              "        ++jt;\n"
              "        if (*it == 1) {\n"
              "            jt = ints.erase(it);\n"
              "            continue;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::map<uint32, uint32> my_map) {\n" // #7365
              "  std::map<uint32, uint32>::iterator itr = my_map.begin();\n"
              "  switch (itr->first) {\n"
              "  case 0:\n"
              "    my_map.erase(itr);\n"
              "    continue;\n"
              "  case 1:\n"
              "    itr->second = 1;\n"
              "    break;\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseReturn1() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseReturn2() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1) {\n"
              "            foo.erase(it);\n"
              "            return;\n"
              "        }\n"
              "        else {\n"
              "            foo.erase(it);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseReturn3() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (somecondition) {\n"
              "            if (*it == 1)\n"
              "                foo.erase(it);\n"
              "            else\n"
              "                *it = 0;\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        if (a) {\n"
              "            if (b)\n"
              "                foo.erase(it);\n"
              "            else\n"
              "                *it = 0;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5] -> [test.cpp:9]: (error) Iterator 'it' used after element has been erased.\n", errout.str());
    }

    void eraseGoto() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        goto abc;\n"
              "    }\n"
              "bar:\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign1() {
        check("void f()\n"
              "{\n"
              "    for (std::vector<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        it = foo.begin();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign2() {
        check("void f(list<int> &ints)\n"
              "{\n"
              "    for (list<int>::iterator it = ints.begin(); it != ints.end();) {\n"
              "        if (*it == 123) {\n"
              "            list<int>::iterator copy = it;\n"
              "            ++copy;\n"
              "            ints.erase(it);\n"
              "            it = copy;\n"
              "        } else {\n"
              "            it->second = 123;\n"
              "            ++it;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign3() {
        check("void f(std::list<list<int> >& l) {\n"
              "    std::list<std::list<int> >::const_iterator i = l.begin();\n"
              "    std::list<int>::const_iterator j = (*i).begin();\n"
              "    cout << *j << endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign4() {
        check("void f(std::list<int> data) {\n"
              "  std::list<int>::const_iterator it = data.begin();\n"
              "  it = data.erase(it);\n"
              "  it = data.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(Data data) {\n"
              "  std::list<int>::const_iterator it = data.ints.begin();\n"
              "  it = data.ints.erase(it);\n"
              "  it = data.ints.erase(it);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssignByFunctionCall() {
        check("void f(std::list<list<int> >& l) {\n"
              "    std::list<foo>::const_iterator i;\n"
              "    bar(i);\n"
              "    cout << *i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseErase() {
        check("void f(std::vector<ints> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator iter;\n"
              "    iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    ints.erase(iter);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Invalid iterator: iter\n", errout.str());
    }

    void eraseByValue() {
        check("void f()\n"
              "{\n"
              "    std::set<int> foo;\n"
              "    for (std::set<int>::iterator it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(*it);\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Iterator 'it' becomes invalid when deleted by value from 'foo'\n", "", errout.str());

        check("int f(std::set<int> foo) {\n"
              "    std::set<int>::iterator it = foo.begin();\n"
              "    foo.erase(*it);\n"
              "    return *it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:3]: (error) Iterator 'it' used after element has been erased.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::set<int> foo;\n"
              "    std::set<int>::iterator it = foo.begin();\n"
              "    foo.erase(*it);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #5669
        check("void f() {\n"
              "    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );\n"
              "    m_SetLoadedFactories.erase(*aIt);\n"
              "    m_SetLoadedFactories.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::list<int>& m_ImplementationMap) {\n"
              "    std::list<int>::iterator aIt = m_ImplementationMap.find( xEle );\n"
              "    m_ImplementationMap.erase(*aIt);\n"
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Invalid iterator: aIt\n", errout.str());

        check("void f(const std::list<int>& m_ImplementationMap) {\n"
              "    std::list<int>::iterator aIt = m_ImplementationMap.find( xEle1 );\n"
              "    std::list<int>::iterator bIt = m_ImplementationMap.find( xEle2 );\n"
              "    m_ImplementationMap.erase(*bIt);\n"
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseIf() {
        // #4816
        check("void func(std::list<std::string> strlist) {\n"
              "    for (std::list<std::string>::iterator str = strlist.begin(); str != strlist.end(); str++) {\n"
              "        if (func2(*str)) {\n"
              "    	       strlist.erase(str);\n"
              "            if (strlist.empty())\n"
              "                 return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:4]: (error) Iterator 'str' used after element has been erased.\n", errout.str());
    }

    void eraseOnVector() {
        check("void f(const std::vector<int>& m_ImplementationMap) {\n"
              "    std::vector<int>::iterator aIt = m_ImplementationMap.find( xEle );\n"
              "    m_ImplementationMap.erase(something(unknown));\n" // All iterators become invalidated when erasing from std::vector
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) After erase(), the iterator 'aIt' may be invalid.\n", errout.str());

        check("void f(const std::vector<int>& m_ImplementationMap) {\n"
              "    std::vector<int>::iterator aIt = m_ImplementationMap.find( xEle );\n"
              "    m_ImplementationMap.erase(*aIt);\n" // All iterators become invalidated when erasing from std::vector
              "    m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Invalid iterator: aIt\n", errout.str());

        check("void f(const std::vector<int>& m_ImplementationMap) {\n"
              "    std::vector<int>::iterator aIt = m_ImplementationMap.find( xEle1 );\n"
              "    std::vector<int>::iterator bIt = m_ImplementationMap.find( xEle2 );\n"
              "    m_ImplementationMap.erase(*bIt);\n" // All iterators become invalidated when erasing from std::vector
              "    aIt = m_ImplementationMap.erase(aIt);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) After erase(), the iterator 'aIt' may be invalid.\n", errout.str());
    }

    void pushback1() {
        check("void f(const std::vector<int> &foo)\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    *it;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) After push_back(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback2() {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    {\n"
              "        int *it = &foo[0];\n"
              "        *it = 456;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pushback3() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.push_back(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) After push_back(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback4() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    ints.push_back(1);\n"
              "    int *first = &ints[0];\n"
              "    ints.push_back(2);\n"
              "    *first;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Invalid pointer 'first' after push_back().\n", errout.str());
    }

    void pushback5() {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator i;\n"
              "\n"
              "    for (i=v.begin(); i!=v.end(); ++i)\n"
              "    {\n"
              "    }\n"
              "\n"
              "    for (i=rhs.v.begin(); i!=rhs.v.end(); ++i)\n"
              "    {\n"
              "        v.push_back(*i);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pushback6() {
        // ticket #735
        check("void f()\n"
              "{\n"
              "    vector<int> v;\n"
              "    vector.push_back(1);\n"
              "    vector.push_back(2);\n"
              "    for (vector<int>::iterator it = v.begin(); it != v.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1)\n"
              "            v.push_back(10);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) After push_back(), the iterator 'it' may be invalid.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> v;\n"
              "    vector.push_back(1);\n"
              "    vector.push_back(2);\n"
              "    for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)\n"
              "    {\n"
              "        if (*it == 1)\n"
              "            v.push_back(10);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) After push_back(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback7() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); it++)\n"
              "    {\n"
              "        foo.push_back(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) After push_back(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback8() {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::const_iterator end = ints.end();\n"
              "    ints.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    unsigned int sum = 0;\n"
              "    for (it = ints.begin(); it != end; ++it)\n"
              "    {\n"
              "        sum += *it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) After push_back(), the iterator 'end' may be invalid.\n", errout.str());
    }

    void pushback9() {
        check("struct A {\n"
              "    std::vector<int> ints;\n"
              "};\n"
              "\n"
              "void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    A a;\n"
              "    std::vector<int>::const_iterator i = ints.begin();\n"
              "    std::vector<int>::const_iterator e = ints.end();\n"
              "    while (i != e)\n"
              "    {\n"
              "        a.ints.push_back(*i);\n"
              "        ++i;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pushback10() {
        check("void f(std::vector<int> &foo)\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.reserve(100);\n"
              "    *it = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) After reserve(), the iterator 'it' may be invalid.\n", errout.str());

        // in loop
        check("void f()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    foo.push_back(10);\n"
              "    std::vector<int>::iterator it;\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.reserve(123);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) After reserve(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback11() {
        // #2798
        check("void f() {\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator it = ints.begin();\n"
              "    if (it == ints.begin()) {\n"
              "        ints.push_back(0);\n"
              "    } else {\n"
              "        ints.insert(it,0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pushback12() {
        // #4197
        check("void foo(double s)\n"
              "{\n"
              "    std::vector<double> vec;\n"
              "    for( std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it )\n"
              "    {\n"
              "        vec.insert( it, s );\n"
              "        for(unsigned int i = 0; i < 42; i++)\n"
              "        {}\n"
              "        *it;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) After insert(), the iterator 'it' may be invalid.\n"
                      "[test.cpp:9]: (error) After insert(), the iterator 'it' may be invalid.\n", errout.str());
    }

    void pushback13() {
        check("bool Preprocessor::ConcatenateIncludeName(SmallString<128> &FilenameBuffer, SourceLocation &End) {\n"
              "    unsigned PreAppendSize = FilenameBuffer.size();\n"
              "    FilenameBuffer.resize(PreAppendSize + CurTok.getLength());\n"
              "    const char *BufPtr = &FilenameBuffer[PreAppendSize];\n"
              "    return true;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void insert1() {
        check("void f(std::vector<int> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator iter = ints.begin() + 5;\n"
              "    ints.insert(ints.begin(), 1);\n"
              "    ++iter;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) After insert(), the iterator 'iter' may be invalid.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator iter = ints.begin();\n"
              "    ints.insert(iter, 1);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    std::vector<int>::iterator iter = ints.begin();\n"
              "    ints.insert(iter, 1);\n"
              "    ints.insert(iter, 2);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) After insert(), the iterator 'iter' may be invalid.\n", errout.str());

        check("void* f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(Bar());\n"
              "    void* v = &i->foo;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) After insert(), the iterator 'i' may be invalid.\n", errout.str());

        check("Foo f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(Bar());\n"
              "    return i->foo;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) After insert(), the iterator 'i' may be invalid.\n", errout.str());

        check("void f(const std::vector<Bar>& bars) {\n"
              "    for(std::vector<Bar>::iterator i = bars.begin(); i != bars.end(); ++i) {\n"
              "        i = bars.insert(i, bar);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::vector<Bar>& bars) {\n"
              "    for(std::vector<Bar>::iterator i = bars.begin(); i != bars.end(); ++i) {\n"
              "        bars.insert(i, bar);\n"
              "        i = bars.insert(i, bar);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) After insert(), the iterator 'i' may be invalid.\n"
                      "[test.cpp:4]: (error) After insert(), the iterator 'i' may be invalid.\n", errout.str());

        check("void* f(const std::vector<Bar>& bars) {\n"
              "    std::vector<Bar>::iterator i = bars.begin();\n"
              "    bars.insert(i, Bar());\n"
              "    i = bars.insert(i, Bar());\n"
              "    void* v = &i->foo;\n"
              "    return v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) After insert(), the iterator 'i' may be invalid.\n", errout.str());
    }

    void insert2() {
        // Ticket: #2169
        check("void f(std::vector<int> &vec) {\n"
              "    for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); ++iter)\n"
              "    {\n"
              "        vec.insert(iter, 0);\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::vector<int> &vec) {\n"
              "    for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); ++iter)\n"
              "    {\n"
              "        if (*it == 0) {\n"
              "            vec.insert(iter, 0);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    template<size_t n, typename T>
    static size_t getArraylength(const T(&)[n]) {
        return n;
    }

    void stlBoundaries1() {
        const std::string stlCont[] = {
            "list", "set", "multiset", "map", "multimap"
        };

        for (size_t i = 0; i < getArraylength(stlCont); ++i) {
            check("void f()\n"
                  "{\n"
                  "    std::" + stlCont[i] + "<int>::iterator it;\n"
                  "    for (it = ab.begin(); it < ab.end(); ++it)\n"
                  "        ;\n"
                  "}");

            ASSERT_EQUALS_MSG("[test.cpp:4]: (error) Dangerous comparison using operator< on iterator.\n", errout.str(), stlCont[i]);
        }

        check("void f() {\n"
              "    std::forward_list<int>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous comparison using operator< on iterator.\n", errout.str());

        // #5926 no FP Dangerous comparison using operator< on iterator on std::deque
        check("void f() {\n"
              "    std::deque<int>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void stlBoundaries2() {
        check("void f()\n"
              "{\n"
              "    std::vector<std::string> files;\n"
              "    std::vector<std::string>::const_iterator it;\n"
              "    for (it = files.begin(); it < files.end(); it++) { }\n"
              "    for (it = files.begin(); it < files.end(); it++) { };\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void stlBoundaries3() {
        check("void f()\n"
              "{\n"
              "    set<int> files;\n"
              "    set<int>::const_iterator current;\n"
              "    for (current = files.begin(); current != files.end(); ++current)\n"
              "    {\n"
              "       assert(*current < 100)\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("bool f() {\n"
              "    static set<Foo>::const_iterator current;\n"
              "    return 25 > current->bar;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid iterator 'current' used.\n", errout.str());
    }

    void stlBoundaries4() {

        check("void f() {\n"
              "    std::forward_list<std::vector<std::vector<int>>>::iterator it;\n"
              "    for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous comparison using operator< on iterator.\n", errout.str());

        // don't crash
        check("void f() {\n"
              "    if (list < 0) ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    if (list < 0) {\n"
              "        std::forward_list<std::vector<std::vector<int>>>::iterator it;\n"
              "        for (it = ab.begin(); ab.end() > it; ++it) {}\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Dangerous comparison using operator< on iterator.\n", errout.str());
    }

    void stlBoundaries5() {
        check("class iterator { int foo(); };\n"
              "int foo() {\n"
              "    iterator i;\n"
              "    return i.foo();;\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        check("class iterator {\n"
              "    Class operator*();\n"
              "    iterator& operator++();\n"
              "    int foo();\n"
              "};\n"
              "int foo() {\n"
              "    iterator i;\n"
              "    return i.foo();;\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:8]: (error, inconclusive) Invalid iterator 'i' used.\n", errout.str());
    }

    void stlBoundaries6() { // #7106
        check("void foo(std::vector<int>& vec) {\n"
              "    for (Function::iterator BB : vec) {\n"
              "        for (int Inst : *BB)\n"
              "        {\n"
              "        }\n"
              "    }\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
    }


    void if_find() {
        // ---------------------------
        // set::find
        // ---------------------------

        // error (simple)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (s.find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if ((*s).find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s->find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (array-like pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (array)
        check("void f(std::set<int> s [10])\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (undefined length array)
        check("void f(std::set<int> s [])\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (vector)
        check("void f(std::vector<std::set<int> > s)\n"
              "{\n"
              "    if (s[0].find(12)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // error (assignment)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (a || (x = s.find(12))) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // ok (simple)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (s.find(123) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if ((*s).find(12) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (array-like pointer)
        check("void f(std::set<int> *s)\n"
              "{\n"
              "    if (s[0].find(12) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (array)
        check("void f(std::set<int> s [10])\n"
              "{\n"
              "    if (s[0].find(123) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (undefined length array)
        check("void f(std::set<int> s [])\n"
              "{\n"
              "    if (s[0].find(123) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (vector)
        check("void f(std::vector<std::set<int> > s)\n"
              "{\n"
              "    if (s[0].find(123) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (assignment)
        check("void f(std::set<int> s)\n"
              "{\n"
              "    if (a || (x = s.find(12)) != s.end()) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (dereference, #6402)
        check("void f(std::set<Foo> s) {\n"
              "    if (s.find(12).member) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // ---------------------------
        // std::find
        // ---------------------------

        // error
        check("void f()\n"
              "{\n"
              "    if (std::find(a,b,c)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Suspicious condition. The result of find() is an iterator, but it is not properly checked.\n", errout.str());

        // ok
        check("void f()\n"
              "{\n"
              "    if (std::find(a,b,c) != c) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ok (less than comparison, #6217)
        check("void f(std::vector<int> s)\n"
              "{\n"
              "    if (std::find(a, b, c) < d) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3714 - segmentation fault for syntax error
        check("void f() {\n"
              "    if (()) { }\n"
              "}");

        // #3865
        check("void f() {\n"
              "    if ((std::find(a,b,c)) != b) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if_str_find() {
        // error (simple)
        check("void f(const std::string &s)\n"
              "{\n"
              "    if (s.find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Inefficient usage of string::find() in condition; string::compare() would be faster.\n", errout.str());

        // error (pointer)
        check("void f(const std::string *s)\n"
              "{\n"
              "    if ((*s).find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Inefficient usage of string::find() in condition; string::compare() would be faster.\n", errout.str());

        // error (pointer)
        check("void f(const std::string *s)\n"
              "{\n"
              "    if (s->find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Inefficient usage of string::find() in condition; string::compare() would be faster.\n", errout.str());

        // error (vector)
        check("void f(const std::vector<std::string> &s)\n"
              "{\n"
              "    if (s[0].find(\"abc\")) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Inefficient usage of string::find() in condition; string::compare() would be faster.\n", errout.str());

        // #3162
        check("void f(const std::string& s1, const std::string& s2)\n"
              "{\n"
              "    if ((!s1.empty()) && (0 == s1.find(s2))) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Inefficient usage of string::find() in condition; string::compare() would be faster.\n", errout.str());

        // #4102
        check("void f(const std::string &define) {\n"
              "    if (define.find(\"=\") + 1U == define.size());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::string a, std::string b) {\n"  // #4480
              "    if (a.find(\"<\") < b.find(\">\")) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(const std::string &s) {\n"
              "    if (foo(s.find(\"abc\"))) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7349 - std::string::find_first_of
        check("void f(const std::string &s) {\n"
              "    if (s.find_first_of(\"abc\")==0) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void size1() {
        const char* code = "struct Fred {\n"
                           "    void foo();\n"
                           "    std::list<int> x;\n"
                           "};\n"
                           "void Fred::foo()\n"
                           "{\n"
                           "    if (x.size() == 0) {}\n"
                           "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:7]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "std::list<int> x;\n"
               "void f()\n"
               "{\n"
               "    if (x.size() == 0) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (x.size() == 0) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (0 == x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (x.size() != 0) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (0 != x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (x.size() > 0) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (0 < x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code =  "void f()\n"
                "{\n"
                "    std::list<int> x;\n"
                "    if (x.size() >= 1) {}\n"
                "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (x.size() < 1) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (1 <= x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (1 > x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    if (!x.size()) {}\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    std::list<int> x;\n"
              "    fun(x.size());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        code ="void f()\n"
              "{\n"
              "    std::list<int> x;\n"
              "    fun(!x.size());\n"
              "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        code = "void f()\n"
               "{\n"
               "    std::list<int> x;\n"
               "    fun(a && x.size());\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:4]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n" // #4039
              "    std::list<int> x;\n"
              "    fun(width % x.size() != 0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4584
        check("void f() {\n"
              "    std::list<int> x;\n"
              "    if (foo + 1 > x.size()) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f() {\n"
              "    std::list<int> x;\n"
              "    if (x.size() < 1 + foo) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void size2() {
        const char* code = "struct Fred {\n"
                           "    std::list<int> x;\n"
                           "};\n"
                           "struct Wilma {\n"
                           "    Fred f;\n"
                           "    void foo();\n"
                           "};\n"
                           "void Wilma::foo()\n"
                           "{\n"
                           "    if (f.x.size() == 0) {}\n"
                           "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:10]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());
    }

    void size3() {
        const char* code = "namespace N {\n"
                           "    class Zzz {\n"
                           "    public:\n"
                           "        std::list<int> x;\n"
                           "    };\n"
                           "}\n"
                           "using namespace N;\n"
                           "Zzz * zzz;\n"
                           "int main() {\n"
                           "    if (zzz->x.size() > 0) { }\n"
                           "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:10]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());

        code = "namespace N {\n"
               "    class Zzz {\n"
               "    public:\n"
               "        std::list<int> x;\n"
               "    };\n"
               "}\n"
               "using namespace N;\n"
               "int main() {\n"
               "    Zzz * zzz;\n"
               "    if (zzz->x.size() > 0) { }\n"
               "}";
        check(code, false, Standards::CPP03);
        ASSERT_EQUALS("[test.cpp:10]: (performance) Possible inefficient checking for 'x' emptiness.\n", errout.str());
        check(code);
        ASSERT_EQUALS("", errout.str());
    }

    void size4() { // #2652 - don't warn about vector/deque
        check("void f(std::vector<int> &v) {\n"
              "    if (v.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::deque<int> &v) {\n"
              "    if (v.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(std::array<int,3> &a) {\n"
              "    if (a.size() > 0U) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantCondition1() {
        check("void f(string haystack)\n"
              "{\n"
              "    if (haystack.find(needle) != haystack.end())\n"
              "        haystack.remove(needle);"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Redundant checking of STL container element existence before removing it.\n", errout.str());
    }

    void missingInnerComparison1() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        if (a) {\n"
              "            it++;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4] -> [test.cpp:2]: (warning) Missing bounds check for extra iterator increment in loop.\n", errout.str());

        check("void f(std::map<int,int> &ints) {\n"
              "    for (std::map<int,int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        ++it->second;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingInnerComparison2() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        if (a) {\n"
              "            it++;\n"
              "            if (it == ints.end())\n"
              "                return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingInnerComparison3() {
        check("void f(std::set<int> &ints) {\n"
              "    for (std::set<int>::iterator it = ints.begin(); it != ints.end(); ++it) {\n"
              "        for (std::set<int>::iterator it = ints2.begin(); it != ints2.end(); ++it)\n"
              "        { }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingInnerComparison4() {
        check("function f1(std::list<int> &l1) {\n"
              "    for(std::list<int>::iterator i = l1.begin(); i != l1.end(); i++) {\n"
              "        if (*i == 44) {\n"
              "            l1.insert(++i, 55);\n"
              "            break;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("function f1(std::list<int> &l1) {\n"
              "    for(std::list<int>::iterator i = l1.begin(); i != l1.end(); i++) {\n"
              "        if (*i == 44) {\n"
              "            l1.insert(++i, 55);\n"
              "            return;\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingInnerComparison5() {
        check("void f() {\n"
              "    for(it = map1.begin(); it != map1.end(); it++) {\n"
              "        str[i++] = (*it).first;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingInnerComparison6() {
        check("void f(std::string &s) {\n"
              "    for(string::iterator it = s.begin(); it != s.end(); it++) {\n"
              "        it = s.insert(++it, 0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void cstr() {
        check("void f() {\n"
              "    std::string errmsg;\n"
              "    throw errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after throwing exception.\n", errout.str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("const char *get_msg() {\n"
              "    std::ostringstream errmsg;\n"
              "    return errmsg.str().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return std::string(\"ERROR: \" + errmsg).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return (\"ERROR: \" + errmsg).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("const char *get_msg() {\n"
              "    std::string errmsg;\n"
              "    return (\"ERROR: \" + std::string(\"crash me\")).c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("void f() {\n"
              "    std::ostringstream errmsg;\n"
              "    const char *c = errmsg.str().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("std::string f();\n"
              "\n"
              "void foo() {\n"
              "    const char *c = f().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("class Foo {\n"
              "    const char *f();\n"
              "};\n"
              "const char *Foo::f() {\n"
              "    std::string s;\n"
              "    return s.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("class Foo {\n"
              "    std::string GetVal() const;\n"
              "};\n"
              "const char *f() {\n"
              "    Foo f;\n"
              "    return f.GetVal().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("const char* foo() {\n"
              "    static std::string text;\n"
              "    text = \"hello world\\n\";\n"
              "    return text.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #3427

        // Implicit conversion back to std::string
        check("std::string get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant.\n", errout.str());

        check("const std::string& get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant.\n", errout.str());

        check("class Foo {\n"
              "    std::string GetVal() const;\n"
              "};\n"
              "std::string f() {\n"
              "    Foo f;\n"
              "    return f.GetVal().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant.\n", errout.str());

        check("std::string get_msg() {\n"
              "    std::string errmsg;\n"
              "    return errmsg;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string get_msg() {\n" // #3678
              "    MyStringClass errmsg;\n"
              "    return errmsg.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void Foo1(const std::string& str) {}\n"
              "void Foo2(char* c, const std::string str) {}\n"
              "void Foo3(std::string& rstr) {}\n"
              "void Foo4(std::string str, const std::string& str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    std::stringstream ss(\"foo\");\n"
              "    Foo1(str);\n"
              "    Foo1(str.c_str());\n"
              "    Foo2(str.c_str(), str);\n"
              "    Foo2(str.c_str(), str.c_str());\n"
              "    Foo3(str.c_str());\n"
              "    Foo4(str, str);\n"
              "    Foo4(str.c_str(), str);\n"
              "    Foo4(str, str.c_str());\n"
              "    Foo4(ss.str(), ss.str().c_str());\n"
              "    Foo4(str.c_str(), str.c_str());\n"
              "}");

        ASSERT_EQUALS("[test.cpp:9]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant.\n"
                      "[test.cpp:11]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant.\n"
                      "[test.cpp:14]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant.\n"
                      "[test.cpp:15]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant.\n"
                      "[test.cpp:16]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant.\n"
                      "[test.cpp:17]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 1 is slow and redundant.\n"
                      "[test.cpp:17]: (performance) Passing the result of c_str() to a function that takes std::string as argument no. 2 is slow and redundant.\n", errout.str());

        check("void Foo1(const std::string& str) {}\n"
              "void Foo2(char* c, const std::string str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo1(str, foo);\n" // Don't crash
              "    Foo2(str.c_str());\n" // Don't crash
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct Foo {\n"
              "    void func(std::string str) const {}\n"
              "    static void sfunc(std::string str) {}\n"
              "};\n"
              "void func(std::string str) {}\n"
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo foo;\n"
              "    func(str.c_str());\n"
              "    Foo::sfunc(str.c_str());\n"
              "    foo.func(str.c_str());\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (performance) Passing the result of c_str() to a function that takes std::string as argument 1 is slow and redundant.\n"
                           "[test.cpp:10]: (performance) Passing the result of c_str() to a function that takes std::string as argument 1 is slow and redundant.\n",
                           "", errout.str());

        check("void svgFile(const std::string &content, const std::string &fileName, const double end = 1000., const double start = 0.);\n"
              "void Bar(std::string filename) {\n"
              "    std::string str = \"bar\";\n"
              "    std::ofstream svgFile(filename.c_str(), std::ios::trunc);\n"
              "    svgFile << \"test\";\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void Foo(const char* p) {}\n"
              "void Foo(const std::string& str) {Foo(str.c_str());}\n" // Overloaded
              "void Bar() {\n"
              "    std::string str = \"bar\";\n"
              "    Foo(str);\n"
              "    Foo(str.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int atoi(const std::string& str) {\n" // #3729: Don't suggest recursive call
              "    return atoi(str.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "\n"
              "const char *f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("class Fred {\n"
              "    std::string hello();\n"
              "    const char *f();\n"
              "};\n"
              "std::string Fred::hello()\n"
              "{\n"
              "     return \"hello\";\n"
              "}\n"
              "const char *Fred::f()\n"
              "{\n"
              "    return hello().c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        // #4183 - using MyStringClass.c_str()
        check("void a(const std::string &str);\n"
              "\n"
              "void b() {\n"
              "    MyStringClass s;\n"
              "    a(s.c_str());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::string Format(const char * name) {\n" // #4938
              "    return String::Format(\"%s:\", name).c_str();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7480
        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "const char* GetMapAuthor(int index) {\n"
              "    const InternalMapInfo* mapInfo = &internal_getMapInfo;\n"
              "    return mapInfo->author.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "std::string GetMapAuthor(int index) {\n"
              "    const InternalMapInfo* mapInfo = &internal_getMapInfo;\n"
              "    return mapInfo->author.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (performance) Returning the result of c_str() in a function that returns std::string is slow and redundant.\n", errout.str());

        check("struct InternalMapInfo {\n"
              "    std::string author;\n"
              "};\n"
              "const char* GetMapAuthor(int index) {\n"
              "    const InternalMapInfo mapInfo = internal_getMapInfo;\n"
              "    return mapInfo.author.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());

        check("struct S {\n" // #7656
              "    std::string data;\n"
              "};\n"
              "const S& getS();\n"
              "const char* test() {\n"
              "    const struct S &s = getS();\n"
              "    return s.data.c_str();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct S {\n" // #7930
              "    std::string data;\n"
              "};\n"
              "const char* test() {\n"
              "    S s;\n"
              "    std::string &ref = s.data;\n"
              "    return ref.c_str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Dangerous usage of c_str(). The value returned by c_str() is invalid after this call.\n", errout.str());
    }

    void autoPointer() {

        // ticket 2846
        check("void f()\n"
              "{\n"
              "    std::vector<int*> vec;\n"
              "    vec.push_back(new int(3));\n"
              "    std::auto_ptr<int> ret(vec[0]);\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // ticket 2839
        check("template <class MUTEX_TYPE>\n"
              "class Guarded\n"
              "{\n"
              "    typedef std::auto_ptr<MUTEX_TYPE > WriteGuardType;\n"
              "    virtual WriteGuardType getWriteGuard(bool enabledGuard = true);\n"
              "};\n"
              "class SafeSharedMemory : public Guarded<int>\n"
              "{\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    auto_ptr< ns1:::MyClass > y;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2;\n"
              "    p2 = new T;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector< std::auto_ptr< ns1::MyClass> > v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) You can randomly lose access to pointers if you store 'auto_ptr' pointers in an STL container.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::vector< auto_ptr< MyClass> > v;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) You can randomly lose access to pointers if you store 'auto_ptr' pointers in an STL container.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int *i = new int;\n"
              "    auto_ptr<int> x(i);\n"
              "    auto_ptr<int> y;\n"
              "    y = x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Copying 'auto_ptr' pointer to another does not create two equal objects since one has lost its ownership of the pointer.\n", errout.str());

        check("std::auto_ptr<A> function();\n"
              "int quit;"
              "void f() { quit = true; }\n"
             );
        ASSERT_EQUALS("", errout.str());

        // ticket #748
        check("void f()\n"
              "{\n"
              "    T* var = new T[10];\n"
              "    auto_ptr<T> p2( var );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    foo::bar::baz* var = new foo::bar::baz[10];\n"
              "    auto_ptr<foo::bar::baz> p2( var );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2( new T[] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2( new T[5] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<foo::bar> p(new foo::bar[10]);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2;\n"
              "    p2.reset( new T[] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());


        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2( new T[][] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2;\n"
              "    p2 = new T[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T::B> p2;\n"
              "    p2 = new T::B[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T> p2;\n"
              "    p2.reset( new T[10] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    auto_ptr<T::B> p2;\n"
              "    p2.reset( new T::B[10] );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with operator 'new[]'.\n", errout.str());

        // ticket #2887 (infinite loop)
        check("A::A(std::auto_ptr<X> e){}");
        ASSERT_EQUALS("", errout.str());

        // ticket #4390
        check("auto_ptr<ConnectionStringReadStorage> CreateRegistryStringStorage() {\n"
              "    return auto_ptr<ConnectionStringReadStorage>(new RegistryConnectionStringStorage());\n"
              "}\n"
              "\n"
              "void LookupWindowsUserAccountName() {\n"
              "    auto_ptr_array<char> domainName(new char[42]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #749
        check("int main()\n"
              "{\n"
              "    int *i = malloc(sizeof(int));\n"
              "    auto_ptr<int> x(i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> x((int*)malloc(sizeof(int)*4));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> b(static_cast<int*>(malloc(sizeof(int)*4)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> x = (int*)malloc(sizeof(int)*4);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> x = static_cast<int*>(malloc(sizeof(int)*4));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> x;\n"
              "    x.reset((int*)malloc(sizeof(int)*4));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());

        check("int main()\n"
              "{\n"
              "    auto_ptr<int> x;\n"
              "    x.reset(static_cast<int*>(malloc(sizeof(int)*4)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Object pointed by an 'auto_ptr' is destroyed using operator 'delete'. You should not use 'auto_ptr' for pointers obtained with function 'malloc'.\n", errout.str());
    }

    void uselessCalls() {
        check("void f()\n"
              "{\n"
              "    string s1, s2;\n"
              "    s1.swap(s2);\n"
              "    s2.swap(s2);\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    std::string s1, s2;\n"
              "    s1.swap(s2);\n"
              "    s2.swap(s2);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (performance) It is inefficient to swap a object with itself by calling 's2.swap(s2)'\n", errout.str());

        check("void f()\n"
              "{\n"
              "    std::string s1, s2;\n"
              "    s1.compare(s2);\n"
              "    s2.compare(s2);\n"
              "    s1.compare(s2.c_str());\n"
              "    s1.compare(0, s1.size(), s1);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (warning) It is inefficient to call 's2.compare(s2)' as it always returns 0.\n", errout.str());

        // #7370 False positive uselessCallsCompare on unknown type
        check("class ReplayIteratorImpl{\n"
              "  int Compare(ReplayIteratorImpl* other) {\n"
              "    int cmp;\n"
              "    int ret = cursor_->compare(cursor_, other->cursor_, &cmp);\n"
              "    return (cmp);\n"
              "  }\n"
              "  WT_CURSOR *cursor_;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int x=1;\n"
              "    std::string s1, s2;\n"
              "    s1 = s1.substr();\n"
              "    s2 = s1.substr(x);\n"
              "    s1 = s2.substr(0, x);\n"
              "    s1 = s2.substr(0,std::string::npos);\n"
              "    s1 = s2.substr(x+5-n, 0);\n"
              "};");
        ASSERT_EQUALS("[test.cpp:5]: (performance) Ineffective call of function \'substr\' because it returns a copy of "
                      "the object. Use operator= instead.\n"
                      "[test.cpp:8]: (performance) Ineffective call of function \'substr\' because it returns a copy of "
                      "the object. Use operator= instead.\n"
                      "[test.cpp:9]: (performance) Ineffective call of function \'substr\' because it returns an empty string.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int x=1;\n"
              "    string s1, s2;\n"
              "    s1 = s1.substr();\n"
              "    s2 = s1.substr(x);\n"
              "    s1 = s2.substr(0, x);\n"
              "    s1 = s2.substr(0,std::string::npos);\n"
              "    s1 = s2.substr(x+5-n, 0);\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("int main()\n"
              "{\n"
              "    std::string str = \"a1b1\";\n"
              "    return str.find(str[1], 2);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(std::vector<int>& v) {\n"
              "    v.empty();\n"
              "    return v.empty();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Ineffective call of function 'empty()'. Did you intend to call 'clear()' instead?\n", errout.str());

        check("void f() {\n" // #4938
              "    OdString str;\n"
              "    str.empty();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n" // #4032
              "    const std::string greeting(\"Hello World !!!\");\n"
              "    const std::string::size_type npos = greeting.rfind(\" \");\n"
              "    if (npos != std::string::npos)\n"
              "        std::cout << greeting.substr(0, npos) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    std::vector<int> a;\n"
              "    std::remove(a.begin(), a.end(), val);\n"
              "    std::remove_if(a.begin(), a.end(), val);\n"
              "    std::unique(a.begin(), a.end(), val);\n"
              "    x = std::remove(a.begin(), a.end(), val);\n"
              "    a.erase(std::remove(a.begin(), a.end(), val));\n"
              "    std::remove(\"foo.txt\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of std::remove() ignored. Elements remain in container.\n"
                      "[test.cpp:4]: (warning) Return value of std::remove_if() ignored. Elements remain in container.\n"
                      "[test.cpp:5]: (warning) Return value of std::unique() ignored. Elements remain in container.\n", errout.str());

        // #4431 - fp
        check("bool f() {\n"
              "    return x ? true : (y.empty());\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #8360
        check("void f(std::string s) {\n"
              "    for (;s.empty();) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void stabilityOfChecks() {
        // Stability test: 4684 cppcheck crash in template function call.
        check("template<class T>\n"
              "class EffectivityRangeData {};\n"
              "template<class T>\n"
              "class EffectivityRange {\n"
              "    void unite() {\n"
              "        x < vector < EffectivityRangeData<int >> >();\n"
              "        EffectivityRange<int> er;\n"
              "    }\n"
              "    void shift() { EffectivityRangeData<int>::iterator it;  } \n"
              "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dereferenceInvalidIterator() {
        // Test simplest "if" with && case
        check("void foo(std::string::iterator& i) {\n"
              "    if (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        check("void foo(std::string::iterator& i) {\n"
              "    if(foo) { bar(); }\n"
              "    else if (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test suggested correction doesn't report an error
        check("void foo(std::string::iterator& i) {\n"
              "    if (i != str.end() && std::isalpha(*i)) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Test "while" with "&&" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        check("void foo(std::string::iterator& i) {\n"
              "    do {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    } while (std::isalpha(*i) && i != str.end());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test "while" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (!(!std::isalpha(*i) || i == str.end())) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test fix for "while" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    while (!(i == str.end() || !std::isalpha(*i))) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Test "for" with "&&" case
        check("void foo(std::string::iterator& i) {\n"
              "    for (; std::isalpha(*i) && i != str.end() ;) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test "for" with "||" case
        check("void foo(std::string::iterator& i) {\n"
              "    for (; std::isalpha(*i) || i == str.end() ;) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test that a dereference outside the condition part of a "for"
        // loop does not result in a false positive
        check("void foo(std::string::iterator& i) {\n"
              "    for (char c = *i; isRunning && i != str.end() ;) {\n"
              "        std::cout << c;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Test that other "&&" terms in the condition don't invalidate the check
        check("void foo(char* c, std::string::iterator& i) {\n"
              "    if (*c && std::isalpha(*i) && i != str.end()) {\n"
              "        std::cout << *i;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test that dereference of different variable doesn't trigger a false positive
        check("void foo(const char* c, std::string::iterator& i) {\n"
              "    if (std::isalpha(*c) && i != str.end()) {\n"
              "        std::cout << *c;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // Test case involving "rend()" instead of "end()"
        check("void foo(std::string::iterator& i) {\n"
              "    while (std::isalpha(*i) && i != str.rend()) {\n"
              "        std::cout << *i;\n"
              "        i ++;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Possible dereference of an invalid iterator: i\n", errout.str());

        // Test that mixed "&&" and "||" don't result in a false positive
        check("void foo(std::string::iterator& i) {\n"
              "    if ((i == str.end() || *i) || (isFoo() && i != str.end())) {\n"
              "        std::cout << \"foo\";\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void dereferenceInvalidIterator2() {
        // Self-implemented iterator class
        check("class iterator {\n"
              "public:\n"
              "    CCommitPointer m_ptr;\n"
              "    iterator() {}\n"
              "    CCommitPointer& operator*() {\n"
              "        return m_ptr;\n"
              "    }\n"
              "    CCommitPointer* operator->() {\n"
              "        return &m_ptr;\n"
              "    }\n"
              "    iterator& operator++() {\n"
              "        ++m_ptr.m_place;\n"
              "        return *this;\n"
              "    }\n"
              "    }; \n"
              "    iterator begin() {\n"
              "    iterator it; \n"
              "    it->m_place = 0;\n"
              "    return it; \n"
              "}\n", true);
        ASSERT_EQUALS("[test.cpp:18]: (error, inconclusive) Invalid iterator 'it' used.\n", errout.str());
    }

    void loopAlgoElementAssign() {
        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = 1;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::fill algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = x + 1;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo(int a, int b) {\n"
              "    for(int& x:v)\n"
              "        x = a + b;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::fill or std::generate algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x += 1;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = f();\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::generate algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        f();\n"
              "        x = 1;\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        x = 1;\n"
              "        f();\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        // There should probably be a message for unconditional break
        check("void foo() {\n"
              "    for(int& x:v) {\n"
              "        x = 1;\n"
              "        break;\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x = ++x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());
    }

    void loopAlgoAccumulateAssign() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n += x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = n + x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n += 1;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::distance algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = n + 1;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::distance algorithm instead of a raw loop.\n", errout.str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b &= f(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b |= f(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b = b && f(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool f(int);\n"
              "void foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v)\n"
              "        b = b || f(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int& x:v)\n"
              "        n = ++x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());
    }

    void loopAlgoContainerInsert() {
        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::copy algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(f(x));\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(x + 1);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(x);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::copy algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(f(x));\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_front(x + 1);\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(v);\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v)\n"
              "        c.push_back(0);\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());
    }

    void loopAlgoIncrement() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n++;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::distance algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        ++n;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::distance algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        x++;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    for(int& x:v)\n"
              "        ++x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:3]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());
    }

    void loopAlgoConditional() {
        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        if (pred(x)) {\n"
              "            x = 1; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::replace_if algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n += x; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n += 1; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::count_if algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            n++; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::count_if algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        if (pred(x)) {\n"
              "            x = x + 1; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::transform algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            c.push_back(x); \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::copy_if algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            return false; \n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::any_of algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            break; \n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::any_of algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void f();\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            f(); \n"
              "            break; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::any_of algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "void f(int x);\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            f(x); \n"
              "            break; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Consider using std::find_if algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b = true;\n"
              "        }\n"
              "    }\n"
              "    if(b) {}\n"
              "    return true;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b |= true;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    bool b = false;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            b &= true;\n"
              "        }\n"
              "    }\n"
              "    return true;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:6]: (style) Consider using std::any_of, std::all_of, std::none_of, or std::accumulate algorithm instead of a raw loop.\n", errout.str());

        check("bool pred(int x);\n"
              "bool foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            return false; \n"
              "        }\n"
              "        return true;\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        // There is no transform_if
        check("bool pred(int x);\n"
              "void foo() {\n"
              "    std::vector<int> c;\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            c.push_back(x + 1); \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        check("bool pred(int x);\n"
              "void foo() {\n"
              "    for(int& x:v) {\n"
              "        x++;\n"
              "        if (pred(x)) {\n"
              "            x = 1; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());

        check("bool pred(int x);\n"
              "void f();\n"
              "void foo() {\n"
              "    for(int x:v) {\n"
              "        if (pred(x)) {\n"
              "            if(x) { return; }\n"
              "            break; \n"
              "        }\n"
              "    }\n"
              "}\n",
              true);
        ASSERT_EQUALS("", errout.str());
    }

    void loopAlgoMinMax() {
        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > n ? x : n;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::max_element algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x < n ? x : n;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::min_element algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > n ? n : x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::min_element algorithm instead of a raw loop.\n", errout.str());

        check("void foo() {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x < n ? n : x;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::max_element algorithm instead of a raw loop.\n", errout.str());

        check("void foo(int m) {\n"
              "    int n = 0;\n"
              "    for(int x:v)\n"
              "        n = x > m ? x : n;\n"
              "}\n",
              true);
        ASSERT_EQUALS("[test.cpp:4]: (style) Consider using std::accumulate algorithm instead of a raw loop.\n", errout.str());
    }
};

REGISTER_TEST(TestStl)
