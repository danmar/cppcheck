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

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include "cppcheck.h"
#include "timer.h"
#include "settings.h"
#include "cmdlineparser.h"


static void AddFilesToList(const std::string& FileList, std::vector<std::string>& PathNames)
{
    // to keep things initially simple, if the file can't be opened, just be
    // silent and move on
    // ideas : we could also require this should be an xml file, with the filenames
    // specified in an xml structure
    // we could elaborate this then, to also include the I-paths, ...
    // basically for everything that makes the command line very long
    // xml is a bonus then, since we can easily extend it
    // we need a good parser then -> suggestion : TinyXml
    // drawback : creates a dependency
    std::ifstream Files(FileList.c_str());
    if (Files)
    {
        std::string FileName;
        while (std::getline(Files, FileName)) // next line
        {
            if (!FileName.empty())
            {
                PathNames.push_back(FileName);
            }
        }
    }
}

CmdLineParser::CmdLineParser(Settings *settings)
    : _settings(settings)
    , _showHelp(false)
    , _showVersion(false)
    , _showErrorMessages(false)
{
}

void CmdLineParser::PrintMessage(const std::string &message)
{
    std::cout << message << std::endl;
}

bool CmdLineParser::ParseFromArgs(int argc, const char* const argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            _showVersion = true;
            return true;
        }
        // Flag used for various purposes during debugging
        else if (strcmp(argv[i], "--debug") == 0)
            _settings->debug = _settings->debugwarnings = true;

        // Show debug warnings
        else if (strcmp(argv[i], "--debug-warnings") == 0)
            _settings->debugwarnings = true;

        // Inconclusive checking - keep this for compatibility but don't
        // handle it
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
            ;

        // Checking coding style
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0)
        {
            const std::string errmsg = _settings->addEnabled("style");
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strcmp(argv[i], "--exitcode-suppressions") == 0)
        {
            ++i;

            if (i >= argc)
            {
                PrintMessage("cppcheck: No file specified for the --exitcode-suppressions option");
                return false;
            }

            std::ifstream f(argv[i]);
            if (!f.is_open())
            {
                PrintMessage("cppcheck: Couldn't open the file \"" + std::string(argv[i]) + "\"");
                return false;
            }
            const std::string errmsg(_settings->nofail.parseFile(f));
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strcmp(argv[i], "--suppressions") == 0)
        {
            ++i;

            if (i >= argc)
            {
                PrintMessage("cppcheck: No file specified for the --suppressions option");
                return false;
            }

            std::ifstream f(argv[i]);
            if (!f.is_open())
            {
                std::string message("cppcheck: Couldn't open the file \"");
                message += std::string(argv[i]);
                message += "\"";
                PrintMessage(message);
                return false;
            }
            const std::string errmsg(_settings->nomsg.parseFile(f));
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Enables inline suppressions.
        else if (strcmp(argv[i], "--inline-suppr") == 0)
            _settings->_inlineSuppressions = true;

        // Verbose error messages (configuration info)
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            _settings->_verbose = true;

        // Force checking of files that have "too many" configurations
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0)
            _settings->_force = true;

        // Write results in results.xml
        else if (strcmp(argv[i], "--xml") == 0)
            _settings->_xml = true;

        // Only print something when there are errors
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            _settings->_errorsOnly = true;

        // Check if there are unused functions
        else if (strcmp(argv[i], "--unused-functions") == 0)
        {
            const std::string errmsg = _settings->addEnabled("unusedFunctions");
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Append userdefined code to checked source code
        else if (strncmp(argv[i], "--append=", 9) == 0)
            _settings->append(9 + argv[i]);

        else if (strncmp(argv[i], "--enable=", 9) == 0)
        {
            const std::string errmsg = _settings->addEnabled(argv[i] + 9);
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // --error-exitcode=1
        else if (strncmp(argv[i], "--error-exitcode=", 17) == 0)
        {
            std::string temp = argv[i];
            temp = temp.substr(17);
            std::istringstream iss(temp);
            if (!(iss >> _settings->_exitCode))
            {
                _settings->_exitCode = 0;
                PrintMessage("cppcheck: Argument must be an integer. Try something like '--error-exitcode=1'");
                return false;
            }
        }

        // User define
        else if (strncmp(argv[i], "-D", 2) == 0)
        {
            if (!_settings->userDefines.empty())
                _settings->userDefines += ";";
            if (strcmp(argv[i], "-D") == 0)
                _settings->userDefines += argv[++i];
            else
                _settings->userDefines += 2 + argv[i];
        }

        // Include paths
        else if (strcmp(argv[i], "-I") == 0 || strncmp(argv[i], "-I", 2) == 0)
        {
            std::string path;

            // "-I path/"
            if (strcmp(argv[i], "-I") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    PrintMessage("cppcheck: argument to '-I' is missing");
                    return false;
                }
                path = argv[i];
            }

            // "-Ipath/"
            else
            {
                path = argv[i];
                path = path.substr(2);
            }

            // If path doesn't end with / or \, add it
            if (path[path.length()-1] != '/' && path[path.length()-1] != '\\')
                path += '/';

            _settings->_includePaths.push_back(path);
        }

        // file list specified
        else if (strncmp(argv[i], "--file-list=", 12) == 0)
        {
            // open this file and read every input file (1 file name per line)
            AddFilesToList(12 + argv[i], _pathnames);
        }

        // Report progress
        else if (strcmp(argv[i], "--report-progress") == 0)
        {
            _settings->reportProgress = true;
        }

        // Output formatter
        else if (strcmp(argv[i], "--template") == 0)
        {
            // "--template path/"
            ++i;
            if (i >= argc)
            {
                PrintMessage("cppcheck: argument to '--template' is missing");
                return false;
            }

            _settings->_outputFormat = argv[i];
            if (_settings->_outputFormat == "gcc")
                _settings->_outputFormat = "{file}:{line}: {severity}: {message}";
            else if (_settings->_outputFormat == "vs")
                _settings->_outputFormat = "{file}({line}): {severity}: {message}";
        }

        // Checking threads
        else if (strcmp(argv[i], "-j") == 0 ||
                 strncmp(argv[i], "-j", 2) == 0)
        {
            std::string numberString;

            // "-j 3"
            if (strcmp(argv[i], "-j") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    PrintMessage("cppcheck: argument to '-j' is missing");
                    return false;
                }

                numberString = argv[i];
            }

            // "-j3"
            else if (strncmp(argv[i], "-j", 2) == 0)
            {
                numberString = argv[i];
                numberString = numberString.substr(2);
            }

            std::istringstream iss(numberString);
            if (!(iss >> _settings->_jobs))
            {
                PrintMessage("cppcheck: argument to '-j' is not a number");
                return false;
            }

            if (_settings->_jobs > 10000)
            {
                // This limit is here just to catch typos. If someone has
                // need for more jobs, this value should be increased.
                PrintMessage("cppcheck: argument for '-j' is allowed to be 10000 at max");
                return false;
            }
        }

        // deprecated: auto deallocated classes..
        else if (strcmp(argv[i], "--auto-dealloc") == 0)
        {
            ++i;
        }

        // print all possible error messages..
        else if (strcmp(argv[i], "--errorlist") == 0)
        {
            //_cppcheck->getErrorMessages();
            _showErrorMessages = true;
            return true;
        }

        // documentation..
        else if (strcmp(argv[i], "--doc") == 0)
        {
            std::ostringstream doc;
            // Get documentation..
            for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
            {
                doc << "===" << (*it)->name() << "===\n"
                    << (*it)->classInfo() << "\n\n";
            }

            std::string doc2(doc.str());
            while (doc2.find("\n\n\n") != std::string::npos)
                doc2.erase(doc2.find("\n\n\n"), 1);
            std::cout << doc2;
            return true;
        }

        // --test-2-pass Experimental 2-pass checking of files
        // This command line flag will be removed
        else if (strcmp(argv[i], "--test-2-pass") == 0)
        {
            _settings->test_2_pass = true;
        }

        // show timing information..
        else if (strncmp(argv[i], "--showtime=", 11) == 0)
        {
            const std::string showtimeMode = argv[i] + 11;
            if (showtimeMode == "file")
                _settings->_showtime = SHOWTIME_FILE;
            else if (showtimeMode == "summary")
                _settings->_showtime = SHOWTIME_SUMMARY;
            else if (showtimeMode == "top5")
                _settings->_showtime = SHOWTIME_TOP5;
            else
                _settings->_showtime = SHOWTIME_NONE;
        }

        // Print help
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            _pathnames.clear();
            _showHelp = true;
            break;
        }

        else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0)
        {
            std::string message("cppcheck: error: unrecognized command line option \"");
            message += argv[i];
            message +=  "\"";
            PrintMessage(message);
            return false;
        }

        else
            _pathnames.push_back(argv[i]);
    }

    if (_settings->isEnabled("unusedFunctions") && _settings->_jobs > 1)
    {
        PrintMessage("unusedFunctions check can't be used with -j option, so it was disabled.");
    }

    // FIXME: Make the _settings.test_2_pass thread safe
    if (_settings->test_2_pass && _settings->_jobs > 1)
    {
        PrintMessage("--test-2-pass doesn't work with -j option yet.");
    }


    if (argc <= 1)
        _showHelp = true;

    if (_showHelp)
    {
        PrintHelp();
    }
    else if (_pathnames.empty())
    {
        PrintMessage("cppcheck: No C or C++ source files found.");
        return false;
    }

    return true;
}

void CmdLineParser::PrintHelp()
{
    std::cout <<   "Cppcheck - A tool for static C/C++ code analysis\n"
              "\n"
              "Syntax:\n"
              "    cppcheck [--append=file] [-D<ID>] [--enable=<id>] [--error-exitcode=[n]]\n"
              "             [--exitcode-suppressions file] [--file-list=file.txt] [--force]\n"
              "             [--help] [-Idir] [--inline-suppr] [-j [jobs]] [--quiet]\n"
              "             [--report-progress] [--style] [--suppressions file.txt]\n"
              "             [--verbose] [--version] [--xml] [file or path1] [file or path]\n"
              "\n"
              "If path is given instead of filename, *.cpp, *.cxx, *.cc, *.c++ and *.c files\n"
              "are checked recursively from given directory.\n\n"
              "Options:\n"
              "    --append=file        This allows you to provide information about\n"
              "                         functions by providing an implementation for these.\n"
              "    -D<ID>               By default Cppcheck checks all configurations.\n"
              "                         Use -D to limit the checking. When -D is used the\n"
              "                         checking is limited to the given configuration.\n"
              "                         Example: -DDEBUG=1 -D__cplusplus\n"
              "    --enable=id          Enable additional checks. The available ids are:\n"
              "                          * all - enable all checks\n"
              "                          * style - Check coding style\n"
              "                          * unusedFunction - check for unused functions\n"
              "                          * missingInclude - check for missing includes\n"
              "                         Several ids can be given if you separate them with commas\n"
              "    --error-exitcode=[n] If errors are found, integer [n] is returned instead\n"
              "                         of default 0. EXIT_FAILURE is returned\n"
              "                         if arguments are not valid or if no input files are\n"
              "                         provided. Note that your operating system can\n"
              "                         modify this value, e.g. 256 can become 0.\n"
              "    --exitcode-suppressions file\n"
              "                         Used when certain messages should be displayed but\n"
              "                         should not cause a non-zero exitcode.\n"
              "    --file-list=file     Specify the files to check in a text file. One Filename per line.\n"
              "    -f, --force          Force checking on files that have \"too many\"\n"
              "                         configurations\n"
              "    -h, --help           Print this help\n"
              "    -I [dir]             Give include path. Give several -I parameters to give\n"
              "                         several paths. First given path is checked first. If\n"
              "                         paths are relative to source files, this is not needed\n"
              "    --inline-suppr       Enable inline suppressions. Use them by placing one or\n"
              "                         more comments in the form: // cppcheck-suppress memleak\n"
              "                         on the lines before the warning to suppress.\n"
              "    -j [jobs]            Start [jobs] threads to do the checking simultaneously.\n"
              "    -q, --quiet          Only print error messages\n"
              "    --report-progress    Report progress messages while checking a file.\n"
              "    -s, --style          deprecated, use --enable=style\n"
              "    --suppressions file  Suppress warnings listed in the file. Filename and line\n"
              "                         are optional. The format of the single line in file is:\n"
              "                         [error id]:[filename]:[line]\n"
              "    --template '[text]'  Format the error messages. E.g.\n"
              "                         '{file}:{line},{severity},{id},{message}' or\n"
              "                         '{file}({line}):({severity}) {message}'\n"
              "                         Pre-defined templates: gcc, vs\n"
              "    -v, --verbose        More detailed error reports\n"
              "    --version            Print out version number\n"
              "    --xml                Write results in xml to error stream.\n"
              "\n"
              "Example usage:\n"
              "  # Recursively check the current folder. Print the progress on the screen and\n"
              "    write errors in a file:\n"
              "    cppcheck . 2> err.txt\n"
              "  # Recursively check ../myproject/ and don't print progress:\n"
              "    cppcheck --quiet ../myproject/\n"
              "  # Check only files one.cpp and two.cpp and give all information there is:\n"
              "    cppcheck -v -s one.cpp two.cpp\n"
              "  # Check f.cpp and search include files from inc1/ and inc2/:\n"
              "    cppcheck -I inc1/ -I inc2/ f.cpp\n"
              "\n"
              "For more information:\n"
              "    http://cppcheck.sf.net/manual.pdf\n";
}
