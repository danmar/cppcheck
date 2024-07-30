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
#include "helpers.h"
#include "token.h"
#include "programmemory.h"
#include "vfvalue.h"

class TestProgramMemory : public TestFixture {
public:
    TestProgramMemory() : TestFixture("TestProgramMemory") {}

private:
    void run() override {
        TEST_CASE(copyOnWrite);
    }

    void copyOnWrite() const {
        SimpleTokenList tokenlist("1+1;");
        Token* tok = tokenlist.front();
        const nonneg int id = 123;
        tok->exprId(id);

        ProgramMemory pm;
        const ValueFlow::Value* v = pm.getValue(id);
        ASSERT(!v);
        pm.setValue(tok, ValueFlow::Value{41});

        v = pm.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);

        // create a copy
        ProgramMemory pm2 = pm;

        // make sure the value was copied
        v = pm2.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);

        // set a value in the copy to trigger copy-on-write
        pm2.setValue(tok, ValueFlow::Value{42});

        // make another copy and set another value
        ProgramMemory pm3 = pm2;

        // set a value in the copy to trigger copy-on-write
        pm3.setValue(tok, ValueFlow::Value{43});

        // make sure the value was set
        v = pm2.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(42, v->intvalue);

        // make sure the value was set
        v = pm3.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(43, v->intvalue);

        // make sure the original value remains unchanged
        v = pm.getValue(id);
        ASSERT(v);
        ASSERT_EQUALS(41, v->intvalue);
    }
};

REGISTER_TEST(TestProgramMemory)
