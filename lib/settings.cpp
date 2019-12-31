/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

std::atomic<bool> Settings::mTerminated;

const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";

Settings::Settings()
    : mEnabled(0),
      checkConfiguration(false),
      checkLibrary(false),
      checkHeaders(true),
      checkUnusedTemplates(false),
      debugSimplified(false),
      debugnormal(false),
      debugwarnings(false),
      debugtemplate(false),
      dump(false),
      enforcedLang(None),
      exceptionHandling(false),
      exitCode(0),
      experimental(false),
      force(false),
      inconclusive(false),
      verification(false),
      debugVerification(false),
      inlineSuppressions(false),
      jobs(1),
      jointSuppressionReport(false),
      loadAverage(0),
      maxConfigs(12),
      checkAllConfigurations(true),
      maxCtuDepth(2),
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

std::vector<Settings::Diff> Settings::loadDiffFile(std::istream &istr)
{
    std::vector<Settings::Diff> ret;
    std::string line;
    std::string filename;
    while (std::getline(istr, line)) {
        if (line.compare(0, 11, "diff --git ") == 0) {
            std::string::size_type pos = line.rfind(" b/");
            if (pos == std::string::npos)
                continue;
            filename = line.substr(pos+3);
        }
        if (line.compare(0,4,"@@ -") == 0) {
            std::string::size_type pos1 = line.find(" ",4);
            if (pos1 == std::string::npos)
                continue;
            std::string::size_type pos2 = line.find(" ",pos1 + 1);
            if (pos2 == std::string::npos || pos2 < pos1+3)
                continue;
            if (line[pos1+1] != '+')
                continue;
            std::string::size_type posComma = line.find(",", pos1);
            if (posComma > pos2)
                continue;
            std::string line1 = line.substr(pos1 + 2, posComma - pos1 - 2);
            std::string line2 = line.substr(posComma+1, pos2 - posComma - 1);
            Diff diff;
            diff.filename = filename;
            diff.fromLine = std::atoi(line1.c_str());
            diff.toLine = std::atoi(line1.c_str()) + std::atoi(line2.c_str());
            ret.push_back(diff);
        }
    }
    return ret;
}
