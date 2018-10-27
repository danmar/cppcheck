/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
    TemplateSimplifier(TokenList &tokenlist, const Settings *settings, ErrorLogger *errorLogger);
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
        TokenAndName(Token *tok, const std::string &s, const std::string &n);
        bool operator == (const TokenAndName & rhs) const {
            return token == rhs.token && scope == rhs.scope && name == rhs.name;
        }
        Token *token;
        std::string scope;
        std::string name;
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
     * @param forward declaration or forward declaration
     * @return -1 to bail out or positive integer to identity the position
     * of the template name.
     */
    static int getTemplateNamePosition(const Token *tok, bool forward = false);

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
    bool simplifyCalculations();

private:
    /**
     * Get template declarations
     * @param codeWithTemplates set to true if code has templates
     * @param forward declaration or forward declaration
     * @return list of template declarations
     */
    std::list<TokenAndName> getTemplateDeclarations(bool &codeWithTemplates, bool forward = false);

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
     * Expand a template. Create "expanded" class/function at end of tokenlist.
     * @param templateDeclaration               Template declaration information
     * @param templateDeclarationToken          Template declaration token
     * @param templateInstantiation             Full name of template
     * @param typeParametersInDeclaration       The type parameters of the template
     * @param newName                           New name of class/function.
     * @param copy                              copy or expand in place
     */
    void expandTemplate(
        const TokenAndName &templateDeclaration,
        const Token *templateDeclarationToken,
        const TokenAndName &templateInstantiation,
        const std::vector<const Token *> &typeParametersInDeclaration,
        const std::string &newName,
        bool copy);

    /**
     * Replace all matching template usages  'Foo < int >' => 'Foo<int>'
     * @param instantiationToken Template instantiation token
     * @param templateName full template name with scope info
     * @param typeStringsUsedInTemplateInstantiation template parameters. list of token strings.
     * @param newName The new type name
     */
    void replaceTemplateUsage(Token *const instantiationToken,
                              const std::string &templateName,
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
    bool removeTemplate(Token *tok);

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
    void eraseTokens(Token *begin, const Token *end);

    /**
     * Delete specified token without invalidating pointer to following token.
     * tok will be invalidated.
     * @param tok token to delete
     */
    static void deleteToken(Token *tok);

    /**
     * Get the new token name.
     * @param tok name token
     * @param &typeStringsUsedInTemplateInstantiation type strings use in template instantiation
     * @return new token name
     */
    std::string getNewName(
        Token *tok2,
        std::list<std::string> &typeStringsUsedInTemplateInstantiation);

    TokenList &mTokenList;
    const Settings *mSettings;
    ErrorLogger *mErrorLogger;

    std::list<TokenAndName> mTemplateDeclarations;
    std::list<TokenAndName> mTemplateInstantiations;
    std::list<TokenAndName> mInstantiatedTemplates;
    std::list<TokenAndName> mMemberFunctionsToDelete;
    std::vector<Token *> mTypesUsedInTemplateInstantiation;
};

/// @}
//---------------------------------------------------------------------------
#endif // templatesimplifierH
