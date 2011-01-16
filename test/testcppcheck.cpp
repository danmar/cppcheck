/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "testsuite.h"
#include "path.h"

#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>

// use tinyxml with STL
#include "tinyxml/tinyxml.h"

extern std::ostringstream errout;
extern std::ostringstream output;

class TestCppcheck : public TestFixture
{
public:
    TestCppcheck() : TestFixture("TestCppcheck")
    { }

private:

    void check(const std::string &data)
    {
        errout.str("");
        output.str("");
        CppCheck cppCheck(*this);
        cppCheck.addFile("file.cpp", data);
        cppCheck.check();
    }

    void run()
    {
        TEST_CASE(nonexistingpath);

        TEST_CASE(include);
        //TEST_CASE(getErrorMessages);
        //TEST_CASE(parseOutputtingArgs);
        //TEST_CASE(parseArgsAndCheck);
    }

#if 0
    bool argCheck(int argc, const char *argv[])
    {
        errout.str("");
        output.str("");
        CppCheck cppCheck(*this);
        return cppCheck.parseFromArgs(argc, argv);
    }

    bool argCheckWithCoutCerrRedirect(int argc, const char * argv[])
    {
        // redirect cout and cerr
        std::stringstream out, err;
        std::streambuf* oldCout, *oldCerr;

        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        oldCout = std::cout.rdbuf(); // back up cout's streambuf
        oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(out.rdbuf()); // assign streambuf to cout
        std::cerr.rdbuf(err.rdbuf()); // assign streambuf to cerr

        bool result = argCheck(argc, argv);

        std::cout.rdbuf(oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(oldCerr); // restore cerrs's original streambuf

        errout << err.str();
        output << out.str();

        return result;
    }

    bool argCheckWithCheck(int argc, const char *argv[], const std::string &data)
    {
        errout.str("");
        output.str("");
        CppCheck cppCheck(*this);
        cppCheck.addFile("file.cpp", data);
        bool result = cppCheck.parseFromArgs(argc, argv);
        if (result)
            cppCheck.check();

        return result;
    }

    bool argCheckReturnSettings(int argc, const char *argv[], Settings &settings)
    {
        errout.str("");
        output.str("");
        CppCheck cppCheck(*this);
        cppCheck.addFile("file.cpp", "void foo(){}");
        bool result = cppCheck.parseFromArgs(argc, argv);
        if (result)
            settings = cppCheck.settings();

        return result;
    }

    void parseArgsAndCheck()
    {
        {
            const char *argv[] = {"cppcheck", "--showtime=top5"};
            const char *data = "void foo(){}";
            ASSERT_EQUALS(true, argCheckWithCheck(2, argv, data));
            ASSERT_EQUALS("", errout.str());
//            ASSERT_EQUALS(true, output.str().find("Overall time:") != std::string::npos);
        }
    }
#endif

    void parseErrorList(const char* xmlData)
    {
        TiXmlDocument doc;
        doc.Parse(xmlData);
        // parsing must be successful
        ASSERT_EQUALS(false, doc.Error());
        // root element must be "results"
        TiXmlElement* root = doc.FirstChildElement();
        ASSERT_EQUALS("results", root->Value());

        TiXmlElement* error = root->FirstChildElement();
        std::list<std::string> idList;

        while (error)
        {
            // only childs of type "error"
            ASSERT_EQUALS("error", error->Value());
            // attributes id, msg, severity
            ASSERT_EQUALS(error->Attribute("msg") == NULL, false);
            ASSERT_EQUALS(error->Attribute("severity") == NULL, false);
            const char* id = error->Attribute("id");
            ASSERT_EQUALS(id == NULL, false);
            // no duplicate ids
            std::stringstream msg;
            msg << "Duplicate id " << id;
            ASSERT_EQUALS_MSG(idList.end() == std::find(idList.begin(), idList.end(), id), true, msg.str());
            idList.push_back(id);
            error = error->NextSiblingElement();
        }
    }

#if 0
    void parseOutputtingArgs()
    {
        {
            const char *argv[] = { "cppcheck", "--errorlist" };
            ASSERT_EQUALS(true, argCheckWithCoutCerrRedirect(2, argv));
            ASSERT_EQUALS("", errout.str());
            parseErrorList(output.str().c_str());
        }

        {
            const char *argv[] = {"cppcheck", "--help"};
            ASSERT_EQUALS(true, argCheck(2, argv));
            ASSERT_EQUALS("", errout.str());
            ASSERT_EQUALS(true, output.str().find("Example usage") != std::string::npos);
        }

        {
            const char *argv[] = {"cppcheck", "-h"};
            ASSERT_EQUALS(true, argCheck(2, argv));
            ASSERT_EQUALS("", errout.str());
            ASSERT_EQUALS(true, output.str().find("Example usage") != std::string::npos);
        }

        {
            const char *argv[] = {"cppcheck"};
            ASSERT_EQUALS(true, argCheck(1, argv));
            ASSERT_EQUALS("", errout.str());
            ASSERT_EQUALS(true, output.str().find("Example usage") != std::string::npos);
        }

        {
            const char *argv[] = {"cppcheck", "--version"};
            ASSERT_EQUALS(true, argCheck(2, argv));
            ASSERT_EQUALS("", errout.str());
            ASSERT_EQUALS(std::string("Cppcheck ") + CppCheck::version() + "\n", output.str());
        }

        {
            const char *argv[] = {"cppcheck", "--doc"};
            ASSERT_EQUALS(true, argCheck(2, argv));
            ASSERT_EQUALS("", errout.str());
            ASSERT_EQUALS(true, output.str().find("===Bounds checking===") != std::string::npos);
            ASSERT_EQUALS(true, output.str().find("===Unused functions===") != std::string::npos);
        }
    }
#endif

    void nonexistingpath()
    {
        CppCheckExecutor exec;
        const char *argv[] = { "", "idontexist" };
        int retval = exec.check(2, argv);

        ASSERT_EQUALS(retval, EXIT_FAILURE);
    }

    void include()
    {
        ErrorLogger::ErrorMessage errorMessage;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.setfile("ab/cd/../ef.h");
        errorMessage._callStack.push_back(loc);
        const std::string fname(Path::toNativeSeparators("ab/ef.h"));
        ASSERT_EQUALS("<error file=\"" + fname + "\" line=\"0\" id=\"\" severity=\"style\" msg=\"\"/>", errorMessage.toXML(false,1));
        ASSERT_EQUALS("[" + fname + ":0]: ", errorMessage.toString(false));
    }

    void getErrorMessages()
    {
        errout.str("");
        CppCheck cppCheck(*this);
        cppCheck.getErrorMessages();
    }

};

REGISTER_TEST(TestCppcheck)
