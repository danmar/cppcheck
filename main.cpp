
#include "tokenize.h"   // <- Tokenizer
#include "CommonCheck.h"

#include "CheckMemoryLeak.h"
#include "CheckBufferOverrun.h"
#include "CheckClass.h"
#include "CheckHeaders.h"
#include "CheckOther.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#ifdef __BORLANDC__
#include <dir.h>
#else
#include <glob.h>
#endif

//---------------------------------------------------------------------------
bool Debug = false;
bool ShowAll = false;
bool CheckCodingStyle = false;
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId);


static void AddFiles( std::vector<std::string> &filenames, const char path[], const char pattern[] )
{
    #ifdef __BORLANDC__
    struct ffblk f;
    for ( int done = findfirst(pattern, &f, 0); ! done; done = findnext(&f) )
    {
        std::ostringstream fname;
        fname << path << f.ff_name;
        filenames.push_back( fname.str() );
    }
    findclose(&f);
    #else
    glob_t glob_results;
    glob(pattern, 0, 0, &glob_results);
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        std::ostringstream fname;
        fname << path << glob_results.gl_pathv[i];
        filenames.push_back( fname.str() );
    }
    globfree(&glob_results);
    #endif
}

static void RecursiveAddFiles( std::vector<std::string> &filenames, const char path[] )
{
    AddFiles( filenames, path, "*.cpp" );
    AddFiles( filenames, path, "*.cc" );
    AddFiles( filenames, path, "*.c" );

    #ifdef __BORLANDC__
    struct ffblk f ;
    for ( int done = findfirst("*", &f, FA_DIREC); ! done; done = findnext(&f) )
    {
        if ( f.ff_attrib != FA_DIREC || f.ff_name[0] == '.' )
            continue;
        chdir( f.ff_name );
        std::ostringstream curdir;
        curdir << path << f.ff_name << "/";
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    findclose(&f);
    #else
    // gcc / cygwin..
    glob_t glob_results;
    #ifdef __GNUC__
    // gcc..
    glob("*", GLOB_ONLYDIR, 0, &glob_results);
    #else
    // cygwin..
    glob("*", 0, 0, &glob_results);
    #endif
    for ( unsigned int i = 0; i < glob_results.gl_pathc; i++ )
    {
        const char *dirname = glob_results.gl_pathv[i];
        if ( dirname[0] == '.' )
            continue;

        chdir( dirname );
        std::ostringstream curdir;
        curdir << path << dirname << "/";
        RecursiveAddFiles( filenames, curdir.str().c_str() );
        chdir( ".." );
    }
    globfree(&glob_results);
    #endif
}

//---------------------------------------------------------------------------
// Main function of cppcheck
//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;

    bool Recursive = false;

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

        else if (strcmp(argv[i],"--recursive")==0)
            Recursive = true;

        else if (strchr(argv[i],'*'))
        {
            AddFiles( filenames, "", argv[i] );
        }

        else
        {
            filenames.push_back( argv[i] );
        }
    }

    // No filename given.. automaticly search for available files.
    if ( Recursive )
        RecursiveAddFiles( filenames, "" );

    if (filenames.empty())
    {
        std::cout << "C/C++ code checking.\n"
                     "\n"
                     "Syntax:\n"
                     "    cppcheck [--all] [--style] [--recursive] [filename1] [filename2]\n"
                     "\n"
                     "Options:\n"
                     "    --all    Normally a message is only shown if cppcheck is sure\n"
                     "             it has found a bug.\n"
                     "             When this option is given, all messages are shown.\n"
                     "\n"
                     "    --style  Check coding style.\n"
                     "    --recursive  Recursively check all *.cpp, *.cc and *.c files\n";
        return 0;
    }

    std::sort( filenames.begin(), filenames.end() );

    for (unsigned int c = 0; c < filenames.size(); c++)
    {
        errout.str("");
        CppCheck(filenames[c].c_str(), c);
        std::cerr << errout.str();
    }

    if ( CheckCodingStyle && filenames.size() > 1 )
    {
        errout.str("");
        CheckGlobalFunctionUsage(filenames);
        if ( ! errout.str().empty() )
        {
            std::cerr << "\n";
            std::cerr << errout.str();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

static void CppCheck(const char FileName[], unsigned int FileId)
{
    OnlyReportUniqueErrors = true;

    std::cout << "Checking " << FileName << "...\n";

    // Tokenize the file
    tokens = tokens_back = NULL;
    Files.clear();
    Tokenize(FileName);

    FillFunctionList(FileId);

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    CheckMemset();


    // Check for unwanted unsigned division
    // Not accurate yet. Very important to run it before 'SimplifyTokenList'
    if ( ShowAll )
        CheckUnsignedDivision();


    // Including header which is not needed (too many false positives)
    //if ( CheckCodingStyle )
    //    WarningIncludeHeader();


    SimplifyTokenList();

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
        if (ext && strcmp(ext,".cpp")==0)
            WarningOldStylePointerCast();

        // Use standard functions instead
        WarningIsDigit();
        WarningIsAlpha();

        CheckOperatorEq1();

        // if (a) delete a;
        WarningRedundantCode();

        // if (condition);
        WarningIf();

        // Variable scope
        CheckVariableScope();
    }


    // Clean up tokens..
    DeallocateTokens();

    if ( errout.str().empty() )
        std::cout << "No errors found\n";
}
//---------------------------------------------------------------------------




