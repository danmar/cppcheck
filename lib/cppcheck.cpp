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
#include "cppcheck.h"

#include "check.h"
#include "checkunusedfunctions.h"
#include "clangimport.h"
#include "color.h"
#include "ctu.h"
#include "errortypes.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h" // Preprocessor
#include "standards.h"
#include "suppressions.h"
#include "timer.h"
#include "token.h"
#include "tokenize.h" // Tokenizer
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "version.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream> // <- TEMPORARY
#include <new>
#include <set>
#include <sstream> // IWYU pragma: keep
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

#include "json.h"

#include <simplecpp.h>

#include <tinyxml2.h>

#ifdef HAVE_RULES
#ifdef _WIN32
#define PCRE_STATIC
#endif
#include <pcre.h>
#endif

class SymbolDatabase;

static const char Version[] = CPPCHECK_VERSION_STRING;
static const char ExtraVersion[] = "";

static const char FILELIST[] = "cppcheck-addon-ctu-file-list";

static TimerResults s_timerResults;

// CWE ids used
static const CWE CWE398(398U);  // Indicator of Poor Code Quality

// File deleter
namespace {
    class FilesDeleter {
    public:
        FilesDeleter() = default;
        ~FilesDeleter() {
            for (const std::string& fileName: mFilenames)
                std::remove(fileName.c_str());
        }
        void addFile(const std::string& fileName) {
            mFilenames.push_back(fileName);
        }
    private:
        std::vector<std::string> mFilenames;
    };
}

namespace {
    struct AddonInfo {
        std::string name;
        std::string scriptFile; // addon script
        std::string executable; // addon executable
        std::string args;       // special extra arguments
        std::string python;     // script interpreter
        bool ctu = false;
        std::string runScript;

        static std::string getFullPath(const std::string &fileName, const std::string &exename) {
            if (Path::isFile(fileName))
                return fileName;

            const std::string exepath = Path::getPathFromFilename(exename);
            if (Path::isFile(exepath + fileName))
                return exepath + fileName;
            if (Path::isFile(exepath + "addons/" + fileName))
                return exepath + "addons/" + fileName;

#ifdef FILESDIR
            if (Path::isFile(FILESDIR + ("/" + fileName)))
                return FILESDIR + ("/" + fileName);
            if (Path::isFile(FILESDIR + ("/addons/" + fileName)))
                return FILESDIR + ("/addons/" + fileName);
#endif
            return "";
        }

        std::string parseAddonInfo(const picojson::value &json, const std::string &fileName, const std::string &exename) {
            const std::string& json_error = picojson::get_last_error();
            if (!json_error.empty()) {
                return "Loading " + fileName + " failed. " + json_error;
            }
            if (!json.is<picojson::object>())
                return "Loading " + fileName + " failed. Bad json.";
            picojson::object obj = json.get<picojson::object>();
            if (obj.count("args")) {
                if (!obj["args"].is<picojson::array>())
                    return "Loading " + fileName + " failed. args must be array.";
                for (const picojson::value &v : obj["args"].get<picojson::array>())
                    args += " " + v.get<std::string>();
            }

            if (obj.count("ctu")) {
                // ctu is specified in the config file
                if (!obj["ctu"].is<bool>())
                    return "Loading " + fileName + " failed. ctu must be boolean.";
                ctu = obj["ctu"].get<bool>();
            } else {
                ctu = false;
            }

            if (obj.count("python")) {
                // Python was defined in the config file
                if (obj["python"].is<picojson::array>()) {
                    return "Loading " + fileName +" failed. python must not be an array.";
                }
                python = obj["python"].get<std::string>();
            } else {
                python = "";
            }

            if (obj.count("executable")) {
                if (!obj["executable"].is<std::string>())
                    return "Loading " + fileName + " failed. executable must be a string.";
                executable = getFullPath(obj["executable"].get<std::string>(), fileName);
                return "";
            }

            return getAddonInfo(obj["script"].get<std::string>(), exename);
        }

        std::string getAddonInfo(const std::string &fileName, const std::string &exename) {
            if (fileName[0] == '{') {
                std::istringstream in(fileName);
                picojson::value json;
                in >> json;
                return parseAddonInfo(json, fileName, exename);
            }
            if (fileName.find('.') == std::string::npos)
                return getAddonInfo(fileName + ".py", exename);

            if (endsWith(fileName, ".py")) {
                scriptFile = getFullPath(fileName, exename);
                if (scriptFile.empty())
                    return "Did not find addon " + fileName;

                std::string::size_type pos1 = scriptFile.rfind('/');
                if (pos1 == std::string::npos)
                    pos1 = 0;
                else
                    pos1++;
                std::string::size_type pos2 = scriptFile.rfind('.');
                if (pos2 < pos1)
                    pos2 = std::string::npos;
                name = scriptFile.substr(pos1, pos2 - pos1);

                runScript = getFullPath("runaddon.py", exename);

                return "";
            }

            if (!endsWith(fileName, ".json"))
                return "Failed to open addon " + fileName;

            std::ifstream fin(fileName);
            if (!fin.is_open())
                return "Failed to open " + fileName;
            picojson::value json;
            fin >> json;
            return parseAddonInfo(json, fileName, exename);
        }
    };
}

static std::string cmdFileName(std::string f)
{
    f = Path::toNativeSeparators(f);
    if (f.find(' ') != std::string::npos)
        return "\"" + f + "\"";
    return f;
}

static std::vector<std::string> split(const std::string &str, const std::string &sep=" ")
{
    std::vector<std::string> ret;
    for (std::string::size_type startPos = 0U; startPos < str.size();) {
        startPos = str.find_first_not_of(sep, startPos);
        if (startPos == std::string::npos)
            break;

        if (str[startPos] == '\"') {
            const std::string::size_type endPos = str.find('\"', startPos + 1);
            ret.push_back(str.substr(startPos + 1, endPos - startPos - 1));
            startPos = (endPos < str.size()) ? (endPos + 1) : endPos;
            continue;
        }

        const std::string::size_type endPos = str.find(sep, startPos + 1);
        ret.push_back(str.substr(startPos, endPos - startPos));
        startPos = endPos;
    }

    return ret;
}

static int getPid()
{
#ifndef _WIN32
    return getpid();
#else
    return _getpid();
#endif
}

static std::string getDumpFileName(const Settings& settings, const std::string& filename)
{
    if (!settings.dumpFile.empty())
        return settings.dumpFile;

    std::string extension;
    if (settings.dump)
        extension = ".dump";
    else
        extension = "." + std::to_string(getPid()) + ".dump";

    if (!settings.dump && !settings.buildDir.empty())
        return AnalyzerInformation::getAnalyzerInfoFile(settings.buildDir, filename, emptyString) + extension;
    return filename + extension;
}

static std::string getCtuInfoFileName(const std::string &dumpFile)
{
    return dumpFile.substr(0, dumpFile.size()-4) + "ctu-info";
}

static void createDumpFile(const Settings& settings,
                           const std::string& filename,
                           std::ofstream& fdump,
                           std::string& dumpFile)
{
    if (!settings.dump && settings.addons.empty())
        return;
    dumpFile = getDumpFileName(settings, filename);

    fdump.open(dumpFile);
    if (!fdump.is_open())
        return;

    {
        std::ofstream fout(getCtuInfoFileName(dumpFile));
    }

    std::string language;
    switch (settings.enforcedLang) {
    case Settings::Language::C:
        language = " language=\"c\"";
        break;
    case Settings::Language::CPP:
        language = " language=\"cpp\"";
        break;
    case Settings::Language::None:
        if (Path::isCPP(filename))
            language = " language=\"cpp\"";
        else if (Path::isC(filename))
            language = " language=\"c\"";
        break;
    }

    fdump << "<?xml version=\"1.0\"?>\n";
    fdump << "<dumps" << language << ">\n";
    fdump << "  <platform"
          << " name=\"" << settings.platform.toString() << '\"'
          << " char_bit=\"" << settings.platform.char_bit << '\"'
          << " short_bit=\"" << settings.platform.short_bit << '\"'
          << " int_bit=\"" << settings.platform.int_bit << '\"'
          << " long_bit=\"" << settings.platform.long_bit << '\"'
          << " long_long_bit=\"" << settings.platform.long_long_bit << '\"'
          << " pointer_bit=\"" << (settings.platform.sizeof_pointer * settings.platform.char_bit) << '\"'
          << "/>" << '\n';
}

static std::string executeAddon(const AddonInfo &addonInfo,
                                const std::string &defaultPythonExe,
                                const std::string &file,
                                const std::string &premiumArgs,
                                const std::function<bool(std::string,std::vector<std::string>,std::string,std::string&)> &executeCommand)
{
    const std::string redirect = "2>&1";

    std::string pythonExe;

    if (!addonInfo.executable.empty())
        pythonExe = addonInfo.executable;
    else if (!addonInfo.python.empty())
        pythonExe = cmdFileName(addonInfo.python);
    else if (!defaultPythonExe.empty())
        pythonExe = cmdFileName(defaultPythonExe);
    else {
#ifdef _WIN32
        const char *py_exes[] = { "python3.exe", "python.exe" };
#else
        const char *py_exes[] = { "python3", "python" };
#endif
        for (const char* py_exe : py_exes) {
            std::string out;
#ifdef _MSC_VER
            // FIXME: hack to avoid debug assertion with _popen() in executeCommand() for non-existing commands
            const std::string cmd = std::string(py_exe) + " --version >NUL";
            if (system(cmd.c_str()) != 0) {
                // TODO: get more detailed error?
                break;
            }
#endif
            if (executeCommand(py_exe, split("--version"), redirect, out) && out.compare(0, 7, "Python ") == 0 && std::isdigit(out[7])) {
                pythonExe = py_exe;
                break;
            }
        }
        if (pythonExe.empty())
            throw InternalError(nullptr, "Failed to auto detect python");
    }

    std::string args;
    if (addonInfo.executable.empty())
        args = cmdFileName(addonInfo.runScript) + " " + cmdFileName(addonInfo.scriptFile);
    args += std::string(args.empty() ? "" : " ") + "--cli" + addonInfo.args;
    if (!premiumArgs.empty() && !addonInfo.executable.empty())
        args += " " + premiumArgs;

    const std::string fileArg = (endsWith(file, FILELIST, sizeof(FILELIST)-1) ? " --file-list " : " ") + cmdFileName(file);
    args += fileArg;

    std::string result;
    if (!executeCommand(pythonExe, split(args), redirect, result)) {
        std::string message("Failed to execute addon '" + addonInfo.name + "' (command: '" + pythonExe + " " + args + "'). Exitcode is nonzero.");
        if (result.size() > 2) {
            message = message + "\n" + message + "\nOutput:\n" + result;
            message.resize(message.find_last_not_of("\n\r"));
        }
        throw InternalError(nullptr, message);
    }

    // Validate output..
    std::istringstream istr(result);
    std::string line;
    while (std::getline(istr, line)) {
        if (line.compare(0,9,"Checking ", 0, 9) != 0 && !line.empty() && line[0] != '{') {
            result.erase(result.find_last_not_of('\n') + 1, std::string::npos); // Remove trailing newlines
            throw InternalError(nullptr, "Failed to execute '" + pythonExe + " " + args + "'. " + result);
        }
    }

    // Valid results
    return result;
}

static std::string getDefinesFlags(const std::string &semicolonSeparatedString)
{
    std::string flags;
    for (const std::string &d: split(semicolonSeparatedString, ";"))
        flags += "-D" + d + " ";
    return flags;
}

CppCheck::CppCheck(ErrorLogger &errorLogger,
                   bool useGlobalSuppressions,
                   std::function<bool(std::string,std::vector<std::string>,std::string,std::string&)> executeCommand)
    : mErrorLogger(errorLogger)
    , mUseGlobalSuppressions(useGlobalSuppressions)
    , mExecuteCommand(std::move(executeCommand))
{}

CppCheck::~CppCheck()
{
    while (!mFileInfo.empty()) {
        delete mFileInfo.back();
        mFileInfo.pop_back();
    }
    s_timerResults.showResults(mSettings.showtime);

    if (mPlistFile.is_open()) {
        mPlistFile << ErrorLogger::plistFooter();
        mPlistFile.close();
    }
}

const char * CppCheck::version()
{
    return Version;
}

const char * CppCheck::extraVersion()
{
    return ExtraVersion;
}

static bool reportClangErrors(std::istream &is, const std::function<void(const ErrorMessage&)>& reportErr, std::vector<ErrorMessage> &warnings)
{
    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == ' ' || line[0] == '`' || line[0] == '-')
            continue;

        std::string::size_type pos3 = line.find(": error: ");
        if (pos3 == std::string::npos)
            pos3 = line.find(": fatal error:");
        if (pos3 == std::string::npos)
            pos3 = line.find(": warning:");
        if (pos3 == std::string::npos)
            continue;

        // file:line:column: error: ....
        const std::string::size_type pos2 = line.rfind(':', pos3 - 1);
        const std::string::size_type pos1 = line.rfind(':', pos2 - 1);

        if (pos1 >= pos2 || pos2 >= pos3)
            continue;

        const std::string filename = line.substr(0, pos1);
        const std::string linenr = line.substr(pos1+1, pos2-pos1-1);
        const std::string colnr = line.substr(pos2+1, pos3-pos2-1);
        const std::string msg = line.substr(line.find(':', pos3+1) + 2);

        const std::string locFile = Path::toNativeSeparators(filename);
        ErrorMessage::FileLocation loc;
        loc.setfile(locFile);
        loc.line = strToInt<int>(linenr);
        loc.column = strToInt<unsigned int>(colnr);
        ErrorMessage errmsg({std::move(loc)},
                            locFile,
                            Severity::error,
                            msg,
                            "syntaxError",
                            Certainty::normal);

        if (line.compare(pos3, 10, ": warning:") == 0) {
            warnings.push_back(std::move(errmsg));
            continue;
        }

        reportErr(errmsg);

        return true;
    }
    return false;
}

unsigned int CppCheck::check(const std::string &path)
{
    if (mSettings.clang) {
        if (!mSettings.quiet)
            mErrorLogger.reportOut(std::string("Checking ") + path + "...", Color::FgGreen);

        const std::string lang = Path::isCPP(path) ? "-x c++" : "-x c";
        const std::string analyzerInfo = mSettings.buildDir.empty() ? std::string() : AnalyzerInformation::getAnalyzerInfoFile(mSettings.buildDir, path, emptyString);
        const std::string clangcmd = analyzerInfo + ".clang-cmd";
        const std::string clangStderr = analyzerInfo + ".clang-stderr";
        const std::string clangAst = analyzerInfo + ".clang-ast";
        std::string exe = mSettings.clangExecutable;
#ifdef _WIN32
        // append .exe if it is not a path
        if (Path::fromNativeSeparators(mSettings.clangExecutable).find('/') == std::string::npos) {
            exe += ".exe";
        }
#endif

        std::string flags(lang + " ");
        if (Path::isCPP(path) && !mSettings.standards.stdValue.empty())
            flags += "-std=" + mSettings.standards.stdValue + " ";

        for (const std::string &i: mSettings.includePaths)
            flags += "-I" + i + " ";

        flags += getDefinesFlags(mSettings.userDefines);

        const std::string args2 = "-fsyntax-only -Xclang -ast-dump -fno-color-diagnostics " + flags + path;
        const std::string redirect2 = analyzerInfo.empty() ? std::string("2>&1") : ("2> " + clangStderr);
        if (!mSettings.buildDir.empty()) {
            std::ofstream fout(clangcmd);
            fout << exe << " " << args2 << " " << redirect2 << std::endl;
        } else if (mSettings.verbose && !mSettings.quiet) {
            mErrorLogger.reportOut(exe + " " + args2);
        }

        std::string output2;
        if (!mExecuteCommand(exe,split(args2),redirect2,output2) || output2.find("TranslationUnitDecl") == std::string::npos) {
            std::cerr << "Failed to execute '" << exe << " " << args2 << " " << redirect2 << "'" << std::endl;
            return 0;
        }

        // Ensure there are not syntax errors...
        std::vector<ErrorMessage> compilerWarnings;
        if (!mSettings.buildDir.empty()) {
            std::ifstream fin(clangStderr);
            auto reportError = [this](const ErrorMessage& errorMessage) {
                reportErr(errorMessage);
            };
            if (reportClangErrors(fin, reportError, compilerWarnings))
                return 0;
        } else {
            std::istringstream istr(output2);
            auto reportError = [this](const ErrorMessage& errorMessage) {
                reportErr(errorMessage);
            };
            if (reportClangErrors(istr, reportError, compilerWarnings))
                return 0;
        }

        if (!mSettings.buildDir.empty()) {
            std::ofstream fout(clangAst);
            fout << output2 << std::endl;
        }

        try {
            std::istringstream ast(output2);
            Tokenizer tokenizer(&mSettings, this);
            tokenizer.list.appendFileIfNew(path);
            clangimport::parseClangAstDump(&tokenizer, ast);
            ValueFlow::setValues(tokenizer.list,
                                 const_cast<SymbolDatabase&>(*tokenizer.getSymbolDatabase()),
                                 this,
                                 &mSettings,
                                 &s_timerResults);
            if (mSettings.debugnormal)
                tokenizer.printDebugOutput(1);
            checkNormalTokens(tokenizer);

            // create dumpfile
            std::ofstream fdump;
            std::string dumpFile;
            createDumpFile(mSettings, path, fdump, dumpFile);
            if (fdump.is_open()) {
                fdump << "<dump cfg=\"\">\n";
                for (const ErrorMessage& errmsg: compilerWarnings)
                    fdump << "  <clang-warning file=\"" << toxml(errmsg.callStack.front().getfile()) << "\" line=\"" << errmsg.callStack.front().line << "\" column=\"" << errmsg.callStack.front().column << "\" message=\"" << toxml(errmsg.shortMessage()) << "\"/>\n";
                fdump << "  <standards>\n";
                fdump << "    <c version=\"" << mSettings.standards.getC() << "\"/>\n";
                fdump << "    <cpp version=\"" << mSettings.standards.getCPP() << "\"/>\n";
                fdump << "  </standards>\n";
                tokenizer.dump(fdump);
                fdump << "</dump>\n";
                fdump << "</dumps>\n";
                fdump.close();
            }

            // run addons
            executeAddons(dumpFile);

        } catch (const InternalError &e) {
            internalError(path, "Processing Clang AST dump failed: " + e.errorMessage);
        } catch (const TerminateException &) {
            // Analysis is terminated
            return mExitCode;
        } catch (const std::exception &e) {
            internalError(path, std::string("Processing Clang AST dump failed: ") + e.what());
        }

        return mExitCode;
    }

    return checkFile(Path::simplifyPath(path), emptyString);
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    std::istringstream iss(content);
    return checkFile(Path::simplifyPath(path), emptyString, &iss);
}

unsigned int CppCheck::check(const ImportProject::FileSettings &fs)
{
    CppCheck temp(mErrorLogger, mUseGlobalSuppressions, mExecuteCommand);
    temp.mSettings = mSettings;
    if (!temp.mSettings.userDefines.empty())
        temp.mSettings.userDefines += ';';
    if (mSettings.clang)
        temp.mSettings.userDefines += fs.defines;
    else
        temp.mSettings.userDefines += fs.cppcheckDefines();
    temp.mSettings.includePaths = fs.includePaths;
    temp.mSettings.userUndefs.insert(fs.undefs.cbegin(), fs.undefs.cend());
    if (fs.standard.find("++") != std::string::npos)
        temp.mSettings.standards.setCPP(fs.standard);
    else if (!fs.standard.empty())
        temp.mSettings.standards.setC(fs.standard);
    if (fs.platformType != cppcheck::Platform::Type::Unspecified)
        temp.mSettings.platform.set(fs.platformType);
    if (mSettings.clang) {
        temp.mSettings.includePaths.insert(temp.mSettings.includePaths.end(), fs.systemIncludePaths.cbegin(), fs.systemIncludePaths.cend());
        return temp.check(Path::simplifyPath(fs.filename));
    }
    const unsigned int returnValue = temp.checkFile(Path::simplifyPath(fs.filename), fs.cfg);
    mSettings.nomsg.addSuppressions(temp.mSettings.nomsg.getSuppressions());
    return returnValue;
}

static simplecpp::TokenList createTokenList(const std::string& filename, std::vector<std::string>& files, simplecpp::OutputList* outputList, std::istream* fileStream)
{
    if (fileStream)
        return {*fileStream, files, filename, outputList};

    return {filename, files, outputList};
}

unsigned int CppCheck::checkFile(const std::string& filename, const std::string &cfgname, std::istream* fileStream)
{
    mExitCode = 0;

    FilesDeleter filesDeleter;

    if (Settings::terminated())
        return mExitCode;

    const Timer fileTotalTimer(mSettings.showtime == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL, filename);

    if (!mSettings.quiet) {
        std::string fixedpath = Path::simplifyPath(filename);
        fixedpath = Path::toNativeSeparators(fixedpath);
        mErrorLogger.reportOut(std::string("Checking ") + fixedpath + ' ' + cfgname + std::string("..."), Color::FgGreen);

        if (mSettings.verbose) {
            mErrorLogger.reportOut("Defines:" + mSettings.userDefines);
            std::string undefs;
            for (const std::string& U : mSettings.userUndefs) {
                if (!undefs.empty())
                    undefs += ';';
                undefs += ' ' + U;
            }
            mErrorLogger.reportOut("Undefines:" + undefs);
            std::string includePaths;
            for (const std::string &I : mSettings.includePaths)
                includePaths += " -I" + I;
            mErrorLogger.reportOut("Includes:" + includePaths);
            mErrorLogger.reportOut(std::string("Platform:") + mSettings.platform.toString());
        }
    }

    if (mPlistFile.is_open()) {
        mPlistFile << ErrorLogger::plistFooter();
        mPlistFile.close();
    }

    CheckUnusedFunctions checkUnusedFunctions(nullptr, nullptr, nullptr);

    try {
        Preprocessor preprocessor(mSettings, this);
        std::set<std::string> configurations;

        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1 = createTokenList(filename, files, &outputList, fileStream);

        // If there is a syntax error, report it and stop
        const auto output_it = std::find_if(outputList.cbegin(), outputList.cend(), [](const simplecpp::Output &output){
            return Preprocessor::hasErrors(output);
        });
        if (output_it != outputList.cend()) {
            const simplecpp::Output &output = *output_it;
            std::string file = Path::fromNativeSeparators(output.location.file());
            if (mSettings.relativePaths)
                file = Path::getRelativePath(file, mSettings.basePaths);

            const ErrorMessage::FileLocation loc1(file, output.location.line, output.location.col);
            std::list<ErrorMessage::FileLocation> callstack(1, loc1);

            ErrorMessage errmsg(callstack,
                                "",
                                Severity::error,
                                output.msg,
                                "syntaxError",
                                Certainty::normal);
            reportErr(errmsg);
            return mExitCode;
        }

        if (mSettings.library.markupFile(filename)) {
            Tokenizer tokenizer(&mSettings, this, &preprocessor);
            tokenizer.createTokens(std::move(tokens1));
            checkUnusedFunctions.getFileInfo(&tokenizer, &mSettings);
            return EXIT_SUCCESS;
        }

        if (!preprocessor.loadFiles(tokens1, files))
            return mExitCode;

        if (!mSettings.plistOutput.empty()) {
            std::string filename2;
            if (filename.find('/') != std::string::npos)
                filename2 = filename.substr(filename.rfind('/') + 1);
            else
                filename2 = filename;
            const std::size_t fileNameHash = std::hash<std::string> {}(filename);
            filename2 = mSettings.plistOutput + filename2.substr(0, filename2.find('.')) + "_" + std::to_string(fileNameHash) + ".plist";
            mPlistFile.open(filename2);
            mPlistFile << ErrorLogger::plistHeader(version(), files);
        }

        std::ostringstream dumpProlog;
        if (mSettings.dump || !mSettings.addons.empty()) {
            dumpProlog << "  <rawtokens>" << std::endl;
            for (unsigned int i = 0; i < files.size(); ++i)
                dumpProlog << "    <file index=\"" << i << "\" name=\"" << ErrorLogger::toxml(files[i]) << "\"/>" << std::endl;
            for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
                dumpProlog
                    << "    <tok "
                    << "fileIndex=\"" << tok->location.fileIndex << "\" "
                    << "linenr=\"" << tok->location.line << "\" "
                    << "column=\"" << tok->location.col << "\" "
                    << "str=\"" << ErrorLogger::toxml(tok->str()) << "\""
                    << "/>" << std::endl;
            }
            dumpProlog << "  </rawtokens>" << std::endl;
        }

        // Parse comments and then remove them
        preprocessor.inlineSuppressions(tokens1, mSettings.nomsg);
        if (mSettings.dump || !mSettings.addons.empty()) {
            mSettings.nomsg.dump(dumpProlog);
        }
        tokens1.removeComments();
        preprocessor.removeComments();

        if (!mSettings.buildDir.empty()) {
            // Get toolinfo
            std::ostringstream toolinfo;
            toolinfo << CPPCHECK_VERSION_STRING;
            toolinfo << (mSettings.severity.isEnabled(Severity::warning) ? 'w' : ' ');
            toolinfo << (mSettings.severity.isEnabled(Severity::style) ? 's' : ' ');
            toolinfo << (mSettings.severity.isEnabled(Severity::performance) ? 'p' : ' ');
            toolinfo << (mSettings.severity.isEnabled(Severity::portability) ? 'p' : ' ');
            toolinfo << (mSettings.severity.isEnabled(Severity::information) ? 'i' : ' ');
            toolinfo << mSettings.userDefines;
            mSettings.nomsg.dump(toolinfo);

            // Calculate hash so it can be compared with old hash / future hashes
            const std::size_t hash = preprocessor.calculateHash(tokens1, toolinfo.str());
            std::list<ErrorMessage> errors;
            if (!mAnalyzerInformation.analyzeFile(mSettings.buildDir, filename, cfgname, hash, errors)) {
                while (!errors.empty()) {
                    reportErr(errors.front());
                    errors.pop_front();
                }
                return mExitCode;  // known results => no need to reanalyze file
            }
        }

        // write dump file xml prolog
        std::ofstream fdump;
        std::string dumpFile;
        createDumpFile(mSettings, filename, fdump, dumpFile);
        if (fdump.is_open()) {
            fdump << dumpProlog.str();
            dumpProlog.str("");
            if (!mSettings.dump)
                filesDeleter.addFile(dumpFile);
        }

        // Get directives
        preprocessor.setDirectives(tokens1);
        preprocessor.simplifyPragmaAsm(&tokens1);

        preprocessor.setPlatformInfo(&tokens1);

        // Get configurations..
        if ((mSettings.checkAllConfigurations && mSettings.userDefines.empty()) || mSettings.force) {
            Timer t("Preprocessor::getConfigs", mSettings.showtime, &s_timerResults);
            configurations = preprocessor.getConfigs(tokens1);
        } else {
            configurations.insert(mSettings.userDefines);
        }

        if (mSettings.checkConfiguration) {
            for (const std::string &config : configurations)
                (void)preprocessor.getcode(tokens1, config, files, true);

            return 0;
        }

#ifdef HAVE_RULES
        // Run define rules on raw code
        const auto rules_it = std::find_if(mSettings.rules.cbegin(), mSettings.rules.cend(), [](const Settings::Rule& rule) {
            return rule.tokenlist == "define";
        });
        if (rules_it != mSettings.rules.cend()) {
            std::string code;
            const std::list<Directive> &directives = preprocessor.getDirectives();
            for (const Directive &dir : directives) {
                if (dir.str.compare(0,8,"#define ") == 0 || dir.str.compare(0,9,"#include ") == 0)
                    code += "#line " + std::to_string(dir.linenr) + " \"" + dir.file + "\"\n" + dir.str + '\n';
            }
            Tokenizer tokenizer2(&mSettings, this);
            std::istringstream istr2(code);
            tokenizer2.list.createTokens(istr2);
            executeRules("define", tokenizer2);
        }
#endif

        if (!mSettings.force && configurations.size() > mSettings.maxConfigs) {
            if (mSettings.severity.isEnabled(Severity::information)) {
                tooManyConfigsError(Path::toNativeSeparators(filename),configurations.size());
            } else {
                mTooManyConfigs = true;
            }
        }

        std::set<unsigned long long> hashes;
        int checkCount = 0;
        bool hasValidConfig = false;
        std::list<std::string> configurationError;
        for (const std::string &currCfg : configurations) {
            // bail out if terminated
            if (Settings::terminated())
                break;

            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!mSettings.force && ++checkCount > mSettings.maxConfigs)
                break;

            if (!mSettings.userDefines.empty()) {
                mCurrentConfig = mSettings.userDefines;
                const std::vector<std::string> v1(split(mSettings.userDefines, ";"));
                for (const std::string &cfg: split(currCfg, ";")) {
                    if (std::find(v1.cbegin(), v1.cend(), cfg) == v1.cend()) {
                        mCurrentConfig += ";" + cfg;
                    }
                }
            } else {
                mCurrentConfig = currCfg;
            }

            if (mSettings.preprocessOnly) {
                Timer t("Preprocessor::getcode", mSettings.showtime, &s_timerResults);
                std::string codeWithoutCfg = preprocessor.getcode(tokens1, mCurrentConfig, files, true);
                t.stop();

                if (codeWithoutCfg.compare(0,5,"#file") == 0)
                    codeWithoutCfg.insert(0U, "//");
                std::string::size_type pos = 0;
                while ((pos = codeWithoutCfg.find("\n#file",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find("\n#endfile",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find(Preprocessor::macroChar,pos)) != std::string::npos)
                    codeWithoutCfg[pos] = ' ';
                reportOut(codeWithoutCfg);
                continue;
            }

            Tokenizer tokenizer(&mSettings, this, &preprocessor);
            if (mSettings.showtime != SHOWTIME_MODES::SHOWTIME_NONE)
                tokenizer.setTimerResults(&s_timerResults);

            try {
                // Create tokens, skip rest of iteration if failed
                {
                    Timer timer("Tokenizer::createTokens", mSettings.showtime, &s_timerResults);
                    simplecpp::TokenList tokensP = preprocessor.preprocess(tokens1, mCurrentConfig, files, true);
                    tokenizer.createTokens(std::move(tokensP));
                }
                hasValidConfig = true;

                // If only errors are printed, print filename after the check
                if (!mSettings.quiet && (!mCurrentConfig.empty() || checkCount > 1)) {
                    std::string fixedpath = Path::simplifyPath(filename);
                    fixedpath = Path::toNativeSeparators(fixedpath);
                    mErrorLogger.reportOut("Checking " + fixedpath + ": " + mCurrentConfig + "...", Color::FgGreen);
                }

                if (!tokenizer.tokens())
                    continue;

                // skip rest of iteration if just checking configuration
                if (mSettings.checkConfiguration)
                    continue;

                // Check raw tokens
                checkRawTokens(tokenizer);

                // Simplify tokens into normal form, skip rest of iteration if failed
                if (!tokenizer.simplifyTokens1(mCurrentConfig))
                    continue;

                // dump xml if --dump
                if ((mSettings.dump || !mSettings.addons.empty()) && fdump.is_open()) {
                    fdump << "<dump cfg=\"" << ErrorLogger::toxml(mCurrentConfig) << "\">" << std::endl;
                    fdump << "  <standards>" << std::endl;
                    fdump << "    <c version=\"" << mSettings.standards.getC() << "\"/>" << std::endl;
                    fdump << "    <cpp version=\"" << mSettings.standards.getCPP() << "\"/>" << std::endl;
                    fdump << "  </standards>" << std::endl;
                    preprocessor.dump(fdump);
                    tokenizer.dump(fdump);
                    fdump << "</dump>" << std::endl;
                }

                // Need to call this even if the hash will skip this configuration
                mSettings.nomsg.markUnmatchedInlineSuppressionsAsChecked(tokenizer);

                // Skip if we already met the same simplified token list
                if (mSettings.force || mSettings.maxConfigs > 1) {
                    const std::size_t hash = tokenizer.list.calculateHash();
                    if (hashes.find(hash) != hashes.end()) {
                        if (mSettings.debugwarnings)
                            purgedConfigurationMessage(filename, mCurrentConfig);
                        continue;
                    }
                    hashes.insert(hash);
                }

                // Check normal tokens
                checkNormalTokens(tokenizer);

                // Analyze info..
                if (!mSettings.buildDir.empty())
                    checkUnusedFunctions.parseTokens(tokenizer, filename.c_str(), &mSettings);

#ifdef HAVE_RULES
                // handling of "simple" rules has been removed.
                if (hasRule("simple"))
                    throw InternalError(nullptr, "Handling of \"simple\" rules has been removed in Cppcheck. Use --addon instead.");
#endif

            } catch (const simplecpp::Output &o) {
                // #error etc during preprocessing
                configurationError.push_back((mCurrentConfig.empty() ? "\'\'" : mCurrentConfig) + " : [" + o.location.file() + ':' + std::to_string(o.location.line) + "] " + o.msg);
                --checkCount; // don't count invalid configurations
                continue;

            } catch (const TerminateException &) {
                // Analysis is terminated
                return mExitCode;

            } catch (const InternalError &e) {
                ErrorMessage errmsg = ErrorMessage::fromInternalError(e, &tokenizer.list, filename);
                reportErr(errmsg);
            }
        }

        if (!hasValidConfig && configurations.size() > 1 && mSettings.severity.isEnabled(Severity::information)) {
            std::string msg;
            msg = "This file is not analyzed. Cppcheck failed to extract a valid configuration. Use -v for more details.";
            msg += "\nThis file is not analyzed. Cppcheck failed to extract a valid configuration. The tested configurations have these preprocessor errors:";
            for (const std::string &s : configurationError)
                msg += '\n' + s;

            const std::string locFile = Path::toNativeSeparators(filename);
            ErrorMessage::FileLocation loc;
            loc.setfile(locFile);
            ErrorMessage errmsg({std::move(loc)},
                                locFile,
                                Severity::information,
                                msg,
                                "noValidConfiguration",
                                Certainty::normal);
            reportErr(errmsg);
        }

        // dumped all configs, close root </dumps> element now
        if (fdump.is_open()) {
            fdump << "</dumps>" << std::endl;
            fdump.close();
        }

        executeAddons(dumpFile);

    } catch (const TerminateException &) {
        // Analysis is terminated
        return mExitCode;
    } catch (const std::runtime_error &e) {
        internalError(filename, std::string("Checking file failed: ") + e.what());
    } catch (const std::bad_alloc &) {
        internalError(filename, "Checking file failed: out of memory");
    } catch (const InternalError &e) {
        internalError(filename, "Checking file failed: " + e.errorMessage);
    }

    if (!mSettings.buildDir.empty()) {
        mAnalyzerInformation.setFileInfo("CheckUnusedFunctions", checkUnusedFunctions.analyzerInfo());
        mAnalyzerInformation.close();
    }

    // In jointSuppressionReport mode, unmatched suppressions are
    // collected after all files are processed
    if (!mSettings.useSingleJob() && (mSettings.severity.isEnabled(Severity::information) || mSettings.checkConfiguration)) {
        Suppressions::reportUnmatchedSuppressions(mSettings.nomsg.getUnmatchedLocalSuppressions(filename, isUnusedFunctionCheckEnabled()), *this);
    }

    mErrorList.clear();

    return mExitCode;
}

// TODO: replace with ErrorMessage::fromInternalError()
void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fullmsg("Bailing out from analysis: " + msg);

    const ErrorMessage::FileLocation loc1(filename, 0, 0);
    std::list<ErrorMessage::FileLocation> callstack(1, loc1);

    ErrorMessage errmsg(callstack,
                        emptyString,
                        Severity::error,
                        fullmsg,
                        "internalError",
                        Certainty::normal);

    mErrorLogger.reportErr(errmsg);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a raw token list
//---------------------------------------------------------------------------
void CppCheck::checkRawTokens(const Tokenizer &tokenizer)
{
#ifdef HAVE_RULES
    // Execute rules for "raw" code
    executeRules("raw", tokenizer);
#else
    (void)tokenizer;
#endif
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a normal token list
//---------------------------------------------------------------------------

void CppCheck::checkNormalTokens(const Tokenizer &tokenizer)
{
    // TODO: this should actually be the behavior if only "--enable=unusedFunction" is specified - see #10648
    const char* unusedFunctionOnly = std::getenv("UNUSEDFUNCTION_ONLY");
    const bool doUnusedFunctionOnly = unusedFunctionOnly && (std::strcmp(unusedFunctionOnly, "1") == 0);

    const std::time_t maxTime = mSettings.checksMaxTime > 0 ? std::time(nullptr) + mSettings.checksMaxTime : 0;

    // call all "runChecks" in all registered Check classes
    for (Check *check : Check::instances()) {
        if (Settings::terminated())
            return;

        if (maxTime > 0 && std::time(nullptr) > maxTime) {
            if (mSettings.debugwarnings) {
                ErrorMessage::FileLocation loc;
                loc.setfile(tokenizer.list.getFiles()[0]);
                ErrorMessage errmsg({std::move(loc)},
                                    emptyString,
                                    Severity::debug,
                                    "Checks maximum time exceeded",
                                    "checksMaxTime",
                                    Certainty::normal);
                reportErr(errmsg);
            }
            return;
        }

        if (doUnusedFunctionOnly && dynamic_cast<CheckUnusedFunctions*>(check) == nullptr)
            continue;

        Timer timerRunChecks(check->name() + "::runChecks", mSettings.showtime, &s_timerResults);
        check->runChecks(tokenizer, this);
    }

    if (mSettings.clang)
        // TODO: Use CTU for Clang analysis
        return;


    if (mSettings.useSingleJob() || !mSettings.buildDir.empty()) {
        // Analyse the tokens..

        CTU::FileInfo *fi1 = CTU::getFileInfo(&tokenizer);
        if (fi1) {
            if (mSettings.useSingleJob())
                mFileInfo.push_back(fi1);
            if (!mSettings.buildDir.empty())
                mAnalyzerInformation.setFileInfo("ctu", fi1->toString());
        }

        for (const Check *check : Check::instances()) {
            if (doUnusedFunctionOnly && dynamic_cast<const CheckUnusedFunctions*>(check) == nullptr)
                continue;

            Check::FileInfo *fi = check->getFileInfo(&tokenizer, &mSettings);
            if (fi != nullptr) {
                if (mSettings.useSingleJob())
                    mFileInfo.push_back(fi);
                if (!mSettings.buildDir.empty())
                    mAnalyzerInformation.setFileInfo(check->name(), fi->toString());
            }
        }
    }

#ifdef HAVE_RULES
    executeRules("normal", tokenizer);
#endif
}

//---------------------------------------------------------------------------

#ifdef HAVE_RULES
bool CppCheck::hasRule(const std::string &tokenlist) const
{
    return std::any_of(mSettings.rules.cbegin(), mSettings.rules.cend(), [&](const Settings::Rule& rule) {
        return rule.tokenlist == tokenlist;
    });
}

static const char * pcreErrorCodeToString(const int pcreExecRet)
{
    switch (pcreExecRet) {
    case PCRE_ERROR_NULL:
        return "Either code or subject was passed as NULL, or ovector was NULL "
               "and ovecsize was not zero (PCRE_ERROR_NULL)";
    case PCRE_ERROR_BADOPTION:
        return "An unrecognized bit was set in the options argument (PCRE_ERROR_BADOPTION)";
    case PCRE_ERROR_BADMAGIC:
        return "PCRE stores a 4-byte \"magic number\" at the start of the compiled code, "
               "to catch the case when it is passed a junk pointer and to detect when a "
               "pattern that was compiled in an environment of one endianness is run in "
               "an environment with the other endianness. This is the error that PCRE "
               "gives when the magic number is not present (PCRE_ERROR_BADMAGIC)";
    case PCRE_ERROR_UNKNOWN_NODE:
        return "While running the pattern match, an unknown item was encountered in the "
               "compiled pattern. This error could be caused by a bug in PCRE or by "
               "overwriting of the compiled pattern (PCRE_ERROR_UNKNOWN_NODE)";
    case PCRE_ERROR_NOMEMORY:
        return "If a pattern contains back references, but the ovector that is passed "
               "to pcre_exec() is not big enough to remember the referenced substrings, "
               "PCRE gets a block of memory at the start of matching to use for this purpose. "
               "If the call via pcre_malloc() fails, this error is given. The memory is "
               "automatically freed at the end of matching. This error is also given if "
               "pcre_stack_malloc() fails in pcre_exec(). "
               "This can happen only when PCRE has been compiled with "
               "--disable-stack-for-recursion (PCRE_ERROR_NOMEMORY)";
    case PCRE_ERROR_NOSUBSTRING:
        return "This error is used by the pcre_copy_substring(), pcre_get_substring(), "
               "and pcre_get_substring_list() functions (see below). "
               "It is never returned by pcre_exec() (PCRE_ERROR_NOSUBSTRING)";
    case PCRE_ERROR_MATCHLIMIT:
        return "The backtracking limit, as specified by the match_limit field in a pcre_extra "
               "structure (or defaulted) was reached. "
               "See the description above (PCRE_ERROR_MATCHLIMIT)";
    case PCRE_ERROR_CALLOUT:
        return "This error is never generated by pcre_exec() itself. "
               "It is provided for use by callout functions that want to yield a distinctive "
               "error code. See the pcrecallout documentation for details (PCRE_ERROR_CALLOUT)";
    case PCRE_ERROR_BADUTF8:
        return "A string that contains an invalid UTF-8 byte sequence was passed as a subject, "
               "and the PCRE_NO_UTF8_CHECK option was not set. If the size of the output vector "
               "(ovecsize) is at least 2, the byte offset to the start of the the invalid UTF-8 "
               "character is placed in the first element, and a reason code is placed in the "
               "second element. The reason codes are listed in the following section. For "
               "backward compatibility, if PCRE_PARTIAL_HARD is set and the problem is a truncated "
               "UTF-8 character at the end of the subject (reason codes 1 to 5), "
               "PCRE_ERROR_SHORTUTF8 is returned instead of PCRE_ERROR_BADUTF8";
    case PCRE_ERROR_BADUTF8_OFFSET:
        return "The UTF-8 byte sequence that was passed as a subject was checked and found to "
               "be valid (the PCRE_NO_UTF8_CHECK option was not set), but the value of "
               "startoffset did not point to the beginning of a UTF-8 character or the end of "
               "the subject (PCRE_ERROR_BADUTF8_OFFSET)";
    case PCRE_ERROR_PARTIAL:
        return "The subject string did not match, but it did match partially. See the "
               "pcrepartial documentation for details of partial matching (PCRE_ERROR_PARTIAL)";
    case PCRE_ERROR_BADPARTIAL:
        return "This code is no longer in use. It was formerly returned when the PCRE_PARTIAL "
               "option was used with a compiled pattern containing items that were not supported "
               "for partial matching. From release 8.00 onwards, there are no restrictions on "
               "partial matching (PCRE_ERROR_BADPARTIAL)";
    case PCRE_ERROR_INTERNAL:
        return "An unexpected internal error has occurred. This error could be caused by a bug "
               "in PCRE or by overwriting of the compiled pattern (PCRE_ERROR_INTERNAL)";
    case PCRE_ERROR_BADCOUNT:
        return "This error is given if the value of the ovecsize argument is negative "
               "(PCRE_ERROR_BADCOUNT)";
    case PCRE_ERROR_RECURSIONLIMIT:
        return "The internal recursion limit, as specified by the match_limit_recursion "
               "field in a pcre_extra structure (or defaulted) was reached. "
               "See the description above (PCRE_ERROR_RECURSIONLIMIT)";
    case PCRE_ERROR_DFA_UITEM:
        return "PCRE_ERROR_DFA_UITEM";
    case PCRE_ERROR_DFA_UCOND:
        return "PCRE_ERROR_DFA_UCOND";
    case PCRE_ERROR_DFA_WSSIZE:
        return "PCRE_ERROR_DFA_WSSIZE";
    case PCRE_ERROR_DFA_RECURSE:
        return "PCRE_ERROR_DFA_RECURSE";
    case PCRE_ERROR_NULLWSLIMIT:
        return "PCRE_ERROR_NULLWSLIMIT";
    case PCRE_ERROR_BADNEWLINE:
        return "An invalid combination of PCRE_NEWLINE_xxx options was "
               "given (PCRE_ERROR_BADNEWLINE)";
    case PCRE_ERROR_BADOFFSET:
        return "The value of startoffset was negative or greater than the length "
               "of the subject, that is, the value in length (PCRE_ERROR_BADOFFSET)";
    case PCRE_ERROR_SHORTUTF8:
        return "This error is returned instead of PCRE_ERROR_BADUTF8 when the subject "
               "string ends with a truncated UTF-8 character and the PCRE_PARTIAL_HARD option is set. "
               "Information about the failure is returned as for PCRE_ERROR_BADUTF8. "
               "It is in fact sufficient to detect this case, but this special error code for "
               "PCRE_PARTIAL_HARD precedes the implementation of returned information; "
               "it is retained for backwards compatibility (PCRE_ERROR_SHORTUTF8)";
    case PCRE_ERROR_RECURSELOOP:
        return "This error is returned when pcre_exec() detects a recursion loop "
               "within the pattern. Specifically, it means that either the whole pattern "
               "or a subpattern has been called recursively for the second time at the same "
               "position in the subject string. Some simple patterns that might do this "
               "are detected and faulted at compile time, but more complicated cases, "
               "in particular mutual recursions between two different subpatterns, "
               "cannot be detected until run time (PCRE_ERROR_RECURSELOOP)";
    case PCRE_ERROR_JIT_STACKLIMIT:
        return "This error is returned when a pattern that was successfully studied "
               "using a JIT compile option is being matched, but the memory available "
               "for the just-in-time processing stack is not large enough. See the pcrejit "
               "documentation for more details (PCRE_ERROR_JIT_STACKLIMIT)";
    case PCRE_ERROR_BADMODE:
        return "This error is given if a pattern that was compiled by the 8-bit library "
               "is passed to a 16-bit or 32-bit library function, or vice versa (PCRE_ERROR_BADMODE)";
    case PCRE_ERROR_BADENDIANNESS:
        return "This error is given if a pattern that was compiled and saved is reloaded on a "
               "host with different endianness. The utility function pcre_pattern_to_host_byte_order() "
               "can be used to convert such a pattern so that it runs on the new host (PCRE_ERROR_BADENDIANNESS)";
    case PCRE_ERROR_DFA_BADRESTART:
        return "PCRE_ERROR_DFA_BADRESTART";
#if PCRE_MAJOR >= 8 && PCRE_MINOR >= 32
    case PCRE_ERROR_BADLENGTH:
        return "This error is given if pcre_exec() is called with a negative value for the length argument (PCRE_ERROR_BADLENGTH)";
    case PCRE_ERROR_JIT_BADOPTION:
        return "This error is returned when a pattern that was successfully studied using a JIT compile "
               "option is being matched, but the matching mode (partial or complete match) does not correspond "
               "to any JIT compilation mode. When the JIT fast path function is used, this error may be "
               "also given for invalid options. See the pcrejit documentation for more details (PCRE_ERROR_JIT_BADOPTION)";
#endif
    }
    return "";
}

void CppCheck::executeRules(const std::string &tokenlist, const Tokenizer &tokenizer)
{
    // There is no rule to execute
    if (!hasRule(tokenlist))
        return;

    // Write all tokens in a string that can be parsed by pcre
    std::ostringstream ostr;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        ostr << " " << tok->str();
    const std::string str(ostr.str());

    for (const Settings::Rule &rule : mSettings.rules) {
        if (rule.pattern.empty() || rule.id.empty() || rule.severity == Severity::none || rule.tokenlist != tokenlist)
            continue;

        if (!mSettings.quiet) {
            reportOut("Processing rule: " + rule.pattern, Color::FgGreen);
        }

        const char *pcreCompileErrorStr = nullptr;
        int erroffset = 0;
        pcre * const re = pcre_compile(rule.pattern.c_str(),0,&pcreCompileErrorStr,&erroffset,nullptr);
        if (!re) {
            if (pcreCompileErrorStr) {
                const std::string msg = "pcre_compile failed: " + std::string(pcreCompileErrorStr);
                const ErrorMessage errmsg(std::list<ErrorMessage::FileLocation>(),
                                          emptyString,
                                          Severity::error,
                                          msg,
                                          "pcre_compile",
                                          Certainty::normal);

                reportErr(errmsg);
            }
            continue;
        }

        // Optimize the regex, but only if PCRE_CONFIG_JIT is available
#ifdef PCRE_CONFIG_JIT
        const char *pcreStudyErrorStr = nullptr;
        pcre_extra * const pcreExtra = pcre_study(re, PCRE_STUDY_JIT_COMPILE, &pcreStudyErrorStr);
        // pcre_study() returns NULL for both errors and when it can not optimize the regex.
        // The last argument is how one checks for errors.
        // It is NULL if everything works, and points to an error string otherwise.
        if (pcreStudyErrorStr) {
            const std::string msg = "pcre_study failed: " + std::string(pcreStudyErrorStr);
            const ErrorMessage errmsg(std::list<ErrorMessage::FileLocation>(),
                                      emptyString,
                                      Severity::error,
                                      msg,
                                      "pcre_study",
                                      Certainty::normal);

            reportErr(errmsg);
            // pcre_compile() worked, but pcre_study() returned an error. Free the resources allocated by pcre_compile().
            pcre_free(re);
            continue;
        }
#else
        const pcre_extra * const pcreExtra = nullptr;
#endif

        int pos = 0;
        int ovector[30]= {0};
        while (pos < (int)str.size()) {
            const int pcreExecRet = pcre_exec(re, pcreExtra, str.c_str(), (int)str.size(), pos, 0, ovector, 30);
            if (pcreExecRet < 0) {
                const std::string errorMessage = pcreErrorCodeToString(pcreExecRet);
                if (!errorMessage.empty()) {
                    const ErrorMessage errmsg(std::list<ErrorMessage::FileLocation>(),
                                              emptyString,
                                              Severity::error,
                                              std::string("pcre_exec failed: ") + errorMessage,
                                              "pcre_exec",
                                              Certainty::normal);

                    reportErr(errmsg);
                }
                break;
            }
            const unsigned int pos1 = (unsigned int)ovector[0];
            const unsigned int pos2 = (unsigned int)ovector[1];

            // jump to the end of the match for the next pcre_exec
            pos = (int)pos2;

            // determine location..
            ErrorMessage::FileLocation loc;
            loc.setfile(tokenizer.list.getSourceFilePath());
            loc.line = 0;

            std::size_t len = 0;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
                len = len + 1U + tok->str().size();
                if (len > pos1) {
                    loc.setfile(tokenizer.list.getFiles().at(tok->fileIndex()));
                    loc.line = tok->linenr();
                    break;
                }
            }

            const std::list<ErrorMessage::FileLocation> callStack(1, loc);

            // Create error message
            std::string summary;
            if (rule.summary.empty())
                summary = "found '" + str.substr(pos1, pos2 - pos1) + "'";
            else
                summary = rule.summary;
            const ErrorMessage errmsg(callStack, tokenizer.list.getSourceFilePath(), rule.severity, summary, rule.id, Certainty::normal);

            // Report error
            reportErr(errmsg);
        }

        pcre_free(re);
#ifdef PCRE_CONFIG_JIT
        // Free up the EXTRA PCRE value (may be NULL at this point)
        if (pcreExtra) {
            pcre_free_study(pcreExtra);
        }
#endif
    }
}
#endif

void CppCheck::executeAddons(const std::string& dumpFile)
{
    if (!dumpFile.empty()) {
        std::vector<std::string> f{dumpFile};
        executeAddons(f);
    }
}

void CppCheck::executeAddons(const std::vector<std::string>& files)
{
    if (mSettings.addons.empty() || files.empty())
        return;

    FilesDeleter filesDeleter;

    std::string fileList;

    if (files.size() >= 2 || endsWith(files[0], ".ctu-info")) {
        fileList = Path::getPathFromFilename(files[0]) + FILELIST;
        filesDeleter.addFile(fileList);
        std::ofstream fout(fileList);
        for (const std::string& f: files)
            fout << f << std::endl;
    }

    for (const std::string &addon : mSettings.addons) {
        struct AddonInfo addonInfo;
        const std::string &failedToGetAddonInfo = addonInfo.getAddonInfo(addon, mSettings.exename);
        if (!failedToGetAddonInfo.empty()) {
            reportOut(failedToGetAddonInfo, Color::FgRed);
            mExitCode = 1;
            continue;
        }
        if (addonInfo.name != "misra" && !addonInfo.ctu && endsWith(files.back(), ".ctu-info"))
            continue;

        const std::string results =
            executeAddon(addonInfo, mSettings.addonPython, fileList.empty() ? files[0] : fileList, mSettings.premiumArgs, mExecuteCommand);
        std::istringstream istr(results);
        std::string line;

        const bool misraC2023 = mSettings.premiumArgs.find("--misra-c-2023") != std::string::npos;

        while (std::getline(istr, line)) {
            if (line.compare(0,1,"{") != 0)
                continue;

            picojson::value res;
            std::istringstream istr2(line);
            istr2 >> res;
            if (!res.is<picojson::object>())
                continue;

            picojson::object obj = res.get<picojson::object>();

            ErrorMessage errmsg;

            if (obj.count("file") > 0) {
                std::string fileName = obj["file"].get<std::string>();
                const int64_t lineNumber = obj["linenr"].get<int64_t>();
                const int64_t column = obj["column"].get<int64_t>();
                errmsg.callStack.emplace_back(std::move(fileName), lineNumber, column);
            } else if (obj.count("loc") > 0) {
                for (const picojson::value &locvalue: obj["loc"].get<picojson::array>()) {
                    picojson::object loc = locvalue.get<picojson::object>();
                    std::string fileName = loc["file"].get<std::string>();
                    const int64_t lineNumber = loc["linenr"].get<int64_t>();
                    const int64_t column = loc["column"].get<int64_t>();
                    const std::string info = loc["info"].get<std::string>();
                    errmsg.callStack.emplace_back(std::move(fileName), info, lineNumber, column);
                }
            }

            errmsg.id = obj["addon"].get<std::string>() + "-" + obj["errorId"].get<std::string>();
            if (misraC2023 && errmsg.id.compare(0, 12, "misra-c2012-") == 0)
                errmsg.id = "misra-c2023-" + errmsg.id.substr(12);
            const std::string text = obj["message"].get<std::string>();
            errmsg.setmsg(text);
            const std::string severity = obj["severity"].get<std::string>();
            errmsg.severity = Severity::fromString(severity);
            if (errmsg.severity == Severity::SeverityType::none) {
                if (!endsWith(errmsg.id, "-logChecker"))
                    continue;
            }
            else if (!mSettings.severity.isEnabled(errmsg.severity))
                continue;
            errmsg.file0 = ((files.size() == 1) ? files[0] : "");

            reportErr(errmsg);
        }
    }
}

void CppCheck::executeAddonsWholeProgram(const std::map<std::string, std::size_t> &files)
{
    if (mSettings.addons.empty())
        return;

    std::vector<std::string> ctuInfoFiles;
    for (const auto &f: files) {
        const std::string &dumpFileName = getDumpFileName(mSettings, f.first);
        ctuInfoFiles.push_back(getCtuInfoFileName(dumpFileName));
    }

    try {
        executeAddons(ctuInfoFiles);
    } catch (const InternalError& e) {
        internalError("", "Whole program analysis failed: " + e.errorMessage);
    }

    if (mSettings.buildDir.empty()) {
        for (const std::string &f: ctuInfoFiles)
            std::remove(f.c_str());
    }
}

Settings &CppCheck::settings()
{
    return mSettings;
}

void CppCheck::tooManyConfigsError(const std::string &file, const int numberOfConfigurations)
{
    if (!mSettings.severity.isEnabled(Severity::information) && !mTooManyConfigs)
        return;

    mTooManyConfigs = false;

    if (mSettings.severity.isEnabled(Severity::information) && file.empty())
        return;

    std::list<ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        loclist.emplace_back(file);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << mSettings.maxConfigs;
    if (numberOfConfigurations > mSettings.maxConfigs)
        msg << " of " << numberOfConfigurations << " configurations. Use --force to check all configurations.\n";
    if (file.empty())
        msg << " configurations. Use --force to check all configurations. For more details, use --enable=information.\n";
    msg << "The checking of the file will be interrupted because there are too many "
        "#ifdef configurations. Checking of all #ifdef configurations can be forced "
        "by --force command line option or from GUI preferences. However that may "
        "increase the checking time.";
    if (file.empty())
        msg << " For more details, use --enable=information.";


    ErrorMessage errmsg(loclist,
                        emptyString,
                        Severity::information,
                        msg.str(),
                        "toomanyconfigs", CWE398,
                        Certainty::normal);

    reportErr(errmsg);
}

void CppCheck::purgedConfigurationMessage(const std::string &file, const std::string& configuration)
{
    mTooManyConfigs = false;

    if (mSettings.severity.isEnabled(Severity::information) && file.empty())
        return;

    std::list<ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        loclist.emplace_back(file);
    }

    ErrorMessage errmsg(loclist,
                        emptyString,
                        Severity::information,
                        "The configuration '" + configuration + "' was not checked because its code equals another one.",
                        "purgedConfiguration",
                        Certainty::normal);

    reportErr(errmsg);
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorMessage &msg)
{
    if (msg.severity == Severity::none && (msg.id == "logChecker" || endsWith(msg.id, "-logChecker"))) {
        mErrorLogger.reportErr(msg);
        return;
    }

    if (!mSettings.library.reportErrors(msg.file0))
        return;

    const std::string errmsg = msg.toString(mSettings.verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors
    if (std::find(mErrorList.cbegin(), mErrorList.cend(), errmsg) != mErrorList.cend())
        return;

    if (!mSettings.buildDir.empty())
        mAnalyzerInformation.reportErr(msg);

    // TODO: only convert if necessary
    const auto errorMessage = Suppressions::ErrorMessage::fromErrorMessage(msg);

    if (mSettings.nomsg.isSuppressed(errorMessage, mUseGlobalSuppressions)) {
        return;
    }

    if (!mSettings.nofail.isSuppressed(errorMessage) && !mSettings.nomsg.isSuppressed(errorMessage)) {
        mExitCode = 1;
    }

    mErrorList.push_back(errmsg);

    mErrorLogger.reportErr(msg);
    // check if plistOutput should be populated and the current output file is open and the error is not suppressed
    if (!mSettings.plistOutput.empty() && mPlistFile.is_open() && !mSettings.nomsg.isSuppressed(errorMessage)) {
        // add error to plist output file
        mPlistFile << ErrorLogger::plistData(msg);
    }
}

void CppCheck::reportOut(const std::string &outmsg, Color c)
{
    mErrorLogger.reportOut(outmsg, c);
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    mErrorLogger.reportProgress(filename, stage, value);
}

void CppCheck::getErrorMessages(ErrorLogger &errorlogger)
{
    Settings s;
    s.severity.enable(Severity::warning);
    s.severity.enable(Severity::style);
    s.severity.enable(Severity::portability);
    s.severity.enable(Severity::performance);
    s.severity.enable(Severity::information);

    CppCheck cppcheck(errorlogger, true, nullptr);
    cppcheck.purgedConfigurationMessage(emptyString,emptyString);
    cppcheck.mTooManyConfigs = true;
    cppcheck.tooManyConfigsError(emptyString,0U);
    // TODO: add functions to get remaining error messages

    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().cbegin(); it != Check::instances().cend(); ++it)
        (*it)->getErrorMessages(&errorlogger, &s);

    Preprocessor::getErrorMessages(&errorlogger, &s);
}

void CppCheck::analyseClangTidy(const ImportProject::FileSettings &fileSettings)
{
    std::string allIncludes;
    for (const std::string &inc : fileSettings.includePaths) {
        allIncludes = allIncludes + "-I\"" + inc + "\" ";
    }

    const std::string allDefines = getDefinesFlags(fileSettings.defines);

#ifdef _WIN32
    const char exe[] = "clang-tidy.exe";
#else
    const char exe[] = "clang-tidy";
#endif

    const std::string args = "-quiet -checks=*,-clang-analyzer-*,-llvm* \"" + fileSettings.filename + "\" -- " + allIncludes + allDefines;
    std::string output;
    if (!mExecuteCommand(exe, split(args), emptyString, output)) {
        std::cerr << "Failed to execute '" << exe << "'" << std::endl;
        return;
    }

    // parse output and create error messages
    std::istringstream istr(output);
    std::string line;

    if (!mSettings.buildDir.empty()) {
        const std::string analyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(mSettings.buildDir, fileSettings.filename, emptyString);
        std::ofstream fcmd(analyzerInfoFile + ".clang-tidy-cmd");
        fcmd << istr.str();
    }

    while (std::getline(istr, line)) {
        if (line.find("error") == std::string::npos && line.find("warning") == std::string::npos)
            continue;

        std::size_t endColumnPos = line.find(": error:");
        if (endColumnPos == std::string::npos) {
            endColumnPos = line.find(": warning:");
        }

        const std::size_t endLinePos = line.rfind(':', endColumnPos-1);
        const std::size_t endNamePos = line.rfind(':', endLinePos - 1);
        const std::size_t endMsgTypePos = line.find(':', endColumnPos + 2);
        const std::size_t endErrorPos = line.rfind('[', std::string::npos);
        if (endLinePos==std::string::npos || endNamePos==std::string::npos || endMsgTypePos==std::string::npos || endErrorPos==std::string::npos)
            continue;

        const std::string lineNumString = line.substr(endNamePos + 1, endLinePos - endNamePos - 1);
        const std::string columnNumString = line.substr(endLinePos + 1, endColumnPos - endLinePos - 1);
        const std::string messageString = line.substr(endMsgTypePos + 1, endErrorPos - endMsgTypePos - 1);
        const std::string errorString = line.substr(endErrorPos, line.length());

        std::string fixedpath = Path::simplifyPath(line.substr(0, endNamePos));
        const int64_t lineNumber = strToInt<int64_t>(lineNumString);
        const int64_t column = strToInt<int64_t>(columnNumString);
        fixedpath = Path::toNativeSeparators(fixedpath);

        ErrorMessage errmsg;
        errmsg.callStack.emplace_back(fixedpath, lineNumber, column);

        errmsg.id = "clang-tidy-" + errorString.substr(1, errorString.length() - 2);
        if (errmsg.id.find("performance") != std::string::npos)
            errmsg.severity = Severity::SeverityType::performance;
        else if (errmsg.id.find("portability") != std::string::npos)
            errmsg.severity = Severity::SeverityType::portability;
        else if (errmsg.id.find("cert") != std::string::npos || errmsg.id.find("misc") != std::string::npos || errmsg.id.find("unused") != std::string::npos)
            errmsg.severity = Severity::SeverityType::warning;
        else
            errmsg.severity = Severity::SeverityType::style;

        errmsg.file0 = fixedpath;
        errmsg.setmsg(messageString);
        reportErr(errmsg);
    }
}

bool CppCheck::analyseWholeProgram()
{
    bool errors = false;
    // Init CTU
    CTU::maxCtuDepth = mSettings.maxCtuDepth;
    // Analyse the tokens
    CTU::FileInfo ctu;
    for (const Check::FileInfo *fi : mFileInfo) {
        const CTU::FileInfo *fi2 = dynamic_cast<const CTU::FileInfo *>(fi);
        if (fi2) {
            ctu.functionCalls.insert(ctu.functionCalls.end(), fi2->functionCalls.cbegin(), fi2->functionCalls.cend());
            ctu.nestedCalls.insert(ctu.nestedCalls.end(), fi2->nestedCalls.cbegin(), fi2->nestedCalls.cend());
        }
    }
    for (Check *check : Check::instances())
        errors |= check->analyseWholeProgram(&ctu, mFileInfo, mSettings, *this);  // TODO: ctu
    return errors && (mExitCode > 0);
}

void CppCheck::analyseWholeProgram(const std::string &buildDir, const std::map<std::string, std::size_t> &files)
{
    executeAddonsWholeProgram(files);
    if (buildDir.empty()) {
        removeCtuInfoFiles(files);
        return;
    }
    if (mSettings.checks.isEnabled(Checks::unusedFunction))
        CheckUnusedFunctions::analyseWholeProgram(mSettings, this, buildDir);
    std::list<Check::FileInfo*> fileInfoList;
    CTU::FileInfo ctuFileInfo;

    // Load all analyzer info data..
    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt);
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        const std::string::size_type firstColon = filesTxtLine.find(':');
        if (firstColon == std::string::npos)
            continue;
        const std::string::size_type lastColon = filesTxtLine.rfind(':');
        if (firstColon == lastColon)
            continue;
        const std::string xmlfile = buildDir + '/' + filesTxtLine.substr(0,firstColon);
        //const std::string sourcefile = filesTxtLine.substr(lastColon+1);

        tinyxml2::XMLDocument doc;
        const tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
        if (error != tinyxml2::XML_SUCCESS)
            continue;

        const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
        if (rootNode == nullptr)
            continue;

        for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "FileInfo") != 0)
                continue;
            const char *checkClassAttr = e->Attribute("check");
            if (!checkClassAttr)
                continue;
            if (std::strcmp(checkClassAttr, "ctu") == 0) {
                ctuFileInfo.loadFromXml(e);
                continue;
            }
            for (const Check *check : Check::instances()) {
                if (checkClassAttr == check->name())
                    fileInfoList.push_back(check->loadFileInfoFromXml(e));
            }
        }
    }

    // Set CTU max depth
    CTU::maxCtuDepth = mSettings.maxCtuDepth;

    // Analyse the tokens
    for (Check *check : Check::instances())
        check->analyseWholeProgram(&ctuFileInfo, fileInfoList, mSettings, *this);

    for (Check::FileInfo *fi : fileInfoList)
        delete fi;
}

bool CppCheck::isUnusedFunctionCheckEnabled() const
{
    return (mSettings.useSingleJob() && mSettings.checks.isEnabled(Checks::unusedFunction));
}

void CppCheck::removeCtuInfoFiles(const std::map<std::string, std::size_t> &files)
{
    if (mSettings.buildDir.empty()) {
        for (const auto& f: files) {
            const std::string &dumpFileName = getDumpFileName(mSettings, f.first);
            const std::string &ctuInfoFileName = getCtuInfoFileName(dumpFileName);
            std::remove(ctuInfoFileName.c_str());
        }
        for (const auto& fs: mSettings.project.fileSettings) {
            const std::string &dumpFileName = getDumpFileName(mSettings, fs.filename);
            const std::string &ctuInfoFileName = getCtuInfoFileName(dumpFileName);
            std::remove(ctuInfoFileName.c_str());
        }
    }
}
