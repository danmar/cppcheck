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
        if (!var || !var->declarationId() || var->isArray() || !Token::simpleMatch(var->typeStartToken(), "FILE *"))
            continue;

        if (var->isLocal()) {
            if (var->nameToken()->strAt(1) == "(") // initialize by calling "ctor"
                filepointers.insert(std::make_pair(var->declarationId(), Filepointer(UNKNOWN)));
            else
                filepointers.insert(std::make_pair(var->declarationId(), Filepointer(CLOSED)));
        } else {
            filepointers.insert(std::make_pair(var->declarationId(), Filepointer(UNKNOWN)));
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
        return (true);

    static std::set<std::string> knownTypes;
    if (knownTypes.empty()) {
        knownTypes.insert("struct"); // If a type starts with the struct keyword, its a complex type
        knownTypes.insert("string");
        knownTypes.insert("wstring");
    }

    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);
    return ((knownTypes.find(varTypeTok->str()) != knownTypes.end() || (varTypeTok->strAt(1) == "<" && varTypeTok->linkAt(1) && varTypeTok->linkAt(1)->strAt(1) != "::")) && !var->isPointer() && !var->isArray());
}

static bool isKnownType(const Variable* var, const Token* varTypeTok)
{
    return (varTypeTok->isStandardType() || varTypeTok->next()->isStandardType() || isComplexType(var, varTypeTok));
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

            if (Token::Match(tok->next(), "( %any%")) {
                const Token *arg = tok->tokAt(2);
                int argnr = 1;
                while (arg) {
                    if (Token::Match(arg, "%str% [,)]") && _settings->library.isargformatstr(tok->str(),argnr)) {
                        formatString = arg->str();
                        if (arg->strAt(1) == ",")
                            argListTok = arg->tokAt(2);
                        else
                            argListTok = 0;
                        break;
                    }

                    arg = arg->nextArgument();
                    argnr++;
                }
            }

            if (!formatString.empty()) {
                /* formatstring found in library */
            } else if (Token::Match(tok, "printf|scanf|wprintf|wscanf ( %str%")) {
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
                if (Token::Match(formatStringTok, "%str% [,)]")) {
                    argListTok = formatStringTok->nextArgument(); // Find third parameter (first argument of va_args)
                    formatString = formatStringTok->str();
                } else {
                    continue;
                }
            } else if (Token::Match(tok, "snprintf|fnprintf (") || (Token::simpleMatch(tok, "swprintf (") && !Token::Match(tok->tokAt(2)->nextArgument(), "%str%"))) {
                const Token* formatStringTok = tok->tokAt(2);
                for (int i = 0; i < 2 && formatStringTok; i++) {
                    formatStringTok = formatStringTok->nextArgument(); // Find third parameter (format string)
                }
                if (Token::Match(formatStringTok, "%str% [,)]")) {
                    argListTok = formatStringTok->nextArgument(); // Find fourth parameter (first argument of va_args)
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
                    if (i == formatString.end())
                        break;
                } else if (percent) {
                    percent = false;

                    bool _continue = false;
                    std::string width;
                    unsigned int parameterPosition = 0;
                    bool hasParameterPosition = false;
                    while (i != formatString.end() && *i != ']' && !std::isalpha(*i)) {
                        if (*i == '*') {
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
                        const Variable *variableInfo;
                        const Token *varTypeTok;
                        const Function *functionInfo;
                        bool element;

                        if (getArgumentInfo(argListTok, &variableInfo, &varTypeTok, &functionInfo, element)) {
                            if (varTypeTok->str() == "static")
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
                                std::string specifier;
                                bool done = false;
                                while (!done) {
                                    switch (*i) {
                                    case 's':
                                        if (variableInfo && argListTok->type() != Token::eString && isKnownType(variableInfo, varTypeTok) && (!variableInfo->isPointer() && !variableInfo->isArray()))
                                            invalidPrintfArgTypeError_s(tok, numFormat);
                                        done = true;
                                        break;
                                    case 'n':
                                        if ((variableInfo && isKnownType(variableInfo, varTypeTok) && ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->strAt(-1) == "const")) || argListTok->type() == Token::eString)
                                            invalidPrintfArgTypeError_n(tok, numFormat);
                                        done = true;
                                        break;
                                    case 'c':
                                    case 'x':
                                    case 'X':
                                    case 'o':
                                        specifier += *i;
                                        if (functionInfo && ((varTypeTok->isStandardType() || functionInfo->retType) && varTypeTok->next()->str() != "*")) {
                                            if (!Token::Match(varTypeTok, "bool|short|long|int|char|size_t") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_int(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (variableInfo && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray()) {
                                            if (!Token::Match(varTypeTok, "bool|short|long|int|char|size_t") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_int(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (varTypeTok->type() == Token::eString) {
                                            invalidPrintfArgTypeError_int(tok, numFormat, specifier, varTypeTok);
                                        }
                                        done = true;
                                        break;
                                    case 'd':
                                    case 'i':
                                        specifier += *i;
                                        if (functionInfo && (varTypeTok->isStandardType() || functionInfo->retType) && varTypeTok->next()->str() != "*") {
                                            if (((varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "bool|short|long|int")) && varTypeTok->str() != "char") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (variableInfo && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray()) {
                                            if (((varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "bool|short|long|int")) && varTypeTok->str() != "char") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_sint(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (varTypeTok->type() == Token::eString) {
                                            invalidPrintfArgTypeError_sint(tok, numFormat, specifier, varTypeTok);
                                        }
                                        done = true;
                                        break;
                                    case 'u':
                                        specifier += *i;
                                        if (functionInfo && ((varTypeTok->isStandardType() || functionInfo->retType) || varTypeTok->next()->str() != "*")) {
                                            if (((!varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "char|short|long|int|size_t")) && varTypeTok->str() != "bool") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (variableInfo && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray()) {
                                            if (((!varTypeTok->isUnsigned() || !Token::Match(varTypeTok, "char|short|long|int|size_t")) && varTypeTok->str() != "bool") ||
                                                (specifier[0] == 'l' && (varTypeTok->str() != "long" || (specifier[1] == 'l' && !varTypeTok->isLong()))) ||
                                                (specifier.find("I64") != std::string::npos && (varTypeTok->str() != "long" || !varTypeTok->isLong()))) {
                                                invalidPrintfArgTypeError_uint(tok, numFormat, specifier, varTypeTok);
                                            }
                                        } else if (varTypeTok->type() == Token::eString) {
                                            invalidPrintfArgTypeError_uint(tok, numFormat, specifier, varTypeTok);
                                        }
                                        done = true;
                                        break;
                                    case 'p':
                                        if (functionInfo && varTypeTok->type() == Token::eType && varTypeTok->next()->str() != "*")
                                            invalidPrintfArgTypeError_p(tok, numFormat, varTypeTok);
                                        else if (variableInfo && varTypeTok && isKnownType(variableInfo, varTypeTok) && !variableInfo->isPointer() && !variableInfo->isArray())
                                            invalidPrintfArgTypeError_p(tok, numFormat, varTypeTok);
                                        else if (varTypeTok->type() == Token::eString)
                                            invalidPrintfArgTypeError_p(tok, numFormat, varTypeTok);
                                        done = true;
                                        break;
                                    case 'e':
                                    case 'E':
                                    case 'f':
                                    case 'g':
                                    case 'G':
                                        specifier += *i;
                                        if (functionInfo && (((varTypeTok->isStandardType() || functionInfo->retType) && !Token::Match(varTypeTok, "float|double")) ||
                                                             (!element && Token::simpleMatch(varTypeTok->next(), "*")) ||
                                                             (element && !Token::simpleMatch(varTypeTok->next(), "*")) ||
                                                             (specifier[0] == 'l' && (!varTypeTok->isLong() || varTypeTok->str() != "double")) ||
                                                             (specifier[0] != 'l' && varTypeTok->isLong())))
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, varTypeTok);
                                        else if (variableInfo && ((isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "float|double")) ||
                                                                  (!element && variableInfo->isArrayOrPointer()) ||
                                                                  (element && !variableInfo->isArrayOrPointer())))
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, varTypeTok);
                                        else if (varTypeTok->type() == Token::eString)
                                            invalidPrintfArgTypeError_float(tok, numFormat, specifier, varTypeTok);
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

            // Check that all parameter positions reference an actual parameter
            for (std::set<unsigned int>::const_iterator it = parameterPositionsUsed.begin() ; it != parameterPositionsUsed.end() ; ++it) {
                if (((*it == 0) || (*it > numFormat)) && _settings->isEnabled("warning"))
                    wrongPrintfScanfPosixParameterPositionError(tok, tok->str(), *it, numFormat);
            }

            // Mismatching number of parameters => warning
            if (numFormat != numFunction)
                wrongPrintfScanfArgumentsError(tok, tok->str(), numFormat, numFunction);
        }
    }
}

// We currently only support string literals, variables, and functions.
/// @todo add non-string literals, and generic expressions

bool CheckIO::getArgumentInfo(const Token * tok, const Variable **var, const Token **typeTok, const Function **func, bool &element) const
{
    if (tok) {
        if (tok->type() == Token::eString) {
            *var = 0;
            *typeTok = tok;
            *func = 0;
            element = false;
            return true;
        } else if (tok->type() == Token::eVariable || tok->type() == Token::eFunction || Token::Match(tok, "%type% ::")) {
            while (Token::Match(tok, "%type% ::"))
                tok = tok->tokAt(2);
            if (!tok || !(tok->type() == Token::eVariable || tok->type() == Token::eFunction))
                return false;
            const Token *varTok = 0;
            const Token *tok1 = tok->next();
            for (; tok1; tok1 = tok1->next()) {
                if (tok1->str() == "," || tok1->str() == ")") {
                    if (tok1->previous()->str() == "]") {
                        varTok = tok1->linkAt(-1)->previous();
                        if (varTok->str() == ")" && varTok->link()->previous()->type() == Token::eFunction) {
                            const Function * function = varTok->link()->previous()->function();
                            if (function && function->retDef) {
                                *var = 0;
                                *typeTok = function->retDef;
                                *func = function;
                                element = true;
                                return true;
                            } else
                                return false;
                        }
                    } else if (tok1->previous()->str() == ")" && tok1->linkAt(-1)->previous()->type() == Token::eFunction) {
                        const Function * function = tok1->linkAt(-1)->previous()->function();
                        if (function && function->retDef) {
                            *var = 0;
                            *typeTok = function->retDef;
                            *func = function;
                            element = false;
                            return true;
                        } else
                            return false;
                    } else
                        varTok = tok1->previous();
                    break;
                } else if (tok1->str() == "(" || tok1->str() == "{" || tok1->str() == "[")
                    tok1 = tok1->link();
                else if (tok1->str() == "<" && tok1->link())
                    tok1 = tok1->link();
                else if (!(tok1->str() == "." || tok1->type() == Token::eVariable || tok1->type() == Token::eFunction))
                    return false;
            }

            if (varTok) {
                const Variable *variableInfo = varTok->variable();
                *var = variableInfo;
                element = tok1->previous()->str() == "]";
                *func = 0;

                // look for std::vector operator [] and use template type as return type
                if (variableInfo) {
                    if (element && Token::Match(variableInfo->typeStartToken(), "std :: vector|array <")) {
                        *typeTok = variableInfo->typeStartToken()->tokAt(4);
                        element = false;    // not really an array element
                    } else
                        *typeTok = variableInfo->typeStartToken();
                } else
                    return false;

                return true;
            }
        }
    }

    return false;
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

void CheckIO::wrongPrintfScanfPosixParameterPositionError(const Token* tok, const std::string& functionName,
        unsigned int index, unsigned int numFunction)
{
    std::ostringstream errmsg;
    errmsg << functionName << ": ";
    if (index == 0) {
        errmsg << "parameter positions start at 1, not 0";
    } else {
        errmsg << "referencing parameter " << index << " while " << numFunction << " arguments given";
    }
    reportError(tok, Severity::warning, "wrongPrintfScanfParameterPositionError", errmsg.str());
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
void CheckIO::invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat, const Token* type)
{
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an address but the argument type is \'";
    if (type) {
        if (type->type() == Token::eString) {
            if (type->isLong())
                errmsg << "const wchar_t *";
            else
                errmsg << "const char *";
        } else {
            type->stringify(errmsg, false, true);
            if (type->strAt(1) == "*")
                errmsg << " *";
        }
    } else
        errmsg << "Unknown";
    errmsg << "\'.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_p", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, const std::string& specifier, const Token* type)
{
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires a";
    if (specifier.find("I64") != std::string::npos)
        errmsg << " long long ";
    else
        errmsg << (specifier[0] == 'l' ? " long " : "n ")
               << (specifier[0] == 'l' && specifier[1] == 'l' ? "long " : "");
    errmsg << "integer but the argument type is \'";
    if (type) {
        if (type->type() == Token::eString) {
            if (type->isLong())
                errmsg << "const wchar_t *";
            else
                errmsg << "const char *";
        } else {
            if (type->originalName().empty()) {
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
            } else {
                if ((type->originalName() == "__int64" || type->originalName() == "__int32") && type->isUnsigned())
                    errmsg << "unsigned ";
                errmsg << type->originalName() << " {aka ";
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
                errmsg << "}";
            }
        }
    } else
        errmsg << "Unknown";
    errmsg << "\'.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_int", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_uint(const Token* tok, unsigned int numFormat, const std::string& specifier, const Token* type)
{
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires an unsigned ";
    if (specifier.find("I64") != std::string::npos)
        errmsg << "long long ";
    else
        errmsg << (specifier[0] == 'l' ? "long " : "")
               << (specifier[0] == 'l' && specifier[1] == 'l' ? "long " : "");
    errmsg << "integer but the argument type is \'";
    if (type) {
        if (type->type() == Token::eString) {
            if (type->isLong())
                errmsg << "const wchar_t *";
            else
                errmsg << "const char *";
        } else {
            if (type->originalName().empty()) {
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
            } else {
                if ((type->originalName() == "__int64" || type->originalName() == "__int32") && type->isUnsigned())
                    errmsg << "unsigned ";
                errmsg << type->originalName() << " {aka ";
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
                errmsg << "}";
            }
        }
    } else
        errmsg << "Unknown";
    errmsg << "\'.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_uint", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_sint(const Token* tok, unsigned int numFormat, const std::string& specifier, const Token* type)
{
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires a signed ";
    if (specifier.find("I64") != std::string::npos)
        errmsg << "long long ";
    else
        errmsg << (specifier[0] == 'l' ? "long " : "")
               << (specifier[0] == 'l' && specifier[1] == 'l' ? "long " : "");
    errmsg << "integer but the argument type is \'";
    if (type) {
        if (type->type() == Token::eString) {
            if (type->isLong())
                errmsg << "const wchar_t *";
            else
                errmsg << "const char *";
        } else {
            if (type->originalName().empty()) {
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
            } else {
                if ((type->originalName() == "__int64" || type->originalName() == "__int32") && type->isUnsigned())
                    errmsg << "unsigned ";
                errmsg << type->originalName() << " {aka ";
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
                errmsg << "}";
            }
        }
    } else
        errmsg << "Unknown";
    errmsg << "\'.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_sint", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, const std::string& specifier, const Token* type)
{
    std::ostringstream errmsg;
    errmsg << "%" << specifier << " in format string (no. " << numFormat << ") requires a floating point number but the argument type is \'";
    if (type) {
        if (type->type() == Token::eString) {
            if (type->isLong())
                errmsg << "const wchar_t *";
            else
                errmsg << "const char *";
        } else {
            if (type->originalName().empty()) {
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
            } else {
                if ((type->originalName() == "__int64" || type->originalName() == "__int32") && type->isUnsigned())
                    errmsg << "unsigned ";
                errmsg << type->originalName() << " {aka ";
                type->stringify(errmsg, false, true);
                if (type->strAt(1) == "*")
                    errmsg << " *";
                errmsg << "}";
            }
        }
    } else
        errmsg << "Unknown";
    errmsg << "\'.";
    reportError(tok, Severity::warning, "invalidPrintfArgType_float", errmsg.str());
}
void CheckIO::invalidLengthModifierError(const Token* tok, unsigned int numFormat, const std::string& modifier)
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
