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

#if defined(__CYGWIN__)
#define _BSD_SOURCE // required to have popen() and pclose()
#endif

#include "cppcheckexecutor.h"

#include "analyzerinfo.h"
#include "checkersreport.h"
#include "cmdlinelogger.h"
#include "cmdlineparser.h"
#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "json.h"
#include "settings.h"
#include "singleexecutor.h"
#include "suppressions.h"
#include "utils.h"

#if defined(HAS_THREADING_MODEL_THREAD)
#include "threadexecutor.h"
#endif
#if defined(HAS_THREADING_MODEL_FORK)
#include "processexecutor.h"
#endif

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef USE_UNIX_SIGNAL_HANDLING
#include "signalhandler.h"
#endif

#ifdef USE_WINDOWS_SEH
#include "sehwrapper.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(WIN32) && !defined(__MINGW32__)
#include <sys/wait.h> // WIFEXITED and friends
#endif

namespace {
    class SarifReport {
    public:
        void addFinding(ErrorMessage msg) {
            mFindings.push_back(std::move(msg));
        }

        std::string getRuleShortDescription(const ErrorMessage& finding) const {
            return getRuleDescription(finding.id, false, finding);
        }

        std::string getRuleFullDescription(const ErrorMessage& finding) const {
            return getRuleDescription(finding.id, true, finding);
        }

        picojson::array serializeRules() const {
            picojson::array ret;
            std::set<std::string> ruleIds;
            for (const auto& finding : mFindings) {
                // github only supports findings with locations
                if (finding.callStack.empty())
                    continue;
                if (ruleIds.insert(finding.id).second) {
                    picojson::object rule;
                    rule["id"] = picojson::value(finding.id);
                    // rule.name
                    rule["name"] = picojson::value(getRuleShortDescription(finding));
                    // rule.shortDescription.text
                    picojson::object shortDescription;
                    shortDescription["text"] = picojson::value(getRuleShortDescription(finding));
                    rule["shortDescription"] = picojson::value(shortDescription);
                    // rule.fullDescription.text
                    picojson::object fullDescription;
                    fullDescription["text"] = picojson::value(getRuleFullDescription(finding));
                    rule["fullDescription"] = picojson::value(fullDescription);
                    // rule.help.text
                    picojson::object help;
                    help["text"] = picojson::value(getRuleFullDescription(finding));
                    rule["help"] = picojson::value(help);
                    // rule.properties.precision, rule.properties.problem.severity
                    picojson::object properties;
                    properties["precision"] = picojson::value(sarifPrecision(finding));
                    // Only set security-severity for findings that are actually security-related
                    if (isSecurityRelatedFinding(finding.id)) {
                        double securitySeverity = 0;
                        if (ErrorLogger::isCriticalErrorId(finding.id)) {
                            securitySeverity = 9.9; // critical = 9.0+
                        }
                        else if (finding.severity == Severity::error) {
                            securitySeverity = 8.5; // high = 7.0 to 8.9
                        }
                        else if (finding.severity == Severity::warning) {
                            securitySeverity = 5.5; // medium = 4.0 to 6.9
                        }
                        else if (finding.severity == Severity::performance ||
                                 finding.severity == Severity::portability || 
                                 finding.severity == Severity::style) {
                            securitySeverity = 2.0; // low = 0.1 to 3.9
                        }
                        if (securitySeverity > 0.0)
                        {
                            properties["security-severity"] = picojson::value(std::to_string(securitySeverity));
                            const picojson::array tags{picojson::value("security")};
                            properties["tags"] = picojson::value(tags);
                        }
                    }
                    // Set problem.severity for use with github
                    std::string problemSeverity;
                    if (ErrorLogger::isCriticalErrorId(finding.id) || finding.severity == Severity::error)
                        problemSeverity = "error";
                    else if (finding.severity == Severity::warning) {
                        problemSeverity = "warning";
                    }
                    else {
                        problemSeverity = "note"; // style, information, performance, portability
                    }
                    properties["problem.severity"] = picojson::value(problemSeverity);
                    rule["properties"] = picojson::value(properties);
                    // rule.defaultConfiguration.level
                    picojson::object defaultConfiguration;
                    defaultConfiguration["level"] = picojson::value(problemSeverity);
                    rule["defaultConfiguration"] = picojson::value(defaultConfiguration);

                    ret.emplace_back(rule);
                }
            }
            return ret;
        }

        static picojson::array serializeLocations(const ErrorMessage& finding) {
            picojson::array ret;
            for (const auto& location : finding.callStack) {
                picojson::object physicalLocation;
                picojson::object artifactLocation;
                artifactLocation["uri"] = picojson::value(location.getfile(false));
                physicalLocation["artifactLocation"] = picojson::value(artifactLocation);
                picojson::object region;
                region["startLine"] = picojson::value(static_cast<int64_t>(location.line < 1 ? 1 : location.line));
                region["startColumn"] = picojson::value(static_cast<int64_t>(location.column < 1 ? 1 : location.column));
                region["endLine"] = region["startLine"];
                region["endColumn"] = region["startColumn"];
                physicalLocation["region"] = picojson::value(region);
                picojson::object loc;
                loc["physicalLocation"] = picojson::value(physicalLocation);
                ret.emplace_back(loc);
            }
            return ret;
        }

        picojson::array serializeResults() const {
            picojson::array results;
            for (const auto& finding : mFindings) {
                // github only supports findings with locations
                if (finding.callStack.empty())
                    continue;
                picojson::object res;
                res["level"] = picojson::value(sarifSeverity(finding));
                res["locations"] = picojson::value(serializeLocations(finding));
                picojson::object message;
                message["text"] = picojson::value(finding.shortMessage());
                res["message"] = picojson::value(message);
                res["ruleId"] = picojson::value(finding.id);
                results.emplace_back(res);
            }
            return results;
        }

        picojson::value serializeRuns(const std::string& productName, const std::string& version) const {
            picojson::object driver;
            driver["name"] = picojson::value(productName);
            driver["semanticVersion"] = picojson::value(version);
            driver["informationUri"] = picojson::value("https://cppcheck.sourceforge.io");
            driver["rules"] = picojson::value(serializeRules());
            picojson::object tool;
            tool["driver"] = picojson::value(driver);
            picojson::object run;
            run["tool"] = picojson::value(tool);
            run["results"] = picojson::value(serializeResults());
            picojson::array runs{picojson::value(run)};
            return picojson::value(runs);
        }

        std::string serialize(std::string productName) const {
            const auto nameAndVersion = Settings::getNameAndVersion(productName);
            productName = nameAndVersion.first.empty() ? "Cppcheck" : nameAndVersion.first;
            std::string version = nameAndVersion.first.empty() ? CppCheck::version() : nameAndVersion.second;
            if (version.find(' ') != std::string::npos)
                version.erase(version.find(' '), std::string::npos);

            picojson::object doc;
            doc["version"] = picojson::value("2.1.0");
            doc["$schema"] = picojson::value("https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/schemas/sarif-schema-2.1.0.json");
            doc["runs"] = serializeRuns(productName, version);

            return picojson::value(doc).serialize(true);
        }
    private:
        // Following the pattern of existing mapping functions in errorlogger.cpp
        std::string getRuleDescription(const std::string& ruleId, bool fullDescription, const ErrorMessage& finding) const {
            // Structure similar to IdMapping in checkers.h but for descriptions
            struct RuleDescription {
                const char* ruleId;
                const char* shortDesc;
                const char* fullDesc;
            };

            // Mapping of Cppcheck rule IDs to descriptions
            static const RuleDescription ruleDescriptions[] = {
                {"nullPointer", "Null pointer dereference", 
                 "Detects when code dereferences a pointer that may be null, which can lead to undefined behavior or program crashes."},
                {"uninitvar", "Uninitialized variable", 
                 "Detects usage of variables that have not been initialized, which can lead to unpredictable behavior."},
                {"memleakOnRealloc", "Memory leak on realloc", 
                 "Detects memory leaks that can occur when realloc() fails and the original memory is not freed."},
                {"memleak", "Memory leak", 
                 "Detects memory allocations that are not properly freed, leading to memory leaks."},
                {"resourceLeak", "Resource leak", 
                 "Detects when system resources (files, handles, etc.) are not properly closed or released."},
                {"deallocret", "Deallocation of returned value", 
                 "Detects when a function returns a pointer to memory that has been deallocated."},
                {"deallocuse", "Use after free", 
                 "Detects when code continues to use memory after it has been freed (use-after-free)."},
                {"doubleFree", "Double free", 
                 "Detects when the same memory is freed multiple times, which leads to undefined behavior."},
                {"arrayIndexOutOfBounds", "Array index out of bounds", 
                 "Detects when array indices exceed the bounds of the array."},
                {"bufferAccessOutOfBounds", "Buffer access out of bounds", 
                 "Detects when buffer access operations exceed the allocated buffer size."},
                {"stringLiteralWrite", "Write to string literal", 
                 "Detects attempts to modify string literals, which is undefined behavior."},
                {"unusedVariable", "Unused variable", 
                 "Detects variables that are declared but never used in the code."},
                {"unreadVariable", "Variable assigned but never used", 
                 "Detects variables that are assigned values but the values are never read."},
                {"duplicateCondition", "Duplicate condition", 
                 "Detects when the same condition is checked multiple times in related code paths."},
                {"unreachableCode", "Unreachable code", 
                 "Detects code that can never be executed due to program flow."},
                {"invalidFunctionArg", "Invalid function argument", 
                 "Detects when function arguments are invalid or out of expected range."},
                {"syntaxError", "Syntax error", 
                 "Detects syntax errors in the source code."},
                {"cppcheckError", "Cppcheck internal error", 
                 "Internal error occurred during analysis."},
                {"unusedFunction", "Unused function", 
                 "Detects functions that are declared but never called in the code."},
                {"ignoredReturnValue", "Ignored return value", 
                 "Detects when the return value of a function is ignored when it should be checked."},
                {"passedByValue", "Parameter passed by value", 
                 "Detects when function parameters should be passed by const reference instead of by value for better performance."},
                {"derefInvalidIteratorRedundantCheck", "Redundant iterator check", 
                 "Detects redundant checks for invalid iterator dereferencing."},
                {"stlFindInsert", "Inefficient STL usage", 
                 "Detects when searching before insertion is unnecessary and suggests using more efficient STL methods like try_emplace."},
                {"uselessCallsSubstr", "Ineffective substr call", 
                 "Detects ineffective calls to substr() that can be replaced with more efficient methods like resize() or pop_back()."},
                {"uninitMemberVar", "Uninitialized member variable", 
                 "Detects member variables that are not initialized in the constructor."},
                {"stlIfStrFind", "Inefficient string find usage", 
                 "Detects inefficient usage of string::find() in conditions where string::starts_with() or other methods could be faster."},
                {"invalidScanfArgType_int", "Invalid scanf argument type", 
                 "Detects when scanf format specifiers don't match the argument types provided."},
                {"constStatement", "Const statement", 
                 "Detects statements that have no effect and could indicate a programming error."},
                {"variableScope", "Variable scope", 
                 "Detects when variable scope can be reduced to improve code clarity and performance."},
                {"postfixOperator", "Inefficient postfix operator", 
                 "Detects when prefix operators (++i) should be used instead of postfix (i++) for better performance."},
                {"useStlAlgorithm", "Use STL algorithm", 
                 "Suggests using STL algorithms instead of raw loops for better readability and performance."},
                {"redundantAssignment", "Redundant assignment", 
                 "Detects redundant assignments where a variable is assigned but the value is never used."},
                {"duplicateExpression", "Duplicate expression", 
                 "Detects identical expressions on both sides of operators which may indicate copy-paste errors."},
                {"oppositeInnerCondition", "Opposite inner condition", 
                 "Detects when an inner condition is always false due to an outer condition."},
                {"knownConditionTrueFalse", "Known condition", 
                 "Detects conditions that are always true or false due to previous code."},
                {"clarifyCondition", "Unclear condition", 
                 "Suggests adding parentheses to clarify operator precedence in conditions."},
                {"redundantCondition", "Redundant condition", 
                 "Detects redundant conditions that don't affect the program flow."},
                {"missingReturn", "Missing return statement", 
                 "Detects functions that should return a value but may not have a return statement."},
                {"invalidContainer", "Invalid container usage", 
                 "Detects invalid usage of STL containers that could lead to undefined behavior."},
                {"containerOutOfBounds", "Container out of bounds", 
                 "Detects when container access operations exceed the container size."},
                {"danglingPointer", "Dangling pointer", 
                 "Detects pointers that reference memory that has been deallocated."},
                {"memsetClass", "Memset on class", 
                 "Detects when memset is used on classes with constructors/destructors which is undefined behavior."},
                {"noExplicitConstructor", "Missing explicit constructor", 
                 "Suggests adding 'explicit' keyword to single-argument constructors to prevent implicit conversions."},
                {"noCopyConstructor", "Missing copy constructor", 
                 "Detects classes that should have a copy constructor but don't."},
                {"noOperatorEq", "Missing assignment operator", 
                 "Detects classes that should have an assignment operator but don't."},
                {"virtualDestructor", "Missing virtual destructor", 
                 "Detects base classes that should have virtual destructors."},
                {"useInitializationList", "Use initialization list", 
                 "Suggests using constructor initialization lists instead of assignment in constructor body."},
                {"cstyleCast", "C-style cast", 
                 "Suggests using C++ style casts instead of C-style casts for better type safety."},
                {"redundantCopy", "Redundant copy", 
                 "Detects unnecessary copying of objects that could be avoided."},
                {"returnTempReference", "Return temporary reference", 
                 "Detects when functions return references to temporary objects."},
                {"nullPointerArithmetic", "Null pointer arithmetic", 
                 "Detects arithmetic operations performed on null pointers."},
                {"nullPointerRedundantCheck", "Redundant null pointer check", 
                 "Detects redundant checks for null pointers that are already known to be null or non-null."},
                {"nullPointerDefaultArg", "Null pointer default argument", 
                 "Detects when null pointers are used as default arguments which may cause dereferencing issues."},
                {"nullPointerOutOfMemory", "Null pointer from memory allocation", 
                 "Detects when memory allocation functions return null due to out-of-memory conditions."},
                {"nullPointerOutOfResources", "Null pointer from resource exhaustion", 
                 "Detects when resource allocation functions return null due to resource exhaustion."},
                {"leakReturnValNotUsed", "Memory leak from unused return value", 
                 "Detects memory leaks when the return value of allocation functions is not used."},
                {"leakUnsafeArgAlloc", "Unsafe argument allocation leak", 
                 "Detects memory leaks in unsafe argument allocation patterns."},
                {"mismatchAllocDealloc", "Mismatched allocation/deallocation", 
                 "Detects when memory allocated with one function is freed with an incompatible function."},
                {"autovarInvalidDeallocation", "Invalid automatic variable deallocation", 
                 "Detects attempts to deallocate automatic (stack) variables."},
                {"arrayIndexOutOfBoundsCond", "Conditional array bounds violation", 
                 "Detects array index out of bounds conditions that may occur under certain circumstances."},
                {"pointerOutOfBounds", "Pointer out of bounds", 
                 "Detects when pointer arithmetic results in pointers outside valid memory ranges."},
                {"pointerOutOfBoundsCond", "Conditional pointer out of bounds", 
                 "Detects pointer out of bounds conditions that may occur under certain circumstances."},
                {"negativeIndex", "Negative array index", 
                 "Detects when negative values are used as array indices."},
                {"objectIndex", "Invalid object indexing", 
                 "Detects invalid indexing operations on objects."},
                {"argumentSize", "Invalid argument size", 
                 "Detects when function arguments have invalid sizes that could cause buffer overflows."},
                {"bufferOverflow", "Buffer overflow", 
                 "Detects potential buffer overflow conditions."},
                {"pointerArithmetic", "Unsafe pointer arithmetic", 
                 "Detects potentially unsafe pointer arithmetic operations."},
                {"uninitdata", "Uninitialized data", 
                 "Detects usage of uninitialized data structures."},
                {"uninitStructMember", "Uninitialized struct member", 
                 "Detects uninitialized members in struct/class instances."},
                {"danglingLifetime", "Dangling lifetime", 
                 "Detects when object lifetimes end before their usage is complete."},
                {"returnDanglingLifetime", "Return dangling lifetime", 
                 "Detects when functions return references or pointers to objects with ended lifetimes."},
                {"danglingReference", "Dangling reference", 
                 "Detects references that point to objects that no longer exist."},
                {"danglingTempReference", "Dangling temporary reference", 
                 "Detects references to temporary objects that have been destroyed."},
                {"danglingTemporaryLifetime", "Dangling temporary lifetime", 
                 "Detects when temporary object lifetimes end before their usage is complete."},
                {"invalidLifetime", "Invalid object lifetime", 
                 "Detects invalid object lifetime usage patterns."},
                {"useClosedFile", "Use of closed file", 
                 "Detects operations on file handles that have been closed."},
                {"wrongPrintfScanfArgNum", "Wrong printf/scanf argument count", 
                 "Detects when the number of printf/scanf arguments doesn't match the format string."},
                {"invalidScanfFormatWidth", "Invalid scanf format width", 
                 "Detects invalid width specifiers in scanf format strings."},
                {"invalidscanf", "Invalid scanf usage", 
                 "Detects invalid usage patterns of scanf functions."},
                {"integerOverflow", "Integer overflow", 
                 "Detects potential integer overflow conditions."},
                {"floatConversionOverflow", "Float conversion overflow", 
                 "Detects when floating-point conversions may cause overflow."},
                {"negativeMemoryAllocationSize", "Negative memory allocation size", 
                 "Detects when negative values are used for memory allocation sizes."},
                {"IOWithoutPositioning", "I/O without positioning", 
                 "Detects file I/O operations that may have undefined behavior due to positioning issues."},
                {"incompatibleFileOpen", "Incompatible file open", 
                 "Detects when files are opened with incompatible modes or flags."},
                {"writeReadOnlyFile", "Write to read-only file", 
                 "Detects attempts to write to files opened in read-only mode."},
                {"zerodiv", "Division by zero", 
                 "Detects potential division by zero operations."},
                {"zerodivcond", "Conditional division by zero", 
                 "Detects division by zero conditions that may occur under certain circumstances."},
                {"invalidLengthModifierError", "Invalid length modifier", 
                 "Detects invalid length modifiers in format strings."},
                {"preprocessorErrorDirective", "Preprocessor error directive", 
                 "Detects preprocessor #error directives that indicate compilation issues."},
                // Add more mappings as needed
            };

            // Find the rule description
            for (const auto& desc : ruleDescriptions) {
                if (ruleId == desc.ruleId) {
                    return fullDescription ? desc.fullDesc : desc.shortDesc;
                }
            }

            // Fallback for unknown rules - use the actual error message content
            return fullDescription ? finding.verboseMessage() : finding.shortMessage();
        }

        static bool isSecurityRelatedFinding(const std::string& ruleId) {
            // Security-related findings that have actual security implications
            static const std::unordered_set<std::string> securityRelatedIds = {
                // Memory safety issues
                "nullPointer", "nullPointerArithmetic", "nullPointerRedundantCheck", "nullPointerDefaultArg",
                "nullPointerOutOfMemory", "nullPointerOutOfResources",
                "memleak", "memleakOnRealloc", "resourceLeak", "leakReturnValNotUsed", "leakUnsafeArgAlloc",
                "deallocret", "deallocuse", "doubleFree", "mismatchAllocDealloc", "autovarInvalidDeallocation",
                
                // Buffer overflow and array bounds issues
                "arrayIndexOutOfBounds", "arrayIndexOutOfBoundsCond", "bufferAccessOutOfBounds",
                "pointerOutOfBounds", "pointerOutOfBoundsCond", "negativeIndex", "objectIndex",
                "argumentSize", "stringLiteralWrite", "bufferOverflow", "pointerArithmetic",
                
                // Uninitialized data issues
                "uninitvar", "uninitdata", "uninitStructMember",
                
                // Use after free and lifetime issues
                "danglingLifetime", "returnDanglingLifetime", "danglingReference", "danglingTempReference",
                "danglingTemporaryLifetime", "invalidLifetime", "useClosedFile",
                
                // Format string vulnerabilities
                "wrongPrintfScanfArgNum", "invalidScanfFormatWidth", "invalidscanf", "invalidFunctionArg",
                
                // Integer overflow and underflow
                "integerOverflow", "floatConversionOverflow", "negativeMemoryAllocationSize",
                
                // File and resource handling
                "IOWithoutPositioning", "incompatibleFileOpen", "writeReadOnlyFile",
                
                // Other security-relevant issues
                "zerodiv", "zerodivcond", "invalidLengthModifierError", "preprocessorErrorDirective"
            };
            
            return securityRelatedIds.find(ruleId) != securityRelatedIds.end();
        }

        static std::string sarifSeverity(const ErrorMessage& errmsg) {
            if (ErrorLogger::isCriticalErrorId(errmsg.id))
                return "error";
            switch (errmsg.severity) {
            case Severity::error:
            case Severity::warning:
            case Severity::style:
            case Severity::portability:
            case Severity::performance:
                return "warning";
            case Severity::information:
            case Severity::internal:
            case Severity::debug:
            case Severity::none:
                return "note";
            }
            return "note";
        }

        static std::string sarifPrecision(const ErrorMessage& errmsg) {
            if (errmsg.certainty == Certainty::inconclusive)
                return "medium";
            return "high";
        }

        std::vector<ErrorMessage> mFindings;
    };

    class CmdLineLoggerStd : public CmdLineLogger
    {
    public:
        CmdLineLoggerStd() = default;

        void printMessage(const std::string &message) override
        {
            printRaw("cppcheck: " + message);
        }

        void printError(const std::string &message) override
        {
            printMessage("error: " + message);
        }

        void printRaw(const std::string &message) override
        {
            std::cout << message << std::endl; // TODO: should not append newline
        }
    };

    class StdLogger : public ErrorLogger
    {
    public:
        explicit StdLogger(const Settings& settings)
            : mSettings(settings)
            , mGuidelineMapping(createGuidelineMapping(settings.reportType))
        {
            if (!mSettings.outputFile.empty()) {
                mErrorOutput = new std::ofstream(settings.outputFile);
            }
            if (!mSettings.buildDir.empty()) {
                mCheckersFile = Path::join(settings.buildDir, "checkers.txt");
            }
        }

        ~StdLogger() override {
            if (mSettings.outputFormat == Settings::OutputFormat::sarif) {
                reportErr(mSarifReport.serialize(mSettings.cppcheckCfgProductName));
            }
            delete mErrorOutput;
        }

        StdLogger(const StdLogger&) = delete;
        StdLogger& operator=(const SingleExecutor &) = delete;

        void resetLatestProgressOutputTime() {
            mLatestProgressOutputTime = std::time(nullptr);
        }

        /**
         * Helper function to print out errors. Appends a line change.
         * @param errmsg String printed to error stream
         */
        void reportErr(const std::string &errmsg);

        void reportMetric(const std::string &metric) override
        {
            mFileMetrics.push_back(metric);
        }

        void reportMetrics()
        {
            if (!mFileMetrics.empty()) {
                auto &out = mErrorOutput ? *mErrorOutput : std::cerr;
                out << "    <metrics>" << std::endl;
                for (const auto &metric : mFileMetrics) {
                    out << "        " << metric << std::endl;
                }
                out << "    </metrics>" << std::endl;
            }
        }

        /**
         * @brief Write the checkers report
         */
        void writeCheckersReport(const Suppressions& supprs);

        bool hasCriticalErrors() const {
            return !mCriticalErrors.empty();
        }

        const std::string& getCtuInfo() const {
            return mCtuInfo;
        }

        void readActiveCheckers() {
            if (mCheckersFile.empty())
                return;

            std::ifstream fin(mCheckersFile);
            if (fin.is_open())
            {
                std::set<std::string> activeCheckers;
                std::string line;
                // cppcheck-suppress accessMoved - FP
                while (std::getline(fin, line))
                {
                    // cppcheck-suppress accessMoved - FP
                    activeCheckers.emplace(std::move(line));
                }
                mActiveCheckers = std::move(activeCheckers);
            }
        }

    private:
        /**
         * Information about progress is directed here. This should be
         * called by the CppCheck class only.
         *
         * @param outmsg Progress message e.g. "Checking main.cpp..."
         */
        void reportOut(const std::string &outmsg, Color c = Color::Reset) override;

        /** xml output of errors */
        void reportErr(const ErrorMessage &msg) override;

        void reportProgress(const std::string &filename, const char stage[], std::size_t value) override;

        /**
         * Reference to current settings; set while check() is running for reportError().
         */
        const Settings& mSettings;

        /**
         * Used to filter out duplicate error messages.
         */
        // TODO: store hashes instead of the full messages
        std::unordered_set<std::string> mShownErrors;

        /**
         * Report progress time
         */
        std::time_t mLatestProgressOutputTime{};

        /**
         * Error output
         */
        std::ofstream* mErrorOutput{};

        /**
         * Checkers that has been executed
         */
        std::set<std::string> mActiveCheckers;

        /**
         * List of critical errors
         */
        std::string mCriticalErrors;

        /**
         * CTU information
         */
        std::string mCtuInfo;

        /**
         * SARIF report generator
         */
        SarifReport mSarifReport;

        /**
         * Coding standard guideline mapping
         */
        std::map<std::string, std::string> mGuidelineMapping;

        /**
         * File metrics
         */
        std::vector<std::string> mFileMetrics;

        /**
         * The file the cached active checkers are stored in
         */
        std::string mCheckersFile;
    };
}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    Settings settings;
    CmdLineLoggerStd logger;
    Suppressions supprs;
    CmdLineParser parser(logger, settings, supprs);
    if (!parser.fillSettingsFromArgs(argc, argv)) {
        return EXIT_FAILURE;
    }
    if (Settings::terminated()) {
        return EXIT_SUCCESS;
    }

    settings.loadSummaries();

    mFiles = parser.getFiles();
    mFileSettings = parser.getFileSettings();

    const int ret = check_wrapper(settings, supprs);

    return ret;
}

int CppCheckExecutor::check_wrapper(const Settings& settings, Suppressions& supprs)
{
#ifdef USE_WINDOWS_SEH
    if (settings.exceptionHandling) {
        CALL_WITH_SEH_WRAPPER(check_internal(settings, supprs));
    }
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    if (settings.exceptionHandling)
        register_signal_handler(settings.exceptionOutput);
#endif
    return check_internal(settings, supprs);
}

bool CppCheckExecutor::reportSuppressions(const Settings &settings, const SuppressionList& suppressions, bool unusedFunctionCheckEnabled, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, ErrorLogger& errorLogger) {
    const auto& suppr = suppressions.getSuppressions();
    if (std::any_of(suppr.begin(), suppr.end(), [](const SuppressionList::Suppression& s) {
        return s.errorId == "unmatchedSuppression" && s.fileName.empty() && s.lineNumber == SuppressionList::Suppression::NO_LINE;
    }))
        return false;

    bool err = false;
    if (settings.useSingleJob()) {
        // the two inputs may only be used exclusively
        assert(!(!files.empty() && !fileSettings.empty()));

        for (auto i = files.cbegin(); i != files.cend(); ++i) {
            err |= SuppressionList::reportUnmatchedSuppressions(
                suppressions.getUnmatchedLocalSuppressions(*i, unusedFunctionCheckEnabled), errorLogger);
        }

        for (auto i = fileSettings.cbegin(); i != fileSettings.cend(); ++i) {
            err |= SuppressionList::reportUnmatchedSuppressions(
                suppressions.getUnmatchedLocalSuppressions(i->file, unusedFunctionCheckEnabled), errorLogger);
        }
    }
    if (settings.inlineSuppressions) {
        // report unmatched unusedFunction suppressions
        err |= SuppressionList::reportUnmatchedSuppressions(
            suppressions.getUnmatchedInlineSuppressions(unusedFunctionCheckEnabled), errorLogger);
    }

    err |= SuppressionList::reportUnmatchedSuppressions(suppressions.getUnmatchedGlobalSuppressions(unusedFunctionCheckEnabled), errorLogger);
    return err;
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(const Settings& settings, Suppressions& supprs) const
{
    StdLogger stdLogger(settings);

    if (settings.reportProgress >= 0)
        stdLogger.resetLatestProgressOutputTime();

    if (settings.outputFormat == Settings::OutputFormat::xml) {
        stdLogger.reportErr(ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName, settings.xml_version));
    }

    if (!settings.buildDir.empty()) {
        std::list<std::string> fileNames;
        for (auto i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            fileNames.emplace_back(i->path());
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.userDefines, mFileSettings);

        stdLogger.readActiveCheckers();
    }

    if (!settings.checkersReportFilename.empty())
        std::remove(settings.checkersReportFilename.c_str());

    CppCheck cppcheck(settings, supprs, stdLogger, true, executeCommand);

    unsigned int returnValue = 0;
    if (settings.useSingleJob()) {
        // Single process
        SingleExecutor executor(cppcheck, mFiles, mFileSettings, settings, supprs, stdLogger);
        returnValue = executor.check();
    } else {
#if defined(HAS_THREADING_MODEL_THREAD)
        if (settings.executor == Settings::ExecutorType::Thread) {
            ThreadExecutor executor(mFiles, mFileSettings, settings, supprs, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
#if defined(HAS_THREADING_MODEL_FORK)
        if (settings.executor == Settings::ExecutorType::Process) {
            ProcessExecutor executor(mFiles, mFileSettings, settings, supprs, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
    }

    returnValue |= cppcheck.analyseWholeProgram(settings.buildDir, mFiles, mFileSettings, stdLogger.getCtuInfo());

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportSuppressions(settings, supprs.nomsg, settings.checks.isEnabled(Checks::unusedFunction), mFiles, mFileSettings, stdLogger);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError("",0U);
    }

    stdLogger.writeCheckersReport(supprs);

    if (settings.outputFormat == Settings::OutputFormat::xml) {
        if (settings.xml_version == 3)
            stdLogger.reportMetrics();
        stdLogger.reportErr(ErrorMessage::getXMLFooter(settings.xml_version));
    }

    if (settings.safety && stdLogger.hasCriticalErrors())
        return EXIT_FAILURE;

    if (returnValue)
        return settings.exitCode;
    return EXIT_SUCCESS;
}

void StdLogger::writeCheckersReport(const Suppressions& supprs)
{
    if (!mCheckersFile.empty())
    {
        std::ofstream fout(mCheckersFile);
        for (const auto& c : mActiveCheckers)
        {
            fout << c << std::endl;
        }
    }

    const bool summary = mSettings.safety || mSettings.severity.isEnabled(Severity::information);
    const bool xmlReport = mSettings.outputFormat == Settings::OutputFormat::xml && mSettings.xml_version == 3;
    const bool textReport = !mSettings.checkersReportFilename.empty();

    if (!summary && !xmlReport && !textReport)
        return;

    CheckersReport checkersReport(mSettings, mActiveCheckers);

    const auto& suppressions = supprs.nomsg.getSuppressions();
    const bool summarySuppressed = std::any_of(suppressions.cbegin(), suppressions.cend(), [](const SuppressionList::Suppression& s) {
        return s.errorId == "checkersReport";
    });

    if (summary && !summarySuppressed) {
        ErrorMessage msg;
        msg.severity = Severity::information;
        msg.id = "checkersReport";

        const int activeCheckers = checkersReport.getActiveCheckersCount();
        const int totalCheckers = checkersReport.getAllCheckersCount();

        std::string what;
        if (mCriticalErrors.empty())
            what = std::to_string(activeCheckers) + "/" + std::to_string(totalCheckers);
        else
            what = "There was critical errors";
        if (!xmlReport && !textReport)
            what += " (use --checkers-report=<filename> to see details)";
        msg.setmsg("Active checkers: " + what);

        reportErr(msg);
    }

    if (textReport) {
        std::ofstream fout(mSettings.checkersReportFilename);
        if (fout.is_open())
            fout << checkersReport.getReport(mCriticalErrors);
    }

    if (xmlReport) {
        reportErr("    </errors>\n");
        if (mSettings.safety)
            reportErr("    <safety/>\n");
        if (mSettings.inlineSuppressions)
            reportErr("    <inline-suppr/>\n");
        if (!suppressions.empty()) {
            std::ostringstream suppressionsXml;
            supprs.nomsg.dump(suppressionsXml);
            reportErr(suppressionsXml.str());
        }
        reportErr(checkersReport.getXmlReport(mCriticalErrors));
    }
}

#ifdef _WIN32
// fix trac ticket #439 'Cppcheck reports wrong filename for filenames containing 8-bit ASCII'
static inline std::string ansiToOEM(const std::string &msg, bool doConvert)
{
    if (doConvert) {
        const unsigned msglength = msg.length();
        // convert ANSI strings to OEM strings in two steps
        std::vector<WCHAR> wcContainer(msglength);
        std::string result(msglength, '\0');

        // ansi code page characters to wide characters
        MultiByteToWideChar(CP_ACP, 0, msg.data(), msglength, wcContainer.data(), msglength);
        // wide characters to oem codepage characters
        WideCharToMultiByte(CP_OEMCP, 0, wcContainer.data(), msglength, &result[0], msglength, nullptr, nullptr);

        return result; // hope for return value optimization
    }
    return msg;
}
#else
// no performance regression on non-windows systems
#define ansiToOEM(msg, doConvert) (msg)
#endif

void StdLogger::reportErr(const std::string &errmsg)
{
    if (mErrorOutput)
        *mErrorOutput << errmsg << std::endl;
    else {
        std::cerr << ansiToOEM(errmsg, mSettings.outputFormat != Settings::OutputFormat::xml) << std::endl;
    }
}

void StdLogger::reportOut(const std::string &outmsg, Color c)
{
    if (c == Color::Reset)
        std::cout << ansiToOEM(outmsg, true) << std::endl;
    else
        std::cout << c << ansiToOEM(outmsg, true) << Color::Reset << std::endl;
}

// TODO: remove filename parameter?
void StdLogger::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    (void)filename;

    if (!mLatestProgressOutputTime)
        return;

    // Report progress messages every x seconds
    const std::time_t currentTime = std::time(nullptr);
    if (currentTime >= (mLatestProgressOutputTime + mSettings.reportProgress))
    {
        mLatestProgressOutputTime = currentTime;

        // format a progress message
        std::ostringstream ostr;
        ostr << "progress: "
             << stage
             << ' ' << value << '%';

        // Report progress message
        reportOut(ostr.str());
    }
}

void StdLogger::reportErr(const ErrorMessage &msg)
{
    if (msg.severity == Severity::internal && (msg.id == "logChecker" || endsWith(msg.id, "-logChecker"))) {
        const std::string& checker = msg.shortMessage();
        mActiveCheckers.emplace(checker);
        return;
    }

    if (msg.severity == Severity::internal && msg.id == "ctuinfo") {
        mCtuInfo += msg.shortMessage() + "\n";
        return;
    }

    if (ErrorLogger::isCriticalErrorId(msg.id) && mCriticalErrors.find(msg.id) == std::string::npos) {
        if (!mCriticalErrors.empty())
            mCriticalErrors += ", ";
        mCriticalErrors += msg.id;
        if (msg.severity == Severity::internal)
            mCriticalErrors += " (suppressed)";
    }

    if (msg.severity == Severity::internal)
        return;

    ErrorMessage msgCopy = msg;
    msgCopy.guideline = getGuideline(msgCopy.id, mSettings.reportType,
                                     mGuidelineMapping, msgCopy.severity);
    msgCopy.classification = getClassification(msgCopy.guideline, mSettings.reportType);

    // TODO: there should be no need for verbose and default messages here
    const std::string msgStr = msgCopy.toString(mSettings.verbose, mSettings.templateFormat, mSettings.templateLocation);

    // Alert only about unique errors
    if (!mSettings.emitDuplicates && !mShownErrors.insert(msgStr).second)
        return;

    if (mSettings.outputFormat == Settings::OutputFormat::sarif)
        mSarifReport.addFinding(std::move(msgCopy));
    else if (mSettings.outputFormat == Settings::OutputFormat::xml)
        reportErr(msgCopy.toXML());
    else
        reportErr(msgStr);
}

/**
 * Execute a shell command and read the output from it. Returns true if command terminated successfully.
 */
// cppcheck-suppress passedByValueCallback - used as callback so we need to preserve the signature
// NOLINTNEXTLINE(performance-unnecessary-value-param) - used as callback so we need to preserve the signature
int CppCheckExecutor::executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output_)
{
    output_.clear();

#ifdef _WIN32
    // Extra quoutes are needed in windows if filename has space
    if (exe.find(" ") != std::string::npos)
        exe = "\"" + exe + "\"";
#endif

    std::string joinedArgs;
    for (const std::string &arg : args) {
        if (!joinedArgs.empty())
            joinedArgs += " ";
        if (arg.find(' ') != std::string::npos)
            joinedArgs += '"' + arg + '"';
        else
            joinedArgs += arg;
    }

    std::string cmd = exe + " " + joinedArgs + " " + redirect;

#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
    FILE* p = _popen(cmd.c_str(), "r");
#else
    FILE *p = popen(cmd.c_str(), "r");
#endif
    //std::cout << "invoking command '" << cmd << "'" << std::endl;
    if (!p) {
        // TODO: how to provide to caller?
        //const int err = errno;
        //std::cout << "popen() errno " << std::to_string(err) << std::endl;
        return -1;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), p) != nullptr)
        output_ += buffer;

#ifdef _WIN32
    const int res = _pclose(p);
#elif defined(__APPLE__) && defined(__MACH__)
    // the W* macros cast to int* on macOS
    int res = pclose(p);
#else
    const int res = pclose(p);
#endif
    if (res == -1) { // error occurred
        // TODO: how to provide to caller?
        //const int err = errno;
        //std::cout << "pclose() errno " << std::to_string(err) << std::endl;
        return res;
    }
#if !defined(WIN32) && !defined(__MINGW32__)
    if (WIFEXITED(res)) {
        return WEXITSTATUS(res);
    }
    if (WIFSIGNALED(res)) {
        return WTERMSIG(res);
    }
#endif
    return res;
}

