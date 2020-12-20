/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "settings.h"

#include "valueflow.h"

#include <algorithm>
#include <fstream>

std::atomic<bool> Settings::mTerminated;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

Settings::Settings()
    : mEnabled(0),
      bugHunting(false),
      bugHuntingCheckFunctionMaxTime(INT_MAX),
      checkAllConfigurations(true),
      checkConfiguration(false),
      checkHeaders(true),
      checkLibrary(false),
      checkUnusedTemplates(true),
      clang(false),
      clangExecutable("clang"),
      clangTidy(false),
      daca(false),
      debugBugHunting(false),
      debugnormal(false),
      debugSimplified(false),
      debugtemplate(false),
      debugwarnings(false),
      dump(false),
      enforcedLang(None),
      exceptionHandling(false),
      exitCode(0),
      experimental(false),
      force(false),
      inconclusive(false),
      inlineSuppressions(false),
      jobs(1),
      jointSuppressionReport(false),
      loadAverage(0),
      maxConfigs(12),
      maxCtuDepth(2),
      maxTemplateRecursion(100),
      preprocessOnly(false),
      quiet(false),
      relativePaths(false),
      reportProgress(false),
      showtime(SHOWTIME_MODES::SHOWTIME_NONE),
      verbose(false),
      xml(false),
      xml_version(2)
{
}

std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("cppcheck: --enable parameter is empty");
            const std::string errmsg(addEnabled(str.substr(prevPos, pos - prevPos)));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("cppcheck: --enable parameter is empty");
        return addEnabled(str.substr(prevPos));
    }

    if (str == "all") {
        mEnabled |= WARNING | STYLE | PERFORMANCE | PORTABILITY | INFORMATION | UNUSED_FUNCTION | MISSING_INCLUDE;
    } else if (str == "warning") {
        mEnabled |= WARNING;
    } else if (str == "style") {
        mEnabled |= STYLE;
    } else if (str == "performance") {
        mEnabled |= PERFORMANCE;
    } else if (str == "portability") {
        mEnabled |= PORTABILITY;
    } else if (str == "information") {
        mEnabled |= INFORMATION | MISSING_INCLUDE;
    } else if (str == "unusedFunction") {
        mEnabled |= UNUSED_FUNCTION;
    } else if (str == "missingInclude") {
        mEnabled |= MISSING_INCLUDE;
    }
#ifdef CHECK_INTERNAL
    else if (str == "internal") {
        mEnabled |= INTERNAL;
    }
#endif
    else {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string();
}

bool Settings::isEnabled(Severity::SeverityType severity) const
{
    switch (severity) {
    case Severity::none:
        return true;
    case Severity::error:
        return true;
    case Severity::warning:
        return isEnabled(WARNING);
    case Severity::style:
        return isEnabled(STYLE);
    case Severity::performance:
        return isEnabled(PERFORMANCE);
    case Severity::portability:
        return isEnabled(PORTABILITY);
    case Severity::information:
        return isEnabled(INFORMATION);
    case Severity::debug:
        return false;
    default:
        return false;
    }
}

bool Settings::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!isEnabled(Settings::WARNING) && (value->condition || value->defaultArg))
        return false;
    if (!inconclusive && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}

static std::vector<std::string> getSummaryFiles(const std::string &filename)
{
    std::vector<std::string> ret;
    std::ifstream fin(filename);
    if (!fin.is_open())
        return ret;
    std::string line;
    while (std::getline(fin, line)) {
        std::string::size_type dotA = line.find(".a");
        std::string::size_type colon = line.find(":");
        if (colon > line.size() || dotA > colon)
            continue;
        std::string f = line.substr(0,colon);
        f[dotA + 1] = 's';
        ret.push_back(f);
    }
    return ret;
}

static std::vector<std::string> getSummaryData(const std::string &line, const std::string &data)
{
    std::vector<std::string> ret;
    const std::string::size_type start = line.find(" " + data + ":[");
    if (start == std::string::npos)
        return ret;
    const std::string::size_type end = line.find("]", start);
    if (end >= line.size())
        return ret;

    std::string::size_type pos1 = start + 3 + data.size();
    while (pos1 < end) {
        std::string::size_type pos2 = line.find_first_of(",]",pos1);
        ret.push_back(line.substr(pos1, pos2-pos1-1));
        pos1 = pos2 + 1;
    }

    return ret;
}

static void removeFunctionCalls(const std::string& calledFunction,
                                std::map<std::string, std::vector<std::string>> &functionCalledBy,
                                std::map<std::string, std::vector<std::string>> &functionCalls,
                                std::vector<std::string> &add)
{
    std::vector<std::string> calledBy = functionCalledBy[calledFunction];
    functionCalledBy.erase(calledFunction);
    for (const std::string &c: calledBy) {
        std::vector<std::string> &calls = functionCalls[c];
        calls.erase(std::remove(calls.begin(), calls.end(), calledFunction), calls.end());
        if (calls.empty()) {
            add.push_back(calledFunction);
            removeFunctionCalls(c, functionCalledBy, functionCalls, add);
        }
    }
}

void Settings::loadSummaries()
{
    if (buildDir.empty())
        return;

    std::vector<std::string> return1;
    std::map<std::string, std::vector<std::string>> functionCalls;
    std::map<std::string, std::vector<std::string>> functionCalledBy;

    // extract "functionNoreturn" and "functionCalledBy" from summaries
    std::vector<std::string> summaryFiles = getSummaryFiles(buildDir + "/files.txt");
    for (const std::string &filename: summaryFiles) {
        std::ifstream fin(buildDir + '/' + filename);
        if (!fin.is_open())
            continue;
        std::string line;
        while (std::getline(fin, line)) {
            // Get function name
            const std::string::size_type pos1 = 0;
            const std::string::size_type pos2 = line.find(" ", pos1);
            const std::string functionName = (pos2 == std::string::npos) ? line : line.substr(0, pos2);
            std::vector<std::string> call = getSummaryData(line, "call");
            functionCalls[functionName] = call;
            if (call.empty())
                return1.push_back(functionName);
            else {
                for (const std::string &c: call) {
                    functionCalledBy[c].push_back(functionName);
                }
            }
        }
    }
    summaryReturn.insert(return1.cbegin(), return1.cend());

    // recursively set "summaryNoreturn"
    for (const std::string &f: return1) {
        std::vector<std::string> return2;
        removeFunctionCalls(f, functionCalledBy, functionCalls, return2);
        summaryReturn.insert(return2.cbegin(), return2.cend());
    }
}
