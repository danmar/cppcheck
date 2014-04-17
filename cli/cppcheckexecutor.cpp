/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#define USE_UNIX_SIGNAL_HANDLING
#include <execinfo.h>
#include <cxxabi.h>
#endif

#ifdef USE_UNIX_SIGNAL_HANDLING
#include <signal.h>
#include <cstdio>
#endif

#if defined(_MSC_VER)
#define USE_WINDOWS_SEH
#include <Windows.h>
#include <excpt.h>
#endif

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
                // If the include path is not found, warn user (unless --quiet
                // was used) and remove the non-existing path from the list.
                if (!settings._errorsOnly)
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

    if (!parseFromArgs(&cppCheck, argc, argv)) {
        return EXIT_FAILURE;
    }

    if (cppCheck.settings().exceptionHandling) {
        return check_wrapper(cppCheck, argc, argv);
    } else {
        return check_internal(cppCheck, argc, argv);
    }
}

#if defined(USE_UNIX_SIGNAL_HANDLING)
/* (declare this list here, so it may be used in signal handlers in addition to main())
 * A list of signals available in ISO C
 * Check out http://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
 * For now we only want to detect abnormal behaviour for a few selected signals:
 */
struct Signaltype {
    int signalnumber;
    const char *signalname;
};
#define DECLARE_SIGNAL(x) {x, #x}
static const Signaltype listofsignals[] = {
    // don't care: SIGABRT,
    DECLARE_SIGNAL(SIGFPE),
    DECLARE_SIGNAL(SIGILL),
    DECLARE_SIGNAL(SIGINT),
    DECLARE_SIGNAL(SIGSEGV),
    // don't care: SIGTERM
};

/**
 *  Simple helper function:
 * \return size of array
 * */
template<typename T, int size>
size_t GetArrayLength(const T(&)[size])
{
    return size;
}

// 32 vs. 64bit
#define ADDRESSDISPLAYLENGTH ((sizeof(long)==8)?12:8)

/*
 * Try to print the callstack.
 * That is very sensitive to the operating system, hardware, compiler and runtime!
 */
static void print_stacktrace(FILE* f, bool demangling)
{
#if defined(__GNUC__)
    void *array[32]= {0}; // the less resources the better...
    const int depth = backtrace(array, (int)GetArrayLength(array));
    char **symbolstrings = backtrace_symbols(array, depth);
    if (symbolstrings) {
        fputs("Callstack:\n", f);
        const int offset=3; // the first two entries are simply within our own exception handling code, third is within libc
        for (int i = offset; i < depth; ++i) {
            const char * const symbol = symbolstrings[i];
            //fprintf(f, "\"%s\"\n", symbol);
            char * realname = nullptr;
            const char * const firstBracketName = strchr(symbol, '(');
            const char * const firstBracketAddress = strchr(symbol, '[');
            const char * const secondBracketAddress = strchr(firstBracketAddress, ']');
            const char * const beginAddress = firstBracketAddress+3;
            const int addressLen = int(secondBracketAddress-beginAddress);
            const int padLen = int(ADDRESSDISPLAYLENGTH-addressLen);
            //fprintf(f, "AddressDisplayLength=%d    addressLen=%d      padLen=%d\n", ADDRESSDISPLAYLENGTH, addressLen, padLen);
            if (demangling && firstBracketName) {
                const char * const plus = strchr(firstBracketName, '+');
                if (plus && (plus>(firstBracketName+1))) {
                    char input_buffer[512]= {0};
                    strncpy(input_buffer, firstBracketName+1, plus-firstBracketName-1);
                    char output_buffer[1024]= {0};
                    size_t length = GetArrayLength(output_buffer);
                    int status=0;
                    realname = abi::__cxa_demangle(input_buffer, output_buffer, &length, &status); // non-NULL on success
                }
            }
            fprintf(f, "#%d 0x",
                    i-offset);
            if (padLen>0)
                fprintf(f, "%0*d",
                        padLen, 0);
            if (realname) {
                fprintf(f, "%.*s in %s\n",
                        (int)(secondBracketAddress-firstBracketAddress-3), firstBracketAddress+3,
                        realname);
            } else {
                fprintf(f, "%.*s in %.*s\n",
                        (int)(secondBracketAddress-firstBracketAddress-3), firstBracketAddress+3,
                        (int)(firstBracketAddress-symbol), symbol);
            }
        }
        free(symbolstrings);
    } else {
        fputs("Callstack could not be obtained\n", f);
    }
#endif
}

/*
 * Simple mapping
 */
static const char *signal_name(int signo)
{
    for (size_t s=0; s<GetArrayLength(listofsignals); ++s) {
        if (listofsignals[s].signalnumber==signo)
            return listofsignals[s].signalname;
    }
    return "";
}

/*
 * Entry pointer for signal handlers
 */
static void CppcheckSignalHandler(int signo, siginfo_t * info, void * /*context*/)
{
    const char * const signame=signal_name(signo);
    bool bPrintCallstack=true;
    FILE* f=stderr;
    switch (signo) {
    case SIGILL:
        fprintf(f, "Internal error (caught signal %d=%s at 0x%p)\n",
                signo, signame, info->si_addr);
        break;
    case SIGFPE:
        fprintf(f, "Internal error (caught signal %d=%s at 0x%p)\n",
                signo, signame, info->si_addr);
        break;
    case SIGSEGV:
        fprintf(f, "Internal error (caught signal %d=%s at 0x%p)\n",
                signo, signame, info->si_addr);
        break;
        /*
         case SIGBUS:
            fprintf(f, "Internal error (caught signal %d=%s at 0x%p)\n",
                     signo, signame, info->si_addr);
           break;
         case SIGTRAP:
           fprintf(f, "Internal error (caught signal %d=%s at 0x%p)\n",
                    signo, signame, info->si_addr);
           break;
        */
    case SIGINT:
        bPrintCallstack=false;
        break;
    default:
        fprintf(f, "Internal error (caught signal %d)\n",
                signo);
        break;
    }
    if (bPrintCallstack) {
        print_stacktrace(f, true);
        fputs("\nPlease report this to the cppcheck developers!\n", f);
    }
    abort();
}
#endif

#ifdef USE_WINDOWS_SEH
/*
 * Any evaluation of the information about the exception needs to be done here!
 */
static int filterException(int code, PEXCEPTION_POINTERS ex)
{
    // TODO we should try to extract more information here
    //   - address, read/write
    switch (ex->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        fprintf(stderr, "Internal error (EXCEPTION_ACCESS_VIOLATION)\n");
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        fprintf(stderr, "Internal error (EXCEPTION_IN_PAGE_ERROR)\n");
        break;
    default:
        fprintf(stderr, "Internal error (%d)\n",
                code);
        break;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/**
 * Signal/SEH handling
 * Has to be clean for using with SEH on windows, i.e. no construction of C++ object instances is allowed!
 * TODO Check for multi-threading issues!
 *
 */
int CppCheckExecutor::check_wrapper(CppCheck& cppcheck, int argc, const char* const argv[])
{
#ifdef USE_WINDOWS_SEH
    __try {
        return check_internal(cppcheck, argc, argv);
    } __except (filterException(GetExceptionCode(), GetExceptionInformation())) {
        // reporting to stdout may not be helpful within a GUI application..
        fprintf(stderr, "Please report this to the cppcheck developers!\n");
        return -1;
    }
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=CppcheckSignalHandler;
    for (std::size_t s=0; s<GetArrayLength(listofsignals); ++s) {
        sigaction(listofsignals[s].signalnumber, &act, NULL);
    }
    return check_internal(cppcheck, argc, argv);
#else
    return check_internal(cppcheck, argc, argv);
#endif
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(CppCheck& cppcheck, int /*argc*/, const char* const argv[])
{
    Settings& settings = cppcheck.settings();
    _settings = &settings;
    bool std = settings.library.load(argv[0], "std.cfg");
    bool posix = true;
    if (settings.standards.posix)
        posix = settings.library.load(argv[0], "posix.cfg");

    if (!std || !posix) {
        const std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        const std::string msg("Failed to load " + std::string(!std ? "std.cfg" : "posix.cfg") + ". Your Cppcheck installation is broken, please re-install.");
#ifdef CFGDIR
        const std::string details("The Cppcheck binary was compiled with CFGDIR set to \"" +
                                  std::string(CFGDIR) + "\" and will therefore search for "
                                  "std.cfg in that path.");
#else
        const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(argv[0])) + "cfg");
        const std::string details("The Cppcheck binary was compiled without CFGDIR set. Either the "
                                  "std.cfg should be available in " + cfgfolder + " or the CFGDIR "
                                  "should be configured.");
#endif
        ErrorLogger::ErrorMessage errmsg(callstack, Severity::information, msg+" "+details, "failedToLoadCfg", false);
        reportErr(errmsg);
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
            if (!_settings->library.markupFile(i->first)
                || !_settings->library.processMarkupAfterCode(i->first)) {
                returnValue += cppcheck.check(i->first);
                processedsize += i->second;
                if (!settings._errorsOnly)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }

        // second loop to parse all markup files which may not work until all
        // c/cpp files have been parsed and checked
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
            if (_settings->library.markupFile(i->first) && _settings->library.processMarkupAfterCode(i->first)) {
                returnValue += cppcheck.check(i->first);
                processedsize += i->second;
                if (!settings._errorsOnly)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }

        cppcheck.checkFunctionUsage();
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
        cppcheck.tooManyConfigsError("",0U);

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
