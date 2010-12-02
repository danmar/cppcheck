/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <string>
#include <map>
#include <vector>

class Token;
class ErrorLogger;
class Settings;
class SymbolDatabase;

/// @addtogroup Core
/// @{

/** @brief The main purpose is to tokenize the source code. It also has functions that simplify the token list */
class Tokenizer
{
private:
    /** Deallocate lists */
    void deallocateTokens();

public:
    Tokenizer();
    Tokenizer(const Settings * settings, ErrorLogger *errorLogger);
    virtual ~Tokenizer();

    /** Is the code JAVA/C#. Used for bailouts */
    bool isJavaOrCSharp() const
    {
        if (_files.size() != 1)
            return false;
        const std::string::size_type pos = _files[0].rfind(".");
        if (pos != std::string::npos)
            return (_files[0].substr(pos) == ".java" ||
                    _files[0].substr(pos) == ".cs");
        return false;
    }

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
     * @param configuration E.g. "A" for code where "#ifdef A" is true
     * @return false if Source code contains syntax errors
     */
    bool tokenize(std::istream &code,
                  const char FileName[],
                  const std::string &configuration = "",
                  const bool preprocessorCondition = false);

    /**
     * Create tokens from code.
     * The code must be preprocessed first:
     * - multiline strings are not handled.
     * - UTF in the code are not handled.
     * - comments are not handled.
     * @param code input stream for code
     */
    void createTokens(std::istream &code);

    /** Set variable id */
    void setVarId();

    /**
     * Simplify tokenlist
     *
     * @return false if there is an error that requires aborting
     * the checking of this file.
     */
    bool simplifyTokenList();

    static void deleteTokens(Token *tok);
    static const char *getParameterName(const Token *ftok, unsigned int par);

    std::string fileLine(const Token *tok) const;

    /**
     * Calculates sizeof value for given type.
     * @param type Token which will contain e.g. "int", "*", or string.
     * @return sizeof for given type, or 0 if it can't be calculated.
     */
    unsigned int sizeOfType(const Token *type) const;

    const std::vector<std::string> *getFiles() const;

    void fillFunctionList();
    const Token *getFunctionTokenByName(const char funcname[]) const;
    const Token *tokens() const;

    std::string file(const Token *tok) const;

    /**
     * get error messages
     */
    virtual void getErrorMessages();

    /** Simplify assignment in function call "f(x=g());" => "x=g();f(x);"
     */
    void simplifyAssignmentInFunctionCall();

    /**
     * Simplify constant calculations such as "1+2" => "3"
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyCalculations();

    /** Insert array size where it isn't given */
    void arraySize();

    /** Simplify labels */
    void labels();

    /** Remove redundant assignment */
    void removeRedundantAssignment();

    /**
     * Replace sizeof() to appropriate size.
     */
    void simplifySizeof();

    /**
     * Simplify variable declarations (split up)
     */
    void simplifyVarDecl();

    /**
     * Simplify variable initialization
     * ; int *p(0);
     * =>
     * ; int *p = 0;
     */
    void simplifyInitVar();
    Token * initVar(Token * tok);

    /**
     * Colapse compound standard types into a single token.
     * unsigned long long int => long _isUnsigned=true,_isLong=true
     */
    void simplifyStdType();

    /**
     * Simplify question mark - colon operator
     * Example: 0 ? (2/0) : 0 => 0
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyQuestionMark();

    /**
     * Simplify compound assignments
     * Example: ";a+=b;" => ";a=a+b;"
     */
    void simplifyCompoundAssignment();

    /**
     * simplify if-assignments
     * Example: "if(a=b);" => "a=b;if(a);"
     */
    void simplifyIfAssign();

    /**
     * Simplify multiple assignmetns.
     * Example: "a = b = c = 0;" => "a = 0; b = 0; c = 0;"
     */
    void simplifyVariableMultipleAssign();

    /**
     * simplify if-not
     * Example: "if(0==x);" => "if(!x);"
     */
    void simplifyIfNot();

    /**
     * simplify if-not NULL
     * - "if(0!=x);" => "if(x);"
     */
    void simplifyIfNotNull();

    /** @brief simplify if (a) { if (a) */
    void simplifyIfSameInnerCondition();

    /**
     * Simplify the "not" and "and" keywords to "!" and "&&"
     * accordingly.
     * Examples:
     * - "if (not p)" => "if (!p)"
     * - "if (p and q)" => "if (p && q)"
     */
    bool simplifyLogicalOperators();

    /**
     * Simplify comma into a semicolon when possible:
     * - "delete a, delete b" => "delete a; delete b;"
     * - "a = 0, b = 0;" => "a = 0; b = 0;"
     * - "return a(), b;" => "a(); return b;"
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

    SymbolDatabase * _symbolDatabase;

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno, bool split = false);
    void addtoken(const Token *tok, const unsigned int lineno, const unsigned int fileno);

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

    /**
     * Reduces "; ;" to ";", except in "( ; ; )"
     */
    void removeRedundantSemicolons();

    /** Simplify function calls - constant return value
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyFunctionReturn();

    /** Struct initialization */
    void simplifyStructInit();

    /** Struct simplification
     * "struct S { } s;" => "struct S { }; S s;"
     */

    void simplifyStructDecl();

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

    /** Simplify references */
    void simplifyReference();

    /**
     * Simplify functions like "void f(x) int x; {"
     * into "void f(int x) {"
     */
    void simplifyFunctionParameters();

    /**
     * Simplify templates
     */
    void simplifyTemplates();

    /**
     * Used after simplifyTemplates to perform a little cleanup.
     * Sometimes the simplifyTemplates isn't fully successful and then
     * there are function calls etc with "wrong" syntax.
     */
    void simplifyTemplates2();

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

    /**
     * Use "<" comparison instead of ">"
     * Use "<=" comparison instead of ">="
     */
    void simplifyComparisonOrder();

    /**
     * Change "int const x;" into "const int x;"
     */
    void simplifyConst();

    /**
     * simplify "while (0)"
     */
    void simplifyWhile0();

    /**
     * Simplify while(func() && errno==EINTR)
     */
    void simplifyErrNoInWhile();

    /**
     * Simplify while(func(f))
     */
    void simplifyFuncInWhile();

    /**
     * Replace enum with constant value
     */
    void simplifyEnum();

    /**
     * Remove "std::" before some function names
     */
    void simplifyStd();

    /** Simplify function pointers */
    void simplifyFunctionPointers();

    /**
     * Remove exception specifications. This function calls itself recursively.
     * @param tok First token in scope to cleanup
     */
    void removeExceptionSpecifications(Token *tok) const;

    void insertTokens(Token *dest, const Token *src, unsigned int n);

    /**
     * Send error message to error logger about internal bug.
     * @param tok the token that this bug concerns.
     */
    void cppcheckError(const Token *tok) const;

    /**
     * Setup links for tokens so that one can call Token::link().
     *
     * @return false if there was a mismatch with tokens, this
     * should mean that source code was not valid.
     */
    bool createLinks();

    /** Syntax error */
    void syntaxError(const Token *tok);

    /** Syntax error. Example: invalid number of ')' */
    void syntaxError(const Token *tok, char c);

    /**
     * assert that tokens are ok - used during debugging for example
     * to catch problems in simplifyTokenList.
     * @return always true.
     */
    bool validate() const;

    /**
     * Helper function for simplifyDoWhileAddBraces()
     * @param tok This must be a "do" token, which is
     * not followed by "{".
     */
    bool simplifyDoWhileAddBracesHelper(Token *tok);

    /**
     * Remove __declspec()
     */
    void simplifyDeclspec();

    /**
     * Remove calling convention
     */
    void simplifyCallingConvention();

    /**
     * Remove __attribute__ ((?))
     */
    void simplifyAttribute();

    /**
     * Remove keywords "volatile", "inline", "register", and "restrict"
     */
    void simplifyKeyword();

    /**
     * Remove __asm
     */
    void simplifyAsm();

    /**
     * Simplify bitfields - the field width is removed as we don't use it.
     */
    void simplifyBitfields();

    /**
     * Remove __builtin_expect(...), likely(...), and unlikely(...)
     */
    void simplifyBuiltinExpect();

    /**
     * Remove Microsoft MFC 'DECLARE_MESSAGE_MAP()'
     */
    void simplifyMicrosoftMFC();

    /**
     * Remove Borland code
     */
    void simplifyBorland();

    /**
     * Remove Qt signals and slots
     */
    void simplifyQtSignalsSlots();

    /**
     * This will return a short name describing function parameters
     * e.g. parameters: (int a, char b) should get name "int,char,".
     * This should help to identify functions with the same name,
     * but with different parameters.
     * @param start The "(" token
     * @return, e.g. "int,char,"
     */
    static std::string getNameForFunctionParams(const Token *start);

    /**
     * check for duplicate enum definition
     */
    bool duplicateDefinition(Token **tokPtr, const Token *name);

    /**
     * duplicate enum definition error
     */
    void duplicateEnumError(const Token *tok1, const Token *tok2, const std::string & type);

    bool duplicateTypedef(Token **tokPtr, const Token *name);
    void duplicateTypedefError(const Token *tok1, const Token *tok2, const std::string & type);
    void duplicateDeclarationError(const Token *tok1, const Token *tok2, const std::string &type);

    void unsupportedTypedef(const Token *tok) const;

    /** Was there templates in the code? */
    bool codeWithTemplates() const
    {
        return _codeWithTemplates;
    }

    void setSettings(const Settings *settings)
    {
        _settings = settings;
    }

private:
    /** Disable copy constructor, no implementation */
    Tokenizer(const Tokenizer &);

    /** Disable assignment operator, no implementation */
    Tokenizer &operator=(const Tokenizer &);

    Token *_tokens, *_tokensBack;
    std::map<std::string, unsigned int> _typeSize;
    std::vector<std::string> _files;
    const Settings * _settings;
    ErrorLogger * const _errorLogger;

    /** E.g. "A" for code where "#ifdef A" is true. This is used to
        print additional information in error situations. */
    std::string _configuration;

    /**
     * was there any templates? templates that are "unused" are
     * removed from the token list
     */
    bool _codeWithTemplates;
};

/// @}

//---------------------------------------------------------------------------
#endif
