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

#ifndef SARIFREPORT_H
#define SARIFREPORT_H

#include "errorlogger.h"
#include "json.h"
#include <vector>

class SarifReport {
public:
    void addFinding(ErrorMessage msg);

    picojson::array serializeRules() const;

    static picojson::array serializeLocations(const ErrorMessage& finding);

    picojson::array serializeResults() const;

    picojson::value serializeRuns(const std::string& productName, const std::string& version) const;

    std::string serialize(std::string productName) const;
private:

    static std::string sarifSeverity(const ErrorMessage& errmsg);

    static std::string sarifPrecision(const ErrorMessage& errmsg);

    std::vector<ErrorMessage> mFindings;
};

#endif
