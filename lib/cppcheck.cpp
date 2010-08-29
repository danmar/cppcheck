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

/*
	TODO:
	- handle SHOWTIME_TOP5 in TimerResults
	- sort list by time
	- do not sort the results alphabetically
	- rename "file" to "single"
	- synchronise map access in multithreaded mode or disable timing
	- add unit tests
		- for --showtime (needs input file)
		- for Timer* classes
	- move timer stuff to seperate source/header
*/
enum
{
    SHOWTIME_NONE = 0,
    SHOWTIME_FILE,
    SHOWTIME_SUMMARY,
    SHOWTIME_TOP5
};

class TimerResultsIntf
{
public:
    virtual ~TimerResultsIntf() { }

    virtual void AddResults(const std::string& str, std::clock_t clocks) = 0;
};

struct TimerResultsData
{
    std::clock_t _clocks;
    long _numberOfResults;

    TimerResultsData()
        : _clocks(0)
        , _numberOfResults(0)
    {
    }
};

class TimerResults : public TimerResultsIntf
{
public:
    TimerResults()
    {
    }

    void ShowResults()
    {
        std::clock_t overallClocks = 0;

        std::map<std::string, struct TimerResultsData>::const_iterator I = _results.begin();
        const std::map<std::string, struct TimerResultsData>::const_iterator E = _results.end();

        while (I != E)
        {
            const double sec = (double)I->second._clocks / CLOCKS_PER_SEC;
            const double secAverage = (double)(I->second._clocks / I->second._numberOfResults) / CLOCKS_PER_SEC;
            std::cout << I->first << ": " << sec << "s (avg. " << secAverage << "s - " << I->second._numberOfResults  << " result(s))" << std::endl;

            overallClocks += I->second._clocks;

            ++I;
        }

        const double secOverall = (double)overallClocks / CLOCKS_PER_SEC;
        std::cout << "Overall time: " << secOverall << "s" << std::endl;
    }

    virtual void AddResults(const std::string& str, std::clock_t clocks)
    {
        _results[str]._clocks += clocks;
        _results[str]._numberOfResults++;
    }

private:
    std::map<std::string, struct TimerResultsData> _results;
};

static TimerResults S_timerResults;

class Timer
{
public:
    Timer(const std::string& str, unsigned int showtimeMode, TimerResultsIntf* timerResults = NULL)
        : _str(str)
        , _showtimeMode(showtimeMode)
        , _start(0)
        , _stopped(false)
        , _timerResults(timerResults)
    {
        if (showtimeMode != SHOWTIME_NONE)
            _start = std::clock();
    }

    ~Timer()
    {
        Stop();
    }

    void Stop()
    {
        if ((_showtimeMode != SHOWTIME_NONE) && !_stopped)
        {
            const std::clock_t end = std::clock();
            const std::clock_t diff = end - _start;

            if (_showtimeMode == SHOWTIME_FILE)
            {
                double sec = (double)diff / CLOCKS_PER_SEC;
                std::cout << _str << ": " << sec << "s" << std::endl;
            }
            else
            {
                if (_timerResults)
                    _timerResults->AddResults(_str, diff);
            }
        }

        _stopped = true;
    }

private:
    Timer& operator=(const Timer&); // disallow assignments

    const std::string _str;
    const unsigned int _showtimeMode;
    std::clock_t _start;
    bool _stopped;
    TimerResultsIntf* _timerResults;
};

//---------------------------------------------------------------------------

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

bool CppCheck::parseFromArgs(int argc, const char* const argv[])
{
    std::vector<std::string> pathnames;
    bool showHelp = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            reportOut(std::string("Cppcheck ") + version());
            return true;
        }

        // Flag used for various purposes during debugging
        else if (strcmp(argv[i], "--debug") == 0)
            _settings.debug = _settings.debugwarnings = true;

        // Show debug warnings
        else if (strcmp(argv[i], "--debug-warnings") == 0)
            _settings.debugwarnings = true;

        // Inconclusive checking - keep this for compatibility but don't
        // handle it
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
            ;

        // Only print something when there are errors
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            _settings._errorsOnly = true;

        // Checking coding style
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0)
        {
            const std::string errmsg = _settings.addEnabled("style");
            if (!errmsg.empty())
            {
                reportOut(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strcmp(argv[i], "--suppressions") == 0)
        {
            ++i;

            if (i >= argc)
            {
                reportOut("cppcheck: No file specified for the --suppressions option");
                return false;
            }

            std::ifstream f(argv[i]);
            if (!f.is_open())
            {
                reportOut("cppcheck: Couldn't open the file \"" + std::string(argv[i]) + "\"");
                return false;
            }
            const std::string errmsg(_settings.nomsg.parseFile(f));
            if (!errmsg.empty())
            {
                reportOut(errmsg);
                return false;
            }
        }

        // Filter errors
        else if (strcmp(argv[i], "--exitcode-suppressions") == 0)
        {
            ++i;

            if (i >= argc)
            {
                reportOut("cppcheck: No file specified for the --exitcode-suppressions option");
                return false;
            }

            std::ifstream f(argv[i]);
            if (!f.is_open())
            {
                reportOut("cppcheck: Couldn't open the file \"" + std::string(argv[i]) + "\"");
                return false;
            }
            const std::string errmsg(_settings.nofail.parseFile(f));
            if (!errmsg.empty())
            {
                reportOut(errmsg);
                return false;
            }
        }

        // Enables inline suppressions.
        else if (strcmp(argv[i], "--inline-suppr") == 0)
            _settings._inlineSuppressions = true;

        // Verbose error messages (configuration info)
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            _settings._verbose = true;

        // Force checking of files that have "too many" configurations
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0)
            _settings._force = true;

        // Write results in results.xml
        else if (strcmp(argv[i], "--xml") == 0)
            _settings._xml = true;

        // Check if there are unused functions
        else if (strcmp(argv[i], "--unused-functions") == 0)
        {
            const std::string errmsg = _settings.addEnabled("unusedFunctions");
            if (!errmsg.empty())
            {
                reportOut(errmsg);
                return false;
            }
        }

        // Append userdefined code to checked source code
        else if (strncmp(argv[i], "--append=", 9) == 0)
            _settings.append(9 + argv[i]);

        // show timing information..
        else if (strncmp(argv[i], "--showtime=", 11) == 0)
        {
            const std::string showtimeMode = argv[i] + 11;
            if (showtimeMode == "file")
                _settings._showtime = SHOWTIME_FILE;
            else if (showtimeMode == "summary")
                _settings._showtime = SHOWTIME_SUMMARY;
            else if (showtimeMode == "top5")
                _settings._showtime = SHOWTIME_TOP5;
            else
                _settings._showtime = SHOWTIME_NONE;
        }

        // Print help
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            pathnames.clear();
            _filenames.clear();
            showHelp = true;
            break;
        }


        else if (strncmp(argv[i], "--enable=", 9) == 0)
        {
            const std::string errmsg = _settings.addEnabled(argv[i] + 9);
            if (!errmsg.empty())
            {
                reportOut(errmsg);
                return false;
            }
        }

        // --error-exitcode=1
        else if (strncmp(argv[i], "--error-exitcode=", 17) == 0)
        {
            std::string temp = argv[i];
            temp = temp.substr(17);
            std::istringstream iss(temp);
            if (!(iss >> _settings._exitCode))
            {
                _settings._exitCode = 0;
                reportOut("cppcheck: Argument must be an integer. Try something like '--error-exitcode=1'");
                return false;
            }
        }

        // User define
        else if (strncmp(argv[i], "-D", 2) == 0)
        {
            if (!_settings.userDefines.empty())
                _settings.userDefines += ";";
            _settings.userDefines += 2 + argv[i];
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
                    reportOut("cppcheck: argument to '-I' is missing");
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

            _settings._includePaths.push_back(path);
        }

        // file list specified
        else if (strncmp(argv[i], "--file-list=", 12) == 0)
        {
            // open this file and read every input file (1 file name per line)
            AddFilesToList(12 + argv[i], pathnames);
        }

        // Report progress
        else if (strcmp(argv[i], "--report-progress") == 0)
        {
            _settings.reportProgress = true;
        }

        // Output formatter
        else if (strcmp(argv[i], "--template") == 0)
        {
            // "--template path/"
            ++i;
            if (i >= argc)
            {
                reportOut("cppcheck: argument to '--template' is missing");
                return false;
            }

            _settings._outputFormat = argv[i];
            if (_settings._outputFormat == "gcc")
                _settings._outputFormat = "{file}:{line}: {severity}: {message}";
            else if (_settings._outputFormat == "vs")
                _settings._outputFormat = "{file}({line}): {severity}: {message}";
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
                    reportOut("cppcheck: argument to '-j' is missing");
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
            if (!(iss >> _settings._jobs))
            {
                reportOut("cppcheck: argument to '-j' is not a number");
                return false;
            }

            if (_settings._jobs > 1000)
            {
                reportOut("cppcheck: argument for '-j' is allowed to be 1000 at max");
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
            getErrorMessages();
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
            reportOut(doc2);
            return true;
        }

        // --test-2-pass Experimental 2-pass checking of files
        // This command line flag will be removed
        else if (strcmp(argv[i], "--test-2-pass") == 0)
        {
            _settings.test_2_pass = true;
        }

        else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0)
        {
            reportOut("cppcheck: error: unrecognized command line option \"" + std::string(argv[i]) + "\"");
            return false;
        }

        else
            pathnames.push_back(argv[i]);
    }

    if (_settings.isEnabled("unusedFunctions") && _settings._jobs > 1)
    {
        reportOut("unusedFunctions check can't be used with -j option, so it was disabled.");
    }

    // FIXME: Make the _settings.test_2_pass thread safe
    if (_settings.test_2_pass && _settings._jobs > 1)
    {
        reportOut("--test-2-pass doesn't work with -j option yet.");
    }

    if (!pathnames.empty())
    {
        // Execute recursiveAddFiles() to each given file parameter
        std::vector<std::string>::const_iterator iter;
        for (iter = pathnames.begin(); iter != pathnames.end(); ++iter)
            getFileLister()->recursiveAddFiles(_filenames, iter->c_str(), true);
    }

    if (argc <= 1 || showHelp)
    {
        std::ostringstream oss;
        oss <<   "Cppcheck - A tool for static C/C++ code analysis\n"
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
            "                          * exceptNew - exception safety when using new\n"
            "                          * exceptRealloc - exception safety when reallocating\n"
            "                          * style - Check coding style\n"
            "                          * unusedFunctions - check for unused functions\n"
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
        reportOut(oss.str());
    }
    else if (_filenames.empty())
    {
        reportOut("cppcheck: No C or C++ source files found.");
        return false;
    }

    return true;
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

    std::cout << ErrorLogger::ErrorMessage::getXMLFooter() << std::endl;
}
