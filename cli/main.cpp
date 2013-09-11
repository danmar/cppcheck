/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
 * @version 1.61.99
 *
 * @section overview_sec Overview
 * Cppcheck is a simple tool for static analysis of C/C++ code.
 *
 * The method used is to first tokenize the source code and then analyse the token list.
 * In the token list, the tokens are stored in plain text.
 *
 * The checks are written in C++. The checks are addons that can be easily added/removed.
 *
 * @section writing_checks_sec Writing a check
 * Below is a simple example of a check that detect division with zero:
 * @code
void CheckOther::checkZeroDivision()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "/ 0"))
            reportError(tok, Severity::error, "zerodiv", "Division by zero");
    }
}
 @endcode
 *
 * The function Token::Match is often used in the checks. Through it
 * you can match tokens against patterns.
 *
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
 * -# Simplify the tokenlist (Tokenizer::simplifyTokenList)
 * -# Run the runSimplifiedChecks of all check classes
 *
 * When errors are found, they are reported back to the CppCheckExecutor through the ErrorLogger interface
 */


#include "cppcheckexecutor.h"

/**
 * Main function of cppcheck
 *
 * @param argc Passed to CppCheck::parseFromArgs()
 * @param argv Passed to CppCheck::parseFromArgs()
 * @return What CppCheckExecutor::check() returns.
 */
int main(int argc, char* argv[])
{
    CppCheckExecutor exec;
    return exec.check(argc, argv);
}
