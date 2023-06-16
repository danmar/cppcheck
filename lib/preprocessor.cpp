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


#include "preprocessor.h"

#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "path.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator> // back_inserter
#include <sstream> // IWYU pragma: keep
#include <utility>

#include <simplecpp.h>

static bool sameline(const simplecpp::Token *tok1, const simplecpp::Token *tok2)
{
    return tok1 && tok2 && tok1->location.sameline(tok2->location);
}

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

Directive::Directive(std::string _file, const int _linenr, const std::string &_str) :
    file(std::move(_file)),
    linenr(_linenr),
    str(trim(_str))
{}

char Preprocessor::macroChar = char(1);

Preprocessor::Preprocessor(const Settings& settings, ErrorLogger *errorLogger) : mSettings(settings), mErrorLogger(errorLogger)
{}

Preprocessor::~Preprocessor()
{
    for (const std::pair<const std::string, simplecpp::TokenList*>& tokenList : mTokenLists)
        delete tokenList.second;
}

namespace {
    struct BadInlineSuppression {
        BadInlineSuppression(const simplecpp::Location &l, std::string msg) : location(l), errmsg(std::move(msg)) {}
        simplecpp::Location location;
        std::string errmsg;
    };
}

static bool parseInlineSuppressionCommentToken(const simplecpp::Token *tok, std::list<Suppressions::Suppression> &inlineSuppressions, std::list<BadInlineSuppression> &bad)
{
    const std::string cppchecksuppress("cppcheck-suppress");

    const std::string &comment = tok->str();
    if (comment.size() < cppchecksuppress.size())
        return false;
    const std::string::size_type pos1 = comment.find_first_not_of("/* \t");
    if (pos1 == std::string::npos)
        return false;
    if (pos1 + cppchecksuppress.size() >= comment.size())
        return false;
    if (comment.substr(pos1, cppchecksuppress.size()) != cppchecksuppress)
        return false;

    // skip spaces after "cppcheck-suppress"
    const std::string::size_type pos2 = comment.find_first_not_of(' ', pos1+cppchecksuppress.size());
    if (pos2 == std::string::npos)
        return false;

    if (comment[pos2] == '[') {
        // multi suppress format
        std::string errmsg;
        std::vector<Suppressions::Suppression> suppressions = Suppressions::parseMultiSuppressComment(comment, &errmsg);

        if (!errmsg.empty())
            bad.emplace_back(tok->location, std::move(errmsg));

        std::copy_if(suppressions.cbegin(), suppressions.cend(), std::back_inserter(inlineSuppressions), [](const Suppressions::Suppression& s) {
            return !s.errorId.empty();
        });
    } else {
        //single suppress format
        std::string errmsg;
        Suppressions::Suppression s;
        if (!s.parseComment(comment, &errmsg))
            return false;

        if (!s.errorId.empty())
            inlineSuppressions.push_back(std::move(s));

        if (!errmsg.empty())
            bad.emplace_back(tok->location, std::move(errmsg));
    }

    return true;
}

static void addinlineSuppressions(const simplecpp::TokenList &tokens, const Settings &settings, Suppressions &suppressions, std::list<BadInlineSuppression> &bad)
{
    for (const simplecpp::Token *tok = tokens.cfront(); tok; tok = tok->next) {
        if (!tok->comment)
            continue;

        std::list<Suppressions::Suppression> inlineSuppressions;
        if (!parseInlineSuppressionCommentToken(tok, inlineSuppressions, bad))
            continue;

        if (!sameline(tok->previous, tok)) {
            // find code after comment..
            tok = tok->next;
            while (tok && tok->comment) {
                parseInlineSuppressionCommentToken(tok, inlineSuppressions, bad);
                tok = tok->next;
            }
            if (!tok)
                break;
        }

        if (inlineSuppressions.empty())
            continue;

        // Relative filename
        std::string relativeFilename(tok->location.file());
        if (settings.relativePaths) {
            for (const std::string & basePath : settings.basePaths) {
                const std::string bp = basePath + "/";
                if (relativeFilename.compare(0,bp.size(),bp)==0) {
                    relativeFilename = relativeFilename.substr(bp.size());
                }
            }
        }
        relativeFilename = Path::simplifyPath(relativeFilename);

        // special handling when suppressing { warnings for backwards compatibility
        const bool thisAndNextLine = tok->previous &&
                                     tok->previous->previous &&
                                     tok->next &&
                                     !sameline(tok->previous->previous, tok->previous) &&
                                     tok->location.line + 1 == tok->next->location.line &&
                                     tok->location.fileIndex == tok->next->location.fileIndex &&
                                     tok->previous->str() == "{";

        // Add the suppressions.
        for (Suppressions::Suppression &suppr : inlineSuppressions) {
            suppr.fileName = relativeFilename;
            suppr.lineNumber = tok->location.line;
            suppr.thisAndNextLine = thisAndNextLine;
            suppressions.addSuppression(std::move(suppr));
        }
    }
}

void Preprocessor::inlineSuppressions(const simplecpp::TokenList &tokens, Suppressions &suppressions)
{
    if (!mSettings.inlineSuppressions)
        return;
    std::list<BadInlineSuppression> err;
    ::addinlineSuppressions(tokens, mSettings, suppressions, err);
    for (std::map<std::string,simplecpp::TokenList*>::const_iterator it = mTokenLists.cbegin(); it != mTokenLists.cend(); ++it) {
        if (it->second)
            ::addinlineSuppressions(*it->second, mSettings, suppressions, err);
    }
    for (const BadInlineSuppression &bad : err) {
        error(bad.location.file(), bad.location.line, bad.errmsg);
    }
}

void Preprocessor::setDirectives(const simplecpp::TokenList &tokens)
{
    // directive list..
    mDirectives.clear();

    std::vector<const simplecpp::TokenList *> list;
    list.reserve(1U + mTokenLists.size());
    list.push_back(&tokens);
    for (std::map<std::string, simplecpp::TokenList *>::const_iterator it = mTokenLists.cbegin(); it != mTokenLists.cend(); ++it) {
        list.push_back(it->second);
    }

    for (const simplecpp::TokenList *tokenList : list) {
        for (const simplecpp::Token *tok = tokenList->cfront(); tok; tok = tok->next) {
            if ((tok->op != '#') || (tok->previous && tok->previous->location.line == tok->location.line))
                continue;
            if (tok->next && tok->next->str() == "endfile")
                continue;
            Directive directive(tok->location.file(), tok->location.line, emptyString);
            for (const simplecpp::Token *tok2 = tok; tok2 && tok2->location.line == directive.linenr; tok2 = tok2->next) {
                if (tok2->comment)
                    continue;
                if (!directive.str.empty() && (tok2->location.col > tok2->previous->location.col + tok2->previous->str().size()))
                    directive.str += ' ';
                if (directive.str == "#" && tok2->str() == "file")
                    directive.str += "include";
                else
                    directive.str += tok2->str();
            }
            mDirectives.push_back(std::move(directive));
        }
    }
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

    if (len == 1 && cond->str() == "0")
        return "0";

    if (len == 1 && cond->name) {
        if (defined.find(cond->str()) == defined.end())
            return cond->str();
    }

    if (len == 2 && cond->op == '!' && next1->name) {
        if (defined.find(next1->str()) == defined.end())
            return next1->str() + "=0";
    }

    if (len == 3 && cond->op == '(' && next1->name && next2->op == ')') {
        if (defined.find(next1->str()) == defined.end() && undefined.find(next1->str()) == undefined.end())
            return next1->str();
    }

    if (len == 3 && cond->name && next1->str() == "==" && next2->number) {
        if (defined.find(cond->str()) == defined.end())
            return cond->str() + '=' + cond->next->next->str();
    }

    std::set<std::string> configset;
    for (; sameline(iftok,cond); cond = cond->next) {
        if (cond->op == '!') {
            if (!sameline(iftok,cond->next) || !cond->next->name)
                break;
            if (cond->next->str() == "defined")
                continue;
            configset.insert(cond->next->str() + "=0");
            continue;
        }
        if (cond->str() != "defined")
            continue;
        const simplecpp::Token *dtok = cond->next;
        if (!dtok)
            break;
        if (dtok->op == '(')
            dtok = dtok->next;
        if (sameline(iftok,dtok) && dtok->name && defined.find(dtok->str()) == defined.end() && undefined.find(dtok->str()) == undefined.end())
            configset.insert(dtok->str());
    }
    std::string cfgStr;
    for (const std::string &s : configset) {
        if (!cfgStr.empty())
            cfgStr += ';';
        cfgStr += s;
    }
    return cfgStr;
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
    std::set<std::string> configs2(configs.cbegin(), configs.cend());
    std::string ret;
    for (const std::string &c : configs2) {
        if (c.empty())
            continue;
        if (c == "0")
            return "";
        if (hasDefine(userDefines, c))
            continue;
        if (!ret.empty())
            ret += ';';
        ret += c;
    }
    return ret;
}

static bool isUndefined(const std::string &cfg, const std::set<std::string> &undefined)
{
    for (std::string::size_type pos1 = 0U; pos1 < cfg.size();) {
        const std::string::size_type pos2 = cfg.find(';',pos1);
        const std::string def = (pos2 == std::string::npos) ? cfg.substr(pos1) : cfg.substr(pos1, pos2 - pos1);

        const std::string::size_type eq = def.find('=');
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
    return std::any_of(configs_if.cbegin(), configs_if.cend(),
                       [=](const std::string &cfg) {
        return hasDefine(userDefines, cfg);
    });
}

static const simplecpp::Token *gotoEndIf(const simplecpp::Token *cmdtok)
{
    int level = 0;
    while (nullptr != (cmdtok = cmdtok->next)) {
        if (cmdtok->op == '#' && !sameline(cmdtok->previous,cmdtok) && sameline(cmdtok, cmdtok->next)) {
            if (cmdtok->next->str().compare(0,2,"if")==0)
                ++level;
            else if (cmdtok->next->str() == "endif") {
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
        if (cmdtok->str() == "ifdef" || cmdtok->str() == "ifndef" || cmdtok->str() == "if") {
            std::string config;
            if (cmdtok->str() == "ifdef" || cmdtok->str() == "ifndef") {
                const simplecpp::Token *expr1 = cmdtok->next;
                if (sameline(tok,expr1) && expr1->name && !sameline(tok,expr1->next))
                    config = expr1->str();
                if (defined.find(config) != defined.end())
                    config.clear();
            } else if (cmdtok->str() == "if") {
                config = readcondition(cmdtok, defined, undefined);
            }

            // skip undefined configurations..
            if (isUndefined(config, undefined))
                config.clear();

            bool ifndef = false;
            if (cmdtok->str() == "ifndef")
                ifndef = true;
            else {
                const std::array<std::string, 6> match{"if", "!", "defined", "(", config, ")"};
                int i = 0;
                ifndef = true;
                for (const simplecpp::Token *t = cmdtok; i < match.size(); t = t->next) {
                    if (!t || t->str() != match[i++]) {
                        ifndef = false;
                        break;
                    }
                }
            }

            // include guard..
            if (ifndef && tok->location.fileIndex > 0) {
                bool includeGuard = true;
                for (const simplecpp::Token *t = tok->previous; t; t = t->previous) {
                    if (t->location.fileIndex == tok->location.fileIndex) {
                        includeGuard = false;
                        break;
                    }
                }
                if (includeGuard) {
                    configs_if.emplace_back(/*std::string()*/);
                    configs_ifndef.emplace_back(/*std::string()*/);
                    continue;
                }
            }

            configs_if.push_back((cmdtok->str() == "ifndef") ? std::string() : config);
            configs_ifndef.push_back((cmdtok->str() == "ifndef") ? config : std::string());
            ret.insert(cfg(configs_if,userDefines));
        } else if (cmdtok->str() == "elif" || cmdtok->str() == "else") {
            if (getConfigsElseIsFalse(configs_if,userDefines)) {
                tok = gotoEndIf(tok);
                if (!tok)
                    break;
                tok = tok->previous;
                continue;
            }
            if (cmdtok->str() == "else" &&
                cmdtok->next &&
                !sameline(cmdtok,cmdtok->next) &&
                sameline(cmdtok->next, cmdtok->next->next) &&
                cmdtok->next->op == '#' &&
                cmdtok->next->next->str() == "error") {
                const std::string &ifcfg = cfg(configs_if, userDefines);
                if (!ifcfg.empty()) {
                    if (!elseError.empty())
                        elseError += ';';
                    elseError += ifcfg;
                }
            }
            if (!configs_if.empty())
                configs_if.pop_back();
            if (cmdtok->str() == "elif") {
                std::string config = readcondition(cmdtok, defined, undefined);
                if (isUndefined(config,undefined))
                    config.clear();
                configs_if.push_back(std::move(config));
                ret.insert(cfg(configs_if, userDefines));
            } else if (!configs_ifndef.empty()) {
                configs_if.push_back(configs_ifndef.back());
                ret.insert(cfg(configs_if, userDefines));
            }
        } else if (cmdtok->str() == "endif" && !sameline(tok, cmdtok->next)) {
            if (!configs_if.empty())
                configs_if.pop_back();
            if (!configs_ifndef.empty())
                configs_ifndef.pop_back();
        } else if (cmdtok->str() == "error") {
            if (!configs_ifndef.empty() && !configs_ifndef.back().empty()) {
                if (configs_ifndef.size() == 1U)
                    ret.erase(emptyString);
                std::vector<std::string> configs(configs_if);
                configs.push_back(configs_ifndef.back());
                ret.erase(cfg(configs, userDefines));
                std::set<std::string> temp;
                temp.swap(ret);
                for (const std::string &c: temp) {
                    if (c.find(configs_ifndef.back()) != std::string::npos)
                        ret.insert(c);
                    else if (c.empty())
                        ret.insert("");
                    else
                        ret.insert(c + ";" + configs_ifndef.back());
                }
                if (!elseError.empty())
                    elseError += ';';
                elseError += cfg(configs_ifndef, userDefines);
            }
            if (!configs_if.empty() && !configs_if.back().empty()) {
                const std::string &last = configs_if.back();
                if (last.size() > 2U && last.compare(last.size()-2U,2,"=0") == 0) {
                    std::vector<std::string> configs(configs_if);
                    ret.erase(cfg(configs, userDefines));
                    configs[configs.size() - 1U] = last.substr(0,last.size()-2U);
                    if (configs.size() == 1U)
                        ret.erase("");
                    if (!elseError.empty())
                        elseError += ';';
                    elseError += cfg(configs, userDefines);
                }
            }
        } else if (cmdtok->str() == "define" && sameline(tok, cmdtok->next) && cmdtok->next->name) {
            defined.insert(cmdtok->next->str());
        }
    }
    if (!elseError.empty())
        ret.insert(std::move(elseError));
}


std::set<std::string> Preprocessor::getConfigs(const simplecpp::TokenList &tokens) const
{
    std::set<std::string> ret = { "" };
    if (!tokens.cfront())
        return ret;

    std::set<std::string> defined = { "__cplusplus" };

    ::getConfigs(tokens, defined, mSettings.userDefines, mSettings.userUndefs, ret);

    for (std::map<std::string, simplecpp::TokenList*>::const_iterator it = mTokenLists.cbegin(); it != mTokenLists.cend(); ++it) {
        if (!mSettings.configurationExcluded(it->first))
            ::getConfigs(*(it->second), defined, mSettings.userDefines, mSettings.userUndefs, ret);
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

    for (const std::string &c : configs) {
        if (mSettings.userUndefs.find(c) == mSettings.userUndefs.end()) {
            result[c] = getcode(tokens1, c, files, false);
        }
    }
}

void Preprocessor::preprocess(std::istream &srcCodeStream, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename, const std::list<std::string> &includePaths)
{
    (void)includePaths;

    if (mFile0.empty())
        mFile0 = filename;

    simplecpp::OutputList outputList;
    std::vector<std::string> files;
    const simplecpp::TokenList tokens1(srcCodeStream, files, filename, &outputList);

    const std::set<std::string> configs = getConfigs(tokens1);
    std::copy(configs.cbegin(), configs.cend(), std::back_inserter(resultConfigurations));

    processedFile = tokens1.stringify();
}

static void splitcfg(const std::string &cfg, std::list<std::string> &defines, const std::string &defaultValue)
{
    for (std::string::size_type defineStartPos = 0U; defineStartPos < cfg.size();) {
        const std::string::size_type defineEndPos = cfg.find(';', defineStartPos);
        std::string def = (defineEndPos == std::string::npos) ? cfg.substr(defineStartPos) : cfg.substr(defineStartPos, defineEndPos - defineStartPos);
        if (!defaultValue.empty() && def.find('=') == std::string::npos)
            def += '=' + defaultValue;
        defines.push_back(std::move(def));
        if (defineEndPos == std::string::npos)
            break;
        defineStartPos = defineEndPos + 1U;
    }
}

static simplecpp::DUI createDUI(const Settings &mSettings, const std::string &cfg, const std::string &filename)
{
    simplecpp::DUI dui;

    splitcfg(mSettings.userDefines, dui.defines, "1");
    if (!cfg.empty())
        splitcfg(cfg, dui.defines, emptyString);

    for (const std::string &def : mSettings.library.defines) {
        const std::string::size_type pos = def.find_first_of(" (");
        if (pos == std::string::npos) {
            dui.defines.push_back(def);
            continue;
        }
        std::string s = def;
        if (s[pos] == ' ') {
            s[pos] = '=';
        } else {
            s[s.find(')')+1] = '=';
        }
        dui.defines.push_back(std::move(s));
    }

    dui.undefined = mSettings.userUndefs; // -U
    dui.includePaths = mSettings.includePaths; // -I
    dui.includes = mSettings.userIncludes;  // --include
    // TODO: use mSettings.standards.stdValue instead
    if (Path::isCPP(filename))
        dui.std = mSettings.standards.getCPP();
    else
        dui.std = mSettings.standards.getC();
    dui.clearIncludeCache = mSettings.clearIncludeCache;
    return dui;
}

bool Preprocessor::hasErrors(const simplecpp::Output &output)
{
    switch (output.type) {
    case simplecpp::Output::ERROR:
    case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
    case simplecpp::Output::SYNTAX_ERROR:
    case simplecpp::Output::UNHANDLED_CHAR_ERROR:
    case simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND:
        return true;
    case simplecpp::Output::WARNING:
    case simplecpp::Output::MISSING_HEADER:
    case simplecpp::Output::PORTABILITY_BACKSLASH:
        break;
    }
    return false;
}

bool Preprocessor::hasErrors(const simplecpp::OutputList &outputList)
{
    const auto it = std::find_if(outputList.cbegin(), outputList.cend(), [](const simplecpp::Output &output) {
        return hasErrors(output);
    });
    return it != outputList.cend();
}

void Preprocessor::handleErrors(const simplecpp::OutputList& outputList, bool throwError)
{
    const bool showerror = (!mSettings.userDefines.empty() && !mSettings.force);
    reportOutput(outputList, showerror);
    if (throwError) {
        const auto it = std::find_if(outputList.cbegin(), outputList.cend(), [](const simplecpp::Output &output){
            return hasErrors(output);
        });
        if (it != outputList.cend()) {
            throw *it;
        }
    }
}

bool Preprocessor::loadFiles(const simplecpp::TokenList &rawtokens, std::vector<std::string> &files)
{
    const simplecpp::DUI dui = createDUI(mSettings, emptyString, files[0]);

    simplecpp::OutputList outputList;
    mTokenLists = simplecpp::load(rawtokens, files, dui, &outputList);
    handleErrors(outputList, false);
    return !hasErrors(outputList);
}

void Preprocessor::removeComments()
{
    for (std::pair<const std::string, simplecpp::TokenList*>& tokenList : mTokenLists) {
        if (tokenList.second)
            tokenList.second->removeComments();
    }
}

void Preprocessor::setPlatformInfo(simplecpp::TokenList *tokens) const
{
    tokens->sizeOfType["bool"]          = mSettings.platform.sizeof_bool;
    tokens->sizeOfType["short"]         = mSettings.platform.sizeof_short;
    tokens->sizeOfType["int"]           = mSettings.platform.sizeof_int;
    tokens->sizeOfType["long"]          = mSettings.platform.sizeof_long;
    tokens->sizeOfType["long long"]     = mSettings.platform.sizeof_long_long;
    tokens->sizeOfType["float"]         = mSettings.platform.sizeof_float;
    tokens->sizeOfType["double"]        = mSettings.platform.sizeof_double;
    tokens->sizeOfType["long double"]   = mSettings.platform.sizeof_long_double;
    tokens->sizeOfType["bool *"]        = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["short *"]       = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["int *"]         = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["long *"]        = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["long long *"]   = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["float *"]       = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["double *"]      = mSettings.platform.sizeof_pointer;
    tokens->sizeOfType["long double *"] = mSettings.platform.sizeof_pointer;
}

simplecpp::TokenList Preprocessor::preprocess(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, bool throwError)
{
    const simplecpp::DUI dui = createDUI(mSettings, cfg, files[0]);

    simplecpp::OutputList outputList;
    std::list<simplecpp::MacroUsage> macroUsage;
    std::list<simplecpp::IfCond> ifCond;
    simplecpp::TokenList tokens2(files);
    simplecpp::preprocess(tokens2, tokens1, files, mTokenLists, dui, &outputList, &macroUsage, &ifCond);
    mMacroUsage = macroUsage;
    mIfCond = ifCond;

    handleErrors(outputList, throwError);

    tokens2.removeComments();

    return tokens2;
}

std::string Preprocessor::getcode(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, const bool writeLocations)
{
    simplecpp::TokenList tokens2 = preprocess(tokens1, cfg, files, false);
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
        while (tok->location.line > line) {
            ret << '\n';
            line++;
        }
        if (!tok->macro.empty())
            ret << Preprocessor::macroChar;
        ret << tok->str();
    }

    return ret.str();
}

void Preprocessor::reportOutput(const simplecpp::OutputList &outputList, bool showerror)
{
    for (const simplecpp::Output &out : outputList) {
        switch (out.type) {
        case simplecpp::Output::ERROR:
            if (out.msg.compare(0,6,"#error")!=0 || showerror)
                error(out.location.file(), out.location.line, out.msg);
            break;
        case simplecpp::Output::WARNING:
        case simplecpp::Output::PORTABILITY_BACKSLASH:
            break;
        case simplecpp::Output::MISSING_HEADER: {
            const std::string::size_type pos1 = out.msg.find_first_of("<\"");
            const std::string::size_type pos2 = out.msg.find_first_of(">\"", pos1 + 1U);
            if (pos1 < pos2 && pos2 != std::string::npos)
                missingInclude(out.location.file(), out.location.line, out.msg.substr(pos1+1, pos2-pos1-1), out.msg[pos1] == '\"' ? UserHeader : SystemHeader);
        }
        break;
        case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
        case simplecpp::Output::SYNTAX_ERROR:
        case simplecpp::Output::UNHANDLED_CHAR_ERROR:
            error(out.location.file(), out.location.line, out.msg);
            break;
        case simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND:
            error(emptyString, 0, out.msg);
            break;
        }
    }
}

void Preprocessor::error(const std::string &filename, unsigned int linenr, const std::string &msg)
{
    std::list<ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        std::string file = Path::fromNativeSeparators(filename);
        if (mSettings.relativePaths)
            file = Path::getRelativePath(file, mSettings.basePaths);

        ErrorMessage::FileLocation loc(file, linenr, 0);
        locationList.push_back(std::move(loc));
    }
    mErrorLogger->reportErr(ErrorMessage(locationList,
                                         mFile0,
                                         Severity::error,
                                         msg,
                                         "preprocessorErrorDirective",
                                         Certainty::normal));
}

// Report that include is missing
void Preprocessor::missingInclude(const std::string &filename, unsigned int linenr, const std::string &header, HeaderTypes headerType)
{
    if (!mSettings.checks.isEnabled(Checks::missingInclude) || !mErrorLogger)
        return;

    std::list<ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        locationList.emplace_back(filename, linenr);
    }
    ErrorMessage errmsg(std::move(locationList), mFile0, Severity::information,
                        (headerType==SystemHeader) ?
                        "Include file: <" + header + "> not found. Please note: Cppcheck does not need standard library headers to get proper results." :
                        "Include file: \"" + header + "\" not found.",
                        (headerType==SystemHeader) ? "missingIncludeSystem" : "missingInclude",
                        Certainty::normal);
    mErrorLogger->reportErr(errmsg);
}

void Preprocessor::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
{
    Settings settings2(*settings);
    Preprocessor preprocessor(settings2, errorLogger);
    preprocessor.missingInclude(emptyString, 1, emptyString, UserHeader);
    preprocessor.missingInclude(emptyString, 1, emptyString, SystemHeader);
    preprocessor.error(emptyString, 1, "#error message");   // #error ..
}

void Preprocessor::dump(std::ostream &out) const
{
    // Create a xml dump.

    out << "  <directivelist>" << std::endl;
    for (const Directive &dir : mDirectives) {
        out << "    <directive "
            << "file=\"" << ErrorLogger::toxml(dir.file) << "\" "
            << "linenr=\"" << dir.linenr << "\" "
            // str might contain characters such as '"', '<' or '>' which
            // could result in invalid XML, so run it through toxml().
            << "str=\"" << ErrorLogger::toxml(dir.str) << "\"/>" << std::endl;
    }
    out << "  </directivelist>" << std::endl;

    if (!mMacroUsage.empty()) {
        out << "  <macro-usage>" << std::endl;
        for (const simplecpp::MacroUsage &macroUsage: mMacroUsage) {
            out << "    <macro"
                << " name=\"" << macroUsage.macroName << "\""
                << " file=\"" << ErrorLogger::toxml(macroUsage.macroLocation.file()) << "\""
                << " line=\"" << macroUsage.macroLocation.line << "\""
                << " column=\"" << macroUsage.macroLocation.col << "\""
                << " usefile=\"" << ErrorLogger::toxml(macroUsage.useLocation.file()) << "\""
                << " useline=\"" << macroUsage.useLocation.line << "\""
                << " usecolumn=\"" << macroUsage.useLocation.col << "\""
                << " is-known-value=\"" << (macroUsage.macroValueKnown ? "true" : "false") << "\""
                << "/>" << std::endl;
        }
        out << "  </macro-usage>" << std::endl;
    }

    if (!mIfCond.empty()) {
        out << "  <simplecpp-if-cond>" << std::endl;
        for (const simplecpp::IfCond &ifCond: mIfCond) {
            out << "    <if-cond"
                << " file=\"" << ErrorLogger::toxml(ifCond.location.file()) << "\""
                << " line=\"" << ifCond.location.line << "\""
                << " column=\"" << ifCond.location.col << "\""
                << " E=\"" << ErrorLogger::toxml(ifCond.E) << "\""
                << " result=\"" << ifCond.result << "\""
                << "/>" << std::endl;
        }
        out << "  </simplecpp-if-cond>" << std::endl;
    }
}

std::size_t Preprocessor::calculateHash(const simplecpp::TokenList &tokens1, const std::string &toolinfo) const
{
    std::string hashData = toolinfo;
    for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
        if (!tok->comment)
            hashData += tok->str();
    }
    for (std::map<std::string, simplecpp::TokenList *>::const_iterator it = mTokenLists.cbegin(); it != mTokenLists.cend(); ++it) {
        for (const simplecpp::Token *tok = it->second->cfront(); tok; tok = tok->next) {
            if (!tok->comment)
                hashData += tok->str();
        }
    }
    return (std::hash<std::string>{})(hashData);
}

void Preprocessor::simplifyPragmaAsm(simplecpp::TokenList *tokenList) const
{
    Preprocessor::simplifyPragmaAsmPrivate(tokenList);
    for (const std::pair<const std::string, simplecpp::TokenList*>& list : mTokenLists) {
        Preprocessor::simplifyPragmaAsmPrivate(list.second);
    }
}

void Preprocessor::simplifyPragmaAsmPrivate(simplecpp::TokenList *tokenList)
{
    // assembler code..
    for (simplecpp::Token *tok = tokenList->front(); tok; tok = tok->next) {
        if (tok->op != '#')
            continue;
        if (sameline(tok, tok->previousSkipComments()))
            continue;

        const simplecpp::Token * const tok2 = tok->nextSkipComments();
        if (!tok2 || !sameline(tok, tok2) || tok2->str() != "pragma")
            continue;

        const simplecpp::Token * const tok3 = tok2->nextSkipComments();
        if (!tok3 || !sameline(tok, tok3) || tok3->str() != "asm")
            continue;

        const simplecpp::Token *endasm = tok3;
        while ((endasm = endasm->next) != nullptr) {
            if (endasm->op != '#' || sameline(endasm,endasm->previousSkipComments()))
                continue;
            const simplecpp::Token * const endasm2 = endasm->nextSkipComments();
            if (!endasm2 || !sameline(endasm, endasm2) || endasm2->str() != "pragma")
                continue;
            const simplecpp::Token * const endasm3 = endasm2->nextSkipComments();
            if (!endasm3 || !sameline(endasm2, endasm3) || endasm3->str() != "endasm")
                continue;
            while (sameline(endasm,endasm3))
                endasm = endasm->next;
            break;
        }

        const simplecpp::Token * const tok4 = tok3->next;
        tok->setstr("asm");
        const_cast<simplecpp::Token *>(tok2)->setstr("(");
        const_cast<simplecpp::Token *>(tok3)->setstr(")");
        const_cast<simplecpp::Token *>(tok4)->setstr(";");
        while (tok4->next != endasm)
            tokenList->deleteToken(tok4->next);
    }
}
