/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#include <algorithm>
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
    std::istream *Files;
    std::ifstream Infile;
    if (FileList.compare("-") == 0) { // read from stdin
        Files = &std::cin;
    } else {
        Infile.open(FileList.c_str());
        Files = &Infile;
    }
    if (Files) {
        std::string FileName;
        while (std::getline(*Files, FileName)) { // next line
            if (!FileName.empty()) {
                PathNames.push_back(FileName);
            }
        }
    }
}

static void AddInclPathsToList(const std::string& FileList, std::list<std::string>& PathNames)
{
    // to keep things initially simple, if the file can't be opened, just be
    // silent and move on
    std::ifstream Files(FileList.c_str());
    if (Files) {
        std::string PathName;
        while (std::getline(Files, PathName)) { // next line
            if (!PathName.empty()) {
                PathName = Path::fromNativeSeparators(PathName);
                PathName = Path::removeQuotationMarks(PathName);

                // If path doesn't end with / or \, add it
                if (PathName[PathName.length()-1] != '/')
                    PathName += '/';

                PathNames.push_back(PathName);
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

void CmdLineParser::PrintMessage(const std::string &message) const
{
    std::cout << message << std::endl;
}

bool CmdLineParser::ParseFromArgs(int argc, const char* const argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
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

        // Inconclusive checking (still in testing phase)
        else if (strcmp(argv[i], "--inconclusive") == 0)
            _settings->inconclusive = true;

        // Filter errors
        else if (strncmp(argv[i], "--exitcode-suppressions", 23) == 0) {
            std::string filename;

            // exitcode-suppressions filename.txt
            // Deprecated
            if (strcmp(argv[i], "--exitcode-suppressions") == 0) {
                ++i;

                if (i >= argc || strncmp(argv[i], "-", 1) == 0 ||
                    strncmp(argv[i], "--", 2) == 0) {
                    PrintMessage("cppcheck: No filename specified for the '--exitcode-suppressions' option.");
                    return false;
                }
                filename = argv[i];
            }
            // exitcode-suppressions=filename.txt
            else {
                filename = 24 + argv[i];
            }

            std::ifstream f(filename.c_str());
            if (!f.is_open()) {
                PrintMessage("cppcheck: Couldn't open the file: \"" + filename + "\".");
                return false;
            }
            const std::string errmsg(_settings->nofail.parseFile(f));
            if (!errmsg.empty()) {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strncmp(argv[i], "--suppressions-list=", 20) == 0) {
            std::string filename = argv[i]+20;
            std::ifstream f(filename.c_str());
            if (!f.is_open()) {
                std::string message("cppcheck: Couldn't open the file: \"");
                message += filename;
                message += "\".";
                if (count(filename.begin(), filename.end(), ',') > 0 ||
                    count(filename.begin(), filename.end(), '.') > 1) {
                    // If user tried to pass multiple files (we can only guess that)
                    // e.g. like this: --suppressions-list=a.txt,b.txt
                    // print more detailed error message to tell user how he can solve the problem
                    message += "\nIf you want to pass two files, you can do it e.g. like this:";
                    message += "\n    cppcheck --suppressions-list=a.txt --suppressions-list=b.txt file.cpp";
                }

                PrintMessage(message);
                return false;
            }
            const std::string errmsg(_settings->nomsg.parseFile(f));
            if (!errmsg.empty()) {
                PrintMessage(errmsg);
                return false;
            }
        }

        // Filter errors
        // This is deprecated, see --supressions-list above
        else if (strcmp(argv[i], "--suppressions") == 0) {
            ++i;

            if (i >= argc) {
                PrintMessage("cppcheck: No file specified for the '--suppressions' option.");
                return false;
            }

            std::ifstream f(argv[i]);
            if (!f.is_open()) {
                std::string message("cppcheck: Couldn't open the file: \"");
                message += std::string(argv[i]);
                message += "\".";
                PrintMessage(message);
                return false;
            }
            const std::string errmsg(_settings->nomsg.parseFile(f));
            if (!errmsg.empty()) {
                PrintMessage(errmsg);
                return false;
            }
        }

        else if (strncmp(argv[i], "--suppress=", 11) == 0) {
            std::string suppression = argv[i]+11;
            const std::string errmsg(_settings->nomsg.addSuppressionLine(suppression));
            if (!errmsg.empty()) {
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

        // Output relative paths
        else if (strcmp(argv[i], "-rp") == 0 || strcmp(argv[i], "--relative-paths") == 0)
            _settings->_relativePaths = true;
        else if (strncmp(argv[i], "-rp=", 4) == 0 || strncmp(argv[i], "--relative-paths=", 17) == 0) {
            _settings->_relativePaths = true;
            if (argv[i][argv[i][3]=='='?4:17] != 0) {
                std::string paths = argv[i]+(argv[i][3]=='='?4:17);
                std::string::size_type pos;
                do {
                    pos = paths.find(';');
                    _settings->_basePaths.push_back(Path::fromNativeSeparators(paths.substr(0, pos)));
                    paths.erase(0, pos+1);
                } while (pos != std::string::npos);
            } else {
                PrintMessage("cppcheck: No paths specified for the '" + std::string(argv[i]) + "' option.");
                return false;
            }
        }

        // Write results in results.xml
        else if (strcmp(argv[i], "--xml") == 0)
            _settings->_xml = true;

        // Define the XML file version (and enable XML output)
        else if (strncmp(argv[i], "--xml-version=", 14) == 0) {
            std::string numberString(argv[i]+14);

            std::istringstream iss(numberString);
            if (!(iss >> _settings->_xml_version)) {
                PrintMessage("cppcheck: argument to '--xml-version' is not a number.");
                return false;
            }

            if (_settings->_xml_version < 0 || _settings->_xml_version > 2) {
                // We only have xml versions 1 and 2
                PrintMessage("cppcheck: '--xml-version' can only be 1 or 2.");
                return false;
            }

            // Enable also XML if version is set
            _settings->_xml = true;
        }

        // Only print something when there are errors
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            _settings->_errorsOnly = true;

        // Append userdefined code to checked source code
        else if (strncmp(argv[i], "--append=", 9) == 0) {
            const std::string filename = 9 + argv[i];
            if (!_settings->append(filename)) {
                PrintMessage("cppcheck: Couldn't open the file: \"" + filename + "\".");
                return false;
            }
        }

        else if (strncmp(argv[i], "--enable=", 9) == 0) {
            const std::string errmsg = _settings->addEnabled(argv[i] + 9);
            if (!errmsg.empty()) {
                PrintMessage(errmsg);
                return false;
            }
            // when "style" is enabled, also enable "performance" and "portability"
            if (_settings->isEnabled("style")) {
                _settings->addEnabled("performance");
                _settings->addEnabled("portability");
            }
        }

        // --error-exitcode=1
        else if (strncmp(argv[i], "--error-exitcode=", 17) == 0) {
            std::string temp = argv[i]+17;
            std::istringstream iss(temp);
            if (!(iss >> _settings->_exitCode)) {
                _settings->_exitCode = 0;
                PrintMessage("cppcheck: Argument must be an integer. Try something like '--error-exitcode=1'.");
                return false;
            }
        }

        // User define
        else if (strncmp(argv[i], "-D", 2) == 0) {
            std::string define;

            // "-D define"
            if (strcmp(argv[i], "-D") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    PrintMessage("cppcheck: argument to '-D' is missing.");
                    return false;
                }

                define = argv[i];
            }
            // "-Ddefine"
            else {
                define = 2 + argv[i];
            }

            // No "=", append a "=1"
            if (define.find("=") == std::string::npos)
                define += "=1";

            if (!_settings->userDefines.empty())
                _settings->userDefines += ";";
            _settings->userDefines += define;
        }
        // User undef
        else if (strncmp(argv[i], "-U", 2) == 0) {
            std::string undef;

            // "-U undef"
            if (strcmp(argv[i], "-U") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    PrintMessage("cppcheck: argument to '-U' is missing.");
                    return false;
                }

                undef = argv[i];
            }
            // "-Uundef"
            else {
                undef = 2 + argv[i];
            }

            _settings->userUndefs.insert(undef);
        }

        // Include paths
        else if (strncmp(argv[i], "-I", 2) == 0) {
            std::string path;

            // "-I path/"
            if (strcmp(argv[i], "-I") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    PrintMessage("cppcheck: argument to '-I' is missing.");
                    return false;
                }
                path = argv[i];
            }

            // "-Ipath/"
            else {
                path = 2 + argv[i];
            }
            path = Path::fromNativeSeparators(path);
            path = Path::removeQuotationMarks(path);

            // If path doesn't end with / or \, add it
            if (path[path.length()-1] != '/')
                path += '/';

            _settings->_includePaths.push_back(path);
        } else if (strncmp(argv[i], "--includes-file=", 16) == 0) {
            // open this file and read every input file (1 file name per line)
            AddInclPathsToList(16 + argv[i], _settings->_includePaths);
        }

        // file list specified
        else if (strncmp(argv[i], "--file-list=", 12) == 0) {
            // open this file and read every input file (1 file name per line)
            AddFilesToList(12 + argv[i], _pathnames);
        }

        // Ignored paths
        else if (strncmp(argv[i], "-i", 2) == 0) {
            std::string path;

            // "-i path/"
            if (strcmp(argv[i], "-i") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    PrintMessage("cppcheck: argument to '-i' is missing.");
                    return false;
                }
                path = argv[i];
            }

            // "-ipath/"
            else {
                path = 2 + argv[i];
            }

            if (!path.empty()) {
                path = Path::fromNativeSeparators(path);
                path = Path::simplifyPath(path.c_str());
                path = Path::removeQuotationMarks(path);

                if (FileLister::isDirectory(path)) {
                    // If directory name doesn't end with / or \, add it
                    if (path[path.length()-1] != '/')
                        path += '/';
                }
                _ignoredPaths.push_back(path);
            }
        }

        // Report progress
        else if (strcmp(argv[i], "--report-progress") == 0) {
            _settings->reportProgress = true;
        }

        // --std
        else if (strcmp(argv[i], "--std=posix") == 0) {
            _settings->standards.posix = true;
        }

        // --C99
        else if (strcmp(argv[i], "--std=c99") == 0) {
            _settings->standards.c99 = true;
        }

        else if (strcmp(argv[i], "--std=c++11") == 0) {
            _settings->standards.cpp11 = true;
        }

        // Output formatter
        else if (strcmp(argv[i], "--template") == 0 ||
                 strncmp(argv[i], "--template=", 11) == 0) {
            // "--template path/"
            if (argv[i][10] == '=')
                _settings->_outputFormat = argv[i] + 11;
            else if ((i+1) < argc && argv[i+1][0] != '-') {
                ++i;
                _settings->_outputFormat = argv[i];
            } else {
                PrintMessage("cppcheck: argument to '--template' is missing.");
                return false;
            }

            if (_settings->_outputFormat == "gcc")
                _settings->_outputFormat = "{file}:{line}: {severity}: {message}";
            else if (_settings->_outputFormat == "vs")
                _settings->_outputFormat = "{file}({line}): {severity}: {message}";
            else if (_settings->_outputFormat == "edit")
                _settings->_outputFormat = "{file} +{line}: {severity}: {message}";
        }

        // Checking threads
        else if (strncmp(argv[i], "-j", 2) == 0) {
            std::string numberString;

            // "-j 3"
            if (strcmp(argv[i], "-j") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    PrintMessage("cppcheck: argument to '-j' is missing.");
                    return false;
                }

                numberString = argv[i];
            }

            // "-j3"
            else
                numberString = argv[i]+2;

            std::istringstream iss(numberString);
            if (!(iss >> _settings->_jobs)) {
                PrintMessage("cppcheck: argument to '-j' is not a number.");
                return false;
            }

            if (_settings->_jobs > 10000) {
                // This limit is here just to catch typos. If someone has
                // need for more jobs, this value should be increased.
                PrintMessage("cppcheck: argument for '-j' is allowed to be 10000 at max.");
                return false;
            }
        }

        // print all possible error messages..
        else if (strcmp(argv[i], "--errorlist") == 0) {
            _showErrorMessages = true;
            _settings->_xml = true;
            _exitAfterPrint = true;
        }

        // documentation..
        else if (strcmp(argv[i], "--doc") == 0) {
            std::ostringstream doc;
            // Get documentation..
            for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
                const std::string name((*it)->name());
                const std::string info((*it)->classInfo());
                if (!name.empty() && !info.empty())
                    doc << "===" << name << "===\n"
                        << info << "\n\n";
            }

            std::string doc2(doc.str());
            while (doc2.find("\n\n\n") != std::string::npos)
                doc2.erase(doc2.find("\n\n\n"), 1);
            std::cout << doc2;
            _exitAfterPrint = true;
            return true;
        }

        // show timing information..
        else if (strncmp(argv[i], "--showtime=", 11) == 0) {
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
        else if (strncmp(argv[i], "--rule=", 7) == 0) {
            Settings::Rule rule;
            rule.pattern = 7 + argv[i];
            _settings->rules.push_back(rule);
        }

        // Rule file
        else if (strncmp(argv[i], "--rule-file=", 12) == 0) {
            TiXmlDocument doc;
            if (doc.LoadFile(12+argv[i])) {
                TiXmlElement *node = doc.FirstChildElement();
                for (; node && node->ValueStr() == "rule"; node = node->NextSiblingElement()) {
                    Settings::Rule rule;

                    TiXmlElement *pattern = node->FirstChildElement("pattern");
                    if (pattern) {
                        rule.pattern = pattern->GetText();
                    }

                    TiXmlElement *message = node->FirstChildElement("message");
                    if (message) {
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
        else if (strcmp(argv[i], "--check-config") == 0) {
            _settings->checkConfiguration = true;
        }

        // Specify platform
        else if (strncmp(argv[i], "--platform=", 11) == 0) {
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
            else {
                std::string message("cppcheck: error: unrecognized platform: \"");
                message += platform;
                message +=  "\".";
                PrintMessage(message);
                return false;
            }
        }

        // Set maximum number of #ifdef configurations to check
        else if (strncmp(argv[i], "--max-configs=", 14) == 0) {
            _settings->_force = false;

            std::istringstream iss(14+argv[i]);
            if (!(iss >> _settings->_maxConfigs)) {
                PrintMessage("cppcheck: argument to '--max-configs=' is not a number.");
                return false;
            }

            if (_settings->_maxConfigs < 1) {
                PrintMessage("cppcheck: argument to '--max-configs=' must be greater than 0.");
                return false;
            }
        }

        // Print help
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            _pathnames.clear();
            _showHelp = true;
            _exitAfterPrint = true;
            break;
        }

        else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) {
            std::string message("cppcheck: error: unrecognized command line option: \"");
            message += argv[i];
            message +=  "\".";
            PrintMessage(message);
            return false;
        }

        else {
            std::string path = Path::fromNativeSeparators(argv[i]);
            path = Path::removeQuotationMarks(path);
            _pathnames.push_back(path);
        }
    }

    if (_settings->isEnabled("unusedFunction") && _settings->_jobs > 1) {
        PrintMessage("cppcheck: unusedFunction check can't be used with '-j' option, so it's disabled.");
    }

    if (argc <= 1)
        _showHelp = true;

    if (_showHelp) {
        PrintHelp();
        return true;
    }

    // Print error only if we have "real" command and expect files
    if (!_exitAfterPrint && _pathnames.empty()) {
        PrintMessage("cppcheck: No C or C++ source files found.");
        return false;
    }

    // Use paths _pathnames if no base paths for relative path output are given
    if (_settings->_basePaths.empty() && _settings->_relativePaths)
        _settings->_basePaths = _pathnames;

    return true;
}

void CmdLineParser::PrintHelp() const
{
    std::cout <<   "Cppcheck - A tool for static C/C++ code analysis\n"
              "\n"
              "Syntax:\n"
              "    cppcheck [OPTIONS] [files or paths]\n"
              "\n"
              "If a directory is given instead of a filename, *.cpp, *.cxx, *.cc, *.c++, *.c,\n"
              "*.tpp, and *.txx files are checked recursively from the given directory.\n\n"
              "Options:\n"
              "    --append=<file>      This allows you to provide information about functions\n"
              "                         by providing an implementation for them.\n"
              "    --check-config       Check cppcheck configuration. The normal code\n"
              "                         analysis is disabled by this flag.\n"
              "    -D<ID>               By default Cppcheck checks all configurations. Use -D\n"
              "                         to limit the checking to a particular configuration.\n"
              "                         Example: '-DDEBUG=1 -D__cplusplus'.\n"
              "    -U<ID>               By default Cppcheck checks all configurations. Use -U\n"
              "                         to explicitly hide certain #ifdef <ID> code paths from\n"
              "                         checking.\n"
              "                         Example: '-UDEBUG'\n"
              "    --enable=<id>        Enable additional checks. The available ids are:\n"
              "                          * all\n"
              "                                  Enable all checks\n"
              "                          * style\n"
              "                                  Enable all coding style checks. All messages\n"
              "                                  with the severities 'style', 'performance' and\n"
              "                                  'portability' are enabled.\n"
              "                          * performance\n"
              "                                  Enable performance messages\n"
              "                          * portability\n"
              "                                  Enable portability messages\n"
              "                          * information\n"
              "                                  Enable information messages\n"
              "                          * unusedFunction\n"
              "                                  Check for unused functions\n"
              "                          * missingInclude\n"
              "                                  Warn if there are missing includes. For\n"
              "                                  detailed information, use '--check-config'.\n"
              "                         Several ids can be given if you separate them with\n"
              "                         commas. See also --std\n"
              "    --error-exitcode=<n> If errors are found, integer [n] is returned instead of\n"
              "                         the default '0'. '" << EXIT_FAILURE << "' is returned\n"
              "                         if arguments are not valid or if no input files are\n"
              "                         provided. Note that your operating system can modify\n"
              "                         this value, e.g. '256' can become '0'.\n"
              "    --errorlist          Print a list of all the error messages in XML format.\n"
              "    --exitcode-suppressions=<file>\n"
              "                         Used when certain messages should be displayed but\n"
              "                         should not cause a non-zero exitcode.\n"
              "    --file-list=<file>   Specify the files to check in a text file. Add one\n"
              "                         filename per line. When file is '-,' the file list will\n"
              "                         be read from standard input.\n"
              "    -f, --force          Force checking of all configurations in files. If used\n"
              "                         together with '--max-configs=', the last option is the\n"
              "                         one that is effective.\n"
              "    -h, --help           Print this help.\n"
              "    -I <dir>             Give path to search for include files. Give several -I\n"
              "                         parameters to give several paths. First given path is\n"
              "                         searched for contained header files first. If paths are\n"
              "                         relative to source files, this is not needed.\n"
              "    --includes-file=<file>\n"
              "                         Specify directory paths to search for included header\n"
              "                         files in a text file. Add one include path per line.\n"
              "                         First given path is searched for contained header\n"
              "                         files first. If paths are relative to source files,\n"
              "                         this is not needed.\n"
              "    -i <dir or file>     Give a source file or source file directory to exclude\n"
              "                         from the check. This applies only to source files so\n"
              "                         header files included by source files are not matched.\n"
              "                         Directory name is matched to all parts of the path.\n"
              "    --inconclusive       Allow that Cppcheck reports even though the analysis is\n"
              "                         inconclusive.\n"
              "                         There are false positives with this option. Each result\n"
              "                         must be carefully investigated before you know if it is\n"
              "                         good or bad.\n"
              "    --inline-suppr       Enable inline suppressions. Use them by placing one or\n"
              "                         more comments, like: '// cppcheck-suppress warningId'\n"
              "                         on the lines before the warning to suppress.\n"
              "    -j <jobs>            Start [jobs] threads to do the checking simultaneously.\n"
              "    --max-configs=<limit>\n"
              "                         Maximum number of configurations to check in a file\n"
              "                         before skipping it. Default is '12'. If used together\n"
              "                         with '--force', the last option is the one that is\n"
              "                         effective.\n"
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
              "    -rp, --relative-paths\n"
              "    -rp=<paths>, --relative-paths=<paths>\n"
              "                         Use relative paths in output. When given, <paths> are\n"
              "                         used as base. You can separate multiple paths by ';'.\n"
              "                         Otherwise path where source files are searched is used.\n"
              "                         We use string comparison to create relative paths, so\n"
              "                         using e.g. ~ for home folder does not work. It is\n"
              "                         currently only possible to apply the base paths to\n"
              "                         files that are on a lower level in the directory tree.\n"
              "    --report-progress    Report progress messages while checking a file.\n"
#ifdef HAVE_RULES
              "    --rule=<rule>        Match regular expression.\n"
              "    --rule-file=<file>   Use given rule file. For more information, see: \n"
              "                         https://sourceforge.net/projects/cppcheck/files/Articles/\n"
#endif
              "    --std=<id>           Enable some standard related checks.\n"
              "                         The available options are:\n"
              "                          * posix\n"
              "                                 Checks related to POSIX-specific functionality\n"
              "                          * c99\n"
              "                                 C99 standard related checks\n"
              "                          * c++11\n"
              "                                 C++11 standard related checks\n"
              "                         Example to enable more than one checks:\n"
              "                           'cppcheck --std=c99 --std=posix file.cpp'\n"
              "    --suppress=<spec>    Suppress warnings that match <spec>. The format of\n"
              "                         <spec> is:\n"
              "                         [error id]:[filename]:[line]\n"
              "                         The [filename] and [line] are optional. If [error id]\n"
              "                         is a wildcard '*', all error ids match.\n"
              "    --suppressions-list=<file>\n"
              "                         Suppress warnings listed in the file. Each suppression\n"
              "                         is in the same format as <spec> above.\n"
              "    --template='<text>'  Format the error messages. E.g.\n"
              "                         '{file}:{line},{severity},{id},{message}' or\n"
              "                         '{file}({line}):({severity}) {message}'\n"
              "                         Pre-defined templates: gcc, vs, edit.\n"
              "    -v, --verbose        Output more detailed error information.\n"
              "    --version            Print out version number.\n"
              "    --xml                Write results in xml format to error stream (stderr).\n"
              "    --xml-version=<version>\n"
              "                         Select the XML file version. Currently versions 1 and\n"
              "                         2 are available. The default version is 1."
              "\n"
              "Example usage:\n"
              "  # Recursively check the current folder. Print the progress on the screen and\n"
              "  # write errors to a file:\n"
              "  cppcheck . 2> err.txt\n"
              "\n"
              "  # Recursively check ../myproject/ and don't print progress:\n"
              "  cppcheck --quiet ../myproject/\n"
              "\n"
              "  # Check test.cpp, enable all checks:\n"
              "  cppcheck --enable=all --inconclusive --std=c++11 test.cpp\n"
              "\n"
              "  # Check f.cpp and search include files from inc1/ and inc2/:\n"
              "  cppcheck -I inc1/ -I inc2/ f.cpp\n"
              "\n"
              "For more information:\n"
              "    http://cppcheck.sf.net/manual.pdf\n";
}
