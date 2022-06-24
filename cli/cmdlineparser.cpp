/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "cmdlineparser.h"

#include "check.h"
#include "config.h"
#include "cppcheckexecutor.h"
#include "errortypes.h"
#include "filelister.h"
#include "importproject.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"
#include "utils.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib> // EXIT_FAILURE
#include <cstring>
#include <iostream>
#include <list>
#include <set>

#ifdef HAVE_RULES
// xml is used for rules
#include <tinyxml2.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

static void addFilesToList(const std::string& fileList, std::vector<std::string>& pathNames)
{
    // To keep things initially simple, if the file can't be opened, just be silent and move on.
    std::istream *files;
    std::ifstream infile;
    if (fileList == "-") { // read from stdin
        files = &std::cin;
    } else {
        infile.open(fileList);
        files = &infile;
    }
    if (files && *files) {
        std::string fileName;
        while (std::getline(*files, fileName)) { // next line
            if (!fileName.empty()) {
                pathNames.emplace_back(fileName);
            }
        }
    }
}

static bool addIncludePathsToList(const std::string& fileList, std::list<std::string>* pathNames)
{
    std::ifstream files(fileList);
    if (files) {
        std::string pathName;
        while (std::getline(files, pathName)) { // next line
            if (!pathName.empty()) {
                pathName = Path::removeQuotationMarks(pathName);
                pathName = Path::fromNativeSeparators(pathName);

                // If path doesn't end with / or \, add it
                if (!endsWith(pathName, '/'))
                    pathName += '/';

                pathNames->emplace_back(pathName);
            }
        }
        return true;
    }
    return false;
}

static bool addPathsToSet(const std::string& fileName, std::set<std::string>* set)
{
    std::list<std::string> templist;
    if (!addIncludePathsToList(fileName, &templist))
        return false;
    set->insert(templist.begin(), templist.end());
    return true;
}

CmdLineParser::CmdLineParser(Settings *settings)
    : mSettings(settings)
    , mShowHelp(false)
    , mShowVersion(false)
    , mShowErrorMessages(false)
    , mExitAfterPrint(false)
{}

void CmdLineParser::printMessage(const std::string &message)
{
    std::cout << "cppcheck: " << message << std::endl;
}

void CmdLineParser::printError(const std::string &message)
{
    printMessage("error: " + message);
}

bool CmdLineParser::parseFromArgs(int argc, const char* const argv[])
{
    bool def = false;
    bool maxconfigs = false;

    mSettings->exename = argv[0];
#ifdef __linux__
    // Executing cppcheck in PATH. argv[0] does not contain the path.
    if (mSettings->exename.find_first_of("/\\") == std::string::npos) {
        char buf[PATH_MAX] = {0};
        if (FileLister::fileExists("/proc/self/exe") && readlink("/proc/self/exe", buf, sizeof(buf)-1) > 0)
            mSettings->exename = buf;
    }
#endif

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // User define
            if (std::strncmp(argv[i], "-D", 2) == 0) {
                std::string define;

                // "-D define"
                if (std::strcmp(argv[i], "-D") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-D' is missing.");
                        return false;
                    }

                    define = argv[i];
                }
                // "-Ddefine"
                else {
                    define = 2 + argv[i];
                }

                // No "=", append a "=1"
                if (define.find('=') == std::string::npos)
                    define += "=1";

                if (!mSettings->userDefines.empty())
                    mSettings->userDefines += ";";
                mSettings->userDefines += define;

                def = true;
            }

            // -E
            else if (std::strcmp(argv[i], "-E") == 0) {
                mSettings->preprocessOnly = true;
                mSettings->quiet = true;
            }

            // Include paths
            else if (std::strncmp(argv[i], "-I", 2) == 0) {
                std::string path;

                // "-I path/"
                if (std::strcmp(argv[i], "-I") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-I' is missing.");
                        return false;
                    }
                    path = argv[i];
                }

                // "-Ipath/"
                else {
                    path = 2 + argv[i];
                }
                path = Path::removeQuotationMarks(path);
                path = Path::fromNativeSeparators(path);

                // If path doesn't end with / or \, add it
                if (!endsWith(path,'/'))
                    path += '/';

                mSettings->includePaths.emplace_back(path);
            }

            // User undef
            else if (std::strncmp(argv[i], "-U", 2) == 0) {
                std::string undef;

                // "-U undef"
                if (std::strcmp(argv[i], "-U") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-U' is missing.");
                        return false;
                    }

                    undef = argv[i];
                }
                // "-Uundef"
                else {
                    undef = 2 + argv[i];
                }

                mSettings->userUndefs.insert(undef);
            }

            else if (std::strncmp(argv[i], "--addon=", 8) == 0)
                mSettings->addons.emplace_back(argv[i]+8);

            else if (std::strncmp(argv[i],"--addon-python=", 15) == 0)
                mSettings->addonPython.assign(argv[i]+15);

            // Check configuration
            else if (std::strcmp(argv[i], "--check-config") == 0)
                mSettings->checkConfiguration = true;

            // Check library definitions
            else if (std::strcmp(argv[i], "--check-library") == 0) {
                mSettings->checkLibrary = true;
                // need to add "information" or no messages will be shown at all
                mSettings->addEnabled("information");
            }

            else if (std::strncmp(argv[i], "--clang", 7) == 0) {
                mSettings->clang = true;
                if (std::strncmp(argv[i], "--clang=", 8) == 0) {
                    mSettings->clangExecutable = argv[i] + 8;
                }
            }

            else if (std::strncmp(argv[i], "--config-exclude=",17) ==0) {
                mSettings->configExcludePaths.insert(Path::fromNativeSeparators(argv[i] + 17));
            }

            else if (std::strncmp(argv[i], "--config-excludes-file=", 23) == 0) {
                // open this file and read every input file (1 file name per line)
                const std::string cfgExcludesFile(23 + argv[i]);
                if (!addPathsToSet(cfgExcludesFile, &mSettings->configExcludePaths)) {
                    printError("unable to open config excludes file at '" + cfgExcludesFile + "'");
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--cppcheck-build-dir=", 21) == 0) {
                mSettings->buildDir = Path::fromNativeSeparators(argv[i] + 21);
                if (endsWith(mSettings->buildDir, '/'))
                    mSettings->buildDir.pop_back();
            }

            // Show --debug output after the first simplifications
            else if (std::strcmp(argv[i], "--debug") == 0 ||
                     std::strcmp(argv[i], "--debug-normal") == 0)
                mSettings->debugnormal = true;

            // Flag used for various purposes during debugging
            else if (std::strcmp(argv[i], "--debug-simplified") == 0)
                mSettings->debugSimplified = true;

            // Show template information
            else if (std::strcmp(argv[i], "--debug-template") == 0)
                mSettings->debugtemplate = true;

            // Show debug warnings
            else if (std::strcmp(argv[i], "--debug-warnings") == 0)
                mSettings->debugwarnings = true;

            // documentation..
            else if (std::strcmp(argv[i], "--doc") == 0) {
                std::ostringstream doc;
                // Get documentation..
                for (const Check * it : Check::instances()) {
                    const std::string& name(it->name());
                    const std::string info(it->classInfo());
                    if (!name.empty() && !info.empty())
                        doc << "## " << name << " ##\n"
                            << info << "\n";
                }

                std::cout << doc.str();
                mExitAfterPrint = true;
                return true;
            }

            // dump cppcheck data
            else if (std::strcmp(argv[i], "--dump") == 0)
                mSettings->dump = true;

            else if (std::strncmp(argv[i], "--enable=", 9) == 0) {
                const std::string errmsg = mSettings->addEnabled(argv[i] + 9);
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
                // when "style" is enabled, also enable "warning", "performance" and "portability"
                if (mSettings->severity.isEnabled(Severity::style)) {
                    mSettings->addEnabled("warning");
                    mSettings->addEnabled("performance");
                    mSettings->addEnabled("portability");
                }
            }

            // print all possible error messages..
            else if (std::strcmp(argv[i], "--errorlist") == 0) {
                mShowErrorMessages = true;
                mSettings->xml = true;
                mExitAfterPrint = true;
            }

            // --error-exitcode=1
            else if (std::strncmp(argv[i], "--error-exitcode=", 17) == 0) {
                const std::string temp = argv[i]+17;
                std::istringstream iss(temp);
                if (!(iss >> mSettings->exitCode)) {
                    mSettings->exitCode = 0;
                    printError("argument must be an integer. Try something like '--error-exitcode=1'.");
                    return false;
                }
            }

            // Exception handling inside cppcheck client
            else if (std::strcmp(argv[i], "--exception-handling") == 0)
                mSettings->exceptionHandling = true;

            else if (std::strncmp(argv[i], "--exception-handling=", 21) == 0) {
                mSettings->exceptionHandling = true;
                const std::string exceptionOutfilename = &(argv[i][21]);
                CppCheckExecutor::setExceptionOutput((exceptionOutfilename=="stderr") ? stderr : stdout);
            }

            // Filter errors
            else if (std::strncmp(argv[i], "--exitcode-suppressions=", 24) == 0) {
                // exitcode-suppressions=filename.txt
                std::string filename = 24 + argv[i];

                std::ifstream f(filename);
                if (!f.is_open()) {
                    printError("couldn't open the file: \"" + filename + "\".");
                    return false;
                }
                const std::string errmsg(mSettings->nofail.parseFile(f));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            // use a file filter
            else if (std::strncmp(argv[i], "--file-filter=", 14) == 0)
                mSettings->fileFilters.emplace_back(argv[i] + 14);

            // file list specified
            else if (std::strncmp(argv[i], "--file-list=", 12) == 0)
                // open this file and read every input file (1 file name per line)
                addFilesToList(12 + argv[i], mPathNames);

            // Force checking of files that have "too many" configurations
            else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--force") == 0)
                mSettings->force = true;

            // Print help
            else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
                mPathNames.clear();
                mShowHelp = true;
                mExitAfterPrint = true;
                break;
            }

            // Ignored paths
            else if (std::strncmp(argv[i], "-i", 2) == 0) {
                std::string path;

                // "-i path/"
                if (std::strcmp(argv[i], "-i") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-i' is missing.");
                        return false;
                    }
                    path = argv[i];
                }

                // "-ipath/"
                else {
                    path = 2 + argv[i];
                }

                if (!path.empty()) {
                    path = Path::removeQuotationMarks(path);
                    path = Path::fromNativeSeparators(path);
                    path = Path::simplifyPath(path);

                    if (FileLister::isDirectory(path)) {
                        // If directory name doesn't end with / or \, add it
                        if (!endsWith(path, '/'))
                            path += '/';
                    }
                    mIgnoredPaths.emplace_back(path);
                }
            }

            else if (std::strncmp(argv[i], "--include=", 10) == 0) {
                mSettings->userIncludes.emplace_back(Path::fromNativeSeparators(argv[i] + 10));
            }

            else if (std::strncmp(argv[i], "--includes-file=", 16) == 0) {
                // open this file and read every input file (1 file name per line)
                const std::string includesFile(16 + argv[i]);
                if (!addIncludePathsToList(includesFile, &mSettings->includePaths)) {
                    printError("unable to open includes file at '" + includesFile + "'");
                    return false;
                }
            }

            // Inconclusive checking
            else if (std::strcmp(argv[i], "--inconclusive") == 0)
                mSettings->certainty.enable(Certainty::inconclusive);

            // Enables inline suppressions.
            else if (std::strcmp(argv[i], "--inline-suppr") == 0)
                mSettings->inlineSuppressions = true;

            // Checking threads
            else if (std::strncmp(argv[i], "-j", 2) == 0) {
                std::string numberString;

                // "-j 3"
                if (std::strcmp(argv[i], "-j") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-j' is missing.");
                        return false;
                    }

                    numberString = argv[i];
                }

                // "-j3"
                else
                    numberString = argv[i]+2;

                std::istringstream iss(numberString);
                if (!(iss >> mSettings->jobs)) {
                    printError("argument to '-j' is not a number.");
                    return false;
                }

                if (mSettings->jobs > 10000) {
                    // This limit is here just to catch typos. If someone has
                    // need for more jobs, this value should be increased.
                    printError("argument for '-j' is allowed to be 10000 at max.");
                    return false;
                }
            }

#ifdef THREADING_MODEL_FORK
            else if (std::strncmp(argv[i], "-l", 2) == 0) {
                std::string numberString;

                // "-l 3"
                if (std::strcmp(argv[i], "-l") == 0) {
                    ++i;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("argument to '-l' is missing.");
                        return false;
                    }

                    numberString = argv[i];
                }

                // "-l3"
                else
                    numberString = argv[i]+2;

                std::istringstream iss(numberString);
                if (!(iss >> mSettings->loadAverage)) {
                    printError("argument to '-l' is not a number.");
                    return false;
                }
            }
#endif

            // Enforce language (--language=, -x)
            else if (std::strncmp(argv[i], "--language=", 11) == 0 || std::strcmp(argv[i], "-x") == 0) {
                std::string str;
                if (argv[i][2]) {
                    str = argv[i]+11;
                } else {
                    i++;
                    if (i >= argc || argv[i][0] == '-') {
                        printError("no language given to '-x' option.");
                        return false;
                    }
                    str = argv[i];
                }

                if (str == "c")
                    mSettings->enforcedLang = Settings::C;
                else if (str == "c++")
                    mSettings->enforcedLang = Settings::CPP;
                else {
                    printError("unknown language '" + str + "' enforced.");
                    return false;
                }
            }

            // --library
            else if (std::strncmp(argv[i], "--library=", 10) == 0) {
                mSettings->libraries.emplace_back(argv[i] + 10);
            }

            // Set maximum number of #ifdef configurations to check
            else if (std::strncmp(argv[i], "--max-configs=", 14) == 0) {
                mSettings->force = false;

                std::istringstream iss(14+argv[i]);
                if (!(iss >> mSettings->maxConfigs)) {
                    printError("argument to '--max-configs=' is not a number.");
                    return false;
                }

                if (mSettings->maxConfigs < 1) {
                    printError("argument to '--max-configs=' must be greater than 0.");
                    return false;
                }

                maxconfigs = true;
            }

            // max ctu depth
            else if (std::strncmp(argv[i], "--max-ctu-depth=", 16) == 0)
                mSettings->maxCtuDepth = std::atoi(argv[i] + 16);

            // Write results in file
            else if (std::strncmp(argv[i], "--output-file=", 14) == 0)
                mSettings->outputFile = Path::simplifyPath(Path::fromNativeSeparators(argv[i] + 14));

            // Specify platform
            else if (std::strncmp(argv[i], "--platform=", 11) == 0) {
                const std::string platform(11+argv[i]);

                if (platform == "win32A")
                    mSettings->platform(Settings::Win32A);
                else if (platform == "win32W")
                    mSettings->platform(Settings::Win32W);
                else if (platform == "win64")
                    mSettings->platform(Settings::Win64);
                else if (platform == "unix32")
                    mSettings->platform(Settings::Unix32);
                else if (platform == "unix64")
                    mSettings->platform(Settings::Unix64);
                else if (platform == "native")
                    mSettings->platform(Settings::Native);
                else if (platform == "unspecified")
                    mSettings->platform(Settings::Unspecified);
                else if (!mSettings->loadPlatformFile(argv[0], platform)) {
                    std::string message("unrecognized platform: \"");
                    message += platform;
                    message += "\".";
                    printError(message);
                    return false;
                }
            }

            // Write results in results.plist
            else if (std::strncmp(argv[i], "--plist-output=", 15) == 0) {
                mSettings->plistOutput = Path::simplifyPath(Path::fromNativeSeparators(argv[i] + 15));
                if (mSettings->plistOutput.empty())
                    mSettings->plistOutput = "./";
                else if (!endsWith(mSettings->plistOutput,'/'))
                    mSettings->plistOutput += '/';

                const std::string plistOutput = Path::toNativeSeparators(mSettings->plistOutput);
                if (!FileLister::isDirectory(plistOutput)) {
                    std::string message("plist folder does not exist: \"");
                    message += plistOutput;
                    message += "\".";
                    printError(message);
                    return false;
                }
            }

            // --project
            else if (std::strncmp(argv[i], "--project=", 10) == 0) {
                mSettings->checkAllConfigurations = false; // Can be overridden with --max-configs or --force
                const std::string projectFile = argv[i]+10;
                ImportProject::Type projType = mSettings->project.import(projectFile, mSettings);
                mSettings->project.projectType = projType;
                if (projType == ImportProject::Type::CPPCHECK_GUI) {
                    mPathNames = mSettings->project.guiProject.pathNames;
                    for (const std::string &lib : mSettings->project.guiProject.libraries)
                        mSettings->libraries.emplace_back(lib);

                    for (const std::string &ignorePath : mSettings->project.guiProject.excludedPaths)
                        mIgnoredPaths.emplace_back(ignorePath);

                    const std::string platform(mSettings->project.guiProject.platform);

                    if (platform == "win32A")
                        mSettings->platform(Settings::Win32A);
                    else if (platform == "win32W")
                        mSettings->platform(Settings::Win32W);
                    else if (platform == "win64")
                        mSettings->platform(Settings::Win64);
                    else if (platform == "unix32")
                        mSettings->platform(Settings::Unix32);
                    else if (platform == "unix64")
                        mSettings->platform(Settings::Unix64);
                    else if (platform == "native")
                        mSettings->platform(Settings::Native);
                    else if (platform == "unspecified" || platform == "Unspecified" || platform.empty())
                        ;
                    else if (!mSettings->loadPlatformFile(projectFile.c_str(), platform) && !mSettings->loadPlatformFile(argv[0], platform)) {
                        std::string message("unrecognized platform: \"");
                        message += platform;
                        message += "\".";
                        printError(message);
                        return false;
                    }

                    if (!mSettings->project.guiProject.projectFile.empty())
                        projType = mSettings->project.import(mSettings->project.guiProject.projectFile, mSettings);
                }
                if (projType == ImportProject::Type::VS_SLN || projType == ImportProject::Type::VS_VCXPROJ) {
                    if (mSettings->project.guiProject.analyzeAllVsConfigs == "false")
                        mSettings->project.selectOneVsConfig(mSettings->platformType);
                    if (!CppCheckExecutor::tryLoadLibrary(mSettings->library, argv[0], "windows.cfg")) {
                        // This shouldn't happen normally.
                        printError("failed to load 'windows.cfg'. Your Cppcheck installation is broken. Please re-install.");
                        return false;
                    }
                }
                if (projType == ImportProject::Type::MISSING) {
                    printError("failed to open project '" + projectFile + "'. The file does not exist.");
                    return false;
                }
                if (projType == ImportProject::Type::UNKNOWN) {
                    printError("failed to load project '" + projectFile + "'. The format is unknown.");
                    return false;
                }
                if (projType == ImportProject::Type::FAILURE) {
                    printError("failed to load project '" + projectFile + "'. An error occurred.");
                    return false;
                }
            }

            // --project-configuration
            else if (std::strncmp(argv[i], "--project-configuration=", 24) == 0) {
                mVSConfig = argv[i] + 24;
                if (!mVSConfig.empty() && (mSettings->project.projectType == ImportProject::Type::VS_SLN || mSettings->project.projectType == ImportProject::Type::VS_VCXPROJ))
                    mSettings->project.ignoreOtherConfigs(mVSConfig);
            }

            // Only print something when there are errors
            else if (std::strcmp(argv[i], "-q") == 0 || std::strcmp(argv[i], "--quiet") == 0)
                mSettings->quiet = true;

            // Output relative paths
            else if (std::strcmp(argv[i], "-rp") == 0 || std::strcmp(argv[i], "--relative-paths") == 0)
                mSettings->relativePaths = true;
            else if (std::strncmp(argv[i], "-rp=", 4) == 0 || std::strncmp(argv[i], "--relative-paths=", 17) == 0) {
                mSettings->relativePaths = true;
                if (argv[i][argv[i][3]=='='?4:17] != 0) {
                    std::string paths = argv[i]+(argv[i][3]=='='?4:17);
                    for (;;) {
                        const std::string::size_type pos = paths.find(';');
                        if (pos == std::string::npos) {
                            mSettings->basePaths.emplace_back(Path::fromNativeSeparators(paths));
                            break;
                        }
                        mSettings->basePaths.emplace_back(Path::fromNativeSeparators(paths.substr(0, pos)));
                        paths.erase(0, pos + 1);
                    }
                } else {
                    printError("no paths specified for the '" + std::string(argv[i]) + "' option.");
                    return false;
                }
            }

            // Report progress
            else if (std::strcmp(argv[i], "--report-progress") == 0) {
                mSettings->reportProgress = true;
            }

#ifdef HAVE_RULES
            // Rule given at command line
            else if (std::strncmp(argv[i], "--rule=", 7) == 0) {
                Settings::Rule rule;
                rule.pattern = 7 + argv[i];
                mSettings->rules.emplace_back(rule);
            }

            // Rule file
            else if (std::strncmp(argv[i], "--rule-file=", 12) == 0) {
                tinyxml2::XMLDocument doc;
                if (doc.LoadFile(12+argv[i]) == tinyxml2::XML_SUCCESS) {
                    tinyxml2::XMLElement *node = doc.FirstChildElement();
                    if (node && strcmp(node->Value(), "rules") == 0)
                        node = node->FirstChildElement("rule");
                    for (; node && strcmp(node->Value(), "rule") == 0; node = node->NextSiblingElement()) {
                        Settings::Rule rule;

                        tinyxml2::XMLElement *tokenlist = node->FirstChildElement("tokenlist");
                        if (tokenlist)
                            rule.tokenlist = tokenlist->GetText();

                        tinyxml2::XMLElement *pattern = node->FirstChildElement("pattern");
                        if (pattern) {
                            rule.pattern = pattern->GetText();
                        }

                        tinyxml2::XMLElement *message = node->FirstChildElement("message");
                        if (message) {
                            tinyxml2::XMLElement *severity = message->FirstChildElement("severity");
                            if (severity)
                                rule.severity = Severity::fromString(severity->GetText());

                            tinyxml2::XMLElement *id = message->FirstChildElement("id");
                            if (id)
                                rule.id = id->GetText();

                            tinyxml2::XMLElement *summary = message->FirstChildElement("summary");
                            if (summary)
                                rule.summary = summary->GetText() ? summary->GetText() : "";
                        }

                        if (!rule.pattern.empty())
                            mSettings->rules.emplace_back(rule);
                    }
                } else {
                    printError("unable to load rule-file: " + std::string(12+argv[i]));
                    return false;
                }
            }
#endif

            // show timing information..
            else if (std::strncmp(argv[i], "--showtime=", 11) == 0) {
                const std::string showtimeMode = argv[i] + 11;
                if (showtimeMode == "file")
                    mSettings->showtime = SHOWTIME_MODES::SHOWTIME_FILE;
                else if (showtimeMode == "summary")
                    mSettings->showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY;
                else if (showtimeMode == "top5")
                    mSettings->showtime = SHOWTIME_MODES::SHOWTIME_TOP5;
                else if (showtimeMode.empty())
                    mSettings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
                else {
                    printError("unrecognized showtime mode: \"" + showtimeMode + "\". Supported modes: file, summary, top5.");
                    return false;
                }
            }

            // --std
            else if (std::strncmp(argv[i], "--std=", 6) == 0) {
                const std::string std = argv[i] + 6;
                // TODO: print error when standard is unknown
                if (std::strncmp(std.c_str(), "c++", 3) == 0) {
                    mSettings->standards.cpp = Standards::getCPP(std);
                }
                else if (std::strncmp(std.c_str(), "c", 1) == 0) {
                    mSettings->standards.c = Standards::getC(std);
                }
                else {
                    printError("unknown --std value '" + std + "'");
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--suppress=", 11) == 0) {
                const std::string suppression = argv[i]+11;
                const std::string errmsg(mSettings->nomsg.addSuppressionLine(suppression));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            // Filter errors
            else if (std::strncmp(argv[i], "--suppressions-list=", 20) == 0) {
                std::string filename = argv[i]+20;
                std::ifstream f(filename);
                if (!f.is_open()) {
                    std::string message("couldn't open the file: \"");
                    message += filename;
                    message += "\".";
                    if (std::count(filename.begin(), filename.end(), ',') > 0 ||
                        std::count(filename.begin(), filename.end(), '.') > 1) {
                        // If user tried to pass multiple files (we can only guess that)
                        // e.g. like this: --suppressions-list=a.txt,b.txt
                        // print more detailed error message to tell user how he can solve the problem
                        message += "\nIf you want to pass two files, you can do it e.g. like this:";
                        message += "\n    cppcheck --suppressions-list=a.txt --suppressions-list=b.txt file.cpp";
                    }

                    printError(message);
                    return false;
                }
                const std::string errmsg(mSettings->nomsg.parseFile(f));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--suppress-xml=", 15) == 0) {
                const char * filename = argv[i] + 15;
                const std::string errmsg(mSettings->nomsg.parseXmlFile(filename));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            // Output formatter
            else if (std::strcmp(argv[i], "--template") == 0 ||
                     std::strncmp(argv[i], "--template=", 11) == 0) {
                // "--template format"
                if (argv[i][10] == '=')
                    mSettings->templateFormat = argv[i] + 11;
                else if ((i+1) < argc && argv[i+1][0] != '-') {
                    ++i;
                    mSettings->templateFormat = argv[i];
                } else {
                    printError("argument to '--template' is missing.");
                    return false;
                }

                if (mSettings->templateFormat == "gcc") {
                    mSettings->templateFormat = "{bold}{file}:{line}:{column}: {magenta}warning:{default} {message} [{id}]{reset}\\n{code}";
                    mSettings->templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
                } else if (mSettings->templateFormat == "daca2") {
                    mSettings->daca = true;
                    mSettings->templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
                    mSettings->templateLocation = "{file}:{line}:{column}: note: {info}";
                } else if (mSettings->templateFormat == "vs")
                    mSettings->templateFormat = "{file}({line}): {severity}: {message}";
                else if (mSettings->templateFormat == "edit")
                    mSettings->templateFormat = "{file} +{line}: {severity}: {message}";
                else if (mSettings->templateFormat == "cppcheck1")
                    mSettings->templateFormat = "{callstack}: ({severity}{inconclusive:, inconclusive}) {message}";
                else if (mSettings->templateFormat == "selfcheck") {
                    mSettings->templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\\n{code}";
                    mSettings->templateLocation = "{file}:{line}:{column}: note: {info}\\n{code}";
                    mSettings->daca = true;
                }
            }

            else if (std::strcmp(argv[i], "--template-location") == 0 ||
                     std::strncmp(argv[i], "--template-location=", 20) == 0) {
                // "--template-location format"
                if (argv[i][19] == '=')
                    mSettings->templateLocation = argv[i] + 20;
                else if ((i+1) < argc && argv[i+1][0] != '-') {
                    ++i;
                    mSettings->templateLocation = argv[i];
                } else {
                    printError("argument to '--template' is missing.");
                    return false;
                }
            }

            else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--verbose") == 0)
                mSettings->verbose = true;

            else if (std::strcmp(argv[i], "--version") == 0) {
                mShowVersion = true;
                mExitAfterPrint = true;
                mSettings->loadCppcheckCfg();
                return true;
            }

            // Write results in results.xml
            else if (std::strcmp(argv[i], "--xml") == 0)
                mSettings->xml = true;

            // Define the XML file version (and enable XML output)
            else if (std::strncmp(argv[i], "--xml-version=", 14) == 0) {
                const std::string numberString(argv[i]+14);

                std::istringstream iss(numberString);
                if (!(iss >> mSettings->xml_version)) {
                    printError("argument to '--xml-version' is not a number.");
                    return false;
                }

                if (mSettings->xml_version != 2) {
                    // We only have xml version 2
                    printError("'--xml-version' can only be 2.");
                    return false;
                }

                // Enable also XML if version is set
                mSettings->xml = true;
            }

            else {
                std::string message("unrecognized command line option: \"");
                message += argv[i];
                message += "\".";
                printError(message);
                return false;
            }
        }

        else {
            mPathNames.emplace_back(Path::fromNativeSeparators(Path::removeQuotationMarks(argv[i])));
        }
    }

    mSettings->loadCppcheckCfg();

    // Default template format..
    if (mSettings->templateFormat.empty()) {
        mSettings->templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
        if (mSettings->templateLocation.empty())
            mSettings->templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
    }

    mSettings->project.ignorePaths(mIgnoredPaths);

    if (mSettings->force || maxconfigs)
        mSettings->checkAllConfigurations = true;

    if (mSettings->force)
        mSettings->maxConfigs = INT_MAX;

    else if ((def || mSettings->preprocessOnly) && !maxconfigs)
        mSettings->maxConfigs = 1U;

    if (mSettings->checks.isEnabled(Checks::unusedFunction) && mSettings->jobs > 1) {
        printMessage("unusedFunction check can't be used with '-j' option. Disabling unusedFunction check.");
    }

    if (argc <= 1) {
        mShowHelp = true;
        mExitAfterPrint = true;
    }

    if (mShowHelp) {
        printHelp();
        return true;
    }

    // Print error only if we have "real" command and expect files
    if (!mExitAfterPrint && mPathNames.empty() && mSettings->project.fileSettings.empty()) {
        printError("no C or C++ source files found.");
        return false;
    }

    // Use paths _pathnames if no base paths for relative path output are given
    if (mSettings->basePaths.empty() && mSettings->relativePaths)
        mSettings->basePaths = mPathNames;

    return true;
}

void CmdLineParser::printHelp()
{
    std::cout << "Cppcheck - A tool for static C/C++ code analysis\n"
        "\n"
        "Syntax:\n"
        "    cppcheck [OPTIONS] [files or paths]\n"
        "\n"
        "If a directory is given instead of a filename, *.cpp, *.cxx, *.cc, *.c++, *.c, *.ipp,\n"
        "*.ixx, *.tpp, and *.txx files are checked recursively from the given directory.\n\n"
        "Options:\n"
        "    --addon=<addon>\n"
        "                         Execute addon. i.e. --addon=cert. If options must be\n"
        "                         provided a json configuration is needed.\n"
        "    --addon-python=<python interpreter>\n"
        "                         You can specify the python interpreter either in the\n"
        "                         addon json files or through this command line option.\n"
        "                         If not present, Cppcheck will try \"python3\" first and\n"
        "                         then \"python\".\n"
        "    --bug-hunting\n"
        "                         Enable noisy and soundy analysis. The normal Cppcheck\n"
        "                         analysis is turned off.\n"
        "    --cppcheck-build-dir=<dir>\n"
        "                         Cppcheck work folder. Advantages:\n"
        "                          * whole program analysis\n"
        "                          * faster analysis; Cppcheck will reuse the results if\n"
        "                            the hash for a file is unchanged.\n"
        "                          * some useful debug information, i.e. commands used to\n"
        "                            execute clang/clang-tidy/addons.\n"
        "    --check-config       Check cppcheck configuration. The normal code\n"
        "                         analysis is disabled by this flag.\n"
        "    --check-library      Show information messages when library files have\n"
        "                         incomplete info.\n"
        "    --clang=<path>       Experimental: Use Clang parser instead of the builtin Cppcheck\n"
        "                         parser. Takes the executable as optional parameter and\n"
        "                         defaults to `clang`. Cppcheck will run the given Clang\n"
        "                         executable, import the Clang AST and convert it into\n"
        "                         Cppcheck data. After that the normal Cppcheck analysis is\n"
        "                         used. You must have the executable in PATH if no path is\n"
        "                         given.\n"
        "    --config-exclude=<dir>\n"
        "                         Path (prefix) to be excluded from configuration\n"
        "                         checking. Preprocessor configurations defined in\n"
        "                         headers (but not sources) matching the prefix will not\n"
        "                         be considered for evaluation.\n"
        "    --config-excludes-file=<file>\n"
        "                         A file that contains a list of config-excludes\n"
        "    --dump               Dump xml data for each translation unit. The dump\n"
        "                         files have the extension .dump and contain ast,\n"
        "                         tokenlist, symboldatabase, valueflow.\n"
        "    -D<ID>               Define preprocessor symbol. Unless --max-configs or\n"
        "                         --force is used, Cppcheck will only check the given\n"
        "                         configuration when -D is used.\n"
        "                         Example: '-DDEBUG=1 -D__cplusplus'.\n"
        "    -E                   Print preprocessor output on stdout and don't do any\n"
        "                         further processing.\n"
        "    --enable=<id>        Enable additional checks. The available ids are:\n"
        "                          * all\n"
        "                                  Enable all checks. It is recommended to only\n"
        "                                  use --enable=all when the whole program is\n"
        "                                  scanned, because this enables unusedFunction.\n"
        "                          * warning\n"
        "                                  Enable warning messages\n"
        "                          * style\n"
        "                                  Enable all coding style checks. All messages\n"
        "                                  with the severities 'style', 'warning',\n"
        "                                  'performance' and 'portability' are enabled.\n"
        "                          * performance\n"
        "                                  Enable performance messages\n"
        "                          * portability\n"
        "                                  Enable portability messages\n"
        "                          * information\n"
        "                                  Enable information messages\n"
        "                          * unusedFunction\n"
        "                                  Check for unused functions. It is recommended\n"
        "                                  to only enable this when the whole program is\n"
        "                                  scanned.\n"
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
        "    --file-filter=<str>  Analyze only those files matching the given filter str\n"
        "                         Can be used multiple times\n"
        "                         Example: --file-filter=*bar.cpp analyzes only files\n"
        "                                  that end with bar.cpp.\n"
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
        "    --include=<file>\n"
        "                         Force inclusion of a file before the checked file.\n"
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
        "    -j <jobs>            Start <jobs> threads to do the checking simultaneously.\n"
#ifdef THREADING_MODEL_FORK
    "    -l <load>            Specifies that no new threads should be started if\n"
    "                         there are other threads running and the load average is\n"
    "                         at least <load>.\n"
#endif
    "    --language=<language>, -x <language>\n"
    "                         Forces cppcheck to check all files as the given\n"
    "                         language. Valid values are: c, c++\n"
    "    --library=<cfg>      Load file <cfg> that contains information about types\n"
    "                         and functions. With such information Cppcheck\n"
    "                         understands your code better and therefore you\n"
    "                         get better results. The std.cfg file that is\n"
    "                         distributed with Cppcheck is loaded automatically.\n"
    "                         For more information about library files, read the\n"
    "                         manual.\n"
    "    --max-ctu-depth=N    Max depth in whole program analysis. The default value\n"
    "                         is 2. A larger value will mean more errors can be found\n"
    "                         but also means the analysis will be slower.\n"
    "    --output-file=<file> Write results to file, rather than standard error.\n"
    "    --project=<file>     Run Cppcheck on project. The <file> can be a Visual\n"
    "                         Studio Solution (*.sln), Visual Studio Project\n"
    "                         (*.vcxproj), compile database (compile_commands.json),\n"
    "                         or Borland C++ Builder 6 (*.bpr). The files to analyse,\n"
    "                         include paths, defines, platform and undefines in\n"
    "                         the specified file will be used.\n"
    "    --project-configuration=<config>\n"
    "                         If used together with a Visual Studio Solution (*.sln)\n"
    "                         or Visual Studio Project (*.vcxproj) you can limit\n"
    "                         the configuration cppcheck should check.\n"
    "                         For example: '--project-configuration=Release|Win32'\n"
    "    --max-configs=<limit>\n"
    "                         Maximum number of configurations to check in a file\n"
    "                         before skipping it. Default is '12'. If used together\n"
    "                         with '--force', the last option is the one that is\n"
    "                         effective.\n"
    "    --platform=<type>, --platform=<file>\n"
    "                         Specifies platform specific types and sizes. The\n"
    "                         available builtin platforms are:\n"
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
    "                          * avr8\n"
    "                                 8 bit AVR microcontrollers\n"
    "                          * elbrus-e1cp\n"
    "                                 Elbrus e1c+ architecture\n"
    "                          * pic8\n"
    "                                 8 bit PIC microcontrollers\n"
    "                                 Baseline and mid-range architectures\n"
    "                          * pic8-enhanced\n"
    "                                 8 bit PIC microcontrollers\n"
    "                                 Enhanced mid-range and high end (PIC18) architectures\n"
    "                          * pic16\n"
    "                                 16 bit PIC microcontrollers\n"
    "                          * mips32\n"
    "                                 32 bit MIPS microcontrollers\n"
    "                          * native\n"
    "                                 Type sizes of host system are assumed, but no\n"
    "                                 further assumptions.\n"
    "                          * unspecified\n"
    "                                 Unknown type sizes\n"
    "    --plist-output=<path>\n"
    "                         Generate Clang-plist output files in folder.\n"
    "    -q, --quiet          Do not show progress reports.\n"
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
    "    --rule-file=<file>   Use given rule file. For more information, see:\n"
    "                         http://sourceforge.net/projects/cppcheck/files/Articles/\n"
#endif
    "    --std=<id>           Set standard.\n"
    "                         The available options are:\n"
    "                          * c89\n"
    "                                 C code is C89 compatible\n"
    "                          * c99\n"
    "                                 C code is C99 compatible\n"
    "                          * c11\n"
    "                                 C code is C11 compatible (default)\n"
    "                          * c++03\n"
    "                                 C++ code is C++03 compatible\n"
    "                          * c++11\n"
    "                                 C++ code is C++11 compatible\n"
    "                          * c++14\n"
    "                                 C++ code is C++14 compatible\n"
    "                          * c++17\n"
    "                                 C++ code is C++17 compatible\n"
    "                          * c++20\n"
    "                                 C++ code is C++20 compatible (default)\n"
    "    --suppress=<spec>    Suppress warnings that match <spec>. The format of\n"
    "                         <spec> is:\n"
    "                         [error id]:[filename]:[line]\n"
    "                         The [filename] and [line] are optional. If [error id]\n"
    "                         is a wildcard '*', all error ids match.\n"
    "    --suppressions-list=<file>\n"
    "                         Suppress warnings listed in the file. Each suppression\n"
    "                         is in the same format as <spec> above.\n"
    "    --suppress-xml=<file>\n"
    "                         Suppress warnings listed in a xml file. XML file should\n"
    "                         follow the manual.pdf format specified in section.\n"
    "                         `6.4 XML suppressions` .\n"
    "    --template='<text>'  Format the error messages. Available fields:\n"
    "                           {file}              file name\n"
    "                           {line}              line number\n"
    "                           {column}            column number\n"
    "                           {callstack}         show a callstack. Example:\n"
    "                                                 [file.c:1] -> [file.c:100]\n"
    "                           {inconclusive:text} if warning is inconclusive, text\n"
    "                                               is written\n"
    "                           {severity}          severity\n"
    "                           {message}           warning message\n"
    "                           {id}                warning id\n"
    "                           {cwe}               CWE id (Common Weakness Enumeration)\n"
    "                           {code}              show the real code\n"
    "                           \\t                 insert tab\n"
    "                           \\n                 insert newline\n"
    "                           \\r                 insert carriage return\n"
    "                         Example formats:\n"
    "                         '{file}:{line},{severity},{id},{message}' or\n"
    "                         '{file}({line}):({severity}) {message}' or\n"
    "                         '{callstack} {message}'\n"
    "                         Pre-defined templates: gcc (default), cppcheck1 (old default), vs, edit.\n"
    // Note: template daca2 also exists, but is for internal use (cppcheck scripts).
    "    --template-location='<text>'\n"
    "                         Format error message location. If this is not provided\n"
    "                         then no extra location info is shown.\n"
    "                         Available fields:\n"
    "                           {file}      file name\n"
    "                           {line}      line number\n"
    "                           {column}    column number\n"
    "                           {info}      location info\n"
    "                           {code}      show the real code\n"
    "                           \\t         insert tab\n"
    "                           \\n         insert newline\n"
    "                           \\r         insert carriage return\n"
    "                         Example format (gcc-like):\n"
    "                         '{file}:{line}:{column}: note: {info}\\n{code}'\n"
    "    -U<ID>               Undefine preprocessor symbol. Use -U to explicitly\n"
    "                         hide certain #ifdef <ID> code paths from checking.\n"
    "                         Example: '-UDEBUG'\n"
    "    -v, --verbose        Output more detailed error information.\n"
    "    --version            Print out version number.\n"
    "    --xml                Write results in xml format to error stream (stderr).\n"
    "    --xml-version=<version>\n"
    "                         Select the XML file version. Also implies --xml.\n"
    "                         Currently only version 2 is available. The default version is 2.\n"
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
    "  cppcheck --enable=all --inconclusive --library=posix test.cpp\n"
    "\n"
    "  # Check f.cpp and search include files from inc1/ and inc2/:\n"
    "  cppcheck -I inc1/ -I inc2/ f.cpp\n"
    "\n"
    "For more information:\n"
    "    https://cppcheck.sourceforge.io/manual.pdf\n"
    "\n"
    "Many thanks to the 3rd party libraries we use:\n"
    " * tinyxml2 -- loading project/library/ctu files.\n"
    " * picojson -- loading compile database.\n"
    " * pcre -- rules.\n"
    " * qt -- used in GUI\n";
}
