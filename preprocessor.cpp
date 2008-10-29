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
#include <ctype>
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
    // Get filedata from stream..
    bool ignoreSpace = true;

    std::ostringstream code;
    for (char ch = (char)istr.get(); !istr.eof(); ch = (char)istr.get())
    {
        // Replace assorted special chars with spaces..
        if ( ch < 0 )
            ch = ' ';
        if ( (ch != '\n') && (isspace(ch) || iscntrl(ch)) )
            ch = ' ';

        // Skip spaces after ' ' and after '#'
        if ( ch == ' ' && ignoreSpace )
            continue;
        ignoreSpace = bool(ch == ' ' || ch == '#' || ch == '/');

        // Remove comments..
        if ( ch == '/' )
        {
            char chNext = (char)istr.get();

            if ( chNext == '/' )
            {
                while (!istr.eof() && ch!='\n')
                    ch = (char)istr.get();
                code << "\n";
            }

            else if ( chNext == '*' )
            {
                char chPrev = 0;
                while (!istr.eof() && (chPrev!='*' || ch!='/'))
                {
                    chPrev = ch;
                    ch = (char)istr.get();
                    if (ch == '\n')
                        code << "\n";
                }
            }

            else
            {
                code << std::string(1,ch) << std::string(1,chNext);
            }
        }

        // String constants..
        else if ( ch == '\"' )
        {
            code << "\"";
            do
            {
                ch = (char)istr.get();
                code << std::string(1,ch);
                if ( ch == '\\' )
                {
                    ch = (char)istr.get();
                    code << std::string(1,ch);
                }
            } while ( !istr.eof() && ch != '\"' );
        }

        // char constants..
        else if ( ch == '\'' )
        {
            code << "\'";
            ch = (char)istr.get();
            code << std::string(1,ch);
            if ( ch == '\\' )
            {
                ch = (char)istr.get();
                code << std::string(1,ch);
            }
            ch = (char)istr.get();
            code << "\'";
        }

        // Just some code..
        else
        {
            code << std::string(1, ch);
        }
    }

    std::string codestr( code.str() );

    // Remove all indentation..
    while ( ! codestr.empty() && codestr[0] == ' ' )
        codestr.erase(0, 1);
    while ( codestr.find("\n ") != std::string::npos )
        codestr.erase( 1 + codestr.find("\n "), 1 );

    // Remove all trailing spaces..
    while ( codestr.find(" \n") != std::string::npos )
        codestr.erase( codestr.find(" \n"), 1 );

    // Remove "#if 0" blocks..
    if ( codestr.find("#if 0") != std::string::npos )
    {
        code.str("");
        std::string line;
        std::istringstream istr(codestr.c_str());
        while (getline(istr,line))
        {
            if ( line == "#if 0" )
            {
                code << "\n";
                unsigned int iflevel = 1;
                while ( getline(istr, line) )
                {
                    code << "\n";

                    if ( line.find("#if") != std::string::npos )
                        ++iflevel;

                    else if ( line.find("#endif") != std::string::npos )
                    {
                        --iflevel;
                        if ( iflevel == 0 )
                            break;
                    }
                }
            }
            else
            {
                code << line << "\n";
            }
        }
        codestr = code.str();
    }

    // Get all possible configurations..
    std::list<std::string> cfgs = getcfgs( codestr );

    // Extract the code for each possible configuration..
    result.clear();
    for ( std::list<std::string>::const_iterator it = cfgs.begin(); it != cfgs.end(); ++it )
    {
        result[ *it ] = getcode( codestr, *it );
    }
}



// Get the DEF in this line: "#ifdef DEF"
static std::string getdef(std::string line, bool def)
{
    // If def is true, the line must start with "#ifdef"
    if ( def && line.find("#ifdef ") != 0 )
    {
        return "";
    }

    // If def is false, the line must start with "#ifndef"
    if ( !def && line.find("#ifndef ") != 0 )
    {
        return "";
    }

    // Remove the "#ifdef" or "#ifndef"
    line.erase( 0, line.find(" ") );

    // Remove all spaces.
    while ( line.find(" ") != std::string::npos )
        line.erase( line.find(" "), 1 );

    // The remaining string is our result.
    return line;
}



static std::list<std::string> getcfgs( const std::string &filedata )
{
    std::list<std::string> ret;
    ret.push_back("");

    std::istringstream istr(filedata);
    std::string line;
    while ( getline(istr, line) )
    {
        std::string def = getdef(line, true) + getdef(line, false);
        if (!def.empty())
        {
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
        std::string def = getdef( line, true );
        std::string ndef = getdef( line, false );

        if ( ! def.empty() )
            matching_ifdef.push_back( cfg == def );

        else if ( ! ndef.empty() )
            matching_ifdef.push_back( cfg != ndef );

        else if ( line == "#else" )
            matching_ifdef.back() = ! matching_ifdef.back();

        else if ( line == "#endif" )
            matching_ifdef.pop_back();

        if ( !matching_ifdef.empty() && !matching_ifdef.back() )
            line = "";

        if ( line.find("#if") == 0 ||
             line.find("#else") == 0 ||
             line.find("#endif") == 0 )
            line = "";

        ret << line << "\n";
    }

    return ret.str();
}

