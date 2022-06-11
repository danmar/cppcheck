/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "platform.h"
#include "testsuite.h"

#include <tinyxml2.h>


class TestPlatform : public TestFixture {
public:
    TestPlatform() : TestFixture("TestPlatform") {}

private:
    void run() override {
        TEST_CASE(empty);
        TEST_CASE(valid_config_native_1);
        TEST_CASE(valid_config_native_2);
        TEST_CASE(valid_config_file_1);
        TEST_CASE(valid_config_file_2);
        TEST_CASE(valid_config_file_3);
        TEST_CASE(valid_config_file_4);
        TEST_CASE(invalid_config_file_1);
        TEST_CASE(empty_elements);
    }

    static bool readPlatform(cppcheck::Platform& platform, const char* xmldata) {
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata);
        return platform.loadFromXmlDocument(&doc);
    }

    void empty() const {
        // An empty platform file does not change values, only the type.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n<platform/>";
        cppcheck::Platform platform;
        ASSERT(platform.platform(cppcheck::Platform::Win64));
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(cppcheck::Platform::PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS('\0', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
    }

    void valid_config_native_1() {
        // Verify if native Win32A platform is loaded correctly
        cppcheck::Platform platform;
        ASSERT(platform.platform(cppcheck::Platform::Win32A));
        ASSERT_EQUALS(cppcheck::Platform::Win32A, platform.platformType);
        ASSERT(platform.isWindowsPlatform());
        ASSERT_EQUALS('\0', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
    }

    void valid_config_native_2() {
        // Verify if native Unix64 platform is loaded correctly
        cppcheck::Platform platform;
        ASSERT(platform.platform(cppcheck::Platform::Unix64));
        ASSERT_EQUALS(cppcheck::Platform::Unix64, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS('\0', platform.defaultSign);
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(32, platform.int_bit);
        ASSERT_EQUALS(8, platform.sizeof_long);
        ASSERT_EQUALS(64, platform.long_bit);
    }

    void valid_config_file_1() {
        // Valid platform configuration with all possible values specified.
        // Similar to the avr8 platform file.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit>8</char_bit>\n"
                               "  <default-sign>unsigned</default-sign>\n"
                               "  <sizeof>\n"
                               "    <bool>1</bool>\n"
                               "    <short>2</short>\n"
                               "    <int>2</int>\n"
                               "    <long>4</long>\n"
                               "    <long-long>8</long-long>\n"
                               "    <float>4</float>\n"
                               "    <double>4</double>\n"
                               "    <long-double>4</long-double>\n"
                               "    <pointer>2</pointer>\n"
                               "    <size_t>2</size_t>\n"
                               "    <wchar_t>2</wchar_t>\n"
                               "  </sizeof>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(platform.PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS('u', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(2, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
        ASSERT_EQUALS(4, platform.sizeof_float);
        ASSERT_EQUALS(4, platform.sizeof_double);
        ASSERT_EQUALS(4, platform.sizeof_long_double);
        ASSERT_EQUALS(2, platform.sizeof_pointer);
        ASSERT_EQUALS(2, platform.sizeof_size_t);
        ASSERT_EQUALS(2, platform.sizeof_wchar_t);
        ASSERT_EQUALS(16, platform.short_bit);
        ASSERT_EQUALS(16, platform.int_bit);
        ASSERT_EQUALS(32, platform.long_bit);
        ASSERT_EQUALS(64, platform.long_long_bit);
    }

    void valid_config_file_2() {
        // Valid platform configuration with all possible values specified and
        // char_bit > 8.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit>20</char_bit>\n"
                               "  <default-sign>signed</default-sign>\n"
                               "  <sizeof>\n"
                               "    <bool>1</bool>\n"
                               "    <short>2</short>\n"
                               "    <int>3</int>\n"
                               "    <long>4</long>\n"
                               "    <long-long>5</long-long>\n"
                               "    <float>6</float>\n"
                               "    <double>7</double>\n"
                               "    <long-double>8</long-double>\n"
                               "    <pointer>9</pointer>\n"
                               "    <size_t>10</size_t>\n"
                               "    <wchar_t>11</wchar_t>\n"
                               "  </sizeof>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(platform.PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS(20, platform.char_bit);
        ASSERT_EQUALS('s', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(3, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(5, platform.sizeof_long_long);
        ASSERT_EQUALS(6, platform.sizeof_float);
        ASSERT_EQUALS(7, platform.sizeof_double);
        ASSERT_EQUALS(8, platform.sizeof_long_double);
        ASSERT_EQUALS(9, platform.sizeof_pointer);
        ASSERT_EQUALS(10, platform.sizeof_size_t);
        ASSERT_EQUALS(11, platform.sizeof_wchar_t);
        ASSERT_EQUALS(40, platform.short_bit);
        ASSERT_EQUALS(60, platform.int_bit);
        ASSERT_EQUALS(80, platform.long_bit);
        ASSERT_EQUALS(100, platform.long_long_bit);
    }

    void valid_config_file_3() {
        // Valid platform configuration without any usable information.
        // Similar like an empty file.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit1>8</char_bit1>\n"
                               "  <default-sign1>unsigned</default-sign1>\n"
                               "  <sizeof1>\n"
                               "    <bool1>1</bool1>\n"
                               "    <short1>2</short1>\n"
                               "    <int1>3</int1>\n"
                               "    <long1>4</long1>\n"
                               "    <long-long1>5</long-long1>\n"
                               "    <float1>6</float1>\n"
                               "    <double1>7</double1>\n"
                               "    <long-double1>8</long-double1>\n"
                               "    <pointer1>9</pointer1>\n"
                               "    <size_t1>10</size_t1>\n"
                               "    <wchar_t1>11</wchar_t1>\n"
                               "  </sizeof1>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(platform.platform(cppcheck::Platform::Win64));
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(platform.PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS(8, platform.char_bit);
        ASSERT_EQUALS('\0', platform.defaultSign);
        ASSERT_EQUALS(1, platform.sizeof_bool);
        ASSERT_EQUALS(2, platform.sizeof_short);
        ASSERT_EQUALS(4, platform.sizeof_int);
        ASSERT_EQUALS(4, platform.sizeof_long);
        ASSERT_EQUALS(8, platform.sizeof_long_long);
    }

    void valid_config_file_4() {
        // Valid platform configuration with all possible values specified and
        // set to 0.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit>0</char_bit>\n"
                               "  <default-sign>z</default-sign>\n"
                               "  <sizeof>\n"
                               "    <bool>0</bool>\n"
                               "    <short>0</short>\n"
                               "    <int>0</int>\n"
                               "    <long>0</long>\n"
                               "    <long-long>0</long-long>\n"
                               "    <float>0</float>\n"
                               "    <double>0</double>\n"
                               "    <long-double>0</long-double>\n"
                               "    <pointer>0</pointer>\n"
                               "    <size_t>0</size_t>\n"
                               "    <wchar_t>0</wchar_t>\n"
                               "  </sizeof>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(readPlatform(platform, xmldata));
        ASSERT_EQUALS(platform.PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
        ASSERT_EQUALS(0, platform.char_bit);
        ASSERT_EQUALS('z', platform.defaultSign);
        ASSERT_EQUALS(0, platform.sizeof_bool);
        ASSERT_EQUALS(0, platform.sizeof_short);
        ASSERT_EQUALS(0, platform.sizeof_int);
        ASSERT_EQUALS(0, platform.sizeof_long);
        ASSERT_EQUALS(0, platform.sizeof_long_long);
        ASSERT_EQUALS(0, platform.sizeof_float);
        ASSERT_EQUALS(0, platform.sizeof_double);
        ASSERT_EQUALS(0, platform.sizeof_long_double);
        ASSERT_EQUALS(0, platform.sizeof_pointer);
        ASSERT_EQUALS(0, platform.sizeof_size_t);
        ASSERT_EQUALS(0, platform.sizeof_wchar_t);
        ASSERT_EQUALS(0, platform.short_bit);
        ASSERT_EQUALS(0, platform.int_bit);
        ASSERT_EQUALS(0, platform.long_bit);
        ASSERT_EQUALS(0, platform.long_long_bit);
    }

    void invalid_config_file_1() const {
        // Invalid XML file: mismatching elements "boolt" vs "bool".
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit>8</char_bit>\n"
                               "  <default-sign>unsigned</default-sign>\n"
                               "  <sizeof>\n"
                               "    <boolt>1</bool>\n"
                               "    <short>2</short>\n"
                               "    <int>2</int>\n"
                               "    <long>4</long>\n"
                               "    <long-long>8</long-long>\n"
                               "    <float>4</float>\n"
                               "    <double>4</double>\n"
                               "    <long-double>4</long-double>\n"
                               "    <pointer>2</pointer>\n"
                               "    <size_t>2</size_t>\n"
                               "    <wchar_t>2</wchar_t>\n"
                               "  </sizeof>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(!readPlatform(platform, xmldata));
    }

    void empty_elements() {
        // Valid platform configuration without any usable information.
        // Similar like an empty file.
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<platform>\n"
                               "  <char_bit></char_bit>\n"
                               "  <default-sign></default-sign>\n"
                               "  <sizeof>\n"
                               "    <bool></bool>\n"
                               "    <short></short>\n"
                               "    <int></int>\n"
                               "    <long></long>\n"
                               "    <long-long></long-long>\n"
                               "    <float></float>\n"
                               "    <double></double>\n"
                               "    <long-double></long-double>\n"
                               "    <pointer></pointer>\n"
                               "    <size_t></size_t>\n"
                               "    <wchar_t></wchar_t>\n"
                               "  </sizeof>\n"
                               " </platform>";
        cppcheck::Platform platform;
        ASSERT(platform.platform(cppcheck::Platform::Win64));
        ASSERT(!readPlatform(platform, xmldata));
        ASSERT_EQUALS(platform.PlatformFile, platform.platformType);
        ASSERT(!platform.isWindowsPlatform());
    }
};

REGISTER_TEST(TestPlatform)
