/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "library.h"

#include "astutils.h"
#include "mathlib.h"
#include "path.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstring>
#include <list>
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <stack>
#include <stdexcept>
#include <string>

#include <tinyxml2.h>

static std::vector<std::string> getnames(const char *names)
{
    std::vector<std::string> ret;
    while (const char *p = std::strchr(names,',')) {
        ret.emplace_back(names, p-names);
        names = p + 1;
    }
    ret.emplace_back(names);
    return ret;
}

static void gettokenlistfromvalid(const std::string& valid, TokenList& tokenList)
{
    std::istringstream istr(valid + ',');
    tokenList.createTokens(istr);
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok,"- %num%")) {
            tok->str("-" + tok->strAt(1));
            tok->deleteNext();
        }
    }
}

Library::Error Library::load(const char exename[], const char path[])
{
    if (std::strchr(path,',') != nullptr) {
        std::string p(path);
        for (;;) {
            const std::string::size_type pos = p.find(',');
            if (pos == std::string::npos)
                break;
            const Error &e = load(exename, p.substr(0,pos).c_str());
            if (e.errorcode != ErrorCode::OK)
                return e;
            p = p.substr(pos+1);
        }
        if (!p.empty())
            return load(exename, p.c_str());
        return Error();
    }

    std::string absolute_path;
    // open file..
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(path);
    if (error == tinyxml2::XML_ERROR_FILE_READ_ERROR && Path::getFilenameExtension(path).empty())
        // Reading file failed, try again...
        error = tinyxml2::XML_ERROR_FILE_NOT_FOUND;
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
        // failed to open file.. is there no extension?
        std::string fullfilename(path);
        if (Path::getFilenameExtension(fullfilename).empty()) {
            fullfilename += ".cfg";
            error = doc.LoadFile(fullfilename.c_str());
            if (error != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
                absolute_path = Path::getAbsoluteFilePath(fullfilename);
        }

        std::list<std::string> cfgfolders;
#ifdef FILESDIR
        cfgfolders.emplace_back(FILESDIR "/cfg");
#endif
        if (exename) {
            const std::string exepath(Path::fromNativeSeparators(Path::getPathFromFilename(Path::getCurrentExecutablePath(exename))));
            cfgfolders.push_back(exepath + "cfg");
            cfgfolders.push_back(exepath + "../cfg");
            cfgfolders.push_back(exepath);
        }

        while (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND && !cfgfolders.empty()) {
            const std::string cfgfolder(cfgfolders.back());
            cfgfolders.pop_back();
            const char *sep = (!cfgfolder.empty() && endsWith(cfgfolder,'/') ? "" : "/");
            const std::string filename(cfgfolder + sep + fullfilename);
            error = doc.LoadFile(filename.c_str());
            if (error != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
                absolute_path = Path::getAbsoluteFilePath(filename);
        }
    } else
        absolute_path = Path::getAbsoluteFilePath(path);

    if (error == tinyxml2::XML_SUCCESS) {
        if (mFiles.find(absolute_path) == mFiles.end()) {
            Error err = load(doc);
            if (err.errorcode == ErrorCode::OK)
                mFiles.insert(absolute_path);
            return err;
        }

        return Error(ErrorCode::OK); // ignore duplicates
    }

    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        return Error(ErrorCode::FILE_NOT_FOUND);

    doc.PrintError();
    return Error(ErrorCode::BAD_XML);
}

Library::Container::Yield Library::Container::yieldFrom(const std::string& yieldName)
{
    if (yieldName == "at_index")
        return Container::Yield::AT_INDEX;
    if (yieldName == "item")
        return Container::Yield::ITEM;
    if (yieldName == "buffer")
        return Container::Yield::BUFFER;
    if (yieldName == "buffer-nt")
        return Container::Yield::BUFFER_NT;
    if (yieldName == "start-iterator")
        return Container::Yield::START_ITERATOR;
    if (yieldName == "end-iterator")
        return Container::Yield::END_ITERATOR;
    if (yieldName == "iterator")
        return Container::Yield::ITERATOR;
    if (yieldName == "size")
        return Container::Yield::SIZE;
    if (yieldName == "empty")
        return Container::Yield::EMPTY;
    return Container::Yield::NO_YIELD;
}
Library::Container::Action Library::Container::actionFrom(const std::string& actionName)
{
    if (actionName == "resize")
        return Container::Action::RESIZE;
    if (actionName == "clear")
        return Container::Action::CLEAR;
    if (actionName == "push")
        return Container::Action::PUSH;
    if (actionName == "pop")
        return Container::Action::POP;
    if (actionName == "find")
        return Container::Action::FIND;
    if (actionName == "insert")
        return Container::Action::INSERT;
    if (actionName == "erase")
        return Container::Action::ERASE;
    if (actionName == "change-content")
        return Container::Action::CHANGE_CONTENT;
    if (actionName == "change-internal")
        return Container::Action::CHANGE_INTERNAL;
    if (actionName == "change")
        return Container::Action::CHANGE;
    return Container::Action::NO_ACTION;
}

// cppcheck-suppress unusedFunction - only used in unit tests
bool Library::loadxmldata(const char xmldata[], std::size_t len)
{
    tinyxml2::XMLDocument doc;
    return (tinyxml2::XML_SUCCESS == doc.Parse(xmldata, len)) && (load(doc).errorcode == ErrorCode::OK);
}

Library::Error Library::load(const tinyxml2::XMLDocument &doc)
{
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (rootnode == nullptr) {
        doc.PrintError();
        return Error(ErrorCode::BAD_XML);
    }

    if (strcmp(rootnode->Name(),"def") != 0)
        return Error(ErrorCode::UNSUPPORTED_FORMAT, rootnode->Name());

    const int format = rootnode->IntAttribute("format", 1); // Assume format version 1 if nothing else is specified (very old .cfg files had no 'format' attribute)

    if (format > 2 || format <= 0)
        return Error(ErrorCode::UNSUPPORTED_FORMAT);

    std::set<std::string> unknown_elements;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const std::string nodename = node->Name();
        if (nodename == "memory" || nodename == "resource") {
            // get allocationId to use..
            int allocationId = 0;
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"dealloc")==0) {
                    const std::map<std::string, AllocFunc>::const_iterator it = mDealloc.find(memorynode->GetText());
                    if (it != mDealloc.end()) {
                        allocationId = it->second.groupId;
                        break;
                    }
                }
            }
            if (allocationId == 0) {
                if (nodename == "memory")
                    while (!ismemory(++mAllocId));
                else
                    while (!isresource(++mAllocId));
                allocationId = mAllocId;
            }

            // add alloc/dealloc/use functions..
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                const std::string memorynodename = memorynode->Name();
                if (memorynodename == "alloc" || memorynodename == "realloc") {
                    AllocFunc temp = {0};
                    temp.groupId = allocationId;

                    temp.initData = memorynode->BoolAttribute("init", true);
                    temp.arg = memorynode->IntAttribute("arg", -1);

                    const char *bufferSize = memorynode->Attribute("buffer-size");
                    if (!bufferSize)
                        temp.bufferSize = AllocFunc::BufferSize::none;
                    else {
                        if (std::strncmp(bufferSize, "malloc", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::malloc;
                        else if (std::strncmp(bufferSize, "calloc", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::calloc;
                        else if (std::strncmp(bufferSize, "strdup", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::strdup;
                        else
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, bufferSize);
                        temp.bufferSizeArg1 = 1;
                        temp.bufferSizeArg2 = 2;
                        if (bufferSize[6] == 0) {
                            // use default values
                        } else if (bufferSize[6] == ':' && bufferSize[7] >= '1' && bufferSize[7] <= '5') {
                            temp.bufferSizeArg1 = bufferSize[7] - '0';
                            if (bufferSize[8] == ',' && bufferSize[9] >= '1' && bufferSize[9] <= '5')
                                temp.bufferSizeArg2 = bufferSize[9] - '0';
                        } else
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, bufferSize);
                    }

                    if (memorynodename == "realloc")
                        temp.reallocArg = memorynode->IntAttribute("realloc-arg", 1);

                    if (memorynodename != "realloc")
                        mAlloc[memorynode->GetText()] = temp;
                    else
                        mRealloc[memorynode->GetText()] = temp;
                } else if (memorynodename == "dealloc") {
                    AllocFunc temp = {0};
                    temp.groupId = allocationId;
                    temp.arg = memorynode->IntAttribute("arg", 1);
                    mDealloc[memorynode->GetText()] = temp;
                } else if (memorynodename == "use")
                    functions[memorynode->GetText()].use = true;
                else
                    unknown_elements.insert(memorynodename);
            }
        }

        else if (nodename == "define") {
            const char *name = node->Attribute("name");
            if (name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
            defines.push_back(std::string(name) +
                              " " +
                              value);
        }

        else if (nodename == "function") {
            const char *name = node->Attribute("name");
            if (name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            for (const std::string &s : getnames(name)) {
                const Error &err = loadFunction(node, s, unknown_elements);
                if (err.errorcode != ErrorCode::OK)
                    return err;
            }
        }

        else if (nodename == "reflection") {
            for (const tinyxml2::XMLElement *reflectionnode = node->FirstChildElement(); reflectionnode; reflectionnode = reflectionnode->NextSiblingElement()) {
                if (strcmp(reflectionnode->Name(), "call") != 0) {
                    unknown_elements.insert(reflectionnode->Name());
                    continue;
                }

                const char * const argString = reflectionnode->Attribute("arg");
                if (!argString)
                    return Error(ErrorCode::MISSING_ATTRIBUTE, "arg");

                mReflection[reflectionnode->GetText()] = strToInt<int>(argString);
            }
        }

        else if (nodename == "markup") {
            const char * const extension = node->Attribute("ext");
            if (!extension)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "ext");
            mMarkupExtensions.insert(extension);

            mReportErrors[extension] = (node->Attribute("reporterrors", "true") != nullptr);
            mProcessAfterCode[extension] = (node->Attribute("aftercode", "true") != nullptr);

            for (const tinyxml2::XMLElement *markupnode = node->FirstChildElement(); markupnode; markupnode = markupnode->NextSiblingElement()) {
                const std::string markupnodename = markupnode->Name();
                if (markupnodename == "keywords") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "keyword") == 0) {
                            const char* nodeName = librarynode->Attribute("name");
                            if (nodeName == nullptr)
                                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
                            mKeywords[extension].insert(nodeName);
                        } else
                            unknown_elements.insert(librarynode->Name());
                    }
                }

                else if (markupnodename == "exported") {
                    for (const tinyxml2::XMLElement *exporter = markupnode->FirstChildElement(); exporter; exporter = exporter->NextSiblingElement()) {
                        if (strcmp(exporter->Name(), "exporter") != 0) {
                            unknown_elements.insert(exporter->Name());
                            continue;
                        }

                        const char * const prefix = exporter->Attribute("prefix");
                        if (!prefix)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "prefix");

                        for (const tinyxml2::XMLElement *e = exporter->FirstChildElement(); e; e = e->NextSiblingElement()) {
                            const std::string ename = e->Name();
                            if (ename == "prefix")
                                mExporters[prefix].addPrefix(e->GetText());
                            else if (ename == "suffix")
                                mExporters[prefix].addSuffix(e->GetText());
                            else
                                unknown_elements.insert(ename);
                        }
                    }
                }

                else if (markupnodename == "imported") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "importer") == 0)
                            mImporters[extension].insert(librarynode->GetText());
                        else
                            unknown_elements.insert(librarynode->Name());
                    }
                }

                else if (markupnodename == "codeblocks") {
                    for (const tinyxml2::XMLElement *blocknode = markupnode->FirstChildElement(); blocknode; blocknode = blocknode->NextSiblingElement()) {
                        const std::string blocknodename = blocknode->Name();
                        if (blocknodename == "block") {
                            const char * blockName = blocknode->Attribute("name");
                            if (blockName)
                                mExecutableBlocks[extension].addBlock(blockName);
                        } else if (blocknodename == "structure") {
                            const char * start = blocknode->Attribute("start");
                            if (start)
                                mExecutableBlocks[extension].setStart(start);
                            const char * end = blocknode->Attribute("end");
                            if (end)
                                mExecutableBlocks[extension].setEnd(end);
                            const char * offset = blocknode->Attribute("offset");
                            if (offset)
                                mExecutableBlocks[extension].setOffset(strToInt<int>(offset));
                        }

                        else
                            unknown_elements.insert(blocknodename);
                    }
                }

                else
                    unknown_elements.insert(markupnodename);
            }
        }

        else if (nodename == "container") {
            const char* const id = node->Attribute("id");
            if (!id)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "id");

            Container& container = containers[id];

            const char* const inherits = node->Attribute("inherits");
            if (inherits) {
                const std::unordered_map<std::string, Container>::const_iterator i = containers.find(inherits);
                if (i != containers.end())
                    container = i->second; // Take values from parent and overwrite them if necessary
                else
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, inherits);
            }

            const char* const startPattern = node->Attribute("startPattern");
            if (startPattern) {
                container.startPattern = startPattern;
                container.startPattern2 = container.startPattern + " !!::";
            }
            const char* const endPattern = node->Attribute("endPattern");
            if (endPattern)
                container.endPattern = endPattern;
            const char* const itEndPattern = node->Attribute("itEndPattern");
            if (itEndPattern)
                container.itEndPattern = itEndPattern;
            const char* const opLessAllowed = node->Attribute("opLessAllowed");
            if (opLessAllowed)
                container.opLessAllowed = strcmp(opLessAllowed, "true") == 0;
            const char* const hasInitializerListConstructor = node->Attribute("hasInitializerListConstructor");
            if (hasInitializerListConstructor)
                container.hasInitializerListConstructor = strcmp(hasInitializerListConstructor, "true") == 0;
            const char* const view = node->Attribute("view");
            if (view)
                container.view = strcmp(view, "true") == 0;

            for (const tinyxml2::XMLElement *containerNode = node->FirstChildElement(); containerNode; containerNode = containerNode->NextSiblingElement()) {
                const std::string containerNodeName = containerNode->Name();
                if (containerNodeName == "size" || containerNodeName == "access" || containerNodeName == "other") {
                    for (const tinyxml2::XMLElement *functionNode = containerNode->FirstChildElement(); functionNode; functionNode = functionNode->NextSiblingElement()) {
                        if (strcmp(functionNode->Name(), "function") != 0) {
                            unknown_elements.insert(functionNode->Name());
                            continue;
                        }

                        const char* const functionName = functionNode->Attribute("name");
                        if (!functionName)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "name");

                        const char* const action_ptr = functionNode->Attribute("action");
                        Container::Action action = Container::Action::NO_ACTION;
                        if (action_ptr) {
                            std::string actionName = action_ptr;
                            action = Container::actionFrom(actionName);
                            if (action == Container::Action::NO_ACTION)
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, actionName);
                        }

                        const char* const yield_ptr = functionNode->Attribute("yields");
                        Container::Yield yield = Container::Yield::NO_YIELD;
                        if (yield_ptr) {
                            std::string yieldName = yield_ptr;
                            yield = Container::yieldFrom(yieldName);
                            if (yield == Container::Yield::NO_YIELD)
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, yieldName);
                        }

                        const char* const returnType = functionNode->Attribute("returnType");
                        if (returnType)
                            container.functions[functionName].returnType = returnType;

                        container.functions[functionName].action = action;
                        container.functions[functionName].yield = yield;
                    }

                    if (containerNodeName == "size") {
                        const char* const templateArg = containerNode->Attribute("templateParameter");
                        if (templateArg)
                            container.size_templateArgNo = strToInt<int>(templateArg);
                    } else if (containerNodeName == "access") {
                        const char* const indexArg = containerNode->Attribute("indexOperator");
                        if (indexArg)
                            container.arrayLike_indexOp = strcmp(indexArg, "array-like") == 0;
                    }
                } else if (containerNodeName == "type") {
                    const char* const templateArg = containerNode->Attribute("templateParameter");
                    if (templateArg)
                        container.type_templateArgNo = strToInt<int>(templateArg);

                    const char* const string = containerNode->Attribute("string");
                    if (string)
                        container.stdStringLike = strcmp(string, "std-like") == 0;
                    const char* const associative = containerNode->Attribute("associative");
                    if (associative)
                        container.stdAssociativeLike = strcmp(associative, "std-like") == 0;
                    const char* const unstable = containerNode->Attribute("unstable");
                    if (unstable) {
                        std::string unstableType = unstable;
                        if (unstableType.find("erase") != std::string::npos)
                            container.unstableErase = true;
                        if (unstableType.find("insert") != std::string::npos)
                            container.unstableInsert = true;
                    }
                } else if (containerNodeName == "rangeItemRecordType") {
                    for (const tinyxml2::XMLElement* memberNode = node->FirstChildElement(); memberNode; memberNode = memberNode->NextSiblingElement()) {
                        const char *memberName = memberNode->Attribute("name");
                        const char *memberTemplateParameter = memberNode->Attribute("templateParameter");
                        struct Container::RangeItemRecordTypeItem member;
                        member.name = memberName ? memberName : "";
                        member.templateParameter = memberTemplateParameter ? strToInt<int>(memberTemplateParameter) : -1;
                        container.rangeItemRecordType.emplace_back(std::move(member));
                    }
                } else
                    unknown_elements.insert(containerNodeName);
            }
        }

        else if (nodename == "smart-pointer") {
            const char *className = node->Attribute("class-name");
            if (!className)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "class-name");
            SmartPointer& smartPointer = smartPointers[className];
            smartPointer.name = className;
            for (const tinyxml2::XMLElement* smartPointerNode = node->FirstChildElement(); smartPointerNode;
                 smartPointerNode = smartPointerNode->NextSiblingElement()) {
                const std::string smartPointerNodeName = smartPointerNode->Name();
                if (smartPointerNodeName == "unique")
                    smartPointer.unique = true;
            }
        }

        else if (nodename == "type-checks") {
            for (const tinyxml2::XMLElement *checkNode = node->FirstChildElement(); checkNode; checkNode = checkNode->NextSiblingElement()) {
                const std::string &checkName = checkNode->Name();
                for (const tinyxml2::XMLElement *checkTypeNode = checkNode->FirstChildElement(); checkTypeNode; checkTypeNode = checkTypeNode->NextSiblingElement()) {
                    const std::string checkTypeName = checkTypeNode->Name();
                    const char *typeName = checkTypeNode->GetText();
                    if (!typeName)
                        continue;
                    if (checkTypeName == "check")
                        mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::check;
                    else if (checkTypeName == "suppress")
                        mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::suppress;
                    else if (checkTypeName == "checkFiniteLifetime")
                        mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::checkFiniteLifetime;
                }
            }
        }

        else if (nodename == "podtype") {
            const char * const name = node->Attribute("name");
            if (!name)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            PodType podType = {0};
            podType.stdtype = PodType::Type::NO;
            const char * const stdtype = node->Attribute("stdtype");
            if (stdtype) {
                if (std::strcmp(stdtype, "bool") == 0)
                    podType.stdtype = PodType::Type::BOOL;
                else if (std::strcmp(stdtype, "char") == 0)
                    podType.stdtype = PodType::Type::CHAR;
                else if (std::strcmp(stdtype, "short") == 0)
                    podType.stdtype = PodType::Type::SHORT;
                else if (std::strcmp(stdtype, "int") == 0)
                    podType.stdtype = PodType::Type::INT;
                else if (std::strcmp(stdtype, "long") == 0)
                    podType.stdtype = PodType::Type::LONG;
                else if (std::strcmp(stdtype, "long long") == 0)
                    podType.stdtype = PodType::Type::LONGLONG;
            }
            const char * const size = node->Attribute("size");
            if (size)
                podType.size = strToInt<unsigned int>(size);
            const char * const sign = node->Attribute("sign");
            if (sign)
                podType.sign = *sign;
            for (const std::string &s : getnames(name))
                mPodTypes[s] = podType;
        }

        else if (nodename == "platformtype") {
            const char * const type_name = node->Attribute("name");
            if (type_name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
            PlatformType type;
            type.mType = value;
            std::set<std::string> platform;
            for (const tinyxml2::XMLElement *typenode = node->FirstChildElement(); typenode; typenode = typenode->NextSiblingElement()) {
                const std::string typenodename = typenode->Name();
                if (typenodename == "platform") {
                    const char * const type_attribute = typenode->Attribute("type");
                    if (type_attribute == nullptr)
                        return Error(ErrorCode::MISSING_ATTRIBUTE, "type");
                    platform.insert(type_attribute);
                } else if (typenodename == "signed")
                    type.mSigned = true;
                else if (typenodename == "unsigned")
                    type.mUnsigned = true;
                else if (typenodename == "long")
                    type.mLong = true;
                else if (typenodename == "pointer")
                    type.mPointer= true;
                else if (typenodename == "ptr_ptr")
                    type.mPtrPtr = true;
                else if (typenodename == "const_ptr")
                    type.mConstPtr = true;
                else
                    unknown_elements.insert(typenodename);
            }
            if (platform.empty()) {
                const PlatformType * const type_ptr = platform_type(type_name, emptyString);
                if (type_ptr) {
                    if (*type_ptr == type)
                        return Error(ErrorCode::DUPLICATE_PLATFORM_TYPE, type_name);
                    return Error(ErrorCode::PLATFORM_TYPE_REDEFINED, type_name);
                }
                mPlatformTypes[type_name] = type;
            } else {
                for (const std::string &p : platform) {
                    const PlatformType * const type_ptr = platform_type(type_name, p);
                    if (type_ptr) {
                        if (*type_ptr == type)
                            return Error(ErrorCode::DUPLICATE_PLATFORM_TYPE, type_name);
                        return Error(ErrorCode::PLATFORM_TYPE_REDEFINED, type_name);
                    }
                    mPlatforms[p].mPlatformTypes[type_name] = type;
                }
            }
        }

        else if (nodename == "entrypoint") {
            const char * const type_name = node->Attribute("name");
            if (type_name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            mEntrypoints.emplace(type_name);
        }

        else
            unknown_elements.insert(nodename);
    }
    if (!unknown_elements.empty()) {
        std::string str;
        for (std::set<std::string>::const_iterator i = unknown_elements.cbegin(); i != unknown_elements.cend();) {
            str += *i;
            if (++i != unknown_elements.end())
                str += ", ";
        }
        return Error(ErrorCode::UNKNOWN_ELEMENT, str);
    }
    return Error(ErrorCode::OK);
}

Library::Error Library::loadFunction(const tinyxml2::XMLElement * const node, const std::string &name, std::set<std::string> &unknown_elements)
{
    if (name.empty())
        return Error(ErrorCode::OK);

    // TODO: write debug warning if we modify an existing entry
    Function& func = functions[name];

    for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
        const std::string functionnodename = functionnode->Name();
        if (functionnodename == "noreturn") {
            const char * const text = functionnode->GetText();
            if (strcmp(text, "false") == 0)
                mNoReturn[name] = FalseTrueMaybe::False;
            else if (strcmp(text, "maybe") == 0)
                mNoReturn[name] = FalseTrueMaybe::Maybe;
            else
                mNoReturn[name] = FalseTrueMaybe::True; // Safe
        } else if (functionnodename == "pure")
            func.ispure = true;
        else if (functionnodename == "const") {
            func.ispure = true;
            func.isconst = true; // a constant function is pure
        } else if (functionnodename == "leak-ignore")
            func.leakignore = true;
        else if (functionnodename == "not-overlapping-data") {
            NonOverlappingData nonOverlappingData;
            nonOverlappingData.ptr1Arg = functionnode->IntAttribute("ptr1-arg", -1);
            nonOverlappingData.ptr2Arg = functionnode->IntAttribute("ptr2-arg", -1);
            nonOverlappingData.sizeArg = functionnode->IntAttribute("size-arg", -1);
            nonOverlappingData.strlenArg = functionnode->IntAttribute("strlen-arg", -1);
            mNonOverlappingData[name] = nonOverlappingData;
        } else if (functionnodename == "use-retval") {
            func.useretval = Library::UseRetValType::DEFAULT;
            if (const char *type = functionnode->Attribute("type"))
                if (std::strcmp(type, "error-code") == 0)
                    func.useretval = Library::UseRetValType::ERROR_CODE;
        } else if (functionnodename == "returnValue") {
            if (const char *expr = functionnode->GetText())
                mReturnValue[name] = expr;
            if (const char *type = functionnode->Attribute("type"))
                mReturnValueType[name] = type;
            if (const char *container = functionnode->Attribute("container"))
                mReturnValueContainer[name] = strToInt<int>(container);
            if (const char *unknownReturnValues = functionnode->Attribute("unknownValues")) {
                if (std::strcmp(unknownReturnValues, "all") == 0) {
                    std::vector<MathLib::bigint> values{LLONG_MIN, LLONG_MAX};
                    mUnknownReturnValues[name] = values;
                }
            }
        } else if (functionnodename == "arg") {
            const char* argNrString = functionnode->Attribute("nr");
            if (!argNrString)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "nr");
            const bool bAnyArg = strcmp(argNrString, "any") == 0;
            const bool bVariadicArg = strcmp(argNrString, "variadic") == 0;
            const int nr = (bAnyArg || bVariadicArg) ? -1 : strToInt<int>(argNrString);
            ArgumentChecks &ac = func.argumentChecks[nr];
            ac.optional  = functionnode->Attribute("default") != nullptr;
            ac.variadic = bVariadicArg;
            const char * const argDirection = functionnode->Attribute("direction");
            if (argDirection) {
                const size_t argDirLen = strlen(argDirection);
                if (!strncmp(argDirection, "in", argDirLen)) {
                    ac.direction = ArgumentChecks::Direction::DIR_IN;
                } else if (!strncmp(argDirection, "out", argDirLen)) {
                    ac.direction = ArgumentChecks::Direction::DIR_OUT;
                } else if (!strncmp(argDirection, "inout", argDirLen)) {
                    ac.direction = ArgumentChecks::Direction::DIR_INOUT;
                }
            }
            for (const tinyxml2::XMLElement *argnode = functionnode->FirstChildElement(); argnode; argnode = argnode->NextSiblingElement()) {
                const std::string argnodename = argnode->Name();
                int indirect = 0;
                const char * const indirectStr = argnode->Attribute("indirect");
                if (indirectStr)
                    indirect = strToInt<int>(indirectStr);
                if (argnodename == "not-bool")
                    ac.notbool = true;
                else if (argnodename == "not-null")
                    ac.notnull = true;
                else if (argnodename == "not-uninit")
                    ac.notuninit = indirect;
                else if (argnodename == "formatstr")
                    ac.formatstr = true;
                else if (argnodename == "strz")
                    ac.strz = true;
                else if (argnodename == "valid") {
                    // Validate the validation expression
                    const char *p = argnode->GetText();
                    if (!isCompliantValidationExpression(p))
                        return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, (!p ? "\"\"" : p));
                    // Set validation expression
                    ac.valid = p;
                }
                else if (argnodename == "minsize") {
                    const char *typeattr = argnode->Attribute("type");
                    if (!typeattr)
                        return Error(ErrorCode::MISSING_ATTRIBUTE, "type");

                    ArgumentChecks::MinSize::Type type;
                    if (strcmp(typeattr,"strlen")==0)
                        type = ArgumentChecks::MinSize::Type::STRLEN;
                    else if (strcmp(typeattr,"argvalue")==0)
                        type = ArgumentChecks::MinSize::Type::ARGVALUE;
                    else if (strcmp(typeattr,"sizeof")==0)
                        type = ArgumentChecks::MinSize::Type::SIZEOF;
                    else if (strcmp(typeattr,"mul")==0)
                        type = ArgumentChecks::MinSize::Type::MUL;
                    else if (strcmp(typeattr,"value")==0)
                        type = ArgumentChecks::MinSize::Type::VALUE;
                    else
                        return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, typeattr);

                    if (type == ArgumentChecks::MinSize::Type::VALUE) {
                        const char *valueattr = argnode->Attribute("value");
                        if (!valueattr)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
                        long long minsizevalue = 0;
                        try {
                            minsizevalue = strToInt<long long>(valueattr);
                        } catch (const std::runtime_error&) {
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, valueattr);
                        }
                        if (minsizevalue <= 0)
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, valueattr);
                        ac.minsizes.emplace_back(type, 0);
                        ac.minsizes.back().value = minsizevalue;
                    } else {
                        const char *argattr = argnode->Attribute("arg");
                        if (!argattr)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "arg");
                        if (strlen(argattr) != 1 || argattr[0]<'0' || argattr[0]>'9')
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, argattr);

                        ac.minsizes.reserve(type == ArgumentChecks::MinSize::Type::MUL ? 2 : 1);
                        ac.minsizes.emplace_back(type, argattr[0] - '0');
                        if (type == ArgumentChecks::MinSize::Type::MUL) {
                            const char *arg2attr = argnode->Attribute("arg2");
                            if (!arg2attr)
                                return Error(ErrorCode::MISSING_ATTRIBUTE, "arg2");
                            if (strlen(arg2attr) != 1 || arg2attr[0]<'0' || arg2attr[0]>'9')
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, arg2attr);
                            ac.minsizes.back().arg2 = arg2attr[0] - '0';
                        }
                    }
                    const char* baseTypeAttr = argnode->Attribute("baseType"); // used by VALUE, ARGVALUE
                    if (baseTypeAttr)
                        ac.minsizes.back().baseType = baseTypeAttr;
                }

                else if (argnodename == "iterator") {
                    ac.iteratorInfo.it = true;
                    const char* str = argnode->Attribute("type");
                    ac.iteratorInfo.first = (str && std::strcmp(str, "first") == 0);
                    ac.iteratorInfo.last = (str && std::strcmp(str, "last") == 0);
                    ac.iteratorInfo.container = argnode->IntAttribute("container", 0);
                }

                else
                    unknown_elements.insert(argnodename);
            }
            if (ac.notuninit == 0)
                ac.notuninit = ac.notnull ? 1 : 0;
        } else if (functionnodename == "ignorefunction") {
            func.ignore = true;
        } else if (functionnodename == "formatstr") {
            func.formatstr = true;
            const tinyxml2::XMLAttribute* scan = functionnode->FindAttribute("scan");
            const tinyxml2::XMLAttribute* secure = functionnode->FindAttribute("secure");
            func.formatstr_scan = scan && scan->BoolValue();
            func.formatstr_secure = secure && secure->BoolValue();
        } else if (functionnodename == "warn") {
            WarnInfo wi;
            const char* const severity = functionnode->Attribute("severity");
            if (severity == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "severity");
            wi.severity = Severity::fromString(severity);

            const char* const cstd = functionnode->Attribute("cstd");
            if (cstd) {
                if (!wi.standards.setC(cstd))
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, cstd);
            } else
                wi.standards.c = Standards::C89;

            const char* const cppstd = functionnode->Attribute("cppstd");
            if (cppstd) {
                if (!wi.standards.setCPP(cppstd))
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, cppstd);
            } else
                wi.standards.cpp = Standards::CPP03;

            const char* const reason = functionnode->Attribute("reason");
            const char* const alternatives = functionnode->Attribute("alternatives");
            if (reason && alternatives) {
                // Construct message
                wi.message = std::string(reason) + " function '" + name + "' called. It is recommended to use ";
                std::vector<std::string> alt = getnames(alternatives);
                for (std::size_t i = 0; i < alt.size(); ++i) {
                    wi.message += "'" + alt[i] + "'";
                    if (i == alt.size() - 1)
                        wi.message += " instead.";
                    else if (i == alt.size() - 2)
                        wi.message += " or ";
                    else
                        wi.message += ", ";
                }
            } else {
                const char * const message = functionnode->GetText();
                if (!message)
                    return Error(ErrorCode::MISSING_ATTRIBUTE, "\"reason\" and \"alternatives\" or some text.");

                wi.message = message;
            }

            functionwarn[name] = wi;
        } else if (functionnodename == "container") {
            const char* const action_ptr = functionnode->Attribute("action");
            Container::Action action = Container::Action::NO_ACTION;
            if (action_ptr) {
                std::string actionName = action_ptr;
                action = Container::actionFrom(actionName);
                if (action == Container::Action::NO_ACTION)
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, actionName);
            }
            func.containerAction = action;

            const char* const yield_ptr = functionnode->Attribute("yields");
            Container::Yield yield = Container::Yield::NO_YIELD;
            if (yield_ptr) {
                std::string yieldName = yield_ptr;
                yield = Container::yieldFrom(yieldName);
                if (yield == Container::Yield::NO_YIELD)
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, yieldName);
            }
            func.containerYield = yield;

            const char* const returnType = functionnode->Attribute("returnType");
            if (returnType)
                func.returnType = returnType;
        } else
            unknown_elements.insert(functionnodename);
    }
    return Error(ErrorCode::OK);
}

bool Library::isIntArgValid(const Token *ftok, int argnr, const MathLib::bigint argvalue) const
{
    const ArgumentChecks *ac = getarg(ftok, argnr);
    if (!ac || ac->valid.empty())
        return true;
    if (ac->valid.find('.') != std::string::npos)
        return isFloatArgValid(ftok, argnr, argvalue);
    TokenList tokenList(nullptr);
    gettokenlistfromvalid(ac->valid, tokenList);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (tok->isNumber() && argvalue == MathLib::toLongNumber(tok->str()))
            return true;
        if (Token::Match(tok, "%num% : %num%") && argvalue >= MathLib::toLongNumber(tok->str()) && argvalue <= MathLib::toLongNumber(tok->strAt(2)))
            return true;
        if (Token::Match(tok, "%num% : ,") && argvalue >= MathLib::toLongNumber(tok->str()))
            return true;
        if ((!tok->previous() || tok->previous()->str() == ",") && Token::Match(tok,": %num%") && argvalue <= MathLib::toLongNumber(tok->strAt(1)))
            return true;
    }
    return false;
}

bool Library::isFloatArgValid(const Token *ftok, int argnr, double argvalue) const
{
    const ArgumentChecks *ac = getarg(ftok, argnr);
    if (!ac || ac->valid.empty())
        return true;
    TokenList tokenList(nullptr);
    gettokenlistfromvalid(ac->valid, tokenList);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%num% : %num%") && argvalue >= MathLib::toDoubleNumber(tok->str()) && argvalue <= MathLib::toDoubleNumber(tok->strAt(2)))
            return true;
        if (Token::Match(tok, "%num% : ,") && argvalue >= MathLib::toDoubleNumber(tok->str()))
            return true;
        if ((!tok->previous() || tok->previous()->str() == ",") && Token::Match(tok,": %num%") && argvalue <= MathLib::toDoubleNumber(tok->strAt(1)))
            return true;
        if (Token::Match(tok, "%num%") && MathLib::isFloat(tok->str()) && MathLib::isEqual(tok->str(), MathLib::toString(argvalue)))
            return true;
        if (Token::Match(tok, "! %num%") && MathLib::isFloat(tok->next()->str()))
            return MathLib::isNotEqual(tok->next()->str(), MathLib::toString(argvalue));
    }
    return false;
}

std::string Library::getFunctionName(const Token *ftok, bool &error) const
{
    if (!ftok) {
        error = true;
        return "";
    }
    if (ftok->isName()) {
        for (const Scope *scope = ftok->scope(); scope; scope = scope->nestedIn) {
            if (!scope->isClassOrStruct())
                continue;
            const std::vector<Type::BaseInfo> &derivedFrom = scope->definedType->derivedFrom;
            for (const Type::BaseInfo & baseInfo : derivedFrom) {
                std::string name;
                const Token* tok = baseInfo.nameTok; // baseInfo.name still contains template parameters, but is missing namespaces
                if (tok->str() == "::")
                    tok = tok->next();
                while (Token::Match(tok, "%name%|::")) {
                    name += tok->str();
                    tok = tok->next();
                }
                name += "::" + ftok->str();
                if (functions.find(name) != functions.end() && matchArguments(ftok, name))
                    return name;
            }
        }
        return ftok->str();
    }
    if (ftok->str() == "::") {
        if (!ftok->astOperand2())
            return getFunctionName(ftok->astOperand1(), error);
        return getFunctionName(ftok->astOperand1(),error) + "::" + getFunctionName(ftok->astOperand2(),error);
    }
    if (ftok->str() == "." && ftok->astOperand1()) {
        const std::string type = astCanonicalType(ftok->astOperand1(), ftok->originalName() == "->");
        if (type.empty()) {
            error = true;
            return "";
        }

        return type + "::" + getFunctionName(ftok->astOperand2(),error);
    }
    error = true;
    return "";
}

std::string Library::getFunctionName(const Token *ftok) const
{
    if (!Token::Match(ftok, "%name% )| (") && (ftok->strAt(-1) != "&" || ftok->previous()->astOperand2()))
        return "";

    // Lookup function name using AST..
    if (ftok->astParent()) {
        bool error = false;
        const Token * tok = ftok->astParent()->isUnaryOp("&") ? ftok->astParent()->astOperand1() : ftok->next()->astOperand1();
        const std::string ret = getFunctionName(tok, error);
        return error ? std::string() : ret;
    }

    // Lookup function name without using AST..
    if (Token::simpleMatch(ftok->previous(), "."))
        return "";
    if (!Token::Match(ftok->tokAt(-2), "%name% ::"))
        return ftok->str();
    std::string ret(ftok->str());
    ftok = ftok->tokAt(-2);
    while (Token::Match(ftok, "%name% ::")) {
        ret = ftok->str() + "::" + ret;
        ftok = ftok->tokAt(-2);
    }
    return ret;
}

bool Library::isnullargbad(const Token *ftok, int argnr) const
{
    const ArgumentChecks *arg = getarg(ftok, argnr);
    if (!arg) {
        // scan format string argument should not be null
        const std::string funcname = getFunctionName(ftok);
        const std::unordered_map<std::string, Function>::const_iterator it = functions.find(funcname);
        if (it != functions.cend() && it->second.formatstr && it->second.formatstr_scan)
            return true;
    }
    return arg && arg->notnull;
}

bool Library::isuninitargbad(const Token *ftok, int argnr, int indirect, bool *hasIndirect) const
{
    const ArgumentChecks *arg = getarg(ftok, argnr);
    if (!arg) {
        // non-scan format string argument should not be uninitialized
        const std::string funcname = getFunctionName(ftok);
        const std::unordered_map<std::string, Function>::const_iterator it = functions.find(funcname);
        if (it != functions.cend() && it->second.formatstr && !it->second.formatstr_scan)
            return true;
    }
    if (hasIndirect && arg && arg->notuninit >= 1)
        *hasIndirect = true;
    return arg && arg->notuninit >= indirect;
}


/** get allocation info for function */
const Library::AllocFunc* Library::getAllocFuncInfo(const Token *tok) const
{
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && functions.find(funcname) != functions.end() ? nullptr : getAllocDealloc(mAlloc, funcname);
}

/** get deallocation info for function */
const Library::AllocFunc* Library::getDeallocFuncInfo(const Token *tok) const
{
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && functions.find(funcname) != functions.end() ? nullptr : getAllocDealloc(mDealloc, funcname);
}

/** get reallocation info for function */
const Library::AllocFunc* Library::getReallocFuncInfo(const Token *tok) const
{
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && functions.find(funcname) != functions.end() ? nullptr : getAllocDealloc(mRealloc, funcname);
}

/** get allocation id for function */
int Library::getAllocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getAllocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}

/** get deallocation id for function */
int Library::getDeallocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getDeallocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}

/** get reallocation id for function */
int Library::getReallocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getReallocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}


const Library::ArgumentChecks * Library::getarg(const Token *ftok, int argnr) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const std::unordered_map<std::string, Function>::const_iterator it1 = functions.find(getFunctionName(ftok));
    if (it1 == functions.cend())
        return nullptr;
    const std::map<int,ArgumentChecks>::const_iterator it2 = it1->second.argumentChecks.find(argnr);
    if (it2 != it1->second.argumentChecks.cend())
        return &it2->second;
    const std::map<int,ArgumentChecks>::const_iterator it3 = it1->second.argumentChecks.find(-1);
    if (it3 != it1->second.argumentChecks.cend())
        return &it3->second;
    return nullptr;
}

bool Library::isScopeNoReturn(const Token *end, std::string *unknownFunc) const
{
    if (unknownFunc)
        unknownFunc->clear();

    if (Token::Match(end->tokAt(-2), "!!{ ; }")) {
        const Token *lastTop = end->tokAt(-2)->astTop();
        if (Token::simpleMatch(lastTop, "<<") &&
            Token::simpleMatch(lastTop->astOperand1(), "(") &&
            Token::Match(lastTop->astOperand1()->previous(), "%name% ("))
            return isnoreturn(lastTop->astOperand1()->previous());
    }

    if (!Token::simpleMatch(end->tokAt(-2), ") ; }"))
        return false;

    const Token *funcname = end->linkAt(-2)->previous();
    const Token *start = funcname;
    if (Token::Match(funcname->tokAt(-3),"( * %name% )")) {
        funcname = funcname->previous();
        start = funcname->tokAt(-3);
    } else if (funcname->isName()) {
        while (Token::Match(start, "%name%|.|::"))
            start = start->previous();
    } else {
        return false;
    }
    if (Token::Match(start,"[;{}]") && Token::Match(funcname, "%name% )| (")) {
        if (funcname->isKeyword())
            return false;
        if (funcname->str() == "exit")
            return true;
        if (!isnotnoreturn(funcname)) {
            if (unknownFunc && !isnoreturn(funcname))
                *unknownFunc = funcname->str();
            return true;
        }
    }
    return false;
}

const Library::Container* Library::detectContainerInternal(const Token* const typeStart, DetectContainer detect, bool* isIterator, bool withoutStd) const
{
    const Token* firstLinkedTok = nullptr;
    for (const Token* tok = typeStart; tok && !tok->varId(); tok = tok->next()) {
        if (!tok->link())
            continue;

        firstLinkedTok = tok;
        break;
    }

    for (const std::pair<const std::string, Library::Container> & c : containers) {
        const Container& container = c.second;
        if (container.startPattern.empty())
            continue;

        const int offset = (withoutStd && container.startPattern2.find("std :: ") == 0) ? 7 : 0;

        // If endPattern is undefined, it will always match, but itEndPattern has to be defined.
        if (detect != IteratorOnly && container.endPattern.empty()) {
            if (!Token::Match(typeStart, container.startPattern2.c_str() + offset))
                continue;

            if (isIterator)
                *isIterator = false;
            return &container;
        }

        if (!firstLinkedTok)
            continue;

        const bool matchedStartPattern = Token::Match(typeStart, container.startPattern2.c_str() + offset);
        if (!matchedStartPattern)
            continue;

        if (detect != ContainerOnly && Token::Match(firstLinkedTok->link(), container.itEndPattern.c_str())) {
            if (isIterator)
                *isIterator = true;
            return &container;
        }
        if (detect != IteratorOnly && Token::Match(firstLinkedTok->link(), container.endPattern.c_str())) {
            if (isIterator)
                *isIterator = false;
            return &container;
        }
    }
    return nullptr;
}

const Library::Container* Library::detectContainer(const Token* typeStart) const
{
    return detectContainerInternal(typeStart, ContainerOnly);
}

const Library::Container* Library::detectIterator(const Token* typeStart) const
{
    return detectContainerInternal(typeStart, IteratorOnly);
}

const Library::Container* Library::detectContainerOrIterator(const Token* typeStart, bool* isIterator, bool withoutStd) const
{
    bool res;
    const Library::Container* c = detectContainerInternal(typeStart, Both, &res, withoutStd);
    if (c && isIterator)
        *isIterator = res;
    return c;
}

bool Library::isContainerYield(const Token * const cond, Library::Container::Yield y, const std::string& fallback)
{
    if (!cond)
        return false;
    if (cond->str() == "(") {
        const Token* tok = cond->astOperand1();
        if (tok && tok->str() == ".") {
            if (tok->astOperand1() && tok->astOperand1()->valueType()) {
                if (const Library::Container *container = tok->astOperand1()->valueType()->container) {
                    return tok->astOperand2() && y == container->getYield(tok->astOperand2()->str());
                }
            } else if (!fallback.empty()) {
                return Token::simpleMatch(cond, "( )") && cond->previous()->str() == fallback;
            }
        }
    }
    return false;
}

// returns true if ftok is not a library function
bool Library::isNotLibraryFunction(const Token *ftok) const
{
    if (ftok->isKeyword() || ftok->isStandardType())
        return true;

    if (ftok->function() && ftok->function()->nestedIn && ftok->function()->nestedIn->type != Scope::eGlobal)
        return true;

    // variables are not library functions.
    if (ftok->varId())
        return true;

    return !matchArguments(ftok, getFunctionName(ftok));
}

bool Library::matchArguments(const Token *ftok, const std::string &functionName) const
{
    if (functionName.empty())
        return false;
    const int callargs = numberOfArgumentsWithoutAst(ftok);
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(functionName);
    if (it == functions.cend())
        return false;
    int args = 0;
    int firstOptionalArg = -1;
    for (const std::pair<const int, Library::ArgumentChecks> & argCheck : it->second.argumentChecks) {
        if (argCheck.first > args)
            args = argCheck.first;
        if (argCheck.second.optional && (firstOptionalArg == -1 || firstOptionalArg > argCheck.first))
            firstOptionalArg = argCheck.first;

        if (argCheck.second.formatstr || argCheck.second.variadic)
            return args <= callargs;
    }
    return (firstOptionalArg < 0) ? args == callargs : (callargs >= firstOptionalArg-1 && callargs <= args);
}

const Library::WarnInfo* Library::getWarnInfo(const Token* ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const std::map<std::string, WarnInfo>::const_iterator i = functionwarn.find(getFunctionName(ftok));
    if (i == functionwarn.cend())
        return nullptr;
    return &i->second;
}

bool Library::isCompliantValidationExpression(const char* p)
{
    if (!p || !*p)
        return false;

    bool error = false;
    bool range = false;
    bool has_dot = false;
    bool has_E = false;

    error = *p == '.';
    for (; *p; p++) {
        if (std::isdigit(*p)) {
            error |= (*(p + 1) == '-');
        }
        else if (*p == ':') {
            error |= range | (*(p + 1) == '.');
            range = true;
            has_dot = false;
            has_E = false;
        }
        else if ((*p == '-') || (*p == '+')) {
            error |= (!std::isdigit(*(p + 1)));
        }
        else if (*p == ',') {
            range = false;
            error |= *(p + 1) == '.';
            has_dot = false;
            has_E = false;
        } else if (*p == '.') {
            error |= has_dot | (!std::isdigit(*(p + 1)));
            has_dot = true;
        } else if (*p == 'E' || *p == 'e') {
            error |= has_E;
            has_E = true;
        } else if (*p == '!') {
            error |= !((*(p+1) == '-') || (*(p+1) == '+') || (std::isdigit(*(p + 1))));
        } else
            return false;
    }
    return !error;
}

bool Library::formatstr_function(const Token* ftok) const
{
    if (isNotLibraryFunction(ftok))
        return false;

    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(getFunctionName(ftok));
    if (it != functions.cend())
        return it->second.formatstr;
    return false;
}

int Library::formatstr_argno(const Token* ftok) const
{
    const std::map<int, Library::ArgumentChecks>& argumentChecksFunc = functions.at(getFunctionName(ftok)).argumentChecks;
    auto it = std::find_if(argumentChecksFunc.cbegin(), argumentChecksFunc.cend(), [](const std::pair<const int, Library::ArgumentChecks>& a) {
        return a.second.formatstr;
    });
    return it == argumentChecksFunc.cend() ? -1 : it->first - 1;
}

bool Library::formatstr_scan(const Token* ftok) const
{
    return functions.at(getFunctionName(ftok)).formatstr_scan;
}

bool Library::formatstr_secure(const Token* ftok) const
{
    return functions.at(getFunctionName(ftok)).formatstr_secure;
}

const Library::NonOverlappingData* Library::getNonOverlappingData(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const std::unordered_map<std::string, NonOverlappingData>::const_iterator it = mNonOverlappingData.find(getFunctionName(ftok));
    return (it != mNonOverlappingData.cend()) ? &it->second : nullptr;
}

Library::UseRetValType Library::getUseRetValType(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            const Token* contTok = ftok->astParent()->astOperand1();
            using Yield = Library::Container::Yield;
            const Yield yield = astContainerYield(contTok);
            if (yield == Yield::START_ITERATOR || yield == Yield::END_ITERATOR || yield == Yield::AT_INDEX ||
                yield == Yield::SIZE || yield == Yield::EMPTY || yield == Yield::BUFFER || yield == Yield::BUFFER_NT ||
                ((yield == Yield::ITEM || yield == Yield::ITERATOR) && astContainerAction(contTok) == Library::Container::Action::NO_ACTION))
                return Library::UseRetValType::DEFAULT;
        }
        return Library::UseRetValType::NONE;
    }
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(getFunctionName(ftok));
    if (it != functions.cend())
        return it->second.useretval;
    return Library::UseRetValType::NONE;
}

const std::string& Library::returnValue(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return emptyString;
    const std::map<std::string, std::string>::const_iterator it = mReturnValue.find(getFunctionName(ftok));
    return it != mReturnValue.cend() ? it->second : emptyString;
}

const std::string& Library::returnValueType(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".") && ftok->astParent()->astOperand1()) {
            const Token* contTok = ftok->astParent()->astOperand1();
            if (contTok->valueType() && contTok->valueType()->container)
                return contTok->valueType()->container->getReturnType(ftok->str());
        }
        return emptyString;
    }
    const std::map<std::string, std::string>::const_iterator it = mReturnValueType.find(getFunctionName(ftok));
    return it != mReturnValueType.cend() ? it->second : emptyString;
}

int Library::returnValueContainer(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return -1;
    const std::map<std::string, int>::const_iterator it = mReturnValueContainer.find(getFunctionName(ftok));
    return it != mReturnValueContainer.cend() ? it->second : -1;
}

std::vector<MathLib::bigint> Library::unknownReturnValues(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return std::vector<MathLib::bigint>();
    const std::map<std::string, std::vector<MathLib::bigint>>::const_iterator it = mUnknownReturnValues.find(getFunctionName(ftok));
    return (it == mUnknownReturnValues.cend()) ? std::vector<MathLib::bigint>() : it->second;
}

const Library::Function *Library::getFunction(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const std::unordered_map<std::string, Function>::const_iterator it1 = functions.find(getFunctionName(ftok));
    if (it1 == functions.cend())
        return nullptr;
    return &it1->second;
}


bool Library::hasminsize(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return false;
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(getFunctionName(ftok));
    if (it == functions.cend())
        return false;
    return std::any_of(it->second.argumentChecks.cbegin(), it->second.argumentChecks.cend(), [](const std::pair<const int, Library::ArgumentChecks>& a) {
        return !a.second.minsizes.empty();
    });
}

Library::ArgumentChecks::Direction Library::getArgDirection(const Token* ftok, int argnr) const
{
    const ArgumentChecks* arg = getarg(ftok, argnr);
    if (arg)
        return arg->direction;
    if (formatstr_function(ftok)) {
        const int fs_argno = formatstr_argno(ftok);
        if (fs_argno >= 0 && argnr >= fs_argno) {
            if (formatstr_scan(ftok))
                return ArgumentChecks::Direction::DIR_OUT;
            return ArgumentChecks::Direction::DIR_IN;
        }
    }
    return ArgumentChecks::Direction::DIR_UNKNOWN;
}

bool Library::ignorefunction(const std::string& functionName) const
{
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(functionName);
    if (it != functions.cend())
        return it->second.ignore;
    return false;
}
bool Library::isUse(const std::string& functionName) const
{
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(functionName);
    if (it != functions.cend())
        return it->second.use;
    return false;
}
bool Library::isLeakIgnore(const std::string& functionName) const
{
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(functionName);
    if (it != functions.cend())
        return it->second.leakignore;
    return false;
}
bool Library::isFunctionConst(const std::string& functionName, bool pure) const
{
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(functionName);
    if (it != functions.cend())
        return pure ? it->second.ispure : it->second.isconst;
    return false;
}
bool Library::isFunctionConst(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isConst())
        return true;
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            using Yield = Library::Container::Yield;
            const Yield yield = astContainerYield(ftok->astParent()->astOperand1());
            if (yield == Yield::EMPTY || yield == Yield::SIZE || yield == Yield::BUFFER_NT)
                return true;
        }
        return false;
    }
    const std::unordered_map<std::string, Function>::const_iterator it = functions.find(getFunctionName(ftok));
    return (it != functions.cend() && it->second.isconst);
}

bool Library::isnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return true;
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            const Token* contTok = ftok->astParent()->astOperand1();
            if (astContainerAction(contTok) != Library::Container::Action::NO_ACTION ||
                astContainerYield(contTok) != Library::Container::Yield::NO_YIELD)
                return false;
        }
        return false;
    }
    const std::unordered_map<std::string, FalseTrueMaybe>::const_iterator it = mNoReturn.find(getFunctionName(ftok));
    if (it == mNoReturn.end())
        return false;
    if (it->second == FalseTrueMaybe::Maybe)
        return true;
    return it->second == FalseTrueMaybe::True;
}

bool Library::isnotnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return false;
    if (isNotLibraryFunction(ftok))
        return false;
    const std::unordered_map<std::string, FalseTrueMaybe>::const_iterator it = mNoReturn.find(getFunctionName(ftok));
    if (it == mNoReturn.end())
        return false;
    if (it->second == FalseTrueMaybe::Maybe)
        return false;
    return it->second == FalseTrueMaybe::False;
}

bool Library::markupFile(const std::string &path) const
{
    return mMarkupExtensions.find(Path::getFilenameExtensionInLowerCase(path)) != mMarkupExtensions.end();
}

bool Library::processMarkupAfterCode(const std::string &path) const
{
    const std::map<std::string, bool>::const_iterator it = mProcessAfterCode.find(Path::getFilenameExtensionInLowerCase(path));
    return (it == mProcessAfterCode.cend() || it->second);
}

bool Library::reportErrors(const std::string &path) const
{
    const std::map<std::string, bool>::const_iterator it = mReportErrors.find(Path::getFilenameExtensionInLowerCase(path));
    return (it == mReportErrors.cend() || it->second);
}

bool Library::isexecutableblock(const std::string &file, const std::string &token) const
{
    const std::unordered_map<std::string, CodeBlock>::const_iterator it = mExecutableBlocks.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mExecutableBlocks.cend() && it->second.isBlock(token));
}

int Library::blockstartoffset(const std::string &file) const
{
    int offset = -1;
    const std::unordered_map<std::string, CodeBlock>::const_iterator map_it
        = mExecutableBlocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mExecutableBlocks.end()) {
        offset = map_it->second.offset();
    }
    return offset;
}

const std::string& Library::blockstart(const std::string &file) const
{
    const std::unordered_map<std::string, CodeBlock>::const_iterator map_it
        = mExecutableBlocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mExecutableBlocks.end()) {
        return map_it->second.start();
    }
    return emptyString;
}

const std::string& Library::blockend(const std::string &file) const
{
    const std::unordered_map<std::string, CodeBlock>::const_iterator map_it
        = mExecutableBlocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mExecutableBlocks.end()) {
        return map_it->second.end();
    }
    return emptyString;
}

bool Library::iskeyword(const std::string &file, const std::string &keyword) const
{
    const std::map<std::string, std::set<std::string>>::const_iterator it =
        mKeywords.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mKeywords.end() && it->second.count(keyword));
}

bool Library::isimporter(const std::string& file, const std::string &importer) const
{
    const std::map<std::string, std::set<std::string>>::const_iterator it =
        mImporters.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mImporters.end() && it->second.count(importer) > 0);
}

const Token* Library::getContainerFromYield(const Token* tok, Library::Container::Yield yield) const
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok->tokAt(-2), ". %name% (")) {
        const Token* containerTok = tok->tokAt(-2)->astOperand1();
        if (!astIsContainer(containerTok))
            return nullptr;
        if (containerTok->valueType()->container &&
            containerTok->valueType()->container->getYield(tok->strAt(-1)) == yield)
            return containerTok;
        if (yield == Library::Container::Yield::EMPTY && Token::simpleMatch(tok->tokAt(-1), "empty ( )"))
            return containerTok;
        if (yield == Library::Container::Yield::SIZE && Token::Match(tok->tokAt(-1), "size|length ( )"))
            return containerTok;
    } else if (Token::Match(tok->previous(), "%name% (")) {
        if (const Library::Function* f = this->getFunction(tok->previous())) {
            if (f->containerYield == yield) {
                return tok->astOperand2();
            }
        }
    }
    return nullptr;
}

// cppcheck-suppress unusedFunction
const Token* Library::getContainerFromAction(const Token* tok, Library::Container::Action action) const
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok->tokAt(-2), ". %name% (")) {
        const Token* containerTok = tok->tokAt(-2)->astOperand1();
        if (!astIsContainer(containerTok))
            return nullptr;
        if (containerTok->valueType()->container &&
            containerTok->valueType()->container->getAction(tok->strAt(-1)) == action)
            return containerTok;
        if (Token::simpleMatch(tok->tokAt(-1), "empty ( )"))
            return containerTok;
    } else if (Token::Match(tok->previous(), "%name% (")) {
        if (const Library::Function* f = this->getFunction(tok->previous())) {
            if (f->containerAction == action) {
                return tok->astOperand2();
            }
        }
    }
    return nullptr;
}

bool Library::isSmartPointer(const Token* tok) const
{
    return detectSmartPointer(tok);
}

const Library::SmartPointer* Library::detectSmartPointer(const Token* tok, bool withoutStd) const
{
    std::string typestr = withoutStd ? "std::" : "";
    while (Token::Match(tok, "%name%|::")) {
        typestr += tok->str();
        tok = tok->next();
    }
    auto it = smartPointers.find(typestr);
    if (it == smartPointers.end())
        return nullptr;
    return &it->second;
}

const Library::Container * getLibraryContainer(const Token * tok)
{
    if (!tok)
        return nullptr;
    // TODO: Support dereferencing iterators
    // TODO: Support dereferencing with ->
    if (tok->isUnaryOp("*") && astIsPointer(tok->astOperand1())) {
        for (const ValueFlow::Value& v:tok->astOperand1()->values()) {
            if (!v.isLocalLifetimeValue())
                continue;
            if (v.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                continue;
            return getLibraryContainer(v.tokvalue);
        }
    }
    if (!tok->valueType())
        return nullptr;
    return tok->valueType()->container;
}

Library::TypeCheck Library::getTypeCheck(std::string check,  std::string typeName) const
{
    auto it = mTypeChecks.find(std::pair<std::string, std::string>(std::move(check), std::move(typeName)));
    return it == mTypeChecks.end() ? TypeCheck::def : it->second;
}

bool Library::hasAnyTypeCheck(const std::string& typeName) const
{
    return std::any_of(mTypeChecks.begin(), mTypeChecks.end(), [&](const std::pair<std::pair<std::string, std::string>, Library::TypeCheck>& tc) {
        return tc.first.second == typeName;
    });
}

std::shared_ptr<Token> createTokenFromExpression(const std::string& returnValue,
                                                 const Settings* settings,
                                                 std::unordered_map<nonneg int, const Token*>* lookupVarId)
{
    std::shared_ptr<TokenList> tokenList = std::make_shared<TokenList>(settings);
    {
        const std::string code = "return " + returnValue + ";";
        std::istringstream istr(code);
        if (!tokenList->createTokens(istr))
            return nullptr;
    }

    // combine operators, set links, etc..
    std::stack<Token*> lpar;
    for (Token* tok2 = tokenList->front(); tok2; tok2 = tok2->next()) {
        if (Token::Match(tok2, "[!<>=] =")) {
            tok2->str(tok2->str() + "=");
            tok2->deleteNext();
        } else if (tok2->str() == "(")
            lpar.push(tok2);
        else if (tok2->str() == ")") {
            if (lpar.empty())
                return nullptr;
            Token::createMutualLinks(lpar.top(), tok2);
            lpar.pop();
        }
    }
    if (!lpar.empty())
        return nullptr;

    // set varids
    for (Token* tok2 = tokenList->front(); tok2; tok2 = tok2->next()) {
        if (tok2->str().compare(0, 3, "arg") != 0)
            continue;
        nonneg int const id = strToInt<nonneg int>(tok2->str().c_str() + 3);
        tok2->varId(id);
        if (lookupVarId)
            (*lookupVarId)[id] = tok2;
    }

    // Evaluate expression
    tokenList->createAst();
    Token* expr = tokenList->front()->astOperand1();
    ValueFlow::valueFlowConstantFoldAST(expr, settings);
    return {tokenList, expr};
}
