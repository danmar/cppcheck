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
#include "tokenize.h"

//---------------------------------------------------------------------------

#include <locale>
#include <fstream>


#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
#include <stdlib.h>     // <- strtoul
#include <stdio.h>

#ifdef __BORLANDC__
#include <ctype.h>
#include <mem.h>
#endif

#ifndef _MSC_VER
#define _strdup(str) strdup(str)
#endif



//---------------------------------------------------------------------------

Tokenizer::Tokenizer()
{
    _tokens = 0;
    tokens_back = 0;
    dsymlist = 0;
}

Tokenizer::~Tokenizer()
{
    DeallocateTokens();
}

//---------------------------------------------------------------------------

// Helper functions..

TOKEN *Tokenizer::_gettok(TOKEN *tok, int index)
{
    while (tok && index>0)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}

//---------------------------------------------------------------------------

const TOKEN *Tokenizer::tokens() const
{
    return _tokens;
}

//---------------------------------------------------------------------------
// Defined symbols.
// "#define abc 123" will create a defined symbol "abc" with the value 123
//---------------------------------------------------------------------------



const std::vector<std::string> *Tokenizer::getFiles() const
{
    return &Files;
}

void Tokenizer::Define(const char Name[], const char Value[])
{
    if (!(Name && Name[0]))
        return;

    if (!(Value && Value[0]))
        return;

    // Is 'Value' a decimal value..
    bool dec = true, hex = true;
    for (int i = 0; Value[i]; i++)
    {
        if ( ! isdigit(Value[i]) )
            dec = false;

        if ( ! isxdigit(Value[i]) && (!(i==1 && Value[i]=='x')))
            hex = false;
    }

    if (!dec && !hex)
        return;

    char *strValue = _strdup(Value);

    if (!dec && hex)
    {
		// Convert Value from hexadecimal to decimal
		unsigned long value;
		std::istringstream istr(Value+2);
		istr >> std::hex >> value;
		std::ostringstream ostr;
		ostr << value;
        free(strValue);
        strValue = _strdup(ostr.str().c_str());
    }

    DefineSymbol *NewSym = new DefineSymbol;
    memset(NewSym, 0, sizeof(DefineSymbol));
    NewSym->name = _strdup(Name);
    NewSym->value = strValue;
    NewSym->next = dsymlist;
    dsymlist = NewSym;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

void Tokenizer::addtoken(const char str[], const unsigned int lineno, const unsigned int fileno)
{
    if (str[0] == 0)
        return;

    // Replace hexadecimal value with decimal
	std::ostringstream str2;
	if (strncmp(str,"0x",2)==0)
    {
        str2 << strtoul(str+2, NULL, 16);
    }
	else
	{
		str2 << str;
	}

    TOKEN *newtoken  = new TOKEN;
    newtoken->setstr(str2.str().c_str());
    newtoken->linenr = lineno;
    newtoken->FileIndex = fileno;
    if (tokens_back)
    {
        tokens_back->next = newtoken;
        tokens_back = newtoken;
    }
    else
    {
        _tokens = tokens_back = newtoken;
    }

    // Check if str is defined..
    for (DefineSymbol *sym = dsymlist; sym; sym = sym->next)
    {
        if (strcmp(str,sym->name)==0)
        {
            newtoken->setstr(sym->value);
            break;
        }
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------



int Tokenizer::SizeOfType(const char type[]) const
{
    if (!type)
        return 0;

    std::map<std::string, unsigned int>::const_iterator it = TypeSize.find(type);
    if ( it == TypeSize.end() )
        return 0;

    return it->second;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void Tokenizer::InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n)
{
    while (n > 0)
    {
        TOKEN *NewToken = new TOKEN;
        NewToken->FileIndex = src->FileIndex;
        NewToken->linenr = src->linenr;
        NewToken->setstr(src->str);

        NewToken->next = dest->next;
        dest->next = NewToken;

        dest = dest->next;
        src  = src->next;
        n--;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Tokenize - tokenizes a given file.
//---------------------------------------------------------------------------

void Tokenizer::Tokenize(std::istream &code, const char FileName[])
{
    // Has this file been tokenized already?
    for (unsigned int i = 0; i < Files.size(); i++)
    {
        if ( SameFileName( Files[i].c_str(), FileName ) )
            return;
    }

    // The "Files" vector remembers what files have been tokenized..
    Files.push_back(FileName);

    // Tokenize the file..
    TokenizeCode( code, (unsigned int)(Files.size() - 1) );
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Tokenize - tokenizes input stream
//---------------------------------------------------------------------------

void Tokenizer::TokenizeCode(std::istream &code, const unsigned int FileIndex)
{
    // Tokenize the file.
    unsigned int lineno = 1;
    std::string CurrentToken;
    for (char ch = (char)code.get(); code.good(); ch = (char)code.get())
    {
		// Todo
		if ( ch < 0 )
			continue;

        // Preprocessor stuff?
        if (ch == '#' && CurrentToken.empty())
        {
            std::string line("#");
            {
            char chPrev = '#';
            while ( code.good() )
            {
                ch = (char)code.get();
                if (chPrev!='\\' && ch=='\n')
                    break;
                if (ch!=' ')
                    chPrev = ch;
                if (ch!='\\' && ch!='\n')
                    line += ch;
                if (ch=='\n')
                    ++lineno;
            }
            }
            if (strncmp(line.c_str(),"#include",8)==0 &&
                line.find("\"") != std::string::npos)
            {
                // Extract the filename
                line.erase(0, line.find("\"")+1);
                line.erase(line.find("\""));

                // Relative path..
                if (Files.back().find_first_of("\\/") != std::string::npos)
                {
                    std::string path = Files.back();
                    path.erase( 1 + path.find_last_of("\\/") );
                    line = path + line;
                }

                addtoken("#include", lineno, FileIndex);
                addtoken(line.c_str(), lineno, FileIndex);

                std::ifstream fin( line.c_str() );
                Tokenize(fin, line.c_str());
            }

            else if (strncmp(line.c_str(), "#define", 7) == 0)
            {
                std::string strId;
                enum {Space1, Id, Space2, Value} State;
                State = Space1;
                for (unsigned int i = 8; i < line.length(); i++)
                {
                    if (State==Space1 || State==Space2)
                    {
                        if (isspace(line[i]))
                            continue;
                        State = (State==Space1) ? Id : Value;
                    }

                    else if (State==Id)
                    {
                        if ( isspace( line[i] ) )
                        {
                            strId = CurrentToken;
                            CurrentToken.clear();
                            State = Space2;
                            continue;
                        }
                        else if ( ! isalnum(line[i]) )
                        {
                            break;
                        }
                    }

                    CurrentToken += line[i];
                }

                if (State==Value)
                {
                    addtoken("def", lineno, FileIndex);
                    addtoken(strId.c_str(), lineno, FileIndex);
                    addtoken(";", lineno, FileIndex);
                    Define(strId.c_str(), CurrentToken.c_str());
                }

                CurrentToken.clear();
            }

            else
            {
                addtoken("#", lineno, FileIndex);
                addtoken(";", lineno, FileIndex);
            }

            lineno++;
            continue;
        }

        if (ch == '\n')
        {
            // Add current token..
            addtoken(CurrentToken.c_str(), lineno++, FileIndex);
            CurrentToken.clear();
            continue;
        }

        // Comments..
        if (ch == '/' && code.good())
        {
            bool newstatement = bool( strchr(";{}", CurrentToken.empty() ? '\0' : CurrentToken[0]) != NULL );

            // Add current token..
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();

            // Read next character..
            ch = (char)code.get();

            // If '//'..
            if (ch == '/')
            {
                std::string comment;
                getline( code, comment );   // Parse in the whole comment

                // If the comment says something like "fred is deleted" then generate appropriate tokens for that
                comment = comment + " ";
                if ( newstatement && comment.find(" deleted ")!=std::string::npos )
                {
                    // delete
                    addtoken( "delete", lineno, FileIndex );

                    // fred
                    std::string::size_type pos1 = comment.find_first_not_of(" \t");
                    std::string::size_type pos2 = comment.find(" ", pos1);
                    std::string firstWord = comment.substr( pos1, pos2-pos1 );
                    addtoken( firstWord.c_str(), lineno, FileIndex );

                    // ;
                    addtoken( ";", lineno, FileIndex );
                }

                lineno++;
                continue;
            }

            // If '/*'..
            if (ch == '*')
            {
                char chPrev;
                ch = chPrev = 'A';
                while (code.good() && (chPrev!='*' || ch!='/'))
                {
                    chPrev = ch;
                    ch = (char)code.get();
                    if (ch == '\n')
                        lineno++;
                }
                continue;
            }

            // Not a comment.. add token..
            addtoken("/", lineno, FileIndex);
        }

        // char..
        if (ch == '\'')
        {
            // Add previous token
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();

            // Read this ..
            CurrentToken += ch;
            CurrentToken += (char)code.get();
            CurrentToken += (char)code.get();
            if (CurrentToken[1] == '\\')
                CurrentToken += (char)code.get();

            // Add token and start on next..
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();

            continue;
        }

        // String..
        if (ch == '\"')
        {
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();
            bool special = false;
            char c = ch;
            do
            {
                // Append token..
                CurrentToken += c;

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)code.get();
            }
            while (code.good() && (special || c != '\"'));
            CurrentToken += '\"';
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();
            continue;
        }

        if (strchr("+-*/%&|^?!=<>[](){};:,.",ch))
        {
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();
            CurrentToken += ch;
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();
            continue;
        }


        if (isspace(ch) || iscntrl(ch))
        {
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            CurrentToken.clear();
            continue;
        }

        CurrentToken += ch;
    }
    addtoken( CurrentToken.c_str(), lineno, FileIndex );

    // Combine tokens..
    for (TOKEN *tok = _tokens; tok && tok->next; tok = tok->next)
    {
        tok->combineWithNext("<", "<");
        tok->combineWithNext(">", ">");

        tok->combineWithNext("&", "&");
        tok->combineWithNext("|", "|");

        tok->combineWithNext("+", "=");
        tok->combineWithNext("-", "=");
        tok->combineWithNext("*", "=");
        tok->combineWithNext("/", "=");
        tok->combineWithNext("&", "=");
        tok->combineWithNext("|", "=");

        tok->combineWithNext("=", "=");
        tok->combineWithNext("!", "=");
        tok->combineWithNext("<", "=");
        tok->combineWithNext(">", "=");

        tok->combineWithNext(":", ":");
        tok->combineWithNext("-", ">");

        tok->combineWithNext("private", ":");
        tok->combineWithNext("protected", ":");
        tok->combineWithNext("public", ":");
    }

    // Replace "->" with "."
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( strcmp(tok->str, "->") == 0 )
        {
            tok->setstr(".");
        }
    }

    // typedef..
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if (TOKEN::Match(tok, "typedef %type% %type% ;"))
        {
            const char *type1 = TOKEN::getstr(tok, 1);
            const char *type2 = TOKEN::getstr(tok, 2);
            for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if (tok2->str!=type1 && tok2->str!=type2 && strcmp(tok2->str,type2)==0)
                {
                    tok2->setstr(type1);
                }
            }
        }

        else if (TOKEN::Match(tok, "typedef %type% %type% %type% ;"))
        {
            const char *type1 = TOKEN::getstr(tok, 1);
            const char *type2 = TOKEN::getstr(tok, 2);
            const char *type3 = TOKEN::getstr(tok, 3);

            TOKEN *tok2 = tok;
            while ( ! TOKEN::Match(tok2, ";") )
                tok2 = tok2->next;

            for ( ; tok2; tok2 = tok2->next )
            {
                if (tok2->str!=type3 && strcmp(tok2->str,type3)==0)
                {
                    tok2->setstr(type1);

                    TOKEN *newtok = new TOKEN;
                    newtok->setstr(type2);
                    newtok->FileIndex = tok2->FileIndex;
                    newtok->linenr = tok2->linenr;
                    newtok->next = tok2->next;
                    tok2->next = newtok;
                    tok2 = newtok;
                }
            }
        }
    }


    // Remove __asm..
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( TOKEN::Match(tok->next, "__asm {") )
        {
            while ( tok->next )
            {
                bool last = TOKEN::Match( tok->next, "}" );

                // Unlink and delete tok->next
                TOKEN *next = tok->next;
                tok->next = tok->next->next;
                delete next;

                // break if this was the last token to delete..
                if (last)
                    break;
            }
        }
    }

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Simplify token list
//---------------------------------------------------------------------------

void Tokenizer::SimplifyTokenList()
{

    // Remove the keyword 'unsigned'
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if (tok->next && strcmp(tok->next->str,"unsigned")==0)
        {
            tok->deleteNext();
        }
    }

    // Replace constants..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if (TOKEN::Match(tok,"const %type% %var% = %num% ;"))
        {
            const char *sym = TOKEN::getstr(tok,2);
            const char *num = TOKEN::getstr(tok,4);

            for (TOKEN *tok2 = _gettok(tok,6); tok2; tok2 = tok2->next)
            {
                if (strcmp(tok2->str,sym) == 0)
                {
                    tok2->setstr(num);
                }
            }
        }
    }


    // Fill the map TypeSize..
    TypeSize.clear();
    TypeSize["char"] = sizeof(char);
    TypeSize["short"] = sizeof(short);
    TypeSize["int"] = sizeof(int);
    TypeSize["long"] = sizeof(long);
    TypeSize["float"] = sizeof(float);
    TypeSize["double"] = sizeof(double);
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if (TOKEN::Match(tok,"class %var%"))
        {
            TypeSize[TOKEN::getstr(tok,1)] = 11;
        }

        else if (TOKEN::Match(tok, "struct %var%"))
        {
            TypeSize[TOKEN::getstr(tok,1)] = 13;
        }
    }


    // Replace 'sizeof(type)'..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"sizeof") != 0)
            continue;

        if (TOKEN::Match(tok, "sizeof ( %type% * )"))
        {
            std::ostringstream str;
            // 'sizeof(type *)' has the same size as 'sizeof(char *)'
            str << sizeof(char *);
            tok->setstr( str.str().c_str() );

            for (int i = 0; i < 4; i++)
            {
                tok->deleteNext();
            }
        }

        else if (TOKEN::Match(tok, "sizeof ( %type% )"))
        {
            const char *type = TOKEN::getstr(tok, 2);
            int size = SizeOfType(type);
            if (size > 0)
            {
                std::ostringstream str;
                str << size;
                tok->setstr( str.str().c_str() );
                for (int i = 0; i < 3; i++)
                {
                    tok->deleteNext();
                }
            }
        }

        else if (TOKEN::Match(tok, "sizeof ( * %var% )"))
        {
            tok->setstr("100");
            for ( int i = 0; i < 4; ++i )
                tok->deleteNext();
        }
    }

    // Replace 'sizeof(var)'
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        // type array [ num ] ;
        if ( ! TOKEN::Match(tok, "%type% %var% [ %num% ] ;") )
            continue;

        int size = SizeOfType(tok->str);
        if (size <= 0)
            continue;

        const char *varname = TOKEN::getstr(tok, 1);
        int total_size = size * atoi( TOKEN::getstr(tok, 3) );

        // Replace 'sizeof(var)' with number
        int indentlevel = 0;
        for ( TOKEN *tok2 = _gettok(tok,5); tok2; tok2 = tok2->next )
        {
            if (tok2->str[0] == '{')
            {
                indentlevel++;
            }

            else if (tok2->str[0] == '}')
            {
                indentlevel--;
                if (indentlevel < 0)
                    break;
            }

            // Todo: TOKEN::Match varname directly
            else if (TOKEN::Match(tok2, "sizeof ( %var% )"))
            {
                if (strcmp(TOKEN::getstr(tok2,2), varname) == 0)
                {
                    std::ostringstream str;
                    str << total_size;
                    tok2->setstr(str.str().c_str());
                    // Delete the other tokens..
                    for (int i = 0; i < 3; i++)
                    {
                        tok2->deleteNext();
                    }
                }
            }
        }
    }




    // Simple calculations..
    for ( bool done = false; !done; done = true )
    {
        for (TOKEN *tok = _tokens; tok; tok = tok->next)
        {
            if (TOKEN::Match(tok->next, "* 1") || TOKEN::Match(tok->next, "1 *"))
            {
                for (int i = 0; i < 2; i++)
                    tok->deleteNext();
                done = false;
            }

            // (1-2)
            if (strchr("[,(=<>",tok->str[0]) &&
                TOKEN::IsNumber(TOKEN::getstr(tok,1))   &&
                strchr("+-*/",*(TOKEN::getstr(tok,2))) &&
                TOKEN::IsNumber(TOKEN::getstr(tok,3))   &&
                strchr("],);=<>",*(TOKEN::getstr(tok,4))) )
            {
                int i1 = atoi(TOKEN::getstr(tok,1));
                int i2 = atoi(TOKEN::getstr(tok,3));
                if ( i2 == 0 && *(TOKEN::getstr(tok,2)) == '/' )
                {
                    continue;
                }

                switch (*(TOKEN::getstr(tok,2)))
                {
                    case '+': i1 += i2; break;
                    case '-': i1 -= i2; break;
                    case '*': i1 *= i2; break;
                    case '/': i1 /= i2; break;
                }
                tok = tok->next;
                std::ostringstream str;
                str <<  i1;
                tok->setstr(str.str().c_str());
                for (int i = 0; i < 2; i++)
                {
                    tok->deleteNext();
                }

                done = false;
            }
        }
    }


    // Replace "*(str + num)" => "str[num]"
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if ( ! strchr(";{}(=<>", tok->str[0]) )
            continue;

        TOKEN *next = tok->next;
        if ( ! next )
            break;

        if (TOKEN::Match(next, "* ( %var% + %num% )"))
        {
            const char *str[4] = {"var","[","num","]"};
            str[0] = TOKEN::getstr(tok,3);
            str[2] = TOKEN::getstr(tok,5);

            for (int i = 0; i < 4; i++)
            {
                tok = tok->next;
                tok->setstr(str[i]);
            }

            tok->deleteNext();
            tok->deleteNext();
        }
    }



    // Split up variable declarations if possible..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if ( ! strchr("{};", tok->str[0]) )
            continue;

        TOKEN *type0 = tok->next;
        if (!TOKEN::Match(type0, "%type%"))
            continue;
        if (TOKEN::Match(type0, "else") || TOKEN::Match(type0, "return"))
            continue;

        TOKEN *tok2 = NULL;
        unsigned int typelen = 0;

        if ( TOKEN::Match(type0, "%type% %var% ,") )
        {
            tok2 = _gettok(type0, 2);    // The ',' token
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "%type% * %var% ,") )
        {
            tok2 = _gettok(type0, 3);    // The ',' token
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "%type% %var% [ %num% ] ,") )
        {
            tok2 = _gettok(type0, 5);    // The ',' token
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "%type% * %var% [ %num% ] ,") )
        {
            tok2 = _gettok(type0, 6);    // The ',' token
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "struct %type% %var% ,") )
        {
            tok2 = _gettok(type0, 3);
            typelen = 2;
        }

        else if ( TOKEN::Match(type0, "struct %type% * %var% ,") )
        {
            tok2 = _gettok(type0, 4);
            typelen = 2;
        }


        else if ( TOKEN::Match(type0, "%type% %var% =") )
        {
            tok2 = _gettok(type0, 2);
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "%type% * %var% =") )
        {
            tok2 = _gettok(type0, 3);
            typelen = 1;
        }

        else if ( TOKEN::Match(type0, "struct %type% * %var% =") )
        {
            tok2 = _gettok(type0, 4);
            typelen = 2;
        }

        if (tok2)
        {
            if (tok2->str[0] == ',')
            {
                tok2->setstr(";");
                InsertTokens(tok2, type0, typelen);
            }

            else
            {
                TOKEN *eq = tok2;

                int parlevel = 0;
                while (tok2)
                {
                    if ( strchr("{(", tok2->str[0]) )
                    {
                        parlevel++;
                    }

                    else if ( strchr("})", tok2->str[0]) )
                    {
                        if (parlevel<0)
                            break;
                        parlevel--;
                    }

                    else if ( parlevel==0 && strchr(";,",tok2->str[0]) )
                    {
                        // "type var ="   =>   "type var; var ="
                        TOKEN *VarTok = _gettok(type0,typelen);
                        if (VarTok->str[0]=='*')
                            VarTok = VarTok->next;
                        InsertTokens(eq, VarTok, 2);
                        eq->setstr(";");

                        // "= x, "   =>   "= x; type "
                        if (tok2->str[0] == ',')
                        {
                            tok2->setstr(";");
                            InsertTokens( tok2, type0, typelen );
                        }
                        break;
                    }

                    tok2 = tok2->next;
                }
            }
        }
    }

    // Replace NULL with 0..
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( TOKEN::Match(tok, "NULL") )
            tok->setstr("0");
    }

    // Replace pointer casts of 0.. "(char *)0" => "0"
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( TOKEN::Match(tok->next, "( %type% * ) 0") || TOKEN::Match(tok->next,"( %type% %type% * ) 0") )
        {
            while (!TOKEN::Match(tok->next,"0"))
                tok->deleteNext();
        }
    }


    for ( bool done = false; !done; done = true)
    {
        done &= simplifyConditions();
    };
}
//---------------------------------------------------------------------------


bool Tokenizer::simplifyConditions()
{
    bool ret = true;

    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if (TOKEN::Match(tok, "( true &&") || TOKEN::Match(tok, "&& true &&") || TOKEN::Match(tok->next, "&& true )"))
        {
            tok->deleteNext();
            tok->deleteNext();
            ret = false;
        }

        else if (TOKEN::Match(tok, "( false ||") || TOKEN::Match(tok, "|| false ||") || TOKEN::Match(tok->next, "|| false )"))
        {
            tok->deleteNext();
            tok->deleteNext();
            ret = false;
        }

        // Change numeric constant in condition to "true" or "false"
        const TOKEN *tok2 = tok->at(2);
        if ((TOKEN::Match(tok, "(") || TOKEN::Match(tok, "&&") || TOKEN::Match(tok, "||")) &&
            TOKEN::Match(tok->next, "%num%")                                 &&
            (TOKEN::Match(tok2, ")") || TOKEN::Match(tok2, "&&") || TOKEN::Match(tok2, "||")) )
        {
            tok->next->setstr((strcmp(tok->next->str, "0")!=0) ? "true" : "false");
            ret = false;
        }
    }

    return ret;
}


//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

const TOKEN *Tokenizer::GetFunctionTokenByName( const char funcname[] ) const
{
    for ( unsigned int i = 0; i < FunctionList.size(); ++i )
    {
        if ( strcmp( FunctionList[i]->str, funcname ) == 0 )
        {
            return FunctionList[i];
        }
    }
    return NULL;
}


void Tokenizer::FillFunctionList()
{
    FunctionList.clear();

    bool staticfunc = false;
    bool classfunc = false;

    int indentlevel = 0;
    for ( const TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;

        if (indentlevel > 0)
        {
            continue;
        }

        if (strchr("};", tok->str[0]))
            staticfunc = classfunc = false;

        else if ( strcmp( tok->str, "static" ) == 0 )
            staticfunc = true;

        else if ( strcmp( tok->str, "::" ) == 0 )
            classfunc = true;

        else if (TOKEN::Match(tok, "%var% ("))
        {
            // Check if this is the first token of a function implementation..
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if ( tok2->str[0] == ';' )
                {
                    tok = tok2;
                    break;
                }

                else if ( tok2->str[0] == '{' )
                {
                    break;
                }

                else if ( tok2->str[0] == ')' )
                {
                    if ( TOKEN::Match(tok2, ") {") )
                    {
                        FunctionList.push_back( tok );
                        tok = tok2;
                    }
                    else
                    {
                        tok = tok2;
                        while (tok->next && !strchr(";{", tok->next->str[0]))
                            tok = tok->next;
                    }
                    break;
                }
            }
        }
    }

    // If the FunctionList functions with duplicate names, remove them
    // TODO this will need some better handling
    for ( unsigned int func1 = 0; func1 < FunctionList.size(); )
    {
        bool hasDuplicates = false;
        for ( unsigned int func2 = func1 + 1; func2 < FunctionList.size(); )
        {
            if ( strcmp(FunctionList[func1]->str, FunctionList[func2]->str) == 0 )
            {
                hasDuplicates = true;
                FunctionList.erase( FunctionList.begin() + func2 );
            }
            else
            {
                ++func2;
            }
        }

        if ( ! hasDuplicates )
        {
            ++func1;
        }
        else
        {
            FunctionList.erase( FunctionList.begin() + func1 );
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::settings( const Settings &settings )
{
    _settings = settings;
}

// Deallocate lists..
void Tokenizer::DeallocateTokens()
{
    deleteTokens( _tokens );
    _tokens = 0;
    tokens_back = 0;

    while (dsymlist)
    {
        struct DefineSymbol *next = dsymlist->next;
        free(dsymlist->name);
        free(dsymlist->value);
        delete dsymlist;
        dsymlist = next;
    }

    Files.clear();
}

void Tokenizer::deleteTokens(TOKEN *tok)
{
    while (tok)
    {
        TOKEN *next = tok->next;
        delete tok;
        tok = next;
    }
}

//---------------------------------------------------------------------------

const char *Tokenizer::getParameterName( const TOKEN *ftok, int par )
{
    int _par = 1;
    for ( ; ftok; ftok = ftok->next)
    {
        if ( TOKEN::Match(ftok, ",") )
            ++_par;
        if ( par==_par && TOKEN::Match(ftok, "%var% [,)]") )
            return ftok->str;
    }
    return NULL;
}

//---------------------------------------------------------------------------

std::string Tokenizer::fileLine( const TOKEN *tok ) const
{
    std::ostringstream ostr;
    ostr << "[" << Files.at(tok->FileIndex) << ":" << tok->linenr << "]";
    return ostr.str();
}

//---------------------------------------------------------------------------

bool Tokenizer::SameFileName( const char fname1[], const char fname2[] )
{
#ifdef __linux__
    return bool( strcmp(fname1, fname2) == 0 );
#endif
#ifdef __GNUC__
    return bool( strcasecmp(fname1, fname2) == 0 );
#endif
#ifdef __BORLANDC__
    return bool( stricmp(fname1, fname2) == 0 );
#endif
#ifdef _MSC_VER
    return bool( _stricmp(fname1, fname2) == 0 );
#endif
}

//---------------------------------------------------------------------------
