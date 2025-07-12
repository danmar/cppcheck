/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "sarifreport.h"
#include "settings.h"
#include "cppcheck.h"

void SarifReport::addFinding(ErrorMessage msg) {
    mFindings.push_back(std::move(msg));
}

picojson::array SarifReport::serializeRules() const {
    picojson::array ret;
    std::set<std::string> ruleIds;
    for (const auto& finding : mFindings) {
        // github only supports findings with locations
        if (finding.callStack.empty())
            continue;
        if (ruleIds.insert(finding.id).second) {
            picojson::object rule;
            rule["id"] = picojson::value(finding.id);
            // rule.shortDescription.text
            picojson::object shortDescription;
            shortDescription["text"] = picojson::value(finding.shortMessage());
            rule["shortDescription"] = picojson::value(shortDescription);
            // rule.fullDescription.text
            picojson::object fullDescription;
            fullDescription["text"] = picojson::value(finding.verboseMessage());
            rule["fullDescription"] = picojson::value(fullDescription);
            // rule.help.text
            picojson::object help;
            help["text"] = picojson::value(finding.verboseMessage()); // FIXME provide proper help text
            rule["help"] = picojson::value(help);
            // rule.properties.precision, rule.properties.problem.severity
            picojson::object properties;
            properties["precision"] = picojson::value(sarifPrecision(finding));
            double securitySeverity = 0;
            if (finding.severity == Severity::error && !ErrorLogger::isCriticalErrorId(finding.id))
                securitySeverity = 9.9; // We see undefined behavior
            //else if (finding.severity == Severity::warning)
            //    securitySeverity = 5.1; // We see potential undefined behavior
            if (securitySeverity > 0.5) {
                properties["security-severity"] = picojson::value(securitySeverity);
                const picojson::array tags{picojson::value("security")};
                properties["tags"] = picojson::value(tags);
            }
            rule["properties"] = picojson::value(properties);
            // rule.defaultConfiguration.level
            picojson::object defaultConfiguration;
            defaultConfiguration["level"] = picojson::value(sarifSeverity(finding));
            rule["defaultConfiguration"] = picojson::value(defaultConfiguration);

            ret.emplace_back(rule);
        }
    }
    return ret;
}

picojson::array SarifReport::serializeLocations(const ErrorMessage& finding) {
    picojson::array ret;
    for (const auto& location : finding.callStack) {
        picojson::object physicalLocation;
        picojson::object artifactLocation;
        artifactLocation["uri"] = picojson::value(location.getfile(false));
        physicalLocation["artifactLocation"] = picojson::value(artifactLocation);
        picojson::object region;
        region["startLine"] = picojson::value(static_cast<int64_t>(std::max(1, location.line)));
        region["startColumn"] = picojson::value(static_cast<int64_t>(std::max(1U, location.column)));
        region["endLine"] = region["startLine"];
        region["endColumn"] = region["startColumn"];
        physicalLocation["region"] = picojson::value(region);
        picojson::object loc;
        loc["physicalLocation"] = picojson::value(physicalLocation);
        ret.emplace_back(loc);
    }
    return ret;
}

picojson::array SarifReport::serializeResults() const {
    picojson::array results;
    for (const auto& finding : mFindings) {
        // github only supports findings with locations
        if (finding.callStack.empty())
            continue;
        picojson::object res;
        res["level"] = picojson::value(sarifSeverity(finding));
        res["locations"] = picojson::value(serializeLocations(finding));
        picojson::object message;
        message["text"] = picojson::value(finding.shortMessage());
        res["message"] = picojson::value(message);
        res["ruleId"] = picojson::value(finding.id);
        results.emplace_back(res);
    }
    return results;
}

picojson::value SarifReport::serializeRuns(const std::string& productName, const std::string& version) const {
    picojson::object driver;
    driver["name"] = picojson::value(productName);
    driver["semanticVersion"] = picojson::value(version);
    driver["informationUri"] = picojson::value("https://cppcheck.sourceforge.io");
    driver["rules"] = picojson::value(serializeRules());
    picojson::object tool;
    tool["driver"] = picojson::value(driver);
    picojson::object run;
    run["tool"] = picojson::value(tool);
    run["results"] = picojson::value(serializeResults());
    picojson::array runs{picojson::value(run)};
    return picojson::value(runs);
}

std::string SarifReport::serialize(std::string productName) const {
    const auto nameAndVersion = Settings::getNameAndVersion(productName);
    productName = nameAndVersion.first.empty() ? "Cppcheck" : nameAndVersion.first;
    std::string version = nameAndVersion.first.empty() ? CppCheck::version() : nameAndVersion.second;
    if (version.find(' ') != std::string::npos)
        version.erase(version.find(' '), std::string::npos);

    picojson::object doc;
    doc["version"] = picojson::value("2.1.0");
    doc["$schema"] = picojson::value("https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/schemas/sarif-schema-2.1.0.json");
    doc["runs"] = serializeRuns(productName, version);

    return picojson::value(doc).serialize(true);
}

std::string SarifReport::sarifSeverity(const ErrorMessage& errmsg) {
    if (ErrorLogger::isCriticalErrorId(errmsg.id))
        return "error";
    switch (errmsg.severity) {
    case Severity::error:
    case Severity::warning:
    case Severity::style:
    case Severity::portability:
    case Severity::performance:
        return "warning";
    case Severity::information:
    case Severity::internal:
    case Severity::debug:
    case Severity::none:
        return "note";
    }
    return "note";
}

std::string SarifReport::sarifPrecision(const ErrorMessage& errmsg) {
    if (errmsg.certainty == Certainty::inconclusive)
        return "medium";
    return "high";
}

