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

//---------------------------------------------------------------------------
#ifndef errortypesH
#define errortypesH
//---------------------------------------------------------------------------

#include "config.h"

#include <stdexcept>
#include <list>
#include <string>
#include <utility>

/// @addtogroup Core
/// @{
class Token;

/** @brief Simple container to be thrown when internal error is detected. */
struct CPPCHECKLIB InternalError {
    enum Type {AST, SYNTAX, UNKNOWN_MACRO, INTERNAL, LIMIT, INSTANTIATION};

    InternalError(const Token *tok, std::string errorMsg, Type type = INTERNAL);
    InternalError(const Token *tok, std::string errorMsg, std::string details, Type type = INTERNAL);

    const Token *token;
    std::string errorMessage;
    std::string details;
    Type type;
    std::string id;
};

class TerminateException : public std::runtime_error {
public:
    TerminateException() : std::runtime_error("terminate") {}
};

enum class Certainty {
    normal, inconclusive
};

enum class Checks {
    unusedFunction, missingInclude, internalCheck
};

/** @brief enum class for severity. Used when reporting errors. */
enum class Severity {
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
    debug,
    /**
     * Internal message.
     * Message will not be shown to the user.
     * Tracking what checkers is executed, tracking suppressed critical errors, etc.
     */
    internal
};

CPPCHECKLIB std::string severityToString(Severity severity);
CPPCHECKLIB Severity severityFromString(const std::string &severity);

struct CWE {
    explicit CWE(unsigned short cweId) : id(cweId) {}
    unsigned short id;
};

extern const CWE CWE119; // Improper Restriction of Operations within the Bounds of a Memory Buffer
extern const CWE CWE128; // Wrap-around Error
extern const CWE CWE131; // Incorrect Calculation of Buffer Size
extern const CWE CWE170; // Improper Null Termination
extern const CWE CWE190; // Integer Overflow or Wraparound
extern const CWE CWE195; // Signed to Unsigned Conversion Error
extern const CWE CWE197; // Numeric Truncation Error
extern const CWE CWE252; // Unchecked Return Value
extern const CWE CWE362; // Concurrent Execution using Shared Resource with Improper Synchronization ('Race Condition')
extern const CWE CWE369; // Divide By Zero
extern const CWE CWE398; // Indicator of Poor Code Quality
extern const CWE CWE401; // Improper Release of Memory Before Removing Last Reference ('Memory Leak')
extern const CWE CWE404; // Improper Resource Shutdown or Release
extern const CWE CWE415;
extern const CWE CWE467; // Use of sizeof() on a Pointer Type
extern const CWE CWE475; // Undefined Behavior for Input to API
extern const CWE CWE477; // Use of Obsolete Functions
extern const CWE CWE480; // Use of Incorrect Operator
extern const CWE CWE561; // Dead Code
extern const CWE CWE562; // Return of Stack Variable Address
extern const CWE CWE563; // Assignment to Variable without Use ('Unused Variable')
extern const CWE CWE570; // Expression is Always False
extern const CWE CWE571; // Expression is Always True
extern const CWE CWE587; // Assignment of a Fixed Address to a Pointer
extern const CWE CWE590; // Free of Memory not on the Heap
extern const CWE CWE595; // Comparison of Object References Instead of Object Contents
extern const CWE CWE597; // Use of Wrong Operator in String Comparison
extern const CWE CWE628; // Function Call with Incorrectly Specified Arguments
extern const CWE CWE664; // Improper Control of a Resource Through its Lifetime
extern const CWE CWE665; // Improper Initialization
extern const CWE CWE667; // Improper Locking
extern const CWE CWE672; // Operation on a Resource after Expiration or Release
extern const CWE CWE682; // Incorrect Calculation
extern const CWE CWE683; // Function Call With Incorrect Order of Arguments
extern const CWE CWE685; // Function Call With Incorrect Number of Arguments
extern const CWE CWE686; // Function Call With Incorrect Argument Type
extern const CWE CWE687; // Function Call With Incorrectly Specified Argument Value
extern const CWE CWE688; // Function Call With Incorrect Variable or Reference as Argument
extern const CWE CWE703; // Improper Check or Handling of Exceptional Conditions
extern const CWE CWE704; // Incorrect Type Conversion or Cast
extern const CWE CWE758; // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
extern const CWE CWE762; // Mismatched Memory Management Routines
extern const CWE CWE768; // Incorrect Short Circuit Evaluation
extern const CWE CWE771; // Missing Reference to Active Allocated Resource
extern const CWE CWE772; // Missing Release of Resource after Effective Lifetime
extern const CWE CWE783; // Operator Precedence Logic Error
extern const CWE CWE786; // Access of Memory Location Before Start of Buffer
extern const CWE CWE788; // Access of Memory Location After End of Buffer
extern const CWE CWE825; // Expired Pointer Dereference
extern const CWE CWE833; // Deadlock
extern const CWE CWE834; // Excessive Iteration
extern const CWE CWE910; // Use of Expired File Descriptor

extern const CWE CWE_ARGUMENT_SIZE;          // Indicator of Poor Code Quality
extern const CWE CWE_ARRAY_INDEX_THEN_CHECK; // Indicator of Poor Code Quality
extern const CWE CWE_BUFFER_OVERRUN;         // Access of Memory Location After End of Buffer
extern const CWE CWE_BUFFER_UNDERRUN;        // Access of Memory Location Before Start of Buffer
extern const CWE CWE_INCORRECT_CALCULATION;
extern const CWE CWE_NULL_POINTER_DEREFERENCE;
extern const CWE CWE_ONE_DEFINITION_RULE;
extern const CWE CWE_POINTER_ARITHMETIC_OVERFLOW; // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
extern const CWE CWE_USE_OF_UNINITIALIZED_VARIABLE;
extern const CWE uncheckedErrorConditionCWE;


using ErrorPathItem = std::pair<const Token *, std::string>;
using ErrorPath = std::list<ErrorPathItem>;

/// @}
//---------------------------------------------------------------------------
#endif // errortypesH
