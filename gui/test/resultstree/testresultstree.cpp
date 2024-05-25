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
#include "xmlreportv2.h"

#include "cppcheck.h"
#include "errorlogger.h"
#include "path.h"
#include "settings.h"

#include <QtTest>

// Mock GUI...
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
int ApplicationList::getApplicationCount() const {
    return 0;
}
bool ThreadHandler::isChecking() const {
    return false;
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

QTEST_MAIN(TestResultsTree)

