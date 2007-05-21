

// Todo: Output progress? Using commandline option "--progress"?

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
std::vector<std::string> Files;
static bool Debug = false;
//---------------------------------------------------------------------------
struct TOKEN
{
    unsigned int FileIndex;
    char *str;
    unsigned int linenr;
    struct TOKEN *next;
};
struct TOKEN *tokens, *tokens_back;
void Tokenize(const char FileName[]);
//---------------------------------------------------------------------------
std::vector<std::string> VariableNames;
struct STATEMENT
{
    enum etype {OBRACE, EBRACE,
                DECL, ASSIGN, USE,
                MALLOC, FREE,
                NEW, DELETE,
                NEWARRAY, DELETEARRAY,
                LOOP, ENDLOOP,
                IF, ELSE, ELSEIF, ENDIF,
                SWITCH, ENDSWITCH,
                RETURN, CONTINUE, BREAK};
    etype Type;
    unsigned int VarIndex;
    TOKEN *Token;
};
std::list<STATEMENT> Statements;
void CreateStatementList();
//---------------------------------------------------------------------------

// Memory leak..
void CheckMemoryLeak();

// Buffer overrun..
void CheckBufferOverrun();

// Class
void CheckConstructors();
void CheckUnusedPrivateFunctions();
void CheckMemset();
void CheckOperatorEq1();    // Warning upon "void operator=(.."

// Casting
void WarningOldStylePointerCast();

// Headers..
void WarningHeaderWithImplementation();
void WarningIncludeHeader();

// Use standard functions instead
void WarningIsDigit();

// Redundant code
void WarningRedundantCode();

// Warning upon: if (condition);
void WarningIf();

// Using dangerous functions
void WarningDangerousFunctions();

//---------------------------------------------------------------------------

static void CppCheck(const char FileName[]);

int main(int argc, char* argv[])
{
    Debug = (argc == 3 && strcmp(argv[1], "--debug")==0);

    if (argc == 1 || argc > 3)
    {
        std::cout << "Syntax:\n    checkcode filename\n";
        return 0;
    }

    CppCheck(argv[argc - 1]);

    return 0;
}

static void CppCheck(const char FileName[])
{
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);


    CreateStatementList();


    // Memory leak
    CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrun();


    //std::ofstream f("tokens.txt");
    //for (TOKEN *tok = tokens; tok; tok = tok->next)
    //    f << "[" << Files[tok->FileIndex] << ":" << tok->linenr << "]:" << tok->str << '\n';
    //f.close();

    // Check that all class constructors are ok.
    // Temporarily inactivated to avoid any false positives
    //CheckConstructors();

    // Check that all private functions are called.
    // Temporarily inactivated to avoid any false positives
    CheckUnusedPrivateFunctions();

    // Check that the memsets are valid.
    // This function can do dangerous things if used wrong.
    CheckMemset();


    // Warnings (Inactivated for now)
    /*

    CheckOperatorEq1();

    // Found implementation in header
    // Since this is not a bug I am not enabling it right now
    //WarningHeaderWithImplementation();

    // Warning upon c-style pointer casts
    // This is not very interesting. It should only be shown upon "-Wall" or similar
    const char *ext = strrchr(FileName, '.');
    if (ext && stricmp(ext,".c"))
        WarningOldStylePointerCast();

    // Use standard functions instead
    WarningIsDigit();

    // Including header
    //WarningIncludeHeader();

    */

    // if (a) delete a;
    WarningRedundantCode();

    // if (condition);
    WarningIf();

    // Dangerous functions, such as 'gets' and 'scanf'
    WarningDangerousFunctions();

    // Clean up tokens..
    while (tokens)
    {
        TOKEN *next = tokens->next;
        free(tokens->str);
        delete tokens;
        tokens = next;
    }
}
//---------------------------------------------------------------------------

void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno)
{
    if (str[0] == 0)
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
}
//---------------------------------------------------------------------------

void combine_2tokens(TOKEN *tok, const char str1[], const char str2[])
{
    if (!(tok && tok->next))
        return;
    if (strcmp(tok->str,str1) || strcmp(tok->next->str,str2))
        return;

    free(tok->str);
    free(tok->next->str);
    tok->str = (char *)malloc(strlen(str1)+strlen(str2)+1);
    strcpy(tok->str, str1);
    strcat(tok->str, str2);

    TOKEN *toknext = tok->next;
    tok->next = toknext->next;
    delete toknext;
}
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
            }

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





void ReportErr(const std::string errmsg)
{
    std::cerr << errmsg << std::endl;
}
//---------------------------------------------------------------------------



bool IsName(const char str[])
{
    return (str[0]=='_' || isalpha(str[0]));
}

bool IsNumber(const char str[])
{
    return isdigit(str[0]);
}

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

static bool match(TOKEN *tok, const std::string pattern)
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
            if (!isdigit(tok->str[0]))
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


TOKEN *gettok(TOKEN *tok, int index)
{
    while (tok && index>0)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}

const char *getstr(TOKEN *tok, int index)
{
    tok = gettok(tok, index);
    return tok ? tok->str : "";
}


std::string FileLine(TOKEN *tok)
{
    std::ostringstream ostr;
    ostr << "[" << Files[tok->FileIndex] << ":" << tok->linenr << "]";
    return ostr.str();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Create statement list
//---------------------------------------------------------------------------

void AppendStatement(STATEMENT::etype Type, TOKEN *tok, std::string Var="")
{
    STATEMENT NewStatement;
    NewStatement.Type = Type;
    NewStatement.Token = tok;
    if (Var.empty())
    {
        NewStatement.VarIndex = 0;
    }
    else
    {
        bool Found = false;
        for (unsigned int i = 0; i < VariableNames.size(); i++)
        {
            if (VariableNames[i] == Var)
            {
                Found = true;
                NewStatement.VarIndex = i;
                break;
            }
        }

        if ( ! Found )
        {
            NewStatement.VarIndex = VariableNames.size();
            VariableNames.push_back(Var);
        }
    }
    Statements.push_back(NewStatement);
}

TOKEN *GotoNextStatement(TOKEN *tok)
{
    if (tok && (tok->str[0]=='{' || tok->str[0]=='}'))
        return tok->next;

    if (tok)
        tok = tok->next;
    int parlevel = 0;
    for (; tok; tok = tok->next)
    {
        if (tok->str[0] == '(')
            parlevel++;
        else if (tok->str[0] == ')')
            parlevel--;
        if (strchr("{}", tok->str[0]))
            break;
        if (parlevel==0 && tok->str[0] == ';')
        {
            while (tok && tok->str[0] == ';')
                tok = tok->next;
            break;
        }
    }
    return tok;
}


void GetVariableName(TOKEN * &Token, std::string &varname)
{
    varname = "";

    if (Token == NULL)
        return;

    if (!IsName(Token->str))
        return;

    // Get variable name..
    std::ostringstream ostr;
    bool array = false;
    bool name  = false;
    for (TOKEN *tok = Token; tok; tok = tok->next)
    {
        const char *str = tok->str;

        if ((array) || (str[0]=='['))
        {
            ostr << str;
            array = (str[0] != ']');
        }

        else
        {
            if (name && IsName(str))
                return;
            name = IsName(str);
            if (!name && strcmp(str,".") && strcmp(str,"->") && strcmp(str,"::"))
            {
                if (str[0] == '(')
                    return;
                Token = tok;
                break;
            }
            ostr << str;
        }
    }

    varname = ostr.str();
}


void CreateStatementList()
{
    // Clear lists..
    VariableNames.clear();
    Statements.clear();

    int indentlevel = 0;
    for (TOKEN *tok = tokens; tok; tok = GotoNextStatement(tok))
    {
        if (tok->str[0] == '{')
        {
            AppendStatement(STATEMENT::OBRACE, tok);
            indentlevel++;
        }
        else if (tok->str[0] == '}')
        {
            AppendStatement(STATEMENT::EBRACE, tok);
            indentlevel--;
        }
        else if (indentlevel >= 1)
        {
            bool hasif = false, hasloop = false, hasswitch = false;

            if (strcmp(tok->str,"if")==0)
            {
                hasif = true;
                AppendStatement(STATEMENT::IF, tok);
            }
            else if (strcmp(tok->str,"else")==0)
            {
                hasif = true;
                if (strcmp(getstr(tok,1),"if")==0)
                    AppendStatement(STATEMENT::ELSEIF, tok);
                else
                    AppendStatement(STATEMENT::ELSE, tok);
            }

            else if (strcmp(tok->str,"do")==0 ||
                     strcmp(tok->str,"while")==0 ||
                     strcmp(tok->str,"for")==0)
            {
                AppendStatement(STATEMENT::LOOP, tok);
                hasloop = true;
            }

            else if (strcmp(tok->str,"switch")==0)
            {
                AppendStatement(STATEMENT::SWITCH, tok);
                hasswitch = true;
            }

            // Declaring variables..
            if (IsName(tok->str) && strcmp(tok->str,"delete") && strcmp(tok->str,"return"))
            {
                const char *str1 = getstr(tok, 1);
                bool decl = IsName(str1) || str1[0]=='*';
                for (TOKEN *tok2 = decl ? tok->next : NULL; tok2; tok2 = tok2->next)
                {
                    if (strchr("{};.", tok2->str[0]))
                        break;

                    const char *str1 = getstr(tok2, 1);
                    if (IsName(tok2->str) && strchr("[=,;",str1[0]))
                    {
                        AppendStatement(STATEMENT::DECL, tok2, tok2->str);
                        while (tok2 && !strchr(",;", tok2->str[0]))
                            tok2 = tok2->next;
                        if (tok2->str[0] == ';')
                            break;
                    }
                }
            }


            // Assign..
            for (TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if (strchr("{};", tok2->str[0]))
                    break;

                TOKEN *eq = tok2;
                std::string varname = "";
                GetVariableName(eq, varname);

                // Equal with..
                if (eq && strcmp(eq->str,"=")==0 && !varname.empty())
                {
                    TOKEN *rs = eq->next;

                    bool ismalloc = false;
                    ismalloc |= match(rs, "strdup (");
                    if (rs->str[0]=='(' && IsName(getstr(rs,1)))
                    {
                        ismalloc |= match(rs, "( type * ) malloc (");
                        ismalloc |= match(rs, "( type * * ) malloc (");
                        ismalloc |= match(rs, "( type type * ) malloc (");
                        ismalloc |= match(rs, "( type type * * ) malloc (");
                    }

                    if ( ismalloc )
                        AppendStatement(STATEMENT::MALLOC, tok2, varname);

                    else if ( match(rs,"new type ;") )
                        AppendStatement(STATEMENT::NEW, tok2, varname);

                    else if ( match(rs, "new type (") )
                        AppendStatement(STATEMENT::NEW, tok2, varname);

                    else if ( match(rs, "new type [") )
                        AppendStatement(STATEMENT::NEWARRAY, tok2, varname);

                    else
                        AppendStatement(STATEMENT::ASSIGN, tok2, varname);

                    tok2 = eq;
                }
            }

            // Delete..
            for (TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if (strchr("{};", tok2->str[0]))
                    break;

                if (match(tok2, "free ( var ) ;"))
                    AppendStatement(STATEMENT::FREE, tok2, getstr(tok2, 2));

                if (match(tok2, "delete var ;"))
                    AppendStatement(STATEMENT::DELETE, tok2, getstr(tok2,1));

                if (match(tok2, "delete [ ] var ;"))
                    AppendStatement(STATEMENT::DELETEARRAY, tok2, getstr(tok2,3));
            }

            // Use..
            bool UseVar = false;
            int parlevel = 0;
            for (TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if (parlevel==0 && strchr("{};", tok2->str[0]))
                    break;

                if (tok2->str[0] == '(')
                    parlevel++;
                if (tok2->str[0] == ')')
                    parlevel--;
                if (parlevel == 0 && tok2->str[0] == ',')
                    UseVar = false;

                else if (tok2->str[0]=='=' || tok2->str[0]=='(')
                    UseVar = true;

                else if (UseVar && IsName(tok2->str))
                {
                    std::string varname = "";
                    GetVariableName(tok2, varname);

                    if (!varname.empty() &&
                        varname!="continue" &&
                        varname!="break" &&
                        varname!="return")
                    {
                        AppendStatement(STATEMENT::USE, tok2, varname);
                    }

                    if (tok2->str[0] == ')')
                        parlevel--;

                    if (parlevel==0 && tok2->str[0]==';')
                        break;
                }
            }

            // Return, continue, break..
            for (TOKEN *tok2 = tok; tok2; tok2 = tok2->next)
            {
                if (strchr("{};", tok2->str[0]))
                    break;

                if (strcmp(tok2->str,"continue")==0)
                {
                    AppendStatement(STATEMENT::CONTINUE, tok2);
                    break;
                }

                if (strcmp(tok2->str,"break")==0)
                {
                    AppendStatement(STATEMENT::BREAK, tok2);
                    break;
                }

                if (strcmp(tok2->str,"return")==0)
                {
                    if (IsName(getstr(tok2,1)) && strcmp(getstr(tok2,2),";")==0)
                        AppendStatement(STATEMENT::RETURN, tok2, getstr(tok2,1));

                    else
                        AppendStatement(STATEMENT::RETURN, tok2, ";");

                    break;
                }
            }

            if (hasif)
            {
                AppendStatement(STATEMENT::ENDIF, tok);
            }
            else if (hasloop)
            {
                AppendStatement(STATEMENT::ENDLOOP, tok);
            }
            else if (hasswitch)
            {
                AppendStatement(STATEMENT::ENDSWITCH, tok);
            }
        }
    }

    if (Debug)
    {
        std::list<STATEMENT>::const_iterator it;
        for (it = Statements.begin(); it != Statements.end(); it++)
        {
            STATEMENT s = *it;
            std::cout << it->Token->linenr << " : ";
            switch (s.Type)
            {
                case STATEMENT::OBRACE:
                    std::cout << "{";
                    break;

                case STATEMENT::EBRACE:
                    std::cout << "}";
                    break;

                case STATEMENT::DECL:
                    std::cout << "decl " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::ASSIGN:
                    std::cout << "assign " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::MALLOC:
                    std::cout << "malloc " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::FREE:
                    std::cout << "free " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::NEW:
                    std::cout << "new " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::NEWARRAY:
                    std::cout << "new[] " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::DELETE:
                    std::cout << "delete " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::DELETEARRAY:
                    std::cout << "delete[] " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::USE:
                    std::cout << "use " << VariableNames[s.VarIndex];
                    break;


                case STATEMENT::LOOP:
                    std::cout << "loop";
                    break;

                case STATEMENT::ENDLOOP:
                    std::cout << "endloop";
                    break;


                case STATEMENT::SWITCH:
                    std::cout << "switch";
                    break;

                case STATEMENT::ENDSWITCH:
                    std::cout << "endswitch";
                    break;


                case STATEMENT::IF:
                    std::cout << "if";
                    break;

                case STATEMENT::ELSEIF:
                    std::cout << "elseif";
                    break;

                case STATEMENT::ELSE:
                    std::cout << "else";
                    break;

                case STATEMENT::ENDIF:
                    std::cout << "endif";
                    break;

                case STATEMENT::RETURN:
                    std::cout << "return " << VariableNames[s.VarIndex];
                    break;

                case STATEMENT::CONTINUE:
                    std::cout << "continue";
                    break;

                case STATEMENT::BREAK:
                    std::cout << "break";
                    break;

                default:
                    std::cout << "ERROR. Unknown code!!";
                    break;
            }
            std::cout << "\n";
        }
    }
}
//---------------------------------------------------------------------------






struct VAR
{
    bool        is_class;
    const char *name;
    bool        init;
    bool        is_pointer;
    struct VAR *next;
};
//---------------------------------------------------------------------------

struct VAR *ClassChecking_GetVarList(const char classname[])
{
    // Locate class..
    const char *pattern[] = {"class","","{",NULL};
    pattern[1] = classname;
    TOKEN *tok1 = findtoken(tokens, pattern);

    // All classes..
    struct _class
    {
        const char *name;
        struct _class *next;
    };
    struct _class *classes = NULL;
    const char *pattern_anyclass[] = {"class","",NULL};
    for (TOKEN *t = findtoken(tokens,pattern_anyclass); t; t = findtoken(t->next,pattern_anyclass))
    {
        _class *newclass = new _class;
        newclass->name = t->next->str;
        newclass->next = classes;
        classes = newclass;
    }

    // Get variable list..
    bool is_class = false;
    bool is_pointer = false;
    struct VAR *varlist = NULL;
    unsigned int indentlevel = 0;
    for (TOKEN *tok = tok1; tok; tok = tok->next)
    {
        if (!tok->next)
            break;

        if (tok->str[0] == '{')
            indentlevel++;
        if (tok->str[0] == '}')
        {
            if (indentlevel <= 1)
                break;
            indentlevel--;
        }

        if (strchr(";{}", tok->str[0]))
            is_class = is_pointer = false;
        else if (IsName(tok->str))
        {
            for (_class *c = classes; c; c = c->next)
                is_class |= (strcmp(c->name, tok->str) == 0);
        }

        if (tok->str[0] == '*')
            is_pointer = true;

        // Member variable?
        if ((indentlevel == 1) &&
            (tok->next->str[0] == ';') &&
            (IsName(tok->str)) &&
            (strcmp(tok->str,"const") != 0 ))
        {
            struct VAR *var = new VAR;
            memset(var, 0, sizeof(struct VAR));
            var->name = tok->str;
            var->next = varlist;
            var->is_class = is_class;
            var->is_pointer = is_pointer;
            varlist   = var;
        }
    }

    while (classes)
    {
        _class *next = classes->next;
        delete classes;
        classes = next;
    }

    return varlist;
}
//---------------------------------------------------------------------------

TOKEN * ClassChecking_VarList_RemoveAssigned(TOKEN *_tokens, struct VAR *varlist, const char classname[], const char funcname[])
{
    // Locate class member function
    const char *pattern[] = {"","::","","(",NULL};
    pattern[0] = classname;
    pattern[2] = funcname;

    // Locate member function implementation..
    TOKEN *ftok = findtoken(_tokens, pattern);
    if (!ftok)
        return NULL;

    bool BeginLine = false;
    bool Assign = false;
    unsigned int indentlevel = 0;
    for (; ftok; ftok = ftok->next)
    {
        if (!ftok->next)
            break;

        // Class constructor.. initializing variables like this
        // clKalle::clKalle() : var(value) { }
        if (indentlevel==0 && strcmp(classname,funcname)==0)
        {
            if (Assign &&
                IsName(ftok->str) &&
                ftok->next->str[0]=='(')
            {
                for (struct VAR *var = varlist; var; var = var->next)
                {
                    if (strcmp(var->name,ftok->str))
                        continue;
                    var->init = true;
                    break;
                }
            }

            Assign |= (ftok->str[0] == ':');
        }


        if (ftok->str[0] == '{')
        {
            indentlevel++;
            Assign = false;
        }

        if (ftok->str[0] == '}')
        {
            if (indentlevel <= 1)
                break;
            indentlevel--;
        }

        if (BeginLine && indentlevel>=1 && IsName(ftok->str))
        {
            // Clearing all variables..
            if (match(ftok,"memset ( this ,"))
            {
                for (struct VAR *var = varlist; var; var = var->next)
                    var->init = true;
            }

            // Calling member function?
            if (ftok->next->str[0] == '(')
                ClassChecking_VarList_RemoveAssigned(tokens, varlist, classname, ftok->str);

            // Assignment of member variable?
            if (strcmp(ftok->next->str, "=") == 0)
            {
                for (struct VAR *var = varlist; var; var = var->next)
                {
                    if (strcmp(var->name,ftok->str))
                        continue;
                    var->init = true;
                    break;
                }
            }

            // Calling member function..
            if (strcmp(ftok->next->str,".")==0 || strcmp(ftok->next->str,"->")==0)
            {
                // The functions 'clear' and 'Clear' are supposed to initialize variable.
                if (stricmp(ftok->next->next->str,"clear") == 0)
                {
                    for (struct VAR *var = varlist; var; var = var->next)
                    {
                        if (strcmp(var->name,ftok->str))
                            continue;
                        var->init = true;
                        break;
                    }
                }
            }
        }

        BeginLine = (strchr("{};", ftok->str[0]));
    }

    return ftok;
}






//---------------------------------------------------------------------------
// Memory leak..
//---------------------------------------------------------------------------

struct _variable
{
    int indentlevel;
    unsigned int varindex;
    enum {Any, Data, Malloc, New, NewA} value;
};

void CheckMemoryLeak()
{
    std::vector<_variable *> varlist;

    // if
    int iflevel = 0;
    bool endif = false;
    std::vector<bool> iflist;

    // loop
    int looplevel = 0;
    bool endloop = false;
    std::vector<bool> looplist;

    // switch
    int switchlevel = 0;
    bool endswitch = false;
    std::vector<bool> switchlist;

    // Parse the statement list and locate memory leaks..
    int indentlevel = 0;
    std::list<STATEMENT>::const_iterator it;
    for (it = Statements.begin(); it != Statements.end(); it++)
    {
        switch (it->Type)
        {
            case STATEMENT::OBRACE:
                indentlevel++;
                iflist.push_back(endif);
                if (endif)
                    iflevel++;
                looplist.push_back(endloop);
                if (endloop)
                    looplevel++;
                switchlist.push_back(endswitch);
                if (endswitch)
                    switchlevel++;
                break;

            case STATEMENT::EBRACE:
                // Check if any variables go out of scope..
                if (indentlevel >= 1)
                {
                    for (unsigned int i = 0; i < varlist.size(); i++)
                    {
                        if (varlist[i]->indentlevel != indentlevel)
                            continue;

                        if (varlist[i]->value == _variable::Malloc || varlist[i]->value == _variable::New || varlist[i]->value == _variable::NewA)
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                            ReportErr(ostr.str());
                        }

                        // Delete this instance..
                        delete varlist[i];
                        varlist.erase(varlist.begin()+i);
                    }
                }

                // if level..
                if (iflist.back())
                    iflevel--;
                iflist.pop_back();

                // loop level..
                if (looplist.back())
                    looplevel--;
                looplist.pop_back();

                // switch level..
                if (switchlist.back())
                    switchlevel--;
                switchlist.pop_back();

                // Make sure the varlist is empty..
                if (indentlevel <= 1)
                    varlist.clear();

                if (indentlevel > 0)
                    indentlevel--;
                break;

            case STATEMENT::DECL:
                if (indentlevel > 0)
                {
                    _variable *NewVar = new _variable;
                    memset(NewVar, 0, sizeof(_variable));
                    NewVar->indentlevel = indentlevel;
                    NewVar->varindex = it->VarIndex;
                    varlist.push_back(NewVar);
                }
                break;

            case STATEMENT::IF:
            case STATEMENT::ELSEIF:
            case STATEMENT::ELSE:
                // Conditional statements..
                iflevel++;
                break;

            case STATEMENT::ENDIF:
                iflevel--;
                break;

            // Not very interested in these..
            case STATEMENT::LOOP:
            case STATEMENT::ENDLOOP:
            case STATEMENT::SWITCH:
            case STATEMENT::ENDSWITCH:
                break;

            case STATEMENT::MALLOC:
            case STATEMENT::NEW:
            case STATEMENT::NEWARRAY:
            case STATEMENT::FREE:
            case STATEMENT::DELETE:
            case STATEMENT::DELETEARRAY:
                // Don't handle conditional allocation at the moment..
                if (iflevel>0 && (it->Type==STATEMENT::MALLOC || it->Type==STATEMENT::NEW || it->Type==STATEMENT::NEWARRAY))
                    break;

                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        bool isalloc = (varlist[i]->value==_variable::Malloc || varlist[i]->value==_variable::New || varlist[i]->value==_variable::NewA);
                        bool alloc = (it->Type==STATEMENT::MALLOC || it->Type==STATEMENT::NEW || it->Type==STATEMENT::NEWARRAY);

                        // Already allocated and then allocated again..
                        //bool err = (isalloc & alloc);

                        if (isalloc && !alloc)
                        {
                            // Check that the deallocation matches..
                            bool err = false;

                            err |= (varlist[i]->value==_variable::Malloc && it->Type!=STATEMENT::FREE);
                            err |= (varlist[i]->value==_variable::New    && it->Type!=STATEMENT::DELETE);
                            err |= (varlist[i]->value==_variable::NewA   && it->Type!=STATEMENT::DELETEARRAY);

                            if (err)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(it->Token) << ": Mismatching allocation and deallocation '" << VariableNames[varlist[i]->varindex] << "'";
                                ReportErr(ostr.str());
                            }
                        }

                        if ( it->Type == STATEMENT::MALLOC )
                            varlist[i]->value = _variable::Malloc;

                        else if ( it->Type == STATEMENT::FREE )
                            varlist[i]->value = _variable::Any;

                        else if ( it->Type == STATEMENT::NEW )
                            varlist[i]->value = _variable::New;

                        else if ( it->Type == STATEMENT::NEWARRAY )
                            varlist[i]->value = _variable::NewA;

                        else if ( it->Type == STATEMENT::DELETE )
                            varlist[i]->value = _variable::Any;

                        else if ( it->Type == STATEMENT::DELETEARRAY )
                            varlist[i]->value = _variable::Any;

                        break;
                    }
                }
                break;

            case STATEMENT::ASSIGN:
            case STATEMENT::USE:
                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        varlist[i]->value = _variable::Data;
                        break;
                    }
                }
                break;

            case STATEMENT::RETURN:
                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        varlist[i]->value = _variable::Any;
                    }

                    else if (varlist[i]->value==_variable::New ||
                             varlist[i]->value==_variable::NewA )
                    {
                        std::ostringstream ostr;
                        ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                        ReportErr(ostr.str());
                        varlist[i]->value = _variable::Any;
                        break;
                    }
                }
                break;

            case STATEMENT::CONTINUE:
            case STATEMENT::BREAK:
                // Memory leak if variable is allocated..
                if (looplevel>0)
                {
                    // Find the last loop..
                    for (int i = looplist.size() - 1; i >= 0; i--)
                    {
                        if (switchlist[i])
                            break;

                        if (!looplist[i])
                            continue;

                        int loop_indentlevel = i + 1;

                        for (i = 0; i < (int)varlist.size(); i++)
                        {
                            if (varlist[i]->indentlevel < loop_indentlevel)
                                continue;

                            if (varlist[i]->value==_variable::Malloc ||
                                varlist[i]->value==_variable::New ||
                                varlist[i]->value==_variable::NewA )
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                                ReportErr(ostr.str());
                            }

                            break;
                        }

                        break;
                    }
                }
                break;
        }

        endif = (it->Type == STATEMENT::ENDIF);
        endloop = (it->Type == STATEMENT::ENDLOOP);
        endswitch = (it->Type == STATEMENT::ENDSWITCH);
    }
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

void CheckBufferOverrun()
{
    int indentlevel = 0;
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;

        else if (indentlevel > 0)
        {
            // Declaring array..
            if (match(tok, "type var [ num ] ;"))
            {
                const char *varname = getstr(tok,1);
                unsigned int size = strtoul(getstr(tok,3), NULL, 10);
                int _indentlevel = indentlevel;
                for (TOKEN *tok2 = gettok(tok,5); tok2; tok2 = tok2->next)
                {
                    if (tok2->str[0]=='{')
                    {
                        _indentlevel++;
                    }
                    else if (tok2->str[0]=='}')
                    {
                        _indentlevel--;
                        if (_indentlevel < indentlevel)
                            break;
                    }
                    else
                    {
                        // Array index..
                        if (strcmp(tok2->str,varname)==0 &&
                            strcmp(getstr(tok2,1),"[")==0 &&
                            IsNumber(getstr(tok2,2)) &&
                            strcmp(getstr(tok2,3),"]")==0 )
                        {
                            const char *str = getstr(tok2, 2);
                            if (strtoul(str, NULL, 10) >= size)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(tok2) << ": Array index out of bounds";
                                ReportErr(ostr.str());
                            }
                        }

                        // Loop..
                        const char *strindex = 0;
                        int value = 0;
                        if (match(tok2,"for ( var = 0 ; var < num ; var + + )"))
                        {
                            strindex = getstr(tok2,2);
                            value = atoi(getstr(tok2,8));
                        }
                        if (strindex && value>size)
                        {
                            TOKEN *tok3 = tok2;
                            while (tok3 && strcmp(tok3->str,")"))
                                tok3 = tok3->next;
                            if (!tok3)
                                break;
                            tok3 = tok3->next;
                            if (tok3->str[0] == '{')
                                tok3 = tok3->next;
                            while (tok3 && !strchr(";}",tok3->str[0]))
                            {
                                if (strcmp(tok3->str,varname)==0 &&
                                    strcmp(getstr(tok3,1),"[")==0 &&
                                    strcmp(getstr(tok3,2),strindex)==0 &&
                                    strcmp(getstr(tok3,3),"]")==0 )
                                {
                                    std::ostringstream ostr;
                                    ostr << FileLine(tok3) << ": Buffer overrun";
                                    ReportErr(ostr.str());
                                    break;
                                }
                                tok3 = tok3->next;
                            }

                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// Check that all class constructors are ok.
//---------------------------------------------------------------------------

void CheckConstructors()
{
    // Locate class
    const char *pattern_classname[] = {"class","","{",NULL};
    TOKEN *tok1 = findtoken(tokens, pattern_classname);
    while (tok1)
    {
        const char *classname = tok1->next->str;

        // Are there a class constructor?
        const char *pattern_constructor[] = {"clKalle","::","clKalle","(",NULL};
        pattern_constructor[0] = classname;
        pattern_constructor[2] = classname;
        if (!findtoken(tokens,pattern_constructor))
        {
            // There's no class implementation, it must be somewhere else.
            tok1 = findtoken( tok1->next, pattern_classname );
            continue;
        }

        // Check that all member variables are initialized..
        struct VAR *varlist = ClassChecking_GetVarList(classname);
        ClassChecking_VarList_RemoveAssigned(tokens, varlist, classname, classname);

        // Check if any variables are uninitialized
        for (struct VAR *var = varlist; var; var = var->next)
        {
            if (!var->init && (var->is_pointer || !var->is_class))
            {
                std::ostringstream ostr;
                ostr << "Uninitialized member variable '" << classname << "::" << var->name << "'";
                ReportErr(ostr.str());
            }
        }

        // Delete the varlist..
        while (varlist)
        {
            struct VAR *nextvar = varlist->next;
            delete varlist;
            varlist = nextvar;
        }

        tok1 = findtoken( tok1->next, pattern_classname );
    }
}



//---------------------------------------------------------------------------
// Check: Unused private functions
//---------------------------------------------------------------------------

void CheckUnusedPrivateFunctions()
{
    // Locate some class
    const char *pattern_class[] = {"class","","{",NULL};
    for (TOKEN *tok1 = findtoken(tokens, pattern_class); tok1; tok1 = findtoken(tok1->next, pattern_class))
    {
        const char *classname = tok1->next->str;

        // The class implementation must be available..
        const char *pattern_classconstructor[] = {"","::","",NULL};
        pattern_classconstructor[0] = classname;
        pattern_classconstructor[2] = classname;
        if (!findtoken(tokens,pattern_classconstructor))
            continue;

        // Get private functions..
        std::list<std::string> FuncList;
        FuncList.clear();
        bool priv = false;
        unsigned int indent_level = 0;
        for (TOKEN *tok = tok1; tok; tok = tok->next)
        {
            if (match(tok,"friend class"))
            {
                // Todo: Handle friend classes
                FuncList.clear();
                break;
            }

            if (tok->str[0] == '{')
                indent_level++;
            if (tok->str[0] == '}')
            {
                if (indent_level <= 1)
                    break;
                indent_level--;
            }
            if (strcmp(tok->str,"};") == 0)
                break;
            if (strcmp(tok->str,"private:") == 0)
                priv = true;
            else if (strcmp(tok->str,"public:") == 0)
                priv = false;
            else if (strcmp(tok->str,"protected:") == 0)
                priv = false;
            else if (priv && indent_level == 1)
            {
                if (isalpha(tok->str[0]) &&
                    tok->next->str[0]=='(' &&
                    strcmp(tok->str,classname) != 0)
                {
                    FuncList.push_back(tok->str);
                }
            }
        }

        // Check that all private functions are used..
        const char *pattern_function[] = {"","::",NULL};
        pattern_function[0] = classname;
        bool HasFuncImpl = false;
        for (TOKEN *ftok = findtoken(tokens, pattern_function); ftok; ftok = findtoken(ftok->next,pattern_function))
        {
            int numpar = 0;
            while (ftok && ftok->str[0]!=';' && ftok->str[0]!='{')
            {
                if (ftok->str[0] == '(')
                    numpar++;
                else if (ftok->str[0] == ')')
                    numpar--;
                ftok = ftok->next;
            }

            if (!ftok)
                break;

            if (ftok->str[0] == ';')
                continue;

            if (numpar != 0)
                continue;

            HasFuncImpl = true;

            indent_level = 0;
            while (ftok)
            {
                if (ftok->str[0] == '{')
                    indent_level++;
                if (ftok->str[0] == '}')
                {
                    if (indent_level<=1)
                        break;
                    indent_level--;
                }
                if (ftok->next && ftok->next->str[0] == '(')
                    FuncList.remove(ftok->str);
                ftok = ftok->next;
            }
        }

        while (HasFuncImpl && !FuncList.empty())
        {
            bool fp = false;

            // Final check; check if the function pointer is used somewhere..
            const char *_pattern[] = {"=","",NULL};
            _pattern[1] = FuncList.front().c_str();
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = "(";
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ")";
            fp |= (findtoken(tokens, _pattern) != NULL);
            _pattern[0] = ",";
            fp |= (findtoken(tokens, _pattern) != NULL);

            if (!fp)
            {
                std::ostringstream ostr;
                ostr << "Class '" << classname << "', unused private function: '" << FuncList.front() << "'";
                ReportErr(ostr.str());
            }
            FuncList.pop_front();
        }
    }
}

//---------------------------------------------------------------------------
// Class: Check that memset is not used on classes
//---------------------------------------------------------------------------

void CheckMemset()
{
    // Locate all 'memset' tokens..
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"memset")!=0)
            continue;

        const char *type = NULL;
        if (match(tok, "memset ( var , num , sizeof ( type ) )"))
            type = getstr(tok, 8);
        else if (match(tok, "memset ( & var , num , sizeof ( type ) )"))
            type = getstr(tok, 9);
        else if (match(tok, "memset ( var , num , sizeof ( struct type ) )"))
            type = getstr(tok, 9);
        else if (match(tok, "memset ( & var , num , sizeof ( struct type ) )"))
            type = getstr(tok, 10);

        // No type defined => The tokens didn't match
        if (!(type && type[0]))
            continue;

        // It will be assumed that memset can be used upon 'this'.
        // Todo: Check this too
        if (strcmp(getstr(tok,2),"this") == 0)
            continue;

        // Warn if type is a class..
        const char *pattern1[] = {"class","",NULL};
        pattern1[1] = type;
        if (findtoken(tokens,pattern1))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Using 'memset' on class.";
            ReportErr(ostr.str());
        }

        // Warn if type is a struct that contains any std::*
        const char *pattern2[] = {"struct","","{",NULL};
        pattern2[1] = type;
        for (TOKEN *tstruct = findtoken(tokens, pattern2); tstruct; tstruct = tstruct->next)
        {
            if (tstruct->str[0] == '}')
                break;

            if (match(tstruct, "std :: type var ;"))
            {
                std::ostringstream ostr;
                ostr << FileLine(tok) << ": Using 'memset' on struct that contains a 'std::" << getstr(tstruct,2) << "'";
                ReportErr(ostr.str());
                break;
            }
        }
    }
}




//---------------------------------------------------------------------------
// Class: "void operator=("
//---------------------------------------------------------------------------

void CheckOperatorEq1()
{
    const char *pattern[] = {"void", "operator", "=", "(", NULL};
    if (TOKEN *tok = findtoken(tokens,pattern))
    {
        std::ostringstream ostr;
        ostr << FileLine(tok) << ": 'operator=' should return something";
        ReportErr(ostr.str());
    }
}




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

        if (match(tok, ") {"))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found implementation in header";
            ReportErr(ostr.str());
        }
    }
}






//---------------------------------------------------------------------------
// Warning on C-Style casts.. p = (kalle *)foo;
//---------------------------------------------------------------------------

void WarningOldStylePointerCast()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Old style pointer casting..
        if (!match(tok, "( type * ) var"))
            continue;

        // Is "type" a class?
        const char *pattern[] = {"class","",NULL};
        pattern[1] = getstr(tok, 1);
        if (!findtoken(tokens, pattern))
            continue;

        std::ostringstream ostr;
        ostr << FileLine(tok) << ": C-style pointer casting";
        ReportErr(ostr.str());
    }
}




//---------------------------------------------------------------------------
// Use standard function "isdigit" instead
//---------------------------------------------------------------------------

void WarningIsDigit()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        bool err = false;
        err |= match(tok, "var >= '0' && var <= '9'");
        err |= match(tok, "* var >= '0' && * var <= '9'");
        err |= match(tok, "( var >= '0' ) && ( var <= '9' )");
        err |= match(tok, "( * var >= '0' ) && ( * var <= '9' )");
        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": The condition can be simplified; use 'isdigit'";
            ReportErr(ostr.str());
        }
    }
}
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
            if (stricmp(Files[hfile].c_str(), includefile) == 0)
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
        bool Needed = false;
        bool NeedDeclaration = false;
        int indentlevel = 0;
        for (TOKEN *tok1 = tokens; tok1; tok1 = tok1->next)
        {
            if (tok1->FileIndex != hfile)
                continue;

            if (!tok1->next)
                continue;

            if (!tok1->next->next)
                continue;

            // I'm only interested in stuff that is declared at indentlevel 0
            if (tok1->str[0] == '{')
                indentlevel++;
            if (tok1->str[0] == '}')
                indentlevel--;
            if (indentlevel != 0)
                continue;

            // Class or namespace declaration..
            if (match(tok1,"class var {") ||
                match(tok1,"class var :") ||
                match(tok1,"namespace var {"))
            {
                std::string classname = getstr(tok1, 1);

                // Try to find class usage in "parent" file..
                for (TOKEN *tok2 = tokens; tok2; tok2 = tok2->next)
                {
                    if (tok2->FileIndex != includetok->FileIndex)
                        continue;

                    // Inheritage..
                    Needed |= match(tok2, "class var : " + classname);
                    Needed |= match(tok2, "class var : type " + classname);

                    // Allocating..
                    Needed |= match(tok2, "new " + classname);

                    // Using class..
                    Needed |= match(tok2, classname + " ::");
                    Needed |= match(tok2, classname + " var");
                    NeedDeclaration |= match(tok2, classname + " *");
                }

                if (Needed | NeedDeclaration)
                    break;
            }

            // Variable..
            std::string varname = "";
            if (match(tok1, "type var ;") || match(tok1, "type var ["))
                varname = getstr(tok1, 1);
            if (match(tok1, "type * var ;") || match(tok1, "type * var ["))
                varname = getstr(tok1, 2);
            if (!varname.empty())
            {
                for (TOKEN *tok2 = tokens; tok2; tok2 = tok2->next)
                {
                    if (tok2->FileIndex != includetok->FileIndex)
                        continue;

                    NeedDeclaration |= (tok2->str == varname);
                    Needed |= match(tok2, varname + " .");
                    Needed |= match(tok2, varname + " ->");
                    Needed |= match(tok2, varname + " =");
                }

                if (Needed | NeedDeclaration)
                    break;
            }

            // enum
            if (match(tok1,"enum var {"))
            {
                std::string enumname = getstr(tok1, 1);

                // Try to find enum usage in "parent" file..
                for (TOKEN *tok2 = tokens; tok2; tok2 = tok2->next)
                {
                    if (tok2->FileIndex != includetok->FileIndex)
                        continue;

                    Needed |= (enumname == tok2->str);
                }

                if (Needed)
                    break;
            }

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
                ostr << " (but a declaration in it is needed)";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Redundant code..
//---------------------------------------------------------------------------

void WarningRedundantCode()
{

    // if (p) delete p
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if"))
            continue;

        const char *varname1 = NULL;
        TOKEN *tok2 = NULL;

        if (match(tok,"if ( var )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 4);
        }
        else if (match(tok,"if ( var != NULL )"))
        {
            varname1 = getstr(tok, 2);
            tok2 = gettok(tok, 6);
        }

        if (varname1==NULL || tok2==NULL)
            continue;

        bool err = false;
        if (match(tok2,"delete var ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (match(tok2,"{ delete var ; }"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"delete [ ] var ;"))
            err = (strcmp(getstr(tok2,1),varname1)==0);
        else if (match(tok2,"{ delete [ ] var ; }"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"free ( var )"))
            err = (strcmp(getstr(tok2,2),varname1)==0);
        else if (match(tok2,"{ free ( var ) ; }"))
            err = (strcmp(getstr(tok2,3),varname1)==0);

        if (err)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Redundant condition. It is safe to deallocate a NULL pointer";
            ReportErr(ostr.str());
        }
    }


    // TODO
    // if (haystack.find(needle) != haystack.end())
    //    haystack.remove(needle); 

}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// if (condition);
//---------------------------------------------------------------------------

void WarningIf()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (strcmp(tok->str,"if")==0)
        {
            int parlevel = 0;
            for (TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
            {
                if (tok2->str[0]=='(')
                    parlevel++;
                else if (tok2->str[0]==')')
                {
                    parlevel--;
                    if (parlevel<=0)
                    {
                        if (strcmp(getstr(tok2,1), ";") == 0)
                        { 
                            std::ostringstream ostr;
                            ostr << FileLine(tok) << ": Found \"if (condition);\"";
                            ReportErr(ostr.str());
                        }
                        break;
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Dangerous functions
//---------------------------------------------------------------------------

void WarningDangerousFunctions()
{
    char str[10];
    str[20] = 0;

    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (match(tok, "gets ("))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found 'gets'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }

        else if (match(tok, "scanf (") && strcmp(getstr(tok,2),"\"%s\"") == 0)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found 'scanf'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }
    }
}
