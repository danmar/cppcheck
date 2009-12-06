/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
#include "errorlogger.h"
#include "settings.h"

/// @addtogroup Core
/// @{

/** @brief The cppcheck preprocessor. It has special functionality for extracting the various ifdef configurations that exist in a source file. */
class Preprocessor
{
public:
    Preprocessor(const Settings *settings = 0, ErrorLogger *errorLogger = 0);

    /**
     * Extract the code for each configuration
     * @param istr The (file/string) stream to read from.
     * @param result The map that will get the results
     * @param filename The name of the file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where incude files should be searched from,
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
     * @param istr The (file/string) stream to read from.
     * @param processedFile Give reference to empty string as a parameter,
     * function will fill processed file here. Use this also as a filedata parameter
     * to getcode() if you recieved more than once configurations.
     * @param resultConfigurations List of configurations. Pass these one by one
     * to getcode() with processedFile.
     * @param filename The name of the file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where incude files should be searched from,
     * single path can be e.g. in format "include/".
     * There must be a path separator at the end. Default parameter is empty list.
     * Note that if path from given filename is also extracted and that is used as
     * a last include path if include file was not found from earlier paths.
     */
    void preprocess(std::istream &istr, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename, const std::list<std::string> &includePaths);

    /** Just read the code into a string. Perform simple cleanup of the code */
    static std::string read(std::istream &istr);

    /**
     * Get preprocessed code for a given configuration
     */
    static std::string getcode(const std::string &filedata, std::string cfg, const std::string &filename, ErrorLogger *errorLogger);

    /**
     * simplify condition
     * @param variables Variable values
     * @param condition The condition to simplify
     */
    static void simplifyCondition(const std::map<std::string, std::string> &variables, std::string &condition);

protected:

    static void writeError(const std::string &fileName, const int linenr, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText);

    /**
     * Replace "#if defined" with "#ifdef" where possible
     *
     * @param str The string to be converted
     * @return The replaced string
     */
    static std::string replaceIfDefined(const std::string &str);

    static std::string expandMacros(const std::string &code, std::string filename, ErrorLogger *errorLogger);

    /**
     * Remove comments from code. This should only be called from read().
     * @param str Code processed by read().
     * @return code without comments
     * @throws std::runtime_error when code contains unhandled characters
     */
    static std::string removeComments(const std::string &str);

    /**
     * Remove redundant parantheses from preprocessor commands. This should only be called from read().
     * @param str Code processed by read().
     * @return code with reduced parantheses
     */
    static std::string removeParantheses(const std::string &str);

    /**
     * Returns the string between double quote characters or \< \> characters.
     * @param str e.g. \code#include "menu.h"\endcode or \code#include <menu.h>\endcode
     * After function call it will contain e.g. "menu.h" without double quotes.
     * @return 0 empty string if double quotes or \< \> were not found.
     *         1 if file surrounded with "" was found
     *         2 if file surrounded with \<\> was found
     */
    static int getHeaderFileName(std::string &str);
private:

    /**
     * Remove space that has new line character on left or right side of it.
     *
     * @param str The string to be converted
     * @return The string where space characters have been removed.
     */
    static std::string removeSpaceNearNL(const std::string &str);

    /**
     * Get all possible configurations sorted in alphabetical order.
     * By looking at the ifdefs and ifndefs in filedata
     */
    std::list<std::string> getcfgs(const std::string &filedata, const std::string &filename);

    static std::string getdef(std::string line, bool def);

public:

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
    static bool match_cfg_def(const std::map<std::string, std::string> &cfg, std::string def);

private:
    /**
     * Search includes from code and append code from the included
     * file
     * @param code The source code to modify
     * @param filename The name of the file to check e.g. "src/main.cpp"
     * @param includePaths List of paths where incude files should be searched from,
     * single path can be e.g. in format "include/".
     * There must be a path separator at the end. Default parameter is empty list.
     * Note that if path from given filename is also extracted and that is used as
     * a last include path if include file was not found from earlier paths.
     * @return modified source code
     */
    void handleIncludes(std::string &code, const std::string &filename, const std::list<std::string> &includePaths);

    const Settings *_settings;
    ErrorLogger *_errorLogger;
};

/// @}

//---------------------------------------------------------------------------
#endif

