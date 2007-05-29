

// Todo: Output progress? Using commandline option "--progress"?

#include <iostream>
#include <sstream>

#include "tokenize.h"   // <- Tokenizer
#include "statements.h" // <- Statement list

#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"
#include "CheckOther.h"

//---------------------------------------------------------------------------
bool Debug = false;
static bool ShowWarnings = false;
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[]);


//---------------------------------------------------------------------------
// Main function of cppcheck
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[])
{
    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);


    // Check that the memsets are valid.
    // This function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckMemset();


    SimplifyTokenList();


    // Create a statement list. It's used by for example 'CheckMemoryLeak'
    CreateStatementList();


    // Memory leak
    CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrun();


    // Check that all private functions are called.
    // Temporarily inactivated to avoid any false positives
    CheckUnusedPrivateFunctions();


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



















