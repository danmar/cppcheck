/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <iostream>
#include "testsuite.h"
#include "cmdlineparser.h"
#include "settings.h"

extern std::ostringstream errout;
extern std::ostringstream output;

class RedirectInputOutput
{
public:
    RedirectInputOutput()
    {
        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        _oldCout = std::cout.rdbuf(); // back up cout's streambuf
        _oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(_out.rdbuf()); // assign streambuf to cout
        std::cerr.rdbuf(_err.rdbuf()); // assign streambuf to cerr
    }

    ~RedirectInputOutput()
    {
        std::cout.rdbuf(_oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(_oldCerr); // restore cerrs's original streambuf

        errout << _err.str();
        output << _out.str();
    }

private:
    std::stringstream _out;
    std::stringstream _err;
    std::streambuf* _oldCout;
    std::streambuf *_oldCerr;
};

#define REDIRECT RedirectInputOutput redir;

class TestCmdlineParser : public TestFixture
{
public:
    TestCmdlineParser() : TestFixture("TestCmdlineParser")
    { }

private:

    void run()
    {
        TEST_CASE(nooptions);
        TEST_CASE(helpshort);
        TEST_CASE(helplong);
        TEST_CASE(showversion);
    }

    void nooptions()
    {
        REDIRECT;
        const char *argv[] = {"cppcheck"};
        Settings settings;
        CmdLineParser parser(&settings);
        ASSERT(parser.ParseFromArgs(1, argv));
        ASSERT_EQUALS(true, parser.GetShowHelp());
    }

    void helpshort()
    {
        REDIRECT;
        const char *argv[] = {"cppcheck", "-h"};
        Settings settings;
        CmdLineParser parser(&settings);
        ASSERT(parser.ParseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.GetShowHelp());
    }

    void helplong()
    {
        REDIRECT;
        const char *argv[] = {"cppcheck", "--help"};
        Settings settings;
        CmdLineParser parser(&settings);
        ASSERT(parser.ParseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.GetShowHelp());
    }

    void showversion()
    {
        REDIRECT;
        const char *argv[] = {"cppcheck", "--version"};
        Settings settings;
        CmdLineParser parser(&settings);
        ASSERT(parser.ParseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.GetShowVersion());
    }
};

REGISTER_TEST(TestCmdlineParser)
