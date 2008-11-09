/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


//---------------------------------------------------------------------------
#include "CheckHeaders.h"
#include "tokenize.h"
#include "CommonCheck.h"
#include <algorithm>
#include <list>
#include <sstream>
#include <string>
#include <cstring>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// HEADERS - No implementation in a header
//---------------------------------------------------------------------------

void WarningHeaderWithImplementation()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Only interested in included file
        if (tok->FileIndex == 0)
            continue;

        if (Match(tok, ") {"))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found implementation in header";
            ReportErr(ostr.str());

            // Goto next file..
            unsigned int fileindex = tok->FileIndex;
            while ( tok->next && tok->FileIndex == fileindex )
                tok = tok->next;
        }
    }
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// HEADERS - Unneeded include
//---------------------------------------------------------------------------

void WarningIncludeHeader()
{
    // Including..
    for (TOKEN *includetok = tokens; includetok; includetok = includetok->next)
    {
        if (strcmp(includetok->str, "#include") != 0)
            continue;

        // Get fileindex of included file..
        unsigned int hfile = 0;
        const char *includefile = includetok->next->str;
        while (hfile < Files.size())
        {
            if ( SameFileName( Files[hfile].c_str(), includefile ) )
                break;
            hfile++;
        }
        if (hfile == Files.size())
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
        for ( TOKEN *tok1 = tokens; tok1; tok1 = tok1->next )
        {
            if ( tok1->FileIndex != hfile )
                continue;

            // I'm only interested in stuff that is declared at indentlevel 0
            if (tok1->str[0] == '{')
                indentlevel++;

            else if (tok1->str[0] == '}')
                indentlevel--;

            if (indentlevel != 0)
                continue;

            // Class or namespace declaration..
            // --------------------------------------
            if (Match(tok1,"class %var% {") || Match(tok1,"class %var% :") || Match(tok1,"namespace %var% {"))
                classlist.push_back(Tokenizer::getstr(tok1, 1));

            // Variable declaration..
            // --------------------------------------
            else if (Match(tok1, "%type% %var% ;") || Match(tok1, "%type% %var% ["))
                namelist.push_back(Tokenizer::getstr(tok1, 1));

            else if (Match(tok1, "%type% * %var% ;") || Match(tok1, "%type% * %var% ["))
                namelist.push_back(Tokenizer::getstr(tok1, 2));

            else if (Match(tok1, "const %type% %var% =") || Match(tok1, "const %type% %var% ["))
                namelist.push_back(Tokenizer::getstr(tok1, 2));

            else if (Match(tok1, "const %type% * %var% =") || Match(tok1, "const %type% * %var% ["))
                namelist.push_back(Tokenizer::getstr(tok1, 3));

            // enum..
            // --------------------------------------
            else if (strcmp(tok1->str, "enum") == 0)
            {
                tok1 = tok1->next;
                while (tok1->next && tok1->str[0]!=';')
                {
                    if ( IsName(tok1->str) )
                        namelist.push_back(tok1->str);
                    tok1 = tok1->next;
                }
            }

            // function..
            // --------------------------------------
            else if (Match(tok1,"%type% %var% ("))
                namelist.push_back(Tokenizer::getstr(tok1, 1));

            else if (Match(tok1,"%type% * %var% ("))
                namelist.push_back(Tokenizer::getstr(tok1, 2));

            else if (Match(tok1,"const %type% %var% ("))
                namelist.push_back(Tokenizer::getstr(tok1, 2));

            else if (Match(tok1,"const %type% * %var% ("))
                namelist.push_back(Tokenizer::getstr(tok1, 3));

            // typedef..
            // --------------------------------------
            else if (strcmp(tok1->str,"typedef")==0)
            {
                if (strcmp(Tokenizer::getstr(tok1,1),"enum")==0)
                    continue;
                int parlevel = 0;
                while (tok1->next)
                {
                    if ( strchr("({", tok1->str[0]) )
                        parlevel++;

                    else if ( strchr(")}", tok1->str[0]) )
                        parlevel--;

                    else if (parlevel == 0)
                    {
                        if ( tok1->str[0] == ';' )
                            break;

                        if ( Match(tok1, "%var% ;") )
                            namelist.push_back(tok1->str);
                    }

                    tok1 = tok1->next;
                }
            }
        }


        // Check if the extracted names are used...
        bool Needed = false;
        bool NeedDeclaration = false;
        for (TOKEN *tok1 = tokens; tok1; tok1 = tok1->next)
        {
            if (tok1->FileIndex != includetok->FileIndex)
                continue;

            if ( Match(tok1, ": %var% {") || Match(tok1, ": %type% %var% {") )
            {
                std::string classname = Tokenizer::getstr(tok1, (strcmp(Tokenizer::getstr(tok1,2),"{")) ? 2 : 1);
                if (std::find(classlist.begin(),classlist.end(),classname)!=classlist.end())
                {
                    Needed = true;
                    break;
                }
            }

            if ( ! IsName(tok1->str) )
                continue;

            if (std::find(namelist.begin(),namelist.end(),tok1->str ) != namelist.end())
            {
                Needed = true;
                break;
            }

            if ( ! NeedDeclaration )
                NeedDeclaration = (std::find(classlist.begin(),classlist.end(),tok1->str ) != classlist.end());
        }


        // Not a header file?
        if (includetok->FileIndex == 0)
            Needed |= NeedDeclaration;

        // Not needed!
        if (!Needed)
        {
            std::ostringstream ostr;
            ostr << FileLine(includetok) << ": The included header '" << includefile << "' is not needed";
            if (NeedDeclaration)
                ostr << " (but a forward declaration is needed)";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



