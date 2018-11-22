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
        explicit FindToken(const Token *token) : mToken(token) {}
        bool operator()(const TemplateSimplifier::TokenAndName &tokenAndName) const {
            return tokenAndName.token == mToken;
        }
    private:
        const Token * const mToken;
    };

    class FindName {
    public:
        explicit FindName(const std::string &name) : mName(name) {}
        bool operator()(const TemplateSimplifier::TokenAndName &tokenAndName) const {
            return tokenAndName.name == mName;
        }
    private:
        const std::string mName;
    };
}

TemplateSimplifier::TokenAndName::TokenAndName(Token *tok, const std::string &s, const std::string &n, const Token *nt) :
    token(tok), scope(s), name(n), nameToken(nt)
{
    token->hasTemplateSimplifierPointer(true);
}

TemplateSimplifier::TemplateSimplifier(TokenList &tokenlist, const Settings *settings, ErrorLogger *errorLogger)
    : mTokenList(tokenlist), mSettings(settings), mErrorLogger(errorLogger)
{
}

TemplateSimplifier::~TemplateSimplifier()
{
}

void TemplateSimplifier::cleanupAfterSimplify()
{
    bool goback = false;
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
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
                if (tok == mTokenList.front())
                    goback = true;
            }
        }
    }
}


void TemplateSimplifier::checkComplicatedSyntaxErrorsInTemplates()
{
    // check for more complicated syntax errors when using templates..
    for (const Token *tok = mTokenList.front(); tok; tok = tok->next()) {
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
            for (const Token *tok2 = tok; tok2 && !Token::simpleMatch(tok2, ";"); tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, "{") && (!Token::Match(tok2->previous(), ">|%type%") || Token::simpleMatch(tok2->link(), "} ;")))
                    break;
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

void TemplateSimplifier::eraseTokens(Token *begin, const Token *end)
{
    if (!begin || begin == end)
        return;

    while (begin->next() && begin->next() != end) {
        // check if token has a pointer to it
        if (begin->next()->hasTemplateSimplifierPointer()) {
            // check if the token is in a list
            const std::list<TokenAndName>::iterator it = std::find_if(mTemplateInstantiations.begin(),
                    mTemplateInstantiations.end(),
                    FindToken(begin->next()));
            if (it != mTemplateInstantiations.end()) {
                // remove the pointer flag and prevent the deleted pointer from being used
                it->token->hasTemplateSimplifierPointer(false);
                it->token = nullptr;
            }
            const std::list<TokenAndName>::iterator it2 = std::find_if(mTemplateDeclarations.begin(),
                    mTemplateDeclarations.end(),
                    FindToken(begin->next()));
            if (it2 != mTemplateDeclarations.end()) {
                // remove the pointer flag and prevent the deleted pointer from being used
                it2->token->hasTemplateSimplifierPointer(false);
                it2->token = nullptr;
            }
            for (size_t i = 0; i < mTypesUsedInTemplateInstantiation.size(); ++i) {
                if (mTypesUsedInTemplateInstantiation[i] == begin->next()) {
                    mTypesUsedInTemplateInstantiation[i]->hasTemplateSimplifierPointer(false);
                    mTypesUsedInTemplateInstantiation[i] = nullptr;
                }
            }
        }
        begin->deleteNext();
    }
}

void TemplateSimplifier::deleteToken(Token *tok)
{
    if (tok->next())
        tok->next()->deletePrevious();
    else
        tok->deleteThis();
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
            eraseTokens(tok,tok2);
            deleteToken(tok);
            return false;
        }

        else if (tok2->str() == "{") {
            tok2 = tok2->link()->next();
            if (tok2 && tok2->str() == ";" && tok2->next())
                tok2 = tok2->next();
            eraseTokens(tok, tok2);
            deleteToken(tok);
            return true;
        } else if (tok2->str() == "}") {  // garbage code! (#3449)
            eraseTokens(tok,tok2);
            deleteToken(tok);
            return false;
        }

        // Count ">"
        if (tok2->str() == ">")
            countgt++;

        // don't remove constructor
        if (tok2->str() == "explicit" ||
            (countgt == 1 && Token::Match(tok2->previous(), "> %type% (") &&
             Tokenizer::startOfExecutableScope(tok2->linkAt(1)))) {
            eraseTokens(tok, tok2);
            deleteToken(tok);
            return true;
        }

        if (tok2->str() == ";") {
            tok2 = tok2->next();
            eraseTokens(tok, tok2);
            deleteToken(tok);
            return true;
        }

        if (tok2->str() == "<")
            ++indentlevel;

        else if (indentlevel >= 2 && tok2->str() == ">")
            --indentlevel;

        else if (Token::Match(tok2, "> class|struct|union %name% [,)]")) {
            tok2 = tok2->next();
            eraseTokens(tok, tok2);
            deleteToken(tok);
            return true;
        }
    }

    return false;
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
    for (const ScopeInfo2 &i : scopeInfo)
        ret += (ret.empty() ? "" : " :: ") + i.name;
    return ret;
}

static void setScopeInfo(const Token *tok, std::list<ScopeInfo2> *scopeInfo)
{
    while (tok->str() == "}" && !scopeInfo->empty() && tok == scopeInfo->back().bodyEnd)
        scopeInfo->pop_back();
    if (!Token::Match(tok, "namespace|class|struct|union %name% {|:|::"))
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

bool TemplateSimplifier::getTemplateDeclarations()
{
    bool codeWithTemplates = false;
    std::list<ScopeInfo2> scopeInfo;
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "}|namespace|class|struct|union")) {
            setScopeInfo(tok, &scopeInfo);
            continue;
        }
        if (!Token::simpleMatch(tok, "template <"))
            continue;
        // Some syntax checks, see #6865
        if (!tok->tokAt(2))
            syntaxError(tok->next());
        if (tok->strAt(2)=="typename" &&
            !Token::Match(tok->tokAt(3), "%name%|.|,|=|>"))
            syntaxError(tok->next());
        codeWithTemplates = true;
        const Token * const parmEnd = tok->next()->findClosingBracket();
        for (const Token *tok2 = parmEnd; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ")")
                break;
            // Declaration => add to mTemplateForwardDeclarations
            else if (tok2->str() == ";") {
                const int namepos = getTemplateNamePosition(parmEnd, true);
                if (namepos > 0)
                    mTemplateForwardDeclarations.emplace_back(tok, getScopeName(scopeInfo), parmEnd->strAt(namepos), parmEnd->tokAt(namepos));
                break;
            }
            // Implementation => add to mTemplateDeclarations
            else if (tok2->str() == "{") {
                const int namepos = getTemplateNamePosition(parmEnd, false);
                if (namepos > 0)
                    mTemplateDeclarations.emplace_back(tok, getScopeName(scopeInfo), parmEnd->strAt(namepos), parmEnd->tokAt(namepos));
                break;
            }
        }
    }
    return codeWithTemplates;
}


void TemplateSimplifier::getTemplateInstantiations()
{
    std::list<ScopeInfo2> scopeList;

    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "}|namespace|class|struct|union")) {
            setScopeInfo(tok, &scopeList);
            continue;
        }
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
        } else if (Token::Match(tok->previous(), "(|{|}|;|=|>|<<|:|. %name% ::|<") ||
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
                    templateParameters(tok2->tokAt(2))) {
                    mTemplateInstantiations.emplace_back(tok2->next(), getScopeName(scopeList), tok2->strAt(1), tok2->tokAt(1));
                } else if (Token::Match(tok2->next(), "class|struct"))
                    const_cast<Token *>(tok2)->deleteNext();

            }

            // Add outer template..
            if (templateParameters(tok->next())) {
                const std::string scopeName1(scopeName);
                while (true) {
                    const std::string fullName = scopeName + (scopeName.empty()?"":" :: ") + tok->str();
                    const std::list<TokenAndName>::const_iterator it = std::find_if(mTemplateDeclarations.begin(), mTemplateDeclarations.end(), FindName(fullName));
                    if (it != mTemplateDeclarations.end()) {
                        mTemplateInstantiations.emplace_back(tok, getScopeName(scopeList), fullName, tok);
                        break;
                    } else {
                        if (scopeName.empty()) {
                            mTemplateInstantiations.emplace_back(tok, getScopeName(scopeList), scopeName1 + (scopeName1.empty()?"":" :: ") + tok->str(), tok);
                            break;
                        }
                        const std::string::size_type pos = scopeName.rfind(" :: ");
                        scopeName = (pos == std::string::npos) ? std::string() : scopeName.substr(0,pos);
                    }
                }
            }
        }
    }
}


void TemplateSimplifier::useDefaultArgumentValues()
{
    for (const TokenAndName &template1 : mTemplateDeclarations) {
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
        for (Token *tok = template1.token; tok; tok = tok->next()) {
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
                if (Token::Match(tok, "> class|struct|union %name%"))
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
        for (const TokenAndName &templateInst : mTemplateInstantiations) {
            Token *tok = templateInst.token;

            if (!Token::simpleMatch(tok, (classname + " <").c_str()))
                continue;

            // count the parameters..
            tok = tok->next();
            const unsigned int usedpar = templateParameters(tok);
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

        for (Token * const eqtok : eq) {
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
                    std::list<TokenAndName>::iterator ti = std::find_if(mTemplateInstantiations.begin(),
                                                           mTemplateInstantiations.end(),
                                                           FindToken(tok2));
                    if (ti != mTemplateInstantiations.end())
                        mTemplateInstantiations.erase(ti);
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

            // don't strip args from uninstantiated templates
            std::list<TokenAndName>::iterator ti2 = std::find_if(mTemplateInstantiations.begin(),
                                                    mTemplateInstantiations.end(),
                                                    FindName(template1.name));

            if (ti2 == mTemplateInstantiations.end())
                continue;

            eraseTokens(eqtok, tok2);
            eqtok->deleteThis();
        }
    }
}

void TemplateSimplifier::simplifyTemplateAliases()
{
    std::list<TokenAndName>::iterator it1, it2;
    for (it1 = mTemplateInstantiations.begin(); it1 != mTemplateInstantiations.end();) {
        TokenAndName &templateAlias = *it1;
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
        getTemplateParametersInDeclaration(startToken->tokAt(3), aliasParameters);
        std::map<std::string, unsigned int> aliasParameterNames;
        for (unsigned int argnr = 0; argnr < aliasParameters.size(); ++argnr)
            aliasParameterNames[aliasParameters[argnr]->str()] = argnr;

        // Look for alias usages..
        const Token *endToken = nullptr;
        for (it2 = it1; it2 != mTemplateInstantiations.end(); ++it2) {
            TokenAndName &aliasUsage = *it2;
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
                const Token *temp = aliasToken1;
                while (temp && temp != templateAlias.token) {
                    aliasUsage.token->insertToken(temp->str(), "", true);
                    temp = temp->next();
                }
                aliasUsage.token->str(templateAlias.token->str());
            } else {
                tok2 = TokenList::copyTokens(aliasUsage.token, aliasToken1, templateAlias.token, true);
                deleteToken(aliasUsage.token);
                aliasUsage.token = tok2;
            }
            tok2 = aliasUsage.token->next(); // the '<'
            const Token * const endToken1 = templateAlias.token->next()->findClosingBracket();
            const Token * const endToken2 = TokenList::copyTokens(tok2, templateAlias.token->tokAt(2), endToken1->previous(), false);
            for (const Token *tok1 = templateAlias.token->next(); tok2 != endToken2; tok1 = tok1->next(), tok2 = tok2->next()) {
                if (!tok2->isName())
                    continue;
                if (aliasParameterNames.find(tok2->str()) == aliasParameterNames.end()) {
                    // Create template instance..
                    if (Token::Match(tok1, "%name% <")) {
                        const std::list<TokenAndName>::iterator it = std::find_if(mTemplateInstantiations.begin(),
                                mTemplateInstantiations.end(),
                                FindToken(tok1));
                        if (it != mTemplateInstantiations.end())
                            mTemplateInstantiations.emplace_back(tok2, it->scope, it->name, it->nameToken);
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
                std::list<TokenAndName>::iterator it = std::find_if(mTemplateInstantiations.begin(),
                                                       mTemplateInstantiations.end(),
                                                       FindToken(tok));
                if (it == mTemplateInstantiations.end())
                    continue;
                std::list<TokenAndName>::iterator next = it;
                ++next;
                if (it == it1)
                    it1 = next;
                mTemplateInstantiations.erase(it,next);
            }

            Token::eraseTokens(startToken, endToken);
        }
    }
}

bool TemplateSimplifier::instantiateMatch(const Token *instance, const std::size_t numberOfArguments, const char patternAfter[])
{
//    if (!Token::simpleMatch(instance, (name + " <").c_str()))
//        return false;

    if (numberOfArguments != templateParameters(instance->next()))
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
    namepos = 2;
    while (tok && tok->next()) {
        if (Token::Match(tok->next(), ";|{"))
            return false;
        else if (Token::Match(tok->next(), "%type% <")) {
            const Token *closing = tok->tokAt(2)->findClosingBracket();
            if (closing && Token::Match(closing->next(), ":: ~| %name% (")) {
                if (closing->strAt(1) == "~")
                    closing = closing->next();
                while (tok && tok->next() != closing->next()) {
                    tok = tok->next();
                    namepos++;
                }
                return true;
            }
        }
        tok = tok->next();
        namepos++;
    }
    return false;
}

int TemplateSimplifier::getTemplateNamePosition(const Token *tok, bool forward)
{
    // get the position of the template name
    int namepos = 0, starAmpPossiblePosition = 0;
    if ((forward && Token::Match(tok, "> class|struct|union %type% :|<|;")) ||
        (!forward && Token::Match(tok, "> class|struct|union %type% {|:|<")))
        namepos = 2;
    else if (Token::Match(tok, "> %type% *|&| %type% ("))
        namepos = 2;
    else if (Token::Match(tok, "> %type% %type% <") &&
             Token::simpleMatch(tok->tokAt(3)->findClosingBracket(), "> ("))
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

void TemplateSimplifier::addNamespace(const TokenAndName &templateDeclaration, const Token *tok)
{
    // find start of qualification
    const Token * tokStart = tok;
    int offset = 0;
    while (Token::Match(tokStart->tokAt(-2), "%name% ::")) {
        tokStart = tokStart->tokAt(-2);
        offset -= 2;
    }
    // decide if namespace needs to be inserted in or appended to token list
    const bool insert = tokStart != tok;

    std::string::size_type start = 0;
    std::string::size_type end = 0;
    while ((end = templateDeclaration.scope.find(" ", start)) != std::string::npos) {
        std::string token = templateDeclaration.scope.substr(start, end - start);
        if (insert)
            mTokenList.back()->tokAt(offset)->insertToken(token, "");
        else
            mTokenList.addtoken(token, tok->linenr(), tok->fileIndex());
        start = end + 1;
    }
    if (insert) {
        mTokenList.back()->tokAt(offset)->insertToken(templateDeclaration.scope.substr(start), "");
        mTokenList.back()->tokAt(offset)->insertToken("::", "");
    } else {
        mTokenList.addtoken(templateDeclaration.scope.substr(start), tok->linenr(), tok->fileIndex());
        mTokenList.addtoken("::", tok->linenr(), tok->fileIndex());
    }
}

bool TemplateSimplifier::alreadyHasNamespace(const TokenAndName &templateDeclaration, const Token *tok) const
{
    std::string scope = templateDeclaration.scope;

    // get the length in tokens of the namespace
    std::string::size_type pos = 0;
    int offset = -2;

    while ((pos = scope.find("::", pos)) != std::string::npos) {
        offset -= 2;
        pos += 2;
    }

    return Token::simpleMatch(tok->tokAt(offset), scope.c_str()) ;
}

void TemplateSimplifier::expandTemplate(
    const TokenAndName &templateDeclaration,
    const Token *templateDeclarationToken,
    const TokenAndName &templateInstantiation,
    const std::vector<const Token *> &typeParametersInDeclaration,
    const std::string &newName,
    bool copy)
{
    std::list<ScopeInfo2> scopeInfo;
    bool inTemplateDefinition = false;
    const Token *startOfTemplateDeclaration = nullptr;
    const Token *endOfTemplateDefinition = nullptr;
    const Token * const templateDeclarationNameToken = templateDeclarationToken->tokAt(getTemplateNamePosition(templateDeclarationToken));
    const bool isClass = Token::Match(templateDeclarationToken->next(), "class|struct|union %name% <|{|:");
    const bool isFunction = templateDeclarationNameToken->strAt(1) == "(";

    // add forward declarations
    if (copy && isClass) {
        templateDeclaration.token->insertToken(templateDeclarationToken->strAt(1), "", true);
        templateDeclaration.token->insertToken(newName, "", true);
        templateDeclaration.token->insertToken(";", "", true);
    } else if (copy && isFunction) {
        // check if this is an explicit instantiation
        Token * temp = templateInstantiation.token;
        while (temp && !Token::Match(temp->previous(), "}|;"))
            temp = temp->previous();
        if (Token::Match(temp, "template !!<")) {
            // just delete "template"
            deleteToken(temp);
        } else {
            // add forward declaration
            Token * dst = templateDeclaration.token;
            Token * start;
            Token * end;
            auto it = mTemplateForwardDeclarationsMap.find(dst);
            if (it != mTemplateForwardDeclarationsMap.end()) {
                dst = it->second;
                const Token * temp1 = dst->tokAt(1)->findClosingBracket();
                const Token * temp2 = temp1->tokAt(getTemplateNamePosition(temp1));
                start = temp1->next();
                end = temp2->linkAt(1)->next();
            } else {
                start = templateDeclarationToken->next();
                end = templateDeclarationNameToken->linkAt(1)->next();
            }

            std::map<const Token *, Token *> links;
            while (start && start != end) {
                unsigned int itype = 0;
                while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != start->str())
                    ++itype;

                if (itype < typeParametersInDeclaration.size()) {
                    dst->insertToken(mTypesUsedInTemplateInstantiation[itype]->str(), "", true);
                    dst->previous()->isTemplateArg(true);
                } else {
                    if (start->str() == templateDeclarationNameToken->str())
                        dst->insertToken(newName, "", true);
                    else
                        dst->insertToken(start->str(), "", true);
                    if (start->link()) {
                        if (Token::Match(start, "[|{|(")) {
                            links[start->link()] = dst->previous();
                        } else if (Token::Match(start, "]|}|)")) {
                            Token::createMutualLinks(links[start], dst->previous());
                            links.erase(start);
                        }
                    }
                }

                start = start->next();
            }
            dst->insertToken(";", "", true);
        }
    }

    for (Token *tok3 = mTokenList.front(); tok3; tok3 = tok3 ? tok3->next() : nullptr) {
        if (Token::Match(tok3, "}|namespace|class|struct|union")) {
            setScopeInfo(tok3, &scopeInfo);
            continue;
        }
        if (inTemplateDefinition) {
            if (!endOfTemplateDefinition && tok3->str() == "{")
                endOfTemplateDefinition = tok3->link();
            if (tok3 == endOfTemplateDefinition) {
                inTemplateDefinition = false;
                startOfTemplateDeclaration = nullptr;
            }
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
            startOfTemplateDeclaration = tok3;
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
                 templateInstantiation.name == tok3->str() &&
                 instantiateMatch(tok3, typeParametersInDeclaration.size(), ":: ~| %name% (")) {
            // there must be template..
            bool istemplate = false;
            Token * tok5 = nullptr; // start of function return type
            for (Token *prev = tok3; prev && !Token::Match(prev, "[;{}]"); prev = prev->previous()) {
                if (prev->str() == "template") {
                    istemplate = true;
                    tok5 = prev;
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
            // find function return type start
            tok5 = tok5->next()->findClosingBracket();
            if (tok5)
                tok5 = tok5->next();
            // copy return type
            while (tok5 && tok5 != tok3) {
                // replace name if found
                if (Token::Match(tok5, "%name% <") && tok5->str() == templateInstantiation.name) {
                    if (copy) {
                        if (!templateDeclaration.scope.empty() && tok5->strAt(-1) != "::")
                            addNamespace(templateDeclaration, tok5);
                        mTokenList.addtoken(newName, tok5->linenr(), tok5->fileIndex());
                        tok5 = tok5->next()->findClosingBracket();
                    } else {
                        tok5->str(newName);
                        eraseTokens(tok5, tok5->next()->findClosingBracket()->next());
                    }
                } else if (copy)
                    mTokenList.addtoken(tok5, tok5->linenr(), tok5->fileIndex());

                tok5 = tok5->next();
            }
            if (copy) {
                if (!templateDeclaration.scope.empty() && tok3->strAt(-1) != "::")
                    addNamespace(templateDeclaration, tok3);
                mTokenList.addtoken(newName, tok3->linenr(), tok3->fileIndex());
            }

            while (tok3 && tok3->str() != "::")
                tok3 = tok3->next();

            std::list<TokenAndName>::iterator it = std::find_if(mTemplateDeclarations.begin(),
                                                   mTemplateDeclarations.end(),
                                                   FindToken(startOfTemplateDeclaration));
            if (it != mTemplateDeclarations.end())
                mMemberFunctionsToDelete.push_back(*it);
        }

        // not part of template.. go on to next token
        else
            continue;

        std::stack<Token *> brackets; // holds "(", "[" and "{" tokens

        // FIXME use full name matching somehow
        const std::string lastName = (templateInstantiation.name.find(' ') != std::string::npos) ? templateInstantiation.name.substr(templateInstantiation.name.rfind(' ')+1) : templateInstantiation.name;


        for (; tok3; tok3 = tok3->next()) {
            if (tok3->isName()) {
                // search for this token in the type vector
                unsigned int itype = 0;
                while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != tok3->str())
                    ++itype;

                // replace type with given type..
                if (itype < typeParametersInDeclaration.size()) {
                    unsigned int typeindentlevel = 0;
                    for (const Token *typetok = mTypesUsedInTemplateInstantiation[itype];
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
                        if (copy) {
                            mTokenList.addtoken(typetok, tok3->linenr(), tok3->fileIndex());
                            mTokenList.back()->isTemplateArg(true);
                        }
                    }
                    continue;
                }
            }

            // replace name..
            if (tok3->str() == lastName) {
                if (Token::simpleMatch(tok3->next(), "<")) {
                    Token *closingBracket = tok3->next()->findClosingBracket();
                    if (closingBracket) {
                        // replace multi token name with single token name
                        if (tok3 == templateDeclarationNameToken ||
                            Token::Match(tok3, newName.c_str())) {
                            if (copy) {
                                mTokenList.addtoken(newName, tok3->linenr(), tok3->fileIndex());
                                tok3 = closingBracket;
                            } else {
                                tok3->str(newName);
                                eraseTokens(tok3, closingBracket->next());
                            }
                            continue;
                        } else if (!templateDeclaration.scope.empty() &&
                                   !alreadyHasNamespace(templateDeclaration, tok3) &&
                                   closingBracket->strAt(1) != "(") {
                            if (copy)
                                addNamespace(templateDeclaration, tok3);
                        }
                    }
                } else {
                    if (copy) {
                        // add namespace if necessary
                        if (!templateDeclaration.scope.empty() &&
                            (isClass ? tok3->strAt(1) != "(" : true)) {
                            addNamespace(templateDeclaration, tok3);
                        }
                        mTokenList.addtoken(newName, tok3->linenr(), tok3->fileIndex());
                    } else if (!Token::Match(tok3->next(), ":|{"))
                        tok3->str(newName);
                    continue;
                }
            }

            // copy
            if (copy)
                mTokenList.addtoken(tok3, tok3->linenr(), tok3->fileIndex());

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
                mTemplateInstantiations.emplace_back(mTokenList.back(), getScopeName(scopeInfo), name, tok3);
            }

            // link() newly tokens manually
            else if (copy) {
                if (tok3->str() == "{") {
                    brackets.push(mTokenList.back());
                } else if (tok3->str() == "(") {
                    brackets.push(mTokenList.back());
                } else if (tok3->str() == "[") {
                    brackets.push(mTokenList.back());
                } else if (tok3->str() == "}") {
                    assert(brackets.empty() == false && brackets.top()->str() == "{");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    if (tok3->strAt(1) == ";") {
                        const Token * tokSemicolon = tok3->next();
                        mTokenList.addtoken(tokSemicolon, tokSemicolon->linenr(), tokSemicolon->fileIndex());
                    }
                    brackets.pop();
                    if (brackets.empty()) {
                        inTemplateDefinition = false;
                        break;
                    }
                } else if (tok3->str() == ")") {
                    assert(brackets.empty() == false && brackets.top()->str() == "(");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    brackets.pop();
                } else if (tok3->str() == "]") {
                    assert(brackets.empty() == false && brackets.top()->str() == "[");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    brackets.pop();
                }
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
    while (tok->tokAt(3) && tok->isNumber() && tok->tokAt(2)->isNumber()) { // %any% %num% %any% %num% %any%
        const Token *before = tok->previous();
        if (!before)
            break;
        const Token* op = tok->next();
        const Token* after = tok->tokAt(3);
        const std::string &num1 = op->previous()->str();
        const std::string &num2 = op->next()->str();
        if (Token::Match(before, "* %num% /") && (num2 != "0") && num1 == MathLib::multiply(num2, MathLib::divide(num1, num2))) {
            // Division where result is a whole number
        } else if (!((op->str() == "*" && (isLowerThanMulDiv(before) || before->str() == "*") && isLowerEqualThanMulDiv(after)) || // associative
                     (Token::Match(op, "[/%]") && isLowerThanMulDiv(before) && isLowerEqualThanMulDiv(after)) || // NOT associative
                     (Token::Match(op, "[+-]") && isLowerThanMulDiv(before) && isLowerThanMulDiv(after)) || // Only partially (+) associative, but handled later
                     (Token::Match(op, ">>|<<") && isLowerThanShift(before) && isLowerThanPlusMinus(after)) || // NOT associative
                     (op->str() == "&" && isLowerThanShift(before) && isLowerThanShift(after)) || // associative
                     (op->str() == "^" && isLowerThanAnd(before) && isLowerThanAnd(after)) || // associative
                     (op->str() == "|" && isLowerThanXor(before) && isLowerThanXor(after)) || // associative
                     (op->str() == "&&" && isLowerThanOr(before) && isLowerThanOr(after)) ||
                     (op->str() == "||" && isLowerThanLogicalAnd(before) && isLowerThanLogicalAnd(after))))
            break;

        // Don't simplify "%num% / 0"
        if (Token::Match(op, "[/%] 0"))
            break;

        // Integer operations
        if (Token::Match(op, ">>|<<|&|^|%or%")) {
            // Don't simplify if operand is negative, shifting with negative
            // operand is UB. Bitmasking with negative operand is implementation
            // defined behaviour.
            if (MathLib::isNegative(num1) || MathLib::isNegative(num2))
                break;

            const MathLib::value v1(num1);
            const MathLib::value v2(num2);

            if (!v1.isInt() || !v2.isInt())
                break;

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
            const bool op1 = !MathLib::isNullValue(num1);
            const bool op2 = !MathLib::isNullValue(num2);
            const bool result = (op->str() == "||") ? (op1 || op2) : (op1 && op2);
            tok->str(result ? "1" : "0");
        }

        else if (Token::Match(tok->previous(), "- %num% - %num%"))
            tok->str(MathLib::add(num1, num2));
        else if (Token::Match(tok->previous(), "- %num% + %num%"))
            tok->str(MathLib::subtract(num1, num2));
        else {
            try {
                tok->str(MathLib::calculate(num1, num2, op->str()[0]));
            } catch (InternalError &e) {
                e.token = tok;
                throw;
            }
        }

        tok->deleteNext(2);

        ret = true;
    }

    return ret;
}

// TODO: This is not the correct class for simplifyCalculations(), so it
// should be moved away.
bool TemplateSimplifier::simplifyCalculations()
{
    bool ret = false;
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
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
            if (simplifyNumericCalculations(tok)) {
                ret = true;
                Token *prev = tok->tokAt(-2);
                while (prev && simplifyNumericCalculations(prev)) {
                    tok = prev;
                    prev = prev->tokAt(-2);
                }
            }

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
                    eraseTokens(tok, tok2);
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
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 0 && *|& %any% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "return|case 0 && *|& %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (tok->str() == "1") {
                if (Token::Match(tok->previous(), "[=[(,] 1 %oror% %any% ,|]|)|;|=|%cop%") ||
                    Token::Match(tok->previous(), "return|case 1 %oror% %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 1 %oror% *|& %any% ,|]|)|;|=|%cop%") ||
                           Token::Match(tok->previous(), "return|case 1 %oror% *|& %any% ,|:|;|=|%cop%")) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
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
                    tok = tok->previous();
                }
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
        if (Token::Match(tok, "%name% ,|>|="))
            typeParametersInDeclaration.push_back(tok);
    }
    return tok;
}

bool TemplateSimplifier::matchSpecialization(
    const Token *templateDeclarationNameToken,
    const Token *templateInstantiationNameToken,
    const std::list<const Token *> & specializations)
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
        getTemplateParametersInDeclaration(startToken->tokAt(2), templateParameters);

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

std::string TemplateSimplifier::getNewName(
    Token *tok2,
    std::list<std::string> &typeStringsUsedInTemplateInstantiation)
{
    std::string typeForNewName;
    unsigned int indentlevel = 0;
    for (Token *tok3 = tok2->tokAt(2); tok3 && (indentlevel > 0 || tok3->str() != ">"); tok3 = tok3->next()) {
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
        if (Token::Match(tok3->tokAt(-2), "<|,|:: %name% <") && templateParameters(tok3) > 0)
            ++indentlevel;
        else if (indentlevel > 0 && Token::Match(tok3, "> [,>]"))
            --indentlevel;
        if (indentlevel == 0 && Token::Match(tok3->previous(), "[<,]")) {
            tok3->hasTemplateSimplifierPointer(true);
            mTypesUsedInTemplateInstantiation.push_back(tok3);
        }
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

    return typeForNewName;
}

bool TemplateSimplifier::simplifyTemplateInstantiations(
    const TokenAndName &templateDeclaration,
    const std::list<const Token *> &specializations,
    const std::time_t maxtime,
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

    const bool printDebug = mSettings->debugwarnings;

    // get the position of the template name
    const int namepos = getTemplateNamePosition(tok);
    if (namepos == -1) {
        // debug message that we bail out..
        if (printDebug && mErrorLogger) {
            const std::list<const Token *> callstack(1, tok);
            mErrorLogger->reportErr(ErrorLogger::ErrorMessage(callstack, &mTokenList, Severity::debug, "debug", "simplifyTemplates: bailing out", false));
        }
        return false;
    }

    const bool specialized = Token::simpleMatch(templateDeclaration.token, "template < >");
    bool isfunc = false;

    if (tok->strAt(namepos + 1) == "(")
        isfunc = true;
    else if (tok->strAt(namepos + 1) == "<") {
        const Token *tok1 = tok->tokAt(namepos + 1)->findClosingBracket();
        if (tok1 && tok1->strAt(1) == "(")
            isfunc = true;
    }

    // locate template usage..
    std::string::size_type numberOfTemplateInstantiations = mTemplateInstantiations.size();
    unsigned int recursiveCount = 0;

    bool instantiated = false;

    for (const TokenAndName &instantiation : mTemplateInstantiations) {
        if (numberOfTemplateInstantiations != mTemplateInstantiations.size()) {
            numberOfTemplateInstantiations = mTemplateInstantiations.size();
            simplifyCalculations();
            ++recursiveCount;
            if (recursiveCount > 100) {
                // bail out..
                break;
            }
        }

        // already simplified
        if (!Token::Match(instantiation.token, "%name% <"))
            continue;

        if (instantiation.name != templateDeclaration.name)
            continue;

        if (!matchSpecialization(tok->tokAt(namepos), instantiation.token, specializations))
            continue;

        Token * const tok2 = instantiation.token;
        if (mErrorLogger && !mTokenList.getFiles().empty())
            mErrorLogger->reportProgress(mTokenList.getFiles()[0], "TemplateSimplifier::simplifyTemplateInstantiations()", tok2->progressValue());
#ifdef MAXTIME
        if (std::time(0) > maxtime)
            return false;
#else
        (void)maxtime;
#endif
        assert(mTokenList.validateToken(tok2)); // that assertion fails on examples from #6021

        const Token *startToken = tok2;
        while (Token::Match(startToken->tokAt(-2), "%name% :: %name%") ||
               Token::Match(startToken->tokAt(-2), "> :: %name%")) {
            if (startToken->strAt(-2) == ">") {
                const Token * tok3 = startToken->tokAt(-2)->findOpeningBracket();
                if (tok3)
                    startToken = tok3->previous();
                else
                    break;
            } else
                startToken = startToken->tokAt(-2);
        }

        if (Token::Match(startToken->previous(), ";|{|}|=|const") &&
            (!specialized && !instantiateMatch(tok2, typeParametersInDeclaration.size(), isfunc ? "(" : "*|&| %name%")))
            continue;

        // New type..
        mTypesUsedInTemplateInstantiation.clear();
        std::list<std::string> typeStringsUsedInTemplateInstantiation;
        std::string typeForNewName = getNewName(tok2, typeStringsUsedInTemplateInstantiation);

        if (typeForNewName.empty() || (!typeParametersInDeclaration.empty() && typeParametersInDeclaration.size() != mTypesUsedInTemplateInstantiation.size())) {
            if (printDebug && mErrorLogger) {
                std::list<const Token *> callstack(1, tok2);
                mErrorLogger->reportErr(ErrorLogger::ErrorMessage(callstack, &mTokenList, Severity::debug, "debug",
                                        "Failed to instantiate template \"" + instantiation.name + "\". The checking continues anyway.", false));
            }
            if (typeForNewName.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string newName(templateDeclaration.name + " < " + typeForNewName + " >");

        if (expandedtemplates.find(newName) == expandedtemplates.end()) {
            expandedtemplates.insert(newName);
            expandTemplate(templateDeclaration, tok, instantiation, typeParametersInDeclaration, newName, !specialized);
            instantiated = true;
        }

        // Replace all these template usages..
        replaceTemplateUsage(tok2, instantiation.name, typeStringsUsedInTemplateInstantiation, newName);
    }

    // process uninstantiated templates
    const std::list<TokenAndName>::iterator it = std::find_if(mTemplateInstantiations.begin(),
            mTemplateInstantiations.end(),
            FindName(templateDeclaration.name));

    // TODO: remove the specialized check and handle all uninstantiated templates someday.
    if (it == mTemplateInstantiations.end() && specialized) {
        simplifyCalculations();

        Token * tok2 = const_cast<Token *>(tok->tokAt(namepos));
        if (mErrorLogger && !mTokenList.getFiles().empty())
            mErrorLogger->reportProgress(mTokenList.getFiles()[0], "TemplateSimplifier::simplifyTemplateInstantiations()", tok2->progressValue());
#ifdef MAXTIME
        if (std::time(0) > maxtime)
            return false;
#else
        (void)maxtime;
#endif
        assert(mTokenList.validateToken(tok2)); // that assertion fails on examples from #6021

        Token *startToken = tok2;
        while (Token::Match(startToken->tokAt(-2), "%name% :: %name%") ||
               Token::Match(startToken->tokAt(-2), "> :: %name%")) {
            if (startToken->strAt(-2) == ">") {
                const Token * tok3 = startToken->tokAt(-2)->findOpeningBracket();
                if (tok3)
                    startToken = tok3->previous();
                else
                    break;
            } else
                startToken = startToken->tokAt(-2);
        }

        if (Token::Match(startToken->previous(), ";|{|}|=|const") &&
            (!specialized && !instantiateMatch(tok2, typeParametersInDeclaration.size(), isfunc ? "(" : "*|&| %name%")))
            return false;

        // already simplified
        if (!Token::Match(startToken, "%name% <"))
            return false;

        if (templateDeclaration.scope.empty()) {
            if (startToken->str() != templateDeclaration.name)
                return false;
        } else {
            std::string name = templateDeclaration.scope + " :: " + startToken->str();
            if (name != templateDeclaration.name)
                return false;
        }

        if (!matchSpecialization(tok->tokAt(namepos), startToken, specializations))
            return false;

        tok2 = startToken;

        // New type..
        mTypesUsedInTemplateInstantiation.clear();
        std::list<std::string> typeStringsUsedInTemplateInstantiation;
        std::string typeForNewName = getNewName(tok2, typeStringsUsedInTemplateInstantiation);

        if (typeForNewName.empty()) {
            if (printDebug && mErrorLogger) {
                std::list<const Token *> callstack(1, tok2);
                mErrorLogger->reportErr(ErrorLogger::ErrorMessage(callstack, &mTokenList, Severity::debug, "debug",
                                        "Failed to instantiate template \"" + templateDeclaration.name + "\". The checking continues anyway.", false));
            }
            return false;
        }

        // New classname/funcname..
        const std::string newName(templateDeclaration.name + " < " + typeForNewName + " >");
        if (expandedtemplates.find(newName) == expandedtemplates.end()) {
            expandedtemplates.insert(newName);
            expandTemplate(templateDeclaration, tok, templateDeclaration, typeParametersInDeclaration, newName, false);
            instantiated = true;
        }

        // Replace all these template usages..
        replaceTemplateUsage(startToken, templateDeclaration.name, typeStringsUsedInTemplateInstantiation, newName);
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

void TemplateSimplifier::replaceTemplateUsage(
    Token * const instantiationToken,
    const std::string &templateName,
    const std::list<std::string> &typeStringsUsedInTemplateInstantiation,
    const std::string &newName)
{
    std::list<ScopeInfo2> scopeInfo;
    std::list< std::pair<Token *, Token *> > removeTokens;
    for (Token *nameTok = instantiationToken; nameTok; nameTok = nameTok->next()) {
        if (Token::Match(nameTok, "}|namespace|class|struct|union")) {
            setScopeInfo(nameTok, &scopeInfo);
            continue;
        }
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
        const Token *typetok = (!mTypesUsedInTemplateInstantiation.empty()) ? mTypesUsedInTemplateInstantiation[0] : nullptr;
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
                    if (typeCountInInstantiation < mTypesUsedInTemplateInstantiation.size())
                        typetok = mTypesUsedInTemplateInstantiation[typeCountInInstantiation++];
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
        if (tok2->str() == ">" && typeCountInInstantiation == mTypesUsedInTemplateInstantiation.size()) {
            const Token * const nameTok1 = nameTok;
            nameTok->str(newName);
            for (std::list<TokenAndName>::iterator it = mTemplateInstantiations.begin(); it != mTemplateInstantiations.end(); ++it) {
                if (it->token == nameTok1)
                    it->token = nameTok;
            }
            for (Token *tok = nameTok1->next(); tok != tok2; tok = tok->next()) {
                if (tok->isName()) {
                    std::list<TokenAndName>::iterator ti;
                    for (ti = mTemplateInstantiations.begin(); ti != mTemplateInstantiations.end();) {
                        if (ti->token == tok)
                            mTemplateInstantiations.erase(ti++);
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
        eraseTokens(removeTokens.back().first, removeTokens.back().second);
        removeTokens.pop_back();
    }
}

void TemplateSimplifier::fixForwardDeclaredDefaultArgumentValues()
{
    // try to locate a matching declaration for each forward declaration
    for (const auto & forwardDecl : mTemplateForwardDeclarations) {
        std::vector<const Token *> params1;

        getTemplateParametersInDeclaration(forwardDecl.token, params1);

        for (auto & decl : mTemplateDeclarations) {
            std::vector<const Token *> params2;

            getTemplateParametersInDeclaration(decl.token, params2);

            // make sure the number of arguments match
            if (params1.size() == params2.size()) {
                std::string declName = decl.scope;
                if (!declName.empty())
                    declName += " :: ";
                if (decl.nameToken->strAt(-1) == "::") {
                    const Token * start = decl.nameToken;

                    while (Token::Match(start->tokAt(-2), "%name% ::"))
                        start = start->tokAt(-2);

                    while (start != decl.nameToken) {
                        declName += (start->str() + " ");
                        start = start->next();
                    }
                }
                declName += decl.name;

                std::string forwardDeclName = forwardDecl.scope;
                if (!forwardDeclName.empty())
                    forwardDeclName += " :: ";
                forwardDeclName += forwardDecl.name;

                // make sure the scopes and names match
                if (forwardDeclName == declName) {
                    // save forward declaration for lookup later
                    if ((decl.nameToken->strAt(1) == "(" && forwardDecl.nameToken->strAt(1) == "(") ||
                        (decl.nameToken->strAt(1) == "{" && forwardDecl.nameToken->strAt(1) == ";")) {
                        mTemplateForwardDeclarationsMap[decl.token] = forwardDecl.token;
                    }

                    for (size_t k = 0; k < params1.size(); k++) {
                        // copy default value to declaration if not present
                        if (params1[k]->strAt(1) == "=" && params2[k]->strAt(1) != "=") {
                            const_cast<Token *>(params2[k])->insertToken(params1[k]->strAt(2));
                            const_cast<Token *>(params2[k])->insertToken(params1[k]->strAt(1));
                        }
                    }
                }
            }
        }
    }
}

void TemplateSimplifier::simplifyTemplates(
    const std::time_t maxtime,
    bool &codeWithTemplates)
{
    // TODO: 2 is not the ideal number of loops.
    // We should loop until the number of declarations is 0 but we can't
    // do that until we instantiate unintstantiated templates with their symbolic types.
    // That will allow the uninstantiated template code to be removed from the symbol database.
    // Unfortunately the template simplifier doesn't handle namespaces properly so
    // the uninstantiated template code in the symbol database can't be removed until #8768
    // is fixed.
    for (int i = 0; i < 2; ++i) {
        if (i) {
            mTemplateDeclarations.clear();
            mTemplateForwardDeclarations.clear();
            mTemplateForwardDeclarationsMap.clear();
            mTemplateInstantiations.clear();
            mInstantiatedTemplates.clear();
        }

        bool hasTemplates = getTemplateDeclarations();

        if (i == 0) {
            codeWithTemplates = hasTemplates;
            if (hasTemplates) {
                // There are templates..
                // Remove "typename" unless used in template arguments..
                for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
                    if (tok->str() == "typename")
                        tok->deleteThis();

                    if (Token::simpleMatch(tok, "template <")) {
                        while (tok && tok->str() != ">")
                            tok = tok->next();
                        if (!tok)
                            break;
                    }
                }
            }
        }

        // Make sure there is something to simplify.
        if (mTemplateDeclarations.empty())
            return;

        // Copy default argument values from forward declaration to declaration
        fixForwardDeclaredDefaultArgumentValues();

        // Locate possible instantiations of templates..
        getTemplateInstantiations();

        // Template arguments with default values
        useDefaultArgumentValues();

        simplifyTemplateAliases();

        std::set<std::string> expandedtemplates;

        for (std::list<TokenAndName>::reverse_iterator iter1 = mTemplateDeclarations.rbegin(); iter1 != mTemplateDeclarations.rend(); ++iter1) {
            // get specializations..
            std::list<const Token *> specializations;
            for (std::list<TokenAndName>::const_iterator iter2 = mTemplateDeclarations.begin(); iter2 != mTemplateDeclarations.end(); ++iter2) {
                if (iter1->name == iter2->name) {
                    const Token *tok = iter2->token->next()->findClosingBracket();
                    const int namepos = getTemplateNamePosition(tok);
                    if (namepos > 0)
                        specializations.push_back(tok->tokAt(namepos));
                }
            }

            const bool instantiated = simplifyTemplateInstantiations(
                                          *iter1,
                                          specializations,
                                          maxtime,
                                          expandedtemplates);
            if (instantiated)
                mInstantiatedTemplates.push_back(*iter1);
        }

        for (std::list<TokenAndName>::const_iterator it = mInstantiatedTemplates.begin(); it != mInstantiatedTemplates.end(); ++it) {
            std::list<TokenAndName>::iterator decl;
            for (decl = mTemplateDeclarations.begin(); decl != mTemplateDeclarations.end(); ++decl) {
                if (decl->token == it->token)
                    break;
            }
            if (decl != mTemplateDeclarations.end()) {
                if (Token::simpleMatch(it->token, "template < >")) {
                    // delete the "template < >"
                    Token * tok = it->token;
                    tok->deleteNext(2);
                    tok->deleteThis();
                } else
                    removeTemplate(it->token);
                mTemplateDeclarations.erase(decl);
            }
        }

        // remove out of line member functions
        while (!mMemberFunctionsToDelete.empty()) {
            const std::list<TokenAndName>::iterator it = std::find_if(mTemplateDeclarations.begin(),
                    mTemplateDeclarations.end(),
                    FindToken(mMemberFunctionsToDelete.begin()->token));
            // multiple functions can share the same declaration so make sure it hasn't already been deleted
            if (it != mTemplateDeclarations.end()) {
                removeTemplate(it->token);
                mTemplateDeclarations.erase(it);
            }
            mMemberFunctionsToDelete.erase(mMemberFunctionsToDelete.begin());
        }
    }
}

void TemplateSimplifier::syntaxError(const Token *tok)
{
    throw InternalError(tok, "syntax error", InternalError::SYNTAX);
}
