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

#ifndef ANALYSIS_REPORT_H
#define ANALYSIS_REPORT_H

#include "errorlogger.h"

#include <string>

/**
 * @brief The AnalysisReport class is an abstract class meant to be sub-classed
 * by others classes that will contain the results of a CppCheck analysis, and
 * output those results in a particular format.
 */
class AnalysisReport {
public:
    /**
     * Submit a CppCheck result for inclusion into the report.
     */
    virtual void addFinding(ErrorMessage msg) = 0;

    /**
     * Output the results as a string.
     */
    virtual std::string serialize() = 0;

    virtual ~AnalysisReport() = default;
};

#endif // ANALYSIS_REPORT_H
