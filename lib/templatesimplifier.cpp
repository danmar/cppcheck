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

#include "templatesimplifier.h"
#include "token.h"
#include <sstream>
#include <list>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <cassert>

//---------------------------------------------------------------------------

TemplateSimplifier::TemplateSimplifier()
{
}

TemplateSimplifier::~TemplateSimplifier()
{
}

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

        else if (Token::Match(tok, "%type% <") &&
                 (!tok->previous() || tok->previous()->str() == ";")) {
            const Token *tok2 = tok->tokAt(2);
            std::string type;
            while (Token::Match(tok2, "%type% ,") || Token::Match(tok2, "%num% ,")) {
                type += tok2->str() + ",";
                tok2 = tok2->tokAt(2);
            }
            if (Token::Match(tok2, "%type% > (") || Token::Match(tok2, "%num% > (")) {
                type += tok2->str();
                tok->str(tok->str() + "<" + type + ">");
                Token::eraseTokens(tok, tok2->tokAt(2));
                if (tok == tokens)
                    goback = true;
            }
        }
    }
}


const Token* TemplateSimplifier::hasComplicatedSyntaxErrorsInTemplates(Token *tokens)
{
    // check for more complicated syntax errors when using templates..
    for (const Token *tok = tokens; tok; tok = tok->next()) {
        // skip executing scopes..
        if (Token::Match(tok, ") const| {") || Token::Match(tok, "[,=] {")) {
            while (tok->str() != "{")
                tok = tok->next();
            tok = tok->link();
        }

        // skip executing scopes (ticket #1984)..
        if (Token::simpleMatch(tok, "; {"))
            tok = tok->next()->link();

        // skip executing scopes (ticket #3183)..
        if (Token::simpleMatch(tok, "( {"))
            tok = tok->next()->link();

        // skip executing scopes (ticket #1985)..
        if (Token::simpleMatch(tok, "try {")) {
            tok = tok->next()->link();
            while (Token::simpleMatch(tok, "} catch (")) {
                tok = tok->linkAt(2);
                if (Token::simpleMatch(tok, ") {"))
                    tok = tok->next()->link();
            }
        }

        // not start of statement?
        if (tok->previous() && !Token::Match(tok, "[;{}]"))
            continue;

        // skip starting tokens.. ;;; typedef typename foo::bar::..
        while (Token::Match(tok, "[;{}]"))
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
            if (level > 0) {
//                syntaxError(tok);
                return tok;
            }
        }
    }

    return 0;
}

unsigned int TemplateSimplifier::templateParameters(const Token *tok)
{
    unsigned int numberOfParameters = 0;

    if (!tok)
        return 0;
    if (tok->str() != "<")
        return 0;
    tok = tok->next();

    unsigned int level = 0;

    while (tok) {
        if (level == 0)
            ++numberOfParameters;

        // skip std::
        while (Token::Match(tok, "%var% ::"))
            tok = tok->tokAt(2);
        if (!tok)
            return 0;

        // num/type ..
        if (!tok->isNumber() && !tok->isName())
            return 0;
        tok = tok->next();

        // optional "*"
        if (tok->str() == "*")
            tok = tok->next();

        // inner template
        if (tok->str() == "<") {
            ++level;
            tok = tok->next();
            continue;
        }

        // ,/>
        while (tok->str() == ">") {
            if (level == 0)
                return numberOfParameters;
            --level;
            tok = tok->next();
        }
        if (tok->str() != ",")
            break;
        tok = tok->next();
    }
    return 0;
}

void TemplateSimplifier::removeTemplates(Token *tok)
{
    bool goback = false;
    for (; tok; tok = tok->next()) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        if (!Token::simpleMatch(tok, "template <"))
            continue;

        for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {

            if (tok2->str() == "(") {
                tok2 = tok2->link();
            }

            else if (tok2->str() == "{") {
                tok2 = tok2->link()->next();
                Token::eraseTokens(tok, tok2);
                if (tok2 && tok2->str() == ";" && tok2->next())
                    tok->deleteNext();
                tok->deleteThis();
                goback = true;
                break;
            } else if (tok2->str() == "}") {  // garbage code! (#3449)
                Token::eraseTokens(tok,tok2);
                tok->deleteThis();
                break;
            }
            // don't remove constructor
            if (tok2->str() == "explicit") {
                Token::eraseTokens(tok, tok2);
                tok->deleteThis();
                goback = true;
                break;
            }

            if (tok2->str() == ";") {
                tok2 = tok2->next();
                Token::eraseTokens(tok, tok2);
                tok->deleteThis();
                goback = true;
                break;
            }

            if (Token::Match(tok2, ">|>> class|struct %var% [,)]")) {
                tok2 = tok2->next();
                Token::eraseTokens(tok, tok2);
                tok->deleteThis();
                goback = true;
                break;
            }
        }
    }
}


std::set<std::string> TemplateSimplifier::simplifyTemplatesExpandSpecialized(Token *tokens)
{
    std::set<std::string> expandedtemplates;

    // Locate specialized templates..
    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (tok->str() != "template")
            continue;
        if (!Token::simpleMatch(tok->next(), "< >"))
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
            for (tok3 = tok2; tok3 && tok3->str() != ">"; tok3 = tok3->next()) {
                if (tok3 != tok2)
                    ostr << " ";
                ostr << tok3->str();
            }
            if (!Token::simpleMatch(tok3, "> ("))
                continue;
            s = ostr.str();
        }

        // save search pattern..
        const std::string pattern(s + " > (");

        // remove spaces to create new name
        while (s.find(" ") != std::string::npos)
            s.erase(s.find(" "), 1);
        const std::string name(s + ">");
        expandedtemplates.insert(name);

        // Rename template..
        Token::eraseTokens(tok2, Token::findsimplematch(tok2, "("));
        tok2->str(name);

        // delete the "template < >"
        tok->deleteNext(2);
        tok->deleteThis();

        // Use this special template in the code..
        while (NULL != (tok2 = const_cast<Token *>(Token::findmatch(tok2, pattern.c_str())))) {
            Token::eraseTokens(tok2, Token::findsimplematch(tok2, "("));
            tok2->str(name);
        }
    }

    return expandedtemplates;
}

std::list<Token *> TemplateSimplifier::simplifyTemplatesGetTemplateDeclarations(Token *tokens, bool &codeWithTemplates)
{
    std::list<Token *> templates;
    for (Token *tok = tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "template <")) {
            codeWithTemplates = true;

            for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
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


std::list<Token *> TemplateSimplifier::simplifyTemplatesGetTemplateInstantiations(Token *tokens)
{
    std::list<Token *> used;

    for (Token *tok = tokens; tok; tok = tok->next()) {
        // template definition.. skip it
        if (Token::simpleMatch(tok, "template <")) {
            unsigned int level = 0;

            // Goto the end of the template definition
            for (; tok; tok = tok->next()) {
                // skip '<' .. '>'
                if (tok->str() == "<")
                    ++level;
                else if (tok->str() == ">") {
                    if (level <= 1)
                        break;
                    --level;
                }

                // skip inner '(' .. ')' and '{' .. '}'
                else if (tok->str() == "{" || tok->str() == "(") {
                    // skip inner tokens. goto ')' or '}'
                    tok = tok->link();

                    // this should be impossible. but break out anyway
                    if (!tok)
                        break;

                    // the end '}' for the template definition => break
                    if (tok->str() == "}")
                        break;
                }

                // the end ';' for the template definition
                else if (tok->str() == ";") {
                    break;
                }
            }
            if (!tok)
                break;
        } else if (Token::Match(tok->previous(), "[({};=] %var% <") ||
                   Token::Match(tok->tokAt(-2), "[,:] private|protected|public %var% <")) {

            // Add inner template instantiations first => go to the ">"
            // and then parse backwards, adding all seen instantiations
            const Token *tok2;

            // goto end ">" token
            unsigned int level = 0;
            for (tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "<") {
                    ++level;
                } else if (tok2->str() == ">") {
                    if (level <= 1)
                        break;
                    --level;
                }
            }

            // parse backwards and add template instantiations
            for (; tok2 && tok2 != tok; tok2 = tok2->previous()) {
                if (Token::Match(tok2, ", %var% <") &&
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


void TemplateSimplifier::simplifyTemplatesUseDefaultArgumentValues(const std::list<Token *> &templates,
        const std::list<Token *> &templateInstantiations)
{
    for (std::list<Token *>::const_iterator iter1 = templates.begin(); iter1 != templates.end(); ++iter1) {
        // template parameters with default value has syntax such as:
        //     x = y
        // this list will contain all the '=' tokens for such arguments
        std::list<Token *> eq;

        // parameter number. 1,2,3,..
        std::size_t templatepar = 1;

        // the template classname. This will be empty for template functions
        std::string classname;

        // Scan template declaration..
        for (Token *tok = *iter1; tok; tok = tok->next()) {
            // end of template parameters?
            if (tok->str() == ">") {
                if (Token::Match(tok, "> class|struct %var%"))
                    classname = tok->strAt(2);
                break;
            }

            // next template parameter
            if (tok->str() == ",")
                ++templatepar;

            // default parameter value
            else if (tok->str() == "=")
                eq.push_back(tok);
        }
        if (eq.empty() || classname.empty())
            continue;

        // iterate through all template instantiations
        for (std::list<Token *>::const_iterator iter2 = templateInstantiations.begin(); iter2 != templateInstantiations.end(); ++iter2) {
            Token *tok = *iter2;

            if (!Token::Match(tok, (classname + " < %any%").c_str()))
                continue;

            // count the parameters..
            unsigned int usedpar = 1;
            for (tok = tok->tokAt(3); tok; tok = tok->tokAt(2)) {
                if (tok->str() == ">")
                    break;

                if (tok->str() == ",")
                    ++usedpar;

                else
                    break;
            }
            if (tok && tok->str() == ">") {
                tok = tok->previous();
                std::list<Token *>::const_iterator it = eq.begin();
                for (std::size_t i = (templatepar - eq.size()); it != eq.end() && i < usedpar; ++i)
                    ++it;
                while (it != eq.end()) {
                    tok->insertToken(",");
                    tok = tok->next();
                    const Token *from = (*it)->next();
                    std::stack<Token *> links;
                    while (from && (!links.empty() || (from->str() != "," && from->str() != ">"))) {
                        tok->insertToken(from->str());
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
            (*it)->deleteNext();
            (*it)->deleteThis();
        }
    }
}

bool TemplateSimplifier::simplifyTemplatesInstantiateMatch(const Token *instance, const std::string &name, size_t numberOfArguments, const char patternAfter[])
{
    if (!Token::simpleMatch(instance, (name + " <").c_str()))
        return false;

    if (numberOfArguments != TemplateSimplifier::templateParameters(instance->next()))
        return false;

    if (patternAfter) {
        const Token *tok = Token::findsimplematch(instance, ">");
        if (!tok || !Token::Match(tok->next(), patternAfter))
            return false;
    }

    // nothing mismatching was found..
    return true;
}

int TemplateSimplifier::simplifyTemplatesGetTemplateNamePosition(const Token *tok)
{
    // get the position of the template name
    int namepos = 0;
    if (Token::Match(tok, "> class|struct %type% {|:"))
        namepos = 2;
    else if (Token::Match(tok, "> %type% *|&| %type% ("))
        namepos = 2;
    else if (Token::Match(tok, "> %type% %type% *|&| %type% ("))
        namepos = 3;
    else {
        // Name not found
        return -1;
    }
    if ((tok->strAt(namepos) == "*" || tok->strAt(namepos) == "&"))
        ++namepos;

    return namepos;
}
