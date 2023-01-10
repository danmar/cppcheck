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

#ifndef SARIF_ANALYSIS_REPORT_H
#define SARIF_ANALYSIS_REPORT_H

#include "analysisreport.h"

#include <string>
#include <map>
#include <vector>

/**
 * @brief The SARIFAnalysisReport class is used to collect and export findings of a
 * CppCheck analysis in the open Static Analysis Results Interchange Format (S.A.R.I.F.).
 */
class SARIFAnalysisReport : public AnalysisReport {
public:
    explicit SARIFAnalysisReport(std::string versionNumber);
    void addFinding(ErrorMessage msg) override;
    std::string serialize() override;
private:
    std::string mVersionNumber;
    std::map<std::string, std::vector<ErrorMessage>> mFindings;
};

#endif //SARIF_ANALYSIS_REPORT_H
