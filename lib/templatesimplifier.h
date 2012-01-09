/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include <vector>

class Token;
class ErrorLogger;
class Settings;


/// @addtogroup Core
/// @{

/** @brief Simplify templates from the preprocessed and partially simplified code. */
class TemplateSimplifier {
public:
    TemplateSimplifier();
    virtual ~TemplateSimplifier();

    /**
     * Used after simplifyTemplates to perform a little cleanup.
     * Sometimes the simplifyTemplates isn't fully successful and then
     * there are function calls etc with "wrong" syntax.
     */
    static void cleanupAfterSimplify(Token *tokens);

    /**
     * @return 0 if there are no syntax errors or return token which identifies
     * the location of syntax error.
     */
    static const Token* hasComplicatedSyntaxErrorsInTemplates(Token *tokens);

    /**
     * is the token pointing at a template parameters block
     * < int , 3 > => yes
     * \param tok start token that must point at "<"
     * \return number of parameters (invalid parameters => 0)
     */
    static unsigned int templateParameters(const Token *tok);

    /**
     * Remove "template < ..." they can cause false positives because they are not expanded
     */
    static void removeTemplates(Token *tok);

    /**
     * Expand specialized templates : "template<>.."
     * @return names of expanded templates
     */
    static std::set<std::string> simplifyTemplatesExpandSpecialized(Token *tokens);

    /**
     * Get template declarations
     * @return list of template declarations
     */
    static std::list<Token *> simplifyTemplatesGetTemplateDeclarations(Token *tokens, bool &codeWithTemplates);

    /**
     * Get template instantiations
     * @return list of template instantiations
     */
    static std::list<Token *> simplifyTemplatesGetTemplateInstantiations(Token *tokens);

    /**
     * simplify template instantiations (use default argument values)
     * @param templates list of template declarations
     * @param templateInstantiations list of template instantiations
     */
    static void simplifyTemplatesUseDefaultArgumentValues(const std::list<Token *> &templates,
            const std::list<Token *> &templateInstantiations);

    /**
     * Match template declaration/instantiation
     * @param instance template instantiation
     * @param name name of template
     * @param numberOfArguments number of template arguments
     * @param patternAfter pattern that must match the tokens after the ">"
     * @return match => true
     */
    static bool simplifyTemplatesInstantiateMatch(const Token *instance, const std::string &name, size_t numberOfArguments, const char patternAfter[]);

    /**
     * Match template declaration/instantiation
     * @param tok The ">" token e.g. before "class"
     * @return -1 to bail out or positive integer to identity the position
     * of the template name.
     */
    static int simplifyTemplatesGetTemplateNamePosition(const Token *tok);

    static void addtoken2(Token ** token, const char str[], const unsigned int lineno, const unsigned int fileno);
    static void addtoken2(Token ** token, const Token * tok, const unsigned int lineno, const unsigned int fileno);
    static void simplifyTemplatesExpandTemplate(
        Token *_tokens,
        Token **_tokensBack,
        const Token *tok,
        const std::string &name,
        std::vector<const Token *> &typeParametersInDeclaration,
        const std::string &newName,
        std::vector<const Token *> &typesUsedInTemplateInstantion,
        std::list<Token *> &templateInstantiations);

    /**
     * Simplify templates : expand all instantiatiations for a template
     * @todo It seems that inner templates should be instantiated recursively
     * @param tok token where the template declaration begins
     * @param templateInstantiations a list of template usages (not necessarily just for this template)
     * @param expandedtemplates all templates that has been expanded so far. The full names are stored.
     */
    static void simplifyTemplateInstantions(
        Token *_tokens,
        Token **_tokensBack,
        ErrorLogger *_errorLogger,
        const Settings *_settings,
        const std::vector<std::string> &files,
        const Token *tok,
        std::list<Token *> &templateInstantiations,
        std::set<std::string> &expandedtemplates);

    /**
     * Simplify templates
     */
    static void simplifyTemplates(
        Token *_tokens,
        Token **_tokensBack,
        ErrorLogger *_errorLogger,
        const Settings *_settings,
        const std::vector<std::string> &_files,
        bool &_codeWithTemplates);

    /**
     * Simplify constant calculations such as "1+2" => "3"
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    static bool simplifyCalculations(Token *_tokens);
};

/// @}

//---------------------------------------------------------------------------
#endif
