/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "suppressions.h"
#include "testsuite.h"

#include <list>
#include <string>


class TestErrorLogger : public TestFixture {
public:
    TestErrorLogger() : TestFixture("TestErrorLogger"), fooCpp5("foo.cpp", 5), barCpp8("bar.cpp", 8) {
    }

private:
    const ErrorLogger::ErrorMessage::FileLocation fooCpp5;
    const ErrorLogger::ErrorMessage::FileLocation barCpp8;

    void run() override {
        TEST_CASE(PatternSearchReplace);
        TEST_CASE(FileLocationDefaults);
        TEST_CASE(FileLocationSetFile);
        TEST_CASE(ErrorMessageConstruct);
        TEST_CASE(ErrorMessageConstructLocations);
        TEST_CASE(ErrorMessageVerbose);
        TEST_CASE(ErrorMessageVerboseLocations);
        TEST_CASE(CustomFormat);
        TEST_CASE(CustomFormat2);
        TEST_CASE(CustomFormatLocations);
        TEST_CASE(ToXmlV2);
        TEST_CASE(ToXmlV2Locations);
        TEST_CASE(ToXmlV2Encoding);

        // Inconclusive results in xml reports..
        TEST_CASE(InconclusiveXml);

        // Serialize / Deserialize inconclusive message
        TEST_CASE(SerializeInconclusiveMessage);
        TEST_CASE(DeserializeInvalidInput);
        TEST_CASE(SerializeSanitize);

        TEST_CASE(suppressUnmatchedSuppressions);
    }

    void TestPatternSearchReplace(const std::string& idPlaceholder, const std::string& id) const {
        const std::string plainText = "text";

        ErrorLogger::ErrorMessage message;
        message._id = id;

        std::string serialized = message.toString(true, idPlaceholder + plainText + idPlaceholder);
        ASSERT_EQUALS(id + plainText + id, serialized);

        serialized = message.toString(true, idPlaceholder + idPlaceholder);
        ASSERT_EQUALS(id + id, serialized);

        serialized = message.toString(true, plainText + idPlaceholder + plainText);
        ASSERT_EQUALS(plainText + id + plainText, serialized);
    }

    void PatternSearchReplace() const {
        const std::string idPlaceholder = "{id}";

        const std::string empty;
        TestPatternSearchReplace(idPlaceholder, empty);

        const std::string shortIdValue = "ID";
        ASSERT_EQUALS(true, shortIdValue.length() < idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, shortIdValue);

        const std::string mediumIdValue = "_ID_";
        ASSERT_EQUALS(mediumIdValue.length(), idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, mediumIdValue);

        const std::string longIdValue = "longId";
        ASSERT_EQUALS(true, longIdValue.length() > idPlaceholder.length());
        TestPatternSearchReplace(idPlaceholder, longIdValue);
    }

    void FileLocationDefaults() const {
        ErrorLogger::ErrorMessage::FileLocation loc;
        ASSERT_EQUALS("", loc.getfile());
        ASSERT_EQUALS(0, loc.line);
    }

    void FileLocationSetFile() const {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.setfile("foo.cpp");
        ASSERT_EQUALS("foo.cpp", loc.getfile());
        ASSERT_EQUALS(0, loc.line);
    }

    void ErrorMessageConstruct() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.", "errorId", false);
        ASSERT_EQUALS(1, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(true));
    }

    void ErrorMessageConstructLocations() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.", "errorId", false);
        ASSERT_EQUALS(2, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(true));
    }

    void ErrorMessageVerbose() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        ASSERT_EQUALS(1, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Verbose error", msg.toString(true));
    }

    void ErrorMessageVerboseLocations() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        ASSERT_EQUALS(2, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Verbose error", msg.toString(true));
    }

    void CustomFormat() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        ASSERT_EQUALS(1, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Programming error.", msg.toString(false, "{file}:{line},{severity},{id},{message}"));
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Verbose error", msg.toString(true, "{file}:{line},{severity},{id},{message}"));
    }

    void CustomFormat2() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        ASSERT_EQUALS(1, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - foo.cpp(5):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})"));
        ASSERT_EQUALS("Verbose error - foo.cpp(5):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})"));
    }

    void CustomFormatLocations() const {
        // Check that first location from location stack is used in template
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        ASSERT_EQUALS(2, (int)msg._callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - bar.cpp(8):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})"));
        ASSERT_EQUALS("Verbose error - bar.cpp(8):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})"));
    }

    void ToXmlV2() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"2\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorLogger::ErrorMessage::getXMLHeader());
        ASSERT_EQUALS("    </errors>\n</results>", ErrorLogger::ErrorMessage::getXMLFooter());
        std::string message("        <error id=\"errorId\" severity=\"error\"");
        message += " msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"foo.cpp\" line=\"5\"/>\n        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2Locations() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", false);
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"2\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorLogger::ErrorMessage::getXMLHeader());
        ASSERT_EQUALS("    </errors>\n</results>", ErrorLogger::ErrorMessage::getXMLFooter());
        std::string message("        <error id=\"errorId\" severity=\"error\"");
        message += " msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"bar.cpp\" line=\"8\"/>\n";
        message += "            <location file=\"foo.cpp\" line=\"5\"/>\n        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2Encoding() const {
        {
            std::list<ErrorLogger::ErrorMessage::FileLocation> locs;
            ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nComparing \"\203\" with \"\003\"", "errorId", false);
            const std::string expected("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Comparing &quot;\\203&quot; with &quot;\\003&quot;\"/>");
            ASSERT_EQUALS(expected, msg.toXML());
        }
        {
            const char code1[]="äöü";
            const char code2[]="\x12\x00\x00\x01";
            std::list<ErrorLogger::ErrorMessage::FileLocation> locs;
            ErrorMessage msg1(locs, emptyString, Severity::error, std::string("Programming error.\nReading \"")+code1+"\"", "errorId", false);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\303\\244\\303\\266\\303\\274&quot;\"/>", msg1.toXML());
            ErrorMessage msg2(locs, emptyString, Severity::error, std::string("Programming error.\nReading \"")+code2+"\"", "errorId", false);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\022&quot;\"/>", msg2.toXML());
        }
    }

    void InconclusiveXml() const {
        // Location
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs(1, fooCpp5);

        // Inconclusive error message
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error", "errorId", true);

        // xml version 2 error message
        ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error\" verbose=\"Programming error\" inconclusive=\"true\">\n"
                      "            <location file=\"foo.cpp\" line=\"5\"/>\n"
                      "        </error>",
                      msg.toXML());
    }

    void SerializeInconclusiveMessage() const {
        // Inconclusive error message
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs;
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error", "errorId", true);
        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "12 inconclusive"
                      "17 Programming error"
                      "17 Programming error"
                      "0 ", msg.serialize());

        ErrorMessage msg2;
        msg2.deserialize(msg.serialize());
        ASSERT_EQUALS("errorId", msg2._id);
        ASSERT_EQUALS(Severity::error, msg2._severity);
        ASSERT_EQUALS(true, msg2._inconclusive);
        ASSERT_EQUALS("Programming error", msg2.shortMessage());
        ASSERT_EQUALS("Programming error", msg2.verboseMessage());
    }

    void DeserializeInvalidInput() const {
        ErrorMessage msg;
        ASSERT_THROW(msg.deserialize("500foobar"), InternalError);
    }

    void SerializeSanitize() const {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locs;
        ErrorMessage msg(locs, emptyString, Severity::error, std::string("Illegal character in \"foo\001bar\""), "errorId", false);

        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "33 Illegal character in \"foo\\001bar\""
                      "33 Illegal character in \"foo\\001bar\""
                      "0 ", msg.serialize());

        ErrorMessage msg2;
        msg2.deserialize(msg.serialize());
        ASSERT_EQUALS("errorId", msg2._id);
        ASSERT_EQUALS(Severity::error, msg2._severity);
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.shortMessage());
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.verboseMessage());
    }

    void suppressUnmatchedSuppressions() {
        std::list<Suppressions::Suppression> suppressions;

        // No unmatched suppression
        errout.str("");
        suppressions.clear();
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "*", 0U);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression in a.c
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 0U);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // suppress unmatchedSuppression in a.c at line 10
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 10U);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // don't suppress unmatchedSuppression when file is mismatching
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "b.c", 0U);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout.str());

        // don't suppress unmatchedSuppression when line is mismatching
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 1U);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout.str());
    }
};

REGISTER_TEST(TestErrorLogger)
