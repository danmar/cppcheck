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
#include "checkunusedfunctions.h"
#include "tokenize.h"
#include "token.h"
#include <cctype>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

void CheckUnusedFunctions::parseTokens(const Tokenizer &tokenizer)
{
    // Function declarations..
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
        if (tok->fileIndex() != 0)
            continue;

        // token contains a ':' => skip to next ; or {
        if (tok->str().find(":") != std::string::npos) {
            while (tok && tok->str().find_first_of(";{"))
                tok = tok->next();
            if (tok)
                continue;
            break;
        }

        // If this is a template function, skip it
        if (tok->previous() && tok->previous()->str() == ">")
            continue;

        const Token *funcname = 0;

        if (Token::Match(tok, "%type% %var% ("))
            funcname = tok->next();
        else if (Token::Match(tok, "%type% *|& %var% ("))
            funcname = tok->tokAt(2);
        else if (Token::Match(tok, "%type% :: %var% (") && !Token::Match(tok, tok->strAt(2).c_str()))
            funcname = tok->tokAt(2);

        // Don't assume throw as a function name: void foo() throw () {}
        if (Token::Match(tok->previous(), ")|const") || funcname == 0)
            continue;

        tok = funcname->linkAt(1);

        // Check that ") {" is found..
        if (! Token::Match(tok, ") const| {") &&
            ! Token::Match(tok, ") const| throw ( ) {"))
            funcname = 0;

        if (funcname) {
            FunctionUsage &func = _functions[ funcname->str()];

            if (!func.lineNumber)
                func.lineNumber = funcname->linenr();

            // No filename set yet..
            if (func.filename.empty()) {
                func.filename = tokenizer.getSourceFilePath();
            }
            // Multiple files => filename = "+"
            else if (func.filename != tokenizer.getSourceFilePath()) {
                //func.filename = "+";
                func.usedOtherFile |= func.usedSameFile;
            }
        }
    }

    // Function usage..
    const Token *scopeEnd = NULL;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
        if (scopeEnd == NULL) {
            if (!Token::Match(tok, ")|= const| {"))
                continue;
            scopeEnd = tok;
            while (scopeEnd->str() != "{")
                scopeEnd = scopeEnd->next();
            scopeEnd = scopeEnd->link();
        } else if (tok == scopeEnd) {
            scopeEnd = NULL;
            continue;
        }


        const Token *funcname = 0;

        if (Token::Match(tok->next(), "%var% (")) {
            funcname = tok->next();
        }

        else if (Token::Match(tok, "[;{}.,()[=+-/&|!?:] %var% [(),;:}]"))
            funcname = tok->next();

        else if (Token::Match(tok, "[=(,] &| %var% :: %var%")) {
            funcname = tok->next();
            if (funcname->str() == "&")
                funcname = funcname->next();
            while (Token::Match(funcname,"%var% :: %var%"))
                funcname = funcname->tokAt(2);
            if (!Token::Match(funcname, "%var% [,);]"))
                continue;
        }

        else
            continue;

        // funcname ( => Assert that the end parentheses isn't followed by {
        if (Token::Match(funcname, "%var% (")) {
            if (Token::Match(funcname->linkAt(1), ") const|throw|{"))
                funcname = NULL;
        }

        if (funcname) {
            FunctionUsage &func = _functions[ funcname->str()];

            if (func.filename.empty() || func.filename == "+")
                func.usedOtherFile = true;
            else
                func.usedSameFile = true;
        }
    }
}




void CheckUnusedFunctions::check(ErrorLogger * const errorLogger)
{
    for (std::map<std::string, FunctionUsage>::const_iterator it = _functions.begin(); it != _functions.end(); ++it) {
        const FunctionUsage &func = it->second;
        if (func.usedOtherFile || func.filename.empty())
            continue;
        if (it->first == "main" ||
            it->first == "WinMain" ||
            it->first == "_tmain" ||
            it->first == "if" ||
            (it->first.compare(0, 8, "operator") == 0 && it->first.size() > 8 && !std::isalnum(it->first[8])))
            continue;
        if (! func.usedSameFile) {
            std::string filename;
            if (func.filename == "+")
                filename = "";
            else
                filename = func.filename;
            unusedFunctionError(errorLogger, filename, func.lineNumber, it->first);
        } else if (! func.usedOtherFile) {
            /** @todo add error message "function is only used in <file> it can be static" */
            /*
            std::ostringstream errmsg;
            errmsg << "The function '" << it->first << "' is only used in the file it was declared in so it should have local linkage.";
            _errorLogger->reportErr( errmsg.str() );
            */
        }
    }
}

void CheckUnusedFunctions::unusedFunctionError(ErrorLogger * const errorLogger,
        const std::string &filename, unsigned int lineNumber,
        const std::string &funcname)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        ErrorLogger::ErrorMessage::FileLocation fileLoc;
        fileLoc.setfile(filename);
        fileLoc.line = lineNumber;
        locationList.push_back(fileLoc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList, Severity::style, "The function '" + funcname + "' is never used.", "unusedFunction", false);
    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}
