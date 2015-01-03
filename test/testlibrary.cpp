/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "library.h"
#include "token.h"
#include "tokenlist.h"
#include "testsuite.h"
#include <tinyxml2.h>

class TestLibrary : public TestFixture {
public:
    TestLibrary() : TestFixture("TestLibrary") { }

private:

    void run() {
        TEST_CASE(empty);
        TEST_CASE(function);
        TEST_CASE(function_arg);
        TEST_CASE(function_arg_any);
        TEST_CASE(function_arg_valid);
        TEST_CASE(function_arg_minsize);
        TEST_CASE(memory);
        TEST_CASE(memory2); // define extra "free" allocation functions
        TEST_CASE(resource);
        TEST_CASE(podtype);
        TEST_CASE(version);
    }

    void empty() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n<def/>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT(library.use.empty());
        ASSERT(library.leakignore.empty());
        ASSERT(library.argumentChecks.empty());
    }

    void function() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <noreturn>false</noreturn>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT(library.use.empty());
        ASSERT(library.leakignore.empty());
        ASSERT(library.argumentChecks.empty());
        ASSERT(library.isnotnoreturn("foo"));
    }

    void function_arg() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"><not-uninit/></arg>\n"
                               "    <arg nr=\"2\"><not-null/></arg>\n"
                               "    <arg nr=\"3\"><formatstr/></arg>\n"
                               "    <arg nr=\"4\"><strz/></arg>\n"
                               "    <arg nr=\"5\"><not-bool/></arg>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][1].notuninit);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][2].notnull);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][3].formatstr);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][4].strz);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][5].notbool);
    }

    void function_arg_any() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "<function name=\"foo\">\n"
                               "   <arg nr=\"any\"><not-uninit/></arg>\n"
                               "</function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT_EQUALS(true, library.argumentChecks["foo"][-1].notuninit);
    }

    void function_arg_valid() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"><valid>1:</valid></arg>\n"
                               "    <arg nr=\"2\"><valid>-7:0</valid></arg>\n"
                               "    <arg nr=\"3\"><valid>1:5,8</valid></arg>\n"
                               "    <arg nr=\"4\"><valid>-1,5</valid></arg>\n"
                               "    <arg nr=\"5\"><valid>:1,5</valid></arg>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        // 1-
        ASSERT_EQUALS(false, library.isargvalid("foo", 1, -10));
        ASSERT_EQUALS(false, library.isargvalid("foo", 1, 0));
        ASSERT_EQUALS(true, library.isargvalid("foo", 1, 1));
        ASSERT_EQUALS(true, library.isargvalid("foo", 1, 10));

        // -7-0
        ASSERT_EQUALS(false, library.isargvalid("foo", 2, -10));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 2, -7));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 2, -3));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 2, 0));
        ASSERT_EQUALS(false, library.isargvalid("foo", 2, 1));

        // 1-5,8
        ASSERT_EQUALS(false, library.isargvalid("foo", 3, 0));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 3, 1));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 3, 3));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 3, 5));
        ASSERT_EQUALS(false, library.isargvalid("foo", 3, 6));
        ASSERT_EQUALS(false, library.isargvalid("foo", 3, 7));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 3, 8));
        ASSERT_EQUALS(false, library.isargvalid("foo", 3, 9));

        // -1,5
        ASSERT_EQUALS(false, library.isargvalid("foo", 4, -10));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 4, -1));

        // :1,5
        ASSERT_EQUALS(true,  library.isargvalid("foo", 5, -10));
        ASSERT_EQUALS(true,  library.isargvalid("foo", 5, 1));
        ASSERT_EQUALS(false, library.isargvalid("foo", 5, 2));
    }

    void function_arg_minsize() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"><minsize type=\"strlen\" arg=\"2\"/></arg>\n"
                               "    <arg nr=\"2\"><minsize type=\"argvalue\" arg=\"3\"/></arg>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        // arg1: type=strlen arg2
        const std::list<Library::ArgumentChecks::MinSize> *minsizes = library.argminsizes("foo",1);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT_EQUALS(Library::ArgumentChecks::MinSize::STRLEN, m.type);
            ASSERT_EQUALS(2, m.arg);
        }

        // arg2: type=argvalue arg3
        minsizes = library.argminsizes("foo", 2);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT_EQUALS(Library::ArgumentChecks::MinSize::ARGVALUE, m.type);
            ASSERT_EQUALS(3, m.arg);
        }
    }

    void memory() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <memory>\n"
                               "    <alloc>CreateX</alloc>\n"
                               "    <dealloc>DeleteX</dealloc>\n"
                               "  </memory>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT(library.use.empty());
        ASSERT(library.leakignore.empty());
        ASSERT(library.argumentChecks.empty());

        ASSERT(Library::ismemory(library.alloc("CreateX")));
        ASSERT_EQUALS(library.alloc("CreateX"), library.dealloc("DeleteX"));
    }
    void memory2() const {
        const char xmldata1[] = "<?xml version=\"1.0\"?>\n"
                                "<def>\n"
                                "  <memory>\n"
                                "    <alloc>malloc</alloc>\n"
                                "    <dealloc>free</dealloc>\n"
                                "  </memory>\n"
                                "</def>";
        const char xmldata2[] = "<?xml version=\"1.0\"?>\n"
                                "<def>\n"
                                "  <memory>\n"
                                "    <alloc>foo</alloc>\n"
                                "    <dealloc>free</dealloc>\n"
                                "  </memory>\n"
                                "</def>";

        Library library;
        library.loadxmldata(xmldata1, sizeof(xmldata1));
        library.loadxmldata(xmldata2, sizeof(xmldata2));

        ASSERT_EQUALS(library.dealloc("free"), library.alloc("malloc"));
        ASSERT_EQUALS(library.dealloc("free"), library.alloc("foo"));
    }

    void resource() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <resource>\n"
                               "    <alloc>CreateX</alloc>\n"
                               "    <dealloc>DeleteX</dealloc>\n"
                               "  </resource>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);
        ASSERT(library.use.empty());
        ASSERT(library.leakignore.empty());
        ASSERT(library.argumentChecks.empty());

        ASSERT(Library::isresource(library.alloc("CreateX")));
        ASSERT_EQUALS(library.alloc("CreateX"), library.dealloc("DeleteX"));
    }

    void podtype() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <podtype name=\"s16\" sizeof=\"2\"/>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        const struct Library::PodType *type = library.podtype("s16");
        ASSERT_EQUALS(2U,   type ? type->size : 0U);
        ASSERT_EQUALS(0,    type ? type->sign : '?');
    }

    void version() const {
        {
            const char xmldata [] = "<?xml version=\"1.0\"?>\n"
                                    "<def>\n"
                                    "</def>";
            tinyxml2::XMLDocument doc;
            doc.Parse(xmldata, sizeof(xmldata));

            Library library;
            Library::Error err = library.load(doc);
            ASSERT_EQUALS(err.errorcode, Library::OK);
        }
        {
            const char xmldata [] = "<?xml version=\"1.0\"?>\n"
                                    "<def format=\"1\">\n"
                                    "</def>";
            tinyxml2::XMLDocument doc;
            doc.Parse(xmldata, sizeof(xmldata));

            Library library;
            Library::Error err = library.load(doc);
            ASSERT_EQUALS(err.errorcode, Library::OK);
        }
        {
            const char xmldata [] = "<?xml version=\"1.0\"?>\n"
                                    "<def format=\"42\">\n"
                                    "</def>";
            tinyxml2::XMLDocument doc;
            doc.Parse(xmldata, sizeof(xmldata));

            Library library;
            Library::Error err = library.load(doc);
            ASSERT_EQUALS(err.errorcode, Library::UNSUPPORTED_FORMAT);
        }
    }
};

REGISTER_TEST(TestLibrary)
