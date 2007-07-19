

// Todo: Output progress? Using commandline option "--progress"?

#include <iostream>
#include <sstream>

#include "tokenize.h"   // <- Tokenizer
#include "Statements.h" // <- Statement list

#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"
#include "CheckOther.h"

//---------------------------------------------------------------------------
bool Debug = false;
bool ShowWarnings = false;
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
#ifdef __linux__
        if(strcasecmp(argv[i],"--debug") == 0)
#else
        if (stricmp(argv[i],"--debug") == 0)
#endif
            Debug = true;

#ifdef __linux__
        else if (strcasecmp(argv[i],"-w") == 0)
#else
        else if (stricmp(argv[i],"-w") == 0)
#endif
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

extern bool HasErrors;

static void CppCheck(const char FileName[])
{
    HasErrors = false;

    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);


    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckMemset();


    if ( ShowWarnings )
    {
        // Including header which is not needed
        // Todo: This is really slow!
        WarningIncludeHeader();
    }


    SimplifyTokenList();


    // Create a statement list. It's used by for example 'CheckMemoryLeak'
    CreateStatementList();


    // Memory leak
    CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrun();



    // Warnings
    if (ShowWarnings)
    {
        // Check that all private functions are called.
        CheckUnusedPrivateFunctions();

        // Found implementation in header
        WarningHeaderWithImplementation();

        // Warning upon c-style pointer casts
        const char *ext = strrchr(FileName, '.');
#ifdef __linux__
        if (ext && strcasecmp(ext,".c"))
#else
        if (ext && stricmp(ext,".c"))
#endif
            WarningOldStylePointerCast();

        // Use standard functions instead
        WarningIsDigit();
        WarningIsAlpha();

        CheckOperatorEq1();

        // Check that all class constructors are ok.
        // Temporarily inactivated to avoid any false positives
        //CheckConstructors();

        // if (a) delete a;
        WarningRedundantCode();

        // if (condition);
        WarningIf();

        // Dangerous usage of strtok
        WarningStrTok();
    }


    // Dangerous functions, such as 'gets' and 'scanf'
    WarningDangerousFunctions();

    // Invalid function usage..
    InvalidFunctionUsage();

    // Clean up tokens..
    DeallocateTokens();

    // Todo: How should this work? Activated by a command line switch?
    //if ( ! HasErrors )
    //    std::cout << "No errors found\n";
}
//---------------------------------------------------------------------------



















