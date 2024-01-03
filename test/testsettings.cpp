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


#include "errortypes.h"
#include "settings.h"
#include "fixture.h"
#include "helpers.h"

class TestSettings : public TestFixture {
public:
    TestSettings() : TestFixture("TestSettings") {}

private:
    void run() override {
        TEST_CASE(simpleEnableGroup);
        TEST_CASE(loadCppcheckCfg);
    }

    void simpleEnableGroup() const {
        SimpleEnableGroup<Checks> group;

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.fill();

        ASSERT_EQUALS(4294967295, group.intValue());
        ASSERT_EQUALS(true, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(true, group.isEnabled(Checks::internalCheck));

        group.clear();

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.enable(Checks::unusedFunction);
        group.setEnabled(Checks::missingInclude, true);

        ASSERT_EQUALS(3, group.intValue());
        ASSERT_EQUALS(true, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.disable(Checks::unusedFunction);
        group.setEnabled(Checks::missingInclude, false);

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        SimpleEnableGroup<Checks> newGroup;
        newGroup.enable(Checks::missingInclude);

        group.enable(newGroup);

        ASSERT_EQUALS(2, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));

        group.disable(newGroup);

        ASSERT_EQUALS(0, group.intValue());
        ASSERT_EQUALS(false, group.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, group.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, group.isEnabled(Checks::internalCheck));
    }

    void loadCppcheckCfg()
    {
        {
            Settings s;
            ASSERT_EQUALS("", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            "{}\n");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            "{\n");
            ASSERT_EQUALS("not a valid JSON - syntax error at line 2 near: ", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": ""}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS("", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": "product"}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS("product", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": 1}\n)");
            ASSERT_EQUALS("'productName' is not a string", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": ""}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS("", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": "about"}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS("about", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": 1}\n)");
            ASSERT_EQUALS("'about' is not a string", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": 1}\n)");
            ASSERT_EQUALS("'addons' is not an array", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": ["addon"]}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS(1, s.addons.size());
            ASSERT_EQUALS("addon", *s.addons.cbegin());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": [1]}\n)");
            ASSERT_EQUALS("'addons' array entry is not a string", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": 1}\n)");
            ASSERT_EQUALS("'suppressions' is not an array", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": ["id"]}\n)");
            ASSERT_EQUALS("", s.loadCppcheckCfg());
            ASSERT_EQUALS(1, s.nomsg.getSuppressions().size());
            ASSERT_EQUALS("id", s.nomsg.getSuppressions().cbegin()->errorId);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [""]}\n)");
            ASSERT_EQUALS("could not parse suppression '' - Failed to add suppression. No id.", s.loadCppcheckCfg());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [1]}\n)");
            ASSERT_EQUALS("'suppressions' array entry is not a string", s.loadCppcheckCfg());
        }

        // TODO: test with FILESDIR
    }
};

REGISTER_TEST(TestSettings)
