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

#include "filelister.h"
#include <sstream>
#include <vector>
#include <string>

#ifdef __GNUC__
#include <glob.h>
#include <unistd.h>
#endif
#if defined(__BORLANDC__) || defined(_MSC_VER)
#include <windows.h>
#endif

std::string FileLister::simplifyPath( const char *originalPath )
{
    std::string subPath = "";
    std::vector<std::string> pathParts;
    for( ; *originalPath; ++originalPath )
    {
        if( *originalPath == '/' )
        {
            if( subPath.length() > 0 )
            {
                pathParts.push_back( subPath );
                subPath = "";
            }

            pathParts.push_back( "/" );
        }
        else
            subPath.append( 1, *originalPath );
    }

    if( subPath.length() > 0 )
        pathParts.push_back( subPath );

    for( std::vector<std::string>::size_type i = 0; i < pathParts.size(); ++i )
    {
        if( pathParts[i] == ".." && i > 1 )
        {
            pathParts.erase( pathParts.begin() + i );
            pathParts.erase( pathParts.begin()+i-1 );
            pathParts.erase( pathParts.begin()+i-2 );
            i = 0;
        }
        else if( i > 0 && pathParts[i] == "." )
        {
            pathParts.erase( pathParts.begin()+i );
            i = 0;
        }
        else if( pathParts[i] == "/" && i > 0 && pathParts[i-1] == "/" )
        {
            pathParts.erase( pathParts.begin()+i-1 );
            i = 0;
        }
    }

    std::ostringstream oss;
    for( std::vector<std::string>::size_type i = 0; i < pathParts.size(); ++i )
    {
        oss << pathParts[i];
    }

    return oss.str();
}



bool FileLister::AcceptFile( const std::string &filename )
{
    std::string::size_type dotLocation = filename.find_last_of ( '.' );
    if ( dotLocation == std::string::npos )
        return false;

    std::string extension = filename.substr( dotLocation );

    if( extension == ".cpp" ||
        extension == ".cxx" ||
        extension == ".cc" ||
        extension == ".c" )
    {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
////// This code is for __GNUC__ only /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
// gcc / cygwin..
void FileLister::RecursiveAddFiles( std::vector<std::string> &filenames, const std::string &path, bool recursive )
{
    std::ostringstream oss;
    oss << path;
    if ( path.length() > 0 && path[path.length()-1] == '/' )
        oss << "*";

    glob_t glob_results;
    glob( oss.str().c_str(), GLOB_MARK, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        std::string filename = glob_results.gl_pathv[i];
        if ( filename == "." || filename == ".." || filename.length() == 0 )
            continue;

        if ( filename[filename.length()-1] != '/' )
        {
            // File

            // If recursive is not used, accept all files given by user
            if( !recursive || FileLister::AcceptFile( filename ) )
                filenames.push_back( filename );
        }
        else if( recursive )
        {
            // Directory
            FileLister::RecursiveAddFiles( filenames, filename, recursive );
        }
    }
    globfree(&glob_results);
}
#endif


///////////////////////////////////////////////////////////////////////////////
////// This code is for Borland C++ and Visual C++ ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__BORLANDC__) || defined(_MSC_VER)

void FileLister::RecursiveAddFiles( std::vector<std::string> &filenames, const std::string &path, bool recursive )
{
    std::ostringstream bdir, oss;
    oss << path;
    bdir << path;
    if ( path.length() > 0 && path[path.length()-1] == '/' )
    {
        oss << "*";
    }

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(oss.str().c_str(), &ffd);
    if ( INVALID_HANDLE_VALUE == hFind )
        return;

    do
    {
        std::ostringstream fname;
        fname << bdir.str().c_str() << ffd.cFileName;

        if ( ffd.cFileName[0] == '.' || ffd.cFileName[0] == '\0' )
            continue;

        if ( ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
        {
            // File

            // If recursive is not used, accept all files given by user
            if ( !recursive || FileLister::AcceptFile( ffd.cFileName ) )
                filenames.push_back( fname.str() );
        }
        else if ( recursive )
        {
            // Directory
            fname << "/";
            FileLister::RecursiveAddFiles( filenames, fname.str().c_str(), recursive );
        }
    }
    while (FindNextFile(hFind, &ffd) != FALSE);

    FindClose(hFind);
}

#endif




