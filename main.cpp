
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
bool ShowAll = false;
bool CheckCodingStyle = false;
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
        if (strcmp(argv[i],"--debug") == 0)
            Debug = true;

        // Show all messages
        else if (strcmp(argv[i],"--all") == 0)
            ShowAll = true;

        // Checking coding style.
        else if (strcmp(argv[i],"--style")==0)
            CheckCodingStyle = true;

        else
            fname = argv[i];
    }

    if (!fname)
    {
        std::cout << "C/C++ code checking.\n"
                     "\n"
                     "Syntax:\n"
                     "    cppcheck [--all] [--style] filename\n"
                     "\n"
                     "Options:\n"
                     "    --all    Normally a message is only shown if cppcheck is sure\n"
                     "             it has found a bug.\n"
                     "             When this option is given, all messages are shown.\n"
                     "\n"
                     "    --style  Check coding style.\n";
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

    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);


    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckMemset();


    // Including header which is not needed
    if ( CheckCodingStyle )
        WarningIncludeHeader();


    SimplifyTokenList();


    // Create a statement list. It's used by for example 'CheckMemoryLeak'
    CreateStatementList();


    // Memory leak
    CheckMemoryLeak();

    // Buffer overruns..
    CheckBufferOverrun();

    // Check that all class constructors are ok.
    CheckConstructors();

    if (ShowAll)
    {
        // Check for "if (a=b)"
        CheckIfAssignment();

        // Check for case without break
        // Disabled because it generates many false positives
        // CheckCaseWithoutBreak();

        // Dangerous usage of strtok
        // Disabled because it generates false positives
        //WarningStrTok();
    }



    // Dangerous functions, such as 'gets' and 'scanf'
    WarningDangerousFunctions();


    // Invalid function usage..
    InvalidFunctionUsage();


    if (CheckCodingStyle)
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

        // if (a) delete a;
        WarningRedundantCode();

        // if (condition);
        WarningIf();
    }


    // Clean up tokens..
    DeallocateTokens();

    // Todo: How should this work? Activated by a command line switch?
    if ( ! HasErrors )
        std::cout << "No errors found\n";
}
//---------------------------------------------------------------------------




















