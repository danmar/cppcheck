//---------------------------------------------------------------------------
#include "tokenize.h"
#include "CommonCheck.h"    // <- IsName
//---------------------------------------------------------------------------

#include <locale>
#include <fstream>

#include <map>
#include <string>

#include <stdlib.h>     // <- strtoul

//---------------------------------------------------------------------------

// Helper functions..

static void Define(const char Name[], const char Value[]);

static void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

static void combine_2tokens(TOKEN *tok, const char str1[], const char str2[]);

static int SizeOfType(const char type[]);

static void DeleteNextToken(TOKEN *tok);

//---------------------------------------------------------------------------

std::vector<std::string> Files;
struct TOKEN *tokens, *tokens_back;

//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Defined symbols.
// "#define abc 123" will create a defined symbol "abc" with the value 123
//---------------------------------------------------------------------------

struct DefineSymbol
{
    char *name;
    char *value;
    struct DefineSymbol *next;
};
static struct DefineSymbol * dsymlist;

static void Define(const char Name[], const char Value[])
{
    if (!(Name && Name[0]))
        return;

    if (!(Value && Value[0]))
        return;

    // Is 'Value' a decimal value..
    bool dec = true, hex = true;
    for (int i = 0; Value[i]; i++)
    {
        if ( ! std::isdigit(Value[i]) )
            dec = false;

        if ( ! std::isxdigit(Value[i]) && (!(i==1 && Value[i]=='x')))
            hex = false;
    }

    if (!dec && !hex)
        return;

    char *strValue = strdup(Value);

    if (!dec && hex)
    {
        char str[50];
        unsigned long value = strtoul(Value+2, NULL, 16);
        free(strValue);
        strValue = strdup(itoa(value, str, 10));
    }

    DefineSymbol *NewSym = new DefineSymbol;
    memset(NewSym, 0, sizeof(DefineSymbol));
    NewSym->name = strdup(Name);
    NewSym->value = strValue;
    NewSym->next = dsymlist;
    dsymlist = NewSym;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

static void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno)
{
    if (str[0] == 0)
        return;

    // The keyword 'unsigned' can be skipped for now.
    if (strcmp(str,"unsigned")==0)
        return;

    // Replace hexadecimal value with decimal
    char str2[50];
    memset(str2, 0, sizeof(str2));
    if (strncmp(str,"0x",2)==0)
    {
        unsigned int value = strtoul(str+2, NULL, 16);
        itoa(value, str2, 10);
    }

    TOKEN *newtoken  = new TOKEN;
    memset(newtoken, 0, sizeof(TOKEN));
    newtoken->str    = strdup(str2[0] ? str2 : str);
    newtoken->linenr = lineno;
    newtoken->FileIndex = fileno;
    if (tokens_back)
    {
        tokens_back->next = newtoken;
        tokens_back = newtoken;
    }
    else
    {
        tokens = tokens_back = newtoken;
    }

    // Check if str is defined..
    for (DefineSymbol *sym = dsymlist; sym; sym = sym->next)
    {
        if (strcmp(str,sym->name)==0)
        {
            free(newtoken->str);
            newtoken->str = strdup(sym->value);
            break;
        }
    }
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// combine_2tokens
// Combine two tokens that belong to each other. Ex: "<" and "=" may become "<="
//---------------------------------------------------------------------------

static void combine_2tokens(TOKEN *tok, const char str1[], const char str2[])
{
    if (!(tok && tok->next))
        return;
    if (strcmp(tok->str,str1) || strcmp(tok->next->str,str2))
        return;

    free(tok->str);
    tok->str = (char *)malloc(strlen(str1)+strlen(str2)+1);
    strcpy(tok->str, str1);
    strcat(tok->str, str2);

    DeleteNextToken(tok);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------

std::map<std::string, unsigned int> TypeSize;

static int SizeOfType(const char type[])
{
    if (!type)
        return 0;

    return TypeSize[type];
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// DeleteNextToken. Unlink and delete next token.
//---------------------------------------------------------------------------

static void DeleteNextToken(TOKEN *tok)
{
    TOKEN *next = tok->next;
    tok->next = next->next;
    free(next->str);
    delete next;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n)
{
    while (n > 0)
    {
        TOKEN *NewToken = new TOKEN;
        NewToken->FileIndex = src->FileIndex;
        NewToken->linenr = src->linenr;
        NewToken->str = strdup(src->str);

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

void Tokenize(const char FileName[])
{
    // Has this file been tokenized already?
    for (unsigned int i = 0; i < Files.size(); i++)
    {
        if ( stricmp(Files[i].c_str(), FileName) == 0 )
            return;
    }

    std::ifstream fin(FileName);
    if (!fin.is_open())
        return;

    unsigned int CurrentFile = Files.size();
    Files.push_back(FileName);

    unsigned int lineno = 1;
    char CurrentToken[1000];
    memset(CurrentToken, 0, sizeof(CurrentToken));
    char *pToken = CurrentToken;
    for (char ch = (char)fin.get(); !fin.eof(); ch = (char)fin.get())
    {
        if (ch == '#' && !CurrentToken[0])
        {
            std::string line;
            getline(fin,line);
            line = "#" + line;
            if (strncmp(line.c_str(),"#include",8)==0 &&
                line.find("\"") != std::string::npos)
            {
                // Extract the filename
                line.erase(0, line.find("\"")+1);
                line.erase(line.find("\""));

                // Relative path..
                if (strchr(FileName,'\\'))
                {
                    char path[1000];
                    memset(path,0,sizeof(path));
                    const char *p = strrchr(FileName, '\\');
                    memcpy(path, FileName, p-FileName+1);
                    line = path + line;
                }

                addtoken("#include", lineno, CurrentFile);
                addtoken(line.c_str(), lineno, CurrentFile);

                Tokenize(line.c_str());
            }

            else if (strncmp(line.c_str(), "#define", 7) == 0)
            {
                char *strId = NULL;
                enum {Space1, Id, Space2, Value} State;
                State = Space1;
                for (unsigned int i = 8; i < line.length(); i++)
                {
                    if (State==Space1 || State==Space2)
                    {
                        if (std::isspace(line[i]))
                            continue;
                        State = (State==Space1) ? Id : Value;
                    }

                    else if (State==Id && std::isspace(line[i]))
                    {
                        strId = strdup(CurrentToken);
                        memset(CurrentToken, 0, sizeof(CurrentToken));
                        pToken = CurrentToken;
                        State = Space2;
                        continue;
                    }

                    *pToken = line[i];
                    pToken++;
                }

                if (State==Value)
                {
                    Define(strId, CurrentToken);
                }

                pToken = CurrentToken;
                memset(CurrentToken, 0, sizeof(CurrentToken));
                free(strId);
            }

            lineno++;
            continue;
        }

        if (ch == '\n')
        {
            // Add current token..
            addtoken(CurrentToken, lineno++, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        // Comments..
        if (ch == '/' && !fin.eof())
        {
            // Add current token..
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            // Read next character..
            ch = (char)fin.get();

            // If '//'..
            if (ch == '/')
            {
                while (!fin.eof() && (char)fin.get()!='\n');
                lineno++;
                continue;
            }

            // If '/*'..
            if (ch == '*')
            {
                char chPrev;
                ch = chPrev = 'A';
                while (!fin.eof() && (chPrev!='*' || ch!='/'))
                {
                    chPrev = ch;
                    ch = (char)fin.get();
                    if (ch == '\n')
                        lineno++;
                }
                continue;
            }

            // Not a comment.. add token..
            addtoken("/", lineno, CurrentFile);
        }

        // char..
        if (ch == '\'')
        {
            // Add previous token
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));

            // Read this ..
            CurrentToken[0] = ch;
            CurrentToken[1] = (char)fin.get();
            CurrentToken[2] = (char)fin.get();
            if (CurrentToken[1] == '\\')
                CurrentToken[3] = (char)fin.get();

            // Add token and start on next..
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;

            continue;
        }

        // String..
        if (ch == '\"')
        {
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            bool special = false;
            char c = ch;
            do
            {
                // Append token..
                *pToken = c;
                pToken++;

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)fin.get();
            }
            while (special || c != '\"');
            *pToken = '\"';
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }

        if (strchr("+-*/%&|^?!=<>[](){};:,.",ch))
        {
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            CurrentToken[0] = ch;
            addtoken(CurrentToken, lineno, CurrentFile);
            memset(CurrentToken, 0, sizeof(CurrentToken));
            pToken = CurrentToken;
            continue;
        }


        if (std::isspace(ch) || std::iscntrl(ch))
        {
            addtoken(CurrentToken, lineno, CurrentFile);
            pToken = CurrentToken;
            memset(CurrentToken, 0, sizeof(CurrentToken));
            continue;
        }

        *pToken = ch;
        pToken++;
    }

    // Combine tokens..
    for (TOKEN *tok = tokens; tok && tok->next; tok = tok->next)
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
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// Simplify token list
//---------------------------------------------------------------------------

void SimplifyTokenList()
{
    // Replace constants..
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (match(tok,"const type var = num ;"))
        {
            const char *sym = getstr(tok,2);
            const char *num = getstr(tok,4);

            for (TOKEN *tok2 = gettok(tok,6); tok2; tok2 = tok2->next)
            {
                if (strcmp(tok2->str,sym) == 0)
                {
                    free(tok2->str);
                    tok2->str = strdup(num);
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
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (match(tok,"class type"))
        {
            TypeSize[getstr(tok,1)] = 11;
        }

        else if (match(tok, "struct type"))
        {
            TypeSize[getstr(tok,1)] = 13;
        }
    }


    // Replace 'sizeof(type)'..
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"sizeof") != 0)
            continue;

        if (match(tok, "sizeof ( type * )"))
        {
            free(tok->str);
            char str[10];
            // 'sizeof(type *)' has the same size as 'sizeof(char *)'
            tok->str = strdup(itoa(sizeof(char *), str, 10));

            for (int i = 0; i < 4; i++)
            {
                DeleteNextToken(tok);
            }
        }

        else if (match(tok, "sizeof ( type )"))
        {
            const char *type = getstr(tok, 2);
            int size = SizeOfType(type);
            if (size > 0)
            {
                free(tok->str);
                char str[10];
                tok->str = strdup(itoa(size, str, 10));

                for (int i = 0; i < 3; i++)
                {
                    DeleteNextToken(tok);
                }
            }
        }
    }

    // Replace 'sizeof(var)'
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // type array [ num ] ;
        if ( ! match(tok, "type var [ num ] ;") )
            continue;

        int size = SizeOfType(tok->str);
        if (size <= 0)
            continue;

        const char *varname = getstr(tok, 1);
        int total_size = size * atoi( getstr(tok, 3) );

        // Replace 'sizeof(var)' with number
        int indentlevel = 0;
        for ( TOKEN *tok2 = gettok(tok,5); tok2; tok2 = tok2->next )
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

            else if (match(tok2, "sizeof ( var )"))
            {
                if (strcmp(getstr(tok2,2), varname) == 0)
                {
                    free(tok2->str);
                    char str[20];
                    tok2->str = strdup(itoa(total_size, str, 10));

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

        for (TOKEN *tok = tokens; tok; tok = tok->next)
        {
            if (match(tok->next, "* 1") || match(tok->next, "1 *"))
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
                done = false;

                int i1 = atoi(getstr(tok,1));
                int i2 = atoi(getstr(tok,3));
                switch (*(getstr(tok,2)))
                {
                    case '+': i1 += i2; break;
                    case '-': i1 -= i2; break;
                    case '*': i1 *= i2; break;
                    case '/': i1 /= i2; break;
                }
                tok = tok->next;
                free(tok->str);
                char str[10];
                tok->str = strdup(itoa(i1,str,10));
                for (int i = 0; i < 2; i++)
                {
                    DeleteNextToken(tok);
                }
            }
        }
    }


    // Replace "*(str + num)" => "str[num]"
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( ! strchr(";{}(=<>", tok->str[0]) )
            continue;

        TOKEN *next = tok->next;
        if ( ! next )
            break;

        if (match(next, "* ( var + num )"))
        {
            const char *str[4] = {"var","[","num","]"};
            str[0] = getstr(tok,3);
            str[2] = getstr(tok,5);

            for (int i = 0; i < 4; i++)
            {
                tok = tok->next;
                free(tok->str);
                tok->str = strdup(str[i]);
            }

            DeleteNextToken(tok);
            DeleteNextToken(tok);
        }
    }



    // Split up variable declarations if possible..
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if ( ! strchr("{};", tok->str[0]) )
            continue;

        TOKEN *type0 = tok->next;

        TOKEN *tok2 = NULL;
        unsigned int typelen = 0;

        if ( match(type0, "type var ,") )
        {
            tok2 = gettok(type0, 2);    // The ',' token
            typelen = 1;
        }

        else if ( match(type0, "type * var ,") )
        {
            tok2 = gettok(type0, 3);    // The ',' token
            typelen = 2;
        }

        else if ( match(type0, "type var [ num ] ,") )
        {
            tok2 = gettok(type0, 5);    // The ',' token
            typelen = 1;
        }

        else if ( match(type0, "type * var [ num ] ,") )
        {
            tok2 = gettok(type0, 6);    // The ',' token
            typelen = 2;
        }


        else if ( match(type0, "type var =") )
        {
            tok2 = gettok(type0, 2);
            typelen = 1;
        }

        else if ( match(type0, "type * var =") )
        {
            tok2 = gettok(type0, 3);
            typelen = 2;
        }

        if (tok2)
        {
            if (tok2->str[0] == ',')
            {
                free(tok2->str);
                tok2->str = strdup(";");
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
                        InsertTokens(eq, gettok(type0,typelen), 2);
                        free(eq->str);
                        eq->str = strdup(";");

                        // "= x, "   =>   "= x; type "
                        if (tok2->str[0] == ',')
                        {
                            free(tok2->str);
                            tok2->str = strdup(";");
                            InsertTokens( tok2, type0, typelen );
                        }
                        break;
                    }

                    tok2 = tok2->next;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

TOKEN *findtoken(TOKEN *tok1, const char *tokenstr[])
{
    for (TOKEN *ret = tok1; ret; ret = ret->next)
    {
        unsigned int i = 0;
        TOKEN *tok = ret;
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

bool match(TOKEN *tok, const std::string pattern)
{
    if (!tok)
        return false;

    const char *p = pattern.c_str();
    while (*p)
    {
        char str[50];
        char *s = str;
        while (*p==' ')
            p++;
        while (*p && *p!=' ')
        {
            *s = *p;
            s++;
            p++;
        }
        *s = 0;
        if (str[0] == 0)
            return true;

        if (strcmp(str,"var")==0 || strcmp(str,"type")==0)
        {
            if (!IsName(tok->str))
                return false;
        }
        else if (strcmp(str,"num")==0)
        {
            if (!std::isdigit(tok->str[0]))
                return false;
        }
        else if (strcmp(str, tok->str) != 0)
            return false;

        tok = tok->next;
        if (!tok)
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------

TOKEN *gettok(TOKEN *tok, int index)
{
    while (tok && index>0)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}
//---------------------------------------------------------------------------

const char *getstr(TOKEN *tok, int index)
{
    tok = gettok(tok, index);
    return tok ? tok->str : "";
}
//---------------------------------------------------------------------------





