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

#include <list>
#include <sstream>

static std::string getcode(const std::string &filedata, std::string cfg);

void preprocess(std::istream &istr, std::map<std::string, std::string> &result)
{
    std::ostringstream ostr;
    std::string line;
    while ( getline(istr, line) )
        ostr << line << "\n";

    result.clear();
    result[""] = getcode( ostr.str(), "" );
    result["WIN32"] = getcode( ostr.str(), "WIN32" );
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

