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
        if (!one_line && tok->next())
            linenrs += tok->next()->linenr() - tok->linenr();
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
    std::stack<unsigned int> lineNumbers;

    // fileIndexes holds index for _files vector about currently parsed files
    // every time an include file is completely parsed, last item in the vector
    // is removed and FileIndex is set to point to that value.
    std::stack<unsigned int> fileIndexes;

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
                fileIndexes.push(FileIndex);
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

                lineNumbers.push(lineno);
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

                lineno = lineNumbers.top();
                lineNumbers.pop();
                FileIndex = fileIndexes.top();
                fileIndexes.pop();
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
    simplifySQL();

    // Simplify JAVA/C# code
    if (isJavaOrCSharp())
        simplifyJavaAndCSharp();

    // Concatenate double sharp: 'a ## b' -> 'ab'
    concatenateDoubleSharp();

    if (!createLinks()) {
        // Source has syntax errors, can't proceed
        return false;
    }

    // replace __LINE__ macro with line number
    simplifyLineMacro();

    // replace 'NULL' and similiar '0'-defined macros with '0'
    simplifyNull();

    // combine "- %num%"
    concatenateNegativeNumber();

    // simplify weird but legal code: "[;{}] ( { code; } ) ;"->"[;{}] code;"
    simplifyRoundCurlyParenthesis();

    // Convert K&R function declarations to modern C
    simplifyVarDecl(true);
    if (!simplifyFunctionParameters())
        return false;

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
    const Token *tok = TemplateSimplifier::hasComplicatedSyntaxErrorsInTemplates(_tokens);
    if (tok) {
        syntaxError(tok);
        return true;
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
            const Token *tok2 = Token::findsimplematch(tok, "typedef", tok->link());
            if (tok2) {
                syntaxError(tok2);
                return true;
            }
        }
    }

    return false;
}

void Tokenizer::concatenateDoubleSharp()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // TODO: pattern should be "%var%|%num% ## %var%|%num%"
        while (Token::Match(tok, "%any% ## %any%") &&
               (tok->isName() || tok->isNumber()) &&
               (tok->tokAt(2)->isName() || tok->tokAt(2)->isNumber())) {
            tok->str(tok->str() + tok->strAt(2));
            tok->deleteNext(2);
        }
    }
}

void Tokenizer::simplifyLineMacro()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "__LINE__")
            tok->str(MathLib::toString(tok->linenr()));
    }
}

void Tokenizer::simplifyNull()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "NULL" || tok->str() == "__null" ||
            tok->str() == "'\\0'" || tok->str() == "'\\x0'") {
            tok->str("0");
        } else if (tok->isNumber() &&
                   MathLib::isInt(tok->str()) &&
                   MathLib::toLongNumber(tok->str()) == 0) {
            tok->str("0");
        }
    }
}

void Tokenizer::concatenateNegativeNumber()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "?|:|,|(|[|=|return|case|sizeof|%op% - %num%")) {
            tok->deleteNext();
            tok->next()->str("-" + tok->next()->str());
        }
    }
}

void Tokenizer::simplifyRoundCurlyParenthesis()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (Token::Match(tok, "[;{}] ( {") &&
               Token::simpleMatch(tok->linkAt(2), "} ) ;")) {
            tok->linkAt(2)->previous()->deleteNext(3);
            tok->deleteNext(2);
        }
    }
}

void Tokenizer::simplifySQL()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "EXEC SQL")) {
            const Token *end = tok->tokAt(2);
            while (end && end->str() != ";")
                end = end->next();

            std::string instruction = tok->stringify(end);
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
    bool executablescope = false;
    unsigned int indentlevel = 0;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // Simplify labels in the executable scope..
        if (Token::Match(tok, ") const| {")) {
            tok = tok->next();
            if (tok->str() == "const")
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
                if (tok->str() == ":")
                    break;
            }
            if (!tok)
                break;
            else if (tok->str() == ":" &&
                     (!tok->next() || tok->next()->str() != ";")) {
                tok->insertToken(";");
            }
        } else if (Token::Match(tok, "[;{}] %var% : !!;")) {
            tok = tok->tokAt(2);
            tok->insertToken(";");
        }
    }
}




void Tokenizer::simplifyTemplates()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // #2648 - simple fix for sizeof used as template parameter
        // TODO: this is a bit hardcoded. make a bit more generic
        if (Token::Match(tok, "%var% < sizeof ( %type% ) >") && tok->tokAt(4)->isStandardType()) {
            Token * const tok3 = tok->next();
            const unsigned int sizeOfResult = sizeOfType(tok3->tokAt(3));
            tok3->deleteNext(4);
            tok3->insertToken(MathLib::toString<unsigned int>(sizeOfResult));
        }
    }

    TemplateSimplifier::simplifyTemplates(
        _tokens,
        &_tokensBack,
        _errorLogger,
        _settings,
        _files,
        _codeWithTemplates);
}
//---------------------------------------------------------------------------

std::string Tokenizer::getNameForFunctionParams(const Token *start)
{
    if (start->next() == start->link())
        return "";

    std::string result;
    bool findNextComma = false;
    for (const Token *tok = start->next(); tok && tok != start->link(); tok = tok->next()) {
        if (findNextComma) {
            if (tok->str() == ",")
                findNextComma = false;

            continue;
        }

        result.append(tok->str() + ",");
        findNextComma = true;
    }

    return result;
}

void Tokenizer::setVarId()
{
    // Clear all variable ids
    for (Token *tok = _tokens; tok; tok = tok->next())
        tok->varId(0);

    // Set variable ids..
    _varId = 0;
    unsigned int executableScope = 0;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "{") {
            if (executableScope)
                executableScope++;
            else if (tok->strAt(-1) == ")" || Token::simpleMatch(tok->tokAt(-2), ") const"))
                executableScope = 1;
        } else if (executableScope >= 1 && tok->str() == "}")
            --executableScope;

        if (tok != _tokens && !Token::Match(tok, "[;{}(,] %type%") && !Token::Match(tok, "[;{}(,] ::"))
            continue;

        // Ticket #3104 - "if (NOT x)"
        if (tok->str() == "(" && tok->next()->str() == "NOT")
            continue;

        if (_errorLogger)
            _errorLogger->reportProgress(_files[0], "Tokenize (set variable id)", tok->progressValue());

        // If pattern is "( %type% *|& %var% )" then check if it's a
        // variable declaration or a multiplication / mask
        if (Token::Match(tok, "( %type% *|& %var% [),]") && !tok->next()->isStandardType()) {
            if (!Token::Match(tok->previous(), "%type%"))
                continue;
            if (tok->previous() && tok->previous()->str() == "return")
                continue;
            if (tok->link() && !Token::Match(tok->link()->next(), "const| {") &&
                (!tok->link()->next() || tok->link()->next()->str() != ":"))
                continue;
        }

        if (Token::Match(tok, "[,;{}(] %type%") || Token::Match(tok, "[;{}(,] ::")) {
            // not function declaration?
            // TODO: Better checking
            if (Token::Match(tok->tokAt(-2), "= %var% (")) {
                continue;
            }
            if (tok->str() == "(" &&
                tok->previous() &&
                !tok->previous()->isName() &&
                tok->strAt(-2) != "operator")
                continue;
            if (executableScope && tok->str() == "(" && Token::simpleMatch(tok->link(),") ;")) {
                tok = tok->link();
                continue;
            }

            tok = tok->next();
        }

        if (tok->str() == "new")
            continue;

        if (tok->str() == "throw")
            continue;

        if (tok->str() == "virtual")
            continue;

        if (Token::Match(tok, "class|struct|union %type% :|{|;")) {
            if (tok->strAt(2) == ":") {
                while (tok->next() && !Token::Match(tok->next(),"[;{]"))
                    tok = tok->next();
            }
            continue;
        }

        while (Token::Match(tok, "public:|private:|protected:"))
            tok = tok->next();
        if (!tok)
            break;

        if (tok->str() == "unsigned")
            tok = tok->next();

        if (Token::Match(tok, "using namespace %type% ;")) {
            tok = tok->next();
            continue;
        }

        if (Token::Match(tok, "goto %any% ;"))
            continue;

        if (Token::Match(tok, "else|return|typedef|delete|sizeof"))
            continue;

        while (Token::Match(tok, "const|static|extern|;|mutable"))
            tok = tok->next();

        if (tok && tok->str() == "friend")
            continue;

        if (Token::Match(tok, "struct %type%"))
            tok = tok->next();

        // skip global namespace prefix
        if (tok && tok->str() == "::")
            tok = tok->next();

        while (Token::Match(tok, "%var% ::"))
            tok = tok->tokAt(2);

        // Skip template arguments..
        if (Token::Match(tok, "%type% <")) {
            int level = 1;
            bool again;
            Token *tok2 = tok->tokAt(2);

            do { // Look for start of templates or template arguments
                if (!tok2) // syntax error
                    return;

                again = false;

                if (tok2 && tok2->str() == "const")
                    tok2 = tok2->next();

                while (Token::Match(tok2, "%var% ::"))
                    tok2 = tok2->tokAt(2);

                if (Token::Match(tok2, "%type% <")) {
                    ++level;
                    tok2 = tok2->tokAt(2);
                    again = true;
                } else if (Token::Match(tok2, "%type% *|&| ,")) {
                    tok2 = tok2->tokAt(2);
                    if (!tok2) // syntax error
                        return;
                    if (tok2->str() == ",") {
                        tok2 = tok2->next();
                        if (!tok2) // syntax error
                            return;
                    }
                    again = true;
                } else if (level > 1 && (Token::Match(tok2, "%type% *|&| >") ||
                                         Token::Match(tok2, "%num% >"))) {
                    --level;
                    while (tok2->str() != ">")
                        tok2 = tok2->next();
                    tok2 = tok2->next();
                    if (!tok2) // syntax error
                        return;
                    if (tok2->str() == ",") {
                        tok2 = tok2->next();
                        if (!tok2) // syntax error
                            return;
                    }
                    if (level == 1 && tok2->str() == ">")
                        break;
                    again = true;
                } else {
                    while (tok2 && (tok2->isName() || tok2->isNumber() || tok2->str() == "*" || tok2->str() == "&" || tok2->str() == ","))
                        tok2 = tok2->next();
                    if (tok2 && tok2->str() == "(") {
                        tok2 = tok2->link()->next();
                        if (tok2 && tok2->str() == "(")
                            tok2 = tok2->link()->next();
                        again = true;
                    }
                }
            } while (again);

            do { // Look for end of templates
                again = false;

                if (level == 1 && Token::Match(tok2, "> %var%"))
                    tok = tok2;
                else if (level > 1 && tok2 && tok2->str() == ">") {
                    --level;
                    if (level == 0)
                        tok = tok2;
                    else {
                        tok2 = tok2->next();
                        again = true;
                    }
                } else if (level == 1 && Token::Match(tok2, "> ::|*|& %var%"))
                    tok = tok2->next();
                else
                    continue;       // Not code that I understand / not a variable declaration
            } while (again);
        }

        if (Token::Match(tok, "%str%"))
            continue;

        // Determine name of declared variable..
        std::string varname;
        Token *tok2 = tok ? tok->next() : 0;
        while (tok2) {
            if (tok2->isName()) {
                if (tok2->str() == "const")
                    varname.clear();
                else
                    varname = tok2->str();
            } else if (tok2->str() != "*" && tok2->str() != "&")
                break;

            // a type can't have varid
            if (tok2->previous()->varId() > 0) {
                tok2 = 0;
                break;
            }

            tok2 = tok2->next();
        }

        if (executableScope && Token::simpleMatch(tok2, ") ;"))
            continue;

        if (Token::Match(tok2 ? tok2->tokAt(-2) : 0, "class|struct %type% ;"))
            continue;

        // End of tokens reached..
        if (!tok2)
            break;

        if (varname == "operator" && Token::Match(tok2, "=|+|-|*|/|[| ]| ("))
            continue;

        if (varname == "new" && Token::Match(tok2->tokAt(-2), "operator new (|["))
            continue;

        // Is it a function?
        if (tok2->str() == "(") {
            // Search for function declaration, e.g. void f();
            if (Token::simpleMatch(tok2->next(), ") ;"))
                continue;

            // Search for function declaration, e.g. void f( int c );
            if (!Token::Match(tok2->next(), "%num%") &&
                !Token::Match(tok2->next(), "%bool%") &&
                tok2->next()->str()[0] != '"' &&
                tok2->next()->str()[0] != '\'' &&
                tok2->next()->str() != "*" &&
                tok2->next()->str() != "&" &&
                tok2->next()->varId() == 0)
                continue;
        }

        // Don't set variable id for 'AAA a[0] = 0;' declaration (#2638)
        if (tok2->previous()->varId() && tok2->str() == "[") {
            const Token *tok3 = tok2;
            while (tok3 && tok3->str() == "[") {
                tok3 = tok3->link();
                tok3 = tok3 ? tok3->next() : NULL;
            }
            if (Token::Match(tok3, "= !!{"))
                continue;
        }

        // Variable declaration found => Set variable ids
        if (Token::Match(tok2, "[,();[=]") && !varname.empty()) {
            // Are we in a class declaration?
            // Then start at the start of the class declaration..
            while (NULL != (tok2 = tok2->previous())) {
                if (tok2->str() == "}" || tok2->str() == ")")
                    tok2 = tok2->link();
                else if (tok2->str() == "(")
                    break;
                else if (tok2->str() == "{") {
                    while (NULL != (tok2 = tok2->previous())) {
                        if (Token::Match(tok2, "[,;{})]")) {
                            if (!Token::Match(tok2, ", public|protected|private"))
                                break;
                        }
                        if (Token::Match(tok2, "class|struct"))
                            break;
                    }
                    break;
                }
            }

            /** @todo better handling when classes in different scopes have the same name */
            std::string className;
            if (Token::Match(tok2, "class|struct %type% [:{]"))
                className = tok2->next()->str();

            // Set start token
            if (Token::Match(tok2, "class|struct")) {
                while (tok2->str() != "{")
                    tok2 = tok2->next();
            } else
                tok2 = tok;

            ++_varId;
            int indentlevel = 0;
            int parlevel = 0;
            bool funcDeclaration = false;
            while (NULL != (tok2 = tok2->next())) {
                const char c = tok2->str()[0];
                if (c == varname[0]) {
                    if (tok2->str() == varname && (className.empty() || tok2->varId() == 0)) {
                        const std::string &prev = tok2->previous()->str();

                        /** @todo better handling when classes in different scopes have the same name */
                        if (!className.empty() && Token::Match(tok2->tokAt(-3), ("!!:: " + className + " ::").c_str()))
                            tok2->varId(_varId);

                        else if (tok2->str() == varname && prev != "struct" && prev != "union" && prev != "::" && prev != "." && (!tok2->next() || tok2->next()->str() != "::"))
                            tok2->varId(_varId);
                    }
                } else if (c == '{')
                    ++indentlevel;
                else if (c == '}') {
                    --indentlevel;
                    if (indentlevel < 0)
                        break;

                    // We have reached the end of a loop: "for( int i;;) {  }"
                    if (funcDeclaration && indentlevel <= 0)
                        break;
                } else if (c == '(')
                    ++parlevel;
                else if (c == ')') {
                    // Is this a function parameter or a variable declared in for example a for loop?
                    if (parlevel == 0 && indentlevel == 0 && Token::Match(tok2, ") const| {"))
                        funcDeclaration = true;
                    else
                        --parlevel;
                } else if (parlevel < 0 && c == ';')
                    break;
            }
        }
    }

    // Member functions and variables in this source
    std::list<Token *> allMemberFunctions;
    std::list<Token *> allMemberVars;
    {
        for (Token *tok2 = _tokens; tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "%var% :: %var%")) {
                if (Token::simpleMatch(tok2->tokAt(3), "("))
                    allMemberFunctions.push_back(tok2);
                else if (tok2->tokAt(2)->varId() != 0)
                    allMemberVars.push_back(tok2);
            }
        }
    }

    // class members..
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct %var% {|:")) {
            const std::string &classname(tok->next()->str());

            // What member variables are there in this class?
            std::map<std::string, unsigned int> varlist;
            {
                unsigned int indentlevel = 0;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                    // Indentation..
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}") {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    }

                    // skip parentheses..
                    else if (tok2->str() == "(")
                        tok2 = tok2->link();

                    // Found a member variable..
                    else if (indentlevel == 1 && tok2->varId() > 0)
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

            // Member functions for this class..
            std::list<Token *> funclist;
            {
                const std::string funcpattern(classname + " :: %var% (");
                for (std::list<Token *>::iterator func = allMemberFunctions.begin(); func != allMemberFunctions.end(); ++func) {
                    Token *tok2 = *func;

                    // Found a class function..
                    if (Token::Match(tok2, funcpattern.c_str())) {
                        // Goto the end parenthesis..
                        tok2 = tok2->linkAt(3);
                        if (!tok2)
                            break;

                        // If this is a function implementation.. add it to funclist
                        if (Token::Match(tok2, ") const|volatile| {"))
                            funclist.push_back(tok2);
                    }
                }
            }

            // Update the variable ids..
            // Parse each function..
            for (std::list<Token *>::iterator func = funclist.begin(); func != funclist.end(); ++func) {
                unsigned int indentlevel = 0;
                for (Token *tok2 = *func; tok2; tok2 = tok2->next()) {
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}") {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    } else if (indentlevel > 0 &&
                               tok2->varId() == 0 &&
                               !Token::simpleMatch(tok2->previous(), ".") &&
                               varlist.find(tok2->str()) != varlist.end()) {
                        tok2->varId(varlist[tok2->str()]);
                    }
                }
            }

        }
    }

    // Struct/Class members
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // str.clear is a variable
        // str.clear() is a member function
        if (tok->varId() != 0 &&
            Token::Match(tok->next(), ". %var% !!(") &&
            tok->tokAt(2)->varId() == 0) {
            ++_varId;

            const std::string pattern(std::string(". ") + tok->strAt(2));
            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->varId() == tok->varId()) {
                    if (Token::Match(tok2->next(), pattern.c_str()))
                        tok2->tokAt(2)->varId(_varId);
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
    return(true);
}

bool Tokenizer::createLinks()
{
    std::stack<const Token*> type;
    std::stack<Token*> links1;
    std::stack<Token*> links2;
    std::stack<Token*> links3;
    for (Token *token = _tokens; token; token = token->next()) {
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

void Tokenizer::simplifySizeof()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "class|struct %var%")) {
            // we assume that the size of structs and classes are always
            // 100 bytes.
            _typeSize[tok->next()->str()] = 100;
        }
    }

    // Locate variable declarations and calculate the size
    std::map<unsigned int, std::string> sizeOfVar;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

                sizeOfVar[varId] = MathLib::toString<unsigned int>(size);
            }

            else if (Token::Match(tok->tokAt(-3), "[;{}(,] struct %type% %var% [;,)]")) {
                sizeOfVar[varId] = "100";
            }

            else if (Token::Match(tok->previous(), "%type% %var% [ %num% ] [;=]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [;=]")) {
                const unsigned int size = sizeOfType(tok->previous());
                if (size == 0)
                    continue;

                sizeOfVar[varId] = MathLib::toString<unsigned long>(size * static_cast<unsigned long>(MathLib::toLongNumber(tok->strAt(2))));
            }

            else if (Token::Match(tok->previous(), "%type% %var% [ %num% ] [,)]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [,)]")) {
                Token tempTok(0);
                tempTok.str("*");
                sizeOfVar[varId] = MathLib::toString<unsigned long>(sizeOfType(&tempTok));
            }

            else if (Token::Match(tok->previous(), "%type% %var% [ ] = %str% ;")) {
                const unsigned int size = sizeOfType(tok->tokAt(4));
                if (size == 0)
                    continue;

                sizeOfVar[varId] = MathLib::toString<unsigned int>(size);
            }
        }
    }

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() != "sizeof")
            continue;

        if (!tok->next())
            break;

        if (Token::simpleMatch(tok->next(), "sizeof"))
            continue;

        if (Token::simpleMatch(tok->next(), ". . .")) {
            tok->deleteNext(3);
        }

        // sizeof 'x'
        if (tok->next() && tok->next()->str()[0] == '\'') {
            tok->deleteThis();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            continue;
        }

        // sizeof('x')
        if (Token::Match(tok, "sizeof ( %any% )") && tok->strAt(2)[0] == '\'') {
            tok->deleteNext();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            continue;
        }

        // sizeof "text"
        if (Token::Match(tok->next(), "%str%")) {
            tok->deleteThis();
            std::ostringstream ostr;
            ostr << (Token::getStrLength(tok) + 1);
            tok->str(ostr.str());
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
            // Add parenthesis around the sizeof
            int parlevel = 0;
            for (Token *tempToken = tok->next(); tempToken; tempToken = tempToken->next()) {
                if (tempToken->str() == "(")
                    ++parlevel;
                else if (tempToken->str() == ")")
                    --parlevel;
                if (Token::Match(tempToken, "%var%")) {
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
                    } else if (Token::simpleMatch(tempToken->next(), "- >")) {
                        // We are checking a class or struct, search next varname
                        tempToken = tempToken->tokAt(2);
                        continue;
                    } else if (Token::Match(tempToken->next(), "++|--")) {
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
            tok->str(MathLib::toString<unsigned long>(sizeOfType(tok->tokAt(2))));
            tok->deleteNext(3);
        }

        // sizeof( a )
        else if (Token::Match(tok->next(), "( %var% )") && tok->tokAt(2)->varId() != 0) {
            if (sizeOfVar.find(tok->tokAt(2)->varId()) != sizeOfVar.end()) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
                tok->str(sizeOfVar[tok->varId()]);
            } else {
                // don't try to replace size of variable if variable has
                // similar name with type (#329)
            }
        }

        else if (Token::Match(tok, "sizeof ( %type% )")) {
            unsigned int size = sizeOfType(tok->tokAt(2));
            if (size > 0) {
                tok->str(MathLib::toString<unsigned int>(size));
                tok->deleteNext(3);
            }
        }

        else if (Token::Match(tok, "sizeof ( * %var% )") || Token::Match(tok, "sizeof ( %var% [ %num% ] )")) {
            // Some default value..
            size_t sz = 0;

            unsigned int varid = tok->tokAt((tok->strAt(2) == "*") ? 3 : 2)->varId();
            if (varid != 0) {
                // Try to locate variable declaration..
                const Token *decltok = Token::findmatch(_tokens, "%varid%", varid);
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
                tok->str(MathLib::toString<size_t>(sz));
                Token::eraseTokens(tok, tok->next()->link()->next());
            }
        }
    }

}

bool Tokenizer::simplifyTokenList()
{
    // clear the _functionList so it can't contain dead pointers
    delete _symbolDatabase;
    _symbolDatabase = NULL;

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "* const"))
            tok->deleteNext();
    }

    // simplify references
    simplifyReference();

    simplifyStd();

    simplifyGoto();

    // Combine wide strings
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (tok->str() == "L" && tok->next() && tok->next()->str()[0] == '"') {
            // Combine 'L "string"'
            tok->str(tok->next()->str());
            tok->deleteNext();
        }
    }

    // Combine strings
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str()[0] != '"')
            continue;

        tok->str(simplifyString(tok->str()));
        while (tok->next() && tok->next()->str()[0] == '"') {
            tok->next()->str(simplifyString(tok->next()->str()));

            // Two strings after each other, combine them
            tok->concatStr(tok->next()->str());
            tok->deleteNext();
        }
    }

    simplifySizeof();

    // change array to pointer..
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%type% %var% [ ] [,;=]")) {
            tok->next()->deleteNext(2);
            tok->insertToken("*");
        }
    }

    // Replace constants..
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "const %type% %var% = %num% ;")) {
            unsigned int varId = tok->tokAt(2)->varId();
            if (varId == 0) {
                tok = tok->tokAt(5);
                continue;
            }

            const std::string num = tok->strAt(4);
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

    simplifyCasts();

    // Simplify simple calculations..
    simplifyCalculations();

    // Replace "*(str + num)" => "str[num]"
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!Token::Match(tok, "%var%") && !Token::Match(tok, "%num%")
            && !Token::Match(tok, "]|)")
            && (Token::Match(tok->next(), "* ( %var% + %num% )") ||
                Token::Match(tok->next(), "* ( %var% + %var% )"))) {
            // remove '* ('
            tok->deleteNext(2);

            tok = tok->tokAt(2);
            // '+'->'['
            tok->str("[");

            tok = tok->tokAt(2);
            tok->str("]");
            Token::createMutualLinks(tok->tokAt(-2), tok);
        }
    }

    // Replace "&str[num]" => "(str + num)"
    //TODO: fix the fails testrunner reports:
    //1)
    //test/teststl.cpp:805: Assertion failed.
    //Expected:
    //"[test.cpp:7]: (error) Invalid pointer 'first' after push_back / push_front\n".
    //Actual:
    //"".
    /*for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!Token::Match(tok, "%var%") && !Token::Match(tok, "%num%")
            && !Token::Match(tok, "]|)")
            && (Token::Match(tok->next(), "& %var% [ %num% ]") ||
                Token::Match(tok->next(), "& %var% [ %var% ]"))) {
            tok = tok->next();
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
    }*/

    // simplify "x=realloc(y,0);" => "free(y); x=0;"..
    // and "x = realloc (0, n);" => "x = malloc(n);"
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}:] %var% = realloc ( %var% , 0 ) ;")) {
            const std::string varname(tok->next()->str());
            const unsigned int varid(tok->next()->varId());

            // Delete the "%var% ="
            tok->deleteNext(2);

            // Change function name "realloc" to "free"
            tok->next()->str("free");

            // delete the ", 0"
            tok = tok->tokAt(3);
            tok->deleteNext(2);

            // goto the ";"
            tok = tok->tokAt(2);

            // insert "var=0;"
            tok->insertToken(";");
            tok->insertToken("0");
            tok->insertToken("=");
            tok->insertToken(varname);
            tok->next()->varId(varid);
        } else if (Token::Match(tok, "[;{}:] %var% = realloc ( 0 , %num% ) ;")) {
            tok = tok->tokAt(3);
            // Change function name "realloc" to "malloc"
            tok->str("malloc");

            // delete "0 ,"
            tok->next()->deleteNext(2);
        }
    }

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Simplify variable declarations
    simplifyVarDecl(false);

    elseif();
    simplifyErrNoInWhile();
    simplifyIfAssign();
    simplifyRedundantParenthesis();
    simplifyIfNot();
    simplifyIfNotNull();
    simplifyIfSameInnerCondition();
    simplifyComparisonOrder();
    simplifyNestedStrcat();
    simplifyWhile0();
    simplifyFuncInWhile();

    simplifyIfAssign();    // could be affected by simplifyIfNot

    // In case variable declarations have been updated...
    setVarId();

    bool modified = true;
    while (modified) {
        modified = false;
        modified |= simplifyConditions();
        modified |= simplifyFunctionReturn();
        modified |= simplifyKnownVariables();
        modified |= removeRedundantConditions();
        modified |= simplifyRedundantParenthesis();
        modified |= simplifyQuestionMark();
        modified |= simplifyCalculations();
    }

    // replace strlen(str)
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "strlen ( %str% )")) {
            std::ostringstream ostr;
            ostr << Token::getStrLength(tok->tokAt(2));
            tok->str(ostr.str());
            tok->deleteNext(3);
        }
    }

    // simplify redundant for
    removeRedundantFor();

    // Remove redundant parentheses in return..
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

    simplifyReturn();

    removeRedundantAssignment();

    simplifyComma();

    removeRedundantSemicolons();

    simplifyFlowControl();

    simplifyRedundantConsecutiveBraces();

    if (!validate())
        return false;

    _tokens->assignProgressValues();

    if (_settings->debug) {
        _tokens->printOut(0, _files);

        if (_settings->_verbose)
            getSymbolDatabase()->printOut("Symbol database");
    }

    if (_settings->debugwarnings) {
        printUnknownTypes();
    }

    return true;
}
//---------------------------------------------------------------------------

void Tokenizer::removeMacrosInGlobalScope()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(") {
            tok = tok->link();
            if (Token::Match(tok, ") %type% {") &&
                !Token::Match(tok->next(), "const|namespace|class|struct|union"))
                tok->deleteNext();
        }

        if (tok->str() == "{")
            tok = tok->link();
    }
}
//---------------------------------------------------------------------------

void Tokenizer::removeRedundantAssignment()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
                           !Token::Match(tok2->previous(), "[;{}] %var% = %var% ;") &&
                           !Token::Match(tok2->previous(), "[;{}] %var% = %num% ;") &&
                           !(Token::Match(tok2->previous(), "[;{}] %var% = %any% ;") && tok2->strAt(2)[0] == '\'')) {
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

void Tokenizer::simplifyFlowControl()
{
    unsigned int indentlevel = 0;
    Token *beginindent = 0;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(" || tok->str() == "[") {
            tok = tok->link();
            continue;
        }

        if (tok->str() == "{") {
            if (tok->previous() && tok->previous()->str() == "=") {
                tok = tok->link();
                continue;
            }
            beginindent = tok;
            ++indentlevel;
        } else if (tok->str() == "}") {
            if (!indentlevel)
                break;
            --indentlevel;
        }

        if (!indentlevel)
            continue;

        if (Token::Match(tok, "goto %var% ;")) {
            tok = tok->tokAt(2);
            eraseDeadCode(tok, beginindent->link());

        } else if (Token::Match(tok,"continue|break ;")) {
            tok = tok->next();
            eraseDeadCode(tok, beginindent->link());

        } else if (Token::Match(tok,"return|throw|exit|abort")) {
            //catch the first ';'
            for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(" || tok2->str() == "[") {
                    tok2 = tok2->link();
                } else if (tok2->str() == ";") {
                    tok = tok2;
                    eraseDeadCode(tok, beginindent->link());
                    break;
                } else if (Token::Match(tok2, "[{}]"))
                    break;  //Wrong code.
            }
        }
    }
}


bool Tokenizer::removeRedundantConditions()
{
    // Return value for function. Set to true if there are any simplifications
    bool ret = false;

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() != "if")
            continue;

        if (!Token::Match(tok->next(), "( %bool% ) {"))
            continue;

        // Find matching else
        const Token *elseTag = 0;

        // Find the closing "}"
        elseTag = tok->linkAt(4)->next();

        bool boolValue = false;
        if (tok->strAt(2) == "true")
            boolValue = true;

        // Handle if with else
        if (elseTag && elseTag->str() == "else") {
            if (Token::simpleMatch(elseTag->next(), "if (")) {
                // Handle "else if"
                if (boolValue == false) {
                    // Convert "if( false ) {aaa;} else if() {bbb;}" => "if() {bbb;}"
                    Token::eraseTokens(tok, elseTag->tokAt(2));
                    ret = true;
                } else {
                    // Keep first if, remove every else if and else after it
                    const Token *lastTagInIf = elseTag->tokAt(2);
                    while (lastTagInIf) {
                        if (lastTagInIf->str() == "(") {
                            lastTagInIf = lastTagInIf->link()->next();
                        }

                        lastTagInIf = lastTagInIf->link()->next();
                        if (!Token::simpleMatch(lastTagInIf, "else"))
                            break;

                        lastTagInIf = lastTagInIf->next();
                        if (lastTagInIf->str() == "if")
                            lastTagInIf = lastTagInIf->next();
                    }

                    Token::eraseTokens(elseTag->previous(), lastTagInIf);
                    ret = true;
                }
            } else {
                // Handle else
                if (boolValue == false) {
                    // Convert "if( false ) {aaa;} else {bbb;}" => "{bbb;}" or ";{bbb;}"
                    if (tok->previous())
                        tok = tok->previous();
                    else
                        tok->str(";");

                    Token::eraseTokens(tok, elseTag->next());
                } else {
                    if (elseTag->next()->str() == "{") {
                        // Convert "if( true ) {aaa;} else {bbb;}" => "{aaa;}"
                        const Token *end = elseTag->next()->link();

                        // Remove the "else { aaa; }"
                        Token::eraseTokens(elseTag->previous(), end->next());
                    }

                    // Remove "if( true )"
                    if (tok->previous())
                        tok = tok->previous();
                    else
                        tok->str(";");

                    tok->deleteNext(4);
                }

                ret = true;
            }
        }

        // Handle if without else
        else {
            if (boolValue == false) {
                // Remove if and its content
                if (tok->previous())
                    tok = tok->previous();
                else
                    tok->str(";");

                Token::eraseTokens(tok, elseTag);
            } else {
                // convert "if( true ) {aaa;}" => "{aaa;}"
                if (tok->previous())
                    tok = tok->previous();
                else
                    tok->str(";");

                tok->deleteNext(4);
            }

            ret = true;
        }
    }

    return ret;
}

void Tokenizer::removeRedundantFor()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            unsigned int indentlevel = 0;
            for (const Token *tok2 = tok->linkAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}") {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }

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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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


bool Tokenizer::simplifyIfAddBraces()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(" || tok->str() == "[" ||
            (tok->str() == "{" && tok->previous() && tok->previous()->str() == "=")) {
            tok = tok->link();
            continue;
        }

        if (Token::Match(tok, "if|for|while|BOOST_FOREACH (")) {

            if (tok->strAt(2) == ")") {
                //no arguments inside round braces, abort
                syntaxError(tok);
                return false;
            }
            // don't add "{}" around ";" in "do {} while();" (#609)
            const Token *prev = tok->previous();
            if (prev && prev->str() == "}" && tok->str() == "while") {
                prev = prev->link()->previous();
                if (prev && prev->str() == "do")
                    continue;
            }

            // Goto the ending ')'
            tok = tok->next()->link();

            // there's already '{' after ')', don't bother
            if (tok->next() && tok->next()->str() == "{")
                continue;
        }

        else if (tok->str() == "else") {
            // An else followed by an if or brace don't need to be processed further
            if (Token::Match(tok, "else if|{"))
                continue;
        }

        else {
            continue;
        }

        // If there is no code after the 'if()' or 'else', abort
        if (!tok->next()) {
            syntaxError(tok);
            return false;
        }

        // insert open brace..
        tok->insertToken("{");
        tok = tok->next();
        Token *tempToken = tok;

        bool innerIf = (tempToken->next() && tempToken->next()->str() == "if");

        if (Token::simpleMatch(tempToken->next(), "do {"))
            tempToken = tempToken->linkAt(2);

        // insert close brace..
        // In most cases it would work to just search for the next ';' and insert a closing brace after it.
        // But here are special cases..
        // * if (cond) for (;;) break;
        // * if (cond1) if (cond2) { }
        // * if (cond1) if (cond2) ; else ;
        while (NULL != (tempToken = tempToken->next())) {
            if (tempToken->str() == "{") {
                if (tempToken->previous()->str() == "=") {
                    tempToken = tempToken->link();
                    continue;
                }

                if (tempToken->previous()->str() == "else") {
                    if (innerIf)
                        tempToken = tempToken->link();
                    else
                        tempToken = tempToken->tokAt(-2);
                    break;
                }
                tempToken = tempToken->link();
                if (!tempToken->next())
                    break;
                if (Token::simpleMatch(tempToken, "} else") && !Token::Match(tempToken->tokAt(2), "if|{"))
                    innerIf = false;
                else if (tempToken->next()->isName() && tempToken->next()->str() != "else")
                    break;
                continue;
            }

            if (tempToken->str() == "(" || tempToken->str() == "[") {
                tempToken = tempToken->link();
                continue;
            }

            if (tempToken->str() == "}") {
                // insert closing brace before this token
                tempToken = tempToken->previous();
                break;
            }

            if (tempToken->str() == ";") {
                if (!innerIf)
                    break;

                if (Token::simpleMatch(tempToken, "; else")) {
                    if (tempToken->strAt(2) != "if")
                        innerIf = false;
                } else
                    break;
            }
        }

        if (tempToken) {
            tempToken->insertToken("}");
            Token::createMutualLinks(tok, tempToken->next());

            // move '}' in the same line as 'else' if there's it after the new token,
            // except for '}' which is after '{ ; }'
            tempToken = tempToken->next();
            if (!Token::simpleMatch(tempToken->link(), "{ ; }") && tempToken->next() && tempToken->next()->str() == "else" &&
                tempToken->next()->linenr() != tempToken->linenr())
                tempToken->linenr(tempToken->next()->linenr());
        } else {
            // Can't insert matching "}" so give up.  This is fatal because it
            // causes unbalanced braces.
            syntaxError(tok);
            return false;
        }
    }
    return true;
}

void Tokenizer::simplifyDoWhileAddBraces()
{
    //start from the last token and proceed backwards
    Token *last = _tokens;
    while (last && last->next())
        last = last->next();

    for (Token *tok = last; tok; tok = tok->previous()) {
        // fix for #988
        if (tok->str() == ")" || tok->str() == "]" ||
            (tok->str() == "}" && tok->link()->previous() &&
             tok->link()->previous()->str() == "="))
            tok = tok->link();

        if (!Token::Match(tok, "do !!{"))
            continue;

        Token *tok1 = tok;  // token with "do"
        Token *tok2 = NULL; // token with "while"

        for (Token *tok3 = tok->next(); tok3; tok3 = tok3->next()) {
            if (tok3->str() == "(" || tok3->str() == "[" || tok3->str() == "{") {
                tok3 = tok3->link();
            } else if (tok3->str() == "while") {
                tok2 = tok3;
                break;
            } else if (Token::simpleMatch(tok3, "do {")) {
                // Skip 'do { } while' inside the current "do"
                tok3 = tok3->next()->link();
                if (Token::simpleMatch(tok3->next(), "while"))
                    tok3 = tok3->next();
            }
        }

        if (tok2) {
            // insert "{" after "do"
            tok1->insertToken("{");

            // insert "}" before "while"
            tok2->previous()->insertToken("}");

            Token::createMutualLinks(tok1->next(), tok2->previous());
        }
    }
}

void Tokenizer::simplifyCompoundAssignment()
{
    // Simplify compound assignments:
    // "a+=b" => "a = a + b"
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] (") || Token::Match(tok, "[;{}:] *| (| %var%")) {
            if (tok->str() == ":") {
                if (tok->strAt(-2) != "case")
                    continue;
            }

            // backup current token..
            Token * const tok1 = tok;

            if (tok->next() && tok->next()->str() == "*")
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
                // Enclose the rhs in parantheses..
                if (!Token::Match(tok->tokAt(2), "[;)]")) {
                    // Only enclose rhs in parantheses if there is some operator
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

                        someOperator |= (tok2->isOp() || (tok2->str() == "?"));
                    }
                }

                // simplify the compound assignment..
                tok->str("=");
                tok->insertToken(op);

                std::stack<Token *> tokend;
                for (Token *tok2 = tok->previous(); tok2 && tok2 != tok1; tok2 = tok2->previous()) {
                    // Don't duplicate ++ and --. Put preincrement in lhs. Put
                    // postincrement in rhs.
                    if (tok2->str() == "++" || tok2->str() == "--") {
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
    int parlevel = 0;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
            --parlevel;
        else if (parlevel == 0 && (Token::Match(tok, ";|{|} *| %any% = %any% ? %any% : %any% ;") ||
                                   Token::Match(tok, ";|{|} return %any% ? %any% : %any% ;"))) {
            std::string var(tok->next()->str());
            bool isPointer = false;
            bool isReturn = false;
            int offset = 0;
            if (tok->next()->str() == "*") {
                tok = tok->next();
                var += " " + tok->next()->str();
                isPointer = true;
            } else if (tok->next()->str() == "return") {
                isReturn = true;
                offset = -1;
            }

            const std::string condition(tok->strAt(3 + offset));
            const std::string value1(tok->strAt(5 + offset));
            const std::string value2(tok->strAt(7 + offset));

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
                str = "if ( condition ) { var = value1 ; } else { var = value2 ; }";

            std::string::size_type pos1 = 0;
            while (pos1 != std::string::npos) {
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
            }
        }
    }
}

bool Tokenizer::simplifyConditions()
{
    bool ret = false;

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "! %num%") || Token::Match(tok, "! %bool%")) {
            if (tok->next()->str() == "0" || tok->next()->str() == "false")
                tok->str("true");
            else
                tok->str("false");

            tok->deleteNext();
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
            if (Token::Match(tok->next(), "%num%")) {
                // Compare numbers

                if (cmp == "==" || cmp == "!=") {
                    const std::string op1(tok->next()->str());
                    const std::string op2(tok->strAt(3));

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

bool Tokenizer::simplifyQuestionMark()
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() != "?")
            continue;

        if (!tok->tokAt(-2))
            continue;

        if (!Token::Match(tok->tokAt(-2), "[=,(]"))
            continue;

        if (!Token::Match(tok->previous(), "%bool%") &&
            !Token::Match(tok->previous(), "%num%"))
            continue;

        // Find the ":" token..
        Token *semicolon = 0;
        {
            unsigned int parlevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")") {
                    if (parlevel == 0)
                        break;
                    --parlevel;
                } else if (parlevel == 0 && tok2->str() == ":") {
                    semicolon = tok2;
                    break;
                }
            }
        }
        if (!semicolon || !semicolon->next())
            continue;

        if (tok->previous()->str() == "false" ||
            tok->previous()->str() == "0") {
            // Use code after semicolon, remove code before it.
            semicolon = semicolon->next();
            tok = tok->tokAt(-2);
            Token::eraseTokens(tok, semicolon);

            tok = tok->next();
            ret = true;
        }

        // The condition is true. Delete the operator after the ":"..
        else {
            const Token *end = 0;

            // check the operator after the :
            if (Token::simpleMatch(semicolon, ": (")) {
                end = semicolon->next()->link();
                if (!Token::Match(end, ") !!."))
                    continue;
            }

            // delete the condition token and the "?"
            tok = tok->tokAt(-2);
            tok->deleteNext(2);

            // delete operator after the :
            if (end) {
                Token::eraseTokens(semicolon->previous(), end->next());
                continue;
            }

            int ind = 0;
            for (const Token *endTok = semicolon; endTok; endTok = endTok->next()) {
                if (endTok->str() == ";") {
                    //we can remove the semicolon if after it there's at least another token
                    if (endTok->next())
                        endTok = endTok->next();
                    Token::eraseTokens(semicolon->previous(), endTok);
                    ret = true;
                    break;
                }

                else if (Token::Match(endTok, "[({[]")) {
                    ++ind;
                }

                else if (Token::Match(endTok, "[)}]]")) {
                    --ind;
                    if (ind < 0) {
                        Token::eraseTokens(semicolon->previous(), endTok);
                        ret = true;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void Tokenizer::simplifyCasts()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // #2897 : don't remove cast in such cases:
        // *((char *)a + 1) = 0;
        if (!tok->isName() && Token::simpleMatch(tok->next(), "* (")) {
            tok = tok->linkAt(2);
            continue;
        }
        while (Token::Match(tok->next(), "( %type% *| *| *| ) *|&| %var%") ||
               Token::Match(tok->next(), "( %type% %type% *| *| *| ) *|&| %var%") ||
               (!tok->isName() && (Token::Match(tok->next(), "( %type% * *| *| ) (") ||
                                   Token::Match(tok->next(), "( %type% %type% * *| *| ) (")))) {
            if (tok->isName() && tok->str() != "return")
                break;

            if (Token::simpleMatch(tok->previous(), "operator"))
                break;

            // Remove cast..
            Token::eraseTokens(tok, tok->next()->link()->next());

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
            Token *tok2 = tok->next();
            unsigned int level = 0;
            while (tok2) {
                if (tok2->str() == "<")
                    ++level;
                else if (tok2->str() == ">") {
                    --level;
                    if (level == 0)
                        break;
                }
                tok2 = tok2->next();
            }

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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "{" || tok->str() == "[" || tok->str() == "(") {
            tok = tok->link();
        }

        // Find the function e.g. foo( x ) or foo( x, y )
        else if (Token::Match(tok, "%var% ( %var% [,)]")) {
            // We have found old style function, now we need to change it

            // First step: Get list of argument names in parenthesis
            std::map<std::string, Token *> argumentNames;
            bool bailOut = false;
            Token * tokparam = NULL;

            //floating token used to check for parameters
            Token *tok1 = tok;

            //goto '('
            tok = tok->next();

            while (NULL != (tok1 = tok1->tokAt(2))) {
                if (!Token::Match(tok1, "%var% [,)]")) {
                    bailOut = true;
                    break;
                }

                //same parameters: take note of the parameter
                if (argumentNames.find(tok1->str()) != argumentNames.end())
                    tokparam = tok1;
                else
                    argumentNames[tok1->str()] = tok1;

                if (tok1->next()->str() == ")") {
                    tok1 = tok1->tokAt(2);
                    //expect at least a type name after round brace..
                    if (!tok1 || !tok1->isName())
                        bailOut = true;
                    break;
                }
            }

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            //there should be the sequence '; {' after the round parenthesis
            if (!Token::findsimplematch(tok1, "; {")) {
                tok = tok->link();
                continue;
            }

            // Last step: check out if the declarations between ')' and '{' match the parameters list
            std::map<std::string, Token *> argumentNames2;

            while (tok1 && tok1->str() != "{") {
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

            if (bailOut) {
                tok = tok->link();
                continue;
            }

            //the two containers should hold the same size..
            if (argumentNames.size() != argumentNames2.size()) {
                tok = tok->link();
                //error if the second container is not empty
                if (!argumentNames2.empty()) {
                    syntaxError(tok);
                    return false;
                }
                continue;
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

                //replace the parameter name in the parenthesis with all the declaration
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

    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    int indentlevel = 0;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        else if (indentlevel == 0 && Token::Match(tok, "%var% ( ) { return %num% ; }") && tok->str() != ")") {
            const std::string pattern("(|[|=|%op% " + tok->str() + " ( ) ;|]|)|%op%");
            for (Token *tok2 = _tokens; tok2; tok2 = tok2->next()) {
                if (Token::Match(tok2, pattern.c_str())) {
                    tok2 = tok2->next();
                    tok2->str(tok->strAt(5));
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
    // Split up variable declarations..
    // "int a=4;" => "int a; a=4;"
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "= {")) {
            tok = tok->next()->link();
        }

        if (only_k_r_fpar) {
            if (tok->str() == "(" || tok->str() == "[" || tok->str() == "{") {
                tok = tok->link();
                if (tok->next() && Token::Match(tok, ") !!{"))
                    tok = tok->next();
                else
                    continue;
            } else
                continue;
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

        while (Token::Match(tok2, "%type% %type% *| *| %var%")) {
            if (tok2->str() == "const")
                isconst = true;

            else if (tok2->str() == "static")
                isstatic = true;

            tok2 = tok2->next();
            ++typelen;
        }

        // Don't split up const declaration..
        if (isconst && Token::Match(tok2, "%type% %var% ="))
            continue;

        // strange looking variable declaration => don't split up.
        if (Token::Match(tok2, "%type% *| %var% , %type% *| %var%"))
            continue;

        // check for qualification..
        if (Token::Match(tok2,  ":: %type%")) {
            ++typelen;
            tok2 = tok2->next();
        }

        if (Token::Match(tok2, "%type% :: %type%")) {
            while (tok2 && Token::Match(tok2, "%type% ::")) {
                typelen += 2;
                tok2 = tok2->tokAt(2);
            }
        }

        if (Token::Match(tok2, "%type% *| %var% ,|=")) {
            const bool isPointer = (tok2->next()->str() == "*");
            const Token *varName = tok2->tokAt((isPointer ? 2 : 1));

            if (varName->str() != "operator") {
                tok2 = varName->next(); // The ',' or '=' token

                if (isstatic && tok2->str() == "=") {
                    if (Token::Match(tok2->next(), "%num% ,"))
                        tok2 = tok2->tokAt(2);
                    else
                        tok2 = NULL;
                }
            } else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% * * %var% ,|=")) {
            if (tok2->strAt(3) != "operator")
                tok2 = tok2->tokAt(4);    // The ',' token
            else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% * const %var% ,|=")) {
            if (tok2->strAt(3) != "operator") {
                tok2 = tok2->tokAt(4);    // The ',' token
            } else {
                tok2 = NULL;
            }
        }

        else if (Token::Match(tok2, "%type% %var% [ %any% ] ,|=|[")) {
            tok2 = tok2->tokAt(2);
            if (tok2->next()->isName() || tok2->next()->isNumber()) {
                tok2 = tok2->link()->next();    // The ',' token
                while (Token::Match(tok2, "[ %any% ]") &&
                       (tok2->next()->isName() || tok2->next()->isNumber()))
                    tok2 = tok2->link()->next();
                if (!Token::Match(tok2, "=|,")) {
                    tok2 = NULL;
                }

                if (tok2 && tok2->str() == "=") {
                    while (tok2 && tok2->str() != "," && tok2->str() != ";") {
                        if (tok2->str() == "{")
                            tok2 = tok2->link();
                        tok2 = tok2->next();
                    }
                    if (tok2 && tok2->str() == ";")
                        tok2 = NULL;
                }
            } else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% * %var% [ %any% ] ,")) {
            tok2 = tok2->tokAt(3);
            if (tok2->next()->isName() || tok2->next()->isNumber())
                tok2 = tok2->link()->next();    // The ',' token
            else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% <")) {
            typelen += 2;
            tok2 = tok2->tokAt(2);
            size_t indentlevel = 1;

            for (Token *tok3 = tok2; tok3; tok3 = tok3->next()) {
                ++typelen;

                if (tok3->str() == "<") {
                    ++indentlevel;
                } else if (tok3->str() == ">") {
                    --indentlevel;
                    if (indentlevel == 0) {
                        tok2 = tok3->next();
                        break;
                    }
                } else if (tok3->str() == ";") {
                    break;
                }
            }

            if (!tok2) // syntax error
                break;

            if (Token::Match(tok2, ":: %type%")) {
                typelen += 2;
                tok2 = tok2->tokAt(2);
            }

            if (!tok2) // syntax error
                break;

            if (tok2->str() == "*") {
                tok2 = tok2->next();
            }

            if (Token::Match(tok2, "%var% ,|=")) {
                tok2 = tok2->next();    // The ',' token
                --typelen;
            } else {
                tok2 = NULL;
            }
        } else {
            tok2 = NULL;
        }

        if (!tok2)
            continue;

        if (tok2->str() == ",") {
            tok2->str(";");
            insertTokens(tok2, type0, typelen);
            std::stack<Token *> link1;
            std::stack<Token *> link2;
            while (((typelen--) > 0) && (NULL != (tok2 = tok2->next()))) {
                if (tok2->str() == "(")
                    link1.push(tok2);
                else if (tok2->str() == ")" && !link1.empty()) {
                    Token::createMutualLinks(tok2, link1.top());
                    link1.pop();
                }

                else if (tok2->str() == "[")
                    link2.push(tok2);
                else if (tok2->str() == "]" && !link2.empty()) {
                    Token::createMutualLinks(tok2, link2.top());
                    link2.pop();
                }
            }
        }

        else {
            Token *eq = tok2;

            unsigned int level = 0;
            while (tok2) {
                if (Token::Match(tok2, "[{(]"))
                    tok2 = tok2->link();

                else if (tok2->str() == "<") {
                    if (tok2->previous()->isName() && !tok2->previous()->varId())
                        ++level;
                }

                else if (level > 0 && tok2->str() == ">")
                    --level;

                else if (level == 0 && strchr(";,", tok2->str()[0])) {
                    // "type var ="   =>   "type var; var ="
                    Token *VarTok = type0->tokAt((int)typelen);
                    while (Token::Match(VarTok, "*|&|const"))
                        VarTok = VarTok->next();
                    insertTokens(eq, VarTok, 2);
                    eq->str(";");

                    // "= x, "   =>   "= x; type "
                    if (tok2->str() == ",") {
                        tok2->str(";");
                        insertTokens(tok2, type0, typelen);
                    }
                    break;
                }

                tok2 = tok2->next();
            }
        }
    }
}

void Tokenizer::simplifyPlatformTypes()
{
    enum { isLongLong, isLong, isInt } type;

    /** @todo This assumes a flat address space. Not true for segmented address space (FAR *). */
    if (_settings->sizeof_size_t == 8)
        type = isLongLong;
    else if (_settings->sizeof_size_t == 4 && _settings->sizeof_long == 4)
        type = isLong;
    else if (_settings->sizeof_size_t == 4)
        type = isInt;
    else
        return;

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "std :: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t")) {
            tok->deleteNext();
            tok->deleteThis();
        } else if (Token::Match(tok, ":: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t")) {
            tok->deleteThis();
        }

        if (Token::Match(tok, "size_t|uintptr_t")) {
            tok->str("unsigned");

            switch (type) {
            case isLongLong:
                tok->insertToken("long");
                tok->insertToken("long");
                break;
            case isLong :
                tok->insertToken("long");
                break;
            case isInt:
                tok->insertToken("int");
                break;
            }
        } else if (Token::Match(tok, "ssize_t|ptrdiff_t|intptr_t")) {
            switch (type) {
            case isLongLong:
                tok->str("long");
                tok->insertToken("long");
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
        for (Token *tok = _tokens; tok; tok = tok->next()) {
            if (Token::Match(tok, "BOOL|INT|INT32|HFILE|LONG32"))
                tok->str("int");
            else if (Token::Match(tok, "BOOLEAN|BYTE|UCHAR")) {
                tok->str("unsigned");
                tok->insertToken("char");
            } else if (tok->str() == "CHAR")
                tok->str("char");
            else if (Token::Match(tok, "DWORD|ULONG|COLORREF|LCID|LCTYPE|LGRPID")) {
                tok->str("unsigned");
                tok->insertToken("long");
            } else if (Token::Match(tok, "DWORD_PTR|ULONG_PTR|SIZE_T")) {
                tok->str("unsigned");
                tok->insertToken("long");
                if (_settings->platformType == Settings::Win64)
                    tok->insertToken("long");
            } else if (tok->str() == "FLOAT")
                tok->str("float");
            else if (Token::Match(tok, "HRESULT|LONG"))
                tok->str("long");
            else if (Token::Match(tok, "INT64|LONG64")) {
                tok->str("long");
                tok->insertToken("long");
            } else if (Token::Match(tok, "LONG_PTR|LPARAM|LRESULT|SSIZE_T")) {
                tok->str("long");
                if (_settings->platformType == Settings::Win64)
                    tok->insertToken("long");
            } else if (Token::Match(tok, "LPBOOL|PBOOL")) {
                tok->str("int");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPBYTE|PBOOLEAN|PBYTE|PUCHAR")) {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("char");
            } else if (Token::Match(tok, "LPCSTR|PCSTR")) {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("char");
            } else if (tok->str() == "LPCVOID") {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("void");
            } else if (Token::Match(tok, "LPDWORD|LPCOLORREF|PDWORD|PULONG")) {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("long");
            } else if (Token::Match(tok, "LPINT|PINT")) {
                tok->str("int");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPLONG|PLONG")) {
                tok->str("long");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPSTR|PSTR|PCHAR")) {
                tok->str("char");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPVOID|PVOID|HANDLE|HBITMAP|HBRUSH|HCOLORSPACE|HCURSOR|HDC|HFONT|HGDIOBJ|HGLOBAL|HICON|HINSTANCE|HKEY|HLOCAL|HMENU|HMETAFILE|HMODULE|HPALETTE|HPEN|HRGN|HRSRC|HWND|SERVICE_STATUS_HANDLE|SC_LOCK|SC_HANDLE|HACCEL|HCONV|HCONVLIST|HDDEDATA|HDESK|HDROP|HDWP|HENHMETAFILE|HHOOK|HKL|HMONITOR|HSZ|HWINSTA")) {
                tok->str("void");
                tok->insertToken("*");
            } else if ((tok->str() == "PHANDLE")) {
                tok->str("void");
                tok->insertToken("*");
                tok->insertToken("*");
            } else if (Token::Match(tok, "LPWORD|PWORD|PWSTR|PWCHAR|PUSHORT")) {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("short");
            } else if (tok->str() == "SHORT")
                tok->str("short");
            else if (Token::Match(tok, "UINT|MMRESULT|SOCKET|ULONG32|UINT32|DWORD32")) {
                tok->str("unsigned");
                tok->insertToken("int");
            } else if (Token::Match(tok, "UINT_PTR|WPARAM")) {
                tok->str("unsigned");
                if (_settings->platformType == Settings::Win64) {
                    tok->insertToken("long");
                    tok->insertToken("long");
                } else {
                    tok->insertToken("int");
                }
            } else if (Token::Match(tok, "USHORT|WORD|WCHAR|ATOM|wchar_t|LANGID")) {
                tok->str("unsigned");
                tok->insertToken("short");
            } else if (tok->str() == "VOID")
                tok->str("void");
            else if (tok->str() == "TCHAR") {
                if (_settings->platformType == Settings::Win32A)
                    tok->str("char");
                else {
                    tok->str("unsigned");
                    tok->insertToken("short");
                }
            } else if (tok->str() == "TBYTE") {
                tok->str("unsigned");
                if (_settings->platformType == Settings::Win32A)
                    tok->insertToken("short");
                else
                    tok->insertToken("char");
            } else if (Token::Match(tok, "PTSTR|LPTSTR")) {
                if (_settings->platformType == Settings::Win32A) {
                    tok->str("char");
                    tok->insertToken("*");
                } else {
                    tok->str("unsigned");
                    tok->insertToken("*");
                    tok->insertToken("short");
                }
            } else if (Token::Match(tok, "PCTSTR|LPCTSTR")) {
                tok->str("const");
                if (_settings->platformType == Settings::Win32A) {
                    tok->insertToken("*");
                    tok->insertToken("char");
                } else {
                    tok->insertToken("*");
                    tok->insertToken("short");
                    tok->insertToken("unsigned");
                }
            } else if (Token::Match(tok, "ULONG64|DWORD64")) {
                tok->str("unsigned");
                tok->insertToken("long");
            } else if (tok->str() == "HALF_PTR") {
                if (_settings->platformType == Settings::Win64)
                    tok->str("int");
                else
                    tok->str("short");
            } else if (tok->str() == "INT_PTR") {
                if (_settings->platformType == Settings::Win64) {
                    tok->str("long");
                    tok->insertToken("long");
                } else {
                    tok->str("int");
                }
            } else if (tok->str() == "LPCWSTR") {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("short");
                tok->insertToken("unsigned");
            }
        }
    }
}

void Tokenizer::simplifyStdType()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        // long unsigned => unsigned long
        if (Token::Match(tok, "char|short|int|long|__int8|__int16|__int32|__int64 unsigned|signed")) {
            std::string temp = tok->str();
            tok->str(tok->next()->str());
            tok->next()->str(temp);
        }

        if (!Token::Match(tok, "unsigned|signed|char|short|int|long|__int8|__int16|__int32|__int64"))
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

        if (Token::simpleMatch(tok, "__int8"))
            tok->str("char");
        else if (Token::simpleMatch(tok, "__int16"))
            tok->str("short");
        else if (Token::simpleMatch(tok, "__int32"))
            tok->str("int");
        else if (Token::simpleMatch(tok, "__int64")) {
            tok->str("long");
            tok->isLong(true);
        } else if (Token::simpleMatch(tok, "long")) {
            if (Token::simpleMatch(tok->next(), "long")) {
                tok->isLong(true);
                tok->deleteNext();
            }

            if (Token::simpleMatch(tok->next(), "int"))
                tok->deleteNext();
            else if (Token::simpleMatch(tok->next(), "double")) {
                tok->str("double");
                tok->isLong(true);
                tok->deleteNext();
            }
        } else if (Token::simpleMatch(tok, "short")) {
            if (Token::simpleMatch(tok->next(), "int"))
                tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyIfAssign()
{
    // See also simplifyFunctionAssign

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!Token::Match(tok->next(), "if|while ( !| (| %var% =") &&
            !Token::Match(tok->next(), "if|while ( !| (| %var% . %var% ="))
            continue;

        // simplifying a "while" condition ?
        const bool iswhile(tok->next()->str() == "while");

        // delete the "if"
        tok->deleteNext();

        // Remember if there is a "!" or not. And delete it if there are.
        const bool isNot(tok->strAt(2) == "!");
        if (isNot)
            tok->next()->deleteNext();

        // Delete parenthesis.. and remember how many there are with
        // their links.
        std::stack<Token *> braces;
        while (tok->next()->str() == "(") {
            braces.push(tok->next()->link());
            tok->deleteNext();
        }

        // Skip the "%var% = ..."
        Token *tok2;
        unsigned int indentlevel = 0;
        for (tok2 = tok->next(); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "(")
                ++indentlevel;
            else if (tok2->str() == ")") {
                if (indentlevel == 0)
                    break;
                --indentlevel;
            }
        }

        // Insert "; if|while ( .."
        tok2 = tok2->previous();
        if (Token::simpleMatch(tok->tokAt(2), ".")) {
            tok2->insertToken(tok->strAt(3));
            tok2->insertToken(tok->strAt(2));
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
        tok2->insertToken(";");

        // If it's a while loop.. insert the assignment in the loop
        if (iswhile) {
            indentlevel = 0;
            Token *tok3 = tok2;
            for (tok3 = tok2; tok3; tok3 = tok3->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%var% = %var% = %num% ;") ||
            Token::Match(tok, "%var% = %var% = %var% ;")) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
                tok->deleteNext();
                tok->next()->str(tok->str());
                tok->str("!");
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
            else if (Token::simpleMatch(tok->link()->previous(), "(")) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            }

            else if (Token::Match(tok, "%var% .|:: %var% != 0")) {
                tok = tok->tokAt(2);
                deleteFrom = tok;
                tok->isPointerCompare(true);
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!tok->isName() || (tok->previous() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ;") ||
            Token::Match(tok, "%type% *| %var% ( %type% (")) {
            tok = initVar(tok);
        } else if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ,")) {
            Token *tok1 = tok;
            while (tok1->str() != ",")
                tok1 = tok1->next();
            tok1->str(";");
            Token *tok2 = tok;
            if (Token::Match(tok2, "class|struct|union")) {
                tok1->insertToken(tok2->str());
                tok1 = tok1->next();
                tok2 = tok2->next();
            }
            tok1->insertToken(tok2->str());
            tok1 = tok1->next();
            tok2 = tok2->next();
            if (tok2->str() == "*") {
                tok1->insertToken("*");
            }
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
        for (Token *tok = _tokens; tok; tok = tok->next()) {
            if (tok->isName() && Token::Match(tok, "static| const| static| %type% const| %var% = %any% ;")) {
                bool isconst = false;
                for (const Token *tok2 = tok; tok2->str() != "="; tok2 = tok2->next()) {
                    if (tok2->str() == "const") {
                        isconst = true;
                        break;
                    }
                }
                if (!isconst)
                    continue;

                Token *tok1 = tok;

                // start of statement
                if (tok != _tokens && !Token::Match(tok->previous(),";|{|}|private:|protected:|public:"))
                    continue;
                // skip "const" and "static"
                while (tok->str() == "const" || tok->str() == "static")
                    tok = tok->next();
                // pod type
                if (!tok->isStandardType())
                    continue;

                const Token * const vartok = (tok->next() && tok->next()->str() == "const") ? tok->tokAt(2) : tok->next();
                const Token * const valuetok = vartok->tokAt(2);
                if (valuetok->isNumber() || Token::Match(valuetok, "%str% ;")) {
                    constantValues[vartok->varId()] = valuetok->str();

                    // remove statement
                    while (tok1->str() != ";")
                        tok1->deleteThis();
                    tok1->deleteThis();
                    tok = tok1;
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

            else if (tok2->previous()->str() != "*" &&
                     (Token::Match(tok2, "%var% = %num% ;") ||
                      Token::Match(tok2, "%var% = %str% ;") ||
                      (Token::Match(tok2, "%var% = %any% ;") && tok2->strAt(2)[0] == '\'') ||
                      Token::Match(tok2, "%var% [ ] = %str% ;") ||
                      Token::Match(tok2, "%var% [ %num% ] = %str% ;") ||
                      Token::Match(tok2, "%var% = %bool% ;") ||
                      Token::Match(tok2, "%var% = %var% ;") ||
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

                if (tok2->str() == tok2->strAt(2))
                    continue;

                const Token * const valueToken = tok2->tokAt(2);

                std::string value;
                unsigned int valueVarId = 0;

                Token *tok3 = NULL;
                bool valueIsPointer = false;

                if (!simplifyKnownVariablesGetData(varid, &tok2, &tok3, value, valueVarId, valueIsPointer, floatvars.find(tok2->varId()) != floatvars.end()))
                    continue;

                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
            }

            else if (Token::Match(tok2, "strcpy ( %var% , %str% ) ;")) {
                const unsigned int varid(tok2->tokAt(2)->varId());
                if (varid == 0)
                    continue;
                const std::string structname("");
                const Token * const valueToken = tok2->tokAt(4);
                std::string value(valueToken->str());
                const unsigned int valueVarId(0);
                const bool valueIsPointer(false);
                Token *tok3 = tok2;
                for (int i = 0; i < 6; ++i)
                    tok3 = tok3->next();
                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
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
    Token *tok3 = *_tok3;

    if (Token::Match(tok2->tokAt(-2), "for ( %varid% = %num% ; %varid% <|<= %num% ; ++| %varid% ++| ) {", varid)) {
        // is there a "break" in the for loop?
        bool hasbreak = false;
        unsigned int indentlevel4 = 0;   // indentlevel for tok4
        for (const Token *tok4 = tok2->previous()->link(); tok4; tok4 = tok4->next()) {
            if (tok4->str() == "{")
                ++indentlevel4;
            else if (tok4->str() == "}") {
                if (indentlevel4 <= 1)
                    break;
                --indentlevel4;
            } else if (tok4->str() == "break") {
                hasbreak = true;
                break;
            }
        }
        if (hasbreak)
            return false;

        // no break => the value of the counter value is known after the for loop..
        const std::string compareop = tok2->strAt(5);
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
        if (Token::simpleMatch(tok2->next(), "[")) {
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

        if (Token::simpleMatch(tok2->next(), "= &"))
            tok2 = tok2->tokAt(3);

        tok3 = tok2->next();
    }
    *_tok2 = tok2;
    *_tok3 = tok3;
    return true;
}

bool Tokenizer::simplifyKnownVariablesSimplify(Token **tok2, Token *tok3, unsigned int varid, const std::string &structname, std::string &value, unsigned int valueVarId, bool valueIsPointer, const Token * const valueToken, int indentlevel)
{
    const bool pointeralias(valueToken->isName() || Token::Match(valueToken, "& %var% ["));

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

        // Stop if return or break is found ..
        if (tok3->str() == "break")
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
            !Token::Match(tok3->link()->previous(), "if|for|while|switch"))
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
                !Token::simpleMatch(tok3->previous()->link()->previous(), "if"))
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
            ret = true;
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
                if (Token::simpleMatch(tok3->next(), "="))
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

                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = tok3->linenr();
                loc.setfile(file(tok3));
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "simplifyKnownVariables: bailing out (variable="+tok3->str()+", value="+value+")",
                                                       "debug",
                                                       false);

                if (_errorLogger)
                    _errorLogger->reportErr(errmsg);
                else
                    Check::reportError(errmsg);
            }

            break;
        }

        // Using the variable in condition..
        if (Token::Match(tok3->previous(), ("if ( " + structname + " %varid% ==|!=|<|<=|>|>=|)").c_str(), varid) ||
            Token::Match(tok3, ("( " + structname + " %varid% ==|!=|<|<=|>|>=").c_str(), varid) ||
            Token::Match(tok3, ("!|==|!=|<|<=|>|>= " + structname + " %varid% ==|!=|<|<=|>|>=|)|;").c_str(), varid) ||
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
            const char * const functionName[] = {
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
            const char * const functionName[] = {
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
        if (Token::Match(tok3, ("[(,] " + structname + " %varid% [|%op%").c_str(), varid)) {
            if (!structname.empty()) {
                tok3->deleteNext(2);
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Variable is used in calculation..
        if (((tok3->previous()->varId() > 0) && Token::Match(tok3, ("& " + structname + " %varid%").c_str(), varid)) ||
            Token::Match(tok3, ("[=+-*/%^|[] " + structname + " %varid% [=?+-*/%^|;])]").c_str(), varid) ||
            Token::Match(tok3, ("[(=+-*/%^|[] " + structname + " %varid% <<|>>").c_str(), varid) ||
            Token::Match(tok3, ("<<|>> " + structname + " %varid% %op%|;|]|)").c_str(), varid) ||
            Token::Match(tok3->previous(), ("[=+-*/%^|[] ( " + structname + " %varid% !!=").c_str(), varid)) {
            if (value[0] == '\"')
                break;
            if (!structname.empty()) {
                tok3->deleteNext(2);
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            if (tok3->previous()->str() == "*" && valueIsPointer) {
                tok3 = tok3->previous();
                tok3->deleteThis();
            }
            ret = true;
        }

        if (Token::simpleMatch(tok3, "= {")) {
            unsigned int indentlevel4 = 0;
            for (const Token *tok4 = tok3; tok4; tok4 = tok4->next()) {
                if (tok4->str() == "{")
                    ++indentlevel4;
                else if (tok4->str() == "}") {
                    if (indentlevel4 <= 1)
                        break;
                    --indentlevel4;
                }
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "else if"))
            continue;
        int indent = 0;
        for (Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next()) {
            if (Token::Match(tok2, "(|{"))
                ++indent;
            else if (Token::Match(tok2, ")|}"))
                --indent;

            if (indent == 0 && Token::Match(tok2, "}|;")) {
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


bool Tokenizer::simplifyRedundantParenthesis()
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            // parenthesis
            tok->deleteNext();
            tok->link()->tokAt(-2)->deleteNext();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[,;{}(] ( %var% (") &&
               tok->link()->previous() == tok->linkAt(2)) {
            // We have "( func ( *something* ))", remove the outer
            // parenthesis
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{] ( delete %var% ) ;")) {
            // We have "( delete var )", remove the outer
            // parenthesis
            tok->tokAt(3)->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{] ( delete [ ] %var% ) ;")) {
            // We have "( delete [] var )", remove the outer
            // parenthesis
            tok->tokAt(5)->deleteThis();
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

        if (Token::Match(tok->previous(), "[(!*;{}] ( %var% )") && tok->next()->varId() != 0) {
            // We have "( var )", remove the parenthesis
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
            continue;
        }

        if (Token::Match(tok->previous(), "[(,!] ( %var% . %var% )")) {
            // We have "( var . var )", remove the parenthesis
            tok->deleteThis();
            tok = tok->tokAt(2);
            tok->deleteNext();
            ret = true;
            continue;
        }

        if (Token::Match(tok->previous(), "(|[|,| ( %var% %op% %var% ) ,|]|)") ||
            Token::Match(tok->previous(), "(|[|,| ( %var% %op% %num% ) ,|]|)")) {
            // We have "( var %op% var )", remove the parenthesis
            tok->deleteThis();
            tok = tok->tokAt(2);
            tok->deleteNext();
            ret = true;
            continue;
        }

        if (Token::Match(tok, "( ( %bool% )") ||
            Token::Match(tok, "( ( %num% )")) {
            tok->deleteNext();
            tok->next()->deleteNext();
            ret = true;
        }

        if (Token::simpleMatch(tok->previous(), ", (") &&
            Token::simpleMatch(tok->link(), ") =")) {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }
    }
    return ret;
}

void Tokenizer::simplifyReference()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

                    tok2->deleteNext(6+(tok->strAt(6)==")"));
                }
            }
        }
    }
}

bool Tokenizer::simplifyCalculations()
{
    return TemplateSimplifier::simplifyCalculations(_tokens);
}




void Tokenizer::simplifyGoto()
{
    std::list<Token *> gotos;
    unsigned int indentlevel = 0;
    unsigned int indentspecial = 0;
    Token *beginfunction = 0;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            if (!indentlevel) {
                if (indentspecial)
                    --indentspecial;
                else
                    break;  // break out - it seems the code is wrong
            } else {
                --indentlevel;
                if (!indentlevel) {
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
                        }
                        if (!roundbraces && tok2->str() == "{") {
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
                        if (ret2 && tok2->str() == ";") {
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
            while (tok) {
                if (tok->str() == "{")
                    tok = tok->link();
                else if (tok->str() == "}")
                    break;
                tok = tok->next();
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

        // Insert semicolon after the moved strcat()
        tok->insertToken(";");
    }

}

void Tokenizer::duplicateEnumError(const Token * tok1, const Token * tok2, const std::string & type)
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
                                                   "' hides enumerator with same name"),
                                           "variableHidingEnum",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

// Check if this statement is a duplicate definition.  A duplicate
// definition will hide the enumerator within it's scope so just
// skip the entire scope of the duplicate.
bool Tokenizer::duplicateDefinition(Token ** tokPtr, const Token * name)
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
                if (Token::Match(end->link()->previous(), "if|for|while|switch"))
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
                // look backwards
                if (Token::Match(tok->previous(), "enum|,") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return")) {
                    duplicateEnumError(*tokPtr, name, "Variable");
                    return true;
                }
            }
        }
    }
    return false;
}

void Tokenizer::simplifyEnum()
{
    // Don't simplify enums in java files
    if (isJavaOrCSharp())
        return;

    std::string className;
    int classLevel = 0;
    bool goback = false;
    for (Token *tok = _tokens; tok; tok = tok->next()) {

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
                        tok1->insertToken(MathLib::toString<MathLib::bigint>(lastValue));
                        enumValue = 0;
                        enumValueStart = valueStart->next();
                        enumValueEnd = tok1->next();
                    } else {
                        // value is previous numeric value + 1
                        tok1->insertToken(MathLib::toString<MathLib::bigint>(lastValue));
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
                    if (enumValueEnd->str() == "(" ||
                        enumValueEnd->str() == "[" ||
                        enumValueEnd->str() == "{")
                        ++level;
                    while (enumValueEnd->next() &&
                           (!Token::Match(enumValueEnd->next(), "}|,") || level)) {
                        if (enumValueEnd->next()->str() == "(" ||
                            enumValueEnd->next()->str() == "[" ||
                            enumValueEnd->next()->str() == "{")
                            ++level;
                        else if (enumValueEnd->next()->str() == ")" ||
                                 enumValueEnd->next()->str() == "]" ||
                                 enumValueEnd->next()->str() == "}")
                            --level;

                        enumValueEnd = enumValueEnd->next();
                    }
                    // remember this expression in case it needs to be incremented
                    lastEnumValueStart = enumValueStart;
                    lastEnumValueEnd = enumValueEnd;
                    // skip over expression
                    tok1 = enumValueEnd;
                }

                // find all uses of this enumerator and substitute it's value for it's name
                if (enumName && (enumValue || (enumValueStart && enumValueEnd))) {
                    const std::string pattern = className.empty() ?
                                                std::string("") :
                                                std::string(className + " :: " + enumName->str());
                    int level = 1;
                    bool inScope = true;

                    bool exitThisScope = false;
                    int exitScope = 0;
                    bool simplify = false;
                    bool hasClass = false;
                    const Token *endScope = 0;
                    for (Token *tok2 = tok1->next(); tok2; tok2 = tok2->next()) {
                        if (tok2->str() == "}") {
                            --level;
                            if (level < 0)
                                inScope = false;

                            if (exitThisScope) {
                                if (level < exitScope)
                                    exitThisScope = false;
                            }
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
                            }
                            endScope = tok2->link();
                        } else if (!pattern.empty() && Token::Match(tok2, pattern.c_str())) {
                            simplify = true;
                            hasClass = true;
                        } else if (inScope && !exitThisScope && tok2->str() == enumName->str()) {
                            if (!duplicateDefinition(&tok2, enumName)) {
                                if (Token::simpleMatch(tok2->previous(), "::") ||
                                    Token::Match(tok2->next(), "::|[")) {
                                    // Don't replace this enum if:
                                    // * it's preceded or followed by "::"
                                    // * it's followed by "["
                                } else {
                                    simplify = true;
                                    hasClass = false;
                                }
                            } else {
                                // something with the same name.
                                exitScope = level;
                                if (endScope)
                                    tok2 = endScope->previous();
                            }
                        }

                        if (simplify) {
                            if (enumValue)
                                tok2->str(enumValue->str());
                            else {
                                tok2 = tok2->previous();
                                tok2->deleteNext();
                                tok2 = copyTokens(tok2, enumValueStart, enumValueEnd);
                            }

                            if (hasClass) {
                                tok2->deleteNext(2);
                            }

                            simplify = false;
                        }
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
                const std::string pattern(className.empty() ? "" : (className + " :: " + enumType->str()).c_str());

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
                    else if (!pattern.empty() && ((Token::simpleMatch(tok2, "enum") && Token::Match(tok2->next(), pattern.c_str())) || Token::Match(tok2, pattern.c_str()))) {
                        simplify = true;
                        hasClass = true;
                    } else if (inScope && !exitThisScope && (tok2->str() == enumType->str() || (tok2->str() == "enum" && tok2->next() && tok2->next()->str() == enumType->str()))) {
                        if (Token::simpleMatch(tok2->previous(), "::")) {
                            // Don't replace this enum if it's preceded by "::"
                        } else if (tok2->next() &&
                                   (tok2->next()->isName() || tok2->next()->str() == "(")) {
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
            if (start != _tokens) {
                tok1 = start->previous();
                tok1->deleteNext();
                //no need to remove last token in the list
                if (tok1->tokAt(2))
                    tok1->deleteNext();
                tok = tok1;
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

void Tokenizer::simplifyStd()
{
    std::set<std::string> f;
    f.insert("strcat");
    f.insert("strcpy");
    f.insert("strncat");
    f.insert("strncpy");
    f.insert("free");
    f.insert("malloc");
    f.insert("strdup");

    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

        // function pointer call..
        if (tok && Token::Match(tok->tokAt(-4), "[;{}] ( * %var% )"))
            return true;

        if (!tok->isName())
            return false;

        if (tok->str() == "exit")
            return true;

        while (tok && (Token::Match(tok, "::|.") || tok->isName()))
            tok = tok->previous();

        if (Token::Match(tok, "[;{}]")) {
            if (unknown)
                *unknown = true;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------

const Token *Tokenizer::getFunctionTokenByName(const char funcname[]) const
{
    getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = _symbolDatabase->scopeList.begin(); scope != _symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type == Scope::eFunction) {
            if (scope->classDef->str() == funcname)
                return scope->classDef;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------

// Deallocate lists..
void Tokenizer::deallocateTokens()
{
    deleteTokens(_tokens);
    _tokens = 0;
    _tokensBack = 0;
    _files.clear();
}

void Tokenizer::deleteTokens(Token *tok)
{
    while (tok) {
        Token *next = tok->next();
        delete tok;
        tok = next;
    }
}

//---------------------------------------------------------------------------

void Tokenizer::eraseDeadCode(Token *begin, const Token *end)
{
    if (!begin)
        return;
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
                break;  //too many ending round parenthesis
            --roundbraces;
            tok->deleteNext();
            continue;
        }

        if (roundbraces) {
            tok->deleteNext();
            continue;
        }

        if (Token::Match(tok, "[{};] switch (") && tok->linkAt(2)) {
            if (!checklabel) {
                if (!indentlabel) {
                    //remove 'switch ( ... )'
                    const Token *endround = tok->linkAt(2);
                    Token::eraseTokens(tok, endround->next());
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
        } else if (Token::Match(tok, "[{};] case %any% : ;") || Token::Match(tok, "[{};] default : ;")) {
            if (indentlevel == 1)
                break;      //it seems like the function was called inside a case-default block.
            if (indentlevel == indentcase)
                ++indentlevel;
            if (!checklabel || !indentswitch) {
                tok->deleteNext(3+(tok->next()->str()=="case"));
            } else {
                tok = tok->tokAt(3+(tok->next()->str()=="case"));
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
                    const Token *end2 = tokcheck->next()->link();
                    unsigned int indentlevel2 = indentlevel;
                    while (tok2->next() && tok2->next() != end2->next()) {
                        if (Token::Match(tok2->next(), "{|[|(")) {
                            tok2 = tok2->next()->link();
                        } else if (Token::Match(tok2, "[{};] case %any% : ;") || Token::Match(tok2, "[{};] default : ;")) {
                            tok2->deleteNext(3+(tok2->next()->str()=="case"));
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
        } else {        //no need to keep the other strings, remove them.
            tok->deleteNext();
        }
    }
}

//---------------------------------------------------------------------------

const char *Tokenizer::getParameterName(const Token *ftok, unsigned int par)
{
    unsigned int _par = 1;
    for (; ftok; ftok = ftok->next()) {
        if (ftok->str() == ")")
            break;
        else if (ftok->str() == ",")
            ++_par;
        else if (par == _par && Token::Match(ftok, "%var% [,)]"))
            return ftok->str().c_str();
    }
    return NULL;
}

//---------------------------------------------------------------------------

std::string Tokenizer::fileLine(const Token *tok) const
{
    std::ostringstream ostr;
    ostr << "[" << _files.at(tok->fileIndex()) << ":" << tok->linenr() << "]";
    return ostr.str();
}

std::string Tokenizer::file(const Token *tok) const
{
    return _files.at(tok->fileIndex());
}

//---------------------------------------------------------------------------

void Tokenizer::syntaxError(const Token *tok)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           "syntax error",
                                           "syntaxError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::syntaxError(const Token *tok, char c)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           std::string("Invalid number of character (") +
                                           c +
                                           ") " +
                                           "when these macros are defined: '" +
                                           _configuration +
                                           "'.",
                                           "syntaxError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::cppcheckError(const Token *tok) const
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           "Analysis failed. If the code is valid then please report this failure.",
                                           "cppcheckError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}


void Tokenizer::simplifyMathFunctions()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "atol ( %str% )")) {
            if (!MathLib::isInt(tok->tokAt(2)->strValue())) {
                // Ignore strings which we can't convert
                continue;
            }

            if (tok->previous() &&
                Token::simpleMatch(tok->tokAt(-2), "std ::")) {
                // Delete "std ::"
                tok = tok->tokAt(-2);
                tok->deleteNext();
                tok->deleteThis();
            }

            // Delete atol(
            tok->deleteNext();
            tok->deleteThis();

            // Convert string into a number
            tok->str(MathLib::toString(MathLib::toLongNumber(tok->strValue())));

            // Delete remaining )
            tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyComma()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "for (") ||
            Token::Match(tok, "=|enum {")) {
            tok = tok->next()->link();

            continue;
        }

        if (tok->str() == "(" || tok->str() == "[") {
            tok = tok->link();
            continue;
        }

        // Skip unhandled template specifiers..
        if (Token::Match(tok, "%var% <")) {
            // Todo.. use the link instead.
            unsigned int comparelevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "<")
                    ++comparelevel;
                else if (tok2->str() == ">") {
                    if (!comparelevel) {
                        tok = tok2;
                        break;
                    }
                    ++comparelevel;
                } else if (Token::Match(tok2, "[;{}]"))
                    break;
            }
        }

        // If token after the comma is a constant number, simplification is not required.
        if (tok->str() != "," || Token::Match(tok->next(), "%num%"))
            continue;

        // We must not accept just any keyword, e.g. accepting int
        // would cause function parameters to corrupt.
        if (Token::simpleMatch(tok->next(), "delete")) {
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
                    } else if (Token::Match(tok2, "[;,{}()]")) {
                        break;
                    }
                }
            }
        }

        bool inReturn = false;
        Token *startFrom = NULL;    // next tokean after "; return"
        Token *endAt = NULL;        // first ";" token after "; return"

        // find "; return" pattern before comma
        for (Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
            if (Token::Match(tok2, "[;{}]")) {
                break;

            } else if (tok2->str() == "return" && Token::Match(tok2->previous(), "[;{}]")) {
                inReturn = true;
                startFrom = tok2->next();
                break;
            }
        }

        // find token where return ends and also count commas
        if (inReturn) {
            size_t commaCounter = 0;
            size_t indentlevel = 0;

            for (Token *tok2 = startFrom; tok2; tok2 = tok2->next()) {
                if (tok2->str() == ";") {
                    endAt = tok2;
                    break;

                } else if (tok2->str() == "(") {
                    ++indentlevel;

                } else if (tok2->str() == ")") {
                    --indentlevel;

                } else if (tok2->str() == "," && indentlevel == 0) {
                    ++commaCounter;
                }
            }

            if (commaCounter) {
                indentlevel = 0;

                // change tokens:
                // "; return a ( ) , b ( ) , c ;"
                // to
                // "; return a ( ) ; b ( ) ; c ;"
                for (Token *tok2 = startFrom; tok2 != endAt; tok2 = tok2->next()) {
                    if (tok2->str() == "(") {
                        ++indentlevel;

                    } else if (tok2->str() == ")") {
                        --indentlevel;

                    } else if (tok2->str() == "," && indentlevel == 0) {
                        tok2->str(";");
                        --commaCounter;
                        if (commaCounter == 0) {
                            tok2->insertToken("return");
                        }
                    }
                }

                // delete old "return"
                startFrom->previous()->deleteThis();
                startFrom = 0;   // give dead pointer a value

                tok = endAt;
                if (!tok)
                    return;
            }
        }

    }
}


void Tokenizer::removeExceptionSpecifications(Token *tok) const
{
    while (tok) {
        if (tok->str() == "{")
            tok = tok->link();

        else if (tok->str() == "}")
            break;

        else if (Token::Match(tok, ") const| throw (")) {
            if (tok->next()->str() == "const") {
                Token::eraseTokens(tok->next(), tok->linkAt(3));
                tok = tok->next();
            } else
                Token::eraseTokens(tok, tok->linkAt(2));
            tok->deleteNext();
        }

        else if (Token::Match(tok, "class|namespace|struct|union %type% :|{")) {
            tok = tok->tokAt(2);
            while (tok && !Token::Match(tok, "[;{=]"))
                tok = tok->next();
            if (tok && tok->str() == "{") {
                removeExceptionSpecifications(tok->next());
                tok = tok->link();
            } else
                continue;
        }

        else if (Token::Match(tok, "namespace|struct|union {")) {
            tok = tok->next();
            removeExceptionSpecifications(tok->next());
            tok = tok->link();
        }

        tok = tok->next();
    }
}



bool Tokenizer::validate() const
{
    std::stack<const Token *> linktok;
    const Token *lastTok = 0;
    for (const Token *tok = tokens(); tok; tok = tok->next()) {
        lastTok = tok;
        if (Token::Match(tok, "[{([]")) {
            if (tok->link() == 0) {
                cppcheckError(tok);
                return false;
            }

            linktok.push(tok);
            continue;
        }

        else if (Token::Match(tok, "[})]]")) {
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
            continue;
        }

        if (tok->link() != 0) {
            cppcheckError(tok);
            return false;
        }
    }

    if (!linktok.empty()) {
        cppcheckError(linktok.top());
        return false;
    }

    // Validate that the Tokenizer::_tokensBack is updated correctly during simplifications
    if (lastTok != _tokensBack) {
        cppcheckError(lastTok);
        return false;
    }

    return true;
}

std::string Tokenizer::simplifyString(const std::string &source)
{
    std::string str = source;

    // true when previous char is a \ .
    bool escaped = false;
    for (std::string::size_type i = 0; i + 2 < str.size(); ++i) {
        if (!escaped) {
            if (str[i] == '\\')
                escaped = true;

            continue;
        }

        if (str[i] == 'x') {
            // Hex value
            if (str[i+1] == '0' && str[i+2] == '0')
                str.replace(i, 3, "0");
            else if (i > 0) {
                // We will replace all other character as 'a'
                // If that causes problems in the future, this can
                // be improved. But for now, this should be OK.
                unsigned int n = 1;
                while (n < 2 && std::isxdigit(str[i+1+n]))
                    ++n;
                --i;
                n += 2;
                str.replace(i, n, "a");
            }
        } else if (MathLib::isOctalDigit(str[i])) {
            if (MathLib::isOctalDigit(str[i+1]) &&
                MathLib::isOctalDigit(str[i+2])) {
                if (str[i+1] == '0' && str[i+2] == '0')
                    str.replace(i, 3, "0");
                else {
                    // We will replace all other character as 'a'
                    // If that causes problems in the future, this can
                    // be improved. But for now, this should be OK.
                    --i;
                    str.replace(i, 4, "a");
                }
            }
        }

        escaped = false;
    }

    return str;
}


void Tokenizer::simplifyStructInit()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] struct| %type% %var% = { . %type% =")) {
            // Goto "." and check if the initializations have an expected format
            const Token *tok2 = tok;
            while (tok2->str() != ".")
                tok2 = tok2->next();
            while (tok2 && tok2->str() == ".") {
                if (Token::Match(tok2, ". %type% = %num% [,}]"))
                    tok2 = tok2->tokAt(4);
                else if (Token::Match(tok2, ". %type% = %var% [,}]"))
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "[;(] %any% >|>= %any% [);]")) {
            if (!tok->next()->isName() && !tok->next()->isNumber())
                continue;
            const Token *operand2 = tok->tokAt(3);
            if (!operand2->isName() && !operand2->isNumber())
                continue;
            const std::string op1(tok->next()->str());
            tok->next()->str(tok->strAt(3));
            tok->tokAt(3)->str(op1);
            if (tok->strAt(2) == ">")
                tok->tokAt(2)->str("<");
            else
                tok->tokAt(2)->str("<=");
        } else if (Token::Match(tok, "( %num% ==|!= %var% )")) {
            if (!tok->next()->isName() && !tok->next()->isNumber())
                continue;
            const std::string op1(tok->next()->str());
            tok->next()->str(tok->strAt(3));
            tok->tokAt(3)->str(op1);
        }
    }
}

void Tokenizer::simplifyConst()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "%type% const") &&
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
    t.duplicateTypedefError(0, 0, "Variable");
    t.duplicateDeclarationError(0, 0, "Variable");
    t.duplicateEnumError(0, 0, "Variable");
    t.unnecessaryQualificationError(0, "type");
}

void Tokenizer::simplifyWhile0()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (!Token::Match(tok, "while ( %var% ( %var% ) ) {"))
            continue;

        Token *func = tok->tokAt(2);
        Token *var = tok->tokAt(4);
        Token *end = tok->linkAt(7);
        if (!end)
            break;

        tok->str("int");
        tok->next()->insertToken("cppcheck:r");
        tok->insertToken("while");
        tok->insertToken(";");
        tok->insertToken(")");
        tok->insertToken(var->str());
        tok->next()->varId(var->varId());
        tok->insertToken("(");
        tok->insertToken(func->str());
        tok->insertToken("=");
        tok->insertToken("cppcheck:r");
        Token::createMutualLinks(tok->tokAt(4), tok->tokAt(6));
        end->previous()->insertToken("cppcheck:r");
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
    std::list<bool> skip; // true = in function, false = not in function
    skip.push_back(false);

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        Token *restart;

        // check for start of scope and determine if it is in a function
        if (tok->str() == "{")
            skip.push_back(Token::Match(tok->previous(), "const|)"));

        // end of scope
        else if (tok->str() == "}" && !skip.empty())
            skip.pop_back();

        // check for named struct/union
        else if (Token::Match(tok, "class|struct|union %type% :|{")) {
            Token *isStatic = tok->previous() && tok->previous()->str() == "static" ? tok->previous() : NULL;
            Token *type = tok->next();
            Token *next = tok->tokAt(2);

            while (next && next->str() != "{")
                next = next->next();
            if (!next)
                continue;
            skip.push_back(false);
            tok = next->link();
            restart = next;

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|[")) {
                tok->insertToken(";");
                tok = tok->next();
                if (isStatic) {
                    isStatic->deleteThis();
                    tok->insertToken("static");
                    tok = tok->next();
                }
                tok->insertToken(type->str());
            }

            tok = restart;
        }

        // check for anonymous struct/union
        else if (Token::Match(tok, "struct|union {")) {
            bool inFunction = skip.back();
            skip.push_back(false);
            Token *tok1 = tok;

            restart = tok->next();
            tok = tok->next()->link();

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|[")) {
                std::string name;

                name = "Anonymous" + MathLib::toString<unsigned int>(count++);

                tok1->insertToken(name);

                tok->insertToken(";");
                tok = tok->next();
                tok->insertToken(name);
            }

            // unnamed anonymous struct/union so possibly remove it
            else if (tok->next() && tok->next()->str() == ";") {
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
                    skip.pop_back();
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
    static const char pattern[] = "__cdecl|__stdcall|__fastcall|__thiscall|__clrcall|__syscall|__pascal|__fortran|__far|__near|WINAPI|APIENTRY|CALLBACK";
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (Token::Match(tok, pattern)) {
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyDeclspec()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "__declspec (") && tok->next()->link() && tok->next()->link()->next()) {
            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAttribute()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (Token::simpleMatch(tok, "__attribute__ (") && tok->next()->link() && tok->next()->link()->next()) {
            if (Token::simpleMatch(tok->tokAt(2), "( unused )")) {
                // check if after variable name
                if (Token::Match(tok->next()->link()->next(), ";|=")) {
                    if (Token::Match(tok->previous(), "%type%"))
                        tok->previous()->isUnused(true);
                }

                // check if before variable name
                else if (Token::Match(tok->next()->link()->next(), "%type%"))
                    tok->next()->link()->next()->isUnused(true);
            }

            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

// Remove "volatile", "inline", "register", and "restrict"
void Tokenizer::simplifyKeyword()
{
    static const char pattern[] = "volatile|inline|__inline|__forceinline|register|restrict|__restrict|__restrict__";
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        while (Token::Match(tok, pattern)) {
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAssignmentInFunctionCall()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "(")
            tok = tok->link();

        // Find 'foo(var='. Exclude 'assert(var=' to allow tests to check that assert(...) does not contain side-effects
        else if (Token::Match(tok, "[;{}] %var% ( %var% =") &&
                 Token::simpleMatch(tok->linkAt(2), ") ;") &&
                 tok->next()->str() != "assert") {
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

// Remove __asm..
void Tokenizer::simplifyAsm()
{
    std::string instruction;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "__asm|_asm|asm {") &&
            tok->next()->link()->next()) {
            instruction = tok->tokAt(2)->stringify(tok->next()->link());
            Token::eraseTokens(tok, tok->next()->link()->next());
        }

        else if (Token::Match(tok, "asm|__asm|__asm__ volatile|__volatile__| (")) {
            // Goto "("
            Token *partok = tok->next();
            if (partok->str() != "(")
                partok = partok->next();
            instruction = partok->next()->stringify(partok->link());
            Token::eraseTokens(tok, partok->link()->next());
        }

        else if (Token::simpleMatch(tok, "__asm")) {
            const Token *tok2 = tok;
            while (tok2 && (tok2->isNumber() || tok2->isName() || tok2->str() == ","))
                tok2 = tok2->next();
            if (tok2 && tok2->str() == ";") {
                instruction = tok->next()->stringify(tok2);
                Token::eraseTokens(tok, tok2);
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {

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
                (tok1->tokAt(2)->isBoolean() || Token::Match(tok1->tokAt(2), "%num%") ||
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
            unsigned char offset = (tok->next()->str() == "const") ? 1 : 0;

            if (tok->strAt(3 + offset) != "{") {
                tok->deleteNext(4+offset);
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok->next(), "__builtin_expect (")) {
            // Count parentheses for tok2
            unsigned int parlevel = 0;
            for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")") {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }
                if (parlevel == 1 && tok2->str() == ",") {
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


// Remove Microsoft MFC 'DECLARE_MESSAGE_MAP()'
void Tokenizer::simplifyMicrosoftMFC()
{
    // skip if not Windows
    if (!(_settings->platformType == Settings::Win32A ||
          _settings->platformType == Settings::Win32W ||
          _settings->platformType == Settings::Win64))
        return;

    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "CopyMemory (")) {
            tok->str("memcpy");
        } else if (Token::simpleMatch(tok, "MoveMemory (")) {
            tok->str("memmove");
        } else if (Token::simpleMatch(tok, "FillMemory (")) {
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
        } else if (Token::simpleMatch(tok, "ZeroMemory (")) {
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
        }
    }
}

void Tokenizer::simplifyMicrosoftStringFunctions()
{
    // skip if not Windows
    if (_settings->platformType == Settings::Win32A) {
        for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            } else if (Token::simpleMatch(tok, "_tprintf (")) {
                tok->str("printf");
            } else if (Token::simpleMatch(tok, "_stprintf (")) {
                tok->str("sprintf");
            } else if (Token::simpleMatch(tok, "_sntprintf (")) {
                tok->str("snprintf");
            } else if (Token::simpleMatch(tok, "_tscanf (")) {
                tok->str("scanf");
            } else if (Token::simpleMatch(tok, "_stscanf (")) {
                tok->str("sscanf");
            } else if (Token::Match(tok, "_T ( %str% )")) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
            } else if (Token::Match(tok, "_T ( %any% )") && tok->strAt(2)[0] == '\'') {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    } else if (_settings->platformType == Settings::Win32W ||
               _settings->platformType == Settings::Win64) {
        for (Token *tok = _tokens; tok; tok = tok->next()) {
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
            } else if (Token::simpleMatch(tok, "_tprintf (")) {
                tok->str("wprintf");
            } else if (Token::simpleMatch(tok, "_stprintf (")) {
                tok->str("swprintf");
            } else if (Token::simpleMatch(tok, "_sntprintf (")) {
                tok->str("snwprintf");
            } else if (Token::simpleMatch(tok, "_tscanf (")) {
                tok->str("wscanf");
            } else if (Token::simpleMatch(tok, "_stscanf (")) {
                tok->str("swscanf");
            } else if (Token::Match(tok, "_T ( %str% )")) {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
            } else if (Token::Match(tok, "_T ( %any% )") && tok->strAt(2)[0] == '\'') {
                tok->deleteNext();
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    }
}

// Remove Borland code
void Tokenizer::simplifyBorland()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "( __closure * %var% )")) {
            tok->deleteNext();
        }
    }

    // I think that these classes are always declared at the outer scope
    // I save some time by ignoring inner classes.
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (tok->str() == "{") {
            tok = tok->link();
            if (!tok)
                break;
        }

        if (Token::Match(tok, "class %var% :|{")) {
            // count { and } for tok2
            unsigned int indentlevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next()) {
                if (tok2->str() == "{") {
                    if (indentlevel == 0)
                        indentlevel = 1;
                    else
                        tok2 = tok2->link();
                } else if (tok2->str() == "}") {
                    break;
                } else if (tok2->str() == "__property" &&
                           Token::Match(tok2->previous(), ";|{|}|protected:|public:|__published:")) {
                    while (tok2->next() && !Token::Match(tok2, "{|;"))
                        tok2->deleteThis();
                    if (Token::simpleMatch(tok2, "{")) {
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
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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

            if (Token::simpleMatch(tok2->next(), "Q_OBJECT")) {
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

const SymbolDatabase *Tokenizer::getSymbolDatabase() const
{
    if (!_symbolDatabase)
        _symbolDatabase = new SymbolDatabase(this, _settings, _errorLogger);

    return _symbolDatabase;
}

void Tokenizer::simplifyOperatorName()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
                if (Token::Match(par, "=|.|++|--|%op%")) {
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
    std::vector<Space> classInfo;
    for (Token *tok = _tokens; tok; tok = tok->next()) {
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
                int offset = 3;
                if (tok->strAt(2) == "operator") {
                    const Token *tok1 = tok->tokAt(offset);

                    // check for operator ()
                    if (tok1->str() == "(") {
                        tok1 = tok1->next();
                        ++offset;
                    }

                    while (tok1 && tok1->str() != "(") {
                        if (tok1->str() == ";")
                            break;
                        tok1 = tok1->next();
                        ++offset;
                    }
                    if (!tok1 || tok1->str() != "(")
                        continue;
                } else if (tok->strAt(2) == "~")
                    ++offset;

                if (tok->tokAt(offset) && Token::Match(tok->linkAt(offset), ") const| {|;|:")) {
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

                    if (_settings && _settings->isEnabled("portability"))
                        unnecessaryQualificationError(tok, qualification);

                    tok->deleteNext();
                    tok->deleteThis();
                }
            }
        }
    }
}

void Tokenizer::unnecessaryQualificationError(const Token *tok, const std::string &qualification)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok) {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::portability,
                                           "Extra qualification \'" + qualification + "\' unnecessary and considered an error by many compilers.",
                                           "unnecessaryQualification",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::simplifyReturn()
{
    for (Token *tok = _tokens; tok; tok = tok->next()) {
        if (Token::Match(tok, "return strncat ( %any% , %any% , %any% ) ;")) {
            // Change to: strncat ( %any% , %any% , %any% ) ;
            tok->deleteNext();
            tok->str("strncat");

            // Change to: strncat ( %any% , %any% , %any% ) ; return %any% ;
            tok->tokAt(8)->insertToken("return");
            copyTokens(tok->tokAt(9), tok->tokAt(2), tok->tokAt(2));
            tok->tokAt(10)->insertToken(";");
        }
    }
}

void Tokenizer::printUnknownTypes()
{
    getSymbolDatabase();

    std::set<std::string> unknowns;

    for (unsigned int i = 1; i <= _varId; ++i) {
        const Variable *var = _symbolDatabase->getVariableFromVarId(i);

        // is unknown record type?
        if (var && var->isClass() && !var->type()) {
            std::string    name;

            // single token type?
            if (var->typeStartToken() == var->typeEndToken())
                name = var->typeStartToken()->str();

            // complicated type
            else {
                const Token *tok = var->typeStartToken();
                int level = 0;

                while (tok) {
                    // skip pointer and reference part of type
                    if (level == 0 && (tok->str() ==  "*" || tok->str() == "&"))
                        break;

                    name += tok->str();

                    if (Token::Match(tok, "struct|union"))
                        name += " ";

                    // pointers and referennces are OK in template
                    else if (tok->str() == "<")
                        ++level;
                    else if (tok->str() == ">")
                        --level;

                    if (tok == var->typeEndToken())
                        break;

                    tok = tok->next();
                }
            }

            unknowns.insert(name);
        }
    }

    if (!unknowns.empty()) {
        std::ostringstream ss;

        ss << unknowns.size() << " unknown types:" << std::endl;

        std::set<std::string>::const_iterator it;
        size_t count = 1;

        for (it = unknowns.begin(); it != unknowns.end(); ++it, ++count)
            ss << count << ": " << *it << std::endl;

        if (_errorLogger)
            _errorLogger->reportOut(ss.str());
    }
}

std::string Tokenizer::getSourceFilePath() const
{
    if (_files.empty())
        return std::string("");
    return _files[0];
}

bool Tokenizer::isJava() const
{
    return Path::isJava(getSourceFilePath());
}

bool Tokenizer::isCSharp() const
{
    return Path::isCSharp(getSourceFilePath());
}

bool Tokenizer::isJavaOrCSharp() const
{
    return isJava() || isCSharp();
}

bool Tokenizer::isC() const
{
    return Path::isC(getSourceFilePath());
}

bool Tokenizer::isCPP() const
{
    return Path::isCPP(getSourceFilePath());
}
