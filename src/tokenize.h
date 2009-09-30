/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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


//---------------------------------------------------------------------------
#ifndef tokenizeH
#define tokenizeH
//---------------------------------------------------------------------------
#include <list>
#include <string>
#include <map>
#include <vector>
#include "classinfo.h"

class Token;
class ErrorLogger;
class Settings;

/// @addtogroup Core
/// @{

/** @brief The main purpose is to tokenize the source code. It also has functions that simplify the token list */
class Tokenizer
{
private:
    /** Deallocate lists.. */
    void deallocateTokens();

public:
    Tokenizer();
    Tokenizer(const Settings * settings, ErrorLogger *errorLogger);
    ~Tokenizer();

    /**
     * Tokenize code
     * @param code input stream for code, e.g.
     * \code
     * #file "p.h"
     * class Foo
     * {
     * private:
     * void Bar();
     * };
     *
     * #endfile
     * void Foo::Bar()
     * {
     * }
     * \endcode
     *
     * @param FileName The filename
     * @return false if Source code contains syntax errors
     */
    bool tokenize(std::istream &code, const char FileName[]);

    /**
     * Create tokens from code.
     * @param code input stream for code, same as what tokenize()
     */
    void createTokens(std::istream &code);

    /** Set variable id */
    void setVarId();

    /** Simplify tokenlist */
    void simplifyTokenList();

    static void deleteTokens(Token *tok);
    static const char *getParameterName(const Token *ftok, int par);

    std::string fileLine(const Token *tok) const;

    // Return size.
    int sizeOfType(const char type[]) const;

    const std::vector<std::string> *getFiles() const;

    void fillFunctionList();
    const Token *getFunctionTokenByName(const char funcname[]) const;
    const Token *tokens() const;

    std::string file(const Token *tok) const;

    /**
     * Find a class member function
     * @param tok where to begin the search
     * @param classname name of class
     * @param funcname name of function ("~ Fred" => destructor for fred, "%var%" => any function)
     * @param indentlevel Just an integer that you initialize to 0 before the first call.
     * @return First matching token or NULL.
     */
    static const Token *findClassFunction(const Token *tok, const char classname[], const char funcname[], int &indentlevel);

    /**
     * List of classes in currently checked source code and
     * their member functions and member variables.
     */
    std::map<std::string, ClassInfo> _classInfoList;

#ifndef _MSC_VER
private:
#endif

    /**
     * Replace sizeof() to appropriate size.
     */
    void simplifySizeof();

    /**
     * Simplify variable declarations
     */
    void simplifyVarDecl();

    /**
     * insert an "int" after "unsigned" if needed:
     * "unsigned i" => "unsigned int i"
     * "signed int i" => "int i"
     * "signed i" => "int i"
     */
    void unsignedint();

    /**
     * Simplify question mark - colon operator
     * Example: 0 ? (2/0) : 0 => 0
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyQuestionMark();

    /**
     * simplify if-assignments..
     * Example: "if(a=b);" => "a=b;if(a);"
     */
    void simplifyIfAssign();

    /**
     * simplify if-not..
     * Example: "if(0==x);" => "if(!x);"
     */
    void simplifyIfNot();

    /**
     * simplify if-not NULL..
     * Example: "if(0!=x);" => "if(x);"
     */
    void simplifyIfNotNull();

    /**
     * Simplify the "not" and "and" keywords to "!" and "&&"
     * accordingly.
     * Examples:
     *     "if (not p)" => "if (!p)"
     *     "if (p and q)" => "if (p && q)"
     */
    void simplifyLogicalOperators();

    /**
     * Simplify comma into a semicolon when possible
     * Example: "delete a, delete b" => "delete a; delete b;"
     * Example: "a = 0, b = 0;" => "a = 0; b = 0;"
     * Example: "return a(), b;" => "a(); return b;"
     */
    void simplifyComma();

    /** Add braces to an if-block
     */
    void simplifyIfAddBraces();

    /**
     * Add braces to an do-while block
     */
    void simplifyDoWhileAddBraces();

    /**
     * typedef A mytype;
     * mytype c;
     *
     * Becomes:
     * typedef A mytype;
     * A c;
     */
    void simplifyTypedef();

    /**
     * Simplify casts
     */
    void simplifyCasts();

    /**
     * A simplify function that replaces a variable with its value in cases
     * when the value is known. e.g. "x=10; if(x)" => "x=10;if(10)"
     *
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyKnownVariables();

    /** Replace a "goto" with the statements */
    void simplifyGoto();

    /** Expand nested strcat() calls. */
    void simplifyNestedStrcat();

    /** Simplify "if else" */
    void elseif();

    std::vector<const Token *> _functionList;

    /**
     * Finds matching "end" for "start".
     * @param tok The start tag
     * @param start e.g. "{"
     * @param end e.g. "}"
     * @return The end tag that matches given parameter or 0 if not found.
     */
    static const Token *findClosing(const Token *tok, const char *start, const char *end);

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

    /**
     * Simplify the operator "?:"
     */
    void simplifyConditionOperator();

    /** Simplify conditions
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyConditions();

    /** Remove reduntant code, e.g. if( false ) { int a; } should be
     * removed, because it is never executed.
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool removeReduntantConditions();

    /** Simplify function calls - constant return value
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyFunctionReturn();

    /**
     * Remove redundant paranthesis:
     * - "((x))" => "(x)"
     * - "(function())" => "function()"
     * - "(delete x)" => "delete x"
     * - "(delete [] x)" => "delete [] x"
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyRedundantParanthesis();

    /**
     * Simplify constant calculations such as "1+2" => "3"
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyCalculations();

    /**
     * Simplify functions like "void f(x) int x; {"
     * into "void f(int x) {"
     */
    void simplifyFunctionParameters();

    /**
     * Simplify namespaces by removing them, e.g.
     * "namespace b{ void f(){} }" becomes "void f(){}"
     */
    void simplifyNamespaces();

    /**
     * Simplify templates
     */
    void simplifyTemplates();

    /**
     * Simplify e.g. 'atol("0")' into '0'
     */
    void simplifyMathFunctions();

    /**
     * Modify strings in the token list by replacing hex and oct
     * values. E.g. "\x61" -> "a" and "\000" -> "\0"
     * @param source The string to be modified, e.g. "\x61"
     * @return Modified string, e.g. "a"
     */
    std::string simplifyString(const std::string &source);

    void insertTokens(Token *dest, const Token *src, unsigned int n);

    /**
     * Setup links for tokens so that one can call Token::link().
     *
     * @return false if there was a mismatch with tokens, this
     * should mean that source code was not valid.
     */
    bool createLinks();

    void syntaxError(const Token *tok, char c);

    /**
     * Update _classInfoList to contain class names and member
     * functions and member variables.
     */
    void updateClassList();

    /** Disable assignments.. */
    Tokenizer(const Tokenizer &);

    /**
     * assert that tokens are ok - used during debugging for example
     * to catch problems in simplifyTokenList.
     * @return always true.
     */
    bool validate() const;

    /** Disable assignment operator */
    void operator=(const Tokenizer &);

    Token *_tokens, *_tokensBack;
    std::map<std::string, unsigned int> _typeSize;
    std::vector<std::string> _files;
    const Settings * const _settings;
    ErrorLogger * const _errorLogger;
};

/// @}

//---------------------------------------------------------------------------
#endif
