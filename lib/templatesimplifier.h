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
class TokenList;


/// @addtogroup Core
/// @{

/** @brief Simplify templates from the preprocessed and partially simplified code. */
class CPPCHECKLIB TemplateSimplifier {
    TemplateSimplifier();
    ~TemplateSimplifier();
public:

    /**
     * Used after simplifyTemplates to perform a little cleanup.
     * Sometimes the simplifyTemplates isn't fully successful and then
     * there are function calls etc with "wrong" syntax.
     */
    static void cleanupAfterSimplify(Token *tokens);

    /**
     * \param[in] tokens token list
     * @return false if there are no syntax errors or true
     */
    static void checkComplicatedSyntaxErrorsInTemplates(const Token *tokens);

    /**
     * is the token pointing at a template parameters block
     * < int , 3 > => yes
     * \param tok start token that must point at "<"
     * \return number of parameters (invalid parameters => 0)
     */
    static unsigned int templateParameters(const Token *tok);

    /**
     * Expand specialized templates : "template<>.."
     * @return names of expanded templates
     */
    static std::set<std::string> expandSpecialized(Token *tokens);

    /**
     * Token and its full scopename
     */
    struct TokenAndName {
        TokenAndName(Token *tok, const std::string &s, const std::string &n) : token(tok), scope(s), name(n) {}
        Token *token;
        std::string scope;
        std::string name;
    };

    /**
     * Get template declarations
     * @return list of template declarations
     */
    static std::list<TokenAndName> getTemplateDeclarations(Token *tokens, bool &codeWithTemplates);

    /**
     * Get template instantiations
     * @param tokens start of token list
     * @param declarations template declarations, so names can be matched
     * @return list of template instantiations
     */
    static std::list<TokenAndName> getTemplateInstantiations(Token *tokens, const std::list<TokenAndName> &declarations);

    /**
     * simplify template instantiations (use default argument values)
     * @param templates list of template declarations
     * @param templateInstantiations list of template instantiations
     */
    static void useDefaultArgumentValues(const std::list<TokenAndName> &templates,
                                         std::list<TokenAndName> *templateInstantiations);

    /**
     * simplify template aliases
     * @param templateInstantiations pointer to list of template instantiations
     */
    static void simplifyTemplateAliases(std::list<TokenAndName> *templateInstantiations);

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
     * Expand a template. Create "expanded" class/function at end of tokenlist.
     * @param tokenlist                         The tokenlist that is changed
     * @param templateDeclarationToken          The template declaration token for the template that will be "expanded"
     * @param fullName                          Full name of template
     * @param typeParametersInDeclaration       The type parameters of the template
     * @param newName                           New name of class/function.
     * @param typesUsedInTemplateInstantiation  Type parameters in instantiation
     * @param templateInstantiations            List of template instantiations.
     */
    static void expandTemplate(
        TokenList& tokenlist,
        const Token *templateDeclarationToken,
        const std::string &fullName,
        const std::vector<const Token *> &typeParametersInDeclaration,
        const std::string &newName,
        const std::vector<const Token *> &typesUsedInTemplateInstantiation,
        std::list<TokenAndName> &templateInstantiations);

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
     * Simplify templates : expand all instantiations for a template
     * @todo It seems that inner templates should be instantiated recursively
     * @param tokenlist token list
     * @param errorlogger error logger
     * @param _settings settings
     * @param templateDeclaration template declaration
     * @param specializations template specializations (list each template name token)
     * @param maxtime time when the simplification will stop
     * @param templateInstantiations a list of template usages (not necessarily just for this template)
     * @param expandedtemplates all templates that has been expanded so far. The full names are stored.
     * @return true if the template was instantiated
     */
    static bool simplifyTemplateInstantiations(
        TokenList& tokenlist,
        ErrorLogger* errorlogger,
        const Settings *_settings,
        const TokenAndName &templateDeclaration,
        const std::list<const Token *> &specializations,
        const std::time_t maxtime,
        std::list<TokenAndName> &templateInstantiations,
        std::set<std::string> &expandedtemplates);

    /**
     * Replace all matching template usages  'Foo < int >' => 'Foo<int>'
     * @param instantiationToken Template instantiation token
     * @param templateName full template name with scope info
     * @param typeStringsUsedInTemplateInstantiation template parameters. list of token strings.
     * @param newName The new type name
     * @param typesUsedInTemplateInstantiation template instantiation parameters
     * @param templateInstantiations All seen instantiations
     */
    static void replaceTemplateUsage(Token *const instantiationToken,
                                     const std::string &templateName,
                                     const std::list<std::string> &typeStringsUsedInTemplateInstantiation,
                                     const std::string &newName,
                                     const std::vector<const Token *> &typesUsedInTemplateInstantiation,
                                     std::list<TokenAndName> &templateInstantiations);

    /**
     * Simplify templates
     * @param tokenlist token list
     * @param errorlogger error logger
     * @param _settings settings
     * @param maxtime time when the simplification should be stopped
     * @param _codeWithTemplates output parameter that is set if code contains templates
     */
    static void simplifyTemplates(
        TokenList& tokenlist,
        ErrorLogger* errorlogger,
        const Settings *_settings,
        const std::time_t maxtime,
        bool &_codeWithTemplates);

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
     * @param _tokens start token
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    static bool simplifyCalculations(Token *_tokens);

private:
    /**
     * Remove a specific "template < ..." template class/function
     */
    static bool removeTemplate(Token *tok);

    /** Syntax error */
    static void syntaxError(const Token *tok);

};

/// @}
//---------------------------------------------------------------------------
#endif // templatesimplifierH
