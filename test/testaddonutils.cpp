/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#include "addonutils.h"
#include "settings.h"
#include "testsuite.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <fstream>
#include <cstdio>

class TestAddonUtils : public TestFixture {
public:
    TestAddonUtils() : TestFixture("TestAddonUtils") {}

private:
    Settings settings;

    void run() OVERRIDE {
        settings.exename = "cppcheck";

        TEST_CASE(testConstructors);
        TEST_CASE(testConstructorsJson);
        TEST_CASE(testBrokenJson);
        TEST_CASE(testAppendArgs);
        TEST_CASE(testEnv);
    }

    void testConstructors() const
    {
        ASSERT_NO_THROW(Addon("misra.py", settings.exename));
        ASSERT_NO_THROW(Addon("naming.py", settings.exename));
        ASSERT_NO_THROW(Addon("y2038.py", settings.exename));
        ASSERT_NO_THROW(Addon("cert.py", settings.exename));

        ASSERT_NO_THROW(Addon("misra", settings.exename));
        ASSERT_NO_THROW(Addon("naming", settings.exename));
        ASSERT_NO_THROW(Addon("y2038", settings.exename));
        ASSERT_NO_THROW(Addon("cert", settings.exename));

        ASSERT_NO_THROW(Addon("naming.json", settings.exename));

        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("misra.foo", settings.exename), InternalError);
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("misra.", settings.exename), InternalError);
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon(".py", settings.exename), InternalError);
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon(".", settings.exename), InternalError);
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("", settings.exename), InternalError);
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("/dev/null", settings.exename), InternalError);
    }

    void testConstructorsJson() const
    {
        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("test_addon.json", settings.exename), InternalError);

        std::ofstream test_addon { "test_addon.json" };
        test_addon << "{ \"script\": \"addons/misra.py\" }" << std::endl;
        ASSERT_NO_THROW(Addon("test_addon.json", settings.exename));
#ifdef _WIN32
        DeleteFile("test_addon.json");
#else
        std::remove("test_addon.json");
#endif // _WIN32

        // cppcheck-suppress unusedScopedObject
        ASSERT_THROW(Addon("test_addon.json", settings.exename), InternalError);
    }

    void testBrokenJson() const
    {
        {
            std::ofstream test_addon { "test_addon_broken.json" };
            test_addon << "{ \"script\": \"addons/misra.py }" << std::endl; // missing quote
            // cppcheck-suppress unusedScopedObject
            ASSERT_THROW(Addon("test_addon_broken.json", settings.exename), InternalError);
            std::remove("test_addon_broken.json");
        }

        {
            std::ofstream test_addon { "test_addon_broken.json" };
            test_addon << "{ \"script\": \"addons/misra.py \"" << std::endl; // missing }
            // cppcheck-suppress unusedScopedObject
            ASSERT_THROW(Addon("test_addon_broken.json", settings.exename), InternalError);
            std::remove("test_addon_broken.json");
        }
    }

    void testAppendArgs() const
    {
        auto addon = Addon("misra.py", settings.exename);
        auto saved_args = addon.mArgs;

        addon.appendArgs("");
        ASSERT_EQUALS(saved_args, addon.mArgs);

        addon.appendArgs("--arg1");
        ASSERT_EQUALS(addon.mArgs[0], ' ');
    }

    void testEnv() const
    {
        auto addon = Addon("misra.py", settings.exename);

        ASSERT_EQUALS(addon.getEnvString(), "");

        addon.setEnv("VAR1", "21");
        ASSERT_EQUALS(addon.getEnvString(), "VAR1=21 ");
        addon.setEnv("VAR2", "42");
        ASSERT_EQUALS(addon.getEnvString(), "VAR1=21 VAR2=42 ");
        addon.setEnv("VAR1", "42");
        ASSERT_EQUALS(addon.getEnvString(), "VAR1=42 VAR2=42 ");

        addon.clearEnv();
        ASSERT_EQUALS(addon.getEnvString(), "");
    }
};

REGISTER_TEST(TestAddonUtils)
