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

#ifndef CLI_ANALYSIS_REPORT_H
#define CLI_ANALYSIS_REPORT_H

#include "analysisreport.h"

/**
 * @brief The CLIAnalysisReport class is used to contain the results of a CppCheck analysis
 * and output the results to STDERR.
 */
class CLIAnalysisReport : public AnalysisReport {
public:
    CLIAnalysisReport(bool verbose, std::string templateFormat, std::string templateLocation, std::ofstream* errorOutput);
    void addFinding(const ErrorMessage msg) override;
    std::string serialize() override;

private:
    bool mVerbose;
    std::string mTemplateFormat;
    std::string mTemplateLocation;
    std::ofstream *mErrorOutput;
};

#endif //CLI_ANALYSIS_REPORT_H
