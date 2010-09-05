/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cppcheck.h"

#include "preprocessor.h" // preprocessor.
#include "tokenize.h"   // <- Tokenizer

#include "filelister.h"

#include "check.h"
#include "path.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <ctime>
#include "timer.h"

static TimerResults S_timerResults;

CppCheck::CppCheck(ErrorLogger &errorLogger)
    : _errorLogger(errorLogger)
{
    exitcode = 0;
}

CppCheck::~CppCheck()
{
    if (_settings._showtime != SHOWTIME_NONE)
        S_timerResults.ShowResults();
}

void CppCheck::settings(const Settings &currentSettings)
{
    _settings = currentSettings;
}

void CppCheck::addFile(const std::string &path)
{
    getFileLister()->recursiveAddFiles(_filenames, path.c_str(), true);
}

void CppCheck::addFile(const std::string &path, const std::string &content)
{
    _filenames.push_back(path);
    _fileContents[ path ] = content;
}

void CppCheck::clearFiles()
{
    _filenames.clear();
    _fileContents.clear();
}

const char * CppCheck::version()
{
    return "1.44";
}

unsigned int CppCheck::check()
{
    exitcode = 0;

    std::sort(_filenames.begin(), _filenames.end());

    // TODO: Should this be moved out to its own function so all the files can be
    // analysed before any files are checked?
    if (_settings.test_2_pass && _settings._jobs == 1)
    {
        for (unsigned int c = 0; c < _filenames.size(); c++)
        {
            const std::string fname = _filenames[c];
            if (_settings.terminated())
                break;

            std::string fixedname = Path::toNativeSeparators(fname);
            reportOut("Analysing " + fixedname + "..");

            std::ifstream f(fname.c_str());
            analyseFile(f, fname);
        }
    }

    for (unsigned int c = 0; c < _filenames.size(); c++)
    {
        _errout.str("");
        const std::string fname = _filenames[c];

        if (_settings.terminated())
            break;

        if (_settings._errorsOnly == false)
        {
            std::string fixedpath(fname);
            fixedpath = Path::simplifyPath(fixedpath);
            fixedpath = Path::toNativeSeparators(fixedpath);
            _errorLogger.reportOut(std::string("Checking ") + fixedpath + std::string("..."));
        }

        try
        {
            Preprocessor preprocessor(&_settings, this);
            std::list<std::string> configurations;
            std::string filedata = "";

            if (_fileContents.size() > 0 && _fileContents.find(_filenames[c]) != _fileContents.end())
            {
                // File content was given as a string
                std::istringstream iss(_fileContents[ _filenames[c] ]);
                preprocessor.preprocess(iss, filedata, configurations, fname, _settings._includePaths);
            }
            else
            {
                // Only file name was given, read the content from file
                std::ifstream fin(fname.c_str());
                Timer t("Preprocessor::preprocess", _settings._showtime, &S_timerResults);
                preprocessor.preprocess(fin, filedata, configurations, fname, _settings._includePaths);
            }

            _settings.ifcfg = bool(configurations.size() > 1);

            if (!_settings.userDefines.empty())
            {
                configurations.clear();
                configurations.push_back(_settings.userDefines);
            }

            int checkCount = 0;
            for (std::list<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it)
            {
                // Check only 12 first configurations, after that bail out, unless --force
                // was used.
                if (!_settings._force && checkCount > 11)
                {
                    if (_settings._errorsOnly == false)
                    {
                        const std::string fixedpath = Path::toNativeSeparators(fname);
                        _errorLogger.reportOut(std::string("Bailing out from checking ") + fixedpath +
                                               ": Too many configurations. Recheck this file with --force if you want to check them all.");
                    }

                    break;
                }

                cfg = *it;
                Timer t("Preprocessor::getcode", _settings._showtime, &S_timerResults);
                const std::string codeWithoutCfg = Preprocessor::getcode(filedata, *it, fname, &_errorLogger);
                t.Stop();

                // If only errors are printed, print filename after the check
                if (_settings._errorsOnly == false && it != configurations.begin())
                {
                    std::string fixedpath = Path::simplifyPath(fname);
                    fixedpath = Path::toNativeSeparators(fixedpath);
                    _errorLogger.reportOut(std::string("Checking ") + fixedpath + ": " + cfg + std::string("..."));
                }

                std::string appendCode = _settings.append();
                if (!appendCode.empty())
                    Preprocessor::preprocessWhitespaces(appendCode);

                checkFile(codeWithoutCfg + appendCode, _filenames[c].c_str());
                ++checkCount;
            }
        }
        catch (std::runtime_error &e)
        {
            // Exception was thrown when checking this file..
            const std::string fixedpath = Path::toNativeSeparators(fname);
            _errorLogger.reportOut("Bailing out from checking " + fixedpath + ": " + e.what());
        }

        _errorLogger.reportStatus(c + 1, (unsigned int)_filenames.size());
    }

    // This generates false positives - especially for libraries
    const bool verbose_orig = _settings._verbose;
    _settings._verbose = false;
    if (_settings.isEnabled("unusedFunctions") && _settings._jobs == 1)
    {
        _errout.str("");
        if (_settings._errorsOnly == false)
            _errorLogger.reportOut("Checking usage of global functions..");

        _checkUnusedFunctions.check(this);
    }
    _settings._verbose = verbose_orig;

    _errorList.clear();
    return exitcode;
}

void CppCheck::analyseFile(std::istream &fin, const std::string &filename)
{
    // Preprocess file..
    Preprocessor preprocessor(&_settings, this);
    std::list<std::string> configurations;
    std::string filedata = "";
    preprocessor.preprocess(fin, filedata, configurations, filename, _settings._includePaths);
    const std::string code = Preprocessor::getcode(filedata, "", filename, &_errorLogger);

    // Tokenize..
    Tokenizer tokenizer(&_settings, this);
    std::istringstream istr(code);
    tokenizer.tokenize(istr, filename.c_str(), "");
    tokenizer.simplifyTokenList();

    // Analyse the tokens..
    std::set<std::string> data;
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
    {
        (*it)->analyse(tokenizer.tokens(), data);
    }

    // Save analysis results..
    // TODO: This loop should be protected by a mutex or something like that
    //       The saveAnalysisData must _not_ be called from many threads at the same time.
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
    {
        (*it)->saveAnalysisData(data);
    }
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

void CppCheck::checkFile(const std::string &code, const char FileName[])
{
    if (_settings.terminated())
        return;

    Tokenizer _tokenizer(&_settings, this);
    bool result;

    // Tokenize the file
    std::istringstream istr(code);

    Timer timer("Tokenizer::tokenize", _settings._showtime, &S_timerResults);
    result = _tokenizer.tokenize(istr, FileName, cfg);
    timer.Stop();
    if (!result)
    {
        // File had syntax errors, abort
        return;
    }

    Timer timer2("Tokenizer::fillFunctionList", _settings._showtime, &S_timerResults);
    _tokenizer.fillFunctionList();
    timer2.Stop();

    // call all "runChecks" in all registered Check classes
    for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
    {
        if (_settings.terminated())
            return;

        Timer timerRunChecks((*it)->name() + "::runChecks", _settings._showtime, &S_timerResults);
        (*it)->runChecks(&_tokenizer, &_settings, this);
    }

    Timer timer3("Tokenizer::simplifyTokenList", _settings._showtime, &S_timerResults);
    result = _tokenizer.simplifyTokenList();
    timer3.Stop();
    if (!result)
        return;

    Timer timer4("Tokenizer::fillFunctionList", _settings._showtime, &S_timerResults);
    _tokenizer.fillFunctionList();
    timer4.Stop();

    if (_settings.isEnabled("unusedFunctions") && _settings._jobs == 1)
        _checkUnusedFunctions.parseTokens(_tokenizer);

    // call all "runSimplifiedChecks" in all registered Check classes
    for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
    {
        if (_settings.terminated())
            return;

        Timer timerSimpleChecks((*it)->name() + "::runSimplifiedChecks", _settings._showtime, &S_timerResults);
        (*it)->runSimplifiedChecks(&_tokenizer, &_settings, this);
    }
}

Settings CppCheck::settings() const
{
    return _settings;
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    std::string errmsg = msg.toString();

    // Alert only about unique errors
    if (std::find(_errorList.begin(), _errorList.end(), errmsg) != _errorList.end())
        return;

    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty())
    {
        file = msg._callStack.back().getfile();
        line = msg._callStack.back().line;
    }

    if (_settings.nomsg.isSuppressed(msg._id, file, line))
        return;

    if (!_settings.nofail.isSuppressed(msg._id, file, line))
        exitcode = 1;

    _errorList.push_back(errmsg);
    std::string errmsg2(errmsg);
    if (_settings._verbose)
    {
        errmsg2 += "\n    Defines=\'" + cfg + "\'\n";
    }

    _errorLogger.reportErr(msg);

    _errout << errmsg2 << std::endl;
}

void CppCheck::reportOut(const std::string &outmsg)
{
    _errorLogger.reportOut(outmsg);
}

const std::vector<std::string> &CppCheck::filenames() const
{
    return _filenames;
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const unsigned int value)
{
    _errorLogger.reportProgress(filename, stage, value);
}

void CppCheck::reportStatus(unsigned int /*index*/, unsigned int /*max*/)
{

}

void CppCheck::getErrorMessages()
{
    // call all "getErrorMessages" in all registered Check classes
    std::cout << ErrorLogger::ErrorMessage::getXMLHeader();
    for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
    {
        (*it)->getErrorMessages();
    }

    Tokenizer tokenizer(&_settings, 0);
    tokenizer.getErrorMessages();

    Preprocessor::getErrorMessages(std::cout);

    std::cout << ErrorLogger::ErrorMessage::getXMLFooter() << std::endl;
}
