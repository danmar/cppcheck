/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "addoninfo.h"
#include "check.h"
#include "checkunusedfunctions.h"
#include "clangimport.h"
#include "color.h"
#include "ctu.h"
#include "errortypes.h"
#include "filesettings.h"
#include "library.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "version.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <exception> // IWYU pragma: keep
#include <fstream>
#include <iostream>
#include <new>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "json.h"

#include <simplecpp.h>

#include "xml.h"

#ifdef HAVE_RULES
#ifdef _WIN32
#define PCRE_STATIC
#endif
#include <pcre.h>
#endif

class SymbolDatabase;

static constexpr char Version[] = CPPCHECK_VERSION_STRING;
static constexpr char ExtraVersion[] = "";

static constexpr char FILELIST[] = "cppcheck-addon-ctu-file-list";

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

static std::string cmdFileName(std::string f)
{
    f = Path::toNativeSeparators(std::move(f));
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

static std::string getDumpFileName(const Settings& settings, const std::string& filename)
{
    if (!settings.dumpFile.empty())
        return settings.dumpFile;

    std::string extension;
    if (settings.dump || !settings.buildDir.empty())
        extension = ".dump";
    else
        extension = "." + std::to_string(settings.pid) + ".dump";

    if (!settings.dump && !settings.buildDir.empty())
        return AnalyzerInformation::getAnalyzerInfoFile(settings.buildDir, filename, emptyString) + extension;
    return filename + extension;
}

static std::string getCtuInfoFileName(const std::string &dumpFile)
{
    return dumpFile.substr(0, dumpFile.size()-4) + "ctu-info";
}

static void createDumpFile(const Settings& settings,
                           const FileWithDetails& file,
                           std::ofstream& fdump,
                           std::string& dumpFile)
{
    if (!settings.dump && settings.addons.empty())
        return;
    dumpFile = getDumpFileName(settings, file.spath());

    fdump.open(dumpFile);
    if (!fdump.is_open())
        return;

    if (!settings.buildDir.empty()) {
        std::ofstream fout(getCtuInfoFileName(dumpFile));
    }

    // TODO: enforcedLang should be already applied in FileWithDetails object
    std::string language;
    switch (settings.enforcedLang) {
    case Standards::Language::C:
        language = " language=\"c\"";
        break;
    case Standards::Language::CPP:
        language = " language=\"cpp\"";
        break;
    case Standards::Language::None:
    {
        // TODO: get language from FileWithDetails object
        // TODO: error out on unknown language?
        const Standards::Language lang = Path::identify(file.spath(), settings.cppHeaderProbe);
        if (lang == Standards::Language::CPP)
            language = " language=\"cpp\"";
        else if (lang == Standards::Language::C)
            language = " language=\"c\"";
        break;
    }
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

static std::string detectPython(const CppCheck::ExecuteCmdFn &executeCommand)
{
#ifdef _WIN32
    const char *py_exes[] = { "python3.exe", "python.exe" };
#else
    const char *py_exes[] = { "python3", "python" };
#endif
    for (const char* py_exe : py_exes) {
        std::string out;
#ifdef _MSC_VER
        // FIXME: hack to avoid debug assertion with _popen() in executeCommand() for non-existing commands
        const std::string cmd = std::string(py_exe) + " --version >NUL 2>&1";
        if (system(cmd.c_str()) != 0) {
            // TODO: get more detailed error?
            continue;
        }
#endif
        if (executeCommand(py_exe, split("--version"), "2>&1", out) == EXIT_SUCCESS && startsWith(out, "Python ") && std::isdigit(out[7])) {
            return py_exe;
        }
    }
    return "";
}

static std::vector<picojson::value> executeAddon(const AddonInfo &addonInfo,
                                                 const std::string &defaultPythonExe,
                                                 const std::string &file,
                                                 const std::string &premiumArgs,
                                                 const CppCheck::ExecuteCmdFn &executeCommand)
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
        // store in static variable so we only look this up once
        static const std::string detectedPythonExe = detectPython(executeCommand);
        if (detectedPythonExe.empty())
            throw InternalError(nullptr, "Failed to auto detect python");
        pythonExe = detectedPythonExe;
    }

    std::string args;
    if (addonInfo.executable.empty())
        args = cmdFileName(addonInfo.runScript) + " " + cmdFileName(addonInfo.scriptFile);
    args += std::string(args.empty() ? "" : " ") + "--cli" + addonInfo.args;
    if (!premiumArgs.empty() && !addonInfo.executable.empty())
        args += " " + premiumArgs;

    const bool is_file_list = (file.find(FILELIST) != std::string::npos);
    const std::string fileArg = (is_file_list ? " --file-list " : " ") + cmdFileName(file);
    args += fileArg;

    std::string result;
    if (const int exitcode = executeCommand(pythonExe, split(args), redirect, result)) {
        std::string message("Failed to execute addon '" + addonInfo.name + "' - exitcode is " + std::to_string(exitcode));
        std::string details = pythonExe + " " + args;
        if (result.size() > 2) {
            details += "\nOutput:\n";
            details += result;
            const auto pos = details.find_last_not_of("\n\r");
            if (pos != std::string::npos)
                details.resize(pos + 1);
        }
        throw InternalError(nullptr, std::move(message), std::move(details));
    }

    std::vector<picojson::value> addonResult;

    // Validate output..
    std::istringstream istr(result);
    std::string line;
    while (std::getline(istr, line)) {
        // TODO: also bail out?
        if (line.empty()) {
            //std::cout << "addon '" << addonInfo.name <<  "' result contains empty line" << std::endl;
            continue;
        }

        // TODO: get rid of this
        if (startsWith(line,"Checking ")) {
            //std::cout << "addon '" << addonInfo.name <<  "' result contains 'Checking ' line" << std::endl;
            continue;
        }

        if (line[0] != '{') {
            //std::cout << "addon '" << addonInfo.name <<  "' result is not a JSON" << std::endl;

            result.erase(result.find_last_not_of('\n') + 1, std::string::npos); // Remove trailing newlines
            throw InternalError(nullptr, "Failed to execute '" + pythonExe + " " + args + "'. " + result);
        }

        //std::cout << "addon '" << addonInfo.name <<  "' result is " << line << std::endl;

        // TODO: make these failures?
        picojson::value res;
        const std::string err = picojson::parse(res, line);
        if (!err.empty()) {
            //std::cout << "addon '" << addonInfo.name <<  "' result is not a valid JSON (" << err << ")" << std::endl;
            continue;
        }
        if (!res.is<picojson::object>()) {
            //std::cout << "addon '" << addonInfo.name <<  "' result is not a JSON object" << std::endl;
            continue;
        }
        addonResult.emplace_back(std::move(res));
    }

    // Valid results
    return addonResult;
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
                   ExecuteCmdFn executeCommand)
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
        const int line_i = strToInt<int>(linenr);
        const int column = strToInt<unsigned int>(colnr);
        ErrorMessage::FileLocation loc(locFile, line_i, column);
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

std::string CppCheck::getLibraryDumpData() const {
    std::string out;
    for (const std::string &s : mSettings.libraries) {
        out += "  <library lib=\"" + s + "\"/>\n";
    }
    return out;
}

unsigned int CppCheck::checkClang(const FileWithDetails &file)
{
    if (mSettings.checks.isEnabled(Checks::unusedFunction) && !mUnusedFunctionsCheck)
        mUnusedFunctionsCheck.reset(new CheckUnusedFunctions());

    if (!mSettings.quiet)
        mErrorLogger.reportOut(std::string("Checking ") + file.spath() + " ...", Color::FgGreen);

    // TODO: get language from FileWithDetails object
    bool isCpp;
    if (mSettings.enforcedLang != Standards::None)
        isCpp = (mSettings.enforcedLang == Standards::CPP);
    else
        isCpp = Path::identify(file.spath(), mSettings.cppHeaderProbe) == Standards::Language::CPP;
    const std::string langOpt = isCpp ? "-x c++" : "-x c";
    const std::string analyzerInfo = mSettings.buildDir.empty() ? std::string() : AnalyzerInformation::getAnalyzerInfoFile(mSettings.buildDir, file.spath(), emptyString);
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

    std::string flags(langOpt + " ");
    if (isCpp && !mSettings.standards.stdValueCPP.empty())
        flags += "-std=" + mSettings.standards.stdValueCPP + " ";
    if (!isCpp && !mSettings.standards.stdValueC.empty())
        flags += "-std=" + mSettings.standards.stdValueC + " ";

    for (const std::string &i: mSettings.includePaths)
        flags += "-I" + i + " ";

    flags += getDefinesFlags(mSettings.userDefines);

    const std::string args2 = "-fsyntax-only -Xclang -ast-dump -fno-color-diagnostics " + flags + file.spath();
    const std::string redirect2 = analyzerInfo.empty() ? std::string("2>&1") : ("2> " + clangStderr);
    if (!mSettings.buildDir.empty()) {
        std::ofstream fout(clangcmd);
        fout << exe << " " << args2 << " " << redirect2 << std::endl;
    } else if (mSettings.verbose && !mSettings.quiet) {
        mErrorLogger.reportOut(exe + " " + args2);
    }

    std::string output2;
    const int exitcode = mExecuteCommand(exe,split(args2),redirect2,output2);
    if (exitcode != EXIT_SUCCESS) {
        // TODO: report as proper error
        std::cerr << "Failed to execute '" << exe << " " << args2 << " " << redirect2 << "' - (exitcode: " << exitcode << " / output: " << output2 << ")" << std::endl;
        return 0; // TODO: report as failure?
    }

    if (output2.find("TranslationUnitDecl") == std::string::npos) {
        // TODO: report as proper error
        std::cerr << "Failed to execute '" << exe << " " << args2 << " " << redirect2 << "' - (no TranslationUnitDecl in output)" << std::endl;
        return 0; // TODO: report as failure?
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
        Tokenizer tokenizer(mSettings, *this);
        tokenizer.list.appendFileIfNew(file.spath());
        std::istringstream ast(output2);
        clangimport::parseClangAstDump(tokenizer, ast);
        ValueFlow::setValues(tokenizer.list,
                             const_cast<SymbolDatabase&>(*tokenizer.getSymbolDatabase()),
                             *this,
                             mSettings,
                             &s_timerResults);
        if (mSettings.debugnormal)
            tokenizer.printDebugOutput(1, std::cout);
        checkNormalTokens(tokenizer);

        // create dumpfile
        std::ofstream fdump;
        std::string dumpFile;
        createDumpFile(mSettings, file, fdump, dumpFile);
        if (fdump.is_open()) {
            // TODO: use tinyxml2 to create XML
            fdump << "<dump cfg=\"\">\n";
            for (const ErrorMessage& errmsg: compilerWarnings)
                fdump << "  <clang-warning file=\"" << toxml(errmsg.callStack.front().getfile()) << "\" line=\"" << errmsg.callStack.front().line << "\" column=\"" << errmsg.callStack.front().column << "\" message=\"" << toxml(errmsg.shortMessage()) << "\"/>\n";
            fdump << "  <standards>\n";
            fdump << "    <c version=\"" << mSettings.standards.getC() << "\"/>\n";
            fdump << "    <cpp version=\"" << mSettings.standards.getCPP() << "\"/>\n";
            fdump << "  </standards>\n";
            fdump << getLibraryDumpData();
            tokenizer.dump(fdump);
            fdump << "</dump>\n";
            fdump << "</dumps>\n";
            fdump.close();
        }

        // run addons
        executeAddons(dumpFile, file);

    } catch (const InternalError &e) {
        const ErrorMessage errmsg = ErrorMessage::fromInternalError(e, nullptr, file.spath(), "Bailing out from analysis: Processing Clang AST dump failed");
        reportErr(errmsg);
    } catch (const TerminateException &) {
        // Analysis is terminated
        return mExitCode;
    } catch (const std::exception &e) {
        internalError(file.spath(), std::string("Processing Clang AST dump failed: ") + e.what());
    }

    return mExitCode;
}

unsigned int CppCheck::check(const FileWithDetails &file)
{
    if (mSettings.clang)
        return checkClang(file);

    return checkFile(file, emptyString);
}

unsigned int CppCheck::check(const FileWithDetails &file, const std::string &content)
{
    std::istringstream iss(content);
    return checkFile(file, emptyString, &iss);
}

unsigned int CppCheck::check(const FileSettings &fs)
{
    // TODO: move to constructor when CppCheck no longer owns the settings
    if (mSettings.checks.isEnabled(Checks::unusedFunction) && !mUnusedFunctionsCheck)
        mUnusedFunctionsCheck.reset(new CheckUnusedFunctions());

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
    if (fs.platformType != Platform::Type::Unspecified)
        temp.mSettings.platform.set(fs.platformType);
    if (mSettings.clang) {
        temp.mSettings.includePaths.insert(temp.mSettings.includePaths.end(), fs.systemIncludePaths.cbegin(), fs.systemIncludePaths.cend());
        // TODO: propagate back suppressions
        // TODO: propagate back mFileInfo
        const unsigned int returnValue = temp.check(fs.file);
        if (mUnusedFunctionsCheck)
            mUnusedFunctionsCheck->updateFunctionData(*temp.mUnusedFunctionsCheck);
        return returnValue;
    }
    const unsigned int returnValue = temp.checkFile(fs.file, fs.cfg);
    mSettings.supprs.nomsg.addSuppressions(temp.mSettings.supprs.nomsg.getSuppressions());
    if (mUnusedFunctionsCheck)
        mUnusedFunctionsCheck->updateFunctionData(*temp.mUnusedFunctionsCheck);
    while (!temp.mFileInfo.empty()) {
        mFileInfo.push_back(temp.mFileInfo.back());
        temp.mFileInfo.pop_back();
    }
    // TODO: propagate back more data?
    return returnValue;
}

static simplecpp::TokenList createTokenList(const std::string& filename, std::vector<std::string>& files, simplecpp::OutputList* outputList, std::istream* fileStream)
{
    if (fileStream)
        return {*fileStream, files, filename, outputList};

    return {filename, files, outputList};
}

unsigned int CppCheck::checkFile(const FileWithDetails& file, const std::string &cfgname, std::istream* fileStream)
{
    // TODO: move to constructor when CppCheck no longer owns the settings
    if (mSettings.checks.isEnabled(Checks::unusedFunction) && !mUnusedFunctionsCheck)
        mUnusedFunctionsCheck.reset(new CheckUnusedFunctions());

    mExitCode = 0;

    if (Settings::terminated())
        return mExitCode;

    const Timer fileTotalTimer(mSettings.showtime == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL, file.spath());

    if (!mSettings.quiet) {
        std::string fixedpath = Path::toNativeSeparators(file.spath());
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

    try {
        if (mSettings.library.markupFile(file.spath())) {
            if (mUnusedFunctionsCheck && (mSettings.useSingleJob() || !mSettings.buildDir.empty())) {
                // this is not a real source file - we just want to tokenize it. treat it as C anyways as the language needs to be determined.
                Tokenizer tokenizer(mSettings, *this);
                // enforce the language since markup files are special and do not adhere to the enforced language
                tokenizer.list.setLang(Standards::Language::C, true);
                if (fileStream) {
                    tokenizer.list.createTokens(*fileStream, file.spath());
                }
                else {
                    std::ifstream in(file.spath());
                    tokenizer.list.createTokens(in, file.spath());
                }
                mUnusedFunctionsCheck->parseTokens(tokenizer, mSettings);
                // TODO: set analyzer information
            }
            return EXIT_SUCCESS;
        }

        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1 = createTokenList(file.spath(), files, &outputList, fileStream);

        // If there is a syntax error, report it and stop
        const auto output_it = std::find_if(outputList.cbegin(), outputList.cend(), [](const simplecpp::Output &output){
            return Preprocessor::hasErrors(output);
        });
        if (output_it != outputList.cend()) {
            const simplecpp::Output &output = *output_it;
            std::string locfile = Path::fromNativeSeparators(output.location.file());
            if (mSettings.relativePaths)
                locfile = Path::getRelativePath(locfile, mSettings.basePaths);

            ErrorMessage::FileLocation loc1(locfile, output.location.line, output.location.col);

            ErrorMessage errmsg({std::move(loc1)},
                                "", // TODO: is this correct?
                                Severity::error,
                                output.msg,
                                "syntaxError",
                                Certainty::normal);
            reportErr(errmsg);
            return mExitCode;
        }

        Preprocessor preprocessor(mSettings, *this);

        if (!preprocessor.loadFiles(tokens1, files))
            return mExitCode;

        if (!mSettings.plistOutput.empty()) {
            std::string filename2;
            if (file.spath().find('/') != std::string::npos)
                filename2 = file.spath().substr(file.spath().rfind('/') + 1);
            else
                filename2 = file.spath();
            const std::size_t fileNameHash = std::hash<std::string> {}(file.spath());
            filename2 = mSettings.plistOutput + filename2.substr(0, filename2.find('.')) + "_" + std::to_string(fileNameHash) + ".plist";
            mPlistFile.open(filename2);
            mPlistFile << ErrorLogger::plistHeader(version(), files);
        }

        std::string dumpProlog;
        if (mSettings.dump || !mSettings.addons.empty()) {
            dumpProlog += getDumpFileContentsRawTokens(files, tokens1);
        }

        // Parse comments and then remove them
        mRemarkComments = preprocessor.getRemarkComments(tokens1);
        preprocessor.inlineSuppressions(tokens1, mSettings.supprs.nomsg);
        if (mSettings.dump || !mSettings.addons.empty()) {
            std::ostringstream oss;
            mSettings.supprs.nomsg.dump(oss);
            dumpProlog += oss.str();
        }
        preprocessor.removeComments(tokens1);

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
            mSettings.supprs.nomsg.dump(toolinfo);

            // Calculate hash so it can be compared with old hash / future hashes
            const std::size_t hash = preprocessor.calculateHash(tokens1, toolinfo.str());
            std::list<ErrorMessage> errors;
            if (!mAnalyzerInformation.analyzeFile(mSettings.buildDir, file.spath(), cfgname, hash, errors)) {
                while (!errors.empty()) {
                    reportErr(errors.front());
                    errors.pop_front();
                }
                return mExitCode;  // known results => no need to reanalyze file
            }
        }

        FilesDeleter filesDeleter;

        // write dump file xml prolog
        std::ofstream fdump;
        std::string dumpFile;
        createDumpFile(mSettings, file, fdump, dumpFile);
        if (fdump.is_open()) {
            fdump << dumpProlog;
            if (!mSettings.dump)
                filesDeleter.addFile(dumpFile);
        }

        // Get directives
        std::list<Directive> directives = preprocessor.createDirectives(tokens1);
        preprocessor.simplifyPragmaAsm(tokens1);

        Preprocessor::setPlatformInfo(tokens1, mSettings);

        // Get configurations..
        std::set<std::string> configurations;
        if ((mSettings.checkAllConfigurations && mSettings.userDefines.empty()) || mSettings.force) {
            Timer::run("Preprocessor::getConfigs", mSettings.showtime, &s_timerResults, [&]() {
                configurations = preprocessor.getConfigs(tokens1);
            });
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
        if (hasRule("define")) {
            std::string code;
            for (const Directive &dir : directives) {
                if (startsWith(dir.str,"#define ") || startsWith(dir.str,"#include "))
                    code += "#line " + std::to_string(dir.linenr) + " \"" + dir.file + "\"\n" + dir.str + '\n';
            }
            TokenList tokenlist(&mSettings);
            std::istringstream istr2(code);
            // TODO: asserts when file has unknown extension
            tokenlist.createTokens(istr2, Path::identify(*files.begin(), false)); // TODO: check result?
            executeRules("define", tokenlist);
        }
#endif

        if (!mSettings.force && configurations.size() > mSettings.maxConfigs) {
            if (mSettings.severity.isEnabled(Severity::information)) {
                tooManyConfigsError(Path::toNativeSeparators(file.spath()),configurations.size());
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
                std::string codeWithoutCfg;
                Timer::run("Preprocessor::getcode", mSettings.showtime, &s_timerResults, [&]() {
                    codeWithoutCfg = preprocessor.getcode(tokens1, mCurrentConfig, files, true);
                });

                if (startsWith(codeWithoutCfg,"#file"))
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

            Tokenizer tokenizer(mSettings, *this);
            if (mSettings.showtime != SHOWTIME_MODES::SHOWTIME_NONE)
                tokenizer.setTimerResults(&s_timerResults);
            tokenizer.setDirectives(directives); // TODO: how to avoid repeated copies?

            try {
                // Create tokens, skip rest of iteration if failed
                Timer::run("Tokenizer::createTokens", mSettings.showtime, &s_timerResults, [&]() {
                    simplecpp::TokenList tokensP = preprocessor.preprocess(tokens1, mCurrentConfig, files, true);
                    tokenizer.list.createTokens(std::move(tokensP));
                });
                hasValidConfig = true;

                // locations macros
                mLocationMacros.clear();
                for (const Token* tok = tokenizer.tokens(); tok; tok = tok->next()) {
                    if (!tok->getMacroName().empty())
                        mLocationMacros[Location(files[tok->fileIndex()], tok->linenr())].emplace(tok->getMacroName());
                }

                // If only errors are printed, print filename after the check
                if (!mSettings.quiet && (!mCurrentConfig.empty() || checkCount > 1)) {
                    std::string fixedpath = Path::toNativeSeparators(file.spath());
                    mErrorLogger.reportOut("Checking " + fixedpath + ": " + mCurrentConfig + "...", Color::FgGreen);
                }

                if (!tokenizer.tokens())
                    continue;

                // skip rest of iteration if just checking configuration
                if (mSettings.checkConfiguration)
                    continue;

#ifdef HAVE_RULES
                // Execute rules for "raw" code
                executeRules("raw", tokenizer.list);
#endif

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
                    fdump << getLibraryDumpData();
                    preprocessor.dump(fdump);
                    tokenizer.dump(fdump);
                    fdump << "</dump>" << std::endl;
                }

                // Need to call this even if the hash will skip this configuration
                mSettings.supprs.nomsg.markUnmatchedInlineSuppressionsAsChecked(tokenizer);

                // Skip if we already met the same simplified token list
                if (mSettings.force || mSettings.maxConfigs > 1) {
                    const std::size_t hash = tokenizer.list.calculateHash();
                    if (hashes.find(hash) != hashes.end()) {
                        if (mSettings.debugwarnings)
                            purgedConfigurationMessage(file.spath(), mCurrentConfig);
                        continue;
                    }
                    hashes.insert(hash);
                }

                // Check normal tokens
                checkNormalTokens(tokenizer);
            } catch (const simplecpp::Output &o) {
                // #error etc during preprocessing
                configurationError.push_back((mCurrentConfig.empty() ? "\'\'" : mCurrentConfig) + " : [" + o.location.file() + ':' + std::to_string(o.location.line) + "] " + o.msg);
                --checkCount; // don't count invalid configurations

                if (!hasValidConfig && currCfg == *configurations.rbegin()) {
                    // If there is no valid configuration then report error..
                    std::string locfile = Path::fromNativeSeparators(o.location.file());
                    if (mSettings.relativePaths)
                        locfile = Path::getRelativePath(locfile, mSettings.basePaths);

                    ErrorMessage::FileLocation loc1(locfile, o.location.line, o.location.col);

                    ErrorMessage errmsg({std::move(loc1)},
                                        file.spath(),
                                        Severity::error,
                                        o.msg,
                                        "preprocessorErrorDirective",
                                        Certainty::normal);
                    reportErr(errmsg);
                }
                continue;

            } catch (const TerminateException &) {
                // Analysis is terminated
                return mExitCode;

            } catch (const InternalError &e) {
                ErrorMessage errmsg = ErrorMessage::fromInternalError(e, &tokenizer.list, file.spath());
                reportErr(errmsg);
            }
        }

        if (!hasValidConfig && configurations.size() > 1 && mSettings.severity.isEnabled(Severity::information)) {
            std::string msg;
            msg = "This file is not analyzed. Cppcheck failed to extract a valid configuration. Use -v for more details.";
            msg += "\nThis file is not analyzed. Cppcheck failed to extract a valid configuration. The tested configurations have these preprocessor errors:";
            for (const std::string &s : configurationError)
                msg += '\n' + s;

            const std::string locFile = Path::toNativeSeparators(file.spath());
            ErrorMessage::FileLocation loc(locFile, 0, 0);
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

        executeAddons(dumpFile, file);

    } catch (const TerminateException &) {
        // Analysis is terminated
        return mExitCode;
    } catch (const std::runtime_error &e) {
        internalError(file.spath(), std::string("Checking file failed: ") + e.what());
    } catch (const std::bad_alloc &) {
        internalError(file.spath(), "Checking file failed: out of memory");
    } catch (const InternalError &e) {
        const ErrorMessage errmsg = ErrorMessage::fromInternalError(e, nullptr, file.spath(), "Bailing out from analysis: Checking file failed");
        reportErr(errmsg);
    }

    if (!mSettings.buildDir.empty()) {
        mAnalyzerInformation.close();
    }

    // In jointSuppressionReport mode, unmatched suppressions are
    // collected after all files are processed
    if (!mSettings.useSingleJob() && (mSettings.severity.isEnabled(Severity::information) || mSettings.checkConfiguration)) {
        SuppressionList::reportUnmatchedSuppressions(mSettings.supprs.nomsg.getUnmatchedLocalSuppressions(file, (bool)mUnusedFunctionsCheck), *this);
    }

    mErrorList.clear();

    if (mSettings.showtime == SHOWTIME_MODES::SHOWTIME_FILE || mSettings.showtime == SHOWTIME_MODES::SHOWTIME_TOP5_FILE)
        printTimerResults(mSettings.showtime);

    return mExitCode;
}

// TODO: replace with ErrorMessage::fromInternalError()
void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fullmsg("Bailing out from analysis: " + msg);

    ErrorMessage::FileLocation loc1(filename, 0, 0);

    ErrorMessage errmsg({std::move(loc1)},
                        emptyString,
                        Severity::error,
                        fullmsg,
                        "internalError",
                        Certainty::normal);

    mErrorLogger.reportErr(errmsg);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a normal token list
//---------------------------------------------------------------------------

void CppCheck::checkNormalTokens(const Tokenizer &tokenizer)
{
    CheckUnusedFunctions unusedFunctionsChecker;

    // TODO: this should actually be the behavior if only "--enable=unusedFunction" is specified - see #10648
    const char* unusedFunctionOnly = std::getenv("UNUSEDFUNCTION_ONLY");
    const bool doUnusedFunctionOnly = unusedFunctionOnly && (std::strcmp(unusedFunctionOnly, "1") == 0);

    if (!doUnusedFunctionOnly) {
        const std::time_t maxTime = mSettings.checksMaxTime > 0 ? std::time(nullptr) + mSettings.checksMaxTime : 0;

        // call all "runChecks" in all registered Check classes
        // cppcheck-suppress shadowFunction - TODO: fix this
        for (Check *check : Check::instances()) {
            if (Settings::terminated())
                return;

            if (maxTime > 0 && std::time(nullptr) > maxTime) {
                if (mSettings.debugwarnings) {
                    ErrorMessage::FileLocation loc(tokenizer.list.getFiles()[0], 0, 0);
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

            Timer::run(check->name() + "::runChecks", mSettings.showtime, &s_timerResults, [&]() {
                check->runChecks(tokenizer, this);
            });
        }
    }

    if (mSettings.checks.isEnabled(Checks::unusedFunction) && !mSettings.buildDir.empty()) {
        unusedFunctionsChecker.parseTokens(tokenizer, mSettings);
    }
    if (mUnusedFunctionsCheck && mSettings.useSingleJob() && mSettings.buildDir.empty()) {
        mUnusedFunctionsCheck->parseTokens(tokenizer, mSettings);
    }

    if (mSettings.clang) {
        // TODO: Use CTU for Clang analysis
        return;
    }

    if (mSettings.useSingleJob() || !mSettings.buildDir.empty()) {
        // Analyse the tokens..

        if (CTU::FileInfo * const fi1 = CTU::getFileInfo(tokenizer)) {
            if (!mSettings.buildDir.empty())
                mAnalyzerInformation.setFileInfo("ctu", fi1->toString());
            if (mSettings.useSingleJob())
                mFileInfo.push_back(fi1);
            else
                delete fi1;
        }

        if (!doUnusedFunctionOnly) {
            // cppcheck-suppress shadowFunction - TODO: fix this
            for (const Check *check : Check::instances()) {
                if (Check::FileInfo * const fi = check->getFileInfo(tokenizer, mSettings)) {
                    if (!mSettings.buildDir.empty())
                        mAnalyzerInformation.setFileInfo(check->name(), fi->toString());
                    if (mSettings.useSingleJob())
                        mFileInfo.push_back(fi);
                    else
                        delete fi;
                }
            }
        }
    }

    if (mSettings.checks.isEnabled(Checks::unusedFunction) && !mSettings.buildDir.empty()) {
        mAnalyzerInformation.setFileInfo("CheckUnusedFunctions", unusedFunctionsChecker.analyzerInfo());
    }

#ifdef HAVE_RULES
    executeRules("normal", tokenizer.list);
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

void CppCheck::executeRules(const std::string &tokenlist, const TokenList &list)
{
    // There is no rule to execute
    if (!hasRule(tokenlist))
        return;

    // Write all tokens in a string that can be parsed by pcre
    std::string str;
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        str += " ";
        str += tok->str();
    }

    for (const Settings::Rule &rule : mSettings.rules) {
        if (rule.tokenlist != tokenlist)
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
            const auto pos1 = (unsigned int)ovector[0];
            const auto pos2 = (unsigned int)ovector[1];

            // jump to the end of the match for the next pcre_exec
            pos = (int)pos2;

            // determine location..
            int fileIndex = 0;
            int line = 0;

            std::size_t len = 0;
            for (const Token *tok = list.front(); tok; tok = tok->next()) {
                len = len + 1U + tok->str().size();
                if (len > pos1) {
                    fileIndex = tok->fileIndex();
                    line = tok->linenr();
                    break;
                }
            }

            const std::string& file = list.getFiles()[fileIndex];

            ErrorMessage::FileLocation loc(file, line, 0);

            // Create error message
            const ErrorMessage errmsg({std::move(loc)},
                                      list.getSourceFilePath(),
                                      rule.severity,
                                      !rule.summary.empty() ? rule.summary : "found '" + str.substr(pos1, pos2 - pos1) + "'",
                                      rule.id,
                                      Certainty::normal);

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

void CppCheck::executeAddons(const std::string& dumpFile, const FileWithDetails& file)
{
    if (!dumpFile.empty()) {
        std::vector<std::string> f{dumpFile};
        executeAddons(f, file.spath());
    }
}

void CppCheck::executeAddons(const std::vector<std::string>& files, const std::string& file0)
{
    if (mSettings.addons.empty() || files.empty())
        return;

    FilesDeleter filesDeleter;

    std::string fileList;

    if (files.size() >= 2 || endsWith(files[0], ".ctu-info")) {
        fileList = Path::getPathFromFilename(files[0]) + FILELIST;
        std::ofstream fout(fileList);
        for (const std::string& f: files)
            fout << f << std::endl;
    }

    // ensure all addons have already been resolved - TODO: remove when settings are const after creation
    assert(mSettings.addonInfos.size() == mSettings.addons.size());

    std::string ctuInfo;

    for (const AddonInfo &addonInfo : mSettings.addonInfos) {
        if (addonInfo.name != "misra" && !addonInfo.ctu && endsWith(files.back(), ".ctu-info"))
            continue;

        const std::vector<picojson::value> results =
            executeAddon(addonInfo, mSettings.addonPython, fileList.empty() ? files[0] : fileList, mSettings.premiumArgs, mExecuteCommand);

        const bool misraC2023 = mSettings.premiumArgs.find("--misra-c-2023") != std::string::npos;

        for (const picojson::value& res : results) {
            // TODO: get rid of copy?
            // this is a copy so we can access missing fields and get a default value
            picojson::object obj = res.get<picojson::object>();

            ErrorMessage errmsg;

            if (obj.count("summary") > 0) {
                if (!mSettings.buildDir.empty()) {
                    ctuInfo += res.serialize() + "\n";
                } else {
                    errmsg.severity = Severity::internal;
                    errmsg.id = "ctuinfo";
                    errmsg.setmsg(res.serialize());
                    reportErr(errmsg);
                }
                continue;
            }

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
                    std::string info = loc["info"].get<std::string>();
                    errmsg.callStack.emplace_back(std::move(fileName), std::move(info), lineNumber, column);
                }
            }

            errmsg.id = obj["addon"].get<std::string>() + "-" + obj["errorId"].get<std::string>();
            if (misraC2023 && startsWith(errmsg.id, "misra-c2012-"))
                errmsg.id = "misra-c2023-" + errmsg.id.substr(12);
            errmsg.setmsg(mSettings.getMisraRuleText(errmsg.id, obj["message"].get<std::string>()));
            const std::string severity = obj["severity"].get<std::string>();
            errmsg.severity = severityFromString(severity);
            if (errmsg.severity == Severity::none || errmsg.severity == Severity::internal) {
                if (!endsWith(errmsg.id, "-logChecker"))
                    continue;
                errmsg.severity = Severity::internal;
            }
            else if (!mSettings.severity.isEnabled(errmsg.severity)) {
                // Do not filter out premium misra/cert/autosar messages that has been
                // explicitly enabled with a --premium option
                if (!isPremiumCodingStandardId(errmsg.id))
                    continue;
            }
            errmsg.file0 = file0;

            reportErr(errmsg);
        }
    }

    if (!mSettings.buildDir.empty() && fileList.empty()) {
        const std::string& ctuInfoFile = getCtuInfoFileName(files[0]);
        std::ofstream fout(ctuInfoFile);
        fout << ctuInfo;
    }
}

void CppCheck::executeAddonsWholeProgram(const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const std::string& ctuInfo)
{
    if (mSettings.addons.empty())
        return;

    if (mSettings.buildDir.empty()) {
        const std::string fileName = std::to_string(mSettings.pid) + ".ctu-info";
        FilesDeleter filesDeleter;
        filesDeleter.addFile(fileName);
        std::ofstream fout(fileName);
        fout << ctuInfo;
        fout.close();
        executeAddons({fileName}, "");
        return;
    }

    std::vector<std::string> ctuInfoFiles;
    for (const auto &f: files) {
        const std::string &dumpFileName = getDumpFileName(mSettings, f.path());
        ctuInfoFiles.push_back(getCtuInfoFileName(dumpFileName));
    }

    for (const auto &f: fileSettings) {
        const std::string &dumpFileName = getDumpFileName(mSettings, f.filename());
        ctuInfoFiles.push_back(getCtuInfoFileName(dumpFileName));
    }

    try {
        executeAddons(ctuInfoFiles, "");
    } catch (const InternalError& e) {
        const ErrorMessage errmsg = ErrorMessage::fromInternalError(e, nullptr, "", "Bailing out from analysis: Whole program analysis failed");
        reportErr(errmsg);
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
        loclist.emplace_back(file, 0, 0);
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


    ErrorMessage errmsg(std::move(loclist),
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
        loclist.emplace_back(file, 0, 0);
    }

    ErrorMessage errmsg(std::move(loclist),
                        emptyString,
                        Severity::information,
                        "The configuration '" + configuration + "' was not checked because its code equals another one.",
                        "purgedConfiguration",
                        Certainty::normal);

    reportErr(errmsg);
}

//---------------------------------------------------------------------------

// TODO: part of this logic is duplicated in Executor::hasToLog()
void CppCheck::reportErr(const ErrorMessage &msg)
{
    if (msg.severity == Severity::internal) {
        mErrorLogger.reportErr(msg);
        return;
    }

    if (!mSettings.library.reportErrors(msg.file0))
        return;

    std::set<std::string> macroNames;
    if (!msg.callStack.empty()) {
        const std::string &file = msg.callStack.back().getfile(false);
        int lineNumber = msg.callStack.back().line;
        const auto it = mLocationMacros.find(Location(file, lineNumber));
        if (it != mLocationMacros.cend())
            macroNames = it->second;
    }

    // TODO: only convert if necessary
    const auto errorMessage = SuppressionList::ErrorMessage::fromErrorMessage(msg, macroNames);

    if (mSettings.supprs.nomsg.isSuppressed(errorMessage, mUseGlobalSuppressions)) {
        // Safety: Report critical errors to ErrorLogger
        if (mSettings.safety && ErrorLogger::isCriticalErrorId(msg.id)) {
            mExitCode = 1;

            if (mSettings.supprs.nomsg.isSuppressedExplicitly(errorMessage, mUseGlobalSuppressions)) {
                // Report with internal severity to signal that there is this critical error but
                // it is suppressed
                ErrorMessage temp(msg);
                temp.severity = Severity::internal;
                mErrorLogger.reportErr(temp);
            } else {
                // Report critical error that is not explicitly suppressed
                mErrorLogger.reportErr(msg);
            }
        }
        return;
    }

    // TODO: there should be no need for the verbose and default messages here
    std::string errmsg = msg.toString(mSettings.verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors.
    // This makes sure the errors of a single check() call are unique.
    // TODO: get rid of this? This is forwarded to another ErrorLogger which is also doing this
    if (!mErrorList.emplace(std::move(errmsg)).second)
        return;

    if (!mSettings.buildDir.empty())
        mAnalyzerInformation.reportErr(msg);

    if (!mSettings.supprs.nofail.isSuppressed(errorMessage) && !mSettings.supprs.nomsg.isSuppressed(errorMessage)) {
        mExitCode = 1;
    }

    std::string remark;
    if (!msg.callStack.empty()) {
        for (const auto& r: mRemarkComments) {
            if (r.file != msg.callStack.back().getfile(false))
                continue;
            if (r.lineNumber != msg.callStack.back().line)
                continue;
            remark = r.str;
            break;
        }
    }

    if (!remark.empty()) {
        ErrorMessage msg2(msg);
        msg2.remark = std::move(remark);
        mErrorLogger.reportErr(msg2);
    } else {
        mErrorLogger.reportErr(msg);
    }

    // check if plistOutput should be populated and the current output file is open and the error is not suppressed
    if (!mSettings.plistOutput.empty() && mPlistFile.is_open() && !mSettings.supprs.nomsg.isSuppressed(errorMessage)) {
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
    s.addEnabled("all");

    CppCheck cppcheck(errorlogger, true, nullptr);
    cppcheck.purgedConfigurationMessage(emptyString,emptyString);
    cppcheck.mTooManyConfigs = true;
    cppcheck.tooManyConfigsError(emptyString,0U);
    // TODO: add functions to get remaining error messages

    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().cbegin(); it != Check::instances().cend(); ++it)
        (*it)->getErrorMessages(&errorlogger, &s);

    CheckUnusedFunctions::getErrorMessages(errorlogger);
    Preprocessor::getErrorMessages(errorlogger, s);
}

void CppCheck::analyseClangTidy(const FileSettings &fileSettings)
{
    std::string allIncludes;
    for (const std::string &inc : fileSettings.includePaths) {
        allIncludes = allIncludes + "-I\"" + inc + "\" ";
    }

    const std::string allDefines = getDefinesFlags(fileSettings.defines);

#ifdef _WIN32
    constexpr char exe[] = "clang-tidy.exe";
#else
    constexpr char exe[] = "clang-tidy";
#endif

    const std::string args = "-quiet -checks=*,-clang-analyzer-*,-llvm* \"" + fileSettings.filename() + "\" -- " + allIncludes + allDefines;
    std::string output;
    if (const int exitcode = mExecuteCommand(exe, split(args), emptyString, output)) {
        std::cerr << "Failed to execute '" << exe << "' (exitcode: " << std::to_string(exitcode) << ")" << std::endl;
        return;
    }

    // parse output and create error messages
    std::istringstream istr(output);
    std::string line;

    if (!mSettings.buildDir.empty()) {
        const std::string analyzerInfoFile = AnalyzerInformation::getAnalyzerInfoFile(mSettings.buildDir, fileSettings.filename(), emptyString);
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
        const auto lineNumber = strToInt<int64_t>(lineNumString);
        const auto column = strToInt<int64_t>(columnNumString);
        fixedpath = Path::toNativeSeparators(std::move(fixedpath));

        ErrorMessage errmsg;
        errmsg.callStack.emplace_back(fixedpath, lineNumber, column);

        errmsg.id = "clang-tidy-" + errorString.substr(1, errorString.length() - 2);
        if (errmsg.id.find("performance") != std::string::npos)
            errmsg.severity = Severity::performance;
        else if (errmsg.id.find("portability") != std::string::npos)
            errmsg.severity = Severity::portability;
        else if (errmsg.id.find("cert") != std::string::npos || errmsg.id.find("misc") != std::string::npos || errmsg.id.find("unused") != std::string::npos)
            errmsg.severity = Severity::warning;
        else
            errmsg.severity = Severity::style;

        errmsg.file0 = std::move(fixedpath);
        errmsg.setmsg(messageString);
        reportErr(errmsg);
    }
}

bool CppCheck::analyseWholeProgram()
{
    bool errors = false;
    // Analyse the tokens
    CTU::FileInfo ctu;
    if (mSettings.useSingleJob() || !mSettings.buildDir.empty())
    {
        for (const Check::FileInfo *fi : mFileInfo) {
            const auto *fi2 = dynamic_cast<const CTU::FileInfo *>(fi);
            if (fi2) {
                ctu.functionCalls.insert(ctu.functionCalls.end(), fi2->functionCalls.cbegin(), fi2->functionCalls.cend());
                ctu.nestedCalls.insert(ctu.nestedCalls.end(), fi2->nestedCalls.cbegin(), fi2->nestedCalls.cend());
            }
        }
    }

    // cppcheck-suppress shadowFunction - TODO: fix this
    for (Check *check : Check::instances())
        errors |= check->analyseWholeProgram(&ctu, mFileInfo, mSettings, *this);  // TODO: ctu

    if (mUnusedFunctionsCheck)
        errors |= mUnusedFunctionsCheck->check(mSettings, *this);

    return errors && (mExitCode > 0);
}

unsigned int CppCheck::analyseWholeProgram(const std::string &buildDir, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, const std::string& ctuInfo)
{
    executeAddonsWholeProgram(files, fileSettings, ctuInfo);
    if (mSettings.checks.isEnabled(Checks::unusedFunction))
        CheckUnusedFunctions::analyseWholeProgram(mSettings, *this, buildDir);
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
            // cppcheck-suppress shadowFunction - TODO: fix this
            for (const Check *check : Check::instances()) {
                if (checkClassAttr == check->name())
                    fileInfoList.push_back(check->loadFileInfoFromXml(e));
            }
        }
    }

    // Analyse the tokens
    // cppcheck-suppress shadowFunction - TODO: fix this
    for (Check *check : Check::instances())
        check->analyseWholeProgram(&ctuFileInfo, fileInfoList, mSettings, *this);

    if (mUnusedFunctionsCheck)
        mUnusedFunctionsCheck->check(mSettings, *this);

    for (Check::FileInfo *fi : fileInfoList)
        delete fi;

    return mExitCode;
}

// cppcheck-suppress unusedFunction - only used in tests
void CppCheck::resetTimerResults()
{
    s_timerResults.reset();
}

void CppCheck::printTimerResults(SHOWTIME_MODES mode)
{
    s_timerResults.showResults(mode);
}

bool CppCheck::isPremiumCodingStandardId(const std::string& id) const {
    if (mSettings.premiumArgs.find("--misra") != std::string::npos) {
        if (startsWith(id, "misra-") || startsWith(id, "premium-misra-"))
            return true;
    }
    if (mSettings.premiumArgs.find("--cert") != std::string::npos && startsWith(id, "premium-cert-"))
        return true;
    if (mSettings.premiumArgs.find("--autosar") != std::string::npos && startsWith(id, "premium-autosar-"))
        return true;
    return false;
}

std::string CppCheck::getDumpFileContentsRawTokens(const std::vector<std::string>& files, const simplecpp::TokenList& tokens1) const {
    std::string dumpProlog;
    dumpProlog += "  <rawtokens>\n";
    for (unsigned int i = 0; i < files.size(); ++i) {
        dumpProlog += "    <file index=\"";
        dumpProlog += std::to_string(i);
        dumpProlog += "\" name=\"";
        dumpProlog += ErrorLogger::toxml(Path::getRelativePath(files[i], mSettings.basePaths));
        dumpProlog += "\"/>\n";
    }
    for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
        dumpProlog += "    <tok ";

        dumpProlog += "fileIndex=\"";
        dumpProlog += std::to_string(tok->location.fileIndex);
        dumpProlog += "\" ";

        dumpProlog += "linenr=\"";
        dumpProlog += std::to_string(tok->location.line);
        dumpProlog += "\" ";

        dumpProlog +="column=\"";
        dumpProlog += std::to_string(tok->location.col);
        dumpProlog += "\" ";

        dumpProlog += "str=\"";
        dumpProlog += ErrorLogger::toxml(tok->str());
        dumpProlog += "\"";

        dumpProlog += "/>\n";
    }
    dumpProlog += "  </rawtokens>\n";
    return dumpProlog;
}
