/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include "errorlogger.h"
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
#include <fstream> // IWYU pragma: keep
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <sstream> // IWYU pragma: keep
#include <unordered_set>
#include <utility>

#ifdef HAVE_RULES
// xml is used for rules
#include <tinyxml2.h>
#endif

static bool addFilesToList(const std::string& fileList, std::vector<std::string>& pathNames)
{
    std::istream *files;
    std::ifstream infile;
    if (fileList == "-") { // read from stdin
        files = &std::cin;
    } else {
        infile.open(fileList);
        if (!infile.is_open())
            return false;
        files = &infile;
    }
    if (files && *files) {
        std::string fileName;
        // cppcheck-suppress accessMoved - FP
        while (std::getline(*files, fileName)) { // next line
            // cppcheck-suppress accessMoved - FP
            if (!fileName.empty()) {
                pathNames.emplace_back(std::move(fileName));
            }
        }
    }
    return true;
}

static bool addIncludePathsToList(const std::string& fileList, std::list<std::string>& pathNames)
{
    std::ifstream files(fileList);
    if (files) {
        std::string pathName;
        // cppcheck-suppress accessMoved - FP
        while (std::getline(files, pathName)) { // next line
            if (!pathName.empty()) {
                pathName = Path::removeQuotationMarks(pathName);
                pathName = Path::fromNativeSeparators(pathName);

                // If path doesn't end with / or \, add it
                if (!endsWith(pathName, '/'))
                    pathName += '/';

                pathNames.emplace_back(std::move(pathName));
            }
        }
        return true;
    }
    return false;
}

static bool addPathsToSet(const std::string& fileName, std::set<std::string>& set)
{
    std::list<std::string> templist;
    if (!addIncludePathsToList(fileName, templist))
        return false;
    set.insert(templist.cbegin(), templist.cend());
    return true;
}

CmdLineParser::CmdLineParser(Settings &settings, Suppressions &suppressions, Suppressions &suppressionsNoFail)
    : mSettings(settings)
    , mSuppressions(suppressions)
    , mSuppressionsNoFail(suppressionsNoFail)
{}

void CmdLineParser::printMessage(const std::string &message)
{
    std::cout << "cppcheck: " << message << std::endl;
}

void CmdLineParser::printError(const std::string &message)
{
    printMessage("error: " + message);
}

#if defined(_WIN64) || defined(_WIN32)
bool CmdLineParser::SHOW_DEF_PLATFORM_MSG = true;
#endif

// TODO: normalize/simplify/native all path parameters
// TODO: error out on all missing given files/paths
bool CmdLineParser::parseFromArgs(int argc, const char* const argv[])
{
#if defined(_WIN64) || defined(_WIN32)
    bool default_platform = true;
#endif

    bool def = false;
    bool maxconfigs = false;

    mSettings.exename = Path::getCurrentExecutablePath(argv[0]);

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

                if (!mSettings.userDefines.empty())
                    mSettings.userDefines += ";";
                mSettings.userDefines += define;

                def = true;
            }

            // -E
            else if (std::strcmp(argv[i], "-E") == 0) {
                mSettings.preprocessOnly = true;
                mSettings.quiet = true;
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

                mSettings.includePaths.emplace_back(std::move(path));
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

                mSettings.userUndefs.insert(std::move(undef));
            }

            else if (std::strncmp(argv[i], "--addon=", 8) == 0)
                mSettings.addons.emplace(argv[i]+8);

            else if (std::strncmp(argv[i],"--addon-python=", 15) == 0)
                mSettings.addonPython.assign(argv[i]+15);

            // Check configuration
            else if (std::strcmp(argv[i], "--check-config") == 0)
                mSettings.checkConfiguration = true;

            // Check code exhaustively
            else if (std::strcmp(argv[i], "--check-level=exhaustive") == 0)
                mSettings.setCheckLevelExhaustive();

            // Check code with normal analysis
            else if (std::strcmp(argv[i], "--check-level=normal") == 0)
                mSettings.setCheckLevelNormal();

            // Check library definitions
            else if (std::strcmp(argv[i], "--check-library") == 0) {
                mSettings.checkLibrary = true;
            }

            else if (std::strncmp(argv[i], "--checkers-report=", 18) == 0)
                mSettings.checkersReportFilename = argv[i] + 18;

            else if (std::strncmp(argv[i], "--checks-max-time=", 18) == 0) {
                if (!parseNumberArg(argv[i], 18, mSettings.checksMaxTime, true))
                    return false;
            }

            else if (std::strcmp(argv[i], "--clang") == 0) {
                mSettings.clang = true;
            }

            else if (std::strncmp(argv[i], "--clang=", 8) == 0) {
                mSettings.clang = true;
                mSettings.clangExecutable = argv[i] + 8;
            }

            else if (std::strncmp(argv[i], "--config-exclude=",17) ==0) {
                mSettings.configExcludePaths.insert(Path::fromNativeSeparators(argv[i] + 17));
            }

            else if (std::strncmp(argv[i], "--config-excludes-file=", 23) == 0) {
                // open this file and read every input file (1 file name per line)
                const std::string cfgExcludesFile(23 + argv[i]);
                if (!addPathsToSet(cfgExcludesFile, mSettings.configExcludePaths)) {
                    printError("unable to open config excludes file at '" + cfgExcludesFile + "'");
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--cppcheck-build-dir=", 21) == 0) {
                mSettings.buildDir = Path::fromNativeSeparators(argv[i] + 21);
                if (endsWith(mSettings.buildDir, '/'))
                    mSettings.buildDir.pop_back();

                if (!Path::isDirectory(mSettings.buildDir)) {
                    printError("Directory '" + mSettings.buildDir + "' specified by --cppcheck-build-dir argument has to be existent.");
                    return false;
                }
            }

            // Show --debug output after the first simplifications
            else if (std::strcmp(argv[i], "--debug") == 0 ||
                     std::strcmp(argv[i], "--debug-normal") == 0)
                mSettings.debugnormal = true;

            // Flag used for various purposes during debugging
            else if (std::strcmp(argv[i], "--debug-simplified") == 0)
                mSettings.debugSimplified = true;

            // Show template information
            else if (std::strcmp(argv[i], "--debug-template") == 0)
                mSettings.debugtemplate = true;

            // Show debug warnings
            else if (std::strcmp(argv[i], "--debug-warnings") == 0)
                mSettings.debugwarnings = true;

            else if (std::strncmp(argv[i], "--disable=", 10) == 0) {
                const std::string errmsg = mSettings.removeEnabled(argv[i] + 10);
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

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
                mSettings.dump = true;

            else if (std::strncmp(argv[i], "--enable=", 9) == 0) {
                const std::string enable_arg = argv[i] + 9;
                const std::string errmsg = mSettings.addEnabled(enable_arg);
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
                // when "style" is enabled, also enable "warning", "performance" and "portability"
                if (enable_arg.find("style") != std::string::npos) {
                    mSettings.addEnabled("warning");
                    mSettings.addEnabled("performance");
                    mSettings.addEnabled("portability");
                }
                if (enable_arg.find("information") != std::string::npos) {
                    mSettings.addEnabled("missingInclude");
                    printMessage("'--enable=information' will no longer implicitly enable 'missingInclude' starting with 2.16. Please enable it explicitly if you require it.");
                }
            }

            // print all possible error messages..
            else if (std::strcmp(argv[i], "--errorlist") == 0) {
                mShowErrorMessages = true;
                mSettings.xml = true;
                mExitAfterPrint = true;
            }

            // --error-exitcode=1
            else if (std::strncmp(argv[i], "--error-exitcode=", 17) == 0) {
                if (!parseNumberArg(argv[i], 17, mSettings.exitCode))
                    return false;
            }

            // Exception handling inside cppcheck client
            else if (std::strcmp(argv[i], "--exception-handling") == 0) {
                mSettings.exceptionHandling = true;
            }

            // Exception handling inside cppcheck client
            else if (std::strncmp(argv[i], "--exception-handling=", 21) == 0) {
                const std::string exceptionOutfilename = argv[i] + 21;
                if (exceptionOutfilename != "stderr" && exceptionOutfilename != "stdout") {
                    printError("invalid '--exception-handling' argument");
                    return false;
                }
                mSettings.exceptionHandling = true;
                CppCheckExecutor::setExceptionOutput((exceptionOutfilename == "stderr") ? stderr : stdout);
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
                const std::string errmsg(mSuppressionsNoFail.parseFile(f));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            // use a file filter
            else if (std::strncmp(argv[i], "--file-filter=", 14) == 0)
                mSettings.fileFilters.emplace_back(argv[i] + 14);

            // file list specified
            else if (std::strncmp(argv[i], "--file-list=", 12) == 0) {
                // open this file and read every input file (1 file name per line)
                const std::string fileList = argv[i] + 12;
                if (!addFilesToList(fileList, mPathNames)) {
                    printError("couldn't open the file: \"" + fileList + "\".");
                    return false;
                }
            }

            // Force checking of files that have "too many" configurations
            else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--force") == 0)
                mSettings.force = true;

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

                    if (Path::isDirectory(path)) {
                        // If directory name doesn't end with / or \, add it
                        if (!endsWith(path, '/'))
                            path += '/';
                    }
                    mIgnoredPaths.emplace_back(std::move(path));
                }
            }

            else if (std::strncmp(argv[i], "--include=", 10) == 0) {
                mSettings.userIncludes.emplace_back(Path::fromNativeSeparators(argv[i] + 10));
            }

            else if (std::strncmp(argv[i], "--includes-file=", 16) == 0) {
                // open this file and read every input file (1 file name per line)
                const std::string includesFile(16 + argv[i]);
                if (!addIncludePathsToList(includesFile, mSettings.includePaths)) {
                    printError("unable to open includes file at '" + includesFile + "'");
                    return false;
                }
            }

            // Inconclusive checking
            else if (std::strcmp(argv[i], "--inconclusive") == 0)
                mSettings.certainty.enable(Certainty::inconclusive);

            // Enables inline suppressions.
            else if (std::strcmp(argv[i], "--inline-suppr") == 0)
                mSettings.inlineSuppressions = true;

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

                unsigned int tmp;
                std::string err;
                if (!strToInt(numberString, tmp, &err)) {
                    printError("argument to '-j' is not valid - " +  err + ".");
                    return false;
                }
                if (tmp == 0) {
                    // TODO: implement get CPU logical core count and use that.
                    // Usually, -j 0 would mean "use all available cores," but
                    // if we get a 0, we just stall and don't do any work.
                    printError("argument for '-j' must be greater than 0.");
                    return false;
                }
                if (tmp > 1024) {
                    // Almost nobody has 1024 logical cores, but somebody out
                    // there does.
                    printError("argument for '-j' is allowed to be 1024 at max.");
                    return false;
                }
                mSettings.jobs = tmp;
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

                int tmp;
                std::string err;
                if (!strToInt(numberString, tmp, &err)) {
                    printError("argument to '-l' is not valid - " + err + ".");
                    return false;
                }
                mSettings.loadAverage = tmp;
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
                    mSettings.enforcedLang = Settings::Language::C;
                else if (str == "c++")
                    mSettings.enforcedLang = Settings::Language::CPP;
                else {
                    printError("unknown language '" + str + "' enforced.");
                    return false;
                }
            }

            // --library
            else if (std::strncmp(argv[i], "--library=", 10) == 0) {
                mSettings.libraries.emplace_back(argv[i] + 10);
            }

            // Set maximum number of #ifdef configurations to check
            else if (std::strncmp(argv[i], "--max-configs=", 14) == 0) {
                int tmp;
                if (!parseNumberArg(argv[i], 14, tmp))
                    return false;
                if (tmp < 1) {
                    printError("argument to '--max-configs=' must be greater than 0.");
                    return false;
                }

                mSettings.maxConfigs = tmp;
                mSettings.force = false;
                maxconfigs = true;
            }

            // max ctu depth
            else if (std::strncmp(argv[i], "--max-ctu-depth=", 16) == 0) {
                if (!parseNumberArg(argv[i], 16, mSettings.maxCtuDepth))
                    return false;
            }

            // Write results in file
            else if (std::strncmp(argv[i], "--output-file=", 14) == 0)
                mSettings.outputFile = Path::simplifyPath(Path::fromNativeSeparators(argv[i] + 14));

            // Experimental: limit execution time for extended valueflow analysis. basic valueflow analysis
            // is always executed.
            else if (std::strncmp(argv[i], "--performance-valueflow-max-time=", 33) == 0) {
                if (!parseNumberArg(argv[i], 33, mSettings.performanceValueFlowMaxTime, true))
                    return false;
            }

            else if (std::strncmp(argv[i], "--performance-valueflow-max-if-count=", 37) == 0) {
                if (!parseNumberArg(argv[i], 37, mSettings.performanceValueFlowMaxIfCount, true))
                    return false;
            }

            // Specify platform
            else if (std::strncmp(argv[i], "--platform=", 11) == 0) {
                const std::string platform(11+argv[i]);

                std::string errstr;
                const std::vector<std::string> paths = {argv[0]};
                if (!mSettings.platform.set(platform, errstr, paths)) {
                    printError(errstr);
                    return false;
                }

#if defined(_WIN64) || defined(_WIN32)
                default_platform = false;
#endif

                // TODO: remove
                // these are loaded via external files and thus have Settings::PlatformFile set instead.
                // override the type so they behave like the regular platforms.
                if (platform == "unix32-unsigned")
                    mSettings.platform.type = cppcheck::Platform::Type::Unix32;
                else if (platform == "unix64-unsigned")
                    mSettings.platform.type = cppcheck::Platform::Type::Unix64;
            }

            // Write results in results.plist
            else if (std::strncmp(argv[i], "--plist-output=", 15) == 0) {
                mSettings.plistOutput = Path::simplifyPath(Path::fromNativeSeparators(argv[i] + 15));
                if (mSettings.plistOutput.empty())
                    mSettings.plistOutput = "./";
                else if (!endsWith(mSettings.plistOutput,'/'))
                    mSettings.plistOutput += '/';

                const std::string plistOutput = Path::toNativeSeparators(mSettings.plistOutput);
                if (!Path::isDirectory(plistOutput)) {
                    std::string message("plist folder does not exist: \"");
                    message += plistOutput;
                    message += "\".";
                    printError(message);
                    return false;
                }
            }

            // Special Cppcheck Premium options
            else if (std::strncmp(argv[i], "--premium=", 10) == 0 && isCppcheckPremium()) {
                if (!mSettings.premiumArgs.empty())
                    mSettings.premiumArgs += " ";
                const std::string p(argv[i] + 10);
                mSettings.premiumArgs += "--" + p;
                if (p == "misra-c-2012" || p == "misra-c-2023")
                    mSettings.addons.emplace("misra");
            }

            // --project
            else if (std::strncmp(argv[i], "--project=", 10) == 0) {
                mSettings.checkAllConfigurations = false; // Can be overridden with --max-configs or --force
                std::string projectFile = argv[i]+10;
                ImportProject::Type projType = mSettings.project.import(projectFile, &mSettings);
                mSettings.project.projectType = projType;
                if (projType == ImportProject::Type::CPPCHECK_GUI) {
                    mPathNames = mSettings.project.guiProject.pathNames;
                    for (const std::string &lib : mSettings.project.guiProject.libraries)
                        mSettings.libraries.emplace_back(lib);

                    const auto& excludedPaths = mSettings.project.guiProject.excludedPaths;
                    std::copy(excludedPaths.cbegin(), excludedPaths.cend(), std::back_inserter(mIgnoredPaths));

                    std::string platform(mSettings.project.guiProject.platform);

                    // keep existing platform from command-line intact
                    if (!platform.empty()) {
                        if (platform == "Unspecified") {
                            printMessage("'Unspecified' is a deprecated platform type and will be removed in Cppcheck 2.14. Please use 'unspecified' instead.");
                            platform = "unspecified";
                        }

                        std::string errstr;
                        const std::vector<std::string> paths = {projectFile, argv[0]};
                        if (!mSettings.platform.set(platform, errstr, paths)) {
                            printError(errstr);
                            return false;
                        }
                    }

                    if (!mSettings.project.guiProject.projectFile.empty()) {
                        projectFile = mSettings.project.guiProject.projectFile;
                        projType = mSettings.project.import(mSettings.project.guiProject.projectFile, &mSettings);
                    }
                }
                if (projType == ImportProject::Type::VS_SLN || projType == ImportProject::Type::VS_VCXPROJ) {
                    if (mSettings.project.guiProject.analyzeAllVsConfigs == "false")
                        mSettings.project.selectOneVsConfig(mSettings.platform.type);
                    if (!CppCheckExecutor::tryLoadLibrary(mSettings.library, argv[0], "windows.cfg")) {
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
                if (!mVSConfig.empty() && (mSettings.project.projectType == ImportProject::Type::VS_SLN || mSettings.project.projectType == ImportProject::Type::VS_VCXPROJ))
                    mSettings.project.ignoreOtherConfigs(mVSConfig);
            }

            // Only print something when there are errors
            else if (std::strcmp(argv[i], "-q") == 0 || std::strcmp(argv[i], "--quiet") == 0)
                mSettings.quiet = true;

            // Output relative paths
            else if (std::strcmp(argv[i], "-rp") == 0 || std::strcmp(argv[i], "--relative-paths") == 0)
                mSettings.relativePaths = true;
            else if (std::strncmp(argv[i], "-rp=", 4) == 0 || std::strncmp(argv[i], "--relative-paths=", 17) == 0) {
                mSettings.relativePaths = true;
                if (argv[i][argv[i][3]=='='?4:17] != 0) {
                    std::string paths = argv[i]+(argv[i][3]=='='?4:17);
                    for (;;) {
                        const std::string::size_type pos = paths.find(';');
                        if (pos == std::string::npos) {
                            mSettings.basePaths.emplace_back(Path::fromNativeSeparators(paths));
                            break;
                        }
                        mSettings.basePaths.emplace_back(Path::fromNativeSeparators(paths.substr(0, pos)));
                        paths.erase(0, pos + 1);
                    }
                } else {
                    printError("no paths specified for the '" + std::string(argv[i]) + "' option.");
                    return false;
                }
            }

            // Report progress
            else if (std::strcmp(argv[i], "--report-progress") == 0) {
                mSettings.reportProgress = 10;
            }

            else if (std::strncmp(argv[i], "--report-progress=", 18) == 0) {
                int tmp;
                if (!parseNumberArg(argv[i], 18, tmp, true))
                    return false;
                mSettings.reportProgress = tmp;
            }

#ifdef HAVE_RULES
            // Rule given at command line
            else if (std::strncmp(argv[i], "--rule=", 7) == 0) {
                Settings::Rule rule;
                rule.pattern = 7 + argv[i];
                mSettings.rules.emplace_back(std::move(rule));
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

                        const tinyxml2::XMLElement *tokenlist = node->FirstChildElement("tokenlist");
                        if (tokenlist)
                            rule.tokenlist = tokenlist->GetText();

                        const tinyxml2::XMLElement *pattern = node->FirstChildElement("pattern");
                        if (pattern) {
                            rule.pattern = pattern->GetText();
                        }

                        tinyxml2::XMLElement *message = node->FirstChildElement("message");
                        if (message) {
                            const tinyxml2::XMLElement *severity = message->FirstChildElement("severity");
                            if (severity)
                                rule.severity = Severity::fromString(severity->GetText());

                            const tinyxml2::XMLElement *id = message->FirstChildElement("id");
                            if (id)
                                rule.id = id->GetText();

                            const tinyxml2::XMLElement *summary = message->FirstChildElement("summary");
                            if (summary)
                                rule.summary = summary->GetText() ? summary->GetText() : "";
                        }

                        if (!rule.pattern.empty())
                            mSettings.rules.emplace_back(std::move(rule));
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
                    mSettings.showtime = SHOWTIME_MODES::SHOWTIME_FILE;
                else if (showtimeMode == "file-total")
                    mSettings.showtime = SHOWTIME_MODES::SHOWTIME_FILE_TOTAL;
                else if (showtimeMode == "summary")
                    mSettings.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY;
                else if (showtimeMode == "top5")
                    mSettings.showtime = SHOWTIME_MODES::SHOWTIME_TOP5;
                else if (showtimeMode.empty())
                    mSettings.showtime = SHOWTIME_MODES::SHOWTIME_NONE;
                else {
                    printError("unrecognized showtime mode: \"" + showtimeMode + "\". Supported modes: file, file-total, summary, top5.");
                    return false;
                }
            }

            // --std
            else if (std::strncmp(argv[i], "--std=", 6) == 0) {
                const std::string std = argv[i] + 6;
                // TODO: print error when standard is unknown
                if (std::strncmp(std.c_str(), "c++", 3) == 0) {
                    mSettings.standards.cpp = Standards::getCPP(std);
                }
                else if (std::strncmp(std.c_str(), "c", 1) == 0) {
                    mSettings.standards.c = Standards::getC(std);
                }
                else {
                    printError("unknown --std value '" + std + "'");
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--suppress=", 11) == 0) {
                const std::string suppression = argv[i]+11;
                const std::string errmsg(mSuppressions.addSuppressionLine(suppression));
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
                    if (std::count(filename.cbegin(), filename.cend(), ',') > 0 ||
                        std::count(filename.cbegin(), filename.cend(), '.') > 1) {
                        // If user tried to pass multiple files (we can only guess that)
                        // e.g. like this: --suppressions-list=a.txt,b.txt
                        // print more detailed error message to tell user how he can solve the problem
                        message += "\nIf you want to pass two files, you can do it e.g. like this:";
                        message += "\n    cppcheck --suppressions-list=a.txt --suppressions-list=b.txt file.cpp";
                    }

                    printError(message);
                    return false;
                }
                const std::string errmsg(mSuppressions.parseFile(f));
                if (!errmsg.empty()) {
                    printError(errmsg);
                    return false;
                }
            }

            else if (std::strncmp(argv[i], "--suppress-xml=", 15) == 0) {
                const char * filename = argv[i] + 15;
                const std::string errmsg(mSuppressions.parseXmlFile(filename));
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
                    mSettings.templateFormat = argv[i] + 11;
                else if ((i+1) < argc && argv[i+1][0] != '-') {
                    printMessage("'--template <template>' is deprecated and will be removed in 2.13 - please use '--template=<template>' instead");
                    ++i;
                    mSettings.templateFormat = argv[i];
                } else {
                    printError("argument to '--template' is missing.");
                    return false;
                }
                // TODO: bail out when no placeholders are found?

                if (mSettings.templateFormat == "gcc") {
                    mSettings.templateFormat = "{bold}{file}:{line}:{column}: {magenta}warning:{default} {message} [{id}]{reset}\\n{code}";
                    mSettings.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
                } else if (mSettings.templateFormat == "daca2") {
                    mSettings.daca = true;
                    mSettings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
                    mSettings.templateLocation = "{file}:{line}:{column}: note: {info}";
                } else if (mSettings.templateFormat == "vs")
                    mSettings.templateFormat = "{file}({line}): {severity}: {message}";
                else if (mSettings.templateFormat == "edit")
                    mSettings.templateFormat = "{file} +{line}: {severity}: {message}";
                else if (mSettings.templateFormat == "cppcheck1")
                    mSettings.templateFormat = "{callstack}: ({severity}{inconclusive:, inconclusive}) {message}";
                else if (mSettings.templateFormat == "selfcheck") {
                    mSettings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\\n{code}";
                    mSettings.templateLocation = "{file}:{line}:{column}: note: {info}\\n{code}";
                    mSettings.daca = true;
                }
            }

            else if (std::strcmp(argv[i], "--template-location") == 0 ||
                     std::strncmp(argv[i], "--template-location=", 20) == 0) {
                // "--template-location format"
                if (argv[i][19] == '=')
                    mSettings.templateLocation = argv[i] + 20;
                else if ((i+1) < argc && argv[i+1][0] != '-') {
                    printMessage("'--template-location <template>' is deprecated and will be removed in 2.13 - please use '--template-location=<template>' instead");
                    ++i;
                    mSettings.templateLocation = argv[i];
                } else {
                    printError("argument to '--template-location' is missing.");
                    return false;
                }
                // TODO: bail out when no placeholders are found?
            }

            else if (std::strncmp(argv[i], "--template-max-time=", 20) == 0) {
                if (!parseNumberArg(argv[i], 20, mSettings.templateMaxTime))
                    return false;
            }

            else if (std::strncmp(argv[i], "--typedef-max-time=", 19) == 0) {
                if (!parseNumberArg(argv[i], 19, mSettings.typedefMaxTime))
                    return false;
            }

            else if (std::strncmp(argv[i], "--valueflow-max-iterations=", 27) == 0) {
                if (!parseNumberArg(argv[i], 27, mSettings.valueFlowMaxIterations))
                    return false;
            }

            else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--verbose") == 0)
                mSettings.verbose = true;

            else if (std::strcmp(argv[i], "--version") == 0) {
                mShowVersion = true;
                mExitAfterPrint = true;
                mSettings.loadCppcheckCfg();
                return true;
            }

            // Write results in results.xml
            else if (std::strcmp(argv[i], "--xml") == 0)
                mSettings.xml = true;

            // Define the XML file version (and enable XML output)
            else if (std::strncmp(argv[i], "--xml-version=", 14) == 0) {
                int tmp;
                if (!parseNumberArg(argv[i], 14, tmp))
                    return false;
                if (tmp != 2) {
                    // We only have xml version 2
                    printError("'--xml-version' can only be 2.");
                    return false;
                }

                mSettings.xml_version = tmp;
                // Enable also XML if version is set
                mSettings.xml = true;
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

    mSettings.loadCppcheckCfg();

    // Default template format..
    if (mSettings.templateFormat.empty()) {
        mSettings.templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
        if (mSettings.templateLocation.empty())
            mSettings.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
    }
    // replace static parts of the templates
    substituteTemplateFormatStatic(mSettings.templateFormat);
    substituteTemplateLocationStatic(mSettings.templateLocation);

    mSettings.project.ignorePaths(mIgnoredPaths);

    if (mSettings.force || maxconfigs)
        mSettings.checkAllConfigurations = true;

    if (mSettings.force)
        mSettings.maxConfigs = INT_MAX;

    else if ((def || mSettings.preprocessOnly) && !maxconfigs)
        mSettings.maxConfigs = 1U;

    if (mSettings.checks.isEnabled(Checks::unusedFunction) && mSettings.jobs > 1 && mSettings.buildDir.empty()) {
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

#if defined(_WIN64)
    if (SHOW_DEF_PLATFORM_MSG && default_platform && !mSettings.quiet)
        printMessage("Windows 64-bit binaries currently default to the 'win64' platform. Starting with Cppcheck 2.13 they will default to 'native' instead. Please specify '--platform=win64' explicitly if you rely on this.");
#elif defined(_WIN32)
    if (SHOW_DEF_PLATFORM_MSG && default_platform && !mSettings.quiet)
        printMessage("Windows 32-bit binaries currently default to the 'win32A' platform. Starting with Cppcheck 2.13 they will default to 'native' instead. Please specify '--platform=win32A' explicitly if you rely on this.");
#endif

    // Print error only if we have "real" command and expect files
    if (!mExitAfterPrint && mPathNames.empty() && mSettings.project.fileSettings.empty()) {
        printError("no C or C++ source files found.");
        return false;
    }

    // Use paths _pathnames if no base paths for relative path output are given
    if (mSettings.basePaths.empty() && mSettings.relativePaths)
        mSettings.basePaths = mPathNames;

    return true;
}

void CmdLineParser::printHelp()
{
    const std::string manualUrl(isCppcheckPremium() ?
                                "https://cppcheck.sourceforge.io/manual.pdf" :
                                "https://files.cppchecksolutions.com/manual.pdf");

    std::cout << "Cppcheck - A tool for static C/C++ code analysis\n"
        "\n"
        "Syntax:\n"
        "    cppcheck [OPTIONS] [files or paths]\n"
        "\n"
        "If a directory is given instead of a filename, *.cpp, *.cxx, *.cc, *.c++, *.c, *.ipp,\n"
        "*.ixx, *.tpp, and *.txx files are checked recursively from the given directory.\n\n"
        "Options:\n"
        "    --addon=<addon>\n"
        "                         Execute addon. i.e. --addon=misra. If options must be\n"
        "                         provided a json configuration is needed.\n"
        "    --addon-python=<python interpreter>\n"
        "                         You can specify the python interpreter either in the\n"
        "                         addon json files or through this command line option.\n"
        "                         If not present, Cppcheck will try \"python3\" first and\n"
        "                         then \"python\".\n"
        "    --cppcheck-build-dir=<dir>\n"
        "                         Cppcheck work folder. Advantages:\n"
        "                          * whole program analysis\n"
        "                          * faster analysis; Cppcheck will reuse the results if\n"
        "                            the hash for a file is unchanged.\n"
        "                          * some useful debug information, i.e. commands used to\n"
        "                            execute clang/clang-tidy/addons.\n"
        "    --check-config       Check cppcheck configuration. The normal code\n"
        "                         analysis is disabled by this flag.\n"
        "    --check-level=<level>\n"
        "                         Configure how much checking you want:\n"
        "                          * normal: Cppcheck uses some compromises in the checking so\n"
        "                            the checking will finish in reasonable time.\n"
        "                          * exhaustive: deeper analysis that you choose when you can\n"
        "                            wait.\n"
        "                         The default choice is 'normal'.\n"
        "    --check-library      Show information messages when library files have\n"
        "                         incomplete info.\n"
        "    --checkers-report=<file>\n"
        "                         Write a report of all the active checkers to the given file.\n"
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
        "    --disable=<id>       Disable individual checks.\n"
        "                         Please refer to the documentation of --enable=<id>\n"
        "                         for further details.\n"
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
        "                                  Warn if there are missing includes.\n"
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
    "    --max-configs=<limit>\n"
    "                         Maximum number of configurations to check in a file\n"
    "                         before skipping it. Default is '12'. If used together\n"
    "                         with '--force', the last option is the one that is\n"
    "                         effective.\n"
    "    --max-ctu-depth=N    Max depth in whole program analysis. The default value\n"
    "                         is 2. A larger value will mean more errors can be found\n"
    "                         but also means the analysis will be slower.\n"
    "    --output-file=<file> Write results to file, rather than standard error.\n"
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
    "                         Generate Clang-plist output files in folder.\n";

    if (isCppcheckPremium()) {
        std::cout << "    --premium=<option>\n"
                  << "                         Coding standards:\n"
                  << "                          * autosar           Autosar (partial)\n"
                  << "                          * cert-c-2016       Cert C 2016 checking\n"
                  << "                          * cert-c++-2016     Cert C++ 2016 checking (partial)\n"
                  << "                          * misra-c-2012      Misra C 2012\n"
                  << "                          * misra-c-2023      Misra C 2023\n"
                  << "                          * misra-c++-2008    Misra C++ 2008 (partial)\n"
                  << "                         Other:\n"
                  << "                          * bughunting        Soundy analysis\n"
                  << "                          * cert-c-int-precision=BITS  Integer precision to use in Cert C analysis.\n";
    }

    std::cout <<
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
        "    -q, --quiet          Do not show progress reports.\n"
        "                         Note that this option is not mutually exclusive with --verbose.\n"
        "    -rp=<paths>, --relative-paths=<paths>\n"
        "                         Use relative paths in output. When given, <paths> are\n"
        "                         used as base. You can separate multiple paths by ';'.\n"
        "                         Otherwise path where source files are searched is used.\n"
        "                         We use string comparison to create relative paths, so\n"
        "                         using e.g. ~ for home folder does not work. It is\n"
        "                         currently only possible to apply the base paths to\n"
        "                         files that are on a lower level in the directory tree.\n"
        "    --report-progress    Report progress messages while checking a file (single job only).\n"
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
    "                         Note that this option is not mutually exclusive with --quiet.\n"
    "    --version            Print out version number.\n"
    "    --xml                Write results in xml format to error stream (stderr).\n"
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
    "    " << manualUrl << "\n"
        "\n"
        "Many thanks to the 3rd party libraries we use:\n"
        " * tinyxml2 -- loading project/library/ctu files.\n"
        " * picojson -- loading compile database.\n"
        " * pcre -- rules.\n"
        " * qt -- used in GUI\n";
}

bool CmdLineParser::isCppcheckPremium() const {
    if (mSettings.cppcheckCfgProductName.empty())
        mSettings.loadCppcheckCfg();
    return mSettings.cppcheckCfgProductName.compare(0, 16, "Cppcheck Premium") == 0;
}
