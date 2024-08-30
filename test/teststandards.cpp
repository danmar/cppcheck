/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "fixture.h"
#include "standards.h"

class TestStandards : public TestFixture {
public:
    TestStandards() : TestFixture("TestStandards") {}

private:
    void run() override {
        TEST_CASE(set);
        TEST_CASE(setAlias1);
        TEST_CASE(setAlias2);
        TEST_CASE(getC);
        TEST_CASE(getCPP);
        TEST_CASE(setStd);
    }

    void set() const {
        Standards stds;
        ASSERT_EQUALS_ENUM(Standards::CLatest, stds.c);
        ASSERT_EQUALS("", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, stds.cpp);
        ASSERT_EQUALS("", stds.stdValueCPP);

        ASSERT_EQUALS(true, stds.setC("c99"));
        ASSERT_EQUALS_ENUM(Standards::C99, stds.c);
        ASSERT_EQUALS("c99", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, stds.cpp);
        ASSERT_EQUALS("", stds.stdValueCPP);

        ASSERT_EQUALS(true, stds.setC("c11"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, stds.cpp);
        ASSERT_EQUALS("", stds.stdValueCPP);

        ASSERT_EQUALS(true, stds.setCPP("c++11"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP11, stds.cpp);
        ASSERT_EQUALS("c++11", stds.stdValueCPP);

        ASSERT_EQUALS(true, stds.setCPP("c++23"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP23, stds.cpp);
        ASSERT_EQUALS("c++23", stds.stdValueCPP);

        ASSERT_EQUALS(false, stds.setC("c77"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP23, stds.cpp);
        ASSERT_EQUALS("c++23", stds.stdValueCPP);

        ASSERT_EQUALS(false, stds.setCPP("c+77"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP23, stds.cpp);
        ASSERT_EQUALS("c++23", stds.stdValueCPP);

        ASSERT_EQUALS(false, stds.setC("C23"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP23, stds.cpp);
        ASSERT_EQUALS("c++23", stds.stdValueCPP);

        ASSERT_EQUALS(false, stds.setCPP("C++11"));
        ASSERT_EQUALS_ENUM(Standards::C11, stds.c);
        ASSERT_EQUALS("c11", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPP23, stds.cpp);
        ASSERT_EQUALS("c++23", stds.stdValueCPP);
    }

    void setAlias1() const {
        Standards stds;
        TODO_ASSERT_EQUALS(true, false, stds.setCPP("gnu++11"));
        ASSERT_EQUALS_ENUM(Standards::CLatest, stds.c);
        ASSERT_EQUALS("", stds.stdValueC);
        TODO_ASSERT_EQUALS_ENUM(Standards::CPP11, Standards::CPPLatest, stds.cpp);
        TODO_ASSERT_EQUALS("gnu++11", "", stds.stdValueCPP);
    }

    void setAlias2() const {
        Standards stds;
        TODO_ASSERT_EQUALS(true, false, stds.setC("gnu17"));
        TODO_ASSERT_EQUALS_ENUM(Standards::C17, Standards::CLatest, stds.c);
        TODO_ASSERT_EQUALS("gnu17", "", stds.stdValueC);
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, stds.cpp);
        ASSERT_EQUALS("", stds.stdValueCPP);
    }

    void getC() const {
        ASSERT_EQUALS_ENUM(Standards::C99, Standards::getC("c99"));
        ASSERT_EQUALS_ENUM(Standards::C11, Standards::getC("c11"));
        TODO_ASSERT_EQUALS_ENUM(Standards::C17, Standards::CLatest, Standards::getC("gnu17"));
        TODO_ASSERT_EQUALS_ENUM(Standards::C11, Standards::CLatest, Standards::getC("iso9899:2011"));

        ASSERT_EQUALS_ENUM(Standards::CLatest, Standards::getC(""));
        ASSERT_EQUALS_ENUM(Standards::CLatest, Standards::getC("c77"));
        ASSERT_EQUALS_ENUM(Standards::CLatest, Standards::getC("C99"));
    }

    void getCPP() const {
        ASSERT_EQUALS_ENUM(Standards::CPP11, Standards::getCPP("c++11"));
        ASSERT_EQUALS_ENUM(Standards::CPP23, Standards::getCPP("c++23"));
        TODO_ASSERT_EQUALS_ENUM(Standards::CPP23, Standards::CPPLatest, Standards::getCPP("gnu++23"));

        ASSERT_EQUALS_ENUM(Standards::CPPLatest, Standards::getCPP(""));
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, Standards::getCPP("c++77"));
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, Standards::getCPP("C++11"));
    }

    void setStd() const {
        Standards stds;
        ASSERT(stds.setStd("c11"));
        ASSERT(stds.setStd("c++11"));
        TODO_ASSERT(stds.setStd("gnu11"));
        TODO_ASSERT(stds.setStd("gnu++17"));
        TODO_ASSERT(stds.setStd("iso9899:2011"));

        ASSERT(!stds.setStd("C++11"));
        ASSERT(!stds.setStd("Gnu++11"));
    }
};

REGISTER_TEST(TestStandards)
