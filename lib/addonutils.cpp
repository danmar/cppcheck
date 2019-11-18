/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#include "addonutils.h"

#include "path.h"
#include "utils.h"

#define PICOJSON_USE_INT64
#include <picojson.h>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

static inline void throwAddonError(const std::string &msg)
{
    throw InternalError(nullptr, msg, InternalError::ADDON_ERROR);
}

Addon::Addon(const std::string &addonFileName, const std::string &exename)
    : mArgs("")
{
    getAddonInfo(addonFileName, exename);
}

Addon::~Addon() {}

void Addon::getAddonInfo(const std::string &fileName, const std::string &exename)
{
    if (fileName.find(".") == std::string::npos)
        return getAddonInfo(fileName + ".py", exename);

    if (endsWith(fileName, ".py", 3)) {
        mScriptFile = getFullPath(fileName, exename);
        if (mScriptFile.empty())
            throwAddonError("Did not find addon " + fileName);

        std::string::size_type pos1 = mScriptFile.rfind("/");
        if (pos1 == std::string::npos)
            pos1 = 0;
        else
            pos1++;
        std::string::size_type pos2 = mScriptFile.rfind(".");
        if (pos2 < pos1)
            pos2 = std::string::npos;
        mName = mScriptFile.substr(pos1, pos2 - pos1);

        return;
    } else {
        if (!endsWith(fileName, ".json", 5))
            throwAddonError("Failed to open addon " + fileName);

        std::ifstream fin(fileName);
        if (!fin.is_open())
            throwAddonError("Failed to open " + fileName);

        picojson::value json;
        fin >> json;
        if (!json.is<picojson::object>())
            throwAddonError("Loading " + fileName + " failed. Bad json.");
        picojson::object obj = json.get<picojson::object>();
        if (obj.count("args")) {
            if (!obj["args"].is<picojson::array>())
                throwAddonError("Loading " + fileName + " failed. args must be array.");
            for (const picojson::value &v : obj["args"].get<picojson::array>())
                mArgs += " " + v.get<std::string>();
        }

        return getAddonInfo(obj["script"].get<std::string>(), exename);
    }
}

void Addon::appendArgs(const std::string &arg)
{
    if (arg.empty())
        return;

    if (arg[0] != ' ')
        mArgs.append(' ' + arg);
    else
        mArgs.append(arg);
}

void Addon::setEnv(const std::string &variable, const std::string &value)
{
    if (variable.empty())
        return;
    mEnv[variable] = value;
}

void Addon::clearEnv()
{
    mEnv.clear();
}

std::string Addon::getFullPath(const std::string &fileName, const std::string &exename) const
{
    if (Path::fileExists(fileName))
        return fileName;

    const std::string exepath = Path::getPathFromFilename(exename);
    if (Path::fileExists(exepath + fileName))
        return exepath + fileName;
    if (Path::fileExists(exepath + "addons/" + fileName))
        return exepath + "addons/" + fileName;

#ifdef FILESDIR
    if (Path::fileExists(FILESDIR + ("/" + fileName)))
        return FILESDIR + ("/" + fileName);
    if (Path::fileExists(FILESDIR + ("/addons/" + fileName)))
        return FILESDIR + ("/addons/" + fileName);
#endif
    return "";
}

std::string Addon::getEnvString() const
{
    std::string result = "";
    for (auto const &var : mEnv)
        result += (var.first + "=" + var.second + " ");
    return result;
}

std::string Addon::execute(const std::string &dumpFile) const
{
    const std::string cmd = getEnvString() + "python \"" + mScriptFile + "\" --cli" + mArgs + " \"" + dumpFile + "\"";

#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif
    if (!pipe)
        return "";
    char buffer[1024];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

std::unique_ptr<ErrorLogger::ErrorMessage> Addon::getErrorMessage(const std::string &line) const
{
    if (line.compare(0,1,"{") != 0)
        return nullptr;

    picojson::value res;
    std::istringstream istr2(line);
    istr2 >> res;
    if (!res.is<picojson::object>())
        return nullptr;

    picojson::object obj = res.get<picojson::object>();

    const std::string fileName = obj["file"].get<std::string>();
    const int64_t lineNumber = obj["linenr"].get<int64_t>();
    const int64_t column = obj["column"].get<int64_t>();

    std::unique_ptr<ErrorLogger::ErrorMessage> errmsg(new(ErrorLogger::ErrorMessage));

    errmsg->callStack.emplace_back(ErrorLogger::ErrorMessage::FileLocation(fileName, lineNumber, column));

    errmsg->id = obj["addon"].get<std::string>() + "-" + obj["errorId"].get<std::string>();
    const std::string text = obj["message"].get<std::string>();
    errmsg->setmsg(text);
    const std::string severity = obj["severity"].get<std::string>();
    errmsg->severity = Severity::fromString(severity);
    if (errmsg->severity == Severity::SeverityType::none)
        return nullptr;
    errmsg->file0 = fileName;

    return errmsg;
}