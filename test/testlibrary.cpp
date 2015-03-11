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
        TEST_CASE(function_match_scope);
        TEST_CASE(function_match_args);
        TEST_CASE(function_match_var);
        TEST_CASE(function_arg);
        TEST_CASE(function_arg_any);
        TEST_CASE(function_arg_valid);
        TEST_CASE(function_arg_minsize);
        TEST_CASE(memory);
        TEST_CASE(memory2); // define extra "free" allocation functions
        TEST_CASE(resource);
        TEST_CASE(podtype);
        TEST_CASE(container);
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

        TokenList tokenList(nullptr);
        std::istringstream istr("foo();");
        tokenList.createTokens(istr);
        tokenList.front()->next()->astOperand1(tokenList.front());

        Library library;
        library.load(doc);
        ASSERT(library.use.empty());
        ASSERT(library.leakignore.empty());
        ASSERT(library.argumentChecks.empty());
        ASSERT(library.isnotnoreturn(tokenList.front()));
    }

    void function_match_scope() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"/>"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        {
            TokenList tokenList(nullptr);
            std::istringstream istr("fred.foo(123);"); // <- wrong scope, not library function
            tokenList.createTokens(istr);

            Library library;
            library.load(doc);
            ASSERT(library.isNotLibraryFunction(tokenList.front()->tokAt(2)));
        }

        {
            TokenList tokenList(nullptr);
            std::istringstream istr("Fred::foo(123);"); // <- wrong scope, not library function
            tokenList.createTokens(istr);

            Library library;
            library.load(doc);
            ASSERT(library.isNotLibraryFunction(tokenList.front()->tokAt(2)));
        }
    }

    void function_match_args() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"/>"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        TokenList tokenList(nullptr);
        std::istringstream istr("foo();"); // <- too few arguments, not library function
        tokenList.createTokens(istr);
        tokenList.front()->next()->astOperand1(tokenList.front());

        Library library;
        library.load(doc);
        ASSERT(library.isNotLibraryFunction(tokenList.front()));
    }

    void function_match_var() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"/>"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        TokenList tokenList(nullptr);
        std::istringstream istr("Fred foo(123);"); // <- Variable declaration, not library function
        tokenList.createTokens(istr);
        tokenList.front()->next()->astOperand1(tokenList.front());
        tokenList.front()->next()->varId(1);

        Library library;
        library.load(doc);
        ASSERT(library.isNotLibraryFunction(tokenList.front()->next()));
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

        TokenList tokenList(nullptr);
        std::istringstream istr("foo(a,b,c,d,e);");
        tokenList.createTokens(istr);
        tokenList.front()->next()->astOperand1(tokenList.front());

        // 1-
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 1, -10));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 1, 0));
        ASSERT_EQUALS(true, library.isargvalid(tokenList.front(), 1, 1));
        ASSERT_EQUALS(true, library.isargvalid(tokenList.front(), 1, 10));

        // -7-0
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 2, -10));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 2, -7));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 2, -3));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 2, 0));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 2, 1));

        // 1-5,8
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 3, 0));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 3, 1));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 3, 3));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 3, 5));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 3, 6));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 3, 7));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 3, 8));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 3, 9));

        // -1,5
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 4, -10));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 4, -1));

        // :1,5
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 5, -10));
        ASSERT_EQUALS(true,  library.isargvalid(tokenList.front(), 5, 1));
        ASSERT_EQUALS(false, library.isargvalid(tokenList.front(), 5, 2));
    }

    void function_arg_minsize() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <function name=\"foo\">\n"
                               "    <arg nr=\"1\"><minsize type=\"strlen\" arg=\"2\"/></arg>\n"
                               "    <arg nr=\"2\"><minsize type=\"argvalue\" arg=\"3\"/></arg>\n"
                               "    <arg nr=\"3\"/>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        TokenList tokenList(nullptr);
        std::istringstream istr("foo(a,b,c);");
        tokenList.createTokens(istr);
        tokenList.front()->next()->astOperand1(tokenList.front());

        // arg1: type=strlen arg2
        const std::list<Library::ArgumentChecks::MinSize> *minsizes = library.argminsizes(tokenList.front(),1);
        ASSERT_EQUALS(true, minsizes != nullptr);
        ASSERT_EQUALS(1U, minsizes ? minsizes->size() : 1U);
        if (minsizes && minsizes->size() == 1U) {
            const Library::ArgumentChecks::MinSize &m = minsizes->front();
            ASSERT_EQUALS(Library::ArgumentChecks::MinSize::STRLEN, m.type);
            ASSERT_EQUALS(2, m.arg);
        }

        // arg2: type=argvalue arg3
        minsizes = library.argminsizes(tokenList.front(), 2);
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
                               "  <podtype name=\"s16\" size=\"2\"/>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        const struct Library::PodType *type = library.podtype("s16");
        ASSERT_EQUALS(2U,   type ? type->size : 0U);
        ASSERT_EQUALS(0,    type ? type->sign : '?');
    }

    void container() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def>\n"
                               "  <container id=\"A\" startPattern=\"std :: A &lt;\" endPattern=\"&gt; !!::\">\n"
                               "    <type templateParameter=\"1\"/>\n"
                               "    <size templateParameter=\"4\">\n"
                               "      <function name=\"resize\" action=\"resize\"/>\n"
                               "      <function name=\"clear\" action=\"clear\"/>\n"
                               "      <function name=\"size\" yields=\"size\"/>\n"
                               "      <function name=\"empty\" yields=\"empty\"/>\n"
                               "      <function name=\"push_back\" action=\"push\"/>\n"
                               "      <function name=\"pop_back\" action=\"pop\"/>\n"
                               "    </size>\n"
                               "    <access>\n"
                               "      <function name=\"at\" yields=\"at_index\"/>\n"
                               "      <function name=\"begin\" yields=\"start-iterator\"/>\n"
                               "      <function name=\"end\" yields=\"end-iterator\"/>\n"
                               "      <function name=\"data\" yields=\"buffer\"/>\n"
                               "      <function name=\"c_str\" yields=\"buffer-nt\"/>\n"
                               "      <function name=\"front\" yields=\"item\"/>\n"
                               "      <function name=\"find\" action=\"find\"/>\n"
                               "    </access>\n"
                               "  </container>\n"
                               "  <container id=\"B\" startPattern=\"std :: B &lt;\" inherits=\"A\">\n"
                               "    <size templateParameter=\"3\"/>\n" // Inherits all but templateParameter
                               "  </container>\n"
                               "  <container id=\"C\">\n"
                               "    <type string=\"std-like\"/>\n"
                               "    <access indexOperator=\"array-like\"/>\n"
                               "  </container>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));

        Library library;
        library.load(doc);

        Library::Container& A = library.containers["A"];
        Library::Container& B = library.containers["B"];
        Library::Container& C = library.containers["C"];

        ASSERT_EQUALS(A.type_templateArgNo, 1);
        ASSERT_EQUALS(A.size_templateArgNo, 4);
        ASSERT_EQUALS(A.startPattern, "std :: A <");
        ASSERT_EQUALS(A.endPattern, "> !!::");
        ASSERT_EQUALS(A.stdStringLike, false);
        ASSERT_EQUALS(A.arrayLike_indexOp, false);
        ASSERT_EQUALS(Library::Container::SIZE, A.getYield("size"));
        ASSERT_EQUALS(Library::Container::EMPTY, A.getYield("empty"));
        ASSERT_EQUALS(Library::Container::AT_INDEX, A.getYield("at"));
        ASSERT_EQUALS(Library::Container::START_ITERATOR, A.getYield("begin"));
        ASSERT_EQUALS(Library::Container::END_ITERATOR, A.getYield("end"));
        ASSERT_EQUALS(Library::Container::BUFFER, A.getYield("data"));
        ASSERT_EQUALS(Library::Container::BUFFER_NT, A.getYield("c_str"));
        ASSERT_EQUALS(Library::Container::ITEM, A.getYield("front"));
        ASSERT_EQUALS(Library::Container::NO_YIELD, A.getYield("foo"));
        ASSERT_EQUALS(Library::Container::RESIZE, A.getAction("resize"));
        ASSERT_EQUALS(Library::Container::CLEAR, A.getAction("clear"));
        ASSERT_EQUALS(Library::Container::PUSH, A.getAction("push_back"));
        ASSERT_EQUALS(Library::Container::POP, A.getAction("pop_back"));
        ASSERT_EQUALS(Library::Container::FIND, A.getAction("find"));
        ASSERT_EQUALS(Library::Container::NO_ACTION, A.getAction("foo"));

        ASSERT_EQUALS(B.type_templateArgNo, 1);
        ASSERT_EQUALS(B.size_templateArgNo, 3);
        ASSERT_EQUALS(B.startPattern, "std :: B <");
        ASSERT_EQUALS(B.endPattern, "> !!::");
        ASSERT_EQUALS(B.functions.size(), A.functions.size());

        ASSERT(C.functions.empty());
        ASSERT_EQUALS(C.type_templateArgNo, -1);
        ASSERT_EQUALS(C.size_templateArgNo, -1);
        ASSERT_EQUALS(C.stdStringLike, true);
        ASSERT_EQUALS(C.arrayLike_indexOp, true);
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
