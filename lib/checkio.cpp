/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
    bool firstCout = false;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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

void CheckIO::coutCerrMisusageError(const Token* tok, const std::string& streamName)
{
    reportError(tok, Severity::error, "coutCerrMisusage", "Invalid usage of output stream: '<< std::" + streamName + "'.");
}

//---------------------------------------------------------------------------
// fflush(stdin) <- fflush only applies to output streams in ANSI C
// fread(); fwrite(); <- consecutive read/write statements require repositioning inbetween
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
    unsigned int varListSize = symbolDatabase->getVariableListSize();
    for (unsigned int i = 1; i < varListSize; i++) {
        const Variable* var = symbolDatabase->getVariableFromVarId(i);
        if (!var || !var->varId() || !Token::Match(var->typeStartToken(), "FILE *"))
            continue;

        if (var->isLocal())
            filepointers.insert(std::make_pair(var->varId(), Filepointer(CLOSED)));
        else {
            filepointers.insert(std::make_pair(var->varId(), Filepointer(UNKNOWN)));
            // TODO: If all fopen calls we find open the file in the same type, we can set Filepointer::mode
        }
    }

    unsigned int indent = 0;
    for (const Token* tok = _tokenizer->list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            indent++;
        else if (tok->str() == "}") {
            indent--;
            for (std::map<unsigned int, Filepointer>::iterator i = filepointers.begin(); i != filepointers.end(); i++) {
                if (indent < i->second.mode_indent) {
                    i->second.mode_indent = 0;
                    i->second.mode = UNKNOWN;
                }
                if (indent < i->second.op_indent) {
                    i->second.op_indent = 0;
                    i->second.lastOperation = Filepointer::UNKNOWN_OP;
                }
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
                if (Token::Match(tok, "fflush ( stdin )"))
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
            } else if (!Token::Match(tok, "if|for|while|catch|return")) {
                const Token* const end2 = tok->linkAt(1);
                for (const Token* tok2 = tok->tokAt(2); tok2 != end2; tok2 = tok2->next()) {
                    if (tok2->varId() && filepointers.find(tok2->varId()) != filepointers.end()) {
                        fileTok = tok2;
                        operation = Filepointer::UNKNOWN_OP; // Assume that repositioning was last operation and that the file is opened now
                        break;
                    }
                }
            }

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
}

void CheckIO::fflushOnInputStreamError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::error,
                "fflushOnInputStream", "fflush() called on input stream \"" + varname + "\" may result in undefined behaviour.");
}

void CheckIO::ioWithoutPositioningError(const Token *tok)
{
    reportError(tok, Severity::error,
                "IOWithoutPositioning", "Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush inbetween result in undefined behaviour.");
}

void CheckIO::readWriteOnlyFileError(const Token *tok)
{

    reportError(tok, Severity::error,
                "readWriteOnlyFile", "Read operation on a file that was only opened for writing.");
}

void CheckIO::writeReadOnlyFileError(const Token *tok)
{
    reportError(tok, Severity::error,
                "writeReadOnlyFile", "Write operation on a file that was only opened for reading.");
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
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
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

            else if (std::isdigit(formatstr[i])) {
                format = false;
            }

            else if (std::isalpha(formatstr[i])) {
                if (formatstr[i] != 'c')  // #3490 - field width limits are not necessary for %c
                    invalidScanfError(tok);
                format = false;
            }
        }
    }
}

void CheckIO::invalidScanfError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "invalidscanf", "scanf without field width limits can crash with huge input data\n"
                "scanf without field width limits can crash with huge input data. To fix this error "
                "message add a field width specifier:\n"
                "    %s => %20s\n"
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
    }

    if (varTypeTok->str() == "std")
        varTypeTok = varTypeTok->tokAt(2);
    return(knownTypes.find(varTypeTok->str()) != knownTypes.end() && !var->isPointer() && !var->isArray());
}

static bool isKnownType(const Variable* var, const Token* varTypeTok)
{
    return(varTypeTok->isStandardType() || varTypeTok->next()->isStandardType() || isComplexType(var, varTypeTok));
}

void CheckIO::checkWrongPrintfScanfArguments()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isName()) continue;

        const Token* argListTok = 0; // Points to first va_list argument
        std::string formatString;

        if (Token::Match(tok, "printf|scanf ( %str%")) {
            formatString = tok->strAt(2);
            if (tok->strAt(3) == ",") {
                argListTok = tok->tokAt(4);
            } else if (tok->strAt(3) == ")") {
                argListTok = 0;
            } else {
                continue;
            }
        } else if (Token::Match(tok, "sprintf|fprintf|sscanf|fscanf ( %any%")) {
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
        } else if (Token::Match(tok, "snprintf|fnprintf (")) {
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
        bool scan = Token::Match(tok, "sscanf|fscanf|scanf");
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
                while (i != formatString.end() && *i != ']' && !std::isalpha(*i)) {
                    if (*i == '*') {
                        if (scan)
                            _continue = true;
                        else {
                            numFormat++;
                            if (argListTok)
                                argListTok = argListTok->nextArgument();
                        }
                    }
                    ++i;
                }
                if (i == formatString.end())
                    break;
                if (_continue)
                    continue;

                if (scan || *i != 'm') { // %m is a non-standard extension that requires no parameter on print functions.
                    numFormat++;

                    // Perform type checks
                    if (_settings->isEnabled("style") && argListTok && Token::Match(argListTok->next(), "[,)]")) { // We can currently only check the type of arguments matching this simple pattern.
                        const Variable* variableInfo = symbolDatabase->getVariableFromVarId(argListTok->varId());
                        const Token* varTypeTok = variableInfo ? variableInfo->typeStartToken() : NULL;
                        if (varTypeTok && varTypeTok->str() == "static")
                            varTypeTok = varTypeTok->next();

                        if (scan && varTypeTok) {
                            if ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->str() == "const")
                                invalidScanfArgTypeError(tok, tok->str(), numFormat);
                        } else if (!scan) {
                            switch (*i) {
                            case 's':
                                if (variableInfo && argListTok->type() != Token::eString && isKnownType(variableInfo, varTypeTok) && (!variableInfo->isPointer() && !variableInfo->isArray()))
                                    invalidPrintfArgTypeError_s(tok, numFormat);
                                break;
                            case 'n':
                                if ((varTypeTok && isKnownType(variableInfo, varTypeTok) && ((!variableInfo->isPointer() && !variableInfo->isArray()) || varTypeTok->str() == "const")) || argListTok->type() == Token::eString)
                                    invalidPrintfArgTypeError_n(tok, numFormat);
                                break;
                            case 'c':
                            case 'd':
                            case 'i':
                            case 'u':
                            case 'x':
                            case 'X':
                            case 'o':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if ((varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "unsigned|signed| bool|short|long|int|char|size_t|unsigned|signed") && !variableInfo->isPointer() && !variableInfo->isArray()))
                                    invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                else if (argListTok->type() == Token::eString)
                                    invalidPrintfArgTypeError_int(tok, numFormat, *i);
                                break;
                            case 'p':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if (varTypeTok && isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "unsigned|signed| short|long|int|size_t|unsigned|signed") && !variableInfo->isPointer() && !variableInfo->isArray())
                                    invalidPrintfArgTypeError_p(tok, numFormat);
                                else if (argListTok->type() == Token::eString)
                                    invalidPrintfArgTypeError_p(tok, numFormat);
                                break;
                            case 'e':
                            case 'E':
                            case 'f':
                            case 'g':
                            case 'G':
                                if (varTypeTok && varTypeTok->str() == "const")
                                    varTypeTok = varTypeTok->next();
                                if (varTypeTok && ((isKnownType(variableInfo, varTypeTok) && !Token::Match(varTypeTok, "float|double")) || variableInfo->isPointer() || variableInfo->isArray()))
                                    invalidPrintfArgTypeError_float(tok, numFormat, *i);
                                else if (argListTok->type() == Token::eString)
                                    invalidPrintfArgTypeError_float(tok, numFormat, *i);
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
           << " are given";

    reportError(tok, severity, "wrongPrintfScanfArgNum", errmsg.str());
}

void CheckIO::invalidScanfArgTypeError(const Token* tok, const std::string &functionName, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << functionName << " argument no. " << numFormat << ": requires non-const pointers or arrays as arguments";
    reportError(tok, Severity::warning, "invalidScanfArgType", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%s in format string (no. " << numFormat << ") requires a char* given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_s", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%n in format string (no. " << numFormat << ") requires a pointer to an non-const integer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_n", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat)
{
    std::ostringstream errmsg;
    errmsg << "%p in format string (no. " << numFormat << ") requires an integer or pointer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_p", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires an integer given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_int", errmsg.str());
}
void CheckIO::invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, char c)
{
    std::ostringstream errmsg;
    errmsg << "%" << c << " in format string (no. " << numFormat << ") requires a floating point number given in the argument list";
    reportError(tok, Severity::warning, "invalidPrintfArgType_float", errmsg.str());
}
