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

#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "suppressions.h"
#include "testsuite.h"

#include <iosfwd>
#include <list>
#include <memory>
#include <string>

#include <tinyxml2.h>

class TestErrorLogger : public TestFixture {
public:
    TestErrorLogger() : TestFixture("TestErrorLogger"), fooCpp5("foo.cpp", 5, 1), barCpp8("bar.cpp", 8, 1) {}

private:
    const ErrorMessage::FileLocation fooCpp5;
    const ErrorMessage::FileLocation barCpp8;

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
        TEST_CASE(FromXmlV2);

        // Inconclusive results in xml reports..
        TEST_CASE(InconclusiveXml);

        // Serialize / Deserialize inconclusive message
        TEST_CASE(SerializeInconclusiveMessage);
        TEST_CASE(DeserializeInvalidInput);
        TEST_CASE(SerializeSanitize);
        TEST_CASE(SerializeFileLocation);

        TEST_CASE(suppressUnmatchedSuppressions);
    }

    void TestPatternSearchReplace(const std::string& idPlaceholder, const std::string& id) const {
        const std::string plainText = "text";

        ErrorMessage message;
        message.id = id;

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
        ErrorMessage::FileLocation loc;
        ASSERT_EQUALS("", loc.getfile());
        ASSERT_EQUALS(0, loc.line);
    }

    void FileLocationSetFile() const {
        ErrorMessage::FileLocation loc;
        loc.setfile("foo.cpp");
        ASSERT_EQUALS("foo.cpp", loc.getfile());
        ASSERT_EQUALS(0, loc.line);
    }

    void ErrorMessageConstruct() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(true));
    }

    void ErrorMessageConstructLocations() const {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Programming error.", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(true));
    }

    void ErrorMessageVerbose() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5]: (error) Verbose error", msg.toString(true));
    }

    void ErrorMessageVerboseLocations() const {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Programming error.", msg.toString(false));
        ASSERT_EQUALS("[foo.cpp:5] -> [bar.cpp:8]: (error) Verbose error", msg.toString(true));
    }

    void CustomFormat() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Programming error.", msg.toString(false, "{file}:{line},{severity},{id},{message}"));
        ASSERT_EQUALS("foo.cpp:5,error,errorId,Verbose error", msg.toString(true, "{file}:{line},{severity},{id},{message}"));
    }

    void CustomFormat2() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(1, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - foo.cpp(5):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})"));
        ASSERT_EQUALS("Verbose error - foo.cpp(5):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})"));
    }

    void CustomFormatLocations() const {
        // Check that first location from location stack is used in template
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        ASSERT_EQUALS(2, msg.callStack.size());
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS("Programming error. - bar.cpp(8):(error,errorId)", msg.toString(false, "{message} - {file}({line}):({severity},{id})"));
        ASSERT_EQUALS("Verbose error - bar.cpp(8):(error,errorId)", msg.toString(true, "{message} - {file}({line}):({severity},{id})"));
    }

    void ToXmlV2() const {
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"2\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorMessage::getXMLHeader());
        ASSERT_EQUALS("    </errors>\n</results>", ErrorMessage::getXMLFooter());
        std::string message("        <error id=\"errorId\" severity=\"error\"");
        message += " msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2Locations() const {
        std::list<ErrorMessage::FileLocation> locs = { fooCpp5, barCpp8 };
        locs.back().setinfo("ä");
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nVerbose error", "errorId", Certainty::normal);
        std::string header("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results version=\"2\">\n");
        header += "    <cppcheck version=\"";
        header += CppCheck::version();
        header += "\"/>\n    <errors>";
        ASSERT_EQUALS(header, ErrorMessage::getXMLHeader());
        ASSERT_EQUALS("    </errors>\n</results>", ErrorMessage::getXMLFooter());
        std::string message("        <error id=\"errorId\" severity=\"error\"");
        message += " msg=\"Programming error.\" verbose=\"Verbose error\">\n";
        message += "            <location file=\"bar.cpp\" line=\"8\" column=\"1\" info=\"\\303\\244\"/>\n";
        message += "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n        </error>";
        ASSERT_EQUALS(message, msg.toXML());
    }

    void ToXmlV2Encoding() const {
        {
            std::list<ErrorMessage::FileLocation> locs;
            ErrorMessage msg(locs, emptyString, Severity::error, "Programming error.\nComparing \"\203\" with \"\003\"", "errorId", Certainty::normal);
            const std::string expected("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Comparing &quot;\\203&quot; with &quot;\\003&quot;\"/>");
            ASSERT_EQUALS(expected, msg.toXML());
        }
        {
            const char code1[]="äöü";
            const char code2[]="\x12\x00\x00\x01";
            std::list<ErrorMessage::FileLocation> locs;
            ErrorMessage msg1(locs, emptyString, Severity::error, std::string("Programming error.\nReading \"")+code1+"\"", "errorId", Certainty::normal);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\303\\244\\303\\266\\303\\274&quot;\"/>", msg1.toXML());
            ErrorMessage msg2(locs, emptyString, Severity::error, std::string("Programming error.\nReading \"")+code2+"\"", "errorId", Certainty::normal);
            ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error.\" verbose=\"Reading &quot;\\022&quot;\"/>", msg2.toXML());
        }
    }

    void FromXmlV2() const {
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<error id=\"errorId\""
                               " severity=\"error\""
                               " cwe=\"123\""
                               " inconclusive=\"true\""
                               " msg=\"Programming error.\""
                               " verbose=\"Verbose error\""
                               " hash=\"456\""
                               ">\n"
                               "  <location file=\"bar.cpp\" line=\"8\" column=\"1\"/>\n"
                               "  <location file=\"foo.cpp\" line=\"5\" column=\"2\"/>\n"
                               "</error>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        ErrorMessage msg(doc.FirstChildElement());
        ASSERT_EQUALS("errorId", msg.id);
        ASSERT_EQUALS(Severity::error, msg.severity);
        ASSERT_EQUALS(123u, msg.cwe.id);
        ASSERT_EQUALS(Certainty::inconclusive, msg.certainty);
        ASSERT_EQUALS("Programming error.", msg.shortMessage());
        ASSERT_EQUALS("Verbose error", msg.verboseMessage());
        ASSERT_EQUALS(456u, msg.hash);
        ASSERT_EQUALS(2u, msg.callStack.size());
        ASSERT_EQUALS("foo.cpp", msg.callStack.front().getfile());
        ASSERT_EQUALS(5, msg.callStack.front().line);
        ASSERT_EQUALS(2u, msg.callStack.front().column);
        ASSERT_EQUALS("bar.cpp", msg.callStack.back().getfile());
        ASSERT_EQUALS(8, msg.callStack.back().line);
        ASSERT_EQUALS(1u, msg.callStack.back().column);
    }

    void InconclusiveXml() const {
        // Location
        std::list<ErrorMessage::FileLocation> locs(1, fooCpp5);

        // Inconclusive error message
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error", "errorId", Certainty::inconclusive);

        // xml version 2 error message
        ASSERT_EQUALS("        <error id=\"errorId\" severity=\"error\" msg=\"Programming error\" verbose=\"Programming error\" inconclusive=\"true\">\n"
                      "            <location file=\"foo.cpp\" line=\"5\" column=\"1\"/>\n"
                      "        </error>",
                      msg.toXML());
    }

    void SerializeInconclusiveMessage() const {
        // Inconclusive error message
        std::list<ErrorMessage::FileLocation> locs;
        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error", "errorId", Certainty::inconclusive);
        msg.file0 = "test.cpp";
        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "1 0"
                      "8 test.cpp"
                      "12 inconclusive"
                      "17 Programming error"
                      "17 Programming error"
                      "0 ", msg.serialize());

        ErrorMessage msg2;
        msg2.deserialize(msg.serialize());
        ASSERT_EQUALS("errorId", msg2.id);
        ASSERT_EQUALS(Severity::error, msg2.severity);
        ASSERT_EQUALS("test.cpp", msg2.file0);
        ASSERT_EQUALS(Certainty::inconclusive, msg2.certainty);
        ASSERT_EQUALS("Programming error", msg2.shortMessage());
        ASSERT_EQUALS("Programming error", msg2.verboseMessage());
    }

    void DeserializeInvalidInput() const {
        ErrorMessage msg;
        ASSERT_THROW(msg.deserialize("500foobar"), InternalError);
    }

    void SerializeSanitize() const {
        std::list<ErrorMessage::FileLocation> locs;
        ErrorMessage msg(locs, emptyString, Severity::error, std::string("Illegal character in \"foo\001bar\""), "errorId", Certainty::normal);
        msg.file0 = "1.c";

        ASSERT_EQUALS("7 errorId"
                      "5 error"
                      "1 0"
                      "1 0"
                      "3 1.c"
                      "33 Illegal character in \"foo\\001bar\""
                      "33 Illegal character in \"foo\\001bar\""
                      "0 ", msg.serialize());

        ErrorMessage msg2;
        msg2.deserialize(msg.serialize());
        ASSERT_EQUALS("errorId", msg2.id);
        ASSERT_EQUALS(Severity::error, msg2.severity);
        ASSERT_EQUALS("1.c", msg2.file0);
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.shortMessage());
        ASSERT_EQUALS("Illegal character in \"foo\\001bar\"", msg2.verboseMessage());
    }

    void SerializeFileLocation() const {
        ErrorMessage::FileLocation loc1(":/,;", 654, 33);
        loc1.setfile("[]:;,()");
        loc1.setinfo("abcd:/,");

        std::list<ErrorMessage::FileLocation> locs{loc1};

        ErrorMessage msg(locs, emptyString, Severity::error, "Programming error", "errorId", Certainty::inconclusive);

        ErrorMessage msg2;
        msg2.deserialize(msg.serialize());
        ASSERT_EQUALS("[]:;,()", msg2.callStack.front().getfile(false));
        ASSERT_EQUALS(":/,;", msg2.callStack.front().getOrigFile(false));
        ASSERT_EQUALS(654, msg2.callStack.front().line);
        ASSERT_EQUALS(33, msg2.callStack.front().column);
        ASSERT_EQUALS("abcd:/,", msg2.callStack.front().getinfo());
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
        suppressions.emplace_back("unmatchedSuppression", "*", Suppressions::Suppression::NO_LINE);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression (corresponds to "--suppress=unmatchedSuppression")
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "", Suppressions::Suppression::NO_LINE);
        reportUnmatchedSuppressions(suppressions);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression in a.c
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", Suppressions::Suppression::NO_LINE);
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
        suppressions.emplace_back("unmatchedSuppression", "b.c", Suppressions::Suppression::NO_LINE);
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
