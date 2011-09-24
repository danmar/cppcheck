/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include <stdlib.h> // EXIT_FAILURE
#include "cppcheck.h"
#include "timer.h"
#include "settings.h"
#include "cmdlineparser.h"
#include "path.h"
#include "filelister.h"

#ifdef HAVE_RULES
// xml is used in rules
#include <tinyxml.h>
#endif

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
    , _exitAfterPrint(false)
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
            _exitAfterPrint = true;
            return true;
        }

        // Flag used for various purposes during debugging
        else if (strcmp(argv[i], "--debug") == 0)
            _settings->debug = _settings->debugwarnings = true;

        // Show debug warnings
        else if (strcmp(argv[i], "--debug-warnings") == 0)
            _settings->debugwarnings = true;

        // Print out code that triggers false positive
        else if (strcmp(argv[i], "--debug-fp") == 0)
            _settings->debugFalsePositive = true;

        // Enable all checks - will be removed in future
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
        {
            PrintMessage("cppcheck: -a/--all option is deprecated and will be removed in 1.55 release.");
            PrintMessage("cppcheck:   please use --enable=all instead.");
        }

        // Inconclusive checking (still in testing phase)
        else if (strcmp(argv[i], "--inconclusive") == 0)
            _settings->inconclusive = true;

        // Checking coding style - will be removed in the future
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0)
        {
            PrintMessage("cppcheck: -s/--style option is deprecated and will be removed in 1.55 release.");
            PrintMessage("cppcheck:   please use --enable=style instead.");

            const std::string errmsg = _settings->addEnabled("style");
            if (!errmsg.empty())
            {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strncmp(argv[i], "--exitcode-suppressions", 23) == 0)
        {
            std::string filename;

            // exitcode-suppressions filename.txt
            // Deprecated
            if (strcmp(argv[i], "--exitcode-suppressions") == 0)
            {
                ++i;

                if (i >= argc || strncmp(argv[i], "-", 1) == 0 ||
                    strncmp(argv[i], "--", 2) == 0)
                {
                    PrintMessage("cppcheck: No filename specified for the --exitcode-suppressions option");
                    return false;
                }
                filename = argv[i];
            }
            // exitcode-suppressions=filename.txt
            else
            {
                filename = 24 + argv[i];
            }

            std::ifstream f(filename.c_str());
            if (!f.is_open())
            {
                PrintMessage("cppcheck: Couldn't open the file \"" + std::string(filename) + "\"");
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
        else if (strncmp(argv[i], "--suppressions-list=", 20) == 0)
        {
            std::string filename = argv[i];
            filename = filename.substr(20);
            std::ifstream f(filename.c_str());
            if (!f.is_open())
            {
                std::string message("cppcheck: Couldn't open the file \"");
                message += std::string(filename);
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

        // Filter errors
        // This is deprecated, see --supressions-list above
        else if (strcmp(argv[i], "--suppressions") == 0 &&
                 strlen(argv[i]) == 14)
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

        else if (strncmp(argv[i], "--suppress=", 11) == 0)
        {
            std::string suppression = argv[i];
            suppression = suppression.substr(11);
            const std::string errmsg(_settings->nomsg.addSuppressionLine(suppression));
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

        // Define the XML file version (and enable XML output)
        else if (strncmp(argv[i], "--xml-version=", 14) == 0)
        {
            std::string numberString(argv[i]);
            numberString = numberString.substr(14);

            std::istringstream iss(numberString);
            if (!(iss >> _settings->_xml_version))
            {
                PrintMessage("cppcheck: argument to '--xml-version' is not a number");
                return false;
            }

            if (_settings->_xml_version < 0 || _settings->_xml_version > 2)
            {
                // We only have xml versions 1 and 2
                PrintMessage("cppcheck: --xml-version can only be 1 or 2.");
                return false;
            }

            // Enable also XML if version is set
            _settings->_xml = true;
        }

        // Only print something when there are errors
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            _settings->_errorsOnly = true;

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
            // when "style" is enabled, also enable "performance" and "portability"
            else if (strstr(argv[i]+9, "style"))
            {
                _settings->addEnabled("performance");
                _settings->addEnabled("portability");
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
            std::string define;

            // "-D define"
            if (strcmp(argv[i], "-D") == 0)
            {
                ++i;
                if (i >= argc || strncmp(argv[i], "-", 1) == 0 ||
                    strncmp(argv[i], "--", 2) == 0)
                {
                    PrintMessage("cppcheck: argument to '-D' is missing");
                    return false;
                }

                define = argv[i];
            }
            // "-Ddefine"
            else
            {
                define = 2 + argv[i];
            }

            if (!_settings->userDefines.empty())
                _settings->userDefines += ";";
            _settings->userDefines += define;
        }

        // Include paths
        else if (strncmp(argv[i], "-I", 2) == 0)
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
                path = 2 + argv[i];
            }
            path = Path::fromNativeSeparators(path);
            path = Path::removeQuotationMarks(path);

            // If path doesn't end with / or \, add it
            if (path[path.length()-1] != '/')
                path += '/';

            _settings->_includePaths.push_back(path);
        }

        // file list specified
        else if (strncmp(argv[i], "--file-list=", 12) == 0)
        {
            // open this file and read every input file (1 file name per line)
            AddFilesToList(12 + argv[i], _pathnames);
        }

        // Ignored paths
        else if (strncmp(argv[i], "-i", 2) == 0)
        {
            std::string path;

            // "-i path/"
            if (strcmp(argv[i], "-i") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    PrintMessage("cppcheck: argument to '-i' is missing");
                    return false;
                }
                path = argv[i];
            }

            // "-ipath/"
            else
            {
                path = 2 + argv[i];
            }

            if (!path.empty())
            {
                path = Path::fromNativeSeparators(path);
                path = Path::simplifyPath(path.c_str());
                path = Path::removeQuotationMarks(path);

                if (!FileLister::fileExists(path) && FileLister::isDirectory(path))
                {
                    // If directory name doesn't end with / or \, add it
                    if (path[path.length()-1] != '/')
                        path += '/';
                }
                _ignoredPaths.push_back(path);
            }
        }

        // Report progress
        else if (strcmp(argv[i], "--report-progress") == 0)
        {
            _settings->reportProgress = true;
        }

        // --std
        else if (strcmp(argv[i], "--std=posix") == 0)
        {
            _settings->posix = true;
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
            PrintMessage("cppcheck: --auto-dealloc option is deprecated and will be removed in 1.55 release.");
        }

        // print all possible error messages..
        else if (strcmp(argv[i], "--errorlist") == 0)
        {
            _showErrorMessages = true;
            _settings->_xml = true;
            _exitAfterPrint = true;
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
            _exitAfterPrint = true;
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

#ifdef HAVE_RULES
        // Rule given at command line
        else if (strncmp(argv[i], "--rule=", 7) == 0)
        {
            Settings::Rule rule;
            rule.pattern = 7 + argv[i];
            _settings->rules.push_back(rule);
        }

        // Rule file
        else if (strncmp(argv[i], "--rule-file=", 12) == 0)
        {
            TiXmlDocument doc;
            if (doc.LoadFile(12+argv[i]))
            {
                TiXmlElement *node = doc.FirstChildElement();
                for (; node && node->ValueStr() == "rule"; node = node->NextSiblingElement())
                {
                    Settings::Rule rule;

                    TiXmlElement *pattern = node->FirstChildElement("pattern");
                    if (pattern)
                    {
                        rule.pattern = pattern->GetText();
                    }

                    TiXmlElement *message = node->FirstChildElement("message");
                    if (message)
                    {
                        TiXmlElement *severity = message->FirstChildElement("severity");
                        if (severity)
                            rule.severity = severity->GetText();

                        TiXmlElement *id = message->FirstChildElement("id");
                        if (id)
                            rule.id = id->GetText();

                        TiXmlElement *summary = message->FirstChildElement("summary");
                        if (summary)
                            rule.summary = summary->GetText();
                    }

                    if (!rule.pattern.empty())
                        _settings->rules.push_back(rule);
                }
            }
        }
#endif

        // Check configuration
        else if (strcmp(argv[i], "--check-config") == 0)
        {
            _settings->checkConfiguration = true;
        }

        // Specify platform
        else if (strncmp(argv[i], "--platform=", 11) == 0)
        {
            std::string platform(11+argv[i]);

            if (platform == "win32A")
                _settings->platform(Settings::Win32A);
            else if (platform == "win32W")
                _settings->platform(Settings::Win32W);
            else if (platform == "win64")
                _settings->platform(Settings::Win64);
            else if (platform == "unix32")
                _settings->platform(Settings::Unix32);
            else if (platform == "unix64")
                _settings->platform(Settings::Unix64);
            else
            {
                std::string message("cppcheck: error: unrecognized platform\"");
                message += argv[i];
                message +=  "\"";
                PrintMessage(message);
                return false;
            }
        }

        // Print help
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            _pathnames.clear();
            _showHelp = true;
            _exitAfterPrint = true;
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
        {
            std::string path = Path::fromNativeSeparators(argv[i]);
            path = Path::removeQuotationMarks(path);
            _pathnames.push_back(path);
        }
    }

    if (_settings->isEnabled("unusedFunction") && _settings->_jobs > 1)
    {
        PrintMessage("cppcheck: unusedFunction check can't be used with -j option, so it was disabled.");
    }

    // FIXME: Make the _settings.test_2_pass thread safe
    if (_settings->test_2_pass && _settings->_jobs > 1)
    {
        PrintMessage("cppcheck: --test-2-pass doesn't work with -j option yet.");
    }

    if (argc <= 1)
        _showHelp = true;

    if (_showHelp)
    {
        PrintHelp();
        return true;
    }

    // Print error only if we have "real" command and expect files
    if (!_exitAfterPrint && _pathnames.empty())
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
              "    cppcheck [OPTIONS] [files or paths]\n"
              "\n"
              "If a directory is given instead of a filename, *.cpp, *.cxx, *.cc, *.c++, *.c,\n"
              "*.tpp, and *.txx files are checked recursively from the given directory.\n\n"
              "Options:\n"
              "    --append=<file>      This allows you to provide information about\n"
              "                         functions by providing an implementation for them.\n"
              "    --check-config       Check cppcheck configuration. The normal code\n"
              "                         analysis is disabled by this flag.\n"
              "    -D<ID>               By default Cppcheck checks all configurations.\n"
              "                         Use -D to limit the checking. When -D is used the\n"
              "                         checking is limited to the given configuration.\n"
              "                         Example: -DDEBUG=1 -D__cplusplus\n"
              "    --enable=<id>        Enable additional checks. The available ids are:\n"
              "                          * all\n"
              "                                  Enable all checks\n"
              "                          * style\n"
              "                                  Enable all coding style checks. All messages\n"
              "                                  with the severities 'style', 'performance'\n"
              "                                  and 'portability' are enabled.\n"
              "                          * performance\n"
              "                                  Enable performance messages\n"
              "                          * portability\n"
              "                                  Enable portability messages\n"
              "                          * information\n"
              "                                  Enable information messages\n"
              "                          * unusedFunction\n"
              "                                  Check for unused functions\n"
              "                          * missingInclude\n"
              "                                  Warn if there are missing includes.\n"
              "                                  For detailed information use --check-config\n"
              "                         Several ids can be given if you separate them with\n"
              "                         commas.\n"
              "    --error-exitcode=<n> If errors are found, integer [n] is returned instead\n"
              "                         of the default 0. " << EXIT_FAILURE << " is returned\n"
              "                         if arguments are not valid or if no input files are\n"
              "                         provided. Note that your operating system can\n"
              "                         modify this value, e.g. 256 can become 0.\n"
              "    --errorlist          Print a list of all the error messages in XML format.\n"
              "    --exitcode-suppressions=<file>\n"
              "                         Used when certain messages should be displayed but\n"
              "                         should not cause a non-zero exitcode.\n"
              "    --file-list=<file>   Specify the files to check in a text file. Add one\n"
              "                         filename per line.\n"
              "    -f, --force          Force checking of all configurations in files that have\n"
              "                         \"too many\" configurations.\n"
              "    -h, --help           Print this help.\n"
              "    -I <dir>             Give include path. Give several -I parameters to give\n"
              "                         several paths. First given path is checked first. If\n"
              "                         paths are relative to source files, this is not needed.\n"
              "    -i <dir or file>     Give a source file or source file directory to exclude\n"
              "                         from the check. This applies only to source files so\n"
              "                         header files included by source files are not matched.\n"
              "                         Directory name is matched to all parts of the path.\n"
              "    --inline-suppr       Enable inline suppressions. Use them by placing one or\n"
              "                         more comments, like: // cppcheck-suppress warningId\n"
              "                         on the lines before the warning to suppress.\n"
              "    -j <jobs>            Start [jobs] threads to do the checking simultaneously.\n"
              "    --platform=<type>    Specifies platform specific types and sizes. The\n"
              "                         available platforms are:\n"
              "                          * unix32\n"
              "                                 32 bit unix variant\n"
              "                          * unix64\n"
              "                                 64 bit unix variant\n"
              "                          * win32A\n"
              "                                 32 bit Windows ASCII character encoding\n"
              "                          * win32W\n"
              "                                 32 bit Windows UNICODE character encoding\n"
              "                          * win64\n"
              "                                 64 bit Windows\n"
              "    -q, --quiet          Only print error messages.\n"
              "    --report-progress    Report progress messages while checking a file.\n"
#ifdef HAVE_RULES
              "    --rule=<rule>        Match regular expression.\n"
              "    --rule-file=<file>   Use given rule file. For more information, see: \n"
              "                         https://sourceforge.net/projects/cppcheck/files/Articles/\n"
#endif
              "    -s, --style          Deprecated, use --enable=style\n"
              "    --std=posix          Code is posix\n"
              "    --suppress=<spec>    Suppress warnings that match <spec>. The format of\n"
              "                         <spec> is:\n"
              "                         [error id]:[filename]:[line]\n"
              "                         The [filename] and [line] are optional. If [error id]\n"
              "                         is a wildcard '*', all error ids match.\n"
              "    --suppressions-list=<file>\n"
              "                         Suppress warnings listed in the file. Each suppression\n"
              "                         is in the same format as <spec> above.\n"
              "    --template '<text>'  Format the error messages. E.g.\n"
              "                         '{file}:{line},{severity},{id},{message}' or\n"
              "                         '{file}({line}):({severity}) {message}'\n"
              "                         Pre-defined templates: gcc, vs\n"
              "    -v, --verbose        Output more detailed error information.\n"
              "    --version            Print out version number.\n"
              "    --xml                Write results in xml format to error stream (stderr).\n"
              "    --xml-version=<version>\n"
              "                         Select the XML file version. Currently versions 1 and 2\n"
              "                         are available. The default version is 1."
              "\n"
              "Example usage:\n"
              "  # Recursively check the current folder. Print the progress on the screen and\n"
              "    write errors to a file:\n"
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
