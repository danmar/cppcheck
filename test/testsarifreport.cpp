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

#include "sarifreport.h"
#include "errorlogger.h"
#include "fixture.h"

class TestSarifReport : public TestFixture {
public:
    TestSarifReport() : TestFixture("TestSarifReport") {}

private:

    void run() override {
        TEST_CASE(locationLineColumn);
    }

    void locationLineColumn() const {
        std::list<ErrorMessage::FileLocation> callStack{
            {"test.c", 0, 0}
        };
        ErrorMessage msg(callStack, "test.c", Severity::error, "foo", "bar", Certainty::normal);

        const picojson::array& locs = SarifReport::serializeLocations(msg);
        ASSERT_EQUALS(1, locs.size());
        ASSERT(locs[0].is<picojson::object>());
        const picojson::object& loc1 = locs[0].get<picojson::object>();
        const picojson::object& physicalLocation = loc1.at("physicalLocation").get<picojson::object>();
        const picojson::object& region = physicalLocation.at("region").get<picojson::object>();
        ASSERT_EQUALS(1LL, region.at("startColumn").get<int64_t>());
        ASSERT_EQUALS(1LL, region.at("endColumn").get<int64_t>());
        ASSERT_EQUALS(1LL, region.at("startLine").get<int64_t>());
        ASSERT_EQUALS(1LL, region.at("endLine").get<int64_t>());
    }
};

REGISTER_TEST(TestSarifReport)
