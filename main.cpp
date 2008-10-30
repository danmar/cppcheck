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


#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"

#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"
#include "CheckOther.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>


// Check that the compiler are supported
// This program should be compiled with either GCC/BORLAND/MSC to work..
#ifndef __GNUC__
#ifndef  __BORLANDC__
#ifndef _MSC_VER
#error "C++Check must be compiled by either GCC/BORLAND/MSC to work fully.\n"
#error "Please report that you couldn't compile c++check through the web page:\n"
#error "      https://sourceforge.net/projects/cppcheck/"
#endif
#endif
#endif


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

//---------------------------------------------------------------------------
bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId);


static void AddFiles( std::vector<std::string> &filenames, const char path[], const char pattern[] )
{
    #ifdef __GNUC__
    glob_t glob_results;
    glob(pattern, 0, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        std::ostringstream fname;
        fname << path << glob_results.gl_pathv[i];
        filenames.push_back( fname.str() );
    }
    globfree(&glob_results);
    #endif
    #ifdef __BORLANDC__
    struct ffblk f;
    for ( int done = findfirst(pattern, &f, 0); ! done; done = findnext(&f) )
    {
        std::ostringstream fname;
        fname << path << f.ff_name;
        filenames.push_back( fname.str() );
    }
    findclose(&f);
    #endif
    #ifdef _MSC_VER
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(pattern, &ffd);
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
	#endif
}

static void RecursiveAddFiles( std::vector<std::string> &filenames, const char path[] )
{
    AddFiles( filenames, path, "*.cpp" );
    AddFiles( filenames, path, "*.cc" );
    AddFiles( filenames, path, "*.c" );

    #ifdef __GNUC__
    // gcc / cygwin..
    glob_t glob_results;
    glob("*", GLOB_MARK, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        const char *dirname = glob_results.gl_pathv[i];
        if ( dirname[0] == '.' )
            continue;

        if ( strchr(dirname, '/') == 0 )
            continue;

        chdir( dirname );
        std::ostringstream curdir;
        curdir << path << dirname;
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    globfree(&glob_results);
    #endif
    #ifdef __BORLANDC__
    struct ffblk f ;
    for ( int done = findfirst("*", &f, FA_DIREC); ! done; done = findnext(&f) )
    {
        if ( f.ff_attrib != FA_DIREC || f.ff_name[0] == '.' )
            continue;
        chdir( f.ff_name );
        std::ostringstream curdir;
        curdir << path << f.ff_name << "/";
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    findclose(&f);
    #endif
    #ifdef _MSC_VER
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
				RecursiveAddFiles( filenames, curdir.str().c_str() );
				SetCurrentDirectory( ".." );
			}
		}
		while (FindNextFile(hFind, &ffd) != 0);
	}
	#endif
}

//---------------------------------------------------------------------------
// Main function of cppcheck
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;

    bool Recursive = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i],"--debug") == 0)
            Debug = true;

        // Show all messages
        else if (strcmp(argv[i],"--all") == 0)
            ShowAll = true;

        // Checking coding style.
        else if (strcmp(argv[i],"--style")==0)
            CheckCodingStyle = true;

        else if (strcmp(argv[i],"--recursive")==0)
            Recursive = true;

        else if (strchr(argv[i],'*'))
        {
            AddFiles( filenames, "", argv[i] );
        }

        else
        {
            filenames.push_back( argv[i] );
        }
    }

    // No filename given.. automaticly search for available files.
    if ( Recursive )
        RecursiveAddFiles( filenames, "" );

    if (filenames.empty())
    {
        std::cout << "C/C++ code checking.\n"
                     "\n"
                     "Syntax:\n"
                     "    cppcheck [--all] [--style] [--recursive] [filename1] [filename2]\n"
                     "\n"
                     "Options:\n"
                     "    --all    Normally a message is only shown if cppcheck is sure\n"
                     "             it has found a bug.\n"
                     "             When this option is given, all messages are shown.\n"
                     "\n"
                     "    --style  Check coding style.\n"
                     "    --recursive  Recursively check all *.cpp, *.cc and *.c files\n";
        return 0;
    }

    std::sort( filenames.begin(), filenames.end() );

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        errout.str("");
        CppCheck(filenames[c].c_str(), c);
        std::cerr << errout.str();
    }

    // This generates false positives - especially for libraries
    if ( ShowAll && CheckCodingStyle && filenames.size() > 1 )
    {
        errout.str("");
        std::cout << "Checking usage of global functions (this may take several minutes)..\n";
        CheckGlobalFunctionUsage(filenames);
        if ( ! errout.str().empty() )
        {
            std::cerr << "\n";
            std::cerr << errout.str();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId)
{
    OnlyReportUniqueErrors = true;

    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);

    FillFunctionList(FileId);

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckMemset();


    // Check for unsigned divisions where one operand is signed
    // Very important to run it before 'SimplifyTokenList'
    CheckUnsignedDivision();

    // Give warning when using char variable as array index
    // Doesn't work on simplified token list ('unsigned')
    if ( ShowAll )
        CheckCharVariable();


    // Including header which is not needed (too many false positives)
    //if ( CheckCodingStyle )
    //    WarningIncludeHeader();


    SimplifyTokenList();

    // Memory leak
    CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrun();

    // Check that all class constructors are ok.
    CheckConstructors();

    if (ShowAll)
    {
        // Check for "if (a=b)"
        CheckIfAssignment();

        // Check for case without break
        // Disabled because it generates many false positives
        // CheckCaseWithoutBreak();

        // Dangerous usage of strtok
        // Disabled because it generates false positives
        //WarningStrTok();
    }



    // Dangerous functions, such as 'gets' and 'scanf'
    WarningDangerousFunctions();


    // Invalid function usage..
    InvalidFunctionUsage();


    if (CheckCodingStyle)
    {
        // Check that all private functions are called.
        CheckUnusedPrivateFunctions();

        // Warning upon c-style pointer casts
        const char *ext = strrchr(FileName, '.');
        if (ext && strcmp(ext,".cpp")==0)
            WarningOldStylePointerCast();

        // Use standard functions instead
        WarningIsDigit();
        WarningIsAlpha();

        CheckOperatorEq1();

        // if (a) delete a;
        WarningRedundantCode();

        // if (condition);
        WarningIf();

        // Variable scope (check if the scope could be limited)
        //CheckVariableScope();

        // Check if a constant function parameter is passed by value
        CheckConstantFunctionParameter();

        // Unused struct members..
        CheckStructMemberUsage();

        // Check for various types of incomplete statements that could for example
        // mean that an ';' has been added by accident
        CheckIncompleteStatement();
    }

    // Clean up tokens..
    DeallocateTokens();

    if ( errout.str().empty() )
        std::cout << "No errors found\n";
}
//---------------------------------------------------------------------------




