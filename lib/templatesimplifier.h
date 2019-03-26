/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#ifndef templatesimplifierH
#define templatesimplifierH
//---------------------------------------------------------------------------

#include "config.h"

#include <ctime>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;
class TokenList;

/// @addtogroup Core
/// @{

/** @brief Simplify templates from the preprocessed and partially simplified code. */
class CPPCHECKLIB TemplateSimplifier {
public:
    explicit TemplateSimplifier(Tokenizer *tokenizer);
    ~TemplateSimplifier();

    /**
     * Used after simplifyTemplates to perform a little cleanup.
     * Sometimes the simplifyTemplates isn't fully successful and then
     * there are function calls etc with "wrong" syntax.
     */
    void cleanupAfterSimplify();

    /**
     */
    void checkComplicatedSyntaxErrorsInTemplates();

    /**
     * is the token pointing at a template parameters block
     * < int , 3 > => yes
     * \param tok start token that must point at "<"
     * \return number of parameters (invalid parameters => 0)
     */
    static unsigned int templateParameters(const Token *tok);

    /**
     * Token and its full scopename
     */
    struct TokenAndName {
        /**
         * Constructor used for instantiations.
         * \param tok template instantiation name token "name<...>"
         * \param scope full qualification of template
         */
        TokenAndName(Token *tok, const std::string &s);
        /**
         * Constructor used for declarations.
         * \param tok template declaration token "template < ... >"
         * \param scope full qualification of template
         * \param nt template name token "template < ... > class name"
         * \param pe template parameter end token ">"
         */
        TokenAndName(Token *tok, const std::string &s, const Token *nt, const Token *pe);
        TokenAndName(const TokenAndName& otherTok);
        ~TokenAndName();

        bool operator == (const TokenAndName & rhs) const {
            return token == rhs.token && scope == rhs.scope && name == rhs.name && fullName == rhs.fullName &&
                   nameToken == rhs.nameToken && paramEnd == rhs.paramEnd && flags == rhs.flags;
        }
        Token *token;
        std::string scope;
        std::string name;
        std::string fullName;
        const Token *nameToken;
        const Token *paramEnd;
        unsigned int flags;

        enum {
            fIsClass                 = (1 << 0), // class template
            fIsFunction              = (1 << 1), // function template
            fIsVariable              = (1 << 2), // variable template
            fIsAlias                 = (1 << 3), // alias template
            fIsSpecialization        = (1 << 4), // user specialized template
            fIsPartialSpecialization = (1 << 5), // user partial specialized template
            fIsForwardDeclaration    = (1 << 6), // forward declaration
        };

        bool isClass() const {
            return getFlag(fIsClass);
        }
        void isClass(bool state) {
            setFlag(fIsClass, state);
        }

        bool isFunction() const {
            return getFlag(fIsFunction);
        }
        void isFunction(bool state) {
            setFlag(fIsFunction, state);
        }

        bool isVariable() const {
            return getFlag(fIsVariable);
        }
        void isVariable(bool state) {
            setFlag(fIsVariable, state);
        }

        bool isAlias() const {
            return getFlag(fIsAlias);
        }
        void isAlias(bool state) {
            setFlag(fIsAlias, state);
        }

        bool isSpecialization() const {
            return getFlag(fIsSpecialization);
        }
        void isSpecialization(bool state) {
            setFlag(fIsSpecialization, state);
        }

        bool isPartialSpecialization() const {
            return getFlag(fIsPartialSpecialization);
        }
        void isPartialSpecialization(bool state) {
            setFlag(fIsPartialSpecialization, state);
        }

        bool isForwardDeclaration() const {
            return getFlag(fIsForwardDeclaration);
        }
        void isForwardDeclaration(bool state) {
            setFlag(fIsForwardDeclaration, state);
        }

        /**
         * Get specified flag state.
         * @param flag flag to get state of
         * @return true if flag set or false in flag not set
         */
        bool getFlag(unsigned int flag) const {
            return ((flags & flag) != 0);
        }

        /**
         * Set specified flag state.
         * @param flag flag to set state
         * @param state new state of flag
         */
        void setFlag(unsigned int flag, bool state) {
            flags = state ? flags | flag : flags & ~flag;
        }
    };

    /**
     * Match template declaration/instantiation
     * @param instance template instantiation
     * @param numberOfArguments number of template arguments
     * @param patternAfter pattern that must match the tokens after the ">"
     * @return match => true
     */
    static bool instantiateMatch(const Token *instance, const std::size_t numberOfArguments, const char patternAfter[]);

    /**
     * Match template declaration/instantiation
     * @param tok The ">" token e.g. before "class"
     * @return -1 to bail out or positive integer to identity the position
     * of the template name.
     */
    static int getTemplateNamePosition(const Token *tok);

    /**
     * Get function template name position
     * @param tok The ">" token e.g. before "class"
     * @param namepos return offset to name
     * @return true if name found, false if not
     * */
    static bool getTemplateNamePositionTemplateFunction(const Token *tok, int &namepos);

    /**
     * Get variable template name position
     * @param tok The ">" token
     * @param namepos return offset to name
     * @return true if name found, false if not
     * */
    static bool getTemplateNamePositionTemplateVariable(const Token *tok, int &namepos);

    /**
     * Simplify templates
     * @param maxtime time when the simplification should be stopped
     * @param codeWithTemplates output parameter that is set if code contains templates
     */
    void simplifyTemplates(
        const std::time_t maxtime,
        bool &codeWithTemplates);

    /**
     * Simplify constant calculations such as "1+2" => "3"
     * @param tok start token
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    static bool simplifyNumericCalculations(Token *tok);

    /**
     * Simplify constant calculations such as "1+2" => "3".
     * This also performs simple cleanup of parentheses etc.
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyCalculations(Token* frontToken = nullptr, Token *backToken = nullptr);

    /** Simplify template instantiation arguments.
     * @param start first token of arguments
     * @param end token following last argument token
     */
    void simplifyTemplateArgs(Token *start, Token *end);

private:
    /**
     * Get template declarations
     * @return true if code has templates.
     */
    bool getTemplateDeclarations();

    /** Add template instantiation.
     * @param token first token of instantiation
     * @param scope scope of instantiation
     */
    void addInstantiation(Token *token, const std::string &scope);

    /**
     * Get template instantiations
     */
    void getTemplateInstantiations();

    /**
     * Fix forward declared default argument values by copying them
     * when they are not present in the declaration.
     */
    void fixForwardDeclaredDefaultArgumentValues();

    /**
     * simplify template instantiations (use default argument values)
     */
    void useDefaultArgumentValues();

    /**
     * Try to locate a matching declaration for each user defined
     * specialization.
     */
    void getSpecializations();

    /**
     * Try to locate a matching declaration for each user defined
     * partial specialization.
     */
    void getPartialSpecializations();

    /**
     * simplify template aliases
     */
    void simplifyTemplateAliases();

    /**
     * Simplify templates : expand all instantiations for a template
     * @todo It seems that inner templates should be instantiated recursively
     * @param templateDeclaration template declaration
     * @param specializations template specializations (list each template name token)
     * @param maxtime time when the simplification will stop
     * @param expandedtemplates all templates that has been expanded so far. The full names are stored.
     * @return true if the template was instantiated
     */
    bool simplifyTemplateInstantiations(
        const TokenAndName &templateDeclaration,
        const std::list<const Token *> &specializations,
        const std::time_t maxtime,
        std::set<std::string> &expandedtemplates);

    /**
     * Simplify templates : add namespace to template name
     * @param templateDeclaration template declaration
     * @param tok place to insert namespace
     */
    void addNamespace(const TokenAndName &templateDeclaration, const Token *tok);

    /**
     * Simplify templates : check if namespace already present
     * @param templateDeclaration template declaration
     * @param tok place to start looking for namespace
     * @return true if namespace already present
     */
    static bool alreadyHasNamespace(const TokenAndName &templateDeclaration, const Token *tok);

    /**
     * Expand a template. Create "expanded" class/function at end of tokenlist.
     * @param templateDeclaration               Template declaration information
     * @param templateInstantiation             Full name of template
     * @param typeParametersInDeclaration       The type parameters of the template
     * @param newName                           New name of class/function.
     * @param copy                              copy or expand in place
     */
    void expandTemplate(
        const TokenAndName &templateDeclaration,
        const TokenAndName &templateInstantiation,
        const std::vector<const Token *> &typeParametersInDeclaration,
        const std::string &newName,
        bool copy);

    /**
     * Replace all matching template usages  'Foo < int >' => 'Foo<int>'
     * @param instantiation Template instantiation information.
     * @param typeStringsUsedInTemplateInstantiation template parameters. list of token strings.
     * @param newName The new type name
     */
    void replaceTemplateUsage(const TokenAndName &instantiation,
                              const std::list<std::string> &typeStringsUsedInTemplateInstantiation,
                              const std::string &newName);

    /**
     * @brief TemplateParametersInDeclaration
     * @param tok  template < typename T, typename S >
     *                        ^ tok
     * @param typeParametersInDeclaration  template < typename T, typename S >
     *                                                         ^ [0]       ^ [1]
     * @return  template < typename T, typename S >
     *                                              ^ return
     */
    static const Token * getTemplateParametersInDeclaration(
        const Token * tok,
        std::vector<const Token *> & typeParametersInDeclaration);

    /**
     * Remove a specific "template < ..." template class/function
     */
    static bool removeTemplate(Token *tok);

    /** Syntax error */
    static void syntaxError(const Token *tok);

    static bool matchSpecialization(
        const Token *templateDeclarationNameToken,
        const Token *templateInstantiationNameToken,
        const std::list<const Token *> & specializations);

    /*
     * Same as Token::eraseTokens() but tries to fix up lists with pointers to the deleted tokens.
     * @param begin Tokens after this will be erased.
     * @param end Tokens before this will be erased.
     */
    static void eraseTokens(Token *begin, const Token *end);

    /**
     * Delete specified token without invalidating pointer to following token.
     * tok will be invalidated.
     * @param tok token to delete
     */
    static void deleteToken(Token *tok);

    /**
     * Get the new token name.
     * @param tok2 name token
     * @param &typeStringsUsedInTemplateInstantiation type strings use in template instantiation
     * @return new token name
     */
    std::string getNewName(
        Token *tok2,
        std::list<std::string> &typeStringsUsedInTemplateInstantiation);

    void printOut(
        const TokenAndName &tokenAndName,
        const std::string &indent = "    ") const;
    void printOut(const std::string &text = "") const;

    Tokenizer *mTokenizer;
    TokenList &mTokenList;
    const Settings *mSettings;
    ErrorLogger *mErrorLogger;

    std::list<TokenAndName> mTemplateDeclarations;
    std::list<TokenAndName> mTemplateForwardDeclarations;
    std::map<Token *, Token *> mTemplateForwardDeclarationsMap;
    std::map<Token *, Token *> mTemplateSpecializationMap;
    std::map<Token *, Token *> mTemplatePartialSpecializationMap;
    std::list<TokenAndName> mTemplateInstantiations;
    std::list<TokenAndName> mInstantiatedTemplates;
    std::list<TokenAndName> mMemberFunctionsToDelete;
    std::vector<TokenAndName> mExplicitInstantiationsToDelete;
    std::vector<TokenAndName> mTypesUsedInTemplateInstantiation;
};

/// @}
//---------------------------------------------------------------------------
#endif // templatesimplifierH
