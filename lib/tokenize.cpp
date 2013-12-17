/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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

#include <cstring>
#include <sstream>
#include <cassert>
#include <cctype>
#include <stack>

//---------------------------------------------------------------------------

Tokenizer::Tokenizer() :
    list(0),
    _settings(0),
    _errorLogger(0),
    _symbolDatabase(0),
    _varId(0),
    _codeWithTemplates(false), //is there any templates?
    m_timerResults(NULL)
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
    m_timerResults(NULL)
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

    if (type->type() == Token::eString)
        return static_cast<unsigned int>(Token::getStrLength(type) + 1);

    std::map<std::string, unsigned int>::const_iterator it = _typeSize.find(type->str());
    if (it == _typeSize.end())
        return 0;
    else if (type->isLong()) {
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
    unsigned int commonFileIndex = dest->fileIndex();
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(commonFileIndex);
        tok2->linenr(linenrs);
        tok2->type(tok->type());
        tok2->isUnsigned(tok->isUnsigned());
        tok2->isSigned(tok->isSigned());
        tok2->isPointerCompare(tok->isPointerCompare());
        tok2->isLong(tok->isLong());
        tok2->isExpandedMacro(tok->isExpandedMacro());
        tok2->isAttributeConstructor(tok->isAttributeConstructor());
        tok2->isAttributeUnused(tok->isAttributeUnused());
        tok2->varId(tok->varId());

        // Check for links and fix them up
        if (tok2->str() == "(" || tok2->str() == "[" || tok2->str() == "{")
            links.push(tok2);
        else if (tok2->str() == ")" || tok2->str() == "]" || tok2->str() == "}") {
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

void Tokenizer::duplicateTypedefError(const Token *tok1, const Token *tok2, const std::string &type) const
{
    if (tok1 && !(_settings->isEnabled("style") && _settings->inconclusive))
        return;

    std::list<const Token*> locationList;
    locationList.push_back(tok1);
    locationList.push_back(tok2);
    const std::string tok2_str = tok2 ? tok2->str() : std::string("name");

    reportError(locationList, Severity::style, "variableHidingTypedef",
                std::string("The " + type + " '" + tok2_str + "' hides a typedef with the same name."), true);
}

void Tokenizer::duplicateDeclarationError(const Token *tok1, const Token *tok2, const std::string &type) const
{
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<const Token*> locationList;
    locationList.push_back(tok1);
    locationList.push_back(tok2);
    const std::string tok2_str = tok2 ? tok2->str() : std::string("name");

    reportError(locationList, Severity::style, "unnecessaryForwardDeclaration",
                std::string("The " + type + " '" + tok2_str + "' forward declaration is unnecessary. Type " + type + " is already declared earlier."));
}

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef, bool undefinedStruct) const
{
    // check for an end of definition
    const Token * tok = *tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>|(|{")) {
        const Token * end = tok->next();

        if (end->str() == "[") {
            end = end->link()->next();
        } else if (end->str() == ",") {
            // check for derived class
            if (Token::Match(tok->previous(), "public|private|protected"))
                return false;

            // find end of definition
            while (end && end->next() && !Token::Match(end->next(), ";|)|>")) {
                if (end->next()->str() == "(")
                    end = end->linkAt(1);

                end = end->next();
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

                duplicateTypedefError(*tokPtr, name, "template instantiation");
                *tokPtr = end->link();
                return true;
            }
        }

        if (end) {
            if (Token::simpleMatch(end, ") {")) { // function parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const")) {
                    duplicateTypedefError(*tokPtr, name, "function parameter");
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
                        duplicateTypedefError(*tokPtr, name, "template parameter");
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
                            duplicateTypedefError(*tokPtr, name, "typedef");
                            return true;
                        } else if (tok->previous()->str() == "enum") {
                            duplicateTypedefError(*tokPtr, name, "enum");
                            return true;
                        } else if (tok->previous()->str() == "struct") {
                            if (tok->strAt(-2) == "typedef" &&
                                tok->next()->str() == "{" &&
                                typeDef->strAt(3) != "{") {
                                // declaration after forward declaration
                                return true;
                            } else if (tok->next()->str() == "{") {
                                if (!undefinedStruct)
                                    duplicateTypedefError(*tokPtr, name, "struct");
                                return true;
                            } else if (Token::Match(tok->next(), ")|*")) {
                                return true;
                            } else if (tok->next()->str() == name->str()) {
                                return true;
                            } else if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "struct");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "struct");
                                return false;
                            }
                        } else if (tok->previous()->str() == "union") {
                            if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "union");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "union");
                                return false;
                            }
                        } else if (tok->previous()->str() == "class") {
                            if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "class");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "class");
                                return false;
                            }
                        }

                        tok = tok->previous();
                    }

                    duplicateTypedefError(*tokPtr, name, "variable");
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
    Token *tok = NULL;

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
            return NULL;
    } else if (tok->strAt(3) == ":") {
        tok1 = tok->tokAt(4);
        while (tok1 && tok1->str() != "{")
            tok1 = tok1->next();
        if (!tok1)
            return NULL;

        tok1 = tok1->link();

        name = tok->strAt(2);
    } else { // has a name
        tok1 = tok->linkAt(3);

        if (!tok1)
            return NULL;

        name = tok->strAt(2);
    }

    tok1->insertToken(";");
    tok1 = tok1->next();

    if (tok1->next() && tok1->next()->str() == ";" && tok1->previous()->str() == "}") {
        tok->deleteThis();
        tok1->deleteThis();
        return NULL;
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
static Token *processFunc(Token *tok2, bool inOperator)
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
                while (Token::Match(tok2, "%var% ::"))
                    tok2 = tok2->tokAt(2);

                if (!tok2)
                    return NULL;

                if (tok2->str() == "(" &&
                    tok2->link()->next()->str() == "(") {
                    tok2 = tok2->link();

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }

                // skip over typedef parameter
                if (tok2->next() && tok2->next()->str() == "(") {
                    tok2 = tok2->next()->link();

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

#ifdef MAXTIME
        if (std::time(0) > maxtime)
            return;
#endif

        if (goback) {
            //jump back once, see the comment at the end of the function
            goback = false;
            tok = tok->previous();
        }

        // Skip typedefs inside parentheses (#2453 and #4002)
        if (tok->str() == "(" && tok->strAt(1) == "typedef") {
            tok = tok->next();
            continue;
        }

        if (Token::Match(tok, "class|struct|namespace %any%") &&
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum"))) {
            isNamespace = (tok->str() == "namespace");
            hasClass = true;
            className = tok->next()->str();
            continue;
        } else if (hasClass && tok->str() == ";") {
            hasClass = false;
            continue;
        } else if (hasClass && tok->str() == "{") {
            Space info;
            info.isNamespace = isNamespace;
            info.className = className;
            info.classEnd = tok->link();
            spaceInfo.push_back(info);

            hasClass = false;
            continue;
        } else if (!spaceInfo.empty() && tok->str() == "}" && spaceInfo.back().classEnd == tok) {
            spaceInfo.pop_back();
            continue;
        } else if (tok->str() != "typedef")
            continue;

        // pull struct, union, enum or class definition out of typedef
        // use typedef name for unnamed struct, union, enum or class
        if (Token::Match(tok->next(), "const| struct|enum|union|class %type% {") ||
            Token::Match(tok->next(), "const| struct|enum|union|class {")) {
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
        bool undefinedStruct = false;
        if (Token::Match(tok, "typedef enum|struct %type% %type% ;") && tok->strAt(2) == tok->strAt(3)) {
            if (tok->next()->str() == "enum") {
                tok->deleteNext(3);
                tok->deleteThis();
                if (tok->next())
                    tok->deleteThis();
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
                continue;
            } else {
                const std::string pattern("struct " + tok->strAt(2) + " {|:");
                const Token *tok2 = Token::findmatch(list.front(), pattern.c_str(), tok);
                if (!tok2)
                    undefinedStruct = true;
            }
        }

        Token *typeName;
        std::list<std::string> pointers;
        Token *typeStart = 0;
        Token *typeEnd = 0;
        Token *argStart = 0;
        Token *argEnd = 0;
        Token *arrayStart = 0;
        Token *arrayEnd = 0;
        Token *specStart = 0;
        Token *specEnd = 0;
        Token *typeDef = tok;
        Token *argFuncRetStart = 0;
        Token *argFuncRetEnd = 0;
        Token *funcStart = 0;
        Token *funcEnd = 0;
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
        Token *namespaceStart = 0;
        Token *namespaceEnd = 0;

        // check for invalid input
        if (!tok->next()) {
            syntaxError(tok);
            return;
        }

        if (tok->next()->str() == "::" || Token::Match(tok->next(), "%type%")) {
            typeStart = tok->next();

            while (Token::Match(tokOffset, "const|signed|unsigned|struct|enum %type%") ||
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
        if (!tokOffset) {
            syntaxError(tok);
            return;
        }

        // check for template
        if (tokOffset->str() == "<") {
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

        // check for pointers and references
        while (Token::Match(tokOffset, "*|&|const")) {
            pointers.push_back(tokOffset->str());
            tokOffset = tokOffset->next();
        }

        // check for invalid input
        if (!tokOffset) {
            syntaxError(tok);
            return;
        }

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

                // unhandled function pointer, skip it and continue
                // TODO: handle such typedefs. See ticket #3314
                else if (Token::Match(tokOffset, "( %type% ::") &&
                         Token::Match(tokOffset->link()->tokAt(-3), ":: * %var% ) (")) {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    if (tok == list.front())
                        //now the next token to process is 'tok', not 'tok->next()';
                        goback = true;
                    continue;
                }

                // function pointer
                else if (Token::Match(tokOffset, "( * %var% ) (")) {
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
                else if (Token::Match(tokOffset->link(), ") const| ;|,")) {
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
                else {
                    syntaxError(tok);
                    return;
                }
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
        else if ((tokOffset->str() == "(" &&
                  Token::Match(tokOffset->link()->previous(), "%type% ) (") &&
                  Token::Match(tokOffset->link()->next()->link(), ") const|volatile|;")) ||
                 (Token::simpleMatch(tokOffset, "( (") &&
                  Token::Match(tokOffset->next()->link()->previous(), "%type% ) (") &&
                  Token::Match(tokOffset->next()->link()->next()->link(), ") const|volatile| ) ;|,")) ||
                 (Token::simpleMatch(tokOffset, "( * (") &&
                  Token::Match(tokOffset->linkAt(2)->previous(), "%type% ) (") &&
                  Token::Match(tokOffset->linkAt(2)->next()->link(), ") const|volatile| ) ;|,"))) {
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

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argFuncRetStart->link();

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
            argFuncRetEnd = argFuncRetStart->link();

            tok = argFuncRetEnd->next();
        } else if (Token::Match(tokOffset, "( * ( %type% ) (")) {
            functionRetFuncPtr = true;

            tokOffset = tokOffset->tokAt(5);
            typeName = tokOffset->tokAt(-2);
            argStart = tokOffset;
            argEnd = tokOffset->link();

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argFuncRetStart->link();

            tok = argFuncRetEnd->next();
        }

        // pointer/reference to array
        else if (Token::Match(tokOffset, "( *|& %type% ) [")) {
            ptrToArray = (tokOffset->next()->str() == "*");
            refToArray = (tokOffset->next()->str() == "&");
            tokOffset = tokOffset->tokAt(2);
            typeName = tokOffset;
            arrayStart = tokOffset->tokAt(2);
            arrayEnd = arrayStart->link();
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
            int scope = 0;
            bool inScope = true;
            bool exitThisScope = false;
            int exitScope = 0;
            bool simplifyType = false;
            bool inMemberFunc = false;
            int memberScope = 0;
            bool globalScope = false;
            std::size_t classLevel = spaceInfo.size();

            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (_settings->terminated())
                    return;

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
                        --scope;
                        if (scope < 0)
                            inScope = false;

                        if (exitThisScope) {
                            if (scope < exitScope)
                                exitThisScope = false;
                        }
                    }
                }

                // check for operator typedef
                /** @todo add support for multi-token operators */
                else if (tok2->str() == "operator" &&
                         tok2->next()->str() == typeName->str() &&
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

                // check for member functions
                else if (Token::Match(tok2, ") const| {")) {
                    const Token *func = tok2->link()->previous();
                    if (!func)
                        continue;

                    if (func->previous()) { // Ticket #4239
                        /** @todo add support for multi-token operators */
                        if (func->previous()->str() == "operator")
                            func = func->previous();

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
                }

                // check for entering a new namespace
                else if (Token::Match(tok2, "namespace %any% {")) {
                    if (classLevel < spaceInfo.size() &&
                        spaceInfo[classLevel].isNamespace &&
                        spaceInfo[classLevel].className == tok2->next()->str()) {
                        ++classLevel;
                        pattern.clear();
                        for (std::size_t i = classLevel; i < spaceInfo.size(); ++i)
                            pattern += (spaceInfo[i].className + " :: ");

                        pattern += typeName->str();
                    }
                    ++scope;
                }

                // check for entering a new scope
                else if (tok2->str() == "{") {
                    // keep track of scopes within member function
                    if (inMemberFunc)
                        ++memberScope;

                    ++scope;
                }

                // check for typedef that can be substituted
                else if (Token::Match(tok2, pattern.c_str()) ||
                         (inMemberFunc && tok2->str() == typeName->str())) {
                    std::string pattern1;

                    // member function class variables don't need qualification
                    if (inMemberFunc && tok2->str() == typeName->str())
                        pattern1 = tok2->str();
                    else
                        pattern1 = pattern;

                    if (pattern1.find("::") != std::string::npos) { // has a "something ::"
                        if (tok2->strAt(-1) == "::") {
                            tok2->tokAt(-2)->deleteNext();
                            globalScope = true;
                        }

                        for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                            tok2->deleteNext(2);
                        }
                        simplifyType = true;
                    } else if ((inScope && !exitThisScope) || inMemberFunc) {
                        if (tok2->strAt(-1) == "::") {
                            // Don't replace this typename if it's preceded by "::" unless it's a namespace
                            if (!spaceInfo.empty() && (tok2->strAt(-2) == spaceInfo[0].className) && spaceInfo[0].isNamespace) {
                                tok2->tokAt(-3)->deleteNext(2);
                                simplifyType = true;
                            }
                        } else if (Token::Match(tok2->previous(), "case %type% :")) {
                            tok2 = tok2->next();
                        } else if (duplicateTypedef(&tok2, typeName, typeDef, undefinedStruct)) {
                            exitScope = scope;

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
                        } else if (tok2->previous()->str() != ".") {
                            simplifyType = true;
                        }
                    }
                }

                if (simplifyType) {
                    // can't simplify 'operator functionPtr ()' and 'functionPtr operator ... ()'
                    if (functionPtr && (tok2->previous()->str() == "operator" ||
                                        tok2->next()->str() == "operator")) {
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

                    // check for derived class: class A : some_typedef {
                    bool isDerived = Token::Match(tok2->previous(), "public|protected|private %type% {|,");

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

                    // skip over class or struct in derived class declaration
                    bool structRemoved = false;
                    if (isDerived && Token::Match(typeStart, "class|struct")) {
                        if (typeStart->str() == "struct")
                            structRemoved = true;
                        typeStart = typeStart->next();
                    }

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
                        std::list<std::string>::const_iterator iter;
                        for (iter = pointers.begin(); iter != pointers.end(); ++iter) {
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
                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok3);
                        }

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        if (inTemplate)
                            tok2 = tok2->next();

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

                        Token * tok4 = 0;
                        if (functionPtrRetFuncPtr) {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                            tok4 = tok2;
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        }

                        // skip over variable name if there
                        if (!inCast) {
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
                        if (tok2->next()->str() != ")") {
                            if (tok2->next()->str() != "(")
                                tok2 = tok2->next();

                            // check for function and skip over args
                            if (tok2->next()->str() == "(")
                                tok2 = tok2->next()->link();

                            // check for array
                            if (tok2->next()->str() == "[")
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
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                            Token *tok3 = tok2;

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

                            // skip over name
                            tok2 = tok2->next();

                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok3);
                        }
                    } else if (typeOf) {
                        tok2 = copyTokens(tok2, argStart, argEnd);
                    } else if (tok2->tokAt(2) && tok2->strAt(2) == "[") {
                        while (tok2->tokAt(2) && tok2->strAt(2) == "[")
                            tok2 = tok2->linkAt(2)->previous();
                    }

                    if (arrayStart && arrayEnd) {
                        do {
                            if (!tok2->next()) {
                                syntaxError(tok2);
                                return; // can't recover so quit
                            }

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

                                tok2->insertToken(")");
                                tok2 = tok2->next();
                                Token::createMutualLinks(tok2, tok3);
                            }

                            if (!tok2->next()) {
                                syntaxError(tok2);
                                return; // can't recover so quit
                            }

                            tok2 = copyTokens(tok2, arrayStart, arrayEnd);
                            tok2 = tok2->next();

                            if (tok2->str() == "=") {
                                if (tok2->next()->str() == "{")
                                    tok2 = tok2->next()->link()->next();
                                else if (tok2->next()->str().at(0) == '\"')
                                    tok2 = tok2->tokAt(2);
                            }
                        } while (Token::Match(tok2, ", %var% ;|'|=|,"));
                    }

                    simplifyType = false;
                }
            }

            if (tok->str() == ";")
                done = true;
            else if (tok->str() == ",") {
                arrayStart = 0;
                arrayEnd = 0;
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

                        bool atEnd = false;
                        while (!atEnd) {
                            while (tokOffset->next() && !Token::Match(tokOffset->next(), ";|,"))
                                tokOffset = tokOffset->next();

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
            while (tokbegin && (tokbegin->str() == "&" || tokbegin->str() == "(")) {
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
                        while (openpars--) {
                            tok->deleteNext();
                            tokbegin->deleteNext();
                            --closedpars;
                        }
                    } else
                        break;
                }
            }
        }
    }
}

bool Tokenizer::tokenize(std::istream &code,
                         const char FileName[],
                         const std::string &configuration)
{
    // make sure settings specified
    assert(_settings);

    // Fill the map _typeSize..
    fillTypeSizes();

    _configuration = configuration;

    if (!list.createTokens(code, Path::getRelativePath(Path::simplifyPath(FileName), _settings->_basePaths))) {
        cppcheckError(0);
        return false;
    }

    if (_settings->terminated())
        return false;

    // if MACRO
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if|for|while|BOOST_FOREACH %var% (")) {
            if (Token::simpleMatch(tok, "for each")) {
                // 'for each ( )' -> 'asm ( )'
                tok->str("asm");
                tok->deleteNext();
            } else {
                syntaxError(tok);
                return false;
            }
        }
    }

    // remove MACRO in variable declaration: MACRO int x;
    removeMacroInVarDecl();

    // Combine strings
    combineStrings();

    // replace inline SQL with "asm()" (Oracle PRO*C). Ticket: #1959
    simplifySQL();

    // replace __LINE__ macro with line number
    simplifyFileAndLineMacro();

    // Concatenate double sharp: 'a ## b' -> 'ab'
    concatenateDoubleSharp();

    if (!createLinks()) {
        // Source has syntax errors, can't proceed
        return false;
    }

    // if (x) MACRO() ..
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "if (")) {
            tok = tok->next()->link();
            if (Token::Match(tok, ") %var% (") &&
                tok->next()->isUpperCaseName() &&
                Token::Match(tok->linkAt(2), ") {|else")) {
                syntaxError(tok->next());
                return false;
            }
        }
    }

    if (_settings->terminated())
        return false;

    // replace 'NULL' and similar '0'-defined macros with '0'
    simplifyNull();

    // replace 'sin(0)' to '0' and other similar math expressions
    simplifyMathExpressions();

    // combine "- %num%"
    concatenateNegativeNumberAndAnyPositive();

    // simplify simple calculations
    for (Token *tok = list.front() ? list.front()->next() : NULL; tok; tok = tok->next()) {
        if (tok->isNumber())
            TemplateSimplifier::simplifyNumericCalculations(tok->previous());
    }

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
            list.deallocateTokens();
            return false;
        }
    }

    if (!simplifyAddBraces())
        return false;

    sizeofAddParentheses();

    // Combine tokens..
    combineOperators();

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

    // Convert K&R function declarations to modern C
    simplifyVarDecl(true);
    if (!simplifyFunctionParameters())
        return false;

    // specify array size..
    arraySize();

    // simplify labels and 'case|default'-like syntaxes
    if (!simplifyLabelsCaseDefault())
        return false;

    // simplify '[;{}] * & ( %any% ) =' to '%any% ='
    simplifyMulAndParens();

    // ";a+=b;" => ";a=a+b;"
    simplifyCompoundAssignment();

    if (hasComplicatedSyntaxErrorsInTemplates()) {
        list.deallocateTokens();
        return false;
    }

    if (_settings->terminated())
        return false;

    // Remove __declspec()
    simplifyDeclspec();

    // remove some unhandled macros in global scope
    removeMacrosInGlobalScope();

    // remove calling conventions __cdecl, __stdcall..
    simplifyCallingConvention();

    // remove __attribute__((?))
    simplifyAttribute();

    // remove unnecessary member qualification..
    removeUnnecessaryQualification();

    // remove Microsoft MFC..
    simplifyMicrosoftMFC();

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

    // Remove __builtin_expect, likely and unlikely
    simplifyBuiltinExpect();

    if (hasEnumsWithTypedef()) {
        // #2449: syntax error: enum with typedef in it
        list.deallocateTokens();
        return false;
    }

    simplifyDebugNew();

    // typedef..
    if (m_timerResults) {
        Timer t("Tokenizer::tokenize::simplifyTypedef", _settings->_showtime, m_timerResults);
        simplifyTypedef();
    } else {
        simplifyTypedef();
    }

    for (Token* tok = list.front(); tok;) {
        if (Token::Match(tok, "union|struct|class union|struct|class"))
            tok->deleteNext();
        else
            tok = tok->next();
    }

    // class x y {
    if (_settings->isEnabled("information")) {
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
    if (!validate()) {
        // Source has syntax errors, can't proceed
        return false;
    }

    // enum..
    simplifyEnum();

    // The simplify enum have inner loops
    if (_settings->terminated())
        return false;

    // Remove __asm..
    simplifyAsm();

    // Put ^{} statements in asm()
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "^ {")) {
            Token * start = tok;
            while (start && !Token::Match(start, "[;{}]"))
                start = start->previous();
            if (start)
                start = start->next();
            const Token *last = tok->next()->link();
            if (start != tok) {
                last = last->next();
                while (last && !Token::Match(last->next(), "[;{}()]"))
                    last = last->next();
            }
            if (start && last) {
                std::string asmcode(start->str());
                while (start->next() != last) {
                    asmcode += start->next()->str();
                    start->deleteNext();
                }
                asmcode += last->str();
                start->deleteNext();
                start->insertToken(";");
                start->insertToken(")");
                start->insertToken("\"" + asmcode + "\"");
                start->insertToken("(");
                start->str("asm");
                start->link(NULL);
                start->next()->link(start->tokAt(3));
                start->tokAt(3)->link(start->next());
                tok = start->tokAt(4);
            }
        }
    }

    // When the assembly code has been cleaned up, no @ is allowed
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(")
            tok = tok->link();
        else if (tok->str()[0] == '@') {
            list.deallocateTokens();
            return false;
        }
    }

    // convert platform dependent types to standard types
    // 32 bits: size_t -> unsigned long
    // 64 bits: size_t -> unsigned long long
    simplifyPlatformTypes();

    // collapse compound standard types into a single token
    // unsigned long long int => long _isUnsigned=true,_isLong=true
    simplifyStdType();

    // The simplifyTemplates have inner loops
    if (_settings->terminated())
        return false;

    // simplify bit fields..
    simplifyBitfields();

    // Use "<" comparison instead of ">"
    simplifyComparisonOrder();

    // Simplify '(p == 0)' to '(!p)'
    simplifyIfNot();
    simplifyIfNotNull();

    //simplify for: move out start-statement "for (a;b;c);" => "{ a; for(;b;c); }"
    //not enabled because it fails many tests with testrunner.
    //@todo fix these fails before enabling this simplification
    /*for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(" && ( !tok->previous() || tok->previous()->str() != "for")) {
            tok = tok->link();
            continue;
        }
        if (!Token::Match(tok->previous(),"[{};] for ("))
            continue;

        //find the two needed semicolons inside the 'for'
        const Token *firstsemicolon = Token::findsimplematch(tok->next(), ";", tok->next()->link());
        if (!firstsemicolon)
            continue;
        const Token *secondsemicolon = Token::findsimplematch(firstsemicolon->next(), ";", tok->next()->link());
        if (!secondsemicolon)
            continue;
        if (Token::findsimplematch(secondsemicolon->next(), ";", tok->next()->link()))
            continue;       //no more than two semicolons!
        if (!tok->next()->link()->next())
            continue;       //there should be always something after 'for (...)'

        Token *fortok = tok;
        Token *begin = tok->tokAt(2);
        Token *end = tok->next()->link();
        if ( begin->str() != ";" ) {
            tok = tok->previous();
            tok->insertToken(";");
            tok->insertToken("{");
            tok = tok->next();
            if (end->next()->str() =="{") {
                end = end->next()->link();
                end->insertToken("}");
                Token::createMutualLinks(tok, end->next());
                end = end->link()->previous();
            } else {
                if (end->next()->str() != ";")
                    end->insertToken(";");
                end = end->next();
                end->insertToken("}");
                Token::createMutualLinks(tok, end->next());
            }
            end = firstsemicolon->previous();
            Token::move(begin, end, tok);
            tok = fortok;
            end = fortok->next()->link();
        }
        //every 'for' is changed to 'for(;b;c), now it's possible to convert the 'for' to a 'while'.
        //precisely, 'for(;b;c){code}'-> 'while(b){code + c;}'
        fortok->str("while");
        begin = firstsemicolon->previous();
        begin->deleteNext();
        begin = secondsemicolon->previous();
        begin->deleteNext();
        begin = begin->next();
        if (begin->str() == ")") {        //'for(;b;)' -> 'while(b)'
            if (begin->previous()->str() == "(")    //'for(;;)' -> 'while(true)'
                begin->previous()->insertToken("true");
            tok = fortok;
            continue;
        }
        if (end->next()->str() =="{") {
            tok = end->next()->link()->previous();
            tok->insertToken(";");
        } else {
            tok = end;
            if (end->next()->str() != ";")
                tok->insertToken(";");
            tok->insertToken("{");
            tok = tok->tokAt(2);
            tok->insertToken("}");
            Token::createMutualLinks(tok->previous(), tok->next());
            tok = tok->previous();
        }
        end = end->previous();
        Token::move(begin, end, tok);
        tok = fortok;
    }*/

    // The simplifyTemplates have inner loops
    if (_settings->terminated())
        return false;

    simplifyConst();

    // struct simplification "struct S {} s; => struct S { } ; S s ;
    simplifyStructDecl();

    // struct initialization (must be used before simplifyVarDecl)
    simplifyStructInit();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // The simplifyTemplates have inner loops
    if (_settings->terminated())
        return false;

    // Split up variable declarations.
    simplifyVarDecl(false);

    // specify array size.. needed when arrays are split
    arraySize();

    // f(x=g())   =>   x=g(); f(x)
    simplifyAssignmentInFunctionCall();

    // x = ({ 123; });  =>   { x = 123; }
    simplifyAssignmentBlock();

    // The simplifyTemplates have inner loops
    if (_settings->terminated())
        return false;

    simplifyVariableMultipleAssign();

    // Simplify float casts (float)1 => 1.0
    simplifyFloatCasts();

    // Remove redundant parentheses
    simplifyRedundantParentheses();
    for (Token *tok = list.front(); tok; tok = tok->next())
        while (TemplateSimplifier::simplifyNumericCalculations(tok))
            ;

    // Handle templates..
    simplifyTemplates();

    // The simplifyTemplates have inner loops
    if (_settings->terminated())
        return false;

    // Simplify templates.. sometimes the "simplifyTemplates" fail and
    // then unsimplified function calls etc remain. These have the
    // "wrong" syntax. So this function will just fix so that the
    // syntax is corrected.
    TemplateSimplifier::cleanupAfterSimplify(list.front());

    // Simplify the operator "?:"
    simplifyConditionOperator();

    // remove exception specifications..
    removeExceptionSpecifications();

    // Collapse operator name tokens into single token
    // operator = => operator=
    simplifyOperatorName();

    // Simplify pointer to standard types (C only)
    simplifyPointerToStandardType();

    // simplify function pointers
    simplifyFunctionPointers();

    // "if (not p)" => "if (!p)"
    // "if (p and q)" => "if (p && q)"
    // "if (p or q)" => "if (p || q)"
    while (simplifyLogicalOperators()) { }

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    if (m_timerResults) {
        Timer t("Tokenizer::tokenize::setVarId", _settings->_showtime, m_timerResults);
        setVarId();
    } else {
        setVarId();
    }

    // The simplify enum might have inner loops
    if (_settings->terminated())
        return false;

    // Add std:: in front of std classes, when using namespace std; was given
    simplifyNamespaceStd();

    createLinks2();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Convert e.g. atol("0") into 0
    while (simplifyMathFunctions()) {};

    simplifyDoublePlusAndDoubleMinus();

    simplifyArrayAccessSyntax();

    list.front()->assignProgressValues();

    removeRedundantSemicolons();

    simplifyParameterVoid();

    simplifyRedundantConsecutiveBraces();

    simplifyEmptyNamespaces();

    if (!validate())
        return false;

    createSymbolDatabase();

    // Use symbol database to identify rvalue references. Split && to & &. This is safe, since it doesn't delete any tokens (which might be referenced by symbol database)
    for (std::size_t i = 0; i < _symbolDatabase->getVariableListSize(); i++) {
        const Variable* var = _symbolDatabase->getVariableFromVarId(i);
        if (var && var->isRValueReference()) {
            const_cast<Token*>(var->typeEndToken())->str("&");
            const_cast<Token*>(var->typeEndToken())->insertToken("&");
            const_cast<Token*>(var->typeEndToken()->next())->scope(var->typeEndToken()->scope());
        }
    }

    list.createAst();

    return true;
}
//---------------------------------------------------------------------------

bool Tokenizer::tokenizeCondition(const std::string &code)
{
    assert(_settings);

    // Fill the map _typeSize..
    fillTypeSizes();

    {
        std::istringstream istr(code);
        if (!list.createTokens(istr, "")) {
            cppcheckError(0);
            return false;
        }
    }

    // Combine strings
    combineStrings();

    // Remove "volatile", "inline", "register", and "restrict"
    simplifyKeyword();

    // convert platform dependent types to standard types
    // 32 bits: size_t -> unsigned long
    // 64 bits: size_t -> unsigned long long
    simplifyPlatformTypes();

    // collapse compound standard types into a single token
    // unsigned long long int => long _isUnsigned=true,_isLong=true
    simplifyStdType();

    // Concatenate double sharp: 'a ## b' -> 'ab'
    concatenateDoubleSharp();

    if (!createLinks()) {
        // Source has syntax errors, can't proceed
        return false;
    }

    // replace 'NULL' and similar '0'-defined macros with '0'
    simplifyNull();

    // replace 'sin(0)' to '0' and other similar math expressions
    simplifyMathExpressions();

    // combine "- %num%"
    concatenateNegativeNumberAndAnyPositive();

    // simplify simple calculations
    for (Token *tok = list.front() ? list.front()->next() : NULL;
         tok;
         tok = tok->next()) {
        if (tok->isNumber())
            TemplateSimplifier::simplifyNumericCalculations(tok->previous());
    }

    combineOperators();

    simplifyRedundantParentheses();
    for (Token *tok = list.front();
         tok;
         tok = tok->next())
        while (TemplateSimplifier::simplifyNumericCalculations(tok))
            ;

    while (simplifyLogicalOperators()) { }

    // Convert e.g. atol("0") into 0
    while (simplifyMathFunctions()) {};

    simplifyDoublePlusAndDoubleMinus();

    return true;
}

bool Tokenizer::hasComplicatedSyntaxErrorsInTemplates()
{
    const Token *tok = TemplateSimplifier::hasComplicatedSyntaxErrorsInTemplates(list.front());
    if (tok) {
        syntaxError(tok);
        return true;
    }

    return false;
}

bool Tokenizer::hasEnumsWithTypedef()
{
    for (const Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "enum %var% {")) {
            tok = tok->tokAt(2);
            const Token *tok2 = Token::findsimplematch(tok, "typedef", tok->link());
            if (tok2) {
                syntaxError(tok2);
                return true;
            }
        }
    }

    return false;
}

void Tokenizer::fillTypeSizes()
{
    _typeSize.clear();
    _typeSize["char"] = 1;
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

            // replace "->" with "."
            else if (c1 == '-' && c2 == '>') {
                tok->str(".");
                tok->deleteNext();
                continue;
            }
        }

        else if (tok->str() == ">>" && tok->next()->str() == "=") {
            tok->str(">>=");
            tok->deleteNext();
        }

        else if (tok->str() == "<<" && tok->next()->str() == "=") {
            tok->str("<<=");
            tok->deleteNext();
        }

        else if ((c1 == 'p' || c1 == '_') && tok->next()->str() == ":" && tok->strAt(2) != ":") {
            if (tok->str() == "private" || tok->str() == "protected" || tok->str() == "public" || tok->str() == "__published") {
                tok->str(tok->str() + ":");
                tok->deleteNext();
                continue;
            }
        }
    }
}

void Tokenizer::combineStrings()
{
    // Combine wide strings
    for (Token *tok = list.front();
         tok;
         tok = tok->next()) {
        while (tok->str() == "L" && tok->next() && tok->next()->type() == Token::eString) {
            // Combine 'L "string"'
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
        while (tok->next() && tok->next()->type() == Token::eString) {
            tok->next()->str(simplifyString(tok->next()->str()));

            // Two strings after each other, combine them
            tok->concatStr(tok->next()->str());
            tok->deleteNext();
        }
    }
}

void Tokenizer::concatenateDoubleSharp()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "%num%|%var% ## %num%|%var%")) {
            tok->str(tok->str() + tok->strAt(2));
            tok->deleteNext(2);
        }
    }
}

void Tokenizer::simplifyFileAndLineMacro()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "__FILE__")
            tok->str("\"" + list.file(tok) + "\"");
        else if (tok->str() == "__LINE__")
            tok->str(MathLib::toString(tok->linenr()));
    }
}

void Tokenizer::simplifyNull()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "NULL" && !Token::Match(tok->previous(), "[(,] NULL [,)]"))
            tok->str("0");
        else if (tok->str() == "__null" || tok->str() == "'\\0'" || tok->str() == "'\\x0'")
            tok->str("0");
        else if (tok->isNumber() &&
                 MathLib::isInt(tok->str()) &&
                 MathLib::toLongNumber(tok->str()) == 0)
            tok->str("0");
    }

    // nullptr..
    if (isCPP() && _settings->standards.cpp == Standards::CPP11) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (tok->str() == "nullptr")
                tok->str("0");
        }
    }
}

void Tokenizer::concatenateNegativeNumberAndAnyPositive()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "?|:|,|(|[|{|return|case|sizeof|%op% +|-") || tok->type() == Token::eIncDecOp)
            continue;
        if (tok->next()->str() == "+")
            tok->deleteNext();
        else if (Token::Match(tok->next(), "- %num%")) {
            tok->deleteNext();
            tok->next()->str("-" + tok->next()->str());
        }
    }
}

void Tokenizer::simplifyExternC()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "extern \"C\" {|")) {
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
        while (Token::Match(tok, "[;{}] ( {") &&
               Token::simpleMatch(tok->linkAt(2), "} ) ;")) {
            tok->linkAt(2)->previous()->deleteNext(3);
            tok->deleteNext(2);
        }
        if (Token::Match(tok, "( { %bool%|%char%|%num%|%str%|%var% ; } )")) {
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

void Tokenizer::simplifyDebugNew()
{
    // convert Microsoft DEBUG_NEW macro to new
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "DEBUG_NEW")
            tok->str("new");
    }
}

void Tokenizer::simplifyArrayAccessSyntax()
{
    // 0[a] -> a[0]
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%num% [ %var% ]")) {
            const std::string temp = tok->str();
            tok->str(tok->strAt(2));
            tok->tokAt(2)->str(temp);
        }
    }
}

void Tokenizer::simplifyParameterVoid()
{
    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% ( void )"))
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
    // Convert + + into + and + - into -
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (tok->next()) {
            if (tok->str() == "+") {
                if (tok->next()->str() == "+") {
                    tok->deleteNext();
                    continue;
                } else if (tok->next()->str()[0] == '-') {
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
                if (tok->next()->str() == "+") {
                    tok->deleteNext();
                    continue;
                } else if (tok->next()->str()[0] == '-') {
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
    bool addlength = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% [ ] = { %str% } ;")) {
            Token *t = tok->tokAt(3);
            t->deleteNext();
            t->next()->deleteNext();
            addlength = true;
        }

        if (addlength || Token::Match(tok, "%var% [ ] = %str% ;")) {
            tok = tok->next();
            std::size_t sz = tok->strAt(3).length() - 1;
            tok->insertToken(MathLib::toString((unsigned int)sz));
            addlength = false;
            tok = tok->tokAt(5);
        }

        else if (Token::Match(tok, "%var% [ ] = {")) {
            unsigned int sz = 1;
            tok = tok->next();
            Token *end = tok->linkAt(3);
            for (Token *tok2 = tok->tokAt(4); tok2 && tok2 != end; tok2 = tok2->next()) {
                if (tok2->str() == "{" || tok2->str() == "(" || tok2->str() == "[")
                    tok2 = tok2->link();
                else if (tok2->str() == "<") { // Bailout. TODO: When link() supports <>, this bailout becomes unnecessary
                    sz = 0;
                    break;
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

static Token *skipTernaryOp(Token *);

static Token *skipTernaryOp(Token *tok)
{
    if (!tok || tok->str() != "?")
        return tok;
    unsigned int colonlevel = 1;
    while (NULL != (tok = tok->next())) {
        if (tok->str() == "?") {
            ++colonlevel;
        } else if (tok->str() == ":") {
            --colonlevel;
            if (colonlevel == 0) {
                tok = tok->next();
                break;
            }
        }
        if (Token::Match(tok->next(), "[{};]"))
            break;
    }
    return tok;
}

/** simplify labels and case|default in the code: add a ";" if not already in.*/

bool Tokenizer::simplifyLabelsCaseDefault()
{
    bool executablescope = false;
    unsigned int indentlevel = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Simplify labels in the executable scope..
        if (Token::Match(tok, ") const| {")) {
            tok = tok->next();
            if (tok->str() != "{")
                tok = tok->next();
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
        } else if (tok->str() == "(" || tok->str() == "[")
            tok = tok->link();

        if (Token::Match(tok, "[;{}] case")) {
            while (NULL != (tok = tok->next())) {
                if (tok->str() == "(" || tok->str() == "[") {
                    tok = tok->link();
                } else if (tok->str() == "?") {
                    tok = skipTernaryOp(tok);
                }
                if (Token::Match(tok->next(),"[:{};]"))
                    break;
            }
            if (tok->str() != "case" && tok->next()->str() == ":") {
                tok = tok->next();
                if (tok->next()->str() != ";")
                    tok->insertToken(";");
            } else {
                syntaxError(tok);
                return false;
            }
        } else if (Token::Match(tok, "[;{}] %var% : !!;")) {
            tok = tok->tokAt(2);
            tok->insertToken(";");
        }
    }
    return true;
}




void Tokenizer::simplifyTemplates()
{
    if (isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // #2648 - simple fix for sizeof used as template parameter
        // TODO: this is a bit hardcoded. make a bit more generic
        if (Token::Match(tok, "%var% < sizeof ( %type% ) >") && tok->tokAt(4)->isStandardType()) {
            Token * const tok3 = tok->next();
            const unsigned int sizeOfResult = sizeOfType(tok3->tokAt(3));
            tok3->deleteNext(4);
            tok3->insertToken(MathLib::toString(sizeOfResult));
        }
    }

    TemplateSimplifier::simplifyTemplates(
        list,
        *_errorLogger,
        _settings,
        _codeWithTemplates);
}
//---------------------------------------------------------------------------


static bool setVarIdParseDeclaration(const Token **tok, const std::map<std::string,unsigned int> &variableId, bool executableScope)
{
    const Token *tok2 = *tok;

    bool ref = false;

    if (!tok2->isName())
        return false;

    unsigned int typeCount = 0;
    bool hasstruct = false;   // Is there a "struct" or "class"?
    while (tok2) {
        if (tok2->isName()) {
            if (tok2->str() == "class" || tok2->str() == "struct" || tok2->str() == "union" || tok2->str() == "typename") {
                hasstruct = true;
                typeCount = 0;
            } else if (tok2->str() == "const") {
                ;  // just skip "const"
            } else if (!hasstruct && variableId.find(tok2->str()) != variableId.end() && tok2->previous()->str() != "::") {
                ++typeCount;
                tok2 = tok2->next();
                if (!tok2 || tok2->str() != "::")
                    break;
            } else {
                ++typeCount;
            }
        } else if ((TemplateSimplifier::templateParameters(tok2) > 0) ||
                   Token::Match(tok2, "< >") /* Ticket #4764 */) {
            tok2 = tok2->findClosingBracket();
            if (!Token::Match(tok2, ">|>>"))
                break;
        } else if (tok2->str() == "&" || tok2->str() == "&&") {
            ref = true;
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
            if (tok2->str() == "(" || tok2->str() == "=")
                ;   // reference is assigned => ok
            else if (tok2->str() != ")" || tok2->link()->strAt(-1) != "catch")
                return false;   // not catching by reference => not declaration
        }
    }

    // Check if array declaration is valid (#2638)
    // invalid declaration: AAA a[4] = 0;
    if (typeCount >= 2 && tok2 && tok2->str() == "[") {
        const Token *tok3 = tok2;
        while (tok3 && tok3->str() == "[") {
            tok3 = tok3->link()->next();
        }
        if (Token::Match(tok3, "= %num%"))
            return false;
    }

    return bool(typeCount >= 2 && tok2 && Token::Match(tok2->tokAt(-2), "!!:: %type%"));
}


static void setVarIdStructMembers(Token **tok1,
                                  std::map<unsigned int, std::map<std::string,unsigned int> > *structMembers,
                                  unsigned int *_varId)
{
    Token *tok = *tok1;
    while (Token::Match(tok->next(), ". %var% !!(")) {
        const unsigned int struct_varid = tok->varId();
        tok = tok->tokAt(2);
        if (struct_varid == 0)
            continue;

        // Don't set varid for template function
        if (TemplateSimplifier::templateParameters(tok->next()) > 0)
            break;

        std::map<std::string,unsigned int>& members = (*structMembers)[struct_varid];
        if (members.empty() || members.find(tok->str()) == members.end()) {
            members[tok->str()] = ++(*_varId);
            tok->varId(*_varId);
        } else {
            tok->varId(members[tok->str()]);
        }
    }
    if (tok)
        *tok1 = tok;
}


static void setVarIdClassDeclaration(Token * const startToken,
                                     const std::map<std::string, unsigned int> &variableId,
                                     const unsigned int scopeStartVarId,
                                     std::map<unsigned int, std::map<std::string,unsigned int> > *structMembers,
                                     unsigned int *_varId)
{
    // end of scope
    const Token * const endToken = startToken->link();

    // determine class name
    std::string className;
    for (const Token *tok = startToken->previous(); tok; tok = tok->previous()) {
        if (!tok->isName() && tok->str() != ":")
            break;
        if (Token::Match(tok, "class|struct %type% [:{]")) {
            className = tok->next()->str();
            break;
        }
    }

    // replace varids..
    unsigned int indentlevel = 0;
    bool initList = false;
    for (Token *tok = startToken->next(); tok != endToken; tok = tok->next()) {
        if (tok->str() == "{") {
            initList = false;
            ++indentlevel;
        } else if (tok->str() == "}")
            --indentlevel;
        else if (initList && indentlevel == 0 && Token::Match(tok->previous(), "[,:] %var% (")) {
            const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
            if (it != variableId.end()) {
                tok->varId(it->second);
            }
        } else if (tok->isName() && tok->varId() <= scopeStartVarId) {
            if (indentlevel > 0) {
                if (Token::Match(tok->previous(), "::|."))
                    continue;
                if (tok->next()->str() == "::") {
                    if (tok->str() == className)
                        tok = tok->tokAt(2);
                    else
                        continue;
                }

                const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
                if (it != variableId.end()) {
                    tok->varId(it->second);
                    setVarIdStructMembers(&tok, structMembers, _varId);
                }
            }
        } else if (indentlevel == 0 && tok->str() == ":")
            initList = true;
    }
}



// Update the variable ids..
// Parse each function..
static void setVarIdClassFunction(Token * const startToken,
                                  const Token * const endToken,
                                  const std::map<std::string, unsigned int> &varlist,
                                  std::map<unsigned int, std::map<std::string,unsigned int> > *structMembers,
                                  unsigned int *_varId)
{
    for (Token *tok2 = startToken; tok2 && tok2 != endToken; tok2 = tok2->next()) {
        if (tok2->varId() == 0 && (tok2->previous()->str() != "." || tok2->strAt(-2) == "this")) {
            const std::map<std::string,unsigned int>::const_iterator it = varlist.find(tok2->str());
            if (it != varlist.end()) {
                tok2->varId(it->second);
                setVarIdStructMembers(&tok2, structMembers, _varId);
            }
        }
    }
}

static bool isInitList(const Token *tok)
{
    if (!Token::Match(tok, ") : %var% ("))
        return false;

    tok = tok->linkAt(3);
    while (Token::Match(tok, ") , %var% ("))
        tok = tok->linkAt(3);

    return Token::simpleMatch(tok, ") {");
}

void Tokenizer::setVarId()
{
    // Clear all variable ids
    for (Token *tok = list.front(); tok; tok = tok->next())
        tok->varId(0);

    // Variable declarations can't start with "return" etc.
    std::set<std::string> notstart;
    notstart.insert("goto");
    notstart.insert("NOT");
    notstart.insert("return");
    notstart.insert("sizeof");
    if (!isC()) {
        static const char *str[] = {"delete","friend","new","throw","using","virtual","explicit"};
        notstart.insert(str, str+(sizeof(str)/sizeof(*str)));
    }

    // variable id
    _varId = 0;
    std::map<std::string, unsigned int> variableId;
    std::map<unsigned int, std::map<std::string, unsigned int> > structMembers;
    std::stack< std::map<std::string, unsigned int> > scopeInfo;
    std::stack<bool> executableScope;
    executableScope.push(false);
    std::stack<unsigned int> scopestartvarid;  // varid when scope starts
    scopestartvarid.push(0);
    bool initlist = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {

        // scope info to handle shadow variables..
        if (!initlist && tok->str() == "(" &&
            (Token::simpleMatch(tok->link(), ") {") || Token::Match(tok->link(), ") %type% {") || isInitList(tok->link()))) {
            scopeInfo.push(variableId);
            initlist = Token::simpleMatch(tok->link(), ") :");

            // function declarations
        } else if (!executableScope.top() && tok->str() == "(" && Token::simpleMatch(tok->link(), ") ;")) {
            scopeInfo.push(variableId);
        } else if (!executableScope.top() && Token::simpleMatch(tok, ") ;")) {
            variableId.swap(scopeInfo.top());
            scopeInfo.pop();

        } else if (tok->str() == "{") {
            initlist = false;
            // parse anonymous unions as part of the current scope
            if (!(tok->strAt(-1) == "union" && Token::simpleMatch(tok->link(), "} ;"))) {
                scopestartvarid.push(_varId);
                if (tok->strAt(-1) == ")" || Token::Match(tok->tokAt(-2), ") %type%")) {
                    executableScope.push(true);
                } else {
                    executableScope.push(executableScope.top());
                    scopeInfo.push(variableId);
                }
            }
        } else if (tok->str() == "}") {
            // parse anonymous unions as part of the current scope
            if (!(Token::simpleMatch(tok, "} ;") && Token::simpleMatch(tok->link()->previous(), "union {"))) {
                // Set variable ids in class declaration..
                if (!isC() && !executableScope.top() && tok->link()) {
                    setVarIdClassDeclaration(tok->link(),
                                             variableId,
                                             scopestartvarid.top(),
                                             &structMembers,
                                             &_varId);
                }

                scopestartvarid.pop();
                if (scopestartvarid.empty()) {  // should be impossible
                    scopestartvarid.push(0);
                }

                if (scopeInfo.empty()) {
                    variableId.clear();
                } else {
                    variableId.swap(scopeInfo.top());
                    scopeInfo.pop();
                }

                executableScope.pop();
                if (executableScope.empty()) {   // should not possibly happen
                    executableScope.push(false);
                }
            }
        }

        if (tok == list.front() || Token::Match(tok, "[;{}]") ||
            (Token::Match(tok,"[(,]") && (!executableScope.top() || Token::simpleMatch(tok->link(), ") {"))) ||
            (tok->isName() && tok->str().at(tok->str().length()-1U) == ':')) {

            // No variable declarations in sizeof
            if (Token::simpleMatch(tok->previous(), "sizeof (")) {
                continue;
            }

            if (_settings->terminated())
                return;

            // locate the variable name..
            const Token *tok2 = (tok->isName()) ? tok : tok->next();

            // private: protected: public: etc
            while (tok2 && tok2->str()[tok2->str().size() - 1U] == ':') {
                tok2 = tok2->next();
            }
            if (!tok2)
                break;

            // Variable declaration can't start with "return", etc
            if (notstart.find(tok2->str()) != notstart.end())
                continue;

            const bool decl = setVarIdParseDeclaration(&tok2, variableId, executableScope.top());

            if (decl && Token::Match(tok2->previous(), "%type% [;[=,)]") && tok2->previous()->str() != "const") {
                variableId[tok2->previous()->str()] = ++_varId;
                tok = tok2->previous();
            }

            else if (decl && Token::Match(tok2->previous(), "%type% ( !!)") && Token::simpleMatch(tok2->link(), ") ;")) {
                // In C++ , a variable can't be called operator+ or something like that.
                if (isCPP() &&
                    tok2->previous()->str().size() >= 9 &&
                    tok2->previous()->str().compare(0, 8, "operator") == 0 &&
                    tok2->previous()->str()[8] != '_' &&
                    !std::isalnum(tok2->previous()->str()[8]))
                    continue;

                const Token *tok3 = tok2->next();
                if (!tok3->isStandardType() && tok3->str() != "void" && !Token::Match(tok3,"struct|union|class %type%") && tok3->str() != "." && !setVarIdParseDeclaration(&tok3,variableId,executableScope.top())) {
                    variableId[tok2->previous()->str()] = ++_varId;
                    tok = tok2->previous();
                }
            }
        }

        if (tok->isName()) {
            // don't set variable id after a struct|enum|union
            if (Token::Match(tok->previous(), "struct|enum|union"))
                continue;

            if (!isC()) {
                if (tok->previous() && tok->previous()->str() == "::")
                    continue;
                if (tok->next() && tok->next()->str() == "::")
                    continue;
            }

            const std::map<std::string, unsigned int>::const_iterator it = variableId.find(tok->str());
            if (it != variableId.end()) {
                tok->varId(it->second);
                setVarIdStructMembers(&tok, &structMembers, &_varId);
            }
        } else if (Token::Match(tok, "::|. %var%")) {
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

    // Clear the structMembers because it will be used when member functions
    // are parsed. The old info is not bad, it is just redundant.
    structMembers.clear();

    // Member functions and variables in this source
    std::list<Token *> allMemberFunctions;
    std::list<Token *> allMemberVars;
    {
        for (Token *tok2 = list.front(); tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "%var% :: %var%")) {
                if (tok2->strAt(3) == "(")
                    allMemberFunctions.push_back(tok2);
                else if (tok2->tokAt(2)->varId() != 0)
                    allMemberVars.push_back(tok2);
            }
        }
    }

    // class members..
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct %var% {|:")) {
            const std::string &classname(tok->next()->str());

            // What member variables are there in this class?
            std::map<std::string, unsigned int> varlist;
            const Token* tokStart = Token::findsimplematch(tok, "{");
            if (tokStart) {
                for (const Token *tok2 = tokStart->next(); tok2 != tokStart->link(); tok2 = tok2->next()) {
                    // skip parentheses..
                    if (tok2->str() == "{")
                        tok2 = tok2->link();
                    else if (tok2->str() == "(")
                        tok2 = tok2->link();

                    // Found a member variable..
                    else if (tok2->varId() > 0)
                        varlist[tok2->str()] = tok2->varId();
                }
            }

            // Are there any member variables in this class?
            if (varlist.empty())
                continue;

            // Member variables
            for (std::list<Token *>::iterator func = allMemberVars.begin(); func != allMemberVars.end(); ++func) {
                if (!Token::simpleMatch(*func, classname.c_str()))
                    continue;

                Token *tok2 = *func;
                tok2 = tok2->tokAt(2);
                tok2->varId(varlist[tok2->str()]);
            }

            // Set variable ids in member functions for this class..
            const std::string funcpattern(classname + " :: %var% (");
            for (std::list<Token *>::iterator func = allMemberFunctions.begin(); func != allMemberFunctions.end(); ++func) {
                Token *tok2 = *func;

                // Found a class function..
                if (Token::Match(tok2, funcpattern.c_str())) {
                    // Goto the end parentheses..
                    tok2 = tok2->linkAt(3);
                    if (!tok2)
                        break;

                    // If this is a function implementation.. add it to funclist
                    if (Token::Match(tok2, ") const|volatile| {")) {
                        while (tok2->str() != "{")
                            tok2 = tok2->next();
                        setVarIdClassFunction(tok2, tok2->link(), varlist, &structMembers, &_varId);
                    }

                    // constructor with initializer list
                    if (Token::Match(tok2, ") : %var% (")) {
                        Token *tok3 = tok2;
                        while (Token::Match(tok3, ") [:,] %var% (")) {
                            Token *vartok = tok3->tokAt(2);
                            if (varlist.find(vartok->str()) != varlist.end())
                                vartok->varId(varlist[vartok->str()]);
                            tok3 = tok3->linkAt(3);
                        }
                        if (Token::simpleMatch(tok3, ") {")) {
                            setVarIdClassFunction(tok2, tok3->next()->link(), varlist, &structMembers, &_varId);
                        }
                    }
                }
            }
        }
    }
}

static bool linkBrackets(Tokenizer* tokenizer, std::stack<const Token*>& type, std::stack<Token*>& links, Token* token, char open, char close)
{
    if (token->str()[0] == open) {
        links.push(token);
        type.push(token);
    } else if (token->str()[0] == close) {
        if (links.empty()) {
            // Error, { and } don't match.
            tokenizer->syntaxError(token, open);
            return false;
        }
        if (type.top()->str()[0] != open) {
            tokenizer->syntaxError(type.top(), type.top()->str()[0]);
            return false;
        }
        type.pop();

        Token::createMutualLinks(links.top(), token);
        links.pop();
    }
    return (true);
}

bool Tokenizer::createLinks()
{
    std::stack<const Token*> type;
    std::stack<Token*> links1;
    std::stack<Token*> links2;
    std::stack<Token*> links3;
    for (Token *token = list.front(); token; token = token->next()) {
        if (token->link()) {
            token->link(0);
        }

        bool validSyntax = linkBrackets(this, type, links1, token, '{', '}');
        if (!validSyntax)
            return false;

        validSyntax = linkBrackets(this, type, links2, token, '(', ')');
        if (!validSyntax)
            return false;

        validSyntax = linkBrackets(this, type, links3, token, '[', ']');
        if (!validSyntax)
            return false;
    }

    if (!links1.empty()) {
        // Error, { and } don't match.
        syntaxError(links1.top(), '{');
        return false;
    }

    if (!links2.empty()) {
        // Error, ( and ) don't match.
        syntaxError(links2.top(), '(');
        return false;
    }

    if (!links3.empty()) {
        // Error, [ and ] don't match.
        syntaxError(links3.top(), '[');
        return false;
    }

    return true;
}

void Tokenizer::createLinks2()
{
    if (isC())
        return;

    std::stack<Token*> type;
    for (Token *token = list.front(); token; token = token->next()) {
        if (token->link()) {
            if (Token::Match(token, "{|[|("))
                type.push(token);
            else if (!type.empty() && Token::Match(token, "}|]|)")) {
                while (type.top()->str() == "<")
                    type.pop();
                type.pop();
            } else
                token->link(0);
        }

        else if (token->str() == ";")
            while (!type.empty() && type.top()->str() == "<")
                type.pop();
        else if (token->str() == "<" && token->previous() && token->previous()->isName() && !token->previous()->varId())
            type.push(token);
        else if (token->str() == ">" || token->str() == ">>") {
            if (type.empty() || type.top()->str() != "<") // < and > don't match.
                continue;
            if (token->next() && !Token::Match(token->next(), "%var%|>|&|*|::|,|(|)|{"))
                continue;

            // Check type of open link
            if (type.empty() || type.top()->str() != "<" || (token->str() == ">>" && type.size() < 2)) {
                continue;
            }

            Token* top = type.top();
            type.pop();
            if (token->str() == ">>" && type.top()->str() != "<") {
                type.push(top);
                continue;
            }

            if (token->str() == ">>") { // C++11 right angle bracket
                token->str(">");
                token->insertToken(">");
            }

            Token::createMutualLinks(top, token);
        }
    }
}

void Tokenizer::sizeofAddParentheses()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "sizeof %var%")) {
            Token *tempToken = tok->next();
            while (Token::Match(tempToken, "%var%")) {
                while (tempToken && tempToken->next() && tempToken->next()->str() == "[")
                    tempToken = tempToken->next()->link();
                if (!tempToken || !tempToken->next())
                    break;

                if (Token::Match(tempToken->next(), ". %var%")) {
                    // We are checking a class or struct, search next varname
                    tempToken = tempToken->tokAt(2);
                    continue;
                } else if (tempToken->next()->type() == Token::eIncDecOp) {
                    // We have variable++ or variable--, the sizeof argument
                    // ends after the op
                    tempToken = tempToken->next();
                } else if (Token::Match(tempToken->next(), "[),;}]")) {
                    ;
                } else {
                    break;
                }

                // Ok. Add ( after sizeof and ) after tempToken
                tok->insertToken("(");
                tempToken->insertToken(")");
                Token::createMutualLinks(tok->next(), tempToken->next());
                break;
            }
        }
    }

}

bool Tokenizer::simplifySizeof()
{
    // Locate variable declarations and calculate the size
    std::map<unsigned int, std::string> sizeOfVar;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->varId() != 0 && sizeOfVar.find(tok->varId()) == sizeOfVar.end()) {
            const unsigned int varId = tok->varId();
            if (Token::Match(tok->tokAt(-3), "[;{}(,] %type% * %var% [;,)]") ||
                Token::Match(tok->tokAt(-4), "[;{}(,] const %type% * %var% [;),]") ||
                Token::Match(tok->tokAt(-2), "[;{}(,] %type% %var% [;),]") ||
                Token::Match(tok->tokAt(-3), "[;{}(,] const %type% %var% [;),]")) {
                const unsigned int size = sizeOfType(tok->previous());
                if (size == 0) {
                    continue;
                }

                sizeOfVar[varId] = MathLib::toString(size);
            }

            else if (Token::Match(tok->previous(), "%type% %var% [ %num% ] [;=]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [;=]")) {
                const unsigned int size = sizeOfType(tok->previous());
                if (size == 0)
                    continue;

                sizeOfVar[varId] = MathLib::toString(size * static_cast<unsigned long>(MathLib::toLongNumber(tok->strAt(2))));
            }

            else if (Token::Match(tok->previous(), "%type% %var% [ %num% ] [,)]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [,)]")) {
                Token tempTok(0);
                tempTok.str("*");
                sizeOfVar[varId] = MathLib::toString(sizeOfType(&tempTok));
            }
        }
    }

    bool ret = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "sizeof")
            continue;

        if (!tok->next())
            break;

        if (tok->strAt(1) == "sizeof")
            continue;

        if (Token::simpleMatch(tok->next(), ". . .")) {
            tok->deleteNext(3);
        }

        // sizeof 'x'
        if (tok->next()->type() == Token::eChar) {
            tok->deleteThis();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            ret = true;
            continue;
        }

        // sizeof('x')
        if (Token::Match(tok, "sizeof ( %char% )")) {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            ret = true;
            continue;
        }

        // sizeof "text"
        if (tok->next()->type() == Token::eString) {
            tok->deleteThis();
            std::ostringstream ostr;
            ostr << (Token::getStrLength(tok) + 1);
            tok->str(ostr.str());
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

        // sizeof * (...) -> sizeof(*...)
        if (Token::simpleMatch(tok->next(), "* (") && !Token::simpleMatch(tok->linkAt(2), ") .")) {
            tok->deleteNext();
            tok->next()->insertToken("*");
        }

        // sizeof a++ -> sizeof(a++)
        if (Token::Match(tok->next(), "++|-- %var% !!.") || Token::Match(tok->next(), "%var% ++|--")) {
            tok->insertToken("(");
            tok->tokAt(3)->insertToken(")");
            Token::createMutualLinks(tok->next(), tok->tokAt(4));
        }

        // sizeof 1 => sizeof ( 1 )
        if (tok->next()->isNumber()) {
            Token *tok2 = tok->next();
            tok->insertToken("(");
            tok2->insertToken(")");
            Token::createMutualLinks(tok->next(), tok2->next());
        }

        // sizeof int -> sizeof( int )
        else if (tok->next()->str() != "(") {
            // Add parentheses around the sizeof
            int parlevel = 0;
            for (Token *tempToken = tok->next(); tempToken; tempToken = tempToken->next()) {
                if (tempToken->str() == "(")
                    ++parlevel;
                else if (tempToken->str() == ")") {
                    --parlevel;
                    if (parlevel == 0 && !Token::Match(tempToken, ") . %var%")) {
                        // Ok, we should be clean. Add ) after tempToken
                        tok->insertToken("(");
                        tempToken->insertToken(")");
                        Token::createMutualLinks(tok->next(), tempToken->next());
                        break;
                    }
                }
                if (parlevel == 0 && Token::Match(tempToken, "%var%")) {
                    while (tempToken && tempToken->next() && tempToken->next()->str() == "[") {
                        tempToken = tempToken->next()->link();
                    }
                    if (!tempToken || !tempToken->next()) {
                        break;
                    }

                    if (tempToken->next()->str() == ".") {
                        // We are checking a class or struct, search next varname
                        tempToken = tempToken->next();
                        continue;
                    } else if (tempToken->next()->type() == Token::eIncDecOp) {
                        // We have variable++ or variable--, there should be
                        // nothing after this
                        tempToken = tempToken->tokAt(2);
                    } else if (parlevel > 0 && Token::simpleMatch(tempToken->next(), ") .")) {
                        --parlevel;
                        tempToken = tempToken->tokAt(2);
                        continue;
                    }

                    // Ok, we should be clean. Add ) after tempToken
                    tok->insertToken("(");
                    tempToken->insertToken(")");
                    Token::createMutualLinks(tok->next(), tempToken->next());
                    break;
                }
            }
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
        else if (Token::Match(tok->next(), "( %var% )") && tok->tokAt(2)->varId() != 0) {
            if (sizeOfVar.find(tok->tokAt(2)->varId()) != sizeOfVar.end()) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
                tok->str(sizeOfVar[tok->varId()]);
                ret = true;
            } else {
                // don't try to replace size of variable if variable has
                // similar name with type (#329)
            }
        }

        else if (Token::Match(tok, "sizeof ( %type% )")) {
            const unsigned int size = sizeOfType(tok->tokAt(2));
            if (size > 0) {
                tok->str(MathLib::toString(size));
                tok->deleteNext(3);
                ret = true;
            }
        }

        else if (Token::Match(tok, "sizeof ( * %var% )") || Token::Match(tok, "sizeof ( %var% [ %num% ] )")) {
            // Some default value..
            std::size_t sz = 0;

            const unsigned int varid = tok->tokAt((tok->strAt(2) == "*") ? 3 : 2)->varId();
            if (varid != 0) {
                // Try to locate variable declaration..
                const Token *decltok = Token::findmatch(list.front(), "%varid%", varid);
                if (Token::Match(decltok->previous(), "%type% %var% [")) {
                    sz = sizeOfType(decltok->previous());
                } else if (Token::Match(decltok->previous(), "* %var% [")) {
                    sz = sizeOfType(decltok->previous());
                } else if (Token::Match(decltok->tokAt(-2), "%type% * %var%")) {
                    sz = sizeOfType(decltok->tokAt(-2));
                }
            } else if (tok->strAt(3) == "[" && tok->tokAt(2)->isStandardType()) {
                sz = sizeOfType(tok->tokAt(2));
                if (sz == 0)
                    continue;
                sz *= static_cast<unsigned long>(MathLib::toLongNumber(tok->strAt(4)));
            }

            if (sz > 0) {
                tok->str(MathLib::toString(sz));
                Token::eraseTokens(tok, tok->next()->link()->next());
                ret = true;
            }
        }
    }
    return ret;
}

bool Tokenizer::simplifyTokenList()
{
    // clear the _functionList so it can't contain dead pointers
    deleteSymbolDatabase();

    // Experimental AST handling.
    for (Token *tok = list.front(); tok; tok = tok->next())
        tok->clearAst();

    simplifyCharAt();

    // simplify references
    simplifyReference();

    simplifyStd();

    simplifyGoto();

    if (_settings->terminated())
        return false;

    simplifySizeof();

    simplifyUndefinedSizeArray();

    simplifyCasts();

    // Simplify simple calculations before replace constants, this allows the replacement of constants that are calculated
    // e.g. const static int value = sizeof(X)/sizeof(Y);
    simplifyCalculations();

    // Replace constants..
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "const static| %type% %var% = %num% ;") ||
            Token::Match(tok, "const static| %type% %var% ( %num% ) ;")) {
            unsigned int offset = 0;
            if (tok->strAt(1) == "static")
                offset = 1;
            const unsigned int varId(tok->tokAt(2 + offset)->varId());
            if (varId == 0) {
                tok = tok->tokAt(5 + offset);
                continue;
            }

            const std::string& num = tok->strAt(4 + offset);
            int indent = 1;
            for (Token *tok2 = tok->tokAt(6); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{") {
                    ++indent;
                } else if (tok2->str() == "}") {
                    --indent;
                    if (indent == 0)
                        break;
                }

                // Compare constants, but don't touch members of other structures
                else if (tok2->varId() == varId) {
                    tok2->str(num);
                }
            }
        }
    }

    if (_settings->terminated())
        return false;

    // Simplify simple calculations..
    simplifyCalculations();

    // Replace "*(ptr + num)" => "ptr[num]"
    simplifyOffsetPointerDereference();

    // Replace "&str[num]" => "(str + num)"
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
        if (!Token::Match(tok, "%num%|%var%") && !Token::Match(tok, "]|)") &&
            (Token::Match(tok->next(), "& %var% [ %num%|%var% ]"))) {
            tok = tok->next();

            if (tok->next()->varId()) {
                if (pod.find(tok->next()->varId()) == pod.end()) {
                    tok = tok->tokAt(5);
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

    removeRedundantAssignment();

    simplifyRealloc();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Simplify variable declarations
    simplifyVarDecl(false);

    elseif();
    simplifyErrNoInWhile();
    simplifyIfAndWhileAssign();
    simplifyRedundantParentheses();
    simplifyIfNot();
    simplifyIfNotNull();
    simplifyIfSameInnerCondition();
    simplifyComparisonOrder();
    simplifyNestedStrcat();
    simplifyWhile0();
    simplifyFuncInWhile();

    simplifyIfAndWhileAssign(); // Could be affected by simplifyIfNot

    bool modified = true;
    while (modified) {
        if (_settings->terminated())
            return false;

        modified = false;
        modified |= simplifyConditions();
        modified |= simplifyFunctionReturn();
        modified |= simplifyKnownVariables();

        // replace strlen(str)
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "strlen ( %str% )")) {
                tok->str(MathLib::toString(Token::getStrLength(tok->tokAt(2))));
                tok->deleteNext(3);
                modified = true;
            }
        }

        modified |= removeRedundantConditions();
        modified |= simplifyRedundantParentheses();
        modified |= simplifyConstTernaryOp();
        modified |= simplifyCalculations();
    }

    simplifyConditionOperator();

    // simplify redundant for
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

    while (simplifyMathFunctions()) {};

    if (!validate())
        return false;

    list.front()->assignProgressValues();

    // Create symbol database and then remove const keywords
    createSymbolDatabase();
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "* const"))
            tok->deleteNext();
    }

    list.createAst();

    if (_settings->terminated())
        return false;

    if (_settings->debug) {
        list.front()->printOut(0, list.getFiles());

        if (_settings->_verbose)
            _symbolDatabase->printOut("Symbol database");

        list.front()->printAst();
    }

    if (_settings->debugwarnings) {
        printUnknownTypes();

        // #5054 - the typeStartToken() should come before typeEndToken()
        for (const Token *tok = tokens(); tok; tok = tok->next()) {
            if (tok->varId() == 0U)
                continue;

            const Variable *var = tok->variable();
            if (!var)
                continue;

            const Token * typetok = var->typeStartToken();
            while (typetok && typetok != var->typeEndToken())
                typetok = typetok->next();

            if (typetok != var->typeEndToken()) {
                reportError(tok,
                            Severity::debug,
                            "debug",
                            "Variable::typeStartToken() is not located before Variable::typeEndToken(). The location of the typeStartToken() is '" + var->typeStartToken()->str() + "' at line " + MathLib::toString(var->typeStartToken()->linenr()));
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------

void Tokenizer::removeMacrosInGlobalScope()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::Match(tok, ") %type% {") &&
                !Token::Match(tok->next(), "const|namespace|class|struct|union"))
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
            if (Token::Match(tok2, "%type% (") && Token::Match(tok2->next()->link(), ") const| {")) {
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

        if (tok->str() == "{")
            tok = tok->link();
    }
}
//---------------------------------------------------------------------------

void Tokenizer::removeMacroInVarDecl()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %var% (") && tok->next()->isUpperCaseName()) {
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
            tok2 = tok2 ? tok2->next() : NULL;

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

        if (Token::Match(tok, ") const| {")) {
            // parse in this function..
            std::set<unsigned int> localvars;
            if (tok->next()->str() == "const")
                tok = tok->next();
            const Token * const end = tok->next()->link();
            for (Token *tok2 = tok->next(); tok2 && tok2 != end; tok2 = tok2->next()) {
                // skip local class or struct
                if (Token::Match(tok2, "class|struct %type% {|:")) {
                    // skip to '{'
                    while (tok2 && tok2->str() != "{")
                        tok2 = tok2->next();

                    if (tok2)
                        tok2 = tok2->link(); // skip local class or struct
                    else
                        return;
                } else if (Token::Match(tok2, "[;{}] %type% * %var% ;") && tok2->next()->str() != "return") {
                    tok2 = tok2->tokAt(3);
                    localvars.insert(tok2->varId());
                } else if (Token::Match(tok2, "[;{}] %type% %var% ;") && tok2->next()->isStandardType()) {
                    tok2 = tok2->tokAt(2);
                    localvars.insert(tok2->varId());
                } else if (tok2->varId() &&
                           !Token::Match(tok2->previous(), "[;{}] %var% = %char%|%num%|%var% ;")) {
                    localvars.erase(tok2->varId());
                }
            }
            localvars.erase(0);
            if (!localvars.empty()) {
                for (Token *tok2 = tok->next(); tok2 && tok2 != end;) {
                    if (Token::Match(tok2, "[;{}] %type% %var% ;") && localvars.find(tok2->tokAt(2)->varId()) != localvars.end()) {
                        tok2->deleteNext(3);
                    } else if ((Token::Match(tok2, "[;{}] %type% * %var% ;") &&
                                localvars.find(tok2->tokAt(3)->varId()) != localvars.end()) ||
                               (Token::Match(tok2, "[;{}] %var% = %any% ;") &&
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
        if (tok->str() == "(" || tok->str() == "[" ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "="))
            tok = tok->link();
        else if (Token::Match(tok, "[;{}] %var% = realloc (")) {
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
    for (Token *tok = list.front(); tok; tok = tok ? tok->next() : NULL) {
        if (goback) {
            tok = tok->previous();
            goback = false;
        }
        if (tok->str() == "(" || tok->str() == "[" || tok->str() == "{") {
            tok = tok->link();
            continue;
        }
        if (!Token::Match(tok, "namespace %var% {"))
            continue;
        if (tok->strAt(3) == "}") {
            tok->deleteNext(3);             // remove '%var% { }'
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

        if (begin->str() == "(" || begin->str() == "[" ||
            (begin->str() == "{" && begin->previous() && begin->strAt(-1) == "="))
            begin = begin->link();

        //function scope
        if (!Token::simpleMatch(begin, ") {") && !Token::Match(begin, ") %var% {"))
            continue;

        Token *end = begin->linkAt(1+(begin->next()->str() == "{" ? 0 : 1));
        unsigned int indentlevel = 0;
        bool stilldead = false;

        for (Token *tok = begin; tok != end; tok = tok->next()) {
            if (tok->str() == "(" || tok->str() == "[") {
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
                       Token::Match(tok->previous(), "[;{}] exit (") ||
                       (Token::Match(tok->previous(), "[;{}] %var% (") &&
                        _settings->library.isnoreturn(tok->str())) ||
                       (tok->str() == "throw" && !isC())) {
                //TODO: ensure that we exclude user-defined 'exit|abort|throw', except for 'noreturn'
                //catch the first ';'
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "(" || tok2->str() == "[") {
                        tok2 = tok2->link();
                    } else if (tok2->str() == ";") {
                        tok = tok2;
                        eraseDeadCode(tok, 0);
                        break;
                    } else if (Token::Match(tok2, "[{}]"))
                        break;  //Wrong code.
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
        if (tok->str() != "if")
            continue;

        if (!Token::Match(tok->next(), "( %bool% ) {"))
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
        if (Token::Match(tok, "[;{}] for ( %var% = %num% ; %var% < %num% ; ++| %var% ++| ) {")) {
            // Same variable name..
            const std::string varname(tok->strAt(3));
            const unsigned int varid(tok->tokAt(3)->varId());
            if (varname != tok->strAt(7))
                continue;
            const Token *vartok = tok->tokAt(11);
            if (vartok->str() == "++")
                vartok = vartok->next();
            if (varname != vartok->str())
                continue;

            // Check that the difference of the numeric values is 1
            const MathLib::bigint num1(MathLib::toLongNumber(tok->strAt(5)));
            const MathLib::bigint num2(MathLib::toLongNumber(tok->strAt(9)));
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
                // remove "for ("
                tok->deleteNext(2);

                // If loop variable is read then keep assignment before
                // loop body..
                if (read) {
                    // goto ";"
                    tok = tok->tokAt(4);
                } else {
                    // remove "x = 0 ;"
                    tok->deleteNext(4);
                }

                // remove "x < 1 ; x ++ )"
                tok->deleteNext(7);

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


void Tokenizer::removeRedundantSemicolons()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
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
        if (tokPossibleDo)
            tokPossibleDo=tokPossibleDo->previous();
        if (!tokPossibleDo ||
            tokPossibleDo->str()!="do")
            tokEnd=simplifyAddBracesPair(tok,true);
    } else if (tok->str()=="do") {
        tokEnd=simplifyAddBracesPair(tok,false);
        if (tokEnd!=tok) {
            // walk on to next token, i.e. "while"
            // such that simplifyAddBracesPair does not close other braces
            // before the "while"
            if (tokEnd) {
                tokEnd=tokEnd->next();
                if (!tokEnd) {
                    // no while return input token
                    syntaxError(tok);
                    return NULL;
                }
            }
        }
    } else if (tok->str()=="if") {
        tokEnd=simplifyAddBracesPair(tok,true);
        if (!tokEnd)
            return NULL;
        Token * tokEndNext=tokEnd->next();
        if (tokEndNext && tokEndNext->str()=="else") {
            Token * tokEndNextNext=tokEndNext->next();
            if (tokEndNextNext && tokEndNextNext->str()=="if") {
                // do not change "else if ..." to "else { if ... }"
                tokEnd=simplifyAddBracesToCommand(tokEndNextNext);
            } else if (Token::simpleMatch(tokEndNext, "else }")) {
                syntaxError(tokEndNext);
                return NULL;
            } else
                tokEnd=simplifyAddBracesPair(tokEndNext,false);
        }
    }

    return tokEnd;
}

Token *Tokenizer::simplifyAddBracesPair(Token *tok, bool commandWithCondition)
{
    Token * tokCondition=tok->next();
    Token *tokAfterCondition=tokCondition;
    if (commandWithCondition) {
        if (!tokCondition) {
            // Missing condition
            return tok;
        }
        if (tokCondition->str()=="(")
            tokAfterCondition=tokCondition->link();
        else
            tokAfterCondition=NULL;
        if (!tokAfterCondition) {
            // Bad condition
            syntaxError(tok);
            return NULL;
        }
        tokAfterCondition=tokAfterCondition->next();
    }
    if (!tokAfterCondition ||
        ((tokAfterCondition->type()==Token::eBracket ||
          tokAfterCondition->type()==Token::eExtendedOp)&&
         Token::Match(tokAfterCondition,")|}|>|,"))) {
        // No tokens left where to add braces around
        return tok;
    }
    Token * tokBracesEnd=NULL;
    if (tokAfterCondition->str()=="{") {
        // already surrounded by braces
        tokBracesEnd=tokAfterCondition->link();
    } else if (Token::Match(tokAfterCondition, "try {") &&
               Token::Match(tokAfterCondition->linkAt(1), "} catch (")) {
        tokAfterCondition->previous()->insertToken("{");
        Token * tokOpenBrace = tokAfterCondition->previous();
        Token * tokEnd = tokAfterCondition->linkAt(1)->linkAt(2)->linkAt(1);

        tokEnd->insertToken("}");
        Token * tokCloseBrace = tokEnd->next();

        Token::createMutualLinks(tokOpenBrace, tokCloseBrace);
        tokBracesEnd = tokCloseBrace;
    } else {
        Token * tokEnd = simplifyAddBracesToCommand(tokAfterCondition);
        if (!tokEnd) // Ticket #4887
            return tok;
        if (tokEnd->str()!="}") {
            // Token does not end with brace
            // Look for ; to add own closing brace after it
            while (tokEnd &&
                   tokEnd->str()!=";" &&
                   !((tokEnd->type()==Token::eBracket ||
                      tokEnd->type()==Token::eExtendedOp)&&
                     Token::Match(tokEnd,")|}|>"))) {
                if (tokEnd->type()==Token::eBracket ||
                    (tokEnd->type()==Token::eExtendedOp && tokEnd->str()=="(")) {
                    Token *tokInnerCloseBraket=tokEnd->link();
                    if (!tokInnerCloseBraket) {
                        // Inner bracket does not close
                        return tok;
                    }
                    tokEnd=tokInnerCloseBraket;
                }
                tokEnd=tokEnd->next();
            }
            if (!tokEnd ||
                tokEnd->str()!=";") {
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
        if (Token::Match(tok, "[;{}] (| *| (| %var%")) {
            // backup current token..
            Token * const tok1 = tok;

            if (tok->next()->str() == "*")
                tok = tok->next();

            if (tok->next() && tok->next()->str() == "(") {
                tok = tok->next()->link()->next();
            } else {
                // variable..
                tok = tok->tokAt(2);
                while (Token::Match(tok, ". %var%") ||
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
                Token::Match(tok, "+=|-= '\\0' ;") ||
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
                        if (tok2->str() == "(")
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
                    if (tok2->type() == Token::eIncDecOp) {
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
                    if (Token::Match(tok->next(), "]|)"))
                        tokend.push(tok->next());
                    else if (Token::Match(tok->next(), "(|[")) {
                        Token::createMutualLinks(tok->next(), tokend.top());
                        tokend.pop();
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyConditionOperator()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(" || tok->str() == "[" ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "="))
            tok = tok->link();

        if (Token::Match(tok, "[{};] *| %var% = %any% ? %any% : %any% ;") ||
            Token::Match(tok, "[{};] return %any% ? %any% : %any% ;")) {

            // backup varids so they can be set properly
            std::map<std::string, unsigned int> varid;
            for (const Token *tok2 = tok->next(); tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId())
                    varid[tok2->str()] = tok2->varId();
            }

            std::string var(tok->next()->str());
            bool isPointer = false;
            bool isReturn = false;
            if (tok->next()->str() == "*") {
                tok = tok->next();
                var = tok->next()->str();
                isPointer = true;
            } else if (tok->next()->str() == "return") {
                isReturn = true;
            }

            Token *tok2 = tok->tokAt(3 - (isReturn ? 1 : 0));
            if (!tok2->isName() && !tok2->isNumber() && tok2->str()[0] != '\"')
                continue;
            const std::string condition(tok2->str());
            tok2 = tok2->tokAt(2);
            if (!tok2->isName() && !tok2->isNumber() && tok2->str()[0] != '\"')
                continue;
            const std::string value1(tok2->str());
            tok2 = tok2->tokAt(2);
            if (!tok2->isName() && !tok2->isNumber() && tok2->str()[0] != '\"')
                continue;
            const std::string value2(tok2->str());

            if (isPointer) {
                tok = tok->previous();
                tok->deleteNext(9);
            } else if (isReturn)
                tok->deleteNext(6);
            else
                tok->deleteNext(8);

            Token *starttok = 0;

            std::string str;
            if (isReturn)
                str = "if ( condition ) { return value1 ; } return value2 ;";
            else
                str = "if ( condition ) { * var = value1 ; } else { * var = value2 ; }";

            std::string::size_type pos1 = 0;
            while (pos1 != std::string::npos) {
                if (str[pos1] == '*') {
                    pos1 += 2;
                    if (isPointer) {
                        tok->insertToken("*");
                        tok = tok->next();
                    }
                }
                std::string::size_type pos2 = str.find(" ", pos1);
                if (pos2 == std::string::npos) {
                    tok->insertToken(str.substr(pos1));
                    pos1 = pos2;
                } else {
                    tok->insertToken(str.substr(pos1, pos2 - pos1));
                    pos1 = pos2 + 1;
                }
                tok = tok->next();

                // set links.
                if (tok->str() == "(" || tok->str() == "{")
                    starttok = tok;
                else if (starttok && (tok->str() == ")" || tok->str() == "}")) {
                    Token::createMutualLinks(starttok, tok);
                    starttok = 0;
                } else if (tok->str() == "condition")
                    tok->str(condition);
                else if (tok->str() == "var")
                    tok->str(var);
                else if (tok->str() == "value1")
                    tok->str(value1);
                else if (tok->str() == "value2")
                    tok->str(value2);

                // set varid.
                if (varid.find(tok->str()) != varid.end())
                    tok->varId(varid[tok->str()]);
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
            if (tok->str() == "0" || tok->str() == "false")
                tok->str("true");
            else
                tok->str("false");

            ret = true;
        }

        if (Token::simpleMatch(tok, "( true &&") ||
            Token::simpleMatch(tok, "&& true &&") ||
            Token::simpleMatch(tok->next(), "&& true )")) {
            tok->deleteNext(2);
            ret = true;
        }

        else if (Token::simpleMatch(tok, "( false ||") ||
                 Token::simpleMatch(tok, "|| false ||") ||
                 Token::simpleMatch(tok->next(), "|| false )")) {
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
                    double op1 = MathLib::toDoubleNumber(tok->next()->str());
                    double op2 = MathLib::toDoubleNumber(tok->strAt(3));
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
                bool op1 = (tok->next()->str() == std::string("true"));
                bool op2 = (tok->strAt(3) == std::string("true"));

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
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "?")
            continue;

        if (!Token::Match(tok->tokAt(-2), "<|=|,|(|[|{|}|;|case|return %bool%|%num%") &&
            !Token::Match(tok->tokAt(-4), "<|=|,|(|[|{|}|;|case|return ( %bool%|%num% )"))
            continue;

        const int offset = (tok->previous()->str() == ")") ? 2 : 1;

        if (tok->strAt(-2*offset) == "<" && !TemplateSimplifier::templateParameters(tok->tokAt(-2*offset)))
            continue;

        // Find the token ":" then go to the next token
        Token *semicolon = skipTernaryOp(tok);
        if (!semicolon || semicolon->previous()->str() != ":" || !semicolon->next())
            continue;

        //handle the GNU extension: "x ? : y" <-> "x ? x : y"
        if (semicolon->previous() == tok->next())
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

        if (tok->next()->str() == "false" || tok->next()->str() == "0") {
            // Use code after semicolon, remove code before it.
            Token::eraseTokens(tok, semicolon);

            tok = tok->next();
            ret = true;
        }

        // The condition is true. Delete the operator after the ":"..
        else {
            // delete the condition token and the "?"
            tok->deleteNext(2);

            unsigned int ternaryOplevel = 0;
            for (const Token *endTok = semicolon; endTok; endTok = endTok->next()) {
                if (endTok->str() == "(" || endTok->str() == "[" || endTok->str() == "{") {
                    endTok = endTok->link();
                }

                else if (endTok->str() == "?")
                    ++ternaryOplevel;
                else if (Token::Match(endTok, ")|}|]|;|,|:")) {
                    if (endTok->str() == ":" && ternaryOplevel)
                        --ternaryOplevel;
                    else {
                        Token::eraseTokens(semicolon->tokAt(-2), endTok);
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
            if (!Token::Match(tok2, "%var% [ ]"))
                continue;

            tok = tok2->previous();
            Token *end = tok2->next();
            unsigned int count = 0;
            while (Token::Match(end, "[ ] [;=[]")) {
                end = end->tokAt(2);
                ++count;
            }
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

void Tokenizer::simplifyFloatCasts()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok->next(), "( float|double ) %num%") && MathLib::isInt(tok->strAt(4))) {
            const bool isFloatType(tok->strAt(2) == "float");
            tok->deleteNext(3);
            tok = tok->next();
            // in case of type 'float', add the corresponding suffix 'f'
            tok->str(tok->str() + (isFloatType ? ".0f":".0"));
        }
    }
}

void Tokenizer::simplifyCasts()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // #2897 : don't remove cast in such cases:
        // *((char *)a + 1) = 0;
        // #3596 : remove cast when casting a function pointer:
        // (*(void (*)(char *))fp)(x);
        if (!tok->isName() &&
            Token::simpleMatch(tok->next(), "* (") &&
            !Token::Match(tok->linkAt(2), ") %var%") &&
            !Token::simpleMatch(tok->linkAt(2), ") &")) {
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
        if (Token::Match(tok->next(), "( unsigned| %type% ) %num%") && tok->next()->link()->previous()->isStandardType()) {
            const MathLib::bigint value = MathLib::toLongNumber(tok->next()->link()->next()->str());
            unsigned int bits = 8 * _typeSize[tok->next()->link()->previous()->str()];
            if (!tok->tokAt(2)->isUnsigned())
                bits--;
            if (bits < 31 && value >= 0 && value < (1LL << bits)) {
                Token::eraseTokens(tok, tok->next()->link()->next());
            }
            continue;
        }

        while ((Token::Match(tok->next(), "( %type% *| *| *| ) *|&| %var%") && (tok->str() != ")" || tok->tokAt(2)->isStandardType())) ||
               Token::Match(tok->next(), "( const| %type% %type% *| *| *| ) *|&| %var%") ||
               (!tok->isName() && (Token::Match(tok->next(), "( %type% * *| *| ) (") ||
                                   Token::Match(tok->next(), "( const| %type% %type% * *| *| ) (")))) {
            if (tok->isName() && tok->str() != "return")
                break;

            if (tok->strAt(-1) == "operator")
                break;

            // Remove cast..
            Token::eraseTokens(tok, tok->next()->link()->next());

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
        while (Token::Match(tok->next(), "( %type% * ) 0") ||
               Token::Match(tok->next(), "( %type% %type% * ) 0")) {
            Token::eraseTokens(tok, tok->next()->link()->next());
            if (tok->str() == ")" && tok->link()->previous()) {
                // If there was another cast before this, go back
                // there to check it also. e.g. "(char*)(char*)0"
                tok = tok->link()->previous();
            }
        }

        while (Token::Match(tok->next(), "dynamic_cast|reinterpret_cast|const_cast|static_cast <")) {
            Token *tok2 = tok->linkAt(2);

            if (Token::simpleMatch(tok2, "> (")) {
                Token *closeBracket = tok2->next()->link();
                if (closeBracket) {
                    Token::eraseTokens(tok, tok2->tokAt(2));
                    closeBracket->deleteThis();
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }
}


bool Tokenizer::simplifyFunctionParameters()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "{" || tok->str() == "[" || tok->str() == "(") {
            tok = tok->link();
        }

        // Find the function e.g. foo( x ) or foo( x, y )
        else if (Token::Match(tok, "%var% ( %var% [,)]") &&
                 !(tok->strAt(-1) == ":" || tok->strAt(-1) == ",")) {
            // We have found old style function, now we need to change it

            // First step: Get list of argument names in parentheses
            std::map<std::string, Token *> argumentNames;
            bool bailOut = false;
            Token * tokparam = NULL;

            //take count of the function name..
            const std::string& funcName(tok->str());

            //floating token used to check for parameters
            Token *tok1 = tok;

            while (NULL != (tok1 = tok1->tokAt(2))) {
                if (!Token::Match(tok1, "%var% [,)]")) {
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
                if (tok1->str() == "(" || tok1->str() == ")") {
                    bailOut = true;
                    break;
                }
                if (tok1->str() == ";") {
                    if (tokparam) {
                        syntaxError(tokparam);
                        return false;
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
                        return false;
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
    return true;
}

void Tokenizer::simplifyPointerToStandardType()
{
    if (!isC())
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "& %var% [ 0 ]"))
            continue;

        // Remove '[ 0 ]' suffix
        tok->next()->eraseTokens(tok->next(), tok->tokAt(5));
        // Remove '&' prefix
        tok = tok->previous();
        tok->deleteNext();
    }
}

void Tokenizer:: simplifyFunctionPointers()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // #2873 - dont simplify function pointer usage here:
        // (void)(xy(*p)(0));
        if (Token::simpleMatch(tok, ") (")) {
            tok = tok->next()->link();
            continue;
        }

        // check for function pointer cast
        if (Token::Match(tok, "( %type% *| *| ( * ) (") ||
            Token::Match(tok, "( %type% %type% *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% %type% *| *| ( * ) (")) {
            Token *tok1 = tok;

            if (tok1->str() == "static_cast")
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
        else if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|(|public:|protected:|private:"))
            continue;

        if (Token::Match(tok, "%type% *| *| ( * %var% [| ]| ) ("))
            ;
        else if (Token::Match(tok, "%type% %type% *| *| ( * %var% [| ]| ) ("))
            tok = tok->next();
        else
            continue;

        while (tok->next()->str() == "*")
            tok = tok->next();

        // check that the declaration ends
        const Token *endTok = tok->next()->link()->next()->link();
        if (!Token::Match(endTok, ") ;|,|)|=|["))
            continue;

        // ok simplify this function pointer to an ordinary pointer
        Token::eraseTokens(tok->next()->link(), endTok->next());
        tok->next()->link()->deleteThis();
        tok->deleteNext();
    }
}


bool Tokenizer::simplifyFunctionReturn()
{
    bool ret = false;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            tok = tok->link();

        else if (Token::Match(tok, "%var% ( ) { return %bool%|%char%|%num%|%str% ; }")) {
            const Token* const any = tok->tokAt(5);

            const std::string pattern("(|[|=|%cop% " + tok->str() + " ( ) ;|]|)|%cop%");
            for (Token *tok2 = list.front(); tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, pattern.c_str())) {
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


static void incdec(std::string &value, const std::string &op)
{
    int ivalue = 0;
    std::istringstream istr(value);
    istr >> ivalue;
    if (op == "++")
        ++ivalue;
    else if (op == "--")
        --ivalue;
    std::ostringstream ostr;
    ostr << ivalue;
    value = ostr.str();
}

void Tokenizer::simplifyVarDecl(bool only_k_r_fpar)
{
    simplifyVarDecl(list.front(), NULL, only_k_r_fpar);
}

void Tokenizer::simplifyVarDecl(Token * tokBegin, Token * tokEnd, bool only_k_r_fpar)
{
    // Split up variable declarations..
    // "int a=4;" => "int a; a=4;"
    bool finishedwithkr = true;
    for (Token *tok = tokBegin; tok != tokEnd; tok = tok->next()) {
        if (Token::simpleMatch(tok, "= {")) {
            tok = tok->next()->link();
        }

        if (only_k_r_fpar && finishedwithkr) {
            if (tok->str() == "(" || tok->str() == "[" || tok->str() == "{") {
                tok = tok->link();
                if (tok->next() && Token::Match(tok, ") !!{"))
                    tok = tok->next();
                else
                    continue;
            } else
                continue;
        } else if (tok->str() == "(") {
            if (isCPP()) {
                for (Token * tok2 = tok; tok2 != tok->link(); tok2 = tok2->next()) {
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

        if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|)|public:|protected:|private:"))
            continue;

        Token *type0 = tok;
        if (!Token::Match(type0, "::| %type%"))
            continue;
        if (Token::Match(type0, "else|return|public:|protected:|private:"))
            continue;

        bool isconst = false;
        bool isstatic = false;
        Token *tok2 = type0;
        unsigned int typelen = 1;

        if (tok2->str() == "::") {
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

            tok2 = tok2->next();
            ++typelen;
        }

        // strange looking variable declaration => don't split up.
        if (Token::Match(tok2, "%type% *| %var% , %type% *| %var%"))
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
        while (Token::Match(tok2, "%type% <") || Token::Match(tok2, "%type% ::")) {
            if (tok2->next()->str() == "<" && !TemplateSimplifier::templateParameters(tok2->next())) {
                tok2 = NULL;
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

                if (tok3->str() == "<" && !parens) {
                    ++indentlevel;
                } else if (tok3->str() == ">" && !parens) {
                    if (indentlevel == 0) {
                        tok2 = tok3->next();
                        break;
                    }
                    --indentlevel;
                } else if (tok3->str() == ">>" && !parens) {
                    if (indentlevel <= 1U) {
                        tok2 = tok3->next();
                        break;
                    }
                    indentlevel -= 2;
                } else if (tok3->str() == "(") {
                    ++parens;
                } else if (tok3->str() == ")") {
                    if (!parens) {
                        tok2 = NULL;
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

        //pattern: "%type% *| ... *| const| %var% ,|="
        if (Token::Match(tok2, "%type%") ||
            (tok2 && tok2->previous() && tok2->previous()->str() == ">")) {
            bool ispointer = false;
            Token *varName = tok2;
            if (!tok2->previous() || tok2->previous()->str() != ">")
                varName = varName->next();
            else
                --typelen;
            //skip all the pointer part
            while (varName && varName->str() == "*") {
                ispointer = true;
                varName = varName->next();
            }

            while (Token::Match(varName, "%type% %type%")) {
                if (varName->str() != "const") {
                    ++typelen;
                }
                varName = varName->next();
            }
            //non-VLA case
            if (Token::Match(varName, "%var% ,|=")) {
                if (varName->str() != "operator") {
                    tok2 = varName->next(); // The ',' or '=' token

                    if (tok2->str() == "=") {
                        if (isstatic) {
                            if (Token::Match(tok2->next(), "%num%|%var% ,")) // ticket #5121
                                tok2 = tok2->tokAt(2);
                            else if (Token::Match(tok2->next(), "( %num%|%var% ) ,")) { // ticket #4450
                                tok2->deleteNext();
                                tok2->next()->deleteNext();
                                tok2 = tok2->tokAt(2);
                            } else
                                tok2 = NULL;
                        } else if (isconst && !ispointer) {
                            //do not split const non-pointer variables..
                            while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                                if (tok2->str() == "{" || tok2->str() == "(" || tok2->str() == "[")
                                    tok2 = tok2->link();
                                if (tok2->str() == "<" && TemplateSimplifier::templateParameters(tok2) > 0)
                                    tok2 = tok2->findClosingBracket();
                                tok2 = tok2->next();
                            }
                            if (tok2 && tok2->str() == ";")
                                tok2 = NULL;
                        }
                    }
                } else
                    tok2 = NULL;
            }

            //VLA case
            else if (Token::Match(varName, "%var% [")) {
                tok2 = varName->next();

                while (Token::Match(tok2->link(), "] ,|=|["))
                    tok2 = tok2->link()->next();
                if (!Token::Match(tok2, "=|,"))
                    tok2 = NULL;
                if (tok2 && tok2->str() == "=") {
                    while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                        if (tok2->str() == "{" || tok2->str() == "(" || tok2->str() == "[")
                            tok2 = tok2->link();
                        tok2 = tok2->next();
                    }
                    if (tok2 && tok2->str() == ";")
                        tok2 = NULL;
                }
            } else
                tok2 = NULL;
        } else {
            tok2 = NULL;
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
                if (tok2->str() == "{" || tok2->str() == "(")
                    tok2 = tok2->link();

                else if (tok2->str() == "<" && tok2->previous()->isName() && !tok2->previous()->varId())
                    tok2 = tok2->findClosingBracket();

                else if (std::strchr(";,", tok2->str()[0])) {
                    // "type var ="   =>   "type var; var ="
                    const Token *VarTok = type0->tokAt((int)typelen);
                    while (Token::Match(VarTok, "*|&|const"))
                        VarTok = VarTok->next();
                    list.insertTokens(eq, VarTok, 2);
                    eq->str(";");

                    // "= x, "   =>   "= x; type "
                    if (tok2->str() == ",") {
                        tok2->str(";");
                        list.insertTokens(tok2, type0, typelen);
                    }
                    break;
                }

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
    if (_settings->sizeof_size_t == 8) {
        if (_settings->sizeof_long == 8)
            type = isLong;
        else
            type = isLongLong;
    } else if (_settings->sizeof_size_t == 4 && _settings->sizeof_long == 4)
        type = isLong;
    else if (_settings->sizeof_size_t == 4)
        type = isInt;
    else
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        bool inStd = false;
        if (Token::Match(tok, "std :: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t")) {
            inStd = true;
            tok->deleteNext();
            tok->deleteThis();
        } else if (Token::Match(tok, ":: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t")) {
            tok->deleteThis();
        }

        if (Token::Match(tok, "size_t|uintptr_t|uintmax_t")) {
            if (inStd)
                tok->originalName("std::" + tok->str());
            else
                tok->originalName(tok->str());
            tok->isUnsigned(true);

            switch (type) {
            case isLongLong:
                tok->isLong(true);
                tok->str("long");
                break;
            case isLong :
                tok->str("long");
                break;
            case isInt:
                tok->str("int");
                break;
            }
        } else if (Token::Match(tok, "ssize_t|ptrdiff_t|intptr_t|intmax_t")) {
            if (inStd)
                tok->originalName("std::" + tok->str());
            else
                tok->originalName(tok->str());
            switch (type) {
            case isLongLong:
                tok->isLong(true);
                tok->str("long");
                break;
            case isLong :
                tok->str("long");
                break;
            case isInt:
                tok->str("int");
                break;
            }
        }
    }

    if (_settings->platformType == Settings::Win32A ||
        _settings->platformType == Settings::Win32W ||
        _settings->platformType == Settings::Win64) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::Match(tok, "BOOL|INT|INT32|HFILE|LONG32")) {
                tok->originalName(tok->str());
                tok->str("int");
            } else if (Token::Match(tok, "BOOLEAN|BYTE|UCHAR")) {
                tok->originalName(tok->str());
                tok->str("char");
                tok->isUnsigned(true);
            } else if (tok->str() == "CHAR") {
                tok->originalName(tok->str());
                tok->str("char");
            } else if (Token::Match(tok, "DWORD|ULONG|COLORREF|LCID|LCTYPE|LGRPID")) {
                tok->originalName(tok->str());
                tok->str("long");
                tok->isUnsigned(true);
            } else if (Token::Match(tok, "DWORD_PTR|ULONG_PTR|SIZE_T")) {
                tok->originalName(tok->str());
                tok->str("long");
                tok->isUnsigned(true);
                if (_settings->platformType == Settings::Win64)
                    tok->isLong(true);
            } else if (tok->str() == "FLOAT") {
                tok->originalName(tok->str());
                tok->str("float");
            } else if (Token::Match(tok, "HRESULT|LONG")) {
                tok->originalName(tok->str());
                tok->str("long");
            } else if (Token::Match(tok, "INT8")) {
                tok->originalName(tok->str());
                tok->str("char");
                tok->isSigned(true);
            } else if (Token::Match(tok, "INT64|LONG64|LONGLONG")) {
                tok->originalName(tok->str());
                tok->str("long");
                tok->isLong(true);
            } else if (Token::Match(tok, "LONG_PTR|LPARAM|LRESULT|SSIZE_T")) {
                tok->originalName(tok->str());
                tok->str("long");
                if (_settings->platformType == Settings::Win64)
                    tok->isLong(true);
            } else if (Token::Match(tok, "LPBOOL|PBOOL")) {
                tok->str("int");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPBYTE|PBOOLEAN|PBYTE|PUCHAR")) {
                tok->isUnsigned(true);
                tok->str("char");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPCSTR|PCSTR")) {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("char");
            } else if (tok->str() == "LPCVOID") {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("void");
            } else if (Token::Match(tok, "LPDWORD|LPCOLORREF|PDWORD|PULONG")) {
                tok->isUnsigned(true);
                tok->str("long");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPINT|PINT")) {
                tok->str("int");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPLONG|PLONG")) {
                tok->str("long");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPSTR|PSTR|PCHAR")) {
                tok->str("char");
                tok->insertToken("*");
            } else if (Token::Match(tok, "PWSTR|PWCHAR")) {
                tok->str("wchar_t");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPVOID|PVOID|HANDLE|HBITMAP|HBRUSH|HCOLORSPACE|HCURSOR|HDC|HFONT|HGDIOBJ|HGLOBAL|HICON|HINSTANCE|HKEY|HLOCAL|HMENU|HMETAFILE|HMODULE|HPALETTE|HPEN|HRGN|HRSRC|HWND|SERVICE_STATUS_HANDLE|SC_LOCK|SC_HANDLE|HACCEL|HCONV|HCONVLIST|HDDEDATA|HDESK|HDROP|HDWP|HENHMETAFILE|HHOOK|HKL|HMONITOR|HSZ|HWINSTA")) {
                tok->str("void");
                tok->insertToken("*");
            } else if ((tok->str() == "PHANDLE")) {
                tok->str("void");
                tok->insertToken("*");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPWORD|PWORD|PUSHORT")) {
                tok->isUnsigned(true);
                tok->str("short");
                tok->insertToken("*");
            } else if (Token::Match(tok, "SHORT|INT16")) {
                tok->originalName(tok->str());
                tok->str("short");
            } else if (Token::Match(tok, "UINT|MMRESULT|SOCKET|ULONG32|UINT32|DWORD32")) {
                tok->originalName(tok->str());
                tok->isUnsigned(true);
                tok->str("int");
            } else if (Token::Match(tok, "UINT_PTR|WPARAM")) {
                tok->originalName(tok->str());
                tok->isUnsigned(true);
                if (_settings->platformType == Settings::Win64) {
                    tok->str("long");
                    tok->isLong(true);
                } else {
                    tok->str("int");
                }
            } else if (Token::Match(tok, "USHORT|WORD|ATOM|LANGID")) {
                tok->originalName(tok->str());
                tok->isUnsigned(true);
                tok->str("short");
            } else if (tok->str() == "VOID") {
                tok->originalName(tok->str());
                tok->str("void");
            } else if (tok->str() == "TCHAR") {
                tok->originalName(tok->str());
                if (_settings->platformType == Settings::Win32A)
                    tok->str("char");
                else {
                    tok->str("wchar_t");
                }
            } else if (tok->str() == "TBYTE") {
                tok->originalName(tok->str());
                tok->isUnsigned(true);
                if (_settings->platformType == Settings::Win32A)
                    tok->str("short");
                else
                    tok->str("char");
            } else if (Token::Match(tok, "PTSTR|LPTSTR")) {
                if (_settings->platformType == Settings::Win32A) {
                    tok->str("char");
                    tok->insertToken("*");
                } else {
                    tok->str("wchar_t");
                    tok->insertToken("*");
                }
            } else if (Token::Match(tok, "PCTSTR|LPCTSTR")) {
                tok->str("const");
                if (_settings->platformType == Settings::Win32A) {
                    tok->insertToken("*");
                    tok->insertToken("char");
                } else {
                    tok->insertToken("*");
                    tok->insertToken("wchar_t");
                }
            } else if (Token::Match(tok, "ULONG64|DWORD64|ULONGLONG")) {
                tok->originalName(tok->str());
                tok->isUnsigned(true);
                tok->isLong(true);
                tok->str("long");
            } else if (tok->str() == "HALF_PTR") {
                tok->originalName(tok->str());
                if (_settings->platformType == Settings::Win64)
                    tok->str("int");
                else
                    tok->str("short");
            } else if (tok->str() == "INT_PTR") {
                tok->originalName(tok->str());
                if (_settings->platformType == Settings::Win64) {
                    tok->str("long");
                    tok->isLong(true);
                } else {
                    tok->str("int");
                }
            } else if (tok->str() == "LPWSTR") {
                tok->str("wchar_t");
                tok->insertToken("*");
            } else if (tok->str() == "LPCWSTR") {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("wchar_t");
            } else if (tok->str() == "WCHAR") {
                tok->originalName(tok->str());
                tok->str("wchar_t");
            }
        }
    }
}

void Tokenizer::simplifyStdType()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // long unsigned => unsigned long
        if (Token::Match(tok, "char|short|int|long|__int8|__int16|__int32|__int64 unsigned|signed")) {
            bool isUnsigned = tok->next()->str() == "unsigned";
            tok->deleteNext();
            tok->isUnsigned(isUnsigned);
            tok->isSigned(!isUnsigned);
        }

        else if (!Token::Match(tok, "unsigned|signed|char|short|int|long|__int8|__int16|__int32|__int64"))
            continue;

        // check if signed or unsigned specified
        if (Token::Match(tok, "unsigned|signed")) {
            bool isUnsigned = tok->str() == "unsigned";

            // unsigned i => unsigned int i
            if (!Token::Match(tok->next(), "char|short|int|long|__int8|__int16|__int32|__int64"))
                tok->str("int");
            else
                tok->deleteThis();
            tok->isUnsigned(isUnsigned);
            tok->isSigned(!isUnsigned);
        }

        if (tok->str() == "__int8") {
            tok->originalName(tok->str());
            tok->str("char");
        } else if (tok->str() == "__int16") {
            tok->originalName(tok->str());
            tok->str("short");
        } else if (tok->str() == "__int32") {
            tok->originalName(tok->str());
            tok->str("int");
        } else if (tok->str() == "__int64") {
            tok->originalName(tok->str());
            tok->str("long");
            tok->isLong(true);
        } else if (tok->str() == "int") {
            if (tok->strAt(1) == "long") {
                tok->str("long");
                tok->deleteNext();
            } else if (tok->strAt(1) == "short") {
                tok->str("short");
                tok->deleteNext();
            }
            if (tok->strAt(1) == "long") {
                tok->isLong(true);
                tok->deleteNext();
            }
            if (Token::Match(tok->next(), "unsigned|signed")) {
                tok->isUnsigned(tok->next()->str() == "unsigned");
                tok->isSigned(tok->next()->str() == "signed");
                tok->deleteNext();
                if (tok->strAt(1) == "long")
                    tok->deleteNext();
                else if (tok->strAt(1) == "short")
                    tok->deleteNext();
            }
        } else if (tok->str() == "long") {
            if (tok->strAt(1) == "long") {
                tok->isLong(true);
                tok->deleteNext();
            }
            if (tok->strAt(1) == "int") {
                tok->deleteNext();
                if (Token::Match(tok->next(), "unsigned|signed")) {
                    tok->isUnsigned(tok->next()->str() == "unsigned");
                    tok->isSigned(tok->next()->str() == "signed");
                    tok->deleteNext();
                }
            } else if (tok->strAt(1) == "double") {
                tok->str("double");
                tok->isLong(true);
                tok->deleteNext();
            } else if (Token::Match(tok->next(), "unsigned|signed")) {
                tok->isUnsigned(tok->next()->str() == "unsigned");
                tok->isSigned(tok->next()->str() == "signed");
                tok->deleteNext();
                if (tok->strAt(1) == "int")
                    tok->deleteNext();
            }
        } else if (tok->str() == "short") {
            if (tok->strAt(1) == "int")
                tok->deleteNext();
            if (Token::Match(tok->next(), "unsigned|signed")) {
                tok->isUnsigned(tok->next()->str() == "unsigned");
                tok->isSigned(tok->next()->str() == "signed");
                tok->deleteNext();
                if (tok->strAt(1) == "int")
                    tok->deleteNext();
            }
        }
    }
}

void Tokenizer::simplifyIfAndWhileAssign()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok->next(), "if|while ( !| (| %var% =") &&
            !Token::Match(tok->next(), "if|while ( !| (| %var% . %var% ="))
            continue;

        // simplifying a "while(cond) { }" condition ?
        const bool iswhile(tok->next()->str() == "while");

        // simplifying a "do { } while(cond);" condition ?
        const bool isDoWhile = iswhile && Token::Match(tok, "}") && Token::Match(tok->link()->previous(), "do");
        Token* openBraceTok = tok->link();

        // delete the "if|while"
        tok->deleteNext();

        // Remember if there is a "!" or not. And delete it if there are.
        const bool isNot(tok->strAt(2) == "!");
        if (isNot)
            tok->next()->deleteNext();

        // Delete parentheses.. and remember how many there are with
        // their links.
        std::stack<Token *> braces;
        while (tok->next()->str() == "(") {
            braces.push(tok->next()->link());
            tok->deleteNext();
        }

        // Skip the "%var% = ..."
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
                    tok3->next()->varId(tok2->varId());

                    Token *newTok = tok3->next();
                    newTok->fileIndex(tok2->fileIndex());
                    newTok->linenr(tok2->linenr());

                    // link() newly tokens manually
                    if (Token::Match(newTok, "}|)|]")) {
                        braces2.push(newTok);
                    } else if (Token::Match(newTok, "{|(|[")) {
                        Token::createMutualLinks(newTok, braces2.top());
                        braces2.pop();
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyVariableMultipleAssign()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% = %var% = %num%|%var% ;")) {
            // skip intermediate assignments
            Token *tok2 = tok->previous();
            while (tok2 &&
                   tok2->str() == "=" &&
                   Token::Match(tok2->previous(), "%var%")) {
                tok2 = tok2->tokAt(-2);
            }

            if (!tok2 || tok2->str() != ";") {
                continue;
            }

            Token *stopAt = tok->tokAt(2);
            const Token *valueTok = tok->tokAt(4);
            const std::string value(valueTok->str());
            tok2 = tok2->next();

            while (tok2 != stopAt) {
                tok2->next()->insertToken(";");
                tok2->next()->insertToken(value);
                tok2 = tok2->tokAt(4);
            }
        }
    }
}


void Tokenizer::simplifyIfNot()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "(|&&|%oror%")) {
            tok = tok->next();
            while (tok && tok->str() == "(")
                tok = tok->next();

            if (!tok)
                break;

            if (Token::Match(tok, "0|false == (") ||
                Token::Match(tok, "0|false == %var%")) {
                tok->deleteNext();
                tok->str("!");
            }

            else if (Token::Match(tok, "%var% == 0|false")) {
                tok->deleteNext(2);
                tok = tok->previous();
                tok->insertToken("!");
                tok = tok->next();
            }

            else if (Token::Match(tok, "%var% .|:: %var% == 0|false")) {
                tok = tok->previous();
                tok->insertToken("!");
                tok = tok->tokAt(4);
                tok->deleteNext(2);
            }

            else if (Token::Match(tok, "* %var% == 0|false")) {
                tok = tok->previous();
                tok->insertToken("!");
                tok = tok->tokAt(3);
                tok->deleteNext(2);
            }
        }

        else if (tok->link() && Token::Match(tok, ") == 0|false")) {
            // if( foo(x) == 0 )
            if (Token::Match(tok->link()->tokAt(-2), "( %var%")) {
                tok->deleteNext(2);
                tok->link()->previous()->insertToken(tok->link()->previous()->str());
                tok->link()->tokAt(-2)->str("!");
            }

            // if( (x) == 0 )
            else if (tok->link()->strAt(-1) == "(") {
                tok->deleteNext(2);
                tok->link()->insertToken("(");
                tok->link()->str("!");
                Token *temp = tok->link();
                Token::createMutualLinks(tok->link()->next(), tok);
                temp->link(0);
            }
        }
    }
}


void Tokenizer::simplifyIfNotNull()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token *deleteFrom = NULL;

        // Remove 'x = (x != 0)'
        if (Token::simpleMatch(tok, "= (")) {
            if (Token::Match(tok->tokAt(-2), "[;{}] %var%")) {
                const std::string varname(tok->previous()->str());

                if (Token::simpleMatch(tok->tokAt(2), (varname + " != 0 ) ;").c_str()) ||
                    Token::simpleMatch(tok->tokAt(2), ("0 != " + varname + " ) ;").c_str())) {
                    tok = tok->tokAt(-2);
                    tok->deleteNext(8);
                }
            }
            continue;
        }

        if (Token::Match(tok, "==|!= ("))
            tok = tok->linkAt(1);

        if (Token::Match(tok, "(|&&|%oror%")) {
            tok = tok->next();

            if (!tok)
                break;

            if (Token::simpleMatch(tok, "0 != (") ||
                Token::Match(tok, "0 != %var%")) {
                deleteFrom = tok->previous();
                if (tok->tokAt(2))
                    tok->tokAt(2)->isPointerCompare(true);
            }

            else if (Token::Match(tok, "%var% != 0")) {
                deleteFrom = tok;
                tok->isPointerCompare(true);
                tok->isExpandedMacro(tok->isExpandedMacro() || tok->tokAt(2)->isExpandedMacro());
            }

            else if (Token::Match(tok, "%var% .|:: %var% != 0")) {
                tok = tok->tokAt(2);
                deleteFrom = tok;
                tok->isPointerCompare(true);
                tok->isExpandedMacro(tok->isExpandedMacro() || tok->tokAt(2)->isExpandedMacro());
            }
        }

        else if (tok->link() && Token::simpleMatch(tok, ") != 0")) {
            deleteFrom = tok;
        }

        if (deleteFrom) {
            Token::eraseTokens(deleteFrom, deleteFrom->tokAt(3));
            tok = deleteFrom;
        }
    }
}


void Tokenizer::simplifyIfSameInnerCondition()
{
    // same inner condition
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if ( %var% ) {")) {
            const unsigned int varid(tok->tokAt(2)->varId());
            if (!varid)
                continue;

            for (Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{" || tok2->str() == "}")
                    break;
                if (Token::simpleMatch(tok2, "if (")) {
                    tok2 = tok2->tokAt(2);
                    if (Token::Match(tok2, "%varid% )", varid))
                        tok2->str("true");
                    else if (Token::Match(tok2, "! %varid% )", varid))
                        tok2->next()->varId(varid);
                    break;
                }
            }
        }
    }
}


bool Tokenizer::simplifyLogicalOperators()
{
    bool ret = false;

    // "if (not p)" => "if (!p)"
    // "if (p and q)" => "if (p && q)"
    // "if (p or q)" => "if (p || q)"
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "if|while ( not|compl %var%")) {
            tok->tokAt(2)->str(tok->strAt(2) == "not" ? "!" : "~");
            ret = true;
        } else if (Token::Match(tok, "&& not|compl %var%")) {
            tok->next()->str(tok->next()->str() == "not" ? "!" : "~");
            ret = true;
        } else if (Token::Match(tok, "|| not|compl %var%")) {
            tok->next()->str(tok->next()->str() == "not" ? "!" : "~");
            ret = true;
        }
        // "%var%|) and %var%|("
        else if (Token::Match(tok, "%var% %any%")) {
            if (!Token::Match(tok, "and|or|bitand|bitor|xor|not_eq"))
                continue;

            const Token *tok2 = tok;
            while (NULL != (tok2 = tok2->previous())) {
                if (tok2->str() == ")")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "(|;|{|}"))
                    break;
            }
            if (tok2 && Token::Match(tok2->previous(), "if|while (")) {
                if (tok->str() == "and")
                    tok->str("&&");
                else if (tok->str() == "or")
                    tok->str("||");
                else if (tok->str() == "bitand")
                    tok->str("&");
                else if (tok->str() == "bitor")
                    tok->str("|");
                else if (tok->str() == "xor")
                    tok->str("^");
                else if (tok->str() == "not_eq")
                    tok->str("!=");
                ret = true;
            }
        }
    }
    return ret;
}

// int i(0); => int i; i = 0;
// int i(0), j; => int i; i = 0; int j;
void Tokenizer::simplifyInitVar()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (!tok->isName() || (tok->previous() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (tok->str() == "return")
            continue;

        if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ;") ||
            Token::Match(tok, "%type% *| %var% ( %type% (")) {
            tok = initVar(tok);
        } else if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ,")) {
            Token *tok1 = tok;
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
            if (Token::Match(tok, "%type%|* & %var% = %var% ;")) {
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

            if (tok->isName() && (Token::Match(tok, "static| const| static| %type% const| %var% = %any% ;") ||
                                  Token::Match(tok, "static| const| static| %type% const| %var% ( %any% ) ;"))) {
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
                while (tok->str() == "const" || tok->str() == "static")
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
                }
            }

            else if (tok->varId() && constantValues.find(tok->varId()) != constantValues.end()) {
                tok->str(constantValues[tok->varId()]);
            }
        }
    }

    // variable id for float/double variables
    std::set<unsigned int> floatvars;

    // auto variables..
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // Search for a block of code
        if (! Token::Match(tok, ") const| {"))
            continue;

        // parse the block of code..
        int indentlevel = 0;
        Token *tok2 = tok;
        for (; tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "[;{}] float|double %var% ;")) {
                floatvars.insert(tok2->tokAt(2)->varId());
            }

            if (tok2->str() == "{")
                ++indentlevel;

            else if (tok2->str() == "}") {
                --indentlevel;
                if (indentlevel <= 0)
                    break;
            }

            else if (tok2->previous()->str() != "*" && !Token::Match(tok2->tokAt(-2), "* --|++") &&
                     (Token::Match(tok2, "%var% = %bool%|%char%|%num%|%str%|%var% ;") ||
                      Token::Match(tok2, "%var% [ ] = %str% ;") ||
                      Token::Match(tok2, "%var% [ %num% ] = %str% ;") ||
                      Token::Match(tok2, "%var% = & %var% ;") ||
                      Token::Match(tok2, "%var% = & %var% [ 0 ] ;"))) {
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
                    while (Token::Match(tok3->previous(), ":: %type%"))
                        tok3 = tok3->tokAt(-2);
                    if (Token::Match(tok3->tokAt(-2), "for ( %type%"))
                        continue;
                }

                // struct name..
                const std::string structname = Token::Match(tok2->tokAt(-3), "[;{}] %var% .") ?
                                               std::string(tok2->strAt(-2) + " .") :
                                               std::string("");

                if (Token::Match(tok2, "%varid% = &| %varid%", tok2->varId()))
                    continue;

                const Token * const valueToken = tok2->tokAt(2);

                std::string value;
                unsigned int valueVarId = 0;

                Token *tok3 = NULL;
                bool valueIsPointer = false;

                // there could be a hang here if tok2 is moved back by the function calls below for some reason
                if (_settings->terminated())
                    return false;

                if (!simplifyKnownVariablesGetData(varid, &tok2, &tok3, value, valueVarId, valueIsPointer, floatvars.find(tok2->varId()) != floatvars.end()))
                    continue;

                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
            }

            else if (Token::Match(tok2, "( %var% == %num% ) {")) {
                const unsigned int varid = tok2->next()->varId();
                if (varid == 0)
                    continue;

                const std::string structname;

                const Token *valueToken = tok2->tokAt(3);
                std::string value(tok2->strAt(3)), savedValue = value;
                const unsigned int valueVarId = 0;
                const bool valueIsPointer = false;

                // Insert a "%var% = %num% ;" at the beginning of the scope as simplifyKnownVariablesSimplify might compute an updated value
                Token *scopeStart = tok2->tokAt(5);
                scopeStart->insertToken(tok2->tokAt(1)->str());
                scopeStart = scopeStart->next();
                Token* artificialAssignment = scopeStart;
                scopeStart->insertToken("=");
                scopeStart = scopeStart->next();
                scopeStart->insertToken(valueToken->str());
                scopeStart = scopeStart->next();
                scopeStart->insertToken(";");
                scopeStart = scopeStart->next();

                ret |= simplifyKnownVariablesSimplify(&artificialAssignment, tok2->tokAt(6), varid, structname, value, valueIsPointer, valueVarId, valueToken, -1);

                // Remove the artificial assignment if no modification was done
                if (artificialAssignment->tokAt(2)->str() == savedValue) {
                    Token::eraseTokens(tok2->tokAt(5), scopeStart->next());
                }
            }

            else if (Token::Match(tok2, "strcpy|sprintf ( %var% , %str% ) ;")) {
                const unsigned int varid(tok2->tokAt(2)->varId());
                std::string::size_type n = std::string::npos;
                if (varid == 0)
                    continue;
                const std::string structname;
                const Token * const valueToken = tok2->tokAt(4);
                std::string value(valueToken->str());
                if (tok2->str() == "sprintf") {
                    while ((n = value.find("%%",n+1)) != std::string::npos) {
                        value.replace(n,2,"%");
                    }
                }
                const unsigned int valueVarId(0);
                const bool valueIsPointer(false);
                Token *tok3 = tok2->tokAt(6);
                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);

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
    Token *tok3 = NULL;

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
        const std::string& compareop = tok2->strAt(5);
        if (compareop == "<") {
            value = tok2->strAt(6);
            valueVarId = tok2->tokAt(6)->varId();
        } else
            value = MathLib::toString(MathLib::toLongNumber(tok2->strAt(6)) + 1);

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

        // float value should contain a "."
        else if (tok2->tokAt(2)->isNumber() &&
                 floatvar &&
                 value.find(".") == std::string::npos) {
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
    const bool pointeralias(valueToken->isName() || Token::Match(valueToken, "& %var% ["));

    if (_errorLogger && !list.getFiles().empty())
        _errorLogger->reportProgress(list.getFiles()[0], "Tokenize (simplifyKnownVariables)", tok3->progressValue());

#ifdef MAXTIME
    if (std::time(0) > maxtime)
        return false;
#endif

    bool ret = false;

    Token* bailOutFromLoop = 0;
    int indentlevel3 = indentlevel;
    bool ret3 = false;
    for (; tok3; tok3 = tok3->next()) {
        if (tok3->str() == "{") {
            ++indentlevel3;
        } else if (tok3->str() == "}") {
            --indentlevel3;
            if (indentlevel3 < indentlevel) {
                if (Token::Match((*tok2)->tokAt(-7), "%type% * %var% ; %var% = & %var% ;") &&
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
        if (tok3->str() == "break" || tok3->str() == "continue")
            break;
        if ((indentlevel3 > 1 || !Token::simpleMatch(Token::findsimplematch(tok3,";"), "; }")) && tok3->str() == "return")
            ret3 = true;
        if (ret3 && tok3->str() == ";")
            break;

        if (pointeralias && Token::Match(tok3, ("!!= " + value).c_str()))
            break;

        // Stop if do is found
        if (tok3->str() == "do")
            break;

        // Stop if unknown function call is seen
        // If the variable is a global or a member variable it might be
        // changed by the function call
        // TODO: don't bail out if the variable is a local variable,
        //       then it can't be changed by the function call.
        if (tok3->str() == ")" && tok3->link() &&
            Token::Match(tok3->link()->tokAt(-2), "[;{}] %var% (") &&
            !Token::Match(tok3->link()->previous(), "if|for|while|switch|BOOST_FOREACH"))
            break;

        // Stop if something like 'while (--var)' is found
        if (tok3->str() == "for" || tok3->str() == "while" || tok3->str() == "do") {
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
                bailOutFromLoop = 0;
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
        if (Token::Match(tok3, "%var% = realloc ( %var% ,") &&
            tok3->varId() == varid &&
            tok3->tokAt(4)->varId() == varid) {
            tok3->tokAt(4)->str(value);
            ret = true;
        }

        // condition "(|&&|%OROR% %varid% )|&&|%OROR%
        if (!Token::Match(tok3->previous(), "( %var% )") &&
            Token::Match(tok3->previous(), "&&|(|%oror% %varid% &&|%oror%|)", varid)) {
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // parameter in function call..
        if (tok3->varId() == varid && Token::Match(tok3->previous(), "[(,] %var% [,)]")) {
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
            if (_settings->debugwarnings) {
                // FIXME: Fix all the debug warnings for values and then
                // remove this bailout
                if (pointeralias)
                    break;

                // suppress debug-warning when calling member function
                if (Token::Match(tok3->next(), ". %var% ("))
                    break;

                // suppress debug-warning when assignment
                if (tok3->strAt(1) == "=")
                    break;

                // taking address of variable..
                if (Token::Match(tok3->tokAt(-2), "return|= & %var% ;"))
                    break;

                // parameter in function call..
                if (Token::Match(tok3->tokAt(-2), "%var% ( %var% ,|)") ||
                    Token::Match(tok3->previous(), ", %var% ,|)"))
                    break;

                // conditional increment
                if (Token::Match(tok3->tokAt(-3), ") { ++|--") ||
                    Token::Match(tok3->tokAt(-2), ") { %var% ++|--"))
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
            if (Token::Match(valueToken, "& %var% ;")) {
                tok3->insertToken("&");
                tok3 = tok3->next();
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // pointer alias used in condition..
        if (Token::Match(valueToken,"& %var% ;") && Token::Match(tok3, ("( * " + structname + " %varid% %cop%").c_str(), varid)) {
            tok3->deleteNext();
            if (!structname.empty())
                tok3->deleteNext(2);
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Delete pointer alias
        if (pointeralias && tok3->str() == "delete" &&
            (Token::Match(tok3, "delete %varid% ;", varid) ||
             Token::Match(tok3, "delete [ ] %varid%", varid))) {
            tok3 = (tok3->next() && tok3->next()->str() == "[") ? tok3->tokAt(3) : tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Variable is used in function call..
        if (Token::Match(tok3, ("%var% ( " + structname + " %varid% ,").c_str(), varid)) {
            static const char * const functionName[] = {
                "memcmp","memcpy","memmove","memset",
                "strcmp","strcpy","strncmp","strncpy","strdup"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i) {
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
        if (Token::Match(tok3, ("%var% ( %any% , " + structname + " %varid% ,|)").c_str(), varid)) {
            static const char * const functionName[] = {
                "memcmp","memcpy","memmove",
                "strcmp","strcpy","strncmp","strncpy"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i) {
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
            while (prev && prev->str() != "return" && (prev->isName() || prev->str() == "::"))
                prev = prev->previous();
            if (Token::Match(prev, ";|{|}|>>"))
                break;
        }

        // Variable is used in calculation..
        if (((tok3->previous()->varId() > 0) && Token::Match(tok3, ("& " + structname + " %varid%").c_str(), varid)) ||
            (Token::Match(tok3, ("[=+-*/%^|[] " + structname + " %varid% [=?+-*/%^|;])]").c_str(), varid) && !Token::Match(tok3, ("= " + structname + " %var% =").c_str())) ||
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
            if (tok3->previous()->str() == "*" && (valueIsPointer || Token::Match(valueToken, "& %var% ;"))) {
                tok3 = tok3->previous();
                tok3->deleteThis();
                ret = true;
            }
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
                if (tok4->str() == "(" || tok4->str() == ")")
                    break;

                // Replace variable used in condition..
                if (Token::Match(tok4, "; %var% <|<=|!= %var% ; ++| %var% ++| )")) {
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
            incdec(value, op);
            if (!Token::simpleMatch((*tok2)->tokAt(-2), "for (")) {
                (*tok2)->tokAt(2)->str(value);
                (*tok2)->tokAt(2)->varId(valueVarId);
            }
            ret = true;
        }

        if (indentlevel == indentlevel3 && Token::Match(tok3->next(), "++|-- %varid%", varid) && MathLib::isInt(value) &&
            !Token::Match(tok3->tokAt(3), "[.[]")) {
            incdec(value, tok3->next()->str());
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
        if (tok->str() == "(" || tok->str() == "[" ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "="))
            tok = tok->link();

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
               tok->link()->previous() == tok->next()->link()) {
            // We have "(( *something* ))", remove the inner
            // parentheses
            tok->deleteNext();
            tok->link()->tokAt(-2)->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "! ( %var% )")) {
            // Remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(,;{}] ( %var% (") &&
            tok->link()->previous() == tok->linkAt(2)) {
            // We have "( func ( *something* ))", remove the outer
            // parentheses
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[,;{}] ( delete [| ]| %var% ) ;")) {
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

        if (Token::Match(tok->previous(), "[(!*;{}] ( %var% )") &&
            (tok->next()->varId() != 0 || Token::Match(tok->tokAt(3), "[+-/=]"))) {
            // We have "( var )", remove the parentheses
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{}[]().,!*] ( %var% .")) {
            Token *tok2 = tok->tokAt(2);
            while (Token::Match(tok2, ". %var%")) {
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
            while (tok2 && (Token::Match(tok2,"%bool%|%num%|%var%") || tok2->isArithmeticalOp()))
                tok2 = tok2->next();
            if (tok2 && tok2->str() == ")") {
                tok->link()->deleteThis();
                tok->deleteThis();
                ret = true;
                continue;
            }
        }

        while (Token::Match(tok->previous(), "[{([,:] ( !!{") &&
               Token::Match(tok->link(), ") [;,])]") &&
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

        // Simplify "!!operator !!(%var%|)) ( %num%|%bool% ) %op%|;|,|)"
        if (Token::Match(tok, "( %bool%|%num% ) %cop%|;|,|)") &&
            tok->strAt(-2) != "operator" &&
            tok->previous() &&
            !tok->previous()->isName() &&
            tok->previous()->str() != ")" &&
            (!isCPP() || tok->previous()->str() != ">")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
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
            if (index >= 0 && index <= (MathLib::bigint)Token::getStrLength(tok)) {
                tok->str(std::string("'" + Token::getCharAt(tok, (size_t)index) + "'"));
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
        if (Token::Match(tok, ") const| {")) {
            // replace references in this scope..
            if (tok->next()->str() != "{")
                tok = tok->next();
            Token * const end = tok->next()->link();
            for (Token *tok2 = tok; tok2 && tok2 != end; tok2 = tok2->next()) {
                // found a reference..
                if (Token::Match(tok2, "[;{}] %type% & %var% (|= %var% )| ;")) {
                    const unsigned int ref_id = tok2->tokAt(3)->varId();
                    if (!ref_id)
                        continue;

                    // replace reference in the code..
                    for (Token *tok3 = tok2->tokAt(7); tok3 && tok3 != end; tok3 = tok3->next()) {
                        if (tok3->varId() == ref_id) {
                            tok3->str(tok2->strAt(5));
                            tok3->varId(tok2->tokAt(5)->varId());
                        }
                    }

                    tok2->deleteNext(6+(tok->strAt(6)==")" ? 1 : 0));
                }
            }
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
            && Token::Match(tok->next(), "* ( %var% +|- %num%|%var% )")) {

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

void Tokenizer::simplifyGoto()
{
    std::list<Token *> gotos;
    unsigned int indentlevel = 0;
    unsigned int indentspecial = 0;
    Token *beginfunction = 0;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(" || tok->str() == "[")
            tok = tok->link();

        else if (Token::Match(tok, "class|namespace|struct|union %type% :|{")) {
            tok = tok->tokAt(2);
            while (tok && !Token::Match(tok, "[;{=]"))
                tok = tok->next();
            if (tok && tok->str() == "{")
                ++indentspecial;
            else if (!tok)
                break;
            else
                continue;
        }

        else if (Token::Match(tok, "namespace|struct|union {")) {
            tok = tok->next();
            ++indentspecial;
        }

        else if (tok->str() == "{") {
            if ((!beginfunction && !indentlevel) ||
                (tok->previous() && tok->previous()->str() == "="))
                tok = tok->link();
            else
                ++indentlevel;
        }

        else if (tok->str() == "}") {
            if (indentlevel == 0) {
                if (indentspecial)
                    --indentspecial;
            } else {
                --indentlevel;
                if (indentlevel == 0) {
                    gotos.clear();
                    beginfunction = 0;
                }
            }
        }

        if (!indentlevel && Token::Match(tok, ") const| {"))
            beginfunction = tok;

        else if (indentlevel && Token::Match(tok, "[{};] goto %var% ;"))
            gotos.push_back(tok->next());

        else if (indentlevel == 1 && Token::Match(tok, "[{};] %var% : ;") && tok->next()->str() != "default") {
            // Is this label at the end..
            bool end = false;
            unsigned int level = 0;
            for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(" || tok2->str() == "[")
                    tok2 = tok2->link();

                else if (tok2->str() == "{") {
                    ++level;
                }

                else if (tok2->str() == "}") {
                    if (!level) {
                        end = true;
                        break;
                    }
                    --level;
                }

                if ((Token::Match(tok2, "[{};] %var% : ;") && tok2->next()->str() != "default") ||
                    Token::Match(tok2, "[{};] goto %var% ;")) {
                    break;
                }
            }
            if (!end)
                continue;

            const std::string name(tok->next()->str());

            tok->deleteNext(3);

            // This label is at the end of the function.. replace all matching goto statements..
            for (std::list<Token *>::iterator it = gotos.begin(); it != gotos.end(); ++it) {
                Token *token = *it;
                if (token->next()->str() == name) {
                    // Delete the "goto name;"
                    token = token->previous();
                    // change 'tok' before 'goto' if it coincides with the ';' token after 'name'
                    if (token->tokAt(3) == tok)
                        tok = token;
                    token->deleteNext(3);

                    // Insert the statements..
                    bool ret = false;   // is there return
                    bool ret2 = false;  // is there return in indentlevel 0
                    std::list<Token*> links;
                    std::list<Token*> links2;
                    std::list<Token*> links3;
                    unsigned int lev = 0;
                    unsigned int roundbraces = 0;
                    for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == ")") {
                            if (!roundbraces)
                                break;
                            --roundbraces;
                        }
                        if (tok2->str() == "(")
                            ++roundbraces;

                        if (!roundbraces && tok2->str() == "}") {
                            if (!lev)
                                break;
                            --lev;
                        } else if (!roundbraces && tok2->str() == "{") {
                            ++lev;
                        } else if (!roundbraces && tok2->str() == "return") {
                            ret = true;
                            if (indentlevel == 1 && lev == 0)
                                ret2 = true;
                        }
                        token->insertToken(tok2->str());
                        token = token->next();
                        token->linenr(tok2->linenr());
                        token->varId(tok2->varId());
                        if (ret2 && roundbraces == 0 && tok2->str() == ";") {
                            break;
                        }
                        if (token->str() == "(") {
                            links.push_back(token);
                        } else if (token->str() == ")") {
                            if (links.empty()) {
                                // This should never happen at this point
                                syntaxError(token, ')');
                                return;
                            }

                            Token::createMutualLinks(links.back(), token);
                            links.pop_back();
                        } else if (token->str() == "{") {
                            links2.push_back(token);
                        } else if (token->str() == "}") {
                            if (links2.empty()) {
                                // This should never happen at this point
                                syntaxError(token, '}');
                                return;
                            }

                            Token::createMutualLinks(links2.back(), token);
                            links2.pop_back();
                        } else if (token->str() == "[") {
                            links3.push_back(token);
                        } else if (token->str() == "]") {
                            if (links3.empty()) {
                                // This should never happen at this point
                                syntaxError(token, ']');
                                return;
                            }

                            Token::createMutualLinks(links3.back(), token);
                            links3.pop_back();
                        }
                    }

                    if (!ret) {
                        token->insertToken(";");
                        token->insertToken("return");
                    }
                }
            }

            // goto the end of the function
            if (tok->str() == "{")
                tok = tok->link();
            else {
                while (tok) {
                    if (tok->str() == "{")
                        tok = tok->link();
                    else if (tok->str() == "}")
                        break;
                    tok = tok->next();
                }
            }
            if (!tok)
                break;
            gotos.clear();
            beginfunction = 0;
            indentlevel = 0;
            continue;
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
        while (Token::simpleMatch(tok2, "strcat ( strcat")) {
            tok2 = tok2->tokAt(2);
        }

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

void Tokenizer::duplicateEnumError(const Token * tok1, const Token * tok2, const std::string & type) const
{
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<const Token*> locationList;
    locationList.push_back(tok1);
    locationList.push_back(tok2);
    const std::string tok2_str = tok2 ? tok2->str() : std::string("name");

    reportError(locationList, Severity::style, "variableHidingEnum",
                std::string(type + " '" + tok2_str + "' hides enumerator with same name"));
}

// Check if this statement is a duplicate definition.  A duplicate
// definition will hide the enumerator within it's scope so just
// skip the entire scope of the duplicate.
bool Tokenizer::duplicateDefinition(Token ** tokPtr, const Token * name) const
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
                if (Token::Match(end->link()->previous(), "if|for|while|switch|BOOST_FOREACH"))
                    return false;

                // look backwards
                if (tok->previous()->str() == "enum" ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return") ||
                    Token::Match(tok->tokAt(-2), "%type% &|*")) {
                    duplicateEnumError(*tokPtr, name, "Function parameter");
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
                        duplicateEnumError(*tokPtr, name, "Template parameter");
                        *tokPtr = end->link();
                        return true;
                    }
                }
            } else {
                if (Token::Match(tok->previous(), "enum|,")) {
                    duplicateEnumError(*tokPtr, name, "Variable");
                    return true;
                } else if (Token::Match(tok->previous(), "%type%")) {
                    // look backwards
                    const Token *back = tok;
                    while (back && back->isName())
                        back = back->previous();
                    if (!back || (Token::Match(back, "[(,;{}]") && !Token::Match(back->next(),"return|throw"))) {
                        duplicateEnumError(*tokPtr, name, "Variable");
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

class EnumValue {
public:
    EnumValue() {
        name  = 0;
        value = 0;
        start = 0;
        end   = 0;
    }
    EnumValue(const EnumValue &ev) {
        name  = ev.name;
        value = ev.value;
        start = ev.start;
        end   = ev.end;
    }
    EnumValue(Token *name_, Token *value_, Token *start_, Token *end_) {
        name  = name_;
        value = value_;
        start = start_;
        end   = end_;
    }

    void simplify(const std::map<std::string, EnumValue> &enumValues) {
        for (Token *tok = start; tok; tok = tok->next()) {
            if (enumValues.find(tok->str()) != enumValues.end()) {
                const EnumValue &other = enumValues.find(tok->str())->second;
                if (other.value != NULL)
                    tok->str(other.value->str());
                else {
                    bool islast = (tok == end);
                    Token *last = Tokenizer::copyTokens(tok, other.start, other.end);
                    if (last == tok->next())  // tok->deleteThis() invalidates a pointer that points at the next token
                        last = tok;
                    tok->deleteThis();
                    if (islast) {
                        end = last;
                    }
                    tok = last;
                }
            }
            if (tok == end)
                break;
        }

        // Simplify calculations..
        while (start && start->previous() && TemplateSimplifier::simplifyNumericCalculations(start->previous())) { }

        if (Token::Match(start, "%num% [,}]")) {
            value = start;
            start = end = NULL;
        }
    }

    Token *name;
    Token *value;
    Token *start;
    Token *end;
};

void Tokenizer::simplifyEnum()
{
    std::string className;
    int classLevel = 0;
    bool goback = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {

        if (goback) {
            //jump back once, see the comment at the end of the function
            goback = false;
            tok = tok->previous();
        }

        if (Token::Match(tok, "class|struct|namespace") && tok->next() &&
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum"))) {
            className = tok->next()->str();
            classLevel = 0;
        } else if (tok->str() == "}") {
            --classLevel;
            if (classLevel < 0)
                className = "";
        } else if (tok->str() == "{") {
            ++classLevel;
        } else if (tok->str() == "enum") {
            Token *temp = tok->next();
            if (!temp)
                break;
            if (Token::Match(temp, "class|struct"))
                temp = temp->next();
            if (!Token::Match(temp, "[{:]") &&
                (!temp->isName() || !Token::Match(temp->next(), "[{:;]")))
                continue;
            Token *start = tok;
            Token *enumType = 0;
            Token *typeTokenStart = 0;
            Token *typeTokenEnd = 0;

            // check for C++0x enum class
            if (Token::Match(tok->next(), "class|struct"))
                tok->deleteNext();

            // check for name
            if (tok->next()->isName()) {
                enumType = tok->next();
                tok = tok->next();
            }

            // check for C++0x typed enumeration
            if (tok->next()->str() == ":") {
                tok = tok->next();

                if (!tok->next()) {
                    syntaxError(tok);
                    return; // can't recover
                }

                typeTokenStart = tok->next();
                tok = tok->next();
                typeTokenEnd = typeTokenStart;

                while (typeTokenEnd->next() && (typeTokenEnd->next()->str() == "::" ||
                                                Token::Match(typeTokenEnd->next(), "%type%"))) {
                    typeTokenEnd = typeTokenEnd->next();
                    tok = tok->next();
                }

                if (!tok->next()) {
                    syntaxError(tok);
                    return; // can't recover
                }
            }

            // check for forward declaration
            if (tok->next()->str() == ";") {
                tok = tok->next();

                /** @todo start substitution check at forward declaration */
                // delete forward declaration
                Token::eraseTokens(start, tok);
                start->deleteThis();
                tok = start;
                continue;
            } else if (tok->next()->str() != "{") {
                syntaxError(tok->next());
                return;
            }

            Token *tok1 = tok->next();
            Token *end = tok1->link();
            tok1 = tok1->next();

            MathLib::bigint lastValue = -1;
            Token * lastEnumValueStart = 0;
            Token * lastEnumValueEnd = 0;

            // iterate over all enumerators between { and }
            // Give each enumerator the const value specified or if not specified, 1 + the
            // previous value or 0 if it is the first one.
            std::map<std::string,EnumValue> enumValues;
            for (; tok1 && tok1 != end; tok1 = tok1->next()) {
                Token * enumName = 0;
                Token * enumValue = 0;
                Token * enumValueStart = 0;
                Token * enumValueEnd = 0;

                if (tok1->str() == "(") {
                    tok1 = tok1->link();
                    continue;
                }

                if (Token::Match(tok1->previous(), ",|{ %type% ,|}")) {
                    // no value specified
                    enumName = tok1;
                    ++lastValue;
                    tok1->insertToken("=");
                    tok1 = tok1->next();

                    if (lastEnumValueStart && lastEnumValueEnd) {
                        // previous value was an expression
                        Token *valueStart = tok1;
                        tok1 = copyTokens(tok1, lastEnumValueStart, lastEnumValueEnd);

                        // value is previous expression + 1
                        tok1->insertToken("+");
                        tok1 = tok1->next();
                        tok1->insertToken("1");
                        enumValue = 0;
                        enumValueStart = valueStart->next();
                        enumValueEnd = tok1->next();
                    } else {
                        // value is previous numeric value + 1
                        tok1->insertToken(MathLib::toString(lastValue));
                        enumValue = tok1->next();
                    }
                } else if (Token::Match(tok1->previous(), ",|{ %type% = %num% ,|}")) {
                    // value is specified numeric value
                    enumName = tok1;
                    lastValue = MathLib::toLongNumber(tok1->strAt(2));
                    enumValue = tok1->tokAt(2);
                    lastEnumValueStart = 0;
                    lastEnumValueEnd = 0;
                } else if (Token::Match(tok1->previous(), ",|{ %type% =")) {
                    // value is specified expression
                    enumName = tok1;
                    lastValue = 0;
                    tok1 = tok1->tokAt(2);
                    enumValueStart = tok1;
                    enumValueEnd = tok1;
                    int level = 0;
                    while (enumValueEnd->next() && (!Token::Match(enumValueEnd->next(), "[},]") || level)) {
                        if (Token::Match(enumValueEnd, "(|["))
                            ++level;
                        else if (Token::Match(enumValueEnd->next(), "]|)"))
                            --level;
                        else if (Token::Match(enumValueEnd, "%type% <") && isCPP() && TemplateSimplifier::templateParameters(enumValueEnd->next()) > 1U) {
                            Token *endtoken = enumValueEnd->tokAt(2);
                            while (Token::Match(endtoken,"%any% *| [,>]") && (endtoken->isName() || endtoken->isNumber())) {
                                endtoken = endtoken->next();
                                if (endtoken->str() == "*")
                                    endtoken = endtoken->next();
                                if (endtoken->str() == ",")
                                    endtoken = endtoken->next();
                            }
                            if (endtoken->str() == ">") {
                                enumValueEnd = endtoken;
                                if (Token::simpleMatch(endtoken, "> ( )"))
                                    enumValueEnd = enumValueEnd->next();
                            } else {
                                syntaxError(enumValueEnd);
                                return;
                            }
                        }

                        enumValueEnd = enumValueEnd->next();
                    }
                    // remember this expression in case it needs to be incremented
                    lastEnumValueStart = enumValueStart;
                    lastEnumValueEnd = enumValueEnd;
                    // skip over expression
                    tok1 = enumValueEnd;
                }

                // add enumerator constant..
                if (enumName && (enumValue || (enumValueStart && enumValueEnd))) {
                    EnumValue ev(enumName, enumValue, enumValueStart, enumValueEnd);
                    ev.simplify(enumValues);
                    enumValues[enumName->str()] = ev;
                    lastEnumValueStart = ev.start;
                    lastEnumValueEnd = ev.end;
                    if (ev.start == NULL)
                        lastValue = MathLib::toLongNumber(ev.value->str());
                    tok1 = ev.end ? ev.end : ev.value;
                }
            }

            // Substitute enum values
            {
                if (_settings->terminated())
                    return;

                const std::string pattern = className.empty() ?
                                            std::string("") :
                                            std::string(className + " :: ");
                int level = 0;
                bool inScope = true;

                std::stack<std::set<std::string> > shadowId;  // duplicate ids in inner scope
                bool simplify = false;
                bool hasClass = false;
                EnumValue *ev = NULL;

                if (!tok1)
                    return;
                for (Token *tok2 = tok1->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "}") {
                        --level;
                        if (level < 0)
                            inScope = false;

                        if (!shadowId.empty())
                            shadowId.pop();
                    } else if (tok2->str() == "{") {
                        // Is the same enum redefined?
                        const Token *begin = end->link();
                        if (tok2->fileIndex() == begin->fileIndex() &&
                            tok2->linenr() == begin->linenr() &&
                            Token::Match(begin->tokAt(-2), "enum %type% {") &&
                            Token::Match(tok2->tokAt(-2), "enum %type% {") &&
                            begin->previous()->str() == tok2->previous()->str()) {
                            // remove duplicate enum
                            Token * startToken = tok2->tokAt(-3);
                            tok2 = tok2->link()->next();
                            Token::eraseTokens(startToken, tok2);
                            if (!tok2)
                                break;
                        } else {
                            // Not a duplicate enum..
                            ++level;

                            // Create a copy of the shadow ids for the inner scope
                            if (!shadowId.empty())
                                shadowId.push(shadowId.top());

                            // are there shadow arguments?
                            if (Token::simpleMatch(tok2->previous(), ") {") || Token::simpleMatch(tok2->tokAt(-2), ") const {")) {
                                std::set<std::string> shadowArg;
                                for (const Token* arg = tok2; arg && arg->str() != "("; arg = arg->previous()) {
                                    if (Token::Match(arg->previous(), "%type%|*|& %type% [,)]") &&
                                        enumValues.find(arg->str()) != enumValues.end()) {
                                        // is this a variable declaration
                                        const Token *prev = arg;
                                        while (Token::Match(prev,"%type%|*|&"))
                                            prev = prev->previous();
                                        if (!Token::Match(prev,"[,(] %type%"))
                                            continue;
                                        if (prev->str() == "(" && (!Token::Match(prev->tokAt(-2), "%type%|::|*|& %type% (") || prev->strAt(-2) == "else"))
                                            continue;
                                        shadowArg.insert(arg->str());
                                        if (inScope && _settings->isEnabled("style")) {
                                            const EnumValue enumValue = enumValues.find(arg->str())->second;
                                            duplicateEnumError(arg, enumValue.name, "Function argument");
                                        }
                                    }
                                }
                                if (!shadowArg.empty()) {
                                    if (shadowId.empty())
                                        shadowId.push(shadowArg);
                                    else
                                        shadowId.top().insert(shadowArg.begin(), shadowArg.end());
                                }
                            }

                            // are there shadow variables in the scope?
                            std::set<std::string> shadowVars;
                            for (const Token *tok3 = tok2->next(); tok3 && tok3->str() != "}"; tok3 = tok3->next()) {
                                if (tok3->str() == "{")
                                    tok3 = tok3->link(); // skip inner scopes
                                else if (tok3->isName() && enumValues.find(tok3->str()) != enumValues.end()) {
                                    const Token *prev = tok3->previous();
                                    if ((prev->isName() && !Token::Match(prev,"return|case|throw")) ||
                                        Token::Match(prev, "&|* %type% =")) {
                                        // variable declaration?
                                        shadowVars.insert(tok3->str());
                                        if (inScope && _settings->isEnabled("style")) {
                                            const EnumValue enumValue = enumValues.find(tok3->str())->second;
                                            duplicateEnumError(tok3, enumValue.name, "Variable");
                                        }
                                    }
                                }
                            }
                            if (!shadowVars.empty()) {
                                if (shadowId.empty())
                                    shadowId.push(shadowVars);
                                else
                                    shadowId.top().insert(shadowVars.begin(), shadowVars.end());
                            }
                        }

                        // Function head
                    } else if (Token::Match(tok2, "%var% (")) {
                        const Token *prev = tok2->previous();
                        bool type = false;
                        while (prev && (prev->isName() || Token::Match(prev, "*|&|::"))) {
                            type |= Token::Match(prev, "%type% !!::");
                            prev = prev->previous();
                        }
                        if (type && (!prev || Token::Match(prev, "[;{}]"))) {
                            // skip ( .. )
                            tok2 = tok2->next()->link();
                        }
                    } else if (!pattern.empty() && Token::Match(tok2, pattern.c_str()) && enumValues.find(tok2->strAt(2)) != enumValues.end()) {
                        simplify = true;
                        hasClass = true;
                        ev = &(enumValues.find(tok2->strAt(2))->second);
                    } else if (inScope &&    // enum is in scope
                               (shadowId.empty() || shadowId.top().find(tok2->str()) == shadowId.top().end()) &&   // no shadow enum/var/etc of enum
                               enumValues.find(tok2->str()) != enumValues.end()) {    // tok2 is a enum id with a known value
                        ev = &(enumValues.find(tok2->str())->second);
                        if (!duplicateDefinition(&tok2, ev->name)) {
                            if (tok2->strAt(-1) == "::" ||
                                Token::Match(tok2->next(), "::|[")) {
                                // Don't replace this enum if:
                                // * it's preceded or followed by "::"
                                // * it's followed by "["
                            } else {
                                simplify = true;
                                hasClass = false;
                                ev = &(enumValues.find(tok2->str())->second);
                            }
                        } else {
                            // something with the same name.
                            if (shadowId.empty())
                                shadowId.push(std::set<std::string>());
                            shadowId.top().insert(tok2->str());
                        }
                    }

                    if (simplify) {
                        if (ev->value) {
                            tok2->str(ev->value->str());
                            if (hasClass)
                                tok2->deleteNext(2);
                        } else {
                            tok2 = tok2->previous();
                            tok2->deleteNext(hasClass ? 3 : 1);
                            bool hasOp = false;
                            int indentlevel = 0;
                            for (const Token *enumtok = ev->start; enumtok != ev->end; enumtok = enumtok->next()) {
                                if (enumtok->str() == "(")
                                    ++indentlevel;
                                else if (enumtok->str() == ")")
                                    --indentlevel;
                                if (indentlevel == 0)
                                    hasOp |= enumtok->isOp();
                            }
                            if (!hasOp)
                                tok2 = copyTokens(tok2, ev->start, ev->end);
                            else {
                                tok2->insertToken("(");
                                Token *startPar = tok2->next();
                                tok2 = copyTokens(startPar, ev->start, ev->end);
                                tok2->insertToken(")");
                                Token::createMutualLinks(startPar, tok2->next());
                                tok2 = tok2->next();
                            }
                        }

                        simplify = false;
                    }
                }
            }

            // check for a variable definition: enum {} x;
            if (end->next() && end->next()->str() != ";") {
                Token *tempTok = end;

                tempTok->insertToken(";");
                tempTok = tempTok->next();
                if (typeTokenStart == 0)
                    tempTok->insertToken("int");
                else {
                    Token *tempTok1 = typeTokenStart;

                    tempTok->insertToken(tempTok1->str());

                    while (tempTok1 != typeTokenEnd) {
                        tempTok1 = tempTok1->next();

                        tempTok->insertToken(tempTok1->str());
                        tempTok = tempTok->next();
                    }
                }
            }

            if (enumType) {
                const std::string pattern(className.empty() ? std::string("") : (className + " :: " + enumType->str()));

                // count { and } for tok2
                int level = 0;
                bool inScope = true;

                bool exitThisScope = false;
                int exitScope = 0;
                bool simplify = false;
                bool hasClass = false;
                for (Token *tok2 = end->next(); tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "}") {
                        --level;
                        if (level < 0)
                            inScope = false;

                        if (exitThisScope) {
                            if (level < exitScope)
                                exitThisScope = false;
                        }
                    } else if (tok2->str() == "{")
                        ++level;
                    else if (!pattern.empty() && ((tok2->str() == "enum" && Token::Match(tok2->next(), pattern.c_str())) || Token::Match(tok2, pattern.c_str()))) {
                        simplify = true;
                        hasClass = true;
                    } else if (inScope && !exitThisScope && (tok2->str() == enumType->str() || (tok2->str() == "enum" && tok2->next() && tok2->next()->str() == enumType->str()))) {
                        if (tok2->strAt(-1) == "::") {
                            // Don't replace this enum if it's preceded by "::"
                        } else if (tok2->next() &&
                                   (tok2->next()->isName() || tok2->next()->str() == "(")) {
                            simplify = true;
                            hasClass = false;
                        } else if (tok2->previous()->str() == "(" && tok2->next()->str() == ")") {
                            simplify = true;
                            hasClass = false;
                        }
                    }

                    if (simplify) {
                        if (tok2->str() == "enum")
                            tok2->deleteNext();
                        if (typeTokenStart == 0)
                            tok2->str("int");
                        else {
                            Token *tok3 = typeTokenStart;

                            tok2->str(tok3->str());

                            while (tok3 != typeTokenEnd) {
                                tok3 = tok3->next();

                                tok2->insertToken(tok3->str());
                                tok2 = tok2->next();
                            }
                        }

                        if (hasClass) {
                            tok2->deleteNext(2);
                        }

                        simplify = false;
                    }
                }
            }

            tok1 = start;
            Token::eraseTokens(tok1, end->next());
            if (start != list.front()) {
                tok1 = start->previous();
                tok1->deleteNext();
                //no need to remove last token in the list
                if (tok1->tokAt(2))
                    tok1->deleteNext();
                tok = tok1;
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

void Tokenizer::simplifyStd()
{
    if (isC())
        return;

    std::set<std::string> f;
    f.insert("strcat");
    f.insert("strcpy");
    f.insert("strncat");
    f.insert("strncpy");
    f.insert("free");
    f.insert("malloc");
    f.insert("strdup");

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "std")
            continue;

        if (Token::Match(tok->previous(), "[(,{};] std :: %var% (") &&
            f.find(tok->strAt(2)) != f.end()) {
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
    if (unknown)
        *unknown = false;

    if (Token::simpleMatch(endScopeToken->tokAt(-2), ") ; }")) {
        const Token *tok = endScopeToken->linkAt(-2)->previous();

        if (!tok)
            return true;

        // function pointer call..
        if (Token::Match(tok->tokAt(-4), "[;{}] ( * %var% )"))
            return true;

        if (!tok->isName())
            return false;

        if (tok->str() == "exit")
            return true;

        while (tok && (Token::Match(tok, "::|.") || tok->isName()))
            tok = tok->previous();

        if (Token::Match(tok, "[;{}] %var% (")) {
            if (_settings->library.isnoreturn(tok->next()->str()))
                return true;
            if (_settings->library.isnotnoreturn(tok->next()->str()))
                return false;

            if (_settings->checkLibrary && _settings->isEnabled("information")) {
                reportError(tok->next(),
                            Severity::information,
                            "checkLibraryNoReturn",
                            "--check-library: Function " + tok->next()->str() + "() should have <noreturn> configuration");
            }
        }

        if (Token::Match(tok, "[;{}]")) {
            if (unknown)
                *unknown = true;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------

bool Tokenizer::isFunctionParameterPassedByValue(const Token *fpar) const
{
    // TODO: If symbol database is available, use it.
    const Token *ftok;

    // Look at function call, what parameter number is it?
    unsigned int parentheses = 1;
    unsigned int parameter = 1;
    for (ftok = fpar; ftok; ftok = ftok->previous()) {
        if (ftok->str() == "(") {
            --parentheses;
            if (parentheses == 0) {
                break;
            }
        } else if (ftok->str() == ")") {
            ++parentheses;
        } else if (parentheses == 1 && ftok->str() == ",") {
            ++parameter;
        } else if (Token::Match(ftok, "[;{}]")) {
            break;
        }
    }

    // Is this a function call?
    if (ftok && Token::Match(ftok->tokAt(-2), "[;{}=] %var% (")) {
        const std::string functionName(ftok->previous()->str());

        if (functionName == "return")
            return true;

        // Locate function declaration..
        unsigned int indentlevel = 0;
        for (const Token *tok = tokens(); tok; tok = tok->next()) {
            if (tok->str() == "{")
                ++indentlevel;
            else if (tok->str() == "}")
                indentlevel = (indentlevel > 0) ? indentlevel - 1U : 0U;
            else if (indentlevel == 0 && Token::Match(tok, "%type% (") && tok->str() == functionName) {
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
    bool isgoto = Token::Match(begin->tokAt(-2), "goto %var% ;");
    unsigned int indentlevel = 1,
                 indentcase = 0,
                 indentswitch = 0,
                 indentlabel = 0,
                 roundbraces = 0,
                 indentcheck = 0;
    std::vector<unsigned int> switchindents;
    bool checklabel = false;
    Token *tok = begin;
    Token *tokcheck = 0;
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
                    tok = tok->next()->link()->previous();  //return to initial '{'
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
        } else if (Token::Match(tok, "[{};] case")) {
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
        } else if (Token::Match(tok, "[{};] %var% : ;") && tok->next()->str() != "default") {
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
                        } else if (Token::Match(tok2, "[{};] case")) {
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
            std::string labelpattern = "[{};] " + begin->previous()->str() + " : ;";
            Token *start = tok->tokAt(2);
            if (start && start->str() == "(")
                start = start->link()->next();
            if (start && start->str() == "{") {
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
        } else {        //no need to keep the other strings, remove them.
            tok->deleteNext();
        }
    }
}

//---------------------------------------------------------------------------

void Tokenizer::syntaxError(const Token *tok) const
{
    reportError(tok, Severity::error, "syntaxError", "syntax error");
}

void Tokenizer::syntaxError(const Token *tok, char c) const
{
    reportError(tok, Severity::error, "syntaxError",
                std::string("Invalid number of character (") + c + ") " +
                "when these macros are defined: '" + _configuration + "'.");
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
    reportError(tok, Severity::error, "cppcheckError",
                "Analysis failed. If the code is valid then please report this failure.");
}
// ------------------------------------------------------------------------
// Helper function to check wether number is zero (0 or 0.0 or 0E+0) or not?
// @param s --> a string to check
// @return true in case s is zero and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isZeroNumber(const std::string &s)
{
    const bool isInteger = MathLib::isInt(s);
    const bool isFloat = MathLib::isFloat(s);
    const bool isZeroValue = ((isInteger && (MathLib::toLongNumber(s) == 0L)) // case: integer number
                              || (isFloat && MathLib::toString(MathLib::toDoubleNumber(s)) == "0.0")); // case: float number

    return isZeroValue;
}

// ------------------------------------------------------------------------
// Helper function to check wether number is one (1 or 0.1E+1 or 1E+0) or not?
// @param s --> a string to check
// @return true in case s is one and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isOneNumber(const std::string &s)
{
    const bool isPositive = MathLib::isPositive(s);
    const bool isInteger = MathLib::isInt(s);
    const bool isFloat = MathLib::isFloat(s);
    const bool isZeroValue = ((isPositive && isInteger && (MathLib::toLongNumber(s) == 1L)) // case: integer number
                              || (isPositive && isFloat && MathLib::toString(MathLib::toDoubleNumber(s)) == "1.0")); // case: float number

    return isZeroValue;
}

// ------------------------------------------------------------------------
// Helper function to check wether number is one (2 or 0.2E+1 or 2E+0) or not?
// @param s --> a string to check
// @return true in case s is two and false otherwise.
// ------------------------------------------------------------------------
bool Tokenizer::isTwoNumber(const std::string &s)
{
    const bool isPositive = MathLib::isPositive(s);
    const bool isInteger = MathLib::isInt(s);
    const bool isFloat = MathLib::isFloat(s);
    const bool isZeroValue = ((isPositive && isInteger && (MathLib::toLongNumber(s) == 2L)) // case: integer number
                              || (isPositive && isFloat && MathLib::toString(MathLib::toDoubleNumber(s)) == "2.0")); // case: float number

    return isZeroValue;
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
bool Tokenizer::simplifyMathFunctions()
{
    bool simplifcationMade = false;
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "atol ( %str% )")) { //@todo Add support for atoll()
            if (tok->previous() &&
                Token::simpleMatch(tok->tokAt(-2), "std ::")) {
                tok = tok->tokAt(-2);// set token index two steps back
                tok->deleteNext(2);  // delete "std ::"
            }
            const std::string strNumber = tok->tokAt(2)->strValue(); // get number
            const bool isNotAnInteger = (!MathLib::isInt(strNumber));// check: is not an integer
            if (!strNumber.empty() && isNotAnInteger) {
                // Ignore strings which we can't convert
                continue;
            }
            // remove atol ( %num%
            tok->deleteNext(3);
            // Convert string into a number and insert into token list
            tok->str(MathLib::toString(MathLib::toLongNumber(strNumber)));
            simplifcationMade = true;
        } else if (Token::Match(tok, "abs|fabs|labs|llabs ( %num% )")) {
            if (tok->previous() &&
                Token::simpleMatch(tok->tokAt(-2), "std ::")) {
                tok = tok->tokAt(-2);// set token index two steps back
                tok->deleteNext(2);  // delete "std ::"
            }
            // get number string
            std::string strNumber(tok->tokAt(2)->str());
            // is the string negative?
            if (!strNumber.empty() && strNumber[0] == '-') {
                strNumber = strNumber.substr(1); // remove '-' sign
            }
            tok->deleteNext(3);  // delete e.g. abs ( 1 )
            tok->str(strNumber); // insert result into token list
            simplifcationMade = true;
        } else if (Token::Match(tok, "fma|fmaf|fmal ( %any% , %any% , %any% )")) {
            // Simplify: fma(a,b,c) == > ( a ) * ( b ) + ( c )
            // get parameters
            const std::string a(tok->tokAt(2)->str());
            const std::string b(tok->tokAt(4)->str());
            const std::string c(tok->tokAt(6)->str());
            if (!a.empty() && !b.empty() && !c.empty()) {
                tok->deleteNext(7);  // delete fma call
                tok->str("( " + a + " ) * ( " + b + " ) + ( " + c + " )");  // insert result into token list
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "sqrt|sqrtf|sqrtl|cbrt|cbrtf|cbrtl ( %num% )")) {
            // Simplify: sqrt(0) = 0 and cbrt(0) == 0
            //           sqrt(1) = 1 and cbrt(1) == 1
            // get number string
            const std::string parameter(tok->tokAt(2)->str());
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
            const std::string parameter(tok->tokAt(2)->str());
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
            const std::string parameter(tok->tokAt(2)->str());
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
            const std::string parameter(tok->tokAt(2)->str());
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
            const std::string strLeftNumber(tok->tokAt(2)->str());
            const std::string strRightNumber(tok->tokAt(4)->str());
            const bool isLessEqual =  MathLib::isLessEqual(strLeftNumber, strRightNumber);
            // case: left <= right ==> insert left
            if (!strLeftNumber.empty() && !strRightNumber.empty() && isLessEqual) {
                tok->deleteNext(5);      // delete e.g. fmin ( -1.0, 1.0 )
                tok->str(strLeftNumber); // insert e.g. -1.0
                simplifcationMade = true;
            } else { // case left > right ==> insert right
                tok->deleteNext(5);       // delete e.g. fmin ( 1.0, 0.0 )
                tok->str(strRightNumber); // insert e.g. 0.0
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "fmax|fmaxl|fmaxf ( %num% , %num% )")) {
            // @todo if one of the parameters is NaN the other is returned
            // e.g. printf ("fmax (NaN, -1.0) = %f\n", fmax(NaN,-1.0));
            // e.g. printf ("fmax (-1.0, NaN) = %f\n", fmax(-1.0,NaN));
            const std::string strLeftNumber(tok->tokAt(2)->str());
            const std::string strRightNumber(tok->tokAt(4)->str());
            const bool isLessEqual =  MathLib::isLessEqual(strLeftNumber, strRightNumber);
            // case: left <= right ==> insert right
            if (!strLeftNumber.empty() && !strRightNumber.empty() && isLessEqual) {
                tok->deleteNext(5);      // delete e.g. fmax ( -1.0, 1.0 )
                tok->str(strRightNumber);// insert e.g. 1.0
                simplifcationMade = true;
            } else { // case left > right ==> insert left
                tok->deleteNext(5);       // delete e.g. fmax ( 1.0, 0.0 )
                tok->str(strLeftNumber);  // insert e.g. 1.0
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "isgreater ( %num% , %num% )")) {
            // The isgreater(x,y) function is the same as calculating (x)>(y).
            // It returns true (1) if x is greater than y and false (0) otherwise.
            const std::string strLeftNumber(tok->tokAt(2)->str()); // get left number
            const std::string strRightNumber(tok->tokAt(4)->str()); // get right number
            if (!strRightNumber.empty() && !strLeftNumber.empty()) {
                const bool isGreater =  MathLib::isGreater(strLeftNumber, strRightNumber); // compare numbers
                tok->deleteNext(5); // delete tokens
                tok->str((isGreater == true) ? "true": "false");  // insert results
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "isgreaterequal ( %num% , %num% )")) {
            // The isgreaterequal(x,y) function is the same as calculating (x)>=(y).
            // It returns true (1) if x is greater than or equal to y.
            // False (0) is returned otherwise.
            const std::string strLeftNumber(tok->tokAt(2)->str()); // get left number
            const std::string strRightNumber(tok->tokAt(4)->str()); // get right number
            if (!strRightNumber.empty() && !strLeftNumber.empty()) {
                const bool isGreaterEqual =  MathLib::isGreaterEqual(strLeftNumber, strRightNumber); // compare numbers
                tok->deleteNext(5); // delete tokens
                tok->str((isGreaterEqual == true) ? "true": "false");  // insert results
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "isless ( %num% , %num% )")) {
            // Calling this function is the same as calculating (x)<(y).
            // It returns true (1) if x is less than y.
            // False (0) is returned otherwise.
            const std::string strLeftNumber(tok->tokAt(2)->str()); // get left number
            const std::string strRightNumber(tok->tokAt(4)->str()); // get right number
            if (!strRightNumber.empty() && !strLeftNumber.empty()) {
                const bool isLess = MathLib::isLess(strLeftNumber, strRightNumber); // compare numbers
                tok->deleteNext(5); // delete tokens
                tok->str((isLess == true) ? "true": "false");  // insert results
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "islessequal ( %num% , %num% )")) {
            // Calling this function is the same as calculating (x)<=(y).
            // It returns true (1) if x is less or equal to y.
            // False (0) is returned otherwise.
            const std::string strLeftNumber(tok->tokAt(2)->str()); // get left number
            const std::string strRightNumber(tok->tokAt(4)->str()); // get right number
            if (!strRightNumber.empty() && !strLeftNumber.empty()) {
                const bool isLessEqual = MathLib::isLessEqual(strLeftNumber, strRightNumber); // compare numbers
                tok->deleteNext(5); // delete tokens
                tok->str((isLessEqual == true) ? "true": "false");  // insert results
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "islessgreater ( %num% , %num% )")) {
            // Calling this function is the same as calculating (x)<(y) || (x)>(y).
            // It returns true (1) if x is less than y or x is greater than y.
            // False (0) is returned otherwise.
            const std::string strLeftNumber(tok->tokAt(2)->str()); // get left number
            const std::string strRightNumber(tok->tokAt(4)->str()); // get right number
            if (!strRightNumber.empty() && !strLeftNumber.empty()) {
                const bool isLessOrGreater(MathLib::isLess(strLeftNumber, strRightNumber) ||
                                           MathLib::isGreater(strLeftNumber, strRightNumber));  // compare numbers
                tok->deleteNext(5); // delete tokens
                tok->str((isLessOrGreater == true) ? "true": "false");  // insert results
                simplifcationMade = true;
            }
        } else if (Token::Match(tok, "div|ldiv|lldiv ( %any% , %num% )")) {
            // Calling the function 'div(x,y)' is the same as calculating (x)/(y). In case y has the value 1
            // (the identity element), the call can be simplified to (x).
            const std::string leftParameter(tok->tokAt(2)->str()); // get the left parameter
            const std::string rightNumber(tok->tokAt(4)->str()); // get right number
            if (!rightNumber.empty() && !leftParameter.empty()) {
                if (isOneNumber(rightNumber)) {
                    tok->deleteNext(5); // delete tokens
                    tok->str(leftParameter);  // insert simplified result
                    simplifcationMade = true;
                }
            }
        } else if (Token::Match(tok, "pow|powf|powl (")) {
            if (tok && Token::Match(tok->tokAt(2), "%num% , %num% )")) {
                // In case of pow ( 0 , anyNumber > 0): It can be simplified to 0
                // In case of pow ( 0 , 0 ): It simplified to 1
                // In case of pow ( 1 , anyNumber ): It simplified to 1
                const std::string leftNumber(tok->tokAt(2)->str()); // get the left parameter
                const std::string rightNumber(tok->tokAt(4)->str()); // get the right parameter
                if (!leftNumber.empty() && !rightNumber.empty()) {
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
            }
            if (tok && Token::Match(tok->tokAt(2), "%any% , %num% )")) {
                // In case of pow( x , 1 ): It can be simplified to x.
                const std::string leftParameter(tok->tokAt(2)->str()); // get the left parameter
                const std::string rightNumber(tok->tokAt(4)->str()); // get right number
                if (!rightNumber.empty() && !leftParameter.empty()) {
                    if (isOneNumber(rightNumber)) { // case: x^(1) = x
                        tok->deleteNext(5); // delete tokens
                        tok->str(leftParameter);  // insert simplified result
                        simplifcationMade = true;
                    } else if (isZeroNumber(rightNumber)) { // case: x^(0) = 1
                        tok->deleteNext(5); // delete tokens
                        tok->str("1");  // insert simplified result
                        simplifcationMade = true;
                    }
                }
            }
        }
    }
    // returns true if a simplifcation was performed and false otherwise.
    return simplifcationMade;
}

void Tokenizer::simplifyComma()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {

        if (tok->str() == "(" || tok->str() == "[" ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "=")) {
            tok = tok->link();
            continue;
        }

        // Skip unhandled template specifiers..
        if (Token::Match(tok, "%var% <")) {
            Token* tok2 = tok->next()->link();
            if (tok2)
                tok = tok2;
        }

        if (!tok->next() || tok->str() != ",")
            continue;

        // We must not accept just any keyword, e.g. accepting int
        // would cause function parameters to corrupt.
        if (tok->strAt(1) == "delete") {
            // Handle "delete a, delete b;"
            tok->str(";");
        }

        if (tok->previous() && tok->tokAt(-2)) {
            if (Token::Match(tok->tokAt(-2), "delete %var% , %var% ;") &&
                tok->next()->varId() != 0) {
                // Handle "delete a, b;"
                tok->str(";");
                tok->insertToken("delete");
            } else {
                for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous()) {
                    if (tok2->str() == "=") {
                        // Handle "a = 0, b = 0;"
                        tok->str(";");
                        break;
                    } else if (Token::Match(tok2, "delete %var%") ||
                               Token::Match(tok2, "delete [ ] %var%")) {
                        // Handle "delete a, a = 0;"
                        tok->str(";");
                        break;
                    } else if (Token::Match(tok2, "[:;,{}()]")) {
                        break;
                    }
                }
            }
        }

        bool inReturn = false;
        Token *startFrom = NULL;    // "[;{}]" token before "return"
        Token *endAt = NULL;        // first ";" token after "[;{}] return"

        // find "; return" pattern before comma
        for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous()) {
            if (Token::Match(tok2, "[;{}]")) {
                break;

            } else if (tok2->str() == ")" || tok2->str() == "]" ||
                       (tok2->str() == "}" && tok2->link()->previous() && tok2->link()->previous()->str() == "=")) {
                tok2 = tok2->link();

            } else if (tok2->str() == "return" && Token::Match(tok2->previous(), "[;{}]")) {
                inReturn = true;
                startFrom = tok2->previous();
                break;
            }
        }

        // find token where return ends and also count commas
        if (inReturn) {
            std::size_t commaCounter = 0;

            for (Token *tok2 = startFrom->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    endAt = tok2;
                    break;

                } else if (tok2->str() == "(" || tok2->str() == "[" ||
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
                    if (tok2->str() == "(" || tok2->str() == "[" ||
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


void Tokenizer::removeExceptionSpecifications()
{
    if (isC())
        return;

    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, ") const| throw|noexcept (")) {
            if (tok->next()->str() == "const") {
                Token::eraseTokens(tok->next(), tok->linkAt(3));
                tok = tok->next();
            } else
                Token::eraseTokens(tok, tok->linkAt(2));
            tok->deleteNext();
        } else if (Token::Match(tok, ") const| noexcept ;|{|const")) {
            if (tok->next()->str() == "const")
                tok->next()->deleteNext();
            else
                tok->deleteNext();
        }
    }
}



bool Tokenizer::validate() const
{
    std::stack<const Token *> linktok;
    const Token *lastTok = 0;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        lastTok = tok;
        if (Token::Match(tok, "[{([]") || (tok->str() == "<" && tok->link())) {
            if (tok->link() == 0) {
                cppcheckError(tok);
                return false;
            }

            linktok.push(tok);
        }

        else if (Token::Match(tok, "[})]]") || (tok->str() == ">" && tok->link())) {
            if (tok->link() == 0) {
                cppcheckError(tok);
                return false;
            }

            if (linktok.empty() == true) {
                cppcheckError(tok);
                return false;
            }

            if (tok->link() != linktok.top()) {
                cppcheckError(tok);
                return false;
            }

            if (tok != tok->link()->link()) {
                cppcheckError(tok);
                return false;
            }

            linktok.pop();
        }

        else if (tok->link() != 0) {
            cppcheckError(tok);
            return false;
        }
    }

    if (!linktok.empty()) {
        cppcheckError(linktok.top());
        return false;
    }

    // Validate that the Tokenizer::list.back() is updated correctly during simplifications
    if (lastTok != list.back()) {
        cppcheckError(lastTok);
        return false;
    }

    return true;
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
            while (std::isxdigit(str[i+sz]) && sz < 4)
                sz++;
            if (sz > 2) {
                std::istringstream istr(str.substr(i+2, sz-2));
                istr >> std::hex >> c;
            }
        } else if (MathLib::isOctalDigit(str[i+1])) {
            sz = 2;
            while (MathLib::isOctalDigit(str[i+sz]) && sz < 4)
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


void Tokenizer::simplifyStructInit()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] struct| %type% %var% = { . %type% =")) {
            // Goto "." and check if the initializations have an expected format
            const Token *tok2 = tok;
            while (tok2->str() != ".")
                tok2 = tok2->next();
            while (tok2 && tok2->str() == ".") {
                if (Token::Match(tok2, ". %type% = %num%|%var% [,}]"))
                    tok2 = tok2->tokAt(4);
                else if (Token::Match(tok2, ". %type% = & %var% [,}]"))
                    tok2 = tok2->tokAt(5);
                else
                    break;

                if (Token::simpleMatch(tok2, ", ."))
                    tok2 = tok2->next();
            }
            if (!Token::simpleMatch(tok2, "} ;"))
                continue;

            // Known expression format => Perform simplification
            Token *vartok = tok->tokAt(3);
            if (vartok->str() == "=")
                vartok = vartok->previous();
            vartok->next()->str(";");

            Token *tok3 = vartok->tokAt(2);
            tok3->link(0);
            while (Token::Match(tok3, "[{,] . %type% =")) {
                tok3->str(vartok->str());
                tok3->varId(vartok->varId());
                tok3 = tok3->tokAt(5);
                while (!Token::Match(tok3, "[,}]"))
                    tok3 = tok3->next();
                if (tok3->str() == "}") {
                    tok3->deleteThis();
                    break;
                }
                tok3->previous()->insertToken(";");
            }
        }
    }
}


void Tokenizer::simplifyComparisonOrder()
{
    // Use "<" comparison instead of ">"
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;(] %var%|%num% >|>= %num%|%var% [);]")) {
            const Token *operand2 = tok->tokAt(3);
            const std::string op1(tok->next()->str());
            unsigned int var1 = tok->next()->varId();
            tok->next()->str(operand2->str());
            tok->next()->varId(operand2->varId());
            tok->tokAt(3)->str(op1);
            tok->tokAt(3)->varId(var1);
            if (tok->strAt(2) == ">")
                tok->tokAt(2)->str("<");
            else
                tok->tokAt(2)->str("<=");
        } else if (Token::Match(tok, "( %num% ==|!= %var% )")) {
            const std::string op1(tok->next()->str());
            unsigned int var1 = tok->next()->varId();
            tok->next()->str(tok->strAt(3));
            tok->next()->varId(tok->tokAt(3)->varId());
            tok->tokAt(3)->str(op1);
            tok->tokAt(3)->varId(var1);
        }
    }
}

void Tokenizer::simplifyConst()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->isStandardType() && tok->strAt(1) == "const") {
            tok->next()->str(tok->str());
            tok->str("const");
        } else if (Token::Match(tok, "struct %type% const")) {
            tok->tokAt(2)->str(tok->next()->str());
            tok->str("const");
            tok->next()->str("struct");
        } else if (Token::Match(tok, "%type% const") &&
                   (!tok->previous() || Token::Match(tok->previous(), "[;{}(,]")) &&
                   tok->str().find(":") == std::string::npos &&
                   tok->str() != "operator") {
            tok->next()->str(tok->str());
            tok->str("const");
        }
    }
}

void Tokenizer::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
{
    Tokenizer t(settings, errorLogger);
    t.syntaxError(0, ' ');
    t.cppcheckError(0);
    t.duplicateTypedefError(0, 0, "variable");
    t.duplicateDeclarationError(0, 0, "variable");
    t.duplicateEnumError(0, 0, "variable");
    t.unnecessaryQualificationError(0, "type");
}

void Tokenizer::simplifyWhile0()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // while (0)
        const bool while0(Token::Match(tok->previous(), "[{};] while ( 0|false )"));

        // for (0) - not banal, ticket #3140
        const bool for0((Token::Match(tok->previous(), "[{};] for ( %var% = %num% ; %var% < %num% ;") &&
                         tok->strAt(2) == tok->strAt(6) && tok->strAt(4) == tok->strAt(8)) ||
                        (Token::Match(tok->previous(), "[{};] for ( %type% %var% = %num% ; %var% < %num% ;") &&
                         tok->strAt(3) == tok->strAt(7) && tok->strAt(5) == tok->strAt(9)));

        if (!while0 && !for0)
            continue;

        if (while0 && tok->previous()->str() == "}") {
            // find "do"
            Token *tok2 = tok->previous()->link();
            tok2 = tok2->previous();
            if (tok2 && tok2->str() == "do") {
                bool flowmatch = Token::findmatch(tok2, "continue|break", tok) != NULL;
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
            Token *end = tok->next()->link();
            end = end->next()->link();
            tok = tok->previous();
            eraseDeadCode(tok, end->next());
        }
    }
}

void Tokenizer::simplifyErrNoInWhile()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() != "errno")
            continue;

        Token *endpar = 0;
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
        if (!Token::Match(tok, "while ( %var% ( %var% ) ) {"))
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
        // check for anonymous struct/union
        if (Token::Match(tok, "struct|union {")) {
            if (Token::Match(tok->next()->link(), "} *|&| %type% ,|;|[")) {
                tok->insertToken("Anonymous" + MathLib::toString(count++));
            }
        }
    }

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        Token *restart;

        // check for start of scope and determine if it is in a function
        if (tok->str() == "{")
            skip.push(Token::Match(tok->previous(), "const|)"));

        // end of scope
        else if (tok->str() == "}" && !skip.empty())
            skip.pop();

        // check for named struct/union
        else if (Token::Match(tok, "class|struct|union %type% :|{")) {
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
            restart = next;

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|[|=")) {
                tok->insertToken(";");
                tok = tok->next();
                while (!Token::Match(start, "struct|class|union")) {
                    tok->insertToken(start->str());
                    tok = tok->next();
                    start->deleteThis();
                }
                if (!tok)
                    break; // see #4869 segmentation fault in Tokenizer::simplifyStructDecl (invalid code)
                tok->insertToken(type->str());
                if (start->str() != "class")
                    tok->insertToken(start->str());
            }

            tok = restart;
        }

        // check for anonymous struct/union
        else if (Token::Match(tok, "struct|union {")) {
            bool inFunction = skip.top();
            skip.push(false);
            Token *tok1 = tok;

            restart = tok->next();
            tok = tok->next()->link();

            // unnamed anonymous struct/union so possibly remove it
            if (tok && tok->next() && tok->next()->str() == ";") {
                if (tok1->str() == "union" && inFunction) {
                    // Try to create references in the union..
                    Token *tok2 = tok1->tokAt(2);
                    while (tok2) {
                        if (Token::Match(tok2, "%type% %var% ;"))
                            tok2 = tok2->tokAt(3);
                        else
                            break;
                    }
                    if (!Token::simpleMatch(tok2, "} ;"))
                        continue;
                    Token *vartok = 0;
                    tok2 = tok1->tokAt(2);
                    while (Token::Match(tok2, "%type% %var% ;")) {
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
                if (!(tok1->str() == "union" && !inFunction)) {
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
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "__cdecl|__stdcall|__fastcall|__thiscall|__clrcall|__syscall|__pascal|__fortran|__far|__near|WINAPI|APIENTRY|CALLBACK")) {
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyDeclspec()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "__declspec (") && tok->next()->link() && tok->next()->link()->next()) {
            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAttribute()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "__attribute__ (") && tok->next()->link() && tok->next()->link()->next()) {
            if (Token::simpleMatch(tok->tokAt(2), "( constructor )")) {
                tok->next()->link()->next()->isAttributeConstructor(true);
            }

            if (Token::simpleMatch(tok->tokAt(2), "( unused )")) {
                // check if after variable name
                if (Token::Match(tok->next()->link()->next(), ";|=")) {
                    if (Token::Match(tok->previous(), "%type%"))
                        tok->previous()->isAttributeUnused(true);
                }

                // check if before variable name
                else if (Token::Match(tok->next()->link()->next(), "%type%"))
                    tok->next()->link()->next()->isAttributeUnused(true);
            }

            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

// Remove "volatile", "inline", "register", "restrict", "override", "final" and "constexpr"
// "restrict" keyword
//   - New to 1999 ANSI/ISO C standard
//   - Not in C++ standard yet
void Tokenizer::simplifyKeyword()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        while (Token::Match(tok, "volatile|inline|_inline|__inline|__forceinline|register|__restrict|__restrict__")) {
            tok->deleteThis();
        }
    }

    if (_settings->standards.c >= Standards::C99) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            while (tok->str() == "restrict") {
                tok->deleteThis();
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

    if (_settings->standards.cpp >= Standards::CPP11) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            while (tok->str() == "constexpr") {
                tok->deleteThis();
            }

            // final:
            // void f() final;  <- function is final
            // struct name final { };   <- struct is final
            if (Token::Match(tok, ") final [{;]") || Token::Match(tok, "%type% final [:{]"))
                tok->deleteNext();

            // override
            // void f() override;
            else if (Token::Match(tok, ") override [{;]"))
                tok->deleteNext();
            else if (Token::Match(tok, ") const override [{;]"))
                tok->next()->deleteNext();
        }
    }
}

void Tokenizer::simplifyAssignmentInFunctionCall()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (tok->str() == "(")
            tok = tok->link();

        // Find 'foo(var='. Exclude 'assert(var=' to allow tests to check that assert(...) does not contain side-effects
        else if (Token::Match(tok, "[;{}] %var% ( %var% =") &&
                 Token::simpleMatch(tok->linkAt(2), ") ;") &&
                 !Token::Match(tok->next(), "assert|while")) {
            const std::string funcname(tok->next()->str());
            const Token * const vartok = tok->tokAt(3);

            // Goto ',' or ')'..
            for (Token *tok2 = tok->tokAt(4); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == ";")
                    break;
                else if (tok2->str() == ")" || tok2->str() == ",") {
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
        if (Token::Match(tok, "[;{}] %var% = ( {")) {
            // goto the "} )"
            unsigned int indentlevel = 0;
            Token *tok2 = tok;
            while (NULL != (tok2 = tok2->next())) {
                if (tok2->str() == "(" || tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == ")" || tok2->str() == "}") {
                    if (indentlevel <= 2)
                        break;
                    --indentlevel;
                }
            }
            if (indentlevel == 2 && Token::simpleMatch(tok2, "} )")) {
                tok2 = tok2->tokAt(-3);
                if (Token::Match(tok2, "[;{}] %num%|%var% ;")) {
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
            while (tok2 && tok2->linenr() == tok->linenr() && (tok2->isNumber() || tok2->isName() || tok2->str() == ","))
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
        tok->insertToken(")");
        tok->insertToken("\"" + instruction + "\"");
        tok->insertToken("(");

        tok = tok->next();
        Token::createMutualLinks(tok, tok->tokAt(2));

        //move the new tokens in the same line as ";" if available
        tok = tok->tokAt(2);
        if (tok->next() && tok->next()->str() == ";" &&
            tok->next()->linenr() != tok->linenr()) {
            unsigned int endposition = tok->next()->linenr();
            tok = tok->tokAt(-3);
            for (int i = 0; i < 4; ++i) {
                tok = tok->next();
                tok->linenr(endposition);
            }
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
        Token *last = 0;

        if (Token::Match(tok, ";|{|}|public:|protected:|private: const| %type% %var% :") &&
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
        } else if (Token::Match(tok, ";|{|}|public:|protected:|private: const| %type% : %any% ;") &&
                   tok->next()->str() != "default") {
            const bool offset = (tok->next()->str() == "const");

            if (!Token::Match(tok->tokAt(3 + (offset ? 1 : 0)), "[{};()]")) {
                tok->deleteNext(4 + (offset ? 1 : 0));
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


// Remove __builtin_expect(...), likely(...), and unlikely(...)
void Tokenizer::simplifyBuiltinExpect()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok->next(), "__builtin_expect (")) {
            // Count parentheses for tok2
            const Token* end = tok->linkAt(2);
            for (Token *tok2 = tok->tokAt(3); tok2 != end; tok2 = tok2->next()) {
                if (tok2->str() == "(") {
                    tok2 = tok2->link();
                } else if (tok2->str() == ",") {
                    if (Token::Match(tok2, ", %num% )")) {
                        tok->deleteNext();
                        tok2->deleteNext();
                        tok2->deleteThis();
                    }
                    break;
                }
            }
        } else if (Token::Match(tok->next(), "likely|unlikely (")) {
            // remove closing ')'
            tok->linkAt(2)->deleteThis();

            // remove "likely|unlikely ("
            tok->deleteNext(2);
        }
    }
}


// Add std:: in front of std classes, when using namespace std; was given
void Tokenizer::simplifyNamespaceStd()
{
    if (!isCPP())
        return;

    static const char* stdTypes_[] = { // Types and objects in std namespace that are neither functions nor templates
        "string", "wstring", "u16string", "u32string",
        "iostream", "ostream", "ofstream", "ostringstream",
        "istream", "ifstream", "istringstream", "fstream", "stringstream",
        "wstringstream", "wistringstream", "wostringstream", "wstringbuf",
        "stringbuf", "streambuf", "ios", "filebuf", "ios_base",
        "exception", "bad_exception", "bad_alloc",
        "logic_error", "domain_error", "invalid_argument_", "length_error",
        "out_of_range", "runtime_error", "range_error", "overflow_error", "underflow_error",
        "locale",
        "cout", "cerr", "clog", "cin",
        "wcerr", "wcin", "wclog", "wcout",
        "endl", "ends", "flush",
        "boolalpha", "noboolalpha", "showbase", "noshowbase",
        "showpoint", "noshowpoint", "showpos", "noshowpos",
        "skipws", "noskipws", "unitbuf", "nounitbuf", "uppercase", "nouppercase",
        "dec", "hex", "oct",
        "fixed", "scientific",
        "internal", "left", "right",
        "fpos", "streamoff", "streampos", "streamsize"
    };
    static const std::set<std::string> stdTypes(stdTypes_, stdTypes_+sizeof(stdTypes_)/sizeof(*stdTypes_));
    static const char* stdTemplates_[] = {
        "array", "basic_string", "bitset", "deque", "list", "map", "multimap",
        "priority_queue", "queue", "set", "multiset", "stack", "vector", "pair",
        "iterator", "iterator_traits",
        "unordered_map", "unordered_multimap", "unordered_set", "unordered_multiset",
        "tuple", "function"
    };
    static const std::set<std::string> stdTemplates(stdTemplates_, stdTemplates_+sizeof(stdTemplates_)/sizeof(*stdTemplates_));
    static const char* stdFunctions_[] = {
        "getline",
        "for_each", "find", "find_if", "find_end", "find_first_of",
        "adjacent_find", "count", "count_if", "mismatch", "equal", "search", "search_n",
        "copy", "copy_backward", "swap", "swap_ranges", "iter_swap", "transform", "replace",
        "replace_if", "replace_copy", "replace_copy_if", "fill", "fill_n", "generate", "generate_n", "remove",
        "remove_if", "remove_copy", "remove_copy_if",
        "unique", "unique_copy", "reverse", "reverse_copy",
        "rotate", "rotate_copy", "random_shuffle", "partition", "stable_partition",
        "sort", "stable_sort", "partial_sort", "partial_sort_copy", "nth_element",
        "lower_bound", "upper_bound", "equal_range", "binary_search", "merge", "inplace_merge", "includes",
        "set_union", "set_intersection", "set_difference",
        "set_symmetric_difference", "push_heap", "pop_heap", "make_heap", "sort_heap",
        "min", "max", "min_element", "max_element", "lexicographical_compare", "next_permutation", "prev_permutation",
        "advance", "back_inserter", "distance", "front_inserter", "inserter",
        "make_pair", "make_shared", "make_tuple"
    };
    static const std::set<std::string> stdFunctions(stdFunctions_, stdFunctions_+sizeof(stdFunctions_)/sizeof(*stdFunctions_));

    for (const Token* tok = Token::findsimplematch(list.front(), "using namespace std ;"); tok; tok = tok->next()) {
        bool insert = false;
        if (Token::Match(tok, "%var% (") && !Token::Match(tok->previous(), ".|::") && stdFunctions.find(tok->str()) != stdFunctions.end())
            insert = true;
        else if (Token::Match(tok, "%var% <") && !Token::Match(tok->previous(), ".|::") && stdTemplates.find(tok->str()) != stdTemplates.end())
            insert = true;
        else if (tok->isName() && !tok->varId() && !Token::Match(tok->next(), "(|<") && !Token::Match(tok->previous(), ".|::") && stdTypes.find(tok->str()) != stdTypes.end())
            insert = true;

        if (insert) {
            tok->previous()->insertToken("std");
            tok->previous()->linenr(tok->linenr()); // For stylistic reasons we put the std:: in the same line as the following token
            tok->previous()->fileIndex(tok->fileIndex());
            tok->previous()->insertToken("::");
        }

        else if (_settings->standards.cpp == Standards::CPP11 && Token::Match(tok, "!!:: tr1 ::"))
            tok->next()->str("std");
    }

    for (Token* tok = list.front(); tok; tok = tok->next()) {
        if (_settings->standards.cpp == Standards::CPP11 && Token::simpleMatch(tok, "std :: tr1 ::"))
            Token::eraseTokens(tok, tok->tokAt(3));

        else if (Token::simpleMatch(tok, "using namespace std ;")) {
            Token::eraseTokens(tok, tok->tokAt(4));
            tok->deleteThis();
        }
    }
}


// Remove Microsoft MFC 'DECLARE_MESSAGE_MAP()'
void Tokenizer::simplifyMicrosoftMFC()
{
    // skip if not Windows
    if (!(_settings->platformType == Settings::Win32A ||
          _settings->platformType == Settings::Win32W ||
          _settings->platformType == Settings::Win64))
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok->next(), "DECLARE_MESSAGE_MAP ( )")) {
            tok->deleteNext(3);
        } else if (Token::Match(tok->next(), "DECLARE_DYNAMIC|DECLARE_DYNAMIC_CLASS|DECLARE_DYNCREATE ( %any% )")) {
            tok->deleteNext(4);
        }
    }
}

void Tokenizer::simplifyMicrosoftMemoryFunctions()
{
    // skip if not Windows
    if (!(_settings->platformType == Settings::Win32A ||
          _settings->platformType == Settings::Win32W ||
          _settings->platformType == Settings::Win64))
        return;

    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "CopyMemory|RtlCopyMemory|RtlCopyBytes (")) {
            tok->str("memcpy");
        } else if (Token::Match(tok, "MoveMemory|RtlMoveMemory (")) {
            tok->str("memmove");
        } else if (Token::Match(tok, "FillMemory|RtlFillMemory|RtlFillBytes (")) {
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
        } else if (Token::Match(tok, "ZeroMemory|RtlZeroMemory|RtlZeroBytes|RtlSecureZeroMemory (")) {
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
        } else if (Token::simpleMatch(tok, "RtlCompareMemory (")) {
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

void Tokenizer::simplifyMicrosoftStringFunctions()
{
    // skip if not Windows
    if (_settings->platformType == Settings::Win32A) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::simpleMatch(tok, "_topen (")) {
                tok->str("open");
            } else if (Token::simpleMatch(tok, "_tfopen (")) {
                tok->str("fopen");
            } else if (Token::simpleMatch(tok, "_tcscat (")) {
                tok->str("strcat");
            } else if (Token::simpleMatch(tok, "_tcschr (")) {
                tok->str("strchr");
            } else if (Token::simpleMatch(tok, "_tcscmp (")) {
                tok->str("strcmp");
            } else if (Token::simpleMatch(tok, "_tcsdup (")) {
                tok->str("strdup");
            } else if (Token::simpleMatch(tok, "_tcscpy (")) {
                tok->str("strcpy");
            } else if (Token::simpleMatch(tok, "_tcslen (")) {
                tok->str("strlen");
            } else if (Token::simpleMatch(tok, "_tcsncat (")) {
                tok->str("strncat");
            } else if (Token::simpleMatch(tok, "_tcsncpy (")) {
                tok->str("strncpy");
            } else if (Token::simpleMatch(tok, "_tcsnlen (")) {
                tok->str("strnlen");
            } else if (Token::simpleMatch(tok, "_tcsrchr (")) {
                tok->str("strrchr");
            } else if (Token::simpleMatch(tok, "_tcsstr (")) {
                tok->str("strstr");
            } else if (Token::simpleMatch(tok, "_tcstok (")) {
                tok->str("strtok");
            } else if (Token::simpleMatch(tok, "_ftprintf (")) {
                tok->str("fprintf");
                tok->originalName("_ftprintf");
            } else if (Token::simpleMatch(tok, "_tprintf (")) {
                tok->str("printf");
                tok->originalName("_tprintf");
            } else if (Token::simpleMatch(tok, "_stprintf (")) {
                tok->str("sprintf");
                tok->originalName("_stprintf");
            } else if (Token::simpleMatch(tok, "_sntprintf (")) {
                tok->str("_snprintf");
                tok->originalName("_sntprintf");
            } else if (Token::simpleMatch(tok, "_ftscanf (")) {
                tok->str("fscanf");
                tok->originalName("_ftscanf");
            } else if (Token::simpleMatch(tok, "_tscanf (")) {
                tok->str("scanf");
                tok->originalName("_tscanf");
            } else if (Token::simpleMatch(tok, "_stscanf (")) {
                tok->str("sscanf");
                tok->originalName("_stscanf");
            } else if (Token::simpleMatch(tok, "_ftprintf_s (")) {
                tok->str("fprintf_s");
                tok->originalName("_ftprintf_s");
            } else if (Token::simpleMatch(tok, "_tprintf_s (")) {
                tok->str("printf_s");
                tok->originalName("_tprintf_s");
            } else if (Token::simpleMatch(tok, "_stprintf_s (")) {
                tok->str("sprintf_s");
                tok->originalName("_stprintf_s");
            } else if (Token::simpleMatch(tok, "_sntprintf_s (")) {
                tok->str("_snprintf_s");
                tok->originalName("_sntprintf_s");
            } else if (Token::simpleMatch(tok, "_ftscanf_s (")) {
                tok->str("fscanf_s");
                tok->originalName("_ftscanf_s");
            } else if (Token::simpleMatch(tok, "_tscanf_s (")) {
                tok->str("scanf_s");
                tok->originalName("_tscanf_s");
            } else if (Token::simpleMatch(tok, "_stscanf_s (")) {
                tok->str("sscanf_s");
                tok->originalName("_stscanf_s");
            } else if (Token::Match(tok, "_T ( %char%|%str% )")) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
                while (tok->next() && Token::Match(tok->next(), "_T ( %char%|%str% )")) {
                    tok->next()->deleteNext();
                    tok->next()->deleteThis();
                    tok->next()->deleteNext();
                    tok->concatStr(tok->next()->str());
                    tok->deleteNext();
                }
            }
        }
    } else if (_settings->platformType == Settings::Win32W ||
               _settings->platformType == Settings::Win64) {
        for (Token *tok = list.front(); tok; tok = tok->next()) {
            if (Token::simpleMatch(tok, "_tcscat (")) {
                tok->str("wcscat");
            } else if (Token::simpleMatch(tok, "_tcschr (")) {
                tok->str("wcschr");
            } else if (Token::simpleMatch(tok, "_tcscmp (")) {
                tok->str("wcscmp");
            } else if (Token::simpleMatch(tok, "_tcscpy (")) {
                tok->str("wcscpy");
            } else if (Token::simpleMatch(tok, "_tcsdup (")) {
                tok->str("wcsdup");
            } else if (Token::simpleMatch(tok, "_tcslen (")) {
                tok->str("wcslen");
            } else if (Token::simpleMatch(tok, "_tcsncat (")) {
                tok->str("wcsncat");
            } else if (Token::simpleMatch(tok, "_tcsncpy (")) {
                tok->str("wcsncpy");
            } else if (Token::simpleMatch(tok, "_tcsnlen (")) {
                tok->str("wcsnlen");
            } else if (Token::simpleMatch(tok, "_tcsrchr (")) {
                tok->str("wcsrchr");
            } else if (Token::simpleMatch(tok, "_tcsstr (")) {
                tok->str("wcsstr");
            } else if (Token::simpleMatch(tok, "_tcstok (")) {
                tok->str("wcstok");
            } else if (Token::simpleMatch(tok, "_ftprintf (")) {
                tok->str("fwprintf");
                tok->originalName("_ftprintf");
            } else if (Token::simpleMatch(tok, "_tprintf (")) {
                tok->str("wprintf");
                tok->originalName("_tprintf");
            } else if (Token::simpleMatch(tok, "_stprintf (")) {
                tok->str("swprintf");
                tok->originalName("_stprintf");
            } else if (Token::simpleMatch(tok, "_sntprintf (")) {
                tok->str("_snwprintf");
                tok->originalName("_sntprintf");
            } else if (Token::simpleMatch(tok, "_ftscanf (")) {
                tok->str("fwscanf");
                tok->originalName("_ftscanf");
            } else if (Token::simpleMatch(tok, "_tscanf (")) {
                tok->str("wscanf");
                tok->originalName("_tscanf");
            } else if (Token::simpleMatch(tok, "_stscanf (")) {
                tok->str("swscanf");
                tok->originalName("_stscanf");
            } else if (Token::simpleMatch(tok, "_ftprintf_s (")) {
                tok->str("fwprintf_s");
                tok->originalName("_ftprintf_s");
            } else if (Token::simpleMatch(tok, "_tprintf_s (")) {
                tok->str("wprintf_s");
                tok->originalName("_tprintf_s");
            } else if (Token::simpleMatch(tok, "_stprintf_s (")) {
                tok->str("swprintf_s");
                tok->originalName("_stprintf_s");
            } else if (Token::simpleMatch(tok, "_sntprintf_s (")) {
                tok->str("_snwprintf_s");
                tok->originalName("_sntprintf_s");
            } else if (Token::simpleMatch(tok, "_ftscanf_s (")) {
                tok->str("fwscanf_s");
                tok->originalName("_ftscanf_s");
            } else if (Token::simpleMatch(tok, "_tscanf_s (")) {
                tok->str("wscanf_s");
                tok->originalName("_tscanf_s");
            } else if (Token::simpleMatch(tok, "_stscanf_s (")) {
                tok->str("swscanf_s");
                tok->originalName("_stscanf_s");
            } else if (Token::Match(tok, "_T ( %char%|%str% )")) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
                tok->isLong(true);
                while (tok->next() && Token::Match(tok->next(), "_T ( %char%|%str% )")) {
                    tok->next()->deleteNext();
                    tok->next()->deleteThis();
                    tok->next()->deleteNext();
                    tok->concatStr(tok->next()->str());
                    tok->deleteNext();
                }
            }
        }
    }
}

// Remove Borland code
void Tokenizer::simplifyBorland()
{
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "( __closure * %var% )")) {
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
        }

        else if (Token::Match(tok, "class %var% :|{")) {
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
    for (Token *tok = list.front(); tok; tok = tok->next()) {
        // check for emit which can be outside of class
        if (Token::Match(tok, "emit|Q_EMIT %var% (") &&
            Token::simpleMatch(tok->linkAt(2), ") ;")) {
            tok->deleteThis();
        } else if (!Token::Match(tok, "class %var% :"))
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
                --indentlevel;
                if (indentlevel == 0)
                    break;
            }

            if (tok2->strAt(1) == "Q_OBJECT") {
                tok2->deleteNext();
            } else if (Token::Match(tok2->next(), "public|protected|private slots|Q_SLOTS :")) {
                tok2 = tok2->next();
                tok2->str(tok2->str() + ":");
                tok2->deleteNext(2);
                tok2 = tok2->previous();
            } else if (Token::Match(tok2->next(), "signals|Q_SIGNALS :")) {
                tok2 = tok2->next();
                tok2->str("protected:");
                tok2->deleteNext();
            } else if (Token::Match(tok2->next(), "emit|Q_EMIT %var% (") &&
                       Token::simpleMatch(tok2->linkAt(3), ") ;")) {
                tok2->deleteNext();
            }
        }
    }
}

void Tokenizer::createSymbolDatabase()
{
    if (!_symbolDatabase) {
        _symbolDatabase = new SymbolDatabase(this, _settings, _errorLogger);

        // Set scope pointers
        for (std::list<Scope>::iterator scope = _symbolDatabase->scopeList.begin(); scope != _symbolDatabase->scopeList.end(); ++scope) {
            Token* start = const_cast<Token*>(scope->classStart);
            Token* end = const_cast<Token*>(scope->classEnd);
            if (scope->type == Scope::eGlobal) {
                start = const_cast<Token*>(list.front());
                end = const_cast<Token*>(list.back());
            }
            if (start && end) {
                start->scope(&*scope);
                end->scope(&*scope);
            }
            if (start != end && start->next() != end) {
                for (Token* tok = start->next(); tok != end; tok = tok->next()) {
                    if (tok->str() == "{") {
                        bool break2 = false;
                        for (std::list<Scope*>::const_iterator innerScope = scope->nestedList.begin(); innerScope != scope->nestedList.end(); ++innerScope) {
                            if (tok == (*innerScope)->classStart) { // Is begin of inner scope
                                tok = tok->link();
                                if (!tok || tok->next() == end || !tok->next()) {
                                    break2 = true;
                                    break;
                                }
                                tok = tok->next();
                                break;
                            }
                        }
                        if (break2)
                            break;
                    }
                    tok->scope(&*scope);
                }
            }
        }

        // Set function pointers
        for (Token* tok = list.front(); tok != list.back(); tok = tok->next()) {
            if (Token::Match(tok, "%var% (")) {
                tok->function(_symbolDatabase->findFunction(tok));
            }
        }

        // Set variable pointers
        for (Token* tok = list.front(); tok != list.back(); tok = tok->next()) {
            if (tok->varId())
                tok->variable(_symbolDatabase->getVariableFromVarId(tok->varId()));

            // Set Token::variable pointer for array member variable
            // Since it doesn't point at a fixed location it doesn't have varid
            if (tok->variable() != NULL &&
                tok->variable()->typeScope() &&
                Token::Match(tok, "%var% [|.")) {

                Token *tok2 = tok->next();
                // Locate "]"
                if (tok->next()->str() == "[") {
                    while (tok2 && tok2->str() == "[")
                        tok2 = tok2->link()->next();
                }

                Token *membertok = NULL;
                if (Token::Match(tok2, ". %var%"))
                    membertok = tok2->next();
                else if (Token::Match(tok2, ") . %var%") && tok->strAt(-1) == "(")
                    membertok = tok2->tokAt(2);

                if (membertok) {
                    const Variable *var = tok->variable();
                    if (var && var->typeScope()) {
                        const Variable *membervar = var->typeScope()->getVariable(membertok->str());
                        if (membervar)
                            membertok->variable(membervar);
                    }
                }
            }

            // check for function returning record type
            // func(...).var
            // func(...)[...].var
            else if (tok->function() && tok->next()->str() == "(" &&
                     (Token::Match(tok->next()->link(), ") . %var% !!(") ||
                      (Token::Match(tok->next()->link(), ") [") && Token::Match(tok->next()->link()->next()->link(), "] . %var% !!(")))) {
                const Type *type = tok->function()->retType;
                if (type) {
                    Token *membertok;
                    if (tok->next()->link()->next()->str() == ".")
                        membertok = tok->next()->link()->next()->next();
                    else
                        membertok = tok->next()->link()->next()->link()->next()->next();
                    const Variable *membervar = membertok->variable();
                    if (!membervar) {
                        if (type->classScope) {
                            membervar = type->classScope->getVariable(membertok->str());
                            if (membervar)
                                membertok->variable(membervar);
                        }
                    }
                }
            }
        }
    }
}

void Tokenizer::deleteSymbolDatabase()
{
    // Clear scope, function, and variable pointers
    for (Token* tok = list.front(); tok != list.back(); tok = tok->next()) {
        tok->scope(0);
        tok->function(0);
        tok->variable(0);
    }

    delete _symbolDatabase;
    _symbolDatabase = 0;
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
                    if (par && par->str() == "::" && par->next() && par->next()->isName()) {
                        op += par->str();
                        par = par->next();
                    }
                    done = false;
                }
                if (Token::Match(par, ".|%op%")) {
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
                    if (Token::Match(par, "( ) const| [=;{),]"))
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

            if (par && Token::Match(par->link(), ") const| [=;{),]")) {
                tok->str("operator" + op);
                Token::eraseTokens(tok, par);
            }
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
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum"))) {
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

                if (tok1 && Token::Match(tok1->link(), ") const| {|;|:")) {
                    std::string qualification = tok->str() + "::";

                    // check for extra qualification
                    /** @todo this should be made more generic to handle more levels */
                    if (Token::Match(tok->tokAt(-2), "%type% ::")) {
                        if (classInfo.size() >= 2) {
                            if (classInfo.at(classInfo.size() - 2).className != tok->strAt(-2))
                                continue;
                            else
                                qualification = tok->strAt(-2) + "::" + qualification;
                        } else
                            continue;
                    }

                    if (_settings->isEnabled("portability"))
                        unnecessaryQualificationError(tok, qualification);

                    tok->deleteNext();
                    tok->deleteThis();
                }
            }
        }
    }
}

void Tokenizer::unnecessaryQualificationError(const Token *tok, const std::string &qualification) const
{
    reportError(tok, Severity::portability, "unnecessaryQualification",
                "The extra qualification \'" + qualification + "\' is unnecessary and is considered an error by many compilers.");
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

void Tokenizer::printUnknownTypes()
{
    std::multimap<std::string, const Token *> unknowns;

    for (unsigned int i = 1; i <= _varId; ++i) {
        const Variable *var = _symbolDatabase->getVariableFromVarId(i);

        // is unknown type?
        if (var && !var->type() && !var->typeStartToken()->isStandardType()) {
            std::string name;
            const Token * nameTok;

            // single token type?
            if (var->typeStartToken() == var->typeEndToken()) {
                name = var->typeStartToken()->str();
                nameTok = var->typeStartToken();
            }

            // complicated type
            else {
                const Token *tok = var->typeStartToken();
                int level = 0;

                nameTok =  tok;

                while (tok) {
                    // skip pointer and reference part of type
                    if (level == 0 && (tok->str() ==  "*" || tok->str() == "&"))
                        break;

                    name += tok->str();

                    if (Token::Match(tok, "struct|union"))
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
                const std::string leftExponent = tok2->tokAt(2)->str();
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                if (!Token::Match(tok3->link(), ") , %num% )"))
                    continue;
                Token * const tok4 = tok3->link();
                const std::string rightExponent = tok4->tokAt(2)->str();
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok3->link()->next())) {
                    Token::eraseTokens(tok, tok3->link()->tokAt(4));
                    tok->str("1");
                }
            } else if (Token::Match(tok->tokAt(2), "cos|cosf|cosl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) + pow|powf|powl ( sin|sinf|sinl ("))
                    continue;
                const std::string leftExponent = tok2->tokAt(2)->str();
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                if (!Token::Match(tok3->link(), ") , %num% )"))
                    continue;
                Token * const tok4 = tok3->link();
                const std::string rightExponent = tok4->tokAt(2)->str();
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok3->link()->next())) {
                    Token::eraseTokens(tok, tok3->link()->tokAt(4));
                    tok->str("1");
                }
            } else if (Token::Match(tok->tokAt(2), "sinh|sinhf|sinhl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) - pow|powf|powl ( cosh|coshf|coshl ("))
                    continue;
                const std::string leftExponent = tok2->tokAt(2)->str();
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                if (!Token::Match(tok3->link(), ") , %num% )"))
                    continue;
                Token * const tok4 = tok3->link();
                const std::string rightExponent = tok4->tokAt(2)->str();
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok3->link()->next())) {
                    Token::eraseTokens(tok, tok3->link()->tokAt(4));
                    tok->str("-1");
                }
            } else if (Token::Match(tok->tokAt(2), "cosh|coshf|coshl (")) {
                Token * const tok2 = tok->linkAt(3);
                if (!Token::Match(tok2, ") , %num% ) - pow|powf|powl ( sinh|sinhf|sinhl ("))
                    continue;
                const std::string leftExponent = tok2->tokAt(2)->str();
                if (!isTwoNumber(leftExponent))
                    continue; // left exponent is not 2
                Token * const tok3 = tok2->tokAt(8);
                if (!Token::Match(tok3->link(), ") , %num% )"))
                    continue;
                Token * const tok4 = tok3->link();
                const std::string rightExponent = tok4->tokAt(2)->str();
                if (!isTwoNumber(rightExponent))
                    continue; // right exponent is not 2
                if (tok->tokAt(3)->stringifyList(tok2->next()) == tok3->stringifyList(tok3->link()->next())) {
                    Token::eraseTokens(tok, tok3->link()->tokAt(4));
                    tok->str("-1");
                }
            }
        }
    }
}

const std::string& Tokenizer::getSourceFilePath() const
{
    if (list.getFiles().empty()) {
        static const std::string empty;
        return empty;
    }
    return list.getFiles()[0];
}

bool Tokenizer::isC() const
{
    return _settings->enforcedLang == Settings::C || (_settings->enforcedLang == Settings::None && Path::isC(getSourceFilePath()));
}

bool Tokenizer::isCPP() const
{
    return _settings->enforcedLang == Settings::CPP || (_settings->enforcedLang == Settings::None && Path::isCPP(getSourceFilePath()));
}

void Tokenizer::reportError(const Token* tok, const Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    const std::list<const Token*> callstack(1, tok);
    reportError(callstack, severity, id, msg, inconclusive);
}

void Tokenizer::reportError(const std::list<const Token*>& callstack, Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive) const
{
    ErrorLogger::ErrorMessage errmsg(callstack, &list, severity, id, msg, inconclusive);
    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}
