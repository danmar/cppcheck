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

#include "errortypes.h"

#include "utils.h"

static std::string typeToString(InternalError::Type type)
{
    switch (type) {
    case InternalError::Type::AST:
        return "internalAstError";
    case InternalError::Type::SYNTAX:
        return "syntaxError";
    case InternalError::Type::UNKNOWN_MACRO:
        return "unknownMacro";
    case InternalError::Type::INTERNAL:
        return "internalError";
    case InternalError::Type::LIMIT:
        return "cppcheckLimit";
    case InternalError::Type::INSTANTIATION:
        return "instantiationError";
    }
    cppcheck::unreachable();
}

InternalError::InternalError(const Token *tok, std::string errorMsg, Type type) :
    InternalError(tok, std::move(errorMsg), "", type)
{}

InternalError::InternalError(const Token *tok, std::string errorMsg, std::string details, Type type) :
    token(tok), errorMessage(std::move(errorMsg)), details(std::move(details)), type(type), id(typeToString(type))
{}

std::string severityToString(Severity severity)
{
    switch (severity) {
    case Severity::none:
        return "";
    case Severity::error:
        return "error";
    case Severity::warning:
        return "warning";
    case Severity::style:
        return "style";
    case Severity::performance:
        return "performance";
    case Severity::portability:
        return "portability";
    case Severity::information:
        return "information";
    case Severity::debug:
        return "debug";
    case Severity::internal:
        return "internal";
    }
    throw InternalError(nullptr, "Unknown severity");
}

Severity severityFromString(const std::string& severity)
{
    if (severity.empty())
        return Severity::none;
    if (severity == "none")
        return Severity::none;
    if (severity == "error")
        return Severity::error;
    if (severity == "warning")
        return Severity::warning;
    if (severity == "style")
        return Severity::style;
    if (severity == "performance")
        return Severity::performance;
    if (severity == "portability")
        return Severity::portability;
    if (severity == "information")
        return Severity::information;
    if (severity == "debug")
        return Severity::debug;
    if (severity == "internal")
        return Severity::internal;
    return Severity::none;
}

const CWE CWE119(119U); // Improper Restriction of Operations within the Bounds of a Memory Buffer
const CWE CWE128(128U); // Wrap-around Error
const CWE CWE131(131U); // Incorrect Calculation of Buffer Size
const CWE CWE170(170U); // Improper Null Termination
const CWE CWE190(190U); // Integer Overflow or Wraparound
const CWE CWE195(195U); // Signed to Unsigned Conversion Error
const CWE CWE197(197U); // Numeric Truncation Error
const CWE CWE252(252U); // Unchecked Return Value
const CWE CWE362(362U); // Concurrent Execution using Shared Resource with Improper Synchronization ('Race Condition')
const CWE CWE369(369U); // Divide By Zero
const CWE CWE398(398U); // Indicator of Poor Code Quality
const CWE CWE401(401U); // Improper Release of Memory Before Removing Last Reference ('Memory Leak')
const CWE CWE404(404U); // Improper Resource Shutdown or Release
const CWE CWE415(415U);
const CWE CWE467(467U); // Use of sizeof() on a Pointer Type
const CWE CWE475(475U); // Undefined Behavior for Input to API
const CWE CWE477(477U); // Use of Obsolete Functions
const CWE CWE480(480U); // Use of Incorrect Operator
const CWE CWE561(561U); // Dead Code
const CWE CWE562(562U); // Return of Stack Variable Address
const CWE CWE563(563U); // Assignment to Variable without Use ('Unused Variable')
const CWE CWE570(570U); // Expression is Always False
const CWE CWE571(571U); // Expression is Always True
const CWE CWE587(587U); // Assignment of a Fixed Address to a Pointer
const CWE CWE590(590U); // Free of Memory not on the Heap
const CWE CWE595(595U); // Comparison of Object References Instead of Object Contents
const CWE CWE597(597U); // Use of Wrong Operator in String Comparison
const CWE CWE628(628U); // Function Call with Incorrectly Specified Arguments
const CWE CWE664(664U); // Improper Control of a Resource Through its Lifetime
const CWE CWE665(665U); // Improper Initialization
const CWE CWE667(667U); // Improper Locking
const CWE CWE672(672U); // Operation on a Resource after Expiration or Release
const CWE CWE682(682U); // Incorrect Calculation
const CWE CWE683(683U); // Function Call With Incorrect Order of Arguments
const CWE CWE685(685U); // Function Call With Incorrect Number of Arguments
const CWE CWE686(686U); // Function Call With Incorrect Argument Type
const CWE CWE687(687U); // Function Call With Incorrectly Specified Argument Value
const CWE CWE688(688U); // Function Call With Incorrect Variable or Reference as Argument
const CWE CWE703(703U); // Improper Check or Handling of Exceptional Conditions
const CWE CWE704(704U); // Incorrect Type Conversion or Cast
const CWE CWE758(758U); // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
const CWE CWE762(762U); // Mismatched Memory Management Routines
const CWE CWE768(768U); // Incorrect Short Circuit Evaluation
const CWE CWE771(771U); // Missing Reference to Active Allocated Resource
const CWE CWE772(772U); // Missing Release of Resource after Effective Lifetime
const CWE CWE783(783U); // Operator Precedence Logic Error
const CWE CWE786(786U); // Access of Memory Location Before Start of Buffer
const CWE CWE788(788U); // Access of Memory Location After End of Buffer
const CWE CWE825(825U); // Expired Pointer Dereference
const CWE CWE833(833U); // Deadlock
const CWE CWE834(834U); // Excessive Iteration
const CWE CWE910(910U); // Use of Expired File Descriptor

const CWE CWE_ARGUMENT_SIZE(398U);          // Indicator of Poor Code Quality
const CWE CWE_ARRAY_INDEX_THEN_CHECK(398U); // Indicator of Poor Code Quality
const CWE CWE_BUFFER_OVERRUN(788U);         // Access of Memory Location After End of Buffer
const CWE CWE_BUFFER_UNDERRUN(786U);        // Access of Memory Location Before Start of Buffer
const CWE CWE_INCORRECT_CALCULATION(682U);
const CWE CWE_NULL_POINTER_DEREFERENCE(476U);
const CWE CWE_ONE_DEFINITION_RULE(758U);
const CWE CWE_POINTER_ARITHMETIC_OVERFLOW(758U); // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
const CWE CWE_USE_OF_UNINITIALIZED_VARIABLE(457U);
const CWE uncheckedErrorConditionCWE(391U);
