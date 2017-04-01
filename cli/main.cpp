/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 *
 * @mainpage Cppcheck
 * @version 1.78
 *
 * @section overview_sec Overview
 * Cppcheck is a simple tool for static analysis of C/C++ code.
 *
 * When you write a checker you have access to:
 *  - %Token list - the tokenized code
 *  - Syntax tree - Syntax tree of each expression
 *  - %SymbolDatabase - Information about all types/variables/functions/etc
 *    in the current translation unit
 *  - Library - Configuration of functions/types
 *  - Value flow analysis - Context sensitive analysis that determine possible values for each token
 *
 * Use --debug on the command line to see debug output for the token list
 * and the syntax tree. If both --debug and --verbose is used, the symbol
 * database is also written.
 *
 * The checks are written in C++. The checks are addons that can be
 * easily added/removed.
 *
 * @section writing_checks_sec Writing a check
 * Below is a simple example of a check that detect division with zero:
 * @code
void CheckOther::checkZeroDivision()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // is this a division or modulo?
        if (Token::Match(tok, "[/%]")) {
            // try to get value '0' of rhs
            const ValueFlow::Value *value = tok->astOperand2()->getValue(0);

            // if 'value' is not NULL, rhs can be zero.
            if (value)
                reportError(tok, Severity::error, "zerodiv", "Division by zero");
        }
    }
}
 @endcode
 *
 * The function Token::Match is often used in the checks. Through it
 * you can match tokens against patterns. It is currently not possible
 * to write match expressions that uses the syntax tree, the symbol database,
 * nor the library. Only the token list is used.
 *
 * @section checkclass_sec Creating a new check class from scratch
 * %Check classes inherit from the Check class. The Check class specifies the interface that you must use.
 * To integrate a check class into cppcheck all you need to do is:
 * - Add your source file(s) so they are compiled into the executable.
 * - Create an instance of the class (the Check::Check() constructor registers the class as an addon that Cppcheck then can use).
 *
 *
 * @section embedding_sec Embedding Cppcheck
 * Cppcheck is designed to be easily embeddable into other programs.
 *
 * The "cli/main.cpp" and "cli/cppcheckexecutor.*" files illustrate how cppcheck
 * can be embedded into an application.
 *
 *
 * @section detailed_overview_sec Detailed overview
 * This happens when you execute cppcheck from the command line:
 * -# CppCheckExecutor::check this function executes the Cppcheck
 * -# CppCheck::parseFromArgs parse command line arguments
 *   - The Settings class is used to maintain settings
 *   - Use FileLister and command line arguments to get files to check
 * -# ThreadExecutor create more instances of CppCheck if needed
 * -# CppCheck::check is called for each file. It checks a single file
 * -# Preprocess the file (through Preprocessor)
 *   - Comments are removed
 *   - Macros are expanded
 * -# Tokenize the file (see Tokenizer)
 * -# Run the runChecks of all check classes.
 * -# Simplify the tokenlist (Tokenizer::simplifyTokenList2)
 * -# Run the runSimplifiedChecks of all check classes
 *
 * When errors are found, they are reported back to the CppCheckExecutor through the ErrorLogger interface
 */


#include "cppcheckexecutor.h"

#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
static char exename[1024] = {0};
#endif

/**
 * Main function of cppcheck
 *
 * @param argc Passed to CppCheck::parseFromArgs()
 * @param argv Passed to CppCheck::parseFromArgs()
 * @return What CppCheckExecutor::check() returns.
 */
int main(int argc, char* argv[])
{
    // MS Visual C++ memory leak debug tracing
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    CppCheckExecutor exec;
#ifdef _WIN32
    GetModuleFileNameA(nullptr, exename, sizeof(exename)/sizeof(exename[0])-1);
    argv[0] = exename;
#endif

#ifdef NDEBUG
    try {
#endif
        return exec.check(argc, argv);
#ifdef NDEBUG
    } catch (const InternalError& e) {
        std::cout << e.errorMessage << std::endl;
    } catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception" << std::endl;
    }
    return EXIT_FAILURE;
#endif
}


// Warn about deprecated compilers
#ifdef __clang__
#   if ( __clang_major__ < 2 || ( __clang_major__  == 2 && __clang_minor__ < 9))
#       warning "Using Clang 2.8 or earlier. Support for this version will be removed soon."
#   endif
#elif defined(__GNUC__)
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 4))
#       warning "Using GCC 4.3 or earlier. Support for this version will be removed soon."
#   endif
#elif defined(_MSC_VER)
#   if (_MSC_VER < 1600)
#       pragma message("WARNING: Using Visual Studio 2008 or earlier. Support for this version will be removed soon.")
#   endif
#endif
