/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

//---------------------------------------------------------------------------
#include "checkio.h"

#include "tokenize.h"
#include "symboldatabase.h"

#include <cctype>
#include <cstdlib>

//---------------------------------------------------------------------------

// Register CheckIO..
namespace {
    CheckIO instance;
}


//---------------------------------------------------------------------------
//    std::cout << std::cout;
//---------------------------------------------------------------------------
void CheckIO::checkCoutCerrMisusage()
{
    if (_tokenizer->isC())
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "std :: cout|cerr !!.") && tok->next()->astParent() && tok->next()->astParent()->astOperand1() == tok->next()) {
                const Token* tok2 = tok->next();
                while (tok2->astParent() && tok2->astParent()->str() == "<<") {
                    tok2 = tok2->astParent();
                    if (tok2->astOperand2() && Token::Match(tok2->astOperand2()->previous(), "std :: cout|cerr"))
                        coutCerrMisusageError(tok, tok2->astOperand2()->strAt(1));
                }
            }
        }
    }
}

void CheckIO::coutCerrMisusageError(const Token* tok, const std::string& streamName)
{
    reportError(tok, Severity::error, "coutCerrMisusage", "Invalid usage of output stream: '<< std::" + streamName + "'.");
}

//---------------------------------------------------------------------------
// fflush(stdin) <- fflush only applies to output streams in ANSI C
// fread(); fwrite(); <- consecutive read/write statements require repositioning in between
// fopen("","r"); fwrite(); <- write to read-only file (or vice versa)
// fclose(); fread(); <- Use closed file
//---------------------------------------------------------------------------
enum OpenMode { CLOSED, READ_MODE, WRITE_MODE, RW_MODE, UNKNOWN_OM };
static OpenMode getMode(const std::string& str)
{
    if (str.find('+', 1) != std::string::npos)
        return RW_MODE;
    else if (str.find('w') != std::string::npos || str.find('a') != std::string::npos)
        return WRITE_MODE;
    else if (str.find('r') != std::string::npos)
        return READ_MODE;
    return UNKNOWN_OM;
}

struct Filepointer {
    OpenMode mode;
    unsigned int mode_indent;
    enum Operation {NONE, UNIMPORTANT, READ, WRITE, POSITIONING, OPEN, CLOSE, UNKNOWN_OP} lastOperation;
    unsigned int op_indent;
    enum AppendMode { UNKNOWN_AM, APPEND, APPEND_EX };
    AppendMode append_mode;
    explicit Filepointer(OpenMode mode_ = UNKNOWN_OM)
        : mode(mode_), mode_indent(0), lastOperation(NONE), op_indent(0), append_mode(UNKNOWN_AM) {
    }
};

void CheckIO::checkFileUsage()
{
    static const char* _whitelist[] = {
        "clearerr", "feof", "ferror", "fgetpos", "ftell", "setbuf", "setvbuf", "ungetc", "ungetwc"
    };
    static const std::set<std::string> whitelist(_whitelist, _whitelist + sizeof(_whitelist)/sizeof(*_whitelist));
    const bool windows = _settings->isWindowsPlatform();
    const bool printPortability = _settings->isEnabled("portability");
    const bool printWarnings = _settings->isEnabled("warning");

    std::map<unsigned int, Filepointer> filepointers;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t varListSize = symbolDatabase->getVariableListSize();
    for (std::size_t i = 1; i < varListSize; ++i) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->declarationId() || var->isArray() || !Token::simpleMatch(var->typeStartToken(), "FILE *"))
            continue;

        if (var->isLocal()) {
            if (var->nameToken()->strAt(1) == "(") // initialize by calling "ctor"
                filepointers.insert(std::make_pair(var->declarationId(), Filepointer(UNKNOWN_OM)));
            else
                filepointers.insert(std::make_pair(var->declarationId(), Filepointer(CLOSED)));
        } else {
            filepointers.insert(std::make_pair(var->declarationId(), Filepointer(UNKNOWN_OM)));
            // TODO: If all fopen calls we find open the file in the same type, we can set Filepointer::mode
        }
    }

    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t j = 0; j < functions; ++j) {
        const Scope * scope = symbolDatabase->functionScopes[j];
        unsigned int indent = 0;
        for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() == "{")
                indent++;
            else if (tok->str() == "}") {
                indent--;
                for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); ++i) {
                    if (indent < i->second.mode_indent) {
                        i->second.mode_indent = 0;
                        i->second.mode = UNKNOWN_OM;
                    }
                    if (indent < i->second.op_indent) {
                        i->second.op_indent = 0;
                        i->second.lastOperation = Filepointer::UNKNOWN_OP;
                    }
                }
            } else if (tok->str() == "return" || tok->str() == "continue" || tok->str() == "break") { // Reset upon return, continue or break
                for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); ++i) {
                    i->second.mode_indent = 0;
                    i->second.mode = UNKNOWN_OM;
                    i->second.op_indent = 0;
                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                }
            } else if (Token::Match(tok, "%var% =") &&
                       (tok->strAt(2) != "fopen" && tok->strAt(2) != "freopen" && tok->strAt(2) != "tmpfile" &&
                        (windows ? (tok->str() != "_wfopen" && tok->str() != "_wfreopen") : true))) {
                std::map<unsigned int, Filepointer>::iterator i = filepointers.find(tok->varId());
                if (i != filepointers.end()) {
                    i->second.mode = UNKNOWN_OM;
                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                }
            } else if (Token::Match(tok, "%name% (") && tok->previous() && (!tok->previous()->isName() || Token::Match(tok->previous(), "return|throw"))) {
                std::string mode;
                const Token* fileTok = 0;
                Filepointer::Operation operation = Filepointer::NONE;

                if ((tok->str() == "fopen" || tok->str() == "freopen" || tok->str() == "tmpfile" ||
                     (windows && (tok->str() == "_wfopen" || tok->str() == "_wfreopen"))) &&
                    tok->strAt(-1) == "=") {
                    if (tok->str() != "tmpfile") {
                        const Token* modeTok = tok->tokAt(2)->nextArgument();
                        if (modeTok && modeTok->type() == Token::eString)
                            mode = modeTok->strValue();
                    } else
                        mode = "wb+";
                    fileTok = tok->tokAt(-2);
                    operation = Filepointer::OPEN;
                } else if (windows && Token::Match(tok, "fopen_s|freopen_s|_wfopen_s|_wfreopen_s ( & %name%")) {
                    const Token* modeTok = tok->tokAt(2)->nextArgument()->nextArgument();
                    if (modeTok && modeTok->type() == Token::eString)
                        mode = modeTok->strValue();
                    fileTok = tok->tokAt(3);
                    operation = Filepointer::OPEN;
                } else if ((tok->str() == "rewind" || tok->str() == "fseek" || tok->str() == "fsetpos" || tok->str() == "fflush") ||
                           (windows && tok->str() == "_fseeki64")) {
                    if (printPortability && tok->str() == "fflush") {
                        fileTok = tok->tokAt(2);
                        if (fileTok) {
                            if (fileTok->str() == "stdin")
                                fflushOnInputStreamError(tok, fileTok->str());
                            else {
                                Filepointer& f = filepointers[fileTok->varId()];
                                if (f.mode == READ_MODE)
                                    fflushOnInputStreamError(tok, fileTok->str());
                            }
                        }
                    }

                    fileTok = tok->tokAt(2);
                    operation = Filepointer::POSITIONING;
                } else if (tok->str() == "fgetc" || tok->str() == "fgetwc" ||
                           tok->str() == "fgets" || tok->str() == "fgetws" || tok->str() == "fread" ||
                           tok->str() == "fscanf" || tok->str() == "fwscanf" || tok->str() == "getc" ||
                           (windows && (tok->str() == "fscanf_s" || tok->str() == "fwscanf_s"))) {
                    if (tok->str().find("scanf") != std::string::npos)
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::READ;
                } else if (tok->str() == "fputc" || tok->str() == "fputwc" ||
                           tok->str() == "fputs" || tok->str() == "fputws" || tok->str() == "fwrite" ||
                           tok->str() == "fprintf" || tok->str() == "fwprintf" || tok->str() == "putcc" ||
                           (windows && (tok->str() == "fprintf_s" || tok->str() == "fwprintf_s"))) {
                    if (tok->str().find("printf") != std::string::npos)
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::WRITE;
                } else if (tok->str() == "fclose") {
                    fileTok = tok->tokAt(2);
                    operation = Filepointer::CLOSE;
                } else if (whitelist.find(tok->str()) != whitelist.end()) {
                    fileTok = tok->tokAt(2);
                    if ((tok->str() == "ungetc" || tok->str() == "ungetwc") && fileTok)
                        fileTok = fileTok->nextArgument();
                    operation = Filepointer::UNIMPORTANT;
                } else if (!Token::Match(tok, "if|for|while|catch|switch") && _settings->library.functionpure.find(tok->str()) == _settings->library.functionpure.end()) {
                    const Token* const end2 = tok->linkAt(1);
                    if (scope->functionOf && scope->functionOf->isClassOrStruct() && !scope->function->isStatic() && ((tok->strAt(-1) != "::" && tok->strAt(-1) != ".") || tok->strAt(-2) == "this")) {
                        if (!tok->function() || (tok->function()->nestedIn && tok->function()->nestedIn->isClassOrStruct())) {
                            for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); ++i) {
                                const Variable* var = symbolDatabase->getVariableFromVarId(i->first);
                                if (!var || !(var->isLocal() || var->isGlobal() || var->isStatic())) {
                                    i->second.mode = UNKNOWN_OM;
                                    i->second.mode_indent = 0;
                                    i->second.op_indent = indent;
                                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                                }
                            }
                            continue;
                        }
                    }
                    for (const Token* tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                        if (tok2->varId() && filepointers.find(tok2->varId()) != filepointers.end()) {
                            fileTok = tok2;
                            operation = Filepointer::UNKNOWN_OP; // Assume that repositioning was last operation and that the file is opened now
                            break;
                        }
                    }
                }

                while (Token::Match(fileTok, "%name% ."))
                    fileTok = fileTok->tokAt(2);

                if (!fileTok || !fileTok->varId())
                    continue;

                if (filepointers.find(fileTok->varId()) == filepointers.end()) { // function call indicates: Its a File
                    filepointers.insert(std::make_pair(fileTok->varId(), Filepointer(UNKNOWN_OM)));
                }
                Filepointer& f = filepointers[fileTok->varId()];

                switch (operation) {
                case Filepointer::OPEN:
                    f.mode = getMode(mode);
                    if (mode.find('a') != std::string::npos) {
                        if (f.mode == RW_MODE)
                            f.append_mode = Filepointer::APPEND_EX;
                        else
                            f.append_mode = Filepointer::APPEND;
                    }
                    f.mode_indent = indent;
                    break;
                case Filepointer::POSITIONING:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
                    else if (f.append_mode == Filepointer::APPEND && tok->str() != "fflush" && printWarnings)
                        seekOnAppendedFileError(tok);
                    break;
                case Filepointer::READ:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
                    else if (f.mode == WRITE_MODE)
                        readWriteOnlyFileError(tok);
                    else if (f.lastOperation == Filepointer::WRITE)
                        ioWithoutPositioningError(tok);
                    break;
                case Filepointer::WRITE:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
                    else if (f.mode == READ_MODE)
                        writeReadOnlyFileError(tok);
                    else if (f.lastOperation == Filepointer::READ)
                        ioWithoutPositioningError(tok);
                    break;
                case Filepointer::CLOSE:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
                    else
                        f.mode = CLOSED;
                    f.mode_indent = indent;
                    break;
                case Filepointer::UNIMPORTANT:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
                    break;
                case Filepointer::UNKNOWN_OP:
                    f.mode = UNKNOWN_OM;
                    f.mode_indent = 0;
                    break;
                default:
                    break;
                }
                if (operation != Filepointer::NONE && operation != Filepointer::UNIMPORTANT) {
                    f.op_indent = indent;
                    f.lastOperation = operation;
                }
            }
        }
        for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); ++i) {
            i->second.op_indent = 0;
            i->second.mode = UNKNOWN_OM;
            i->second.lastOperation = Filepointer::UNKNOWN_OP;
        }
    }
}

void CheckIO::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::portability,
                "fflushOnInputStream", "fflush() called on input stream '" + varname + "' may result in undefined behaviour on non-linux systems.");
}

void CheckIO::ioWithoutPositioningError(const Token *tok)
{
    reportError(tok, Severity::error,
                "IOWithoutPositioning", "Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.");
}

void CheckIO::readWriteOnlyFileError(const Token *tok)
{

    reportError(tok, Severity::error,
                "readWriteOnlyFile", "Read operation on a file that was opened only for writing.");
}

void CheckIO::writeReadOnlyFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "writeReadOnlyFile", "Write operation on a file that was opened only for reading.");
}

void CheckIO::useClosedFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "useClosedFile", "Used file that is not opened.");
}

void CheckIO::seekOnAppendedFileError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "seekOnAppendedFile", "Repositioning operation performed on a file opened in append mode has no effect.");
}


//---------------------------------------------------------------------------
// scanf without field width limits can crash with huge input data
//---------------------------------------------------------------------------
void CheckIO::invalidScanf()
{
    const bool printWarning = _settings->isEnabled("warning");
    const bool printPortability = _settings->isEnabled("portability");
    if (!printWarning && !printPortability)
        return;

    const bool windows = _settings->isWindowsPlatform();
    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t j = 0; j < functions; ++j) {
        const Scope * scope = symbolDatabase->functionScopes[j];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const Token *formatToken = nullptr;
            if (Token::Match(tok, "scanf|vscanf ( %str% ,"))
                formatToken = tok->tokAt(2);
            else if (Token::Match(tok, "sscanf|vsscanf|fscanf|vfscanf (")) {
                const Token* nextArg = tok->tokAt(2)->nextArgument();
                if (nextArg && nextArg->type() == Token::eString)
                    formatToken = nextArg;
                else
                    continue;
            } else
                continue;

            bool format = false;

            // scan the string backwards, so we do not need to keep states
            const std::string &formatstr(formatToken->str());
            for (std::size_t i = 1; i < formatstr.length(); i++) {
                if (formatstr[i] == '%')
                    format = !format;

                else if (!format)
                    continue;

                else if (std::isdigit(formatstr[i]) || formatstr[i] == '*') {
                    format = false;
                }

                else if (std::isalpha((unsigned char)formatstr[i]) || formatstr[i] == '[') {
                    if (printWarning && (formatstr[i] == 's' || formatstr[i] == '[' || formatstr[i] == 'S' || (formatstr[i] == 'l' && formatstr[i+1] == 's')))  // #3490 - field width limits are only necessary for string input
                        invalidScanfError(tok, false);
                    else if (printPortability && formatstr[i] != 'n' && formatstr[i] != 'c' && !windows)
                        invalidScanfError(tok, true); // Warn about libc bug in versions prior to 2.13-25
                    format = false;
                }
            }
        }
    }
}

void CheckIO::invalidScanfError(const Token *tok, bool portability)
{
    if (portability)
        reportError(tok, Severity::portability,
                    "invalidscanf_libc", "scanf without field width limits can crash with huge input data on some versions of libc.\n"
                    "scanf without field width limits can crash with huge input data on libc versions older than 2.13-25. Add a field "
                    "width specifier to fix this problem:\n"
                    "    %i => %3i\n"
                    "\n"
                    "Sample program that can crash:\n"
                    "\n"
                    "#include <stdio.h>\n"
                    "int main()\n"
                    "{\n"
                    "    int a;\n"
                    "    scanf(\"%i\", &a);\n"
                    "    return 0;\n"
                    "}\n"
                    "\n"
                    "To make it crash:\n"
                    "perl -e 'print \"5\"x2100000' | ./a.out");
    else
        reportError(tok, Severity::warning,
                    "invalidscanf", "scanf without field width limits can crash with huge input data.\n"
                    "scanf without field width limits can crash with huge input data. Add a field width "
                    "specifier to fix this problem:\n"
                    "    %s => %20s\n"
                    "\n"
                    "Sample program that can crash:\n"
                    "\n"
                    "#include <stdio.h>\n"
                    "int main()\n"
                    "{\n"
                    "    char c[5];\n"
                    "    scanf(\"%s\", c);\n"
                    "    return 0;\n"
                    "}\n"
                    "\n"
                    "Typing in 5 or more characters may make the program crash. The correct usage "
                    "here is 'scanf(\"%4s\", c);', as the maximum field width does not include the "
                    "terminating null byte.\n"
                    "Source: http://linux.die.net/man/3/scanf\n"
                    "Source: http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/libkern/stdio/scanf.c"
                   );
}

//---------------------------------------------------------------------------
//    printf("%u", "xyz"); // Wrong argument type
//    printf("%u%s", 1); // Too few arguments
//    printf("", 1); // Too much arguments
//---------------------------------------------------------------------------

static bool findFormat(unsigned int arg, const Token *firstArg,
                       const Token **formatStringTok, const Token **formatArgTok)
{
    const Token* argTok = firstArg;

    for (unsigned int i = 0; i < arg && argTok; ++i)
        argTok = argTok->nextArgument();

    if (Token::Match(argTok, "%str% [,)]")) {
        *formatArgTok = argTok->nextArgument();
        *formatStringTok = argTok;
        return true;
    } else if (Token::Match(argTok, "%var% [,)]") &&
               argTok->variable() &&
               Token::Match(argTok->variable()->typeStartToken(), "char|wchar_t") &&
               (argTok->variable()->isPointer() ||
                (argTok->variable()->dimensions().size() == 1 &&
                 argTok->variable()->dimensionKnown(0) &&
                 argTok->variable()->dimension(0) != 0))) {
        *formatArgTok = argTok->nextArgument();
        *formatStringTok = nullptr;
        if (argTok->variable()) {
            const Token *varTok = argTok->variable()->nameToken();
            if (Token::Match(varTok, "%name% ; %name% = %str% ;") &&
                varTok->str() == varTok->strAt(2) &&
                Token::Match(varTok->tokAt(-4), "const char|wchar_t * const")) {
                *formatStringTok = varTok->tokAt(4);
            } else if (Token::Match(varTok, "%name% [ %num% ] = %str% ;") &&
                       Token::Match(varTok->tokAt(-2), "const char|wchar_t")) {
                *formatStringTok = varTok->tokAt(5);
            }
        }
        return true;
    }
    return false;
}

// Utility function returning whether iToTest equals iTypename or iOptionalPrefix+iTypename
static inline bool typesMatch(const std::string& iToTest, const std::string& iTypename, const std::string& iOptionalPrefix = "std::")
{
    return (iToTest == iTypename) || (iToTest == iOptionalPrefix + iTypename);
}

void CheckIO::checkWrongPrintfScanfArguments()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const bool printWarning = _settings->isEnabled("warning");
    const bool isWindows = _settings->isWindowsPlatform();

    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t j = 0; j < functions; ++j) {
        const Scope * scope = symbolDatabase->functionScopes[j];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isName()) continue;

            const Token* argListTok = 0; // Points to first va_list argument
            const Token* formatStringTok = 0; // Points to format string token
            std::string formatString;

            bool scan = false;
            bool scanf_s = false;
            int formatStringArgNo = -1;

            if (Token::Match(tok->next(), "( %any%") && _settings->library.formatstr_function(tok->str())) {
                const std::map<int, Library::ArgumentChecks>& argumentChecks = _settings->library.argumentChecks.at(tok->str());
                for (std::map<int, Library::ArgumentChecks>::const_iterator i = argumentChecks.cbegin(); i != argumentChecks.cend(); ++i) {
                    if (i->second.formatstr) {
                        formatStringArgNo = i->first - 1;
                        break;
                    }
                }

                scan = _settings->library.formatstr_scan(tok->str());
                scanf_s = _settings->library.formatstr_secure(tok->str());
            }

            if (formatStringArgNo >= 0) {
                // formatstring found in library. Find format string and first argument belonging to format string.
                if (!findFormat(static_cast<unsigned int>(formatStringArgNo), tok->tokAt(2), &formatStringTok, &argListTok))
                    continue;
            } else if (isWindows && Token::Match(tok, "Format|AppendFormat (") &&
                       Token::Match(tok->tokAt(-2), "%var% .") && tok->tokAt(-2)->variable() &&
                       tok->tokAt(-2)->variable()->typeStartToken()->str() == "CString") {
                // Find second parameter and format string
                if (!findFormat(0, tok->tokAt(2), &formatStringTok, &argListTok))
                    continue;
            } else if (Token::simpleMatch(tok, "swprintf (") && Token::Match(tok->tokAt(2)->nextArgument(), "%str%")) {
                // Find third parameter and format string
                if (!findFormat(1, tok->tokAt(2), &formatStringTok, &argListTok))
                    continue;
            } else if (Token::simpleMatch(tok, "swprintf (") && !Token::Match(tok->tokAt(2)->nextArgument(), "%str%")) {
                // Find forth parameter and format string
                if (!findFormat(2, tok->tokAt(2), &formatStringTok, &argListTok))
                    continue;
            } else if (isWindows && Token::Match(tok, "sprintf_s|swprintf_s (")) {
                // template <size_t size> int sprintf_s(char (&buffer)[size], const char *format, ...);
                if (findFormat(1, tok->tokAt(2), &formatStringTok, &argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
                // int sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...);
                else if (findFormat(2, tok->tokAt(2), &formatStringTok, &argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
            } else if (isWindows && Token::Match(tok, "_snprintf_s|_snwprintf_s (")) {
                // template <size_t size> int _snprintf_s(char (&buffer)[size], size_t count, const char *format, ...);
                if (findFormat(2, tok->tokAt(2), &formatStringTok, &argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
                // int _snprintf_s(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, ...);
                else if (findFormat(3, tok->tokAt(2), &formatStringTok, &argListTok)) {
                    if (!formatStringTok)
                        continue;
                }
            } else {
                continue;
            }

            if (formatStringTok)
                formatString = formatStringTok->str();
            else
                continue;

            // Count format string parameters..
            unsigned int numFormat = 0;
            unsigned int numSecure = 0;
            bool percent = false;
            const Token* argListTok2 = argListTok;
            std::set<unsigned int> parameterPositionsUsed;
            for (std::string::iterator i = formatString.begin(); i != formatString.end(); ++i) {
                if (*i == '%') {
                    percent = !percent;
                } else if (percent && *i == '[') {
                    while (i != formatString.end()) {
                        if (*i == ']') {
                            numFormat++;
                            if (argListTok)
                                argListTok = argListTok->nextArgument();
                            percent = false;
                            break;
                        }
                        ++i;
                    }
                    if (scanf_s) {
                        numSecure++;
                        if (argListTok) {
                            argListTok = argListTok->nextArgument();
                        }
                    }
                    if (i == formatString.end())
                        break;
                } else if (percent) {
                    percent = false;

                    bool _continue = false;
                    bool skip = false;
                    std::string width;
                    unsigned int parameterPosition = 0;
                    bool hasParameterPosition = false;
                    while (i != formatString.end() && *i != '[' && !std::isalpha((unsigned char)*i)) {
                        if (*i == '*') {
                            skip = true;
                            if (scan)
                                _continue = true;
                            else {
                                numFormat++;
                                if (argListTok)
                                    argListTok = argListTok->nextArgument();
                            }
                        } else if (std::isdigit(*i)) {
                            width += *i;
                        } else if (*i == '$') {
                            parameterPosition = static_cast<unsigned int>(std::atoi(width.c_str()));
                            hasParameterPosition = true;
                            width.clear();
                        }
                        ++i;
                    }
                    if (i != formatString.end() && *i == '[') {
                        while (i != formatString.end()) {
                            if (*i == ']') {
                                if (!skip) {
                                    numFormat++;
                                    if (argListTok)
                                        argListTok = argListTok->nextArgument();
                                }
                                break;
                            }
                            ++i;
                        }
                        if (scanf_s && !skip) {
                            numSecure++;
                            if (argListTok) {
                                argListTok = argListTok->nextArgument();
                            }
                        }
                        _continue = true;
                    }
                    if (i == formatString.end())
                        break;
                    if (_continue)
                        continue;

                    if (scan || *i != 'm') { // %m is a non-standard extension that requires no parameter on print functions.
                        ++numFormat;

                        // Handle parameter positions (POSIX extension) - Ticket #4900
                        if (hasParameterPosition) {
                            if (parameterPositionsUsed.find(parameterPosition) == parameterPositionsUsed.end())
                                parameterPositionsUsed.insert(parameterPosition);
                            else // Parameter already referenced, hence don't consider it a new format
                                --numFormat;
                        }

                        // Perform type checks
                        ArgumentInfo argInfo(argListTok, _settings);

                        if (argInfo.typeToken && !argInfo.isLibraryType(_settings)) {
                            if (scan) {
                                std::string specifier;
                                bool done = false;
                                while (!done) {
                                    switch (*i) {
                                    case 's':
                                        specifier += *i;
                                        if (argInfo.variableInfo && argInfo.isKnownType() && argInfo.variableInfo->isArray() && (argInfo.variableInfo->dimensions().size() == 1) && argInfo.variableInfo->dimensions()[0].known) {
                                            if (!width.empty()) {
                                                int numWidth = std::atoi(width.c_str());
                                                if (numWidth != (argInfo.variableInfo->dimension(0) - 1))
                                                    invalidScanfFormatWidthError(tok, numFormat, numWidth, argInfo.variableInfo);
                                            }
                                        }
                                        if (argListTok && argListTok->type() != Token::eString &&
                                            argInfo.isKnownType() && argInfo.isArrayOrPointer() &&
                                            (!Token::Match(argInfo.typeToken, "char|wchar_t") ||
                                             argInfo.typeToken->strAt(-1) == "const")) {
                                            if (!(argInfo.isArrayOrPointer() && argInfo.element && !argInfo.typeToken->isStandardType()))
                                                invalidScanfArgTypeError_s(tok, numFormat, specifier, &argInfo);
                                        }
                                        if (scanf_s) {
                                            numSecure++;
                                            if (argListTok) {
                                                argListTok = argListTok->nextArgument();
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'c':
                                        if (scanf_s) {
                                            numSecure++;
                                            if (argListTok) {
                                                argListTok = argListTok->nextArgument();
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'x':
                                    case 'X':
                                    case 'o':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                        else if (argInfo.isKnownType()) {
                                            if (!Token::Match(argInfo.typeToken, "char|short|int|long")) {
                                                if (argInfo.typeToken->isStandardType() || !argInfo.element)
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (!argInfo.isArrayOrPointer() ||
                                                       argInfo.typeToken->strAt(-1) == "const") {
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'h':
                                                    if (specifier[1] == 'h') {
                                                        if (argInfo.typeToken->str() != "char")
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (argInfo.typeToken->str() != "short")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                        else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                                 typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                                 typesMatch(argInfo.typeToken->originalName(), "intmax_t", "u"))
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             typesMatch(argInfo.typeToken->originalName(), "intmax_t", "u"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") &&
                                                               !typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'j':
                                                    if (argInfo.typeToken->originalName() != "uintmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 't':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'L':
                                                    if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                default:
                                                    if (argInfo.typeToken->str() != "int")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             typesMatch(argInfo.typeToken->originalName(), "intmax_t", "u"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                }
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'n':
                                    case 'd':
                                    case 'i':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                        else if (argInfo.isKnownType()) {
                                            if (!Token::Match(argInfo.typeToken, "char|short|int|long")) {
                                                if (argInfo.typeToken->isStandardType() || !argInfo.element)
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else if (argInfo.typeToken->isUnsigned() ||
                                                       !argInfo.isArrayOrPointer() ||
                                                       argInfo.typeToken->strAt(-1) == "const") {
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'h':
                                                    if (specifier[1] == 'h') {
                                                        if (argInfo.typeToken->str() != "char")
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    } else if (argInfo.typeToken->str() != "short")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                        else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                                 argInfo.typeToken->originalName() == "intmax_t")
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             argInfo.typeToken->originalName() == "intmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    } else if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 'j':
                                                    if (argInfo.typeToken->originalName() != "intmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") &&
                                                        !typesMatch(argInfo.typeToken->originalName(), "ssize_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 't':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                case 'L':
                                                    if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                default:
                                                    if (argInfo.typeToken->str() != "int")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             argInfo.typeToken->originalName() == "intmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, false);
                                                    break;
                                                }
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'u':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                        else if (argInfo.isKnownType()) {
                                            if (!Token::Match(argInfo.typeToken, "char|short|int|long")) {
                                                if (argInfo.typeToken->isStandardType() || !argInfo.element)
                                                    invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else if (!argInfo.typeToken->isUnsigned() ||
                                                       !argInfo.isArrayOrPointer() ||
                                                       argInfo.typeToken->strAt(-1) == "const") {
                                                invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'h':
                                                    if (specifier[1] == 'h') {
                                                        if (argInfo.typeToken->str() != "char")
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (argInfo.typeToken->str() != "short")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                        else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                                 argInfo.typeToken->originalName() == "uintmax_t")
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             argInfo.typeToken->originalName() == "uintmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    } else if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'j':
                                                    if (argInfo.typeToken->originalName() != "uintmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 't':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                case 'L':
                                                    if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             argInfo.typeToken->originalName() == "uintmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                default:
                                                    if (argInfo.typeToken->str() != "int")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             typesMatch(argInfo.typeToken->originalName(), "ssize_t") ||
                                                             argInfo.typeToken->originalName() == "uintmax_t")
                                                        invalidScanfArgTypeError_int(tok, numFormat, specifier, &argInfo, true);
                                                    break;
                                                }
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'e':
                                    case 'E':
                                    case 'f':
                                    case 'g':
                                    case 'G':
                                    case 'a':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                        else if (argInfo.isKnownType()) {
                                            if (!Token::Match(argInfo.typeToken, "float|double")) {
                                                if (argInfo.typeToken->isStandardType())
                                                    invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            } else if (!argInfo.isArrayOrPointer() ||
                                                       argInfo.typeToken->strAt(-1) == "const") {
                                                invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "double" || !argInfo.typeToken->isLong())
                                                            invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                                    } else if (argInfo.typeToken->str() != "double" || argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'L':
                                                    if (argInfo.typeToken->str() != "double" || !argInfo.typeToken->isLong())
                                                        invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                default:
                                                    if (argInfo.typeToken->str() != "float")
                                                        invalidScanfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                }
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'I':
                                        if ((i+1 != formatString.end() && *(i+1) == '6' &&
                                             i+2 != formatString.end() && *(i+2) == '4') ||
                                            (i+1 != formatString.end() && *(i+1) == '3' &&
                                             i+2 != formatString.end() && *(i+2) == '2')) {
                                            specifier += *i++;
                                            specifier += *i++;
                                            if ((i+1) != formatString.end() && !isalpha(*(i+1))) {
                                                specifier += *i;
                                                invalidLengthModifierError(tok, numFormat, specifier);
                                                done = true;
                                            } else {
                                                specifier += *i++;
                                            }
                                        } else {
                                            if ((i+1) != formatString.end() && !isalpha(*(i+1))) {
                                                specifier += *i;
                                                invalidLengthModifierError(tok, numFormat, specifier);
                                                done = true;
                                            } else {
                                                specifier += *i++;
                                            }
                                        }
                                        break;
                                    case 'h':
                                    case 'l':
                                        if (i+1 != formatString.end() && *(i+1) == *i)
                                            specifier += *i++;
                                        // fallthrough
                                    case 'j':
                                    case 'q':
                                    case 't':
                                    case 'z':
                                    case 'L':
                                        // Expect an alphabetical character after these specifiers
                                        if (i != formatString.end() && !isalpha(*(i+1))) {
                                            specifier += *i;
                                            invalidLengthModifierError(tok, numFormat, specifier);
                                            done = true;
                                        } else {
                                            specifier += *i++;
                                        }
                                        break;
                                    default:
                                        done = true;
                                        break;
                                    }
                                }
                            } else if (!scan && printWarning) {
                                std::string specifier;
                                bool done = false;
                                while (!done) {
                                    switch (*i) {
                                    case 's':
                                        if (argListTok->type() != Token::eString &&
                                            argInfo.isKnownType() && !argInfo.isArrayOrPointer()) {
                                            if (!Token::Match(argInfo.typeToken, "char|wchar_t")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_s(tok, numFormat, &argInfo);
                                            }
                                        }
                                        done = true;
                                        break;
                                    case 'n':
                                        if ((argInfo.isKnownType() && (!argInfo.isArrayOrPointer() || argInfo.typeToken->strAt(-1) == "const")) || argListTok->type() == Token::eString)
                                            invalidPrintfArgTypeError_n(tok, numFormat, &argInfo);
                                        done = true;
                                        break;
                                    case 'c':
                                    case 'x':
                                    case 'X':
                                    case 'o':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                        else if (argInfo.isKnownType()) {
                                            if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                                // use %p on pointers and arrays
                                                invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                            } else if (!Token::Match(argInfo.typeToken, "bool|short|long|int|char|wchar_t")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'j':
                                                    if (!(argInfo.typeToken->originalName() == "intmax_t" ||
                                                          argInfo.typeToken->originalName() == "uintmax_t"))
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 't':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    } else if (!(typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                                 typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                                 argInfo.typeToken->originalName() == "WPARAM" ||
                                                                 argInfo.typeToken->originalName() == "UINT_PTR" ||
                                                                 argInfo.typeToken->originalName() == "LONG_PTR" ||
                                                                 argInfo.typeToken->originalName() == "LPARAM" ||
                                                                 argInfo.typeToken->originalName() == "LRESULT"))
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                default:
                                                    if (!Token::Match(argInfo.typeToken, "bool|char|short|wchar_t|int"))
                                                        invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                }
                                            }
                                        } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                            // use %p on pointers and arrays
                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, &argInfo);
                                        }
                                        done = true;
                                        break;
                                    case 'd':
                                    case 'i':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString) {
                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                        } else if (argInfo.isKnownType()) {
                                            if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                                // use %p on pointers and arrays
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (argInfo.typeToken->isUnsigned() && !Token::Match(argInfo.typeToken, "char|short")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else if (!Token::Match(argInfo.typeToken, "bool|char|short|int|long")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                        else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                                 argInfo.typeToken->originalName() == "intmax_t")
                                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             argInfo.typeToken->originalName() == "intmax_t")
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'j':
                                                    if (argInfo.typeToken->originalName() != "intmax_t")
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 't':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    } else if (!typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t"))
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "ssize_t"))
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                default:
                                                    if (!Token::Match(argInfo.typeToken, "bool|char|short|int"))
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "ptrdiff_t") ||
                                                             argInfo.typeToken->originalName() == "intmax_t")
                                                        invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                }
                                            }
                                        } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                            // use %p on pointers and arrays
                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, &argInfo);
                                        }
                                        done = true;
                                        break;
                                    case 'u':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString) {
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                        } else if (argInfo.isKnownType()) {
                                            if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                                // use %p on pointers and arrays
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!argInfo.typeToken->isUnsigned() && argInfo.typeToken->str() != "bool") {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else if (!Token::Match(argInfo.typeToken, "bool|char|short|long|int")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                            } else {
                                                switch (specifier[0]) {
                                                case 'l':
                                                    if (specifier[1] == 'l') {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                        else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                                 argInfo.typeToken->originalName() == "uintmax_t")
                                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    } else if (argInfo.typeToken->str() != "long" || argInfo.typeToken->isLong())
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             argInfo.typeToken->originalName() == "uintmax_t")
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'j':
                                                    if (argInfo.typeToken->originalName() != "uintmax_t")
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'z':
                                                    if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                case 'I':
                                                    if (specifier.find("I64") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "long" || !argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    } else if (specifier.find("I32") != std::string::npos) {
                                                        if (argInfo.typeToken->str() != "int" || argInfo.typeToken->isLong())
                                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    } else if (!typesMatch(argInfo.typeToken->originalName(), "size_t"))
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                default:
                                                    if (!Token::Match(argInfo.typeToken, "bool|char|short|int"))
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    else if (typesMatch(argInfo.typeToken->originalName(), "size_t") ||
                                                             argInfo.typeToken->originalName() == "intmax_t")
                                                        invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                                    break;
                                                }
                                            }
                                        } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, &argInfo);
                                        }
                                        done = true;
                                        break;
                                    case 'p':
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidPrintfArgTypeError_p(tok, numFormat, &argInfo);
                                        else if (argInfo.isKnownType() && !argInfo.isArrayOrPointer())
                                            invalidPrintfArgTypeError_p(tok, numFormat, &argInfo);
                                        done = true;
                                        break;
                                    case 'e':
                                    case 'E':
                                    case 'f':
                                    case 'g':
                                    case 'G':
                                        specifier += *i;
                                        if (argInfo.typeToken->type() == Token::eString)
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                        else if (argInfo.isKnownType()) {
                                            if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                                // use %p on pointers and arrays
                                                invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            } else if (!Token::Match(argInfo.typeToken, "float|double")) {
                                                if (!(!argInfo.isArrayOrPointer() && argInfo.element))
                                                    invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                            } else if ((specifier[0] == 'L' && (!argInfo.typeToken->isLong() || argInfo.typeToken->str() != "double")) ||
                                                       (specifier[0] != 'L' && argInfo.typeToken->isLong()))
                                                invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                        } else if (argInfo.isArrayOrPointer() && !argInfo.element) {
                                            // use %p on pointers and arrays
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, &argInfo);
                                        }
                                        done = true;
                                        break;
                                    case 'h': // Can be 'hh' (signed char or unsigned char) or 'h' (short int or unsigned short int)
                                    case 'l': // Can be 'll' (long long int or unsigned long long int) or 'l' (long int or unsigned long int)
                                        // If the next character is the same (which makes 'hh' or 'll') then expect another alphabetical character
                                        if (i != formatString.end() && *(i+1) == *i) {
                                            if (i+1 != formatString.end()) {
                                                if (!isalpha(*(i+2))) {
                                                    std::string modifier;
                                                    modifier += *i;
                                                    modifier += *(i+1);
                                                    invalidLengthModifierError(tok, numFormat, modifier);
                                                    done = true;
                                                } else {
                                                    specifier = *i++;
                                                    specifier += *i++;
                                                }
                                            } else {
                                                done = true;
                                            }
                                        } else {
                                            if (i != formatString.end()) {
                                                if (!isalpha(*(i+1))) {
                                                    std::string modifier;
                                                    modifier += *i;
                                                    invalidLengthModifierError(tok, numFormat, modifier);
                                                    done = true;
                                                } else {
                                                    specifier = *i++;
                                                }
                                            } else {
                                                done = true;
                                            }
                                        }
                                        break;
                                    case 'I': // Microsoft extension: I for size_t and ptrdiff_t, I32 for __int32, and I64 for __int64
                                        if ((*(i+1) == '3' && *(i+2) == '2') ||
                                            (*(i+1) == '6' && *(i+2) == '4')) {
                                            specifier += *i++;
                                            specifier += *i++;
                                        }
                                        // fallthrough
                                    case 'j': // intmax_t or uintmax_t
                                    case 'z': // size_t
                                    case 't': // ptrdiff_t
                                    case 'L': // long double
                                        // Expect an alphabetical character after these specifiers
                                        if (i != formatString.end() && !isalpha(*(i+1))) {
                                            specifier += *i;
                                            invalidLengthModifierError(tok, numFormat, specifier);
                                            done = true;
                                        } else {
                                            specifier += *i++;
                                        }
                                        break;
                                    default:
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (argListTok)
                            argListTok = argListTok->nextArgument(); // Find next argument
                    }
                }
            }

            // Count printf/scanf parameters..
            unsigned int numFunction = 0;
            while (argListTok2) {
                numFunction++;
                argListTok2 = argListTok2->nextArgument(); // Find next argument
            }

            if (printWarning) {
                // Check that all parameter positions reference an actual parameter
                for (std::set<unsigned int>::const_iterator it = parameterPositionsUsed.begin() ; it != parameterPositionsUsed.end() ; ++it) {
                    if ((*it == 0) || (*it > numFormat))
                        wrongPrintfScanfPosixParameterPositionError(tok, tok->str(), *it, numFormat);
                }
            }

            // Mismatching number of parameters => warning
            if ((numFormat + numSecure) != numFunction)
                wrongPrintfScanfArgumentsError(tok, tok->originalName().empty() ? tok->str() : tok->originalName(), numFormat + numSecure, numFunction);
        }
    }
}

// We currently only support string literals, variables, and functions.
/// @todo add non-string literals, and generic expressions

CheckIO::ArgumentInfo::ArgumentInfo(const Token * tok, const Settings *settings)
    : variableInfo(0)
    , typeToken(0)
    , functionInfo(0)
    , element(false)
    , _template(false)
    , address(false)
    , tempToken(0)
{
    if (tok) {
        if (tok->type() == Token::eString) {
            typeToken = tok;
            return;
        } else if (tok->str() == "&" || tok->type() == Token::eVariable ||
                   tok->type() == Token::eFunction || Token::Match(tok, "%type% ::") ||
                   (Token::Match(tok, "static_cast|reinterpret_cast|const_cast <") &&
                    Token::simpleMatch(tok->linkAt(1), "> (") &&
                    Token::Match(tok->linkAt(1)->linkAt(1), ") ,|)"))) {
            if (Token::Match(tok, "static_cast|reinterpret_cast|const_cast")) {
                typeToken = tok->tokAt(2);
                while (typeToken->str() == "const" || typeToken->str() == "extern")
                    typeToken = typeToken->next();
                return;
            }
            if (tok->str() == "&") {
                address = true;
                tok = tok->next();
            }
            while (Token::Match(tok, "%type% ::"))
                tok = tok->tokAt(2);
            if (!tok || !(tok->type() == Token::eVariable || tok->type() == Token::eFunction))
                return;
            const Token *varTok = nullptr;
            const Token *tok1 = tok->next();
            for (; tok1; tok1 = tok1->next()) {
                if (tok1->str() == "," || tok1->str() == ")") {
                    if (tok1->previous()->str() == "]") {
                        varTok = tok1->linkAt(-1)->previous();
                        if (varTok->str() == ")" && varTok->link()->previous()->type() == Token::eFunction) {
                            const Function * function = varTok->link()->previous()->function();
                            if (function && function->retDef) {
                                typeToken = function->retDef;
                                while (typeToken->str() == "const" || typeToken->str() == "extern")
                                    typeToken = typeToken->next();
                                functionInfo = function;
                                element = true;
                            }
                            return;
                        }
                    } else if (tok1->previous()->str() == ")" && tok1->linkAt(-1)->previous()->type() == Token::eFunction) {
                        const Function * function = tok1->linkAt(-1)->previous()->function();
                        if (function && function->retDef) {
                            typeToken = function->retDef;
                            while (typeToken->str() == "const" || typeToken->str() == "extern")
                                typeToken = typeToken->next();
                            functionInfo = function;
                            element = false;
                        }
                        return;
                    } else
                        varTok = tok1->previous();
                    break;
                } else if (tok1->str() == "(" || tok1->str() == "{" || tok1->str() == "[")
                    tok1 = tok1->link();
                else if (tok1->link() && tok1->str() == "<")
                    tok1 = tok1->link();

                // check for some common well known functions
                else if ((Token::Match(tok1->previous(), "%var% . size|empty|c_str ( ) [,)]") && isStdContainer(tok1->previous())) ||
                         (Token::Match(tok1->previous(), "] . size|empty|c_str ( ) [,)]") && isStdContainer(tok1->previous()->link()->previous()))) {
                    tempToken = new Token(0);
                    tempToken->fileIndex(tok1->fileIndex());
                    tempToken->linenr(tok1->linenr());
                    if (tok1->next()->str() == "size") {
                        // size_t is platform dependent
                        if (settings->sizeof_size_t == 8) {
                            tempToken->str("long");
                            if (settings->sizeof_long != 8)
                                tempToken->isLong(true);
                        } else if (settings->sizeof_size_t == 4) {
                            if (settings->sizeof_long == 4) {
                                tempToken->str("long");
                            } else {
                                tempToken->str("int");
                            }
                        }

                        tempToken->originalName("size_t");
                        tempToken->isUnsigned(true);
                    } else if (tok1->next()->str() == "empty") {
                        tempToken->str("bool");
                    } else if (tok1->next()->str() == "c_str") {
                        tempToken->str("const");
                        tempToken->insertToken("*");
                        if (typeToken->strAt(2) == "string")
                            tempToken->insertToken("char");
                        else
                            tempToken->insertToken("wchar_t");
                    }
                    typeToken = tempToken;
                    return;
                }

                // check for std::vector::at() and std::string::at()
                else if (Token::Match(tok1->previous(), "%var% . at (") &&
                         Token::Match(tok1->linkAt(2), ") [,)]")) {
                    varTok = tok1->previous();
                    variableInfo = varTok->variable();

                    if (!variableInfo || !isStdVectorOrString()) {
                        variableInfo = 0;
                        typeToken = 0;
                    }

                    return;
                } else if (!(tok1->str() == "." || tok1->type() == Token::eVariable || tok1->type() == Token::eFunction))
                    return;
            }

            if (varTok) {
                variableInfo = varTok->variable();
                element = tok1->previous()->str() == "]";

                // look for std::vector operator [] and use template type as return type
                if (variableInfo) {
                    if (element && isStdVectorOrString()) { // isStdVectorOrString sets type token if true
                        element = false;    // not really an array element
                    } else
                        typeToken = variableInfo->typeStartToken();
                }

                return;
            }
        }
    }
}

CheckIO::ArgumentInfo::~ArgumentInfo()
{
    if (tempToken) {
        while (tempToken->next())
            tempToken->deleteNext();

        delete tempToken;
    }
}

bool CheckIO::ArgumentInfo::isStdVectorOrString()
{
    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_vector[] = {
        "array", "vector"
    };
    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_string[] = {
        "string", "u16string", "u32string", "wstring"
    };

    if (variableInfo->isStlType(stl_vector)) {
        typeToken = variableInfo->typeStartToken()->tokAt(4);
        _template = true;
        return true;
    } else if (variableInfo->isStlType(stl_string)) {
        tempToken = new Token(0);
        tempToken->fileIndex(variableInfo->typeStartToken()->fileIndex());
        tempToken->linenr(variableInfo->typeStartToken()->linenr());
        if (variableInfo->typeStartToken()->strAt(2) == "string")
            tempToken->str("char");
        else
            tempToken->str("wchar_t");
        typeToken = tempToken;
        return true;
    } else if (variableInfo->type() && !variableInfo->type()->derivedFrom.empty()) {
        for (std::size_t i = 0, e = variableInfo->type()->derivedFrom.size(); i != e; ++i) {
            if (Token::Match(variableInfo->type()->derivedFrom[i].nameTok, "std :: vector|array <")) {
                typeToken = variableInfo->type()->derivedFrom[i].nameTok->tokAt(4);
                _template = true;
                return true;
            } else if (Token::Match(variableInfo->type()->derivedFrom[i].nameTok, "std :: string|wstring")) {
                tempToken = new Token(0);
                tempToken->fileIndex(variableInfo->typeStartToken()->fileIndex());
                tempToken->linenr(variableInfo->typeStartToken()->linenr());
                if (variableInfo->type()->derivedFrom[i].nameTok->strAt(2) == "string")
                    tempToken->str("char");
                else
                    tempToken->str("wchar_t");
                typeToken = tempToken;
                return true;
            }
        }
    } else if (variableInfo->type()) {
        const Scope * classScope = variableInfo->type()->classScope;
        if (classScope) {
            std::list<Function>::const_iterator functions;
            for (functions = classScope->functionList.begin();
                 functions != classScope->functionList.end(); ++functions) {
                if (functions->name() == "operator[]") {
                    if (Token::Match(functions->retDef, "%type% &")) {
                        typeToken = functions->retDef;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool CheckIO::ArgumentInfo::isStdContainer(const Token *tok)
{
    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_container[] = {
        "array", "bitset", "deque", "forward_list",
        "hash_map", "hash_multimap", "hash_set",
        "list", "map", "multimap", "multiset",
        "priority_queue", "queue", "set", "stack",
        "unordered_map", "unordered_multimap", "unordered_multiset", "unordered_set",
        "vector"
    };
    // THIS ARRAY MUST BE ORDERED ALPHABETICALLY
    static const char* const stl_string[]= {
        "string", "u16string", "u32string", "wstring"
    };

    if (tok && tok->variable()) {
        const Variable* variable = tok->variable();
        if (variable->isStlType(stl_container)) {
            typeToken = variable->typeStartToken()->tokAt(4);
            return true;
        } else if (variable->isStlType(stl_string)) {
            typeToken = variable->typeStartToken();
            return true;
        } else if (variable->type() && !variable->type()->derivedFrom.empty()) {
            const std::vector<Type::BaseInfo>& derivedFrom = variable->type()->derivedFrom;
            for (std::size_t i = 0, e = derivedFrom.size(); i != e; ++i) {
                const Token* nameTok = derivedFrom[i].nameTok;
                if (Token::Match(nameTok, "std :: vector|array|bitset|deque|list|forward_list|map|multimap|multiset|priority_queue|queue|set|stack|hash_map|hash_multimap|hash_set|unordered_map|unordered_multimap|unordered_set|unordered_multiset <")) {
                    typeToken = nameTok->tokAt(4);
                    return true;
                } else if (Token::Match(nameTok, "std :: string|wstring")) {
                    typeToken = nameTok;
                    return true;
                }
            }
        }
    }

    return false;
}

bool CheckIO::ArgumentInfo::isArrayOrPointer() const
{
    if (address)
        return true;
    else if (variableInfo && !_template) {
        return variableInfo->isArrayOrPointer();
    } else {
        const Token *tok = typeToken;
        while (tok && Token::Match(tok, "const|struct"))
            tok = tok->next();
        if (tok && tok->strAt(1) == "*")
            return true;
    }
    return false;
}

bool CheckIO::ArgumentInfo::isComplexType() const
{
    if (variableInfo->type())
        return (true);

    const Token* varTypeTok = typeToken;
    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);

    return ((variableInfo->isStlStringType() || (varTypeTok->strAt(1) == "<" && varTypeTok->linkAt(1) && varTypeTok->linkAt(1)->strAt(1) != "::")) && !variableInfo->isArrayOrPointer());
}

bool CheckIO::ArgumentInfo::isKnownType() const
{
    if (variableInfo)
        return (typeToken->isStandardType() || typeToken->next()->isStandardType() || isComplexType());
    else if (functionInfo)
        return (typeToken->isStandardType() || functionInfo->retType || Token::Match(typeToken, "std :: string|wstring"));

    return typeToken->isStandardType() || Token::Match(typeToken, "std :: string|wstring");
}

bool CheckIO::ArgumentInfo::isLibraryType(const Settings *settings) const
{
    return typeToken && typeToken->isStandardType() && settings->library.podtype(typeToken->str());
}

void CheckIO::wrongPrintfScanfArgumentsError(const Token* tok,
        const std::string &functionName,
        unsigned int numFormat,
        unsigned int numFunction)
{
    Severity::SeverityType severity = numFormat > numFunction ? Severity::error : Severity::warning;
    if (severity != Severity::error && !_settings->isEnabled("warning"))
        return;

    std::ostringstream errmsg;
    errmsg << functionName
           << " format string requires "
           << numFormat
           << " parameter" << (numFormat != 1 ? "s" : "") << " but "
           << (numFormat > numFunction ? "only " : "")
           << numFunction
           << (numFunction != 1 ? " are" : " is")
           << " given.";

    reportError(tok, severity, "wrongPrintfScanfArgNum", errmsg.str());
}

void CheckIO::wrongPrintfScanfPosixParameterPositionError(const Token* tok, const std::string& functionName,
        unsigned int index, unsigned int numFunction)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << functionName << ": ";
    if (index == 0) {
        errmsg << "parameter positions start at 1, not 0";
    } else {
        errmsg << "referencing parameter " << index << " while " << numFunction << " arguments given";
    }
    reportError(tok, Severity::warning, "wrongPrintfScanfParameterPositionError", errmsg.str());
}

void CheckIO::invalidScanfArgTypeError_s(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires a \'";
    if (specifier[0] == 's')
        errmsg << "char";
    else if (specifier[0] == 'S')
        errmsg << "wchar_t";
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidScanfArgType_s", errmsg.str());
}
void CheckIO::invalidScanfArgTypeError_int(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo, bool isUnsigned)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'h') {
        if (specifier[1] == 'h')
            errmsg << (isUnsigned ? "unsigned " : "") << "char";
        else
            errmsg << (isUnsigned ? "unsigned " : "") << "short";
    } else if (specifier[0] == 'l') {
        if (specifier[1] == 'l')
            errmsg << (isUnsigned ? "unsigned " : "") << "long long";
        else
            errmsg << (isUnsigned ? "unsigned " : "") << "long";
    } else if (specifier.find("I32") != std::string::npos) {
        errmsg << (isUnsigned ? "unsigned " : "") << "__int32";
    } else if (specifier.find("I64") != std::string::npos) {
        errmsg << (isUnsigned ? "unsigned " : "") << "__int64";
    } else if (specifier[0] == 'I') {
        errmsg << (isUnsigned ? "size_t" : "ptrdiff_t");
    } else if (specifier[0] == 'j') {
        if (isUnsigned)
            errmsg << "uintmax_t";
        else
            errmsg << "intmax_t";
    } else if (specifier[0] == 'z') {
        if (specifier[1] == 'd')
            errmsg << "ptrdiff_t";
        else
            errmsg << "size_t";
    } else if (specifier[0] == 't') {
        errmsg << (isUnsigned ? "unsigned " : "") << "ptrdiff_t";
    } else if (specifier[0] == 'L') {
        errmsg << (isUnsigned ? "unsigned " : "") << "long long";
    } else {
        errmsg << (isUnsigned ? "unsigned " : "") << "int";
    }
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidScanfArgType_int", errmsg.str());
}
void CheckIO::invalidScanfArgTypeError_float(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'l' && specifier[1] != 'l')
        errmsg << "double";
    else if (specifier[0] == 'L')
        errmsg << "long double";
    else
        errmsg << "float";
    errmsg << " *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidScanfArgType_float", errmsg.str());
}

void CheckIO::invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%s in format string (no. " << numFormat << ") requires \'char *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_s", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%n in format string (no. " << numFormat << ") requires \'int *\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_n", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an address but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_p", errmsg.str());
}
static void printfFormatType(std::ostream& os, const std::string& specifier, bool isUnsigned)
{
    os << "\'";
    if (specifier[0] == 'l') {
        if (specifier[1] == 'l')
            os << (isUnsigned ? "unsigned " : "") << "long long";
        else
            os << (isUnsigned ? "unsigned " : "") << "long";
    } else if (specifier.find("I32") != std::string::npos) {
        os << (isUnsigned ? "unsigned " : "") << "__int32";
    } else if (specifier.find("I64") != std::string::npos) {
        os << (isUnsigned ? "unsigned " : "") << "__int64";
    } else if (specifier[0] == 'I') {
        os << (isUnsigned ? "size_t" : "ptrdiff_t");
    } else if (specifier[0] == 'j') {
        if (isUnsigned)
            os << "uintmax_t";
        else
            os << "intmax_t";
    } else if (specifier[0] == 'z') {
        if (specifier[1] == 'd')
            os << "ssize_t";
        else
            os << "size_t";
    } else if (specifier[0] == 't') {
        os << (isUnsigned ? "unsigned " : "") << "ptrdiff_t";
    } else if (specifier[0] == 'L') {
        os << (isUnsigned ? "unsigned " : "") << "long long";
    } else {
        os << (isUnsigned ? "unsigned " : "") << "int";
    }
    os << "\'";
}
void CheckIO::invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires ";
    printfFormatType(errmsg, specifier, true);
    errmsg << " but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_int", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_uint(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires ";
    printfFormatType(errmsg, specifier, true);
    errmsg << " but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_uint", errmsg.str());
}

void CheckIO::invalidPrintfArgTypeError_sint(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires ";
    printfFormatType(errmsg, specifier, false);
    errmsg << " but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_sint", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires \'";
    if (specifier[0] == 'L')
        errmsg << "long ";
    errmsg << "double\' but the argument type is ";
    argumentType(errmsg, argInfo);
    errmsg << ".";
    reportError(tok, Severity::warning, "invalidPrintfArgType_float", errmsg.str());
}

void CheckIO::argumentType(std::ostream& os, const ArgumentInfo * argInfo)
{
    if (argInfo) {
        os << "\'";
        const Token *type = argInfo->typeToken;
        if (type->type() == Token::eString) {
            if (type->isLong())
                os << "const wchar_t *";
            else
                os << "const char *";
        } else {
            if (type->originalName().empty()) {
                if (type->strAt(-1) == "const")
                    os << "const ";
                while (Token::Match(type, "const|struct")) {
                    os << type->str() << " ";
                    type = type->next();
                }
                while (Token::Match(type, "%any% ::")) {
                    os << type->str() << "::";
                    type = type->tokAt(2);
                }
                type->stringify(os, false, true, false);
                if (type->strAt(1) == "*" && !argInfo->element)
                    os << " *";
                else if (argInfo->variableInfo && !argInfo->element && argInfo->variableInfo->isArray())
                    os << " *";
                else if (type->strAt(1) == "*" && argInfo->variableInfo && argInfo->element && argInfo->variableInfo->isArray())
                    os << " *";
                if (argInfo->address)
                    os << " *";
            } else {
                if ((type->originalName() == "__int64" || type->originalName() == "__int32") && type->isUnsigned())
                    os << "unsigned ";
                os << type->originalName();
                if (type->strAt(1) == "*" || argInfo->address)
                    os << " *";
                os << " {aka ";
                type->stringify(os, false, true, false);
                if (type->strAt(1) == "*" || argInfo->address)
                    os << " *";
                os << "}";
            }
        }
        os << "\'";
    } else
        os << "Unknown";
}

void CheckIO::invalidLengthModifierError(const Token* tok, unsigned int numFormat, const std::string& modifier)
{
    if (!_settings->isEnabled("warning"))
        return;
    std::ostringstream errmsg;
    errmsg << "'" << modifier << "' in format string (no. " << numFormat << ") is a length modifier and cannot be used without a conversion specifier.";
    reportError(tok, Severity::warning, "invalidLengthModifierError", errmsg.str());
}

void CheckIO::invalidScanfFormatWidthError(const Token* tok, unsigned int numFormat, int width, const Variable *var)
{
    MathLib::bigint arrlen = 0;
    std::string varname;

    if (var) {
        arrlen = var->dimension(0);
        varname = var->name();
    }

    std::ostringstream errmsg;
    if (arrlen > width) {
        if (!_settings->inconclusive || !_settings->isEnabled("warning"))
            return;
        errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is smaller than destination buffer"
               << " '" << varname << "[" << arrlen << "]'.";
        reportError(tok, Severity::warning, "invalidScanfFormatWidth_smaller", errmsg.str(), 0U, true);
    } else {
        errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is larger than destination buffer '"
               << varname << "[" << arrlen << "]', use %" << (arrlen - 1) << "s to prevent overflowing it.";
        reportError(tok, Severity::error, "invalidScanfFormatWidth", errmsg.str(), 0U, false);
    }
}
