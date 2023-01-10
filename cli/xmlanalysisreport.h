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

#ifndef XML_ANALYSIS_REPORT_H
#define XML_ANALYSIS_REPORT_H

#include "analysisreport.h"

#include <sstream>

/**
 * @brief The XMLAnalysisReport class is used to contain the results of a CppCheck analysis
 * and output the results in the XML format.
 */
class XMLAnalysisReport : public AnalysisReport {
public:
    explicit XMLAnalysisReport(const std::string& productName);
    void addFinding(const ErrorMessage msg) override;
    std::string serialize() override;
private:
    std::stringstream mBuffer;
};

#endif //XML_ANALYSIS_REPORT_H
