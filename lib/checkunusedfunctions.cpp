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
#include "checkunusedfunctions.h"
#include "tokenize.h"
#include "token.h"
#include "symboldatabase.h"
#include "analyzerinfo.h"
#include <cctype>
#include <tinyxml2.h>
//---------------------------------------------------------------------------



// Register this check class
CheckUnusedFunctions CheckUnusedFunctions::instance;

static const struct CWE CWE561(561U);   // Dead Code


//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

void CheckUnusedFunctions::parseTokens(const Tokenizer &tokenizer, const char FileName[], const Settings *settings, bool clear)
{
    const bool doMarkup = settings->library.markupFile(FileName);
    const SymbolDatabase* symbolDatabase = tokenizer.getSymbolDatabase();

    // Function declarations..
    if (clear)
        _functionDecl.clear();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        const Function* func = scope->function;
        if (!func || !func->token || scope->classStart->fileIndex() != 0)
            continue;

        // Don't warn about functions that are marked by __attribute__((constructor)) or __attribute__((destructor))
        if (func->isAttributeConstructor() || func->isAttributeDestructor() || func->type != Function::eFunction || func->isOperator())
            continue;

        // Don't care about templates
        if (tokenizer.isCPP() && func->retDef->str() == "template")
            continue;

        _functionDecl.push_back(FunctionDecl(func));

        FunctionUsage &usage = _functions[func->name()];

        if (!usage.lineNumber)
            usage.lineNumber = func->token->linenr();

        // No filename set yet..
        if (usage.filename.empty()) {
            usage.filename = tokenizer.list.getSourceFilePath();
        }
        // Multiple files => filename = "+"
        else if (usage.filename != tokenizer.list.getSourceFilePath()) {
            //func.filename = "+";
            usage.usedOtherFile |= usage.usedSameFile;
        }
    }

    // Function usage..
    if (clear)
        _functionCalls.clear();
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {

        // parsing of library code to find called functions
        if (settings->library.isexecutableblock(FileName, tok->str())) {
            const Token * markupVarToken = tok->tokAt(settings->library.blockstartoffset(FileName));
            int scope = 0;
            bool start = true;
            // find all function calls in library code (starts with '(', not if or while etc)
            while (scope || start) {
                if (markupVarToken->str() == settings->library.blockstart(FileName)) {
                    scope++;
                    if (start) {
                        start = false;
                    }
                } else if (markupVarToken->str() == settings->library.blockend(FileName))
                    scope--;
                else if (!settings->library.iskeyword(FileName, markupVarToken->str())) {
                    _functionCalls.insert(markupVarToken->str());
                    if (_functions.find(markupVarToken->str()) != _functions.end())
                        _functions[markupVarToken->str()].usedOtherFile = true;
                    else if (markupVarToken->next()->str() == "(") {
                        FunctionUsage &func = _functions[markupVarToken->str()];
                        func.filename = tokenizer.list.getSourceFilePath();
                        if (func.filename.empty() || func.filename == "+")
                            func.usedOtherFile = true;
                        else
                            func.usedSameFile = true;
                    }
                }
                markupVarToken = markupVarToken->next();
            }
        }

        if (!doMarkup // only check source files
            && settings->library.isexporter(tok->str()) && tok->next() != 0) {
            const Token * propToken = tok->next();
            while (propToken && propToken->str() != ")") {
                if (settings->library.isexportedprefix(tok->str(), propToken->str())) {
                    const Token* nextPropToken = propToken->next();
                    const std::string& value = nextPropToken->str();
                    if (_functions.find(value) != _functions.end()) {
                        _functions[value].usedOtherFile = true;
                    }
                    _functionCalls.insert(value);
                }
                if (settings->library.isexportedsuffix(tok->str(), propToken->str())) {
                    const Token* prevPropToken = propToken->previous();
                    const std::string& value = prevPropToken->str();
                    if (value != ")" && _functions.find(value) != _functions.end()) {
                        _functions[value].usedOtherFile = true;
                    }
                    _functionCalls.insert(value);
                }
                propToken = propToken->next();
            }
        }

        if (doMarkup && settings->library.isimporter(FileName, tok->str()) && tok->next()) {
            const Token * propToken = tok->next();
            if (propToken->next()) {
                propToken = propToken->next();
                while (propToken && propToken->str() != ")") {
                    const std::string& value = propToken->str();
                    if (!value.empty()) {
                        _functions[value].usedOtherFile = true;
                        _functionCalls.insert(value);
                        break;
                    }
                    propToken = propToken->next();
                }
            }
        }

        if (settings->library.isreflection(tok->str())) {
            const int argIndex = settings->library.reflectionArgument(tok->str());
            if (argIndex >= 0) {
                const Token * funcToken = tok->next();
                int index = 0;
                std::string value;
                while (funcToken) {
                    if (funcToken->str()==",") {
                        if (++index == argIndex)
                            break;
                        value = "";
                    } else
                        value += funcToken->str();
                    funcToken = funcToken->next();
                }
                if (index == argIndex) {
                    value = value.substr(1, value.length() - 2);
                    _functions[value].usedOtherFile = true;
                    _functionCalls.insert(value);
                }
            }
        }

        const Token *funcname = nullptr;

        if (tok->scope()->isExecutable() && Token::Match(tok, "%name% (")) {
            funcname = tok;
        } else if (tok->scope()->isExecutable() && Token::Match(tok, "%name% <") && Token::simpleMatch(tok->linkAt(1), "> (")) {
            funcname = tok;
        } else if (Token::Match(tok, "[;{}.,()[=+-/|!?:]")) {
            funcname = tok->next();
            if (funcname && funcname->str() == "&")
                funcname = funcname->next();
            if (funcname && funcname->str() == "::")
                funcname = funcname->next();
            while (Token::Match(funcname, "%name% :: %name%"))
                funcname = funcname->tokAt(2);

            if (!Token::Match(funcname, "%name% [(),;]:}]"))
                continue;
        }

        if (!funcname)
            continue;

        // funcname ( => Assert that the end parentheses isn't followed by {
        if (Token::Match(funcname, "%name% (|<")) {
            const Token *ftok = funcname->next();
            if (ftok->str() == "<")
                ftok = ftok->link();
            if (Token::Match(ftok->linkAt(1), ") const|throw|{"))
                funcname = nullptr;
        }

        if (funcname) {
            FunctionUsage &func = _functions[ funcname->str()];
            const std::string& called_from_file = tokenizer.list.getSourceFilePath();

            if (func.filename.empty() || func.filename == "+" || func.filename != called_from_file)
                func.usedOtherFile = true;
            else
                func.usedSameFile = true;

            _functionCalls.insert(funcname->str());
        }
    }
}




void CheckUnusedFunctions::check(ErrorLogger * const errorLogger, const Settings& settings)
{
    for (std::map<std::string, FunctionUsage>::const_iterator it = _functions.begin(); it != _functions.end(); ++it) {
        const FunctionUsage &func = it->second;
        if (func.usedOtherFile || func.filename.empty())
            continue;
        if (it->first == "main" ||
            (settings.isWindowsPlatform() && (it->first == "WinMain" || it->first == "_tmain")) ||
            it->first == "if")
            continue;
        if (!func.usedSameFile) {
            std::string filename;
            if (func.filename != "+")
                filename = func.filename;
            unusedFunctionError(errorLogger, filename, func.lineNumber, it->first);
        } else if (! func.usedOtherFile) {
            /** @todo add error message "function is only used in <file> it can be static" */
            /*
            std::ostringstream errmsg;
            errmsg << "The function '" << it->first << "' is only used in the file it was declared in so it should have local linkage.";
            _errorLogger->reportErr( errmsg.str() );
            */
        }
    }
}

void CheckUnusedFunctions::unusedFunctionError(ErrorLogger * const errorLogger,
        const std::string &filename, unsigned int lineNumber,
        const std::string &funcname)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        ErrorLogger::ErrorMessage::FileLocation fileLoc;
        fileLoc.setfile(filename);
        fileLoc.line = lineNumber;
        locationList.push_back(fileLoc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList, emptyString, Severity::style, "The function '" + funcname + "' is never used.", "unusedFunction", CWE561, false);
    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}

Check::FileInfo *CheckUnusedFunctions::getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const
{
    if (!settings->isEnabled("unusedFunction"))
        return nullptr;
    if (settings->jobs == 1 && settings->buildDir.empty())
        instance.parseTokens(*tokenizer, tokenizer->list.getFiles().front().c_str(), settings);
    return nullptr;
}

void CheckUnusedFunctions::analyseWholeProgram(const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger)
{
    (void)fileInfo;
    check(&errorLogger, settings);
}

CheckUnusedFunctions::FunctionDecl::FunctionDecl(const Function *f)
    : functionName(f->name()), lineNumber(f->token->linenr())
{
}

std::string CheckUnusedFunctions::analyzerInfo() const
{
    std::ostringstream ret;
    for (std::list<FunctionDecl>::const_iterator it = _functionDecl.begin(); it != _functionDecl.end(); ++it) {
        ret << "    <functiondecl"
            << " functionName=\"" << ErrorLogger::toxml(it->functionName) << '\"'
            << " lineNumber=\"" << it->lineNumber << "\"/>\n";
    }
    for (std::set<std::string>::const_iterator it = _functionCalls.begin(); it != _functionCalls.end(); ++it) {
        ret << "    <functioncall functionName=\"" << ErrorLogger::toxml(*it) << "\"/>\n";
    }
    return ret.str();
}

namespace {
    struct Location {
        Location() : lineNumber(0) {}
        Location(const std::string &f, const int l) : fileName(f), lineNumber(l) {}
        std::string fileName;
        int lineNumber;
    };
}

void CheckUnusedFunctions::analyseWholeProgram(ErrorLogger * const errorLogger, const std::string &buildDir)
{
    std::map<std::string, Location> decls;
    std::set<std::string> calls;

    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt.c_str());
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        const std::string::size_type firstColon = filesTxtLine.find(':');
        if (firstColon == std::string::npos)
            continue;
        const std::string::size_type lastColon = filesTxtLine.rfind(':');
        if (firstColon == lastColon)
            continue;
        const std::string xmlfile = buildDir + '/' + filesTxtLine.substr(0,firstColon);
        const std::string sourcefile = filesTxtLine.substr(lastColon+1);

        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
        if (error != tinyxml2::XML_SUCCESS)
            continue;

        const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
        if (rootNode == nullptr)
            continue;

        for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "FileInfo") == 0) {
                const char *checkattr = e->Attribute("check");
                if (checkattr && std::strcmp(checkattr,"CheckUnusedFunctions")==0) {
                    for (const tinyxml2::XMLElement *e2 = e->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
                        if (!e2->Attribute("functionName"))
                            continue;
                        if (std::strcmp(e2->Name(),"functiondecl")==0 && e2->Attribute("lineNumber")) {
                            decls[e2->Attribute("functionName")] = Location(sourcefile, std::atoi(e2->Attribute("lineNumber")));
                        } else if (std::strcmp(e2->Name(),"functioncall")==0) {
                            calls.insert(e2->Attribute("functionName"));
                        }
                    }
                }
            }
        }
    }

    for (std::map<std::string, Location>::const_iterator decl = decls.begin(); decl != decls.end(); ++decl) {
        const std::string &functionName = decl->first;

        if (functionName == "main" || functionName == "WinMain" || functionName == "_tmain" ||
            functionName == "if")
            continue;

        if (calls.find(functionName) == calls.end()) {
            const Location &loc = decl->second;
            unusedFunctionError(errorLogger, loc.fileName, loc.lineNumber, functionName);
        }
    }
}
