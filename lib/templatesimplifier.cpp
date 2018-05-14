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

#include "templatesimplifier.h"

#include "errorlogger.h"
#include "mathlib.h"
#include "settings.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stack>
#include <utility>

namespace {
    class FindToken {
    public:
        explicit FindToken(const Token *t) : token(t) {}
        bool operator()(const TemplateSimplifier::TokenAndName &t) const {
            return t.token == token;
        }
    private:
        const Token * const token;
    };

    class FindName {
    public:
        explicit FindName(const std::string &s) : name(s) {}
        bool operator()(const TemplateSimplifier::TokenAndName &t) const {
            return t.name == name;
        }
    private:
        const std::string name;
    };
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

            if (Token::simpleMatch(tok,"("))
                tok = tok->link()->next();

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

/// TODO: This is copy pasted from Tokenizer. We should reuse this code.
namespace {
    struct ScopeInfo2 {
        ScopeInfo2(const std::string &name_, const Token *bodyEnd_) : name(name_), bodyEnd(bodyEnd_) {}
        const std::string name;
        const Token * const bodyEnd;
    };
}
static std::string getScopeName(const std::list<ScopeInfo2> &scopeInfo)
{
    std::string ret;
    for (std::list<ScopeInfo2>::const_iterator it = scopeInfo.begin(); it != scopeInfo.end(); ++it)
        ret += (ret.empty() ? "" : " :: ") + (it->name);
    return ret;
}
static std::string getFullName(const std::list<ScopeInfo2> &scopeInfo, const std::string &name)
{
    const std::string &scopeName = getScopeName(scopeInfo);
    return scopeName + (scopeName.empty() ? "" : " :: ") + name;
}

static void setScopeInfo(const Token *tok, std::list<ScopeInfo2> *scopeInfo)
{
    while (tok->str() == "}" && !scopeInfo->empty() && tok == scopeInfo->back().bodyEnd)
        scopeInfo->pop_back();
    if (!Token::Match(tok, "namespace|class|struct %name% {|:|::"))
        return;
    tok = tok->next();
    std::string classname = tok->str();
    while (Token::Match(tok, "%name% :: %name%")) {
        tok = tok->tokAt(2);
        classname += " :: " + tok->str();
    }
    tok = tok->next();
    if (tok && tok->str() == ":") {
        // ...
    }
    if (tok && tok->str() == "{") {
        scopeInfo->emplace_back(classname,tok->link());
    }
}

std::list<TemplateSimplifier::TokenAndName> TemplateSimplifier::getTemplateDeclarations(Token *tokens, bool &codeWithTemplates)
{
    std::list<ScopeInfo2> scopeInfo;
    std::list<TokenAndName> declarations;
    for (Token *tok = tokens; tok; tok = tok->next()) {
        setScopeInfo(tok, &scopeInfo);
        if (!Token::simpleMatch(tok, "template <"))
            continue;
        // Some syntax checks, see #6865
        if (!tok->tokAt(2))
            syntaxError(tok->next());
        if (tok->strAt(2)=="typename" &&
            !Token::Match(tok->tokAt(3), "%name%|.|,|>"))
            syntaxError(tok->next());
        codeWithTemplates = true;
        Token *parmEnd = tok->next()->findClosingBracket();
        for (const Token *tok2 = parmEnd; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ")")
                break;
            // Just a declaration => ignore this
            else if (tok2->str() == ";")
                break;
            // Implementation => add to "templates"
            else if (tok2->str() == "{") {
                const int namepos = getTemplateNamePosition(parmEnd);
                if (namepos > 0)
                    declarations.emplace_back(tok, getScopeName(scopeInfo), getFullName(scopeInfo, parmEnd->strAt(namepos)));
                break;
            }
        }
    }
    return declarations;
}


std::list<TemplateSimplifier::TokenAndName> TemplateSimplifier::getTemplateInstantiations(Token *tokens, const std::list<TokenAndName> &declarations)
{
    std::list<TokenAndName> instantiations;
    std::list<ScopeInfo2> scopeList;

    for (Token *tok = tokens; tok; tok = tok->next()) {
        setScopeInfo(tok, &scopeList);
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
        } else if (Token::Match(tok->previous(), "[({};=] %name% ::|<") ||
                   Token::Match(tok->previous(), "%type% %name% ::|<") ||
                   Token::Match(tok->tokAt(-2), "[,:] private|protected|public %name% ::|<")) {

            std::string scopeName = getScopeName(scopeList);
            while (Token::Match(tok, "%name% :: %name%")) {
                scopeName += (scopeName.empty() ? "" : " :: ") + tok->str();
                tok = tok->tokAt(2);
            }
            if (!Token::Match(tok, "%name% <"))
                continue;

            // Add inner template instantiations first => go to the ">"
            // and then parse backwards, adding all seen instantiations
            const Token *tok2 = tok->next()->findClosingBracket();

            // parse backwards and add template instantiations
            // TODO
            for (; tok2 && tok2 != tok; tok2 = tok2->previous()) {
                if (Token::Match(tok2, ", %name% <") &&
                    TemplateSimplifier::templateParameters(tok2->tokAt(2))) {
                    instantiations.emplace_back(tok2->next(), getScopeName(scopeList), getFullName(scopeList, tok2->strAt(1)));
                }
            }

            // Add outer template..
            if (TemplateSimplifier::templateParameters(tok->next())) {
                const std::string scopeName1(scopeName);
                while (true) {
                    const std::string fullName = scopeName + (scopeName.empty()?"":" :: ") + tok->str();
                    const std::list<TokenAndName>::const_iterator it = std::find_if(declarations.begin(), declarations.end(), FindName(fullName));
                    if (it != declarations.end()) {
                        instantiations.emplace_back(tok, getScopeName(scopeList), fullName);
                        break;
                    } else {
                        if (scopeName.empty()) {
                            instantiations.emplace_back(tok, getScopeName(scopeList), scopeName1 + (scopeName1.empty()?"":" :: ") + tok->str());
                            break;
                        }
                        const std::string::size_type pos = scopeName.rfind(" :: ");
                        scopeName = (pos == std::string::npos) ? std::string() : scopeName.substr(0,pos);
                    }
                }
            }
        }
    }

    return instantiations;
}


void TemplateSimplifier::useDefaultArgumentValues(const std::list<TokenAndName> &templates,
        std::list<TokenAndName> * const templateInstantiations)
{
    for (std::list<TokenAndName>::const_iterator iter1 = templates.begin(); iter1 != templates.end(); ++iter1) {
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
        for (Token *tok = iter1->token; tok; tok = tok->next()) {
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
        for (std::list<TokenAndName>::const_iterator iter2 = templateInstantiations->begin(); iter2 != templateInstantiations->end(); ++iter2) {
            Token *tok = iter2->token;

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
                    std::list<TokenAndName>::iterator ti = std::find_if(templateInstantiations->begin(),
                                                           templateInstantiations->end(),
                                                           FindToken(tok2));
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

void TemplateSimplifier::simplifyTemplateAliases(std::list<TemplateSimplifier::TokenAndName> *templateInstantiations)
{
    std::list<TemplateSimplifier::TokenAndName>::iterator it1, it2;
    for (it1 = templateInstantiations->begin(); it1 != templateInstantiations->end();) {
        TemplateSimplifier::TokenAndName &templateAlias = *it1;
        ++it1;
        Token *startToken = templateAlias.token;
        while (Token::Match(startToken->tokAt(-2), "%name% :: %name%"))
            startToken = startToken->tokAt(-2);
        if (!Token::Match(startToken->tokAt(-4), "> using %name% = %name% ::|<"))
            continue;
        const std::string aliasName(startToken->strAt(-2));
        const Token * const aliasToken1 = startToken;

        // Get start token for alias
        startToken = startToken->tokAt(-5);
        while (Token::Match(startToken, "%name%|<|>|>>|,"))
            startToken = startToken->previous();
        if (!Token::Match(startToken, "[;{}] template <"))
            continue;

        // alias parameters..
        std::vector<const Token *> aliasParameters;
        TemplateSimplifier::getTemplateParametersInDeclaration(startToken->tokAt(3), aliasParameters);
        std::map<std::string, unsigned int> aliasParameterNames;
        for (unsigned int argnr = 0; argnr < aliasParameters.size(); ++argnr)
            aliasParameterNames[aliasParameters[argnr]->str()] = argnr;

        // Look for alias usages..
        const Token *endToken = nullptr;
        for (it2 = it1; it2 != templateInstantiations->end(); ++it2) {
            TemplateSimplifier::TokenAndName &aliasUsage = *it2;
            if (aliasUsage.name != aliasName)
                continue;
            std::vector<std::pair<Token *, Token *>> args;
            Token *tok2 = aliasUsage.token->tokAt(2);
            while (tok2) {
                Token * const start = tok2;
                while (tok2 && !Token::Match(tok2, "[,>;{}]")) {
                    if (tok2->link() && Token::Match(tok2, "(|<|["))
                        tok2 = tok2->link();
                    tok2 = tok2->next();
                }

                args.emplace_back(start, tok2);
                if (tok2 && tok2->str() == ",") {
                    tok2 = tok2->next();
                } else {
                    break;
                }
            }
            if (!tok2 || tok2->str() != ">" || args.size() != aliasParameters.size())
                continue;

            // Replace template alias code..
            aliasUsage.name = templateAlias.name;
            if (aliasUsage.name.find(' ') == std::string::npos) {
                aliasUsage.token->str(templateAlias.token->str());
            } else {
                tok2 = TokenList::copyTokens(aliasUsage.token, aliasToken1, templateAlias.token, true);
                aliasUsage.token->deleteThis();
                aliasUsage.token = tok2;
            }
            tok2 = aliasUsage.token->next(); // the '<'
            const Token * const endToken1 = templateAlias.token->next()->findClosingBracket();
            Token * const endToken2 = TokenList::copyTokens(tok2, templateAlias.token->tokAt(2), endToken1->previous(), false);
            for (const Token *tok1 = templateAlias.token->next(); tok2 != endToken2; tok1 = tok1->next(), tok2 = tok2->next()) {
                if (!tok2->isName())
                    continue;
                if (aliasParameterNames.find(tok2->str()) == aliasParameterNames.end()) {
                    // Create template instance..
                    if (Token::Match(tok1, "%name% <")) {
                        const std::list<TokenAndName>::iterator it = std::find_if(templateInstantiations->begin(),
                                templateInstantiations->end(),
                                FindToken(tok1));
                        if (it != templateInstantiations->end())
                            templateInstantiations->emplace_back(tok2, it->scope, it->name);
                    }
                    continue;
                }
                const unsigned int argnr = aliasParameterNames[tok2->str()];
                const Token * const fromStart = args[argnr].first;
                const Token * const fromEnd   = args[argnr].second->previous();
                Token * const destToken = tok2;
                tok2 = TokenList::copyTokens(tok2, fromStart, fromEnd, true);
                if (tok2 == destToken->next())
                    tok2 = destToken;
                destToken->deleteThis();
            }

            endToken = endToken1->next();

            // Remove alias usage code (parameters)
            Token::eraseTokens(tok2, args.back().second);
        }
        if (endToken) {
            // Remove all template instantiations in template alias
            for (const Token *tok = startToken; tok != endToken; tok = tok->next()) {
                if (!Token::Match(tok, "%name% <"))
                    continue;
                std::list<TokenAndName>::iterator it = std::find_if(templateInstantiations->begin(),
                                                       templateInstantiations->end(),
                                                       FindToken(tok));
                if (it == templateInstantiations->end())
                    continue;
                std::list<TokenAndName>::iterator next = it;
                ++next;
                if (it == it1)
                    it1 = next;
                templateInstantiations->erase(it,next);
            }

            Token::eraseTokens(startToken, endToken);
        }
    }
}

bool TemplateSimplifier::instantiateMatch(const Token *instance, const std::size_t numberOfArguments, const char patternAfter[])
{
//    if (!Token::simpleMatch(instance, (name + " <").c_str()))
//        return false;

    if (numberOfArguments != TemplateSimplifier::templateParameters(instance->next()))
        return false;

    if (patternAfter) {
        const Token *tok = instance;
        unsigned int indentlevel = 0;
        for (tok = instance; tok && (tok->str() != ">" || indentlevel > 0); tok = tok->next()) {
            if (Token::Match(tok, "<|,|(|:: %name% <") && templateParameters(tok->tokAt(2)) > 0)
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

// Utility function for TemplateSimplifier::getTemplateNamePosition, that works on template member functions,
// hence this pattern: "> %type% [%type%] < ... > :: %type% ("
static bool getTemplateNamePositionTemplateMember(const Token *tok, int &namepos)
{
    if (!Token::Match(tok, "> %type% <") && !Token::Match(tok, "> %type% %type% <"))
        return false;

    int currPos = 0;
    currPos = 2 + (Token::Match(tok, "> %type% %type%"));

    // Find the end of the template argument list
    const Token *templateParmEnd = tok->linkAt(currPos);
    if (!templateParmEnd)
        templateParmEnd = tok->tokAt(currPos)->findClosingBracket();
    if (!templateParmEnd)
        return false;

    if (Token::Match(templateParmEnd->next(), ":: %type% (")) {
        // We have a match, and currPos points at the template list opening '<'. Move it to the closing '>'
        for (const Token *tok2 = tok->tokAt(currPos) ; tok2 != templateParmEnd ; tok2 = tok2->next())
            ++currPos;
        namepos = currPos + 2;
        return true;
    }
    return false;
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
    else if (getTemplateNamePositionTemplateMember(tok, namepos))
        ;
    else if (Token::Match(tok, "> %type% *|&| %type% :: %type% (")) {
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
    const Token *templateDeclarationToken,
    const std::string &fullName,
    const std::vector<const Token *> &typeParametersInDeclaration,
    const std::string &newName,
    const std::vector<const Token *> &typesUsedInTemplateInstantiation,
    std::list<TemplateSimplifier::TokenAndName> &templateInstantiations)
{
    std::list<ScopeInfo2> scopeInfo;
    bool inTemplateDefinition = false;
    const Token *endOfTemplateDefinition = nullptr;
    const Token * const templateDeclarationNameToken = templateDeclarationToken->tokAt(getTemplateNamePosition(templateDeclarationToken));
    for (const Token *tok3 = tokenlist.front(); tok3; tok3 = tok3 ? tok3->next() : nullptr) {
        setScopeInfo(const_cast<Token *>(tok3), &scopeInfo);
        if (inTemplateDefinition) {
            if (!endOfTemplateDefinition && tok3->str() == "{")
                endOfTemplateDefinition = tok3->link();
            if (tok3 == endOfTemplateDefinition)
                inTemplateDefinition = false;
        }

        if (tok3->str()=="template") {
            if (tok3->next() && tok3->next()->str()=="<") {
                std::vector<const Token *> localTypeParametersInDeclaration;
                getTemplateParametersInDeclaration(tok3->tokAt(2), localTypeParametersInDeclaration);
                if (localTypeParametersInDeclaration.size() != typeParametersInDeclaration.size())
                    inTemplateDefinition = false; // Partial specialization
                else
                    inTemplateDefinition = true;
            } else {
                inTemplateDefinition = false; // Only template instantiation
            }
        }
        if (Token::Match(tok3, "(|["))
            tok3 = tok3->link();

        // Start of template..
        if (tok3 == templateDeclarationToken) {
            tok3 = tok3->next();
        }

        // member function implemented outside class definition
        else if (inTemplateDefinition &&
                 Token::Match(tok3, "%name% <") &&
                 fullName == getFullName(scopeInfo, tok3->str()) &&
                 TemplateSimplifier::instantiateMatch(tok3, typeParametersInDeclaration.size(), ":: ~| %name% (")) {
            // there must be template..
            bool istemplate = false;
            for (const Token *prev = tok3; prev && !Token::Match(prev, "[;{}]"); prev = prev->previous()) {
                if (prev->str() == "template") {
                    istemplate = true;
                    break;
                }
            }
            if (!istemplate)
                continue;

            const Token *tok4 = tok3->next()->findClosingBracket();
            while (tok4 && tok4->str() != "(")
                tok4 = tok4->next();
            if (!Tokenizer::isFunctionHead(tok4, ":{", true))
                continue;
            tokenlist.addtoken(newName, tok3->linenr(), tok3->fileIndex());
            while (tok3 && tok3->str() != "::")
                tok3 = tok3->next();
        }

        // not part of template.. go on to next token
        else
            continue;

        std::stack<Token *> brackets; // holds "(", "[" and "{" tokens

        // FIXME use full name matching somehow
        const std::string lastName = (fullName.find(' ') != std::string::npos) ? fullName.substr(fullName.rfind(' ')+1) : fullName;

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
                        tokenlist.back()->isTemplateArg(true);
                    }
                    continue;
                }
            }

            // replace name..
            if (tok3->str() == lastName) {
                if (Token::Match(tok3->tokAt(-2), "> :: %name% ( )")) {
                    ; // Ticket #7942: Replacing for out-of-line constructors generates invalid syntax
                } else if (!Token::simpleMatch(tok3->next(), "<")) {
                    tokenlist.addtoken(newName, tok3->linenr(), tok3->fileIndex());
                    continue;
                } else if (tok3 == templateDeclarationNameToken) {
                    tokenlist.addtoken(newName, tok3->linenr(), tok3->fileIndex());
                    tok3 = tok3->next()->findClosingBracket();
                    continue;
                }
            }

            // copy
            tokenlist.addtoken(tok3, tok3->linenr(), tok3->fileIndex());
            if (Token::Match(tok3, "%type% <") && Token::Match(tok3->next()->findClosingBracket(), ">|>>")) {
                const Token *closingBracket = tok3->next()->findClosingBracket();
                if (Token::simpleMatch(closingBracket->next(), "&")) {
                    int num = 0;
                    const Token *par = tok3->next();
                    while (num < typeParametersInDeclaration.size() && par != closingBracket) {
                        const std::string pattern("[<,] " + typeParametersInDeclaration[num]->str() + " [,>]");
                        if (!Token::Match(par, pattern.c_str()))
                            break;
                        ++num;
                        par = par->tokAt(2);
                    }
                    if (num < typeParametersInDeclaration.size() || par != closingBracket)
                        continue;
                }

                std::string name = tok3->str();
                for (const Token *prev = tok3->tokAt(-2); Token::Match(prev, "%name% ::"); prev = prev->tokAt(-2))
                    name = prev->str() + " :: " + name;
                templateInstantiations.emplace_back(tokenlist.back(), getScopeName(scopeInfo), getFullName(scopeInfo, name));
            }

            // link() newly tokens manually
            else if (tok3->str() == "{") {
                brackets.push(tokenlist.back());
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
                if (brackets.empty()) {
                    inTemplateDefinition = false;
                    break;
                }
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
            const bool op1 = !MathLib::isNullValue(tok->str());
            const bool op2 = !MathLib::isNullValue(tok->strAt(2));
            const bool result = (op->str() == "||") ? (op1 || op2) : (op1 && op2);
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
                const bool andAnd = (tok->next()->str() == "&&");
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
                        tok = tok->tokAt(-4);
                        tok->deleteNext(5);
                    } else {
                        tok = tok->previous();
                        tok->deleteNext(2);
                    }
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=([,] 0 [+|]") ||
                           Token::Match(tok->previous(), "return|case 0 [+|]")) {
                    tok = tok->previous();
                    tok->deleteNext(2);
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 0 * %name%|%num% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "[=[(,] 0 * (") ||
                           Token::Match(tok->previous(), "return|case 0 *|&& %name%|%num% ,|:|;|=|%cop%") ||
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

            if (Token::Match(tok->tokAt(-2), "%any% * 1") || Token::Match(tok->previous(), "%any% 1 *")) {
                tok = tok->previous();
                if (tok->str() == "*")
                    tok = tok->previous();
                tok->deleteNext(2);
                ret = true;
            }

            // Remove parentheses around number..
            if (Token::Match(tok->tokAt(-2), "%op%|< ( %num% )") && tok->strAt(-2) != ">") {
                tok = tok->previous();
                tok->deleteThis();
                tok->deleteNext();
                ret = true;
            }

            if (Token::Match(tok->previous(), "( 0 [|+]") ||
                Token::Match(tok->previous(), "[|+-] 0 )")) {
                tok = tok->previous();
                if (Token::Match(tok, "[|+-]"))
                    tok = tok->previous();
                tok->deleteNext(2);
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

const Token * TemplateSimplifier::getTemplateParametersInDeclaration(
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

static bool matchSpecialization(const Token *templateDeclarationNameToken, const Token *templateInstantiationNameToken, const std::list<const Token *> & specializations)
{
    // Is there a matching specialization?
    for (std::list<const Token *>::const_iterator it = specializations.begin(); it != specializations.end(); ++it) {
        if (!Token::Match(*it, "%name% <"))
            continue;
        const Token *startToken = (*it);
        while (startToken->previous() && !Token::Match(startToken->previous(), "[;{}]"))
            startToken = startToken->previous();
        if (!Token::simpleMatch(startToken, "template <"))
            continue;
        std::vector<const Token *> templateParameters;
        TemplateSimplifier::getTemplateParametersInDeclaration(startToken->tokAt(2), templateParameters);

        const Token *instToken = templateInstantiationNameToken->tokAt(2);
        const Token *declToken = (*it)->tokAt(2);
        const Token * const endToken = (*it)->next()->findClosingBracket();
        while (declToken != endToken) {
            if (declToken->str() != instToken->str()) {
                int nr = 0;
                while (nr < templateParameters.size() && templateParameters[nr]->str() != declToken->str())
                    ++nr;

                if (nr == templateParameters.size())
                    break;
            }
            declToken = declToken->next();
            instToken = instToken->next();
        }

        if (declToken == endToken && instToken->str() == ">") {
            // specialization matches.
            return templateDeclarationNameToken == *it;
        }
    }

    // No specialization matches. Return true if the declaration is not a specialization.
    return Token::Match(templateDeclarationNameToken, "%name% !!<");
}

bool TemplateSimplifier::simplifyTemplateInstantiations(
    TokenList& tokenlist,
    ErrorLogger* errorlogger,
    const Settings *_settings,
    const TokenAndName &templateDeclaration,
    const std::list<const Token *> &specializations,
    const std::time_t maxtime,
    std::list<TokenAndName> &templateInstantiations,
    std::set<std::string> &expandedtemplates)
{
    // this variable is not used at the moment. The intention was to
    // allow continuous instantiations until all templates has been expanded
    //bool done = false;

    // Contains tokens such as "T"
    std::vector<const Token *> typeParametersInDeclaration;
    const Token * const tok = getTemplateParametersInDeclaration(templateDeclaration.token->tokAt(2), typeParametersInDeclaration);

    // bail out if the end of the file was reached
    if (!tok)
        return false;

    const bool printDebug = _settings->debugwarnings;

    // get the position of the template name
    const int namepos = TemplateSimplifier::getTemplateNamePosition(tok);
    if (namepos == -1) {
        // debug message that we bail out..
        if (printDebug && errorlogger) {
            const std::list<const Token *> callstack(1, tok);
            errorlogger->reportErr(ErrorLogger::ErrorMessage(callstack, &tokenlist, Severity::debug, "debug", "simplifyTemplates: bailing out", false));
        }
        return false;
    }

    const bool isfunc(tok->strAt(namepos + 1) == "(");

    // locate template usage..
    std::string::size_type numberOfTemplateInstantiations = templateInstantiations.size();
    unsigned int recursiveCount = 0;

    bool instantiated = false;

    for (std::list<TokenAndName>::const_iterator iter2 = templateInstantiations.begin(); iter2 != templateInstantiations.end(); ++iter2) {
        if (numberOfTemplateInstantiations != templateInstantiations.size()) {
            numberOfTemplateInstantiations = templateInstantiations.size();
            simplifyCalculations(tokenlist.front());
            ++recursiveCount;
            if (recursiveCount > 100) {
                // bail out..
                break;
            }
        }

        // already simplified
        if (!Token::Match(iter2->token, "%name% <"))
            continue;

        if (iter2->name != templateDeclaration.name)
            continue;

        if (!matchSpecialization(tok->tokAt(namepos), iter2->token, specializations))
            continue;

        Token * const tok2 = iter2->token;
        if (errorlogger && !tokenlist.getFiles().empty())
            errorlogger->reportProgress(tokenlist.getFiles()[0], "TemplateSimplifier::simplifyTemplateInstantiations()", tok2->progressValue());
#ifdef MAXTIME
        if (std::time(0) > maxtime)
            return false;
#else
        (void)maxtime;
#endif
        assert(tokenlist.validateToken(tok2)); // that assertion fails on examples from #6021

        const Token *startToken = tok2;
        while (Token::Match(startToken->tokAt(-2), "%name% :: %name%"))
            startToken = startToken->tokAt(-2);

        if (Token::Match(startToken->previous(), "[;{}=]") &&
            !TemplateSimplifier::instantiateMatch(tok2, typeParametersInDeclaration.size(), isfunc ? "(" : "*| %name%"))
            continue;

        // New type..
        std::vector<const Token *> typesUsedInTemplateInstantiation;
        std::string typeForNewName;
        std::list<std::string> typeStringsUsedInTemplateInstantiation;
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
                typeStringsUsedInTemplateInstantiation.push_back(tok3->str());
            }
            // add additional type information
            if (!constconst && !Token::Match(tok3, "class|struct|enum")) {
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

        if (typeForNewName.empty() || typeParametersInDeclaration.size() != typesUsedInTemplateInstantiation.size()) {
            if (printDebug && errorlogger) {
                std::list<const Token *> callstack(1, tok2);
                errorlogger->reportErr(ErrorLogger::ErrorMessage(callstack, &tokenlist, Severity::debug, "debug",
                                       "Failed to instantiate template \"" + iter2->name + "\". The checking continues anyway.", false));
            }
            if (typeForNewName.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string newName(templateDeclaration.name + " < " + typeForNewName + " >");

        if (expandedtemplates.find(newName) == expandedtemplates.end()) {
            expandedtemplates.insert(newName);
            TemplateSimplifier::expandTemplate(tokenlist, tok, iter2->name, typeParametersInDeclaration, newName, typesUsedInTemplateInstantiation, templateInstantiations);
            instantiated = true;
        }

        // Replace all these template usages..
        replaceTemplateUsage(tok2, iter2->name, typeStringsUsedInTemplateInstantiation, newName, typesUsedInTemplateInstantiation, templateInstantiations);
    }

    // Template has been instantiated .. then remove the template declaration
    return instantiated;
}

static bool matchTemplateParameters(const Token *nameTok, const std::list<std::string> &strings)
{
    std::list<std::string>::const_iterator it = strings.begin();
    const Token *tok = nameTok->tokAt(2);
    while (tok && it != strings.end() && *it == tok->str()) {
        tok = tok->next();
        ++it;
    }
    return it == strings.end() && tok && tok->str() == ">";
}

void TemplateSimplifier::replaceTemplateUsage(Token * const instantiationToken,
        const std::string &templateName,
        const std::list<std::string> &typeStringsUsedInTemplateInstantiation,
        const std::string &newName,
        const std::vector<const Token *> &typesUsedInTemplateInstantiation,
        std::list<TokenAndName> &templateInstantiations)
{
    std::list<ScopeInfo2> scopeInfo;
    std::list< std::pair<Token *, Token *> > removeTokens;
    for (Token *nameTok = instantiationToken; nameTok; nameTok = nameTok->next()) {
        setScopeInfo(nameTok, &scopeInfo);
        if (!Token::Match(nameTok, "%name% <"))
            continue;
        if (!matchTemplateParameters(nameTok, typeStringsUsedInTemplateInstantiation))
            continue;

        // FIXME Proper name matching
        const std::string lastName(templateName.find(' ') == std::string::npos ? templateName : templateName.substr(templateName.rfind(' ') + 1));
        if (lastName != nameTok->str())
            continue;

        // match parameters
        Token * tok2 = nameTok->tokAt(2);
        unsigned int typeCountInInstantiation = 1U; // There is always at least one type
        const Token *typetok = (!typesUsedInTemplateInstantiation.empty()) ? typesUsedInTemplateInstantiation[0] : nullptr;
        unsigned int indentlevel2 = 0;  // indentlevel for tokgt
        while (tok2 && (indentlevel2 > 0 || tok2->str() != ">")) {
            if (tok2->str() == "<" && templateParameters(tok2) > 0)
                ++indentlevel2;
            else if (indentlevel2 > 0 && Token::Match(tok2, "> [,>]"))
                --indentlevel2;
            else if (indentlevel2 == 0) {
                if (tok2->str() != ",") {
                    if (!typetok ||
                        tok2->isUnsigned() != typetok->isUnsigned() ||
                        tok2->isSigned() != typetok->isSigned() ||
                        tok2->isLong() != typetok->isLong()) {
                        break;
                    }

                    typetok = typetok->next();
                } else {
                    if (typeCountInInstantiation < typesUsedInTemplateInstantiation.size())
                        typetok = typesUsedInTemplateInstantiation[typeCountInInstantiation++];
                    else
                        typetok = nullptr;
                }
            }
            tok2 = tok2->next();
        }

        if (!tok2)
            break;

        // matching template usage => replace tokens..
        // Foo < int >  =>  Foo<int>
        if (tok2->str() == ">" && typeCountInInstantiation == typesUsedInTemplateInstantiation.size()) {
            Token * const nameTok1 = nameTok;
            while (Token::Match(nameTok->tokAt(-2), "%name% :: %name%"))
                nameTok = nameTok->tokAt(-2);
            nameTok->str(newName);
            for (std::list<TokenAndName>::iterator it = templateInstantiations.begin(); it != templateInstantiations.end(); ++it) {
                if (it->token == nameTok1)
                    it->token = nameTok;
            }
            for (Token *tok = nameTok1->next(); tok != tok2; tok = tok->next()) {
                if (tok->isName()) {
                    std::list<TokenAndName>::iterator ti;
                    for (ti = templateInstantiations.begin(); ti != templateInstantiations.end();) {
                        if (ti->token == tok)
                            templateInstantiations.erase(ti++);
                        else
                            ++ti;
                    }
                }
            }
            removeTokens.emplace_back(nameTok, tok2->next());
        }

        nameTok = tok2;
    }
    while (!removeTokens.empty()) {
        Token::eraseTokens(removeTokens.back().first, removeTokens.back().second);
        removeTokens.pop_back();
    }
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

    std::list<TokenAndName> templateDeclarations(TemplateSimplifier::getTemplateDeclarations(tokenlist.front(), _codeWithTemplates));

    // Locate possible instantiations of templates..
    std::list<TokenAndName> templateInstantiations(TemplateSimplifier::getTemplateInstantiations(tokenlist.front(), templateDeclarations));

    // No template instantiations? Then return.
    if (templateInstantiations.empty())
        return;

    // Template arguments with default values
    TemplateSimplifier::useDefaultArgumentValues(templateDeclarations, &templateInstantiations);

    TemplateSimplifier::simplifyTemplateAliases(&templateInstantiations);

    // expand templates
    //bool done = false;
    //while (!done)
    {
        //done = true;
        std::list<TokenAndName> instantiatedTemplates;
        for (std::list<TokenAndName>::reverse_iterator iter1 = templateDeclarations.rbegin(); iter1 != templateDeclarations.rend(); ++iter1) {
            // get specializations..
            std::list<const Token *> specializations;
            for (std::list<TokenAndName>::const_iterator iter2 = templateDeclarations.begin(); iter2 != templateDeclarations.end(); ++iter2) {
                if (iter1->name == iter2->name) {
                    const Token *tok = iter2->token->next()->findClosingBracket();
                    int namepos = getTemplateNamePosition(tok);
                    if (namepos > 0)
                        specializations.push_back(tok->tokAt(namepos));
                }
            }

            bool instantiated = TemplateSimplifier::simplifyTemplateInstantiations(tokenlist,
                                errorlogger,
                                _settings,
                                *iter1,
                                specializations,
                                maxtime,
                                templateInstantiations,
                                expandedtemplates);
            if (instantiated)
                instantiatedTemplates.push_back(*iter1);
        }

        for (std::list<TokenAndName>::const_iterator it = instantiatedTemplates.begin(); it != instantiatedTemplates.end(); ++it) {
            std::list<TokenAndName>::iterator it1;
            for (it1 = templateDeclarations.begin(); it1 != templateDeclarations.end(); ++it1) {
                if (it1->token == it->token)
                    break;
            }
            if (it1 != templateDeclarations.end()) {
                templateDeclarations.erase(it1);
                removeTemplate(it->token);
            }
        }
    }
}

void TemplateSimplifier::syntaxError(const Token *tok)
{
    throw InternalError(tok, "syntax error", InternalError::SYNTAX);
}
