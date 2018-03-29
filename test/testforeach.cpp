/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

#include "foreach.h"
#include "testsuite.h"
#include <vector>


class TestForeach : public TestFixture {
public:
    TestForeach() 
    : TestFixture("TestForeach"), getVectorOnceCount(0) 
    {}

private:

    int getVectorOnceCount;
    const std::vector<int>& getVectorOnce()
    {
        static std::vector<int> v(5, 2);
        getVectorOnceCount++;
        return v;
    }

    std::vector<int> getVector()
    {
        std::vector<int> v(5, 2);
        return v;
    }

    void run() {
        TEST_CASE(container);
        TEST_CASE(rvalue);
        TEST_CASE(checkBreak);
        TEST_CASE(checkContinue);
        TEST_CASE(arrays);
        TEST_CASE(callOnce);
        TEST_CASE(nested);
    }

    void container() {
        std::vector<int> v(5, 2);
        int total = 0;
        int count = 0;

        FOREACH(int x, v) {
            count++;
            total += x;
        }

        ASSERT_EQUALS(total, 10);
        ASSERT_EQUALS(count, 5);
    }

    void rvalue() {
        int total = 0;
        int count = 0;

        FOREACH(int x, getVector()) {
            count++;
            total += x;
        }

        ASSERT_EQUALS(total, 10);
        ASSERT_EQUALS(count, 5);
    }

    void checkBreak() {
        std::vector<int> v(5, 2);
        int total = 0;
        int count = 0;

        FOREACH(int x, v) {
            count++;
            if (count == 3) break;
            total += x;
        }

        ASSERT_EQUALS(total, 4);
        ASSERT_EQUALS(count, 3);
    }

    void checkContinue() {
        std::vector<int> v(5, 2);
        int total = 0;
        int count = 0;

        FOREACH(int x, v) {
            count++;
            if (count == 3) continue;
            total += x;
        }

        ASSERT_EQUALS(total, 8);
        ASSERT_EQUALS(count, 5);
    }

    void arrays() {
        int arr[] = {2, 2, 2, 2, 2};
        int total = 0;
        int count = 0;

        FOREACH(int x, arr) {
            count++;
            total += x;
        }

        ASSERT_EQUALS(total, 10);
        ASSERT_EQUALS(count, 5);
    }

    void callOnce() {
        int total = 0;
        int count = 0;

        FOREACH(int x, getVectorOnce()) {
            count++;
            total += x;
        }

        ASSERT_EQUALS(total, 10);
        ASSERT_EQUALS(count, 5);
        ASSERT_EQUALS(getVectorOnceCount, 1);
    }

    void nested() {
        std::vector<int> v(5, 2);
        int total = 0;
        int count = 0;

        FOREACH(int x, v) {
            FOREACH(int y, v) {
                count++;
                total += x;
                (void)y;
            }
        }

        ASSERT_EQUALS(total, 50);
        ASSERT_EQUALS(count, 25);
    }

};

REGISTER_TEST(TestForeach)
