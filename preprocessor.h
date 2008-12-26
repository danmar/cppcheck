/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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


class Preprocessor
{
public:
    Preprocessor();

    /**
     * Extract the code for each configuration
     * \param istr The (file/string) stream to read from.
     * \param result The map that will get the results
     */
    void preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename);

    /**
     * Extract the code for each configuration. Use this with getcode() to get the
     * file data for each individual configuration.
     *
     * @param istr The (file/string) stream to read from.
     * @param filename
     * @param processedFile Give reference to empty string as a parameter,
     * function will fill processed file here. Use this also as a filedata parameter
     * to getcode() if you recieved more than once configurations.
     * @param resultConfigurations List of configurations. Pass these one by one
     * to getcode() with processedFile.
     */
    void preprocess(std::istream &istr, const std::string &filename, std::string &processedFile, std::list<std::string> &resultConfigurations );

    /** Just read the code into a string. Perform simple cleanup of the code */
    std::string read(std::istream &istr, const std::string &filename);

    /**
     * Get preprocessed code for a given configuration
     */
    static std::string getcode(const std::string &filedata, std::string cfg);

private:

    /**
     * Get all possible configurations. By looking at the ifdefs and ifndefs in filedata
     */
    std::list<std::string> getcfgs( const std::string &filedata );

    static std::string getdef(std::string line, bool def);

    static bool match_cfg_def( std::string cfg, const std::string &def );
};

//---------------------------------------------------------------------------
#endif

