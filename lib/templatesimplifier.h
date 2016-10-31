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

#include <set>
#include <list>
#include <string>
#include <ctime>
#include <vector>
#include "config.h"

class Token;
class TokenList;
class ErrorLogger;
class Settings;


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
     * Get template declarations
     * @return list of template declarations
     */
    static std::list<Token *> getTemplateDeclarations(Token *tokens, bool &codeWithTemplates);

    /**
     * Get template instantiations
     * @return list of template instantiations
     */
    static std::list<Token *> getTemplateInstantiations(Token *tokens);

    /**
     * simplify template instantiations (use default argument values)
     * @param templates list of template declarations
     * @param templateInstantiations list of template instantiations
     */
    static void useDefaultArgumentValues(const std::list<Token *> &templates,
                                         std::list<Token *> *templateInstantiations);

    /**
     * Match template declaration/instantiation
     * @param instance template instantiation
     * @param name name of template
     * @param numberOfArguments number of template arguments
     * @param patternAfter pattern that must match the tokens after the ">"
     * @return match => true
     */
    static bool instantiateMatch(const Token *instance, const std::string &name, const std::size_t numberOfArguments, const char patternAfter[]);

    /**
     * Match template declaration/instantiation
     * @param tok The ">" token e.g. before "class"
     * @return -1 to bail out or positive integer to identity the position
     * of the template name.
     */
    static int getTemplateNamePosition(const Token *tok);

    static void expandTemplate(
        TokenList& tokenlist,
        const Token *tok,
        const std::string &name,
        const std::vector<const Token *> &typeParametersInDeclaration,
        const std::string &newName,
        const std::vector<const Token *> &typesUsedInTemplateInstantiation,
        std::list<Token *> &templateInstantiations);

    /**
     * @brief TemplateParametersInDeclaration
     * @param tok  template < typename T, typename S >
     *                        ^ tok
     * @param typeParametersInDeclaration  template < typename T, typename S >
     *                                                         ^ [0]       ^ [1]
     * @return  template < typename T, typename S >
     *                                              ^ return
     */
    static const Token * TemplateParametersInDeclaration(
        const Token * tok,
        std::vector<const Token *> & typeParametersInDeclaration);

    /**
     * Simplify templates : expand all instantiations for a template
     * @todo It seems that inner templates should be instantiated recursively
     * @param tokenlist token list
     * @param errorlogger error logger
     * @param _settings settings
     * @param tok token where the template declaration begins
     * @param maxtime time when the simplification will stop
     * @param templateInstantiations a list of template usages (not necessarily just for this template)
     * @param expandedtemplates all templates that has been expanded so far. The full names are stored.
     * @return true if the template was instantiated
     */
    static bool simplifyTemplateInstantiations(
        TokenList& tokenlist,
        ErrorLogger* errorlogger,
        const Settings *_settings,
        const Token *tok,
        const std::time_t maxtime,
        std::list<Token *> &templateInstantiations,
        std::set<std::string> &expandedtemplates);

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
