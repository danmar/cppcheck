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

#include "templatesimplifier.h"
#include "mathlib.h"
#include "token.h"
#include "tokenize.h"
#include "errorlogger.h"
#include "settings.h"
#include <algorithm>
#include <sstream>
#include <list>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>

#ifdef GDB_HELPERS

static void printlist(const std::list<Token *> &list)
{
    for (std::list<Token *>::const_iterator it = list.begin(); it != list.end(); ++it) {
        const Token *token = *it;
        std::cout << "   ";
        while (token && !Token::Match(token, "[{};]")) {
            std::cout << " " << token->str();
            token = token->next();
        }
        std::cout << std::endl;
    }
}

static void printvector(const std::vector<const Token *> &v)
{
    for (std::size_t i = 0; i < v.size(); i++) {
        const Token *token = v[i];
        std::cout << "    " << i << ":";
        while (token && !Token::Match(token, "[{};]")) {
            std::cout << " " << token->str();
            token = token->next();
        }
        std::cout << std::endl;
    }
}

#endif

//---------------------------------------------------------------------------

void TemplateSimplifier::cleanupAfterSimplify(Token *tokens)
{
    bool goback = false;
    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        if (tok->str() == "(")
            tok = tok->link();

        else if (Token::Match(tok, "template < > %name%")) {
            const Token *end = tok;
            while (end) {
                if (end->str() == ";")
                    break;
                if (end->str() == "{") {
                    end = end->link()->next();
                    break;
                }
                if (!Token::Match(end, "%name%|::|<|>|,")) {
                    end = nullptr;
                    break;
                }
                end = end->next();
            }
            if (end) {
                Token::eraseTokens(tok,end);
                tok->deleteThis();
            }
        }

        else if (Token::Match(tok, "%type% <") &&
                 (!tok->previous() || tok->previous()->str() == ";")) {
            const Token *tok2 = tok->tokAt(2);
            std::string type;
            while (Token::Match(tok2, "%type%|%num% ,")) {
                type += tok2->str() + ",";
                tok2 = tok2->tokAt(2);
            }
            if (Token::Match(tok2, "%type%|%num% > (")) {
                type += tok2->str();
                tok->str(tok->str() + "<" + type + ">");
                Token::eraseTokens(tok, tok2->tokAt(2));
                if (tok == tokens)
                    goback = true;
            }
        }
    }
}


void TemplateSimplifier::checkComplicatedSyntaxErrorsInTemplates(const Token *tokens)
{
    // check for more complicated syntax errors when using templates..
    for (const Token *tok = tokens; tok; tok = tok->next()) {
        // skip executing scopes (ticket #3183)..
        if (Token::simpleMatch(tok, "( {")) {
            tok = tok->link();
            if (!tok)
                syntaxError(nullptr);
        }
        // skip executing scopes..
        const Token *start = Tokenizer::startOfExecutableScope(tok);
        if (start) {
            tok = start->link();
        }

        // skip executing scopes (ticket #1985)..
        else if (Token::simpleMatch(tok, "try {")) {
            tok = tok->next()->link();
            while (Token::simpleMatch(tok, "} catch (")) {
                tok = tok->linkAt(2);
                if (Token::simpleMatch(tok, ") {"))
                    tok = tok->next()->link();
            }
        }

        if (!tok)
            syntaxError(nullptr);
        // not start of statement?
        if (tok->previous() && !Token::Match(tok, "[;{}]"))
            continue;

        // skip starting tokens.. ;;; typedef typename foo::bar::..
        while (Token::simpleMatch(tok, ";"))
            tok = tok->next();
        while (Token::Match(tok, "typedef|typename"))
            tok = tok->next();
        while (Token::Match(tok, "%type% ::"))
            tok = tok->tokAt(2);
        if (!tok)
            break;

        // template variable or type..
        if (Token::Match(tok, "%type% <")) {
            // these are used types..
            std::set<std::string> usedtypes;

            // parse this statement and see if the '<' and '>' are matching
            unsigned int level = 0;
            for (const Token *tok2 = tok; tok2 && !Token::Match(tok2, "[;{}]"); tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == "<") {
                    bool inclevel = false;
                    if (Token::simpleMatch(tok2->previous(), "operator <"))
                        ;
                    else if (level == 0)
                        inclevel = true;
                    else if (tok2->next() && tok2->next()->isStandardType())
                        inclevel = true;
                    else if (Token::simpleMatch(tok2, "< typename"))
                        inclevel = true;
                    else if (Token::Match(tok2->tokAt(-2), "<|, %type% <") && usedtypes.find(tok2->previous()->str()) != usedtypes.end())
                        inclevel = true;
                    else if (Token::Match(tok2, "< %type%") && usedtypes.find(tok2->next()->str()) != usedtypes.end())
                        inclevel = true;
                    else if (Token::Match(tok2, "< %type%")) {
                        // is the next token a type and not a variable/constant?
                        // assume it's a type if there comes another "<"
                        const Token *tok3 = tok2->next();
                        while (Token::Match(tok3, "%type% ::"))
                            tok3 = tok3->tokAt(2);
                        if (Token::Match(tok3, "%type% <"))
                            inclevel = true;
                    }

                    if (inclevel) {
                        ++level;
                        if (Token::Match(tok2->tokAt(-2), "<|, %type% <"))
                            usedtypes.insert(tok2->previous()->str());
                    }
                } else if (tok2->str() == ">") {
                    if (level > 0)
                        --level;
                } else if (tok2->str() == ">>") {
                    if (level > 0)
                        --level;
                    if (level > 0)
                        --level;
                }
            }
            if (level > 0)
                syntaxError(tok);
        }
    }
}

unsigned int TemplateSimplifier::templateParameters(const Token *tok)
{
    unsigned int numberOfParameters = 1;

    if (!tok)
        return 0;
    if (tok->str() != "<")
        return 0;
    tok = tok->next();

    unsigned int level = 0;

    while (tok) {
        // skip const/volatile
        if (Token::Match(tok, "const|volatile"))
            tok = tok->next();

        // skip struct/union
        if (Token::Match(tok, "struct|union"))
            tok = tok->next();

        // Skip '&'
        if (Token::Match(tok, "& ::| %name%"))
            tok = tok->next();

        // Skip variadic types (Ticket #5774, #6059, #6172)
        if (Token::Match(tok, "%type% . . .")) {
            tok = tok->tokAt(4);
            continue;
        }

        // Skip '=', '?', ':'
        if (Token::Match(tok, "=|?|:"))
            tok = tok->next();
        if (!tok)
            return 0;

        // Skip casts
        if (tok->str() == "(") {
            tok = tok->link();
            if (tok)
                tok = tok->next();
        }

        // skip std::
        if (tok && tok->str() == "::")
            tok = tok->next();
        while (Token::Match(tok, "%name% ::")) {
            tok = tok->tokAt(2);
            if (tok && tok->str() == "*") // Ticket #5759: Class member pointer as a template argument; skip '*'
                tok = tok->next();
        }
        if (!tok)
            return 0;

        // num/type ..
        if (!tok->isNumber() && tok->tokType() != Token::eChar && !tok->isName() && !tok->isOp())
            return 0;
        tok = tok->next();
        if (!tok)
            return 0;

        // * / const
        while (Token::Match(tok, "*|&|&&|const"))
            tok = tok->next();

        if (!tok)
            return 0;

        // Function pointer or prototype..
        while (Token::Match(tok, "(|[")) {
            if (!tok->link())
                syntaxError(tok);

            tok = tok->link()->next();
            while (Token::Match(tok, "const|volatile")) // Ticket #5786: Skip function cv-qualifiers
                tok = tok->next();
        }
        if (!tok)
            return 0;

        // inner template
        if (tok->str() == "<") {
            ++level;
            tok = tok->next();
        }

        if (!tok)
            return 0;

        // ,/>
        while (Token::Match(tok, ">|>>")) {
            if (level == 0)
                return ((tok->str() == ">") ? numberOfParameters : 0);
            --level;
            if (tok->str() == ">>") {
                if (level == 0)
                    return numberOfParameters;
                --level;
            }
            tok = tok->next();

            // * / &
            while (Token::Match(tok, "[*&]"))
                tok = tok->next();

            if (!tok)
                return 0;
        }

        if (tok->str() != ",")
            continue;
        if (level == 0)
            ++numberOfParameters;
        tok = tok->next();
    }
    return 0;
}

bool TemplateSimplifier::removeTemplate(Token *tok)
{
    if (!Token::simpleMatch(tok, "template <"))
        return false;

    int indentlevel = 0;
    unsigned int countgt = 0;   // Counter for ">"
    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {

        if (tok2->str() == "(") {
            tok2 = tok2->link();
        } else if (tok2->str() == ")") {  // garbage code! (#3504)
            Token::eraseTokens(tok,tok2);
            tok->deleteThis();
            return false;
        }

        else if (tok2->str() == "{") {
            tok2 = tok2->link()->next();
            Token::eraseTokens(tok, tok2);
            if (tok2 && tok2->str() == ";" && tok2->next())
                tok->deleteNext();
            tok->deleteThis();
            return true;
        } else if (tok2->str() == "}") {  // garbage code! (#3449)
            Token::eraseTokens(tok,tok2);
            tok->deleteThis();
            return false;
        }

        // Count ">"
        if (tok2->str() == ">")
            countgt++;

        // don't remove constructor
        if (tok2->str() == "explicit" ||
            (countgt == 1 && Token::Match(tok2->previous(), "> %type% (") &&
             Tokenizer::startOfExecutableScope(tok2->linkAt(1)))) {
            Token::eraseTokens(tok, tok2);
            tok->deleteThis();
            return true;
        }

        if (tok2->str() == ";") {
            tok2 = tok2->next();
            Token::eraseTokens(tok, tok2);
            tok->deleteThis();
            return true;
        }

        if (tok2->str() == "<")
            ++indentlevel;

        else if (indentlevel >= 2 && tok2->str() == ">")
            --indentlevel;

        else if (Token::Match(tok2, "> class|struct %name% [,)]")) {
            tok2 = tok2->next();
            Token::eraseTokens(tok, tok2);
            tok->deleteThis();
            return true;
        }
    }

    return false;
}

std::set<std::string> TemplateSimplifier::expandSpecialized(Token *tokens)
{
    std::set<std::string> expandedtemplates;

    // Locate specialized templates..
    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "template < >"))
            continue;

        // what kind of template is this?
        Token *tok2 = tok->tokAt(3);
        while (tok2 && (tok2->isName() || tok2->str() == "*"))
            tok2 = tok2->next();

        if (!TemplateSimplifier::templateParameters(tok2))
            continue;

        // unknown template.. bail out
        if (!tok2->previous()->isName())
            continue;

        tok2 = tok2->previous();
        std::string s;
        {
            std::ostringstream ostr;
            const Token *tok3 = tok2;
            for (; tok3 && tok3->str() != ">"; tok3 = tok3->next()) {
                if (tok3 != tok2)
                    ostr << " ";
                ostr << tok3->str();
            }
            if (!Token::Match(tok3, "> (|{|:"))
                continue;
            s = ostr.str();
        }

        // remove spaces to create new name
        const std::string name(s + " >");
        expandedtemplates.insert(name);

        // Rename template..
        Token::eraseTokens(tok2, Token::findsimplematch(tok2, "<")->findClosingBracket()->next());
        tok2->str(name);

        // delete the "template < >"
        tok->deleteNext(2);
        tok->deleteThis();

        // Use this special template in the code..
        while (nullptr != (tok2 = const_cast<Token *>(Token::findsimplematch(tok2, name.c_str())))) {
            Token::eraseTokens(tok2, Token::findsimplematch(tok2, "<")->findClosingBracket()->next());
            tok2->str(name);
        }
    }

    return expandedtemplates;
}

std::list<Token *> TemplateSimplifier::getTemplateDeclarations(Token *tokens, bool &codeWithTemplates)
{
    std::list<Token *> templates;
    for (Token *tok = tokens; tok; tok = tok->next()) {
        // TODO: handle namespaces. Right now we don't instantiate templates that are defined in namespaces.
        if (Token::Match(tok, "namespace %type% {"))
            tok = tok->linkAt(2);

        if (Token::simpleMatch(tok, "template <")) {
            // Some syntax checks, see #6865
            if (!tok->tokAt(2))
                syntaxError(tok->next());
            if (tok->strAt(2)=="typename" &&
                (!tok->tokAt(3) || !Token::Match(tok->tokAt(3), "%name%|.|,|>")))
                syntaxError(tok->next());
            codeWithTemplates = true;
            Token *parmEnd = tok->next()->findClosingBracket();
            int indentlevel = 0;
            for (const Token *tok2 = parmEnd; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    ++indentlevel;
                else if (tok2->str() == ")")
                    --indentlevel;

                if (indentlevel) // In an argument list; move to the next token
                    continue;

                // Just a declaration => ignore this
                if (tok2->str() == ";")
                    break;
                // Implementation => add to "templates"
                if (tok2->str() == "{") {
                    templates.push_back(tok);
                    break;
                }
            }
        }
    }
    return templates;
}


std::list<Token *> TemplateSimplifier::getTemplateInstantiations(Token *tokens)
{
    std::list<Token *> used;

    for (Token *tok = tokens; tok; tok = tok->next()) {
        // template definition.. skip it
        if (Token::simpleMatch(tok, "template <")) {
            tok = tok->next()->findClosingBracket();
            if (!tok)
                break;
            // #7914
            // Ignore template instantiations within template definitions: they will only be
            // handled if the definition is actually instantiated
            const Token *tok2 = Token::findmatch(tok, "{|;");
            if (tok2 && tok2->str() == "{")
                tok = tok2->link();
        } else if (Token::Match(tok->previous(), "[({};=] %name% <") ||
                   Token::Match(tok->previous(), "%type% %name% <") ||
                   Token::Match(tok->tokAt(-2), "[,:] private|protected|public %name% <")) {

            // Add inner template instantiations first => go to the ">"
            // and then parse backwards, adding all seen instantiations
            const Token *tok2 = tok->next()->findClosingBracket();

            // parse backwards and add template instantiations
            for (; tok2 && tok2 != tok; tok2 = tok2->previous()) {
                if (Token::Match(tok2, ", %name% <") &&
                    TemplateSimplifier::templateParameters(tok2->tokAt(2))) {
                    used.push_back(tok2->next());
                }
            }

            // Add outer template..
            if (TemplateSimplifier::templateParameters(tok->next()))
                used.push_back(tok);
        }
    }

    return used;
}


void TemplateSimplifier::useDefaultArgumentValues(const std::list<Token *> &templates,
        std::list<Token *> * const templateInstantiations)
{
    for (std::list<Token *>::const_iterator iter1 = templates.begin(); iter1 != templates.end(); ++iter1) {
        // template parameters with default value has syntax such as:
        //     x = y
        // this list will contain all the '=' tokens for such arguments
        std::list<Token *> eq;
        // and this set the position of parameters with a default value
        std::set<std::size_t> defaultedArgPos;

        // parameter number. 1,2,3,..
        std::size_t templatepar = 1;

        // parameter depth
        std::size_t templateParmDepth = 0;

        // the template classname. This will be empty for template functions
        std::string classname;

        // Scan template declaration..
        for (Token *tok = *iter1; tok; tok = tok->next()) {
            if (Token::simpleMatch(tok, "template < >")) { // Ticket #5762: Skip specialization tokens
                tok = tok->tokAt(2);
                if (0 == templateParmDepth)
                    break;
                continue;
            }

            if (tok->str() == "(") { // Ticket #6835
                tok = tok->link();
                continue;
            }

            if (tok->str() == "<" && templateParameters(tok))
                ++templateParmDepth;

            // end of template parameters?
            if (tok->str() == ">") {
                if (Token::Match(tok, "> class|struct %name%"))
                    classname = tok->strAt(2);
                if (templateParmDepth<2)
                    break;
                else
                    --templateParmDepth;
            }

            // next template parameter
            if (tok->str() == "," && (1 == templateParmDepth)) // Ticket #5823: Properly count parameters
                ++templatepar;

            // default parameter value?
            else if (Token::Match(tok, "= !!>")) {
                if (defaultedArgPos.insert(templatepar).second) {
                    eq.push_back(tok);
                } else {
                    // Ticket #5605: Syntax error (two equal signs for the same parameter), bail out
                    eq.clear();
                    break;
                }
            }
        }
        if (eq.empty() || classname.empty())
            continue;

        // iterate through all template instantiations
        for (std::list<Token *>::const_iterator iter2 = templateInstantiations->begin(); iter2 != templateInstantiations->end(); ++iter2) {
            Token *tok = *iter2;

            if (!Token::simpleMatch(tok, (classname + " <").c_str()))
                continue;

            // count the parameters..
            tok = tok->next();
            const unsigned int usedpar = TemplateSimplifier::templateParameters(tok);
            tok = tok->findClosingBracket();

            if (tok && tok->str() == ">") {
                tok = tok->previous();
                std::list<Token *>::const_iterator it = eq.begin();
                for (std::size_t i = (templatepar - eq.size()); it != eq.end() && i < usedpar; ++i)
                    ++it;
                while (it != eq.end()) {
                    int indentlevel = 0;
                    tok->insertToken(",");
                    tok = tok->next();
                    const Token *from = (*it)->next();
                    std::stack<Token *> links;
                    while (from && (!links.empty() || indentlevel || !Token::Match(from, ",|>"))) {
                        if (from->str() == "<")
                            ++indentlevel;
                        else if (from->str() == ">")
                            --indentlevel;
                        tok->insertToken(from->str(), from->originalName());
                        tok = tok->next();
                        if (Token::Match(tok, "(|["))
                            links.push(tok);
                        else if (!links.empty() && Token::Match(tok, ")|]")) {
                            Token::createMutualLinks(links.top(), tok);
                            links.pop();
                        }
                        from = from->next();
                    }
                    ++it;
                }
            }
        }

        for (std::list<Token *>::iterator it = eq.begin(); it != eq.end(); ++it) {
            Token * const eqtok = *it;
            Token *tok2;
            int indentlevel = 0;
            for (tok2 = eqtok->next(); tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, ";|)|}|]")) { // bail out #6607
                    tok2 = nullptr;
                    break;
                }
                if (Token::Match(tok2, "(|{|["))
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "%type% <") && templateParameters(tok2->next())) {
                    std::list<Token*>::iterator ti = std::find(templateInstantiations->begin(),
                                                     templateInstantiations->end(),
                                                     tok2);
                    if (ti != templateInstantiations->end())
                        templateInstantiations->erase(ti);
                    ++indentlevel;
                } else if (indentlevel > 0 && tok2->str() == ">")
                    --indentlevel;
                else if (indentlevel == 0 && Token::Match(tok2, ",|>"))
                    break;
                if (indentlevel < 0)
                    break;
            }
            // something went wrong, don't call eraseTokens()
            // with a nullptr "end" parameter (=all remaining tokens).
            if (!tok2)
                continue;

            Token::eraseTokens(eqtok, tok2);
            eqtok->deleteThis();
        }
    }
}

bool TemplateSimplifier::instantiateMatch(const Token *instance, const std::string &name, const std::size_t numberOfArguments, const char patternAfter[])
{
    if (!Token::simpleMatch(instance, (name + " <").c_str()))
        return false;

    if (numberOfArguments != TemplateSimplifier::templateParameters(instance->next()))
        return false;

    if (patternAfter) {
        const Token *tok = instance;
        unsigned int indentlevel = 0;
        for (tok = instance; tok && (tok->str() != ">" || indentlevel > 0); tok = tok->next()) {
            if (Token::Match(tok, "[<,] %name% <") && templateParameters(tok->tokAt(2)) > 0)
                ++indentlevel;
            if (indentlevel > 0 && tok->str() == ">")
                --indentlevel;
        }
        if (!tok || !Token::Match(tok->next(), patternAfter))
            return false;
    }

    // nothing mismatching was found..
    return true;
}

int TemplateSimplifier::getTemplateNamePosition(const Token *tok)
{
    // get the position of the template name
    int namepos = 0, starAmpPossiblePosition = 0;
    if (Token::Match(tok, "> class|struct %type% {|:|<"))
        namepos = 2;
    else if (Token::Match(tok, "> %type% *|&| %type% ("))
        namepos = 2;
    else if (Token::Match(tok, "> %type% %type% *|&| %type% ("))
        namepos = 3;
    else if ((Token::Match(tok, "> %type% <") && Token::Match(tok->linkAt(2), "> :: %type% (")) ||
             (Token::Match(tok, "> %type% %type% <") && Token::Match(tok->linkAt(3), "> :: %type% ("))) {
        namepos = 2 + (Token::Match(tok, "> %type% %type%"));
        const Token *end = tok->linkAt(namepos);
        for (const Token *tok2 = tok->tokAt(namepos) ; tok2 != end ; tok2 = tok2->next())
            ++namepos;
        namepos += 2;
    } else if (Token::Match(tok, "> %type% *|&| %type% :: %type% (")) {
        namepos = 4;
        starAmpPossiblePosition = 2;
    } else if (Token::Match(tok, "> %type% %type% *|&| %type% :: %type% (")) {
        namepos = 5;
        starAmpPossiblePosition = 3;
    } else {
        // Name not found
        return -1;
    }
    if (Token::Match(tok->tokAt(starAmpPossiblePosition ? starAmpPossiblePosition : namepos), "*|&"))
        ++namepos;

    return namepos;
}


void TemplateSimplifier::expandTemplate(
    TokenList& tokenlist,
    const Token *tok,
    const std::string &name,
    const std::vector<const Token *> &typeParametersInDeclaration,
    const std::string &newName,
    const std::vector<const Token *> &typesUsedInTemplateInstantiation,
    std::list<Token *> &templateInstantiations)
{
    bool inTemplateDefinition=false;
    std::vector<const Token *> localTypeParametersInDeclaration;
    for (const Token *tok3 = tokenlist.front(); tok3; tok3 = tok3 ? tok3->next() : nullptr) {
        if (tok3->str()=="template") {
            if (tok3->next() && tok3->next()->str()=="<") {
                TemplateParametersInDeclaration(tok3->tokAt(2), localTypeParametersInDeclaration);
                if (localTypeParametersInDeclaration.size() != typeParametersInDeclaration.size())
                    inTemplateDefinition = false; // Partial specialization
                else
                    inTemplateDefinition = true;
            } else
                inTemplateDefinition = false; // Only template instantiation
        }
        if (Token::Match(tok3, "{|(|["))
            tok3 = tok3->link();

        // Start of template..
        if (tok3 == tok) {
            tok3 = tok3->next();
        }

        // member function implemented outside class definition
        else if (inTemplateDefinition &&
                 TemplateSimplifier::instantiateMatch(tok3, name, typeParametersInDeclaration.size(), ":: ~| %name% (")) {
            tokenlist.addtoken(newName, tok3->linenr(), tok3->fileIndex());
            while (tok3 && tok3->str() != "::")
                tok3 = tok3->next();
        }

        // not part of template.. go on to next token
        else
            continue;

        int indentlevel = 0;
        std::stack<Token *> brackets; // holds "(", "[" and "{" tokens

        for (; tok3; tok3 = tok3->next()) {
            if (tok3->isName()) {
                // search for this token in the type vector
                unsigned int itype = 0;
                while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != tok3->str())
                    ++itype;

                // replace type with given type..
                if (itype < typeParametersInDeclaration.size()) {
                    unsigned int typeindentlevel = 0;
                    for (const Token *typetok = typesUsedInTemplateInstantiation[itype];
                         typetok && (typeindentlevel>0 || !Token::Match(typetok, ",|>"));
                         typetok = typetok->next()) {
                        if (Token::simpleMatch(typetok, ". . .")) {
                            typetok = typetok->tokAt(2);
                            continue;
                        }
                        if (Token::Match(typetok, "%name% <") && templateParameters(typetok->next()) > 0)
                            ++typeindentlevel;
                        else if (typeindentlevel > 0 && typetok->str() == ">")
                            --typeindentlevel;
                        tokenlist.addtoken(typetok, tok3->linenr(), tok3->fileIndex());
                    }
                    continue;
                }
            }

            // replace name..
            if (Token::Match(tok3, (name + " !!<").c_str())) {
                tokenlist.addtoken(newName, tok3->linenr(), tok3->fileIndex());
                continue;
            }

            // copy
            tokenlist.addtoken(tok3, tok3->linenr(), tok3->fileIndex());
            if (Token::Match(tok3, "%type% <") && Token::Match(tok3->next()->findClosingBracket(), ">|>> !!&")) {
                templateInstantiations.push_back(tokenlist.back());
            }

            // link() newly tokens manually
            else if (tok3->str() == "{") {
                brackets.push(tokenlist.back());
                indentlevel++;
            } else if (tok3->str() == "(") {
                brackets.push(tokenlist.back());
            } else if (tok3->str() == "[") {
                brackets.push(tokenlist.back());
            } else if (tok3->str() == "}") {
                assert(brackets.empty() == false && brackets.top()->str() == "{");
                Token::createMutualLinks(brackets.top(), tokenlist.back());
                if (tok3->strAt(1) == ";") {
                    const Token * tokSemicolon = tok3->next();
                    tokenlist.addtoken(tokSemicolon, tokSemicolon->linenr(), tokSemicolon->fileIndex());
                }
                brackets.pop();
                if (indentlevel <= 1 && brackets.empty()) {
                    // there is a bug if indentlevel is 0
                    // the "}" token should only be added if indentlevel is 1 but I add it always intentionally
                    // if indentlevel ever becomes 0, cppcheck will write:
                    // ### Error: Invalid number of character {
                    break;
                }
                --indentlevel;
            } else if (tok3->str() == ")") {
                assert(brackets.empty() == false && brackets.top()->str() == "(");
                Token::createMutualLinks(brackets.top(), tokenlist.back());
                brackets.pop();
            } else if (tok3->str() == "]") {
                assert(brackets.empty() == false && brackets.top()->str() == "[");
                Token::createMutualLinks(brackets.top(), tokenlist.back());
                brackets.pop();
            }
        }

        assert(brackets.empty());
    }
}

static bool isLowerThanLogicalAnd(const Token *lower)
{
    return lower->isAssignmentOp() || Token::Match(lower, "}|;|(|[|]|)|,|?|:|%oror%|return|throw|case");
}
static bool isLowerThanOr(const Token* lower)
{
    return isLowerThanLogicalAnd(lower) || lower->str() == "&&";
}
static bool isLowerThanXor(const Token* lower)
{
    return isLowerThanOr(lower) || lower->str() == "|";
}
static bool isLowerThanAnd(const Token* lower)
{
    return isLowerThanXor(lower) || lower->str() == "^";
}
static bool isLowerThanShift(const Token* lower)
{
    return isLowerThanAnd(lower) || lower->str() == "&";
}
static bool isLowerThanPlusMinus(const Token* lower)
{
    return isLowerThanShift(lower) || Token::Match(lower, "%comp%|<<|>>");
}
static bool isLowerThanMulDiv(const Token* lower)
{
    return isLowerThanPlusMinus(lower) || Token::Match(lower, "+|-");
}
static bool isLowerEqualThanMulDiv(const Token* lower)
{
    return isLowerThanMulDiv(lower) || Token::Match(lower, "[*/%]");
}


bool TemplateSimplifier::simplifyNumericCalculations(Token *tok)
{
    bool ret = false;
    // (1-2)
    while (tok->tokAt(4) && tok->next()->isNumber() && tok->tokAt(3)->isNumber()) { // %any% %num% %any% %num% %any%
        const Token* op = tok->tokAt(2);
        const Token* after = tok->tokAt(4);
        if (Token::Match(tok, "* %num% /") && (op->strAt(1) != "0") && tok->next()->str() == MathLib::multiply(op->strAt(1), MathLib::divide(tok->next()->str(), op->strAt(1)))) {
            // Division where result is a whole number
        } else if (!((op->str() == "*" && (isLowerThanMulDiv(tok) || tok->str() == "*") && isLowerEqualThanMulDiv(after)) || // associative
                     (Token::Match(op, "[/%]") && isLowerThanMulDiv(tok) && isLowerEqualThanMulDiv(after)) || // NOT associative
                     (Token::Match(op, "[+-]") && isLowerThanMulDiv(tok) && isLowerThanMulDiv(after)) || // Only partially (+) associative, but handled later
                     (Token::Match(op, ">>|<<") && isLowerThanShift(tok) && isLowerThanPlusMinus(after)) || // NOT associative
                     (op->str() == "&" && isLowerThanShift(tok) && isLowerThanShift(after)) || // associative
                     (op->str() == "^" && isLowerThanAnd(tok) && isLowerThanAnd(after)) || // associative
                     (op->str() == "|" && isLowerThanXor(tok) && isLowerThanXor(after)) || // associative
                     (op->str() == "&&" && isLowerThanOr(tok) && isLowerThanOr(after)) ||
                     (op->str() == "||" && isLowerThanLogicalAnd(tok) && isLowerThanLogicalAnd(after))))
            break;

        tok = tok->next();

        // Don't simplify "%num% / 0"
        if (Token::Match(op, "[/%] 0"))
            continue;

        // Integer operations
        if (Token::Match(op, ">>|<<|&|^|%or%")) {
            // Don't simplify if operand is negative, shifting with negative
            // operand is UB. Bitmasking with negative operand is implementation
            // defined behaviour.
            if (MathLib::isNegative(tok->str()) || MathLib::isNegative(tok->strAt(2)))
                continue;

            const MathLib::value v1(tok->str());
            const MathLib::value v2(tok->strAt(2));

            if (!v1.isInt() || !v2.isInt())
                continue;

            switch (op->str()[0]) {
            case '<':
                tok->str((v1 << v2).str());
                break;
            case '>':
                tok->str((v1 >> v2).str());
                break;
            case '&':
                tok->str((v1 & v2).str());
                break;
            case '|':
                tok->str((v1 | v2).str());
                break;
            case '^':
                tok->str((v1 ^ v2).str());
                break;
            };
        }

        // Logical operations
        else if (Token::Match(op, "%oror%|&&")) {
            int op1 = !MathLib::isNullValue(tok->str());
            int op2 = !MathLib::isNullValue(tok->strAt(2));
            int result = (op->str() == "||") ? (op1 || op2) : (op1 && op2);
            tok->str(result ? "1" : "0");
        }

        else if (Token::Match(tok->previous(), "- %num% - %num%"))
            tok->str(MathLib::add(tok->str(), tok->strAt(2)));
        else if (Token::Match(tok->previous(), "- %num% + %num%"))
            tok->str(MathLib::subtract(tok->str(), tok->strAt(2)));
        else {
            try {
                tok->str(MathLib::calculate(tok->str(), tok->strAt(2), op->str()[0]));
            } catch (InternalError &e) {
                e.token = tok;
                throw;
            }
        }

        tok->deleteNext(2);
        tok = tok->previous();

        ret = true;
    }

    if (Token::Match(tok, "%oror%|&& %num% %oror%|&&|,|)") || Token::Match(tok, "[(,] %num% %oror%|&&")) {
        tok->next()->str(MathLib::isNullValue(tok->next()->str()) ? "0" : "1");
    }

    return ret;
}

// TODO: This is not the correct class for simplifyCalculations(), so it
// should be moved away.
bool TemplateSimplifier::simplifyCalculations(Token *_tokens)
{
    bool ret = false, goback = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        // Remove parentheses around variable..
        // keep parentheses here: dynamic_cast<Fred *>(p);
        // keep parentheses here: A operator * (int);
        // keep parentheses here: int ( * ( * f ) ( ... ) ) (int) ;
        // keep parentheses here: int ( * * ( * compilerHookVector ) (void) ) ( ) ;
        // keep parentheses here: operator new [] (size_t);
        // keep parentheses here: Functor()(a ... )
        // keep parentheses here: ) ( var ) ;
        if ((Token::Match(tok->next(), "( %name% ) ;|)|,|]") ||
             (Token::Match(tok->next(), "( %name% ) %cop%") && (tok->tokAt(2)->varId()>0 || !Token::Match(tok->tokAt(4), "[*&+-~]")))) &&
            !tok->isName() &&
            tok->str() != ">" &&
            tok->str() != ")" &&
            tok->str() != "]") {
            tok->deleteNext();
            tok = tok->next();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "(|&&|%oror% %char% %comp% %num% &&|%oror%|)")) {
            tok->str(MathLib::toString(MathLib::toLongNumber(tok->str())));
        }

        if (tok->isNumber()) {
            // Remove redundant conditions (0&&x) (1||x)
            if (Token::Match(tok->previous(), "[(=,] 0 &&") ||
                Token::Match(tok->previous(), "[(=,] 1 %oror%")) {
                unsigned int par = 0;
                const Token *tok2 = tok;
                bool andAnd = (tok->next()->str() == "&&");
                for (; tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "[")
                        ++par;
                    else if (tok2->str() == ")" || tok2->str() == "]") {
                        if (par == 0)
                            break;
                        --par;
                    } else if (par == 0 && isLowerThanLogicalAnd(tok2) && (andAnd || tok2->str() != "||"))
                        break;
                }
                if (tok2) {
                    Token::eraseTokens(tok, tok2);
                    ret = true;
                }
                continue;
            }

            if (tok->str() == "0") {
                if ((Token::Match(tok->previous(), "[+-] 0 %cop%|;") && isLowerThanMulDiv(tok->next())) ||
                    (Token::Match(tok->previous(), "%or% 0 %cop%|;") && isLowerThanXor(tok->next()))) {
                    tok = tok->previous();
                    if (Token::Match(tok->tokAt(-4), "[;{}] %name% = %name% [+-|] 0 ;") &&
                        tok->strAt(-3) == tok->previous()->str()) {
                        tok = tok->tokAt(-3);
                        tok->deleteNext(2);
                        tok->deleteThis();
                    }
                    tok->deleteNext();
                    tok->deleteThis();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=([,] 0 [+|]") ||
                           Token::Match(tok->previous(), "return|case 0 [+|]")) {
                    tok = tok->previous();
                    tok->deleteNext(2);
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 0 * %name% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "[=[(,] 0 * %num% ,|]|)|;|%op%") ||
                           Token::Match(tok->previous(), "[=[(,] 0 * (") ||
                           Token::Match(tok->previous(), "return|case 0 *|&& %name% ,|:|;|=|%cop%") ||
                           Token::Match(tok->previous(), "return|case 0 *|&& %num% ,|:|;|%op%") ||
                           Token::Match(tok->previous(), "return|case 0 *|&& (")) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 0 && *|& %any% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "return|case 0 && *|& %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (tok->str() == "1") {
                if (Token::Match(tok->previous(), "[=[(,] 1 %oror% %any% ,|]|)|;|=|%cop%") ||
                    Token::Match(tok->previous(), "return|case 1 %oror% %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 1 %oror% *|& %any% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "return|case 1 %oror% *|& %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (Token::simpleMatch(tok->previous(), "* 1") || Token::simpleMatch(tok, "1 *")) {
                if (tok->previous() && tok->previous()->isConstOp())
                    tok = tok->previous();
                tok->deleteNext();
                tok->deleteThis();
                ret = true;
            }

            // Remove parentheses around number..
            if (Token::Match(tok->tokAt(-2), "%op%|< ( %num% )") && !tok->tokAt(-2)->isName() && tok->strAt(-2) != ">") {
                tok = tok->previous();
                tok->deleteThis();
                tok->deleteNext();
                ret = true;
            }

            if (Token::simpleMatch(tok->previous(), "( 0 |") ||
                Token::simpleMatch(tok->previous(), "| 0 )")) {
                if (tok->previous()->isConstOp())
                    tok = tok->previous();
                tok->deleteNext();
                tok->deleteThis();
                ret = true;
            }

            if (Token::Match(tok, "%num% %comp% %num%") &&
                MathLib::isInt(tok->str()) &&
                MathLib::isInt(tok->strAt(2))) {
                if (Token::Match(tok->previous(), "(|&&|%oror%") && Token::Match(tok->tokAt(3), ")|&&|%oror%|?")) {
                    const MathLib::bigint op1(MathLib::toLongNumber(tok->str()));
                    const std::string &cmp(tok->next()->str());
                    const MathLib::bigint op2(MathLib::toLongNumber(tok->strAt(2)));

                    std::string result;

                    if (cmp == "==")
                        result = (op1 == op2) ? "1" : "0";
                    else if (cmp == "!=")
                        result = (op1 != op2) ? "1" : "0";
                    else if (cmp == "<=")
                        result = (op1 <= op2) ? "1" : "0";
                    else if (cmp == ">=")
                        result = (op1 >= op2) ? "1" : "0";
                    else if (cmp == "<")
                        result = (op1 < op2) ? "1" : "0";
                    else
                        result = (op1 > op2) ? "1" : "0";

                    tok->str(result);
                    tok->deleteNext(2);
                    ret = true;
                    goback = true;
                }
            }
        }
        // Division where result is a whole number
        else if (Token::Match(tok->previous(), "* %num% /") &&
                 tok->str() == MathLib::multiply(tok->strAt(2), MathLib::divide(tok->str(), tok->strAt(2)))) {
            tok->deleteNext(2);
        }

        else if (simplifyNumericCalculations(tok)) {
            ret = true;
            while (Token::Match(tok->tokAt(-2), "%cop%|,|( %num% %cop% %num% %cop%|,|)")) {
                Token *before = tok->tokAt(-2);
                if (simplifyNumericCalculations(before))
                    tok = before;
                else
                    break;
            }
        }
    }
    return ret;
}

const Token * TemplateSimplifier::TemplateParametersInDeclaration(
    const Token * tok,
    std::vector<const Token *> & typeParametersInDeclaration)
{
    typeParametersInDeclaration.clear();
    for (; tok && tok->str() != ">"; tok = tok->next()) {
        if (Token::Match(tok, "%name% ,|>"))
            typeParametersInDeclaration.push_back(tok);
    }
    return tok;
}

bool TemplateSimplifier::simplifyTemplateInstantiations(
    TokenList& tokenlist,
    ErrorLogger* errorlogger,
    const Settings *_settings,
    const Token *tok,
    const std::time_t maxtime,
    std::list<Token *> &templateInstantiations,
    std::set<std::string> &expandedtemplates)
{
    // this variable is not used at the moment. The intention was to
    // allow continuous instantiations until all templates has been expanded
    //bool done = false;

    // Contains tokens such as "T"
    std::vector<const Token *> typeParametersInDeclaration;
    tok = TemplateParametersInDeclaration(tok->tokAt(2), typeParametersInDeclaration);

    // bail out if the end of the file was reached
    if (!tok)
        return false;

    const bool printDebug = _settings->debugwarnings;

    // get the position of the template name
    int namepos = TemplateSimplifier::getTemplateNamePosition(tok);
    if (namepos == -1) {
        // debug message that we bail out..
        if (printDebug && errorlogger) {
            std::list<const Token *> callstack(1, tok);
            errorlogger->reportErr(ErrorLogger::ErrorMessage(callstack, &tokenlist, Severity::debug, "debug", "simplifyTemplates: bailing out", false));
        }
        return false;
    }

    // name of template function/class..
    const std::string name(tok->strAt(namepos));

    const bool isfunc(tok->strAt(namepos + 1) == "(");

    // locate template usage..
    std::string::size_type amountOftemplateInstantiations = templateInstantiations.size();
    unsigned int recursiveCount = 0;

    bool instantiated = false;

    for (std::list<Token *>::const_iterator iter2 = templateInstantiations.begin(); iter2 != templateInstantiations.end(); ++iter2) {
        if (amountOftemplateInstantiations != templateInstantiations.size()) {
            amountOftemplateInstantiations = templateInstantiations.size();
            simplifyCalculations(tokenlist.front());
            ++recursiveCount;
            if (recursiveCount > 100) {
                // bail out..
                break;
            }
        }
        Token * const tok2 = *iter2;
        if (errorlogger && !tokenlist.getFiles().empty())
            errorlogger->reportProgress(tokenlist.getFiles()[0], "TemplateSimplifier::simplifyTemplateInstantiations()", tok2->progressValue());
#ifdef MAXTIME
        if (std::time(0) > maxtime)
            return false;
#else
        (void)maxtime;
#endif
        assert(tokenlist.validateToken(tok2)); // that assertion fails on examples from #6021
        if (tok2->str() != name)
            continue;

        if (Token::Match(tok2->previous(), "[;{}=]") &&
            !TemplateSimplifier::instantiateMatch(*iter2, name, typeParametersInDeclaration.size(), isfunc ? "(" : "*| %name%"))
            continue;

        // New type..
        std::vector<const Token *> typesUsedInTemplateInstantiation;
        std::string typeForNewName;
        std::string typeForPatternMatch;
        unsigned int indentlevel = 0;
        for (const Token *tok3 = tok2->tokAt(2); tok3 && (indentlevel > 0 || tok3->str() != ">"); tok3 = tok3->next()) {
            // #2648 - unhandled parentheses => bail out
            // #2721 - unhandled [ => bail out
            if (Token::Match(tok3, "(|[")) {
                typeForNewName.clear();
                break;
            }
            if (!tok3->next()) {
                typeForNewName.clear();
                break;
            }
            if (Token::Match(tok3->tokAt(-2), "[<,] %name% <") && templateParameters(tok3) > 0)
                ++indentlevel;
            else if (indentlevel > 0 && Token::Match(tok3, "> [,>]"))
                --indentlevel;
            if (indentlevel == 0 && Token::Match(tok3->previous(), "[<,]"))
                typesUsedInTemplateInstantiation.push_back(tok3);
            const bool constconst = tok3->str() == "const" && tok3->strAt(1) == "const";
            if (!constconst) {
                if (!typeForPatternMatch.empty())
                    typeForPatternMatch += ' ';
                typeForPatternMatch += tok3->str();
            }
            // add additional type information
            if (!constconst && tok3->str() != "class") {
                if (tok3->isUnsigned())
                    typeForNewName += "unsigned";
                else if (tok3->isSigned())
                    typeForNewName += "signed";
                if (tok3->isLong())
                    typeForNewName += "long";
                if (!typeForNewName.empty())
                    typeForNewName += ' ';
                typeForNewName += tok3->str();
            }
        }
        std::string templateMatchPattern(name + " < " + typeForPatternMatch + " >");

        if (typeForNewName.empty() || typeParametersInDeclaration.size() != typesUsedInTemplateInstantiation.size()) {
            if (printDebug && errorlogger) {
                std::list<const Token *> callstack(1, tok2);
                errorlogger->reportErr(ErrorLogger::ErrorMessage(callstack, &tokenlist, Severity::debug, "debug",
                                       "Failed to instantiate template \"" + name + "\". The checking continues anyway.", false));
            }
            if (typeForNewName.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string newName(name + " < " + typeForNewName + " >");

        if (expandedtemplates.find(newName) == expandedtemplates.end()) {
            expandedtemplates.insert(newName);
            TemplateSimplifier::expandTemplate(tokenlist, tok,name,typeParametersInDeclaration,newName,typesUsedInTemplateInstantiation,templateInstantiations);
            instantiated = true;
        }

        // Replace all these template usages..
        std::list< std::pair<Token *, Token *> > removeTokens;
        for (Token *tok4 = tok2; tok4; tok4 = tok4->next()) {
            if (Token::simpleMatch(tok4, templateMatchPattern.c_str())) {
                Token * tok5 = tok4->tokAt(2);
                unsigned int typeCountInInstantiation = 1U; // There is always at least one type
                const Token *typetok = (!typesUsedInTemplateInstantiation.empty()) ? typesUsedInTemplateInstantiation[0] : 0;
                unsigned int indentlevel5 = 0;  // indentlevel for tok5
                while (tok5 && (indentlevel5 > 0 || tok5->str() != ">")) {
                    if (tok5->str() == "<" && templateParameters(tok5) > 0)
                        ++indentlevel5;
                    else if (indentlevel5 > 0 && Token::Match(tok5, "> [,>]"))
                        --indentlevel5;
                    else if (indentlevel5 == 0) {
                        if (tok5->str() != ",") {
                            if (!typetok ||
                                tok5->isUnsigned() != typetok->isUnsigned() ||
                                tok5->isSigned() != typetok->isSigned() ||
                                tok5->isLong() != typetok->isLong()) {
                                break;
                            }

                            typetok = typetok ? typetok->next() : 0;
                        } else {
                            typetok = (typeCountInInstantiation < typesUsedInTemplateInstantiation.size()) ? typesUsedInTemplateInstantiation[typeCountInInstantiation] : 0;
                            ++typeCountInInstantiation;
                        }
                    }
                    tok5 = tok5->next();
                }

                // matching template usage => replace tokens..
                // Foo < int >  =>  Foo<int>
                if (tok5 && tok5->str() == ">" && typeCountInInstantiation == typesUsedInTemplateInstantiation.size()) {
                    tok4->str(newName);
                    for (Token *tok6 = tok4->next(); tok6 != tok5; tok6 = tok6->next()) {
                        if (tok6->isName())
                            templateInstantiations.remove(tok6);
                    }
                    removeTokens.push_back(std::pair<Token*,Token*>(tok4, tok5->next()));
                }

                tok4 = tok5;
                if (!tok4)
                    break;
            }
        }
        while (!removeTokens.empty()) {
            Token::eraseTokens(removeTokens.back().first, removeTokens.back().second);
            removeTokens.pop_back();
        }
    }

    // Template has been instantiated .. then remove the template declaration
    return instantiated;
}


void TemplateSimplifier::simplifyTemplates(
    TokenList& tokenlist,
    ErrorLogger* errorlogger,
    const Settings *_settings,
    const std::time_t maxtime,
    bool &_codeWithTemplates
)
{

    std::set<std::string> expandedtemplates(TemplateSimplifier::expandSpecialized(tokenlist.front()));

    if (TemplateSimplifier::getTemplateDeclarations(tokenlist.front(), _codeWithTemplates).empty())
        return;

    // There are templates..
    // Remove "typename" unless used in template arguments..
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->str() == "typename")
            tok->deleteThis();

        if (Token::simpleMatch(tok, "template <")) {
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;
        }
    }

    std::list<Token *> templates(TemplateSimplifier::getTemplateDeclarations(tokenlist.front(), _codeWithTemplates));

    // Locate possible instantiations of templates..
    std::list<Token *> templateInstantiations(TemplateSimplifier::getTemplateInstantiations(tokenlist.front()));

    // No template instantiations? Then return.
    if (templateInstantiations.empty())
        return;

    // Template arguments with default values
    TemplateSimplifier::useDefaultArgumentValues(templates, &templateInstantiations);

    // expand templates
    //bool done = false;
    //while (!done)
    {
        //done = true;
        std::list<Token *> templates2;
        for (std::list<Token *>::reverse_iterator iter1 = templates.rbegin(); iter1 != templates.rend(); ++iter1) {
            bool instantiated = TemplateSimplifier::simplifyTemplateInstantiations(tokenlist,
                                errorlogger,
                                _settings,
                                *iter1,
                                maxtime,
                                templateInstantiations,
                                expandedtemplates);
            if (instantiated)
                templates2.push_back(*iter1);
        }

        for (std::list<Token *>::const_iterator it = templates2.begin(); it != templates2.end(); ++it) {
            std::list<Token *>::iterator it1 = std::find(templates.begin(), templates.end(), *it);
            if (it1 != templates.end()) {
                templates.erase(it1);
                removeTemplate(*it);
            }
        }
    }
}

void TemplateSimplifier::syntaxError(const Token *tok)
{
    throw InternalError(tok, "syntax error", InternalError::SYNTAX);
}
