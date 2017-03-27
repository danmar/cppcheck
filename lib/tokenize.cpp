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
#include "tokenize.h"
#include "mathlib.h"
#include "settings.h"
#include "check.h"
#include "path.h"
#include "symboldatabase.h"
#include "templatesimplifier.h"
#include "timer.h"
#include "utils.h"

#include <cstring>
#include <sstream>
#include <cassert>
#include <cctype>
#include <stack>
#include <iostream>

//---------------------------------------------------------------------------

namespace {
    // local struct used in setVarId
    // in order to store information about the scope
    struct VarIdScopeInfo {
        VarIdScopeInfo()
            :isExecutable(false), isStructInit(false), isEnum(false), startVarid(0) {
        }
        VarIdScopeInfo(bool _isExecutable, bool _isStructInit, bool _isEnum, unsigned int _startVarid)
            :isExecutable(_isExecutable), isStructInit(_isStructInit), isEnum(_isEnum), startVarid(_startVarid) {
        }

        const bool isExecutable;
        const bool isStructInit;
        const bool isEnum;
        const unsigned int startVarid;
    };

    /** Return whether tok is the "{" that starts an enumerator list */
    bool isEnumStart(const Token* tok)
    {
        if (!tok || tok->str() != "{")
            return false;
        return (tok->strAt(-1) == "enum") || (tok->strAt(-2) == "enum");
    }
}

const Token * Tokenizer::isFunctionHead(const Token *tok, const std::string &endsWith) const
{
    return Tokenizer::isFunctionHead(tok, endsWith, isCPP());
}

const Token * Tokenizer::isFunctionHead(const Token *tok, const std::string &endsWith, bool cpp)
{
    if (!tok)
        return nullptr;
    if (tok->str() == "(")
        tok = tok->link();
    if (Token::Match(tok, ") [;{]")) {
        tok = tok->next();
        if (tok->isName())
            tok = tok->next();
        return (endsWith.find(tok->str()) != std::string::npos) ? tok : nullptr;
    }
    if (cpp && tok->str() == ")") {
        tok = tok->next();
        while (Token::Match(tok, "const|noexcept|override|volatile|&|&& !!(") ||
               (Token::Match(tok, "%name% !!(") && tok->isUpperCaseName()))
            tok = tok->next();
        if (Token::Match(tok, "throw|noexcept ("))
            tok = tok->linkAt(1)->next();
        if (Token::Match(tok, "%name% (") && tok->isUpperCaseName())
            tok = tok->linkAt(1)->next();
        if (tok && tok->str() == ".") { // trailing return type
            for (tok = tok->next(); tok && !Token::Match(tok, "[;{]"); tok = tok->next())
                if (tok->link() && Token::Match(tok, "<|[|("))
                    tok = tok->link();
        }
        if (Token::Match(tok, "= 0|default|delete ;"))
            tok = tok->tokAt(2);
        return (tok && endsWith.find(tok->str()) != std::string::npos) ? tok : nullptr;
    }
    return nullptr;
}

/**
 * is tok the start brace { of a class, struct, union, or enum
 */
static bool isClassStructUnionEnumStart(const Token * tok)
{
    if (!Token::Match(tok->previous(), "class|struct|union|enum|%name%|>|>> {"))
        return false;
    const Token * tok2 = tok->previous();
    while (tok2 && !Token::Match(tok2, "class|struct|union|enum|{|}|;"))
        tok2 = tok2->previous();
    return Token::Match(tok2, "class|struct|union|enum");
}

//---------------------------------------------------------------------------

Tokenizer::Tokenizer() :
    list(0),
    _settings(0),
    _errorLogger(0),
    _symbolDatabase(0),
    _varId(0),
    _codeWithTemplates(false), //is there any templates?
    m_timerResults(nullptr)
#ifdef MAXTIME
    ,maxtime(std::time(0) + MAXTIME)
#endif
{
}

Tokenizer::Tokenizer(const Settings *settings, ErrorLogger *errorLogger) :
    list(settings),
    _settings(settings),
    _errorLogger(errorLogger),
    _symbolDatabase(0),
    _varId(0),
    _codeWithTemplates(false), //is there any templates?
    m_timerResults(nullptr)
#ifdef MAXTIME
    ,maxtime(std::time(0) + MAXTIME)
#endif
{
    // make sure settings are specified
    assert(_settings);
}

Tokenizer::~Tokenizer()
{
    delete _symbolDatabase;
}


//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------

unsigned int Tokenizer::sizeOfType(const Token *type) const
{
    if (!type || type->str().empty())
        return 0;

    if (type->tokType() == Token::eString)
        return Token::getStrLength(type) + 1U;

    std::map<std::string, unsigned int>::const_iterator it = _typeSize.find(type->str());
    if (it == _typeSize.end()) {
        const Library::PodType* podtype = _settings->library.podtype(type->str());
        if (!podtype)
            return 0;

        return podtype->size;
    } else if (type->isLong()) {
        if (type->str() == "double")
            return _settings->sizeof_long_double;
        else if (type->str() == "long")
            return _settings->sizeof_long_long;
    }

    return it->second;
}

//---------------------------------------------------------------------------

Token *Tokenizer::copyTokens(Token *dest, const Token *first, const Token *last, bool one_line)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    unsigned int linenrs = dest->linenr();
    const unsigned int commonFileIndex = dest->fileIndex();
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(commonFileIndex);
        tok2->linenr(linenrs);
        tok2->tokType(tok->tokType());
        tok2->flags(tok->flags());
        tok2->varId(tok->varId());

        // Check for links and fix them up
        if (Token::Match(tok2, "(|[|{"))
            links.push(tok2);
        else if (Token::Match(tok2, ")|]|}")) {
            if (links.empty())
                return tok2;

            Token * link = links.top();

            tok2->link(link);
            link->link(tok2);

            links.pop();
        }
        if (!one_line && tok->next())
            linenrs += tok->next()->linenr() - tok->linenr();
    }
    return tok2;
}

//---------------------------------------------------------------------------

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef) const
{
    // check for an end of definition
    const Token * tok = *tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>|(|{")) {
        const Token * end = tok->next();

        if (end->str() == "[") {
            if (!end->link())
                syntaxError(end); // invalid code
            end = end->link()->next();
        } else if (end->str() == ",") {
            // check for derived class
            if (Token::Match(tok->previous(), "public|private|protected"))
                return false;

            // find end of definition
            while (end && end->next() && !Token::Match(end->next(), ";|)|>")) {
                if (end->next()->str() == "(")
                    end = end->linkAt(1);

                end = (end)?end->next():nullptr;
            }
            if (end)
                end = end->next();
        } else if (end->str() == "(") {
            if (tok->previous()->str().find("operator")  == 0) {
                // conversion operator
                return false;
            } else if (tok->previous()->str() == "typedef") {
                // typedef of function returning this type
                return false;
            } else if (Token::Match(tok->previous(), "public:|private:|protected:")) {
                return false;
            } else if (tok->previous()->str() == ">") {
                if (!Token::Match(tok->tokAt(-2), "%type%"))
                    return false;

                if (!Token::Match(tok->tokAt(-3), ",|<"))
                    return false;

                *tokPtr = end->link();
                return true;
            }
        }

        if (end) {
            if (Token::simpleMatch(end, ") {")) { // function parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const|struct")) {
                    // duplicate definition so skip entire function
                    *tokPtr = end->next()->link();
                    return true;
                }
            } else if (end->str() == ">") { // template parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const|volatile")) {
                    // duplicate definition so skip entire template
                    while (end && end->str() != "{")
                        end = end->next();
                    if (end) {
                        *tokPtr = end->link();
                        return true;
                    }
                }
            } else {
                // look backwards
                if (Token::Match(tok->previous(), "typedef|}|>") ||
                    (end->str() == ";" && tok->previous()->str() == ",") ||
                    (tok->previous()->str() == "*" && tok->next()->str() != "(") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     (!Token::Match(tok->previous(), "return|new|const|friend|public|private|protected|throw|extern") &&
                      !Token::simpleMatch(tok->tokAt(-2), "friend class")))) {
                    // scan backwards for the end of the previous statement
                    while (tok && tok->previous() && !Token::Match(tok->previous(), ";|{")) {
                        if (tok->previous()->str() == "}") {
                            tok = tok->previous()->link();
                        } else if (tok->previous()->str() == "typedef") {
                            return true;
                        } else if (tok->previous()->str() == "enum") {
                            return true;
                        } else if (tok->previous()->str() == "struct") {
                            if (tok->strAt(-2) == "typedef" &&
                                tok->next()->str() == "{" &&
                                typeDef->strAt(3) != "{") {
                                // declaration after forward declaration
                                return true;
                            } else if (tok->next()->str() == "{") {
                                return true;
                            } else if (Token::Match(tok->next(), ")|*")) {
                                return true;
                            } else if (tok->next()->str() == name->str()) {
                                return true;
                            } else if (tok->next()->str() != ";") {
                                return true;
                            } else {
                                return false;
                            }
                        } else if (tok->previous()->str() == "union") {
                            if (tok->next()->str() != ";") {
                                return true;
                            } else {
                                return false;
                            }
                        } else if (isCPP() && tok->previous()->str() == "class") {
                            if (tok->next()->str() != ";") {
                                return true;
                            } else {
                                return false;
                            }
                        }
                        if (tok)
                            tok = tok->previous();
                    }

                    if ((*tokPtr)->strAt(1) != "(" || !Token::Match((*tokPtr)->linkAt(1), ") .|(|["))
                        return true;
                }
            }
        }
    }

    return false;
}

void Tokenizer::unsupportedTypedef(const Token *tok) const
{
    if (!_settings->debugwarnings)
        return;

    std::ostringstream str;
    const Token *tok1 = tok;
    unsigned int level = 0;
    while (tok) {
        if (level == 0 && tok->str() == ";")
            break;
        else if (tok->str() == "{")
            ++level;
        else if (tok->str() == "}") {
            if (!level)
                break;
            --level;
        }

        if (tok != tok1)
            str << " ";
        str << tok->str();
        tok = tok->next();
    }
    if (tok)
        str << " ;";

    reportError(tok1, Severity::debug, "debug",
                "Failed to parse \'" + str.str() + "\'. The checking continues anyway.");
}

Token * Tokenizer::deleteInvalidTypedef(Token *typeDef)
{
    Token *tok = nullptr;

    // remove typedef but leave ;
    while (typeDef->next()) {
        if (typeDef->next()->str() == ";") {
            typeDef->deleteNext();
            break;
        } else if (typeDef->next()->str() == "{")
            Token::eraseTokens(typeDef, typeDef->linkAt(1));
        else if (typeDef->next()->str() == "}")
            break;
        typeDef->deleteNext();
    }

    if (typeDef != list.front()) {
        tok = typeDef->previous();
        tok->deleteNext();
    } else {
        list.front()->deleteThis();
        tok = list.front();
    }

    return tok;
}

struct Space {
    Space() : classEnd(nullptr), isNamespace(false) { }
    std::string className;
    const Token * classEnd;
    bool isNamespace;
};

static Token *splitDefinitionFromTypedef(Token *tok)
{
    Token *tok1;
    std::string name;
    bool isConst = false;

    if (tok->next()->str() == "const") {
        tok->deleteNext();
        isConst = true;
    }

    if (tok->strAt(2) == "{") { // unnamed
        tok1 = tok->linkAt(2);

        if (tok1 && tok1->next()) {
            // use typedef name if available
            if (Token::Match(tok1->next(), "%type%"))
                name = tok1->next()->str();
            else { // create a unique name
                static unsigned int count = 0;
                name = "Unnamed" + MathLib::toString(count++);
            }
            tok->next()->insertToken(name);
        } else
            return nullptr;
    } else if (tok->strAt(3) == ":") {
        tok1 = tok->tokAt(4);
        while (tok1 && tok1->str() != "{")
            tok1 = tok1->next();
        if (!tok1)
            return nullptr;

        tok1 = tok1->link();

        name = tok->strAt(2);
    } else { // has a name
        tok1 = tok->linkAt(3);

        if (!tok1)
            return nullptr;

        name = tok->strAt(2);
    }

    tok1->insertToken(";");
    tok1 = tok1->next();

    if (tok1->next() && tok1->next()->str() == ";" && tok1->previous()->str() == "}") {
        tok->deleteThis();
        tok1->deleteThis();
        return nullptr;
    } else {
        tok1->insertToken("typedef");
        tok1 = tok1->next();
        Token * tok3 = tok1;
        if (isConst) {
            tok1->insertToken("const");
            tok1 = tok1->next();
        }
        tok1->insertToken(tok->next()->str()); // struct, union or enum
        tok1 = tok1->next();
        tok1->insertToken(name);
        tok->deleteThis();
        tok = tok3;
    }

    return tok;
}

/* This function is called when processing function related typedefs.
 * If simplifyTypedef generates an "Internal Error" message and the
 * code that generated it deals in some way with functions, then this
 * function will probably need to be extended to handle a new function
 * related pattern */
Token *Tokenizer::processFunc(Token *tok2, bool inOperator) const
{
    if (tok2->next() && tok2->next()->str() != ")" &&
        tok2->next()->str() != ",") {
        // skip over tokens for some types of canonicalization
        if (Token::Match(tok2->next(), "( * %type% ) ("))
            tok2 = tok2->linkAt(5);
        else if (Token::Match(tok2->next(), "* ( * %type% ) ("))
            tok2 = tok2->linkAt(6);
        else if (Token::Match(tok2->next(), "* ( * %type% ) ;"))
            tok2 = tok2->tokAt(5);
        else if (Token::Match(tok2->next(), "* ( %type% [") &&
                 Token::Match(tok2->linkAt(4), "] ) ;|="))
            tok2 = tok2->linkAt(4)->next();
        else if (Token::Match(tok2->next(), "* ( * %type% ("))
            tok2 = tok2->linkAt(5)->next();
        else if (Token::simpleMatch(tok2->next(), "* [") &&
                 Token::simpleMatch(tok2->linkAt(2), "] ;"))
            tok2 = tok2->next();
        else {
            if (tok2->next()->str() == "(")
                tok2 = tok2->next()->link();
            else if (!inOperator && !Token::Match(tok2->next(), "[|>|;")) {
                tok2 = tok2->next();

                while (Token::Match(tok2, "*|&") &&
                       !Token::Match(tok2->next(), ")|>"))
                    tok2 = tok2->next();

                // skip over namespace
                while (Token::Match(tok2, "%name% ::"))
                    tok2 = tok2->tokAt(2);

                if (!tok2)
                    return nullptr;

                if (tok2->str() == "(" &&
                    tok2->link()->next() &&
                    tok2->link()->next()->str() == "(") {
                    tok2 = tok2->link();

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }

                // skip over typedef parameter
                if (tok2->next() && tok2->next()->str() == "(") {
                    tok2 = tok2->next()->link();
                    if (!tok2->next())
                        syntaxError(tok2);

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }
            }
        }
    }
    return tok2;
}

void Tokenizer::simplifyTypedef()
{
    std::vector<Space> spaceInfo;
    bool isNamespace = false;
    std::string className;
    bool hasClass = false;
    bool goback = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (_errorLogger && !list.getFiles().empty())
            _errorLogger->reportProgress(list.getFiles()[0], "Tokenize (typedef)", tok->progressValue());

        if (_settings->terminated())
            return;

        if (isMaxTime())
            return;

        if (goback) {
            //jump back once, see the comment at the end of the function
            goback = false;
            tok = tok->previous();
        }

        if (tok->str() != "typedef") {
            if (tok->str() == "(" && tok->strAt(1) == "typedef") {
                // Skip typedefs inside parentheses (#2453 and #4002)
                tok = tok->next();
            } else if (Token::Match(tok, "class|struct|namespace %any%") &&
                       (!tok->previous() || tok->previous()->str() != "enum")) {
                isNamespace = (tok->str() == "namespace");
                hasClass = true;
                className = tok->next()->str();
            } else if (hasClass && tok->str() == ";") {
                hasClass = false;
            } else if (hasClass && tok->str() == "{") {
                Space info;
                info.isNamespace = isNamespace;
                info.className = className;
                info.classEnd = tok->link();
                spaceInfo.push_back(info);

                hasClass = false;
            } else if (!spaceInfo.empty() && tok->str() == "}" && spaceInfo.back().classEnd == tok) {
                spaceInfo.pop_back();
            }
            continue;
        }

        // pull struct, union, enum or class definition out of typedef
        // use typedef name for unnamed struct, union, enum or class
        if (Token::Match(tok->next(), "const| struct|enum|union|class %type%| {")) {
            Token *tok1 = splitDefinitionFromTypedef(tok);
            if (!tok1)
                continue;
            tok = tok1;
        } else if (Token::Match(tok->next(), "const| struct|class %type% :")) {
            Token *tok1 = tok;
            while (tok1 && tok1->str() != ";" && tok1->str() != "{")
                tok1 = tok1->next();
            if (tok1 && tok1->str() == "{") {
                tok1 = splitDefinitionFromTypedef(tok);
                if (!tok1)
                    continue;
                tok = tok1;
            }
        }

        /** @todo add support for union */
        if (Token::Match(tok->next(), "enum %type% %type% ;") && tok->strAt(2) == tok->strAt(3)) {
            tok->deleteNext(3);
            tok->deleteThis();
            if (tok->next())
                tok->deleteThis();
            //now the next token to process is 'tok', not 'tok->next()';
            goback = true;
            continue;
        }

        Token *typeName;
        Token *typeStart = nullptr;
        Token *typeEnd = nullptr;
        Token *argStart = nullptr;
        Token *argEnd = nullptr;
        Token *arrayStart = nullptr;
        Token *arrayEnd = nullptr;
        Token *specStart = nullptr;
        Token *specEnd = nullptr;
        Token *typeDef = tok;
        Token *argFuncRetStart = nullptr;
        Token *argFuncRetEnd = nullptr;
        Token *funcStart = nullptr;
        Token *funcEnd = nullptr;
        Token *tokOffset = tok->next();
        bool function = false;
        bool functionPtr = false;
        bool functionRef = false;
        bool functionRetFuncPtr = false;
        bool functionPtrRetFuncPtr = false;
        bool ptrToArray = false;
        bool refToArray = false;
        bool ptrMember = false;
        bool typeOf = false;
        Token *namespaceStart = nullptr;
        Token *namespaceEnd = nullptr;

        // check for invalid input
        if (!tokOffset)
            syntaxError(tok);


        if (tokOffset->str() == "::") {
            typeStart = tokOffset;
            tokOffset = tokOffset->next();

            while (Token::Match(tokOffset, "%type% ::"))
                tokOffset = tokOffset->tokAt(2);

            typeEnd = tokOffset;

            if (Token::Match(tokOffset, "%type%"))
                tokOffset = tokOffset->next();
        } else if (Token::Match(tokOffset, "%type% ::")) {
            typeStart = tokOffset;

            do {
                tokOffset = tokOffset->tokAt(2);
            } while (Token::Match(tokOffset, "%type% ::"));

            typeEnd = tokOffset;

            if (Token::Match(tokOffset, "%type%"))
                tokOffset = tokOffset->next();
        } else if (Token::Match(tokOffset, "%type%")) {
            typeStart = tokOffset;

            while (Token::Match(tokOffset, "const|struct|enum %type%") ||
                   (tokOffset->next() && tokOffset->next()->isStandardType()))
                tokOffset = tokOffset->next();

            typeEnd = tokOffset;
            tokOffset = tokOffset->next();

            bool atEnd = false;
            while (!atEnd) {
                if (tokOffset && tokOffset->str() == "::") {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                }

                if (Token::Match(tokOffset, "%type%") &&
                    tokOffset->next() && !Token::Match(tokOffset->next(), "[|;|,|(")) {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                } else if (Token::simpleMatch(tokOffset, "const (")) {
                    typeEnd = tokOffset;
                    tokOffset = tokOffset->next();
                    atEnd = true;
                } else
                    atEnd = true;
            }
        } else
            continue; // invalid input

        // check for invalid input
        if (!tokOffset)
            syntaxError(tok);

        // check for template
        if (!isC() && tokOffset->str() == "<") {
            typeEnd = tokOffset->findClosingBracket();

            while (typeEnd && Token::Match(typeEnd->next(), ":: %type%"))
                typeEnd = typeEnd->tokAt(2);

            if (!typeEnd) {
                // internal error
                return;
            }

            while (Token::Match(typeEnd->next(), "const|volatile"))
                typeEnd = typeEnd->next();

            tok = typeEnd;
            tokOffset = tok->next();
        }

        std::list<std::string> pointers;
        // check for pointers and references
        while (Token::Match(tokOffset, "*|&|&&|const")) {
            pointers.push_back(tokOffset->str());
            tokOffset = tokOffset->next();
        }

        // check for invalid input
        if (!tokOffset)
            syntaxError(tok);

        if (Token::Match(tokOffset, "%type%")) {
            // found the type name
            typeName = tokOffset;
            tokOffset = tokOffset->next();

            // check for array
            if (tokOffset && tokOffset->str() == "[") {
                arrayStart = tokOffset;

                bool atEnd = false;
                while (!atEnd) {
                    while (tokOffset->next() && !Token::Match(tokOffset->next(), ";|,")) {
                        tokOffset = tokOffset->next();
                    }

                    if (!tokOffset->next())
                        return; // invalid input
                    else if (tokOffset->next()->str() == ";")
                        atEnd = true;
                    else if (tokOffset->str() == "]")
                        atEnd = true;
                    else
                        tokOffset = tokOffset->next();
                }

                arrayEnd = tokOffset;
                tokOffset = tokOffset->next();
            }

            // check for end or another
            if (Token::Match(tokOffset, ";|,"))
                tok = tokOffset;

            // or a function typedef
            else if (tokOffset && tokOffset->str() == "(") {
                // unhandled typedef, skip it and continue
                if (typeName->str() == "void") {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    if (tok == list.front())
                        //now the next token to process is 'tok', not 'tok->next()';
                        goback = true;
                    continue;
                }

                // function pointer
                else if (Token::Match(tokOffset, "( * %name% ) (")) {
                    // name token wasn't a name, it was part of the type
                    typeEnd = typeEnd->next();
                    functionPtr = true;
                    tokOffset = tokOffset->next();
                    funcStart = tokOffset;
                    funcEnd = tokOffset;
                    tokOffset = tokOffset->tokAt(3);
                    typeName = tokOffset->tokAt(-2);
                    argStart = tokOffset;
                    argEnd = tokOffset->link();
                    tok = argEnd->next();
                }

                // function
                else if (isFunctionHead(tokOffset->link(), ";,")) {
                    function = true;
                    if (tokOffset->link()->next()->str() == "const") {
                        specStart = tokOffset->link()->next();
                        specEnd = specStart;
                    }
                    argStart = tokOffset;
                    argEnd = tokOffset->link();
                    tok = argEnd->next();
                    if (specStart)
                        tok = tok->next();
                }

                // syntax error
                else
                    syntaxError(tok);
            }

            // unhandled typedef, skip it and continue
            else {
                unsupportedTypedef(typeDef);
                tok = deleteInvalidTypedef(typeDef);
                if (tok == list.front())
                    //now the next token to process is 'tok', not 'tok->next()';
                    goback = true;
                continue;
            }
        }

        // typeof: typedef __typeof__ ( ... ) type;
        else if (Token::simpleMatch(tokOffset->previous(), "__typeof__ (") &&
                 Token::Match(tokOffset->link(), ") %type% ;")) {
            argStart = tokOffset;
            argEnd = tokOffset->link();
            typeName = tokOffset->link()->next();
            tok = typeName->next();
            typeOf = true;
        }

        // function: typedef ... ( ... type )( ... );
        //           typedef ... (( ... type )( ... ));
        //           typedef ... ( * ( ... type )( ... ));
        else if (tokOffset->str() == "(" && (
                     (tokOffset->link() && Token::Match(tokOffset->link()->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->link()->next()->link(), ") const|volatile|;")) ||
                     (Token::simpleMatch(tokOffset, "( (") &&
                      tokOffset->next() && Token::Match(tokOffset->next()->link()->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->next()->link()->next()->link(), ") const|volatile| ) ;|,")) ||
                     (Token::simpleMatch(tokOffset, "( * (") &&
                      tokOffset->linkAt(2) && Token::Match(tokOffset->linkAt(2)->previous(), "%type% ) (") &&
                      Token::Match(tokOffset->linkAt(2)->next()->link(), ") const|volatile| ) ;|,")))) {
            if (tokOffset->next()->str() == "(")
                tokOffset = tokOffset->next();
            else if (Token::simpleMatch(tokOffset, "( * (")) {
                pointers.push_back("*");
                tokOffset = tokOffset->tokAt(2);
            }

            if (tokOffset->link()->strAt(-2) == "*")
                functionPtr = true;
            else
                function = true;
            funcStart = tokOffset->next();
            tokOffset = tokOffset->link();
            funcEnd = tokOffset->tokAt(-2);
            typeName = tokOffset->previous();
            argStart = tokOffset->next();
            argEnd = tokOffset->next()->link();
            if (!argEnd)
                syntaxError(argStart);

            tok = argEnd->next();
            Token *spec = tok;
            if (Token::Match(spec, "const|volatile")) {
                specStart = spec;
                specEnd = spec;
                while (Token::Match(spec->next(), "const|volatile")) {
                    specEnd = spec->next();
                    spec = specEnd;
                }
                tok = specEnd->next();
            }
            if (!tok)
                syntaxError(specEnd);

            if (tok->str() == ")")
                tok = tok->next();
        }

        else if (Token::Match(tokOffset, "( %type% (")) {
            function = true;
            if (tokOffset->link()->next()) {
                tok = tokOffset->link()->next();
                tokOffset = tokOffset->tokAt(2);
                typeName = tokOffset->previous();
                argStart = tokOffset;
                argEnd = tokOffset->link();
            } else {
                // internal error
                continue;
            }
        }

        // pointer to function returning pointer to function
        else if (Token::Match(tokOffset, "( * ( * %type% ) (") &&
                 Token::simpleMatch(tokOffset->linkAt(6), ") ) (") &&
                 Token::Match(tokOffset->linkAt(6)->linkAt(2), ") ;|,")) {
            functionPtrRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(6);
            typeName = tokOffset->tokAt(-2);
            argStart = tokOffset;
            argEnd = tokOffset->link();
            if (!argEnd)
                syntaxError(arrayStart);

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(argFuncRetStart);

            tok = argFuncRetEnd->next();
        }

        // function returning pointer to function
        else if (Token::Match(tokOffset, "( * %type% (") &&
                 Token::simpleMatch(tokOffset->linkAt(3), ") ) (") &&
                 Token::Match(tokOffset->linkAt(3)->linkAt(2), ") ;|,")) {
            functionRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(3);
            typeName = tokOffset->previous();
            argStart = tokOffset;
            argEnd = tokOffset->link();

            argFuncRetStart = argEnd->tokAt(2);
            if (!argFuncRetStart)
                syntaxError(tokOffset);

            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(tokOffset);

            tok = argFuncRetEnd->next();
        } else if (Token::Match(tokOffset, "( * ( %type% ) (")) {
            functionRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(5);
            typeName = tokOffset->tokAt(-2);
            argStart = tokOffset;
            argEnd = tokOffset->link();
            if (!argEnd)
                syntaxError(arrayStart);

            argFuncRetStart = argEnd->tokAt(2);
            if (!argFuncRetStart)
                syntaxError(tokOffset);

            argFuncRetEnd = argFuncRetStart->link();
            if (!argFuncRetEnd)
                syntaxError(tokOffset);

            tok = argFuncRetEnd->next();
        }

        // pointer/reference to array
        else if (Token::Match(tokOffset, "( *|& %type% ) [")) {
            ptrToArray = (tokOffset->next()->str() == "*");
            refToArray = !ptrToArray;
            tokOffset = tokOffset->tokAt(2);
            typeName = tokOffset;
            arrayStart = tokOffset->tokAt(2);
            arrayEnd = arrayStart->link();
            if (!arrayEnd)
                syntaxError(arrayStart);

            tok = arrayEnd->next();
        }

        // pointer to class member
        else if (Token::Match(tokOffset, "( %type% :: * %type% ) ;")) {
            tokOffset = tokOffset->tokAt(2);
            namespaceStart = tokOffset->previous();
            namespaceEnd = tokOffset;
            ptrMember = true;
            tokOffset = tokOffset->tokAt(2);
            typeName = tokOffset;
            tok = tokOffset->tokAt(2);
        }

        // unhandled typedef, skip it and continue
        else {
            unsupportedTypedef(typeDef);
            tok = deleteInvalidTypedef(typeDef);
            if (tok == list.front())
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
            continue;
        }

        bool done = false;
        bool ok = true;

        while (!done) {
            std::string pattern = typeName->str();
            unsigned int scope = 0;
            bool simplifyType = false;
            bool inMemberFunc = false;
            int memberScope = 0;
            bool globalScope = false;
            std::size_t classLevel = spaceInfo.size();

            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (_settings->terminated())
                    return;

                if (tok2->link()) { // Pre-check for performance
                    // check for end of scope
                    if (tok2->str() == "}") {
                        // check for end of member function
                        if (inMemberFunc) {
                            --memberScope;
                            if (memberScope == 0)
                                inMemberFunc = false;
                        }

                        if (classLevel > 0 && tok2 == spaceInfo[classLevel - 1].classEnd) {
                            --classLevel;
                            pattern.clear();

                            for (std::size_t i = classLevel; i < spaceInfo.size(); ++i)
                                pattern += (spaceInfo[i].className + " :: ");

                            pattern += typeName->str();
                        } else {
                            if (scope == 0)
                                break;
                            --scope;
                        }
                    }

                    // check for member functions
                    else if (isCPP() && Token::Match(tok2, ") const| {")) {
                        const Token *func = tok2->link()->previous();
                        if (!func || !func->previous()) // Ticket #4239
                            continue;

                        /** @todo add support for multi-token operators */
                        if (func->previous()->str() == "operator")
                            func = func->previous();

                        if (!func->previous())
                            syntaxError(func);

                        // check for qualifier
                        if (func->previous()->str() == "::") {
                            // check for available and matching class name
                            if (!spaceInfo.empty() && classLevel < spaceInfo.size() &&
                                func->strAt(-2) == spaceInfo[classLevel].className) {
                                memberScope = 0;
                                inMemberFunc = true;
                            }
                        }
                    }

                    // check for entering a new scope
                    else if (tok2->str() == "{") {
                        // check for entering a new namespace
                        if (isCPP() && tok2->strAt(-2) == "namespace") {
                            if (classLevel < spaceInfo.size() &&
                                spaceInfo[classLevel].isNamespace &&
                                spaceInfo[classLevel].className == tok2->previous()->str()) {
                                spaceInfo[classLevel].classEnd = tok2->link();
                                ++classLevel;
                                pattern.clear();
                                for (std::size_t i = classLevel; i < spaceInfo.size(); ++i)
                                    pattern += (spaceInfo[i].className + " :: ");

                                pattern += typeName->str();
                            }
                            ++scope;
                        }

                        // keep track of scopes within member function
                        if (inMemberFunc)
                            ++memberScope;

                        ++scope;
                    }
                }

                // check for operator typedef
                /** @todo add support for multi-token operators */
                else if (isCPP() &&
                         tok2->str() == "operator" &&
                         tok2->next() &&
                         tok2->next()->str() == typeName->str() &&
                         tok2->linkAt(2) &&
                         tok2->strAt(2) == "(" &&
                         Token::Match(tok2->linkAt(2), ") const| {")) {
                    // check for qualifier
                    if (tok2->previous()->str() == "::") {
                        // check for available and matching class name
                        if (!spaceInfo.empty() && classLevel < spaceInfo.size() &&
                            tok2->strAt(-2) == spaceInfo[classLevel].className) {
                            tok2 = tok2->next();
                            simplifyType = true;
                        }
                    }
                }

                // check for typedef that can be substituted
                else if (Token::simpleMatch(tok2, pattern.c_str()) ||
                         (inMemberFunc && tok2->str() == typeName->str())) {

                    // member function class variables don't need qualification
                    if (!(inMemberFunc && tok2->str() == typeName->str()) && pattern.find("::") != std::string::npos) { // has a "something ::"
                        Token *start = tok2;
                        std::size_t count = 0;
                        int back = int(classLevel) - 1;
                        bool good = true;
                        // check for extra qualification
                        while (back >= 0 && Token::Match(start->tokAt(-2), "%type% ::")) {
                            if (start->strAt(-2) == spaceInfo[back].className) {
                                start = start->tokAt(-2);
                                back--;
                                count++;
                            } else {
                                good = false;
                                break;
                            }
                        }
                        // check global namespace
                        if (good && back == 0 && start->strAt(-1) == "::")
                            good = false;

                        if (good) {
                            // remove any extra qualification if present
                            while (count) {
                                tok2->tokAt(-3)->deleteNext(2);
                                --count;
                            }

                            // remove global namespace if present
                            if (tok2->strAt(-1) == "::") {
                                tok2->tokAt(-2)->deleteNext();
                                globalScope = true;
                            }

                            // remove qualification if present
                            for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                                tok2->deleteNext(2);
                            }
                            simplifyType = true;
                        }
                    } else {
                        if (tok2->strAt(-1) == "::") {
                            std::size_t relativeSpaceInfoSize = spaceInfo.size();
                            Token * tokBeforeType = tok2->previous();
                            while (relativeSpaceInfoSize != 0 &&
                                   tokBeforeType && tokBeforeType->str() == "::" &&
                                   tokBeforeType->strAt(-1) == spaceInfo[relativeSpaceInfoSize-1].className) {
                                tokBeforeType = tokBeforeType->tokAt(-2);
                                --relativeSpaceInfoSize;
                            }
                            if (tokBeforeType && tokBeforeType->str() != "::") {
                                Token::eraseTokens(tokBeforeType, tok2);
                                simplifyType = true;
                            }
                        } else if (Token::Match(tok2->previous(), "case|;|{|} %type% :")) {
                            tok2 = tok2->next();
                        } else if (duplicateTypedef(&tok2, typeName, typeDef)) {
                            // skip to end of scope if not already there
                            if (tok2->str() != "}") {
                                while (tok2->next()) {
                                    if (tok2->next()->str() == "{")
                                        tok2 = tok2->linkAt(1)->previous();
                                    else if (tok2->next()->str() == "}")
                                        break;

                                    tok2 = tok2->next();
                                }
                            }
                        } else if (Token::Match(tok2->tokAt(-2), "%type% *|&")) {
                            // Ticket #5868: Don't substitute variable names
                        } else if (tok2->previous()->str() != ".") {
                            simplifyType = true;
                        }
                    }
                }

                if (simplifyType) {
                    // can't simplify 'operator functionPtr ()' and 'functionPtr operator ... ()'
                    if (functionPtr && (tok2->previous()->str() == "operator" ||
                                        (tok2->next() && tok2->next()->str() == "operator"))) {
                        simplifyType = false;
                        tok2 = tok2->next();
                        continue;
                    }

                    // There are 2 categories of typedef substitutions:
                    // 1. variable declarations that preserve the variable name like
                    //    global, local, and function parameters
                    // 2. not variable declarations that have no name like derived
                    //    classes, casts, operators, and template parameters

                    // try to determine which category this substitution is
                    bool inCast = false;
                    bool inTemplate = false;
                    bool inOperator = false;
                    bool inSizeof = false;

                    const bool sameStartEnd = (typeStart == typeEnd);

                    // check for derived class: class A : some_typedef {
                    const bool isDerived = Token::Match(tok2->previous(), "public|protected|private %type% {|,");

                    // check for cast: (some_typedef) A or static_cast<some_typedef>(A)
                    // todo: check for more complicated casts like: (const some_typedef *)A
                    if ((tok2->previous()->str() == "(" && tok2->next()->str() == ")" && tok2->strAt(-2) != "sizeof") ||
                        (tok2->previous()->str() == "<" && Token::simpleMatch(tok2->next(), "> (")))
                        inCast = true;

                    // check for template parameters: t<some_typedef> t1
                    else if (Token::Match(tok2->previous(), "<|,") &&
                             Token::Match(tok2->next(), "&|*| &|*| >|,"))
                        inTemplate = true;

                    else if (Token::Match(tok2->tokAt(-2), "sizeof ( %type% )"))
                        inSizeof = true;

                    // check for operator
                    if (tok2->strAt(-1) == "operator" ||
                        Token::simpleMatch(tok2->tokAt(-2), "operator const"))
                        inOperator = true;

                    if (typeStart->str() == "typename" && tok2->strAt(-1)=="typename") {
                        // Remove one typename if it is already contained in the goal
                        typeStart = typeStart->next();
                    }

                    // skip over class or struct in derived class declaration
                    bool structRemoved = false;
                    if (isDerived && Token::Match(typeStart, "class|struct")) {
                        if (typeStart->str() == "struct")
                            structRemoved = true;
                        typeStart = typeStart->next();
                    }
                    if (typeStart->str() == "struct" && Token::Match(tok2, "%name% ::"))
                        typeStart = typeStart->next();

                    if (sameStartEnd)
                        typeEnd = typeStart;

                    // start substituting at the typedef name by replacing it with the type
                    tok2->str(typeStart->str());

                    // restore qualification if it was removed
                    if (typeStart->str() == "struct" || structRemoved) {
                        if (structRemoved)
                            tok2 = tok2->previous();

                        if (globalScope) {
                            tok2->insertToken("::");
                            tok2 = tok2->next();
                        }

                        for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                            tok2->insertToken(spaceInfo[i].className);
                            tok2 = tok2->next();
                            tok2->insertToken("::");
                            tok2 = tok2->next();
                        }
                    }

                    // add remainder of type
                    tok2 = copyTokens(tok2, typeStart->next(), typeEnd);

                    if (!pointers.empty()) {
                        for (std::list<std::string>::const_iterator iter = pointers.begin(); iter != pointers.end(); ++iter) {
                            tok2->insertToken(*iter);
                            tok2 = tok2->next();
                        }
                    }

                    if (funcStart && funcEnd) {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;
                        tok2 = copyTokens(tok2, funcStart, funcEnd);

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        if (!tok2)
                            break;

                        tok2->insertToken(")");
                        tok2 = tok2->next();
                        Token::createMutualLinks(tok2, tok3);

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        if (specStart) {
                            Token *spec = specStart;
                            tok2->insertToken(spec->str());
                            tok2 = tok2->next();
                            while (spec != specEnd) {
                                spec = spec->next();
                                tok2->insertToken(spec->str());
                                tok2 = tok2->next();
                            }
                        }
                    }

                    else if (functionPtr || functionRef || function) {
                        // don't add parentheses around function names because it
                        // confuses other simplifications
                        bool needParen = true;
                        if (!inTemplate && function && tok2->next() && tok2->next()->str() != "*")
                            needParen = false;
                        if (needParen) {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                        }
                        Token *tok3 = tok2;
                        if (namespaceStart) {
                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd) {
                                tok2->insertToken(tok4->str());
                                tok2 = tok2->next();
                                tok4 = tok4->next();
                            }
                            tok2->insertToken(namespaceEnd->str());
                            tok2 = tok2->next();
                        }
                        if (functionPtr) {
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        } else if (functionRef) {
                            tok2->insertToken("&");
                            tok2 = tok2->next();
                        }

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        if (needParen) {
                            if (!tok2)
                                syntaxError(nullptr);

                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok3);
                        }
                        if (!tok2)
                            syntaxError(nullptr);

                        tok2 = copyTokens(tok2, argStart, argEnd);
                        if (inTemplate) {
                            if (!tok2)
                                syntaxError(nullptr);

                            tok2 = tok2->next();
                        }

                        if (specStart) {
                            Token *spec = specStart;
                            tok2->insertToken(spec->str());
                            tok2 = tok2->next();
                            while (spec != specEnd) {
                                spec = spec->next();
                                tok2->insertToken(spec->str());
                                tok2 = tok2->next();
                            }
                        }
                    } else if (functionRetFuncPtr || functionPtrRetFuncPtr) {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;
                        tok2->insertToken("*");
                        tok2 = tok2->next();

                        Token * tok4 = nullptr;
                        if (functionPtrRetFuncPtr) {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                            tok4 = tok2;
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        }

                        // skip over variable name if there
                        if (!inCast) {
                            if (!tok2 || !tok2->next())
                                syntaxError(nullptr);

                            if (tok2->next()->str() != ")")
                                tok2 = tok2->next();
                        }

                        if (tok4 && functionPtrRetFuncPtr) {
                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok4);
                        }

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        tok2->insertToken(")");
                        tok2 = tok2->next();
                        Token::createMutualLinks(tok2, tok3);

                        tok2 = copyTokens(tok2, argFuncRetStart, argFuncRetEnd);
                    } else if (ptrToArray || refToArray) {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;

                        if (ptrToArray)
                            tok2->insertToken("*");
                        else
                            tok2->insertToken("&");
                        tok2 = tok2->next();

                        // skip over name
                        if (tok2->next() && tok2->next()->str() != ")") {
                            if (tok2->next()->str() != "(")
                                tok2 = tok2->next();

                            // check for function and skip over args
                            if (tok2 && tok2->next() && tok2->next()->str() == "(")
                                tok2 = tok2->next()->link();

                            // check for array
                            if (tok2 && tok2->next() && tok2->next()->str() == "[")
                                tok2 = tok2->next()->link();
                        } else {
                            // syntax error
                        }

                        tok2->insertToken(")");
                        Token::createMutualLinks(tok2->next(), tok3);
                    } else if (ptrMember) {
                        if (Token::simpleMatch(tok2, "* (")) {
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        } else {
                            // This is the case of casting operator.
                            // Name is not available, and () should not be
                            // inserted
                            const bool castOperator = inOperator && Token::Match(tok2, "%type% (");
                            Token *openParenthesis = nullptr;

                            if (!castOperator) {
                                tok2->insertToken("(");
                                tok2 = tok2->next();

                                openParenthesis = tok2;
                            }

                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd) {
                                tok2->insertToken(tok4->str());
                                tok2 = tok2->next();
                                tok4 = tok4->next();
                            }
                            tok2->insertToken(namespaceEnd->str());
                            tok2 = tok2->next();

                            tok2->insertToken("*");
                            tok2 = tok2->next();

                            if (openParenthesis) {
                                // skip over name
                                tok2 = tok2->next();

                                tok2->insertToken(")");
                                tok2 = tok2->next();

                                Token::createMutualLinks(tok2, openParenthesis);
                            }
                        }
                    } else if (typeOf) {
                        tok2 = copyTokens(tok2, argStart, argEnd);
                    } else if (tok2->strAt(2) == "[") {
                        do {
                            if (!tok2->linkAt(2))
                                syntaxError(tok2);

                            tok2 = tok2->linkAt(2)->previous();
                        } while (tok2->strAt(2) == "[");
                    }

                    if (arrayStart && arrayEnd) {
                        do {
                            if (!tok2->next())
                                syntaxError(tok2); // can't recover so quit

                            if (!inCast && !inSizeof)
                                tok2 = tok2->next();

                            // reference to array?
                            if (tok2->str() == "&") {
                                tok2 = tok2->previous();
                                tok2->insertToken("(");
                                Token *tok3 = tok2->next();

                                // handle missing variable name
                                if (tok2->strAt(3) == ")" || tok2->strAt(3) == ",")
                                    tok2 = tok2->tokAt(2);
                                else
                                    tok2 = tok2->tokAt(3);
                                if (!tok2)
                                    syntaxError(nullptr);

                                tok2->insertToken(")");
                                tok2 = tok2->next();
                                Token::createMutualLinks(tok2, tok3);
                            }

                            if (!tok2->next())
                                syntaxError(tok2); // can't recover so quit

                            tok2 = copyTokens(tok2, arrayStart, arrayEnd);
                            if (!tok2->next())
                                syntaxError(tok2);

                            tok2 = tok2->next();

                            if (tok2->str() == "=") {
                                if (!tok2->next())
                                    syntaxError(tok2);
                                if (tok2->next()->str() == "{")
                                    tok2 = tok2->next()->link()->next();
                                else if (tok2->next()->str().at(0) == '\"')
                                    tok2 = tok2->tokAt(2);
                            }
                        } while (Token::Match(tok2, ", %name% ;|=|,"));
                    }

                    simplifyType = false;
                }
                if (!tok2)
                    break;
            }

            if (!tok)
                syntaxError(nullptr);

            if (tok->str() == ";")
                done = true;
            else if (tok->str() == ",") {
                arrayStart = nullptr;
                arrayEnd = nullptr;
                tokOffset = tok->next();
                pointers.clear();

                while (Token::Match(tokOffset, "*|&")) {
                    pointers.push_back(tokOffset->str());
                    tokOffset = tokOffset->next();
                }

                if (Token::Match(tokOffset, "%type%")) {
                    typeName = tokOffset;
                    tokOffset = tokOffset->next();

                    if (tokOffset && tokOffset->str() == "[") {
                        arrayStart = tokOffset;

                        for (;;) {
                            while (tokOffset->next() && !Token::Match(tokOffset->next(), ";|,"))
                                tokOffset = tokOffset->next();

                            if (!tokOffset->next())
                                return; // invalid input
                            else if (tokOffset->next()->str() == ";")
                                break;
                            else if (tokOffset->str() == "]")
                                break;
                            else
                                tokOffset = tokOffset->next();
                        }

                        arrayEnd = tokOffset;
                        tokOffset = tokOffset->next();
                    }

                    if (Token::Match(tokOffset, ";|,"))
                        tok = tokOffset;
                    else {
                        // we encountered a typedef we don't support yet so just continue
                        done = true;
                        ok = false;
                    }
                } else {
                    // we encountered a typedef we don't support yet so just continue
                    done = true;
                    ok = false;
                }
            } else {
                // something is really wrong (internal error)
                done = true;
                ok = false;
            }
        }

        if (ok) {
            // remove typedef
            Token::eraseTokens(typeDef, tok);

            if (typeDef != list.front()) {
                tok = typeDef->previous();
                tok->deleteNext();
                //no need to remove last token in the list
                if (tok->tokAt(2))
                    tok->deleteNext();
            } else {
                list.front()->deleteThis();
                //no need to remove last token in the list
                if (list.front()->next())
                    list.front()->deleteThis();
                tok = list.front();
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
            }
        }
    }
}

void Tokenizer::simplifyMulAndParens()
{
    if (!list.front())
        return;
    for (Token *tok = list.front()->tokAt(3); tok; tok = tok->next()) {
        if (tok->isName()) {
            //fix ticket #2784 - improved by ticket #3184
            unsigned int closedpars = 0;
            Token *tokend = tok->next();
            Token *tokbegin = tok->previous();
            while (tokend && tokend->str() == ")") {
                ++closedpars;
                tokend = tokend->next();
            }
            if (!tokend || !(tokend->isAssignmentOp()))
                continue;
            while (Token::Match(tokbegin, "&|(")) {
                if (tokbegin->str() == "&") {
                    if (Token::Match(tokbegin->tokAt(-2), "[;{}&(] *")) {
                        //remove '* &'
                        tokbegin = tokbegin->tokAt(-2);
                        tokbegin->deleteNext(2);
                    } else if (Token::Match(tokbegin->tokAt(-3), "[;{}&(] * (")) {
                        if (!closedpars)
                            break;
                        --closedpars;
                        //remove ')'
                        tok->deleteNext();
                        //remove '* ( &'
                        tokbegin = tokbegin->tokAt(-3);
                        tokbegin->deleteNext(3);
                    } else
                        break;
                } else if (tokbegin->str() == "(") {
                    if (!closedpars)
                        break;

                    //find consecutive opening parentheses
                    unsigned int openpars = 0;
                    while (tokbegin && tokbegin->str() == "(" && openpars <= closedpars) {
                        ++openpars;
                        tokbegin = tokbegin->previous();
                    }
                    if (!tokbegin || openpars > closedpars)
                        break;

                    if ((openpars == closedpars && Token::Match(tokbegin, "[;{}]")) ||
                        Token::Match(tokbegin->tokAt(-2), "[;{}&(] * &") ||
                        Token::Match(tokbegin->tokAt(-3), "[;{}&(] * ( &")) {
                        //remove the excessive parentheses around the variable
                        while (openpars) {
                            tok->deleteNext();
                            tokbegin->deleteNext();
                            --closedpars;
                            --openpars;
                        }
                    } else
                        break;
                }
            }
        }
    }
}

bool Tokenizer::createTokens(std::istream &code,
                             const std::string& FileName)
{
    // make sure settings specified
    assert(_settings);

    return list.createTokens(code, Path::getRelativePath(Path::simplifyPath(FileName), _settings->basePaths));
}

bool Tokenizer::simplifyTokens1(const std::string &configuration)
{
    // Fill the map _typeSize..
    fillTypeSizes();

    _configuration = configuration;

    if (!simplifyTokenList1(list.getFiles().front().c_str()))
        return false;

    list.createAst();
    list.validateAst();

    createSymbolDatabase();

    // Use symbol database to identify rvalue references. Split && to & &. This is safe, since it doesn't delete any tokens (which might be referenced by symbol database)
    for (std::size_t i = 0; i < _symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = _symbolDatabase->getVariableFromVarId(i);
        if (var && var->isRValueReference()) {
            Token* endTok = const_cast<Token*>(var->typeEndToken());
            endTok->str("&");
            endTok->astOperand1(nullptr);
            endTok->astOperand2(nullptr);
            endTok->insertToken("&");
            endTok->next()->scope(endTok->scope());
        }
    }

    _symbolDatabase->setValueTypeInTokenList();
    ValueFlow::setValues(&list, _symbolDatabase, _errorLogger, _settings);

    printDebugOutput(1);

    return true;
}

bool Tokenizer::tokenize(std::istream &code,
                         const char FileName[],
                         const std::string &configuration)
{
    if (!createTokens(code, FileName))
        return false;

    return simplifyTokens1(configuration);
}
//---------------------------------------------------------------------------

void Tokenizer::findComplicatedSyntaxErrorsInTemplates()
{
    validate();
    TemplateSimplifier::checkComplicatedSyntaxErrorsInTemplates(list.front());
}

void Tokenizer::checkForEnumsWithTypedef()
{
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "enum %name% {")) {
            tok = tok->tokAt(2);
            const Token *tok2 = Token::findsimplematch(tok, "typedef", tok->link());
            if (tok2)
                syntaxError(tok2);
            tok = tok->link();
        }
    }
}

void Tokenizer::fillTypeSizes()
{
    _typeSize.clear();
    _typeSize["char"] = 1;
    _typeSize["_Bool"] = _settings->sizeof_bool;
    _typeSize["bool"] = _settings->sizeof_bool;
    _typeSize["short"] = _settings->sizeof_short;
    _typeSize["int"] = _settings->sizeof_int;
    _typeSize["long"] = _settings->sizeof_long;
    _typeSize["float"] = _settings->sizeof_float;
    _typeSize["double"] = _settings->sizeof_double;
    _typeSize["wchar_t"] = _settings->sizeof_wchar_t;
    _typeSize["size_t"] = _settings->sizeof_size_t;
    _typeSize["*"] = _settings->sizeof_pointer;
}

void Tokenizer::combineOperators()
{
    const bool cpp = isCPP();

    // Combine tokens..
    for (Token *tok = list.front();
         tok && tok->next();
         tok = tok->next()) {
        const char c1 = tok->str()[0];

        if (tok->str().length() == 1 && tok->next()->str().length() == 1) {
            const char c2 = tok->next()->str()[0];

            // combine +-*/ and =
            if (c2 == '=' && (std::strchr("+-*/%&|^=!<>", c1))) {
                tok->str(tok->str() + c2);
                tok->deleteNext();
                continue;
            }

            // simplify "->"
            else if (c1 == '-' && c2 == '>') {
                // If the preceding sequence is "( & %name% )", replace it by "%name%"
                Token *t = tok->tokAt(-4);
                if (Token::Match(t, "( & %name% )")) {
                    t->deleteThis();
                    t->deleteThis();
                    t->deleteNext();
                    tok = t->next();
                }
                // Replace "->" with "."
                tok->str(".");
                tok->originalName("->");
                tok->deleteNext();
                continue;
            }
        } else if (tok->next()->str() == "=") {
            if (tok->str() == ">>") {
                tok->str(">>=");
                tok->deleteNext();
            } else if (tok->str() == "<<") {
                tok->str("<<=");
                tok->deleteNext();
            }
        } else if (cpp && (c1 == 'p' || c1 == '_') &&
                   Token::Match(tok, "private|protected|public|__published : !!:")) {
            bool simplify = false;
            unsigned int par = 0U;
            for (const Token *prev = tok->previous(); prev; prev = prev->previous()) {
                if (prev->str() == ")") {
                    ++par;
                } else if (prev->str() == "(") {
                    if (par == 0U)
                        break;
                    --par;
                }
                if (par != 0U || prev->str() == "(")
                    continue;
                if (Token::Match(prev, "[;{}]")) {
                    simplify = true;
                    break;
                }
                if (prev->isName() && prev->isUpperCaseName())
                    continue;
                if (prev->isName() && prev->str().back() == ':')
                    simplify = true;
                break;
            }
            if (simplify) {
                tok->str(tok->str() + ":");
                tok->deleteNext();
            }
        }
    }
}

void Tokenizer::combineStringAndCharLiterals()
{
    // Combine wide strings and wide characters
    for (Token *tok = list.front();
         tok;
         tok = tok->next()) {
        if (tok->str() == "L" && tok->next() && (tok->next()->tokType() == Token::eString || tok->next()->tokType() == Token::eChar)) {
            // Combine 'L "string"' and 'L 'c''
            tok->str(tok->next()->str());
            tok->deleteNext();
            tok->isLong(true);
        }
    }

    // Combine strings
    for (Token *tok = list.front();
         tok;
         tok = tok->next()) {
        if (tok->str()[0] != '"')
            continue;

        tok->str(simplifyString(tok->str()));
        while (tok->next() && tok->next()->tokType() == Token::eString) {
            // Two strings after each other, combine them
            tok->concatStr(simplifyString(tok->next()->str()));
            tok->deleteNext();
        }
    }
}

void Tokenizer::concatenateNegativeNumberAndAnyPositive()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "?|:|,|(|[|{|return|case|sizeof|%op% +|-") || tok->tokType() == Token::eIncDecOp)
            continue;

        while (tok->next() && tok->next()->str() == "+")
            tok->deleteNext();

        if (Token::Match(tok->next(), "- %num%")) {
            tok->deleteNext();
            tok->next()->str("-" + tok->next()->str());
        }
    }
}

void Tokenizer::simplifyExternC()
{
    if (isC())
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "extern \"C\"")) {
            if (tok->strAt(2) == "{") {
                tok->linkAt(2)->deleteThis();
                tok->deleteNext(2);
            } else
                tok->deleteNext();
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyRoundCurlyParentheses()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "[;{}:] ( {") &&
               Token::simpleMatch(tok->linkAt(2), "} ) ;")) {
            if (tok->str() == ":" && !Token::Match(tok->tokAt(-2),"[;{}] %type% :"))
                break;
            Token *end = tok->linkAt(2)->tokAt(-3);
            if (Token::Match(end, "[;{}] %num%|%str% ;"))
                end->deleteNext(2);
            tok->linkAt(2)->previous()->deleteNext(3);
            tok->deleteNext(2);
        }
        if (Token::Match(tok, "( { %bool%|%char%|%num%|%str%|%name% ; } )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext(3);
        }
    }
}

void Tokenizer::simplifySQL()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "EXEC SQL")) {
            const Token *end = tok->tokAt(2);
            while (end && end->str() != ";")
                end = end->next();

            std::string instruction = tok->stringifyList(end);
            // delete all tokens until ';'
            Token::eraseTokens(tok, end);

            // insert "asm ( "instruction" ) ;"
            tok->str("asm");
            // it can happen that 'end' is NULL when wrong code is inserted
            if (!tok->next())
                tok->insertToken(";");
            tok->insertToken(")");
            tok->insertToken("\"" + instruction + "\"");
            tok->insertToken("(");
            // jump to ';' and continue
            tok = tok->tokAt(3);
        }
    }
}

void Tokenizer::simplifyArrayAccessSyntax()
{
    // 0[a] -> a[0]
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isNumber() && Token::Match(tok, "%num% [ %name% ]")) {
            const std::string number(tok->str());
            Token* indexTok = tok->tokAt(2);
            tok->str(indexTok->str());
            tok->varId(indexTok->varId());
            indexTok->str(number);
        }
    }
}

void Tokenizer::simplifyParameterVoid()
{
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% ( void )"))
            tok->next()->deleteNext();
    }
}

void Tokenizer::simplifyRedundantConsecutiveBraces()
{
    // Remove redundant consecutive braces, i.e. '.. { { .. } } ..' -> '.. { .. } ..'.
    for (Token *tok = list.front(); tok;) {
        if (Token::simpleMatch(tok, "= {")) {
            tok = tok->linkAt(1);
        } else if (Token::simpleMatch(tok, "{ {") && Token::simpleMatch(tok->next()->link(), "} }")) {
            //remove internal parentheses
            tok->next()->link()->deleteThis();
            tok->deleteNext();
        } else
            tok = tok->next();
    }
}

void Tokenizer::simplifyDoublePlusAndDoubleMinus()
{
    // Convert - - into + and + - into -
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (tok->next()) {
            if (tok->str() == "+") {
                if (tok->next()->str()[0] == '-') {
                    tok = tok->next();
                    if (tok->str().size() == 1) {
                        tok = tok->previous();
                        tok->str("-");
                        tok->deleteNext();
                    } else if (tok->isNumber()) {
                        tok->str(tok->str().substr(1));
                        tok = tok->previous();
                        tok->str("-");
                    }
                    continue;
                }
            } else if (tok->str() == "-") {
                if (tok->next()->str()[0] == '-') {
                    tok = tok->next();
                    if (tok->str().size() == 1) {
                        tok = tok->previous();
                        tok->str("+");
                        tok->deleteNext();
                    } else if (tok->isNumber()) {
                        tok->str(tok->str().substr(1));
                        tok = tok->previous();
                        tok->str("+");
                    }
                    continue;
                }
            }

            break;
        }
    }
}

/** Specify array size if it hasn't been given */

void Tokenizer::arraySize()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || !Token::Match(tok, "%var% [ ] ="))
            continue;
        bool addlength = false;
        if (Token::Match(tok, "%var% [ ] = { %str% } ;")) {
            Token *t = tok->tokAt(3);
            t->deleteNext();
            t->next()->deleteNext();
            addlength = true;
        }

        if (addlength || Token::Match(tok, "%var% [ ] = %str% ;")) {
            tok = tok->next();
            std::size_t sz = Token::getStrSize(tok->tokAt(3));
            tok->insertToken(MathLib::toString(sz));
            tok = tok->tokAt(5);
        }

        else if (Token::Match(tok, "%var% [ ] = {")) {
            MathLib::biguint sz = 1;
            tok = tok->next();
            Token *end = tok->linkAt(3);
            for (Token *tok2 = tok->tokAt(4); tok2 && tok2 != end; tok2 = tok2->next()) {
                if (tok2->link() && Token::Match(tok2, "{|(|[|<")) {
                    if (tok2->str() == "[" && tok2->link()->strAt(1) == "=") { // designated initializer
                        if (Token::Match(tok2, "[ %num% ]"))
                            sz = std::max(sz, MathLib::toULongNumber(tok2->strAt(1)) + 1U);
                        else {
                            sz = 0;
                            break;
                        }
                    }
                    tok2 = tok2->link();
                } else if (tok2->str() == ",") {
                    if (!Token::Match(tok2->next(), "[},]"))
                        ++sz;
                    else {
                        tok2 = tok2->previous();
                        tok2->deleteNext();
                    }
                }
            }

            if (sz != 0)
                tok->insertToken(MathLib::toString(sz));

            tok = end->next() ? end->next() : end;
        }
    }
}

static Token *skipTernaryOp(Token *tok)
{
    unsigned int colonlevel = 1;
    while (nullptr != (tok = tok->next())) {
        if (tok->str() == "?") {
            ++colonlevel;
        } else if (tok->str() == ":") {
            --colonlevel;
            if (colonlevel == 0) {
                tok = tok->next();
                break;
            }
        }
        if (tok->link() && Token::Match(tok, "[(<]"))
            tok = tok->link();
        else if (Token::Match(tok->next(), "[{};)]"))
            break;
    }
    if (colonlevel) // Ticket #5214: Make sure the ':' matches the proper '?'
        return 0;
    return tok;
}

const Token * Tokenizer::startOfExecutableScope(const Token * tok)
{
    if (tok->str() != ")")
        return nullptr;

    tok = isFunctionHead(tok, ":{", true);

    if (Token::Match(tok, ": %name% [({]")) {
        while (Token::Match(tok, "[:,] %name% [({]"))
            tok = tok->linkAt(2)->next();
    }

    return (tok && tok->str() == "{") ? tok : nullptr;
}


/** simplify labels and case|default in the code: add a ";" if not already in.*/

void Tokenizer::simplifyLabelsCaseDefault()
{
    bool executablescope = false;
    unsigned int indentlevel = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Simplify labels in the executable scope..
        Token *start = const_cast<Token *>(startOfExecutableScope(tok));
        if (start) {
            tok = start;
            executablescope = true;
        }

        if (!executablescope)
            continue;

        if (tok->str() == "{") {
            if (tok->previous()->str() == "=")
                tok = tok->link();
            else
                ++indentlevel;
        } else if (tok->str() == "}") {
            --indentlevel;
            if (!indentlevel) {
                executablescope = false;
                continue;
            }
        } else if (Token::Match(tok, "(|["))
            tok = tok->link();

        if (Token::Match(tok, "[;{}:] case")) {
            while (nullptr != (tok = tok->next())) {
                if (Token::Match(tok, "(|[")) {
                    tok = tok->link();
                } else if (tok->str() == "?") {
                    Token *tok1 = skipTernaryOp(tok);
                    if (!tok1) {
                        syntaxError(tok);
                    }
                    tok = tok1;
                }
                if (Token::Match(tok->next(),"[:{};]"))
                    break;
            }
            if (!tok)
                break;
            if (tok->str() != "case" && tok->next() && tok->next()->str() == ":") {
                tok = tok->next();
                if (!tok->next())
                    syntaxError(tok);
                if (tok->next()->str() != ";" && tok->next()->str() != "case")
                    tok->insertToken(";");
                else
                    tok = tok->previous();
            } else {
                syntaxError(tok);
            }
        } else if (Token::Match(tok, "[;{}] %name% : !!;")) {
            tok = tok->tokAt(2);
            tok->insertToken(";");
        }
    }
}


void Tokenizer::simplifyCaseRange()
{
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "case %num% . . . %num% :")) {
            MathLib::bigint start = MathLib::toLongNumber(tok->strAt(1));
            MathLib::bigint end = MathLib::toLongNumber(tok->strAt(5));
            end = std::min(start + 50, end); // Simplify it 50 times at maximum
            if (start < end) {
                tok = tok->tokAt(2);
                tok->str(":");
                tok->deleteNext();
                tok->next()->str("case");
                for (MathLib::bigint i = end-1; i > start; i--) {
                    tok->insertToken(":");
                    tok->insertToken(MathLib::toString(i));
                    tok->insertToken("case");
                }
            }
        } else if (Token::Match(tok, "case %char% . . . %char% :")) {
            char start = tok->strAt(1)[1];
            char end = tok->strAt(5)[1];
            if (start < end) {
                tok = tok->tokAt(2);
                tok->str(":");
                tok->deleteNext();
                tok->next()->str("case");
                for (char i = end - 1; i > start; i--) {
                    tok->insertToken(":");
                    tok->insertToken(std::string(1, '\'') + i + '\'');
                    tok->insertToken("case");
                }
            }
        }
    }
}



void Tokenizer::simplifyTemplates()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // simple fix for sizeof used as template parameter
        // TODO: this is a bit hardcoded. make a bit more generic
        if (Token::Match(tok, "%name% < sizeof ( %type% ) >") && tok->tokAt(4)->isStandardType()) {
            Token * const tok3 = tok->next();
            const unsigned int sizeOfResult = sizeOfType(tok3->tokAt(3));
            tok3->deleteNext(4);
            tok3->insertToken(MathLib::toString(sizeOfResult));
        }
        // Ticket #6181: normalize C++11 template parameter list closing syntax
        if (tok->str() == "<" && TemplateSimplifier::templateParameters(tok)) {
            Token *endTok = tok->findClosingBracket();
            if (endTok && endTok->str() == ">>") {
                endTok->str(">");
                endTok->insertToken(">");
            }
        }
    }

    TemplateSimplifier::simplifyTemplates(
        list,
        _errorLogger,
        _settings,
#ifdef MAXTIME
        maxtime,
#else
        0, // ignored
#endif
        _codeWithTemplates);
}
//---------------------------------------------------------------------------


static bool setVarIdParseDeclaration(const Token **tok, const std::map<std::string,unsigned int> &variableId, bool executableScope, bool cpp, bool c)
{
    const Token *tok2 = *tok;
    if (!tok2->isName())
        return false;

    unsigned int typeCount = 0;
    unsigned int singleNameCount = 0;
    bool hasstruct = false;   // Is there a "struct" or "class"?
    bool bracket = false;
    bool ref = false;
    while (tok2) {
        if (tok2->isName()) {
            if (cpp && Token::Match(tok2, "namespace|public|private|protected"))
                return false;
            if (Token::Match(tok2, "struct|union|enum") || (!c && Token::Match(tok2, "class|typename"))) {
                hasstruct = true;
                typeCount = 0;
                singleNameCount = 0;
            } else if (tok2->str() == "const") {
                ;  // just skip "const"
            } else if (!hasstruct && variableId.find(tok2->str()) != variableId.end() && tok2->previous()->str() != "::") {
                ++typeCount;
                tok2 = tok2->next();
                if (!tok2 || tok2->str() != "::")
                    break;
            } else {
                if (tok2->str() != "void" || Token::Match(tok2, "void const| *|(")) // just "void" cannot be a variable type
                    ++typeCount;
                ++singleNameCount;
            }
        } else if (!c && ((TemplateSimplifier::templateParameters(tok2) > 0) ||
                          Token::simpleMatch(tok2, "< >") /* Ticket #4764 */)) {
            tok2 = tok2->findClosingBracket();
            if (tok2->str() != ">")
                break;
            singleNameCount = 1;
        } else if (Token::Match(tok2, "&|&&")) {
            ref = !bracket;
        } else if (singleNameCount == 1 && Token::Match(tok2, "( [*&]") && Token::Match(tok2->link()->next(), "(|[")) {
            bracket = true; // Skip: Seems to be valid pointer to array or function pointer
        } else if (tok2->str() == "::") {
            singleNameCount = 0;
        } else if (tok2->str() != "*" && tok2->str() != "::") {
            break;
        }
        tok2 = tok2->next();
    }

    if (tok2) {
        *tok = tok2;

        // In executable scopes, references must be assigned
        // Catching by reference is an exception
        if (executableScope && ref) {
            if (Token::Match(tok2, "(|=|{|:"))
                ;   // reference is assigned => ok
            else if (tok2->str() != ")" || tok2->link()->strAt(-1) != "catch")
                return false;   // not catching by reference => not declaration
        }
    }

    // Check if array declaration is valid (#2638)
    // invalid declaration: AAA a[4] = 0;
    if (typeCount >= 2 && executableScope && tok2 && tok2->str() == "[") {
        const Token *tok3 = tok2->link()->next();
        while (tok3 && tok3->str() == "[") {
            tok3 = tok3->link()->next();
        }
        if (Token::Match(tok3, "= %num%"))
            return false;
    }

    return bool(typeCount >= 2 && tok2 && Token::Match(tok2->tokAt(-2), "!!:: %type%"));
}


static void setVarIdStructMembers(Token **tok1,
                                  std::map<unsigned int, std::map<std::string, unsigned int> >& structMembers,
                                  unsigned int *_varId)
{
    Token *tok = *tok1;

    if (Token::Match(tok, "%name% = { . %name% =")) {
        const unsigned int struct_varid = tok->varId();
        if (struct_varid == 0)
            return;

        std::map<std::string, unsigned int>& members = structMembers[struct_varid];

        tok = tok->tokAt(3);
        while (tok->str() != "}") {
            if (Token::Match(tok, "{|[|("))
                tok = tok->link();
            if (Token::Match(tok->previous(), "[,{] . %name% =")) {
                tok = tok->next();
                const std::map<std::string, unsigned int>::iterator it = members.find(tok->str());
                if (it == members.end()) {
                    members[tok->str()] = ++(*_varId);
                    tok->varId(*_varId);
                } else {
                    tok->varId(it->second);
                }
            }
            tok = tok->next();
        }

        return;
    }

    while (Token::Match(tok->next(), ". %name% !!(")) {
        const unsigned int struct_varid = tok->varId();
        tok = tok->tokAt(2);
        if (struct_varid == 0)
            continue;

        // Don't set varid for template function
        if (TemplateSimplifier::templateParameters(tok->next()) > 0)
            break;

        std::map<std::string, unsigned int>& members = structMembers[struct_varid];
        const std::map<std::string, unsigned int>::iterator it = members.find(tok->str());
        if (it == members.end()) {
            members[tok->str()] = ++(*_varId);
            tok->varId(*_varId);
        } else {
            tok->varId(it->second);
        }
    }
    // tok can't be null
    *tok1 = tok;
}


void Tokenizer::setVarIdClassDeclaration(const Token * const startToken,
        const std::map<std::string, unsigned int> &variableId,
        const unsigned int scopeStartVarId,
        std::map<unsigned int, std::map<std::string,unsigned int> >& structMembers)
{
    // end of scope
    const Token * const endToken = startToken->link();

    // determine class name
    std::string className;
    for (const Token *tok = startToken->previous(); tok; tok = tok->previous()) {
        if (!tok->isName() && tok->str() != ":")
            break;
        if (Token::Match(tok, "class|struct|enum %type% [:{]")) {
            className = tok->next()->str();
            break;
        }
    }

    // replace varids..
    unsigned int indentlevel = 0;
    bool initList = false;
    bool inEnum = false;
    const Token *initListArgLastToken = nullptr;
    for (Token *tok = startToken->next(); tok != endToken; tok = tok->next()) {
        if (!tok)
            syntaxError(nullptr);
        if (initList) {
            if (tok == initListArgLastToken)
                initListArgLastToken = nullptr;
            else if (!initListArgLastToken &&
                     Token::Match(tok->previous(), "%name%|>|>> {|(") &&
                     Token::Match(tok->link(), "}|) ,|{"))
                initListArgLastToken = tok->link();
        }
        if (tok->str() == "{") {
            inEnum = isEnumStart(tok);
            if (initList && !initListArgLastToken)
                initList = false;
            ++indentlevel;
        } else if (tok->str() == "}") {
            --indentlevel;
            inEnum = false;
        } else if (initList && indentlevel == 0 && Token::Match(tok->previous(), "[,:] %name% [({]")) {
            const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
            if (it != variableId.end()) {
                tok->varId(it->second);
            }
        } else if (tok->isName() && tok->varId() <= scopeStartVarId) {
            if (indentlevel > 0 || initList) {
                if (Token::Match(tok->previous(), "::|.") && tok->strAt(-2) != "this" && !Token::simpleMatch(tok->tokAt(-5), "( * this ) ."))
                    continue;
                if (!tok->next())
                    syntaxError(nullptr);
                if (tok->next()->str() == "::") {
                    if (tok->str() == className)
                        tok = tok->tokAt(2);
                    else
                        continue;
                }

                if (!inEnum) {
                    const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
                    if (it != variableId.end()) {
                        tok->varId(it->second);
                        setVarIdStructMembers(&tok, structMembers, &_varId);
                    }
                }
            }
        } else if (indentlevel == 0 && tok->str() == ":" && !initListArgLastToken)
            initList = true;
    }
}



// Update the variable ids..
// Parse each function..
static void setVarIdClassFunction(const std::string &classname,
                                  Token * const startToken,
                                  const Token * const endToken,
                                  const std::map<std::string, unsigned int> &varlist,
                                  std::map<unsigned int, std::map<std::string, unsigned int> >& structMembers,
                                  unsigned int *_varId)
{
    for (Token *tok2 = startToken; tok2 && tok2 != endToken; tok2 = tok2->next()) {
        if (tok2->varId() != 0 || !tok2->isName())
            continue;
        if (Token::Match(tok2->tokAt(-2), ("!!" + classname + " ::").c_str()))
            continue;
        if (Token::Match(tok2->tokAt(-4), "%name% :: %name% ::")) // Currently unsupported
            continue;
        if (Token::Match(tok2->tokAt(-2), "!!this .") && !Token::simpleMatch(tok2->tokAt(-5), "( * this ) ."))
            continue;

        const std::map<std::string,unsigned int>::const_iterator it = varlist.find(tok2->str());
        if (it != varlist.end()) {
            tok2->varId(it->second);
            setVarIdStructMembers(&tok2, structMembers, _varId);
        }
    }
}



void Tokenizer::setVarId()
{
    // Clear all variable ids
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isName())
            tok->varId(0);
    }

    setPodTypes();

    setVarIdPass1();

    setVarIdPass2();
}


// Variable declarations can't start with "return" etc.
static const std::set<std::string> notstart_c = make_container< std::set<std::string> > ()
        << "goto" << "NOT" << "return" << "sizeof"<< "typedef";
static const std::set<std::string> notstart_cpp = make_container< std::set<std::string> > ()
        << notstart_c
        << "delete" << "friend" << "new" << "throw" << "using" << "virtual" << "explicit" << "const_cast" << "dynamic_cast" << "reinterpret_cast" << "static_cast" << "template";

void Tokenizer::setVarIdPass1()
{
    // Variable declarations can't start with "return" etc.
    const std::set<std::string>& notstart = (isC()) ? notstart_c : notstart_cpp;

    // variable id
    _varId = 0;
    std::map<std::string, unsigned int> variableId;
    std::map<unsigned int, std::map<std::string, unsigned int> > structMembers;
    std::stack< std::map<std::string, unsigned int> > scopeInfo;

    std::stack<VarIdScopeInfo> scopeStack;

    scopeStack.push(VarIdScopeInfo());
    std::stack<const Token *> functionDeclEndStack;
    bool initlist = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!functionDeclEndStack.empty() && tok == functionDeclEndStack.top()) {
            functionDeclEndStack.pop();
            if (tok->str() == ":")
                initlist = true;
            else if (tok->str() == ";") {
                if (scopeInfo.empty())
                    cppcheckError(tok);
                variableId.swap(scopeInfo.top());
                scopeInfo.pop();
            } else if (tok->str() == "{")
                scopeStack.push(VarIdScopeInfo(true, scopeStack.top().isStructInit || tok->strAt(-1) == "=", /*isEnum=*/false, _varId));
        } else if (!initlist && tok->str()=="(") {
            const Token * newFunctionDeclEnd = nullptr;
            if (!scopeStack.top().isExecutable)
                newFunctionDeclEnd = isFunctionHead(tok, "{:;");
            else {
                Token const * const tokenLinkNext = tok->link()->next();
                if (tokenLinkNext && tokenLinkNext->str() == "{") // might be for- or while-loop or if-statement
                    newFunctionDeclEnd = tokenLinkNext;
            }
            if (newFunctionDeclEnd &&
                (functionDeclEndStack.empty() || newFunctionDeclEnd != functionDeclEndStack.top())) {
                functionDeclEndStack.push(newFunctionDeclEnd);
                scopeInfo.push(variableId);
            }
        } else if (Token::Match(tok, "{|}")) {
            const Token * const startToken = (tok->str() == "{") ? tok : tok->link();

            // parse anonymous unions as part of the current scope
            if (!Token::Match(startToken->previous(), "union|struct|enum {") &&
                !(initlist && Token::Match(startToken->previous(), "%name%|>|>>") && Token::Match(startToken->link(), "} ,|{"))) {

                if (tok->str() == "{") {
                    bool isExecutable;
                    if (tok->strAt(-1) == ")" || Token::Match(tok->tokAt(-2), ") %type%") ||
                        (initlist && tok->strAt(-1) == "}")) {
                        isExecutable = true;
                    } else {
                        isExecutable = ((scopeStack.top().isExecutable || initlist || tok->strAt(-1) == "else") &&
                                        !isClassStructUnionEnumStart(tok));
                        if (!(scopeStack.top().isStructInit || tok->strAt(-1) == "="))
                            scopeInfo.push(variableId);
                    }
                    initlist = false;
                    scopeStack.push(VarIdScopeInfo(isExecutable, scopeStack.top().isStructInit || tok->strAt(-1) == "=", isEnumStart(tok), _varId));
                } else { /* if (tok->str() == "}") */
                    bool isNamespace = false;
                    for (const Token *tok1 = tok->link()->previous(); tok1 && tok1->isName(); tok1 = tok1->previous())
                        isNamespace |= (tok1->str() == "namespace");
                    // Set variable ids in class declaration..
                    if (!initlist && !isC() && !scopeStack.top().isExecutable && tok->link() && !isNamespace) {
                        setVarIdClassDeclaration(tok->link(),
                                                 variableId,
                                                 scopeStack.top().startVarid,
                                                 structMembers);
                    }

                    if (!scopeStack.top().isStructInit) {
                        if (scopeInfo.empty()) {
                            variableId.clear();
                        } else {
                            variableId.swap(scopeInfo.top());
                            scopeInfo.pop();
                        }
                    }

                    scopeStack.pop();
                    if (scopeStack.empty()) {  // should be impossible
                        scopeStack.push(VarIdScopeInfo());
                    }
                }
            }
        }

        if (!scopeStack.top().isStructInit &&
            (tok == list.front() ||
             Token::Match(tok, "[;{}]") ||
             (tok->str() == "(" && isFunctionHead(tok,"{")) ||
             (tok->str() == "(" && !scopeStack.top().isExecutable && isFunctionHead(tok,";:")) ||
             (tok->str() == "," && !scopeStack.top().isExecutable) ||
             (tok->isName() && tok->str().back() == ':'))) {

            // No variable declarations in sizeof
            if (Token::simpleMatch(tok->previous(), "sizeof (")) {
                continue;
            }

            if (_settings->terminated())
                return;

            // locate the variable name..
            const Token *tok2 = (tok->isName()) ? tok : tok->next();

            // private: protected: public: etc
            while (tok2 && tok2->str().back() == ':') {
                tok2 = tok2->next();
            }
            if (!tok2)
                break;

            // Variable declaration can't start with "return", etc
            if (notstart.find(tok2->str()) != notstart.end())
                continue;

            if (!isC() && Token::simpleMatch(tok2, "const new"))
                continue;

            bool decl = setVarIdParseDeclaration(&tok2, variableId, scopeStack.top().isExecutable, isCPP(), isC());
            if (decl) {
                const Token* prev2 = tok2->previous();
                if (Token::Match(prev2, "%type% [;[=,)]") && tok2->previous()->str() != "const")
                    ;
                else if (Token::Match(prev2, "%type% :") && tok->strAt(-1) == "for")
                    ;
                else if (Token::Match(prev2, "%type% ( !!)") && Token::simpleMatch(tok2->link(), ") ;")) {
                    // In C++ , a variable can't be called operator+ or something like that.
                    if (isCPP() &&
                        prev2->isOperatorKeyword())
                        continue;

                    const Token *tok3 = tok2->next();
                    if (!tok3->isStandardType() && tok3->str() != "void" && !Token::Match(tok3, "struct|union|class %type%") && tok3->str() != "." && !Token::Match(tok2->link()->previous(), "[&*]")) {
                        if (!scopeStack.top().isExecutable) {
                            // Detecting initializations with () in non-executable scope is hard and often impossible to be done safely. Thus, only treat code as a variable that definitely is one.
                            decl = false;
                            bool rhs = false;
                            for (; tok3; tok3 = tok3->nextArgumentBeforeCreateLinks2()) {
                                if (tok3->str() == "=") {
                                    rhs = true;
                                    continue;
                                }

                                if (tok3->str() == ",") {
                                    rhs = false;
                                    continue;
                                }

                                if (rhs)
                                    continue;

                                if (tok3->isLiteral() ||
                                    (tok3->isName() && (variableId.find(tok3->str()) != variableId.end())) ||
                                    tok3->isOp() ||
                                    tok3->str() == "(" ||
                                    notstart.find(tok3->str()) != notstart.end()) {
                                    decl = true;
                                    break;
                                }
                            }
                        }
                    } else
                        decl = false;
                } else if (isCPP() && Token::Match(prev2, "%type% {") && Token::simpleMatch(tok2->link(), "} ;")) { // C++11 initialization style
                    if (Token::Match(prev2, "do|try|else") || Token::Match(prev2->tokAt(-2), "struct|class|:"))
                        continue;
                } else
                    decl = false;

                if (decl) {
                    variableId[prev2->str()] = ++_varId;

                    // set varid for template parameters..
                    tok = tok->next();
                    while (Token::Match(tok, "%name%|::"))
                        tok = tok->next();
                    if (tok && tok->str() == "<") {
                        const Token *end = tok->findClosingBracket();
                        while (tok != end) {
                            if (tok->isName() && variableId.find(tok->str()) != variableId.end())
                                tok->varId(variableId[tok->str()]);
                            tok = tok->next();
                        }
                    }

                    tok = tok2->previous();
                }
            }
        }

        if (tok->isName()) {
            // don't set variable id after a struct|enum|union
            if (Token::Match(tok->previous(), "struct|enum|union") || (isCPP() && tok->strAt(-1) == "class"))
                continue;

            if (!isC()) {
                if (tok->previous() && tok->previous()->str() == "::")
                    continue;
                if (tok->next() && tok->next()->str() == "::")
                    continue;
            }

            // function declaration inside executable scope? Function declaration is of form: type name "(" args ")"
            if (scopeStack.top().isExecutable && Token::Match(tok, "%name% [,)]")) {
                bool par = false;
                const Token *start, *end;

                // search begin of function declaration
                for (start = tok; Token::Match(start, "%name%|*|&|,|("); start = start->previous()) {
                    if (start->str() == "(") {
                        if (par)
                            break;
                        par = true;
                    }
                    if (Token::Match(start, "[(,]")) {
                        if (!Token::Match(start, "[(,] %type% %name%|*|&"))
                            break;
                    }
                    if (start->varId() > 0U)
                        break;
                }

                // search end of function declaration
                for (end = tok->next(); Token::Match(end, "%name%|*|&|,"); end = end->next()) {}

                // there are tokens which can't appear at the begin of a function declaration such as "return"
                const bool isNotstartKeyword = start->next() && notstart.find(start->next()->str()) != notstart.end();

                // now check if it is a function declaration
                if (Token::Match(start, "[;{}] %type% %name%|*") && par && Token::simpleMatch(end, ") ;") && !isNotstartKeyword)
                    // function declaration => don't set varid
                    continue;
            }

            if (!scopeStack.top().isEnum) {
                const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
                if (it != variableId.end()) {
                    tok->varId(it->second);
                    setVarIdStructMembers(&tok, structMembers, &_varId);
                }
            }
        } else if (Token::Match(tok, "::|. %name%")) {
            // Don't set varid after a :: or . token
            tok = tok->next();
        } else if (tok->str() == ":" && Token::Match(tok->tokAt(-2), "class %type%")) {
            do {
                tok = tok->next();
            } while (tok && (tok->isName() || tok->str() == ","));
            if (!tok)
                break;
            tok = tok->previous();
        }
    }
}

void Tokenizer::setVarIdPass2()
{
    std::map<unsigned int, std::map<std::string, unsigned int> > structMembers;

    // Member functions and variables in this source
    std::list<Token *> allMemberFunctions;
    std::list<Token *> allMemberVars;
    if (!isC()) {
        for (Token *tok2 = list.front(); tok2; tok2 = tok2->next()) {
            const Token* tok3 = nullptr;
            if (Token::Match(tok2, "%name% :: ~| %name%"))
                tok3 = tok2->next();
            else if (Token::Match(tok2, "%name% <") && Token::Match(tok2->next()->findClosingBracket(),"> :: ~| %name%"))
                tok3 = tok2->next()->findClosingBracket()->next();
            else
                continue;

            while (Token::Match(tok3, ":: ~| %name%")) {
                tok3 = tok3->next();
                if (tok3->str() == "~")
                    tok3 = tok3->next();
                tok3 = tok3->next();
            }
            if (!tok3)
                syntaxError(tok2);
            const std::string& str3 = tok3->str();
            if (str3 == "(")
                allMemberFunctions.push_back(tok2);
            else if (str3 != "::" && tok2->strAt(-1) != "::") // Support only one depth
                allMemberVars.push_back(tok2);
        }
    }

    // class members..
    std::map<std::string, std::map<std::string, unsigned int> > varsByClass;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "namespace|class|struct %name% {|:|::"))
            continue;

        std::string classname(tok->next()->str());
        const Token* tokStart = tok->tokAt(2);
        unsigned int nestedCount = 1;
        while (Token::Match(tokStart, ":: %name%")) {
            classname += " :: " + tokStart->strAt(1);
            tokStart = tokStart->tokAt(2);
            nestedCount++;
        }

        std::map<std::string, unsigned int>& thisClassVars = varsByClass[classname];
        while (tokStart && tokStart->str() != "{") {
            if (Token::Match(tokStart, "public|private|protected %name%"))
                tokStart = tokStart->next();
            if (tokStart->strAt(1) == "," || tokStart->strAt(1) == "{") {
                const std::map<std::string, unsigned int>& baseClassVars = varsByClass[tokStart->str()];
                thisClassVars.insert(baseClassVars.begin(), baseClassVars.end());
            }
            tokStart = tokStart->next();
        }
        // What member variables are there in this class?
        if (tokStart) {
            for (Token *tok2 = tokStart->next(); tok2 && tok2 != tokStart->link(); tok2 = tok2->next()) {
                // skip parentheses..
                if (tok2->link()) {
                    if (tok2->str() == "{") {
                        if (tok2->strAt(-1) == ")" || tok2->strAt(-2) == ")")
                            setVarIdClassFunction(classname, tok2, tok2->link(), thisClassVars, structMembers, &_varId);
                        tok2 = tok2->link();
                    } else if (tok2->str() == "(" && tok2->link()->strAt(1) != "(")
                        tok2 = tok2->link();
                }

                // Found a member variable..
                else if (tok2->varId() > 0)
                    thisClassVars[tok2->str()] = tok2->varId();
            }
        }

        // Are there any member variables in this class?
        if (thisClassVars.empty())
            continue;

        // Member variables
        for (std::list<Token *>::iterator func = allMemberVars.begin(); func != allMemberVars.end(); ++func) {
            if (!Token::simpleMatch(*func, classname.c_str()))
                continue;

            Token *tok2 = *func;
            tok2 = tok2->tokAt(2);
            tok2->varId(thisClassVars[tok2->str()]);
        }

        if (isC() || tok->str() == "namespace")
            continue;

        // Set variable ids in member functions for this class..
        for (std::list<Token *>::iterator func = allMemberFunctions.begin(); func != allMemberFunctions.end(); ++func) {
            Token *tok2 = *func;

            if (!Token::Match(tok2, classname.c_str()))
                continue;

            if (Token::Match(tok2, "%name% <"))
                tok2 = tok2->next()->findClosingBracket();

            // Found a class function..
            if (!Token::Match(tok2, "%any% :: ~| %name%"))
                continue;

            // Goto the end parentheses..
            tok2 = tok2->tokAt(nestedCount*2);
            if (tok2->str() == "~")
                tok2 = tok2->linkAt(2);
            else
                tok2 = tok2->linkAt(1);

            // If this is a function implementation.. add it to funclist
            Token * start = const_cast<Token *>(isFunctionHead(tok2, "{"));
            if (start) {
                setVarIdClassFunction(classname, start, start->link(), thisClassVars, structMembers, &_varId);
            }

            if (Token::Match(tok2, ") %name% ("))
                tok2 = tok2->linkAt(2);

            // constructor with initializer list
            if (!Token::Match(tok2, ") : ::| %name%"))
                continue;

            Token *tok3 = tok2;
            while (Token::Match(tok3, "[)}] [,:]")) {
                tok3 = tok3->tokAt(2);
                if (Token::Match(tok3, ":: %name%"))
                    tok3 = tok3->next();
                while (Token::Match(tok3, "%name% :: %name%"))
                    tok3 = tok3->tokAt(2);
                if (!Token::Match(tok3, "%name% (|{|<"))
                    break;

                // set varid
                std::map<std::string, unsigned int>::const_iterator varpos = thisClassVars.find(tok3->str());
                if (varpos != thisClassVars.end())
                    tok3->varId(varpos->second);

                // goto end of var
                if (tok3->strAt(1) == "<") {
                    tok3 = tok3->next()->findClosingBracket();
                    if (tok3 && tok3->next() && tok3->next()->link())
                        tok3 = tok3->next()->link();
                } else
                    tok3 = tok3->linkAt(1);
            }
            if (Token::Match(tok3, ")|} {")) {
                setVarIdClassFunction(classname, tok2, tok3->next()->link(), thisClassVars, structMembers, &_varId);
            }
        }
    }
}

static void linkBrackets(const Tokenizer * const tokenizer, std::stack<const Token*>& type, std::stack<Token*>& links, Token * const token, const char open, const char close)
{
    if (token->str()[0] == open) {
        links.push(token);
        type.push(token);
    } else if (token->str()[0] == close) {
        if (links.empty()) {
            // Error, { and } don't match.
            tokenizer->syntaxError(token, open);
        }
        if (type.top()->str()[0] != open) {
            tokenizer->syntaxError(type.top(), type.top()->str()[0]);
        }
        type.pop();

        Token::createMutualLinks(links.top(), token);
        links.pop();
    }
}

void Tokenizer::createLinks()
{
    std::stack<const Token*> type;
    std::stack<Token*> links1;
    std::stack<Token*> links2;
    std::stack<Token*> links3;
    for (Token *token = list.front(); token; token = token->next()) {
        if (token->link()) {
            token->link(0);
        }

        linkBrackets(this, type, links1, token, '{', '}');

        linkBrackets(this, type, links2, token, '(', ')');

        linkBrackets(this, type, links3, token, '[', ']');
    }

    if (!links1.empty()) {
        // Error, { and } don't match.
        syntaxError(links1.top(), '{');
    }

    if (!links2.empty()) {
        // Error, ( and ) don't match.
        syntaxError(links2.top(), '(');
    }

    if (!links3.empty()) {
        // Error, [ and ] don't match.
        syntaxError(links3.top(), '[');
    }
}

void Tokenizer::createLinks2()
{
    if (isC())
        return;

    const Token * templateToken = nullptr;
    bool isStruct = false;

    std::stack<Token*> type;
    for (Token *token = list.front(); token; token = token->next()) {
        if (Token::Match(token, "struct|class %name% [:<]"))
            isStruct = true;
        else if (Token::Match(token, "[;{}]"))
            isStruct = false;

        if (token->link()) {
            if (Token::Match(token, "{|[|("))
                type.push(token);
            else if (!type.empty() && Token::Match(token, "}|]|)")) {
                while (type.top()->str() == "<")
                    type.pop();
                type.pop();
                templateToken = nullptr;
            } else
                token->link(0);
        } else if (!templateToken && !isStruct && Token::Match(token, "%oror%|&&|;")) {
            if (Token::Match(token, "&& [,>]"))
                continue;
            while (!type.empty() && type.top()->str() == "<")
                type.pop();
        } else if (token->str() == "<" && token->previous() && token->previous()->isName() && !token->previous()->varId()) {
            type.push(token);
            if (!templateToken && (token->previous()->str() == "template"))
                templateToken = token;
        } else if (token->str() == ">") {
            if (type.empty() || type.top()->str() != "<") // < and > don't match.
                continue;
            if (token->next() && !Token::Match(token->next(), "%name%|>|&|*|::|,|(|)|{|}|;|[|:"))
                continue;

            // if > is followed by [ .. "new a<b>[" is expected
            if (token->strAt(1) == "[") {
                Token *prev = type.top()->previous();
                while (prev && Token::Match(prev->previous(), ":: %name%"))
                    prev = prev->tokAt(-2);
                if (prev && prev->str() != "new")
                    prev = prev->previous();
                if (!prev || prev->str() != "new")
                    continue;
            }

            Token* top = type.top();
            type.pop();

            if (top == templateToken)
                templateToken = nullptr;

            Token::createMutualLinks(top, token);
        }
    }
}

void Tokenizer::sizeofAddParentheses()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token* next = tok->next();
        if (Token::Match(tok, "sizeof !!(") && next && (next->isLiteral() || next->isName() || Token::Match(next, "[*~!]"))) {
            Token *endToken = next;
            while (Token::Match(endToken->next(), "%name%|%num%|%str%|[|(|.|::|++|--|!|~") || (Token::Match(endToken, "%type% * %op%|?|:|const|;|,"))) {
                if (endToken->strAt(1) == "[" || endToken->strAt(1) == "(")
                    endToken = endToken->linkAt(1);
                else
                    endToken = endToken->next();
            }

            // Add ( after sizeof and ) behind endToken
            tok->insertToken("(");
            endToken->insertToken(")");
            Token::createMutualLinks(tok->next(), endToken->next());
        }
    }
}

bool Tokenizer::simplifySizeof()
{
    // Locate variable declarations and calculate the size
    std::map<unsigned int, unsigned int> sizeOfVar;
    std::map<unsigned int, const Token *> declTokOfVar;
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->varId() != 0 && sizeOfVar.find(tok->varId()) == sizeOfVar.end()) {
            const unsigned int varId = tok->varId();
            if (Token::Match(tok->tokAt(-3), "[;{}(,] %type% * %name% [;,)]") ||
                Token::Match(tok->tokAt(-4), "[;{}(,] const %type% * %name% [;),]") ||
                Token::Match(tok->tokAt(-2), "[;{}(,] %type% %name% [;),]") ||
                Token::Match(tok->tokAt(-3), "[;{}(,] const %type% %name% [;),]")) {
                const unsigned int size = sizeOfType(tok->previous());
                if (size == 0) {
                    continue;
                }

                sizeOfVar[varId] = size;
                declTokOfVar[varId] = tok;
            }

            else if (Token::Match(tok->previous(), "%type% %name% [ %num% ] [[;=]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %name% [ %num% ] [[;=]")) {
                unsigned int size = sizeOfType(tok->previous());
                if (size == 0)
                    continue;

                const Token* tok2 = tok->next();
                do {
                    const MathLib::bigint num = MathLib::toLongNumber(tok2->strAt(1));
                    if (num<0)
                        break;
                    size *= (unsigned)num;
                    tok2 = tok2->tokAt(3);
                } while (Token::Match(tok2, "[ %num% ]"));
                if (Token::Match(tok2, "[;=]")) {
                    sizeOfVar[varId] = size;
                    declTokOfVar[varId] = tok;
                }
                if (!tok2) {
                    syntaxError(tok);
                }
                tok = tok2;
            }

            else if (Token::Match(tok->previous(), "%type% %name% [ %num% ] [,)]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %name% [ %num% ] [,)]")) {
                Token tempTok(0);
                tempTok.str("*");
                sizeOfVar[varId] = sizeOfType(&tempTok);
                declTokOfVar[varId] = tok;
            }
        }
    }

    bool ret = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "sizeof")
            continue;

        if (Token::simpleMatch(tok->next(), ". . .")) {
            tok->deleteNext(3);
        }

        // sizeof('x')
        if (Token::Match(tok->next(), "( %char% )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream sz;
            sz << ((isC()) ? _settings->sizeof_int : 1);
            tok->str(sz.str());
            ret = true;
            continue;
        }

        // sizeof ("text")
        if (Token::Match(tok->next(), "( %str% )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream ostr;
            ostr << (Token::getStrLength(tok) + 1);
            tok->str(ostr.str());
            ret = true;
            continue;
        }

        // sizeof(type *) => sizeof(*)
        if (Token::Match(tok->next(), "( %type% * )")) {
            tok->next()->deleteNext();
        }

        if (Token::simpleMatch(tok->next(), "( * )")) {
            tok->str(MathLib::toString(sizeOfType(tok->tokAt(2))));
            tok->deleteNext(3);
            ret = true;
        }

        // sizeof( a )
        else if (Token::Match(tok->next(), "( %var% )")) {
            std::map<unsigned int, unsigned int>::const_iterator sizeOfVarPos = sizeOfVar.find(tok->tokAt(2)->varId());
            if (sizeOfVarPos != sizeOfVar.end()) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
                tok->str(MathLib::toString(sizeOfVarPos->second));
                ret = true;
            } else {
                // don't try to replace size of variable if variable has
                // similar name with type (#329)
            }
        }

        else if (Token::Match(tok->next(), "( %type% )")) {
            const unsigned int size = sizeOfType(tok->tokAt(2));
            if (size > 0) {
                tok->str(MathLib::toString(size));
                tok->deleteNext(3);
                ret = true;
            }
        }

        else if (Token::simpleMatch(tok->next(), "( *") || Token::Match(tok->next(), "( %name% [")) {
            unsigned int derefs = 0;

            const Token* nametok = tok->tokAt(2);
            if (nametok->str() == "*") {
                do {
                    nametok = nametok->next();
                    derefs++;
                } while (nametok && nametok->str() == "*");

                if (!Token::Match(nametok, "%name% )"))
                    continue;
            } else {
                const Token* tok2 = nametok->next();
                do {
                    tok2 = tok2->link()->next();
                    derefs++;
                } while (tok2 && tok2->str() == "[");

                if (!tok2 || tok2->str() != ")")
                    continue;
            }

            // Some default value
            MathLib::biguint size = 0;

            const unsigned int varid = nametok->varId();
            if (derefs != 0 && varid != 0 && declTokOfVar.find(varid) != declTokOfVar.end()) {
                // Try to locate variable declaration..
                const Token *decltok = declTokOfVar[varid];
                if (Token::Match(decltok->previous(), "%type%|* %name% [")) {
                    size = sizeOfType(decltok->previous());
                } else if (Token::Match(decltok->tokAt(-2), "%type% * %name%")) {
                    size = sizeOfType(decltok->tokAt(-2));
                }
                // Multi-dimensional array..
                if (Token::Match(decltok, "%name% [") && Token::simpleMatch(decltok->linkAt(1), "] [")) {
                    const Token *tok2 = decltok;
                    for (unsigned int i = 0; i < derefs; i++)
                        tok2 = tok2->linkAt(1); // Skip all dimensions that are derefenced before the sizeof call
                    while (Token::Match(tok2, "] [ %num% ]")) {
                        size *= MathLib::toULongNumber(tok2->strAt(2));
                        tok2 = tok2->linkAt(1);
                    }
                    if (Token::simpleMatch(tok2, "] ["))
                        continue;
                }
            } else if (nametok->strAt(1) == "[" && nametok->isStandardType()) {
                size = sizeOfType(nametok);
                if (size == 0)
                    continue;
                const Token *tok2 = nametok->next();
                while (Token::Match(tok2, "[ %num% ]")) {
                    size *= MathLib::toULongNumber(tok2->strAt(1));
                    tok2 = tok2->link()->next();
                }
                if (!tok2 || tok2->str() != ")")
                    continue;
            }

            if (size > 0) {
                tok->str(MathLib::toString(size));
                Token::eraseTokens(tok, tok->next()->link()->next());
                ret = true;
            }
        }
    }
    return ret;
}

bool Tokenizer::simplifyTokenList1(const char FileName[])
{
    if (_settings->terminated())
        return false;

    // if MACRO
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if|for|while|BOOST_FOREACH %name% (")) {
            if (Token::simpleMatch(tok, "for each")) {
                // 'for each ( )' -> 'asm ( )'
                tok->str("asm");
                tok->deleteNext();
            } else {
                syntaxError(tok);
            }
        }
    }

    // remove MACRO in variable declaration: MACRO int x;
    removeMacroInVarDecl();

    // Combine strings and character literals, e.g. L"string", L'c', "string1" "string2"
    combineStringAndCharLiterals();

    // replace inline SQL with "asm()" (Oracle PRO*C). Ticket: #1959
    simplifySQL();

    createLinks();

    // Bail out if code is garbage
    if (const Token *garbage = findGarbageCode())
        syntaxError(garbage);

    // if (x) MACRO() ..
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "if (")) {
            tok = tok->next()->link();
            if (Token::Match(tok, ") %name% (") &&
                tok->next()->isUpperCaseName() &&
                Token::Match(tok->linkAt(2), ") {|else")) {
                syntaxError(tok->next());
            }
        }
    }

    if (_settings->terminated())
        return false;

    // Remove [[deprecated]]
    simplifyDeprecated();

    // remove __attribute__((?))
    simplifyAttribute();

    // Simplify the C alternative tokens (and, or, etc.)
    simplifyCAlternativeTokens();

    // replace 'sin(0)' to '0' and other similar math expressions
    simplifyMathExpressions();

    // combine "- %num%"
    concatenateNegativeNumberAndAnyPositive();

    // Combine tokens..
    combineOperators();

    // remove extern "C" and extern "C" {}
    if (isCPP())
        simplifyExternC();

    // simplify weird but legal code: "[;{}] ( { code; } ) ;"->"[;{}] code;"
    simplifyRoundCurlyParentheses();

    // check for simple syntax errors..
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "> struct {") &&
            Token::simpleMatch(tok->linkAt(2), "} ;")) {
            syntaxError(tok);
        }
    }

    if (!simplifyAddBraces())
        return false;

    sizeofAddParentheses();

    // Simplify: 0[foo] -> *(foo)
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "0 [") && tok->linkAt(1)) {
            tok->str("*");
            tok->next()->str("(");
            tok->linkAt(1)->str(")");
        }
    }

    if (_settings->terminated())
        return false;

    // Remove "volatile", "inline", "register", and "restrict"
    simplifyKeyword();

    // simplify simple calculations inside <..>
    if (isCPP()) {
        Token *lt = nullptr;
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "[;{}]"))
                lt = nullptr;
            else if (Token::Match(tok, "%type% <"))
                lt = tok->next();
            else if (lt && Token::Match(tok, ">|>> %name%|::|(")) {
                const Token * const end = tok;
                for (tok = lt; tok != end; tok = tok->next()) {
                    if (tok->isNumber())
                        TemplateSimplifier::simplifyNumericCalculations(tok->previous());
                }
                lt = tok->next();
            }
        }
    }

    // Convert K&R function declarations to modern C
    simplifyVarDecl(true);
    simplifyFunctionParameters();

    // simplify case ranges (gcc extension)
    simplifyCaseRange();

    // simplify labels and 'case|default'-like syntaxes
    simplifyLabelsCaseDefault();

    // simplify '[;{}] * & ( %any% ) =' to '%any% ='
    simplifyMulAndParens();

    if (!isC() && !_settings->library.markupFile(FileName)) {
        findComplicatedSyntaxErrorsInTemplates();
    }

    if (_settings->terminated())
        return false;

    // remove calling conventions __cdecl, __stdcall..
    simplifyCallingConvention();

    // Remove __declspec()
    simplifyDeclspec();
    validate();
    // remove some unhandled macros in global scope
    removeMacrosInGlobalScope();

    // remove undefined macro in class definition:
    // class DLLEXPORT Fred { };
    // class Fred FINAL : Base { };
    removeMacroInClassDef();

    // That call here fixes #7190
    validate();

    // remove unnecessary member qualification..
    removeUnnecessaryQualification();

    // convert Microsoft memory functions
    simplifyMicrosoftMemoryFunctions();

    // convert Microsoft string functions
    simplifyMicrosoftStringFunctions();

    if (_settings->terminated())
        return false;

    // Remove Qt signals and slots
    simplifyQtSignalsSlots();

    // remove Borland stuff..
    simplifyBorland();

    // syntax error: enum with typedef in it
    checkForEnumsWithTypedef();

    // Remove __asm..
    simplifyAsm();

    // Add parentheses to ternary operator where necessary
    prepareTernaryOpForAST();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    // typedef..
    if (m_timerResults) {
        Timer t("Tokenizer::tokenize::simplifyTypedef", _settings->showtime, m_timerResults);
        simplifyTypedef();
    } else {
        simplifyTypedef();
    }

    // Add parentheses to ternary operator where necessary
    // TODO: this is only necessary if one typedef simplification had a comma and was used within ?:
    // If typedef handling is refactored and moved to symboldatabase someday we can remove this
    prepareTernaryOpForAST();

    for (Token* tok = list.front(); tok;) {
        if (Token::Match(tok, "union|struct|class union|struct|class"))
            tok->deleteNext();
        else
            tok = tok->next();
    }

    // class x y {
    if (isCPP() && _settings->isEnabled("information")) {
        for (const Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "class %type% %type% [:{]")) {
                unhandled_macro_class_x_y(tok);
            }
        }
    }

    // catch bad typedef canonicalization
    //
    // to reproduce bad typedef, download upx-ucl from:
    // http://packages.debian.org/sid/upx-ucl
    // analyse the file src/stub/src/i386-linux.elf.interp-main.c
    validate();

    // The simplify enum have inner loops
    if (_settings->terminated())
        return false;

    // Put ^{} statements in asm()
    simplifyAsm2();

    // Order keywords "static" and "const"
    simplifyStaticConst();

    // convert platform dependent types to standard types
    // 32 bits: size_t -> unsigned long
    // 64 bits: size_t -> unsigned long long
    simplifyPlatformTypes();

    // collapse compound standard types into a single token
    // unsigned long long int => long (with _isUnsigned=true,_isLong=true)
    simplifyStdType();

    if (_settings->terminated())
        return false;

    // simplify bit fields..
    simplifyBitfields();

    if (_settings->terminated())
        return false;

    // struct simplification "struct S {} s; => struct S { } ; S s ;
    simplifyStructDecl();

    if (_settings->terminated())
        return false;

    // x = ({ 123; });  =>   { x = 123; }
    simplifyAssignmentBlock();

    if (_settings->terminated())
        return false;

    simplifyVariableMultipleAssign();

    // Collapse operator name tokens into single token
    // operator = => operator=
    simplifyOperatorName();

    // Remove redundant parentheses
    simplifyRedundantParentheses();

    if (!isC()) {
        // TODO: Only simplify template parameters
        for (Token *tok = list.front(); tok; tok = tok->next())
            while (TemplateSimplifier::simplifyNumericCalculations(tok))
                ;

        // Handle templates..
        simplifyTemplates();

        // The simplifyTemplates have inner loops
        if (_settings->terminated())
            return false;

        // sometimes the "simplifyTemplates" fail and then unsimplified
        // function calls etc remain. These have the "wrong" syntax. So
        // this function will just fix so that the syntax is corrected.
        validate(); // #6847 - invalid code
        TemplateSimplifier::cleanupAfterSimplify(list.front());
    }

    // Simplify pointer to standard types (C only)
    simplifyPointerToStandardType();

    // simplify function pointers
    simplifyFunctionPointers();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    validate(); // #6772 "segmentation fault (invalid code) in Tokenizer::setVarId"

    if (m_timerResults) {
        Timer t("Tokenizer::tokenize::setVarId", _settings->showtime, m_timerResults);
        setVarId();
    } else {
        setVarId();
    }

    // Link < with >
    createLinks2();

    // specify array size
    arraySize();

    // The simplify enum might have inner loops
    if (_settings->terminated())
        return false;

    // Add std:: in front of std classes, when using namespace std; was given
    simplifyNamespaceStd();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Convert e.g. atol("0") into 0
    simplifyMathFunctions();

    simplifyDoublePlusAndDoubleMinus();

    simplifyArrayAccessSyntax();

    Token::assignProgressValues(list.front());

    removeRedundantSemicolons();

    simplifyParameterVoid();

    simplifyRedundantConsecutiveBraces();

    simplifyEmptyNamespaces();

    elseif();

    SimplifyNamelessRValueReferences();


    validate();
    return true;
}

bool Tokenizer::simplifyTokenList2()
{
    // clear the _functionList so it can't contain dead pointers
    deleteSymbolDatabase();

    // Clear AST,ValueFlow. These will be created again at the end of this function.
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        tok->clearAst();
        tok->clearValueFlow();
    }

    // f(x=g())   =>   x=g(); f(x)
    simplifyAssignmentInFunctionCall();

    // ";a+=b;" => ";a=a+b;"
    simplifyCompoundAssignment();

    simplifyCharAt();

    // simplify references
    simplifyReference();

    simplifyStd();

    if (_settings->terminated())
        return false;

    simplifySizeof();

    simplifyUndefinedSizeArray();

    simplifyCasts();

    // Simplify simple calculations before replace constants, this allows the replacement of constants that are calculated
    // e.g. const static int value = sizeof(X)/sizeof(Y);
    simplifyCalculations();

    if (_settings->terminated())
        return false;

    // Replace "*(ptr + num)" => "ptr[num]"
    simplifyOffsetPointerDereference();

    // Replace "&str[num]" => "(str + num)"
    simplifyOffsetPointerReference();

    removeRedundantAssignment();

    simplifyRealloc();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Simplify variable declarations
    simplifyVarDecl(false);

    simplifyErrNoInWhile();
    simplifyIfAndWhileAssign();
    simplifyRedundantParentheses();
    simplifyNestedStrcat();
    simplifyFuncInWhile();

    simplifyIfAndWhileAssign();

    // replace strlen(str)
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "strlen ( %str% )")) {
            tok->str(MathLib::toString(Token::getStrLength(tok->tokAt(2))));
            tok->deleteNext(3);
        }
    }

    bool modified = true;
    while (modified) {
        if (_settings->terminated())
            return false;

        modified = false;
        modified |= simplifyConditions();
        modified |= simplifyFunctionReturn();
        modified |= simplifyKnownVariables();
        modified |= simplifyStrlen();

        modified |= removeRedundantConditions();
        modified |= simplifyRedundantParentheses();
        modified |= simplifyConstTernaryOp();
        modified |= simplifyCalculations();
        validate();
    }

    // simplify redundant loops
    simplifyWhile0();
    removeRedundantFor();

    // Remove redundant parentheses in return..
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "return (")) {
            Token *tok2 = tok->next()->link();
            if (Token::simpleMatch(tok2, ") ;")) {
                tok->deleteNext();
                tok2->deleteThis();
            } else {
                break;
            }
        }
    }

    simplifyReturnStrncat();

    removeRedundantAssignment();

    simplifyComma();

    removeRedundantSemicolons();

    simplifyFlowControl();

    simplifyRedundantConsecutiveBraces();

    simplifyEmptyNamespaces();

    simplifyMathFunctions();

    validate();

    Token::assignProgressValues(list.front());

    list.createAst();
    // skipping this here may help improve performance. Might be enabled later on demand. #7208
    // list.validateAst();

    // Create symbol database and then remove const keywords
    createSymbolDatabase();
    _symbolDatabase->setValueTypeInTokenList();

    ValueFlow::setValues(&list, _symbolDatabase, _errorLogger, _settings);

    if (_settings->terminated())
        return false;

    printDebugOutput(2);

    return true;
}
//---------------------------------------------------------------------------

void Tokenizer::printDebugOutput(unsigned int simplification) const
{
    const bool debug = (simplification != 1U && _settings->debug) ||
                       (simplification != 2U && _settings->debugnormal);

    if (debug && list.front()) {
        list.front()->printOut(0, list.getFiles());

        if (_settings->xml)
            std::cout << "<debug>" << std::endl;

        if (_symbolDatabase) {
            if (_settings->xml)
                _symbolDatabase->printXml(std::cout);
            else if (_settings->verbose) {
                _symbolDatabase->printOut("Symbol database");
            }
        }

        if (_settings->verbose)
            list.front()->printAst(_settings->verbose, _settings->xml, std::cout);

        list.front()->printValueFlow(_settings->xml, std::cout);

        if (_settings->xml)
            std::cout << "</debug>" << std::endl;
    }

    if (simplification == 2U && _settings->debugwarnings) {
        printUnknownTypes();

        // the typeStartToken() should come before typeEndToken()
        for (unsigned int varid = 1; varid < _symbolDatabase->getVariableListSize(); varid++) {
            const Variable *var = _symbolDatabase->getVariableFromVarId(varid);
            if (!var)
                continue;

            const Token * typetok = var->typeStartToken();
            while (typetok && typetok != var->typeEndToken())
                typetok = typetok->next();

            if (typetok != var->typeEndToken()) {
                reportError(var->typeStartToken(),
                            Severity::debug,
                            "debug",
                            "Variable::typeStartToken() of variable '" + var->name() + "' is not located before Variable::typeEndToken(). The location of the typeStartToken() is '" + var->typeStartToken()->str() + "' at line " + MathLib::toString(var->typeStartToken()->linenr()));
            }
        }
    }
}

void Tokenizer::dump(std::ostream &out) const
{
    // Create a xml data dump.
    // The idea is not that this will be readable for humans. It's a
    // data dump that 3rd party tools could load and get useful info from.

    // tokens..
    out << "  <tokenlist>" << std::endl;
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        out << "    <token id=\"" << tok << "\" file=\"" << ErrorLogger::toxml(list.file(tok)) << "\" linenr=\"" << tok->linenr() << '\"';
        out << " str=\"" << ErrorLogger::toxml(tok->str()) << '\"';
        out << " scope=\"" << tok->scope() << '\"';
        if (tok->isName()) {
            out << " type=\"name\"";
            if (tok->isUnsigned())
                out << " isUnsigned=\"true\"";
            else if (tok->isSigned())
                out << " isSigned=\"true\"";
        } else if (tok->isNumber()) {
            out << " type=\"number\"";
            if (MathLib::isInt(tok->str()))
                out << " isInt=\"True\"";
            if (MathLib::isFloat(tok->str()))
                out << " isFloat=\"True\"";
        } else if (tok->tokType() == Token::eString)
            out << " type=\"string\" strlen=\"" << Token::getStrLength(tok) << '\"';
        else if (tok->tokType() == Token::eChar)
            out << " type=\"char\"";
        else if (tok->isBoolean())
            out << " type=\"boolean\"";
        else if (tok->isOp()) {
            out << " type=\"op\"";
            if (tok->isArithmeticalOp())
                out << " isArithmeticalOp=\"True\"";
            else if (tok->isAssignmentOp())
                out << " isAssignmentOp=\"True\"";
            else if (tok->isComparisonOp())
                out << " isComparisonOp=\"True\"";
            else if (tok->tokType() == Token::eLogicalOp)
                out << " isLogicalOp=\"True\"";
        }
        if (tok->link())
            out << " link=\"" << tok->link() << '\"';
        if (tok->varId() > 0U)
            out << " varId=\"" << MathLib::toString(tok->varId()) << '\"';
        if (tok->variable())
            out << " variable=\"" << tok->variable() << '\"';
        if (tok->function())
            out << " function=\"" << tok->function() << '\"';
        if (!tok->values().empty())
            out << " values=\"" << &tok->values() << '\"';
        if (tok->type())
            out << " type-scope=\"" << tok->type()->classScope << '\"';
        if (tok->astParent())
            out << " astParent=\"" << tok->astParent() << '\"';
        if (tok->astOperand1())
            out << " astOperand1=\"" << tok->astOperand1() << '\"';
        if (tok->astOperand2())
            out << " astOperand2=\"" << tok->astOperand2() << '\"';
        out << "/>" << std::endl;
    }
    out << "  </tokenlist>" << std::endl;

    _symbolDatabase->printXml(out);
    if (list.front())
        list.front()->printValueFlow(true, out);
}

void Tokenizer::removeMacrosInGlobalScope()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::Match(tok, ") %type% {") &&
                !Token::Match(tok->next(), "const|namespace|class|struct|union|noexcept"))
                tok->deleteNext();
        }

        if ((!tok->previous() || Token::Match(tok->previous(), "[;{}]")) &&
            Token::Match(tok, "%type%") && tok->isUpperCaseName()) {
            const Token *tok2 = tok->next();
            if (tok2 && tok2->str() == "(")
                tok2 = tok2->link()->next();

            // remove unknown macros before namespace|class|struct|union
            if (Token::Match(tok2, "namespace|class|struct|union")) {
                // is there a "{" for?
                const Token *tok3 = tok2;
                while (tok3 && !Token::Match(tok3,"[;{}()]"))
                    tok3 = tok3->next();
                if (tok3 && tok3->str() == "{") {
                    Token::eraseTokens(tok, tok2);
                    tok->deleteThis();
                }
                continue;
            }

            // replace unknown macros before foo(
            if (Token::Match(tok2, "%type% (") && isFunctionHead(tok2->next(), "{")) {
                std::string typeName;
                for (const Token* tok3 = tok; tok3 != tok2; tok3 = tok3->next())
                    typeName += tok3->str();
                Token::eraseTokens(tok, tok2);
                tok->str(typeName);
            }

            // remove unknown macros before foo::foo(
            if (Token::Match(tok2, "%type% :: %type%")) {
                const Token *tok3 = tok2;
                while (Token::Match(tok3, "%type% :: %type% ::"))
                    tok3 = tok3->tokAt(2);
                if (Token::Match(tok3, "%type% :: %type% (") && tok3->str() == tok3->strAt(2)) {
                    Token::eraseTokens(tok, tok2);
                    tok->deleteThis();
                }
                continue;
            }
        }

        // Skip executable scopes
        if (tok->str() == "{") {
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (prev && prev->str() == ")")
                tok = tok->link();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::removeMacroInClassDef()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct %name% %name% {|:") &&
            (tok->next()->isUpperCaseName() || tok->tokAt(2)->isUpperCaseName())) {
            if (tok->next()->isUpperCaseName() && !tok->tokAt(2)->isUpperCaseName())
                tok->deleteNext();
            else if (!tok->next()->isUpperCaseName() && tok->tokAt(2)->isUpperCaseName())
                tok->next()->deleteNext();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::removeMacroInVarDecl()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %name% (") && tok->next() && tok->next()->isUpperCaseName()) {
            // goto ')' parentheses
            const Token *tok2 = tok;
            int parlevel = 0;
            while (tok2) {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")") {
                    if (--parlevel <= 0)
                        break;
                }
                tok2 = tok2->next();
            }
            tok2 = tok2 ? tok2->next() : nullptr;

            // check if this is a variable declaration..
            const Token *tok3 = tok2;
            while (tok3 && tok3->isUpperCaseName())
                tok3 = tok3->next();
            if (tok3 && (tok3->isStandardType() || Token::Match(tok3,"const|static|struct|union|class")))
                Token::eraseTokens(tok,tok2);
        }
    }
}
//---------------------------------------------------------------------------

void Tokenizer::removeRedundantAssignment()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            tok = tok->link();

        Token * start = const_cast<Token *>(startOfExecutableScope(tok));
        if (start) {
            tok = start->previous();
            // parse in this function..
            std::set<unsigned int> localvars;
            const Token * const end = tok->next()->link();
            for (Token *tok2 = tok->next(); tok2 && tok2 != end; tok2 = tok2->next()) {
                // skip local class or struct
                if (Token::Match(tok2, "class|struct %type% {|:")) {
                    // skip to '{'
                    tok2 = tok2->tokAt(2);
                    while (tok2 && tok2->str() != "{")
                        tok2 = tok2->next();

                    if (tok2)
                        tok2 = tok2->link(); // skip local class or struct
                    else
                        return;
                } else if (Token::Match(tok2, "[;{}] %type% * %name% ;") && tok2->next()->str() != "return") {
                    tok2 = tok2->tokAt(3);
                    localvars.insert(tok2->varId());
                } else if (Token::Match(tok2, "[;{}] %type% %name% ;") && tok2->next()->isStandardType()) {
                    tok2 = tok2->tokAt(2);
                    localvars.insert(tok2->varId());
                } else if (tok2->varId() &&
                           !Token::Match(tok2->previous(), "[;{}] %name% = %char%|%num%|%name% ;")) {
                    localvars.erase(tok2->varId());
                }
            }
            localvars.erase(0);
            if (!localvars.empty()) {
                for (Token *tok2 = tok->next(); tok2 && tok2 != end;) {
                    if (Token::Match(tok2, "[;{}] %type% %name% ;") && localvars.find(tok2->tokAt(2)->varId()) != localvars.end()) {
                        tok2->deleteNext(3);
                    } else if ((Token::Match(tok2, "[;{}] %type% * %name% ;") &&
                                localvars.find(tok2->tokAt(3)->varId()) != localvars.end()) ||
                               (Token::Match(tok2, "[;{}] %name% = %any% ;") &&
                                localvars.find(tok2->next()->varId()) != localvars.end())) {
                        tok2->deleteNext(4);
                    } else
                        tok2 = tok2->next();
                }
            }
        }
    }
}

void Tokenizer::simplifyRealloc()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "(|[") ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "="))
            tok = tok->link();
        else if (Token::Match(tok, "[;{}] %name% = realloc (")) {
            tok = tok->tokAt(3);
            if (Token::simpleMatch(tok->next(), "( 0 ,")) {
                //no "x = realloc(0,);"
                if (!Token::simpleMatch(tok->next()->link(), ") ;") || tok->next()->link()->previous() == tok->tokAt(3))
                    continue;

                // delete "0 ,"
                tok->next()->deleteNext(2);

                // Change function name "realloc" to "malloc"
                tok->str("malloc");
                tok = tok->next()->link();
            } else {
                Token *tok2 = tok->next()->link()->tokAt(-2);
                //no "x = realloc(,0);"
                if (!Token::simpleMatch(tok2, ", 0 ) ;") || tok2 == tok->tokAt(2))
                    continue;

                //remove ", 0"
                tok2 = tok2->previous();
                tok2->deleteNext(2);
                //change "realloc" to "free"
                tok->str("free");
                //insert "0" after "var ="
                tok = tok->previous();
                tok->insertToken("0");
                //move "var = 0" between "free(...)" and ";"
                tok2 = tok2->next();
                Token::move(tok->previous(), tok->next(), tok2);
                //add missing ";" after "free(...)"
                tok2->insertToken(";");
                //goto before last ";" and continue
                tok = tok->next();
            }
        }
    }
}

void Tokenizer::simplifyEmptyNamespaces()
{
    if (isC())
        return;

    bool goback = false;
    for (Token *tok = list.front(); tok; tok = tok ? tok->next() : nullptr) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        if (Token::Match(tok, "(|[|{")) {
            tok = tok->link();
            continue;
        }
        if (!Token::Match(tok, "namespace %name% {"))
            continue;
        if (tok->strAt(3) == "}") {
            tok->deleteNext(3);             // remove '%name% { }'
            if (!tok->previous()) {
                // remove 'namespace' or replace it with ';' if isolated
                tok->deleteThis();
                goback = true;
            } else {                    // '%any% namespace %any%'
                tok = tok->previous();  // goto previous token
                tok->deleteNext();      // remove next token: 'namespace'
            }
        } else {
            tok = tok->tokAt(2);
        }
    }
}

void Tokenizer::simplifyFlowControl()
{
    for (Token *begin = list.front(); begin; begin = begin->next()) {

        if (Token::Match(begin, "(|[") ||
            (begin->str() == "{" && begin->previous() && begin->strAt(-1) == "="))
            begin = begin->link();

        //function scope
        if (!Token::simpleMatch(begin, ") {") && !Token::Match(begin, ") %name% {"))
            continue;

        Token* end = begin->linkAt(1+(begin->next()->str() == "{" ? 0 : 1));
        unsigned int indentlevel = 0;
        bool stilldead = false;

        for (Token *tok = begin; tok && tok != end; tok = tok->next()) {
            if (Token::Match(tok, "(|[")) {
                tok = tok->link();
                continue;
            }

            if (tok->str() == "{") {
                if (tok->previous() && tok->previous()->str() == "=") {
                    tok = tok->link();
                    continue;
                }
                ++indentlevel;
            } else if (tok->str() == "}") {
                if (!indentlevel)
                    break;
                --indentlevel;
                if (stilldead) {
                    eraseDeadCode(tok, 0);
                    if (indentlevel == 1 || tok->next()->str() != "}" || !Token::Match(tok->next()->link()->previous(), ";|{|}|do {"))
                        stilldead = false;
                    continue;
                }
            }

            if (!indentlevel)
                continue;

            if (Token::Match(tok,"continue|break ;")) {
                tok = tok->next();
                eraseDeadCode(tok, 0);

            } else if (Token::Match(tok,"return|goto") ||
                       (Token::Match(tok->previous(), "[;{}] %name% (") &&
                        _settings->library.isnoreturn(tok)) ||
                       (isCPP() && tok->str() == "throw")) {
                if (tok->next()->str() == "}")
                    syntaxError(tok->next()); // invalid code like in #6731
                //TODO: ensure that we exclude user-defined 'exit|abort|throw', except for 'noreturn'
                //catch the first ';'
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "(|[")) {
                        tok2 = tok2->link();
                    } else if (tok2->str() == ";") {
                        tok = tok2;
                        eraseDeadCode(tok, 0);
                        break;
                    } else if (Token::Match(tok2, "[{}]"))
                        break;
                }
                //if everything is removed, then remove also the code after an inferior scope
                //only if the actual scope is not special
                if (indentlevel > 1 && tok->next()->str() == "}" && Token::Match(tok->next()->link()->previous(), ";|{|}|do {"))
                    stilldead = true;
            }
        }
        begin = end;
    }
}


bool Tokenizer::removeRedundantConditions()
{
    // Return value for function. Set to true if there are any simplifications
    bool ret = false;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "if ( %bool% ) {"))
            continue;

        // Find matching else
        Token *elseTag = tok->linkAt(4)->next();

        bool boolValue = (tok->strAt(2) == "true");

        // Handle if with else
        if (Token::simpleMatch(elseTag, "else {")) {
            // Handle else
            if (boolValue == false) {
                // Convert "if( false ) {aaa;} else {bbb;}" => "{bbb;}"

                //remove '(false)'
                tok->deleteNext(3);
                //delete dead code inside scope
                eraseDeadCode(tok, elseTag);
                //remove 'else'
                elseTag->deleteThis();
                //remove 'if'
                tok->deleteThis();
            } else {
                // Convert "if( true ) {aaa;} else {bbb;}" => "{aaa;}"
                const Token *end = elseTag->next()->link()->next();

                // Remove "else { bbb; }"
                elseTag = elseTag->previous();
                eraseDeadCode(elseTag, end);

                // Remove "if( true )"
                tok->deleteNext(3);
                tok->deleteThis();
            }

            ret = true;
        }

        // Handle if without else
        else {
            if (boolValue == false) {
                //remove '(false)'
                tok->deleteNext(3);
                //delete dead code inside scope
                eraseDeadCode(tok, elseTag);
                //remove 'if'
                tok->deleteThis();
            } else {
                // convert "if( true ) {aaa;}" => "{aaa;}"
                tok->deleteNext(3);
                tok->deleteThis();
            }

            ret = true;
        }
    }

    return ret;
}

void Tokenizer::removeRedundantFor()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] for ( %name% = %num% ; %name% < %num% ; ++| %name% ++| ) {") ||
            Token::Match(tok, "[;{}] for ( %type% %name% = %num% ; %name% < %num% ; ++| %name% ++| ) {")) {
            // Same variable name..
            const Token* varTok = tok->tokAt(3);
            bool type = varTok->next()->isName();
            if (type)
                varTok = varTok->next();
            const std::string varname(varTok->str());
            const unsigned int varid(varTok->varId());
            if (varname != varTok->strAt(4))
                continue;
            const Token *vartok2 = tok->linkAt(2)->previous();
            if (vartok2->str() == "++")
                vartok2 = vartok2->previous();
            else if (vartok2->strAt(-1) != "++")
                continue;
            if (varname != vartok2->str())
                continue;

            // Check that the difference of the numeric values is 1
            const MathLib::bigint num1(MathLib::toLongNumber(varTok->strAt(2)));
            const MathLib::bigint num2(MathLib::toLongNumber(varTok->strAt(6)));
            if (num1 + 1 != num2)
                continue;

            // check how loop variable is used in loop..
            bool read = false;
            bool write = false;
            const Token* end = tok->linkAt(2)->next()->link();
            for (const Token *tok2 = tok->linkAt(2); tok2 != end; tok2 = tok2->next()) {
                if (tok2->str() == varname) {
                    if (tok2->previous()->isArithmeticalOp() &&
                        tok2->next() &&
                        (tok2->next()->isArithmeticalOp() || tok2->next()->str() == ";")) {
                        read = true;
                    } else {
                        read = write = true;
                        break;
                    }
                }
            }

            // Simplify loop if loop variable isn't written
            if (!write) {
                Token* bodyBegin = tok->linkAt(2)->next();
                // remove "for ("
                tok->deleteNext(2);

                // If loop variable is read then keep assignment before
                // loop body..
                if (type) {
                    tok->insertToken("{");
                    Token::createMutualLinks(tok->next(), bodyBegin->link());
                    bodyBegin->deleteThis();
                    tok = tok->tokAt(6);
                } else if (read) {
                    // goto ";"
                    tok = tok->tokAt(4);
                } else {
                    // remove "x = 0 ;"
                    tok->deleteNext(4);
                }

                // remove "x < 1 ; x ++ )"
                tok->deleteNext(7);

                if (!type) {
                    // Add assignment after the loop body so the loop variable
                    // get the correct end value
                    Token *tok2 = tok->next()->link();
                    tok2->insertToken(";");
                    tok2->insertToken(MathLib::toString(num2));
                    tok2->insertToken("=");
                    tok2->insertToken(varname);
                    tok2->next()->varId(varid);
                }
            }
        }
    }
}


void Tokenizer::removeRedundantSemicolons()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->link() && tok->str() == "(") {
            tok = tok->link();
            continue;
        }
        for (;;) {
            if (Token::simpleMatch(tok, "; ;")) {
                tok->deleteNext();
            } else if (Token::simpleMatch(tok, "; { ; }")) {
                tok->deleteNext(3);
            } else {
                break;
            }
        }
    }
}


bool Tokenizer::simplifyAddBraces()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token const * tokRet=simplifyAddBracesToCommand(tok);
        if (!tokRet)
            return false;
    }
    return true;
}

Token *Tokenizer::simplifyAddBracesToCommand(Token *tok)
{
    Token * tokEnd=tok;
    if (Token::Match(tok,"for|switch|BOOST_FOREACH")) {
        tokEnd=simplifyAddBracesPair(tok,true);
    } else if (tok->str()=="while") {
        Token *tokPossibleDo=tok->previous();
        if (tokPossibleDo &&
            tokPossibleDo->str()=="}")
            tokPossibleDo=tokPossibleDo->link();
        if (!tokPossibleDo ||
            tokPossibleDo->strAt(-1) != "do")
            tokEnd=simplifyAddBracesPair(tok,true);
    } else if (tok->str()=="do") {
        tokEnd=simplifyAddBracesPair(tok,false);
        if (tokEnd!=tok) {
            // walk on to next token, i.e. "while"
            // such that simplifyAddBracesPair does not close other braces
            // before the "while"
            if (tokEnd) {
                tokEnd=tokEnd->next();
                if (!tokEnd || tokEnd->str()!="while") // no while
                    syntaxError(tok);
            }
        }
    } else if (tok->str()=="if") {
        tokEnd=simplifyAddBracesPair(tok,true);
        if (!tokEnd)
            return nullptr;
        if (tokEnd->strAt(1) == "else") {
            Token * tokEndNextNext= tokEnd->tokAt(2);
            if (!tokEndNextNext || tokEndNextNext->str() == "}")
                syntaxError(tokEndNextNext);
            if (tokEndNextNext->str() == "if")
                // do not change "else if ..." to "else { if ... }"
                tokEnd=simplifyAddBracesToCommand(tokEndNextNext);
            else
                tokEnd=simplifyAddBracesPair(tokEnd->next(),false);
        }
    }

    return tokEnd;
}

Token *Tokenizer::simplifyAddBracesPair(Token *tok, bool commandWithCondition)
{
    Token * tokCondition=tok->next();
    if (!tokCondition) // Missing condition
        return tok;

    Token *tokAfterCondition=tokCondition;
    if (commandWithCondition) {
        if (tokCondition->str()=="(")
            tokAfterCondition=tokCondition->link();
        else
            syntaxError(tok); // Bad condition

        if (!tokAfterCondition || tokAfterCondition->strAt(1) == "]")
            syntaxError(tok); // Bad condition

        tokAfterCondition=tokAfterCondition->next();
        if (!tokAfterCondition || Token::Match(tokAfterCondition, ")|}|,")) {
            // No tokens left where to add braces around
            return tok;
        }
    }
    Token * tokBracesEnd=nullptr;
    if (tokAfterCondition->str()=="{") {
        // already surrounded by braces
        tokBracesEnd=tokAfterCondition->link();
    } else if (Token::simpleMatch(tokAfterCondition, "try {") &&
               Token::simpleMatch(tokAfterCondition->linkAt(1), "} catch (")) {
        tokAfterCondition->previous()->insertToken("{");
        Token * tokOpenBrace = tokAfterCondition->previous();
        Token * tokEnd = tokAfterCondition->linkAt(1)->linkAt(2)->linkAt(1);
        if (!tokEnd) {
            syntaxError(tokAfterCondition);
        }
        tokEnd->insertToken("}");
        Token * tokCloseBrace = tokEnd->next();

        Token::createMutualLinks(tokOpenBrace, tokCloseBrace);
        tokBracesEnd = tokCloseBrace;
    } else if (Token::Match(tokAfterCondition, "%name% : {")) {
        tokAfterCondition->previous()->insertToken("{");
        tokAfterCondition->linkAt(2)->insertToken("}");
        tokBracesEnd = tokAfterCondition->linkAt(2)->next();
        Token::createMutualLinks(tokAfterCondition->previous(), tokBracesEnd);
    } else {
        Token * tokEnd = simplifyAddBracesToCommand(tokAfterCondition);
        if (!tokEnd) // Ticket #4887
            return tok;
        if (tokEnd->str()!="}") {
            // Token does not end with brace
            // Look for ; to add own closing brace after it
            while (tokEnd && !Token::Match(tokEnd, ";|)|}")) {
                if (tokEnd->tokType()==Token::eBracket || tokEnd->str() == "(") {
                    tokEnd = tokEnd->link();
                    if (!tokEnd) {
                        // Inner bracket does not close
                        return tok;
                    }
                }
                tokEnd=tokEnd->next();
            }
            if (!tokEnd || tokEnd->str() != ";") {
                // No trailing ;
                return tok;
            }
        }

        tokAfterCondition->previous()->insertToken("{");
        Token * tokOpenBrace=tokAfterCondition->previous();

        tokEnd->insertToken("}");
        Token * TokCloseBrace=tokEnd->next();

        Token::createMutualLinks(tokOpenBrace,TokCloseBrace);
        tokBracesEnd=TokCloseBrace;
    }

    return tokBracesEnd;
}

void Tokenizer::simplifyCompoundAssignment()
{
    // Simplify compound assignments:
    // "a+=b" => "a = a + b"
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "[;{}] (| *| (| %name%"))
            continue;
        // backup current token..
        Token * const tok1 = tok;

        if (tok->next()->str() == "*")
            tok = tok->next();

        if (tok->next() && tok->next()->str() == "(") {
            tok = tok->next()->link()->next();
        } else {
            // variable..
            tok = tok->tokAt(2);
            while (Token::Match(tok, ". %name%") ||
                   Token::Match(tok, "[|(")) {
                if (tok->str() == ".")
                    tok = tok->tokAt(2);
                else {
                    // goto "]" or ")"
                    tok = tok->link();

                    // goto next token..
                    tok = tok ? tok->next() : 0;
                }
            }
        }
        if (!tok)
            break;

        // Is current token at a compound assignment: +=|-=|.. ?
        const std::string &str = tok->str();
        std::string op;  // operator used in assignment
        if (tok->isAssignmentOp() && str.size() == 2)
            op = str.substr(0, 1);
        else if (tok->isAssignmentOp() && str.size() == 3)
            op = str.substr(0, 2);
        else {
            tok = tok1;
            continue;
        }

        // Remove the whole statement if it says: "+=0;", "-=0;", "*=1;" or "/=1;"
        if (Token::Match(tok, "+=|-= 0 ;") ||
            Token::simpleMatch(tok, "|= 0 ;") ||
            Token::Match(tok, "*=|/= 1 ;")) {
            tok = tok1;
            while (tok->next()->str() != ";")
                tok->deleteNext();
        } else {
            // Enclose the rhs in parentheses..
            if (!Token::Match(tok->tokAt(2), "[;)]")) {
                // Only enclose rhs in parentheses if there is some operator
                bool someOperator = false;
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->link() && Token::Match(tok2, "{|[|("))
                        tok2 = tok2->link();

                    if (Token::Match(tok2->next(), "[;)]")) {
                        if (someOperator) {
                            tok->insertToken("(");
                            tok2->insertToken(")");
                            Token::createMutualLinks(tok->next(), tok2->next());
                        }
                        break;
                    }

                    someOperator |= (tok2->isOp() || tok2->str() == "?");
                }
            }

            // simplify the compound assignment..
            tok->str("=");
            tok->insertToken(op);

            std::stack<Token *> tokend;
            for (Token *tok2 = tok->previous(); tok2 && tok2 != tok1; tok2 = tok2->previous()) {
                // Don't duplicate ++ and --. Put preincrement in lhs. Put
                // postincrement in rhs.
                if (tok2->tokType() == Token::eIncDecOp) {
                    // pre increment/decrement => don't copy
                    if (tok2->next()->isName()) {
                        continue;
                    }

                    // post increment/decrement => move from lhs to rhs
                    tok->insertToken(tok2->str());
                    tok2->deleteThis();
                    continue;
                }

                // Copy token from lhs to rhs
                tok->insertToken(tok2->str());
                tok->next()->varId(tok2->varId());
                if (Token::Match(tok->next(), "]|)|}"))
                    tokend.push(tok->next());
                else if (Token::Match(tok->next(), "(|[|{")) {
                    Token::createMutualLinks(tok->next(), tokend.top());
                    tokend.pop();
                }
            }
        }
    }
}

bool Tokenizer::simplifyConditions()
{
    bool ret = false;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "! %bool%|%num%")) {
            tok->deleteThis();
            if (Token::Match(tok, "0|false"))
                tok->str("true");
            else
                tok->str("false");

            ret = true;
        }

        if (Token::simpleMatch(tok, "&& true &&")) {
            tok->deleteNext(2);
            ret = true;
        }

        else if (Token::simpleMatch(tok, "|| false ||")) {
            tok->deleteNext(2);
            ret = true;
        }

        else if (Token::Match(tok, "(|&& true && true &&|)")) {
            tok->deleteNext(2);
            ret = true;
        }

        else if (Token::Match(tok, "%oror%|( false %oror% false %oror%|)")) {
            tok->deleteNext(2);
            ret = true;
        }

        else if (Token::simpleMatch(tok, "( true ||") ||
                 Token::simpleMatch(tok, "( false &&")) {
            Token::eraseTokens(tok->next(), tok->link());
            ret = true;
        }

        else if (Token::simpleMatch(tok, "|| true )") ||
                 Token::simpleMatch(tok, "&& false )")) {
            tok = tok->next();
            Token::eraseTokens(tok->next()->link(), tok);
            ret = true;
        }

        else if (Token::simpleMatch(tok, "&& false &&") ||
                 Token::simpleMatch(tok, "|| true ||")) {
            //goto '('
            Token *tok2 = tok;
            while (tok2->previous()) {
                if (tok2->previous()->str() == ")")
                    tok2 = tok2->previous()->link();
                else {
                    tok2 = tok2->previous();
                    if (tok2->str() == "(")
                        break;
                }
            }
            if (!tok2)
                continue;
            //move tok to 'true|false' position
            tok = tok->next();
            //remove everything before 'true|false'
            Token::eraseTokens(tok2, tok);
            //remove everything after 'true|false'
            Token::eraseTokens(tok, tok2->link());
            ret = true;
        }

        // Change numeric constant in condition to "true" or "false"
        if (Token::Match(tok, "if|while ( %num% )|%oror%|&&")) {
            tok->tokAt(2)->str((tok->strAt(2) != "0") ? "true" : "false");
            ret = true;
        }
        if (Token::Match(tok, "&&|%oror% %num% )|%oror%|&&")) {
            tok->next()->str((tok->next()->str() != "0") ? "true" : "false");
            ret = true;
        }

        // Reduce "(%num% == %num%)" => "(true)"/"(false)"
        if (Token::Match(tok, "&&|%oror%|(") &&
            (Token::Match(tok->next(), "%num% %any% %num%") ||
             Token::Match(tok->next(), "%bool% %any% %bool%")) &&
            Token::Match(tok->tokAt(4), "&&|%oror%|)|?")) {
            std::string cmp = tok->strAt(2);
            bool result = false;
            if (tok->next()->isNumber()) {
                // Compare numbers

                if (cmp == "==" || cmp == "!=") {
                    const std::string& op1(tok->next()->str());
                    const std::string& op2(tok->strAt(3));

                    bool eq = false;
                    if (MathLib::isInt(op1) && MathLib::isInt(op2))
                        eq = (MathLib::toLongNumber(op1) == MathLib::toLongNumber(op2));
                    else
                        eq = (op1 == op2);

                    if (cmp == "==")
                        result = eq;
                    else
                        result = !eq;
                } else {
                    const double op1 = MathLib::toDoubleNumber(tok->next()->str());
                    const double op2 = MathLib::toDoubleNumber(tok->strAt(3));
                    if (cmp == ">=")
                        result = (op1 >= op2);
                    else if (cmp == ">")
                        result = (op1 > op2);
                    else if (cmp == "<=")
                        result = (op1 <= op2);
                    else if (cmp == "<")
                        result = (op1 < op2);
                    else
                        cmp = "";
                }
            } else {
                // Compare boolean
                const bool op1 = (tok->next()->str() == std::string("true"));
                const bool op2 = (tok->strAt(3) == std::string("true"));

                if (cmp == "==")
                    result = (op1 == op2);
                else if (cmp == "!=")
                    result = (op1 != op2);
                else if (cmp == ">=")
                    result = (op1 >= op2);
                else if (cmp == ">")
                    result = (op1 > op2);
                else if (cmp == "<=")
                    result = (op1 <= op2);
                else if (cmp == "<")
                    result = (op1 < op2);
                else
                    cmp = "";
            }

            if (! cmp.empty()) {
                tok = tok->next();
                tok->deleteNext(2);

                tok->str(result ? "true" : "false");
                ret = true;
            }
        }
    }

    return ret;
}

bool Tokenizer::simplifyConstTernaryOp()
{
    bool ret = false;
    const Token *templateParameterEnd = 0; // The end of the current template parameter list, if any
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "<" && TemplateSimplifier::templateParameters(tok))
            templateParameterEnd = tok->findClosingBracket();
        if (tok == templateParameterEnd)
            templateParameterEnd = 0; // End of the current template parameter list
        if (tok->str() != "?")
            continue;

        if (!Token::Match(tok->tokAt(-2), "<|=|,|(|[|{|}|;|case|return %bool%|%num%") &&
            !Token::Match(tok->tokAt(-4), "<|=|,|(|[|{|}|;|case|return ( %bool%|%num% )"))
            continue;

        const int offset = (tok->previous()->str() == ")") ? 2 : 1;

        if (tok->strAt(-2*offset) == "<") {
            if (isC() || !TemplateSimplifier::templateParameters(tok->tokAt(-2*offset)))
                continue; // '<' is less than; the condition is not a constant
        }

        // Find the token ":" then go to the next token
        Token *colon = skipTernaryOp(tok);
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
            ret = true;
        }

        // The condition is true. Delete the operator after the ":"..
        else {
            // delete the condition token and the "?"
            tok->deleteNext(2);

            unsigned int ternaryOplevel = 0;
            for (const Token *endTok = colon; endTok; endTok = endTok->next()) {
                if (Token::Match(endTok, "(|[|{")) {
                    endTok = endTok->link();
                }

                else if (endTok->str() == "?")
                    ++ternaryOplevel;
                else if (Token::Match(endTok, ")|}|]|;|,|:|>")) {
                    if (endTok->str() == ":" && ternaryOplevel)
                        --ternaryOplevel;
                    else if (endTok->str() == ">" && !templateParameterEnd)
                        ;
                    else {
                        Token::eraseTokens(colon->tokAt(-2), endTok);
                        ret = true;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void Tokenizer::simplifyUndefinedSizeArray()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%type%")) {
            Token *tok2 = tok->next();
            while (tok2 && tok2->str() == "*")
                tok2 = tok2->next();
            if (!Token::Match(tok2, "%name% [ ] ;|["))
                continue;

            tok = tok2->previous();
            Token *end = tok2->next();
            unsigned int count = 0;
            do {
                end = end->tokAt(2);
                ++count;
            } while (Token::Match(end, "[ ] [;=[]"));
            if (Token::Match(end, "[;=]")) {
                do {
                    tok2->deleteNext(2);
                    tok->insertToken("*");
                } while (--count);
                tok = end;
            } else
                tok = tok->tokAt(3);
        }
    }
}

void Tokenizer::simplifyCasts()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Don't remove cast in such cases:
        // *((char *)a + 1) = 0;
        // Remove cast when casting a function pointer:
        // (*(void (*)(char *))fp)(x);
        if (!tok->isName() &&
            Token::simpleMatch(tok->next(), "* (") &&
            !Token::Match(tok->linkAt(2), ") %name%|&")) {
            tok = tok->linkAt(2);
            continue;
        }
        // #3935 : don't remove cast in such cases:
        // ((char *)a)[1] = 0;
        if (tok->str() == "(" && Token::simpleMatch(tok->link(), ") [")) {
            tok = tok->link();
            continue;
        }
        // #4164 : ((unsigned char)1) => (1)
        if (Token::Match(tok->next(), "( %type% ) %num%") && tok->next()->link()->previous()->isStandardType()) {
            const MathLib::bigint value = MathLib::toLongNumber(tok->next()->link()->next()->str());
            unsigned int bits = _settings->char_bit * _typeSize[tok->next()->link()->previous()->str()];
            if (!tok->tokAt(2)->isUnsigned() && bits > 0)
                bits--;
            if (bits < 31 && value >= 0 && value < (1LL << bits)) {
                tok->linkAt(1)->next()->isCast(true);
                Token::eraseTokens(tok, tok->next()->link()->next());
            }
            continue;
        }

        while ((Token::Match(tok->next(), "( %type% *| *| *|&| ) *|&| %name%") && (tok->str() != ")" || tok->tokAt(2)->isStandardType())) ||
               Token::Match(tok->next(), "( const| %type% * *| *|&| ) *|&| %name%") ||
               Token::Match(tok->next(), "( const| %type% %type% *| *| *|&| ) *|&| %name%") ||
               (!tok->isName() && (Token::Match(tok->next(), "( %type% * *| *|&| ) (") ||
                                   Token::Match(tok->next(), "( const| %type% %type% * *| *|&| ) (")))) {
            if (tok->isName() && tok->str() != "return")
                break;

            if (isCPP() && tok->strAt(-1) == "operator")
                break;

            // Remove cast..
            Token::eraseTokens(tok, tok->next()->link()->next());

            // Set isCasted flag.
            Token *tok2 = tok->next();
            if (!Token::Match(tok2, "%name% [|."))
                tok2->isCast(true);
            else {
                // TODO: handle more complex expressions
                tok2->next()->isCast(true);
            }

            // Remove '* &'
            if (Token::simpleMatch(tok, "* &")) {
                tok->deleteNext();
                tok->deleteThis();
            }

            if (tok->str() == ")" && tok->link()->previous()) {
                // If there was another cast before this, go back
                // there to check it also. e.g. "(int)(char)x"
                tok = tok->link()->previous();
            }
        }

        // Replace pointer casts of 0.. "(char *)0" => "0"
        while (Token::Match(tok->next(), "( %type% %type%| * *| ) 0")) {
            tok->linkAt(1)->next()->isCast(true);
            Token::eraseTokens(tok, tok->next()->link()->next());
            if (tok->str() == ")" && tok->link()->previous()) {
                // If there was another cast before this, go back
                // there to check it also. e.g. "(char*)(char*)0"
                tok = tok->link()->previous();
            }
        }

        if (Token::Match(tok->next(), "dynamic_cast|reinterpret_cast|const_cast|static_cast <")) {
            Token *tok2 = tok->linkAt(2);
            if (!Token::simpleMatch(tok2, "> ("))
                break;

            tok2->tokAt(2)->isCast(true);
            Token::eraseTokens(tok, tok2->next());
        }
    }
}


void Tokenizer::simplifyFunctionParameters()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->link() && Token::Match(tok, "{|[|(")) {
            tok = tok->link();
        }

        // Find the function e.g. foo( x ) or foo( x, y )
        else if (Token::Match(tok, "%name% ( %name% [,)]") &&
                 !(tok->strAt(-1) == ":" || tok->strAt(-1) == ",")) {
            // We have found old style function, now we need to change it

            // First step: Get list of argument names in parentheses
            std::map<std::string, Token *> argumentNames;
            bool bailOut = false;
            Token * tokparam = nullptr;

            //take count of the function name..
            const std::string& funcName(tok->str());

            //floating token used to check for parameters
            Token *tok1 = tok;

            while (nullptr != (tok1 = tok1->tokAt(2))) {
                if (!Token::Match(tok1, "%name% [,)]")) {
                    bailOut = true;
                    break;
                }

                //same parameters: take note of the parameter
                if (argumentNames.find(tok1->str()) != argumentNames.end())
                    tokparam = tok1;
                else if (tok1->str() != funcName)
                    argumentNames[tok1->str()] = tok1;
                else {
                    if (tok1->next()->str() == ")") {
                        if (tok1->previous()->str() == ",") {
                            tok1 = tok1->tokAt(-2);
                            tok1->deleteNext(2);
                        } else {
                            tok1 = tok1->previous();
                            tok1->deleteNext();
                            bailOut = true;
                            break;
                        }
                    } else {
                        tok1 = tok1->tokAt(-2);
                        tok1->next()->deleteNext(2);
                    }
                }

                if (tok1->next()->str() == ")") {
                    tok1 = tok1->tokAt(2);
                    //expect at least a type name after round brace..
                    if (!tok1 || !tok1->isName())
                        bailOut = true;
                    break;
                }
            }

            //goto '('
            tok = tok->next();

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            tok1 = tok->link()->next();

            // there should be the sequence '; {' after the round parentheses
            for (const Token* tok2 = tok1; tok2; tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, "; {"))
                    break;
                else if (tok2->str() == "{") {
                    bailOut = true;
                    break;
                }
            }

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            // Last step: check out if the declarations between ')' and '{' match the parameters list
            std::map<std::string, Token *> argumentNames2;

            while (tok1 && tok1->str() != "{") {
                if (Token::Match(tok1, "(|)")) {
                    bailOut = true;
                    break;
                }
                if (tok1->str() == ";") {
                    if (tokparam) {
                        syntaxError(tokparam);
                    }
                    Token *tok2 = tok1->previous();
                    while (tok2->str() == "]")
                        tok2 = tok2->link()->previous();

                    //it should be a name..
                    if (!tok2->isName()) {
                        bailOut = true;
                        break;
                    }

                    if (argumentNames2.find(tok2->str()) != argumentNames2.end()) {
                        //same parameter names...
                        syntaxError(tok1);
                    } else
                        argumentNames2[tok2->str()] = tok2;

                    if (argumentNames.find(tok2->str()) == argumentNames.end()) {
                        //non-matching parameter... bailout
                        bailOut = true;
                        break;
                    }
                }
                tok1 = tok1->next();
            }

            if (bailOut || !tok1) {
                tok = tok->link();
                continue;
            }

            //the two containers may not hold the same size...
            //in that case, the missing parameters are defined as 'int'
            if (argumentNames.size() != argumentNames2.size()) {
                //move back 'tok1' to the last ';'
                tok1 = tok1->previous();
                std::map<std::string, Token *>::iterator it;
                for (it = argumentNames.begin(); it != argumentNames.end(); ++it) {
                    if (argumentNames2.find(it->first) == argumentNames2.end()) {
                        //add the missing parameter argument declaration
                        tok1->insertToken(";");
                        tok1->insertToken(it->first);
                        //register the change inside argumentNames2
                        argumentNames2[it->first] = tok1->next();
                        tok1->insertToken("int");
                    }
                }
            }

            while (tok->str() != ")") {
                //initialize start and end tokens to be moved
                Token *declStart = argumentNames2[tok->next()->str()];
                Token *declEnd = declStart;
                while (declStart->previous()->str() != ";" && declStart->previous()->str() != ")")
                    declStart = declStart->previous();
                while (declEnd->next()->str() != ";" && declEnd->next()->str() != "{")
                    declEnd = declEnd->next();

                //remove ';' after declaration
                declEnd->deleteNext();

                //replace the parameter name in the parentheses with all the declaration
                Token::replace(tok->next(), declStart, declEnd);

                //since there are changes to tokens, put tok where tok1 is
                tok = declEnd->next();
            }
            //goto forward and continue
            tok = tok->next()->link();
        }
    }
}

void Tokenizer::simplifyPointerToStandardType()
{
    if (!isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "& %name% [ 0 ] !!["))
            continue;

        // Remove '[ 0 ]' suffix
        tok->next()->eraseTokens(tok->next(), tok->tokAt(5));
        // Remove '&' prefix
        tok = tok->previous();
        if (!tok)
            break;
        tok->deleteNext();
    }
}

void Tokenizer::simplifyFunctionPointers()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // #2873 - do not simplify function pointer usage here:
        // (void)(xy(*p)(0));
        if (Token::simpleMatch(tok, ") (")) {
            tok = tok->next()->link();
            continue;
        }

        // check for function pointer cast
        if (Token::Match(tok, "( %type% %type%| *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% %type%| *| *| ( * ) (")) {
            Token *tok1 = tok;

            if (isCPP() && tok1->str() == "static_cast")
                tok1 = tok1->next();

            tok1 = tok1->next();

            if (Token::Match(tok1->next(), "%type%"))
                tok1 = tok1->next();

            while (tok1->next()->str() == "*")
                tok1 = tok1->next();

            // check that the cast ends
            if (!Token::Match(tok1->linkAt(4), ") )|>"))
                continue;

            // ok simplify this function pointer cast to an ordinary pointer cast
            tok1->deleteNext();
            tok1->next()->deleteNext();
            Token::eraseTokens(tok1->next(), tok1->linkAt(2)->next());
            continue;
        }

        // check for start of statement
        else if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|,|(|public:|protected:|private:"))
            continue;

        if (Token::Match(tok, "delete|else|return|throw|typedef"))
            continue;

        while (Token::Match(tok, "%type%|:: %type%|::"))
            tok = tok->next();

        Token *tok2 = (tok && tok->isName()) ? tok->next() : nullptr;
        while (Token::Match(tok2, "*|&"))
            tok2 = tok2->next();
        if (!tok2 || tok2->str() != "(")
            continue;
        while (Token::Match(tok2, "(|:: %type%"))
            tok2 = tok2->tokAt(2);
        if (!Token::Match(tok2, "(|:: * *| %name%"))
            continue;
        tok2 = tok2->tokAt(2);
        if (tok2->str() == "*")
            tok2 = tok2->next();
        while (Token::Match(tok2, "%type%|:: %type%|::"))
            tok2 = tok2->next();

        if (!Token::Match(tok2, "%name% ) (") &&
            !Token::Match(tok2, "%name% [ ] ) (") &&
            !(Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") ) (")))
            continue;

        while (tok->str() != "(")
            tok = tok->next();

        // check that the declaration ends
        if (!tok || !tok->link() || !tok->link()->next()) {
            syntaxError(nullptr);
        }
        Token *endTok = tok->link()->next()->link();
        if (Token::simpleMatch(endTok, ") throw ("))
            endTok = endTok->linkAt(2);
        if (!Token::Match(endTok, ") const| ;|,|)|=|[|{"))
            continue;

        if (endTok->strAt(1) == "const")
            endTok->deleteNext();

        // ok simplify this function pointer to an ordinary pointer
        Token::eraseTokens(tok->link(), endTok->next());
        tok->link()->deleteThis();
        while (Token::Match(tok, "( %type% ::"))
            tok->deleteNext(2);
        tok->deleteThis();
    }
}


bool Tokenizer::simplifyFunctionReturn()
{
    bool ret = false;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            tok = tok->link();

        else if (Token::Match(tok, "%name% ( ) { return %bool%|%char%|%num%|%str% ; }") && tok->strAt(-1) != "::") {
            const Token* const any = tok->tokAt(5);

            const std::string pattern("(|[|=|return|%op% " + tok->str() + " ( ) ;|]|)|%cop%");
            for (Token *tok2 = list.front(); tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, pattern.c_str())) {
                    if (tok->str() != tok2->strAt(1))
                        // Ticket #7916: tok is for instance "foo < bar >", a single token for an instantiation,
                        // and tok2->strAt(1) is "foo"; bail out (TODO: we can probably handle this pattern)
                        continue;
                    tok2 = tok2->next();
                    tok2->str(any->str());
                    tok2->deleteNext(2);
                    ret = true;
                }
            }
        }
    }

    return ret;
}

void Tokenizer::simplifyVarDecl(const bool only_k_r_fpar)
{
    simplifyVarDecl(list.front(), nullptr, only_k_r_fpar);
}

void Tokenizer::simplifyVarDecl(Token * tokBegin, const Token * const tokEnd, const bool only_k_r_fpar)
{
    // Split up variable declarations..
    // "int a=4;" => "int a; a=4;"
    bool finishedwithkr = true;
    for (Token *tok = tokBegin; tok != tokEnd; tok = tok->next()) {
        if (Token::simpleMatch(tok, "= {")) {
            tok = tok->next()->link();
        }
        if (!tok) {
            syntaxError(tokBegin);
        }
        if (only_k_r_fpar && finishedwithkr) {
            if (Token::Match(tok, "(|[|{")) {
                tok = tok->link();
                if (tok->next() && Token::Match(tok, ") !!{"))
                    tok = tok->next();
                else
                    continue;
            } else
                continue;
        } else if (tok->str() == "(") {
            if (isCPP()) {
                for (Token * tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
                    if (Token::Match(tok2, "[(,] [")) {
                        // lambda function at tok2->next()
                        // find start of lambda body
                        Token * lambdaBody = tok2;
                        while (lambdaBody && lambdaBody != tok2->link() && lambdaBody->str() != "{")
                            lambdaBody = lambdaBody->next();
                        if (lambdaBody && lambdaBody != tok2->link() && lambdaBody->link())
                            simplifyVarDecl(lambdaBody, lambdaBody->link()->next(), only_k_r_fpar);
                    }
                }
            }
            tok = tok->link();
        }

        if (!tok)
            syntaxError(nullptr); // #7043 invalid code
        if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|)|public:|protected:|private:"))
            continue;

        Token *type0 = tok;
        if (!Token::Match(type0, "::|extern| %type%"))
            continue;
        if (Token::Match(type0, "else|return|public:|protected:|private:"))
            continue;

        bool isconst = false;
        bool isstatic = false;
        Token *tok2 = type0;
        unsigned int typelen = 1;

        if (Token::Match(tok2, "::|extern")) {
            tok2 = tok2->next();
            typelen++;
        }

        //check if variable is declared 'const' or 'static' or both
        while (tok2) {
            if (!Token::Match(tok2, "const|static") && Token::Match(tok2, "%type% const|static")) {
                tok2 = tok2->next();
                ++typelen;
            }

            if (tok2->str() == "const")
                isconst = true;

            else if (tok2->str() == "static")
                isstatic = true;

            else if (Token::Match(tok2, "%type% :: %type%")) {
                tok2 = tok2->next();
                ++typelen;
            }

            else
                break;

            if (tok2->strAt(1) == "*")
                break;

            if (Token::Match(tok2->next(), "& %name% ,"))
                break;

            tok2 = tok2->next();
            ++typelen;
        }

        // strange looking variable declaration => don't split up.
        if (Token::Match(tok2, "%type% *|&| %name% , %type% *|&| %name%"))
            continue;

        if (Token::Match(tok2, "struct|union|class %type%")) {
            tok2 = tok2->next();
            ++typelen;
        }

        // check for qualification..
        if (Token::Match(tok2,  ":: %type%")) {
            ++typelen;
            tok2 = tok2->next();
        }

        //skip combinations of templates and namespaces
        while (!isC() && (Token::Match(tok2, "%type% <") || Token::Match(tok2, "%type% ::"))) {
            if (tok2->next()->str() == "<" && !TemplateSimplifier::templateParameters(tok2->next())) {
                tok2 = nullptr;
                break;
            }
            typelen += 2;
            tok2 = tok2->tokAt(2);
            if (tok2 && tok2->previous()->str() == "::")
                continue;
            unsigned int indentlevel = 0;
            unsigned int parens = 0;

            for (Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                ++typelen;

                if (!parens && tok3->str() == "<") {
                    ++indentlevel;
                } else if (!parens && tok3->str() == ">") {
                    if (indentlevel == 0) {
                        tok2 = tok3->next();
                        break;
                    }
                    --indentlevel;
                } else if (!parens && tok3->str() == ">>") {
                    if (indentlevel <= 1U) {
                        tok2 = tok3->next();
                        break;
                    }
                    indentlevel -= 2;
                } else if (tok3->str() == "(") {
                    ++parens;
                } else if (tok3->str() == ")") {
                    if (!parens) {
                        tok2 = nullptr;
                        break;
                    }
                    --parens;
                } else if (tok3->str() == ";") {
                    break;
                }
            }

            if (Token::Match(tok2,  ":: %type%")) {
                ++typelen;
                tok2 = tok2->next();
            }
        }

        //pattern: "%type% *| ... *| const| %name% ,|="
        if (Token::Match(tok2, "%type%") ||
            (tok2 && tok2->previous() && tok2->previous()->str() == ">")) {
            Token *varName = tok2;
            if (!tok2->previous() || tok2->previous()->str() != ">")
                varName = varName->next();
            else
                --typelen;
            //skip all the pointer part
            bool isPointerOrRef = false;
            while (Token::simpleMatch(varName, "*") || Token::Match(varName, "& %name% ,")) {
                isPointerOrRef = true;
                varName = varName->next();
            }

            while (Token::Match(varName, "%type% %type%")) {
                if (varName->str() != "const") {
                    ++typelen;
                }
                varName = varName->next();
            }
            //non-VLA case
            if (Token::Match(varName, "%name% ,|=")) {
                if (varName->str() != "operator") {
                    tok2 = varName->next(); // The ',' or '=' token

                    if (tok2->str() == "=" && (isstatic || (isconst && !isPointerOrRef))) {
                        //do not split const non-pointer variables..
                        while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                            if (Token::Match(tok2, "{|(|["))
                                tok2 = tok2->link();
                            if (!isC() && tok2->str() == "<" && TemplateSimplifier::templateParameters(tok2) > 0) {
                                tok2 = tok2->findClosingBracket();
                            }
                            if (!tok2)
                                syntaxError(nullptr); // #6881 invalid code
                            tok2 = tok2->next();
                        }
                        if (tok2 && tok2->str() == ";")
                            tok2 = nullptr;
                    }
                } else
                    tok2 = nullptr;
            }

            //VLA case
            else if (Token::Match(varName, "%name% [")) {
                tok2 = varName->next();

                while (Token::Match(tok2->link(), "] ,|=|["))
                    tok2 = tok2->link()->next();
                if (!Token::Match(tok2, "=|,"))
                    tok2 = nullptr;
                if (tok2 && tok2->str() == "=") {
                    while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                        if (Token::Match(tok2, "{|(|["))
                            tok2 = tok2->link();
                        tok2 = tok2->next();
                    }
                    if (tok2 && tok2->str() == ";")
                        tok2 = nullptr;
                }
            }

            // brace initialization
            else if (Token::Match(varName, "%name% {")) {
                tok2 = varName->next();
                tok2 = tok2->link();
                if (tok2)
                    tok2 = tok2->next();
                if (tok2 && tok2->str() != ",")
                    tok2 = nullptr;
            }

            // parenthesis, functions can't be declared like:
            // int f1(a,b), f2(c,d);
            // so if there is a comma assume this is a variable declaration
            else if (Token::Match(varName, "%name% (") && Token::simpleMatch(varName->linkAt(1), ") ,")) {
                tok2 = varName->linkAt(1)->next();
            }

            else
                tok2 = nullptr;
        } else {
            tok2 = nullptr;
        }

        if (!tok2) {
            if (only_k_r_fpar)
                finishedwithkr = false;
            continue;
        }

        if (tok2->str() == ",") {
            tok2->str(";");
            //TODO: should we have to add also template '<>' links?
            list.insertTokens(tok2, type0, typelen);
        }

        else {
            Token *eq = tok2;

            while (tok2) {
                if (Token::Match(tok2, "{|("))
                    tok2 = tok2->link();

                else if (!isC() && tok2->str() == "<" && tok2->previous()->isName() && !tok2->previous()->varId())
                    tok2 = tok2->findClosingBracket();

                else if (std::strchr(";,", tok2->str()[0])) {
                    // "type var ="   =>   "type var; var ="
                    const Token *varTok = type0->tokAt((int)typelen);
                    while (Token::Match(varTok, "*|&|const"))
                        varTok = varTok->next();
                    if (!varTok)
                        syntaxError(tok2); // invalid code
                    list.insertTokens(eq, varTok, 2);
                    eq->str(";");

                    // "= x, "   =>   "= x; type "
                    if (tok2->str() == ",") {
                        tok2->str(";");
                        list.insertTokens(tok2, type0, typelen);
                    }
                    break;
                }
                if (tok2)
                    tok2 = tok2->next();
            }
        }
        finishedwithkr = (only_k_r_fpar && tok2 && tok2->strAt(1) == "{");
    }
}

void Tokenizer::simplifyPlatformTypes()
{
    enum { isLongLong, isLong, isInt } type;

    /** @todo This assumes a flat address space. Not true for segmented address space (FAR *). */

    if (_settings->sizeof_size_t == _settings->sizeof_long)
        type = isLong;
    else if (_settings->sizeof_size_t == _settings->sizeof_long_long)
        type = isLongLong;
    else if (_settings->sizeof_size_t == _settings->sizeof_int)
        type = isInt;
    else
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // pre-check to reduce unneeded match calls
        if (!Token::Match(tok, "std| ::| %type%"))
            continue;
        bool isUnsigned;
        if (Token::Match(tok, "std| ::| size_t|uintptr_t|uintmax_t"))
            isUnsigned = true;
        else if (Token::Match(tok, "std| ::| ssize_t|ptrdiff_t|intptr_t|intmax_t"))
            isUnsigned = false;
        else
            continue;

        bool inStd = false;
        if (tok->str() == "::") {
            tok->deleteThis();
        } else if (tok->str() == "std") {
            if (tok->next()->str() != "::")
                continue;
            inStd = true;
            tok->deleteNext();
            tok->deleteThis();
        }

        if (inStd)
            tok->originalName("std::" + tok->str());
        else
            tok->originalName(tok->str());
        if (isUnsigned)
            tok->isUnsigned(true);

        switch (type) {
        case isLongLong:
            tok->isLong(true);
            tok->str("long");
            break;
        case isLong:
            tok->str("long");
            break;
        case isInt:
            tok->str("int");
            break;
        }
    }

    const std::string platform_type(_settings->platformString());

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->tokType() != Token::eType && tok->tokType() != Token::eName)
            continue;

        const Library::PlatformType * const platformtype = _settings->library.platform_type(tok->str(), platform_type);

        if (platformtype) {
            // check for namespace
            if (tok->strAt(-1) == "::") {
                const Token * tok1 = tok->tokAt(-2);
                // skip when non-global namespace defined
                if (tok1 && tok1->tokType() == Token::eName)
                    continue;
                tok = tok->previous();
                tok->deleteThis();
            }
            Token *typeToken;
            if (platformtype->_const_ptr) {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken(platformtype->_type);
                typeToken = tok;
            } else if (platformtype->_pointer) {
                tok->str(platformtype->_type);
                typeToken = tok;
                tok->insertToken("*");
            } else if (platformtype->_ptr_ptr) {
                tok->str(platformtype->_type);
                typeToken = tok;
                tok->insertToken("*");
                tok->insertToken("*");
            } else {
                tok->originalName(tok->str());
                tok->str(platformtype->_type);
                typeToken = tok;
            }
            if (platformtype->_signed)
                typeToken->isSigned(true);
            if (platformtype->_unsigned)
                typeToken->isUnsigned(true);
            if (platformtype->_long)
                typeToken->isLong(true);
        }
    }
}

void Tokenizer::simplifyStdType()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "char|short|int|long|unsigned|signed|double|float") || (_settings->standards.c >= Standards::C99 && Token::Match(tok, "complex|_Complex"))) {
            bool isFloat= false;
            bool isSigned = false;
            bool isUnsigned = false;
            bool isComplex = false;
            unsigned int countLong = 0;
            Token* typeSpec = nullptr;

            Token* tok2 = tok;
            for (; tok2->next(); tok2 = tok2->next()) {
                if (tok2->str() == "long") {
                    countLong++;
                    if (!isFloat)
                        typeSpec = tok2;
                } else if (tok2->str() == "short") {
                    typeSpec = tok2;
                } else if (tok2->str() == "unsigned")
                    isUnsigned = true;
                else if (tok2->str() == "signed")
                    isSigned = true;
                else if (Token::Match(tok2, "float|double")) {
                    isFloat = true;
                    typeSpec = tok2;
                } else if (_settings->standards.c >= Standards::C99 && Token::Match(tok2, "complex|_Complex"))
                    isComplex = !isFloat || tok2->str() == "_Complex" || Token::Match(tok2->next(), "*|&|%name%"); // Ensure that "complex" is not the variables name
                else if (Token::Match(tok2, "char|int")) {
                    if (!typeSpec)
                        typeSpec = tok2;
                } else
                    break;
            }

            if (!typeSpec) { // unsigned i; or similar declaration
                if (!isComplex) { // Ensure that "complex" is not the variables name
                    tok->str("int");
                    tok->isSigned(isSigned);
                    tok->isUnsigned(isUnsigned);
                }
            } else {
                typeSpec->isLong(typeSpec->isLong() || (isFloat && countLong == 1) || countLong > 1);
                typeSpec->isComplex(typeSpec->isComplex() || (isFloat && isComplex));
                typeSpec->isSigned(typeSpec->isSigned() || isSigned);
                typeSpec->isUnsigned(typeSpec->isUnsigned() || isUnsigned);

                // Remove specifiers
                const Token* tok3 = tok->previous();
                tok2 = tok2->previous();
                while (tok3 != tok2) {
                    if (tok2 != typeSpec &&
                        (isComplex || !Token::Match(tok2, "complex|_Complex")))  // Ensure that "complex" is not the variables name
                        tok2->deleteThis();
                    tok2 = tok2->previous();
                }
            }
        }
    }
}

void Tokenizer::simplifyStaticConst()
{
    // This function will simplify the token list so that the qualifiers "extern", "static"
    // and "const" appear in the reverse order to what is in the array below.
    const std::string qualifiers[] = {"const", "static", "extern"};

    // Move 'const' before all other qualifiers and types and then
    // move 'static' before all other qualifiers and types, ...
    for (size_t i = 0; i < sizeof(qualifiers)/sizeof(qualifiers[0]); i++) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {

            // Keep searching for a qualifier
            if (!tok->next() || tok->next()->str() != qualifiers[i])
                continue;

            // Look backwards to find the beginning of the declaration
            Token* leftTok = tok;
            for (; leftTok; leftTok = leftTok->previous()) {
                if (!Token::Match(leftTok, "%type%|static|const|extern|struct|::") ||
                    (isCPP() && Token::Match(leftTok, "private:|protected:|public:|operator")))
                    break;
            }

            // The token preceding the declaration should indicate the start of a declaration
            if (leftTok == tok ||
                (leftTok && !Token::Match(leftTok, ";|{|}|(|,|private:|protected:|public:"))) {
                continue;
            }

            // Move the qualifier to the left-most position in the declaration
            tok->deleteNext();
            if (!leftTok) {
                list.front()->insertToken(qualifiers[i], emptyString, false);
                list.front()->swapWithNext();
            } else if (leftTok->next())
                leftTok->next()->insertToken(qualifiers[i], emptyString, true);
            else
                leftTok->insertToken(qualifiers[i]);
        }
    }
}

void Tokenizer::simplifyIfAndWhileAssign()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok->next(), "if|while ("))
            continue;

        const Token* tokAt3 = tok->tokAt(3);
        if (!Token::Match(tokAt3, "!| (| %name% =") &&
            !Token::Match(tokAt3, "!| (| %name% . %name% =") &&
            !Token::Match(tokAt3, "0 == (| %name% =") &&
            !Token::Match(tokAt3, "0 == (| %name% . %name% ="))
            continue;

        // simplifying a "while(cond) { }" condition ?
        const bool iswhile(tok->next()->str() == "while");

        // simplifying a "do { } while(cond);" condition ?
        const bool isDoWhile = iswhile && Token::simpleMatch(tok, "}") && Token::simpleMatch(tok->link()->previous(), "do");
        Token* openBraceTok = tok->link();

        // delete the "if|while"
        tok->deleteNext();

        // Remember if there is a "!" or not. And delete it if there are.
        const bool isNot(Token::Match(tok->tokAt(2), "!|0"));
        if (isNot)
            tok->next()->deleteNext((tok->strAt(2) == "0") ? 2 : 1);

        // Delete parentheses.. and remember how many there are with
        // their links.
        std::stack<Token *> braces;
        while (tok->next()->str() == "(") {
            braces.push(tok->next()->link());
            tok->deleteNext();
        }

        // Skip the "%name% = ..."
        Token *tok2;
        for (tok2 = tok->next(); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ")")
                break;
        }

        // Insert "; if|while ( .."
        tok2 = tok2->previous();
        if (tok->strAt(2) == ".") {
            tok2->insertToken(tok->strAt(3));
            tok2->next()->varId(tok->tokAt(3)->varId());
            tok2->insertToken(".");
        }
        tok2->insertToken(tok->next()->str());
        tok2->next()->varId(tok->next()->varId());

        while (! braces.empty()) {
            tok2->insertToken("(");
            Token::createMutualLinks(tok2->next(), braces.top());
            braces.pop();
        }

        if (isNot)
            tok2->next()->insertToken("!");
        tok2->insertToken(iswhile ? "while" : "if");
        if (isDoWhile) {
            tok2->insertToken("}");
            Token::createMutualLinks(openBraceTok, tok2->next());
        }

        tok2->insertToken(";");

        // delete the extra "}"
        if (isDoWhile)
            tok->deleteThis();

        // If it's a while loop, insert the assignment in the loop
        if (iswhile && !isDoWhile) {
            unsigned int indentlevel = 0;
            Token *tok3 = tok2;

            for (; tok3; tok3 = tok3->next()) {
                if (tok3->str() == "{")
                    ++indentlevel;
                else if (tok3->str() == "}") {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }
            }

            if (tok3 && indentlevel == 1) {
                tok3 = tok3->previous();
                std::stack<Token *> braces2;

                for (tok2 = tok2->next(); tok2 && tok2 != tok; tok2 = tok2->previous()) {
                    tok3->insertToken(tok2->str());
                    Token *newTok = tok3->next();

                    newTok->varId(tok2->varId());
                    newTok->fileIndex(tok2->fileIndex());
                    newTok->linenr(tok2->linenr());

                    // link() new tokens manually
                    if (tok2->link()) {
                        if (Token::Match(newTok, "}|)|]|>")) {
                            braces2.push(newTok);
                        } else {
                            Token::createMutualLinks(newTok, braces2.top());
                            braces2.pop();
                        }
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyVariableMultipleAssign()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%name% = %name% = %num%|%name% ;")) {
            // skip intermediate assignments
            Token *tok2 = tok->previous();
            while (tok2 &&
                   tok2->str() == "=" &&
                   Token::Match(tok2->previous(), "%name%")) {
                tok2 = tok2->tokAt(-2);
            }

            if (!tok2 || tok2->str() != ";") {
                continue;
            }

            Token *stopAt = tok->tokAt(2);
            const Token *valueTok = stopAt->tokAt(2);
            const std::string& value(valueTok->str());
            tok2 = tok2->next();

            while (tok2 != stopAt) {
                tok2->next()->insertToken(";");
                tok2->next()->insertToken(value);
                tok2 = tok2->tokAt(4);
            }
        }
    }
}

// Binary operators simplification map
static const std::map<std::string, std::string> cAlternativeTokens = make_container< std::map<std::string, std::string> >()
        << std::make_pair("and", "&&")
        << std::make_pair("and_eq", "&=")
        << std::make_pair("bitand", "&")
        << std::make_pair("bitor", "|")
        << std::make_pair("not_eq", "!=")
        << std::make_pair("or", "||")
        << std::make_pair("or_eq", "|=")
        << std::make_pair("xor", "^")
        << std::make_pair("xor_eq", "^=") ;

// Simplify the C alternative tokens:
//  and      =>     &&
//  and_eq   =>     &=
//  bitand   =>     &
//  bitor    =>     |
//  compl    =>     ~
//  not      =>     !
//  not_eq   =>     !=
//  or       =>     ||
//  or_eq    =>     |=
//  xor      =>     ^
//  xor_eq   =>     ^=
bool Tokenizer::simplifyCAlternativeTokens()
{
    /* executable scope level */
    unsigned int executableScopeLevel = 0;

    bool ret = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{") {
            if (executableScopeLevel > 0 || Token::simpleMatch(tok->previous(), ") {"))
                ++executableScopeLevel;
            continue;
        }

        if (tok->str() == "}") {
            if (executableScopeLevel > 0)
                --executableScopeLevel;
            continue;
        }

        if (!tok->isName())
            continue;

        const std::map<std::string, std::string>::const_iterator cOpIt = cAlternativeTokens.find(tok->str());
        if (cOpIt != cAlternativeTokens.end()) {
            if (!Token::Match(tok->previous(), "%name%|%num%|%char%|)|]|> %name% %name%|%num%|%char%|%op%|("))
                continue;
            tok->str(cOpIt->second);
            ret = true;
        } else if (Token::Match(tok, "not|compl")) {
            // Don't simplify 'not p;' (in case 'not' is a type)
            if (!Token::Match(tok->next(), "%name%|(") ||
                Token::Match(tok->previous(), "[;{}]") ||
                (executableScopeLevel == 0U && tok->strAt(-1) == "("))
                continue;
            tok->str((tok->str() == "not") ? "!" : "~");
            ret = true;
        }
    }
    return ret;
}

// int i(0); => int i; i = 0;
// int i(0), j; => int i; i = 0; int j;
void Tokenizer::simplifyInitVar()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || (tok->previous() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (tok->str() == "return")
            continue;

        if (Token::Match(tok, "class|struct|union| %type% *| %name% ( &| %any% ) ;")) {
            tok = initVar(tok);
        } else if (Token::Match(tok, "%type% *| %name% ( %type% (")) {
            const Token* tok2 = tok->tokAt(2);
            if (!tok2->link())
                tok2 = tok2->next();
            if (!tok2->link() || (tok2->link()->strAt(1) == ";" && !Token::simpleMatch(tok2->linkAt(2), ") (")))
                tok = initVar(tok);
        } else if (Token::Match(tok, "class|struct|union| %type% *| %name% ( &| %any% ) ,")) {
            Token *tok1 = tok->tokAt(5);
            while (tok1->str() != ",")
                tok1 = tok1->next();
            tok1->str(";");

            const unsigned int numTokens = (Token::Match(tok, "class|struct|union")) ? 2U : 1U;
            list.insertTokens(tok1, tok, numTokens);
            tok = initVar(tok);
        }
    }
}

Token * Tokenizer::initVar(Token * tok)
{
    // call constructor of class => no simplification
    if (Token::Match(tok, "class|struct|union")) {
        if (tok->strAt(2) != "*")
            return tok;

        tok = tok->next();
    } else if (!tok->isStandardType() && tok->next()->str() != "*")
        return tok;

    // goto variable name..
    tok = tok->next();
    if (tok->str() == "*")
        tok = tok->next();

    // sizeof is not a variable name..
    if (tok->str() == "sizeof")
        return tok;

    // check initializer..
    if (tok->tokAt(2)->isStandardType() || tok->strAt(2) == "void")
        return tok;
    else if (!tok->tokAt(2)->isNumber() && !Token::Match(tok->tokAt(2), "%type% (") && tok->strAt(2) != "&" && tok->tokAt(2)->varId() == 0)
        return tok;

    // insert '; var ='
    tok->insertToken(";");
    tok->next()->insertToken(tok->str());
    tok->tokAt(2)->varId(tok->varId());
    tok = tok->tokAt(2);
    tok->insertToken("=");

    // goto '('..
    tok = tok->tokAt(2);

    // delete ')'
    tok->link()->deleteThis();

    // delete this
    tok->deleteThis();

    return tok;
}


bool Tokenizer::simplifyKnownVariables()
{
    // return value for function. Set to true if any simplifications are made
    bool ret = false;

    // constants..
    {
        std::map<unsigned int, std::string> constantValues;
        bool goback = false;
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (goback) {
                tok = tok->previous();
                goback = false;
            }
            // Reference to variable
            if (Token::Match(tok, "%type%|* & %name% = %name% ;")) {
                Token *start = tok->previous();
                while (Token::Match(start,"%type%|*|&"))
                    start = start->previous();
                if (!Token::Match(start,"[;{}]"))
                    continue;
                const Token *reftok = tok->tokAt(2);
                const Token *vartok = reftok->tokAt(2);
                int level = 0;
                for (Token *tok2 = tok->tokAt(6); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{") {
                        ++level;
                    } else if (tok2->str() == "}") {
                        if (level <= 0)
                            break;
                        --level;
                    } else if (tok2->varId() == reftok->varId()) {
                        tok2->str(vartok->str());
                        tok2->varId(vartok->varId());
                    }
                }
                Token::eraseTokens(start, tok->tokAt(6));
                tok = start;
            }

            if (tok->isName() && (Token::Match(tok, "static| const| static| %type% const| %name% = %any% ;") ||
                                  Token::Match(tok, "static| const| static| %type% const| %name% ( %any% ) ;"))) {
                bool isconst = false;
                for (const Token *tok2 = tok; (tok2->str() != "=") && (tok2->str() != "("); tok2 = tok2->next()) {
                    if (tok2->str() == "const") {
                        isconst = true;
                        break;
                    }
                }
                if (!isconst)
                    continue;

                Token *tok1 = tok;

                // start of statement
                if (tok != list.front() && !Token::Match(tok->previous(),";|{|}|private:|protected:|public:"))
                    continue;
                // skip "const" and "static"
                while (Token::Match(tok, "const|static"))
                    tok = tok->next();
                // pod type
                if (!tok->isStandardType())
                    continue;

                const Token * const vartok = (tok->next() && tok->next()->str() == "const") ? tok->tokAt(2) : tok->next();
                const Token * const valuetok = vartok->tokAt(2);
                if (Token::Match(valuetok, "%bool%|%char%|%num%|%str% )| ;")) {
                    //check if there's not a reference usage inside the code
                    bool withreference = false;
                    for (const Token *tok2 = valuetok->tokAt(2); tok2; tok2 = tok2->next()) {
                        if (Token::Match(tok2,"(|[|,|{|return|%op% & %varid%", vartok->varId())) {
                            withreference = true;
                            break;
                        }
                    }
                    //don't simplify 'f(&x)' to 'f(&100)'
                    if (withreference)
                        continue;

                    constantValues[vartok->varId()] = valuetok->str();

                    // remove statement
                    while (tok1->next()->str() != ";")
                        tok1->deleteNext();
                    tok1->deleteNext();
                    tok1->deleteThis();
                    tok = tok1;
                    goback = true;
                    ret = true;
                }
            }

            else if (tok->varId() && constantValues.find(tok->varId()) != constantValues.end()) {
                tok->str(constantValues[tok->varId()]);
            }
        }
    }

    // variable id for float/double variables
    std::set<unsigned int> floatvars;
    std::set<unsigned int> arrays;

    // auto variables..
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Search for a block of code
        Token *start = const_cast<Token *>(startOfExecutableScope(tok));
        if (!start)
            continue;

        tok = start;
        // parse the block of code..
        int indentlevel = 0;
        Token *tok2 = tok;
        for (; tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "[;{}] float|double %name% ;")) {
                floatvars.insert(tok2->tokAt(2)->varId());
            }

            if (Token::Match(tok2, "[;{}] %type% *| %name% [")) {
                const Token *nameToken = tok2->tokAt(2);
                if (nameToken->str() == "*")
                    nameToken = nameToken->next();
                arrays.insert(nameToken->varId());
            }

            if (tok2->str() == "{")
                ++indentlevel;

            else if (tok2->str() == "}") {
                --indentlevel;
                if (indentlevel <= 0)
                    break;
            }

            else if (Token::simpleMatch(tok2, "for ("))
                tok2 = tok2->next()->link();

            else if (tok2->previous()->str() != "*" && !Token::Match(tok2->tokAt(-2), "* --|++") &&
                     (Token::Match(tok2, "%name% = %bool%|%char%|%num%|%str%|%name% ;") ||
                      Token::Match(tok2, "%name% [ %num%| ] = %str% ;") ||
                      Token::Match(tok2, "%name% = & %name% ;") ||
                      (Token::Match(tok2, "%name% = & %name% [ 0 ] ;") && arrays.find(tok2->tokAt(3)->varId()) != arrays.end()))) {
                const unsigned int varid = tok2->varId();
                if (varid == 0)
                    continue;

                // initialization of static variable => the value is not *known*
                {
                    bool isstatic = false;
                    const Token *decl = tok2->previous();
                    while (decl && (decl->isName() || decl->str() == "*")) {
                        if (decl->str() == "static") {
                            isstatic = true;
                            break;
                        }
                        decl = decl->previous();
                    }
                    if (isstatic)
                        continue;
                }

                // skip loop variable
                if (Token::Match(tok2->tokAt(-2), "(|:: %type%")) {
                    const Token *tok3 = tok2->previous();
                    do {
                        tok3 = tok3->tokAt(-2);
                    } while (Token::Match(tok3->previous(), ":: %type%"));
                    if (Token::Match(tok3->tokAt(-2), "for ( %type%"))
                        continue;
                }

                // struct name..
                if (Token::Match(tok2, "%varid% = &| %varid%", tok2->varId()))
                    continue;

                const std::string structname = Token::Match(tok2->tokAt(-3), "[;{}] %name% .") ?
                                               std::string(tok2->strAt(-2) + " .") :
                                               std::string("");

                const Token * const valueToken = tok2->tokAt(2);

                std::string value;
                unsigned int valueVarId = 0;

                Token *tok3 = nullptr;
                bool valueIsPointer = false;

                // there could be a hang here if tok2 is moved back by the function calls below for some reason
                if (_settings->terminated())
                    return false;

                if (!simplifyKnownVariablesGetData(varid, &tok2, &tok3, value, valueVarId, valueIsPointer, floatvars.find(tok2->varId()) != floatvars.end()))
                    continue;

                if (valueVarId > 0 && arrays.find(valueVarId) != arrays.end())
                    continue;

                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
            }

            else if (Token::Match(tok2, "strcpy|sprintf ( %name% , %str% ) ;")) {
                const unsigned int varid(tok2->tokAt(2)->varId());
                if (varid == 0)
                    continue;

                const Token * const valueToken = tok2->tokAt(4);
                std::string value(valueToken->str());
                if (tok2->str() == "sprintf") {
                    std::string::size_type n = 0;
                    while ((n = value.find("%%", n)) != std::string::npos) {
                        // Replace "%%" with "%" - erase the first '%' and continue past the second '%'
                        value.erase(n, 1);
                        ++n;
                    }
                }
                const unsigned int valueVarId(0);
                const bool valueIsPointer(false);
                Token *tok3 = tok2->tokAt(6);
                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, emptyString, value, valueVarId, valueIsPointer, valueToken, indentlevel);

                // there could be a hang here if tok2 was moved back by the function call above for some reason
                if (_settings->terminated())
                    return false;
            }
        }

        if (tok2)
            tok = tok2->previous();
    }

    return ret;
}

bool Tokenizer::simplifyKnownVariablesGetData(unsigned int varid, Token **_tok2, Token **_tok3, std::string &value, unsigned int &valueVarId, bool &valueIsPointer, bool floatvar)
{
    Token *tok2 = *_tok2;
    Token *tok3 = nullptr;

    if (Token::simpleMatch(tok2->tokAt(-2), "for (")) {
        // only specific for loops is handled
        if (!Token::Match(tok2, "%varid% = %num% ; %varid% <|<= %num% ; ++| %varid% ++| ) {", varid))
            return false;

        // is there a "break" in the for loop?
        bool hasbreak = false;
        const Token* end4 = tok2->linkAt(-1)->linkAt(1);
        for (const Token *tok4 = tok2->previous()->link(); tok4 != end4; tok4 = tok4->next()) {
            if (tok4->str() == "break") {
                hasbreak = true;
                break;
            }
        }
        if (hasbreak)
            return false;

        // no break => the value of the counter value is known after the for loop..
        const Token* compareTok = tok2->tokAt(5);
        if (compareTok->str() == "<") {
            value = compareTok->next()->str();
            valueVarId = compareTok->next()->varId();
        } else
            value = MathLib::toString(MathLib::toLongNumber(compareTok->next()->str()) + 1);

        // Skip for-body..
        tok3 = tok2->previous()->link()->next()->link()->next();
    } else {
        value = tok2->strAt(2);
        valueVarId = tok2->tokAt(2)->varId();
        if (tok2->strAt(1) == "[") {
            value = tok2->next()->link()->strAt(2);
            valueVarId = 0;
        } else if (value == "&") {
            value = tok2->strAt(3);
            valueVarId = tok2->tokAt(3)->varId();

            // *ptr = &var; *ptr = 5;
            // equals
            // var = 5; not *var = 5;
            if (tok2->strAt(4) == ";")
                valueIsPointer = true;
        }

        // Add a '.0' to a decimal value and therefore convert it to an floating point number.
        else if (MathLib::isDec(tok2->strAt(2)) && floatvar) {
            value += ".0";
        }

        // float variable: convert true/false to 1.0 / 0.0
        else if (tok2->tokAt(2)->isBoolean() && floatvar) {
            value = (value == "true") ? "1.0" : "0.0";
        }

        if (Token::simpleMatch(tok2->next(), "= &"))
            tok2 = tok2->tokAt(3);

        tok3 = tok2->next();
    }
    *_tok2 = tok2;
    *_tok3 = tok3;
    return true;
}

bool Tokenizer::simplifyKnownVariablesSimplify(Token **tok2, Token *tok3, unsigned int varid, const std::string &structname, std::string &value, unsigned int valueVarId, bool valueIsPointer, const Token * const valueToken, int indentlevel) const
{
    const bool pointeralias(valueToken->isName() || Token::Match(valueToken, "& %name% ["));
    const bool varIsGlobal = (indentlevel == 0);
    const bool printDebug = _settings->debugwarnings;

    if (_errorLogger && !list.getFiles().empty())
        _errorLogger->reportProgress(list.getFiles()[0], "Tokenize (simplifyKnownVariables)", tok3->progressValue());

    if (isMaxTime())
        return false;

    bool ret = false;

    Token* bailOutFromLoop = nullptr;
    int indentlevel3 = indentlevel;
    bool ret3 = false;
    for (; tok3; tok3 = tok3->next()) {
        if (tok3->str() == "{") {
            ++indentlevel3;
        } else if (tok3->str() == "}") {
            --indentlevel3;
            if (indentlevel3 < indentlevel) {
                if (Token::Match((*tok2)->tokAt(-7), "%type% * %name% ; %name% = & %name% ;") &&
                    (*tok2)->strAt(-5) == (*tok2)->strAt(-3)) {
                    (*tok2) = (*tok2)->tokAt(-4);
                    Token::eraseTokens((*tok2), (*tok2)->tokAt(6));
                }
                break;
            }
        }

        // Stop if there is a pointer alias and a shadow variable is
        // declared in an inner scope (#3058)
        if (valueIsPointer && tok3->varId() > 0 &&
            tok3->previous() && (tok3->previous()->isName() || tok3->previous()->str() == "*") &&
            valueToken->str() == "&" &&
            valueToken->next() &&
            valueToken->next()->isName() &&
            tok3->str() == valueToken->next()->str() &&
            tok3->varId() > valueToken->next()->varId()) {
            // more checking if this is a variable declaration
            bool decl = true;
            for (const Token *tok4 = tok3->previous(); tok4; tok4 = tok4->previous()) {
                if (Token::Match(tok4, "[;{}]"))
                    break;

                else if (tok4->isName()) {
                    if (tok4->varId() > 0) {
                        decl = false;
                        break;
                    }
                }

                else if (!Token::Match(tok4, "[&*]")) {
                    decl = false;
                    break;
                }
            }
            if (decl)
                break;
        }

        // Stop if label is found
        if (Token::Match(tok3, "; %type% : ;"))
            break;

        // Stop if break/continue is found ..
        if (Token::Match(tok3, "break|continue"))
            break;
        if ((indentlevel3 > 1 || !Token::simpleMatch(Token::findsimplematch(tok3,";"), "; }")) && tok3->str() == "return")
            ret3 = true;
        if (ret3 && tok3->str() == ";")
            break;

        if (pointeralias && Token::Match(tok3, ("!!= " + value).c_str()))
            break;

        // Stop if a loop is found
        if (pointeralias && Token::Match(tok3, "do|for|while"))
            break;

        // Stop if unknown function call is seen and the variable is global: it might be
        // changed by the function call
        if (varIsGlobal && tok3->str() == ")" && tok3->link() &&
            Token::Match(tok3->link()->tokAt(-2), "[;{}] %name% (") &&
            !Token::Match(tok3->link()->previous(), "if|for|while|switch|BOOST_FOREACH"))
            break;

        // Stop if something like 'while (--var)' is found
        if (Token::Match(tok3, "for|while|do")) {
            const Token *endpar = tok3->next()->link();
            if (Token::simpleMatch(endpar, ") {"))
                endpar = endpar->next()->link();
            bool bailout = false;
            for (const Token *tok4 = tok3; tok4 && tok4 != endpar; tok4 = tok4->next()) {
                if (Token::Match(tok4, "++|-- %varid%", varid) ||
                    Token::Match(tok4, "%varid% ++|--|=", varid)) {
                    bailout = true;
                    break;
                }
            }
            if (bailout)
                break;
        }

        if (bailOutFromLoop) {
            // This could be a loop, skip it, but only if it doesn't contain
            // the variable we are checking for. If it contains the variable
            // we will bail out.
            if (tok3->varId() == varid) {
                // Continue
                //tok2 = bailOutFromLoop;
                break;
            } else if (tok3 == bailOutFromLoop) {
                // We have skipped the loop
                bailOutFromLoop = nullptr;
                continue;
            }

            continue;
        } else if (tok3->str() == "{" && tok3->previous()->str() == ")") {
            // There is a possible loop after the assignment. Try to skip it.
            if (tok3->previous()->link() &&
                tok3->previous()->link()->strAt(-1) != "if")
                bailOutFromLoop = tok3->link();
            continue;
        }

        // Variable used in realloc (see Ticket #1649)
        if (Token::Match(tok3, "%name% = realloc ( %name% ,") &&
            tok3->varId() == varid &&
            tok3->tokAt(4)->varId() == varid) {
            tok3->tokAt(4)->str(value);
            ret = true;
        }

        // condition "(|&&|%OROR% %varid% )|&&|%OROR%|;
        if (!Token::Match(tok3->previous(), "( %name% )") &&
            Token::Match(tok3->previous(), "&&|(|%oror% %varid% &&|%oror%|)|;", varid)) {
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // parameter in function call..
        if (tok3->varId() == varid && Token::Match(tok3->previous(), "[(,] %name% [,)]")) {
            // If the parameter is passed by value then simplify it
            if (isFunctionParameterPassedByValue(tok3)) {
                tok3->str(value);
                tok3->varId(valueVarId);
                ret = true;
            }
        }

        // Variable is used somehow in a non-defined pattern => bail out
        if (tok3->varId() == varid) {
            // This is a really generic bailout so let's try to avoid this.
            // There might be lots of false negatives.
            if (printDebug) {
                // FIXME: Fix all the debug warnings for values and then
                // remove this bailout
                if (pointeralias)
                    break;

                // suppress debug-warning when calling member function
                if (Token::Match(tok3->next(), ". %name% ("))
                    break;

                // suppress debug-warning when assignment
                if (tok3->strAt(1) == "=")
                    break;

                // taking address of variable..
                if (Token::Match(tok3->tokAt(-2), "return|= & %name% ;"))
                    break;

                // parameter in function call..
                if (Token::Match(tok3->tokAt(-2), "%name% ( %name% ,|)") ||
                    Token::Match(tok3->previous(), ", %name% ,|)"))
                    break;

                // conditional increment
                if (Token::Match(tok3->tokAt(-3), ") { ++|--") ||
                    Token::Match(tok3->tokAt(-2), ") { %name% ++|--"))
                    break;

                reportError(tok3, Severity::debug, "debug",
                            "simplifyKnownVariables: bailing out (variable="+tok3->str()+", value="+value+")");
            }

            break;
        }

        // Using the variable in condition..
        if (Token::Match(tok3->previous(), ("if ( " + structname + " %varid% %cop%|)").c_str(), varid) ||
            Token::Match(tok3, ("( " + structname + " %varid% %comp%").c_str(), varid) ||
            Token::Match(tok3, ("%comp%|!|= " + structname + " %varid% %cop%|)|;").c_str(), varid) ||
            Token::Match(tok3->previous(), "strlen|free ( %varid% )", varid)) {
            if (value[0] == '\"' && tok3->previous()->str() != "strlen") {
                // bail out if value is a string unless if it's just given
                // as parameter to strlen
                break;
            }
            if (!structname.empty()) {
                tok3->deleteNext(2);
            }
            if (Token::Match(valueToken, "& %name% ;")) {
                tok3->insertToken("&");
                tok3 = tok3->next();
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // pointer alias used in condition..
        if (Token::Match(valueToken,"& %name% ;") && Token::Match(tok3, ("( * " + structname + " %varid% %cop%").c_str(), varid)) {
            tok3->deleteNext();
            if (!structname.empty())
                tok3->deleteNext(2);
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Delete pointer alias
        if (isCPP() && pointeralias && (tok3->str() == "delete") && tok3->next() &&
            (Token::Match(tok3->next(), "%varid% ;", varid) ||
             Token::Match(tok3->next(), "[ ] %varid%", varid))) {
            tok3 = (tok3->next()->str() == "[") ? tok3->tokAt(3) : tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Variable is used in function call..
        if (Token::Match(tok3, ("%name% ( " + structname + " %varid% ,").c_str(), varid)) {
            static const char * const functionName[] = {
                // always simplify
                "strcmp", "strdup",
                // don't simplify buffer value
                "memcmp","memcpy","memmove","memset","strcpy","strncmp","strncpy"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i) {
                if (valueVarId == 0U && i >= 2)
                    break;
                if (tok3->str() == functionName[i]) {
                    Token *par1 = tok3->tokAt(2);
                    if (!structname.empty()) {
                        par1->deleteNext();
                        par1->deleteThis();
                    }
                    par1->str(value);
                    par1->varId(valueVarId);
                    break;
                }
            }
        }

        // Variable is used as 2nd parameter in function call..
        if (Token::Match(tok3, ("%name% ( %any% , " + structname + " %varid% ,|)").c_str(), varid)) {
            static const char * const functionName[] = {
                // always simplify
                "strcmp","strcpy","strncmp","strncpy",
                // don't simplify buffer value
                "memcmp","memcpy","memmove"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i) {
                if (valueVarId == 0U && i >= 4)
                    break;
                if (tok3->str() == functionName[i]) {
                    Token *par = tok3->tokAt(4);
                    if (!structname.empty()) {
                        par->deleteNext();
                        par->deleteThis();
                    }
                    par->str(value);
                    par->varId(valueVarId);
                    break;
                }
            }
        }

        // array usage
        if (value[0] != '\"' && Token::Match(tok3, ("[(,] " + structname + " %varid% [|%cop%").c_str(), varid)) {
            if (!structname.empty()) {
                tok3->deleteNext(2);
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // The >> operator is sometimes used to assign a variable in C++
        if (isCPP() && Token::Match(tok3, (">> " + structname + " %varid%").c_str(), varid)) {
            // bailout for such code:   ; std :: cin >> i ;
            const Token *prev = tok3->previous();
            while (prev && prev->str() != "return" && Token::Match(prev, "%name%|::|*"))
                prev = prev->previous();
            if (Token::Match(prev, ";|{|}|>>"))
                break;
        }

        // Variable is used in calculation..
        if (((tok3->previous()->varId() > 0) && Token::Match(tok3, ("& " + structname + " %varid%").c_str(), varid)) ||
            (Token::Match(tok3, ("[=+-*/%^|[] " + structname + " %varid% [=?+-*/%^|;])]").c_str(), varid) && !Token::Match(tok3, ("= " + structname + " %name% =").c_str())) ||
            Token::Match(tok3, ("[(=+-*/%^|[] " + structname + " %varid% <<|>>").c_str(), varid) ||
            Token::Match(tok3, ("<<|>> " + structname + " %varid% %cop%|;|]|)").c_str(), varid) ||
            Token::Match(tok3->previous(), ("[=+-*/%^|[] ( " + structname + " %varid% !!=").c_str(), varid)) {
            if (value[0] == '\"')
                break;
            if (!structname.empty()) {
                tok3->deleteNext(2);
                ret = true;
            }
            tok3 = tok3->next();
            if (tok3->str() != value)
                ret = true;
            tok3->str(value);
            tok3->varId(valueVarId);
            if (tok3->previous()->str() == "*" && (valueIsPointer || Token::Match(valueToken, "& %name% ;"))) {
                tok3 = tok3->previous();
                tok3->deleteThis();
                ret = true;
            } else if (Token::Match(valueToken, "& %name% ;"))
                tok3->insertToken("&", emptyString, true);
        }

        if (Token::simpleMatch(tok3, "= {")) {
            const Token* const end4 = tok3->linkAt(1);
            for (const Token *tok4 = tok3; tok4 != end4; tok4 = tok4->next()) {
                if (Token::Match(tok4, "{|, %varid% ,|}", varid)) {
                    tok4->next()->str(value);
                    tok4->next()->varId(valueVarId);
                    ret = true;
                }
            }
        }

        // Using the variable in for-condition..
        if (Token::simpleMatch(tok3, "for (")) {
            for (Token *tok4 = tok3->tokAt(2); tok4; tok4 = tok4->next()) {
                if (Token::Match(tok4, "(|)"))
                    break;

                // Replace variable used in condition..
                if (Token::Match(tok4, "; %name% <|<=|!= %name% ; ++| %name% ++| )")) {
                    const Token *inctok = tok4->tokAt(5);
                    if (inctok->str() == "++")
                        inctok = inctok->next();
                    if (inctok->varId() == varid)
                        break;

                    if (tok4->next()->varId() == varid) {
                        tok4->next()->str(value);
                        tok4->next()->varId(valueVarId);
                        ret = true;
                    }
                    if (tok4->tokAt(3)->varId() == varid) {
                        tok4->tokAt(3)->str(value);
                        tok4->tokAt(3)->varId(valueVarId);
                        ret = true;
                    }
                }
            }
        }

        if (indentlevel == indentlevel3 && Token::Match(tok3->next(), "%varid% ++|--", varid) && MathLib::isInt(value)) {
            const std::string op(tok3->strAt(2));
            if (Token::Match(tok3, "[{};] %any% %any% ;")) {
                tok3->deleteNext(3);
            } else {
                tok3 = tok3->next();
                tok3->str(value);
                tok3->varId(valueVarId);
                tok3->deleteNext();
            }
            value = MathLib::incdec(value, op);
            if (!Token::simpleMatch((*tok2)->tokAt(-2), "for (")) {
                (*tok2)->tokAt(2)->str(value);
                (*tok2)->tokAt(2)->varId(valueVarId);
            }
            ret = true;
        }

        if (indentlevel == indentlevel3 && Token::Match(tok3->next(), "++|-- %varid%", varid) && MathLib::isInt(value) &&
            !Token::Match(tok3->tokAt(3), "[.[]")) {
            value = MathLib::incdec(value, tok3->next()->str());
            (*tok2)->tokAt(2)->str(value);
            (*tok2)->tokAt(2)->varId(valueVarId);
            if (Token::Match(tok3, "[;{}] %any% %any% ;")) {
                tok3->deleteNext(3);
            } else {
                tok3->deleteNext();
                tok3->next()->str(value);
                tok3->next()->varId(valueVarId);
            }
            tok3 = tok3->next();
            ret = true;
        }

        // return variable..
        if (Token::Match(tok3, "return %varid% %any%", varid) &&
            (tok3->tokAt(2)->isExtendedOp() || tok3->strAt(2) == ";") &&
            value[0] != '\"') {
            tok3->next()->str(value);
            tok3->next()->varId(valueVarId);
        }

        else if (pointeralias && Token::Match(tok3, "return * %varid% ;", varid) && value[0] != '\"') {
            tok3->deleteNext();
            tok3->next()->str(value);
            tok3->next()->varId(valueVarId);
        }
    }
    return ret;
}


void Tokenizer::elseif()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "else if"))
            continue;

        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "(|{|["))
                tok2 = tok2->link();

            if (Token::Match(tok2, "}|;")) {
                if (tok2->next() && tok2->next()->str() != "else") {
                    tok->insertToken("{");
                    tok2->insertToken("}");
                    Token::createMutualLinks(tok->next(), tok2->next());
                    break;
                }
            }
        }
    }
}


bool Tokenizer::simplifyRedundantParentheses()
{
    bool ret = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "(")
            continue;

        if (Token::simpleMatch(tok, "( {"))
            continue;

        if (Token::Match(tok->link(), ") %num%")) {
            tok = tok->link();
            continue;
        }

        // !!operator = ( x ) ;
        if (tok->strAt(-2) != "operator" &&
            tok->previous() && tok->previous()->str() == "=" &&
            tok->next() && tok->next()->str() != "{" &&
            Token::simpleMatch(tok->link(), ") ;")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            continue;
        }

        while (Token::simpleMatch(tok, "( (") &&
               tok->link() && tok->link()->previous() == tok->next()->link()) {
            // We have "(( *something* ))", remove the inner
            // parentheses
            tok->deleteNext();
            tok->link()->tokAt(-2)->deleteNext();
            ret = true;
        }

        if (isCPP() && Token::Match(tok->tokAt(-2), "[;{}=(] new (") && Token::Match(tok->link(), ") [;,{}[]")) {
            // Remove the parentheses in "new (type)" constructs
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "! ( %name% )")) {
            // Remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(,;{}] ( %name% ) .")) {
            // Remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(,;{}] ( %name% (") &&
            tok->link()->previous() == tok->linkAt(2)) {
            // We have "( func ( *something* ))", remove the outer
            // parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[,;{}] ( delete [| ]| %name% ) ;")) {
            // We have "( delete [| ]| var )", remove the outer
            // parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (!Token::simpleMatch(tok->tokAt(-2), "operator delete") &&
            Token::Match(tok->previous(), "delete|; (") &&
            (tok->previous()->str() != "delete" || tok->next()->varId() > 0) &&
            Token::Match(tok->link(), ") ;|,")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(!*;{}] ( %name% )") &&
            (tok->next()->varId() != 0 || Token::Match(tok->tokAt(3), "[+-/=]")) && !tok->next()->isStandardType()) {
            // We have "( var )", remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{}[(,!*] ( %name% .")) {
            Token *tok2 = tok->tokAt(2);
            while (Token::Match(tok2, ". %name%")) {
                tok2 = tok2->tokAt(2);
            }
            if (tok2 != tok->link())
                break;
            // We have "( var . var . ... . var )", remove the parentheses
            tok = tok->previous();
            tok->deleteNext();
            tok2->deleteThis();
            ret = true;
            continue;
        }

        if (Token::simpleMatch(tok->previous(), "? (") && Token::simpleMatch(tok->link(), ") :")) {
            const Token *tok2 = tok->next();
            while (tok2 && (Token::Match(tok2,"%bool%|%num%|%name%") || tok2->isArithmeticalOp()))
                tok2 = tok2->next();
            if (tok2 && tok2->str() == ")") {
                tok->link()->deleteThis();
                tok->deleteThis();
                ret = true;
                continue;
            }
        }

        while (Token::Match(tok->previous(), "[{([,] ( !!{") &&
               Token::Match(tok->link(), ") [;,])]") &&
               !Token::simpleMatch(tok->tokAt(-2), "operator ,") && // Ticket #5709
               !Token::findsimplematch(tok, ",", tok->link())) {
            // We have "( ... )", remove the parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::simpleMatch(tok->previous(), ", (") &&
            Token::simpleMatch(tok->link(), ") =")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        // Simplify "!!operator !!%name%|)|>|>> ( %num%|%bool% ) %op%|;|,|)"
        if (Token::Match(tok, "( %bool%|%num% ) %cop%|;|,|)") &&
            tok->strAt(-2) != "operator" &&
            tok->previous() &&
            !Token::Match(tok->previous(), "%name%|)") &&
            (!(isCPP() && Token::Match(tok->previous(),">|>>")))) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "*|& ( %name% )")) {
            // We may have a variable declaration looking like "type_name *(var_name)"
            Token *tok2 = tok->tokAt(-2);
            while (Token::Match(tok2, "%type%|static|const|extern") && tok2->str() != "operator") {
                tok2 = tok2->previous();
            }
            if (tok2 && !Token::Match(tok2, "[;,{]")) {
                // Not a variable declaration
            } else {
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    }
    return ret;
}

void Tokenizer::simplifyCharAt()
{
    // Replace "string"[0] with 's'
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%str% [ %num% ]")) {
            const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(2));
            // Check within range
            if (index >= 0 && index <= Token::getStrLength(tok)) {
                tok->str("'" + Token::getCharAt(tok, index) + "'");
                tok->deleteNext(3);
            }
        }
    }
}

void Tokenizer::simplifyReference()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // starting executable scope..
        Token *start = const_cast<Token *>(startOfExecutableScope(tok));
        if (start) {
            tok = start;
            // replace references in this scope..
            Token * const end = tok->link();
            for (Token *tok2 = tok; tok2 && tok2 != end; tok2 = tok2->next()) {
                // found a reference..
                if (Token::Match(tok2, "[;{}] %type% & %name% (|= %name% )| ;")) {
                    const unsigned int refId = tok2->tokAt(3)->varId();
                    if (!refId)
                        continue;

                    // replace reference in the code..
                    for (Token *tok3 = tok2->tokAt(7); tok3 && tok3 != end; tok3 = tok3->next()) {
                        if (tok3->varId() == refId) {
                            tok3->str(tok2->strAt(5));
                            tok3->varId(tok2->tokAt(5)->varId());
                        }
                    }

                    tok2->deleteNext(6+(tok2->strAt(6)==")" ? 1 : 0));
                }
            }
            tok = end;
        }
    }
}

bool Tokenizer::simplifyCalculations()
{
    return TemplateSimplifier::simplifyCalculations(list.front());
}

void Tokenizer::simplifyOffsetPointerDereference()
{
    // Replace "*(str + num)" => "str[num]" and
    // Replace "*(str - num)" => "str[-num]"
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() && !tok->isLiteral()
            && !Token::Match(tok, "]|)|++|--")
            && Token::Match(tok->next(), "* ( %name% +|- %num%|%name% )")) {

            // remove '* ('
            tok->deleteNext(2);

            // '+'->'['
            tok = tok->tokAt(2);
            Token* const openBraceTok = tok;
            const bool isNegativeIndex = (tok->str() == "-");
            tok->str("[");

            // Insert a "-" in front of the number or variable
            if (isNegativeIndex) {
                if (tok->next()->isName()) {
                    tok->insertToken("-");
                    tok = tok->next();
                } else
                    tok->next()->str(std::string("-") + tok->next()->str());
            }

            tok = tok->tokAt(2);
            tok->str("]");
            Token::createMutualLinks(openBraceTok, tok);
        }
    }
}

void Tokenizer::simplifyOffsetPointerReference()
{
    std::set<unsigned int> pod;
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isStandardType()) {
            tok = tok->next();
            while (tok && (tok->str() == "*" || tok->isName())) {
                if (tok->varId() > 0) {
                    pod.insert(tok->varId());
                    break;
                }
                tok = tok->next();
            }
            if (!tok)
                break;
        }
    }

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%num%|%name%|]|)") &&
            (Token::Match(tok->next(), "& %name% [ %num%|%name% ] !!["))) {
            tok = tok->next();

            if (tok->next()->varId()) {
                if (pod.find(tok->next()->varId()) == pod.end()) {
                    tok = tok->tokAt(5);
                    if (!tok)
                        syntaxError(tok);
                    continue;
                }
            }

            // '&' => '('
            tok->str("(");

            tok = tok->next();
            // '[' => '+'
            tok->deleteNext();
            tok->insertToken("+");

            tok = tok->tokAt(3);
            //remove ']'
            tok->str(")");
            Token::createMutualLinks(tok->tokAt(-4), tok);
        }
    }
}

void Tokenizer::simplifyNestedStrcat()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (! Token::Match(tok, "[;{}] strcat ( strcat (")) {
            continue;
        }

        // find inner strcat call
        Token *tok2 = tok->tokAt(3);
        while (Token::simpleMatch(tok2, "strcat ( strcat"))
            tok2 = tok2->tokAt(2);

        if (tok2->strAt(3) != ",")
            continue;

        // If we have this code:
        //   strcat(strcat(dst, foo), bar);
        // We move this part of code before all strcat() calls: strcat(dst, foo)
        // And place "dst" token where the code was.
        Token *prevTok = tok2->previous();

        // Move tokens to new place
        Token::move(tok2, tok2->next()->link(), tok);
        tok = tok2->next()->link();

        // Insert the "dst" token
        prevTok->insertToken(tok2->strAt(2));
        prevTok->next()->varId(tok2->tokAt(2)->varId());

        // Insert semicolon after the moved strcat()
        tok->insertToken(";");
    }
}

// Check if this statement is a duplicate definition.  A duplicate
// definition will hide the enumerator within it's scope so just
// skip the entire scope of the duplicate.
bool Tokenizer::duplicateDefinition(Token ** tokPtr)
{
    // check for an end of definition
    const Token * tok = *tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>")) {
        const Token * end = tok->next();

        if (end->str() == "[") {
            end = end->link()->next();
        } else if (end->str() == ",") {
            // check for function argument
            if (Token::Match(tok->previous(), "(|,"))
                return false;

            // find end of definition
            int level = 0;
            while (end->next() && (!Token::Match(end->next(), ";|)|>") ||
                                   (end->next()->str() == ")" && level == 0))) {
                if (end->next()->str() == "(")
                    ++level;
                else if (end->next()->str() == ")")
                    --level;

                end = end->next();
            }
        } else if (end->str() == ")") {
            // check for function argument
            if (tok->previous()->str() == ",")
                return false;
        }

        if (end) {
            if (Token::simpleMatch(end, ") {")) { // function parameter ?
                // make sure it's not a conditional
                if (Token::Match(end->link()->previous(), "if|for|while|switch|BOOST_FOREACH") || Token::Match(end->link()->tokAt(-2), ":|,"))
                    return false;

                // look backwards
                if (tok->previous()->str() == "enum" ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return") ||
                    Token::Match(tok->tokAt(-2), "%type% &|*")) {
                    // duplicate definition so skip entire function
                    *tokPtr = end->next()->link();
                    return true;
                }
            } else if (end->str() == ">") { // template parameter ?
                // look backwards
                if (tok->previous()->str() == "enum" ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return")) {
                    // duplicate definition so skip entire template
                    while (end && end->str() != "{")
                        end = end->next();
                    if (end) {
                        *tokPtr = end->link();
                        return true;
                    }
                }
            } else {
                if (Token::Match(tok->previous(), "enum|,"))
                    return true;
                else if (Token::Match(tok->previous(), "%type%")) {
                    // look backwards
                    const Token *back = tok;
                    while (back && back->isName())
                        back = back->previous();
                    if (!back || (Token::Match(back, "[(,;{}]") && !Token::Match(back->next(),"return|throw")))
                        return true;
                }
            }
        }
    }
    return false;
}

namespace {
    const std::set<std::string> stdFunctionsPresentInC = make_container< std::set<std::string> > () <<
            "strcat" <<
            "strcpy" <<
            "strncat" <<
            "strncpy" <<
            "free" <<
            "malloc" <<
            "strdup";
}

void Tokenizer::simplifyStd()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "std")
            continue;

        if (Token::Match(tok->previous(), "[(,{};] std :: %name% (") &&
            stdFunctionsPresentInC.find(tok->strAt(2)) != stdFunctionsPresentInC.end()) {
            tok->deleteNext();
            tok->deleteThis();
        }
    }
}

//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

bool Tokenizer::IsScopeNoReturn(const Token *endScopeToken, bool *unknown) const
{
    std::string unknownFunc;
    const bool ret = _settings->library.isScopeNoReturn(endScopeToken,&unknownFunc);
    if (unknown)
        *unknown = !unknownFunc.empty();
    if (!unknownFunc.empty() && _settings->checkLibrary && _settings->isEnabled("information")) {
        // Is function global?
        bool globalFunction = true;
        if (Token::simpleMatch(endScopeToken->tokAt(-2), ") ; }")) {
            const Token * const ftok = endScopeToken->linkAt(-2)->previous();
            if (ftok &&
                ftok->isName() &&
                ftok->function() &&
                ftok->function()->nestedIn &&
                ftok->function()->nestedIn->type != Scope::eGlobal) {
                globalFunction = false;
            }
        }

        // don't warn for nonglobal functions (class methods, functions hidden in namespaces) since they can't be configured yet
        // FIXME: when methods and namespaces can be configured properly, remove the "globalFunction" check
        if (globalFunction) {
            reportError(endScopeToken->previous(),
                        Severity::information,
                        "checkLibraryNoReturn",
                        "--check-library: Function " + unknownFunc + "() should have <noreturn> configuration");
        }
    }
    return ret;
}

//---------------------------------------------------------------------------

bool Tokenizer::isFunctionParameterPassedByValue(const Token *fpar) const
{
    if (isC()) // C does not support references
        return true;

    // TODO: If symbol database is available, use it.
    const Token *ftok;

    // Look at function call, what parameter number is it?
    unsigned int parameter = 1;
    for (ftok = fpar->previous(); ftok; ftok = ftok->previous()) {
        if (ftok->str() == "(")
            break;
        else if (ftok->str() == ")")
            ftok = ftok->link();
        else if (ftok->str() == ",")
            ++parameter;
        else if (Token::Match(ftok, "[;{}]"))
            break;
    }

    // Is this a function call?
    if (ftok && Token::Match(ftok->tokAt(-2), "[;{}=] %name% (")) {
        const std::string& functionName(ftok->previous()->str());

        if (functionName == "return")
            return true;

        // Locate function declaration..
        for (const Token *tok = tokens(); tok; tok = tok->next()) {
            if (tok->str() == "{")
                tok = tok->link();
            else if (Token::Match(tok, "%type% (") && tok->str() == functionName) {
                // Goto parameter
                tok = tok->tokAt(2);
                unsigned int par = 1;
                while (tok && par < parameter) {
                    if (tok->str() == ")")
                        break;
                    if (tok->str() == ",")
                        ++par;
                    tok = tok->next();
                }
                if (!tok)
                    return false;

                // If parameter was found, determine if it's passed by value
                if (par == parameter) {
                    bool knowntype = false;
                    while (tok && tok->isName()) {
                        knowntype |= tok->isStandardType();
                        knowntype |= (tok->str() == "struct");
                        tok = tok->next();
                    }
                    if (!tok || !knowntype)
                        return false;
                    if (tok->str() != "," && tok->str() != ")")
                        return false;
                    return true;
                }
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------

void Tokenizer::eraseDeadCode(Token *begin, const Token *end)
{
    if (!begin)
        return;
    const bool isgoto = Token::Match(begin->tokAt(-2), "goto %name% ;");
    unsigned int indentlevel = 1,
                 indentcase = 0,
                 indentswitch = 0,
                 indentlabel = 0,
                 roundbraces = 0,
                 indentcheck = 0;
    std::vector<unsigned int> switchindents;
    bool checklabel = false;
    Token *tok = begin;
    Token *tokcheck = nullptr;
    while (tok->next() && tok->next() != end) {
        if (tok->next()->str() == "(") {
            ++roundbraces;
            tok->deleteNext();
            continue;
        } else if (tok->next()->str() == ")") {
            if (!roundbraces)
                break;  //too many ending round parentheses
            --roundbraces;
            tok->deleteNext();
            continue;
        }

        if (roundbraces) {
            tok->deleteNext();
            continue;
        }

        if (Token::Match(tok, "[{};] switch (")) {
            if (!checklabel) {
                if (!indentlabel) {
                    //remove 'switch ( ... )'
                    Token::eraseTokens(tok, tok->linkAt(2)->next());
                } else {
                    tok = tok->linkAt(2);
                }
                if (tok->next()->str() == "{") {
                    ++indentswitch;
                    indentcase = indentlevel + 1;
                    switchindents.push_back(indentcase);
                }
            } else {
                tok = tok->linkAt(2);
                if (Token::simpleMatch(tok, ") {")) {
                    ++indentswitch;
                    indentcase = indentlevel + 1;
                    switchindents.push_back(indentcase);
                }
            }
        } else if (tok->next()->str() == "{") {
            ++indentlevel;
            if (!checklabel) {
                checklabel = true;
                tokcheck = tok;
                indentcheck = indentlevel;
                indentlabel = 0;
            }
            tok = tok->next();
        } else if (tok->next()->str() == "}") {
            --indentlevel;
            if (!indentlevel)
                break;

            if (!checklabel) {
                tok->deleteNext();
            } else {
                if (indentswitch && indentlevel == indentcase)
                    --indentlevel;
                if (indentlevel < indentcheck) {
                    const Token *end2 = tok->next();
                    tok = end2->link()->previous();  //return to initial '{'
                    if (indentswitch && Token::simpleMatch(tok, ") {") && Token::Match(tok->link()->tokAt(-2), "[{};] switch ("))
                        tok = tok->link()->tokAt(-2);       //remove also 'switch ( ... )'
                    Token::eraseTokens(tok, end2->next());
                    checklabel = false;
                    tokcheck = 0;
                    indentcheck = 0;
                } else {
                    tok = tok->next();
                }
            }
            if (indentswitch && indentlevel <= indentcase) {
                --indentswitch;
                switchindents.pop_back();
                if (!indentswitch)
                    indentcase = 0;
                else
                    indentcase = switchindents[indentswitch-1];
            }
        } else if (Token::Match(tok, "[{};:] case")) {
            const Token *tok2 = Token::findsimplematch(tok->next(), ": ;", end);
            if (!tok2) {
                tok->deleteNext();
                continue;
            }
            if (indentlevel == 1)
                break;      //it seems like the function was called inside a case-default block.
            if (indentlevel == indentcase)
                ++indentlevel;
            tok2 = tok2->next();
            if (!checklabel || !indentswitch) {
                Token::eraseTokens(tok, tok2->next());
            } else {
                tok = const_cast<Token *>(tok2);
            }
        }  else if (Token::Match(tok, "[{};] default : ;")) {
            if (indentlevel == 1)
                break;      //it seems like the function was called inside a case-default block.
            if (indentlevel == indentcase)
                ++indentlevel;
            if (!checklabel || !indentswitch) {
                tok->deleteNext(3);
            } else {
                tok = tok->tokAt(3);
            }
        } else if (Token::Match(tok, "[{};] %name% : ;") && tok->next()->str() != "default") {
            if (checklabel) {
                indentlabel = indentlevel;
                tok = tokcheck->next();
                checklabel = false;
                indentlevel = indentcheck;
            } else {
                if (indentswitch) {
                    //Before stopping the function, since the 'switch()'
                    //instruction is removed, there's no sense to keep the
                    //case instructions. Remove them, if there are any.
                    Token *tok2 = tok->tokAt(3);
                    unsigned int indentlevel2 = indentlevel;
                    while (tok2->next() && tok2->next() != end) {
                        if (Token::Match(tok2->next(), "{|[|(")) {
                            tok2 = tok2->next()->link();
                        } else if (Token::Match(tok2, "[{};:] case")) {
                            const Token *tok3 = Token::findsimplematch(tok2->next(), ": ;", end);
                            if (!tok3) {
                                tok2 = tok2->next();
                                continue;
                            }
                            Token::eraseTokens(tok2, tok3->next());
                        } else if (Token::Match(tok2, "[{};] default : ;")) {
                            tok2->deleteNext(3);
                        } else if (tok2->next()->str() == "}") {
                            --indentlevel2;
                            if (indentlevel2 <= indentcase)
                                break;
                            tok2 = tok2->next();
                        } else {
                            tok2 = tok2->next();
                        }
                    }
                }
                break;  //stop removing tokens, we arrived to the label.
            }
        } else if (isgoto && Token::Match(tok, "[{};] do|while|for|BOOST_FOREACH")) {
            //it's possible that code inside loop is not dead,
            //because of the possible presence of the label pointed by 'goto'
            const Token *start = tok->tokAt(2);
            if (start && start->str() == "(")
                start = start->link()->next();
            if (start && start->str() == "{") {
                std::string labelpattern = "[{};] " + begin->previous()->str() + " : ;";
                bool simplify = true;
                for (Token *tok2 = start->next(); tok2 != start->link(); tok2 = tok2->next()) {
                    if (Token::Match(tok2, labelpattern.c_str())) {
                        simplify = false;
                        break;
                    }
                }
                //bailout for now
                if (!simplify)
                    break;
            }
            tok->deleteNext();
        } else {
            // no need to keep the other strings, remove them.
            if (tok->strAt(1) == "while") {
                if (tok->str() == "}" && tok->link()->strAt(-1) == "do")
                    tok->link()->previous()->deleteThis();
            }
            tok->deleteNext();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::syntaxError(const Token *tok) const
{
    printDebugOutput(0);
    throw InternalError(tok, "syntax error", InternalError::SYNTAX);
}

void Tokenizer::syntaxError(const Token *tok, char c) const
{
    printDebugOutput(0);
    if (_configuration.empty())
        throw InternalError(tok,
                            std::string("Invalid number of character '") + c + "' when no macros are defined.",
                            InternalError::SYNTAX);
    else
        throw InternalError(tok,
                            std::string("Invalid number of character '") + c + "' when these macros are defined: '" + _configuration + "'.",
                            InternalError::SYNTAX);
}

void Tokenizer::unhandled_macro_class_x_y(const Token *tok) const
{
    reportError(tok,
                Severity::information,
                "class_X_Y",
                "The code '" +
                tok->str() + " " +
                tok->strAt(1) + " " +
                tok->strAt(2) + " " +
                tok->strAt(3) + "' is not handled. You can use -I or --include to add handling of this code.");
}

void Tokenizer::cppcheckError(const Token *tok) const
{
    printDebugOutput(0);
    throw InternalError(tok, "Analysis failed. If the code is valid then please report this failure.", InternalError::INTERNAL);
}
/**
 * Helper function to check whether number is equal to integer constant X
 * or floating point pattern X.0
 * @param s the string to check
 * @param intConstant the integer constant to check against
 * @param floatConstant the string with stringified float constant to check against
 * @return true in case s is equal to X or X.0 and false otherwise.
 */
static bool isNumberOneOf(const std::string &s, const MathLib::bigint& intConstant, const char* floatConstant)
{
    if (MathLib::isInt(s)) {
        if (MathLib::toLongNumber(s) == intConstant)
            return true;
    } else if (MathLib::isFloat(s)) {
        if (MathLib::toString(MathLib::toDoubleNumber(s)) == floatConstant)
            return true;
    }
    return false;
}

// ------------------------------------------------------------------------
// Helper function to check whether number is zero (0 or 0.0 or 0E+0) or not?
// @param s the string to check
// @return true in case s is zero and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isZeroNumber(const std::string &s)
{
    return isNumberOneOf(s, 0L, "0.0");
}

// ------------------------------------------------------------------------
// Helper function to check whether number is one (1 or 0.1E+1 or 1E+0) or not?
// @param s the string to check
// @return true in case s is one and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isOneNumber(const std::string &s)
{
    if (!MathLib::isPositive(s))
        return false;
    return isNumberOneOf(s, 1L, "1.0");
}

// ------------------------------------------------------------------------
// Helper function to check whether number is two (2 or 0.2E+1 or 2E+0) or not?
// @param s the string to check
// @return true in case s is two and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isTwoNumber(const std::string &s)
{
    if (!MathLib::isPositive(s))
        return false;
    return isNumberOneOf(s, 2L, "2.0");
}

// ------------------------------------------------------
// Simplify math functions.
// It simplifies following functions: atol(), abs(), fabs()
// labs(), llabs(), fmin(), fminl(), fminf(), fmax(), fmaxl()
// fmaxf(), isgreater(), isgreaterequal(), isless()
// islessgreater(), islessequal(), pow(), powf(), powl(),
// div(),ldiv(),lldiv(), cbrt(), cbrtl(), cbtrf(), sqrt(),
// sqrtf(), sqrtl(), exp(), expf(), expl(), exp2(),
// exp2f(), exp2l(), log2(), log2f(), log2l(), log1p(),
// log1pf(), log1pl(), log10(), log10l(), log10f(),
// log(),logf(),logl(),logb(),logbf(),logbl(), acosh()
// acoshf(), acoshl(), acos(), acosf(), acosl(), cosh()
// coshf(), coshf(), cos(), cosf(), cosl(), erfc(),
// erfcf(), erfcl(), ilogb(), ilogbf(), ilogbf(), erf(),
// erfl(), erff(), asin(), asinf(), asinf(), asinh(),
// asinhf(), asinhl(), tan(), tanf(), tanl(), tanh(),
// tanhf(), tanhl(), atan(), atanf(), atanl(), atanh(),
// atanhf(), atanhl(), expm1(), expm1l(), expm1f(), fma()
// in the tokenlist.
//
// Reference:
// - http://www.cplusplus.com/reference/cmath/
// ------------------------------------------------------
void Tokenizer::simplifyMathFunctions()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isName() && !tok->varId() && tok->strAt(1) == "(") { // precondition for function
            bool simplifcationMade = false;
            if (Token::Match(tok, "atol ( %str% )")) { //@todo Add support for atoll()
                if (Token::simpleMatch(tok->tokAt(-2), "std ::")) {
                    tok = tok->tokAt(-2);// set token index two steps back
                    tok->deleteNext(2);  // delete "std ::"
                }
                const std::string& strNumber = tok->tokAt(2)->strValue(); // get number
                const bool isNotAnInteger = (!MathLib::isInt(strNumber));// check: is not an integer
                if (strNumber.empty() || isNotAnInteger) {
                    // Ignore strings which we can't convert
                    continue;
                }
                // Convert string into a number and insert into token list
                tok->str(MathLib::toString(MathLib::toLongNumber(strNumber)));
                // remove ( %num% )
                tok->deleteNext(3);
                simplifcationMade = true;
            } else if (Token::Match(tok, "sqrt|sqrtf|sqrtl|cbrt|cbrtf|cbrtl ( %num% )")) {
                // Simplify: sqrt(0) = 0 and cbrt(0) == 0
                //           sqrt(1) = 1 and cbrt(1) == 1
                // get number string
                const std::string& parameter(tok->strAt(2));
                // is parameter 0 ?
                if (isZeroNumber(parameter)) {
                    tok->deleteNext(3);  // delete tokens
                    tok->str("0"); // insert result into token list
                    simplifcationMade = true;
                } else if (isOneNumber(parameter)) {
                    tok->deleteNext(3);  // delete tokens
                    tok->str("1"); // insert result into token list
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "exp|expf|expl|exp2|exp2f|exp2l|cos|cosf|cosl|cosh|coshf|coshl|erfc|erfcf|erfcl ( %num% )")) {
                // Simplify: exp[f|l](0)  = 1 and exp2[f|l](0) = 1
                //           cosh[f|l](0) = 1 and cos[f|l](0)  = 1
                //           erfc[f|l](0) = 1
                // get number string
                const std::string& parameter(tok->strAt(2));
                // is parameter 0 ?
                if (isZeroNumber(parameter)) {
                    tok->deleteNext(3);  // delete tokens
                    tok->str("1"); // insert result into token list
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "log1p|log1pf|log1pl|sin|sinf|sinl|sinh|sinhf|sinhl|erf|erff|erfl|asin|asinf|asinl|asinh|asinhf|asinhl|tan|tanf|tanl|tanh|tanhf|tanhl|atan|atanf|atanl|atanh|atanhf|atanhl|expm1|expm1f|expm1l ( %num% )")) {
                // Simplify: log1p[f|l](0) = 0 and sin[f|l](0)  = 0
                //           sinh[f|l](0)  = 0 and erf[f|l](0)  = 0
                //           asin[f|l](0)  = 0 and sinh[f|l](0) = 0
                //           tan[f|l](0)   = 0 and tanh[f|l](0) = 0
                //           atan[f|l](0)  = 0 and atanh[f|l](0)= 0
                //           expm1[f|l](0) = 0
                // get number string
                const std::string& parameter(tok->strAt(2));
                // is parameter 0 ?
                if (isZeroNumber(parameter)) {
                    tok->deleteNext(3);  // delete tokens
                    tok->str("0"); // insert result into token list
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "log2|log2f|log2l|log|logf|logl|log10|log10f|log10l|logb|logbf|logbl|acosh|acoshf|acoshl|acos|acosf|acosl|ilogb|ilogbf|ilogbl ( %num% )")) {
                // Simplify: log2[f|l](1)  = 0 , log10[f|l](1)  = 0
                //           log[f|l](1)   = 0 , logb10[f|l](1) = 0
                //           acosh[f|l](1) = 0 , acos[f|l](1)   = 0
                //           ilogb[f|l](1) = 0
                // get number string
                const std::string& parameter(tok->strAt(2));
                // is parameter 1 ?
                if (isOneNumber(parameter)) {
                    tok->deleteNext(3);  // delete tokens
                    tok->str("0"); // insert result into token list
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "fmin|fminl|fminf ( %num% , %num% )")) {
                // @todo if one of the parameters is NaN the other is returned
                // e.g. printf ("fmin (NaN, -1.0) = %f\n", fmin(NaN,-1.0));
                // e.g. printf ("fmin (-1.0, NaN) = %f\n", fmin(-1.0,NaN));
                const std::string& strLeftNumber(tok->strAt(2));
                const std::string& strRightNumber(tok->strAt(4));
                const bool isLessEqual =  MathLib::isLessEqual(strLeftNumber, strRightNumber);
                // case: left <= right ==> insert left
                if (isLessEqual) {
                    tok->str(strLeftNumber); // insert e.g. -1.0
                    tok->deleteNext(5);      // delete e.g. fmin ( -1.0, 1.0 )
                    simplifcationMade = true;
                } else { // case left > right ==> insert right
                    tok->str(strRightNumber); // insert e.g. 0.0
                    tok->deleteNext(5);       // delete e.g. fmin ( 1.0, 0.0 )
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "fmax|fmaxl|fmaxf ( %num% , %num% )")) {
                // @todo if one of the parameters is NaN the other is returned
                // e.g. printf ("fmax (NaN, -1.0) = %f\n", fmax(NaN,-1.0));
                // e.g. printf ("fmax (-1.0, NaN) = %f\n", fmax(-1.0,NaN));
                const std::string& strLeftNumber(tok->strAt(2));
                const std::string& strRightNumber(tok->strAt(4));
                const bool isLessEqual =  MathLib::isLessEqual(strLeftNumber, strRightNumber);
                // case: left <= right ==> insert right
                if (isLessEqual) {
                    tok->str(strRightNumber);// insert e.g. 1.0
                    tok->deleteNext(5);      // delete e.g. fmax ( -1.0, 1.0 )
                    simplifcationMade = true;
                } else { // case left > right ==> insert left
                    tok->str(strLeftNumber);  // insert e.g. 1.0
                    tok->deleteNext(5);       // delete e.g. fmax ( 1.0, 0.0 )
                    simplifcationMade = true;
                }
            } else if (Token::Match(tok, "pow|powf|powl (")) {
                if (Token::Match(tok->tokAt(2), "%num% , %num% )")) {
                    // In case of pow ( 0 , anyNumber > 0): It can be simplified to 0
                    // In case of pow ( 0 , 0 ): It simplified to 1
                    // In case of pow ( 1 , anyNumber ): It simplified to 1
                    const std::string& leftNumber(tok->strAt(2)); // get the left parameter
                    const std::string& rightNumber(tok->strAt(4)); // get the right parameter
                    const bool isLeftNumberZero = isZeroNumber(leftNumber);
                    const bool isLeftNumberOne = isOneNumber(leftNumber);
                    const bool isRightNumberZero = isZeroNumber(rightNumber);
                    if (isLeftNumberZero && !isRightNumberZero && MathLib::isPositive(rightNumber)) { // case: 0^(y) = 0 and y > 0
                        tok->deleteNext(5); // delete tokens
                        tok->str("0");  // insert simplified result
                        simplifcationMade = true;
                    } else if (isLeftNumberZero && isRightNumberZero) { // case: 0^0 = 1
                        tok->deleteNext(5); // delete tokens
                        tok->str("1");  // insert simplified result
                        simplifcationMade = true;
                    } else if (isLeftNumberOne) { // case 1^(y) = 1
                        tok->deleteNext(5); // delete tokens
                        tok->str("1");  // insert simplified result
                        simplifcationMade = true;
                    }
                }
                if (Token::Match(tok->tokAt(2), "%any% , %num% )")) {
                    // In case of pow( x , 1 ): It can be simplified to x.
                    const std::string& leftParameter(tok->strAt(2)); // get the left parameter
                    const std::string& rightNumber(tok->strAt(4)); // get right number
                    if (isOneNumber(rightNumber)) { // case: x^(1) = x
                        tok->str(leftParameter);  // insert simplified result
                        tok->deleteNext(5); // delete tokens
                        simplifcationMade = true;
                    } else if (isZeroNumber(rightNumber)) { // case: x^(0) = 1
                        tok->deleteNext(5); // delete tokens
                        tok->str("1");  // insert simplified result
                        simplifcationMade = true;
                    }
                }
            }
            // Jump back to begin of statement if a simplification was performed
            if (simplifcationMade) {
                while (tok->previous() && tok->str() != ";") {
                    tok = tok->previous();
                }
            }
        }
    }
}

void Tokenizer::simplifyComma()
{
    bool inReturn = false;

    for (Token *tok = list.front(); tok; tok = tok->next()) {

        // skip enums
        if (Token::Match(tok, "enum class| %name%| :| %name%| {")) {
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (tok)
                tok = tok->link()->next();
        }

        if (!tok)
            syntaxError(nullptr); // invalid code like in #4195
        if (Token::Match(tok, "(|[") ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "=")) {
            tok = tok->link();
            continue;
        }

        // Skip unhandled template specifiers..
        if (tok->link() && tok->str() == "<")
            tok = tok->link();

        if (tok->str() == "return" && Token::Match(tok->previous(), "[;{}]"))
            inReturn = true;

        if (inReturn && Token::Match(tok, "[;{}?:]"))
            inReturn = false;

        if (!tok->next() || tok->str() != ",")
            continue;

        // We must not accept just any keyword, e.g. accepting int
        // would cause function parameters to corrupt.
        if (isCPP() && tok->strAt(1) == "delete") {
            // Handle "delete a, delete b;"
            tok->str(";");
        }

        if (isCPP() && Token::Match(tok->tokAt(-2), "delete %name% , %name% ;") &&
            tok->next()->varId() != 0) {
            // Handle "delete a, b;" - convert to delete a; b;
            tok->str(";");
        } else if (!inReturn && tok->tokAt(-2)) {
            bool replace = false;
            for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous()) {
                if (tok2->str() == "=") {
                    // Handle "a = 0, b = 0;"
                    replace = true;
                } else if (isCPP() && (Token::Match(tok2, "delete %name%") ||
                                       Token::Match(tok2, "delete [ ] %name%"))) {
                    // Handle "delete a, a = 0;"
                    replace = true;
                } else if (Token::Match(tok2, "[?:;,{}()]")) {
                    if (replace && Token::Match(tok2, "[;{}]"))
                        tok->str(";");
                    break;
                }
            }
        }

        // find token where return ends and also count commas
        if (inReturn) {
            Token *startFrom = nullptr;    // "[;{}]" token before "return"
            Token *endAt = nullptr;        // first ";" token after "[;{}] return"

            // find "; return" pattern before comma
            for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous()) {
                if (tok2->str() == "return") {
                    startFrom = tok2->previous();
                    break;
                }
            }
            if (!startFrom)
                // to be very sure...
                return;
            std::size_t commaCounter = 0;
            for (Token *tok2 = startFrom->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    endAt = tok2;
                    break;

                } else if (Token::Match(tok2, "(|[") ||
                           (tok2->str() == "{" && tok2->previous() && tok2->previous()->str() == "=")) {
                    tok2 = tok2->link();

                } else if (tok2->str() == ",") {
                    ++commaCounter;
                }
            }

            if (!endAt)
                //probably a syntax error
                return;

            if (commaCounter) {
                // change tokens:
                // "; return a ( ) , b ( ) , c ;"
                // to
                // "; a ( ) ; b ( ) ; return c ;"

                // remove "return"
                startFrom->deleteNext();
                for (Token *tok2 = startFrom->next(); tok2 != endAt; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "(|[") ||
                        (tok2->str() == "{" && tok2->previous() && tok2->previous()->str() == "=")) {
                        tok2 = tok2->link();

                    } else if (tok2->str() == ",") {
                        tok2->str(";");
                        --commaCounter;
                        if (commaCounter == 0) {
                            tok2->insertToken("return");
                        }
                    }
                }
                tok = endAt;
            }
        }
    }
}


void Tokenizer::validate() const
{
    std::stack<const Token *> linkTokens;
    const Token *lastTok = nullptr;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        lastTok = tok;
        if (Token::Match(tok, "[{([]") || (tok->str() == "<" && tok->link())) {
            if (tok->link() == nullptr)
                cppcheckError(tok);

            linkTokens.push(tok);
        }

        else if (Token::Match(tok, "[})]]") || (tok->str() == ">" && tok->link())) {
            if (tok->link() == nullptr)
                cppcheckError(tok);

            if (linkTokens.empty() == true)
                cppcheckError(tok);

            if (tok->link() != linkTokens.top())
                cppcheckError(tok);

            if (tok != tok->link()->link())
                cppcheckError(tok);

            linkTokens.pop();
        }

        else if (tok->link() != nullptr)
            cppcheckError(tok);
    }

    if (!linkTokens.empty())
        cppcheckError(linkTokens.top());

    // Validate that the Tokenizer::list.back() is updated correctly during simplifications
    if (lastTok != list.back())
        cppcheckError(lastTok);
}


const Token * Tokenizer::findGarbageCode() const
{
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if|while|for|switch")) { // if|while|for|switch (EXPR) { ... }
            if (tok->previous() && !Token::Match(tok->previous(), "%name%|:|;|{|}|(|)|,"))
                return tok;
            if (Token::Match(tok->previous(), "[(,]"))
                continue;
            if (!Token::Match(tok->next(), "( !!)"))
                return tok;
            if (tok->str() != "for") {
                if (isGarbageExpr(tok->next(), tok->linkAt(1)))
                    return tok;
            }
        }
    }

    // Code must not start with an arithmetical operand
    if (Token::Match(list.front(), "%cop%"))
        return list.front();

    // Code must end with } ; ) NAME
    if (!Token::Match(list.back(), "%name%|;|}|)"))
        return list.back();
    if (list.back()->str() == ")" && !Token::Match(list.back()->link()->previous(), "%name% ("))
        return list.back();
    if (Token::Match(list.back(), "void|char|short|int|long|float|double|const|volatile|static|inline|struct|class|enum|union|template|sizeof|break|continue|typedef"))
        return list.back();

    return nullptr;
}

bool Tokenizer::isGarbageExpr(const Token *start, const Token *end)
{
    std::set<std::string> controlFlowKeywords;
    controlFlowKeywords.insert("goto");
    controlFlowKeywords.insert("do");
    controlFlowKeywords.insert("if");
    controlFlowKeywords.insert("else");
    controlFlowKeywords.insert("for");
    controlFlowKeywords.insert("while");
    controlFlowKeywords.insert("switch");
    controlFlowKeywords.insert("break");
    controlFlowKeywords.insert("continue");
    controlFlowKeywords.insert("return");
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (controlFlowKeywords.find(tok->str()) != controlFlowKeywords.end())
            return true;
        if (tok->str() == ";")
            return true;
        if (tok->str() == "{")
            tok = tok->link();
    }
    return false;
}

std::string Tokenizer::simplifyString(const std::string &source)
{
    std::string str = source;

    for (std::string::size_type i = 0; i + 1U < str.size(); ++i) {
        if (str[i] != '\\')
            continue;

        int c = 'a';   // char
        unsigned int sz = 0;    // size of stringdata
        if (str[i+1] == 'x') {
            sz = 2;
            while (sz < 4 && std::isxdigit((unsigned char)str[i+sz]))
                sz++;
            if (sz > 2) {
                std::istringstream istr(str.substr(i+2, sz-2));
                istr >> std::hex >> c;
            }
        } else if (MathLib::isOctalDigit(str[i+1])) {
            sz = 2;
            while (sz < 4 && MathLib::isOctalDigit(str[i+sz]))
                sz++;
            std::istringstream istr(str.substr(i+1, sz-1));
            istr >> std::oct >> c;
            str = str.substr(0,i) + (char)c + str.substr(i+sz);
            continue;
        }

        if (sz <= 2)
            i++;
        else if (i+sz < str.size())
            str.replace(i, sz, std::string(1U, (char)c));
        else
            str.replace(i, str.size() - i - 1U, "a");
    }

    return str;
}

void Tokenizer::simplifyWhile0()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // while (0)
        const bool while0(Token::Match(tok->previous(), "[{};] while ( 0|false )"));

        // for (0) - not banal, ticket #3140
        const bool for0((Token::Match(tok->previous(), "[{};] for ( %name% = %num% ; %name% < %num% ;") &&
                         tok->strAt(2) == tok->strAt(6) && tok->strAt(4) == tok->strAt(8)) ||
                        (Token::Match(tok->previous(), "[{};] for ( %type% %name% = %num% ; %name% < %num% ;") &&
                         tok->strAt(3) == tok->strAt(7) && tok->strAt(5) == tok->strAt(9)));

        if (!while0 && !for0)
            continue;

        if (while0 && tok->previous()->str() == "}") {
            // find "do"
            Token *tok2 = tok->previous()->link();
            tok2 = tok2->previous();
            if (tok2 && tok2->str() == "do") {
                const bool flowmatch = Token::findmatch(tok2, "continue|break", tok) != nullptr;
                // delete "do ({)"
                tok2->deleteThis();
                if (!flowmatch)
                    tok2->deleteThis();

                // delete "(}) while ( 0 ) (;)"
                tok = tok->previous();
                tok->deleteNext(4);  // while ( 0 )
                if (tok->next() && tok->next()->str() == ";")
                    tok->deleteNext(); // ;
                if (!flowmatch)
                    tok->deleteThis(); // }

                continue;
            }
        }

        // remove "while (0) { .. }"
        if (Token::simpleMatch(tok->next()->link(), ") {")) {
            Token *end = tok->next()->link(), *old_prev = tok->previous();
            end = end->next()->link();
            eraseDeadCode(old_prev, end->next());
            if (old_prev && old_prev->next())
                tok = old_prev->next();
            else
                break;
        }
    }
}

void Tokenizer::simplifyErrNoInWhile()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "errno")
            continue;

        Token *endpar = nullptr;
        if (Token::Match(tok->previous(), "&& errno == EINTR ) { ;| }"))
            endpar = tok->tokAt(3);
        else if (Token::Match(tok->tokAt(-2), "&& ( errno == EINTR ) ) { ;| }"))
            endpar = tok->tokAt(4);
        else
            continue;

        if (Token::simpleMatch(endpar->link()->previous(), "while (")) {
            Token *tok1 = tok->previous();
            if (tok1->str() == "(")
                tok1 = tok1->previous();

            // erase "&& errno == EINTR"
            tok1 = tok1->previous();
            Token::eraseTokens(tok1, endpar);

            // tok is invalid.. move to endpar
            tok = endpar;
        }
    }
}


void Tokenizer::simplifyFuncInWhile()
{
    unsigned int count = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "while ( %name% ( %name% ) ) {"))
            continue;

        Token *func = tok->tokAt(2);
        Token *var = tok->tokAt(4);
        Token *end = tok->next()->link()->next()->link();

        const unsigned int varid = ++_varId; // Create new variable
        const std::string varname("cppcheck:r" + MathLib::toString(++count));
        tok->str("int");
        tok->next()->insertToken(varname);
        tok->tokAt(2)->varId(varid);
        tok->insertToken("while");
        tok->insertToken(";");
        tok->insertToken(")");
        tok->insertToken(var->str());
        tok->next()->varId(var->varId());
        tok->insertToken("(");
        tok->insertToken(func->str());
        tok->insertToken("=");
        tok->insertToken(varname);
        tok->next()->varId(varid);
        Token::createMutualLinks(tok->tokAt(4), tok->tokAt(6));
        end->previous()->insertToken(varname);
        end->previous()->varId(varid);
        end->previous()->insertToken("=");
        Token::move(func, func->tokAt(3), end->previous());
        end->previous()->insertToken(";");

        tok = end;
    }
}

void Tokenizer::simplifyStructDecl()
{
    // A counter that is used when giving unique names for anonymous structs.
    unsigned int count = 0;

    // Skip simplification of unions in class definition
    std::stack<bool> skip; // true = in function, false = not in function
    skip.push(false);

    // Add names for anonymous structs
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName())
            continue;
        // check for anonymous struct/union
        if (Token::Match(tok, "struct|union {")) {
            if (Token::Match(tok->next()->link(), "} *|&| %type% ,|;|[|(|{")) {
                tok->insertToken("Anonymous" + MathLib::toString(count++));
            }
        }
        // check for anonymous enum
        else if ((Token::simpleMatch(tok, "enum {") && Token::Match(tok->next()->link(), "} %type%| ,|;|[|(|{")) ||
                 (Token::Match(tok, "enum : %type% {") && Token::Match(tok->linkAt(3), "} %type%| ,|;|[|(|{"))) {
            tok->insertToken("Anonymous" + MathLib::toString(count++));
        }
    }

    for (Token *tok = list.front(); tok; tok = tok->next()) {

        // check for start of scope and determine if it is in a function
        if (tok->str() == "{")
            skip.push(Token::Match(tok->previous(), "const|)"));

        // end of scope
        else if (tok->str() == "}" && !skip.empty())
            skip.pop();

        // check for named struct/union
        else if (Token::Match(tok, "class|struct|union|enum %type% :|{")) {
            Token *start = tok;
            while (Token::Match(start->previous(), "%type%"))
                start = start->previous();
            Token *type = tok->next();
            Token *next = tok->tokAt(2);

            while (next && next->str() != "{")
                next = next->next();
            if (!next)
                continue;
            skip.push(false);
            tok = next->link();
            if (!tok)
                break; // see #4869 segmentation fault in Tokenizer::simplifyStructDecl (invalid code)
            Token *restart = next;

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|[|=|(|{")) {
                tok->insertToken(";");
                tok = tok->next();
                while (!Token::Match(start, "struct|class|union|enum")) {
                    tok->insertToken(start->str());
                    tok = tok->next();
                    start->deleteThis();
                }
                if (!tok)
                    break; // see #4869 segmentation fault in Tokenizer::simplifyStructDecl (invalid code)
                tok->insertToken(type->str());
                if (start->str() != "class") {
                    tok->insertToken(start->str());
                    tok = tok->next();
                }

                tok = tok->tokAt(2);

                // check for initialization
                if (tok && (tok->next()->str() == "(" || tok->next()->str() == "{")) {
                    tok->insertToken("=");
                    tok = tok->next();

                    if (start->str() == "enum") {
                        if (tok->next()->str() == "{") {
                            tok->next()->str("(");
                            tok->linkAt(1)->str(")");
                        }
                    }
                }
            }

            tok = restart;
        }

        // check for anonymous struct/union
        else if (Token::Match(tok, "struct|union {")) {
            const bool inFunction = skip.top();
            skip.push(false);
            Token *tok1 = tok;

            Token *restart = tok->next();
            tok = tok->next()->link();

            // unnamed anonymous struct/union so possibly remove it
            if (tok && tok->next() && tok->next()->str() == ";") {
                if (inFunction && tok1->str() == "union") {
                    // Try to create references in the union..
                    Token *tok2 = tok1->tokAt(2);
                    while (tok2) {
                        if (Token::Match(tok2, "%type% %name% ;"))
                            tok2 = tok2->tokAt(3);
                        else
                            break;
                    }
                    if (!Token::simpleMatch(tok2, "} ;"))
                        continue;
                    Token *vartok = nullptr;
                    tok2 = tok1->tokAt(2);
                    while (Token::Match(tok2, "%type% %name% ;")) {
                        if (!vartok) {
                            vartok = tok2->next();
                            tok2 = tok2->tokAt(3);
                        } else {
                            tok2->insertToken("&");
                            tok2 = tok2->tokAt(2);
                            tok2->insertToken(vartok->str());
                            tok2->next()->varId(vartok->varId());
                            tok2->insertToken("=");
                            tok2 = tok2->tokAt(4);
                        }
                    }
                }

                // don't remove unnamed anonymous unions from a class, struct or union
                if (!(!inFunction && tok1->str() == "union")) {
                    skip.pop();
                    tok1->deleteThis();
                    if (tok1->next() == tok) {
                        tok1->deleteThis();
                        tok = tok1;
                    } else
                        tok1->deleteThis();
                    restart = tok1->previous();
                    tok->deleteThis();
                    if (tok->next())
                        tok->deleteThis();
                }
            }

            if (!restart) {
                simplifyStructDecl();
                return;
            } else if (!restart->next())
                return;

            tok = restart;
        }
    }
}

void Tokenizer::simplifyCallingConvention()
{
    bool windows = _settings->isWindowsPlatform();

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "__cdecl|__stdcall|__fastcall|__thiscall|__clrcall|__syscall|__pascal|__fortran|__far|__near") || (windows && Token::Match(tok, "WINAPI|APIENTRY|CALLBACK"))) {
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyDeclspec()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "__declspec|_declspec (") && tok->next()->link() && tok->next()->link()->next()) {
            if (Token::Match(tok->tokAt(2), "noreturn|nothrow")) {
                Token *tok1 = tok->next()->link()->next();
                while (tok1 && !Token::Match(tok1, "%name%")) {
                    tok1 = tok1->next();
                }
                if (tok1) {
                    if (tok->strAt(2) == "noreturn")
                        tok1->isAttributeNoreturn(true);
                    else
                        tok1->isAttributeNothrow(true);
                }
            } else if (tok->strAt(2) == "property")
                tok->next()->link()->insertToken("__property");

            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAttribute()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%type% (") && !_settings->library.isNotLibraryFunction(tok)) {
            if (_settings->library.isFunctionConst(tok->str(), true))
                tok->isAttributePure(true);
            if (_settings->library.isFunctionConst(tok->str(), false))
                tok->isAttributeConst(true);
        }
        while (Token::Match(tok, "__attribute__|__attribute (") && tok->next()->link() && tok->next()->link()->next()) {
            if (Token::Match(tok->tokAt(2), "( constructor|__constructor__")) {
                // prototype for constructor is: void func(void);
                if (!tok->next()->link()->next())
                    syntaxError(tok);

                if (tok->next()->link()->next()->str() == "void") { // __attribute__((constructor)) void func() {}
                    if (!tok->next()->link()->next()->next())
                        syntaxError(tok);

                    tok->next()->link()->next()->next()->isAttributeConstructor(true);
                } else if (tok->next()->link()->next()->str() == ";" && tok->linkAt(-1) && tok->previous()->link()->previous()) // void func() __attribute__((constructor));
                    tok->previous()->link()->previous()->isAttributeConstructor(true);
                else // void __attribute__((constructor)) func() {}
                    tok->next()->link()->next()->isAttributeConstructor(true);
            }

            else if (Token::Match(tok->tokAt(2), "( destructor|__destructor__")) {
                // prototype for destructor is: void func(void);
                if (!tok->next()->link()->next())
                    syntaxError(tok);

                if (tok->next()->link()->next()->str() == "void") // __attribute__((destructor)) void func() {}
                    tok->next()->link()->next()->next()->isAttributeDestructor(true);
                else if (tok->next()->link()->next()->str() == ";" && tok->linkAt(-1) && tok->previous()->link()->previous()) // void func() __attribute__((destructor));
                    tok->previous()->link()->previous()->isAttributeDestructor(true);
                else // void __attribute__((destructor)) func() {}
                    tok->next()->link()->next()->isAttributeDestructor(true);
            }

            else if (Token::Match(tok->tokAt(2), "( unused|__unused__|used|__used__ )")) {
                Token *vartok = nullptr;

                // check if after variable name
                if (Token::Match(tok->next()->link()->next(), ";|=")) {
                    if (Token::Match(tok->previous(), "%type%"))
                        vartok = tok->previous();
                }

                // check if before variable name
                else if (Token::Match(tok->next()->link()->next(), "%type%"))
                    vartok = tok->next()->link()->next();

                if (vartok) {
                    const std::string &attribute(tok->strAt(3));
                    if (attribute.find("unused") != std::string::npos)
                        vartok->isAttributeUnused(true);
                    else
                        vartok->isAttributeUsed(true);
                }
            }

            else if (Token::Match(tok->tokAt(2), "( pure|__pure__|const|__const__|noreturn|__noreturn__|nothrow|__nothrow__ )")) {
                Token *functok = nullptr;

                // type func(...) __attribute__((attribute));
                if (tok->previous() && tok->previous()->link() && Token::Match(tok->previous()->link()->previous(), "%name% ("))
                    functok = tok->previous()->link()->previous();

                // type __attribute__((attribute)) func() { }
                else {
                    Token *tok2 = tok->next()->link();
                    while (Token::Match(tok2, ") __attribute__|__attribute ("))
                        tok2 = tok2->linkAt(2);
                    if (Token::Match(tok2, ") %name% ("))
                        functok = tok2->next();
                }

                if (functok) {
                    const std::string &attribute(tok->strAt(3));
                    if (attribute.find("pure") != std::string::npos)
                        functok->isAttributePure(true);
                    else if (attribute.find("const") != std::string::npos)
                        functok->isAttributeConst(true);
                    else if (attribute.find("noreturn") != std::string::npos)
                        functok->isAttributeNoreturn(true);
                    else if (attribute.find("nothrow") != std::string::npos)
                        functok->isAttributeNothrow(true);
                }
            }

            else if (Token::simpleMatch(tok->previous(), "} __attribute__ ( ( packed )")) {
                tok->previous()->isAttributePacked(true);
            }

            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

namespace {
    const std::set<std::string> keywords = make_container< std::set<std::string> >()
                                           << "volatile"
                                           << "inline"
                                           << "_inline"
                                           << "__inline"
                                           << "__forceinline"
                                           << "register"
                                           << "__restrict"
                                           << "__restrict__" ;
}
// Remove "volatile", "inline", "register", "restrict", "override", "final", "static" and "constexpr"
// "restrict" keyword
//   - New to 1999 ANSI/ISO C standard
//   - Not in C++ standard yet
void Tokenizer::simplifyKeyword()
{

    // FIXME: There is a risk that "keywords" are removed by mistake. This
    // code should be fixed so it doesn't remove variables etc. Nonstandard
    // keywords should be defined with a library instead. For instance the
    // linux kernel code at least uses "_inline" as struct member name at some
    // places.
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (keywords.find(tok->str()) == keywords.end())
            continue;

        // Don't remove struct members
        if (Token::simpleMatch(tok->previous(), "."))
            continue;

        // Simplify..
        tok->deleteThis();
    }

    if (isC() || _settings->standards.cpp == Standards::CPP03) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (tok->str() == "auto")
                tok->deleteThis();
        }
    }

    if (_settings->standards.c >= Standards::C99) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            while (tok->str() == "restrict") {
                tok->deleteThis();
            }

            // simplify static keyword:
            // void foo( int [ static 5 ] ); ==> void foo( int [ 5 ] );
            if (Token::Match(tok, "[ static %num%")) {
                tok->deleteNext();
            }
        }
    }

    if (_settings->standards.c >= Standards::C11) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            while (tok->str() == "_Atomic") {
                tok->deleteThis();
            }
        }
    }

    if (isCPP() && _settings->standards.cpp >= Standards::CPP11) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            while (tok->str() == "constexpr") {
                tok->deleteThis();
            }

            // final:
            // 1) struct name final { };   <- struct is final
            if (Token::Match(tok, "%type% final [:{]")) {
                tok->deleteNext();
                continue;
            }
            // final:
            // 2) void f() final;  <- function is final
            if (Token::Match(tok, ") const|override|final")) {
                Token* specifier = tok->tokAt(2);
                while (Token::Match(specifier, "const|override|final")) {
                    specifier=specifier->next();
                }
                if (Token::Match(specifier, "[{;]")) {
                    specifier = tok->next();
                    while (!Token::Match(specifier, "[{;]")) {
                        if (specifier->str()=="final")
                            specifier->deleteThis();
                        else
                            specifier=specifier->next();
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyAssignmentInFunctionCall()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(")
            tok = tok->link();

        // Find 'foo(var='. Exclude 'assert(var=' to allow tests to check that assert(...) does not contain side-effects
        else if (Token::Match(tok, "[;{}] %name% ( %name% =") &&
                 Token::simpleMatch(tok->linkAt(2), ") ;") &&
                 !Token::Match(tok->next(), "assert|while")) {
            const std::string& funcname(tok->next()->str());
            Token* const vartok = tok->tokAt(3);

            // Goto ',' or ')'..
            for (Token *tok2 = vartok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->link() && Token::Match(tok2, "(|[|{"))
                    tok2 = tok2->link();
                else if (tok2->str() == ";")
                    break;
                else if (Token::Match(tok2, ")|,")) {
                    tok2 = tok2->previous();

                    tok2->insertToken(vartok->str());
                    tok2->next()->varId(vartok->varId());

                    tok2->insertToken("(");
                    Token::createMutualLinks(tok2->next(), tok->linkAt(2));

                    tok2->insertToken(funcname);
                    tok2->insertToken(";");

                    Token::eraseTokens(tok, vartok);
                    break;
                }
            }
        }
    }
}

void Tokenizer::simplifyAssignmentBlock()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %name% = ( {")) {
            const std::string &varname = tok->next()->str();

            // goto the "} )"
            unsigned int indentlevel = 0;
            Token *tok2 = tok;
            while (nullptr != (tok2 = tok2->next())) {
                if (Token::Match(tok2, "(|{"))
                    ++indentlevel;
                else if (Token::Match(tok2, ")|}")) {
                    if (indentlevel <= 2)
                        break;
                    --indentlevel;
                } else if (indentlevel == 2 && tok2->str() == varname && Token::Match(tok2->previous(), "%type%|*"))
                    // declaring variable in inner scope with same name as lhs variable
                    break;
            }
            if (indentlevel == 2 && Token::simpleMatch(tok2, "} )")) {
                tok2 = tok2->tokAt(-3);
                if (Token::Match(tok2, "[;{}] %num%|%name% ;")) {
                    tok2->insertToken("=");
                    tok2->insertToken(tok->next()->str());
                    tok2->next()->varId(tok->next()->varId());
                    tok->deleteNext(3);
                    tok2->tokAt(5)->deleteNext();
                }
            }
        }
    }
}

// Remove __asm..
void Tokenizer::simplifyAsm()
{
    std::string instruction;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "__asm|_asm|asm {") &&
            tok->next()->link()->next()) {
            instruction = tok->tokAt(2)->stringifyList(tok->next()->link());
            Token::eraseTokens(tok, tok->next()->link()->next());
        }

        else if (Token::Match(tok, "asm|__asm|__asm__ volatile|__volatile|__volatile__| (")) {
            // Goto "("
            Token *partok = tok->next();
            if (partok->str() != "(")
                partok = partok->next();
            instruction = partok->next()->stringifyList(partok->link());
            Token::eraseTokens(tok, partok->link()->next());
        }

        else if (Token::Match(tok, "_asm|__asm")) {
            const Token *tok2 = tok;
            while (tok2 && (tok2->isNumber() || tok2->isName() || tok2->str() == "," || tok2->str() == ":"))
                tok2 = tok2->next();
            if (!tok2 || tok2->str() == ";" || tok2->linenr() != tok->linenr()) {
                instruction = tok->next()->stringifyList(tok2);
                Token::eraseTokens(tok, tok2);
                if (!tok2 || tok2->str() != ";")
                    tok->insertToken(";");
            } else
                continue;
        }

        else
            continue;

        // insert "asm ( "instruction" )"
        tok->str("asm");
        if (tok->strAt(1) != ";" && tok->strAt(1) != "{")
            tok->insertToken(";");
        tok->insertToken(")");
        tok->insertToken("\"" + instruction + "\"");
        tok->insertToken("(");

        tok = tok->next();
        Token::createMutualLinks(tok, tok->tokAt(2));

        //move the new tokens in the same line as ";" if available
        tok = tok->tokAt(2);
        if (tok->next() && tok->next()->str() == ";" &&
            tok->next()->linenr() != tok->linenr()) {
            const unsigned int endposition = tok->next()->linenr();
            tok = tok->tokAt(-3);
            for (int i = 0; i < 4; ++i) {
                tok = tok->next();
                tok->linenr(endposition);
            }
        }
    }
}

void Tokenizer::simplifyAsm2()
{
    // Block declarations: ^{}
    // A C extension used to create lambda like closures.

    // Put ^{} statements in asm()
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "^")
            continue;

        if (Token::simpleMatch(tok, "^ {") || Token::simpleMatch(tok->linkAt(1), ") {")) {
            Token * start = tok;
            while (start && !Token::Match(start, "[,(;{}=]")) {
                if (start->link() && Token::Match(start, ")|]|>"))
                    start = start->link();
                start = start->previous();
            }

            const Token *last = tok->next()->link();
            if (Token::simpleMatch(last, ") {"))
                last = last->linkAt(1);
            last = last->next();
            while (last && !Token::Match(last, "%cop%|,|;|{|}|)")) {
                if (Token::Match(last, "(|["))
                    last = last->link();
                last = last->next();
            }

            if (start && last) {
                std::string asmcode;
                while (start->next() != last) {
                    asmcode += start->next()->str();
                    start->deleteNext();
                }
                if (last->str() == "}")
                    start->insertToken(";");
                start->insertToken(")");
                start->insertToken("\"" + asmcode + "\"");
                start->insertToken("(");
                start->insertToken("asm");
                start->tokAt(2)->link(start->tokAt(4));
                start->tokAt(4)->link(start->tokAt(2));
                tok = start->tokAt(4);
            }
        }
    }

    // When the assembly code has been cleaned up, no @ is allowed
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (!tok)
                syntaxError(nullptr);
        } else if (tok->str()[0] == '@') {
            syntaxError(nullptr);
        }
    }
}

// Simplify bitfields
void Tokenizer::simplifyBitfields()
{
    bool goback = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (goback) {
            goback = false;
            tok = tok->previous();
        }
        Token *last = nullptr;

        if (!Token::Match(tok, ";|{|}|public:|protected:|private:"))
            continue;

        bool isEnum = false;
        if (tok->str() == "}") {
            const Token *type = tok->link()->previous();
            while (type && type->isName()) {
                if (type->str() == "enum") {
                    isEnum = true;
                    break;
                }
                type = type->previous();
            }
        }

        if (Token::Match(tok->next(), "const| %type% %name% :") &&
            !Token::Match(tok->next(), "case|public|protected|private|class|struct") &&
            !Token::simpleMatch(tok->tokAt(2), "default :")) {
            Token *tok1 = (tok->next()->str() == "const") ? tok->tokAt(3) : tok->tokAt(2);
            if (tok1 && tok1->tokAt(2) &&
                (Token::Match(tok1->tokAt(2), "%bool%|%num%") ||
                 !Token::Match(tok1->tokAt(2), "public|protected|private| %type% ::|<|,|{|;"))) {
                while (tok1->next() && !Token::Match(tok1->next(), "[;,)]{}]")) {
                    if (Token::Match(tok1->next(), "[([]"))
                        Token::eraseTokens(tok1, tok1->next()->link());
                    tok1->deleteNext();
                }

                last = tok1->next();
            }
        } else if (isEnum && Token::Match(tok, "} %name%| : %num% ;")) {
            if (tok->next()->str() == ":") {
                tok->deleteNext(2);
                tok->insertToken("Anonymous");
            } else {
                tok->next()->deleteNext(2);
            }
        } else if (Token::Match(tok->next(), "const| %type% : %num%|%bool% ;") &&
                   tok->next()->str() != "default") {
            const int offset = (tok->next()->str() == "const") ? 1 : 0;
            if (!Token::Match(tok->tokAt(3 + offset), "[{};()]")) {
                tok->deleteNext(4 + offset);
                goback = true;
            }
        }

        if (last && last->str() == ",") {
            Token *tok1 = last;
            tok1->str(";");

            Token *tok2 = tok->next();
            tok1->insertToken(tok2->str());
            tok1 = tok1->next();
            tok1->isSigned(tok2->isSigned());
            tok1->isUnsigned(tok2->isUnsigned());
            tok1->isLong(tok2->isLong());
        }
    }
}


namespace {
    // Types and objects in std namespace that are neither functions nor templates
    const std::set<std::string> stdTypes = make_container<std::set<std::string> >() <<
                                           "string"<< "wstring"<< "u16string"<< "u32string" <<
                                           "iostream"<< "ostream"<< "ofstream"<< "ostringstream" <<
                                           "istream"<< "ifstream"<< "istringstream"<< "fstream"<< "stringstream" <<
                                           "wstringstream"<< "wistringstream"<< "wostringstream"<< "wstringbuf" <<
                                           "stringbuf"<< "streambuf"<< "ios"<< "filebuf"<< "ios_base" <<
                                           "exception"<< "bad_exception"<< "bad_alloc" <<
                                           "logic_error"<< "domain_error"<< "invalid_argument_"<< "length_error" <<
                                           "out_of_range"<< "runtime_error"<< "range_error"<< "overflow_error"<< "underflow_error" <<
                                           "locale" <<
                                           "cout"<< "cerr"<< "clog"<< "cin" <<
                                           "wcerr"<< "wcin"<< "wclog"<< "wcout" <<
                                           "endl"<< "ends"<< "flush" <<
                                           "boolalpha"<< "noboolalpha"<< "showbase"<< "noshowbase" <<
                                           "showpoint"<< "noshowpoint"<< "showpos"<< "noshowpos" <<
                                           "skipws"<< "noskipws"<< "unitbuf"<< "nounitbuf"<< "uppercase"<< "nouppercase" <<
                                           "dec"<< "hex"<< "oct" <<
                                           "fixed"<< "scientific" <<
                                           "internal"<< "left"<< "right" <<
                                           "fpos"<< "streamoff"<< "streampos"<< "streamsize";

    const std::set<std::string> stdTemplates = make_container<std::set<std::string> >() <<
            "array"<< "basic_string"<< "bitset"<< "deque"<< "list"<< "map"<< "multimap" <<
            "priority_queue"<< "queue"<< "set"<< "multiset"<< "stack"<< "vector"<< "pair" <<
            "iterator"<< "iterator_traits" <<
            "unordered_map"<< "unordered_multimap"<< "unordered_set"<< "unordered_multiset" <<
            "tuple"<< "function";
    const std::set<std::string> stdFunctions = make_container<std::set<std::string> >() <<
            "getline" <<
            "for_each"<< "find"<< "find_if"<< "find_end"<< "find_first_of" <<
            "adjacent_find"<< "count"<< "count_if"<< "mismatch"<< "equal"<< "search"<< "search_n" <<
            "copy"<< "copy_backward"<< "swap"<< "swap_ranges"<< "iter_swap"<< "transform"<< "replace" <<
            "replace_if"<< "replace_copy"<< "replace_copy_if"<< "fill"<< "fill_n"<< "generate"<< "generate_n"<< "remove" <<
            "remove_if"<< "remove_copy"<< "remove_copy_if" <<
            "unique"<< "unique_copy"<< "reverse"<< "reverse_copy" <<
            "rotate"<< "rotate_copy"<< "random_shuffle"<< "partition"<< "stable_partition" <<
            "sort"<< "stable_sort"<< "partial_sort"<< "partial_sort_copy"<< "nth_element" <<
            "lower_bound"<< "upper_bound"<< "equal_range"<< "binary_search"<< "merge"<< "inplace_merge"<< "includes" <<
            "set_union"<< "set_intersection"<< "set_difference" <<
            "set_symmetric_difference"<< "push_heap"<< "pop_heap"<< "make_heap"<< "sort_heap" <<
            "min"<< "max"<< "min_element"<< "max_element"<< "lexicographical_compare"<< "next_permutation"<< "prev_permutation" <<
            "advance"<< "back_inserter"<< "distance"<< "front_inserter"<< "inserter" <<
            "make_pair"<< "make_shared"<< "make_tuple";
}


// Add std:: in front of std classes, when using namespace std; was given
void Tokenizer::simplifyNamespaceStd()
{
    if (!isCPP())
        return;

    const bool isCPP11  = _settings->standards.cpp == Standards::CPP11;

    for (const Token* tok = Token::findsimplematch(list.front(), "using namespace std ;"); tok; tok = tok->next()) {
        bool insert = false;
        if (!Token::Match(tok->previous(), ".|::")) {
            if (Token::Match(tok, "%name% (") && !Token::Match(tok->linkAt(1)->next(), "%name%|{") && stdFunctions.find(tok->str()) != stdFunctions.end())
                insert = true;
            else if (Token::Match(tok, "%name% <") && stdTemplates.find(tok->str()) != stdTemplates.end())
                insert = true;
            else if (tok->isName() && !tok->varId() && !Token::Match(tok->next(), "(|<") && stdTypes.find(tok->str()) != stdTypes.end())
                insert = true;
        }

        if (insert) {
            tok->previous()->insertToken("std");
            tok->previous()->linenr(tok->linenr()); // For stylistic reasons we put the std:: in the same line as the following token
            tok->previous()->fileIndex(tok->fileIndex());
            tok->previous()->insertToken("::");
        } else if (isCPP11 && Token::Match(tok, "!!:: tr1 ::"))
            tok->next()->str("std");
    }

    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (isCPP11 && Token::simpleMatch(tok, "std :: tr1 ::"))
            Token::eraseTokens(tok, tok->tokAt(3));

        else if (Token::simpleMatch(tok, "using namespace std ;")) {
            Token::eraseTokens(tok, tok->tokAt(4));
            tok->deleteThis();
        }
    }
}


void Tokenizer::simplifyMicrosoftMemoryFunctions()
{
    // skip if not Windows
    if (!_settings->isWindowsPlatform())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->strAt(1) != "(")
            continue;

        if (Token::Match(tok, "CopyMemory|RtlCopyMemory|RtlCopyBytes")) {
            tok->str("memcpy");
        } else if (Token::Match(tok, "MoveMemory|RtlMoveMemory")) {
            tok->str("memmove");
        } else if (Token::Match(tok, "FillMemory|RtlFillMemory|RtlFillBytes")) {
            // FillMemory(dst, len, val) -> memset(dst, val, len)
            tok->str("memset");

            Token *tok1 = tok->tokAt(2);
            if (tok1)
                tok1 = tok1->nextArgument(); // Second argument
            if (tok1) {
                Token *tok2 = tok1->nextArgument(); // Third argument

                if (tok2)
                    Token::move(tok1->previous(), tok2->tokAt(-2), tok->next()->link()->previous()); // Swap third with second argument
            }
        } else if (Token::Match(tok, "ZeroMemory|RtlZeroMemory|RtlZeroBytes|RtlSecureZeroMemory")) {
            // ZeroMemory(dst, len) -> memset(dst, 0, len)
            tok->str("memset");

            Token *tok1 = tok->tokAt(2);
            if (tok1)
                tok1 = tok1->nextArgument(); // Second argument

            if (tok1) {
                tok1 = tok1->previous();
                tok1->insertToken("0");
                tok1 = tok1->next();
                tok1->insertToken(",");
            }
        } else if (Token::simpleMatch(tok, "RtlCompareMemory")) {
            // RtlCompareMemory(src1, src2, len) -> memcmp(src1, src2, len)
            tok->str("memcmp");
            // For the record, when memcmp returns 0, both strings are equal.
            // When RtlCompareMemory returns len, both strings are equal.
            // It might be needed to improve this replacement by something
            // like ((len - memcmp(src1, src2, len)) % (len + 1)) to
            // respect execution path (if required)
        }
    }
}

namespace {
    struct triplet {
        triplet(const char* m, const char* u) :  mbcs(m), unicode(u) {}
        std::string mbcs, unicode;
    };

    const std::map<std::string, triplet> apis = make_container< std::map<std::string, triplet> >() <<
            std::make_pair("_topen", triplet("open", "_wopen")) <<
            std::make_pair("_tsopen_s", triplet("_sopen_s", "_wsopen_s")) <<
            std::make_pair("_tfopen", triplet("fopen", "_wfopen")) <<
            std::make_pair("_tfopen_s", triplet("fopen_s", "_wfopen_s")) <<
            std::make_pair("_tfreopen", triplet("freopen", "_wfreopen")) <<
            std::make_pair("_tfreopen_s", triplet("freopen_s", "_wfreopen_s")) <<
            std::make_pair("_tcscat", triplet("strcat", "wcscat")) <<
            std::make_pair("_tcschr", triplet("strchr", "wcschr")) <<
            std::make_pair("_tcscmp", triplet("strcmp", "wcscmp")) <<
            std::make_pair("_tcsdup", triplet("strdup", "wcsdup")) <<
            std::make_pair("_tcscpy", triplet("strcpy", "wcscpy")) <<
            std::make_pair("_tcslen", triplet("strlen", "wcslen")) <<
            std::make_pair("_tcsncat", triplet("strncat", "wcsncat")) <<
            std::make_pair("_tcsncpy", triplet("strncpy", "wcsncpy")) <<
            std::make_pair("_tcsnlen", triplet("strnlen", "wcsnlen")) <<
            std::make_pair("_tcsrchr", triplet("strrchr", "wcsrchr")) <<
            std::make_pair("_tcsstr", triplet("strstr", "wcsstr")) <<
            std::make_pair("_tcstok", triplet("strtok", "wcstok")) <<
            std::make_pair("_ftprintf", triplet("fprintf", "fwprintf")) <<
            std::make_pair("_tprintf", triplet("printf", "wprintf")) <<
            std::make_pair("_stprintf", triplet("sprintf", "swprintf")) <<
            std::make_pair("_sntprintf", triplet("_snprintf", "_snwprintf")) <<
            std::make_pair("_ftscanf", triplet("fscanf", "fwscanf")) <<
            std::make_pair("_tscanf", triplet("scanf", "wscanf")) <<
            std::make_pair("_stscanf", triplet("sscanf", "swscanf")) <<
            std::make_pair("_ftprintf_s", triplet("fprintf_s", "fwprintf_s")) <<
            std::make_pair("_tprintf_s", triplet("printf_s", "wprintf_s")) <<
            std::make_pair("_stprintf_s", triplet("sprintf_s", "swprintf_s")) <<
            std::make_pair("_sntprintf_s", triplet("_snprintf_s", "_snwprintf_s")) <<
            std::make_pair("_ftscanf_s", triplet("fscanf_s", "fwscanf_s")) <<
            std::make_pair("_tscanf_s", triplet("scanf_s", "wscanf_s")) <<
            std::make_pair("_stscanf_s", triplet("sscanf_s", "swscanf_s"));
}

void Tokenizer::simplifyMicrosoftStringFunctions()
{
    // skip if not Windows
    if (!_settings->isWindowsPlatform())
        return;

    const bool ansi = _settings->platformType == Settings::Win32A;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->strAt(1) != "(")
            continue;

        std::map<std::string, triplet>::const_iterator match = apis.find(tok->str());
        if (match!=apis.end()) {
            tok->str(ansi ? match->second.mbcs : match->second.unicode);
            tok->originalName(match->first);
        } else if (Token::Match(tok, "_T ( %char%|%str% )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            if (!ansi)
                tok->isLong(true);
            while (Token::Match(tok->next(), "_T ( %char%|%str% )")) {
                tok->next()->deleteNext();
                tok->next()->deleteThis();
                tok->next()->deleteNext();
                tok->concatStr(tok->next()->str());
                tok->deleteNext();
            }
        }
    }
}

// Remove Borland code
void Tokenizer::simplifyBorland()
{
    // skip if not Windows
    if (!_settings->isWindowsPlatform())
        return;
    if (isC())
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( __closure * %name% )")) {
            tok->deleteNext();
        }
    }

    // I think that these classes are always declared at the outer scope
    // I save some time by ignoring inner classes.
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{" && !Token::Match(tok->tokAt(-2), "namespace %type%")) {
            tok = tok->link();
            if (!tok)
                break;
        } else if (Token::Match(tok, "class %name% :|{")) {
            while (tok && tok->str() != "{" && tok->str() != ";")
                tok = tok->next();
            if (!tok)
                break;
            if (tok->str() == ";")
                continue;

            const Token* end = tok->link()->next();
            for (Token *tok2 = tok->next(); tok2 != end; tok2 = tok2->next()) {
                if (tok2->str() == "__property" &&
                    Token::Match(tok2->previous(), ";|{|}|protected:|public:|__published:")) {
                    while (tok2->next() && !Token::Match(tok2->next(), "{|;"))
                        tok2->deleteNext();
                    tok2->deleteThis();
                    if (tok2->str() == "{") {
                        Token::eraseTokens(tok2, tok2->link());
                        tok2->deleteNext();
                        tok2->deleteThis();

                        // insert "; __property ;"
                        tok2->previous()->insertToken(";");
                        tok2->previous()->insertToken("__property");
                        tok2->previous()->insertToken(";");
                    }
                }
            }
        }
    }
}

// Remove Qt signals and slots
void Tokenizer::simplifyQtSignalsSlots()
{
    if (isC())
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // check for emit which can be outside of class
        if (Token::Match(tok, "emit|Q_EMIT %name% (") &&
            Token::simpleMatch(tok->linkAt(2), ") ;")) {
            tok->deleteThis();
        } else if (!Token::Match(tok, "class %name% :"))
            continue;

        if (tok->previous() && tok->previous()->str() == "enum") {
            tok = tok->tokAt(2);
            continue;
        }

        // count { and } for tok2
        unsigned int indentlevel = 0;
        for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                ++indentlevel;
                if (indentlevel == 1)
                    tok = tok2;
                else
                    tok2 = tok2->link();
            } else if (tok2->str() == "}") {
                if (indentlevel<2)
                    break;
                else
                    --indentlevel;
            }
            if (tok2->strAt(1) == "Q_OBJECT")
                tok2->deleteNext();

            if (Token::Match(tok2->next(), "public|protected|private slots|Q_SLOTS :")) {
                tok2 = tok2->next();
                tok2->str(tok2->str() + ":");
                tok2->deleteNext(2);
                tok2 = tok2->previous();
            } else if (Token::Match(tok2->next(), "signals|Q_SIGNALS :")) {
                tok2 = tok2->next();
                tok2->str("protected:");
                tok2->deleteNext();
            } else if (Token::Match(tok2->next(), "emit|Q_EMIT %name% (") &&
                       Token::simpleMatch(tok2->linkAt(3), ") ;")) {
                tok2->deleteNext();
            }
        }
    }
}

void Tokenizer::createSymbolDatabase()
{
    if (!_symbolDatabase)
        _symbolDatabase = new SymbolDatabase(this, _settings, _errorLogger);
    _symbolDatabase->validate();
}

void Tokenizer::deleteSymbolDatabase()
{
    delete _symbolDatabase;
    _symbolDatabase = nullptr;
}

static bool operatorEnd(const Token * tok)
{
    if (tok && tok->str() == ")") {
        tok = tok->next();
        while (tok && !Token::Match(tok, "[=;{),]")) {
            if (Token::Match(tok, "const|volatile")) {
                tok = tok->next();
            } else if (tok->str() == "noexcept") {
                tok = tok->next();
                if (tok && tok->str() == "(") {
                    tok = tok->link()->next();
                }
            } else if (tok->str() == "throw" && tok->next() && tok->next()->str() == "(") {
                tok = tok->next()->link()->next();
            }
            // unknown macros ") MACRO {" and ") MACRO(...) {"
            else if (tok->isUpperCaseName()) {
                tok = tok->next();
                if (tok && tok->str() == "(") {
                    tok = tok->link()->next();
                }
            } else
                return false;
        }

        return true;
    }

    return false;
}

void Tokenizer::simplifyOperatorName()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "operator") {
            // operator op
            std::string op;
            Token *par = tok->next();
            bool done = false;
            while (!done && par) {
                done = true;
                if (par && par->isName()) {
                    op += par->str();
                    par = par->next();
                    // merge namespaces eg. 'operator std :: string () const {'
                    if (Token::Match(par, ":: %name%|%op%|.")) {
                        op += par->str();
                        par = par->next();
                    }
                    done = false;
                }
                if (Token::Match(par, ".|%op%|,")) {
                    op += par->str();
                    par = par->next();
                    done = false;
                }
                if (Token::simpleMatch(par, "[ ]")) {
                    op += "[]";
                    par = par->tokAt(2);
                    done = false;
                }
                if (Token::Match(par, "( *| )")) {
                    // break out and simplify..
                    if (operatorEnd(par->next()))
                        break;

                    while (par->str() != ")") {
                        op += par->str();
                        par = par->next();
                    }
                    op += ")";
                    par = par->next();
                    done = false;
                }
            }

            if (par && operatorEnd(par->link())) {
                tok->str("operator" + op);
                Token::eraseTokens(tok, par);
            }

            if (!op.empty())
                tok->isOperatorKeyword(true);
        }
    }

    if (_settings->debugwarnings) {
        const Token *tok = list.front();

        while ((tok = Token::findsimplematch(tok, "operator")) != nullptr) {
            reportError(tok, Severity::debug, "debug",
                        "simplifyOperatorName: found unsimplified operator name");
            tok = tok->next();
        }
    }
}

// remove unnecessary member qualification..
void Tokenizer::removeUnnecessaryQualification()
{
    if (isC())
        return;

    std::vector<Space> classInfo;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct|namespace %type% :|{") &&
            (!tok->previous() || tok->previous()->str() != "enum")) {
            Space info;
            info.isNamespace = tok->str() == "namespace";
            tok = tok->next();
            info.className = tok->str();
            tok = tok->next();
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (!tok)
                return;
            info.classEnd = tok->link();
            classInfo.push_back(info);
        } else if (!classInfo.empty()) {
            if (tok == classInfo.back().classEnd)
                classInfo.pop_back();
            else if (tok->str() == classInfo.back().className &&
                     !classInfo.back().isNamespace && tok->previous()->str() != ":" &&
                     (Token::Match(tok, "%type% :: ~| %type% (") ||
                      Token::Match(tok, "%type% :: operator"))) {
                const Token *tok1 = tok->tokAt(3);
                if (tok->strAt(2) == "operator") {
                    // check for operator ()
                    if (tok1->str() == "(")
                        tok1 = tok1->next();

                    while (tok1 && tok1->str() != "(") {
                        if (tok1->str() == ";")
                            break;
                        tok1 = tok1->next();
                    }
                    if (!tok1 || tok1->str() != "(")
                        continue;
                } else if (tok->strAt(2) == "~")
                    tok1 = tok1->next();

                if (!tok1 || !Token::Match(tok1->link(), ") const| {|;|:")) {
                    continue;
                }

                const bool isConstructorOrDestructor =
                    Token::Match(tok, "%type% :: ~| %type%") && (tok->strAt(2) == tok->str() || (tok->strAt(2) == "~" && tok->strAt(3) == tok->str()));
                if (!isConstructorOrDestructor) {
                    bool isPrependedByType = Token::Match(tok->previous(), "%type%");
                    if (!isPrependedByType) {
                        const Token* tok2 = tok->tokAt(-2);
                        isPrependedByType = Token::Match(tok2, "%type% *|&");
                    }
                    if (!isPrependedByType) {
                        const Token* tok3 = tok->tokAt(-3);
                        isPrependedByType = Token::Match(tok3, "%type% * *|&");
                    }
                    if (!isPrependedByType) {
                        // It's not a constructor declaration and it's not a function declaration so
                        // this is a function call which can have all the qualifiers just fine - skip.
                        continue;
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyDeprecated()
{
    if (_settings->standards.cpp != Standards::CPP11 || isC())
        return; // It is actually a C++14 feature, however, there seems to be nothing dangerous about removing it for C++11 as well

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->link() && Token::simpleMatch(tok, "[ [ deprecated")) {
            Token::eraseTokens(tok, tok->link()->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyReturnStrncat()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "return strncat (") &&
            Token::simpleMatch(tok->linkAt(2), ") ;") &&
            tok->strAt(3) != ")" && tok->strAt(3) != ",") {

            //first argument
            Token *tok2 = tok->tokAt(3);

            //check if there are at least three arguments
            for (unsigned char i = 0; i < 2; ++i) {
                tok2 = tok2->nextArgument();
                if (!tok2) {
                    tok = tok->linkAt(2)->next();
                    break;
                }
            }
            if (!tok2)
                continue;

            tok2 = tok2->nextArgument();
            //we want only three arguments
            if (tok2) {
                tok = tok->linkAt(2)->next();
                continue;
            }

            // Remove 'return'
            tok->deleteThis();

            // Add 'return arg1 ;' after 'strncat(arg1, arg2, arg3);'
            tok = tok->next();

            tok2 = tok->link()->next();
            tok2->insertToken(";");

            //the last token of the first argument before ','
            Token *end = tok->next()->nextArgument()->tokAt(-2);

            //all the first argument is copied
            copyTokens(tok2, tok->next(), end);
            tok2->insertToken("return");
        }
    }
}

void Tokenizer::printUnknownTypes() const
{
    if (!_symbolDatabase)
        return;

    std::multimap<std::string, const Token *> unknowns;

    for (unsigned int i = 1; i <= _varId; ++i) {
        const Variable *var = _symbolDatabase->getVariableFromVarId(i);

        // is unknown type?
        if (var && !var->type() && !var->typeStartToken()->isStandardType()) {
            std::string name;
            const Token * nameTok;

            // single token type?
            if (var->typeStartToken() == var->typeEndToken()) {
                nameTok = var->typeStartToken();
                name = nameTok->str();
            }

            // complicated type
            else {
                const Token *tok = var->typeStartToken();
                int level = 0;

                nameTok =  tok;

                while (tok) {
                    // skip pointer and reference part of type
                    if (level == 0 && Token::Match(tok, "*|&"))
                        break;

                    name += tok->str();

                    if (Token::Match(tok, "struct|union|enum"))
                        name += " ";

                    // pointers and references are OK in template
                    else if (tok->str() == "<")
                        ++level;
                    else if (tok->str() == ">")
                        --level;

                    if (tok == var->typeEndToken())
                        break;

                    tok = tok->next();
                }
            }

            unknowns.insert(std::pair<std::string, const Token *>(name, nameTok));
        }
    }

    if (!unknowns.empty()) {
        std::multimap<std::string, const Token *>::const_iterator it;
        std::string last;
        size_t count = 0;

        for (it = unknowns.begin(); it != unknowns.end(); ++it) {
            // skip types is std namespace because they are not interesting
            if (it->first.find("std::") != 0) {
                if (it->first != last) {
                    last = it->first;
                    count = 1;
                    reportError(it->second, Severity::debug, "debug", "Unknown type \'" + it->first + "\'.");
                } else {
                    if (count < 3) // limit same type to 3
                        reportError(it->second, Severity::debug, "debug", "Unknown type \'" + it->first + "\'.");
                    count++;
                }
            }
        }
    }
}

void Tokenizer::simplifyMathExpressions()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {

        //simplify Pythagorean trigonometric identity: pow(sin(x),2)+pow(cos(x),2) = 1
        //                                             pow(cos(x),2)+pow(sin(x),2) = 1
        // @todo: sin(x) * sin(x) + cos(x) * cos(x) = 1
        //        cos(x) * cos(x) + sin(x) * sin(x) = 1
        //simplify Hyperbolic identity: pow(sinh(x),2)-pow(cosh(x),2) = -1
        //                              pow(cosh(x),2)-pow(sinh(x),2) = -1
        // @todo: sinh(x) * sinh(x) - cosh(x) * cosh(x) = -1
        //        cosh(x) * cosh(x) - sinh(x) * sinh(x) = -1
        if (Token::Match(tok, "pow|powf|powl (")) {
            if (Token::Match(tok->tokAt(2), "sin|sinf|sinl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) + pow|powf|powl ( cos|cosf|cosl ("))
                    continue;
                const std::string& leftExponent = tok2->strAt(2);
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                Token * const tok4 = tok3->link();
                if (!Token::Match(tok4, ") , %num% )"))
                    continue;
                const std::string& rightExponent = tok4->strAt(2);
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok4->next())) {
                    Token::eraseTokens(tok, tok4->tokAt(4));
                    tok->str("1");
                }
            } else if (Token::Match(tok->tokAt(2), "cos|cosf|cosl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) + pow|powf|powl ( sin|sinf|sinl ("))
                    continue;
                const std::string& leftExponent = tok2->strAt(2);
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                Token * const tok4 = tok3->link();
                if (!Token::Match(tok4, ") , %num% )"))
                    continue;
                const std::string& rightExponent = tok4->strAt(2);
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok4->next())) {
                    Token::eraseTokens(tok, tok4->tokAt(4));
                    tok->str("1");
                }
            } else if (Token::Match(tok->tokAt(2), "sinh|sinhf|sinhl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) - pow|powf|powl ( cosh|coshf|coshl ("))
                    continue;
                const std::string& leftExponent = tok2->strAt(2);
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                Token * const tok4 = tok3->link();
                if (!Token::Match(tok4, ") , %num% )"))
                    continue;
                const std::string& rightExponent = tok4->strAt(2);
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok4->next())) {
                    Token::eraseTokens(tok, tok4->tokAt(4));
                    tok->str("-1");
                }
            } else if (Token::Match(tok->tokAt(2), "cosh|coshf|coshl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) - pow|powf|powl ( sinh|sinhf|sinhl ("))
                    continue;
                const std::string& leftExponent = tok2->strAt(2);
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                Token * const tok4 = tok3->link();
                if (!Token::Match(tok4, ") , %num% )"))
                    continue;
                const std::string& rightExponent = tok4->strAt(2);
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok4->next())) {
                    Token::eraseTokens(tok, tok4->tokAt(4));
                    tok->str("-1");
                }
            }
        }
    }
}

bool Tokenizer::simplifyStrlen()
{
    // replace strlen(str)
    bool modified=false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "strlen ( %str% )")) {
            tok->str(MathLib::toString(Token::getStrLength(tok->tokAt(2))));
            tok->deleteNext(3);
            modified=true;
        }
    }
    return modified;
}

void Tokenizer::prepareTernaryOpForAST()
{
    // http://en.cppreference.com/w/cpp/language/operator_precedence says about ternary operator:
    //       "The expression in the middle of the conditional operator (between ? and :) is parsed as if parenthesized: its precedence relative to ?: is ignored."
    // The AST parser relies on this function to add such parentheses where necessary.
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "?") {
            bool parenthesesNeeded = false;
            unsigned int depth = 0;
            Token* tok2 = tok->next();
            for (; tok2; tok2 = tok2->next()) {
                if (tok2->link() && Token::Match(tok2, "[|(|<"))
                    tok2 = tok2->link();
                else if (tok2->str() == ":") {
                    if (depth == 0)
                        break;
                    depth--;
                } else if (tok2->str() == ";" || (tok2->link() && tok2->str() != "{" && tok2->str() != "}"))
                    break;
                else if (tok2->str() == ",")
                    parenthesesNeeded = true;
                else if (tok2->str() == "?") {
                    depth++;
                    parenthesesNeeded = true;
                }
            }
            if (parenthesesNeeded && tok2 && tok2->str() == ":") {
                tok->insertToken("(");
                tok2->insertToken(")", emptyString, true);
                Token::createMutualLinks(tok->next(), tok2->previous());
            }
        }
    }
}

void Tokenizer::reportError(const Token* tok, const Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    const std::list<const Token*> callstack(1, tok);
    reportError(callstack, severity, id, msg, inconclusive);
}

void Tokenizer::reportError(const std::list<const Token*>& callstack, Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    const ErrorLogger::ErrorMessage errmsg(callstack, &list, severity, id, msg, inconclusive);
    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::setPodTypes()
{
    if (!_settings)
        return;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName())
            continue;

        // pod type
        const struct Library::PodType *podType = _settings->library.podtype(tok->str());
        if (podType) {
            const Token *prev = tok->previous();
            while (prev && prev->isName())
                prev = prev->previous();
            if (prev && !Token::Match(prev, ";|{|}|,|("))
                continue;
            tok->isStandardType(true);
        }
    }
}

void Tokenizer::SimplifyNamelessRValueReferences()
{
    // Simplify nameless rValue references - named ones are simplified later
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "&& [,)]")) {
            tok->str("&");
            tok->insertToken("&");
        }
    }
}
