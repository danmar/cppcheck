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
#include "mathlib.h"
#include "errorlogger.h"
#include "token.h"
#include "settings.h"
#include "check.h"
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
        if (Token::simpleMatch(tok, ") {") || Token::Match(tok, ") %var% {") || Token::Match(tok, "[,=] {")) {
            while (tok->str() != "{")
                tok = tok->next();
            tok = tok->link();
        }

        // skip executing scopes (ticket #1984)..
        else if (Token::simpleMatch(tok, "; {"))
            tok = tok->next()->link();

        // skip executing scopes (ticket #3183)..
        else if (Token::simpleMatch(tok, "( {"))
            tok = tok->next()->link();

        // skip executing scopes (ticket #1985)..
        else if (Token::simpleMatch(tok, "try {")) {
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
            } else if (tok2->str() == ")") {  // garbage code! (#3504)
                Token::eraseTokens(tok,tok2);
                tok->deleteThis();
                break;
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
            Token * const eqtok = *it;
            const Token *tok2;
            for (tok2 = eqtok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == "," || tok2->str() == ">")
                    break;
            }
            Token::eraseTokens(eqtok, tok2);
            eqtok->deleteThis();
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


void TemplateSimplifier::addtoken2(Token ** token, const char str[], const unsigned int lineno, const unsigned int fileno)
{
    (*token)->insertToken(str);
    (*token)->linenr(lineno);
    (*token)->fileIndex(fileno);
}

void TemplateSimplifier::addtoken2(Token ** token, const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    (*token)->insertToken(tok->str());
    (*token)->linenr(lineno);
    (*token)->fileIndex(fileno);
    (*token)->isUnsigned(tok->isUnsigned());
    (*token)->isSigned(tok->isSigned());
    (*token)->isLong(tok->isLong());
    (*token)->isUnused(tok->isUnused());
}

void TemplateSimplifier::simplifyTemplatesExpandTemplate(
    Token *_tokens,
    Token **_tokensBack,
    const Token *tok,
    const std::string &name,
    std::vector<const Token *> &typeParametersInDeclaration,
    const std::string &newName,
    std::vector<const Token *> &typesUsedInTemplateInstantion,
    std::list<Token *> &templateInstantiations)
{
    for (const Token *tok3 = _tokens; tok3; tok3 = tok3->next()) {
        if (tok3->str() == "{" || tok3->str() == "(")
            tok3 = tok3->link();

        // Start of template..
        if (tok3 == tok) {
            tok3 = tok3->next();
        }

        // member function implemented outside class definition
        else if (TemplateSimplifier::simplifyTemplatesInstantiateMatch(tok3, name, typeParametersInDeclaration.size(), ":: ~| %var% (")) {
            addtoken2(_tokensBack, newName.c_str(), tok3->linenr(), tok3->fileIndex());
            while (tok3->str() != "::")
                tok3 = tok3->next();
        }

        // not part of template.. go on to next token
        else
            continue;

        int indentlevel = 0;
        std::stack<Token *> braces;     // holds "{" tokens
        std::stack<Token *> brackets;   // holds "(" tokens
        std::stack<Token *> brackets2;  // holds "[" tokens

        for (; tok3; tok3 = tok3->next()) {
            if (tok3->str() == "{")
                ++indentlevel;

            else if (tok3->str() == "}") {
                if (indentlevel <= 1 && brackets.empty() && brackets2.empty()) {
                    // there is a bug if indentlevel is 0
                    // the "}" token should only be added if indentlevel is 1 but I add it always intentionally
                    // if indentlevel ever becomes 0, cppcheck will write:
                    // ### Error: Invalid number of character {
                    addtoken2(_tokensBack, "}", tok3->linenr(), tok3->fileIndex());
                    Token::createMutualLinks(braces.top(), *_tokensBack);
                    braces.pop();
                    break;
                }
                --indentlevel;
            }


            if (tok3->isName()) {
                // search for this token in the type vector
                unsigned int itype = 0;
                while (itype < typeParametersInDeclaration.size() && typeParametersInDeclaration[itype]->str() != tok3->str())
                    ++itype;

                // replace type with given type..
                if (itype < typeParametersInDeclaration.size()) {
                    for (const Token *typetok = typesUsedInTemplateInstantion[itype];
                         typetok && !Token::Match(typetok, "[,>]");
                         typetok = typetok->next()) {
                        addtoken2(_tokensBack, typetok, tok3->linenr(), tok3->fileIndex());
                    }
                    continue;
                }
            }

            // replace name..
            if (Token::Match(tok3, (name + " !!<").c_str())) {
                addtoken2(_tokensBack, newName.c_str(), tok3->linenr(), tok3->fileIndex());
                continue;
            }

            // copy
            addtoken2(_tokensBack, tok3, tok3->linenr(), tok3->fileIndex());
            if (Token::Match(tok3, "%type% <")) {
                //if (!Token::simpleMatch(tok3, (name + " <").c_str()))
                //done = false;
                templateInstantiations.push_back(*_tokensBack);
            }

            // link() newly tokens manually
            if (tok3->str() == "{") {
                braces.push(*_tokensBack);
            } else if (tok3->str() == "}") {
                assert(braces.empty() == false);
                Token::createMutualLinks(braces.top(), *_tokensBack);
                braces.pop();
            } else if (tok3->str() == "(") {
                brackets.push(*_tokensBack);
            } else if (tok3->str() == "[") {
                brackets2.push(*_tokensBack);
            } else if (tok3->str() == ")") {
                assert(brackets.empty() == false);
                Token::createMutualLinks(brackets.top(), *_tokensBack);
                brackets.pop();
            } else if (tok3->str() == "]") {
                assert(brackets2.empty() == false);
                Token::createMutualLinks(brackets2.top(), *_tokensBack);
                brackets2.pop();
            }

        }

        assert(braces.empty());
        assert(brackets.empty());
    }
}

// TODO: This is not the correct class for simplifyCalculations(), so it
// should be moved away.
bool TemplateSimplifier::simplifyCalculations(Token *_tokens)
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // Remove parentheses around variable..
        // keep parentheses here: dynamic_cast<Fred *>(p);
        // keep parentheses here: A operator * (int);
        // keep parentheses here: int ( * ( * f ) ( ... ) ) (int) ;
        // keep parentheses here: int ( * * ( * compilerHookVector ) (void) ) ( ) ;
        // keep parentheses here: operator new [] (size_t);
        // keep parentheses here: Functor()(a ... )
        // keep parentheses here: ) ( var ) ;
        if (Token::Match(tok->next(), "( %var% ) ;|)|,|]|%op%") &&
            !tok->isName() &&
            tok->str() != ">" &&
            tok->str() != "]" &&
            !Token::simpleMatch(tok->previous(), "operator") &&
            !Token::simpleMatch(tok->previous(), "* )") &&
            !Token::simpleMatch(tok->previous(), ") )") &&
            !Token::Match(tok->tokAt(-2), "* %var% )") &&
            !Token::Match(tok->tokAt(-2), "%type% ( ) ( %var%") &&
            !Token::Match(tok, ") ( %var% ) ;")
           ) {
            tok->deleteNext();
            tok = tok->next();
            tok->deleteNext();
            ret = true;
        }

        if (tok->str()[0] == '\'' && tok->str().size() == 3 &&
            Token::Match(tok->previous(), "(|&&|%oror% %any% ==|!=|<=|<|>=|> %num% &&|%oror%|)")) {
            tok->str(MathLib::toString(tok->str()[1] & 0xff));
        }

        if (tok->isNumber()) {
            if (tok->str() == "0") {
                if (Token::Match(tok->previous(), "[+-|] 0")) {
                    tok = tok->previous();
                    if (Token::Match(tok->tokAt(-4), "[;{}] %var% = %var% [+-|] 0 ;") &&
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
                    tok->deleteNext();
                    tok->deleteThis();
                    ret = true;
                } else if (Token::Match(tok->previous(), "[=[(,] 0 * %any% ,|]|)|;|=|%op%") ||
                           Token::Match(tok->previous(), "return|case 0 * %any% ,|:|;|=|%op%")) {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (Token::simpleMatch(tok->previous(), "* 1") || Token::simpleMatch(tok, "1 *")) {
                if (tok->previous() && tok->previous()->isOp())
                    tok = tok->previous();
                tok->deleteNext();
                tok->deleteThis();
                ret = true;
            }

            // Remove parentheses around number..
            if (Token::Match(tok->tokAt(-2), "%any% ( %num% )") && !tok->tokAt(-2)->isName() && tok->strAt(-2) != ">") {
                tok = tok->previous();
                tok->deleteThis();
                tok->deleteNext();
                ret = true;
            }

            if (Token::simpleMatch(tok->previous(), "( 0 ||") ||
                Token::simpleMatch(tok->previous(), "|| 0 )") ||
                Token::simpleMatch(tok->previous(), "( 0 |") ||
                Token::simpleMatch(tok->previous(), "| 0 )") ||
                Token::simpleMatch(tok->previous(), "( 1 &&") ||
                Token::simpleMatch(tok->previous(), "&& 1 )")) {
                if (tok->previous()->isOp())
                    tok = tok->previous();
                tok->deleteNext();
                tok->deleteThis();
                ret = true;
            }

            if (Token::Match(tok, "%num% ==|!=|<=|>=|<|> %num%") &&
                MathLib::isInt(tok->str()) &&
                MathLib::isInt(tok->strAt(2))) {
                if (Token::Match(tok->previous(), "(|&&|%oror%") && Token::Match(tok->tokAt(3), ")|&&|%oror%")) {
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
                    else if (cmp == ">")
                        result = (op1 > op2) ? "1" : "0";

                    tok->str(result);
                    tok->deleteNext(2);
                    ret = true;
                }
            }

            if (Token::Match(tok->previous(), "[([,=] %num% <<|>> %num%")) {
                const MathLib::bigint op1(MathLib::toLongNumber(tok->str()));
                const MathLib::bigint op2(MathLib::toLongNumber(tok->strAt(2)));
                MathLib::bigint result;

                if (tok->next()->str() == "<<")
                    result = op1 << op2;
                else
                    result = op1 >> op2;

                std::ostringstream ss;
                ss << result;

                tok->str(ss.str());
                tok->deleteNext(2);
            }
        }

        else if (tok->next() && tok->next()->isNumber()) {
            // (1-2)
            while (Token::Match(tok, "[[,(=<>+-*|&^] %num% [+-*/] %num% ]|,|)|;|=|%op%") ||
                   Token::Match(tok, "<< %num% [+-*/] %num% ]|,|)|;|=|%op%") ||
                   Token::Match(tok, "[[,(=<>+-*|&^] %num% [+-*/] %num% <<|>>") ||
                   Token::Match(tok, "<< %num% [+-*/] %num% <<") ||
                   Token::Match(tok, "[(,[] %num% [|&^] %num% [];,);]") ||
                   Token::Match(tok, "(|%op% %num% [+-*/] %num% )|%op%") ||
                   Token::Match(tok,"return|case %num% [+-*/] %num% ;|,|=|:|%op%")) {
                tok = tok->next();

                // Don't simplify "%num% / 0"
                if (Token::simpleMatch(tok->next(), "/ 0"))
                    continue;

                // & | ^
                if (Token::Match(tok->next(), "[&|^]")) {
                    std::string result;
                    const std::string first(tok->str());
                    const std::string second(tok->strAt(2));
                    const char op = tok->next()->str()[0];
                    if (op == '&')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) & MathLib::toLongNumber(second));
                    else if (op == '|')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) | MathLib::toLongNumber(second));
                    else if (op == '^')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) ^ MathLib::toLongNumber(second));

                    if (!result.empty()) {
                        ret = true;
                        tok->str(result);
                        tok->deleteNext(2);
                        continue;
                    }
                }

                // Division where result is a whole number
                if (Token::Match(tok->previous(), "* %num% /") &&
                    tok->str() == MathLib::multiply(tok->strAt(2), MathLib::divide(tok->str(), tok->strAt(2)))) {
                }

                // + and - are calculated after * and /
                else if (Token::Match(tok->next(), "[+-/]")) {
                    if (Token::Match(tok->previous(), "[*/%]"))
                        continue;
                    if (Token::Match(tok->tokAt(3), "[*/%]"))
                        continue;
                }

                if (Token::Match(tok->previous(), "- %num% - %num%"))
                    tok->str(MathLib::add(tok->str(), tok->strAt(2)));
                else if (Token::Match(tok->previous(), "- %num% + %num%"))
                    tok->str(MathLib::subtract(tok->str(), tok->strAt(2)));
                else {
                    try {
                        tok->str(MathLib::calculate(tok->str(), tok->strAt(2), tok->next()->str()[0]));
                    } catch (InternalError &e) {
                        e.token = tok;
                        throw;
                    }
                }

                tok->deleteNext(2);

                // evaluate "2 + 2 - 2 - 2"
                // as (((2 + 2) - 2) - 2) = 0
                // instead of ((2 + 2) - (2 - 2)) = 4
                if (Token::Match(tok->next(), "[+-*/]")) {
                    tok = tok->previous();
                    continue;
                }

                ret = true;
            }
        }
    }
    return ret;
}


void TemplateSimplifier::simplifyTemplateInstantions(
    Token *_tokens,
    Token **_tokensBack,
    ErrorLogger *_errorLogger,
    const Settings *_settings,
    const std::vector<std::string> &files,
    const Token *tok,
    std::list<Token *> &templateInstantiations,
    std::set<std::string> &expandedtemplates)
{
    // this variable is not used at the moment. The intention was to
    // allow continuous instantiations until all templates has been expanded
    //bool done = false;

    // Contains tokens such as "T"
    std::vector<const Token *> typeParametersInDeclaration;
    for (tok = tok->tokAt(2); tok && tok->str() != ">"; tok = tok->next()) {
        if (Token::Match(tok, "%var% ,|>"))
            typeParametersInDeclaration.push_back(tok);
    }

    // bail out if the end of the file was reached
    if (!tok)
        return;

    // get the position of the template name
    int namepos = TemplateSimplifier::simplifyTemplatesGetTemplateNamePosition(tok);
    if (namepos == -1) {
        // debug message that we bail out..
        if (_settings->debugwarnings) {
            std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = tok->linenr();
            loc.setfile(files[tok->fileIndex()]);
            locationList.push_back(loc);

            const ErrorLogger::ErrorMessage errmsg(locationList,
                                                   Severity::debug,
                                                   "simplifyTemplates: bailing out",
                                                   "debug",
                                                   false);

            if (_errorLogger)
                _errorLogger->reportErr(errmsg);
            else
                Check::reportError(errmsg);
        }
        return;
    }

    // name of template function/class..
    const std::string name(tok->strAt(namepos));

    const bool isfunc(tok->strAt(namepos + 1) == "(");

    // locate template usage..
    std::string::size_type amountOftemplateInstantiations = templateInstantiations.size();
    unsigned int recursiveCount = 0;

    for (std::list<Token *>::const_iterator iter2 = templateInstantiations.begin(); iter2 != templateInstantiations.end(); ++iter2) {
        if (amountOftemplateInstantiations != templateInstantiations.size()) {
            amountOftemplateInstantiations = templateInstantiations.size();
            simplifyCalculations(_tokens);
            ++recursiveCount;
            if (recursiveCount > 100) {
                // bail out..
                break;
            }
        }

        Token * const tok2 = *iter2;
        if (tok2->str() != name)
            continue;

        if (Token::Match(tok2->previous(), "[;{}=]") &&
            !TemplateSimplifier::simplifyTemplatesInstantiateMatch(*iter2, name, typeParametersInDeclaration.size(), isfunc ? "(" : "*| %var%"))
            continue;

        // New type..
        std::vector<const Token *> typesUsedInTemplateInstantion;
        std::string typeForNewNameStr;
        std::string templateMatchPattern(name + " < ");
        for (const Token *tok3 = tok2->tokAt(2); tok3 && tok3->str() != ">"; tok3 = tok3->next()) {
            // #2648 - unhandled parenthesis => bail out
            // #2721 - unhandled [ => bail out
            if (tok3->str() == "(" || tok3->str() == "[") {
                typeForNewNameStr.clear();
                break;
            }
            if (!tok3->next()) {
                typeForNewNameStr.clear();
                break;
            }
            templateMatchPattern += tok3->str();
            templateMatchPattern += " ";
            if (Token::Match(tok3->previous(), "[<,]"))
                typesUsedInTemplateInstantion.push_back(tok3);
            // add additional type information
            if (tok3->isUnsigned())
                typeForNewNameStr += "unsigned";
            else if (tok3->isSigned())
                typeForNewNameStr += "signed";
            if (tok3->isLong())
                typeForNewNameStr += "long";
            typeForNewNameStr += tok3->str();
        }
        templateMatchPattern += ">";
        const std::string typeForNewName(typeForNewNameStr);

        if (typeForNewName.empty() || typeParametersInDeclaration.size() != typesUsedInTemplateInstantion.size()) {
            if (_settings->debugwarnings) {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = tok2->linenr();
                loc.setfile(files[tok2->fileIndex()]);
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "Failed to instantiate template. The checking continues anyway.",
                                                       "debug",
                                                       false);

                _errorLogger->reportErr(errmsg);
            }
            if (typeForNewName.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string newName(name + "<" + typeForNewName + ">");

        if (expandedtemplates.find(newName) == expandedtemplates.end()) {
            expandedtemplates.insert(newName);
            TemplateSimplifier::simplifyTemplatesExpandTemplate(_tokens,_tokensBack, tok,name,typeParametersInDeclaration,newName,typesUsedInTemplateInstantion,templateInstantiations);
        }

        // Replace all these template usages..
        std::list< std::pair<Token *, Token *> > removeTokens;
        for (Token *tok4 = tok2; tok4; tok4 = tok4->next()) {
            if (Token::simpleMatch(tok4, templateMatchPattern.c_str())) {
                Token * tok5 = tok4->tokAt(2);
                unsigned int typeCountInInstantion = 1U; // There is always atleast one type
                const Token *typetok = (!typesUsedInTemplateInstantion.empty()) ? typesUsedInTemplateInstantion[0] : 0;
                while (tok5 && tok5->str() != ">") {
                    if (tok5->str() != ",") {
                        if (!typetok ||
                            tok5->isUnsigned() != typetok->isUnsigned() ||
                            tok5->isSigned() != typetok->isSigned() ||
                            tok5->isLong() != typetok->isLong()) {
                            break;
                        }

                        typetok = typetok ? typetok->next() : 0;
                    } else {
                        typetok = (typeCountInInstantion < typesUsedInTemplateInstantion.size()) ? typesUsedInTemplateInstantion[typeCountInInstantion] : 0;
                        ++typeCountInInstantion;
                    }
                    tok5 = tok5->next();
                }

                // matching template usage => replace tokens..
                // Foo < int >  =>  Foo<int>
                if (tok5 && tok5->str() == ">" && typeCountInInstantion == typesUsedInTemplateInstantion.size()) {
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
}


void TemplateSimplifier::simplifyTemplates(
    Token *_tokens,
    Token **_tokensBack,
    ErrorLogger *_errorLogger,
    const Settings *_settings,
    const std::vector<std::string> &_files,
    bool &_codeWithTemplates
)
{

    std::set<std::string> expandedtemplates(TemplateSimplifier::simplifyTemplatesExpandSpecialized(_tokens));

    // Locate templates and set member variable _codeWithTemplates if the code has templates.
    // this info is used by checks
    std::list<Token *> templates(TemplateSimplifier::simplifyTemplatesGetTemplateDeclarations(_tokens,_codeWithTemplates));

    if (templates.empty()) {
        TemplateSimplifier::removeTemplates(_tokens);
        return;
    }

    // There are templates..
    // Remove "typename" unless used in template arguments..
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "typename")
            tok->deleteThis();

        if (Token::simpleMatch(tok, "template <")) {
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;
        }
    }

    // Locate possible instantiations of templates..
    std::list<Token *> templateInstantiations(TemplateSimplifier::simplifyTemplatesGetTemplateInstantiations(_tokens));

    // No template instantiations? Then remove all templates.
    if (templateInstantiations.empty()) {
        TemplateSimplifier::removeTemplates(_tokens);
        return;
    }

    // Template arguments with default values
    TemplateSimplifier::simplifyTemplatesUseDefaultArgumentValues(templates, templateInstantiations);

    // expand templates
    //bool done = false;
    //while (!done)
    {
        //done = true;
        for (std::list<Token *>::reverse_iterator iter1 = templates.rbegin(); iter1 != templates.rend(); ++iter1) {
            TemplateSimplifier::simplifyTemplateInstantions(
                _tokens,
                _tokensBack,
                _errorLogger,
                _settings,
                _files,

                *iter1, templateInstantiations, expandedtemplates);
        }
    }

    TemplateSimplifier::removeTemplates(_tokens);
}
