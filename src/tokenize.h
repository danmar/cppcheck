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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


//---------------------------------------------------------------------------
#ifndef tokenizeH
#define tokenizeH
//---------------------------------------------------------------------------
#include <list>
#include <string>
#include <map>
#include <vector>

class Token;
class ErrorLogger;
class Settings;

class Tokenizer
{
private:
    // Deallocate lists..
    void deallocateTokens();

public:
    Tokenizer();
    Tokenizer(const Settings * settings, ErrorLogger *errorLogger);
    ~Tokenizer();

    /**
     * Tokenize code
     * @param code input stream for code, e.g.
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


    // Helper functions for handling the tokens list..

    static void deleteTokens(Token *tok);
    static const char *getParameterName(const Token *ftok, int par);

    std::string fileLine(const Token *tok) const;

    // Return size.
    int sizeOfType(const char type[]) const;

    void initTokens();

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
     * Simplify variable declarations
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyVarDecl();

    /**
     * insert an "int" after "unsigned" if needed:
     * "unsigned i" => "unsigned int i"
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
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyIfAssign();

    /**
     * simplify if-not..
     * Example: "if(0==x);" => "if(!x);"
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyIfNot();

    /**
     * simplify the "not" keyword to "!"
     * Example: "if (not p)" => "if (!p)"
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyNot();

    /**
     * Simplify comma into a semicolon when possible
     * Example: "delete a, delete b" => "delete a; delete b;"
     * Example: "a = 0, b = 0;" => "a = 0; b = 0;"
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyComma();

protected:

    /** Add braces to an if-block
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyIfAddBraces();

    /** Simplify casts
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyCasts();

    /**
     * A simplify function that replaces a variable with its value in cases
     * when the value is known. e.g. "x=10; if(x)" => "x=10;if(10)"
     *
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyKnownVariables();

    /** Simplify "if else" */
    bool elseif();

    std::vector<const Token *> _functionList;

private:

    /**
     * Finds matching "end" for "start".
     * @param tok The start tag
     * @param start e.g. "{"
     * @param end e.g. "}"
     * @return The end tag that matches given parameter or 0 if not found.
     */
    static const Token *findClosing(const Token *tok, const char *start, const char *end);

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

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
    bool simplifyFunctionParameters();

    /**
     * Simplify namespaces by removing them, e.g.
     * "namespace b{ void f(){} }" becomes "void f(){}"
     */
    void simplifyNamespaces();

    /**
     * Simplify templates
     */
    void simplifyTemplates();

    void insertTokens(Token *dest, Token *src, unsigned int n);

    /**
     * Setup links for tokens so that one can call Token::link().
     *
     * @return false if there was a mismatch with tokens, this
     * should mean that source code was not valid.
     */
    bool createLinks();

    void syntaxError(const Token *tok, char c);

    Token *_tokens, *_tokensBack;
    std::map<std::string, unsigned int> _typeSize;
    std::vector<std::string> _files;
    const Settings * const _settings;
    ErrorLogger * const _errorLogger;
};

//---------------------------------------------------------------------------
#endif
