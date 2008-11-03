/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki and Reijo Tomperi
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

#include "FileLister.h"
#include <sstream>

#ifdef __GNUC__
#include <glob.h>
#include <unistd.h>
#endif
#ifdef __BORLANDC__
#include <dir.h>
#endif
#ifdef _MSC_VER
#include <windows.h>
#endif

bool FileLister::AcceptFile( const std::string &filename )
{
    std::string::size_type dotLocation = filename.find_last_of ( '.' );
    if ( dotLocation == std::string::npos )
        return false;

    std::string extension = filename.substr( dotLocation );

    if( extension == ".cpp" ||
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
    if( recursive )
    {
        if ( path.length() > 0 && path[path.length()-1] != '/' )
            oss << "/";

        oss << "*";
    }

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
////// This code is for __BORLANDC__ only /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__

void FileLister::AddFiles( std::vector<std::string> &filenames, const std::string &path, const std::string &pattern )
{
    struct ffblk f;
    for ( int done = findfirst(pattern.c_str(), &f, 0); ! done; done = findnext(&f) )
    {
        std::ostringstream fname;
        fname << path << f.ff_name;
        filenames.push_back( fname.str() );
    }
    findclose(&f);
}


// TODO, this should be complitely rewritten to work similarly like with __GNUC__,
// but I don't have a compiler to do the work
void FileLister::RecursiveAddFiles( std::vector<std::string> &filenames, const std::string &path, bool recursive )
{
    if( !recursive )
    {
        // Simulate old behaviour
        if ( path.find( '*' ) == std::string::npos )
            filenames.push_back( path );
        else
            FileLister::AddFiles( filenames, "", path );

        return;
    }

    AddFiles( filenames, path, "*.cpp" );
    AddFiles( filenames, path, "*.cc" );
    AddFiles( filenames, path, "*.c" );

    struct ffblk f ;
    for ( int done = findfirst("*", &f, FA_DIREC); ! done; done = findnext(&f) )
    {
        if ( f.ff_attrib != FA_DIREC || f.ff_name[0] == '.' )
            continue;
        chdir( f.ff_name );
        std::ostringstream curdir;
        curdir << path << f.ff_name << "/";
        FileLister::RecursiveAddFiles( filenames, curdir.str().c_str(), true );
        chdir( ".." );
    }
    findclose(&f);
}

#endif

///////////////////////////////////////////////////////////////////////////////
////// This code is for _MSC_VER only /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

void FileLister::AddFiles( std::vector<std::string> &filenames, const std::string &path, const std::string &pattern )
{

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(pattern.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            std::ostringstream fname;
            fname << path << ffd.cFileName;
            filenames.push_back( fname.str() );
        }
        while (FindNextFile(hFind, &ffd) != 0);
    }
}

// TODO, this should be complitely rewritten to work similarly like with __GNUC__,
// but I don't have a compiler to do the work
void FileLister::RecursiveAddFiles( std::vector<std::string> &filenames, const std::string &path, bool recursive )
{
    if( !recursive )
    {
        // Simulate old behaviour
        if ( path.find( '*' ) == std::string::npos )
            filenames.push_back( path );
        else
            FileLister::AddFiles( filenames, "", path );

        return;
    }

    AddFiles( filenames, path, "*.cpp" );
    AddFiles( filenames, path, "*.cc" );
    AddFiles( filenames, path, "*.c" );

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile("*", &ffd);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if ( (ffd.cFileName[0]!='.') &&
                 (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                SetCurrentDirectory( ffd.cFileName );
                std::ostringstream curdir;
                curdir << path << ffd.cFileName << "/";
                FileLister::RecursiveAddFiles( filenames, curdir.str().c_str(), true );
                SetCurrentDirectory( ".." );
            }
        }
        while (FindNextFile(hFind, &ffd) != 0);
    }
}


#endif




