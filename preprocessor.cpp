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
#include "CommonCheck.h"

#include <algorithm>

#include <sstream>

#ifdef __BORLANDC__
#include <ctype>
#endif




/**
 * Extract the code for each configuration
 * \param istr The (file/string) stream to read from.
 * \param result The map that will get the results
 */
void Preprocessor::preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename)
{
    // Get filedata from stream..
    bool ignoreSpace = true;

    int lineno = 1;

    std::ostringstream code;
    for (char ch = (char)istr.get(); istr.good(); ch = (char)istr.get())
    {
        if ( ch < 0 )
        {
            // Bad content..
            errout << "[" << filename << ":" << lineno << "] Bad character found: " << int((unsigned char)ch) << std::endl;
            result.clear();
            return;
        }

        if ( ch == '\n' )
            ++lineno;

        // Replace assorted special chars with spaces..
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
                while (istr.good() && ch!='\n')
                    ch = (char)istr.get();
                code << "\n";
                ++lineno;
            }

            else if ( chNext == '*' )
            {
                char chPrev = 0;
                while (istr.good() && (chPrev!='*' || ch!='/'))
                {
                    chPrev = ch;
                    ch = (char)istr.get();
                    if (ch == '\n')
                    {
                        code << "\n";
                        ++lineno;
                    }
                }
            }

            else
            {
                if ( chNext == '\n' )
                    ++lineno;
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
            } while ( istr.good() && ch != '\"' );
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
    if ( !codestr.empty() && codestr[0] == ' ' )
        codestr.erase( 0, codestr.find_first_not_of(" ") );
    std::string::size_type loc = 0;
    while ( (loc = codestr.find("\n ", loc)) != std::string::npos )
        codestr.erase( 1 + loc, 1 );

    // Remove all trailing spaces..
    loc = 0;
    while ( (loc = codestr.find(" \n", loc)) != std::string::npos )
        codestr.erase( loc, 1 );

    // Using the backslash at the end of a line..
    while ( (loc = codestr.rfind("\\\n")) != std::string::npos )
    {
        codestr.erase(loc, 2);
        if (loc > 0 && codestr[loc-1] != ' ')
            codestr.insert(loc, " ");
        if ( (loc = codestr.find("\n", loc)) != std::string::npos)
            codestr.insert( loc, "\n" );
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
std::string Preprocessor::getdef(std::string line, bool def)
{
    // If def is true, the line must start with "#ifdef"
    if ( def && line.find("#ifdef ") != 0 && line.find("#if ") != 0 && line.find("#elif ") != 0 )
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



std::list<std::string> Preprocessor::getcfgs( const std::string &filedata )
{
    std::list<std::string> ret;
    ret.push_back("");

    std::list<std::string> deflist;

    std::istringstream istr(filedata);
    std::string line;
    while ( getline(istr, line) )
    {
        std::string def = getdef(line, true) + getdef(line, false);
        if (!def.empty())
        {
            if ( ! deflist.empty() && line.find("#elif ") == 0 )
                deflist.pop_back();
            deflist.push_back(def);
            def = "";
            for ( std::list<std::string>::const_iterator it = deflist.begin(); it != deflist.end(); ++it)
            {
                if ( *it == "0" )
                    break;
                if ( *it == "1" )
                    continue;
                if ( ! def.empty() )
                    def += ";";
                def += *it;
            }

            if (std::find(ret.begin(), ret.end(), def) == ret.end())
                ret.push_back( def );
        }

        if ( line.find("#else") == 0 && ! deflist.empty() )
        {
            std::string def( ( deflist.back() == "1" ) ? "0" : "1" );
            deflist.pop_back();
            deflist.push_back( def );
        }

        if ( line.find("#endif") == 0 && ! deflist.empty()  )
            deflist.pop_back();
    }

    return ret;
}



bool Preprocessor::match_cfg_def( std::string cfg, const std::string &def )
{
    if ( def == "0" )
        return false;

    if ( def == "1" )
        return true;

    if ( cfg.empty() )
        return false;

    while ( ! cfg.empty() )
    {
        if ( cfg.find(";") == std::string::npos )
            return bool(cfg == def);
        std::string _cfg = cfg.substr( 0, cfg.find(";") );
        if ( _cfg == def )
            return true;
        cfg.erase( 0, cfg.find(";") + 1 );
    }

    return false;
}


std::string Preprocessor::getcode(const std::string &filedata, std::string cfg)
{
    std::ostringstream ret;

    bool match = true;
    std::list<bool> matching_ifdef;
    std::list<bool> matched_ifdef;

    std::istringstream istr(filedata);
    std::string line;
    while ( getline(istr, line) )
    {
        std::string def = getdef( line, true );
        std::string ndef = getdef( line, false );

        if ( line.find("#elif ") == 0 )
        {
            if ( matched_ifdef.back() )
            {
                matching_ifdef.back() = false;
            }
            else
            {
                if ( match_cfg_def(cfg, def) )
                {
                    matching_ifdef.back() = true;
                    matched_ifdef.back() = true;
                }
            }
        }

        else if ( ! def.empty() )
        {
            matching_ifdef.push_back( match_cfg_def(cfg, def) );
            matched_ifdef.push_back( matching_ifdef.back() );
        }

        else if ( ! ndef.empty() )
        {
            matching_ifdef.push_back( ! match_cfg_def(cfg, ndef) );
            matched_ifdef.push_back( matching_ifdef.back() );
        }

        else if ( line == "#else" )
        {
            if ( ! matched_ifdef.empty() )
                matching_ifdef.back() = ! matched_ifdef.back();
        }

        else if ( line == "#endif" )
        {
            if ( ! matched_ifdef.empty() )
                matched_ifdef.pop_back();
            if ( ! matching_ifdef.empty() )
                matching_ifdef.pop_back();
        }

        if ( !line.empty() && line[0] == '#' )
        {
            match = true;
            for ( std::list<bool>::const_iterator it = matching_ifdef.begin(); it != matching_ifdef.end(); ++it )
                match &= bool(*it);
        }
        if ( ! match )
            line = "";

        if ( line.find("#if") == 0 ||
             line.find("#else") == 0 ||
             line.find("#elif") == 0 ||
             line.find("#endif") == 0 )
            line = "";

        ret << line << "\n";
    }

    return ret.str();
}

