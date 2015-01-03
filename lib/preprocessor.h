/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#ifndef preprocessorH
#define preprocessorH
//---------------------------------------------------------------------------

#include <map>
#include <istream>
#include <string>
#include <list>
#include <set>
#include "config.h"

class ErrorLogger;
class Settings;

/// @addtogroup Core
/// @{

/**
 * @brief The cppcheck preprocessor.
 * The preprocessor has special functionality for extracting the various ifdef
 * configurations that exist in a source file.
 */
class CPPCHECKLIB Preprocessor {
public:

    /**
     * Include file types.
     */
    enum HeaderTypes {
        NoHeader = 0,
        UserHeader,
        SystemHeader
    };

    /** character that is inserted in expanded macros */
    static char macroChar;

    Preprocessor(Settings *settings = nullptr, ErrorLogger *errorLogger = nullptr);

    static bool missingIncludeFlag;
    static bool missingSystemIncludeFlag;

    /**
     * Extract the code for each configuration
     * @param istr The (file/string) stream to read from.
     * @param result The map that will get the results
     * @param filename The name of the file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where include files should be searched from,
     * single path can be e.g. in format "include/".
     * There must be a path separator at the end. Default parameter is empty list.
     * Note that if path from given filename is also extracted and that is used as
     * a last include path if include file was not found from earlier paths.
     */
    void preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename, const std::list<std::string> &includePaths = std::list<std::string>());

    /**
     * Extract the code for each configuration. Use this with getcode() to get the
     * file data for each individual configuration.
     *
     * @param srcCodeStream The (file/string) stream to read from.
     * @param processedFile Give reference to empty string as a parameter,
     * function will fill processed file here. Use this also as a filedata parameter
     * to getcode() if you received more than once configurations.
     * @param resultConfigurations List of configurations. Pass these one by one
     * to getcode() with processedFile.
     * @param filename The name of the file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where include files should be searched from,
     * single path can be e.g. in format "include/".
     * There must be a path separator at the end. Default parameter is empty list.
     * Note that if path from given filename is also extracted and that is used as
     * a last include path if include file was not found from earlier paths.
     */
    void preprocess(std::istream &srcCodeStream, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename, const std::list<std::string> &includePaths);

    /** Just read the code into a string. Perform simple cleanup of the code */
    std::string read(std::istream &istr, const std::string &filename);

    /** read preprocessor statements into a string. */
    static std::string readpreprocessor(std::istream &istr, const unsigned int bom);

    /** should __cplusplus be defined? */
    static bool cplusplus(const Settings *settings, const std::string &filename);

    /**
     * Get preprocessed code for a given configuration
     * @param filedata file data including preprocessing 'if', 'define', etc
     * @param cfg configuration to read out
     * @param filename name of source file
     */
    std::string getcode(const std::string &filedata, const std::string &cfg, const std::string &filename);

    /**
     * simplify condition
     * @param variables Variable values
     * @param condition The condition to simplify
     * @param match if true, 'defined(A)' is replaced with 0 if A is not defined
     */
    void simplifyCondition(const std::map<std::string, std::string> &variables, std::string &condition, bool match);

    /**
     * preprocess all whitespaces
     * @param processedFile The data to be processed
     */
    static void preprocessWhitespaces(std::string &processedFile);

    /**
     * make sure empty configuration macros are not used in code. the given code must be a single configuration
     * @param code The input code
     * @param cfg configuration
     * @return true => configuration is valid
     */
    bool validateCfg(const std::string &code, const std::string &cfg);
    void validateCfgError(const std::string &cfg, const std::string &macro);

    void handleUndef(std::list<std::string> &configurations) const;

    /**
     * report error
     * @param fileName name of file that the error was found in
     * @param linenr linenr in file
     * @param errorLogger Error logger to write error to
     * @param errorType id string for error
     * @param errorText Plain text
     */
    static void writeError(const std::string &fileName, const unsigned int linenr, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText);

    /**
     * Replace "#if defined" with "#ifdef" where possible
     *
     * @param str The string to be converted
     * @return The replaced string
     */
    std::string replaceIfDefined(const std::string &str) const;

    /**
     * expand macros in code. ifdefs etc are ignored so the code must be a single configuration
     * @param code The input code
     * @param filename filename of source file
     * @param cfg user given -D configuration
     * @param errorLogger Error logger to write errors to (if any)
     * @return the expanded string
     */
    static std::string expandMacros(const std::string &code, std::string filename, const std::string &cfg, ErrorLogger *errorLogger);

    /**
     * Remove comments from code. This should only be called from read().
     * If there are inline suppressions, the _settings member is modified
     * @param str Code processed by read().
     * @param filename filename
     * @return code without comments
     */
    std::string removeComments(const std::string &str, const std::string &filename);

    /**
     * Cleanup 'if 0' from the code
     * @param code Code processed by read().
     * @return code without 'if 0'
     */
    static std::string removeIf0(const std::string &code);

    /**
     * Remove redundant parentheses from preprocessor commands. This should only be called from read().
     * @param str Code processed by read().
     * @return code with reduced parentheses
     */
    static std::string removeParentheses(const std::string &str);

    /**
     * clean up #-preprocessor lines (only)
     * @param processedFile The data to be processed
     */
    static std::string preprocessCleanupDirectives(const std::string &processedFile);

    /**
     * Returns the string between double quote characters or \< \> characters.
     * @param str e.g. \code#include "menu.h"\endcode or \code#include <menu.h>\endcode
     * After function call it will contain e.g. "menu.h" without double quotes.
     * @return NoHeader empty string if double quotes or \< \> were not found.
     *         UserHeader if file surrounded with "" was found
     *         SystemHeader if file surrounded with \<\> was found
     */
    static Preprocessor::HeaderTypes getHeaderFileName(std::string &str);
private:

    /**
     * Remove space that has new line character on left or right side of it.
     *
     * @param str The string to be converted
     * @return The string where space characters have been removed.
     */
    static std::string removeSpaceNearNL(const std::string &str);

    static std::string getdef(std::string line, bool def);

public:

    /**
     * Get all possible configurations sorted in alphabetical order.
     * By looking at the ifdefs and ifndefs in filedata
     */
    std::list<std::string> getcfgs(const std::string &filedata, const std::string &filename, const std::map<std::string, std::string> &defs);

    /**
     * Remove asm(...) from a string
     * @param str Code
     */
    static void removeAsm(std::string &str);

    /**
     * Evaluate condition 'numerically'
     * @param cfg configuration
     * @param def condition
     * @return result when evaluating the condition
     */
    bool match_cfg_def(std::map<std::string, std::string> cfg, std::string def);

    static void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings);

    /**
     * handle includes for a specific configuration
     * @param code code in string
     * @param filePath filename of code
     * @param includePaths Paths where headers might be
     * @param defs defines (only values)
     * @param pragmaOnce includes that has already been included and contains a \#pragma once statement
     * @param includes provide a empty list. this is just used to prevent recursive inclusions.
     * \return resulting string
     */
    std::string handleIncludes(const std::string &code, const std::string &filePath, const std::list<std::string> &includePaths, std::map<std::string,std::string> &defs, std::set<std::string> &pragmaOnce, std::list<std::string> includes);

    void setFile0(const std::string &f) {
        file0 = f;
    }

private:
    void missingInclude(const std::string &filename, unsigned int linenr, const std::string &header, HeaderTypes headerType);

    void error(const std::string &filename, unsigned int linenr, const std::string &msg);

    /**
     * Search includes from code and append code from the included
     * file
     * @param[in,out] code The source code to modify
     * @param filePath Relative path to file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where include files should be searched from,
     * single path can be e.g. in format "include/".
     * There must be a path separator at the end. Default parameter is empty list.
     * Note that if path from given filename is also extracted and that is used as
     * a last include path if include file was not found from earlier paths.
     */
    void handleIncludes(std::string &code, const std::string &filePath, const std::list<std::string> &includePaths);

    Settings *_settings;
    ErrorLogger *_errorLogger;

    /** filename for cpp/c file - useful when reporting errors */
    std::string file0;
};

/// @}
//---------------------------------------------------------------------------
#endif // preprocessorH
