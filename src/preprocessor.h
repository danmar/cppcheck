/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
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


class Preprocessor
{
public:
    Preprocessor();

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
    void preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename, const std::list<std::string> &includePaths = std::list<std::string>(), ErrorLogger *errorLogger = 0);

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

protected:

    /**
     * Replace "#if defined" with "#ifdef" where possible
     *
     * @param str The string to be converted
     * @return The replaced string
     */
    static std::string replaceIfDefined(const std::string &str);

    static std::string expandMacros(std::string code, const std::string &filename, ErrorLogger *errorLogger);

private:

    /**
     * Remove space that has new line character on left or right side of it.
     *
     * @param str The string to be converted
     * @return The string where space characters have been removed.
     */
    static std::string removeSpaceNearNL(const std::string &str);

    /**
     * Get all possible configurations. By looking at the ifdefs and ifndefs in filedata
     */
    std::list<std::string> getcfgs(const std::string &filedata);

    static std::string getdef(std::string line, bool def);

    static bool match_cfg_def(std::string cfg, const std::string &def);

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
    static void handleIncludes(std::string &code, const std::string &filename, const std::list<std::string> &includePaths);

    /**
     * Returns the string between double quote characters.
     * @param str e.g. '#include "menu.h"'
     * @return e.g. 'menu.h' or empty string if double quotes were
     * not found.
     */
    static std::string getHeaderFileName(const std::string &str);
};

//---------------------------------------------------------------------------
#endif

