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
#include "CommonCheck.h"    // <- IsName
//---------------------------------------------------------------------------

#include <locale>
#include <fstream>


#include <string>
#include <cstring>
#include <iostream>
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



std::vector<std::string> *Tokenizer::getFiles()
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
// combine_2tokens
// Combine two tokens that belong to each other. Ex: "<" and "=" may become "<="
//---------------------------------------------------------------------------

void Tokenizer::combine_2tokens(TOKEN *tok, const char str1[], const char str2[])
{
    if (!(tok && tok->next))
        return;
    if (strcmp(tok->str,str1) || strcmp(tok->next->str,str2))
        return;

	std::string newstr(std::string(str1) + std::string(str2));
	tok->setstr( newstr.c_str() );

    DeleteNextToken(tok);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------



int Tokenizer::SizeOfType(const char type[])
{
    if (!type)
        return 0;

    return TypeSize[type];
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// DeleteNextToken. Unlink and delete next token.
//---------------------------------------------------------------------------

void Tokenizer::DeleteNextToken(TOKEN *tok)
{
    TOKEN *next = tok->next;
    tok->next = next->next;
    delete next;
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
        combine_2tokens(tok, "<", "<");
        combine_2tokens(tok, ">", ">");

        combine_2tokens(tok, "&", "&");
        combine_2tokens(tok, "|", "|");

        combine_2tokens(tok, "+", "=");
        combine_2tokens(tok, "-", "=");
        combine_2tokens(tok, "*", "=");
        combine_2tokens(tok, "/", "=");
        combine_2tokens(tok, "&", "=");
        combine_2tokens(tok, "|", "=");

        combine_2tokens(tok, "=", "=");
        combine_2tokens(tok, "!", "=");
        combine_2tokens(tok, "<", "=");
        combine_2tokens(tok, ">", "=");

        combine_2tokens(tok, ":", ":");
        combine_2tokens(tok, "-", ">");

        combine_2tokens(tok, "private", ":");
        combine_2tokens(tok, "protected", ":");
        combine_2tokens(tok, "public", ":");
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
        if (Match(tok, "typedef %type% %type% ;"))
        {
            const char *type1 = getstr(tok, 1);
            const char *type2 = getstr(tok, 2);
            for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if (tok2->str!=type1 && tok2->str!=type2 && strcmp(tok2->str,type2)==0)
                {
                    tok2->setstr(type1);
                }
            }
        }

        else if (Match(tok, "typedef %type% %type% %type% ;"))
        {
            const char *type1 = getstr(tok, 1);
            const char *type2 = getstr(tok, 2);
            const char *type3 = getstr(tok, 3);

            TOKEN *tok2 = tok;
            while ( ! Match(tok2, ";") )
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
        if ( Match(tok->next, "__asm {") )
        {
            while ( tok->next )
            {
                bool last = Match( tok->next, "}" );

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
            DeleteNextToken( tok );
        }
    }

    // Replace constants..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if (Match(tok,"const %type% %var% = %num% ;"))
        {
            const char *sym = getstr(tok,2);
            const char *num = getstr(tok,4);

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
        if (Match(tok,"class %var%"))
        {
            TypeSize[getstr(tok,1)] = 11;
        }

        else if (Match(tok, "struct %var%"))
        {
            TypeSize[getstr(tok,1)] = 13;
        }
    }


    // Replace 'sizeof(type)'..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"sizeof") != 0)
            continue;

        if (Match(tok, "sizeof ( %type% * )"))
        {
            std::ostringstream str;
            // 'sizeof(type *)' has the same size as 'sizeof(char *)'
            str << sizeof(char *);
            tok->setstr( str.str().c_str() );

            for (int i = 0; i < 4; i++)
            {
                DeleteNextToken(tok);
            }
        }

        else if (Match(tok, "sizeof ( %type% )"))
        {
            const char *type = getstr(tok, 2);
            int size = SizeOfType(type);
            if (size > 0)
            {
                std::ostringstream str;
                str << size;
                tok->setstr( str.str().c_str() );
                for (int i = 0; i < 3; i++)
                {
                    DeleteNextToken(tok);
                }
            }
        }

        else if (Match(tok, "sizeof ( * %var% )"))
        {
            tok->setstr("100");
            for ( int i = 0; i < 4; ++i )
                DeleteNextToken(tok);
        }
    }

    // Replace 'sizeof(var)'
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        // type array [ num ] ;
        if ( ! Match(tok, "%type% %var% [ %num% ] ;") )
            continue;

        int size = SizeOfType(tok->str);
        if (size <= 0)
            continue;

        const char *varname = getstr(tok, 1);
        int total_size = size * atoi( getstr(tok, 3) );

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

            // Todo: Match varname directly
            else if (Match(tok2, "sizeof ( %var% )"))
            {
                if (strcmp(getstr(tok2,2), varname) == 0)
                {
                    std::ostringstream str;
                    str << total_size;
                    tok2->setstr(str.str().c_str());
                    // Delete the other tokens..
                    for (int i = 0; i < 3; i++)
                    {
                        DeleteNextToken(tok2);
                    }
                }
            }
        }
    }




    // Simple calculations..

    bool done = false;
    while (!done)
    {
        done = true;

        for (TOKEN *tok = _tokens; tok; tok = tok->next)
        {
            if (Match(tok->next, "* 1") || Match(tok->next, "1 *"))
            {
                for (int i = 0; i < 2; i++)
                    DeleteNextToken(tok);
                done = false;
            }

            // (1-2)
            if (strchr("[,(=<>",tok->str[0]) &&
                IsNumber(getstr(tok,1))   &&
                strchr("+-*/",*(getstr(tok,2))) &&
                IsNumber(getstr(tok,3))   &&
                strchr("],);=<>",*(getstr(tok,4))) )
            {
                int i1 = atoi(getstr(tok,1));
                int i2 = atoi(getstr(tok,3));
                if ( i2 == 0 && *(getstr(tok,2)) == '/' )
                {
                    continue;
                }

                switch (*(getstr(tok,2)))
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
                    DeleteNextToken(tok);
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

        if (Match(next, "* ( %var% + %num% )"))
        {
            const char *str[4] = {"var","[","num","]"};
            str[0] = getstr(tok,3);
            str[2] = getstr(tok,5);

            for (int i = 0; i < 4; i++)
            {
                tok = tok->next;
                tok->setstr(str[i]);
            }

            DeleteNextToken(tok);
            DeleteNextToken(tok);
        }
    }



    // Split up variable declarations if possible..
    for (TOKEN *tok = _tokens; tok; tok = tok->next)
    {
        if ( ! strchr("{};", tok->str[0]) )
            continue;

        TOKEN *type0 = tok->next;
        if (!Match(type0, "%type%"))
            continue;
        if (Match(type0, "else") || Match(type0, "return"))
            continue;

        TOKEN *tok2 = NULL;
        unsigned int typelen = 0;

        if ( Match(type0, "%type% %var% ,") )
        {
            tok2 = _gettok(type0, 2);    // The ',' token
            typelen = 1;
        }

        else if ( Match(type0, "%type% * %var% ,") )
        {
            tok2 = _gettok(type0, 3);    // The ',' token
            typelen = 1;
        }

        else if ( Match(type0, "%type% %var% [ %num% ] ,") )
        {
            tok2 = _gettok(type0, 5);    // The ',' token
            typelen = 1;
        }

        else if ( Match(type0, "%type% * %var% [ %num% ] ,") )
        {
            tok2 = _gettok(type0, 6);    // The ',' token
            typelen = 1;
        }

        else if ( Match(type0, "struct %type% %var% ,") )
        {
            tok2 = _gettok(type0, 3);
            typelen = 2;
        }

        else if ( Match(type0, "struct %type% * %var% ,") )
        {
            tok2 = _gettok(type0, 4);
            typelen = 2;
        }


        else if ( Match(type0, "%type% %var% =") )
        {
            tok2 = _gettok(type0, 2);
            typelen = 1;
        }

        else if ( Match(type0, "%type% * %var% =") )
        {
            tok2 = _gettok(type0, 3);
            typelen = 1;
        }

        else if ( Match(type0, "struct %type% * %var% =") )
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
        if ( Match(tok, "NULL") )
            tok->setstr("0");
    }

    // Replace pointer casts of 0.. "(char *)0" => "0"
    for ( TOKEN *tok = _tokens; tok; tok = tok->next )
    {
        if ( Match(tok->next, "( %type% * ) 0") || Match(tok->next,"( %type% %type% * ) 0") )
        {
            while (!Match(tok->next,"0"))
                DeleteNextToken(tok);
        }
    }
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

const TOKEN *Tokenizer::findtoken(const TOKEN *tok1, const char *tokenstr[])
{
    for (const TOKEN *ret = tok1; ret; ret = ret->next)
    {
        unsigned int i = 0;
        const TOKEN *tok = ret;
        while (tokenstr[i])
        {
            if (!tok)
                return NULL;
            if (*(tokenstr[i]) && strcmp(tokenstr[i],tok->str))
                break;
            tok = tok->next;
            i++;
        }
        if (!tokenstr[i])
            return ret;
    }
    return NULL;
}
//---------------------------------------------------------------------------

const TOKEN *Tokenizer::gettok(const TOKEN *tok, int index)
{
    while (tok && index>0)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}
//---------------------------------------------------------------------------

const char *Tokenizer::getstr(const TOKEN *tok, int index)
{
    tok = gettok(tok, index);
    return tok ? tok->str : "";
}
//---------------------------------------------------------------------------



// Deallocate lists..
void Tokenizer::DeallocateTokens()
{
    while (_tokens)
    {
        TOKEN *next = _tokens->next;
        delete _tokens;
        _tokens = next;
    }
    tokens_back = _tokens;

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


void Tokenizer::FillFunctionList(const unsigned int file_id)
{
    FunctionList.clear();

    std::list<const char *> _usedfunc;
    if ( file_id == 0 )
    {
        GlobalFunctions.clear();
    }

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
            if ( _settings._checkCodingStyle )
            {
                const char *funcname = 0;

                if ( Match(tok,"%var% (") )
                    funcname = tok->str;
                else if ( Match(tok, "= %var% ;") ||
                          Match(tok, "= %var% ,") )
                    funcname = tok->next->str;

                if ( std::find(_usedfunc.begin(), _usedfunc.end(), funcname) == _usedfunc.end() )
                    _usedfunc.push_back( funcname );
            }

            continue;
        }

        if (strchr("};", tok->str[0]))
            staticfunc = classfunc = false;

        else if ( strcmp( tok->str, "static" ) == 0 )
            staticfunc = true;

        else if ( strcmp( tok->str, "::" ) == 0 )
            classfunc = true;

        else if (Match(tok, "%var% ("))
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
                    if ( Match(tok2, ") {") )
                    {
                        if (_settings._checkCodingStyle && !staticfunc && !classfunc && tok->FileIndex==0)
                            GlobalFunctions.push_back( GlobalFunction(file_id, tok->str) );
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


    for (std::list<const char *>::const_iterator it = _usedfunc.begin(); it != _usedfunc.end(); ++it)
    {
        if ( *it != 0 )
        {
            UsedGlobalFunctions.push_back( GlobalFunction(file_id, *it) );
        }
    }
}

//--------------------------------------------------------------------------


void Tokenizer::CheckGlobalFunctionUsage(const std::vector<std::string> &filenames)
{
    // Iterator for GlobalFunctions
    std::list<GlobalFunction>::const_iterator func;

    // Iterator for UsedGlobalFunctions
    std::list<GlobalFunction>::const_iterator usedfunc;

    unsigned int i1 = 0;
    unsigned int i2 = 1;

    // Check that every function in GlobalFunctions are used
    for ( func = GlobalFunctions.begin(); func != GlobalFunctions.end(); func++ )
    {
        if ( GlobalFunctions.size() > 100 )
        {
            ++i1;
            if ( i1 > (i2 * GlobalFunctions.size()) / 100 )
            {
                if ( (i2 % 10) == 0 )
                    std::cout << i2 << "%";
                else
                    std::cout << ".";
                std::cout.flush();
                ++i2;
            }
        }

        const std::string &funcname = func->name();

        if ( funcname == "main" || funcname == "WinMain" )
            continue;

        // Check if this global function is used in any of the other files..
        bool UsedOtherFile = false;
        bool UsedAnyFile = false;
        for ( usedfunc = UsedGlobalFunctions.begin(); usedfunc != UsedGlobalFunctions.end(); usedfunc++ )
        {
            if ( funcname == usedfunc->name() )
            {
                UsedAnyFile = true;
                if (func->file_id() != usedfunc->file_id())
                {
                    UsedOtherFile = true;
                    break;
                }
            }
        }

        if ( ! UsedAnyFile )
        {
            std::ostringstream errmsg;
            errmsg << "[" << filenames[func->file_id()] << "]: "
                   << "The function '" << func->name() << "' is never used.";
            ReportErr( errmsg.str() );
        }
        else if ( ! UsedOtherFile )
        {
            std::ostringstream errmsg;
            errmsg << "[" << filenames[func->file_id()] << "]: "
                   << "The linkage of the function '" << func->name() << "' can be local (static) instead of global";
            ReportErr( errmsg.str() );
        }
    }

    std::cout << "\n";
}
//---------------------------------------------------------------------------

void Tokenizer::settings( const Settings &settings )
{
    _settings = settings;
}
