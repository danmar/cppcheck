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


//---------------------------------------------------------------------------
#include "ctu.h"
#include "astutils.h"
#include "symboldatabase.h"
#include <tinyxml2.h>
//---------------------------------------------------------------------------

std::string CTU::getFunctionId(const Tokenizer *tokenizer, const Function *function)
{
    return tokenizer->list.file(function->tokenDef) + ':' + MathLib::toString(function->tokenDef->linenr());
}

bool CTU::findPath(const CTU::FileInfo::FunctionCall &from,
                   const CTU::FileInfo::UnsafeUsage &to,
                   const std::map<std::string, std::list<CTU::FileInfo::NestedCall>> &nestedCalls)
{
    if (from.functionId == to.functionId && from.argnr == to.argnr)
        return true;

    const std::map<std::string, std::list<FileInfo::NestedCall>>::const_iterator nc = nestedCalls.find(from.functionId);
    if (nc == nestedCalls.end())
        return false;

    for (std::list<FileInfo::NestedCall>::const_iterator it = nc->second.begin(); it != nc->second.end(); ++it) {
        if (from.functionId == it->id && from.argnr == it->argnr && it->id2 == to.functionId && it->argnr2 == to.argnr)
            return true;
    }

    return false;
}



CTU::FileInfo::Location::Location(const Tokenizer *tokenizer, const Token *tok)
{
    fileName = tokenizer->list.file(tok);
    linenr = tok->linenr();
}

std::string CTU::FileInfo::toString() const
{
    std::ostringstream out;

    // Function calls..
    for (const CTU::FileInfo::FunctionCall &functionCall : functionCalls) {
        out << "    <function-call"
            << " id=\"" << functionCall.functionId << '\"'
            << " functionName=\"" << functionCall.functionName << '\"'
            << " argnr=\"" << functionCall.argnr << '\"'
            << " argExpr=\"" << functionCall.argumentExpression << '\"'
            << " argvalue=\"" << functionCall.argvalue << '\"'
            << " valueType=\"" << functionCall.valueType << '\"'
            << " fileName=\"" << functionCall.location.fileName << '\"'
            << " linenr=\"" << functionCall.location.linenr << '\"'
            << "/>\n";
    }

    // Nested calls..
    for (const CTU::FileInfo::NestedCall &nestedCall : nestedCalls) {
        out << "    <nested-call"
            << " id=\"" << nestedCall.id << '\"'
            << " id2=\"" << nestedCall.id2 << '\"'
            << " functionName=\"" << nestedCall.functionName << '\"'
            << " argnr=\"" << nestedCall.argnr << '\"'
            << " argnr2=\"" << nestedCall.argnr2 << '\"'
            << " fileName=\"" << nestedCall.location.fileName << '\"'
            << " linenr=\"" << nestedCall.location.linenr << '\"'
            << "/>\n";
    }

    return out.str();
}

std::string CTU::FileInfo::UnsafeUsage::toString() const
{
    std::ostringstream out;
    out << "    <unsafe-usage"
        << " id=\"" << functionId << '\"'
        << " argnr=\"" << argnr << '\"'
        << " argname=\"" << argumentName << '\"'
        << " fileName=\"" << location.fileName << '\"'
        << " linenr=\"" << location.linenr << '\"'
        << "/>\n";
    return out.str();
}

CTU::FileInfo::NestedCall::NestedCall(const Tokenizer *tokenizer, const Scope *scope, unsigned int argnr_, const Token *tok)
    :
    id(getFunctionId(tokenizer, scope->function)),
    functionName(scope->className),
    argnr(argnr_),
    argnr2(0),
    location(CTU::FileInfo::Location(tokenizer, tok))
{
}

void CTU::FileInfo::loadFromXml(const tinyxml2::XMLElement *xmlElement)
{
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        const char *id = e->Attribute("id");
        if (!id)
            continue;
        const char *functionName = e->Attribute("functionName");
        if (!functionName)
            continue;
        const char *argnr = e->Attribute("argnr");
        if (!argnr || !MathLib::isInt(argnr))
            continue;
        const char *fileName = e->Attribute("fileName");
        if (!fileName)
            continue;
        const char *linenr = e->Attribute("linenr");
        if (!linenr || !MathLib::isInt(linenr))
            continue;

        if (std::strcmp(e->Name(), "function-call") == 0) {
            FunctionCall functionCall;
            functionCall.functionId = id;
            functionCall.functionName = functionName;
            functionCall.argnr = std::atoi(argnr);
            const char *argExpr = e->Attribute("argExpr");
            if (!argExpr)
                continue;
            functionCall.argumentExpression =
                functionCall.valueType = (ValueFlow::Value::ValueType)std::atoi(e->Attribute("valueType"));
            functionCall.argvalue = MathLib::toLongNumber(e->Attribute("argvalue"));
            functionCall.location.fileName = fileName;
            functionCall.location.linenr = std::atoi(linenr);
            functionCalls.push_back(functionCall);
        } else if (std::strcmp(e->Name(), "nested-call") == 0) {
            NestedCall nestedCall;
            nestedCall.functionName = functionName;
            nestedCall.id = id;
            const char *id2 = e->Attribute("id2");
            if (!id2)
                continue;
            nestedCall.id2 = id2;
            nestedCall.argnr = std::atoi(argnr);
            const char *argnr2 = e->Attribute("argnr2");
            if (!argnr2 || !MathLib::isInt(argnr2))
                continue;
            nestedCall.argnr2 = std::atoi(argnr2);
            nestedCall.location.fileName = fileName;
            nestedCall.location.linenr = std::atoi(linenr);
            nestedCalls.push_back(nestedCall);
        }
    }
}

std::map<std::string, std::list<CTU::FileInfo::NestedCall>> CTU::FileInfo::getNestedCallsMap() const
{
    std::map<std::string, std::list<CTU::FileInfo::NestedCall>> ret;
    for (const CTU::FileInfo::NestedCall &nc : nestedCalls)
        ret[nc.id].push_back(nc);
    return ret;
}

std::list<CTU::FileInfo::UnsafeUsage> CTU::loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement)
{
    std::list<CTU::FileInfo::UnsafeUsage> ret;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "unsafe-usage") != 0)
            continue;
        const char *id = e->Attribute("id");
        if (!id)
            continue;
        const char *argnr = e->Attribute("argnr");
        if (!argnr || !MathLib::isInt(argnr))
            continue;
        const char *argname = e->Attribute("argname");
        if (!argname)
            continue;
        const char *fileName = e->Attribute("fileName");
        if (!fileName)
            continue;
        const char *linenr = e->Attribute("linenr");
        if (!linenr || !MathLib::isInt(linenr))
            continue;
        ret.push_back(FileInfo::UnsafeUsage(id, std::atoi(argnr), argname, FileInfo::Location(fileName, std::atoi(linenr))));
    }
    return ret;
}

static int isCallFunction(const Scope *scope, int argnr, const Token **tok)
{
    const Variable * const argvar = scope->function->getArgumentVar(argnr);
    if (!argvar->isPointer())
        return -1;
    for (const Token *tok2 = scope->bodyStart; tok2 != scope->bodyEnd; tok2 = tok2->next()) {
        if (tok2->variable() != argvar)
            continue;
        if (!Token::Match(tok2->previous(), "[(,] %var% [,)]"))
            break;
        int argnr2 = 1;
        const Token *prev = tok2;
        while (prev && prev->str() != "(") {
            if (Token::Match(prev,"]|)"))
                prev = prev->link();
            else if (prev->str() == ",")
                ++argnr2;
            prev = prev->previous();
        }
        if (!prev || !Token::Match(prev->previous(), "%name% ("))
            break;
        if (!prev->astOperand1() || !prev->astOperand1()->function())
            break;
        *tok = prev->previous();
        return argnr2;
    }
    return -1;
}


CTU::FileInfo *CTU::getFileInfo(const Tokenizer *tokenizer)
{
    const SymbolDatabase * const symbolDatabase = tokenizer->getSymbolDatabase();

    FileInfo *fileInfo = new FileInfo;

    // Parse all functions in TU
    for (const Scope &scope : symbolDatabase->scopeList) {
        if (!scope.isExecutable() || scope.type != Scope::eFunction || !scope.function)
            continue;
        const Function *const function = scope.function;

        // source function calls
        for (const Token *tok = scope.bodyStart; tok != scope.bodyEnd; tok = tok->next()) {
            if (tok->str() != "(" || !tok->astOperand1() || !tok->astOperand2())
                continue;
            if (!tok->astOperand1()->function())
                continue;
            const std::vector<const Token *> args(getArguments(tok->previous()));
            for (int argnr = 0; argnr < args.size(); ++argnr) {
                const Token *argtok = args[argnr];
                if (!argtok)
                    continue;
                if (argtok->hasKnownIntValue()) {
                    struct FileInfo::FunctionCall functionCall;
                    functionCall.valueType = ValueFlow::Value::INT;
                    functionCall.functionId = getFunctionId(tokenizer, tok->astOperand1()->function());
                    functionCall.functionName = tok->astOperand1()->expressionString();
                    functionCall.location.fileName = tokenizer->list.file(tok);
                    functionCall.location.linenr = tok->linenr();
                    functionCall.argnr = argnr + 1;
                    functionCall.argumentExpression = argtok->expressionString();
                    functionCall.argvalue = argtok->values().front().intvalue;
                    fileInfo->functionCalls.push_back(functionCall);
                    continue;
                }
                // pointer to uninitialized data..
                if (!argtok->isUnaryOp("&"))
                    continue;
                argtok = argtok->astOperand1();
                if (!argtok || !argtok->valueType() || argtok->valueType()->pointer != 0)
                    continue;
                if (argtok->values().size() != 1U)
                    continue;
                const ValueFlow::Value &v = argtok->values().front();
                if (v.valueType == ValueFlow::Value::UNINIT && !v.isInconclusive()) {
                    struct FileInfo::FunctionCall functionCall;
                    functionCall.valueType = ValueFlow::Value::UNINIT;
                    functionCall.functionId = getFunctionId(tokenizer, tok->astOperand1()->function());
                    functionCall.functionName = tok->astOperand1()->expressionString();
                    functionCall.location.fileName = tokenizer->list.file(tok);
                    functionCall.location.linenr = tok->linenr();
                    functionCall.argnr = argnr + 1;
                    functionCall.argvalue = 0;
                    functionCall.argumentExpression = argtok->expressionString();
                    fileInfo->functionCalls.push_back(functionCall);
                    continue;
                }
            }
        }

        // Nested function calls
        for (int argnr = 0; argnr < function->argCount(); ++argnr) {
            const Token *tok;
            int argnr2 = isCallFunction(&scope, argnr, &tok);
            if (argnr2 > 0) {
                FileInfo::NestedCall nestedCall(tokenizer, &scope, argnr+1, tok);
                nestedCall.id  = getFunctionId(tokenizer, function);
                nestedCall.id2 = getFunctionId(tokenizer, tok->function());
                nestedCall.argnr2 = argnr2;
                fileInfo->nestedCalls.push_back(nestedCall);
            }
        }
    }

    return fileInfo;
}
