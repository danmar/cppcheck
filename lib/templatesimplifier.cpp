/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include <map>
#include <stack>
#include <utility>

namespace {
    class FindToken {
    public:
        explicit FindToken(const Token *token) : mToken(token) {}
        bool operator()(const TemplateSimplifier::TokenAndName &tokenAndName) const {
            return tokenAndName.token() == mToken;
        }
    private:
        const Token * const mToken;
    };

    class FindName {
    public:
        explicit FindName(const std::string &name) : mName(name) {}
        bool operator()(const TemplateSimplifier::TokenAndName &tokenAndName) const {
            return tokenAndName.name() == mName;
        }
    private:
        const std::string mName;
    };

    class FindFullName {
    public:
        explicit FindFullName(const std::string &fullName) : mFullName(fullName) {}
        bool operator()(const TemplateSimplifier::TokenAndName &tokenAndName) const {
            return tokenAndName.fullName() == mFullName;
        }
    private:
        const std::string mFullName;
    };
}

TemplateSimplifier::TokenAndName::TokenAndName(Token *token, const std::string &scope) :
    mToken(token), mScope(scope), mName(mToken ? mToken->str() : ""),
    mFullName(mScope.empty() ? mName : (mScope + " :: " + mName)),
    mNameToken(nullptr), mParamEnd(nullptr), mFlags(0)
{
    if (mToken) {
        if (mToken->strAt(1) == "<") {
            const Token *end = mToken->next()->findClosingBracket();
            if (end && end->strAt(1) == "(") {
                isFunction(true);
            }
        }
        mToken->templateSimplifierPointer(this);
    }
}

TemplateSimplifier::TokenAndName::TokenAndName(Token *token, const std::string &scope, const Token *nameToken, const Token *paramEnd) :
    mToken(token), mScope(scope), mName(nameToken->str()),
    mFullName(mScope.empty() ? mName : (mScope + " :: " + mName)),
    mNameToken(nameToken), mParamEnd(paramEnd), mFlags(0)
{
    // only set flags for declaration
    if (mToken && mNameToken && mParamEnd) {
        isSpecialization(Token::simpleMatch(mToken, "template < >"));

        if (!isSpecialization()) {
            if (Token::simpleMatch(mToken->next()->findClosingBracket(), "> template <")) {
                const Token * temp = mNameToken->tokAt(-2);
                while (Token::Match(temp, ">|%name% ::")) {
                    if (temp->str() == ">")
                        temp = temp->findOpeningBracket()->previous();
                    else
                        temp = temp->tokAt(-2);
                }
                isPartialSpecialization(temp->strAt(1) == "<");
            } else
                isPartialSpecialization(mNameToken->strAt(1) == "<");
        }

        isAlias(mParamEnd->strAt(1) == "using");

        if (isAlias() && isPartialSpecialization()) {
            throw InternalError(mToken, "partial specialization of alias templates is not permitted", InternalError::SYNTAX);
        }
        if (isAlias() && isSpecialization()) {
            throw InternalError(mToken, "explicit specialization of alias templates is not permitted", InternalError::SYNTAX);
        }

        isFriend(mParamEnd->strAt(1) == "friend");
        const Token *next = mParamEnd->next();
        if (isFriend())
            next = next->next();

        isClass(Token::Match(next, "class|struct|union %name% <|{|:|;|::"));
        if (mToken->strAt(1) == "<" && !isSpecialization()) {
            const Token *end = mToken->next()->findClosingBracket();
            isVariadic(end && Token::findmatch(mToken->tokAt(2), "typename|class ...", end));
        }
        const Token *tok1 = mNameToken->next();
        if (tok1->str() == "<") {
            const Token *closing = tok1->findClosingBracket();
            if (closing)
                tok1 = closing->next();
            else
                throw InternalError(mToken, "unsupported syntax", InternalError::SYNTAX);
        }
        isFunction(tok1->str() == "(");
        isVariable(!isClass() && !isAlias() && !isFriend() && Token::Match(tok1, "=|;"));
        if (!isFriend()) {
            if (isVariable())
                isForwardDeclaration(tok1->str() == ";");
            else if (!isAlias()) {
                if (isFunction())
                    tok1 = tok1->link()->next();
                while (tok1 && !Token::Match(tok1, ";|{")) {
                    if (tok1->str() == "<")
                        tok1 = tok1->findClosingBracket();
                    else if (Token::Match(tok1, "(|[") && tok1->link())
                        tok1 = tok1->link();
                    if (tok1)
                        tok1 = tok1->next();
                }
                if (tok1)
                    isForwardDeclaration(tok1->str() == ";");
            }
        }
        // check for member class or function and adjust scope
        if ((isFunction() || isClass()) &&
            (mNameToken->strAt(-1) == "::" || Token::simpleMatch(mNameToken->tokAt(-2), ":: ~"))) {
            const Token * start = mNameToken;
            if (start->strAt(-1) == "~")
                start = start->previous();
            const Token *end = start;

            while (start && (Token::Match(start->tokAt(-2), "%name% ::") ||
                             (Token::simpleMatch(start->tokAt(-2), "> ::") &&
                              start->tokAt(-2)->findOpeningBracket() &&
                              Token::Match(start->tokAt(-2)->findOpeningBracket()->previous(), "%name% <")))) {
                if (start->strAt(-2) == ">")
                    start = start->tokAt(-2)->findOpeningBracket()->previous();
                else
                    start = start->tokAt(-2);
            }

            if (start && start != end) {
                if (!mScope.empty())
                    mScope += " ::";
                while (start && start->next() != end) {
                    if (start->str() == "<")
                        start = start->findClosingBracket();
                    else {
                        if (!mScope.empty())
                            mScope += " ";
                        mScope += start->str();
                    }
                    start = start->next();
                }
                if (start)
                    mFullName = mScope.empty() ? mName : (mScope + " :: " + mName);
            }
        }
    }

    // make sure at most only one family flag is set
    assert(isClass() ? !(isFunction() || isVariable()) : true);
    assert(isFunction() ? !(isClass() || isVariable()) : true);
    assert(isVariable() ? !(isClass() || isFunction()) : true);

    if (mToken)
        mToken->templateSimplifierPointer(this);
}

TemplateSimplifier::TokenAndName::TokenAndName(const TokenAndName& other) :
    mToken(other.mToken), mScope(other.mScope), mName(other.mName), mFullName(other.mFullName),
    mNameToken(other.mNameToken), mParamEnd(other.mParamEnd), mFlags(other.mFlags)
{
    if (mToken)
        mToken->templateSimplifierPointer(this);
}

TemplateSimplifier::TokenAndName::~TokenAndName()
{
    if (mToken && mToken->templateSimplifierPointers())
        mToken->templateSimplifierPointers()->erase(this);
}

const Token * TemplateSimplifier::TokenAndName::aliasStartToken() const
{
    if (mParamEnd)
        return mParamEnd->tokAt(4);
    return nullptr;
}

const Token * TemplateSimplifier::TokenAndName::aliasEndToken() const
{
    if (aliasStartToken())
        return Token::findsimplematch(aliasStartToken(), ";");
    return nullptr;
}

bool TemplateSimplifier::TokenAndName::isAliasToken(const Token *tok) const
{
    const Token *end = aliasEndToken();

    for (const Token *tok1 = aliasStartToken(); tok1 != end; tok1 = tok1->next()) {
        if (tok1 == tok)
            return true;
    }
    return false;
}

TemplateSimplifier::TemplateSimplifier(Tokenizer *tokenizer)
    : mTokenizer(tokenizer), mTokenList(tokenizer->list), mSettings(tokenizer->mSettings),
      mErrorLogger(tokenizer->mErrorLogger), mChanged(false)
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
        while (Token::Match(tok, ";|{"))
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
                    else if (level == 0 && Token::Match(tok2->previous(), "%type%")) {
                        // @todo add better expression detection
                        if (!(Token::Match(tok2->next(), "*| %type%|%num% ;") ||
                              Token::Match(tok2->next(), "*| %type% . %type% ;"))) {
                            inclevel = true;
                        }
                    } else if (tok2->next() && tok2->next()->isStandardType() && !Token::Match(tok2->tokAt(2), "(|{"))
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
                    } else if (tok2->strAt(-1) == ">")
                        syntaxError(tok);

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
    if (Token::Match(tok->previous(), "%var% <"))
        return 0;
    tok = tok->next();
    if (!tok || tok->str() == ">")
        return 0;

    unsigned int level = 0;

    while (tok) {
        // skip template template
        if (level == 0 && Token::simpleMatch(tok, "template <")) {
            const Token *closing = tok->next()->findClosingBracket();
            if (closing) {
                if (closing->str() == ">>")
                    return numberOfParameters;
                tok = closing->next();
                if (Token::Match(tok, ">|>>|>>="))
                    return numberOfParameters;
                else if (tok->str() == ",") {
                    ++numberOfParameters;
                    tok = tok->next();
                    continue;
                }
            } else
                return 0;
        }

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
        if (Token::simpleMatch(tok, "...")) {
            if ((tok->previous()->isName() && !Token::Match(tok->tokAt(-2), "<|,|::")) ||
                (!tok->previous()->isName() && tok->strAt(-1) != ">"))
                return 0; // syntax error
            tok = tok->next();
            if (!tok)
                return 0;
            if (tok->str() == ">") {
                if (level == 0)
                    return numberOfParameters;
                --level;
            } else if (tok->str() == ">>" || tok->str() == ">>=") {
                if (level == 1)
                    return numberOfParameters;
                level -= 2;
            } else if (tok->str() == "," && level == 0) {
                ++numberOfParameters;
                tok = tok->next();
                continue;
            }
        }

        // Skip '=', '?', ':'
        if (Token::Match(tok, "=|?|:"))
            tok = tok->next();
        if (!tok)
            return 0;

        // Skip links
        if (Token::Match(tok, "(|{")) {
            tok = tok->link();
            if (tok)
                tok = tok->next();
            if (!tok)
                return 0;
            if (tok->str() == ">" && level == 0)
                return numberOfParameters;
            else if ((tok->str() == ">>" || tok->str() == ">>=") && level == 1)
                return numberOfParameters;
            else if (tok->str() == ",") {
                if (level == 0)
                    ++numberOfParameters;
                tok = tok->next();
            }
            continue;
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
        while (Token::Match(tok, ">|>>|>>=")) {
            if (level == 0)
                return tok->str() == ">" && !Token::Match(tok->next(), "%num%") ? numberOfParameters : 0;
            --level;
            if (tok->str() == ">>" || tok->str() == ">>=") {
                if (level == 0)
                    return !Token::Match(tok->next(), "%num%") ? numberOfParameters : 0;
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

const Token *TemplateSimplifier::findTemplateDeclarationEnd(const Token *tok)
{
    return const_cast<const Token *>(findTemplateDeclarationEnd(const_cast<Token *>(tok)));
}

Token *TemplateSimplifier::findTemplateDeclarationEnd(Token *tok)
{
    if (Token::simpleMatch(tok, "template <")) {
        tok = tok->next()->findClosingBracket();
        if (tok)
            tok = tok->next();
    }

    if (!tok)
        return nullptr;

    Token * tok2 = tok;
    bool in_init = false;
    while (tok2 && !Token::Match(tok2, ";|{")) {
        if (tok2->str() == "<")
            tok2 = tok2->findClosingBracket();
        else if (Token::Match(tok2, "(|[") && tok2->link())
            tok2 = tok2->link();
        else if (tok2->str() == ":")
            in_init = true;
        else if (in_init && Token::Match(tok2, "%name% (|{")) {
            tok2 = tok2->linkAt(1);
            if (tok2->strAt(1) == "{")
                in_init = false;
        }
        if (tok2)
            tok2 = tok2->next();
    }
    if (tok2 && tok2->str() == "{") {
        tok = tok2->link();
        if (tok && tok->strAt(1) == ";")
            tok = tok->next();
    } else if (tok2 && tok2->str() == ";")
        tok = tok2;
    else
        tok = nullptr;

    return tok;
}

void TemplateSimplifier::eraseTokens(Token *begin, const Token *end)
{
    if (!begin || begin == end)
        return;

    while (begin->next() && begin->next() != end) {
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

    Token *end = findTemplateDeclarationEnd(tok);
    if (end && end->next()) {
        eraseTokens(tok, end->next());
        deleteToken(tok);
        return true;
    }

    return false;
}

bool TemplateSimplifier::getTemplateDeclarations()
{
    bool codeWithTemplates = false;
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "template <"))
            continue;
        // ignore template template parameter
        if (tok->strAt(-1) == "<" || tok->strAt(-1) == ",")
            continue;
        // ignore nested template
        if (tok->strAt(-1) == ">")
            continue;
        // skip to last nested template parameter
        const Token *tok1 = tok;
        while (tok1 && tok1->next()) {
            const Token *closing = tok1->next()->findClosingBracket();
            if (!Token::simpleMatch(closing, "> template <"))
                break;
            tok1 = closing->next();
        }
        if (!tok1)
            syntaxError(tok);
        if (!tok1->next())
            syntaxError(tok);
        // Some syntax checks, see #6865
        if (!tok->tokAt(2))
            syntaxError(tok->next());
        if (tok->strAt(2)=="typename" &&
            !Token::Match(tok->tokAt(3), "%name%|...|,|=|>"))
            syntaxError(tok->next());
        codeWithTemplates = true;
        const Token * const parmEnd = tok1->next()->findClosingBracket();
        for (const Token *tok2 = parmEnd; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(" && tok2->link())
                tok2 = tok2->link();
            else if (tok2->str() == ")")
                break;
            // skip decltype(...)
            else if (Token::simpleMatch(tok2, "decltype ("))
                tok2 = tok2->linkAt(1);
            else if (Token::Match(tok2, "{|=|;")) {
                const int namepos = getTemplateNamePosition(parmEnd);
                if (namepos > 0) {
                    TokenAndName decl(tok, tok->scopeInfo()->name, parmEnd->tokAt(namepos), parmEnd);
                    if (decl.isForwardDeclaration()) {
                        // Declaration => add to mTemplateForwardDeclarations
                        mTemplateForwardDeclarations.emplace_back(std::move(decl));
                    } else {
                        // Implementation => add to mTemplateDeclarations
                        mTemplateDeclarations.emplace_back(std::move(decl));
                    }
                    Token *end = findTemplateDeclarationEnd(tok);
                    if (end)
                        tok = end;
                    break;
                }
            }
        }
    }
    return codeWithTemplates;
}

void TemplateSimplifier::addInstantiation(Token *token, const std::string &scope)
{
    simplifyTemplateArgs(token->tokAt(2), token->next()->findClosingBracket());

    TokenAndName instantiation(token, scope);

    // check if instantiation already exists before adding it
    std::list<TokenAndName>::iterator it = std::find(mTemplateInstantiations.begin(),
                                           mTemplateInstantiations.end(),
                                           instantiation);

    if (it == mTemplateInstantiations.end())
        mTemplateInstantiations.emplace_back(instantiation);
}

static void getFunctionArguments(const Token *nameToken, std::vector<const Token *> &args)
{
    const Token *argToken;

    if (nameToken->strAt(1) == "(")
        argToken = nameToken->tokAt(2);
    else if (nameToken->strAt(1) == "<") {
        const Token *end = nameToken->next()->findClosingBracket();
        if (end)
            argToken = end->tokAt(2);
        else
            return;
    } else
        return;

    if (argToken->str() == ")")
        return;

    args.push_back(argToken);

    while ((argToken = argToken->nextArgumentBeforeCreateLinks2()))
        args.push_back(argToken);
}

static bool areAllParamsTypes(const std::vector<const Token *> &params)
{
    if (params.empty())
        return false;

    for (auto param : params) {
        if (!Token::Match(param->previous(), "typename|class %name% ,|>"))
            return false;
    }

    return true;
}

void TemplateSimplifier::getTemplateInstantiations()
{
    std::multimap<std::string, const TokenAndName *> functionNameMap;

    for (const auto & decl : mTemplateDeclarations) {
        if (decl.isFunction())
            functionNameMap.insert(std::make_pair(decl.name(), &decl));
    }

    for (const auto & decl : mTemplateForwardDeclarations) {
        if (decl.isFunction())
            functionNameMap.insert(std::make_pair(decl.name(), &decl));
    }

    const Token *skip = nullptr;

    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {

        // template definition.. skip it
        if (Token::simpleMatch(tok, "template <")) {
            tok = tok->next()->findClosingBracket();
            if (!tok)
                break;

            const bool isUsing = tok->strAt(1) == "using";
            if (isUsing && Token::Match(tok->tokAt(2), "%name% <")) {
                // Can't have specialized type alias so ignore it
                Token *tok2 = Token::findsimplematch(tok->tokAt(3), ";");
                if (tok2)
                    tok = tok2;
            } else if (tok->strAt(-1) == "<") {
                // Don't ignore user specialization but don't consider it an instantiation.
                // Instantiations in return type, function parameters, and executable code
                // are not ignored.
                unsigned int pos = getTemplateNamePosition(tok);
                if (pos > 0)
                    skip = tok->tokAt(pos);
            } else {
                // #7914
                // Ignore template instantiations within template definitions: they will only be
                // handled if the definition is actually instantiated

                Token * tok2 = findTemplateDeclarationEnd(tok->next());
                if (tok2)
                    tok = tok2;
            }
        } else if (Token::Match(tok, "template using %name% <")) {
            // Can't have specialized type alias so ignore it
            Token *tok2 = Token::findsimplematch(tok->tokAt(3), ";");
            if (tok2)
                tok = tok2;
        } else if (Token::Match(tok, "using %name% <")) {
            // Can't have specialized type alias so ignore it
            Token *tok2 = Token::findsimplematch(tok->tokAt(2), ";");
            if (tok2)
                tok = tok2;
        } else if (Token::Match(tok->previous(), "(|{|}|;|=|>|<<|:|.|*|&|return|<|,|!|[ %name% ::|<|(") ||
                   Token::Match(tok->previous(), "%type% %name% ::|<") ||
                   Token::Match(tok->tokAt(-2), "[,:] private|protected|public %name% ::|<")) {
            std::string scopeName = tok->scopeInfo()->name;
            std::string qualification;
            Token * qualificationTok = tok;
            while (Token::Match(tok, "%name% :: %name%")) {
                qualification += (qualification.empty() ? "" : " :: ") + tok->str();
                tok = tok->tokAt(2);
            }

            // skip specialization
            if (tok == skip) {
                skip = nullptr;
                continue;
            }

            // look for function instantiation with type deduction
            if (tok->strAt(1) == "(") {
                std::vector<const Token *> instantiationArgs;
                getFunctionArguments(tok, instantiationArgs);

                std::string fullName;
                if (!qualification.empty())
                    fullName = qualification + " :: " + tok->str();
                else if (!scopeName.empty())
                    fullName = scopeName + " :: " + tok->str();
                else
                    fullName = tok->str();

                // get all declarations with this name
                for (auto pos = functionNameMap.lower_bound(tok->str());
                     pos != functionNameMap.upper_bound(tok->str()); ++pos) {
                    // look for declaration with same qualification or constructor with same qualification
                    if (pos->second->fullName() == fullName ||
                        (pos->second->scope() == fullName && tok->str() == pos->second->name())) {
                        std::vector<const Token *> templateParams;
                        getTemplateParametersInDeclaration(pos->second->token()->tokAt(2), templateParams);

                        // todo: handle more than one template parameter
                        if (templateParams.size() != 1 || !areAllParamsTypes(templateParams))
                            continue;

                        std::vector<const Token *> declarationParams;
                        getFunctionArguments(pos->second->nameToken(), declarationParams);

                        // function argument counts must match
                        if (instantiationArgs.empty() || instantiationArgs.size() != declarationParams.size())
                            continue;

                        size_t match = 0;
                        size_t argMatch = 0;
                        for (size_t i = 0; i < declarationParams.size(); ++i) {
                            // fixme: only type deducton from literals is supported
                            bool isArgLiteral = Token::Match(instantiationArgs[i], "%num%|%str%|%char%|%bool% ,|)");
                            if (isArgLiteral && Token::Match(declarationParams[i], "const| %type% &| %name%| ,|)")) {
                                match++;

                                // check if parameter types match
                                if (templateParams[0]->str() == declarationParams[i]->str())
                                    argMatch = i;
                                else {
                                    // todo: check if non-template args match for function overloads
                                }
                            }
                        }

                        if (match == declarationParams.size()) {
                            const Token *arg = instantiationArgs[argMatch];
                            tok->insertToken(">");
                            switch (arg->tokType()) {
                            case Token::eBoolean:
                                tok->insertToken("bool");
                                break;
                            case Token::eChar:
                                if (arg->isLong())
                                    tok->insertToken("wchar_t");
                                else
                                    tok->insertToken("char");
                                break;
                            case Token::eString:
                                tok->insertToken("*");
                                if (arg->isLong())
                                    tok->insertToken("wchar_t");
                                else
                                    tok->insertToken("char");
                                tok->insertToken("const");
                                break;
                            case Token::eNumber: {
                                MathLib::value num(arg->str());
                                if (num.isFloat()) {
                                    // MathLib::getSuffix doesn't work for floating point numbers
                                    char suffix = arg->str().back();
                                    if (suffix == 'f' || suffix == 'F')
                                        tok->insertToken("float");
                                    else if (suffix == 'l' || suffix == 'L') {
                                        tok->insertToken("double");
                                        tok->next()->isLong(true);
                                    } else
                                        tok->insertToken("double");
                                } else if (num.isInt()) {
                                    std::string suffix = MathLib::getSuffix(tok->strAt(3));
                                    if (suffix.find("LL") != std::string::npos) {
                                        tok->insertToken("long");
                                        tok->next()->isLong(true);
                                    } else if (suffix.find('L') != std::string::npos)
                                        tok->insertToken("long");
                                    else
                                        tok->insertToken("int");
                                    if (suffix.find('U') != std::string::npos)
                                        tok->next()->isUnsigned(true);
                                }
                                break;
                            }
                            default:
                                break;
                            }
                            tok->insertToken("<");
                            break;
                        }
                    }
                }
            }

            if (!Token::Match(tok, "%name% <") ||
                Token::Match(tok, "const_cast|dynamic_cast|reinterpret_cast|static_cast"))
                continue;

            if (tok == skip) {
                skip = nullptr;
                continue;
            }

            // Add inner template instantiations first => go to the ">"
            // and then parse backwards, adding all seen instantiations
            Token *tok2 = tok->next()->findClosingBracket();

            // parse backwards and add template instantiations
            // TODO
            for (; tok2 && tok2 != tok; tok2 = tok2->previous()) {
                if (Token::Match(tok2, ",|< %name% <") &&
                    (tok2->strAt(3) == ">" || templateParameters(tok2->tokAt(2)))) {
                    addInstantiation(tok2->next(), tok->scopeInfo()->name);
                } else if (Token::Match(tok2->next(), "class|struct"))
                    tok2->deleteNext();
            }

            // Add outer template..
            if (templateParameters(tok->next()) || tok->strAt(2) == ">") {
                const std::string scopeName1(scopeName);
                while (true) {
                    const std::string fullName = scopeName + (scopeName.empty()?"":" :: ") +
                                                 qualification + (qualification.empty()?"":" :: ") + tok->str();
                    const std::list<TokenAndName>::const_iterator it = std::find_if(mTemplateDeclarations.begin(), mTemplateDeclarations.end(), FindFullName(fullName));
                    if (it != mTemplateDeclarations.end()) {
                        // full name matches
                        addInstantiation(tok, it->scope());
                        break;
                    } else {
                        // full name doesn't match so try with using namespaces if available
                        bool found = false;
                        for (const auto & nameSpace :  tok->scopeInfo()->usingNamespaces) {
                            std::string fullNameSpace = scopeName + (scopeName.empty()?"":" :: ") +
                                                        nameSpace + (qualification.empty()?"":" :: ") + qualification;
                            std::string newFullName = fullNameSpace + " :: " + tok->str();
                            const std::list<TokenAndName>::const_iterator it1 = std::find_if(mTemplateDeclarations.begin(), mTemplateDeclarations.end(), FindFullName(newFullName));
                            if (it1 != mTemplateDeclarations.end()) {
                                // insert using namespace into token stream
                                std::string::size_type offset = 0;
                                std::string::size_type pos = 0;
                                while ((pos = nameSpace.substr(offset).find(' ')) != std::string::npos) {
                                    qualificationTok->insertToken(nameSpace.substr(offset, pos), "", true);
                                    offset = offset + pos + 1;
                                }
                                qualificationTok->insertToken(nameSpace.substr(offset), "", true);
                                qualificationTok->insertToken("::", "", true);
                                addInstantiation(tok, it1->scope());
                                found = true;
                                break;
                            }
                        }
                        if (found)
                            break;

                        if (scopeName.empty()) {
                            if (!qualification.empty())
                                addInstantiation(tok, qualification);
                            else
                                addInstantiation(tok,  tok->scopeInfo()->name);
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
    for (TokenAndName &declaration : mTemplateDeclarations)
        useDefaultArgumentValues(declaration);

    for (TokenAndName &declaration : mTemplateForwardDeclarations)
        useDefaultArgumentValues(declaration);
}

void TemplateSimplifier::useDefaultArgumentValues(TokenAndName &declaration)
{
    // Ticket #5762: Skip specialization tokens
    if (declaration.isSpecialization() || declaration.isAlias() || declaration.isFriend())
        return;

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

    // map type parameter name to index
    std::map<std::string, unsigned int> typeParameterNames;

    // Scan template declaration..
    for (Token *tok = declaration.token(); tok; tok = tok->next()) {
        if (tok->link() && Token::Match(tok, "{|(|[")) { // Ticket #6835
            tok = tok->link();
            continue;
        }

        if (tok->str() == "<" &&
            (tok->strAt(1) == ">" || (tok->previous()->isName() &&
                                      typeParameterNames.find(tok->strAt(-1)) == typeParameterNames.end())))
            ++templateParmDepth;

        // end of template parameters?
        if (tok->str() == ">") {
            if (templateParmDepth<2)
                break;
            else
                --templateParmDepth;
        }

        // map type parameter name to index
        if (Token::Match(tok, "typename|class|%type% %name% ,|>"))
            typeParameterNames[tok->strAt(1)] = templatepar - 1;

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
    if (eq.empty())
        return;

    // iterate through all template instantiations
    for (const TokenAndName &instantiation : mTemplateInstantiations) {
        if (declaration.fullName() != instantiation.fullName())
            continue;

        // instantiation arguments..
        std::vector<std::vector<const Token *>> instantiationArgs;
        std::size_t index = 0;
        const Token *end = instantiation.token()->next()->findClosingBracket();
        if (!end)
            continue;
        if (end != instantiation.token()->tokAt(2))
            instantiationArgs.resize(1);
        for (const Token *tok1 = instantiation.token()->tokAt(2); tok1 && tok1 != end; tok1 = tok1->next()) {
            if (tok1->link() && Token::Match(tok1, "{|(|[")) {
                const Token *endLink = tok1->link();
                do {
                    instantiationArgs[index].push_back(tok1);
                    tok1 = tok1->next();
                } while (tok1 && tok1 != endLink);
                instantiationArgs[index].push_back(tok1);
            } else if (tok1->str() == "<" &&
                       (tok1->strAt(1) == ">" || (tok1->previous()->isName() &&
                                                  typeParameterNames.find(tok1->strAt(-1)) == typeParameterNames.end()))) {
                const Token *endLink = tok1->findClosingBracket();
                do {
                    instantiationArgs[index].push_back(tok1);
                    tok1 = tok1->next();
                } while (tok1 && tok1 != endLink);
                instantiationArgs[index].push_back(tok1);
            } else if (tok1->str() == ",") {
                ++index;
                instantiationArgs.resize(index + 1);
            } else
                instantiationArgs[index].push_back(tok1);
        }

        // count the parameters..
        Token *tok = instantiation.token()->next();
        unsigned int usedpar = templateParameters(tok);
        Token *instantiationEnd = tok->findClosingBracket();
        tok = instantiationEnd;

        if (tok && tok->str() == ">") {
            tok = tok->previous();
            std::list<Token *>::const_iterator it = eq.begin();
            for (std::size_t i = (templatepar - eq.size()); it != eq.end() && i < usedpar; ++i)
                ++it;
            int count = 0;
            while (it != eq.end()) {
                int indentlevel = 0;
                if ((usedpar + count) && usedpar <= (instantiationArgs.size() + count)) {
                    tok->insertToken(",");
                    tok = tok->next();
                }
                const Token *from = (*it)->next();
                std::stack<Token *> links;
                while (from && (!links.empty() || indentlevel || !Token::Match(from, ",|>"))) {
                    if (from->str() == "<" &&
                        (from->strAt(1) == ">" || (from->previous()->isName() &&
                                                   typeParameterNames.find(from->strAt(-1)) == typeParameterNames.end())))
                        ++indentlevel;
                    else if (from->str() == ">" && (links.empty() || links.top()->str() == "<" || indentlevel))
                        --indentlevel;
                    auto entry = typeParameterNames.find(from->str());
                    if (entry != typeParameterNames.end() && entry->second < instantiationArgs.size()) {
                        for (const Token *tok1 : instantiationArgs[entry->second]) {
                            tok->insertToken(tok1->str(), tok1->originalName());
                            tok = tok->next();

                            if (Token::Match(tok, "(|[|{"))
                                links.push(tok);
                            else if (!links.empty() && Token::Match(tok, ")|]|}")) {
                                Token::createMutualLinks(links.top(), tok);
                                links.pop();
                            }
                        }
                    } else {
                        tok->insertToken(from->str(), from->originalName());
                        tok = tok->next();

                        if (Token::Match(tok, "(|[|{"))
                            links.push(tok);
                        else if (!links.empty() && Token::Match(tok, ")|]|}")) {
                            Token::createMutualLinks(links.top(), tok);
                            links.pop();
                        }
                    }
                    from = from->next();
                }
                ++it;
                count++;
                usedpar++;
            }
        }

        simplifyTemplateArgs(instantiation.token()->next(), instantiationEnd);
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
            else if (Token::Match(tok2, "%type% <") && (tok2->strAt(2) == ">" || templateParameters(tok2->next()))) {
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
                                                FindName(declaration.name()));

        if (ti2 == mTemplateInstantiations.end())
            continue;

        eraseTokens(eqtok, tok2);
        eqtok->deleteThis();

        // update parameter end pointer
        declaration.paramEnd(declaration.token()->next()->findClosingBracket());
    }
}

void TemplateSimplifier::simplifyTemplateAliases()
{
    for (std::list<TokenAndName>::iterator it1 = mTemplateDeclarations.begin(); it1 != mTemplateDeclarations.end();) {
        TokenAndName &aliasDeclaration = *it1;

        if (!aliasDeclaration.isAlias()) {
            ++it1;
            continue;
        }

        // alias parameters..
        std::vector<const Token *> aliasParameters;
        getTemplateParametersInDeclaration(aliasDeclaration.token()->tokAt(2), aliasParameters);
        std::map<std::string, unsigned int> aliasParameterNames;
        for (unsigned int argnr = 0; argnr < aliasParameters.size(); ++argnr)
            aliasParameterNames[aliasParameters[argnr]->str()] = argnr;

        // Look for alias usages..
        bool found = false;
        for (std::list<TokenAndName>::iterator it2 = mTemplateInstantiations.begin(); it2 != mTemplateInstantiations.end();) {
            TokenAndName &aliasUsage = *it2;
            if (!aliasUsage.token() || aliasUsage.fullName() != aliasDeclaration.fullName()) {
                ++it2;
                continue;
            }

            // don't recurse
            if (aliasDeclaration.isAliasToken(aliasUsage.token())) {
                ++it2;
                continue;
            }

            std::vector<std::pair<Token *, Token *>> args;
            Token *tok2 = aliasUsage.token()->tokAt(2);
            while (tok2) {
                Token * const start = tok2;
                while (tok2 && !Token::Match(tok2, "[,>;{}]")) {
                    if (tok2->link() && Token::Match(tok2, "(|["))
                        tok2 = tok2->link();
                    else if (tok2->str() == "<") {
                        tok2 = tok2->findClosingBracket();
                        if (!tok2)
                            break;
                    }
                    tok2 = tok2->next();
                }

                args.emplace_back(start, tok2);
                if (tok2 && tok2->str() == ",") {
                    tok2 = tok2->next();
                } else {
                    break;
                }
            }
            if (!tok2 || tok2->str() != ">" ||
                (!aliasDeclaration.isVariadic() && (args.size() != aliasParameters.size())) ||
                (aliasDeclaration.isVariadic() && (args.size() < aliasParameters.size()))) {
                ++it2;
                continue;
            }

            mChanged = true;

            // copy template-id from declaration to after instantiation
            Token * dst = aliasUsage.token()->next()->findClosingBracket();
            Token * end = TokenList::copyTokens(dst, aliasDeclaration.aliasStartToken(), aliasDeclaration.aliasEndToken()->previous(), false)->next();

            // replace parameters
            for (Token *tok1 = dst->next(); tok1 != end; tok1 = tok1->next()) {
                if (!tok1->isName())
                    continue;
                if (aliasParameterNames.find(tok1->str()) != aliasParameterNames.end()) {
                    const unsigned int argnr = aliasParameterNames[tok1->str()];
                    const Token * const fromStart = args[argnr].first;
                    const Token * const fromEnd   = args[argnr].second->previous();
                    Token *temp = TokenList::copyTokens(tok1, fromStart, fromEnd, true);
                    const bool tempOK(temp && temp != tok1->next());
                    tok1->deleteThis();
                    if (tempOK)
                        tok1 = temp; // skip over inserted parameters
                } else if (tok1->str() == "typename")
                    tok1->deleteThis();
            }

            // add new instantiations
            for (Token *tok1 = dst->next(); tok1 != end; tok1 = tok1->next()) {
                if (!tok1->isName())
                    continue;
                if (aliasParameterNames.find(tok2->str()) == aliasParameterNames.end()) {
                    // Create template instance..
                    if (Token::Match(tok1, "%name% <")) {
                        const std::list<TokenAndName>::iterator it = std::find_if(mTemplateInstantiations.begin(),
                                mTemplateInstantiations.end(),
                                FindToken(tok1));
                        if (it != mTemplateInstantiations.end())
                            addInstantiation(tok2, it->scope());
                    }
                }
            }

            // erase the instantiation tokens
            eraseTokens(aliasUsage.token()->previous(), dst->next());
            found = true;

            // erase this instantiation
            it2 = mTemplateInstantiations.erase(it2);
        }

        if (found) {
            Token *end = const_cast<Token *>(aliasDeclaration.aliasEndToken());

            // remove declaration tokens
            if (aliasDeclaration.token()->previous())
                eraseTokens(aliasDeclaration.token()->previous(), end->next() ? end->next() : end);
            else {
                eraseTokens(mTokenList.front(), end->next() ? end->next() : end);
                deleteToken(mTokenList.front());
            }

            // remove declaration
            it1 = mTemplateDeclarations.erase(it1);
        } else
            ++it1;
    }
}

bool TemplateSimplifier::instantiateMatch(const Token *instance, const std::size_t numberOfArguments, const char patternAfter[])
{
    assert(instance->strAt(1) == "<");

    if (numberOfArguments != templateParameters(instance->next()))
        return false;

    if (patternAfter) {
        const Token *tok = instance->next()->findClosingBracket();
        if (!tok || !Token::Match(tok->next(), patternAfter))
            return false;
    }

    // nothing mismatching was found..
    return true;
}

// Utility function for TemplateSimplifier::getTemplateNamePosition, that works on template functions
bool TemplateSimplifier::getTemplateNamePositionTemplateFunction(const Token *tok, int &namepos)
{
    namepos = 1;
    while (tok && tok->next()) {
        if (Token::Match(tok->next(), ";|{"))
            return false;
        // skip decltype(...)
        else if (Token::simpleMatch(tok->next(), "decltype (")) {
            const Token * end = tok->linkAt(2)->previous();
            while (tok->next() && tok != end) {
                tok = tok->next();
                namepos++;
            }
        } else if (Token::Match(tok->next(), "%type% <")) {
            const Token *closing = tok->tokAt(2)->findClosingBracket();
            if (closing) {
                if (closing->strAt(1) == "(" && Tokenizer::isFunctionHead(closing->next(), ";|{|:", true))
                    return true;
                while (tok->next() && tok->next() != closing) {
                    tok = tok->next();
                    namepos++;
                }
            }
        } else if (Token::Match(tok->next(), "%type% (") && Tokenizer::isFunctionHead(tok->tokAt(2), ";|{|:", true)) {
            return true;
        }
        tok = tok->next();
        namepos++;
    }
    return false;
}

bool TemplateSimplifier::getTemplateNamePositionTemplateVariable(const Token *tok, int &namepos)
{
    namepos = 1;
    while (tok && tok->next()) {
        if (Token::Match(tok->next(), ";|{|(|using"))
            return false;
        // skip decltype(...)
        else if (Token::simpleMatch(tok->next(), "decltype (")) {
            const Token * end = tok->linkAt(2);
            while (tok->next() && tok != end) {
                tok = tok->next();
                namepos++;
            }
        } else if (Token::Match(tok->next(), "%type% <")) {
            const Token *closing = tok->tokAt(2)->findClosingBracket();
            if (closing) {
                if (Token::Match(closing->next(), "=|;"))
                    return true;
                while (tok->next() && tok->next() != closing) {
                    tok = tok->next();
                    namepos++;
                }
            }
        } else if (Token::Match(tok->next(), "%type% =|;")) {
            return true;
        }
        tok = tok->next();
        namepos++;
    }
    return false;
}

bool TemplateSimplifier::getTemplateNamePositionTemplateClass(const Token *tok, int &namepos)
{
    if (Token::Match(tok, "> friend| class|struct|union %type% :|<|;|{|::")) {
        namepos = tok->strAt(1) == "friend" ? 3 : 2;
        tok = tok->tokAt(namepos);
        while (Token::Match(tok, "%type% :: %type%") ||
               (Token::Match(tok, "%type% <") && Token::Match(tok->next()->findClosingBracket(), "> :: %type%"))) {
            if (tok->strAt(1) == "::") {
                tok = tok->tokAt(2);
                namepos += 2;
            } else {
                const Token *end = tok->next()->findClosingBracket();
                if (!end || !end->tokAt(2)) {
                    // syntax error
                    namepos = -1;
                    return true;
                }
                end = end->tokAt(2);
                do {
                    tok = tok->next();
                    namepos += 1;
                } while (tok && tok != end);
            }
        }
        return true;
    }
    return false;
}

int TemplateSimplifier::getTemplateNamePosition(const Token *tok)
{
    assert(tok && tok->str() == ">");

    auto it = mTemplateNamePos.find(tok);
    if (!mSettings->debugtemplate && it != mTemplateNamePos.end()) {
        return it->second;
    }
    // get the position of the template name
    int namepos = 0;
    if (getTemplateNamePositionTemplateClass(tok, namepos))
        ;
    else if (Token::Match(tok, "> using %name% =")) {
        // types may not be defined in alias template declarations
        if (!Token::Match(tok->tokAt(4), "class|struct|union|enum %name%| {"))
            namepos = 2;
    } else if (getTemplateNamePositionTemplateVariable(tok, namepos))
        ;
    else if (!getTemplateNamePositionTemplateFunction(tok, namepos))
        namepos = -1; // Name not found
    mTemplateNamePos[tok] = namepos;
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
    bool inTemplate = false;
    int level = 0;
    while ((end = templateDeclaration.scope().find(" ", start)) != std::string::npos) {
        std::string token = templateDeclaration.scope().substr(start, end - start);
        // done if scopes overlap
        if (token == tokStart->str() && tok->strAt(-1) != "::")
            break;
        if (token == "<") {
            inTemplate = true;
            ++level;
        }
        if (inTemplate) {
            if (insert)
                mTokenList.back()->tokAt(offset)->str(mTokenList.back()->strAt(offset) + token);
            else
                mTokenList.back()->str(mTokenList.back()->str() + token);
            if (token == ">") {
                --level;
                if (level == 0)
                    inTemplate = false;
            }
        } else {
            if (insert)
                mTokenList.back()->tokAt(offset)->insertToken(token, "");
            else
                mTokenList.addtoken(token, tok->linenr(), tok->column(), tok->fileIndex());
        }
        start = end + 1;
    }
    // don't add if it already exists
    std::string token = templateDeclaration.scope().substr(start, end - start);
    if (token != tokStart->str() || tok->strAt(-1) != "::") {
        if (insert) {
            if (!inTemplate)
                mTokenList.back()->tokAt(offset)->insertToken(templateDeclaration.scope().substr(start), "");
            else
                mTokenList.back()->tokAt(offset)->str(mTokenList.back()->strAt(offset) + templateDeclaration.scope().substr(start));
            mTokenList.back()->tokAt(offset)->insertToken("::", "");
        } else {
            if (!inTemplate)
                mTokenList.addtoken(templateDeclaration.scope().substr(start), tok->linenr(), tok->column(), tok->fileIndex());
            else
                mTokenList.back()->str(mTokenList.back()->str() + templateDeclaration.scope().substr(start));
            mTokenList.addtoken("::", tok->linenr(), tok->column(), tok->fileIndex());
        }
    }
}

bool TemplateSimplifier::alreadyHasNamespace(const TokenAndName &templateDeclaration, const Token *tok)
{
    std::string scope = templateDeclaration.scope();

    // get the length in tokens of the namespace
    std::string::size_type pos = 0;
    int offset = -2;

    while ((pos = scope.find("::", pos)) != std::string::npos) {
        offset -= 2;
        pos += 2;
    }

    return Token::simpleMatch(tok->tokAt(offset), scope.c_str(), scope.size());
}

void TemplateSimplifier::expandTemplate(
    const TokenAndName &templateDeclaration,
    const TokenAndName &templateInstantiation,
    const std::vector<const Token *> &typeParametersInDeclaration,
    const std::string &newName,
    bool copy)
{
    bool inTemplateDefinition = false;
    const Token *startOfTemplateDeclaration = nullptr;
    const Token *endOfTemplateDefinition = nullptr;
    const Token * const templateDeclarationNameToken = templateDeclaration.nameToken();
    const Token * const templateDeclarationToken = templateDeclaration.paramEnd();
    const bool isClass = templateDeclaration.isClass();
    const bool isFunction = templateDeclaration.isFunction();
    const bool isSpecialization = templateDeclaration.isSpecialization();
    const bool isVariable = templateDeclaration.isVariable();
    struct newInstantiation {
        newInstantiation(Token *t, const std::string &s) : token(t), scope(s) { }
        Token *token;
        std::string scope;
    };
    std::vector<newInstantiation> newInstantiations;

    // add forward declarations
    if (copy && isClass) {
        templateDeclaration.token()->insertToken(templateDeclarationToken->strAt(1), "", true);
        templateDeclaration.token()->insertToken(newName, "", true);
        templateDeclaration.token()->insertToken(";", "", true);
    } else if ((isFunction && (copy || isSpecialization)) ||
               (isVariable && !isSpecialization) ||
               (isClass && isSpecialization && mTemplateSpecializationMap.find(templateDeclaration.token()) != mTemplateSpecializationMap.end())) {
        Token * dst = templateDeclaration.token();
        Token * dstStart = dst->previous();
        bool isStatic = false;
        std::string scope;
        Token * start;
        Token * end;
        auto it = mTemplateForwardDeclarationsMap.find(dst);
        if (it != mTemplateForwardDeclarationsMap.end()) {
            dst = it->second;
            dstStart = dst->previous();
            const Token * temp1 = dst->tokAt(1)->findClosingBracket();
            const Token * temp2 = temp1->tokAt(getTemplateNamePosition(temp1));
            start = temp1->next();
            end = temp2->linkAt(1)->next();
        } else {
            auto it2 = mTemplateSpecializationMap.find(dst);
            if (it2 != mTemplateSpecializationMap.end()) {
                dst = it2->second;
                dstStart = dst->previous();
                isStatic = dst->next()->findClosingBracket()->strAt(1) == "static";
                const Token * temp = templateDeclarationNameToken;
                while (Token::Match(temp->tokAt(-2), "%name% ::")) {
                    scope.insert(0, temp->strAt(-2) + " :: ");
                    temp = temp->tokAt(-2);
                }
            }
            start = templateDeclarationToken->next();
            end = templateDeclarationNameToken->next();
            if (end->str() == "<")
                end = end->findClosingBracket()->next();
            if (end->str() == "(")
                end = end->link()->next();
            else if (isVariable && end->str() == "=") {
                Token *temp = end->next();
                while (temp && temp->str() != ";") {
                    if (temp->link() && Token::Match(temp, "{|[|("))
                        temp = temp->link();
                    temp = temp->next();
                }
                end = temp;
            }
        }
        unsigned int typeindentlevel = 0;
        while (end && !(typeindentlevel == 0 && Token::Match(end, ";|{|:"))) {
            if (Token::Match(end, "<|(|{"))
                ++typeindentlevel;
            else if (Token::Match(end, ">|)|}"))
                --typeindentlevel;
            end = end->next();
        }

        if (isStatic) {
            dst->insertToken("static", "", true);
            if (start) {
                dst->previous()->linenr(start->linenr());
                dst->previous()->column(start->column());
            }
        }

        std::map<const Token *, Token *> links;
        bool inAssignment = false;
        while (start && start != end) {
            if (isVariable && start->str() == "=")
                inAssignment = true;
            unsigned int itype = 0;
            while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != start->str())
                ++itype;

            if (itype < typeParametersInDeclaration.size() &&
                (!isVariable || !Token::Match(typeParametersInDeclaration[itype]->previous(), "<|, %type% >|,"))) {
                typeindentlevel = 0;
                std::stack<Token *> brackets1; // holds "(" and "{" tokens
                bool pointerType = false;
                Token * const dst1 = dst->previous();
                for (const Token *typetok = mTypesUsedInTemplateInstantiation[itype].token();
                     typetok && (typeindentlevel > 0 || !Token::Match(typetok, ",|>"));
                     typetok = typetok->next()) {
                    if (typeindentlevel == 0 && typetok->str() == "*")
                        pointerType = true;
                    if (Token::simpleMatch(typetok, "..."))
                        continue;
                    if (Token::Match(typetok, "%name% <") && (typetok->strAt(2) == ">" || templateParameters(typetok->next())))
                        ++typeindentlevel;
                    else if (typeindentlevel > 0 && typetok->str() == ">")
                        --typeindentlevel;
                    else if (typetok->str() == "(")
                        ++typeindentlevel;
                    else if (typetok->str() == ")")
                        --typeindentlevel;
                    dst->insertToken(typetok->str(), typetok->originalName(), true);
                    dst->previous()->linenr(start->linenr());
                    dst->previous()->column(start->column());
                    Token *previous = dst->previous();
                    previous->isTemplateArg(true);
                    previous->isSigned(typetok->isSigned());
                    previous->isUnsigned(typetok->isUnsigned());
                    previous->isLong(typetok->isLong());
                    if (Token::Match(previous, "{|(|[")) {
                        brackets1.push(previous);
                    } else if (previous->str() == "}") {
                        assert(brackets1.empty() == false);
                        assert(brackets1.top()->str() == "{");
                        Token::createMutualLinks(brackets1.top(), previous);
                        brackets1.pop();
                    } else if (previous->str() == ")") {
                        assert(brackets1.empty() == false);
                        assert(brackets1.top()->str() == "(");
                        Token::createMutualLinks(brackets1.top(), previous);
                        brackets1.pop();
                    } else if (previous->str() == "]") {
                        assert(brackets1.empty() == false);
                        assert(brackets1.top()->str() == "[");
                        Token::createMutualLinks(brackets1.top(), previous);
                        brackets1.pop();
                    }
                }
                if (pointerType && Token::simpleMatch(dst1, "const")) {
                    dst->insertToken("const", dst1->originalName(), true);
                    dst->previous()->linenr(start->linenr());
                    dst->previous()->column(start->column());
                    dst1->deleteThis();
                }
            } else {
                if (isSpecialization && !copy && !scope.empty() && Token::Match(start, (scope + templateDeclarationNameToken->str()).c_str())) {
                    // skip scope
                    while (start->strAt(1) != templateDeclarationNameToken->str())
                        start = start->next();
                } else if (start->str() == templateDeclarationNameToken->str() &&
                           !(templateDeclaration.isFunction() && templateDeclaration.scope().empty() &&
                             (start->strAt(-1) == "." || Token::simpleMatch(start->tokAt(-2), ". template")))) {
                    if (start->strAt(1) != "<" || Token::Match(start, newName.c_str()) || !inAssignment) {
                        dst->insertToken(newName, "", true);
                        dst->previous()->linenr(start->linenr());
                        dst->previous()->column(start->column());
                        if (start->strAt(1) == "<")
                            start = start->next()->findClosingBracket();
                    } else {
                        dst->insertToken(start->str(), "", true);
                        dst->previous()->linenr(start->linenr());
                        dst->previous()->column(start->column());
                        newInstantiations.emplace_back(dst->previous(), templateDeclaration.scope());
                    }
                } else {
                    // check if type is a template
                    if (start->strAt(1) == "<") {
                        // get the instantiated name
                        Token * closing = start->next()->findClosingBracket();
                        if (closing) {
                            std::string name;
                            const Token * type = start;
                            while (type && type != closing->next()) {
                                if (!name.empty())
                                    name += " ";
                                name += type->str();
                                type = type->next();
                            }
                            // check if type is instantiated
                            for (const auto & inst : mTemplateInstantiations) {
                                if (Token::simpleMatch(inst.token(), name.c_str(), name.size())) {
                                    // use the instantiated name
                                    dst->insertToken(name, "", true);
                                    dst->previous()->linenr(start->linenr());
                                    dst->previous()->column(start->column());
                                    start = closing;
                                    break;
                                }
                            }
                        }
                        // just copy the token if it wasn't instantiated
                        if (start != closing) {
                            dst->insertToken(start->str(), start->originalName(), true);
                            dst->previous()->linenr(start->linenr());
                            dst->previous()->column(start->column());
                            dst->previous()->isSigned(start->isSigned());
                            dst->previous()->isUnsigned(start->isUnsigned());
                            dst->previous()->isLong(start->isLong());
                        }
                    } else {
                        dst->insertToken(start->str(), start->originalName(), true);
                        dst->previous()->linenr(start->linenr());
                        dst->previous()->column(start->column());
                        dst->previous()->isSigned(start->isSigned());
                        dst->previous()->isUnsigned(start->isUnsigned());
                        dst->previous()->isLong(start->isLong());
                    }
                }

                if (!start)
                    continue;

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
        dst->previous()->linenr(dst->tokAt(-2)->linenr());
        dst->previous()->column(dst->tokAt(-2)->column() + 1);

        if (isVariable || isFunction)
            simplifyTemplateArgs(dstStart, dst);
    }

    if (copy && (isClass || isFunction)) {
        // check if this is an explicit instantiation
        Token * start = templateInstantiation.token();
        while (start && !Token::Match(start->previous(), "}|;|extern"))
            start = start->previous();
        if (Token::Match(start, "template !!<")) {
            if (start->strAt(-1) == "extern")
                start = start->previous();
            mExplicitInstantiationsToDelete.emplace_back(start, "");
        }
    }

    for (Token *tok3 = mTokenList.front(); tok3; tok3 = tok3 ? tok3->next() : nullptr) {
        if (inTemplateDefinition) {
            if (!endOfTemplateDefinition) {
                if (isVariable) {
                    Token *temp = tok3->findClosingBracket();
                    if (temp) {
                        while (temp && temp->str() != ";") {
                            if (temp->link() && Token::Match(temp, "{|[|("))
                                temp = temp->link();
                            temp = temp->next();
                        }
                        endOfTemplateDefinition = temp;
                    }
                } else if (tok3->str() == "{")
                    endOfTemplateDefinition = tok3->link();
            }
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
            if (tok3->str() == "static")
                tok3 = tok3->next();
        }

        // member function implemented outside class definition
        else if (inTemplateDefinition &&
                 Token::Match(tok3, "%name% <") &&
                 templateInstantiation.name() == tok3->str() &&
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
            std::stack<Token *> brackets2; // holds "(" and "{" tokens
            while (tok5 && tok5 != tok3) {
                // replace name if found
                if (Token::Match(tok5, "%name% <") && tok5->str() == templateInstantiation.name()) {
                    if (copy) {
                        if (!templateDeclaration.scope().empty() && tok5->strAt(-1) != "::")
                            addNamespace(templateDeclaration, tok5);
                        mTokenList.addtoken(newName, tok5->linenr(), tok5->column(), tok5->fileIndex());
                        tok5 = tok5->next()->findClosingBracket();
                    } else {
                        tok5->str(newName);
                        eraseTokens(tok5, tok5->next()->findClosingBracket()->next());
                    }
                } else if (copy) {
                    bool added = false;
                    if (tok5->isName() && !Token::Match(tok5, "class|typename|struct") && !tok5->isStandardType()) {
                        // search for this token in the type vector
                        unsigned int itype = 0;
                        while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != tok5->str())
                            ++itype;

                        // replace type with given type..
                        if (itype < typeParametersInDeclaration.size()) {
                            unsigned int typeindentlevel = 0;
                            std::stack<Token *> brackets1; // holds "(" and "{" tokens
                            for (const Token *typetok = mTypesUsedInTemplateInstantiation[itype].token();
                                 typetok && (typeindentlevel>0 || !Token::Match(typetok, ",|>"));
                                 typetok = typetok->next()) {
                                if (!Token::simpleMatch(typetok, "...")) {
                                    if (Token::Match(typetok, "%name% <") && (typetok->strAt(2) == ">" || templateParameters(typetok->next())))
                                        ++typeindentlevel;
                                    else if (typeindentlevel > 0 && typetok->str() == ">")
                                        --typeindentlevel;
                                    else if (typetok->str() == "(")
                                        ++typeindentlevel;
                                    else if (typetok->str() == ")")
                                        --typeindentlevel;
                                    mTokenList.addtoken(typetok, tok5);
                                    Token *back = mTokenList.back();
                                    if (Token::Match(back, "{|(|[")) {
                                        brackets1.push(back);
                                    } else if (back->str() == "}") {
                                        assert(brackets1.empty() == false);
                                        assert(brackets1.top()->str() == "{");
                                        Token::createMutualLinks(brackets1.top(), back);
                                        brackets1.pop();
                                    } else if (back->str() == ")") {
                                        assert(brackets1.empty() == false);
                                        assert(brackets1.top()->str() == "(");
                                        Token::createMutualLinks(brackets1.top(), back);
                                        brackets1.pop();
                                    } else if (back->str() == "]") {
                                        assert(brackets1.empty() == false);
                                        assert(brackets1.top()->str() == "[");
                                        Token::createMutualLinks(brackets1.top(), back);
                                        brackets1.pop();
                                    }
                                    back->isTemplateArg(true);
                                    added = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!added) {
                        mTokenList.addtoken(tok5);
                        Token *back = mTokenList.back();
                        if (Token::Match(back, "{|(|[")) {
                            brackets2.push(back);
                        } else if (back->str() == "}") {
                            assert(brackets2.empty() == false);
                            assert(brackets2.top()->str() == "{");
                            Token::createMutualLinks(brackets2.top(), back);
                            brackets2.pop();
                        } else if (back->str() == ")") {
                            assert(brackets2.empty() == false);
                            assert(brackets2.top()->str() == "(");
                            Token::createMutualLinks(brackets2.top(), back);
                            brackets2.pop();
                        } else if (back->str() == "]") {
                            assert(brackets2.empty() == false);
                            assert(brackets2.top()->str() == "[");
                            Token::createMutualLinks(brackets2.top(), back);
                            brackets2.pop();
                        }
                    }
                }

                tok5 = tok5->next();
            }
            if (copy) {
                if (!templateDeclaration.scope().empty() && tok3->strAt(-1) != "::")
                    addNamespace(templateDeclaration, tok3);
                mTokenList.addtoken(newName, tok3->linenr(), tok3->column(), tok3->fileIndex());
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
        const std::string lastName = (templateInstantiation.name().find(' ') != std::string::npos) ? templateInstantiation.name().substr(templateInstantiation.name().rfind(' ')+1) : templateInstantiation.name();

        std::stack<const Token *> templates;
        for (; tok3; tok3 = tok3->next()) {
            if (tok3->isName() && !Token::Match(tok3, "class|typename|struct") && !tok3->isStandardType()) {
                // search for this token in the type vector
                unsigned int itype = 0;
                while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != tok3->str())
                    ++itype;

                // replace type with given type..
                if (itype < typeParametersInDeclaration.size()) {
                    unsigned int typeindentlevel = 0;
                    std::stack<Token *> brackets1; // holds "(" and "{" tokens
                    Token * const beforeTypeToken = mTokenList.back();
                    bool pointerType = false;
                    for (const Token *typetok = mTypesUsedInTemplateInstantiation[itype].token();
                         typetok && (typeindentlevel > 0 || !Token::Match(typetok, ",|>"));
                         typetok = typetok->next()) {
                        if (typeindentlevel == 0 && typetok->str() == "*")
                            pointerType = true;
                        if (Token::simpleMatch(typetok, "..."))
                            continue;
                        if (Token::Match(typetok, "%name% <") &&
                            (typetok->strAt(2) == ">" || templateParameters(typetok->next()))) {
                            brackets1.push(typetok->next());
                            ++typeindentlevel;
                        } else if (typeindentlevel > 0 && typetok->str() == ">" && brackets1.top()->str() == "<") {
                            --typeindentlevel;
                            brackets1.pop();
                        } else if (Token::Match(typetok, "const_cast|dynamic_cast|reinterpret_cast|static_cast <")) {
                            brackets1.push(typetok->next());
                            ++typeindentlevel;
                        } else if (typetok->str() == "(")
                            ++typeindentlevel;
                        else if (typetok->str() == ")")
                            --typeindentlevel;
                        Token *back;
                        if (copy) {
                            mTokenList.addtoken(typetok, tok3);
                            back = mTokenList.back();
                        } else
                            back = const_cast<Token *>(typetok);
                        if (Token::Match(back, "{|(|["))
                            brackets1.push(back);
                        else if (back->str() == "}") {
                            assert(brackets1.empty() == false);
                            assert(brackets1.top()->str() == "{");
                            if (copy)
                                Token::createMutualLinks(brackets1.top(), back);
                            brackets1.pop();
                        } else if (back->str() == ")") {
                            assert(brackets1.empty() == false);
                            assert(brackets1.top()->str() == "(");
                            if (copy)
                                Token::createMutualLinks(brackets1.top(), back);
                            brackets1.pop();
                        } else if (back->str() == "]") {
                            assert(brackets1.empty() == false);
                            assert(brackets1.top()->str() == "[");
                            if (copy)
                                Token::createMutualLinks(brackets1.top(), back);
                            brackets1.pop();
                        }
                        if (copy)
                            back->isTemplateArg(true);
                    }
                    if (pointerType && Token::simpleMatch(beforeTypeToken, "const")) {
                        mTokenList.addtoken(beforeTypeToken);
                        beforeTypeToken->deleteThis();
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
                                mTokenList.addtoken(newName, tok3);
                                tok3 = closingBracket;
                            } else {
                                tok3->str(newName);
                                eraseTokens(tok3, closingBracket->next());
                            }
                            continue;
                        } else if (!templateDeclaration.scope().empty() &&
                                   !alreadyHasNamespace(templateDeclaration, tok3) &&
                                   !Token::Match(closingBracket->next(), "(|::")) {
                            if (copy)
                                addNamespace(templateDeclaration, tok3);
                        }
                    }
                } else {
                    // don't modify friend
                    if (Token::Match(tok3->tokAt(-3), "> friend class|struct|union")) {
                        if (copy)
                            mTokenList.addtoken(tok3);
                    } else if (copy) {
                        // add namespace if necessary
                        if (!templateDeclaration.scope().empty() &&
                            (isClass ? tok3->strAt(1) != "(" : true)) {
                            addNamespace(templateDeclaration, tok3);
                        }
                        mTokenList.addtoken(newName, tok3);
                    } else if (!Token::Match(tok3->next(), ":|{|=|;"))
                        tok3->str(newName);
                    continue;
                }
            }

            // copy
            if (copy)
                mTokenList.addtoken(tok3);

            // look for template definitions
            if (Token::simpleMatch(tok3, "template <")) {
                Token * tok2 = findTemplateDeclarationEnd(tok3);
                if (tok2)
                    templates.push(tok2);
            } else if (!templates.empty() && templates.top() == tok3)
                templates.pop();

            if (Token::Match(tok3, "%type% <") &&
                !Token::Match(tok3, "template|static_cast|const_cast|reinterpret_cast|dynamic_cast") &&
                Token::Match(tok3->next()->findClosingBracket(), ">|>>")) {
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

                std::string scope;
                const Token *prev = tok3;
                for (; Token::Match(prev->tokAt(-2), "%name% ::"); prev = prev->tokAt(-2)) {
                    if (scope.empty())
                        scope = prev->strAt(-2);
                    else
                        scope = prev->strAt(-2) + " :: " + scope;
                }

                // check for global scope
                if (prev->strAt(-1) != "::") {
                    // adjust for current scope
                    std::string token_scope = tok3->scopeInfo()->name;
                    std::string::size_type end = token_scope.find_last_of(" :: ");
                    if (end != std::string::npos) {
                        token_scope.resize(end);
                        if (scope.empty())
                            scope = token_scope;
                        else
                            scope = token_scope + " :: " + scope;
                    }
                }

                // don't add instantiations in template definitions
                if (templates.empty()) {
                    if (copy)
                        newInstantiations.emplace_back(mTokenList.back(), scope);
                    else if (!inTemplateDefinition)
                        newInstantiations.emplace_back(tok3, scope);
                }
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
                    assert(brackets.empty() == false);
                    assert(brackets.top()->str() == "{");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    if (tok3->strAt(1) == ";") {
                        const Token * tokSemicolon = tok3->next();
                        mTokenList.addtoken(tokSemicolon, tokSemicolon->linenr(), tokSemicolon->column(), tokSemicolon->fileIndex());
                    }
                    brackets.pop();
                    if (brackets.empty() && !Token::Match(tok3, "} >|,|{|%cop%")) {
                        inTemplateDefinition = false;
                        break;
                    }
                } else if (tok3->str() == ")") {
                    assert(brackets.empty() == false);
                    assert(brackets.top()->str() == "(");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    brackets.pop();
                } else if (tok3->str() == "]") {
                    assert(brackets.empty() == false);
                    assert(brackets.top()->str() == "[");
                    Token::createMutualLinks(brackets.top(), mTokenList.back());
                    brackets.pop();
                }
            }
        }

        assert(brackets.empty());
    }

    // add new instantiations
    for (const auto & inst : newInstantiations) {
        std::string fullName = inst.scope + (inst.scope.empty() ? "" : " :: ") + inst.token->str();
        simplifyTemplateArgs(inst.token->tokAt(2), inst.token->next()->findClosingBracket());
        // only add recursive instantiation if its arguments are a constant expression
        if (templateDeclaration.fullName() != fullName ||
            (inst.token->tokAt(2)->isNumber() || inst.token->tokAt(2)->isStandardType()))
            mTemplateInstantiations.emplace_back(inst.token, inst.scope);
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


bool TemplateSimplifier::simplifyNumericCalculations(Token *tok, bool isTemplate)
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
        if (Token::Match(op, "[/%] 0")) {
            if (isTemplate) throw InternalError(op, "Instantiation error: Divide by zero in template instantiation.", InternalError::INSTANTIATION);
            else return ret;
        }

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
            }
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

static Token *skipTernaryOp(Token *tok, Token *backToken)
{
    unsigned int colonLevel = 1;
    while (nullptr != (tok = tok->next())) {
        if (tok->str() == "?") {
            ++colonLevel;
        } else if (tok->str() == ":") {
            --colonLevel;
            if (colonLevel == 0) {
                tok = tok->next();
                break;
            }
        }
        if (tok->link() && tok->str() == "(")
            tok = tok->link();
        else if (Token::Match(tok->next(), "[{};)]") || tok->next() == backToken)
            break;
    }
    if (colonLevel > 0) // Ticket #5214: Make sure the ':' matches the proper '?'
        return nullptr;
    return tok;
}

void TemplateSimplifier::simplifyTemplateArgs(Token *start, Token *end)
{
    // start could be erased so use the token before start if available
    Token * first = (start && start->previous()) ? start->previous() : mTokenList.front();
    bool again = true;

    while (again) {
        again = false;

        for (Token *tok = first->next(); tok && tok != end; tok = tok->next()) {
            if (tok->str() == "sizeof") {
                // sizeof('x')
                if (Token::Match(tok->next(), "( %char% )")) {
                    tok->deleteNext();
                    tok->deleteThis();
                    tok->deleteNext();
                    std::ostringstream sz;
                    sz << 1;
                    tok->str(sz.str());
                    again = true;
                }

                // sizeof ("text")
                else if (Token::Match(tok->next(), "( %str% )")) {
                    tok->deleteNext();
                    tok->deleteThis();
                    tok->deleteNext();
                    std::ostringstream ostr;
                    ostr << (Token::getStrLength(tok) + 1);
                    tok->str(ostr.str());
                    again = true;
                }

                else if (Token::Match(tok->next(), "( %type% * )")) {
                    tok->str(MathLib::toString(mTokenizer->sizeOfType(tok->tokAt(3))));
                    tok->deleteNext(4);
                    again = true;
                } else if (Token::simpleMatch(tok->next(), "( * )")) {
                    tok->str(MathLib::toString(mTokenizer->sizeOfType(tok->tokAt(2))));
                    tok->deleteNext(3);
                    again = true;
                } else if (Token::Match(tok->next(), "( %type% )")) {
                    const unsigned int size = mTokenizer->sizeOfType(tok->tokAt(2));
                    if (size > 0) {
                        tok->str(MathLib::toString(size));
                        tok->deleteNext(3);
                        again = true;
                    }
                } else if (tok->strAt(1) == "(") {
                    tok = tok->linkAt(1);
                }
            } else if (Token::Match(tok, "%num% %comp% %num%") &&
                       MathLib::isInt(tok->str()) &&
                       MathLib::isInt(tok->strAt(2))) {
                if ((Token::Match(tok->previous(), "(|&&|%oror%|,") || tok == start) &&
                    (Token::Match(tok->tokAt(3), ")|&&|%oror%|?") || tok->tokAt(3) == end)) {
                    const MathLib::bigint op1(MathLib::toLongNumber(tok->str()));
                    const std::string &cmp(tok->next()->str());
                    const MathLib::bigint op2(MathLib::toLongNumber(tok->strAt(2)));

                    std::string result;

                    if (cmp == "==")
                        result = (op1 == op2) ? "true" : "false";
                    else if (cmp == "!=")
                        result = (op1 != op2) ? "true" : "false";
                    else if (cmp == "<=")
                        result = (op1 <= op2) ? "true" : "false";
                    else if (cmp == ">=")
                        result = (op1 >= op2) ? "true" : "false";
                    else if (cmp == "<")
                        result = (op1 < op2) ? "true" : "false";
                    else
                        result = (op1 > op2) ? "true" : "false";

                    tok->str(result);
                    tok->deleteNext(2);
                    again = true;
                    tok = tok->previous();
                }
            }
        }

        if (simplifyCalculations(first->next(), end))
            again = true;

        for (Token *tok = first->next(); tok && tok != end; tok = tok->next()) {
            if (tok->str() == "?" &&
                ((tok->previous()->isNumber() || tok->previous()->isBoolean()) ||
                 Token::Match(tok->tokAt(-3), "( %bool%|%num% )"))) {
                const int offset = (tok->previous()->str() == ")") ? 2 : 1;

                // Find the token ":" then go to the next token
                Token *colon = skipTernaryOp(tok, end);
                if (!colon || colon->previous()->str() != ":" || !colon->next())
                    continue;

                //handle the GNU extension: "x ? : y" <-> "x ? x : y"
                if (colon->previous() == tok->next())
                    tok->insertToken(tok->strAt(-offset));

                // go back before the condition, if possible
                tok = tok->tokAt(-2);
                if (offset == 2) {
                    // go further back before the "("
                    tok = tok->tokAt(-2);
                    //simplify the parentheses
                    tok->deleteNext();
                    tok->next()->deleteNext();
                }

                if (Token::Match(tok->next(), "false|0")) {
                    // Use code after colon, remove code before it.
                    Token::eraseTokens(tok, colon);

                    tok = tok->next();
                    again = true;
                }

                // The condition is true. Delete the operator after the ":"..
                else {
                    // delete the condition token and the "?"
                    tok->deleteNext(2);

                    unsigned int ternaryOplevel = 0;
                    for (const Token *endTok = colon; endTok; endTok = endTok->next()) {
                        if (Token::Match(endTok, "(|[|{"))
                            endTok = endTok->link();
                        else if (endTok->str() == "<" && (endTok->strAt(1) == ">" || templateParameters(endTok)))
                            endTok = endTok->findClosingBracket();
                        else if (endTok->str() == "?")
                            ++ternaryOplevel;
                        else if (Token::Match(endTok, ")|}|]|;|,|:|>")) {
                            if (endTok->str() == ":" && ternaryOplevel)
                                --ternaryOplevel;
                            else if (endTok->str() == ">" && !end)
                                ;
                            else {
                                Token::eraseTokens(colon->tokAt(-2), endTok);
                                again = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        for (Token *tok = first->next(); tok && tok != end; tok = tok->next()) {
            if (Token::Match(tok, "( %num%|%bool% )") &&
                (tok->previous() && !Token::Match(tok->previous(), "%name%"))) {
                tok->deleteThis();
                tok->deleteNext();
                again = true;
            }
        }
    }
}

static bool validTokenStart(bool bounded, const Token *tok, const Token *frontToken, int offset)
{
    if (!bounded)
        return true;

    if (frontToken)
        frontToken = frontToken->previous();

    while (tok && offset <= 0) {
        if (tok == frontToken)
            return false;
        ++offset;
        tok = tok->previous();
    }

    return tok && offset > 0;
}

static bool validTokenEnd(bool bounded, const Token *tok, const Token *backToken, int offset)
{
    if (!bounded)
        return true;

    while (tok && offset >= 0) {
        if (tok == backToken)
            return false;
        --offset;
        tok = tok->next();
    }

    return tok && offset < 0;
}

// TODO: This is not the correct class for simplifyCalculations(), so it
// should be moved away.
bool TemplateSimplifier::simplifyCalculations(Token* frontToken, Token *backToken, bool isTemplate)
{
    bool ret = false;
    const bool bounded = frontToken || backToken;
    if (!frontToken) {
        frontToken = mTokenList.front();
    }
    for (Token *tok = frontToken; tok && tok != backToken; tok = tok->next()) {
        // Remove parentheses around variable..
        // keep parentheses here: dynamic_cast<Fred *>(p);
        // keep parentheses here: A operator * (int);
        // keep parentheses here: int ( * ( * f ) ( ... ) ) (int) ;
        // keep parentheses here: int ( * * ( * compilerHookVector ) (void) ) ( ) ;
        // keep parentheses here: operator new [] (size_t);
        // keep parentheses here: Functor()(a ... )
        // keep parentheses here: ) ( var ) ;
        if (validTokenEnd(bounded, tok, backToken, 4) &&
            (Token::Match(tok->next(), "( %name% ) ;|)|,|]") ||
             (Token::Match(tok->next(), "( %name% ) %cop%") &&
              (tok->tokAt(2)->varId()>0 ||
               !Token::Match(tok->tokAt(4), "[*&+-~]")))) &&
            !tok->isName() &&
            tok->str() != ">" &&
            tok->str() != ")" &&
            tok->str() != "]") {
            tok->deleteNext();
            tok = tok->next();
            tok->deleteNext();
            ret = true;
        }

        if (validTokenEnd(bounded, tok, backToken, 3) &&
            Token::Match(tok->previous(), "(|&&|%oror% %char% %comp% %num% &&|%oror%|)")) {
            tok->str(MathLib::toString(MathLib::toLongNumber(tok->str())));
        }

        if (validTokenEnd(bounded, tok, backToken, 5) &&
            Token::Match(tok, "decltype ( %type% { } )")) {
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
            ret = true;
        }

        if (validTokenEnd(bounded, tok, backToken, 3) &&
            Token::Match(tok, "decltype ( %bool%|%num% )")) {
            tok->deleteThis();
            tok->deleteThis();
            if (tok->isBoolean())
                tok->str("bool");
            else if (MathLib::isFloat(tok->str())) {
                // MathLib::getSuffix doesn't work for floating point numbers
                char suffix = tok->str().back();
                if (suffix == 'f' || suffix == 'F')
                    tok->str("float");
                else if (suffix == 'l' || suffix == 'L') {
                    tok->str("double");
                    tok->isLong(true);
                } else
                    tok->str("double");
            } else if (MathLib::isInt(tok->str())) {
                std::string suffix = MathLib::getSuffix(tok->str());
                if (suffix.find("LL") != std::string::npos) {
                    tok->str("long");
                    tok->isLong(true);
                } else if (suffix.find('L') != std::string::npos)
                    tok->str("long");
                else
                    tok->str("int");
                tok->isUnsigned(suffix.find('U') != std::string::npos);
            }
            tok->deleteNext();
            ret = true;
        }

        if (validTokenEnd(bounded, tok, backToken, 2) &&
            (Token::Match(tok, "char|short|int|long { }") ||
             Token::Match(tok, "char|short|int|long ( )"))) {
            tok->str("0"); // FIXME add type suffix
            tok->isSigned(false);
            tok->isUnsigned(false);
            tok->isLong(false);
            tok->deleteNext();
            tok->deleteNext();
            ret = true;
        }

        if (tok && tok->isNumber()) {
            if (validTokenEnd(bounded, tok, backToken, 2) &&
                simplifyNumericCalculations(tok, isTemplate)) {
                ret = true;
                Token *prev = tok->tokAt(-2);
                while (validTokenStart(bounded, tok, frontToken, -2) &&
                       prev && simplifyNumericCalculations(prev, isTemplate)) {
                    tok = prev;
                    prev = prev->tokAt(-2);
                }
            }

            // Remove redundant conditions (0&&x) (1||x)
            if (validTokenStart(bounded, tok, frontToken, -1) &&
                validTokenEnd(bounded, tok, backToken, 1) &&
                (Token::Match(tok->previous(), "[(=,] 0 &&") ||
                 Token::Match(tok->previous(), "[(=,] 1 %oror%"))) {
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

            if (tok->str() == "0" && validTokenStart(bounded, tok, frontToken, -1)) {
                if (validTokenEnd(bounded, tok, backToken, 1) &&
                    ((Token::Match(tok->previous(), "[+-] 0 %cop%|;") && isLowerThanMulDiv(tok->next())) ||
                     (Token::Match(tok->previous(), "%or% 0 %cop%|;") && isLowerThanXor(tok->next())))) {
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
                } else if (validTokenEnd(bounded, tok, backToken, 1) &&
                           (Token::Match(tok->previous(), "[=([,] 0 [+|]") ||
                            Token::Match(tok->previous(), "return|case 0 [+|]"))) {
                    tok = tok->previous();
                    tok->deleteNext(2);
                    ret = true;
                } else if ((((Token::Match(tok->previous(), "[=[(,] 0 * %name%|%num% ,|]|)|;|=|%cop%") ||
                              Token::Match(tok->previous(), "return|case 0 *|&& %name%|%num% ,|:|;|=|%cop%")) &&
                             validTokenEnd(bounded, tok, backToken, 3)) ||
                            (((Token::Match(tok->previous(), "[=[(,] 0 * (") ||
                               Token::Match(tok->previous(), "return|case 0 *|&& (")) &&
                              validTokenEnd(bounded, tok, backToken, 2))))) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (validTokenEnd(bounded, tok, backToken, 4) &&
                           (Token::Match(tok->previous(), "[=[(,] 0 && *|& %any% ,|]|)|;|=|%cop%") ||
                            Token::Match(tok->previous(), "return|case 0 && *|& %any% ,|:|;|=|%cop%"))) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (tok->str() == "1" && validTokenStart(bounded, tok, frontToken, -1)) {
                if (validTokenEnd(bounded, tok, backToken, 3) &&
                    (Token::Match(tok->previous(), "[=[(,] 1 %oror% %any% ,|]|)|;|=|%cop%") ||
                     Token::Match(tok->previous(), "return|case 1 %oror% %any% ,|:|;|=|%cop%"))) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                } else if (validTokenEnd(bounded, tok, backToken, 4) &&
                           (Token::Match(tok->previous(), "[=[(,] 1 %oror% *|& %any% ,|]|)|;|=|%cop%") ||
                            Token::Match(tok->previous(), "return|case 1 %oror% *|& %any% ,|:|;|=|%cop%"))) {
                    tok->deleteNext();
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if ((Token::Match(tok->tokAt(-2), "%any% * 1") &&
                 validTokenStart(bounded, tok, frontToken, -2)) ||
                (Token::Match(tok->previous(), "%any% 1 *") &&
                 validTokenStart(bounded, tok, frontToken, -1))) {
                tok = tok->previous();
                if (tok->str() == "*")
                    tok = tok->previous();
                tok->deleteNext(2);
                ret = true;
            }

            // Remove parentheses around number..
            if (validTokenStart(bounded, tok, frontToken, -2) &&
                Token::Match(tok->tokAt(-2), "%op%|< ( %num% )") &&
                tok->strAt(-2) != ">") {
                tok = tok->previous();
                tok->deleteThis();
                tok->deleteNext();
                ret = true;
            }

            if (validTokenStart(bounded, tok, frontToken, -1) &&
                validTokenEnd(bounded, tok, backToken, 1) &&
                (Token::Match(tok->previous(), "( 0 [|+]") ||
                 Token::Match(tok->previous(), "[|+-] 0 )"))) {
                tok = tok->previous();
                if (Token::Match(tok, "[|+-]"))
                    tok = tok->previous();
                tok->deleteNext(2);
                ret = true;
            }

            if (validTokenEnd(bounded, tok, backToken, 2) &&
                Token::Match(tok, "%num% %comp% %num%") &&
                MathLib::isInt(tok->str()) &&
                MathLib::isInt(tok->strAt(2))) {
                if (validTokenStart(bounded, tok, frontToken, -1) &&
                    Token::Match(tok->previous(), "(|&&|%oror%") &&
                    Token::Match(tok->tokAt(3), ")|&&|%oror%|?")) {
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
    assert(tok->strAt(-1) == "<");

    typeParametersInDeclaration.clear();
    const Token *end = tok->previous()->findClosingBracket();
    for (; tok && tok!= end; tok = tok->next()) {
        if (Token::simpleMatch(tok, "template <")) {
            const Token *closing = tok->next()->findClosingBracket();
            if (closing)
                tok = closing->next();
        } else if (tok->link() && Token::Match(tok, "{|(|["))
            tok = tok->link();
        else if (Token::Match(tok, "%name% ,|>|="))
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
        if (!endToken)
            continue;
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

        if (declToken && instToken && declToken == endToken && instToken->str() == ">") {
            // specialization matches.
            return templateDeclarationNameToken == *it;
        }
    }

    // No specialization matches. Return true if the declaration is not a specialization.
    return Token::Match(templateDeclarationNameToken, "%name% !!<") &&
           (templateDeclarationNameToken->str().find('<') == std::string::npos);
}

std::string TemplateSimplifier::getNewName(
    Token *tok2,
    std::list<std::string> &typeStringsUsedInTemplateInstantiation)
{
    std::string typeForNewName;
    unsigned int indentlevel = 0;
    const Token * endToken = tok2->next()->findClosingBracket();
    for (Token *tok3 = tok2->tokAt(2); tok3 != endToken && (indentlevel > 0 || tok3->str() != ">"); tok3 = tok3->next()) {
        // #2721 - unhandled [ => bail out
        if (tok3->str() == "[") {
            typeForNewName.clear();
            break;
        }
        if (!tok3->next()) {
            typeForNewName.clear();
            break;
        }
        if (Token::Match(tok3->tokAt(-2), "<|,|:: %name% <") && (tok3->strAt(1) == ">" || templateParameters(tok3)))
            ++indentlevel;
        else if (indentlevel > 0 && Token::Match(tok3, "> [,>]"))
            --indentlevel;
        if (indentlevel == 0 && Token::Match(tok3->previous(), "[<,]")) {
            mTypesUsedInTemplateInstantiation.emplace_back(tok3, "");
        }
        if (tok3->str() == "(")
            ++indentlevel;
        else if (tok3->str() == ")")
            --indentlevel;
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
    getTemplateParametersInDeclaration(templateDeclaration.token()->tokAt(2), typeParametersInDeclaration);
    const bool printDebug = mSettings->debugwarnings;
    const bool specialized = templateDeclaration.isSpecialization();
    const bool isfunc = templateDeclaration.isFunction();
    const bool isVar = templateDeclaration.isVariable();

    // locate template usage..
    std::string::size_type numberOfTemplateInstantiations = mTemplateInstantiations.size();
    unsigned int recursiveCount = 0;

    bool instantiated = false;

    for (const TokenAndName &instantiation : mTemplateInstantiations) {
        // skip deleted instantiations
        if (!instantiation.token())
            continue;
        if (numberOfTemplateInstantiations != mTemplateInstantiations.size()) {
            numberOfTemplateInstantiations = mTemplateInstantiations.size();
            ++recursiveCount;
            if (recursiveCount > mSettings->maxTemplateRecursion) {
                std::list<std::string> typeStringsUsedInTemplateInstantiation;
                const std::string typeForNewName = templateDeclaration.name() + "<" + getNewName(instantiation.token(), typeStringsUsedInTemplateInstantiation) + ">";

                const std::list<const Token *> callstack(1, instantiation.token());
                const ErrorMessage errmsg(callstack,
                                          &mTokenizer->list,
                                          Severity::information,
                                          "templateRecursion",
                                          "TemplateSimplifier: max template recursion ("
                                          + MathLib::toString(mSettings->maxTemplateRecursion)
                                          + ") reached for template '"+typeForNewName+"'. You might want to limit Cppcheck recursion.",
                                          false);
                if (mErrorLogger && mSettings->isEnabled(Settings::INFORMATION))
                    mErrorLogger->reportErr(errmsg);

                // bail out..
                break;
            }
        }

        // already simplified
        if (!Token::Match(instantiation.token(), "%name% <"))
            continue;

        if (!((instantiation.fullName() == templateDeclaration.fullName()) ||
              (instantiation.name() == templateDeclaration.name() &&
               instantiation.fullName() == templateDeclaration.scope()))) {
            // FIXME: fallback to not matching scopes until type deduction works

            // names must match
            if (instantiation.name() != templateDeclaration.name())
                continue;

            // scopes must match when present
            if (!instantiation.scope().empty() && !templateDeclaration.scope().empty())
                continue;
        }

        // make sure constructors and destructors don't match each other
        if (templateDeclaration.nameToken()->strAt(-1) == "~" && instantiation.token()->strAt(-1) != "~")
            continue;

        // template families should match
        if (!instantiation.isFunction() && templateDeclaration.isFunction()) {
            // there are exceptions
            if (!Token::simpleMatch(instantiation.token()->tokAt(-2), "decltype ("))
                continue;
        }

        if (templateDeclaration.isFunction() && instantiation.isFunction()) {
            std::vector<const Token*> declFuncArgs;
            getFunctionArguments(templateDeclaration.nameToken(), declFuncArgs);
            std::vector<const Token*> instFuncParams;
            getFunctionArguments(instantiation.token(), instFuncParams);

            if (declFuncArgs.size() != instFuncParams.size()) {
                // check for default arguments
                const Token* tok = templateDeclaration.nameToken()->tokAt(2);
                const Token* end = templateDeclaration.nameToken()->linkAt(1);
                size_t count = 0;
                for (; tok != end; tok = tok->next()) {
                    if (tok->str() == "=")
                        count++;
                }

                if (instFuncParams.size() < (declFuncArgs.size() - count) || instFuncParams.size() > declFuncArgs.size())
                    continue;
            }
        }

        // A global function can't be called through a pointer.
        if (templateDeclaration.isFunction() && templateDeclaration.scope().empty() &&
            (instantiation.token()->strAt(-1) == "." ||
             Token::simpleMatch(instantiation.token()->tokAt(-2), ". template")))
            continue;

        if (!matchSpecialization(templateDeclaration.nameToken(), instantiation.token(), specializations))
            continue;

        Token * const tok2 = instantiation.token();
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
        while (Token::Match(startToken->tokAt(-2), ">|%name% :: %name%")) {
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
            (!specialized && !instantiateMatch(tok2, typeParametersInDeclaration.size(), isfunc ? "(" : isVar ? ";|%op%|(" : "*|&|::| %name%")))
            continue;

        // New type..
        mTypesUsedInTemplateInstantiation.clear();
        std::list<std::string> typeStringsUsedInTemplateInstantiation;
        std::string typeForNewName = getNewName(tok2, typeStringsUsedInTemplateInstantiation);

        if ((typeForNewName.empty() && !templateDeclaration.isVariadic()) ||
            (!typeParametersInDeclaration.empty() && typeParametersInDeclaration.size() != mTypesUsedInTemplateInstantiation.size())) {
            if (printDebug && mErrorLogger) {
                std::list<const Token *> callstack(1, tok2);
                mErrorLogger->reportErr(ErrorMessage(callstack, &mTokenList, Severity::debug, "debug",
                                                     "Failed to instantiate template \"" + instantiation.name() + "\". The checking continues anyway.", false));
            }
            if (typeForNewName.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string newName(templateDeclaration.name() + " < " + typeForNewName + " >");
        const std::string newFullName(templateDeclaration.scope() + (templateDeclaration.scope().empty() ? "" : " :: ") + newName);

        if (expandedtemplates.insert(newFullName).second) {
            expandTemplate(templateDeclaration, instantiation, typeParametersInDeclaration, newName, !specialized && !isVar);
            instantiated = true;
            mChanged = true;
        }

        // Replace all these template usages..
        replaceTemplateUsage(instantiation, typeStringsUsedInTemplateInstantiation, newName);
    }

    // process uninstantiated templates
    // TODO: remove the specialized check and handle all uninstantiated templates someday.
    if (!instantiated && specialized) {
        Token * tok2 = const_cast<Token *>(templateDeclaration.nameToken());
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
        while (Token::Match(startToken->tokAt(-2), ">|%name% :: %name%")) {
            if (startToken->strAt(-2) == ">") {
                const Token * tok3 = startToken->tokAt(-2)->findOpeningBracket();
                if (tok3)
                    startToken = tok3->previous();
                else
                    break;
            } else
                startToken = startToken->tokAt(-2);
        }

        // TODO: re-enable when specialized check is removed
        // if (Token::Match(startToken->previous(), ";|{|}|=|const") &&
        //     (!specialized && !instantiateMatch(tok2, typeParametersInDeclaration.size(), isfunc ? "(" : isVar ? ";|%op%|(" : "*|&|::| %name%")))
        //     return false;

        // already simplified
        if (!Token::Match(tok2, "%name% <"))
            return false;

        if (!matchSpecialization(templateDeclaration.nameToken(), tok2, specializations))
            return false;

        // New type..
        mTypesUsedInTemplateInstantiation.clear();
        std::list<std::string> typeStringsUsedInTemplateInstantiation;
        std::string typeForNewName = getNewName(tok2, typeStringsUsedInTemplateInstantiation);

        if (typeForNewName.empty()) {
            if (printDebug && mErrorLogger) {
                std::list<const Token *> callstack(1, tok2);
                mErrorLogger->reportErr(ErrorMessage(callstack, &mTokenList, Severity::debug, "debug",
                                                     "Failed to instantiate template \"" + templateDeclaration.name() + "\". The checking continues anyway.", false));
            }
            return false;
        }

        // New classname/funcname..
        const std::string newName(templateDeclaration.name() + " < " + typeForNewName + " >");
        const std::string newFullName(templateDeclaration.scope() + (templateDeclaration.scope().empty() ? "" : " :: ") + newName);

        if (expandedtemplates.insert(newFullName).second) {
            expandTemplate(templateDeclaration, templateDeclaration, typeParametersInDeclaration, newName, !specialized && !isVar);
            instantiated = true;
            mChanged = true;
        }

        // Replace all these template usages..
        replaceTemplateUsage(templateDeclaration, typeStringsUsedInTemplateInstantiation, newName);
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
    const TokenAndName &instantiation,
    const std::list<std::string> &typeStringsUsedInTemplateInstantiation,
    const std::string &newName)
{
    std::list< std::pair<Token *, Token *> > removeTokens;
    for (Token *nameTok = mTokenList.front(); nameTok; nameTok = nameTok->next()) {
        if (!Token::Match(nameTok, "%name% <") ||
            Token::Match(nameTok, "template|const_cast|dynamic_cast|reinterpret_cast|static_cast"))
            continue;

        std::set<TemplateSimplifier::TokenAndName*>* pointers = nameTok->templateSimplifierPointers();

        // check if instantiation matches token instantiation from pointer
        if (pointers && pointers->size()) {
            // check full name
            if (instantiation.fullName() != (*pointers->begin())->fullName()) {
                // FIXME:  fallback to just matching name
                if (nameTok->str() != instantiation.name())
                    continue;
            }
        }
        // no pointer available look at tokens directly
        else {
            // FIXME:  fallback to just matching name
            if (nameTok->str() != instantiation.name())
                continue;
        }

        if (!matchTemplateParameters(nameTok, typeStringsUsedInTemplateInstantiation))
            continue;

        // match parameters
        Token * tok2 = nameTok->tokAt(2);
        const Token * endToken = nameTok->next()->findClosingBracket();
        unsigned int typeCountInInstantiation = tok2->str() == ">" ? 0U : 1U;
        const Token *typetok = (!mTypesUsedInTemplateInstantiation.empty()) ? mTypesUsedInTemplateInstantiation[0].token() : nullptr;
        unsigned int indentlevel2 = 0;  // indentlevel for tokgt
        while (tok2 != endToken && (indentlevel2 > 0 || tok2->str() != ">")) {
            if (tok2->str() == "<" && (tok2->strAt(1) == ">" || templateParameters(tok2)))
                ++indentlevel2;
            else if (tok2->str() == "(")
                ++indentlevel2;
            else if (tok2->str() == ")")
                --indentlevel2;
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
                        typetok = mTypesUsedInTemplateInstantiation[typeCountInInstantiation++].token();
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

            for (Token *tok = nameTok1->next(); tok != tok2; tok = tok->next()) {
                if (tok->isName() && tok->templateSimplifierPointers() && !tok->templateSimplifierPointers()->empty()) {
                    std::list<TokenAndName>::iterator ti;
                    for (ti = mTemplateInstantiations.begin(); ti != mTemplateInstantiations.end();) {
                        if (ti->token() == tok) {
                            mTemplateInstantiations.erase(ti++);
                            break;
                        } else {
                            ++ti;
                        }
                    }
                }
            }
            // Fix crash in #9007
            if (Token::simpleMatch(nameTok->previous(), ">"))
                mTemplateNamePos.erase(nameTok->previous());
            removeTokens.emplace_back(nameTok, tok2->next());
        }

        nameTok = tok2;
    }
    while (!removeTokens.empty()) {
        eraseTokens(removeTokens.back().first, removeTokens.back().second);
        removeTokens.pop_back();
    }
}

static bool specMatch(
    const TemplateSimplifier::TokenAndName &spec,
    const TemplateSimplifier::TokenAndName &decl)
{
    // make sure decl is really a declaration
    if (decl.isPartialSpecialization() || decl.isSpecialization() || decl.isAlias() || decl.isFriend())
        return false;

    if (!spec.isSameFamily(decl))
        return false;

    // make sure the scopes and names match
    if (spec.fullName() == decl.fullName()) {
        if (spec.isFunction()) {
            std::vector<const Token*> specArgs;
            std::vector<const Token*> declArgs;
            getFunctionArguments(spec.nameToken(), specArgs);
            getFunctionArguments(decl.nameToken(), declArgs);

            if (specArgs.size() == declArgs.size()) {
                // @todo make sure function parameters also match
                return true;
            }
        } else
            return true;
    }

    return false;
}

void TemplateSimplifier::getSpecializations()
{
    // try to locate a matching declaration for each user defined specialization
    for (auto & spec : mTemplateDeclarations) {
        if (spec.isSpecialization()) {
            bool found = false;
            for (auto & decl : mTemplateDeclarations) {
                if (specMatch(spec, decl)) {
                    mTemplateSpecializationMap[spec.token()] = decl.token();
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (auto & decl : mTemplateForwardDeclarations) {
                    if (specMatch(spec, decl)) {
                        mTemplateSpecializationMap[spec.token()] = decl.token();
                        break;
                    }
                }
            }
        }
    }
}

void TemplateSimplifier::getPartialSpecializations()
{
    // try to locate a matching declaration for each user defined partial specialization
    for (auto & spec : mTemplateDeclarations) {
        if (spec.isPartialSpecialization()) {
            bool found = false;
            for (auto & decl : mTemplateDeclarations) {
                if (specMatch(spec, decl)) {
                    mTemplatePartialSpecializationMap[spec.token()] = decl.token();
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (auto & decl : mTemplateForwardDeclarations) {
                    if (specMatch(spec, decl)) {
                        mTemplatePartialSpecializationMap[spec.token()] = decl.token();
                        break;
                    }
                }
            }
        }
    }
}

void TemplateSimplifier::fixForwardDeclaredDefaultArgumentValues()
{
    // try to locate a matching declaration for each forward declaration
    for (const auto & forwardDecl : mTemplateForwardDeclarations) {
        std::vector<const Token *> params1;

        getTemplateParametersInDeclaration(forwardDecl.token()->tokAt(2), params1);

        for (auto & decl : mTemplateDeclarations) {
            // skip partializations, type aliases and friends
            if (decl.isPartialSpecialization() || decl.isAlias() || decl.isFriend())
                continue;

            std::vector<const Token *> params2;

            getTemplateParametersInDeclaration(decl.token()->tokAt(2), params2);

            // make sure the number of arguments match
            if (params1.size() == params2.size()) {
                // make sure the scopes and names match
                if (forwardDecl.fullName() == decl.fullName()) {
                    // save forward declaration for lookup later
                    if ((decl.nameToken()->strAt(1) == "(" && forwardDecl.nameToken()->strAt(1) == "(") ||
                        (decl.nameToken()->strAt(1) == "{" && forwardDecl.nameToken()->strAt(1) == ";")) {
                        mTemplateForwardDeclarationsMap[decl.token()] = forwardDecl.token();
                    }

                    for (size_t k = 0; k < params1.size(); k++) {
                        // copy default value to declaration if not present
                        if (params1[k]->strAt(1) == "=" && params2[k]->strAt(1) != "=") {
                            int level = 0;
                            const Token *end = params1[k]->next();
                            while (end && !(level == 0 && Token::Match(end, ",|>"))) {
                                if (Token::Match(end, "{|(|<"))
                                    level++;
                                else if (Token::Match(end, "}|)|>"))
                                    level--;
                                end = end->next();
                            }
                            if (end)
                                TokenList::copyTokens(const_cast<Token *>(params2[k]), params1[k]->next(), end->previous());
                            break;
                        }
                    }

                    // update parameter end pointer
                    decl.paramEnd(decl.token()->next()->findClosingBracket());
                }
            }
        }
    }
}

void TemplateSimplifier::printOut(const TokenAndName &tokenAndName, const std::string &indent) const
{
    std::cout << indent << "token: ";
    if (tokenAndName.token())
        std::cout << "\"" << tokenAndName.token()->str() << "\" " << mTokenList.fileLine(tokenAndName.token());
    else
        std::cout << "nullptr";
    std::cout << std::endl;
    std::cout << indent << "scope: \"" << tokenAndName.scope() << "\"" << std::endl;
    std::cout << indent << "name: \"" << tokenAndName.name() << "\"" << std::endl;
    std::cout << indent << "fullName: \"" << tokenAndName.fullName() << "\"" << std::endl;
    std::cout << indent << "nameToken: ";
    if (tokenAndName.nameToken())
        std::cout << "\"" << tokenAndName.nameToken()->str() << "\" " << mTokenList.fileLine(tokenAndName.nameToken());
    else
        std::cout << "nullptr";
    std::cout << std::endl;
    std::cout << indent << "paramEnd: ";
    if (tokenAndName.paramEnd())
        std::cout << "\"" << tokenAndName.paramEnd()->str() << "\" " << mTokenList.fileLine(tokenAndName.paramEnd());
    else
        std::cout << "nullptr";
    std::cout << std::endl;
    std::cout << indent << "flags: ";
    if (tokenAndName.isClass())
        std::cout << " isClass";
    if (tokenAndName.isFunction())
        std::cout << " isFunction";
    if (tokenAndName.isVariable())
        std::cout << " isVariable";
    if (tokenAndName.isAlias())
        std::cout << " isAlias";
    if (tokenAndName.isSpecialization())
        std::cout << " isSpecialization";
    if (tokenAndName.isPartialSpecialization())
        std::cout << " isPartialSpecialization";
    if (tokenAndName.isForwardDeclaration())
        std::cout << " isForwardDeclaration";
    if (tokenAndName.isVariadic())
        std::cout << " isVariadic";
    if (tokenAndName.isFriend())
        std::cout << " isFriend";
    std::cout << std::endl;
    if (tokenAndName.token() && !tokenAndName.paramEnd() && tokenAndName.token()->strAt(1) == "<") {
        const Token *end = tokenAndName.token()->next()->findClosingBracket();
        if (end) {
            const Token *start = tokenAndName.token()->next();
            std::cout << indent << "type: ";
            while (start && start != end) {
                std::cout << start->str();
                start = start->next();
            }
            std::cout << end->str() << std::endl;
        }
    } else if (tokenAndName.isAlias() && tokenAndName.paramEnd()) {
        if (tokenAndName.aliasStartToken()) {
            std::cout << indent << "aliasStartToken: \"" << tokenAndName.aliasStartToken()->str() << "\" "
                      << mTokenList.fileLine(tokenAndName.aliasStartToken()) << std::endl;
        }
        if (tokenAndName.aliasEndToken()) {
            std::cout << indent << "aliasEndToken: \"" << tokenAndName.aliasEndToken()->str() << "\" "
                      << mTokenList.fileLine(tokenAndName.aliasEndToken()) << std::endl;
        }
    }
}

void TemplateSimplifier::printOut(const std::string & text) const
{
    std::cout << std::endl;
    std::cout << text << std::endl;
    std::cout << std::endl;
    std::cout << "mTemplateDeclarations: " << mTemplateDeclarations.size() << std::endl;
    int count = 0;
    for (const auto & decl : mTemplateDeclarations) {
        std::cout << "mTemplateDeclarations[" << count++ << "]:" << std::endl;
        printOut(decl);
    }
    std::cout << "mTemplateForwardDeclarations: " << mTemplateForwardDeclarations.size() << std::endl;
    count = 0;
    for (const auto & decl : mTemplateForwardDeclarations) {
        std::cout << "mTemplateForwardDeclarations[" << count++ << "]:" << std::endl;
        printOut(decl);
    }
    std::cout << "mTemplateForwardDeclarationsMap: " << mTemplateForwardDeclarationsMap.size() << std::endl;
    unsigned int mapIndex = 0;
    for (const auto & mapItem : mTemplateForwardDeclarationsMap) {
        unsigned int declIndex = 0;
        for (const auto & decl : mTemplateDeclarations) {
            if (mapItem.first == decl.token()) {
                unsigned int forwardIndex = 0;
                for (const auto & forwardDecl : mTemplateForwardDeclarations) {
                    if (mapItem.second == forwardDecl.token()) {
                        std::cout << "mTemplateForwardDeclarationsMap[" << mapIndex << "]:" << std::endl;
                        std::cout << "    mTemplateDeclarations[" << declIndex
                                  << "] => mTemplateForwardDeclarations[" << forwardIndex << "]" << std::endl;
                        break;
                    }
                    forwardIndex++;
                }
                break;
            }
            declIndex++;
        }
        mapIndex++;
    }
    std::cout << "mTemplateSpecializationMap: " << mTemplateSpecializationMap.size() << std::endl;
    for (const auto & mapItem : mTemplateSpecializationMap) {
        unsigned int decl1Index = 0;
        for (const auto & decl1 : mTemplateDeclarations) {
            if (decl1.isSpecialization() && mapItem.first == decl1.token()) {
                bool found = false;
                unsigned int decl2Index = 0;
                for (const auto & decl2 : mTemplateDeclarations) {
                    if (mapItem.second == decl2.token()) {
                        std::cout << "mTemplateSpecializationMap[" << mapIndex << "]:" << std::endl;
                        std::cout << "    mTemplateDeclarations[" << decl1Index
                                  << "] => mTemplateDeclarations[" << decl2Index << "]" << std::endl;
                        found = true;
                        break;
                    }
                    decl2Index++;
                }
                if (!found) {
                    decl2Index = 0;
                    for (const auto & decl2 : mTemplateForwardDeclarations) {
                        if (mapItem.second == decl2.token()) {
                            std::cout << "mTemplateSpecializationMap[" << mapIndex << "]:" << std::endl;
                            std::cout << "    mTemplateDeclarations[" << decl1Index
                                      << "] => mTemplateForwardDeclarations[" << decl2Index << "]" << std::endl;
                            break;
                        }
                        decl2Index++;
                    }
                }
                break;
            }
            decl1Index++;
        }
        mapIndex++;
    }
    std::cout << "mTemplatePartialSpecializationMap: " << mTemplatePartialSpecializationMap.size() << std::endl;
    for (const auto & mapItem : mTemplatePartialSpecializationMap) {
        unsigned int decl1Index = 0;
        for (const auto & decl1 : mTemplateDeclarations) {
            if (mapItem.first == decl1.token()) {
                bool found = false;
                unsigned int decl2Index = 0;
                for (const auto & decl2 : mTemplateDeclarations) {
                    if (mapItem.second == decl2.token()) {
                        std::cout << "mTemplatePartialSpecializationMap[" << mapIndex << "]:" << std::endl;
                        std::cout << "    mTemplateDeclarations[" << decl1Index
                                  << "] => mTemplateDeclarations[" << decl2Index << "]" << std::endl;
                        found = true;
                        break;
                    }
                    decl2Index++;
                }
                if (!found) {
                    decl2Index = 0;
                    for (const auto & decl2 : mTemplateForwardDeclarations) {
                        if (mapItem.second == decl2.token()) {
                            std::cout << "mTemplatePartialSpecializationMap[" << mapIndex << "]:" << std::endl;
                            std::cout << "    mTemplateDeclarations[" << decl1Index
                                      << "] => mTemplateForwardDeclarations[" << decl2Index << "]" << std::endl;
                            break;
                        }
                        decl2Index++;
                    }
                }
                break;
            }
            decl1Index++;
        }
        mapIndex++;
    }
    std::cout << "mTemplateInstantiations: " << mTemplateInstantiations.size() << std::endl;
    count = 0;
    for (const auto & decl : mTemplateInstantiations) {
        std::cout << "mTemplateInstantiations[" << count++ << "]:" << std::endl;
        printOut(decl);
    }
}

void TemplateSimplifier::simplifyTemplates(
    const std::time_t maxtime,
    bool &codeWithTemplates)
{
    // convert "sizeof ..." to "sizeof..."
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "sizeof ...")) {
            tok->str("sizeof...");
            tok->deleteNext();
        }
    }

    // Remove "typename" unless used in template arguments or using type alias..
    for (Token *tok = mTokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "typename %name%") && !Token::Match(tok->tokAt(-3), "using %name% ="))
            tok->deleteThis();

        if (Token::simpleMatch(tok, "template <")) {
            tok = tok->next()->findClosingBracket();
            if (!tok)
                break;
        }
    }

    mTokenizer->calculateScopes();

    unsigned int passCount = 0;
    const unsigned int passCountMax = 10;
    for (; passCount < passCountMax; ++passCount) {
        if (passCount) {
            // it may take more than one pass to simplify type aliases
            bool usingChanged = false;
            while (mTokenizer->simplifyUsing())
                usingChanged = true;

            if (!usingChanged && !mChanged)
                break;

            mChanged = usingChanged;
            mTemplateDeclarations.clear();
            mTemplateForwardDeclarations.clear();
            mTemplateForwardDeclarationsMap.clear();
            mTemplateSpecializationMap.clear();
            mTemplatePartialSpecializationMap.clear();
            mTemplateInstantiations.clear();
            mInstantiatedTemplates.clear();
            mExplicitInstantiationsToDelete.clear();
            mTemplateNamePos.clear();
        }

        bool hasTemplates = getTemplateDeclarations();

        if (passCount == 0)
            codeWithTemplates = hasTemplates;

        // Make sure there is something to simplify.
        if (mTemplateDeclarations.empty() && mTemplateForwardDeclarations.empty())
            return;

        if (mSettings->debugtemplate && mSettings->debugnormal) {
            std::string title("Template Simplifier pass " + std::to_string(passCount + 1));
            mTokenList.front()->printOut(title.c_str(), mTokenList.getFiles());
        }

        // Copy default argument values from forward declaration to declaration
        fixForwardDeclaredDefaultArgumentValues();

        // Locate user defined specializations.
        getSpecializations();

        // Locate user defined partial specializations.
        getPartialSpecializations();

        // Locate possible instantiations of templates..
        getTemplateInstantiations();

        // Template arguments with default values
        useDefaultArgumentValues();

        simplifyTemplateAliases();

        if (mSettings->debugtemplate)
            printOut("### Template Simplifier pass " + std::to_string(passCount + 1) + " ###");

        std::set<std::string> expandedtemplates;

        for (std::list<TokenAndName>::reverse_iterator iter1 = mTemplateDeclarations.rbegin(); iter1 != mTemplateDeclarations.rend(); ++iter1) {
            if (iter1->isAlias() || iter1->isFriend())
                continue;

            // get specializations..
            std::list<const Token *> specializations;
            for (std::list<TokenAndName>::const_iterator iter2 = mTemplateDeclarations.begin(); iter2 != mTemplateDeclarations.end(); ++iter2) {
                if (iter2->isAlias() || iter2->isFriend())
                    continue;

                if (iter1->fullName() == iter2->fullName())
                    specializations.push_back(iter2->nameToken());
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
                if (decl->token() == it->token())
                    break;
            }
            if (decl != mTemplateDeclarations.end()) {
                if (it->isSpecialization()) {
                    // delete the "template < >"
                    Token * tok = it->token();
                    tok->deleteNext(2);
                    tok->deleteThis();
                } else {
                    // remove forward declaration if found
                    auto it1 = mTemplateForwardDeclarationsMap.find(it->token());
                    if (it1 != mTemplateForwardDeclarationsMap.end())
                        removeTemplate(it1->second);
                    removeTemplate(it->token());
                }
                mTemplateDeclarations.erase(decl);
            }
        }

        // remove out of line member functions
        while (!mMemberFunctionsToDelete.empty()) {
            const std::list<TokenAndName>::iterator it = std::find_if(mTemplateDeclarations.begin(),
                    mTemplateDeclarations.end(),
                    FindToken(mMemberFunctionsToDelete.begin()->token()));
            // multiple functions can share the same declaration so make sure it hasn't already been deleted
            if (it != mTemplateDeclarations.end()) {
                removeTemplate(it->token());
                mTemplateDeclarations.erase(it);
            }
            mMemberFunctionsToDelete.erase(mMemberFunctionsToDelete.begin());
        }

        // remove explicit instantiations
        for (TokenAndName & j : mExplicitInstantiationsToDelete) {
            Token * start = j.token();
            if (start) {
                Token * end = start->next();
                while (end && end->str() != ";")
                    end = end->next();
                if (start->previous())
                    start = start->previous();
                if (end && end->next())
                    end = end->next();
                eraseTokens(start, end);
            }
        }
    }

    if (passCount == passCountMax) {
        if (mSettings->debugwarnings) {
            const std::list<const Token*> locationList(1, mTokenList.front());
            const ErrorMessage errmsg(locationList, &mTokenizer->list,
                                      Severity::debug,
                                      "debug",
                                      "TemplateSimplifier: pass count limit hit before simplifications were finished.",
                                      false);
            if (mErrorLogger)
                mErrorLogger->reportErr(errmsg);
        }
    }
}

void TemplateSimplifier::syntaxError(const Token *tok)
{
    throw InternalError(tok, "syntax error", InternalError::SYNTAX);
}
