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

#include "addoninfo.h"

#include "path.h"
#include "utils.h"

#include <fstream>
#include <string>
#include <vector>

#include "json.h"

static std::string getFullPath(const std::string &fileName, const std::string &exename) {
    if (Path::isFile(fileName))
        return fileName;

    const std::string exepath = Path::getPathFromFilename(exename);
    if (Path::isFile(exepath + fileName))
        return exepath + fileName;
    if (Path::isFile(exepath + "addons/" + fileName))
        return exepath + "addons/" + fileName;

#ifdef FILESDIR
    if (Path::isFile(FILESDIR + ("/" + fileName)))
        return FILESDIR + ("/" + fileName);
    if (Path::isFile(FILESDIR + ("/addons/" + fileName)))
        return FILESDIR + ("/addons/" + fileName);
#endif
    return "";
}

static std::string parseAddonInfo(AddonInfo& addoninfo, const picojson::value &json, const std::string &fileName, const std::string &exename) {
    const std::string& json_error = picojson::get_last_error();
    if (!json_error.empty()) {
        return "Loading " + fileName + " failed. " + json_error;
    }
    if (!json.is<picojson::object>())
        return "Loading " + fileName + " failed. Bad json.";
    picojson::object obj = json.get<picojson::object>();
    if (obj.count("args")) {
        if (!obj["args"].is<picojson::array>())
            return "Loading " + fileName + " failed. args must be array.";
        for (const picojson::value &v : obj["args"].get<picojson::array>())
            addoninfo.args += " " + v.get<std::string>();
    }

    if (obj.count("ctu")) {
        // ctu is specified in the config file
        if (!obj["ctu"].is<bool>())
            return "Loading " + fileName + " failed. ctu must be boolean.";
        addoninfo.ctu = obj["ctu"].get<bool>();
    } else {
        addoninfo.ctu = false;
    }

    if (obj.count("python")) {
        // Python was defined in the config file
        if (obj["python"].is<picojson::array>()) {
            return "Loading " + fileName +" failed. python must not be an array.";
        }
        addoninfo.python = obj["python"].get<std::string>();
    } else {
        addoninfo.python = "";
    }

    if (obj.count("executable")) {
        if (!obj["executable"].is<std::string>())
            return "Loading " + fileName + " failed. executable must be a string.";
        addoninfo.executable = getFullPath(obj["executable"].get<std::string>(), fileName);
        return "";
    }

    return addoninfo.getAddonInfo(obj["script"].get<std::string>(), exename);
}

std::string AddonInfo::getAddonInfo(const std::string &fileName, const std::string &exename) {
    if (fileName[0] == '{') {
        picojson::value json;
        const std::string err = picojson::parse(json, fileName);
        (void)err; // TODO: report
        return parseAddonInfo(*this, json, fileName, exename);
    }
    if (fileName.find('.') == std::string::npos)
        return getAddonInfo(fileName + ".py", exename);

    if (endsWith(fileName, ".py")) {
        scriptFile = Path::fromNativeSeparators(getFullPath(fileName, exename));
        if (scriptFile.empty())
            return "Did not find addon " + fileName;

        std::string::size_type pos1 = scriptFile.rfind('/');
        if (pos1 == std::string::npos)
            pos1 = 0;
        else
            pos1++;
        std::string::size_type pos2 = scriptFile.rfind('.');
        if (pos2 < pos1)
            pos2 = std::string::npos;
        name = scriptFile.substr(pos1, pos2 - pos1);

        runScript = getFullPath("runaddon.py", exename);

        return "";
    }

    if (!endsWith(fileName, ".json"))
        return "Failed to open addon " + fileName;

    std::ifstream fin(fileName);
    if (!fin.is_open())
        return "Failed to open " + fileName;
    picojson::value json;
    fin >> json;
    return parseAddonInfo(*this, json, fileName, exename);
}
