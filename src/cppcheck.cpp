/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#include "tokenize.h"   // <- Tokenizer

#include "checkmemoryleak.h"
#include "checkbufferoverrun.h"
#include "checkclass.h"
#include "checkheaders.h"
#include "checkother.h"
#include "checkfunctionusage.h"
#include "filelister.h"

#include "errormessage.h"

#include <algorithm>
#include <sstream>
#include <cstring>
#include <fstream>
#include <map>

//---------------------------------------------------------------------------

CppCheck::CppCheck(ErrorLogger &errorLogger)
{
    _errorLogger = &errorLogger;
}

CppCheck::~CppCheck()
{

}

void CppCheck::settings(const Settings &settings)
{
    _settings = settings;
}

void CppCheck::addFile(const std::string &path)
{
    _filenames.push_back(path);
}

void CppCheck::addFile(const std::string &path, const std::string &content)
{
    _filenames.push_back(path);
    _fileContents[ path ] = content;
}

std::string CppCheck::parseFromArgs(int argc, const char* const argv[])
{
    std::vector<std::string> pathnames;

    for (int i = 1; i < argc; i++)
    {
        // Flag used for various purposes during debugging
        if (strcmp(argv[i], "--debug") == 0)
            _settings._debug = true;

        // Show all messages
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
            _settings._showAll = true;

        // Only print something when there are errors
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            _settings._errorsOnly = true;

        // Checking coding style
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0)
            _settings._checkCodingStyle = true;

        // Verbose error messages (configuration info)
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            _settings._verbose = true;

        // Force checking of files that have "too many" configurations
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0)
            _settings._force = true;

        else
            pathnames.push_back(argv[i]);
    }

    if (pathnames.size() > 0)
    {
        // Execute RecursiveAddFiles() to each given file parameter
        std::vector<std::string>::const_iterator iter;
        for (iter = pathnames.begin(); iter != pathnames.end(); iter++)
            FileLister::RecursiveAddFiles(_filenames, iter->c_str(), true);
    }

    if (_filenames.empty())
    {
        std::ostringstream oss;
        oss <<   "cppcheck 1.27\n"
        "\n"
        "C/C++ code checking\n"
        "\n"
        "Syntax:\n"
        "    cppcheck [--all] [--force] [--quiet] [--style] [--verbose] [file or path1] [file or path]\n"
        "\n"
        "If path is given instead of filename, *.cpp, *.cxx, *.cc and *.c files are \n"
        "checked recursively from given directory.\n\n"
        "Options:\n"
        "    -a, --all        Make the checking more sensitive. More bugs are detected,\n"
        "                     but there are also more false positives\n"
        "    -f, --force      Force checking on files that have \"too many\" configurations\n"
        "    -q, --quiet      Only print error messages\n"
        "    -s, --style      Check coding style\n"
        "    -v, --verbose    More detailed error reports\n"
        "\n"
        "Example usage:\n"
        "  # Recursively check the current folder. Print the progress on the screen and write errors in a file:\n"
        "    cppcheck . 2> err.txt\n"
        "  # Recursively check ../myproject/ and print only most fatal errors:\n"
        "    cppcheck --quiet ../myproject/\n"
        "  # Check only files one.cpp and two.cpp and give all information there is:\n"
        "    cppcheck -v -a -s one.cpp two.cpp\n";
        return oss.str();
    }

    // Check function usage if "--style" and "--all" was given.
    // There will be false positives for exported library functions
    if (_settings._showAll && _settings._checkCodingStyle)
        _settings._checkFunctionUsage = true;

    return "";
}

unsigned int CppCheck::check()
{
    _checkFunctionUsage.setErrorLogger(this);
    std::sort(_filenames.begin(), _filenames.end());
    for (unsigned int c = 0; c < _filenames.size(); c++)
    {
        _errout.str("");
        std::string fname = _filenames[c];


        Preprocessor preprocessor;
        std::list<std::string> configurations;
        std::string filedata = "";
        if (_fileContents.size() > 0 && _fileContents.find(_filenames[c]) != _fileContents.end())
        {
            // File content was given as a string
            std::istringstream iss(_fileContents[ _filenames[c] ]);
            preprocessor.preprocess(iss, filedata, configurations);
        }
        else
        {
            // Only file name was given, read the content from file
            std::ifstream fin(fname.c_str());
            preprocessor.preprocess(fin, filedata, configurations);
        }

        int checkCount = 0;
        for (std::list<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it)
        {
            // Check only 12 first configurations, after that bail out, unless --force
            // was used.
            if (!_settings._force && checkCount > 11)
            {
                if (_settings._errorsOnly == false)
                    _errorLogger->reportOut(std::string("Bailing out from checking ") + fname + ": Too many configurations. Recheck this file with --force if you want to check them all.");

                break;
            }

            cfg = *it;
            std::string codeWithoutCfg = Preprocessor::getcode(filedata, *it);

            // If only errors are printed, print filename after the check
            if (_settings._errorsOnly == false)
                _errorLogger->reportOut(std::string("Checking ") + fname + ": " + cfg + std::string("..."));

            checkFile(codeWithoutCfg, _filenames[c].c_str());
            ++checkCount;
        }

        if (_settings._errorsOnly == false && _errout.str().empty())
            _errorLogger->reportOut("No errors found");
    }

    // This generates false positives - especially for libraries
    _settings._verbose = false;
    if (_settings._checkFunctionUsage)
    {
        _errout.str("");
        if (_settings._errorsOnly == false)
            _errorLogger->reportOut("Checking usage of global functions (this may take several minutes)..");

        _checkFunctionUsage.check();
    }

    unsigned int result = _errorList.size();
    _errorList.clear();
    return result;
}


//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

void CppCheck::checkFile(const std::string &code, const char FileName[])
{
    Tokenizer _tokenizer;

    // Tokenize the file
    {
        std::istringstream istr(code);
        _tokenizer.tokenize(istr, FileName);
    }

    // Set variable id
    _tokenizer.setVarId();

    _tokenizer.fillFunctionList();

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckClass checkClass(&_tokenizer, _settings, this);
    checkClass.noMemset();


    // Coding style checks that must be run before the simplifyTokenList
    CheckOther checkOther(&_tokenizer, this);
    if (_settings._checkCodingStyle)
    {
        // Check for unsigned divisions where one operand is signed
        checkOther.CheckUnsignedDivision();

        // Give warning when using char variable as array index
        checkOther.CheckCharVariable();

        // Usage of local variables
        checkOther.functionVariableUsage();
    }


    _tokenizer.simplifyTokenList();


    if (_settings._checkFunctionUsage)
        _checkFunctionUsage.parseTokens(_tokenizer);

    // Class for detecting buffer overruns and related problems
    CheckBufferOverrunClass checkBufferOverrun(&_tokenizer, _settings, this);

    // Memory leak
    CheckMemoryLeakClass checkMemoryLeak(&_tokenizer, _settings, this);
    if (ErrorMessage::memleak(_settings))
        checkMemoryLeak.CheckMemoryLeak();

    // Check that all class constructors are ok.
    if (ErrorMessage::noConstructor(_settings) || ErrorMessage::uninitVar(_settings))
        checkClass.constructors();

    // Check that all base classes have virtual destructors
    checkClass.virtualDestructor();

    if (_settings._showAll)
    {
        // Buffer overruns..
        checkBufferOverrun.bufferOverrun();

        // Check for "if (a=b)"
        checkOther.CheckIfAssignment();
    }

    // Dangerous functions, such as 'gets' and 'scanf'
    checkBufferOverrun.dangerousFunctions();

    // Warning upon c-style pointer casts
    if (ErrorMessage::cstyleCast(_settings))
    {
        const char *ext = strrchr(FileName, '.');
        if (ext && strcmp(ext, ".cpp") == 0)
            checkOther.WarningOldStylePointerCast();
    }

    // if (a) delete a;
    if (ErrorMessage::redundantIfDelete0(_settings))
        checkOther.WarningRedundantCode();

    // strtol and strtoul usage
    if (ErrorMessage::dangerousUsageStrtol(_settings))
        checkOther.InvalidFunctionUsage();

    // Check that all private functions are called.
    if (ErrorMessage::unusedPrivateFunction(_settings))
        checkClass.privateFunctions();



    if (_settings._checkCodingStyle)
    {
        checkClass.operatorEq();

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

        // Unreachable code below a 'return' statement
        checkOther.unreachableCode();
    }
}
//---------------------------------------------------------------------------

void CppCheck::reportErr(const std::string &errmsg)
{
    if (/*OnlyReportUniqueErrors*/ true)
    {
        if (std::find(_errorList.begin(), _errorList.end(), errmsg) != _errorList.end())
            return;
        _errorList.push_back(errmsg);
    }

    std::string errmsg2(errmsg);
    if (_settings._verbose)
    {
        errmsg2 += "\n    Defines=\'" + cfg + "\'\n";
    }


    _errorLogger->reportErr(errmsg2);

    _errout << errmsg2 << std::endl;
}

void CppCheck::reportOut(const std::string &outmsg)
{
    // This is currently never called. It is here just to comply with
    // the interface.
}
