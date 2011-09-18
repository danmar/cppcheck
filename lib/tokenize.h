/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include <list>
#include <vector>
#include <set>

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
     * @param preprocessorCondition Set this flag to true if the code is a preprocessor condition. It disables some simplifications
     * @return false if source code contains syntax errors
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

    /**
     * Delete all tokens in given token list
     * @param tok token list to delete
     */
    static void deleteTokens(Token *tok);

    /**
     * Simplify '* & %any% =' to '%any% ='
     */
    void simplifyMulAnd(void);

    /**
     * Get parameter name of function
     * @param ftok The token for the function name in a function
     *             implementation/declaration
     * @param par   parameter number (1,2,3,..)
     * @return if the parameter was found then the parameter name is
     *         returned. Otherwise NULL is returned.
     */
    static const char *getParameterName(const Token *ftok, unsigned int par);

    /**
     * Get file:line for a given token
     * @param tok given token
     * @return location for given token
     */
    std::string fileLine(const Token *tok) const;

    /**
     * Calculates sizeof value for given type.
     * @param type Token which will contain e.g. "int", "*", or string.
     * @return sizeof for given type, or 0 if it can't be calculated.
     */
    unsigned int sizeOfType(const Token *type) const;

    /**
     * Get filenames (the sourcefile + the files it include).
     * The first filename is the filename for the sourcefile
     * @return vector with filenames
     */
    const std::vector<std::string> *getFiles() const;

    /** recreate symbol database */
    void fillFunctionList();

    /**
     * Get function token by function name
     * @todo better handling of overloaded functions
     * @todo only scan parent scopes
     * @param funcname function name
     */
    const Token *getFunctionTokenByName(const char funcname[]) const;

    /** get tokens */
    const Token *tokens() const;

    /**
     * get filename for given token
     * @param tok The given token
     * @return filename for the given token
     */
    std::string file(const Token *tok) const;

    /**
     * get error messages that the tokenizer generate
     */
    virtual void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings);

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

    /** Remove macros in global scope */
    void removeMacrosInGlobalScope();

    /** Remove redundant assignment */
    void removeRedundantAssignment();

    /** Remove redudant code after return */
    void removeRedundantCodeAfterReturn();

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
     * Convert platform dependent types to standard types.
     * 32 bits: size_t -> unsigned long
     * 64 bits: size_t -> unsigned long long
     */
    void simplifyPlatformTypes();

    /**
     * Collapse compound standard types into a single token.
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
     * Simplify multiple assignments.
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
     * Example: "if(0!=x);" => "if(x);"
     * Special case: 'x = (0 != x);' is removed.
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
     * @return true if no syntax errors
     */
    bool simplifyIfAddBraces();

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

    /**
     * Utility function for simplifyKnownVariables. Get data about an
     * assigned variable.
     */
    bool simplifyKnownVariablesGetData(unsigned int varid, Token **_tok2, Token **_tok3, std::string &value, unsigned int &valueVarId, bool &valueIsPointer, bool floatvar);

    /**
     * utility function for simplifyKnownVariables. Perform simplification
     * of a given variable
     */
    bool simplifyKnownVariablesSimplify(Token **tok2, Token *tok3, unsigned int varid, const std::string &structname, std::string &value, unsigned int valueVarId, bool valueIsPointer, const Token * const valueToken, int indentlevel);

    /** Replace a "goto" with the statements */
    void simplifyGoto();

    /** Expand nested strcat() calls. */
    void simplifyNestedStrcat();

    /** Simplify "if else" */
    void elseif();

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

    /** Remove redundant code, e.g. if( false ) { int a; } should be
     * removed, because it is never executed.
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool removeReduntantConditions();

    /**
     * Remove redundant for:
     * "for (x=0;x<1;x++) { }" => "{ x = 1; }"
     */
    void removeRedundantFor();


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
     * Remove redundant parenthesis:
     * - "((x))" => "(x)"
     * - "(function())" => "function()"
     * - "(delete x)" => "delete x"
     * - "(delete [] x)" => "delete [] x"
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyRedundantParenthesis();

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
     * Expand specialized templates : "template<>.."
     * @return names of expanded templates
     */
    std::set<std::string> simplifyTemplatesExpandSpecialized();

    /**
     * Get template declarations
     * @return list of template declarations
     */
    std::list<Token *> simplifyTemplatesGetTemplateDeclarations();

    /**
     * Get template instantiations
     * @return list of template instantiations
     */
    std::list<Token *> simplifyTemplatesGetTemplateInstantiations();

    /**
     * simplify template instantiations (use default argument values)
     * @param templates list of template declarations
     * @param instantiations list of template instantiations
     */
    void simplifyTemplatesUseDefaultArgumentValues(const std::list<Token *> &templates,
            const std::list<Token *> &instantiations);

    /**
     * Simplify templates : expand all instantiatiations for a template
     * @todo It seems that inner templates should be instantiated recursively
     * @param tok token where the template declaration begins
     * @param used a list of template usages (not necessarily just for this template)
     * @param expandedtemplates all templates that has been expanded so far. The full names are stored.
     */
    void simplifyTemplatesInstantiate(const Token *tok,
                                      std::list<Token *> &used,
                                      std::set<std::string> &expandedtemplates);

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
     * Copy tokens.
     * @param dest destination token where copied tokens will be inserted after
     * @param first first token to copy
     * @param last last token to copy
     * @return new location of last token copied
     */
    Token *copyTokens(Token *dest, const Token *first, const Token *last);

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
     * Remove unnecessary member qualification
     */
    void removeUnnecessaryQualification();

    /**
     * unnecessary member qualification error
     */
    void unnecessaryQualificationError(const Token *tok, const std::string &qualification);

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
     * Collapse operator name tokens into single token
     * operator = => operator=
     */
    void simplifyOperatorName();

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

    bool duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef, bool undefinedStruct);
    void duplicateTypedefError(const Token *tok1, const Token *tok2, const std::string & type);

    /**
     * Report error - duplicate declarations
     */
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

    const SymbolDatabase *getSymbolDatabase() const;

    Token *deleteInvalidTypedef(Token *typeDef);

    /**
     * Get variable count.
     * @return number of variables
     */
    unsigned int varIdCount() const
    {
        return _varId;
    }

    /**
     * Simplify e.g. 'return(strncat(temp,"a",1));' into
     * strncat(temp,"a",1); return temp;
     */
    void simplifyReturn();

private:
    /** Disable copy constructor, no implementation */
    Tokenizer(const Tokenizer &);

    /** Disable assignment operator, no implementation */
    Tokenizer &operator=(const Tokenizer &);

    /** Token list */
    Token *_tokens, *_tokensBack;

    /** sizeof information for known types */
    std::map<std::string, unsigned int> _typeSize;

    /** filenames for the tokenized source code (source + included) */
    std::vector<std::string> _files;

    /** settings */
    const Settings * _settings;

    /** errorlogger */
    ErrorLogger * const _errorLogger;

    /** E.g. "A" for code where "#ifdef A" is true. This is used to
        print additional information in error situations. */
    std::string _configuration;

    /**
     * was there any templates? templates that are "unused" are
     * removed from the token list
     */
    bool _codeWithTemplates;

    /** Symbol database that all checks etc can use */
    mutable SymbolDatabase *_symbolDatabase;

    /** variable count */
    unsigned int _varId;
};

/// @}

//---------------------------------------------------------------------------
#endif
