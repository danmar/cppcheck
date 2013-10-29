/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include "cppcheckexecutor.h"
#include "cmdlineparser.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "filelister.h"
#include "path.h"
#include "pathmatch.h"
#include "preprocessor.h"
#include "threadexecutor.h"
#include <iostream>
#include <sstream>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <cstring>
#include <algorithm>
#include <climits>

CppCheckExecutor::CppCheckExecutor()
    : _settings(0), time1(0), errorlist(false)
{
}

CppCheckExecutor::~CppCheckExecutor()
{
}

bool CppCheckExecutor::parseFromArgs(CppCheck *cppcheck, int argc, const char* const argv[])
{
    Settings& settings = cppcheck->settings();
    CmdLineParser parser(&settings);
    bool success = parser.ParseFromArgs(argc, argv);

    if (success) {
        if (parser.GetShowVersion() && !parser.GetShowErrorMessages()) {
            const char * extraVersion = cppcheck->extraVersion();
            if (*extraVersion != 0)
                std::cout << "Cppcheck " << cppcheck->version() << " ("
                          << extraVersion << ')' << std::endl;
            else
                std::cout << "Cppcheck " << cppcheck->version() << std::endl;
        }

        if (parser.GetShowErrorMessages()) {
            errorlist = true;
            std::cout << ErrorLogger::ErrorMessage::getXMLHeader(settings._xml_version);
            cppcheck->getErrorMessages();
            std::cout << ErrorLogger::ErrorMessage::getXMLFooter(settings._xml_version) << std::endl;
        }

        if (parser.ExitAfterPrinting())
            std::exit(EXIT_SUCCESS);
    } else {
        std::exit(EXIT_FAILURE);
    }

    // Check that all include paths exist
    {
        std::list<std::string>::iterator iter;
        for (iter = settings._includePaths.begin();
             iter != settings._includePaths.end();
            ) {
            const std::string path(Path::toNativeSeparators(*iter));
            if (FileLister::isDirectory(path))
                ++iter;
            else {
                // If the include path is not found, warn user and remove the
                // non-existing path from the list.
                std::cout << "cppcheck: warning: Couldn't find path given by -I '" << path << '\'' << std::endl;
                iter = settings._includePaths.erase(iter);
            }
        }
    }

    const std::vector<std::string>& pathnames = parser.GetPathNames();

    if (!pathnames.empty()) {
        // Execute recursiveAddFiles() to each given file parameter
        std::vector<std::string>::const_iterator iter;
        for (iter = pathnames.begin(); iter != pathnames.end(); ++iter)
            FileLister::recursiveAddFiles(_files, Path::toNativeSeparators(*iter), _settings->library.markupExtensions());
    }

    if (!_files.empty()) {
        // Remove header files from the list of ignored files.
        // Also output a warning for the user.
        // TODO: Remove all unknown files? (use FileLister::acceptFile())
        bool warn = false;
        std::vector<std::string> ignored = parser.GetIgnoredPaths();
        for (std::vector<std::string>::iterator i = ignored.begin(); i != ignored.end();) {
            const std::string extension = Path::getFilenameExtension(*i);
            if (extension == ".h" || extension == ".hpp") {
                i = ignored.erase(i);
                warn = true;
            } else
                ++i;
        }
        if (warn) {
            std::cout << "cppcheck: filename exclusion does not apply to header (.h and .hpp) files." << std::endl;
            std::cout << "cppcheck: Please use --suppress for ignoring results from the header files." << std::endl;
        }

#if defined(_WIN32)
        // For Windows we want case-insensitive path matching
        const bool caseSensitive = false;
#else
        const bool caseSensitive = true;
#endif
        PathMatch matcher(parser.GetIgnoredPaths(), caseSensitive);
        for (std::map<std::string, std::size_t>::iterator i = _files.begin(); i != _files.end();) {
            if (matcher.Match(i->first))
                _files.erase(i++);
            else
                ++i;
        }
    } else {
        std::cout << "cppcheck: error: could not find or open any of the paths given." << std::endl;
        return false;
    }

    if (!_files.empty()) {
        return true;
    } else {
        std::cout << "cppcheck: error: no files to check - all paths ignored." << std::endl;
        return false;
    }
}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    Preprocessor::missingIncludeFlag = false;

    CppCheck cppCheck(*this, true);

    Settings& settings = cppCheck.settings();
    _settings = &settings;

    settings.library.load(argv[0], "std");

    if (!parseFromArgs(&cppCheck, argc, argv)) {
        return EXIT_FAILURE;
    }

    if (settings.reportProgress)
        time1 = std::time(0);

    if (settings._xml) {
        reportErr(ErrorLogger::ErrorMessage::getXMLHeader(settings._xml_version));
    }

    unsigned int returnValue = 0;
    if (settings._jobs == 1) {
        // Single process

        std::size_t totalfilesize = 0;
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
            totalfilesize += i->second;
        }

        std::size_t processedsize = 0;
        unsigned int c = 0;
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
            if (!_settings->library.markupFile(i->first)) {
                returnValue += cppCheck.check(i->first);
                processedsize += i->second;
                if (!settings._errorsOnly)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }

        // second loop to parse all markup files which may not work until all
        // c/cpp files have been parsed and checked
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
            if (_settings->library.markupFile(i->first)) {
                returnValue += cppCheck.check(i->first);
                processedsize += i->second;
                if (!settings._errorsOnly)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }

        cppCheck.checkFunctionUsage();
    } else if (!ThreadExecutor::isEnabled()) {
        std::cout << "No thread support yet implemented for this platform." << std::endl;
    } else {
        // Multiple processes
        ThreadExecutor executor(_files, settings, *this);
        returnValue = executor.check();
    }

    if (settings.isEnabled("information") || settings.checkConfiguration)
        reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions());

    if (!settings.checkConfiguration) {
        cppCheck.tooManyConfigsError("",0U);

        if (settings.isEnabled("missingInclude") && Preprocessor::missingIncludeFlag) {
            const std::list<ErrorLogger::ErrorMessage::FileLocation> callStack;
            ErrorLogger::ErrorMessage msg(callStack,
                                          Severity::information,
                                          "Cppcheck cannot find all the include files (use --check-config for details)\n"
                                          "Cppcheck cannot find all the include files. Cppcheck can check the code without the "
                                          "include files found. But the results will probably be more accurate if all the include "
                                          "files are found. Please check your project's include directories and add all of them "
                                          "as include directories for Cppcheck. To see what files Cppcheck cannot find use "
                                          "--check-config.",
                                          "missingInclude",
                                          false);
            reportInfo(msg);
        }
    }

    if (settings._xml) {
        reportErr(ErrorLogger::ErrorMessage::getXMLFooter(settings._xml_version));
    }

    _settings = 0;
    if (returnValue)
        return settings._exitCode;
    else
        return 0;
}

void CppCheckExecutor::reportErr(const std::string &errmsg)
{
    // Alert only about unique errors
    if (_errorList.find(errmsg) != _errorList.end())
        return;

    _errorList.insert(errmsg);
    std::cerr << errmsg << std::endl;
}

void CppCheckExecutor::reportOut(const std::string &outmsg)
{
    std::cout << outmsg << std::endl;
}

void CppCheckExecutor::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    (void)filename;

    if (!time1)
        return;

    // Report progress messages every 10 seconds
    const std::time_t time2 = std::time(NULL);
    if (time2 >= (time1 + 10)) {
        time1 = time2;

        // format a progress message
        std::ostringstream ostr;
        ostr << "progress: "
             << stage
             << ' ' << value << '%';

        // Report progress message
        reportOut(ostr.str());
    }
}

void CppCheckExecutor::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    reportErr(msg);
}

void CppCheckExecutor::reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal)
{
    if (filecount > 1) {
        std::ostringstream oss;
        oss << fileindex << '/' << filecount
            << " files checked " <<
            (sizetotal > 0 ? static_cast<long>(static_cast<long double>(sizedone) / sizetotal*100) : 0)
            << "% done";
        std::cout << oss.str() << std::endl;
    }
}

void CppCheckExecutor::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    if (errorlist) {
        reportOut(msg.toXML(false, _settings->_xml_version));
    } else if (_settings->_xml) {
        reportErr(msg.toXML(_settings->_verbose, _settings->_xml_version));
    } else {
        reportErr(msg.toString(_settings->_verbose, _settings->_outputFormat));
    }
}
