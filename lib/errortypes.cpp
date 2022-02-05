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

#include "errortypes.h"

std::string Severity::toString(Severity::SeverityType severity)
{
    switch (severity) {
    case none:
        return "";
    case error:
        return "error";
    case warning:
        return "warning";
    case style:
        return "style";
    case performance:
        return "performance";
    case portability:
        return "portability";
    case information:
        return "information";
    case debug:
        return "debug";
    }
    throw InternalError(nullptr, "Unknown severity");
}
Severity::SeverityType Severity::fromString(const std::string& severity)
{
    if (severity.empty())
        return none;
    if (severity == "none")
        return none;
    if (severity == "error")
        return error;
    if (severity == "warning")
        return warning;
    if (severity == "style")
        return style;
    if (severity == "performance")
        return performance;
    if (severity == "portability")
        return portability;
    if (severity == "information")
        return information;
    if (severity == "debug")
        return debug;
    return none;
}
