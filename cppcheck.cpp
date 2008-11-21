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
#include "cppcheck.h"

#include "preprocessor.h" // preprocessor.

#include "CommonCheck.h"
#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"
#include "CheckOther.h"
#include "FileLister.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <map>

//---------------------------------------------------------------------------

CppCheck::CppCheck() : _tokenizer( this )
{
    _settings._debug = false;
    _settings._showAll = false;
    _settings._checkCodingStyle = false;
    _settings._errorsOnly = false;
}

CppCheck::~CppCheck()
{

}

void CppCheck::check(int argc, char* argv[])
{
    std::vector<std::string> pathnames;
    bool Recursive = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i],"--debug") == 0)
            _settings._debug = true;

        // Show all messages
        else if (strcmp(argv[i],"--all") == 0)
            _settings._showAll = true;

        // Checking coding style.
        else if (strcmp(argv[i],"--style")==0)
            _settings._checkCodingStyle = true;

        // Only print something when there are errors
        else if (strcmp(argv[i],"--errorsonly")==0)
            _settings._errorsOnly = true;

        else if (strcmp(argv[i],"--recursive")==0)
            Recursive = true;
        else
            pathnames.push_back( argv[i] );
    }

    _tokenizer.settings( _settings );

    std::vector<std::string> filenames;
    // --recursive was used
    if ( Recursive )
    {
        if( pathnames.size() == 0 )
        {
            // Handle situation: cppcheck --recursive
            FileLister::RecursiveAddFiles( filenames, "", true );
        }
        else
        {
            // Handle situation: cppcheck --recursive path1 path2

            // Execute RecursiveAddFiles() to each given file parameter
            std::vector<std::string>::const_iterator iter;
            for(iter=pathnames.begin(); iter!=pathnames.end(); iter++)
                FileLister::RecursiveAddFiles( filenames, iter->c_str(), true );
        }
    }
    else
    {
        std::vector<std::string>::const_iterator iter;
        for(iter=pathnames.begin(); iter!=pathnames.end(); iter++)
            FileLister::RecursiveAddFiles( filenames, iter->c_str(), false );
    }



    if (filenames.empty())
    {
        std::cout << "C/C++ code checking.\n"
                     "\n"
                     "Syntax:\n"
                     "    cppcheck [--all] [--style] [--errorsonly] [--recursive] [filename1] [filename2]\n"
                     "\n"
                     "Options:\n"
                     "    --all           Normally a message is only shown if cppcheck is sure\n"
                     "                    it has found a bug.\n"
                     "                    When this option is given, all messages are shown.\n"
                     "\n"
                     "    --style         Check coding style.\n"
                     "    --errorsonly    Only print something when there is an error\n"
                     "    --recursive     Recursively check all *.cpp, *.cc and *.c files\n";
        return;
    }

    std::sort( filenames.begin(), filenames.end() );

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        errout.str("");
        std::string fname = filenames[c];

        // If only errors are printed, print filename after the check
        if (!_settings._errorsOnly)
            std::cout << "Checking " << fname << "...\n";

        std::ifstream fin( fname.c_str() );
        std::map<std::string, std::string> code;
        Preprocessor preprocessor( this );
        preprocessor.preprocess(fin, code, fname);
        for ( std::map<std::string,std::string>::const_iterator it = code.begin(); it != code.end(); ++it )
            checkFile(it->second, filenames[c].c_str(), c);

        if (_settings._errorsOnly)
        {
            if ( !errout.str().empty() )
            {
                std::cout << "Errors found in " << fname << ":\n";
                std::cerr << errout.str();
            }
        }
        else
        {
            if ( errout.str().empty() )
                std::cout << "No errors found\n";
            else
                std::cerr << errout.str();
        }
    }

    // This generates false positives - especially for libraries
    if ( _settings._showAll && _settings._checkCodingStyle && filenames.size() > 1 )
    {
        errout.str("");
        std::cout << "Checking usage of global functions (this may take several minutes)..\n";
        _tokenizer.CheckGlobalFunctionUsage(filenames);
        if ( ! errout.str().empty() )
        {
            std::cerr << "\n";
            std::cerr << errout.str();
        }
    }

}


//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

void CppCheck::checkFile(const std::string &code, const char FileName[], unsigned int FileId)
{
    // Tokenize the file
    {
    std::istringstream istr(code);
    _tokenizer.Tokenize(istr, FileName);
    }

    _tokenizer.FillFunctionList(FileId);

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckClass checkClass( &_tokenizer, _settings, this );
    checkClass.CheckMemset();


    // Check for unsigned divisions where one operand is signed
    // Very important to run it before 'SimplifyTokenList'
    CheckOther checkOther( &_tokenizer, this );
    if ( _settings._checkCodingStyle )
        checkOther.CheckUnsignedDivision();

    // Give warning when using char variable as array index
    // Doesn't work on simplified token list ('unsigned')
    if ( _settings._checkCodingStyle )
        checkOther.CheckCharVariable();


    // Including header which is not needed (too many false positives)
//    if ( _settings._checkCodingStyle )
//    {
//        CheckHeaders checkHeaders( &tokenizer );
//        checkHeaders.WarningIncludeHeader();
//    }


    _tokenizer.SimplifyTokenList();

    // Memory leak
    CheckMemoryLeakClass checkMemoryLeak( &_tokenizer, _settings, this );
    checkMemoryLeak.CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrunClass checkBufferOverrun( &_tokenizer, this );
    checkBufferOverrun.CheckBufferOverrun();

    // Check that all class constructors are ok.
    checkClass.CheckConstructors();

    if (_settings._showAll)
    {
        // Check for "if (a=b)"
        checkOther.CheckIfAssignment();

        // Check for case without break
        // Disabled because it generates many false positives
        // CheckCaseWithoutBreak();

        // Dangerous usage of strtok
        // Disabled because it generates false positives
        //WarningStrTok();
    }



    // Dangerous functions, such as 'gets' and 'scanf'
    checkBufferOverrun.WarningDangerousFunctions();


    // Invalid function usage..
    checkOther.InvalidFunctionUsage();


    if (_settings._checkCodingStyle)
    {
        // Check that all private functions are called.
        checkClass.CheckUnusedPrivateFunctions();

        // Warning upon c-style pointer casts
        const char *ext = strrchr(FileName, '.');
        if (ext && strcmp(ext,".cpp")==0)
            checkOther.WarningOldStylePointerCast();

        // Use standard functions instead
        checkOther.WarningIsDigit();
        checkOther.WarningIsAlpha();

        checkClass.CheckOperatorEq1();

        // if (a) delete a;
        checkOther.WarningRedundantCode();

        // if (condition);
        checkOther.WarningIf();

        // Variable scope (check if the scope could be limited)
        //CheckVariableScope();

        // Check if a constant function parameter is passed by value
        checkOther.CheckConstantFunctionParameter();

        // Unused struct members..
        checkOther.CheckStructMemberUsage();

        // Check for various types of incomplete statements that could for example
        // mean that an ';' has been added by accident
        checkOther.CheckIncompleteStatement();
    }

    // Clean up tokens..
    _tokenizer.DeallocateTokens();

}
//---------------------------------------------------------------------------

void CppCheck::reportErr( const std::string &errmsg)
{
    if ( /*OnlyReportUniqueErrors*/ true )
    {
        if ( std::find( _errorList.begin(), _errorList.end(), errmsg ) != _errorList.end() )
            return;
        _errorList.push_back( errmsg );
    }
    errout << errmsg << std::endl;
}

void CppCheck::reportErr( const TOKEN *token, const std::string &errmsg)
{
    std::string message = _tokenizer.fileLine( token ) + errmsg;
    reportErr( message );
}
