/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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

    // open file..
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(path);
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
        // failed to open file.. is there no extension?
        std::string fullfilename(path);
        if (Path::getFilenameExtension(fullfilename) == "") {
            fullfilename += ".cfg";
            error = doc.LoadFile(fullfilename.c_str());
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
        }
    }

    return (error == tinyxml2::XML_NO_ERROR) ?
           load(doc) :
           Error(error == tinyxml2::XML_ERROR_FILE_NOT_FOUND ? FILE_NOT_FOUND : BAD_XML);
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
        return Error(BAD_ELEMENT, rootnode->Name());

    const char* format_string = rootnode->Attribute("format");
    int format = 1;
    if (format_string)
        format = atoi(format_string);

    if (format > 1)
        return Error(UNSUPPORTED_FORMAT);

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        std::string nodename = node->Name();
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
                if (strcmp(node->Name(), "memory")==0)
                    while (!ismemory(++allocid));
                else
                    while (!isresource(++allocid));
                allocationId = allocid;
            }

            // add alloc/dealloc/use functions..
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"alloc")==0) {
                    _alloc[memorynode->GetText()] = allocationId;
                    const char *init = memorynode->Attribute("init");
                    if (init && strcmp(init,"false")==0) {
                        returnuninitdata.insert(memorynode->GetText());
                    }
                } else if (strcmp(memorynode->Name(),"dealloc")==0)
                    _dealloc[memorynode->GetText()] = allocationId;
                else if (strcmp(memorynode->Name(),"use")==0)
                    use.insert(memorynode->GetText());
                else
                    return Error(BAD_ELEMENT, memorynode->Name());
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
                std::string functionnodename = functionnode->Name();
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
                        std::string argnodename = argnode->Name();
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
                            return Error(BAD_ATTRIBUTE, argnode->Name());
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
                    return Error(BAD_ELEMENT, functionnode->Name());
            }
        }

        else if (nodename == "reflection") {
            for (const tinyxml2::XMLElement *reflectionnode = node->FirstChildElement(); reflectionnode; reflectionnode = reflectionnode->NextSiblingElement()) {
                if (strcmp(reflectionnode->Name(), "call") != 0)
                    return Error(BAD_ELEMENT, reflectionnode->Name());

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
                std::string markupnodename = markupnode->Name();
                if (markupnodename == "keywords") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "keyword") == 0) {
                            const char* nodeName = librarynode->Attribute("name");
                            if (nodeName == nullptr)
                                return Error(MISSING_ATTRIBUTE, "name");
                            _keywords[extension].insert(nodeName);
                        } else
                            return Error(BAD_ELEMENT, librarynode->Name());
                    }
                }

                else if (markupnodename == "exported") {
                    for (const tinyxml2::XMLElement *exporter = markupnode->FirstChildElement(); exporter; exporter = exporter->NextSiblingElement()) {
                        if (strcmp(exporter->Name(), "exporter") != 0)
                            return Error(BAD_ELEMENT, exporter->Name());

                        const char * const prefix = exporter->Attribute("prefix");
                        if (!prefix)
                            return Error(MISSING_ATTRIBUTE, "prefix");

                        for (const tinyxml2::XMLElement *e = exporter->FirstChildElement(); e; e = e->NextSiblingElement()) {
                            if (strcmp(e->Name(), "prefix") == 0)
                                _exporters[prefix].addPrefix(e->GetText());
                            else if (strcmp(e->Name(), "suffix") == 0)
                                _exporters[prefix].addSuffix(e->GetText());
                            else
                                return Error(BAD_ELEMENT, e->Name());
                        }
                    }
                }

                else if (markupnodename == "imported") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "importer") == 0)
                            _importers[extension].insert(librarynode->GetText());
                        else
                            return Error(BAD_ELEMENT, librarynode->Name());
                    }
                }

                else if (markupnodename == "codeblocks") {
                    for (const tinyxml2::XMLElement *blocknode = markupnode->FirstChildElement(); blocknode; blocknode = blocknode->NextSiblingElement()) {
                        if (strcmp(blocknode->Name(), "block") == 0) {
                            const char * blockName = blocknode->Attribute("name");
                            if (blockName)
                                _executableblocks[extension].addBlock(blockName);
                        } else if (strcmp(blocknode->Name(), "structure") == 0) {
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
                            return Error(BAD_ELEMENT, blocknode->Name());
                    }
                }

                else
                    return Error(BAD_ELEMENT, markupnode->Name());
            }
        }

        else if (nodename == "podtype") {
            const char * const name = node->Attribute("name");
            if (!name)
                return Error(MISSING_ATTRIBUTE, "name");
            PodType podType = {0};
            const char * const size = node->Attribute("sizeof");
            if (size)
                podType.size = atoi(size);
            const char * const sign = node->Attribute("sign");
            if (sign)
                podType.sign = *sign;
            podtypes[name] = podType;
        }

        else
            return Error(BAD_ELEMENT, node->Name());
    }
    return Error(OK);
}

bool Library::isargvalid(const std::string &functionName, int argnr, const MathLib::bigint argvalue) const
{
    const ArgumentChecks *ac = getarg(functionName, argnr);
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

const Library::ArgumentChecks * Library::getarg(const std::string &functionName, int argnr) const
{
    std::map<std::string, std::map<int, ArgumentChecks> >::const_iterator it1;
    it1 = argumentChecks.find(functionName);
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
    if (funcname && Token::Match(funcname->tokAt(-3),"( * %var% )")) {
        funcname = funcname->previous();
        start = funcname->tokAt(-3);
    } else if (funcname->isName()) {
        while (Token::Match(start, "%var%|.|::"))
            start = start->previous();
    } else {
        return false;
    }
    if (Token::Match(start,"[;{}]") && Token::Match(funcname, "%var% )| (")) {
        if (funcname->str() == "exit")
            return true;
        if (!isnotnoreturn(funcname->str())) {
            if (unknownFunc && !isnoreturn(funcname->str()))
                *unknownFunc = funcname->str();
            return true;
        }
    }
    return false;
}

