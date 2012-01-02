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
#ifdef _MSC_VER
#pragma warning(disable: 4503)
#endif

#include "tokenize.h"
#include "token.h"
#include "mathlib.h"
#include "settings.h"
#include "errorlogger.h"
#include "check.h"
#include "path.h"
#include "symboldatabase.h"
#include "templatesimplifier.h"

#include <string>
#include <cstring>
#include <sstream>
#include <list>
#include <cassert>
#include <cctype>
#include <stack>
#include <cstdlib>

//---------------------------------------------------------------------------

Tokenizer::Tokenizer() :
    _tokens(0), //no tokens to start with
    _tokensBack(0),
    _settings(0),
    _errorLogger(0),
    _symbolDatabase(0),
    _varId(0),
    _codeWithTemplates(false) //is there any templates?
{
}

Tokenizer::Tokenizer(const Settings *settings, ErrorLogger *errorLogger) :
    _tokens(0), //no tokens to start with
    _tokensBack(0),
    _settings(settings),
    _errorLogger(errorLogger),
    _symbolDatabase(0),
    _varId(0),
    _codeWithTemplates(false) //is there any templates?
{
    // make sure settings are specified
    assert(_settings);
}

Tokenizer::~Tokenizer()
{
    deallocateTokens();
    delete _symbolDatabase;
}

//---------------------------------------------------------------------------

// Helper functions..


//---------------------------------------------------------------------------

const Token *Tokenizer::tokens() const
{
    return _tokens;
}


const std::vector<std::string> *Tokenizer::getFiles() const
{
    return &_files;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

void Tokenizer::addtoken(const char str[], const unsigned int lineno, const unsigned int fileno, bool split)
{
    if (str[0] == 0)
        return;

    // If token contains # characters, split it up
    if (split && strstr(str, "##")) {
        std::string temp;
        for (unsigned int i = 0; str[i]; ++i) {
            if (strncmp(&str[i], "##", 2) == 0) {
                addtoken(temp.c_str(), lineno, fileno, false);
                temp.clear();
                addtoken("##", lineno, fileno, false);
                ++i;
            } else
                temp += str[i];
        }
        addtoken(temp.c_str(), lineno, fileno, false);
        return;
    }

    // Replace hexadecimal value with decimal
    std::ostringstream str2;
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        str2 << std::strtoul(str + 2, NULL, 16);
    } else if (strncmp(str, "_Bool", 5) == 0) {
        str2 << "bool";
    } else {
        str2 << str;
    }

    if (_tokensBack) {
        _tokensBack->insertToken(str2.str());
    } else {
        _tokens = new Token(&_tokensBack);
        _tokensBack = _tokens;
        _tokensBack->str(str2.str());
    }

    _tokensBack->linenr(lineno);
    _tokensBack->fileIndex(fileno);
}

void Tokenizer::addtoken(const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    if (tok == 0)
        return;

    if (_tokensBack) {
        _tokensBack->insertToken(tok->str());
    } else {
        _tokens = new Token(&_tokensBack);
        _tokensBack = _tokens;
        _tokensBack->str(tok->str());
    }

    _tokensBack->linenr(lineno);
    _tokensBack->fileIndex(fileno);
    _tokensBack->isUnsigned(tok->isUnsigned());
    _tokensBack->isSigned(tok->isSigned());
    _tokensBack->isLong(tok->isLong());
    _tokensBack->isUnused(tok->isUnused());
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------



unsigned int Tokenizer::sizeOfType(const Token *type) const
{
    if (!type || type->str().empty())
        return 0;

    if (type->str()[0] == '"')
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

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void Tokenizer::insertTokens(Token *dest, const Token *src, unsigned int n)
{
    std::stack<Token *> link;

    while (n > 0) {
        dest->insertToken(src->str());
        dest = dest->next();

        // Set links
        if (Token::Match(dest, "(|[|{"))
            link.push(dest);
        else if (!link.empty() && Token::Match(dest, ")|]|}")) {
            Token::createMutualLinks(dest, link.top());
            link.pop();
        }

        dest->fileIndex(src->fileIndex());
        dest->linenr(src->linenr());
        dest->varId(src->varId());
        dest->isName(src->isName());
        dest->isNumber(src->isNumber());
        dest->isBoolean(src->isBoolean());
        dest->isUnsigned(src->isUnsigned());
        dest->isSigned(src->isSigned());
        dest->isPointerCompare(src->isPointerCompare());
        dest->isLong(src->isLong());
        dest->isUnused(src->isUnused());
        src  = src->next();
        --n;
    }
}

//---------------------------------------------------------------------------

Token *Tokenizer::copyTokens(Token *dest, const Token *first, const Token *last)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    for (const Token *tok = first; tok != last->next(); tok = tok->next()) {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(dest->fileIndex());
        tok2->linenr(dest->linenr());
        tok2->isName(tok->isName());
        tok2->isNumber(tok->isNumber());
        tok2->isBoolean(tok->isBoolean());
        tok2->isUnsigned(tok->isUnsigned());
        tok2->isSigned(tok->isSigned());
        tok2->isPointerCompare(tok->isPointerCompare());
        tok2->isLong(tok->isLong());
        tok2->isUnused(tok->isUnused());
        tok2->setExpandedMacro(tok->isExpandedMacro());
        tok2->varId(tok->varId());

        // Check for links and fix them up
        if (tok2->str() == "(" || tok2->str() == "[" || tok2->str() == "{")
            links.push(tok2);
        else if (tok2->str() == ")" || tok2->str() == "]" || tok2->str() == "}") {
            Token * link = links.top();

            tok2->link(link);
            link->link(tok2);

            links.pop();
        }
    }
    return tok2;
}

//---------------------------------------------------------------------------
// Tokenize - tokenizes a given file.
//---------------------------------------------------------------------------

void Tokenizer::createTokens(std::istream &code)
{
    // line number in parsed code
    unsigned int lineno = 1;

    // The current token being parsed
    std::string CurrentToken;

    // lineNumbers holds line numbers for files in fileIndexes
    // every time an include file is completely parsed, last item in the vector
    // is removed and lineno is set to point to that value.
    std::vector<unsigned int> lineNumbers;

    // fileIndexes holds index for _files vector about currently parsed files
    // every time an include file is completely parsed, last item in the vector
    // is removed and FileIndex is set to point to that value.
    std::vector<unsigned int> fileIndexes;

    // FileIndex. What file in the _files vector is read now?
    unsigned int FileIndex = 0;

    bool expandedMacro = false;

    // Read one byte at a time from code and create tokens
    for (char ch = (char)code.get(); code.good(); ch = (char)code.get()) {
        if (ch == '$') {
            while (code.peek() == '$')
                code.get();
            ch = ' ';
            expandedMacro = true;
        } else if (ch == '\n') {
            expandedMacro = false;
        }

        // char/string..
        // multiline strings are not handled. The preprocessor should handle that for us.
        else if (ch == '\'' || ch == '\"') {
            std::string line;

            // read char
            bool special = false;
            char c = ch;
            do {
                // Append token..
                line += c;

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)code.get();
            } while (code.good() && (special || c != ch));
            line += ch;

            // Handle #file "file.h"
            if (CurrentToken == "#file") {
                // Extract the filename
                line = line.substr(1, line.length() - 2);

                // Has this file been tokenized already?
                ++lineno;
                bool foundOurfile = false;
                fileIndexes.push_back(FileIndex);
                for (unsigned int i = 0; i < _files.size(); ++i) {
                    if (Path::sameFileName(_files[i], line)) {
                        // Use this index
                        foundOurfile = true;
                        FileIndex = i;
                    }
                }

                if (!foundOurfile) {
                    // The "_files" vector remembers what files have been tokenized..
                    _files.push_back(Path::simplifyPath(line.c_str()));
                    FileIndex = static_cast<unsigned int>(_files.size() - 1);
                }

                lineNumbers.push_back(lineno);
                lineno = 0;
            } else {
                // Add previous token
                addtoken(CurrentToken.c_str(), lineno, FileIndex);
                if (!CurrentToken.empty())
                    _tokensBack->setExpandedMacro(expandedMacro);

                // Add content of the string
                addtoken(line.c_str(), lineno, FileIndex);
                if (!line.empty())
                    _tokensBack->setExpandedMacro(expandedMacro);
            }

            CurrentToken.clear();

            continue;
        }

        if (ch == '.' &&
            CurrentToken.length() > 0 &&
            std::isdigit(CurrentToken[0])) {
            // Don't separate doubles "5.4"
        } else if (strchr("+-", ch) &&
                   CurrentToken.length() > 0 &&
                   std::isdigit(CurrentToken[0]) &&
                   CurrentToken.compare(0,2,"0x") != 0 &&
                   (CurrentToken[CurrentToken.length()-1] == 'e' ||
                    CurrentToken[CurrentToken.length()-1] == 'E')) {
            // Don't separate doubles "4.2e+10"
        } else if (CurrentToken.empty() && ch == '.' && std::isdigit(code.peek())) {
            // tokenize .125 into 0.125
            CurrentToken = "0";
        } else if (ch=='&' && code.peek() == '&') {
            if (!CurrentToken.empty()) {
                addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
                if (!CurrentToken.empty())
                    _tokensBack->setExpandedMacro(expandedMacro);
                CurrentToken.clear();
            }

            // &&
            ch = (char)code.get();
            addtoken("&&", lineno, FileIndex, true);
            _tokensBack->setExpandedMacro(expandedMacro);
            continue;
        } else if (ch==':' && CurrentToken.empty() && code.peek() == ' ') {
            // :
            addtoken(":", lineno, FileIndex, true);
            _tokensBack->setExpandedMacro(expandedMacro);
            CurrentToken.clear();
            continue;
        } else if (ch==':' && CurrentToken.empty() && code.peek() == ':') {
            // ::
            ch = (char)code.get();
            addtoken("::", lineno, FileIndex, true);
            _tokensBack->setExpandedMacro(expandedMacro);
            CurrentToken.clear();
            continue;
        } else if (strchr("+-*/%&|^?!=<>[](){};:,.~\n ", ch)) {
            if (CurrentToken == "#file") {
                // Handle this where strings are handled
                continue;
            } else if (CurrentToken == "#endfile") {
                if (lineNumbers.empty() || fileIndexes.empty()) {
                    cppcheckError(0);
                    deallocateTokens();
                    return;
                }

                lineno = lineNumbers.back();
                lineNumbers.pop_back();
                FileIndex = fileIndexes.back();
                fileIndexes.pop_back();
                CurrentToken.clear();
                continue;
            }

            addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
            if (!CurrentToken.empty())
                _tokensBack->setExpandedMacro(expandedMacro);

            CurrentToken.clear();

            if (ch == '\n') {
                ++lineno;
                continue;
            } else if (ch == ' ') {
                continue;
            }

            CurrentToken += ch;
            // Add "++", "--" or ">>" token
            if ((ch == '+' || ch == '-' || ch == '>') && (code.peek() == ch))
                CurrentToken += (char)code.get();
            addtoken(CurrentToken.c_str(), lineno, FileIndex);
            _tokensBack->setExpandedMacro(expandedMacro);
            CurrentToken.clear();
            continue;
        }

        CurrentToken += ch;
    }
    addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
    if (!CurrentToken.empty())
        _tokensBack->setExpandedMacro(expandedMacro);
    _tokens->assignProgressValues();
}

void Tokenizer::duplicateTypedefError(const Token *tok1, const Token *tok2, const std::string &type)
{
    if (tok1 && !(_settings->isEnabled("style") && _settings->inconclusive))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    std::string tok2_str;
    if (tok1 && tok2) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok1->linenr();
        loc.setfile(file(tok1));
        locationList.push_back(loc);
        loc.line = tok2->linenr();
        loc.setfile(file(tok2));
        locationList.push_back(loc);
        tok2_str = tok2->str();
    } else
        tok2_str = "name";

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2_str +
                                                   "' hides typedef with same name"),
                                           "variableHidingTypedef",
                                           true);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::duplicateDeclarationError(const Token *tok1, const Token *tok2, const std::string &type)
{
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    std::string tok2_str;
    if (tok1 && tok2) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok1->linenr();
        loc.setfile(file(tok1));
        locationList.push_back(loc);
        loc.line = tok2->linenr();
        loc.setfile(file(tok2));
        locationList.push_back(loc);
        tok2_str = tok2->str();
    } else
        tok2_str = "name";

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2_str +
                                                   "' forward declaration unnecessary, already declared"),
                                           "unnecessaryForwardDeclaration",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef, bool undefinedStruct)
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
            int level = 0;
            while (end && end->next() && (!Token::Match(end->next(), ";|)|>") ||
                                          (end->next()->str() == ")" && level == 0))) {
                if (end->next()->str() == "(")
                    ++level;
                else if (end->next()->str() == ")")
                    --level;

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

                duplicateTypedefError(*tokPtr, name, "Template instantiation");
                *tokPtr = end->link();
                return true;
            } else if (Token::Match(tok->previous(), "%type%")) {
                if (end->link()->next()->str() == "{") {
                    duplicateTypedefError(*tokPtr, name, "Function");
                    *tokPtr = end->link()->next()->link();
                    return true;
                }
            }
        }

        if (end) {
            if (Token::simpleMatch(end, ") {")) { // function parameter ?
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const")) {
                    duplicateTypedefError(*tokPtr, name, "Function parameter");
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
                        duplicateTypedefError(*tokPtr, name, "Template parameter");
                        *tokPtr = end->link();
                        return true;
                    }
                }
            } else {
                // look backwards
                if (Token::Match(tok->previous(), "typedef|}|>") ||
                    (tok->previous()->str() == "*" && tok->next()->str() != "(") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     (!Token::Match(tok->previous(), "return|new|const|friend|public|private|protected|throw|extern") &&
                      !Token::simpleMatch(tok->tokAt(-2), "friend class")))) {
                    // scan backwards for the end of the previous statement
                    int level = (tok->previous()->str() == "}") ? 1 : 0;
                    while (tok && tok->previous() && (!Token::Match(tok->previous(), ";|{") || (level != 0))) {
                        if (tok->previous()->str() == "}") {
                            tok = tok->previous()->link();
                        } else if (tok->previous()->str() == "typedef") {
                            duplicateTypedefError(*tokPtr, name, "Typedef");
                            return true;
                        } else if (tok->previous()->str() == "enum") {
                            duplicateTypedefError(*tokPtr, name, "Enum");
                            return true;
                        } else if (tok->previous()->str() == "struct") {
                            if (tok->strAt(-2) == "typedef" &&
                                tok->next()->str() == "{" &&
                                typeDef->strAt(3) != "{") {
                                // declaration after forward declaration
                                return true;
                            } else if (tok->next()->str() == "{") {
                                if (!undefinedStruct)
                                    duplicateTypedefError(*tokPtr, name, "Struct");
                                return true;
                            } else if (Token::Match(tok->next(), ")|*")) {
                                return true;
                            } else if (tok->next()->str() == name->str()) {
                                return true;
                            } else if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "Struct");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Struct");
                                return false;
                            }
                        } else if (tok->previous()->str() == "union") {
                            if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "Union");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Union");
                                return false;
                            }
                        } else if (tok->previous()->str() == "class") {
                            if (tok->next()->str() != ";") {
                                duplicateTypedefError(*tokPtr, name, "Class");
                                return true;
                            } else {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Class");
                                return false;
                            }
                        } else if (tok->previous()->str() == "{")
                            --level;

                        tok = tok->previous();
                    }

                    duplicateTypedefError(*tokPtr, name, "Variable");
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
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = tok1->linenr();
    loc.setfile(file(tok1));
    locationList.push_back(loc);

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::debug,
                                           "Failed to parse \'" + str.str() + "\'. The checking continues anyway.",
                                           "debug",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

Token * Tokenizer::deleteInvalidTypedef(Token *typeDef)
{
    Token *tok = NULL;
    unsigned int level = 0;

    // remove typedef but leave ;
    while (typeDef->next()) {
        if (level == 0 && typeDef->next()->str() == ";") {
            typeDef->deleteNext();
            break;
        } else if (typeDef->next()->str() == "{")
            ++level;
        else if (typeDef->next()->str() == "}") {
            if (!level)
                break;
            --level;
        }
        typeDef->deleteNext();
    }

    if (typeDef != _tokens) {
        tok = typeDef->previous();
        tok->deleteNext();
    } else {
        _tokens->deleteThis();
        tok = _tokens;
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
                name = "Unnamed" + MathLib::toString<unsigned int>(count++);
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

    if (tok1->next() && tok1->next()->str() == ";" && tok1 && tok1->previous()->str() == "}") {
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

                if (tok2->str() == "(" &&
                    tok2->link()->next()->str() == "(") {
                    tok2 = tok2->link();

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }

                // skip over typedef parameter
                if (tok2->next()->str() == "(") {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (_errorLogger && !_files.empty())
            _errorLogger->reportProgress(_files[0], "Tokenize (typedef)", tok->progressValue());

        if (goback) {
            //jump back once, see the comment at the end of the function
            goback = false;
            tok = tok->previous();
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

        // check for syntax errors
        if (tok->previous() && tok->previous()->str() == "(") {
            syntaxError(tok);
            continue;
        }

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
                const Token *tok2 = Token::findmatch(_tokens, pattern.c_str(), tok);
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
        unsigned short offset = 1;
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

        if (Token::simpleMatch(tok->next(), "::") ||
            Token::Match(tok->next(), "%type%")) {
            typeStart = tok->next();
            offset = 1;

            while (Token::Match(tok->tokAt(offset), "const|signed|unsigned|struct|enum %type%") ||
                   (tok->tokAt(offset + 1) && tok->tokAt(offset + 1)->isStandardType()))
                ++offset;

            typeEnd = tok->tokAt(offset++);

            bool atEnd = false;
            while (!atEnd) {
                if (Token::simpleMatch(tok->tokAt(offset), "::"))
                    typeEnd = tok->tokAt(offset++);

                if (Token::Match(tok->tokAt(offset), "%type%") &&
                    tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), "[|;|,|("))
                    typeEnd = tok->tokAt(offset++);
                else if (Token::simpleMatch(tok->tokAt(offset), "const (")) {
                    typeEnd = tok->tokAt(offset++);
                    atEnd = true;
                } else
                    atEnd = true;
            }
        } else
            continue; // invalid input

        // check for invalid input
        if (!tok->tokAt(offset)) {
            syntaxError(tok);
            return;
        }

        // check for template
        if (tok->strAt(offset) == "<") {
            unsigned int level = 0;
            unsigned int paren = 0;
            typeEnd = tok->tokAt(offset + 1);
            for (; typeEnd ; typeEnd = typeEnd->next()) {
                if (typeEnd->str() == ">") {
                    if (!paren) {
                        if (!level)
                            break;
                        --level;
                    }
                } else if (typeEnd->str() == "<") {
                    if (!paren)
                        ++level;
                } else if (typeEnd->str() == "(")
                    ++paren;
                else if (typeEnd->str() == ")") {
                    if (!paren)
                        break;
                    --paren;
                }
            }

            while (typeEnd && Token::Match(typeEnd->next(), ":: %type%"))
                typeEnd = typeEnd->tokAt(2);

            if (!typeEnd) {
                // internal error
                return;
            }

            while (Token::Match(typeEnd->next(), "const|volatile"))
                typeEnd = typeEnd->next();

            tok = typeEnd;
            offset = 1;
        }

        // check for pointers and references
        while (Token::Match(tok->tokAt(offset), "*|&|const"))
            pointers.push_back(tok->strAt(offset++));

        // check for invalid input
        if (!tok->tokAt(offset)) {
            syntaxError(tok);
            return;
        }

        if (Token::Match(tok->tokAt(offset), "%type%")) {
            // found the type name
            typeName = tok->tokAt(offset++);

            // check for array
            if (tok->tokAt(offset) && tok->strAt(offset) == "[") {
                arrayStart = tok->tokAt(offset);

                bool atEnd = false;
                while (!atEnd) {
                    while (tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), ";|,"))
                        ++offset;

                    if (!tok->tokAt(offset + 1))
                        return; // invalid input
                    else if (tok->strAt(offset + 1) == ";")
                        atEnd = true;
                    else if (tok->strAt(offset) == "]")
                        atEnd = true;
                    else
                        ++offset;
                }

                arrayEnd = tok->tokAt(offset++);
            }

            // check for end or another
            if (Token::Match(tok->tokAt(offset), ";|,"))
                tok = tok->tokAt(offset);

            // or a function typedef
            else if (Token::simpleMatch(tok->tokAt(offset), "(")) {
                // unhandled typedef, skip it and continue
                if (typeName->str() == "void") {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    if (tok == _tokens)
                        //now the next token to process is 'tok', not 'tok->next()';
                        goback = true;
                    continue;
                }

                // unhandled function pointer, skip it and continue
                // TODO: handle such typedefs. See ticket #3314
                else if (Token::Match(tok->tokAt(offset), "( %type% ::") &&
                         Token::Match(tok->linkAt(offset)->tokAt(-3), ":: * %var% ) (")) {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    if (tok == _tokens)
                        //now the next token to process is 'tok', not 'tok->next()';
                        goback = true;
                    continue;
                }

                // function pointer
                else if (Token::Match(tok->tokAt(offset), "( * %var% ) (")) {
                    // name token wasn't a name, it was part of the type
                    typeEnd = typeEnd->next();
                    functionPtr = true;
                    funcStart = tok->tokAt(offset + 1);
                    funcEnd = tok->tokAt(offset + 1);
                    typeName = tok->tokAt(offset + 2);
                    argStart = tok->tokAt(offset + 4);
                    argEnd = tok->linkAt(offset + 4);
                    tok = argEnd->next();
                }

                // function
                else if (Token::Match(tok->linkAt(offset), ") const| ;|,")) {
                    function = true;
                    if (tok->linkAt(offset)->next()->str() == "const") {
                        specStart = tok->linkAt(offset)->next();
                        specEnd = specStart;
                    }
                    argStart = tok->tokAt(offset);
                    argEnd = tok->linkAt(offset);
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
                if (tok == _tokens)
                    //now the next token to process is 'tok', not 'tok->next()';
                    goback = true;
                continue;
            }
        }

        // typeof: typedef __typeof__ ( ... ) type;
        else if (Token::simpleMatch(tok->tokAt(offset - 1), "__typeof__ (") &&
                 Token::Match(tok->linkAt(offset), ") %type% ;")) {
            argStart = tok->tokAt(offset);
            argEnd = tok->linkAt(offset);
            typeName = tok->linkAt(offset)->next();
            tok = typeName->next();
            typeOf = true;
        }

        // function: typedef ... ( .... type )( ... );
        //           typedef ... (( .... type )( ... ));
        //           typedef ... ( * ( .... type )( ... ));
        else if ((tok->strAt(offset) == "(" &&
                  Token::Match(tok->linkAt(offset)->previous(), "%type% ) (") &&
                  Token::Match(tok->linkAt(offset)->next()->link(), ") const|volatile|;")) ||
                 (Token::simpleMatch(tok->tokAt(offset), "( (") &&
                  Token::Match(tok->linkAt(offset + 1)->previous(), "%type% ) (") &&
                  Token::Match(tok->linkAt(offset + 1)->next()->link(), ") const|volatile| ) ;|,")) ||
                 (Token::simpleMatch(tok->tokAt(offset), "( * (") &&
                  Token::Match(tok->linkAt(offset + 2)->previous(), "%type% ) (") &&
                  Token::Match(tok->linkAt(offset + 2)->next()->link(), ") const|volatile| ) ;|,"))) {
            if (tok->strAt(offset + 1) == "(")
                ++offset;
            else if (Token::simpleMatch(tok->tokAt(offset), "( * (")) {
                ++offset;
                pointers.push_back("*");
                ++offset;
            }

            if (tok->linkAt(offset)->strAt(-2) == "*")
                functionPtr = true;
            else
                function = true;
            funcStart = tok->tokAt(offset + 1);
            funcEnd = tok->linkAt(offset)->tokAt(-2);
            typeName = tok->linkAt(offset)->previous();
            argStart = tok->linkAt(offset)->next();
            argEnd = tok->linkAt(offset)->next()->link();
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

        else if (Token::Match(tok->tokAt(offset), "( %type% (")) {
            function = true;
            if (tok->linkAt(offset)->next()) {
                typeName = tok->tokAt(offset + 1);
                argStart = tok->tokAt(offset + 2);
                argEnd = tok->linkAt(offset + 2);
                tok = tok->linkAt(offset)->next();
            } else {
                // internal error
                continue;
            }
        }

        // pointer to function returning pointer to function
        else if (Token::Match(tok->tokAt(offset), "( * ( * %type% ) (") &&
                 Token::simpleMatch(tok->linkAt(offset + 6), ") ) (") &&
                 Token::Match(tok->linkAt(offset + 6)->linkAt(2), ") ;|,")) {
            functionPtrRetFuncPtr = true;

            typeName = tok->tokAt(offset + 4);
            argStart = tok->tokAt(offset + 6);
            argEnd = tok->linkAt(offset + 6);

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->linkAt(2);

            tok = argFuncRetEnd->next();
        }

        // function returning pointer to function
        else if (Token::Match(tok->tokAt(offset), "( * %type% (") &&
                 Token::simpleMatch(tok->linkAt(offset + 3), ") ) (") &&
                 Token::Match(tok->linkAt(offset + 3)->linkAt(2), ") ;|,")) {
            functionRetFuncPtr = true;

            typeName = tok->tokAt(offset + 2);
            argStart = tok->tokAt(offset + 3);
            argEnd = tok->linkAt(offset + 3);

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->linkAt(2);

            tok = argFuncRetEnd->next();
        } else if (Token::Match(tok->tokAt(offset), "( * ( %type% ) (")) {
            functionRetFuncPtr = true;

            typeName = tok->tokAt(offset + 3);
            argStart = tok->tokAt(offset + 5);
            argEnd = tok->linkAt(offset + 5);

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->linkAt(2);

            tok = argFuncRetEnd->next();
        }

        // pointer/reference to array
        else if (Token::Match(tok->tokAt(offset), "( *|& %type% ) [")) {
            ptrToArray = (tok->strAt(offset + 1) == "*");
            refToArray = (tok->strAt(offset + 1) == "&");
            typeName = tok->tokAt(offset + 2);
            arrayStart = tok->tokAt(offset + 4);
            arrayEnd = arrayStart->link();
            tok = arrayEnd->next();
        }

        // pointer to class member
        else if (Token::Match(tok->tokAt(offset), "( %type% :: * %type% ) ;")) {
            namespaceStart = tok->tokAt(offset + 1);
            namespaceEnd = tok->tokAt(offset + 2);
            ptrMember = true;
            typeName = tok->tokAt(offset + 4);
            tok = tok->tokAt(offset + 6);
        }

        // unhandled typedef, skip it and continue
        else {
            unsupportedTypedef(typeDef);
            tok = deleteInvalidTypedef(typeDef);
            if (tok == _tokens)
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
                        if (Token::simpleMatch(tok2->previous(), "::")) {
                            tok2->tokAt(-2)->deleteNext();
                            globalScope = true;
                        }

                        for (std::size_t i = classLevel; i < spaceInfo.size(); ++i) {
                            tok2->deleteNext(2);
                        }
                        simplifyType = true;
                    } else if ((inScope && !exitThisScope) || inMemberFunc) {
                        if (Token::simpleMatch(tok2->previous(), "::")) {
                            // Don't replace this typename if it's preceded by "::" unless it's a namespace
                            if (!spaceInfo.empty() && (tok2->strAt(-2) == spaceInfo[0].className) && spaceInfo[0].isNamespace) {
                                tok2 = tok2->tokAt(-3);
                                tok2->deleteNext(2);
                                tok2 = tok2->next();
                                simplifyType = true;
                            }
                        } else if (Token::Match(tok2->previous(), "case %type% :")) {
                            tok2 = tok2->next();
                        } else if (duplicateTypedef(&tok2, typeName, typeDef, undefinedStruct)) {
                            exitScope = scope;

                            // skip to end of scope if not already there
                            if (tok2->str() != "}") {
                                int level = 0;
                                while (tok2->next() && (tok2->next()->str() != "}" || level)) {
                                    if (tok2->next()->str() == "{")
                                        ++level;
                                    else if (tok2->next()->str() == "}")
                                        --level;

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
                        break;
                    }

                    // There are 2 categories of typedef substitutions:
                    // 1. variable declarations that preserve the variable name like
                    //    global, local, and function parameters
                    // 2. not variable declarations that have no name like derived
                    //    classes, casts, operators, and template parameters

                    // try to determine which category this substitution is
                    bool isDerived = false;
                    bool inCast = false;
                    bool inTemplate = false;
                    bool inOperator = false;
                    bool inSizeof = false;

                    // check for derived class: class A : some_typedef {
                    isDerived = Token::Match(tok2->previous(), "public|protected|private %type% {|,");

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
                    if (Token::simpleMatch(tok2->previous(), "operator") ||
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
                        // don't add parenthesis around function names because it
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
                offset = 1;
                pointers.clear();

                while (Token::Match(tok->tokAt(offset), "*|&"))
                    pointers.push_back(tok->strAt(offset++));

                if (Token::Match(tok->tokAt(offset), "%type%")) {
                    typeName = tok->tokAt(offset++);

                    if (tok->tokAt(offset) && tok->strAt(offset) == "[") {
                        arrayStart = tok->tokAt(offset);

                        bool atEnd = false;
                        while (!atEnd) {
                            while (tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), ";|,"))
                                ++offset;

                            if (!tok->tokAt(offset + 1))
                                return; // invalid input
                            else if (tok->strAt(offset + 1) == ";")
                                atEnd = true;
                            else if (tok->strAt(offset) == "]")
                                atEnd = true;
                            else
                                ++offset;
                        }

                        arrayEnd = tok->tokAt(offset++);
                    }

                    if (Token::Match(tok->tokAt(offset), ";|,"))
                        tok = tok->tokAt(offset);
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

            if (typeDef != _tokens) {
                tok = typeDef->previous();
                tok->deleteNext();
                //no need to remove last token in the list
                if (tok->tokAt(2))
                    tok->deleteNext();
            } else {
                _tokens->deleteThis();
                //no need to remove last token in the list
                if (_tokens->next())
                    _tokens->deleteThis();
                tok = _tokens;
                //now the next token to process is 'tok', not 'tok->next()';
                goback = true;
            }
        }
    }
}

void Tokenizer::simplifyMulAndParens()
{
    for (Token *tok = _tokens->tokAt(3); tok; tok = tok->next()) {
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
                         const std::string &configuration,
                         const bool preprocessorCondition)
{
    // make sure settings specified
    assert(_settings);

    // Fill the map _typeSize..
    _typeSize.clear();
    _typeSize["char"] = 1;
    _typeSize["bool"] = _settings->sizeof_bool;
    _typeSize["short"] = _settings->sizeof_short;
    _typeSize["int"] = _settings->sizeof_int;
    _typeSize["long"] = _settings->sizeof_long;
    _typeSize["float"] = _settings->sizeof_float;
    _typeSize["double"] = _settings->sizeof_double;
    _typeSize["size_t"] = _settings->sizeof_size_t;
    _typeSize["*"] = _settings->sizeof_pointer;

    _configuration = configuration;

    // The "_files" vector remembers what files have been tokenized..
    _files.push_back(Path::simplifyPath(FileName));

    createTokens(code);

    // if MACRO
    for (const Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "if|for|while %var% (")) {
            syntaxError(tok);
            return false;
        }
    }

    // replace inline SQL with "asm()" (Oracle PRO*C). Ticket: #1959
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "EXEC SQL")) {
            // delete all tokens until ";"
            const Token *end = tok->tokAt(2);
            while (end && end->str() != ";")
                end = end->next();

            std::string instruction = tok->stringify(end);
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

    // Simplify JAVA/C# code
    if (isJavaOrCSharp())
        simplifyJavaAndCSharp();

    if (!createLinks()) {
        // Source has syntax errors, can't proceed
        return false;
    }

    //easy simplifications...
    for (Token *tok = _tokens; tok; tok = tok->next()) {

        // replace __LINE__ macro with line number
        if (tok->str() == "__LINE__")
            tok->str(MathLib::toString(tok->linenr()));

        // 'double sharp' token concatenation
        // TODO: pattern should be "%var%|%num% ## %var%|%num%"
        while (Token::Match(tok, "%any% ## %any%") &&
               (tok->isName() || tok->isNumber()) &&
               (tok->tokAt(2)->isName() || tok->tokAt(2)->isNumber())) {
            tok->str(tok->str() + tok->strAt(2));
            tok->deleteNext(2);
        }

        //Replace NULL with 0..
        if (tok->str() == "NULL" || tok->str() == "__null" ||
            tok->str() == "'\\0'" || tok->str() == "'\\x0'") {
            tok->str("0");
        } else if (tok->isNumber() &&
                   MathLib::isInt(tok->str()) &&
                   MathLib::toLongNumber(tok->str()) == 0) {
            tok->str("0");
        }

        // Combine "- %num%" ..
        if (Token::Match(tok, "?|:|,|(|[|=|return|case|sizeof|%op% - %num%")) {
            tok->deleteNext();
            tok->next()->str("-" + tok->next()->str());
        }

        // simplify round "(" parenthesis between "[;{}] and "{"
        if (Token::Match(tok, "[;{}] ( {") &&
            Token::simpleMatch(tok->linkAt(2), "} ) ;")) {
            tok->linkAt(2)->previous()->deleteNext(2);
            tok->deleteNext(2);
        }
    }


    // Convert K&R function declarations to modern C
    simplifyVarDecl(true);
    simplifyFunctionParameters();

    // check for simple syntax errors..
    for (const Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "> struct {") &&
            Token::simpleMatch(tok->linkAt(2), "} ;")) {
            syntaxError(tok);
            deallocateTokens();
            return false;
        }
    }

    // specify array size..
    arraySize();

    simplifyDoWhileAddBraces();

    if (!simplifyIfAddBraces())
        return false;

    // Combine tokens..
    for (Token *tok = _tokens; tok && tok->next(); tok = tok->next()) {
        const char c1 = tok->str()[0];

        if (tok->str().length() == 1 && tok->next()->str().length() == 1) {
            const char c2 = tok->next()->str()[0];

            // combine equal tokens..
            if (c1 == c2 && (c1 == '<' || c1 == '|' || c1 == ':')) {
                tok->str(tok->str() + c2);
                tok->deleteNext();
                if (c1 == '<' && Token::simpleMatch(tok->next(), "=")) {
                    tok->str("<<=");
                    tok->deleteNext();
                }
                continue;
            }

            // combine +-*/ and =
            else if (c2 == '=' && (strchr("+-*/%&|^=!<>", c1))) {
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

        else if ((c1 == 'p' || c1 == '_') && tok->next()->str() == ":" && tok->strAt(2) != ":") {
            if (tok->str() == "private" || tok->str() == "protected" || tok->str() == "public" || tok->str() == "__published") {
                tok->str(tok->str() + ":");
                tok->deleteNext();
                continue;
            }
        }
    }

    // simplify labels and 'case|default'-like syntaxes
    simplifyLabelsCaseDefault();

    // simplify '[;{}] * & ( %any% ) =' to '%any% ='
    simplifyMulAndParens();

    // ";a+=b;" => ";a=a+b;"
    simplifyCompoundAssignment();

    if (!preprocessorCondition) {
        if (hasComplicatedSyntaxErrorsInTemplates()) {
            deallocateTokens();
            return false;
        }
    }

    simplifyDefaultAndDeleteInsideClass();

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

    // Remove Qt signals and slots
    simplifyQtSignalsSlots();

    // remove Borland stuff..
    simplifyBorland();

    // Remove "volatile", "inline", "register", and "restrict"
    simplifyKeyword();

    // Remove __builtin_expect, likely and unlikely
    simplifyBuiltinExpect();

    if (hasEnumsWithTypedef()) {
        // #2449: syntax error: enum with typedef in it
        deallocateTokens();
        return false;
    }

    simplifyDebugNew();

    // typedef..
    simplifyTypedef();

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

    // Remove __asm..
    simplifyAsm();

    // When the assembly code has been cleaned up, no @ is allowed
    for (const Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(")
            tok = tok->link();
        else if (tok->str()[0] == '@') {
            deallocateTokens();
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
    /*for (Token* tok = _tokens; tok; tok = tok->next()) {
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

    simplifyConst();

    // struct simplification "struct S {} s; => struct S { } ; S s ;
    simplifyStructDecl();

    // struct initialization (must be used before simplifyVarDecl)
    simplifyStructInit();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl(false);

    // f(x=g())   =>   x=g(); f(x)
    simplifyAssignmentInFunctionCall();

    simplifyVariableMultipleAssign();

    // Remove redundant parentheses
    simplifyRedundantParenthesis();

    // Handle templates..
    simplifyTemplates();

    // Simplify templates.. sometimes the "simplifyTemplates" fail and
    // then unsimplified function calls etc remain. These have the
    // "wrong" syntax. So this function will just fix so that the
    // syntax is corrected.
    TemplateSimplifier::cleanupAfterSimplify(_tokens);

    // Simplify the operator "?:"
    simplifyConditionOperator();

    // remove exception specifications..
    removeExceptionSpecifications(_tokens);

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

    if (!preprocessorCondition) {
        setVarId();

        // Change initialisation of variable to assignment
        simplifyInitVar();
    }

    // Convert e.g. atol("0") into 0
    simplifyMathFunctions();

    simplifyDoublePlusAndDoubleMinus();

    simplifyArrayAccessSyntax();

    _tokens->assignProgressValues();

    removeRedundantSemicolons();

    simplifyReservedWordNullptr();

    simplifyParameterVoid();

    simplifyRedundantConsecutiveBraces();

    return validate();
}
//---------------------------------------------------------------------------

bool Tokenizer::hasComplicatedSyntaxErrorsInTemplates()
{
    // check for more complicated syntax errors when using templates..
    for (const Token *tok = _tokens; tok; tok = tok->next()) {
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
                syntaxError(tok);
                return true;
            }
        }
    }

    return false;
}

void Tokenizer::simplifyDefaultAndDeleteInsideClass()
{
    // Remove "= default|delete" inside class|struct definitions
    // Todo: Remove it if it is used "externally" too.
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "struct|class %var% :|{")) {
            unsigned int indentlevel = 0;
            for (Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}") {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                } else if (indentlevel == 1 && Token::Match(tok2, ") = delete|default ;")) {
                    Token * const end = tok2->tokAt(4);
                    tok2 = tok2->link()->previous();

                    // operator ==|>|<|..
                    if (Token::Match(tok2->previous(), "operator %any%"))
                        tok2 = tok2->previous();
                    else if (Token::simpleMatch(tok2->tokAt(-2), "operator [ ]"))
                        tok2 = tok2->tokAt(-2);
                    else if (Token::simpleMatch(tok2->tokAt(-2), "operator ( )"))
                        tok2 = tok2->tokAt(-2);
                    else if (Token::simpleMatch(tok2->tokAt(-3), "operator delete [ ]"))
                        tok2 = tok2->tokAt(-3);

                    while ((tok2->isName() && tok2->str().find(":") == std::string::npos) ||
                           Token::Match(tok2, "[&*~]"))
                        tok2 = tok2->previous();
                    if (Token::Match(tok2, "[;{}]") || tok2->isName())
                        Token::eraseTokens(tok2, end);
                    else
                        tok2 = end->previous();
                }
            }
        }
    }
}

bool Tokenizer::hasEnumsWithTypedef()
{
    for (const Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "enum %var% {")) {
            tok = tok->tokAt(2);
            const Token *tok2 = Token::findmatch(tok, "typedef", tok->link());
            if (tok2) {
                syntaxError(tok2);
                return true;
            }
        }
    }

    return false;
}

void Tokenizer::simplifyDebugNew()
{
    // convert Microsoft DEBUG_NEW macro to new
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "DEBUG_NEW")
            tok->str("new");
    }
}

void Tokenizer::simplifyArrayAccessSyntax()
{
    // 0[a] -> a[0]
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%num% [ %var% ]")) {
            const std::string temp = tok->str();
            tok->str(tok->strAt(2));
            tok->tokAt(2)->str(temp);
        }
    }
}

void Tokenizer::simplifyParameterVoid()
{
    for (Token* tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% ( void )"))
            tok->next()->deleteNext();
    }
}

void Tokenizer::simplifyReservedWordNullptr()
{
    if (_settings->standards.cpp11) {
        for (Token *tok = _tokens; tok; tok = tok->next()) {
            if (tok->str() == "nullptr")
                tok->str("0");
        }
    }
}

void Tokenizer::simplifyRedundantConsecutiveBraces()
{
    // Remove redundant consecutive braces, i.e. '.. { { .. } } ..' -> '.. { .. } ..'.
    for (Token *tok = _tokens; tok;) {
        if (Token::simpleMatch(tok, "{ {") && Token::simpleMatch(tok->next()->link(), "} }")) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (tok->next()) {
            if (tok->str() == "+") {
                if (tok->next()->str() == "+") {
                    tok->deleteNext();
                    continue;
                } else if (tok->next()->str() == "-") {
                    tok->str("-");
                    tok->deleteNext();
                    continue;
                }
            } else if (tok->str() == "-") {
                if (tok->next()->str() == "-") {
                    tok->str("+");
                    tok->deleteNext();
                    continue;
                } else if (tok->next()->str() == "+") {
                    tok->deleteNext();
                    continue;
                }
            }

            break;
        }
    }
}

void Tokenizer::simplifyJavaAndCSharp()
{
    // better don't call isJava in the loop
    const bool isJava_ = isJava();
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "private")
            tok->str("private:");
        else if (tok->str() == "protected")
            tok->str("protected:");
        else if (tok->str() == "public")
            tok->str("public:");

        else if (isJava_) {
            if (Token::Match(tok, ") throws %var% {"))
                tok->deleteNext(2);
        } else {
            //remove 'using var;' from code
            if (Token::Match(tok, "using %var% ;") &&
                (!tok->previous() || Token::Match(tok->previous(), "[,;{}]"))) {
                tok->deleteNext(2);
                tok->deleteThis();
            }

            //simplify C# arrays of arrays and multidimension arrays
            while (Token::Match(tok, "%type% [ ,|]") &&
                   (!tok->previous() || Token::Match(tok->previous(), "[,;{}]"))) {
                Token *tok2 = tok->tokAt(2);
                unsigned int count = 1;
                while (tok2 && tok2->str() == ",") {
                    ++count;
                    tok2 = tok2->next();
                }
                if (!tok2 || tok2->str() != "]")
                    break;
                tok2 = tok2->next();
                while (Token::Match(tok2, "[ ,|]")) {
                    tok2 = tok2->next();
                    while (tok2 && tok2->str() == ",") {
                        ++count;
                        tok2 = tok2->next();
                    }
                    if (!tok2 || tok2->str() != "]")
                        break;
                    ++count;
                    tok2 = tok2->next();
                }
                if (!tok2)
                    break;
                else if (Token::Match(tok2, "%var% [;,=]")) {
                    Token::eraseTokens(tok, tok2);
                    do {
                        tok->insertToken("*");
                    } while (--count);
                    tok = tok2->tokAt(2);
                }
            }
            if (!tok)
                break;
        }
    }
}

/** Specify array size if it hasn't been given */

void Tokenizer::arraySize()
{
    bool addlength = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% [ ] = { %str% } ;")) {
            Token *t = tok->tokAt(3);
            t->deleteNext();
            t->next()->deleteNext();
            addlength = true;
        }

        if (addlength || Token::Match(tok, "%var% [ ] = %str% ;")) {
            tok = tok->next();
            std::size_t sz = tok->strAt(3).length() - 1;
            tok->insertToken(MathLib::toString<unsigned int>((unsigned int)sz));
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
                tok->insertToken(MathLib::toString<unsigned int>(sz));

            tok = end->next() ? end->next() : end;
        }
    }
}

/** simplify labels and case|default in the code: add a ";" if not already in.*/

void Tokenizer::simplifyLabelsCaseDefault()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, ") const| {")) {
            // Simplify labels in the executable scope..
            unsigned int indentlevel = 0;
            while (NULL != (tok = tok->next())) {
                if (tok->str() == "{") {
                    if (tok->previous() && tok->previous()->str() == "=")
                        tok = tok->link();
                    else
                        ++indentlevel;
                } else if (tok->str() == "}") {
                    --indentlevel;
                    if (!indentlevel)
                        break;
                } else if (tok->str() == "(" || tok->str() == "[")
                    tok = tok->link();

                if (Token::Match(tok, "[;{}] case")) {
                    while (NULL != (tok = tok->next())) {
                        if (tok->str() == ":")
                            break;
                    }
                    if (Token::Match(tok, ": !!;")) {
                        tok->insertToken(";");
                    }
                } else if (Token::Match(tok, "[;{}] %var% : !!;")) {
                    tok = tok->tokAt(2);
                    tok->insertToken(";");
                }
            }
        }
    }
}


/**
 * is the token pointing at a template parameters block
 * < int , 3 > => yes
 * \param tok start token that must point at "<"
 * \return number of parameters (invalid parameters => 0)
 */
static unsigned int templateParameters(const Token *tok)
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
        if (tok->str() == 