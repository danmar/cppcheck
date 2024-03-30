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

#ifdef HAVE_RULES

#include "fixture.h"
#include "regex.h"

#include <list>
#include <string>
#include <utility>

class TestRegEx : public TestFixture {
public:
    TestRegEx() : TestFixture("TestRegEx") {}

private:
    void run() override {
        TEST_CASE(match);
        TEST_CASE(nomatch);
        TEST_CASE(compileError);
        TEST_CASE(recompile);
        TEST_CASE(notcompiled);
        TEST_CASE(copy);
        TEST_CASE(multimatch);
        TEST_CASE(partialmatch);
        TEST_CASE(exactmatch);
    }

    void match() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("begin.*end"));
        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };
        ASSERT_EQUALS("", r.match("begin-123-end", std::move(f)));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(13, e);
    }

    void nomatch() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("begin.*end"));
        int called = 0;
        auto f = [&](int /*start*/, int /*end*/) {
            ++called;
        };
        ASSERT_EQUALS("", r.match("end-123-begin", std::move(f)));
        ASSERT_EQUALS(0, called);
    }

    void compileError() const {
        Regex r;
        ASSERT_EQUALS("pcre_compile failed: missing terminating ] for character class", r.compile("["));
    }

    void recompile() const {
        Regex r;
        ASSERT_EQUALS("", r.compile(".*"));
        ASSERT_EQUALS("regular expression has already been compiled", r.compile(".*"));
    }

    void notcompiled()const {
        Regex r;
        ASSERT_EQUALS("regular expression hat not been compiled yet", r.match("string", {}));
    }

    void copy() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("begin.*end"));

        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };

        {
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            Regex r2 = r;
            ASSERT_EQUALS("", r2.match("begin-123-end", f));
            ASSERT_EQUALS(1, called);
            ASSERT_EQUALS(0, s);
            ASSERT_EQUALS(13, e);
        }

        called = 0;
        s = -1;
        e = -1;
        ASSERT_EQUALS("", r.match("begin-123-end", f));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(13, e);
    }

    void multimatch() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("info:.*"));

        std::string input =
            "info: start\n"
            "info: init\n"
            "warn: missing\n"
            "warn: invalid\n"
            "info: done\n"
            "error: notclean\n";

        std::list<std::string> matches;
        auto f = [&](int start, int end) {
            matches.push_back(input.substr(start, end - start));
        };
        ASSERT_EQUALS("", r.match(input, std::move(f)));
        ASSERT_EQUALS(3, matches.size());
        auto it = matches.cbegin();
        ASSERT_EQUALS("info: start", *it);
        ASSERT_EQUALS("info: init", *(++it));
        ASSERT_EQUALS("info: done", *(++it));
    }

    void partialmatch() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("123"));
        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };
        ASSERT_EQUALS("", r.match("begin-123-end", std::move(f)));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(6, s);
        ASSERT_EQUALS(9, e);
    }

    void exactmatch() const {
        Regex r;
        ASSERT_EQUALS("", r.compile("^123$"));

        int called = 0;
        int s = -1;
        int e = -1;
        auto f = [&](int start, int end) {
            ++called;
            s = start;
            e = end;
        };

        ASSERT_EQUALS("", r.match("begin-123-end", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r.match("123\n123", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r.match("123123", f));
        ASSERT_EQUALS(0, called);
        ASSERT_EQUALS(-1, s);
        ASSERT_EQUALS(-1, e);

        ASSERT_EQUALS("", r.match("123", f));
        ASSERT_EQUALS(1, called);
        ASSERT_EQUALS(0, s);
        ASSERT_EQUALS(3, e);
    }

    // TODO: how to provoke a match() error?
};

REGISTER_TEST(TestRegEx)

#endif // HAVE_RULES
