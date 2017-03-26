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


#include "preprocessor.h"
#include "path.h"
#include "errorlogger.h"
#include "settings.h"
#include "simplecpp.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <vector>
#include <set>
#include <cstdint>

/**
 * Remove heading and trailing whitespaces from the input parameter.
 * If string is all spaces/tabs, return empty string.
 * @param s The string to trim.
 */
static std::string trim(const std::string& s)
{
    const std::string::size_type beg = s.find_first_not_of(" \t");
    if (beg == std::string::npos)
        return "";
    const std::string::size_type end = s.find_last_not_of(" \t");
    return s.substr(beg, end - beg + 1);
}

Directive::Directive(const std::string &_file, const int _linenr, const std::string &_str):
    file(_file),
    linenr(_linenr),
    str(trim(_str))
{
}

bool Preprocessor::missingIncludeFlag;
bool Preprocessor::missingSystemIncludeFlag;

char Preprocessor::macroChar = char(1);

Preprocessor::Preprocessor(Settings& settings, ErrorLogger *errorLogger) : _settings(settings), _errorLogger(errorLogger)
{
}

Preprocessor::~Preprocessor()
{
    for (std::map<std::string, simplecpp::TokenList *>::iterator it = tokenlists.begin(); it != tokenlists.end(); ++it)
        delete it->second;
}


static void inlineSuppressions(const simplecpp::TokenList &tokens, Settings &_settings)
{
    std::list<std::string> suppressionIDs;

    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (tok->comment) {
            std::istringstream iss(tok->str.substr(2));
            std::string word;
            iss >> word;
            if (word != "cppcheck-suppress")
                continue;
            iss >> word;
            if (iss)
                suppressionIDs.push_back(word);
            continue;
        }

        if (suppressionIDs.empty())
            continue;

        // Relative filename
        std::string relativeFilename(tok->location.file());
        if (_settings.relativePaths) {
            for (std::size_t j = 0U; j < _settings.basePaths.size(); ++j) {
                const std::string bp = _settings.basePaths[j] + "/";
                if (relativeFilename.compare(0,bp.size(),bp)==0) {
                    relativeFilename = relativeFilename.substr(bp.size());
                }
            }
        }

        // Add the suppressions.
        for (std::list<std::string>::const_iterator it = suppressionIDs.begin(); it != suppressionIDs.end(); ++it) {
            _settings.nomsg.addSuppression(*it, relativeFilename, tok->location.line);
        }
        suppressionIDs.clear();
    }
}

void Preprocessor::inlineSuppressions(const simplecpp::TokenList &tokens)
{
    if (!_settings.inlineSuppressions)
        return;
    ::inlineSuppressions(tokens, _settings);
    for (std::map<std::string,simplecpp::TokenList*>::const_iterator it = tokenlists.begin(); it != tokenlists.end(); ++it) {
        if (it->second)
            ::inlineSuppressions(*it->second, _settings);
    }
}

void Preprocessor::setDirectives(const simplecpp::TokenList &tokens1)
{
    // directive list..
    directives.clear();

    std::vector<const simplecpp::TokenList *> list;
    list.reserve(1U + tokenlists.size());
    list.push_back(&tokens1);
    for (std::map<std::string, simplecpp::TokenList *>::const_iterator it = tokenlists.begin(); it != tokenlists.end(); ++it) {
        list.push_back(it->second);
    }

    for (std::vector<const simplecpp::TokenList *>::const_iterator it = list.begin(); it != list.end(); ++it) {
        for (const simplecpp::Token *tok = (*it)->cfront(); tok; tok = tok ? tok->next : nullptr) {
            if ((tok->op != '#') || (tok->previous && tok->previous->location.line == tok->location.line))
                continue;
            if (tok->next && tok->next->str == "endfile")
                continue;
            Directive directive(tok->location.file(), tok->location.line, emptyString);
            for (const simplecpp::Token *tok2 = tok; tok2 && tok2->location.line == directive.linenr; tok2 = tok2->next) {
                if (tok2->comment)
                    continue;
                if (!directive.str.empty() && (tok2->location.col > tok2->previous->location.col + tok2->previous->str.size()))
                    directive.str += ' ';
                if (directive.str == "#" && tok2->str == "file")
                    directive.str += "include";
                else
                    directive.str += tok2->str;
            }
            directives.push_back(directive);
        }
    }
}

static bool sameline(const simplecpp::Token *tok1, const simplecpp::Token *tok2)
{
    return tok1 && tok2 && tok1->location.sameline(tok2->location);
}

static std::string readcondition(const simplecpp::Token *iftok, const std::set<std::string> &defined, const std::set<std::string> &undefined)
{
    const simplecpp::Token *cond = iftok->next;
    if (!sameline(iftok,cond))
        return "";

    const simplecpp::Token *next1 = cond->next;
    const simplecpp::Token *next2 = next1 ? next1->next : nullptr;
    const simplecpp::Token *next3 = next2 ? next2->next : nullptr;

    unsigned int len = 1;
    if (sameline(iftok,next1))
        len = 2;
    if (sameline(iftok,next2))
        len = 3;
    if (sameline(iftok,next3))
        len = 4;

    if (len == 1 && cond->str == "0")
        return "0";

    if (len == 1 && cond->name) {
        if (defined.find(cond->str) == defined.end())
            return cond->str;
    }

    if (len == 3 && cond->op == '(' && next1->name && next2->op == ')') {
        if (defined.find(next1->str) == defined.end() && undefined.find(next1->str) == undefined.end())
            return next1->str;
    }

    if (len == 3 && cond->name && next1->str == "==" && next2->number) {
        if (defined.find(cond->str) == defined.end())
            return cond->str + '=' + cond->next->next->str;
    }

    std::set<std::string> configset;
    for (; sameline(iftok,cond); cond = cond->next) {
        if (cond->op == '!')
            break;
        if (cond->str != "defined")
            continue;
        const simplecpp::Token *dtok = cond->next;
        if (!dtok)
            break;
        if (dtok->op == '(')
            dtok = dtok->next;
        if (sameline(iftok,dtok) && dtok->name && defined.find(dtok->str) == defined.end() && undefined.find(dtok->str) == undefined.end())
            configset.insert(dtok->str);
    }
    std::string cfg;
    for (std::set<std::string>::const_iterator it = configset.begin(); it != configset.end(); ++it) {
        if (!cfg.empty())
            cfg += ';';
        cfg += *it;
    }
    return cfg;
}

static bool hasDefine(const std::string &userDefines, const std::string &cfg)
{
    if (cfg.empty()) {
        return false;
    }

    std::string::size_type pos = 0;
    while (pos < userDefines.size()) {
        pos = userDefines.find(cfg, pos);
        if (pos == std::string::npos)
            break;
        const std::string::size_type pos2 = pos + cfg.size();
        if ((pos == 0 || userDefines[pos-1U] == ';') && (pos2 == userDefines.size() || userDefines[pos2] == '='))
            return true;
        pos = pos2;
    }
    return false;
}

static std::string cfg(const std::vector<std::string> &configs, const std::string &userDefines)
{
    std::set<std::string> configs2(configs.begin(), configs.end());
    std::string ret;
    for (std::set<std::string>::const_iterator it = configs2.begin(); it != configs2.end(); ++it) {
        if (it->empty())
            continue;
        if (*it == "0")
            return "";
        if (hasDefine(userDefines, *it))
            continue;
        if (!ret.empty())
            ret += ';';
        ret += *it;
    }
    return ret;
}

static bool isUndefined(const std::string &cfg, const std::set<std::string> &undefined)
{
    for (std::string::size_type pos1 = 0U; pos1 < cfg.size();) {
        const std::string::size_type pos2 = cfg.find(';',pos1);
        const std::string def = (pos2 == std::string::npos) ? cfg.substr(pos1) : cfg.substr(pos1, pos2 - pos1);

        std::string::size_type eq = def.find('=');
        if (eq == std::string::npos && undefined.find(def) != undefined.end())
            return true;
        if (eq != std::string::npos && undefined.find(def.substr(0,eq)) != undefined.end() && def.substr(eq) != "=0")
            return true;

        pos1 = (pos2 == std::string::npos) ? pos2 : pos2 + 1U;
    }
    return false;
}

static bool getConfigsElseIsFalse(const std::vector<std::string> &configs_if, const std::string &userDefines)
{
    for (unsigned int i = 0; i < configs_if.size(); ++i) {
        if (hasDefine(userDefines, configs_if[i]))
            return true;
    }
    return false;
}

static const simplecpp::Token *gotoEndIf(const simplecpp::Token *cmdtok)
{
    int level = 0;
    while (nullptr != (cmdtok = cmdtok->next)) {
        if (cmdtok->op == '#' && !sameline(cmdtok->previous,cmdtok) && sameline(cmdtok, cmdtok->next)) {
            if (cmdtok->next->str.compare(0,2,"if")==0)
                ++level;
            else if (cmdtok->next->str == "endif") {
                --level;
                if (level < 0)
                    return cmdtok;
            }
        }
    }
    return nullptr;
}

static void getConfigs(const simplecpp::TokenList &tokens, std::set<std::string> &defined, const std::string &userDefines, const std::set<std::string> &undefined, std::set<std::string> &ret)
{
    std::vector<std::string> configs_if;
    std::vector<std::string> configs_ifndef;
    std::string elseError;

    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (tok->op != '#' || sameline(tok->previous, tok))
            continue;
        const simplecpp::Token *cmdtok = tok->next;
        if (!sameline(tok, cmdtok))
            continue;
        if (cmdtok->str == "ifdef" || cmdtok->str == "ifndef" || cmdtok->str == "if") {
            std::string config;
            if (cmdtok->str == "ifdef" || cmdtok->str == "ifndef") {
                const simplecpp::Token *expr1 = cmdtok->next;
                if (sameline(tok,expr1) && expr1->name && !sameline(tok,expr1->next))
                    config = expr1->str;
                if (defined.find(config) != defined.end())
                    config.clear();
            } else if (cmdtok->str == "if") {
                config = readcondition(cmdtok, defined, undefined);
            }

            // skip undefined configurations..
            if (isUndefined(config, undefined))
                config.clear();

            configs_if.push_back((cmdtok->str == "ifndef") ? std::string() : config);
            configs_ifndef.push_back((cmdtok->str == "ifndef") ? config : std::string());
            ret.insert(cfg(configs_if,userDefines));
        } else if (cmdtok->str == "elif" || cmdtok->str == "else") {
            if (getConfigsElseIsFalse(configs_if,userDefines)) {
                tok = gotoEndIf(tok);
                if (!tok)
                    break;
                tok = tok->previous;
                continue;
            }
            if (cmdtok->str == "else" &&
                cmdtok->next &&
                !sameline(cmdtok,cmdtok->next) &&
                sameline(cmdtok->next, cmdtok->next->next) &&
                cmdtok->next->op == '#' &&
                cmdtok->next->next->str == "error") {
                if (!elseError.empty())
                    elseError += ';';
                elseError += cfg(configs_if, userDefines);
            }
            if (!configs_if.empty())
                configs_if.pop_back();
            if (cmdtok->str == "elif") {
                std::string config = readcondition(cmdtok, defined, undefined);
                if (isUndefined(config,undefined))
                    config.clear();
                configs_if.push_back(config);
                ret.insert(cfg(configs_if, userDefines));
            } else if (!configs_ifndef.empty()) {
                configs_if.push_back(configs_ifndef.back());
                ret.insert(cfg(configs_if, userDefines));
            }
        } else if (cmdtok->str == "endif" && !sameline(tok, cmdtok->next)) {
            if (!configs_if.empty())
                configs_if.pop_back();
            if (!configs_ifndef.empty())
                configs_ifndef.pop_back();
        } else if (cmdtok->str == "error") {
            if (!configs_ifndef.empty() && !configs_ifndef.back().empty()) {
                std::vector<std::string> configs(configs_if);
                configs.push_back(configs_ifndef.back());
                ret.insert(cfg(configs, userDefines));
            }
        } else if (cmdtok->str == "define" && sameline(tok, cmdtok->next) && cmdtok->next->name) {
            defined.insert(cmdtok->next->str);
        }
    }
    if (!elseError.empty())
        ret.insert(elseError);
}


std::set<std::string> Preprocessor::getConfigs(const simplecpp::TokenList &tokens) const
{
    std::set<std::string> ret;
    ret.insert("");
    if (!tokens.cfront())
        return ret;

    std::set<std::string> defined;
    defined.insert("__cplusplus");

    ::getConfigs(tokens, defined, _settings.userDefines, _settings.userUndefs, ret);

    for (std::map<std::string, simplecpp::TokenList*>::const_iterator it = tokenlists.begin(); it != tokenlists.end(); ++it) {
        if (!_settings.configurationExcluded(it->first))
            ::getConfigs(*(it->second), defined, _settings.userDefines, _settings.userUndefs, ret);
    }

    return ret;
}


void Preprocessor::preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename, const std::list<std::string> &includePaths)
{
    (void)includePaths;

    simplecpp::OutputList outputList;
    std::vector<std::string> files;
    const simplecpp::TokenList tokens1(istr, files, filename, &outputList);


    const std::set<std::string> configs = getConfigs(tokens1);

    for (std::set<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it) {
        if (_settings.userUndefs.find(*it) == _settings.userUndefs.end()) {
            result[ *it ] = getcode(tokens1, *it, files, false);
        }
    }
}

std::string Preprocessor::removeSpaceNearNL(const std::string &str)
{
    std::string tmp;
    char prev = '\n'; // treat start of file as newline
    for (std::size_t i = 0; i < str.size(); i++) {
        if (str[i] == ' ' &&
            (prev == '\n' ||
             i + 1 >= str.size() || // treat end of file as newline
             str[i+1] == '\n'
            )
           ) {
            // Ignore space that has new line in either side of it
        } else {
            tmp.append(1, str[i]);
            prev = str[i];
        }
    }

    return tmp;
}

void Preprocessor::preprocessWhitespaces(std::string &processedFile)
{
    // Replace all tabs with spaces..
    std::replace(processedFile.begin(), processedFile.end(), '\t', ' ');

    // Remove space characters that are after or before new line character
    processedFile = removeSpaceNearNL(processedFile);
}

void Preprocessor::preprocess(std::istream &srcCodeStream, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename, const std::list<std::string> &includePaths)
{
    (void)includePaths;

    if (file0.empty())
        file0 = filename;

    simplecpp::OutputList outputList;
    std::vector<std::string> files;
    const simplecpp::TokenList tokens1(srcCodeStream, files, filename, &outputList);

    const std::set<std::string> configs = getConfigs(tokens1);
    for (std::set<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it)
        resultConfigurations.push_back(*it);

    processedFile = tokens1.stringify();
}

static void splitcfg(const std::string &cfg, std::list<std::string> &defines, const std::string &defaultValue)
{
    for (std::string::size_type defineStartPos = 0U; defineStartPos < cfg.size();) {
        const std::string::size_type defineEndPos = cfg.find(';', defineStartPos);
        std::string def = (defineEndPos == std::string::npos) ? cfg.substr(defineStartPos) : cfg.substr(defineStartPos, defineEndPos - defineStartPos);
        if (!defaultValue.empty() && def.find('=') == std::string::npos)
            def += '=' + defaultValue;
        defines.push_back(def);
        if (defineEndPos == std::string::npos)
            break;
        defineStartPos = defineEndPos + 1U;
    }
}

static simplecpp::DUI createDUI(const Settings &_settings, const std::string &cfg, const std::string &filename)
{
    simplecpp::DUI dui;

    splitcfg(_settings.userDefines, dui.defines, "1");
    if (!cfg.empty())
        splitcfg(cfg, dui.defines, emptyString);

    for (std::vector<std::string>::const_iterator it = _settings.library.defines.begin(); it != _settings.library.defines.end(); ++it) {
        if (it->compare(0,8,"#define ")!=0)
            continue;
        std::string s = it->substr(8);
        std::string::size_type pos = s.find_first_of(" (");
        if (pos == std::string::npos) {
            dui.defines.push_back(s);
            continue;
        }
        if (s[pos] == ' ') {
            s[pos] = '=';
        } else {
            s[s.find(')')+1] = '=';
        }
        dui.defines.push_back(s);
    }

    if (Path::isCPP(filename))
        dui.defines.push_back("__cplusplus");

    dui.undefined = _settings.userUndefs; // -U
    dui.includePaths = _settings.includePaths; // -I
    dui.includes = _settings.userIncludes;  // --include
    return dui;
}

static bool hasErrors(const simplecpp::OutputList &outputList)
{
    for (simplecpp::OutputList::const_iterator it = outputList.begin(); it != outputList.end(); ++it) {
        switch (it->type) {
        case simplecpp::Output::ERROR:
        case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
        case simplecpp::Output::SYNTAX_ERROR:
            return true;
        case simplecpp::Output::WARNING:
        case simplecpp::Output::MISSING_HEADER:
        case simplecpp::Output::PORTABILITY_BACKSLASH:
            break;
        };
    }
    return false;
}


void Preprocessor::loadFiles(const simplecpp::TokenList &rawtokens, std::vector<std::string> &files)
{
    const simplecpp::DUI dui = createDUI(_settings, emptyString, files[0]);

    simplecpp::OutputList outputList;

    tokenlists = simplecpp::load(rawtokens, files, dui, &outputList);
}

void Preprocessor::removeComments()
{
    for (std::map<std::string, simplecpp::TokenList*>::iterator it = tokenlists.begin(); it != tokenlists.end(); ++it) {
        if (it->second)
            it->second->removeComments();
    }
}

void Preprocessor::setPlatformInfo(simplecpp::TokenList *tokens) const
{
    tokens->sizeOfType["bool"]          = _settings.sizeof_bool;
    tokens->sizeOfType["short"]         = _settings.sizeof_short;
    tokens->sizeOfType["int"]           = _settings.sizeof_int;
    tokens->sizeOfType["long"]          = _settings.sizeof_long;
    tokens->sizeOfType["long long"]     = _settings.sizeof_long_long;
    tokens->sizeOfType["float"]         = _settings.sizeof_float;
    tokens->sizeOfType["double"]        = _settings.sizeof_double;
    tokens->sizeOfType["long double"]   = _settings.sizeof_long_double;
    tokens->sizeOfType["bool *"]        = _settings.sizeof_pointer;
    tokens->sizeOfType["short *"]       = _settings.sizeof_pointer;
    tokens->sizeOfType["int *"]         = _settings.sizeof_pointer;
    tokens->sizeOfType["long *"]        = _settings.sizeof_pointer;
    tokens->sizeOfType["long long *"]   = _settings.sizeof_pointer;
    tokens->sizeOfType["float *"]       = _settings.sizeof_pointer;
    tokens->sizeOfType["double *"]      = _settings.sizeof_pointer;
    tokens->sizeOfType["long double *"] = _settings.sizeof_pointer;
}

std::string Preprocessor::getcode(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, const bool writeLocations)
{
    const simplecpp::DUI dui = createDUI(_settings, cfg, files[0]);

    simplecpp::OutputList outputList;
    std::list<simplecpp::MacroUsage> macroUsage;
    simplecpp::TokenList tokens2(files);
    simplecpp::preprocess(tokens2, tokens1, files, tokenlists, dui, &outputList, &macroUsage);

    bool showerror = (!_settings.userDefines.empty() && !_settings.force);
    reportOutput(outputList, showerror);
    if (hasErrors(outputList))
        return "";

    // ensure that guessed define macros without value are not used in the code
    if (!validateCfg(cfg, macroUsage))
        return "";

    // assembler code locations..
    std::set<simplecpp::Location> assemblerLocations;
    for (std::list<Directive>::const_iterator dirIt = directives.begin(); dirIt != directives.end(); ++dirIt) {
        const Directive &d1 = *dirIt;
        if (d1.str.compare(0, 11, "#pragma asm") != 0)
            continue;
        std::list<Directive>::const_iterator dirIt2 = dirIt;
        ++dirIt2;
        if (dirIt2 == directives.end())
            continue;

        const Directive &d2 = *dirIt2;
        if (d2.str.compare(0,14,"#pragma endasm") != 0 || d1.file != d2.file)
            continue;

        simplecpp::Location loc(files);
        loc.fileIndex = ~0U;
        loc.col = 0U;
        for (unsigned int i = 0; i < files.size(); ++i) {
            if (files[i] == d1.file) {
                loc.fileIndex = i;
                break;
            }
        }

        for (unsigned int linenr = d1.linenr + 1U; linenr < d2.linenr; linenr++) {
            loc.line = linenr;
            assemblerLocations.insert(loc);
        }
    }

    unsigned int prevfile = 0;
    unsigned int line = 1;
    std::ostringstream ret;
    for (const simplecpp::Token *tok = tokens2.cfront(); tok; tok = tok->next) {
        if (writeLocations && tok->location.fileIndex != prevfile) {
            ret << "\n#line " << tok->location.line << " \"" << tok->location.file() << "\"\n";
            prevfile = tok->location.fileIndex;
            line = tok->location.line;
        }

        if (tok->previous && line >= tok->location.line) // #7912
            ret << ' ';
        bool newline = false;
        while (tok->location.line > line) {
            ret << '\n';
            line++;
            newline = true;
        }
        if (newline) {
            simplecpp::Location loc = tok->location;
            loc.col = 0U;
            if (assemblerLocations.find(loc) != assemblerLocations.end()) {
                ret << "asm();";
                while (assemblerLocations.find(loc) != assemblerLocations.end()) {
                    loc.line++;
                }
                while (tok && tok->location.line < loc.line)
                    tok = tok->next;
                if (!tok)
                    break;
                while (line < tok->location.line) {
                    ret << '\n';
                    ++line;
                }
            }
        }
        if (!tok->macro.empty())
            ret << Preprocessor::macroChar;
        ret << tok->str;
    }

    return ret.str();
}

std::string Preprocessor::getcode(const std::string &filedata, const std::string &cfg, const std::string &filename)
{
    simplecpp::OutputList outputList;
    std::vector<std::string> files;

    std::istringstream istr(filedata);
    simplecpp::TokenList tokens1(istr, files, Path::simplifyPath(filename), &outputList);
    inlineSuppressions(tokens1);
    tokens1.removeComments();
    removeComments();
    setDirectives(tokens1);

    reportOutput(outputList, true);

    if (hasErrors(outputList))
        return "";

    return getcode(tokens1, cfg, files, filedata.find("#file") != std::string::npos);
}

void Preprocessor::reportOutput(const simplecpp::OutputList &outputList, bool showerror)
{
    for (simplecpp::OutputList::const_iterator it = outputList.begin(); it != outputList.end(); ++it) {
        switch (it->type) {
        case simplecpp::Output::ERROR:
            if (it->msg.compare(0,6,"#error")!=0 || showerror)
                error(it->location.file(), it->location.line, it->msg);
            break;
        case simplecpp::Output::WARNING:
        case simplecpp::Output::PORTABILITY_BACKSLASH:
            break;
        case simplecpp::Output::MISSING_HEADER: {
            const std::string::size_type pos1 = it->msg.find_first_of("<\"");
            const std::string::size_type pos2 = it->msg.find_first_of(">\"", pos1 + 1U);
            if (pos1 < pos2 && pos2 != std::string::npos)
                missingInclude(it->location.file(), it->location.line, it->msg.substr(pos1+1, pos2-pos1-1), it->msg[pos1] == '\"' ? UserHeader : SystemHeader);
        }
        break;
        case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
        case simplecpp::Output::SYNTAX_ERROR:
            error(it->location.file(), it->location.line, it->msg);
            break;
        };
    }
}

void Preprocessor::error(const std::string &filename, unsigned int linenr, const std::string &msg)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        ErrorLogger::ErrorMessage::FileLocation loc(filename, linenr);
        locationList.push_back(loc);
    }
    _errorLogger->reportErr(ErrorLogger::ErrorMessage(locationList,
                            file0,
                            Severity::error,
                            msg,
                            "preprocessorErrorDirective",
                            false));
}

// Report that include is missing
void Preprocessor::missingInclude(const std::string &filename, unsigned int linenr, const std::string &header, HeaderTypes headerType)
{
    const std::string fname = Path::fromNativeSeparators(filename);
    if (_settings.nomsg.isSuppressed("missingInclude", fname, linenr))
        return;
    if (headerType == SystemHeader && _settings.nomsg.isSuppressed("missingIncludeSystem", fname, linenr))
        return;

    if (headerType == SystemHeader)
        missingSystemIncludeFlag = true;
    else
        missingIncludeFlag = true;
    if (_errorLogger && _settings.checkConfiguration) {

        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        if (!filename.empty()) {
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = linenr;
            loc.setfile(Path::toNativeSeparators(filename));
            locationList.push_back(loc);
        }
        ErrorLogger::ErrorMessage errmsg(locationList, file0, Severity::information,
                                         (headerType==SystemHeader) ?
                                         "Include file: <" + header + "> not found. Please note: Cppcheck does not need standard library headers to get proper results." :
                                         "Include file: \"" + header + "\" not found.",
                                         (headerType==SystemHeader) ? "missingIncludeSystem" : "missingInclude",
                                         false);
        _errorLogger->reportInfo(errmsg);
    }
}

bool Preprocessor::validateCfg(const std::string &cfg, const std::list<simplecpp::MacroUsage> &macroUsageList)
{
    bool ret = true;
    std::list<std::string> defines;
    splitcfg(cfg, defines, emptyString);
    for (std::list<std::string>::const_iterator defineIt = defines.begin(); defineIt != defines.end(); ++defineIt) {
        if (defineIt->find('=') != std::string::npos)
            continue;
        const std::string macroName(defineIt->substr(0, defineIt->find('(')));
        for (std::list<simplecpp::MacroUsage>::const_iterator usageIt = macroUsageList.begin(); usageIt != macroUsageList.end(); ++usageIt) {
            const simplecpp::MacroUsage &mu = *usageIt;
            if (mu.macroName != macroName)
                continue;
            bool directiveLocation = false;
            for (std::list<Directive>::const_iterator dirIt = directives.begin(); dirIt != directives.end(); ++dirIt) {
                if (mu.useLocation.file() == dirIt->file && mu.useLocation.line == dirIt->linenr) {
                    directiveLocation = true;
                    break;
                }
            }
            if (!directiveLocation) {
                if (_settings.isEnabled("information"))
                    validateCfgError(mu.useLocation.file(), mu.useLocation.line, cfg, macroName);
                ret = false;
            }
        }
    }

    return ret;
}

void Preprocessor::validateCfgError(const std::string &file, const unsigned int line, const std::string &cfg, const std::string &macro)
{
    const std::string id = "ConfigurationNotChecked";
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc(file, line);
    locationList.push_back(loc);
    ErrorLogger::ErrorMessage errmsg(locationList, file0, Severity::information, "Skipping configuration '" + cfg + "' since the value of '" + macro + "' is unknown. Use -D if you want to check it. You can use -U to skip it explicitly.", id, false);
    _errorLogger->reportInfo(errmsg);
}

void Preprocessor::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
{
    Settings settings2(*settings);
    Preprocessor preprocessor(settings2, errorLogger);
    settings2.checkConfiguration=true;
    preprocessor.missingInclude(emptyString, 1, emptyString, UserHeader);
    preprocessor.missingInclude(emptyString, 1, emptyString, SystemHeader);
    preprocessor.validateCfgError(emptyString, 1, "X", "X");
    preprocessor.error(emptyString, 1, "#error message");   // #error ..
}

void Preprocessor::dump(std::ostream &out) const
{
    // Create a xml directive dump.
    // The idea is not that this will be readable for humans. It's a
    // data dump that 3rd party tools could load and get useful info from.
    out << "  <directivelist>" << std::endl;

    for (std::list<Directive>::const_iterator it = directives.begin(); it != directives.end(); ++it) {
        out << "    <directive "
            << "file=\"" << it->file << "\" "
            << "linenr=\"" << it->linenr << "\" "
            // str might contain characters such as '"', '<' or '>' which
            // could result in invalid XML, so run it through toxml().
            << "str=\"" << ErrorLogger::toxml(it->str) << "\"/>" << std::endl;
    }
    out << "  </directivelist>" << std::endl;
}

static const std::uint32_t crc32Table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static std::uint32_t crc32(const std::string &data)
{
    std::uint32_t crc = ~0U;
    for (std::string::const_iterator c = data.begin(); c != data.end(); ++c) {
        crc = crc32Table[(crc ^ (unsigned char)(*c)) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ ~0U;
}

unsigned int Preprocessor::calculateChecksum(const simplecpp::TokenList &tokens1, const std::string &toolinfo) const
{
    std::ostringstream ostr;
    ostr << toolinfo << '\n';
    for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
        if (!tok->comment)
            ostr << tok->str;
    }
    for (std::map<std::string, simplecpp::TokenList *>::const_iterator it = tokenlists.begin(); it != tokenlists.end(); ++it) {
        for (const simplecpp::Token *tok = it->second->cfront(); tok; tok = tok->next) {
            if (!tok->comment)
                ostr << tok->str;
        }
    }
    return crc32(ostr.str());
}
