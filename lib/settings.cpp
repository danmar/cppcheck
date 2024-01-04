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

std::string Settings::loadCppcheckCfg()
{
    static const std::string cfgFilename = "cppcheck.cfg";
    std::string fileName;
#ifdef FILESDIR
    if (Path::isFile(Path::join(FILESDIR, cfgFilename)))
        fileName = Path::join(FILESDIR, cfgFilename);
#endif
    // cppcheck-suppress knownConditionTrueFalse
    if (fileName.empty()) {
        fileName = Path::getPathFromFilename(exename) + cfgFilename;
        if (!Path::isFile(fileName))
            return "";
    }

    std::ifstream fin(fileName);
    if (!fin.is_open())
        return "could not open file";
    picojson::value json;
    fin >> json;
    {
        const std::string& lastErr = picojson::get_last_error();
        if (!lastErr.empty())
            return "not a valid JSON - " + lastErr;
    }
    const picojson::object& obj = json.get<picojson::object>();
    {
        const picojson::object::const_iterator it = obj.find("productName");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<std::string>())
                return "'productName' is not a string";
            cppcheckCfgProductName = v.get<std::string>();
        }
    }
    {
        const picojson::object::const_iterator it = obj.find("about");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<std::string>())
                return "'about' is not a string";
            cppcheckCfgAbout = v.get<std::string>();
        }
    }
    {
        const picojson::object::const_iterator it = obj.find("addons");
        if (it != obj.cend()) {
            const auto& entry = it->second;
            if (!entry.is<picojson::array>())
                return "'addons' is not an array";
            for (const picojson::value &v : entry.get<picojson::array>())
            {
                if (!v.is<std::string>())
                    return "'addons' array entry is not a string";
                const std::string &s = v.get<std::string>();
                if (!Path::isAbsolute(s))
                    addons.emplace(Path::join(Path::getPathFromFilename(fileName), s));
                else
                    addons.emplace(s);
            }
        }
    }
    {
        const picojson::object::const_iterator it = obj.find("suppressions");
        if (it != obj.cend()) {
            const auto& entry = it->second;
            if (!entry.is<picojson::array>())
                return "'suppressions' is not an array";
            for (const picojson::value &v : entry.get<picojson::array>())
            {
                if (!v.is<std::string>())
                    return "'suppressions' array entry is not a string";
                const std::string &s = v.get<std::string>();
                const std::string err = nomsg.addSuppressionLine(s);
                if (!err.empty())
                    return "could not parse suppression '" + s + "' - " + err;
            }
        }
    }
    {
        const picojson::object::const_iterator it = obj.find("safety");
        if (it != obj.cend()) {
            const auto& v = it->second;
            if (!v.is<bool>())
                return "'safety' is not a bool";
            safety = v.get<bool>();
        }
    }

    return "";
}

std::string Settings::parseEnabled(const std::string &str, std::tuple<SimpleEnableGroup<Severity>, SimpleEnableGroup<Checks>> &groups)
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
        SimpleEnableGroup<Severity> newSeverity;
        newSeverity.fill();
        newSeverity.disable(Severity::error);
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
    std::tuple<SimpleEnableGroup<Severity>, SimpleEnableGroup<Checks>> groups;
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
    severity.enable(Severity::error);
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
