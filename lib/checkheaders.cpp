/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include "checkheaders.h"
#include "tokenize.h"
#include "filelister.h"
#include "token.h"

#include <algorithm>
#include <list>
#include <sstream>
#include <string>
#include <cstring>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// HEADERS - No implementation in a header
//---------------------------------------------------------------------------

CheckHeaders::CheckHeaders(const Tokenizer *tokenizer, ErrorLogger *errorLogger)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckHeaders::~CheckHeaders()
{

}

void CheckHeaders::warningHeaderWithImplementation()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Only interested in included file
        if (tok->fileIndex() == 0)
            continue;

        if (Token::simpleMatch(tok, ") {"))
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Found implementation in header";

            // TODO, this check is currently not used, but if it is some day
            // it should give correct id and severity by calling proper function
            // from errorLogger. It should not call reportErr directly.
//            std::list<ErrorLogger::ErrorMessage::FileLocation> empty;
            //          empty.push_back(FileLocation());
            // _errorLogger->reportErr(empty, "severity", ostr.str(), "id");
            // Goto next file..
            unsigned int fileindex = tok->fileIndex();
            while (tok->next() && tok->fileIndex() == fileindex)
                tok = tok->next();
        }
    }
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// HEADERS - Unneeded include
//---------------------------------------------------------------------------

void CheckHeaders::warningIncludeHeader()
{
    // Including..
    for (const Token *includetok = _tokenizer->tokens(); includetok; includetok = includetok->next())
    {
        if (includetok->str() != "#include")
            continue;

        // Get fileindex of included file..
        unsigned int hfile = 0;
        const std::string includefile = includetok->strAt(1);
        while (hfile < _tokenizer->getFiles()->size())
        {
            if (getFileLister()->sameFileName(_tokenizer->getFiles()->at(hfile), includefile))
                break;
            ++hfile;
        }
        if (hfile == _tokenizer->getFiles()->size())
            continue;

        // This header is needed if:
        // * It contains some needed class declaration
        // * It contains some needed function declaration
        // * It contains some needed constant value
        // * It contains some needed variable
        // * It contains some needed enum

        std::list<std::string> classlist;
        std::list<std::string> namelist;

        // Extract classes and names in the header..
        int indentlevel = 0;
        for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
        {
            if (tok1->fileIndex() != hfile)
                continue;

            // I'm only interested in stuff that is declared at indentlevel 0
            if (tok1->str() == "{")
                ++indentlevel;

            else if (tok1->str() == "}")
                --indentlevel;

            if (indentlevel != 0)
                continue;

            // Class or namespace declaration..
            // --------------------------------------
            if (Token::Match(tok1, "class %var% {") || Token::Match(tok1, "class %var% :") || Token::Match(tok1, "namespace %var% {"))
                classlist.push_back(tok1->strAt(1));

            // Variable declaration..
            // --------------------------------------
            else if (Token::Match(tok1, "%type% %var% ;") || Token::Match(tok1, "%type% %var% ["))
                namelist.push_back(tok1->strAt(1));

            else if (Token::Match(tok1, "%type% * %var% ;") || Token::Match(tok1, "%type% * %var% ["))
                namelist.push_back(tok1->strAt(2));

            else if (Token::Match(tok1, "const %type% %var% =") || Token::Match(tok1, "const %type% %var% ["))
                namelist.push_back(tok1->strAt(2));

            else if (Token::Match(tok1, "const %type% * %var% =") || Token::Match(tok1, "const %type% * %var% ["))
                namelist.push_back(tok1->strAt(3));

            // enum..
            // --------------------------------------
            else if (tok1->str() == "enum")
            {
                tok1 = tok1->next();
                while (! Token::Match(tok1, "; %any%"))
                {
                    if (tok1->isName())
                        namelist.push_back(tok1->str());
                    tok1 = tok1->next();
                }
            }

            // function..
            // --------------------------------------
            else if (Token::Match(tok1, "%type% %var% ("))
                namelist.push_back(tok1->strAt(1));

            else if (Token::Match(tok1, "%type% * %var% ("))
                namelist.push_back(tok1->strAt(2));

            else if (Token::Match(tok1, "const %type% %var% ("))
                namelist.push_back(tok1->strAt(2));

            else if (Token::Match(tok1, "const %type% * %var% ("))
                namelist.push_back(tok1->strAt(3));

            // typedef..
            // --------------------------------------
            else if (tok1->str() == "typedef")
            {
                if (tok1->strAt(1) == "enum")
                    continue;
                int parlevel = 0;
                while (tok1->next())
                {
                    if (Token::Match(tok1, "[({]"))
                        ++parlevel;

                    else if (Token::Match(tok1, "[)}]"))
                        --parlevel;

                    else if (parlevel == 0)
                    {
                        if (tok1->str() == ";")
                            break;

                        if (Token::Match(tok1, "%var% ;"))
                            namelist.push_back(tok1->str());
                    }

                    tok1 = tok1->next();
                }
            }
        }


        // Check if the extracted names are used...
        bool Needed = false;
        bool NeedDeclaration = false;
        for (const Token *tok1 = _tokenizer->tokens(); tok1; tok1 = tok1->next())
        {
            if (tok1->fileIndex() != includetok->fileIndex())
                continue;

            if (Token::Match(tok1, ": %var% {") || Token::Match(tok1, ": %type% %var% {"))
            {
                const std::string classname = tok1->strAt(((tok1->strAt(2) != "{")) ? 2 : 1);
                if (std::find(classlist.begin(), classlist.end(), classname) != classlist.end())
                {
                    Needed = true;
                    break;
                }
            }

            if (! tok1->isName())
                continue;

            if (std::find(namelist.begin(), namelist.end(), tok1->str().c_str()) != namelist.end())
            {
                Needed = true;
                break;
            }

            if (! NeedDeclaration)
                NeedDeclaration = (std::find(classlist.begin(), classlist.end(), tok1->str().c_str()) != classlist.end());
        }


        // Not a header file?
        if (includetok->fileIndex() == 0)
            Needed |= NeedDeclaration;

        // Not needed!
        if (!Needed)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(includetok) << ": The included header '" << includefile << "' is not needed";
            if (NeedDeclaration)
                ostr << " (but a forward declaration is needed)";

            // TODO, this check is currently not used, but if it is some day
            // it should give correct id and severity by calling proper function
            // from errorLogger. It should not call reportErr directly.
//            std::list<FileLocation> empty;
            //          empty.push_back(FileLocation());
            // _errorLogger->reportErr(empty, "severity", ostr.str(), "id"); // TODO
        }
    }
}
//---------------------------------------------------------------------------



