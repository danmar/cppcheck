/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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


#include "preprocessor.h"

#include <algorithm>
#include <list>
#include <sstream>

/**
 * Get all possible configurations. By looking at the ifdefs and ifndefs in filedata
 */
static std::list<std::string> getcfgs( const std::string &filedata );

/**
 * Get preprocessed code for a given configuration
 */
static std::string getcode(const std::string &filedata, std::string cfg);


/**
 * Extract the code for each configuration
 * \param istr The (file/string) stream to read from.
 * \param result The map that will get the results
 */
void preprocess(std::istream &istr, std::map<std::string, std::string> &result)
{
    std::ostringstream ostr;
    std::string line;
    while ( getline(istr, line) )
        ostr << line << "\n";

    // Get all possible configurations..
    std::list<std::string> cfgs = getcfgs( ostr.str() );

    // Extract the code for each possible configuration..
    result.clear();
    for ( std::list<std::string>::const_iterator it = cfgs.begin(); it != cfgs.end(); ++it )
    {
        result[ *it ] = getcode( ostr.str(), *it );
    }
}



static std::list<std::string> getcfgs( const std::string &filedata )
{
    std::list<std::string> ret;
    ret.push_back("");

    std::istringstream istr(filedata);
    std::string line;
    while ( getline(istr, line) )
    {
        if ( line.find("#ifdef ")==0 || line.find("#ifndef ")==0 )
        {
            std::string def( line.substr(line.find(" ") + 1) );
            if (std::find(ret.begin(), ret.end(), def) == ret.end())
                ret.push_back( def );
        }
    }

    return ret;
}




static std::string getcode(const std::string &filedata, std::string cfg)
{
    std::ostringstream ret;

    std::list<bool> matching_ifdef;

    std::istringstream istr(filedata);
    std::string line;
    while ( getline(istr, line) )
    {
        if ( line.find("#ifdef ") == 0 )
            matching_ifdef.push_back( !cfg.empty() && line.find(cfg) != std::string::npos );

        else if ( line.find("#ifndef ") == 0 )
            matching_ifdef.push_back( cfg.empty() || line.find(cfg) == std::string::npos );

        else if ( line.find("#else") == 0)
            matching_ifdef.back() = ! matching_ifdef.back();

        else if ( line.find("#endif") == 0 )
            matching_ifdef.pop_back();

        if ( !matching_ifdef.empty() && !matching_ifdef.back() )
            line = "";

        if ( line.find("#") == 0 )
            line = "";

        ret << line << "\n";
    }

    return ret.str();
}

