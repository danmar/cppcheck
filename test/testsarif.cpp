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

void testFunction() {
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
}

// Unused function
static void unusedFunction() {
    // This function is never called
}

int main() {
    testFunction();
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

                const int64_t line = static_cast<int64_t>(region.at("startLine").get<double>());
                const int64_t col  = static_cast<int64_t>(region.at("startColumn").get<double>());
                ASSERT(line > 0);
                ASSERT(col >= 0);
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
            const std::string name    = r.at("name").get<std::string>();

            // Should not contain specific variable names from the test code
            // Check for exact variable names with word boundaries to avoid false positives
            // The test uses variables: ptr, array, mem, p, x, testFunction, unusedFunction
            ASSERT_EQUALS(std::string::npos, name.find("'ptr'"));
            ASSERT_EQUALS(std::string::npos, name.find("'array'"));
            ASSERT_EQUALS(std::string::npos, name.find("'mem'"));
            ASSERT_EQUALS(std::string::npos, name.find("'p'"));
            ASSERT_EQUALS(std::string::npos, name.find("'x'"));
            ASSERT_EQUALS(std::string::npos, name.find("testFunction"));
            ASSERT_EQUALS(std::string::npos, name.find("unusedFunction"));

            // Should not contain empty quotes
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
};

REGISTER_TEST(TestSarif)
