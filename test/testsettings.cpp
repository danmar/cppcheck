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


#include "errortypes.h"
#include "settings.h"
#include "fixture.h"
#include "helpers.h"
#include "suppressions.h"

class TestSettings : public TestFixture {
public:
    TestSettings() : TestFixture("TestSettings") {}

private:
    void run() override {
        TEST_CASE(simpleEnableGroup);
        TEST_CASE(loadCppcheckCfg);
        TEST_CASE(loadCppcheckCfgSafety);
        TEST_CASE(getNameAndVersion);
        TEST_CASE(ruleTexts);
        TEST_CASE(checkLevelDefault);
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
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            "{}\n");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            "{\n");
            ASSERT_EQUALS("not a valid JSON - syntax error at line 2 near: ", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": ""}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS("", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": "product"}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS("product", s.cppcheckCfgProductName);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"productName": 1}\n)");
            ASSERT_EQUALS("'productName' is not a string", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": ""}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS("", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": "about"}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS("about", s.cppcheckCfgAbout);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"about": 1}\n)");
            ASSERT_EQUALS("'about' is not a string", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": 1}\n)");
            ASSERT_EQUALS("'addons' is not an array", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": ["addon"]}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(1, s.addons.size());
            ASSERT_EQUALS("addon", *s.addons.cbegin());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": [1]}\n)");
            ASSERT_EQUALS("'addons' array entry is not a string", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"addons": []}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(0, s.addons.size());
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": 1}\n)");
            ASSERT_EQUALS("'suppressions' is not an array", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": ["id"]}\n)");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(1, s.supprs.nomsg.getSuppressions().size());
            ASSERT_EQUALS("id", s.supprs.nomsg.getSuppressions().cbegin()->errorId);
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [""]}\n)");
            ASSERT_EQUALS("could not parse suppression '' - Failed to add suppression. No id.", Settings::loadCppcheckCfg(s, s.supprs));
        }
        {
            Settings s;
            ScopedFile file("cppcheck.cfg",
                            R"({"suppressions": [1]}\n)");
            ASSERT_EQUALS("'suppressions' array entry is not a string", Settings::loadCppcheckCfg(s, s.supprs));
        }

        // TODO: test with FILESDIR
    }

    void loadCppcheckCfgSafety() const
    {
        // Test the "safety" flag
        {
            Settings s;
            s.safety = false;
            ScopedFile file("cppcheck.cfg", "{}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(false, s.safety);
        }

        {
            Settings s;
            s.safety = true;
            ScopedFile file("cppcheck.cfg", "{\"safety\": false}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(true, s.safety);
        }

        {
            Settings s;
            s.safety = false;
            ScopedFile file("cppcheck.cfg", "{\"safety\": true}");
            ASSERT_EQUALS("", Settings::loadCppcheckCfg(s, s.supprs));
            ASSERT_EQUALS(true, s.safety);
        }
    }

    void getNameAndVersion() const
    {
        {
            const auto nameVersion = Settings::getNameAndVersion("Cppcheck Premium 12.3.4");
            ASSERT_EQUALS("Cppcheck Premium", nameVersion.first);
            ASSERT_EQUALS("12.3.4", nameVersion.second);
        }

        {
            const auto nameVersion = Settings::getNameAndVersion("Cppcheck Premium 12.3.4s");
            ASSERT_EQUALS("Cppcheck Premium", nameVersion.first);
            ASSERT_EQUALS("12.3.4s", nameVersion.second);
        }
    }

    void ruleTexts() const
    {
        Settings s;
        s.setMisraRuleTexts("1.1 text 1\n1.2 text 2\n1.3 text 3\r\n");
        ASSERT_EQUALS("text 1", s.getMisraRuleText("misra-c2012-1.1", "---"));
        ASSERT_EQUALS("text 2", s.getMisraRuleText("misra-c2012-1.2", "---"));
        ASSERT_EQUALS("text 3", s.getMisraRuleText("misra-c2012-1.3", "---"));
    }

    void checkLevelDefault() const
    {
        Settings s;
        ASSERT_EQUALS_ENUM(s.checkLevel, Settings::CheckLevel::exhaustive);
        ASSERT_EQUALS(s.vfOptions.maxIfCount, -1);
        ASSERT_EQUALS(s.vfOptions.maxSubFunctionArgs, 256);
        ASSERT_EQUALS(true, s.vfOptions.doConditionExpressionAnalysis);
        ASSERT_EQUALS(-1, s.vfOptions.maxForwardBranches);
    }
};

REGISTER_TEST(TestSettings)
