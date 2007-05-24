

// Todo: Output progress? Using commandline option "--progress"?

#include <iostream>
#include <sstream>

#include "tokenize.h"   // <- Tokenizer
#include "statements.h" // <- Statement list

#include "CommonCheck.h"

#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"

//---------------------------------------------------------------------------
bool Debug = false;
static bool ShowWarnings = false;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------



// Casting
void WarningOldStylePointerCast();

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
    const char *fname = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (stricmp(argv[i],"--debug") == 0)
            Debug = true;

        else if (stricmp(argv[i],"-w") == 0)
            ShowWarnings = true;

        else
            fname = argv[i];
    }

    if (!fname)
    {
        std::cout << "checkcode [-w] filename\n";
        std::cout << "-w : enables extra warnings\n";
        return 0;
    }

    CppCheck(fname);

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

    // Check that all private functions are called.
    // Temporarily inactivated to avoid any false positives
    CheckUnusedPrivateFunctions();

    // Check that the memsets are valid.
    // This function can do dangerous things if used wrong.
    CheckMemset();


    // Warnings
    if (ShowWarnings)
    {
        // Found implementation in header
        WarningHeaderWithImplementation();

        // Warning upon c-style pointer casts
        const char *ext = strrchr(FileName, '.');
        if (ext && stricmp(ext,".c"))
            WarningOldStylePointerCast();

        // Use standard functions instead
        WarningIsDigit();

        // Including header
        //WarningIncludeHeader();

        CheckOperatorEq1();

        // Check that all class constructors are ok.
        // Temporarily inactivated to avoid any false positives
        //CheckConstructors();
    }




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






bool IsName(const char str[])
{
    return (str[0]=='_' || std::isalpha(str[0]));
}

bool IsNumber(const char str[])
{
    return std::isdigit(str[0]);
}

//---------------------------------------------------------------------------












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



