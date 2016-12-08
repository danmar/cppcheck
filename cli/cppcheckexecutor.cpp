/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "analyzerinfo.h"
#include "cmdlineparser.h"
#include "cppcheck.h"
#include "filelister.h"
#include "path.h"
#include "pathmatch.h"
#include "preprocessor.h"
#include "threadexecutor.h"
#include "utils.h"

#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sstream>

#if !defined(NO_UNIX_SIGNAL_HANDLING) && defined(__GNUC__) && !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(__OS2__)
#define USE_UNIX_SIGNAL_HANDLING
#include <cstdio>
#include <signal.h>
#include <unistd.h>
#if defined(__APPLE__)
#   define _XOPEN_SOURCE // ucontext.h APIs can only be used on Mac OSX >= 10.7 if _XOPEN_SOURCE is defined
#   include <ucontext.h>
#   undef _XOPEN_SOURCE
#elif !defined(__OpenBSD__)
#   include <ucontext.h>
#endif
#ifdef __linux__
#include <sys/syscall.h>
#include <sys/types.h>
#endif
#endif

#if !defined(NO_UNIX_BACKTRACE_SUPPORT) && defined(USE_UNIX_SIGNAL_HANDLING) && defined(__GNUC__) && defined(__GLIBC__) && !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(__NetBSD__) && !defined(__SVR4) && !defined(__QNX__)
#define USE_UNIX_BACKTRACE_SUPPORT
#include <cxxabi.h>
#include <execinfo.h>
#endif

#if defined(_MSC_VER)
#define USE_WINDOWS_SEH
#include <Windows.h>
#include <DbgHelp.h>
#include <excpt.h>
#include <TCHAR.H>
#endif


/*static*/ FILE* CppCheckExecutor::exceptionOutput = stdout;

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
    const bool success = parser.ParseFromArgs(argc, argv);

    if (success) {
        if (parser.GetShowVersion() && !parser.GetShowErrorMessages()) {
            const char * const extraVersion = cppcheck->extraVersion();
            if (*extraVersion != 0)
                std::cout << "Cppcheck " << cppcheck->version() << " ("
                          << extraVersion << ')' << std::endl;
            else
                std::cout << "Cppcheck " << cppcheck->version() << std::endl;
        }

        if (parser.GetShowErrorMessages()) {
            errorlist = true;
            std::cout << ErrorLogger::ErrorMessage::getXMLHeader(settings.xml_version);
            cppcheck->getErrorMessages();
            std::cout << ErrorLogger::ErrorMessage::getXMLFooter(settings.xml_version) << std::endl;
        }

        if (parser.ExitAfterPrinting()) {
            settings.terminate();
            return true;
        }
    } else {
        return false;
    }

    // Check that all include paths exist
    {
        std::list<std::string>::iterator iter;
        for (iter = settings.includePaths.begin();
             iter != settings.includePaths.end();
            ) {
            const std::string path(Path::toNativeSeparators(*iter));
            if (FileLister::isDirectory(path))
                ++iter;
            else {
                // If the include path is not found, warn user and remove the non-existing path from the list.
                if (settings.isEnabled("information"))
                    std::cout << "(information) Couldn't find path given by -I '" << path << '\'' << std::endl;
                iter = settings.includePaths.erase(iter);
            }
        }
    }

    // Output a warning for the user if he tries to exclude headers
    bool warn = false;
    const std::vector<std::string>& ignored = parser.GetIgnoredPaths();
    for (std::vector<std::string>::const_iterator i = ignored.cbegin(); i != ignored.cend(); ++i) {
        if (Path::isHeader(*i)) {
            warn = true;
            break;
        }
    }
    if (warn) {
        std::cout << "cppcheck: filename exclusion does not apply to header (.h and .hpp) files." << std::endl;
        std::cout << "cppcheck: Please use --suppress for ignoring results from the header files." << std::endl;
    }

    const std::vector<std::string>& pathnames = parser.GetPathNames();

#if defined(_WIN32)
    // For Windows we want case-insensitive path matching
    const bool caseSensitive = false;
#else
    const bool caseSensitive = true;
#endif
    if (!pathnames.empty()) {
        // Execute recursiveAddFiles() to each given file parameter
        PathMatch matcher(ignored, caseSensitive);
        for (std::vector<std::string>::const_iterator iter = pathnames.begin(); iter != pathnames.end(); ++iter)
            FileLister::recursiveAddFiles(_files, Path::toNativeSeparators(*iter), _settings->library.markupExtensions(), matcher);
    }

    if (_files.empty() && settings.project.fileSettings.empty()) {
        std::cout << "cppcheck: error: could not find or open any of the paths given." << std::endl;
        if (!ignored.empty())
            std::cout << "cppcheck: Maybe all paths were ignored?" << std::endl;
        return false;
    }
    return true;
}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    Preprocessor::missingIncludeFlag = false;
    Preprocessor::missingSystemIncludeFlag = false;

    CppCheck cppCheck(*this, true);

    const Settings& settings = cppCheck.settings();
    _settings = &settings;

    if (!parseFromArgs(&cppCheck, argc, argv)) {
        return EXIT_FAILURE;
    }
    if (settings.terminated()) {
        return EXIT_SUCCESS;
    }
    if (cppCheck.settings().exceptionHandling) {
        return check_wrapper(cppCheck, argc, argv);
    } else {
        return check_internal(cppCheck, argc, argv);
    }
}

/**
 *  Simple helper function:
 * \return size of array
 * */
template<typename T, int size>
std::size_t GetArrayLength(const T(&)[size])
{
    return size;
}


#if defined(USE_UNIX_SIGNAL_HANDLING)
/*
 * Try to print the callstack.
 * That is very sensitive to the operating system, hardware, compiler and runtime!
 * The code is not meant for production environment, it's using functions not whitelisted for usage in a signal handler function.
 */
static void print_stacktrace(FILE* output, bool demangling, int maxdepth, bool lowMem)
{
#if defined(USE_UNIX_BACKTRACE_SUPPORT)
// 32 vs. 64bit
#define ADDRESSDISPLAYLENGTH ((sizeof(long)==8)?12:8)
    const int fd = fileno(output);
    void *array[32]= {0}; // the less resources the better...
    const int currentdepth = backtrace(array, (int)GetArrayLength(array));
    const int offset=2; // some entries on top are within our own exception handling code or libc
    if (maxdepth<0)
        maxdepth=currentdepth-offset;
    else
        maxdepth = std::min(maxdepth, currentdepth);
    if (lowMem) {
        fputs("Callstack (symbols only):\n", output);
        backtrace_symbols_fd(array+offset, maxdepth, fd);
        fflush(output);
    } else {
        char **symbolstrings = backtrace_symbols(array, currentdepth);
        if (symbolstrings) {
            fputs("Callstack:\n", output);
            for (int i = offset; i < maxdepth; ++i) {
                const char * const symbol = symbolstrings[i];
                char * realname = nullptr;
                const char * const firstBracketName     = strchr(symbol, '(');
                const char * const firstBracketAddress  = strchr(symbol, '[');
                const char * const secondBracketAddress = strchr(firstBracketAddress, ']');
                const char * const beginAddress         = firstBracketAddress+3;
                const int addressLen = int(secondBracketAddress-beginAddress);
                const int padLen     = int(ADDRESSDISPLAYLENGTH-addressLen);
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
                const int ordinal=i-offset;
                fprintf(output, "#%-2d 0x",
                        ordinal);
                if (padLen>0)
                    fprintf(output, "%0*d",
                            padLen, 0);
                if (realname) {
                    fprintf(output, "%.*s in %s\n",
                            (int)(secondBracketAddress-firstBracketAddress-3), firstBracketAddress+3,
                            realname);
                } else {
                    fprintf(output, "%.*s in %.*s\n",
                            (int)(secondBracketAddress-firstBracketAddress-3), firstBracketAddress+3,
                            (int)(firstBracketAddress-symbol), symbol);
                }
            }
            free(symbolstrings);
        } else {
            fputs("Callstack could not be obtained\n", output);
        }
    }
#undef ADDRESSDISPLAYLENGTH
#endif
}

static const size_t MYSTACKSIZE = 16*1024+SIGSTKSZ; // wild guess about a reasonable buffer
static char mytstack[MYSTACKSIZE]= {0}; // alternative stack for signal handler
static bool bStackBelowHeap=false; // lame attempt to locate heap vs. stack address space. See CppCheckExecutor::check_wrapper()

/*
 * \param[in] ptr address to be examined.
 * \return true if address is supposed to be on stack (contrary to heap). If ptr is 0 false will be returned.
 * If unknown better return false.
 */
static bool IsAddressOnStack(const void* ptr)
{
    if (nullptr==ptr)
        return false;
    char a;
    if (bStackBelowHeap)
        return ptr < &a;
    else
        return ptr > &a;
}

/* (declare this list here, so it may be used in signal handlers in addition to main())
 * A list of signals available in ISO C
 * Check out http://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
 * For now we only want to detect abnormal behaviour for a few selected signals:
 */

#define DECLARE_SIGNAL(x) << std::make_pair(x, #x)
typedef std::map<int, std::string> Signalmap_t;
static const Signalmap_t listofsignals = make_container< Signalmap_t > ()
        DECLARE_SIGNAL(SIGABRT)
        DECLARE_SIGNAL(SIGBUS)
        DECLARE_SIGNAL(SIGFPE)
        DECLARE_SIGNAL(SIGILL)
        DECLARE_SIGNAL(SIGINT)
        DECLARE_SIGNAL(SIGQUIT)
        DECLARE_SIGNAL(SIGSEGV)
        DECLARE_SIGNAL(SIGSYS)
        // don't care: SIGTERM
        DECLARE_SIGNAL(SIGUSR1)
        DECLARE_SIGNAL(SIGUSR2)
        ;
#undef DECLARE_SIGNAL
/*
 * Entry pointer for signal handlers
 * It uses functions which are not safe to be called from a signal handler,
 * (http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_04 has a whitelist)
 * but when ending up here something went terribly wrong anyway.
 * And all which is left is just printing some information and terminate.
 */
static void CppcheckSignalHandler(int signo, siginfo_t * info, void * context)
{
    int type = -1;
    pid_t killid = getpid();
#if defined(__linux__) && defined(REG_ERR)
    const ucontext_t* const uc = reinterpret_cast<const ucontext_t*>(context);
    killid = (pid_t) syscall(SYS_gettid);
    if (uc) {
        type = (int)uc->uc_mcontext.gregs[REG_ERR] & 2;
    }
#endif
    const Signalmap_t::const_iterator it=listofsignals.find(signo);
    const char * const signame = (it==listofsignals.end()) ? "unknown" : it->second.c_str();
    bool printCallstack=true;
    bool lowMem=false;
    bool unexpectedSignal=true;
    const bool isaddressonstack = IsAddressOnStack(info->si_addr);
    FILE* output = CppCheckExecutor::getExceptionOutput();
    switch (signo) {
    case SIGABRT:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        fputs(" - out of memory?\n", output);
        lowMem=true; // educated guess
        break;
    case SIGBUS:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case BUS_ADRALN: // invalid address alignment
            fputs(" - BUS_ADRALN", output);
            break;
        case BUS_ADRERR: // nonexistent physical address
            fputs(" - BUS_ADRERR", output);
            break;
        case BUS_OBJERR: // object-specific hardware error
            fputs(" - BUS_OBJERR", output);
            break;
#ifdef BUS_MCEERR_AR
        case BUS_MCEERR_AR: // Hardware memory error consumed on a machine check;
            fputs(" - BUS_MCEERR_AR", output);
            break;
#endif
#ifdef BUS_MCEERR_AO
        case BUS_MCEERR_AO: // Hardware memory error detected in process but not consumed
            fputs(" - BUS_MCEERR_AO", output);
            break;
#endif
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).\n",
                (unsigned long)info->si_addr);
        break;
    case SIGFPE:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case FPE_INTDIV: //     integer divide by zero
            fputs(" - FPE_INTDIV", output);
            break;
        case FPE_INTOVF: //     integer overflow
            fputs(" - FPE_INTOVF", output);
            break;
        case FPE_FLTDIV: //     floating-point divide by zero
            fputs(" - FPE_FLTDIV", output);
            break;
        case FPE_FLTOVF: //     floating-point overflow
            fputs(" - FPE_FLTOVF", output);
            break;
        case FPE_FLTUND: //     floating-point underflow
            fputs(" - FPE_FLTUND", output);
            break;
        case FPE_FLTRES: //     floating-point inexact result
            fputs(" - FPE_FLTRES", output);
            break;
        case FPE_FLTINV: //     floating-point invalid operation
            fputs(" - FPE_FLTINV", output);
            break;
        case FPE_FLTSUB: //     subscript out of range
            fputs(" - FPE_FLTSUB", output);
            break;
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).\n",
                (unsigned long)info->si_addr);
        break;
    case SIGILL:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case ILL_ILLOPC: //     illegal opcode
            fputs(" - ILL_ILLOPC", output);
            break;
        case ILL_ILLOPN: //    illegal operand
            fputs(" - ILL_ILLOPN", output);
            break;
        case ILL_ILLADR: //    illegal addressing mode
            fputs(" - ILL_ILLADR", output);
            break;
        case ILL_ILLTRP: //    illegal trap
            fputs(" - ILL_ILLTRP", output);
            break;
        case ILL_PRVOPC: //    privileged opcode
            fputs(" - ILL_PRVOPC", output);
            break;
        case ILL_PRVREG: //    privileged register
            fputs(" - ILL_PRVREG", output);
            break;
        case ILL_COPROC: //    coprocessor error
            fputs(" - ILL_COPROC", output);
            break;
        case ILL_BADSTK: //    internal stack error
            fputs(" - ILL_BADSTK", output);
            break;
        default:
            break;
        }
        fprintf(output, " (at 0x%lx).%s\n",
                (unsigned long)info->si_addr,
                (isaddressonstack)?" Stackoverflow?":"");
        break;
    case SIGINT:
        unexpectedSignal=false; // legal usage: interrupt application via CTRL-C
        fputs("cppcheck received signal ", output);
        fputs(signame, output);
        printCallstack=true;
        fputs(".\n", output);
        break;
    case SIGSEGV:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        switch (info->si_code) {
        case SEGV_MAPERR: //    address not mapped to object
            fputs(" - SEGV_MAPERR", output);
            break;
        case SEGV_ACCERR: //    invalid permissions for mapped object
            fputs(" - SEGV_ACCERR", output);
            break;
        default:
            break;
        }
        fprintf(output, " (%sat 0x%lx).%s\n",
                (type==-1)? "" :
                (type==0) ? "reading " : "writing ",
                (unsigned long)info->si_addr,
                (isaddressonstack)?" Stackoverflow?":""
               );
        break;
    case SIGUSR1:
    case SIGUSR2:
        fputs("cppcheck received signal ", output);
        fputs(signame, output);
        fputs(".\n", output);
        break;
    default:
        fputs("Internal error: cppcheck received signal ", output);
        fputs(signame, output);
        fputs(".\n", output);
        break;
    }
    if (printCallstack) {
        print_stacktrace(output, true, -1, lowMem);
    }
    if (unexpectedSignal) {
        fputs("\nPlease report this to the cppcheck developers!\n", output);
    }
    fflush(output);

    // now let things proceed, shutdown and hopefully dump core for post-mortem analysis
    signal(signo, SIG_DFL);
    kill(killid, signo);
}
#endif

#ifdef USE_WINDOWS_SEH
namespace {
    const ULONG maxnamelength = 512;
    struct IMAGEHLP_SYMBOL64_EXT : public IMAGEHLP_SYMBOL64 {
        TCHAR NameExt[maxnamelength]; // actually no need to worry about character encoding here
    };
    typedef BOOL (WINAPI *fpStackWalk64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
    fpStackWalk64 pStackWalk64;
    typedef DWORD64(WINAPI *fpSymGetModuleBase64)(HANDLE, DWORD64);
    fpSymGetModuleBase64 pSymGetModuleBase64;
    typedef BOOL (WINAPI *fpSymGetSymFromAddr64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
    fpSymGetSymFromAddr64 pSymGetSymFromAddr64;
    typedef BOOL (WINAPI *fpSymGetLineFromAddr64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);
    fpSymGetLineFromAddr64 pSymGetLineFromAddr64;
    typedef DWORD (WINAPI *fpUnDecorateSymbolName)(const TCHAR*, PTSTR, DWORD, DWORD) ;
    fpUnDecorateSymbolName pUnDecorateSymbolName;
    typedef PVOID(WINAPI *fpSymFunctionTableAccess64)(HANDLE, DWORD64);
    fpSymFunctionTableAccess64 pSymFunctionTableAccess64;
    typedef BOOL (WINAPI *fpSymInitialize)(HANDLE, PCSTR, BOOL);
    fpSymInitialize pSymInitialize;

    HMODULE hLibDbgHelp;
// avoid explicit dependency on Dbghelp.dll
    bool loadDbgHelp()
    {
        hLibDbgHelp = ::LoadLibraryW(L"Dbghelp.dll");
        if (!hLibDbgHelp)
            return false;
        pStackWalk64 = (fpStackWalk64) ::GetProcAddress(hLibDbgHelp, "StackWalk64");
        pSymGetModuleBase64 = (fpSymGetModuleBase64) ::GetProcAddress(hLibDbgHelp, "SymGetModuleBase64");
        pSymGetSymFromAddr64 = (fpSymGetSymFromAddr64) ::GetProcAddress(hLibDbgHelp, "SymGetSymFromAddr64");
        pSymGetLineFromAddr64 = (fpSymGetLineFromAddr64)::GetProcAddress(hLibDbgHelp, "SymGetLineFromAddr64");
        pSymFunctionTableAccess64 = (fpSymFunctionTableAccess64)::GetProcAddress(hLibDbgHelp, "SymFunctionTableAccess64");
        pSymInitialize = (fpSymInitialize) ::GetProcAddress(hLibDbgHelp, "SymInitialize");
        pUnDecorateSymbolName = (fpUnDecorateSymbolName)::GetProcAddress(hLibDbgHelp, "UnDecorateSymbolName");
        return true;
    }


    void PrintCallstack(FILE* outputFile, PEXCEPTION_POINTERS ex)
    {
        if (!loadDbgHelp())
            return;
        const HANDLE hProcess   = GetCurrentProcess();
        const HANDLE hThread    = GetCurrentThread();
        BOOL result = pSymInitialize(
                          hProcess,
                          0,
                          TRUE
                      );
        CONTEXT             context = *(ex->ContextRecord);
        STACKFRAME64        stack= {0};
#ifdef _M_IX86
        stack.AddrPC.Offset    = context.Eip;
        stack.AddrPC.Mode      = AddrModeFlat;
        stack.AddrStack.Offset = context.Esp;
        stack.AddrStack.Mode   = AddrModeFlat;
        stack.AddrFrame.Offset = context.Ebp;
        stack.AddrFrame.Mode   = AddrModeFlat;
#else
        stack.AddrPC.Offset    = context.Rip;
        stack.AddrPC.Mode      = AddrModeFlat;
        stack.AddrStack.Offset = context.Rsp;
        stack.AddrStack.Mode   = AddrModeFlat;
        stack.AddrFrame.Offset = context.Rsp;
        stack.AddrFrame.Mode   = AddrModeFlat;
#endif
        IMAGEHLP_SYMBOL64_EXT symbol;
        symbol.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL64);
        symbol.MaxNameLength = maxnamelength;
        DWORD64 displacement   = 0;
        int beyond_main=-1; // emergency exit, see below
        for (ULONG frame = 0; ; frame++) {
            result = pStackWalk64
                     (
#ifdef _M_IX86
                         IMAGE_FILE_MACHINE_I386,
#else
                         IMAGE_FILE_MACHINE_AMD64,
#endif
                         hProcess,
                         hThread,
                         &stack,
                         &context,
                         nullptr,
                         pSymFunctionTableAccess64,
                         pSymGetModuleBase64,
                         nullptr
                     );
            if (!result)  // official end...
                break;
            pSymGetSymFromAddr64(hProcess, (ULONG64)stack.AddrPC.Offset, &displacement, &symbol);
            TCHAR undname[maxnamelength]= {0};
            pUnDecorateSymbolName((const TCHAR*)symbol.Name, (PTSTR)undname, (DWORD)GetArrayLength(undname), UNDNAME_COMPLETE);
            if (beyond_main>=0)
                ++beyond_main;
            if (_tcscmp(undname, _T("main"))==0)
                beyond_main=0;
            fprintf(outputFile,
                    "%lu. 0x%08I64X in ",
                    frame, (ULONG64)stack.AddrPC.Offset);
            fputs((const char *)undname, outputFile);
            fputc('\n', outputFile);
            if (0==stack.AddrReturn.Offset || beyond_main>2) // StackWalk64() sometimes doesn't reach any end...
                break;
        }

        FreeLibrary(hLibDbgHelp);
        hLibDbgHelp=0;
    }

    void writeMemoryErrorDetails(FILE* outputFile, PEXCEPTION_POINTERS ex, const char* description)
    {
        fputs(description, outputFile);
        fprintf(outputFile, " (instruction: 0x%p) ", ex->ExceptionRecord->ExceptionAddress);
        // Using %p for ULONG_PTR later on, so it must have size identical to size of pointer
        // This is not the universally portable solution but good enough for Win32/64
        C_ASSERT(sizeof(void*) == sizeof(ex->ExceptionRecord->ExceptionInformation[1]));
        switch (ex->ExceptionRecord->ExceptionInformation[0]) {
        case 0:
            fprintf(outputFile, "reading from 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        case 1:
            fprintf(outputFile, "writing to 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        case 8:
            fprintf(outputFile, "data execution prevention at 0x%p",
                    reinterpret_cast<void*>(ex->ExceptionRecord->ExceptionInformation[1]));
            break;
        default:
            break;
        }
    }

    /*
     * Any evaluation of the exception needs to be done here!
     */
    int filterException(int code, PEXCEPTION_POINTERS ex)
    {
        FILE *outputFile = stdout;
        fputs("Internal error: ", outputFile);
        switch (ex->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            writeMemoryErrorDetails(outputFile, ex, "Access violation");
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fputs("Out of array bounds", outputFile);
            break;
        case EXCEPTION_BREAKPOINT:
            fputs("Breakpoint", outputFile);
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            fputs("Misaligned data", outputFile);
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            fputs("Denormalized floating-point value", outputFile);
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            fputs("Floating-point divide-by-zero", outputFile);
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            fputs("Inexact floating-point value", outputFile);
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            fputs("Invalid floating-point operation", outputFile);
            break;
        case EXCEPTION_FLT_OVERFLOW:
            fputs("Floating-point overflow", outputFile);
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            fputs("Floating-point stack overflow", outputFile);
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            fputs("Floating-point underflow", outputFile);
            break;
        case EXCEPTION_GUARD_PAGE:
            fputs("Page-guard access", outputFile);
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            fputs("Illegal instruction", outputFile);
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            writeMemoryErrorDetails(outputFile, ex, "Invalid page access");
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            fputs("Integer divide-by-zero", outputFile);
            break;
        case EXCEPTION_INT_OVERFLOW:
            fputs("Integer overflow", outputFile);
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            fputs("Invalid exception dispatcher", outputFile);
            break;
        case EXCEPTION_INVALID_HANDLE:
            fputs("Invalid handle", outputFile);
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            fputs("Non-continuable exception", outputFile);
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            fputs("Invalid instruction", outputFile);
            break;
        case EXCEPTION_SINGLE_STEP:
            fputs("Single instruction step", outputFile);
            break;
        case EXCEPTION_STACK_OVERFLOW:
            fputs("Stack overflow", outputFile);
            break;
        default:
            fprintf(outputFile, "Unknown exception (%d)\n",
                    code);
            break;
        }
        fputc('\n', outputFile);
        PrintCallstack(outputFile, ex);
        fflush(outputFile);
        return EXCEPTION_EXECUTE_HANDLER;
    }
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
    FILE *outputFile = stdout;
    __try {
        return check_internal(cppcheck, argc, argv);
    } __except (filterException(GetExceptionCode(), GetExceptionInformation())) {
        // reporting to stdout may not be helpful within a GUI application...
        fputs("Please report this to the cppcheck developers!\n", outputFile);
        return -1;
    }
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    // determine stack vs. heap
    char stackVariable;
    char *heapVariable=(char*)malloc(1);
    bStackBelowHeap = &stackVariable < heapVariable;
    free(heapVariable);

    // set up alternative stack for signal handler
    stack_t segv_stack;
    segv_stack.ss_sp = mytstack;
    segv_stack.ss_flags = 0;
    segv_stack.ss_size = MYSTACKSIZE;
    sigaltstack(&segv_stack, nullptr);

    // install signal handler
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags=SA_SIGINFO|SA_ONSTACK;
    act.sa_sigaction=CppcheckSignalHandler;
    for (std::map<int, std::string>::const_iterator sig=listofsignals.begin(); sig!=listofsignals.end(); ++sig) {
        sigaction(sig->first, &act, nullptr);
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
    bool std = tryLoadLibrary(settings.library, argv[0], "std.cfg");
    bool posix = true;
    if (settings.standards.posix)
        posix = tryLoadLibrary(settings.library, argv[0], "posix.cfg");
    bool windows = true;
    if (settings.isWindowsPlatform())
        windows = tryLoadLibrary(settings.library, argv[0], "windows.cfg");

    if (!std || !posix || !windows) {
        const std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        const std::string msg("Failed to load " + std::string(!std ? "std.cfg" : !posix ? "posix.cfg" : "windows.cfg") + ". Your Cppcheck installation is broken, please re-install.");
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
        ErrorLogger::ErrorMessage errmsg(callstack, emptyString, Severity::information, msg+" "+details, "failedToLoadCfg", false);
        reportErr(errmsg);
        return EXIT_FAILURE;
    }

    if (settings.reportProgress)
        time1 = std::time(0);

    if (settings.xml) {
        reportErr(ErrorLogger::ErrorMessage::getXMLHeader(settings.xml_version));
    }

    if (!settings.buildDir.empty()) {
        std::list<std::string> fileNames;
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i)
            fileNames.push_back(i->first);
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.project.fileSettings);
    }

    unsigned int returnValue = 0;
    if (settings.jobs == 1) {
        // Single process
        settings.jointSuppressionReport = true;

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
                if (!settings.quiet)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }

        // filesettings
        c = 0;
        for (std::list<ImportProject::FileSettings>::const_iterator fs = settings.project.fileSettings.begin(); fs != settings.project.fileSettings.end(); ++fs) {
            returnValue += cppcheck.check(*fs);
            ++c;
            if (!settings.quiet)
                reportStatus(c, settings.project.fileSettings.size(), c, settings.project.fileSettings.size());
        }

        // second loop to parse all markup files which may not work until all
        // c/cpp files have been parsed and checked
        for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
            if (_settings->library.markupFile(i->first) && _settings->library.processMarkupAfterCode(i->first)) {
                returnValue += cppcheck.check(i->first);
                processedsize += i->second;
                if (!settings.quiet)
                    reportStatus(c + 1, _files.size(), processedsize, totalfilesize);
                c++;
            }
        }
        cppcheck.analyseWholeProgram();
    } else if (!ThreadExecutor::isEnabled()) {
        std::cout << "No thread support yet implemented for this platform." << std::endl;
    } else {
        // Multiple processes
        ThreadExecutor executor(_files, settings, *this);
        returnValue = executor.check();
    }

    cppcheck.analyseWholeProgram(_settings->buildDir, _files);

    if (settings.isEnabled("information") || settings.checkConfiguration) {
        const bool enableUnusedFunctionCheck = cppcheck.isUnusedFunctionCheckEnabled();

        if (settings.jointSuppressionReport) {
            for (std::map<std::string, std::size_t>::const_iterator i = _files.begin(); i != _files.end(); ++i) {
                reportUnmatchedSuppressions(settings.nomsg.getUnmatchedLocalSuppressions(i->first, enableUnusedFunctionCheck));
            }
        }

        reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions(enableUnusedFunctionCheck));
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError("",0U);

        if (settings.isEnabled("missingInclude") && (Preprocessor::missingIncludeFlag || Preprocessor::missingSystemIncludeFlag)) {
            const std::list<ErrorLogger::ErrorMessage::FileLocation> callStack;
            ErrorLogger::ErrorMessage msg(callStack,
                                          emptyString,
                                          Severity::information,
                                          "Cppcheck cannot find all the include files (use --check-config for details)\n"
                                          "Cppcheck cannot find all the include files. Cppcheck can check the code without the "
                                          "include files found. But the results will probably be more accurate if all the include "
                                          "files are found. Please check your project's include directories and add all of them "
                                          "as include directories for Cppcheck. To see what files Cppcheck cannot find use "
                                          "--check-config.",
                                          Preprocessor::missingIncludeFlag ? "missingInclude" : "missingIncludeSystem",
                                          false);
            reportInfo(msg);
        }
    }

    if (settings.xml) {
        reportErr(ErrorLogger::ErrorMessage::getXMLFooter(settings.xml_version));
    }

    _settings = 0;
    if (returnValue)
        return settings.exitCode;
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
    const std::time_t time2 = std::time(nullptr);
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
        reportOut(msg.toXML(false, _settings->xml_version));
    } else if (_settings->xml) {
        reportErr(msg.toXML(_settings->verbose, _settings->xml_version));
    } else {
        reportErr(msg.toString(_settings->verbose, _settings->outputFormat));
    }
}

void CppCheckExecutor::setExceptionOutput(FILE* exception_output)
{
    exceptionOutput=exception_output;
}

FILE* CppCheckExecutor::getExceptionOutput()
{
    return exceptionOutput;
}

bool CppCheckExecutor::tryLoadLibrary(Library& destination, const char* basepath, const char* filename)
{
    const Library::Error err = destination.load(basepath, filename);

    if (err.errorcode == Library::UNKNOWN_ELEMENT)
        std::cout << "cppcheck: Found unknown elements in configuration file '" << filename << "': " << err.reason << std::endl;
    else if (err.errorcode != Library::OK) {
        std::string errmsg;
        switch (err.errorcode) {
        case Library::OK:
            break;
        case Library::FILE_NOT_FOUND:
            errmsg = "File not found";
            break;
        case Library::BAD_XML:
            errmsg = "Bad XML";
            break;
        case Library::UNKNOWN_ELEMENT:
            errmsg = "Unexpected element";
            break;
        case Library::MISSING_ATTRIBUTE:
            errmsg = "Missing attribute";
            break;
        case Library::BAD_ATTRIBUTE_VALUE:
            errmsg = "Bad attribute value";
            break;
        case Library::UNSUPPORTED_FORMAT:
            errmsg = "File is of unsupported format version";
            break;
        case Library::DUPLICATE_PLATFORM_TYPE:
            errmsg = "Duplicate platform type";
            break;
        case Library::PLATFORM_TYPE_REDEFINED:
            errmsg = "Platform type redefined";
            break;
        }
        if (!err.reason.empty())
            errmsg += " '" + err.reason + "'";
        std::cout << "cppcheck: Failed to load library configuration file '" << filename << "'. " << errmsg << std::endl;
        return false;
    }
    return true;
}
