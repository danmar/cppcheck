/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
#include "settings.h"
#include "errorlogger.h"
#include "token.h"

class Tokenizer
{
private:
    // Deallocate lists..
    void DeallocateTokens();

public:
    Tokenizer();
    ~Tokenizer();

    /**
     * Tokenize code
     * @param code input stream for code
     * @param FileName The filename
     */
    void tokenize(std::istream &code, const char FileName[]);

    /** Set variable id */
    void setVarId();

    /** Simplify tokenlist */
    void simplifyTokenList();


    // Helper functions for handling the tokens list..

    static void deleteTokens(Token *tok);
    static const char *getParameterName(const Token *ftok, int par);

    static bool SameFileName(const char fname1[], const char fname2[]);


    std::string fileLine(const Token *tok) const;

    // Return size.
    int SizeOfType(const char type[]) const;

    void initTokens();

    const std::vector<std::string> *getFiles() const;

    void fillFunctionList();
    const Token *GetFunctionTokenByName(const char funcname[]) const;
    const Token *tokens() const;

    std::string file(const Token *tok) const;

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
     * Remove redundant paranthesis: "((x))" => "(x)"
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

    void InsertTokens(Token *dest, Token *src, unsigned int n);

    Token *_tokensBack;
    std::map<std::string, unsigned int> _typeSize;
    std::vector<std::string> _files;
    Token *_tokens;
};

//---------------------------------------------------------------------------
#endif
