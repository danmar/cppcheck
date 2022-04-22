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

#include "settings.h"
#include "path.h"
#include "summaries.h"
#include "valueflow.h"

#include <fstream>

#define PICOJSON_USE_INT64
#include <picojson.h>

std::atomic<bool> Settings::mTerminated;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

Settings::Settings()
    : checkAllConfigurations(true),
    checkConfiguration(false),
    checkHeaders(true),
    checkLibrary(false),
    checkUnusedTemplates(true),
    clang(false),
    clangExecutable("clang"),
    clangTidy(false),
    daca(false),
    debugnormal(false),
    debugSimplified(false),
    debugtemplate(false),
    debugwarnings(false),
    dump(false),
    enforcedLang(None),
    exceptionHandling(false),
    exitCode(0),
    force(false),
    inlineSuppressions(false),
    jobs(1),
    jointSuppressionReport(false),
#ifdef THREADING_MODEL_FORK
    loadAverage(0),
#endif
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
    severity.setEnabled(Severity::error, true);
    certainty.setEnabled(Certainty::normal, true);
}

void Settings::loadCppcheckCfg()
{
    std::string fileName = Path::getPathFromFilename(exename) + "cppcheck.cfg";
#ifdef FILESDIR
    if (Path::fileExists(FILESDIR "/cppcheck.cfg"))
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
                addons.push_back(Path::getPathFromFilename(fileName) + s);
            else
                addons.push_back(s);
        }
    }
    if (obj.count("suppressions") && obj["suppressions"].is<picojson::array>()) {
        for (const picojson::value &v : obj["suppressions"].get<picojson::array>())
            nomsg.addSuppressionLine(v.get<std::string>());
    }
}

std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("--enable parameter is empty");
            const std::string errmsg(addEnabled(str.substr(prevPos, pos - prevPos)));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("--enable parameter is empty");
        return addEnabled(str.substr(prevPos));
    }

    if (str == "all") {
        severity.fill();
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
        checks.enable(Checks::missingInclude);
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
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string();
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
