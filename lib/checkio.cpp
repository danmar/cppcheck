/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "token.h"
#include "errorlogger.h"
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
    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        bool firstCout = false;
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() == "(")
                tok = tok->link();

            if (Token::Match(tok, "std :: cout|cerr")) {
                if (firstCout && tok->strAt(-1) == "<<" && tok->strAt(3) != ".") {
                    coutCerrMisusageError(tok, tok->strAt(2));
                    firstCout = false;
                } else if (tok->strAt(3) == "<<")
                    firstCout = true;
            } else if (firstCout && tok->str() == ";")
                firstCout = false;
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
enum OpenMode {CLOSED, READ_MODE, WRITE_MODE, RW_MODE, UNKNOWN};
static OpenMode getMode(const std::string& str)
{
    if (str.find('+', 1) != std::string::npos)
        return RW_MODE;
    else if (str.find('w') != std::string::npos || str.find('a') != std::string::npos)
        return WRITE_MODE;
    else if (str.find('r') != std::string::npos)
        return READ_MODE;
    return UNKNOWN;
}

struct Filepointer {
    OpenMode mode;
    unsigned int mode_indent;
    enum Operation {NONE, UNIMPORTANT, READ, WRITE, POSITIONING, OPEN, CLOSE, UNKNOWN_OP} lastOperation;
    unsigned int op_indent;
    Filepointer(OpenMode mode_ = UNKNOWN)
        : mode(mode_), mode_indent(0), lastOperation(NONE), op_indent(0) {
    }
};

void CheckIO::checkFileUsage()
{
    static const char* _whitelist[] = {
        "clearerr", "feof", "ferror", "fgetpos", "ftell", "setbuf", "setvbuf", "ungetc"
    };
    static const std::set<std::string> whitelist(_whitelist, _whitelist + sizeof(_whitelist)/sizeof(*_whitelist));

    std::map<unsigned int, Filepointer> filepointers;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t varListSize = symbolDatabase->getVariableListSize();
    for (std::size_t i = 1; i < varListSize; ++i) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->varId() || var->isArray() || !Token::simpleMatch(var->typeStartToken(), "FILE *"))
            continue;

        if (var->isLocal()) {
            if (var->nameToken()->strAt(1) == "(") // initialize by calling "ctor"
                filepointers.insert(std::make_pair(var->varId(), Filepointer(UNKNOWN)));
            else
                filepointers.insert(std::make_pair(var->varId(), Filepointer(CLOSED)));
        } else {
            filepointers.insert(std::make_pair(var->varId(), Filepointer(UNKNOWN)));
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
                        i->second.mode = UNKNOWN;
                    }
                    if (indent < i->second.op_indent) {
                        i->second.op_indent = 0;
                        i->second.lastOperation = Filepointer::UNKNOWN_OP;
                    }
                }
            } else if (tok->str() == "return" || tok->str() == "continue" || tok->str() == "break") { // Reset upon return, continue or break
                for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); ++i) {
                    i->second.mode_indent = 0;
                    i->second.mode = UNKNOWN;
                    i->second.op_indent = 0;
                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                }
            } else if (tok->varId() && Token::Match(tok, "%var% =") && (tok->strAt(2) != "fopen" && tok->strAt(2) != "freopen" && tok->strAt(2) != "tmpfile")) {
                std::map<unsigned int, Filepointer>::iterator i = filepointers.find(tok->varId());
                if (i != filepointers.end()) {
                    i->second.mode = UNKNOWN;
                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                }
            } else if (Token::Match(tok, "%var% (") && tok->previous() && (!tok->previous()->isName() || Token::Match(tok->previous(), "return|throw"))) {
                std::string mode;
                const Token* fileTok = 0;
                Filepointer::Operation operation = Filepointer::NONE;

                if ((tok->str() == "fopen" || tok->str() == "freopen" || tok->str() == "tmpfile") && tok->strAt(-1) == "=") {
                    if (tok->str() != "tmpfile") {
                        const Token* modeTok = tok->tokAt(2)->nextArgument();
                        if (modeTok && modeTok->type() == Token::eString)
                            mode = modeTok->strValue();
                    } else
                        mode = "wb+";
                    fileTok = tok->tokAt(-2);
                    operation = Filepointer::OPEN;
                } else if (tok->str() == "rewind" || tok->str() == "fseek" || tok->str() == "fsetpos" || tok->str() == "fflush") {
                    if (Token::simpleMatch(tok, "fflush ( stdin )"))
                        fflushOnInputStreamError(tok, tok->strAt(2));
                    else {
                        fileTok = tok->tokAt(2);
                        operation = Filepointer::POSITIONING;
                    }
                } else if (tok->str() == "fgetc" || tok->str() == "fgets" || tok->str() == "fread" || tok->str() == "fscanf" || tok->str() == "getc") {
                    if (tok->str() == "fscanf")
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::READ;
                } else if (tok->str() == "fputc" || tok->str() == "fputs" || tok->str() == "fwrite" || tok->str() == "fprintf" || tok->str() == "putcc") {
                    if (tok->str() == "fprintf")
                        fileTok = tok->tokAt(2);
                    else
                        fileTok = tok->linkAt(1)->previous();
                    operation = Filepointer::WRITE;
                } else if (tok->str() == "fclose") {
                    fileTok = tok->tokAt(2);
                    operation = Filepointer::CLOSE;
                } else if (whitelist.find(tok->str()) != whitelist.end()) {
                    fileTok = tok->tokAt(2);
                    if (tok->str() == "ungetc" && fileTok)
                        fileTok = fileTok->nextArgument();
                    operation = Filepointer::UNIMPORTANT;
                } else if (!Token::Match(tok, "if|for|while|catch|switch")) {
                    const Token* const end2 = tok->linkAt(1);
                    for (const Token* tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                        if (tok2->varId() && filepointers.find(tok2->varId()) != filepointers.end()) {
                            fileTok = tok2;
                            operation = Filepointer::UNKNOWN_OP; // Assume that repositioning was last operation and that the file is opened now
                            break;
                        }
                    }
                }

                while (Token::Match(fileTok, "%var% ."))
                    fileTok = fileTok->tokAt(2);

                if (!fileTok || !fileTok->varId())
                    continue;

                if (filepointers.find(fileTok->varId()) == filepointers.end()) { // function call indicates: Its a File
                    filepointers.insert(std::make_pair(fileTok->varId(), Filepointer(UNKNOWN)));
                }
                Filepointer& f = filepointers[fileTok->varId()];

                switch (operation) {
                case Filepointer::OPEN:
                    f.mode = getMode(mode);
                    f.mode_indent = indent;
                    break;
                case Filepointer::POSITIONING:
                    if (f.mode == CLOSED)
                        useClosedFileError(tok);
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
                    f.mode = UNKNOWN;
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
            i->second.mode = UNKNOWN;
            i->second.lastOperation = Filepointer::UNKNOWN_OP;
        }
    }
}

void CheckIO::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream '" + varname + "' results in undefined behaviour.");
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


//---------------------------------------------------------------------------
// scanf without field width limits can crash with huge input data
//---------------------------------------------------------------------------
void CheckIO::invalidScanf()
{
    if (!_settings->isEnabled("warning") && !_settings->isEnabled("portability"))
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t j = 0; j < functions; ++j) {
        const Scope * scope = symbolDatabase->functionScopes[j];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const Token *formatToken = 0;
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

            // scan the string backwards, so we dont need to keep states
            const std::string &formatstr(formatToken->str());
            for (unsigned int i = 1; i < formatstr.length(); i++) {
                if (formatstr[i] == '%')
                    format = !format;

                else if (!format)
                    continue;

                else if (std::isdigit(formatstr[i]) || formatstr[i] == '*') {
                    format = false;
                }

                else if (std::isalpha(formatstr[i]) || formatstr[i] == '[') {
                    if ((formatstr[i] == 's' || formatstr[i] == '[' || formatstr[i] == 'S' || (formatstr[i] == 'l' && formatstr[i+1] == 's')) && _settings->isEnabled("warning"))  // #3490 - field width limits are only necessary for string input
                        invalidScanfError(tok, false);
                    else if (formatstr[i] != 'n' && formatstr[i] != 'c' && _settings->platformType != Settings::Win32A && _settings->platformType != Settings::Win32W && _settings->platformType != Settings::Win64 && _settings->isEnabled("portability"))
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
                    "invalidscanf", "scanf without field width limits can crash with huge input data on some versions of libc.\n"
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
                    "To make it crash, type in more than 5 characters.");
}

//---------------------------------------------------------------------------
//    printf("%u", "xyz"); // Wrong argument type
//    printf("%u%s", 1); // Too few arguments
//    printf("", 1); // Too much arguments
//---------------------------------------------------------------------------
static bool isComplexType(const Variable* var, const Token* varTypeTok)
{
    if (var->type())
        return(true);

    static std::set<std::string> knownTypes;
    if (knownTypes.empty()) {
        knownTypes.insert("struct"); // If a type starts with the struct keyword, its a complex type
        knownTypes.insert("string");
        knownTypes.insert("wstring");
    }

    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);
    return((knownTypes.find(varTypeTok->str()) != knownTypes.end() || (varTypeTok->strAt(1) == "<" && varTypeTok->linkAt(1) && varTypeTok->linkAt(1)->strAt(1) != "::")) && !var->isPointer() && !var->isArray());
}

static bool isKnownType(const Variable* var, const Token* varTypeTok)
{
    return(varTypeTok->isStandardType() || varTypeTok->next()->isStandardType() || isComplexType(var, varTypeTok));
}

void CheckIO::checkWrongPrintfScanfArguments()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    bool warning = _settings->isEnabled("warning");

    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t j = 0; j < functions; ++j) {
        const Scope * scope = symbolDatabase->functionScopes[j];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isName()) continue;

            const Token* argListTok = 0; // Points to first va_list argument
            std::string formatString;

            if (Token::Match(tok, "printf|scanf|wprintf|wscanf ( %str%")) {
                formatString = tok->strAt(2);
                if (tok->strAt(3) == ",") {
                    argListTok = tok->tokAt(4);
                } else if (tok->strAt(3) == ")") {
                    argListTok = 0;
                } else {
                    continue;
                }
            } else if (Token::Match(tok, "sprintf|fprintf|sscanf|fscanf|swscanf|fwprintf|fwscanf ( %any%") || (Token::simpleMatch(tok, "swprintf (") && Token::Match(tok->tokAt(2)->nextArgument(), "%str%"))) {
                const Token* formatStringTok = tok->tokAt(2)->nextArgument(); // Find second parameter (format string)
                if (Token::Match(formatStringTok, "%str% ,")) {
                    argListTok = formatStringTok->nextArgument(); // Find third parameter (first argument of va_args)
                    formatString = formatStringTok->str();
                } else if (Token::Match(formatStringTok, "%str% )")) {
                    argListTok = 0; // Find third parameter (first argument of va_args)
                    formatString = formatStringTok->str();
                } else {
                    continue;
                }
            } else if (Token::Match(tok, "snprintf|fnprintf (") || (Token::simpleMatch(tok, "swprintf (") && !Token::Match(tok->tokAt(2)->nextArgument(), "%str%"))) {
                const Token* formatStringTok = tok->tokAt(2);
                for (int i = 0; i < 2 && formatStringTok; i++) {
                    formatStringTok = formatStringTok->nextArgument(); // Find third parameter (format string)
                }
                if (Token::Match(formatStringTok, "%str% ,")) {
                    argListTok = formatStringTok->nextArgument(); // Find fourth parameter (first argument of va_args)
                    formatString = formatStringTok->str();
                } else if (Token::Match(formatStringTok, "%str% )")) {
                    argListTok = 0; // Find fourth parameter (first argument of va_args)
                    formatString = formatStringTok->str();
                } else {
                    continue;
                }
            } else {
                continue;
            }

            // Count format string parameters..
            bool scan = Token::Match(tok, "sscanf|fscanf|scanf|swscanf|fwscanf|wscanf");
            unsigned int numFormat = 0;
            bool percent = false;
            const Token* argListTok2 = argListTok;
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
                    if (i == formatString.end())
                        break;
                } else if (percent) {
                    percent = false;

                    bool _continue = false;
                    std::string width;
                    while (i != formatString.end() && *i != ']' && !std::isalpha(*i)) {
                        if (*i == '*') {
                            if (scan)
                                _continue = true;
                            else {
                                numFormat++;
                                if (argListTok)
                                    argListTok = argListTok->nextArgument();
                            }
                        } else if (std::isdigit(*i))
                            width += *i;
                        ++i;
                    }
                    if (i == formatString.end())
                        break;
                    if (_continue)
                        continue;

                    if (scan || *i != 'm') { // %m is a non-standard extension that requires no parameter on print functions.
                        numFormat++;

                        // Perform type checks
                        if (argListTok && Token::Match(argListTok->next(), "[,)]")) { // We can currently only check the type of arguments matching this simple pattern.
                            const Variable *variableInfo = argListTok->variable();
                            const Token* varTypeTok = variableInfo ? variableInfo->typeStartToken() : NULL;
                            if (varTypeTok && varTypeTok->str() == "static")
                                varTypeTok = varTypeTok->next();

                            if (scan && varTypeTok) {
                                if (warning && ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->strAt(-1) == "const"))
                                    invalidScanfArgTypeError(tok, tok->str(), numFormat);

                                if (*i == 's' && variableInfo && isKnownType(variableInfo, varTypeTok) && variableInfo->isArray() && (variableInfo->dimensions().size() == 1) && variableInfo->dimensions()[0].known) {
                                    if (!width.empty()) {
                                        int numWidth = std::atoi(width.c_str());
                                        if (numWidth != (variableInfo->dimension(0) - 1))
                                            invalidScanfFormatWidthError(tok, numFormat, numWidth, variableInfo);
                                    }
                                }
                            } else if (!scan && warning) {
                                switch (*i) {
                                case 's':
                                    if (variableInfo && argListTok->type() != Token::eString && isKnownType(variableInfo, varTypeTok) && (!variableInfo->isPointer() && !variableInfo->isArray()))
                                        invalidPrintfArgTypeError_s(tok, numFormat);
                                    break;
                                case 'n':
                                    if ((varTypeTok && isKnownType(variableInfo, varTypeTok) && ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->strAt(-1) == "const")) || argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_n(tok, numFormat);
                                    break;
                                case 'c':
                                case 'x':
                                case 'X':
                                case 'o':
                                    if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "bool|short|long|int|char|size_t") && !variableInfo->isPointer() && !variableInfo->isArray())
                                        invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                    else if (argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                    break;
                                case 'd':
                                case 'i':
                                    if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray()) {
                                        if ((varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "bool|short|long|int")) && varTypeTok->str() != "char")
                                            invalidPrintfArgTypeError_sint(tok, numFormat, *i);
                                    } else if (argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_sint(tok, numFormat, *i);
                                    break;
                                case 'u':
                                    if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray()) {
                                        if ((!varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "char|short|long|int|size_t")) && varTypeTok->str() != "bool")
                                            invalidPrintfArgTypeError_uint(tok, numFormat, *i);
                                    } else if (argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_uint(tok, numFormat, *i);
                                    break;
                                case 'p':
                                    if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "short|long|int|size_t") && !variableInfo->isPointer() && !variableInfo->isArray())
                                        invalidPrintfArgTypeError_p(tok, numFormat);
                                    else if (argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_p(tok, numFormat);
                                    break;
                                case 'e':
                                case 'E':
                                case 'f':
                                case 'g':
                                case 'G':
                                    if (varTypeTok && ((isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "float|double")) || variableInfo->isPointer() || variableInfo->isArray()))
                                        invalidPrintfArgTypeError_float(tok, numFormat, *i);
                                    else if (argListTok->type() == Token::eString)
                                        invalidPrintfArgTypeError_float(tok, numFormat, *i);
                                    break;
                                case 'h': // Can be 'hh' (signed char or unsigned char) or 'h' (short int or unsigned short int)
                                case 'l': // Can be 'll' (long long int or unsigned long long int) or 'l' (long int or unsigned long int)
                                    // If the next character is the same (which makes 'hh' or 'll') then expect another alphabetical character
                                    if (i != formatString.end() && *(i+1) == *i) {
                                        if (i+1 != formatString.end() && !isalpha(*(i+2))) {
                                            std::string modifier;
                                            modifier += *i;
                                            modifier += *(i+1);
                                            invalidLengthModifierError(tok, numFormat, modifier);
                                        }
                                    } else {
                                        if (i != formatString.end() && !isalpha(*(i+1))) {
                                            std::string modifier;
                                            modifier += *i;
                                            invalidLengthModifierError(tok, numFormat, modifier);
                                        }
                                    }
                                    break;
                                case 'j': // intmax_t or uintmax_t
                                case 'z': // size_t
                                case 't': // ptrdiff_t
                                case 'L': // long double
                                    // Expect an alphabetical character after these specifiers
                                    if (i != formatString.end() && !isalpha(*(i+1))) {
                                        std::string modifier;
                                        modifier += *i;
                                        invalidLengthModifierError(tok, numFormat, modifier);
                                    }
                                    break;
                                default:
                                    break;
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

            // Mismatching number of parameters => warning
            if (numFormat != numFunction)
                wrongPrintfScanfArgumentsError(tok, tok->str(), numFormat, numFunction);
        }
    }
}

void CheckIO::wrongPrintfScanfArgumentsError(const Token* tok,
        const std::string &functionName,
        unsigned int numFormat,
        unsigned int numFunction)
{
    Severity::SeverityType severity = numFormat > numFunction ? Severity::error : Severity::warning;
    if (severity != Severity::error && !_settings->isEnabled("style"))
        return;

    std::ostringstream errmsg;
    errmsg << functionName
           << " format string has "
           << numFormat
           << " parameters but "
           << (numFormat > numFunction ? "only " : "")
           << numFunction
           << " are given.";

    reportError(tok, severity, "wrongPrintfScanfArgNum", errmsg.str());
}

void CheckIO::invalidScanfArgTypeError(const Token* tok, const std::string &functionName, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << functionName << " argument no. " << numFormat << ": requires a non-const pointer or array as argument.";
    reportError(tok, Severity::warning, "invalidScanfArgType", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%s in format string (no. " << numFormat << ") requires a char* given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_s", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%n in format string (no. " << numFormat << ") requires a pointer to an non-const integer given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_n", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an address given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_p", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires an integer given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_int", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_uint(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires an unsigned integer given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_uint", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_sint(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires a signed integer given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_sint", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires a floating point number given in the argument list.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_float", errmsg.str());
}
void CheckIO::invalidLengthModifierError(const Token* tok, unsigned int numFormat, std::string& modifier)
{
    std::ostringstream errmsg;
    errmsg << "'" << modifier << "' in format string (no. " << numFormat << ") is a length modifier and cannot be used without a conversion specifier.";
    reportError(tok, Severity::warning, "invalidLengthModifierError", errmsg.str());
}

void CheckIO::invalidScanfFormatWidthError(const Token* tok, unsigned int numFormat, int width, const Variable *var)
{
    std::ostringstream errmsg;
    Severity::SeverityType severity = Severity::warning;
    bool inconclusive = false;

    if (var) {
        if (var->dimension(0) > width) {
            if (!_settings->inconclusive)
                return;
            inconclusive = true;
            errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is smaller than destination buffer"
                   << " '" << var->name() << "[" << var->dimension(0) << "]'.";
        } else {
            errmsg << "Width " << width << " given in format string (no. " << numFormat << ") is larger than destination buffer '"
                   << var->name() << "[" << var->dimension(0) << "]', use %" << (var->dimension(0) - 1) << "s to prevent overflowing it.";
            severity = Severity::error;
        }

    } else
        errmsg << "Width " << width << " given in format string (no. " << numFormat << ") doesn't match destination buffer.";

    if (severity == Severity::error || _settings->isEnabled("style"))
        reportError(tok, severity, "invalidScanfFormatWidth", errmsg.str(), inconclusive);
}
