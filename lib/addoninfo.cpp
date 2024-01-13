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

static std::string validateJSON(const picojson::value &json, const std::string &schema_fn)
{
    auto validatorSchema = std::make_shared<valijson::Schema>();
    std::ifstream fin(schema_fn);
    if (!fin.is_open())
        return "Failed to open " + schema_fn;
    picojson::value schema_json;
    fin >> schema_json;

    auto schemaAdapter = valijson::adapters::PicoJsonAdapter(schema_json);
    valijson::SchemaParser parser;
    parser.populateSchema(schemaAdapter, *validatorSchema);
    //std::cout << "Schema:" << std::endl << schemaStr << std::endl << std::endl;;

    auto targetAdapter = valijson::adapters::PicoJsonAdapter(json);
    valijson::ValidationResults results;
    auto validator = valijson::Validator();
    auto isValid = validator.validate(
        *validatorSchema,
        targetAdapter,
        &results);
    if (!isValid) {
        std::string result("JSON schema validation failed");
        std::string sep(": ");
        valijson::ValidationResults::Error error;
        while (results.popError(error)) {
            result += sep;
            sep = ", ";
            for (const std::string &contextElement : error.context) {
                result += contextElement;
                result += " ";
            }
            result += error.description;
        }
        return result;
    }

    return "";
}

static std::string parseAddonInfo(AddonInfo& addoninfo, const picojson::value &json, const std::string &report_filename, const std::string &exename) {
    const std::string& json_error = picojson::get_last_error();
    if (!json_error.empty()) {
        return "Loading " + report_filename + " failed. " + json_error;
    }

    std::string schema = getFullPath("addon.schema.json",exename);
    std::string issues = validateJSON(json,schema);
    if (!issues.empty()) {
        return "Loading " + report_filename + " failed. " + issues;
    }

    // TODO: remove/complete default value handling for missing fields
    const picojson::object& obj = json.get<picojson::object>();
    {
        const auto it = obj.find("args");
        if (it != obj.cend()) {
            for (const picojson::value &v : it->second.get<picojson::array>()) {
                addoninfo.args += " " + v.get<std::string>();
            }
        }
    }

    {
        const auto it = obj.find("ctu");
        if (it != obj.cend()) {
            // ctu is specified in the config file
            addoninfo.ctu = it->second.get<bool>();
        }
        else {
            addoninfo.ctu = false;
        }
    }

    {
        const auto it = obj.find("python");
        if (it != obj.cend()) {
            // Python was defined in the config file
            addoninfo.python = it->second.get<std::string>();
        }
        else {
            addoninfo.python = "";
        }
    }

    {
        const auto it = obj.find("executable");
        if (it != obj.cend()) {
            addoninfo.executable = getFullPath(it->second.get<std::string>(), exename);
            return ""; // TODO: why bail out?
        }
    }

    const auto& val = obj.find("script")->second;
    return addoninfo.getAddonInfo(val.get<std::string>(), exename);
}

std::string AddonInfo::getAddonInfo(const std::string &fileName, const std::string &exename) {
    if (fileName[0] == '{') {
        picojson::value json;
        const std::string err = picojson::parse(json, fileName);
        (void)err; // TODO: report
        return parseAddonInfo(*this, json, "inline JSON", exename);
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
