/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef severityH
#define severityH
//---------------------------------------------------------------------------

#include "config.h"

#include <list>
#include <string>

/// @addtogroup Core
/// @{
class Token;

/** @brief Simple container to be thrown when internal error is detected. */
struct InternalError {
    enum Type {AST, SYNTAX, UNKNOWN_MACRO, INTERNAL, LIMIT, INSTANTIATION};
    InternalError(const Token *tok, const std::string &errorMsg, Type type = INTERNAL);
    const Token *token;
    std::string errorMessage;
    Type type;
    std::string id;
};

/** @brief enum class for severity. Used when reporting errors. */
class CPPCHECKLIB Severity {
public:
    /**
     * Message severities.
     */
    enum SeverityType {
        /**
         * No severity (default value).
         */
        none,
        /**
         * Programming error.
         * This indicates severe error like memory leak etc.
         * The error is certain.
         */
        error,
        /**
         * Warning.
         * Used for dangerous coding style that can cause severe runtime errors.
         * For example: forgetting to initialize a member variable in a constructor.
         */
        warning,
        /**
         * Style warning.
         * Used for general code cleanup recommendations. Fixing these
         * will not fix any bugs but will make the code easier to maintain.
         * For example: redundant code, unreachable code, etc.
         */
        style,
        /**
         * Performance warning.
         * Not an error as is but suboptimal code and fixing it probably leads
         * to faster performance of the compiled code.
         */
        performance,
        /**
         * Portability warning.
         * This warning indicates the code is not properly portable for
         * different platforms and bitnesses (32/64 bit). If the code is meant
         * to compile in different platforms and bitnesses these warnings
         * should be fixed.
         */
        portability,
        /**
         * Checking information.
         * Information message about the checking (process) itself. These
         * messages inform about header files not found etc issues that are
         * not errors in the code but something user needs to know.
         */
        information,
        /**
         * Debug message.
         * Debug-mode message useful for the developers.
         */
        debug
    };

    static std::string toString(SeverityType severity) {
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
    static SeverityType fromString(const std::string &severity) {
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
};

struct CWE {
    explicit CWE(unsigned short cweId) : id(cweId) {}
    unsigned short id;
};

typedef std::pair<const Token *, std::string> ErrorPathItem;
typedef std::list<ErrorPathItem> ErrorPath;

/// @}
//---------------------------------------------------------------------------
#endif // severityH
