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

#include "fixture.h"
#include "helpers.h"
#include "json.h"

#include <cstddef>
#include <sstream>
#include <string>
#include <fstream>

class TestSarif : public TestFixture
{
public:
    TestSarif() : TestFixture("TestSarif")
    {}

private:
    // Shared test code with various error types
    const std::string testCode = R"(
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <memory>

class TestClass {
public:
    TestClass() : value(0) {}
    ~TestClass() { delete ptr; }
    
    void setValue(int v) { value = v; }
    int getValue() const { return value; }
    
private:
    int value;
    int* ptr = nullptr;
};

void testSecurityViolations() {
    // Null pointer dereference
    int* ptr = nullptr;
    *ptr = 5;
    
    // Array bounds violation  
    int array[5];
    array[10] = 1;
    
    // Memory leak
    int* mem = (int*)malloc(sizeof(int) * 10);
    // forgot to free mem
    
    // Uninitialized variable
    int x;
    printf("%d", x);
    
    // Double free
    int* p = (int*)malloc(sizeof(int));
    free(p);
    free(p);
    
    // Buffer overflow with strcpy
    char buffer[10];
    char source[20] = "This is too long";
    strcpy(buffer, source);
    
    // Integer overflow
    int large = 2147483647;
    int overflow = large + 1;
    
    // Use after free
    int* freed = (int*)malloc(sizeof(int));
    free(freed);
    *freed = 42;
    
    // Format string vulnerability
    char userInput[] = "%s%s%s";
    printf(userInput);
}

void testStyleAndPortabilityIssues() {
    // Redundant assignment
    int redundant = 5;
    redundant = redundant;
    
    // Unused variable
    int unused = 42;
    
    // Variable scope reduction
    int i;
    for (i = 0; i < 10; i++) {
        // i could be declared in for loop
    }
    
    // Const correctness
    TestClass obj;
    obj.getValue(); // should be const
    
    // C-style cast (prefer static_cast)
    double d = 3.14;
    int cStyleCast = (int)d;
    
    // Increment in condition
    int counter = 0;
    while (++counter < 10) {
        // prefer pre-increment outside condition
    }
    
    // Comparison of bool with integer
    bool flag = true;
    if (flag == 1) {
        // prefer if(flag)
    }
    
    // Assignment in condition
    int result;
    if (result = getValue()) {
        // should be ==
    }
    
    // Inefficient string concatenation
    std::string str = "Hello";
    str = str + " World";
    
    // Suspicious semicolon
    for (int j = 0; j < 5; j++);
    {
        printf("This block runs only once\n");
    }
}

int getValue() {
    return 42;
}

void testErrorHandling() {
    // File operations without error checking
    FILE* file = fopen("nonexistent.txt", "r");
    fgetc(file); // potential null pointer
    
    // Division by zero
    int zero = 0;
    int result = 10 / zero;
    
    // Missing return statement
    // (This function should return int but doesn't always)
}

void testSTLIssues() {
    std::vector<int> vec;
    
    // Out of bounds access
    vec[0] = 1; // vector is empty
    
    // Iterator invalidation
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (*it == 0) {
            vec.push_back(1); // invalidates iterator
        }
    }
    
    // Inefficient vector usage
    std::vector<int> v(1000);
    for (int i = 0; i < 1000; i++) {
        v.push_back(i); // should use resize or reserve
    }
}

void testFunctionIssues() {
    // Too many arguments (this will trigger complexFunction)
    complexFunction(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    
    // Recursive function without base case
    recursiveWithoutBase(5);
}

void complexFunction(int a, int b, int c, int d, int e, int f, 
                    int g, int h, int i, int j, int k, int l) {
    // Function with too many parameters
    printf("%d\n", a + b + c + d + e + f + g + h + i + j + k + l);
}

void recursiveWithoutBase(int n) {
    printf("%d\n", n);
    recursiveWithoutBase(n - 1); // infinite recursion
}

// Unused function
static void unusedFunction() {
    // This function is never called
}

// Function with unused parameter
void functionWithUnusedParam(int used, int unused) {
    printf("%d\n", used);
}

// Missing override specifier
class Base {
public:
    virtual void virtualFunc() {}
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void virtualFunc() {} // should have override
};

int main() {
    testSecurityViolations();
    testStyleAndPortabilityIssues();
    testErrorHandling();
    testSTLIssues();
    testFunctionIssues();
    
    functionWithUnusedParam(1, 2);
    
    return 0;
}
    )";

    void run() override
    {
        TEST_CASE(sarifBasicStructure);
        TEST_CASE(sarifGeneratedOutput);
        TEST_CASE(sarifRuleDescriptions);
        TEST_CASE(sarifSecuritySeverity);
        TEST_CASE(sarifLocationInfo);
        TEST_CASE(sarifGenericDescriptions);
        TEST_CASE(sarifCweTags);
        TEST_CASE(sarifRuleCoverage);
        TEST_CASE(sarifSeverityLevels);
        TEST_CASE(sarifNonSecurityRules);
    }

    // Helper to run cppcheck and capture SARIF output
    std::string runCppcheckSarif(const std::string& code)
    {
        // Create temporary file
        const std::string filename = "sarif_test_temp.cpp";
        std::ofstream file(filename);
        file << code;
        file.close();

        // Run cppcheck with SARIF output on the file
        std::string cmd = "./cppcheck --output-format=sarif --enable=all " + filename + " 2>&1";
        FILE* pipe      = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            std::remove(filename.c_str());
            return "";
        }

        std::string result;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);

        // Clean up
        std::remove(filename.c_str());

        // Extract just the JSON part (skip "Checking..." line)
        size_t jsonStart = result.find('{');
        if (jsonStart != std::string::npos)
        {
            return result.substr(jsonStart);
        }
        return "";
    }

    // Helper to parse and validate SARIF JSON
    bool validateSarifJson(const std::string& sarifOutput, std::string& errorMsg)
    {
        if (sarifOutput.empty())
        {
            errorMsg = "Empty SARIF output";
            return false;
        }

        picojson::value json;
        std::string parseError = picojson::parse(json, sarifOutput);
        if (!parseError.empty())
        {
            errorMsg = "JSON parse error: " + parseError;
            return false;
        }

        if (!json.is<picojson::object>())
        {
            errorMsg = "Root is not an object";
            return false;
        }

        const picojson::object& root = json.get<picojson::object>();

        // Check required SARIF fields
        if (root.find("version") == root.end() || !root.at("version").is<std::string>())
        {
            errorMsg = "Missing or invalid version field";
            return false;
        }

        if (root.find("$schema") == root.end() || !root.at("$schema").is<std::string>())
        {
            errorMsg = "Missing or invalid $schema field";
            return false;
        }

        if (root.find("runs") == root.end() || !root.at("runs").is<picojson::array>())
        {
            errorMsg = "Missing or invalid runs field";
            return false;
        }

        const picojson::array& runs = root.at("runs").get<picojson::array>();
        if (runs.empty())
        {
            errorMsg = "Empty runs array";
            return false;
        }

        return true;
    }

    void sarifBasicStructure()
    {
        // Create a simple test with null pointer dereference
        const std::string testCode = R"(
            int main() {
                int* p = nullptr;
                *p = 5; // null pointer dereference
                return 0;
            }
        )";

        const std::string sarif = runCppcheckSarif(testCode);

        std::string errorMsg;
        const bool isValid = validateSarifJson(sarif, errorMsg);
        if (!isValid)
        {
            // Print for debugging
            std::cout << "SARIF Output: " << sarif << std::endl;
            std::cout << "Error: " << errorMsg << std::endl;
        }
        ASSERT_EQUALS(true, isValid);

        // Parse and check specific SARIF structure
        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root = json.get<picojson::object>();

        ASSERT_EQUALS("2.1.0", root.at("version").get<std::string>());
        ASSERT(root.at("$schema").get<std::string>().find("sarif-schema") != std::string::npos);

        const picojson::array& runs = root.at("runs").get<picojson::array>();
        ASSERT_EQUALS(1, static_cast<int>(runs.size()));

        const picojson::object& run = runs[0].get<picojson::object>();
        ASSERT(run.find("tool") != run.end());
        ASSERT(run.find("results") != run.end());

        const picojson::object& tool = run.at("tool").get<picojson::object>();
        ASSERT(tool.find("driver") != tool.end());

        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        ASSERT_EQUALS("Cppcheck", driver.at("name").get<std::string>());
        ASSERT(driver.find("rules") != driver.end());
    }

    void sarifGeneratedOutput()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        std::string errorMsg;
        ASSERT_EQUALS(true, validateSarifJson(sarif, errorMsg));

        // Parse and check for different error types
        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::array& results = run.at("results").get<picojson::array>();

        ASSERT(results.size() > 0);

        // Check that we have different severity levels
        bool hasError = false;
        for (const auto& result : results)
        {
            const picojson::object& res = result.get<picojson::object>();
            const std::string level     = res.at("level").get<std::string>();
            if (level == "error")
                hasError = true;
        }

        ASSERT_EQUALS(true, hasError);
    }

    void sarifRuleDescriptions()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        ASSERT(rules.size() > 0);

        // Check rule structure
        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();

            ASSERT(r.find("id") != r.end());
            ASSERT(r.find("name") != r.end());
            ASSERT(r.find("shortDescription") != r.end());
            ASSERT(r.find("fullDescription") != r.end());
            ASSERT(r.find("help") != r.end());
            ASSERT(r.find("defaultConfiguration") != r.end());

            const std::string ruleId = r.at("id").get<std::string>();
            const std::string name   = r.at("name").get<std::string>();

            // Check that generic descriptions don't contain empty quotes
            ASSERT_EQUALS(std::string::npos, name.find("''"));

            const picojson::object& shortDesc = r.at("shortDescription").get<picojson::object>();
            const std::string shortText       = shortDesc.at("text").get<std::string>();
            ASSERT_EQUALS(std::string::npos, shortText.find("''"));
        }
    }

    void sarifSecuritySeverity()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Check that security-related rules have security-severity
        bool foundSecurityRule = false;
        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            const std::string ruleId  = r.at("id").get<std::string>();

            if (ruleId == "nullPointer" || ruleId == "arrayIndexOutOfBounds" || ruleId == "doubleFree" ||
                ruleId == "memleak" || ruleId == "uninitvar")
            {
                const picojson::object& props = r.at("properties").get<picojson::object>();
                if (props.find("security-severity") != props.end())
                {
                    foundSecurityRule        = true;
                    const std::string secSev = props.at("security-severity").get<std::string>();
                    ASSERT(std::stod(secSev) > 0.0);

                    // Should also have tags
                    if (props.find("tags") != props.end())
                    {
                        const picojson::array& tags = props.at("tags").get<picojson::array>();
                        bool hasSecurityTag         = false;
                        bool hasCweTag              = false;
                        for (const auto& tag : tags)
                        {
                            const std::string tagStr = tag.get<std::string>();
                            if (tagStr == "security")
                            {
                                hasSecurityTag = true;
                            }
                            else if (tagStr.find("external/cwe/cwe-") == 0)
                            {
                                hasCweTag = true;
                            }
                        }
                        ASSERT_EQUALS(true, hasSecurityTag);
                        ASSERT_EQUALS(true, hasCweTag);
                    }
                }
            }
        }

        ASSERT_EQUALS(true, foundSecurityRule);
    }

    void sarifLocationInfo()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::array& results = run.at("results").get<picojson::array>();

        ASSERT(results.size() > 0);

        // Check location information
        for (const auto& result : results)
        {
            const picojson::object& res = result.get<picojson::object>();

            ASSERT(res.find("locations") != res.end());
            const picojson::array& locations = res.at("locations").get<picojson::array>();
            ASSERT(locations.size() > 0);

            for (const auto& location : locations)
            {
                const picojson::object& loc = location.get<picojson::object>();
                ASSERT(loc.find("physicalLocation") != loc.end());

                const picojson::object& physLoc = loc.at("physicalLocation").get<picojson::object>();
                ASSERT(physLoc.find("artifactLocation") != physLoc.end());
                ASSERT(physLoc.find("region") != physLoc.end());

                const picojson::object& region = physLoc.at("region").get<picojson::object>();
                ASSERT(region.find("startLine") != region.end());
                ASSERT(region.find("startColumn") != region.end());

                // Line numbers should be positive, columns can be 0 or positive
                if (region.at("startLine").is<double>())
                {
                    const int64_t line = static_cast<int64_t>(region.at("startLine").get<double>());
                    ASSERT(line > 0);
                }

                if (region.at("startColumn").is<double>())
                {
                    const int64_t col = static_cast<int64_t>(region.at("startColumn").get<double>());
                    ASSERT(col >= 0);
                }
            }
        }
    }

    void sarifGenericDescriptions()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Check that descriptions are properly genericized
        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            const std::string ruleId  = r.at("id").get<std::string>();
            const std::string name    = r.at("name").get<std::string>();

            // Check that generic descriptions don't contain empty quotes
            ASSERT_EQUALS(std::string::npos, name.find("''"));

            const picojson::object& shortDesc = r.at("shortDescription").get<picojson::object>();
            const std::string shortText       = shortDesc.at("text").get<std::string>();
            ASSERT_EQUALS(std::string::npos, shortText.find("''"));

            const picojson::object& fullDesc = r.at("fullDescription").get<picojson::object>();
            const std::string fullText       = fullDesc.at("text").get<std::string>();
            ASSERT_EQUALS(std::string::npos, fullText.find("''"));
        }
    }

    void sarifCweTags()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Check that security-related rules have CWE tags
        bool foundNullPointerWithCwe = false;
        bool foundArrayBoundsWithCwe = false;
        bool foundMemleakWithCwe     = false;
        bool foundDoubleFreeWithCwe  = false;

        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            const std::string ruleId  = r.at("id").get<std::string>();

            // Only check security-related rules that should have CWE tags
            if (ruleId == "nullPointer" || ruleId == "arrayIndexOutOfBounds" || ruleId == "doubleFree" ||
                ruleId == "memleak")
            {
                const picojson::object& props = r.at("properties").get<picojson::object>();

                // Verify this rule has security-severity (prerequisite for CWE tags)
                if (props.find("security-severity") != props.end())
                {
                    // Should have tags array
                    ASSERT(props.find("tags") != props.end());
                    const picojson::array& tags = props.at("tags").get<picojson::array>();

                    bool hasSecurityTag = false;
                    bool hasCweTag      = false;
                    std::string cweTag;

                    for (const auto& tag : tags)
                    {
                        const std::string tagStr = tag.get<std::string>();
                        if (tagStr == "security")
                        {
                            hasSecurityTag = true;
                        }
                        else if (tagStr.find("external/cwe/cwe-") == 0)
                        {
                            hasCweTag = true;
                            cweTag    = tagStr;

                            // Validate CWE tag format: external/cwe/cwe-<number>
                            ASSERT_EQUALS(0, tagStr.find("external/cwe/cwe-"));
                            std::string cweNumber = tagStr.substr(17);  // After "external/cwe/cwe-"
                            ASSERT(cweNumber.length() > 0);

                            // Verify it's a valid number
                            for (char c : cweNumber)
                            {
                                ASSERT(c >= '0' && c <= '9');
                            }

                            // Track specific CWE mappings we expect
                            if (ruleId == "nullPointer" && cweNumber == "476")
                                foundNullPointerWithCwe = true;
                            else if (ruleId == "arrayIndexOutOfBounds" && cweNumber == "788")
                                foundArrayBoundsWithCwe = true;
                            else if (ruleId == "memleak" && cweNumber == "401")
                                foundMemleakWithCwe = true;
                            else if (ruleId == "doubleFree" && cweNumber == "415")
                                foundDoubleFreeWithCwe = true;
                        }
                    }

                    ASSERT_EQUALS(true, hasSecurityTag);
                    ASSERT_EQUALS(true, hasCweTag);
                }
            }
        }

        // Verify we found at least some of the expected CWE mappings
        // Note: Not all may be present depending on what the test code triggers
        bool foundAnyCweMapping =
            foundNullPointerWithCwe || foundArrayBoundsWithCwe || foundMemleakWithCwe || foundDoubleFreeWithCwe;
        ASSERT_EQUALS(true, foundAnyCweMapping);
    }

    void sarifRuleCoverage()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Verify we have a good variety of rules triggered
        std::set<std::string> ruleIds;
        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            const std::string ruleId  = r.at("id").get<std::string>();
            ruleIds.insert(ruleId);
        }

        // We should have at least 10 different rules triggered by our comprehensive test
        ASSERT(ruleIds.size() >= 10);

        // Check for some specific expected rules from different categories
        std::vector<std::string> expectedRules = {
            "nullPointer",            // Security
            "arrayIndexOutOfBounds",  // Security
            "memleak",                // Security
            "uninitvar",              // Security
            "unusedVariable",         // Style
            "redundantAssignment",    // Style
            "unusedFunction",         // Style
            "constParameter",         // Style/Performance
            "cstyleCast",             // Style
            "variableScope"           // Style
        };

        int foundExpectedRules = 0;
        for (const std::string& expectedRule : expectedRules)
        {
            if (ruleIds.find(expectedRule) != ruleIds.end())
            {
                foundExpectedRules++;
            }
        }

        // We should find at least half of our expected rules
        ASSERT(foundExpectedRules >= static_cast<int>(expectedRules.size() / 2));
    }

    void sarifSeverityLevels()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::array& results = run.at("results").get<picojson::array>();

        ASSERT(results.size() > 0);

        // Track different severity levels
        bool hasError   = false;
        bool hasWarning = false;
        bool hasNote    = false;

        for (const auto& result : results)
        {
            const picojson::object& res = result.get<picojson::object>();
            const std::string level     = res.at("level").get<std::string>();

            if (level == "error")
                hasError = true;
            else if (level == "warning")
                hasWarning = true;
            else if (level == "note")
                hasNote = true;
        }

        // Our comprehensive test should trigger multiple severity levels
        ASSERT_EQUALS(true, hasError);
        // We should have at least one non-error level
        ASSERT(hasWarning || hasNote);

        // Verify rule consistency between rules and results
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        std::set<std::string> ruleIdsInRules;
        std::set<std::string> ruleIdsInResults;

        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            ruleIdsInRules.insert(r.at("id").get<std::string>());
        }

        for (const auto& result : results)
        {
            const picojson::object& res = result.get<picojson::object>();
            ruleIdsInResults.insert(res.at("ruleId").get<std::string>());
        }

        // Every rule ID in results should have a corresponding rule definition
        for (const std::string& resultRuleId : ruleIdsInResults)
        {
            ASSERT(ruleIdsInRules.find(resultRuleId) != ruleIdsInRules.end());
        }
    }

    void sarifNonSecurityRules()
    {
        const std::string sarif = runCppcheckSarif(testCode);

        picojson::value json;
        picojson::parse(json, sarif);
        const picojson::object& root   = json.get<picojson::object>();
        const picojson::array& runs    = root.at("runs").get<picojson::array>();
        const picojson::object& run    = runs[0].get<picojson::object>();
        const picojson::object& tool   = run.at("tool").get<picojson::object>();
        const picojson::object& driver = tool.at("driver").get<picojson::object>();
        const picojson::array& rules   = driver.at("rules").get<picojson::array>();

        // Verify non-security rules don't have security properties
        bool foundNonSecurityRule = false;

        for (const auto& rule : rules)
        {
            const picojson::object& r = rule.get<picojson::object>();
            const std::string ruleId  = r.at("id").get<std::string>();

            // Check non-security rules (style, performance, etc.)
            if (ruleId == "unusedVariable" || ruleId == "redundantAssignment" || ruleId == "constParameter" ||
                ruleId == "cstyleCast" || ruleId == "variableScope" || ruleId == "unusedFunction")
            {
                foundNonSecurityRule = true;

                const picojson::object& props = r.at("properties").get<picojson::object>();

                // Non-security rules should NOT have security-severity
                ASSERT(props.find("security-severity") == props.end());

                // If they have tags, they should not include security or CWE tags
                if (props.find("tags") != props.end())
                {
                    const picojson::array& tags = props.at("tags").get<picojson::array>();
                    for (const auto& tag : tags)
                    {
                        const std::string tagStr = tag.get<std::string>();
                        ASSERT(tagStr != "security");
                        ASSERT(tagStr.find("external/cwe/cwe-") != 0);
                    }
                }

                // Should still have basic properties
                ASSERT(props.find("precision") != props.end());
                ASSERT(props.find("problem.severity") != props.end());
            }
        }

        ASSERT_EQUALS(true, foundNonSecurityRule);
    }
};

REGISTER_TEST(TestSarif)
