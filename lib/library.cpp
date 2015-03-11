/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "path.h"
#include "tinyxml2.h"
#include "tokenlist.h"
#include "mathlib.h"
#include "token.h"
#include "symboldatabase.h"

#include <string>
#include <algorithm>

Library::Library() : allocid(0)
{
}

Library::Error Library::load(const char exename[], const char path[])
{
    if (std::strchr(path,',') != nullptr) {
        std::string p(path);
        while (p.find(",") != std::string::npos) {
            const std::string::size_type pos = p.find(",");
            const Error &e = load(exename, p.substr(0,pos).c_str());
            if (e.errorcode != OK)
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
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
        // failed to open file.. is there no extension?
        std::string fullfilename(path);
        if (Path::getFilenameExtension(fullfilename) == "") {
            fullfilename += ".cfg";
            error = doc.LoadFile(fullfilename.c_str());
            if (error != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
                absolute_path = Path::getAbsoluteFilePath(fullfilename.c_str());
        }

        if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
            // Try to locate the library configuration in the installation folder..
#ifdef CFGDIR
            const std::string cfgfolder(CFGDIR);
#else
            if (!exename)
                return Error(FILE_NOT_FOUND);
            const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(exename)) + "cfg");
#endif
            const char *sep = (!cfgfolder.empty() && cfgfolder[cfgfolder.size()-1U]=='/' ? "" : "/");
            const std::string filename(cfgfolder + sep + fullfilename);
            error = doc.LoadFile(filename.c_str());
            if (error != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
                absolute_path = Path::getAbsoluteFilePath(filename.c_str());
        }
    } else
        absolute_path = Path::getAbsoluteFilePath(path);

    if (error == tinyxml2::XML_NO_ERROR) {
        if (_files.find(absolute_path) == _files.end()) {
            Error err = load(doc);
            if (err.errorcode == OK)
                _files.insert(absolute_path);
            return err;
        }

        return Error(OK); // ignore duplicates
    }

    return Error(error == tinyxml2::XML_ERROR_FILE_NOT_FOUND ? FILE_NOT_FOUND : BAD_XML);
}

bool Library::loadxmldata(const char xmldata[], std::size_t len)
{
    tinyxml2::XMLDocument doc;
    return (tinyxml2::XML_NO_ERROR == doc.Parse(xmldata, len)) && (load(doc).errorcode == OK);
}

Library::Error Library::load(const tinyxml2::XMLDocument &doc)
{
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (rootnode == nullptr)
        return Error(BAD_XML);

    if (strcmp(rootnode->Name(),"def") != 0)
        return Error(UNSUPPORTED_FORMAT, rootnode->Name());

    const char* format_string = rootnode->Attribute("format");
    int format = 1;
    if (format_string)
        format = atoi(format_string);

    if (format > 1)
        return Error(UNSUPPORTED_FORMAT);

    std::set<std::string> unknown_elements;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const std::string nodename = node->Name();
        if (nodename == "memory" || nodename == "resource") {
            // get allocationId to use..
            int allocationId = 0;
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"dealloc")==0) {
                    const std::map<std::string,int>::const_iterator it = _dealloc.find(memorynode->GetText());
                    if (it != _dealloc.end()) {
                        allocationId = it->second;
                        break;
                    }
                }
            }
            if (allocationId == 0) {
                if (nodename == "memory")
                    while (!ismemory(++allocid));
                else
                    while (!isresource(++allocid));
                allocationId = allocid;
            }

            // add alloc/dealloc/use functions..
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                const std::string memorynodename = memorynode->Name();
                if (memorynodename == "alloc") {
                    _alloc[memorynode->GetText()] = allocationId;
                    const char *init = memorynode->Attribute("init");
                    if (init && strcmp(init,"false")==0) {
                        returnuninitdata.insert(memorynode->GetText());
                    }
                } else if (memorynodename == "dealloc")
                    _dealloc[memorynode->GetText()] = allocationId;
                else if (memorynodename == "use")
                    use.insert(memorynode->GetText());
                else
                    unknown_elements.insert(memorynodename);
            }
        }

        else if (nodename == "define") {
            const char *name = node->Attribute("name");
            if (name == nullptr)
                return Error(MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(MISSING_ATTRIBUTE, "value");
            defines.push_back(std::string("#define ") +
                              name +
                              " " +
                              value +
                              "\n");
        }

        else if (nodename == "function") {
            const char *name_char = node->Attribute("name");
            if (name_char == nullptr)
                return Error(MISSING_ATTRIBUTE, "name");
            std::string name = name_char;

            for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
                const std::string functionnodename = functionnode->Name();
                if (functionnodename == "noreturn")
                    _noreturn[name] = (strcmp(functionnode->GetText(), "true") == 0);
                else if (functionnodename == "pure")
                    functionpure.insert(name);
                else if (functionnodename == "const") {
                    functionconst.insert(name);
                    functionpure.insert(name); // a constant function is pure
                } else if (functionnodename == "leak-ignore")
                    leakignore.insert(name);
                else if (functionnodename == "use-retval")
                    useretval.insert(name);
                else if (functionnodename == "arg" && functionnode->Attribute("nr") != nullptr) {
                    const bool bAnyArg = strcmp(functionnode->Attribute("nr"),"any")==0;
                    const int nr = (bAnyArg) ? -1 : atoi(functionnode->Attribute("nr"));
                    bool notbool = false;
                    bool notnull = false;
                    bool notuninit = false;
                    bool formatstr = false;
                    bool strz = false;
                    std::string& valid = argumentChecks[name][nr].valid;
                    std::list<ArgumentChecks::MinSize>& minsizes = argumentChecks[name][nr].minsizes;
                    for (const tinyxml2::XMLElement *argnode = functionnode->FirstChildElement(); argnode; argnode = argnode->NextSiblingElement()) {
                        const std::string argnodename = argnode->Name();
                        if (argnodename == "not-bool")
                            notbool = true;
                        else if (argnodename == "not-null")
                            notnull = true;
                        else if (argnodename == "not-uninit")
                            notuninit = true;
                        else if (argnodename == "formatstr")
                            formatstr = true;
                        else if (argnodename == "strz")
                            strz = true;
                        else if (argnodename == "valid") {
                            // Validate the validation expression
                            const char *p = argnode->GetText();
                            bool error = false;
                            bool range = false;
                            for (; *p; p++) {
                                if (std::isdigit(*p))
                                    error |= (*(p+1) == '-');
                                else if (*p == ':')
                                    error |= range;
                                else if (*p == '-')
                                    error |= (!std::isdigit(*(p+1)));
                                else if (*p == ',')
                                    range = false;
                                else
                                    error = true;

                                range |= (*p == ':');
                            }
                            if (error)
                                return Error(BAD_ATTRIBUTE_VALUE, argnode->GetText());

                            // Set validation expression
                            valid = argnode->GetText();
                        }

                        else if (argnodename == "minsize") {
                            const char *typeattr = argnode->Attribute("type");
                            if (!typeattr)
                                return Error(MISSING_ATTRIBUTE, "type");

                            ArgumentChecks::MinSize::Type type;
                            if (strcmp(typeattr,"strlen")==0)
                                type = ArgumentChecks::MinSize::STRLEN;
                            else if (strcmp(typeattr,"argvalue")==0)
                                type = ArgumentChecks::MinSize::ARGVALUE;
                            else if (strcmp(typeattr,"sizeof")==0)
                                type = ArgumentChecks::MinSize::SIZEOF;
                            else if (strcmp(typeattr,"mul")==0)
                                type = ArgumentChecks::MinSize::MUL;
                            else
                                return Error(BAD_ATTRIBUTE_VALUE, typeattr);

                            const char *argattr  = argnode->Attribute("arg");
                            if (!argattr)
                                return Error(MISSING_ATTRIBUTE, "arg");
                            if (strlen(argattr) != 1 || argattr[0]<'0' || argattr[0]>'9')
                                return Error(BAD_ATTRIBUTE_VALUE, argattr);

                            minsizes.push_back(ArgumentChecks::MinSize(type,argattr[0]-'0'));
                            if (type == ArgumentChecks::MinSize::MUL) {
                                const char *arg2attr  = argnode->Attribute("arg2");
                                if (!arg2attr)
                                    return Error(MISSING_ATTRIBUTE, "arg2");
                                if (strlen(arg2attr) != 1 || arg2attr[0]<'0' || arg2attr[0]>'9')
                                    return Error(BAD_ATTRIBUTE_VALUE, arg2attr);
                                minsizes.back().arg2 = arg2attr[0] - '0';
                            }
                        }

                        else
                            unknown_elements.insert(argnodename);
                    }
                    argumentChecks[name][nr].notbool   = notbool;
                    argumentChecks[name][nr].notnull   = notnull;
                    argumentChecks[name][nr].notuninit = notuninit;
                    argumentChecks[name][nr].formatstr = formatstr;
                    argumentChecks[name][nr].strz      = strz;
                } else if (functionnodename == "ignorefunction") {
                    _ignorefunction.insert(name);
                } else if (functionnodename == "formatstr") {
                    const tinyxml2::XMLAttribute* scan = functionnode->FindAttribute("scan");
                    const tinyxml2::XMLAttribute* secure = functionnode->FindAttribute("secure");
                    _formatstr[name] = std::make_pair(scan && scan->BoolValue(), secure && secure->BoolValue());
                } else
                    unknown_elements.insert(functionnodename);
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
                    return Error(MISSING_ATTRIBUTE, "arg");

                _reflection[reflectionnode->GetText()] = atoi(argString);
            }
        }

        else if (nodename == "markup") {
            const char * const extension = node->Attribute("ext");
            if (!extension)
                return Error(MISSING_ATTRIBUTE, "ext");
            _markupExtensions.insert(extension);

            const char * const reporterrors = node->Attribute("reporterrors");
            _reporterrors[extension] = (reporterrors && strcmp(reporterrors, "true") == 0);
            const char * const aftercode = node->Attribute("aftercode");
            _processAfterCode[extension] = (aftercode && strcmp(aftercode, "true") == 0);

            for (const tinyxml2::XMLElement *markupnode = node->FirstChildElement(); markupnode; markupnode = markupnode->NextSiblingElement()) {
                const std::string markupnodename = markupnode->Name();
                if (markupnodename == "keywords") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "keyword") == 0) {
                            const char* nodeName = librarynode->Attribute("name");
                            if (nodeName == nullptr)
                                return Error(MISSING_ATTRIBUTE, "name");
                            _keywords[extension].insert(nodeName);
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
                            return Error(MISSING_ATTRIBUTE, "prefix");

                        for (const tinyxml2::XMLElement *e = exporter->FirstChildElement(); e; e = e->NextSiblingElement()) {
                            const std::string ename = e->Name();
                            if (ename == "prefix")
                                _exporters[prefix].addPrefix(e->GetText());
                            else if (ename == "suffix")
                                _exporters[prefix].addSuffix(e->GetText());
                            else
                                unknown_elements.insert(ename);
                        }
                    }
                }

                else if (markupnodename == "imported") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "importer") == 0)
                            _importers[extension].insert(librarynode->GetText());
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
                                _executableblocks[extension].addBlock(blockName);
                        } else if (blocknodename == "structure") {
                            const char * start = blocknode->Attribute("start");
                            if (start)
                                _executableblocks[extension].setStart(start);
                            const char * end = blocknode->Attribute("end");
                            if (end)
                                _executableblocks[extension].setEnd(end);
                            const char * offset = blocknode->Attribute("offset");
                            if (offset)
                                _executableblocks[extension].setOffset(atoi(offset));
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
                return Error(MISSING_ATTRIBUTE, "id");

            Container& container = containers[id];

            const char* const inherits = node->Attribute("inherits");
            if (inherits) {
                std::map<std::string, Container>::const_iterator i = containers.find(inherits);
                if (i != containers.end())
                    container = i->second; // Take values from parent and overwrite them if necessary
                else
                    return Error(BAD_ATTRIBUTE_VALUE, inherits);
            }

            const char* const startPattern = node->Attribute("startPattern");
            if (startPattern)
                container.startPattern = startPattern;
            const char* const endPattern = node->Attribute("endPattern");
            if (endPattern)
                container.endPattern = endPattern;

            for (const tinyxml2::XMLElement *containerNode = node->FirstChildElement(); containerNode; containerNode = containerNode->NextSiblingElement()) {
                const std::string containerNodeName = containerNode->Name();
                if (containerNodeName == "size" || containerNodeName == "access" || containerNodeName == "other") {
                    for (const tinyxml2::XMLElement *functionNode = containerNode->FirstChildElement(); functionNode; functionNode = functionNode->NextSiblingElement()) {
                        if (std::string(functionNode->Name()) != "function") {
                            unknown_elements.insert(functionNode->Name());
                            continue;
                        }

                        const char* const functionName = functionNode->Attribute("name");
                        if (!functionName)
                            return Error(MISSING_ATTRIBUTE, "name");

                        const char* const action_ptr = functionNode->Attribute("action");
                        Container::Action action = Container::NO_ACTION;
                        if (action_ptr) {
                            std::string actionName = action_ptr;
                            if (actionName == "resize")
                                action = Container::RESIZE;
                            else if (actionName == "clear")
                                action = Container::CLEAR;
                            else if (actionName == "push")
                                action = Container::PUSH;
                            else if (actionName == "pop")
                                action = Container::POP;
                            else if (actionName == "find")
                                action = Container::FIND;
                            else
                                return Error(BAD_ATTRIBUTE_VALUE, actionName);
                        }

                        const char* const yield_ptr = functionNode->Attribute("yields");
                        Container::Yield yield = Container::NO_YIELD;
                        if (yield_ptr) {
                            std::string yieldName = yield_ptr;
                            if (yieldName == "at_index")
                                yield = Container::AT_INDEX;
                            else if (yieldName == "item")
                                yield = Container::ITEM;
                            else if (yieldName == "buffer")
                                yield = Container::BUFFER;
                            else if (yieldName == "buffer-nt")
                                yield = Container::BUFFER_NT;
                            else if (yieldName == "start-iterator")
                                yield = Container::START_ITERATOR;
                            else if (yieldName == "end-iterator")
                                yield = Container::END_ITERATOR;
                            else if (yieldName == "size")
                                yield = Container::SIZE;
                            else if (yieldName == "empty")
                                yield = Container::EMPTY;
                            else
                                return Error(BAD_ATTRIBUTE_VALUE, yieldName);
                        }

                        container.functions[functionName].action = action;
                        container.functions[functionName].yield = yield;
                    }

                    if (containerNodeName == "size") {
                        const char* const templateArg = containerNode->Attribute("templateParameter");
                        if (templateArg)
                            container.size_templateArgNo = atoi(templateArg);
                    } else if (containerNodeName == "access") {
                        const char* const indexArg = containerNode->Attribute("indexOperator");
                        if (indexArg)
                            container.arrayLike_indexOp = std::string(indexArg) == "array-like";
                    }
                } else if (containerNodeName == "type") {
                    const char* const templateArg = containerNode->Attribute("templateParameter");
                    if (templateArg)
                        container.type_templateArgNo = atoi(templateArg);

                    const char* const string = containerNode->Attribute("string");
                    if (string)
                        container.stdStringLike = std::string(string) == "std-like";
                } else
                    unknown_elements.insert(containerNodeName);
            }
        }

        else if (nodename == "podtype") {
            const char * const name = node->Attribute("name");
            if (!name)
                return Error(MISSING_ATTRIBUTE, "name");
            PodType podType = {0};
            const char * const size = node->Attribute("size");
            if (size)
                podType.size = atoi(size);
            const char * const sign = node->Attribute("sign");
            if (sign)
                podType.sign = *sign;
            podtypes[name] = podType;
        }

        else if (nodename == "platformtype") {
            const char * const type_name = node->Attribute("name");
            if (type_name == nullptr)
                return Error(MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(MISSING_ATTRIBUTE, "value");
            PlatformType type;
            type._type = value;
            std::set<std::string> platform;
            for (const tinyxml2::XMLElement *typenode = node->FirstChildElement(); typenode; typenode = typenode->NextSiblingElement()) {
                const std::string typenodename = typenode->Name();
                if (typenodename == "platform") {
                    const char * const type_attribute = typenode->Attribute("type");
                    if (type_attribute == nullptr)
                        return Error(MISSING_ATTRIBUTE, "type");
                    platform.insert(type_attribute);
                } else if (typenodename == "signed")
                    type._signed = true;
                else if (typenodename == "unsigned")
                    type._unsigned = true;
                else if (typenodename == "long")
                    type._long = true;
                else if (typenodename == "pointer")
                    type._pointer= true;
                else if (typenodename == "ptr_ptr")
                    type._ptr_ptr = true;
                else if (typenodename == "const_ptr")
                    type._const_ptr = true;
                else
                    unknown_elements.insert(typenodename);
            }
            if (platform.empty()) {
                const PlatformType * const type_ptr = platform_type(type_name, "");
                if (type_ptr) {
                    if (*type_ptr == type)
                        return Error(DUPLICATE_PLATFORM_TYPE, type_name);
                    return Error(PLATFORM_TYPE_REDEFINED, type_name);
                }
                platform_types[type_name] = type;
            } else {
                std::set<std::string>::const_iterator it;
                for (it = platform.begin(); it != platform.end(); ++it) {
                    const PlatformType * const type_ptr = platform_type(type_name, *it);
                    if (type_ptr) {
                        if (*type_ptr == type)
                            return Error(DUPLICATE_PLATFORM_TYPE, type_name);
                        return Error(PLATFORM_TYPE_REDEFINED, type_name);
                    }
                    platforms[*it]._platform_types[type_name] = type;
                }
            }
        }

        else
            unknown_elements.insert(nodename);
    }
    if (!unknown_elements.empty()) {
        std::string str;
        for (std::set<std::string>::const_iterator i = unknown_elements.begin(); i != unknown_elements.end();) {
            str += *i;
            if (++i != unknown_elements.end())
                str += ", ";
        }
        return Error(UNKNOWN_ELEMENT, str);
    }
    return Error(OK);
}

bool Library::isargvalid(const Token *ftok, int argnr, const MathLib::bigint argvalue) const
{
    const ArgumentChecks *ac = getarg(ftok, argnr);
    if (!ac || ac->valid.empty())
        return true;
    TokenList tokenList(0);
    std::istringstream istr(ac->valid + ',');
    tokenList.createTokens(istr);
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok,"- %num%")) {
            tok->str("-" + tok->strAt(1));
            tok->deleteNext();
        }
    }
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

const Library::ArgumentChecks * Library::getarg(const Token *ftok, int argnr) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    std::map<std::string, std::map<int, ArgumentChecks> >::const_iterator it1;
    it1 = argumentChecks.find(ftok->str());
    if (it1 == argumentChecks.end())
        return nullptr;
    const std::map<int,ArgumentChecks>::const_iterator it2 = it1->second.find(argnr);
    if (it2 != it1->second.end())
        return &it2->second;
    const std::map<int,ArgumentChecks>::const_iterator it3 = it1->second.find(-1);
    if (it3 != it1->second.end())
        return &it3->second;
    return nullptr;
}

bool Library::isScopeNoReturn(const Token *end, std::string *unknownFunc) const
{
    if (unknownFunc)
        unknownFunc->clear();

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

const Library::Container* Library::detectContainer(const Token* typeStart) const
{
    for (std::map<std::string, Container>::const_iterator i = containers.begin(); i != containers.end(); ++i) {
        const Container& container = i->second;
        if (container.startPattern.empty())
            continue;

        if (Token::Match(typeStart, container.startPattern.c_str())) {
            if (container.endPattern.empty())
                return &container;

            for (const Token* tok = typeStart; tok && !tok->varId(); tok = tok->next()) {
                if (tok->link()) {
                    if (Token::Match(tok->link(), container.endPattern.c_str()))
                        return &container;
                    break;
                }
            }
        }
    }
    return nullptr;
}
// returns true if ftok is not a library function
bool Library::isNotLibraryFunction(const Token *ftok) const
{
    // methods are not library functions
    // called from tokenizer, ast is not created properly yet
    if (Token::Match(ftok->previous(),"::|."))
        return true;
    if (ftok->function() && ftok->function()->nestedIn && ftok->function()->nestedIn->type != Scope::eGlobal)
        return true;

    // variables are not library functions.
    if (ftok->varId())
        return true;

    int callargs = 0;
    for (const Token *tok = ftok->tokAt(2); tok && tok->str() != ")"; tok = tok->next()) {
        if (callargs == 0)
            callargs = 1;
        if (tok->str() == ",")
            callargs++;
        else if (tok->link() && Token::Match(tok, "<|(|["))
            tok = tok->link();
    }
    const std::map<std::string, std::map<int, ArgumentChecks> >::const_iterator it = argumentChecks.find(ftok->str());
    if (it == argumentChecks.end())
        return (callargs != 0);
    int args = 0;
    for (std::map<int, ArgumentChecks>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
        if (it2->first > args)
            args = it2->first;
        if (it2->second.formatstr)
            return args > callargs;
    }
    return args != callargs;
}

bool Library::isnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return true;
    if (isNotLibraryFunction(ftok))
        return false;
    std::map<std::string, bool>::const_iterator it = _noreturn.find(ftok->str());
    return (it != _noreturn.end() && it->second);
}

bool Library::isnotnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return false;
    if (isNotLibraryFunction(ftok))
        return false;
    std::map<std::string, bool>::const_iterator it = _noreturn.find(ftok->str());
    return (it != _noreturn.end() && !it->second);
}

bool Library::markupFile(const std::string &path) const
{
    return _markupExtensions.find(Path::getFilenameExtensionInLowerCase(path)) != _markupExtensions.end();
}

bool Library::processMarkupAfterCode(const std::string &path) const
{
    const std::map<std::string, bool>::const_iterator it = _processAfterCode.find(Path::getFilenameExtensionInLowerCase(path));
    return (it == _processAfterCode.end() || it->second);
}

bool Library::reportErrors(const std::string &path) const
{
    const std::map<std::string, bool>::const_iterator it = _reporterrors.find(Path::getFilenameExtensionInLowerCase(path));
    return (it == _reporterrors.end() || it->second);
}

bool Library::isexecutableblock(const std::string &file, const std::string &token) const
{
    const std::map<std::string, CodeBlock>::const_iterator it = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != _executableblocks.end() && it->second.isBlock(token));
}

int Library::blockstartoffset(const std::string &file) const
{
    int offset = -1;
    const std::map<std::string, CodeBlock>::const_iterator map_it
        = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != _executableblocks.end()) {
        offset = map_it->second.offset();
    }
    return offset;
}

const std::string& Library::blockstart(const std::string &file) const
{
    const std::map<std::string, CodeBlock>::const_iterator map_it
        = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != _executableblocks.end()) {
        return map_it->second.start();
    }
    return emptyString;
}

const std::string& Library::blockend(const std::string &file) const
{
    const std::map<std::string, CodeBlock>::const_iterator map_it
        = _executableblocks.find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != _executableblocks.end()) {
        return map_it->second.end();
    }
    return emptyString;
}

bool Library::iskeyword(const std::string &file, const std::string &keyword) const
{
    const std::map<std::string, std::set<std::string> >::const_iterator it =
        _keywords.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != _keywords.end() && it->second.count(keyword));
}

bool Library::isimporter(const std::string& file, const std::string &importer) const
{
    const std::map<std::string, std::set<std::string> >::const_iterator it =
        _importers.find(Path::getFilenameExtensionInLowerCase(file));
    return (it != _importers.end() && it->second.count(importer) > 0);
}
