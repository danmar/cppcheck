/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "testresultstree.h"

#include "resultstree.h"

// headers that declare mocked functions/variables
#include "applicationlist.h"
#include "common.h"
#include "threadhandler.h"
#include "projectfile.h"

#include "application.h"
#include "cppcheck.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "path.h"
#include "report.h"
#include "settings.h"
#include "showtypes.h"
#include "suppressions.h"
#include "xmlreport.h"

#include <cstddef>
#include <set>
#include <string>
#include <utility>

#include <QModelIndex>
#include <QString>
#include <QtTest>

class TestReport : public Report {
public:
    explicit TestReport(QString format) : Report(QString()), format(std::move(format)) {}
    void writeHeader() override {
        output.clear();
    }
    void writeFooter() override {}
    void writeError(const ErrorItem &error) override {
        QString line = format;
        line.replace("{id}", error.errorId);
        line.replace("{classification}", error.classification);
        line.replace("{guideline}", error.guideline);
        output += (output.isEmpty() ? "" : "\n") + line;
    }
    QString format;
    QString output;
};

// Mock GUI...
ProjectFile::ProjectFile(QObject *parent) : QObject(parent) {}
ProjectFile *ProjectFile::mActiveProject;
void ProjectFile::addSuppression(const SuppressionList::Suppression & /*unused*/) {}
QString ProjectFile::getWarningTags(std::size_t /*unused*/) const {
    return QString();
}
void ProjectFile::setWarningTags(std::size_t /*unused*/, const QString& /*unused*/) {}
bool ProjectFile::write(const QString & /*unused*/) {
    return true;
}
std::string severityToString(Severity severity) {
    return std::to_string((int)severity);
}
ApplicationList::ApplicationList(QObject *parent) : QObject(parent) {}
ApplicationList::~ApplicationList() = default;
int ApplicationList::getApplicationCount() const {
    return 0;
}
ThreadHandler::ThreadHandler(QObject *parent) : QObject(parent) {}
ThreadHandler::~ThreadHandler() = default;
bool ThreadHandler::isChecking() const {
    return false;
}
void ThreadHandler::stop() {
    throw 1;
}
void ThreadHandler::threadDone() {
    throw 1;
}
Application& ApplicationList::getApplication(const int /*unused*/) {
    throw 1;
}
const Application& ApplicationList::getApplication(const int index) const {
    return mApplications.at(index);
}
QString getPath(const QString &type) {
    return "/" + type;
}
void setPath(const QString & /*unused*/, const QString & /*unused*/) {}
QString XmlReport::quoteMessage(const QString &message) {
    return message;
}
QString XmlReport::unquoteMessage(const QString &message) {
    return message;
}
XmlReport::XmlReport(const QString& filename) : Report(filename) {}
void ThreadResult::fileChecked(const QString & /*unused*/) {
    throw 1;
}
void ThreadResult::reportOut(const std::string & /*unused*/, Color /*unused*/) {
    throw 1;
}
void ThreadResult::reportErr(const ErrorMessage & /*unused*/) {
    throw 1;
}

// Mock LIB...
bool Path::isHeader(std::string const& /*unused*/) {
    return false;
}
const std::set<std::string> ErrorLogger::mCriticalErrorIds;
std::string ErrorMessage::FileLocation::getfile(bool /*unused*/) const {
    return std::string();
}
const char* CppCheck::version() {
    return "1.0";
}
std::pair<std::string, std::string> Settings::getNameAndVersion(const std::string& /*unused*/) {
    throw 1;
}
Severity severityFromString(const std::string& severity) {
    return (Severity)std::stoi(severity);
}

// Test...

void TestResultsTree::test1() const
{
    // #12772 : GUI: information messages are shown even though information tool button is deselected
    ResultsTree tree(nullptr);
    tree.showResults(ShowTypes::ShowType::ShowInformation, false);
    ErrorItem errorItem;
    errorItem.errorPath << QErrorPathItem();
    errorItem.severity = Severity::information;
    tree.addErrorItem(errorItem);
    QCOMPARE(tree.isRowHidden(0,QModelIndex()), true);  // Added item is hidden
    tree.showResults(ShowTypes::ShowType::ShowInformation, true);
    QCOMPARE(tree.isRowHidden(0,QModelIndex()), false); // Show item
}

void TestResultsTree::testReportType() const
{
    TestReport report("{id},{classification},{guideline}");

    int msgCount = 0;
    auto createErrorItem = [&msgCount](const Severity severity, const QString& errorId) -> ErrorItem {
        ++msgCount;
        ErrorItem errorItem;
        errorItem.errorPath << QErrorPathItem(ErrorMessage::FileLocation("file1.c", msgCount, 1));
        errorItem.severity = severity;
        errorItem.errorId = errorId;
        errorItem.summary = "test summary " + QString::number(msgCount);
        return errorItem;
    };

    // normal report with 2 errors
    ResultsTree tree(nullptr);
    tree.updateSettings(false, false, false, false, false);
    tree.addErrorItem(createErrorItem(Severity::style, "id1"));
    tree.addErrorItem(createErrorItem(Severity::style, "unusedVariable")); // Misra C 2.8
    tree.saveResults(&report);
    QCOMPARE(report.output, "id1,,\nunusedVariable,,");

    // switch to Misra C report and check that "id1" is not shown
    tree.setReportType(ReportType::misraC);
    tree.saveResults(&report);
    QCOMPARE(report.output, "unusedVariable,Advisory,2.8");

    // add "missingReturn" and check that it is added properly
    tree.addErrorItem(createErrorItem(Severity::warning, "missingReturn")); // Misra C 17.4
    tree.saveResults(&report);
    QCOMPARE(report.output,
             "unusedVariable,Advisory,2.8\n"
             "missingReturn,Mandatory,17.4");
}


void TestResultsTree::testGetGuidelineError() const
{
    TestReport report("{id},{classification},{guideline}");

    int msgCount = 0;
    auto createErrorItem = [&msgCount](const Severity severity, const QString& errorId) -> ErrorItem {
        ++msgCount;
        ErrorItem errorItem;
        errorItem.errorPath << QErrorPathItem(ErrorMessage::FileLocation("file1.c", msgCount, 1));
        errorItem.severity = severity;
        errorItem.errorId = errorId;
        errorItem.summary = "test summary " + QString::number(msgCount);
        return errorItem;
    };

    // normal report with 2 errors
    ResultsTree tree(nullptr);
    tree.setReportType(ReportType::misraC);
    tree.addErrorItem(createErrorItem(Severity::error, "id1")); // error severity => guideline 1.3
    tree.saveResults(&report);
    QCOMPARE(report.output, "id1,Required,1.3");
}

QTEST_MAIN(TestResultsTree)

