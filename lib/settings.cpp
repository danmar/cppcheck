/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include "path.h"
#include "summaries.h"
#include "timer.h"
#include "vfvalue.h"

#include <fstream>

#include "json.h"

std::atomic<bool> Settings::mTerminated;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

Settings::Settings()
{
    severity.setEnabled(Severity::error, true);
    certainty.setEnabled(Certainty::normal, true);
    setCheckLevelNormal();
}

void Settings::loadCppcheckCfg()
{
    std::string fileName = Path::getPathFromFilename(exename) + "cppcheck.cfg";
#ifdef FILESDIR
    if (Path::isFile(FILESDIR "/cppcheck.cfg"))
        fileName = FILESDIR "/cppcheck.cfg";
#endif

    std::ifstream fin(fileName);
    if (!fin.is_open())
        return;
    picojson::value json;
    fin >> json;
    if (!picojson::get_last_error().empty())
        return;
    picojson::object obj = json.get<picojson::object>();
    if (obj.count("productName") && obj["productName"].is<std::string>())
        cppcheckCfgProductName = obj["productName"].get<std::string>();
    if (obj.count("about") && obj["about"].is<std::string>())
        cppcheckCfgAbout = obj["about"].get<std::string>();
    if (obj.count("addons") && obj["addons"].is<picojson::array>()) {
        for (const picojson::value &v : obj["addons"].get<picojson::array>()) {
            const std::string &s = v.get<std::string>();
            if (!Path::isAbsolute(s))
                addons.emplace(Path::getPathFromFilename(fileName) + s);
            else
                addons.emplace(s);
        }
    }
    if (obj.count("suppressions") && obj["suppressions"].is<picojson::array>()) {
        for (const picojson::value &v : obj["suppressions"].get<picojson::array>())
            nomsg.addSuppressionLine(v.get<std::string>());
    }
}

std::string Settings::parseEnabled(const std::string &str, std::tuple<SimpleEnableGroup<Severity::SeverityType>, SimpleEnableGroup<Checks>> &groups)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("--enable parameter is empty");
            std::string errmsg(parseEnabled(str.substr(prevPos, pos - prevPos), groups));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("--enable parameter is empty");
        return parseEnabled(str.substr(prevPos), groups);
    }

    auto& severity = std::get<0>(groups);
    auto& checks = std::get<1>(groups);

    if (str == "all") {
        // "error" is always enabled and cannot be controlled - so exclude it from "all"
        SimpleEnableGroup<Severity::SeverityType> newSeverity;
        newSeverity.fill();
        newSeverity.disable(Severity::SeverityType::error);
        severity.enable(newSeverity);
        checks.enable(Checks::missingInclude);
        checks.enable(Checks::unusedFunction);
    } else if (str == "warning") {
        severity.enable(Severity::warning);
    } else if (str == "style") {
        severity.enable(Severity::style);
    } else if (str == "performance") {
        severity.enable(Severity::performance);
    } else if (str == "portability") {
        severity.enable(Severity::portability);
    } else if (str == "information") {
        severity.enable(Severity::information);
    } else if (str == "unusedFunction") {
        checks.enable(Checks::unusedFunction);
    } else if (str == "missingInclude") {
        checks.enable(Checks::missingInclude);
    }
#ifdef CHECK_INTERNAL
    else if (str == "internal") {
        checks.enable(Checks::internalCheck);
    }
#endif
    else {
        // the actual option is prepending in the applyEnabled() call
        if (str.empty())
            return " parameter is empty";
        return " parameter with the unknown name '" + str + "'";
    }

    return "";
}

std::string Settings::addEnabled(const std::string &str)
{
    return applyEnabled(str, true);
}

std::string Settings::removeEnabled(const std::string &str)
{
    return applyEnabled(str, false);
}

std::string Settings::applyEnabled(const std::string &str, bool enable)
{
    std::tuple<SimpleEnableGroup<Severity::SeverityType>, SimpleEnableGroup<Checks>> groups;
    std::string errmsg = parseEnabled(str, groups);
    if (!errmsg.empty())
        return (enable ? "--enable" : "--disable") + errmsg;

    const auto s = std::get<0>(groups);
    const auto c = std::get<1>(groups);
    if (enable) {
        severity.enable(s);
        checks.enable(c);
    }
    else {
        severity.disable(s);
        checks.disable(c);
    }
    // FIXME: hack to make sure "error" is always enabled
    severity.enable(Severity::SeverityType::error);
    return errmsg;
}

bool Settings::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!severity.isEnabled(Severity::warning) && (value->condition || value->defaultArg))
        return false;
    if (!certainty.isEnabled(Certainty::inconclusive) && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}

void Settings::loadSummaries()
{
    Summaries::loadReturn(buildDir, summaryReturn);
}


void Settings::setCheckLevelExhaustive()
{
    // Checking can take a little while. ~ 10 times slower than normal analysis is OK.
    performanceValueFlowMaxIfCount = -1;
    performanceValueFlowMaxSubFunctionArgs = 256;
}

void Settings::setCheckLevelNormal()
{
    // Checking should finish in reasonable time.
    performanceValueFlowMaxSubFunctionArgs = 8;
    performanceValueFlowMaxIfCount = 100;
}
