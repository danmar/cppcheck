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

static const char ATTR_CALL_ID[] = "call-id";
static const char ATTR_CALL_FUNCNAME[] = "call-funcname";
static const char ATTR_CALL_ARGNR[] = "call-argnr";
static const char ATTR_CALL_ARGEXPR[] = "call-argexpr";
static const char ATTR_CALL_ARGVALUETYPE[] = "call-argvaluetype";
static const char ATTR_CALL_ARGVALUE[] = "call-argvalue";
static const char ATTR_WARNING[] = "warning";
static const char ATTR_LOC_FILENAME[] = "filename";
static const char ATTR_LOC_LINENR[] = "linenr";
static const char ATTR_INFO[] = "info";
static const char ATTR_MY_ID[] = "my-id";
static const char ATTR_MY_ARGNR[] = "my-argnr";
static const char ATTR_MY_ARGNAME[] = "my-argname";


std::string CTU::getFunctionId(const Tokenizer *tokenizer, const Function *function)
{
    return tokenizer->list.file(function->tokenDef) + ':' + MathLib::toString(function->tokenDef->linenr()) + ':' + MathLib::toString(function->tokenDef->col());
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
        out << functionCall.toXmlString();
    }

    // Nested calls..
    for (const CTU::FileInfo::NestedCall &nestedCall : nestedCalls) {
        out << nestedCall.toXmlString() << "\n";
    }

    return out.str();
}

std::string CTU::FileInfo::CallBase::toBaseXmlString() const
{
    std::ostringstream out;
    out << " " << ATTR_CALL_ID << "=\"" << callId << "\""
        << " " << ATTR_CALL_FUNCNAME << "=\"" << callFunctionName << "\""
        << " " << ATTR_CALL_ARGNR << "=\"" << callArgNr << "\""
        << " " << ATTR_LOC_FILENAME << "=\"" << location.fileName << "\""
        << " " << ATTR_LOC_LINENR << "=\"" << location.linenr << "\"";
    return out.str();
}

std::string CTU::FileInfo::FunctionCall::toXmlString() const
{
    std::ostringstream out;
    out << "<function-call"
        << toBaseXmlString()
        << " " << ATTR_CALL_ARGEXPR << "=\"" << callArgumentExpression << "\""
        << " " << ATTR_CALL_ARGVALUETYPE << "=\"" << callValueType << "\""
        << " " << ATTR_CALL_ARGVALUE << "=\"" << callArgValue << "\"";
    if (warning)
        out << " " << ATTR_WARNING << "=\"true\"";
    if (callValuePath.empty())
        out << "/>";
    else {
        out << ">\n";
        for (const ErrorLogger::ErrorMessage::FileLocation &loc : callValuePath)
            out << "  <path"
                << " " << ATTR_LOC_FILENAME << "=\"" << loc.getfile() << "\""
                << " " << ATTR_LOC_LINENR << "=\"" << loc.line << "\""
                << " " << ATTR_INFO << "=\"" << loc.getinfo() << "\"/>\n";
        out << "</function-call>";
    }
    return out.str();
}

std::string CTU::FileInfo::NestedCall::toXmlString() const
{
    std::ostringstream out;
    out << "<function-call"
        << toBaseXmlString()
        << " " << ATTR_MY_ID << "=\"" << myId << "\""
        << " " << ATTR_MY_ARGNR << "=\"" << myArgNr << "\""
        << "/>";
    return out.str();
}

std::string CTU::FileInfo::UnsafeUsage::toString() const
{
    std::ostringstream out;
    out << "    <unsafe-usage"
        << " " << ATTR_MY_ID << "=\"" << myId << '\"'
        << " " << ATTR_MY_ARGNR << "=\"" << myArgNr << '\"'
        << " " << ATTR_MY_ARGNAME << "=\"" << myArgumentName << '\"'
        << " " << ATTR_LOC_FILENAME << "=\"" << location.fileName << '\"'
        << " " << ATTR_LOC_LINENR << "=\"" << location.linenr << '\"'
        << "/>\n";
    return out.str();
}

std::string CTU::toString(const std::list<CTU::FileInfo::UnsafeUsage> &unsafeUsage)
{
    std::ostringstream ret;
    for (const CTU::FileInfo::UnsafeUsage &u : unsafeUsage)
        ret << u.toString();
    return ret.str();
}

CTU::FileInfo::CallBase::CallBase(const Tokenizer *tokenizer, const Token *callToken)
    : callId(getFunctionId(tokenizer, callToken->function()))
    , callArgNr(0)
    , callFunctionName(callToken->next()->astOperand1()->expressionString())
    , location(CTU::FileInfo::Location(tokenizer, callToken))
{
}

CTU::FileInfo::NestedCall::NestedCall(const Tokenizer *tokenizer, const Function *myFunction, const Token *callToken)
    : CallBase(tokenizer, callToken)
    , myId(getFunctionId(tokenizer, myFunction))
    , myArgNr(0)
{
}

static std::string readAttrString(const tinyxml2::XMLElement *e, const char *attr, bool *error)
{
    const char *value = e->Attribute(attr);
    if (!value && error)
        *error = true;
    return value ? value : "";
}

static long long readAttrInt(const tinyxml2::XMLElement *e, const char *attr, bool *error)
{
    const char *value = e->Attribute(attr);
    if (!value && error)
        *error = true;
    return value ? std::atoi(value) : 0;
}

bool CTU::FileInfo::CallBase::loadBaseFromXml(const tinyxml2::XMLElement *e)
{
    bool error = false;
    callId = readAttrString(e, ATTR_CALL_ID, &error);
    callFunctionName = readAttrString(e, ATTR_CALL_FUNCNAME, &error);
    callArgNr = readAttrInt(e, ATTR_CALL_ARGNR, &error);
    location.fileName = readAttrString(e, ATTR_LOC_FILENAME, &error);
    location.linenr = readAttrInt(e, ATTR_LOC_LINENR, &error);
    return !error;
}

bool CTU::FileInfo::FunctionCall::loadFromXml(const tinyxml2::XMLElement *e)
{
    if (!loadBaseFromXml(e))
        return false;
    bool error=false;
    callArgumentExpression = readAttrString(e, ATTR_CALL_ARGEXPR, &error);
    callValueType = (ValueFlow::Value::ValueType)readAttrInt(e, ATTR_CALL_ARGVALUETYPE, &error);
    callArgValue = readAttrInt(e, ATTR_CALL_ARGVALUE, &error);
    const char *w = e->Attribute(ATTR_WARNING);
    warning = w && std::strcmp(w, "true") == 0;
    for (const tinyxml2::XMLElement *e2 = e->FirstChildElement(); !error && e2; e2 = e2->NextSiblingElement()) {
        if (std::strcmp(e2->Name(), "path") != 0)
            continue;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.setfile(readAttrString(e2, ATTR_LOC_FILENAME, &error));
        loc.line = readAttrInt(e2, ATTR_LOC_LINENR, &error);
        loc.setinfo(readAttrString(e2, ATTR_INFO, &error));
    }
    return !error;
}

bool CTU::FileInfo::NestedCall::loadFromXml(const tinyxml2::XMLElement *e)
{
    if (!loadBaseFromXml(e))
        return false;
    bool error = false;
    myId = readAttrString(e, ATTR_MY_ID, &error);
    myArgNr = readAttrInt(e, ATTR_MY_ARGNR, &error);
    return !error;
}

void CTU::FileInfo::loadFromXml(const tinyxml2::XMLElement *xmlElement)
{
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "function-call") == 0) {
            FunctionCall functionCall;
            if (functionCall.loadFromXml(e))
                functionCalls.push_back(functionCall);
        } else if (std::strcmp(e->Name(), "nested-call") == 0) {
            NestedCall nestedCall;
            if (nestedCall.loadFromXml(e))
                nestedCalls.push_back(nestedCall);
        }
    }
}

std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> CTU::FileInfo::getCallsMap() const
{
    std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> ret;
    for (const CTU::FileInfo::NestedCall &nc : nestedCalls)
        ret[nc.callId].push_back(&nc);
    for (const CTU::FileInfo::FunctionCall &fc : functionCalls)
        ret[fc.callId].push_back(&fc);
    return ret;
}

std::list<CTU::FileInfo::UnsafeUsage> CTU::loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement)
{
    std::list<CTU::FileInfo::UnsafeUsage> ret;
    for (const tinyxml2::XMLElement *e = xmlElement->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "unsafe-usage") != 0)
            continue;
        bool error = false;
        FileInfo::UnsafeUsage unsafeUsage;
        unsafeUsage.myId = readAttrString(e, ATTR_MY_ID, &error);
        unsafeUsage.myArgNr = readAttrInt(e, ATTR_MY_ARGNR, &error);
        unsafeUsage.myArgumentName = readAttrString(e, ATTR_MY_ARGNAME, &error);
        unsafeUsage.location.fileName = readAttrString(e, ATTR_LOC_FILENAME, &error);
        unsafeUsage.location.linenr = readAttrInt(e, ATTR_LOC_LINENR, &error);
        if (!error)
            ret.push_back(unsafeUsage);
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
                for (const ValueFlow::Value &value : argtok->values()) {
                    if (!value.isIntValue() || value.intvalue != 0 || value.isInconclusive())
                        continue;
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = ValueFlow::Value::INT;
                    functionCall.callId = getFunctionId(tokenizer, tok->astOperand1()->function());
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location.fileName = tokenizer->list.file(tok);
                    functionCall.location.linenr = tok->linenr();
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    functionCall.callArgValue = value.intvalue;
                    functionCall.warning = !value.errorSeverity();
                    for (const ErrorPathItem &i : value.errorPath) {
                        ErrorLogger::ErrorMessage::FileLocation loc;
                        loc.setfile(tokenizer->list.file(i.first));
                        loc.line = i.first->linenr();
                        loc.setinfo(i.second);
                        functionCall.callValuePath.push_back(loc);
                    }
                    fileInfo->functionCalls.push_back(functionCall);
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
                    FileInfo::FunctionCall functionCall;
                    functionCall.callValueType = ValueFlow::Value::UNINIT;
                    functionCall.callId = getFunctionId(tokenizer, tok->astOperand1()->function());
                    functionCall.callFunctionName = tok->astOperand1()->expressionString();
                    functionCall.location.fileName = tokenizer->list.file(tok);
                    functionCall.location.linenr = tok->linenr();
                    functionCall.callArgNr = argnr + 1;
                    functionCall.callArgValue = 0;
                    functionCall.callArgumentExpression = argtok->expressionString();
                    functionCall.warning = false;
                    fileInfo->functionCalls.push_back(functionCall);
                    continue;
                }
            }
        }

        // Nested function calls
        for (int argnr = 0; argnr < function->argCount(); ++argnr) {
            const Token *tok;
            const int argnr2 = isCallFunction(&scope, argnr, &tok);
            if (argnr2 > 0) {
                FileInfo::NestedCall nestedCall(tokenizer, function, tok);
                nestedCall.myArgNr = argnr + 1;
                nestedCall.callArgNr = argnr2;
                fileInfo->nestedCalls.push_back(nestedCall);
            }
        }
    }

    return fileInfo;
}

static bool isUnsafeFunction(const Tokenizer *tokenizer, const Settings *settings, const Scope *scope, int argnr, const Token **tok, const Check *check, bool (*isUnsafeUsage)(const Check *check, const Token *argtok))
{
    const Variable * const argvar = scope->function->getArgumentVar(argnr);
    if (!argvar->isPointer())
        return false;
    for (const Token *tok2 = scope->bodyStart; tok2 != scope->bodyEnd; tok2 = tok2->next()) {
        if (Token::Match(tok2, ")|else {")) {
            tok2 = tok2->linkAt(1);
            if (Token::findmatch(tok2->link(), "return|throw", tok2))
                return false;
            if (isVariableChanged(tok2->link(), tok2, argvar->declarationId(), false, settings, tokenizer->isCPP()))
                return false;
        }
        if (Token::Match(tok2, "%oror%|&&|?")) {
            tok2 = tok2->findExpressionStartEndTokens().second;
            continue;
        }
        if (tok2->variable() != argvar)
            continue;
        if (!isUnsafeUsage(check, tok2))
            return false;
        *tok = tok2;
        return true;
    }
    return false;
}

std::list<CTU::FileInfo::UnsafeUsage> CTU::getUnsafeUsage(const Tokenizer *tokenizer, const Settings *settings, const Check *check, bool (*isUnsafeUsage)(const Check *check, const Token *argtok))
{
    std::list<CTU::FileInfo::UnsafeUsage> unsafeUsage;

    // Parse all functions in TU
    const SymbolDatabase * const symbolDatabase = tokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (!scope.isExecutable() || scope.type != Scope::eFunction || !scope.function)
            continue;
        const Function *const function = scope.function;

        // "Unsafe" functions unconditionally reads data before it is written..
        for (int argnr = 0; argnr < function->argCount(); ++argnr) {
            const Token *tok;
            if (isUnsafeFunction(tokenizer, settings, &scope, argnr, &tok, check, isUnsafeUsage))
                unsafeUsage.push_back(CTU::FileInfo::UnsafeUsage(CTU::getFunctionId(tokenizer, function), argnr+1, tok->str(), CTU::FileInfo::Location(tokenizer,tok)));
        }
    }

    return unsafeUsage;
}

static bool findPath(const std::string &callId,
                     unsigned int callArgNr,
                     CTU::FileInfo::InvalidValueType invalidValue,
                     const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap,
                     const CTU::FileInfo::CallBase *path[10],
                     int index,
                     bool warning)
{
    if (index >= 10)
        return false;

    const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>>::const_iterator it = callsMap.find(callId);
    if (it == callsMap.end())
        return false;

    for (const CTU::FileInfo::CallBase *c : it->second) {
        if (c->callArgNr != callArgNr)
            continue;

        const CTU::FileInfo::FunctionCall *functionCall = dynamic_cast<const CTU::FileInfo::FunctionCall *>(c);
        if (functionCall) {
            if (!warning && functionCall->warning)
                continue;
            switch (invalidValue) {
            case CTU::FileInfo::InvalidValueType::null:
                if (functionCall->callValueType != ValueFlow::Value::INT || functionCall->callArgValue != 0)
                    continue;
                break;
            case CTU::FileInfo::InvalidValueType::uninit:
                if (functionCall->callValueType != ValueFlow::Value::UNINIT)
                    continue;
                break;
            };
            path[index] = functionCall;
            return true;
        }

        const CTU::FileInfo::NestedCall *nestedCall = dynamic_cast<const CTU::FileInfo::NestedCall *>(c);
        if (!nestedCall)
            continue;

        if (findPath(nestedCall->myId, nestedCall->myArgNr, invalidValue, callsMap, path, index + 1, warning)) {
            path[index] = nestedCall;
            return true;
        }
    }

    return false;
}

std::list<ErrorLogger::ErrorMessage::FileLocation> CTU::FileInfo::getErrorPath(InvalidValueType invalidValue,
        const CTU::FileInfo::UnsafeUsage &unsafeUsage,
        const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap,
        const char info[],
        const FunctionCall * * const functionCallPtr,
        bool warning) const
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;

    const CTU::FileInfo::CallBase *path[10] = {0};

    if (!findPath(unsafeUsage.myId, unsafeUsage.myArgNr, invalidValue, callsMap, path, 0, warning))
        return locationList;

    const std::string value1 = (invalidValue == InvalidValueType::null) ? "null" : "uninitialized";

    for (int index = 9; index >= 0; index--) {
        if (!path[index])
            continue;

        const CTU::FileInfo::FunctionCall *functionCall = dynamic_cast<const CTU::FileInfo::FunctionCall *>(path[index]);

        if (functionCall) {
            if (functionCallPtr)
                *functionCallPtr = functionCall;
            for (const ErrorLogger::ErrorMessage::FileLocation &loc : functionCall->callValuePath)
                locationList.push_back(loc);
        }

        ErrorLogger::ErrorMessage::FileLocation fileLoc;
        fileLoc.setfile(path[index]->location.fileName);
        fileLoc.line = path[index]->location.linenr;
        fileLoc.setinfo("Calling function " + path[index]->callFunctionName + ", " + MathLib::toString(path[index]->callArgNr) + getOrdinalText(path[index]->callArgNr) + " argument is " + value1);
        locationList.push_back(fileLoc);
    }

    ErrorLogger::ErrorMessage::FileLocation fileLoc2;
    fileLoc2.setfile(unsafeUsage.location.fileName);
    fileLoc2.line = unsafeUsage.location.linenr;
    fileLoc2.setinfo(replaceStr(info, "ARG", unsafeUsage.myArgumentName));
    locationList.push_back(fileLoc2);

    return locationList;
}
