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

#include "checkersreport.h"
#include "checkers.h"
#include <sstream>

static bool isCppcheckPremium(const Settings& settings) {
    return (settings.cppcheckCfgProductName.compare(0, 16, "Cppcheck Premium") == 0);
}

static std::string getMisraRuleSeverity(const std::string& rule) {
    if (checkers::misraRuleSeverity.count(rule) > 0)
        return checkers::misraRuleSeverity.at(rule);
    return "style";
}

static bool isMisraRuleInconclusive(const std::string& rule) {
    return rule == "8.3";
}

static bool isMisraRuleActive(const std::string& rule, int amendment, const std::string& severity, const Settings& settings) {
    if (!isCppcheckPremium(settings) && amendment >= 3)
        return false;
    const bool inconclusive = isMisraRuleInconclusive(rule);
    if (inconclusive && !settings.certainty.isEnabled(Certainty::inconclusive))
        return false;
    if (severity == "warning")
        return settings.severity.isEnabled(Severity::warning);
    if (severity == "style")
        return settings.severity.isEnabled(Severity::style);
    return true; // error severity
}

CheckersReport::CheckersReport(const Settings& settings, const std::set<std::string>& activeCheckers)
    : mSettings(settings), mActiveCheckers(activeCheckers)
{}

int CheckersReport::getActiveCheckersCount()
{
    if (mAllCheckersCount == 0) {
        countCheckers();
    }
    return mActiveCheckersCount;
}

int CheckersReport::getAllCheckersCount()
{
    if (mAllCheckersCount == 0) {
        countCheckers();
    }
    return mAllCheckersCount;
}

void CheckersReport::countCheckers()
{
    mActiveCheckersCount = mAllCheckersCount = 0;

    for (const auto& checkReq: checkers::allCheckers) {
        if (mActiveCheckers.count(checkReq.first) > 0)
            ++mActiveCheckersCount;
        ++mAllCheckersCount;
    }
    for (const auto& checkReq: checkers::premiumCheckers) {
        if (mActiveCheckers.count(checkReq.first) > 0)
            ++mActiveCheckersCount;
        ++mAllCheckersCount;
    }
    if (mSettings.premiumArgs.find("misra-c-") != std::string::npos || mSettings.addons.count("misra")) {
        for (const checkers::MisraInfo& info: checkers::misraC2012Rules) {
            const std::string rule = std::to_string(info.a) + "." + std::to_string(info.b);
            const std::string severity = getMisraRuleSeverity(rule);
            const bool active = isMisraRuleActive(rule, info.amendment, severity, mSettings);
            if (active)
                ++mActiveCheckersCount;
            ++mAllCheckersCount;
        }
    }
}

std::string CheckersReport::getReport(const std::string& criticalErrors) const
{
    std::ostringstream fout;

    fout << "Critical errors" << std::endl;
    fout << "---------------" << std::endl;
    if (!criticalErrors.empty()) {
        fout << "There was critical errors (" << criticalErrors << ")" << std::endl;
        fout << "All checking is skipped for a file with such error" << std::endl;
    } else {
        fout << "No critical errors, all files were checked." << std::endl;
        fout << "Important: Analysis is still not guaranteed to be 'complete' it is possible there are false negatives." << std::endl;
    }

    fout << std::endl << std::endl;
    fout << "Open source checkers" << std::endl;
    fout << "--------------------" << std::endl;

    int maxCheckerSize = 0;
    for (const auto& checkReq: checkers::allCheckers) {
        const std::string& checker = checkReq.first;
        if (checker.size() > maxCheckerSize)
            maxCheckerSize = checker.size();
    }
    for (const auto& checkReq: checkers::allCheckers) {
        const std::string& checker = checkReq.first;
        const bool active = mActiveCheckers.count(checkReq.first) > 0;
        const std::string& req = checkReq.second;
        fout << (active ? "Yes  " : "No   ") << checker;
        if (!active && !req.empty())
            fout << std::string(maxCheckerSize + 4 - checker.size(), ' ') << "require:" + req;
        fout << std::endl;
    }

    const bool cppcheckPremium = isCppcheckPremium(mSettings);

    auto reportSection = [&fout, cppcheckPremium]
                             (const std::string& title,
                             const Settings& settings,
                             const std::set<std::string>& activeCheckers,
                             const std::map<std::string, std::string>& premiumCheckers,
                             const std::string& substring) {
        fout << std::endl << std::endl;
        fout << title << std::endl;
        fout << std::string(title.size(), '-') << std::endl;
        if (!cppcheckPremium) {
            fout << "Not available, Cppcheck Premium is not used" << std::endl;
            return;
        }
        int maxCheckerSize = 0;
        for (const auto& checkReq: premiumCheckers) {
            const std::string& checker = checkReq.first;
            if (checker.find(substring) != std::string::npos && checker.size() > maxCheckerSize)
                maxCheckerSize = checker.size();
        }
        for (const auto& checkReq: premiumCheckers) {
            const std::string& checker = checkReq.first;
            if (checker.find(substring) == std::string::npos)
                continue;
            std::string req = checkReq.second;
            bool active = cppcheckPremium && activeCheckers.count(checker) > 0;
            if (req == "warning")
                active &= settings.severity.isEnabled(Severity::warning);
            else if (req == "style")
                active &= settings.severity.isEnabled(Severity::style);
            else if (!req.empty())
                active = false; // FIXME: handle req
            fout << (active ? "Yes  " : "No   ") << checker;
            if (!req.empty())
                req = "premium," + req;
            else
                req = "premium";
            if (!active)
                fout << std::string(maxCheckerSize + 4 - checker.size(), ' ') << "require:" + req;
            fout << std::endl;
        }
    };

    reportSection("Premium checkers", mSettings, mActiveCheckers, checkers::premiumCheckers, "::");
    reportSection("Autosar", mSettings, mActiveCheckers, checkers::premiumCheckers, "Autosar: ");
    reportSection("Cert C", mSettings, mActiveCheckers, checkers::premiumCheckers, "Cert C: ");
    reportSection("Cert C++", mSettings, mActiveCheckers, checkers::premiumCheckers, "Cert C++: ");

    int misra = 0;
    if (mSettings.premiumArgs.find("misra-c-2012") != std::string::npos)
        misra = 2012;
    else if (mSettings.premiumArgs.find("misra-c-2023") != std::string::npos)
        misra = 2023;
    else if (mSettings.addons.count("misra"))
        misra = 2012;

    if (misra == 0) {
        fout << std::endl << std::endl;
        fout << "Misra C" << std::endl;
        fout << "-------" << std::endl;
        fout << "Misra is not enabled" << std::endl;
    } else {
        fout << std::endl << std::endl;
        fout << "Misra C " << misra << std::endl;
        fout << "------------" << std::endl;
        for (const checkers::MisraInfo& info: checkers::misraC2012Rules) {
            const std::string rule = std::to_string(info.a) + "." + std::to_string(info.b);
            const std::string severity = getMisraRuleSeverity(rule);
            const bool active = isMisraRuleActive(rule, info.amendment, severity, mSettings);
            const bool inconclusive = isMisraRuleInconclusive(rule);
            fout << (active ? "Yes  " : "No   ") << rule;
            std::string extra;
            if (misra == 2012 && info.amendment >= 1)
                extra = " amendment:" + std::to_string(info.amendment);
            std::string reqs;
            if (info.amendment >= 3)
                reqs += ",premium";
            if (severity != "error")
                reqs += "," + severity;
            if (inconclusive)
                reqs += ",inconclusive";
            if (!active && !reqs.empty())
                extra += " require:" + reqs.substr(1);
            if (!extra.empty())
                fout << std::string(7 - rule.size(), ' ') << extra;
            fout << '\n';
        }
    }

    reportSection("Misra C++ 2008", mSettings, mActiveCheckers, checkers::premiumCheckers, "Misra C++: ");

    return fout.str();
}
