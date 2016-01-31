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

#include "settings.h"
#include "preprocessor.h"       // Preprocessor
#include "utils.h"
#include "tinyxml2.h"

#include <fstream>
#include <set>

Settings::Settings()
    : _terminated(false),
      debug(false),
      debugnormal(false),
      debugwarnings(false),
      dump(false),
      exceptionHandling(false),
      inconclusive(false),
      jointSuppressionReport(false),
      experimental(false),
      quiet(false),
      inlineSuppressions(false),
      verbose(false),
      force(false),
      relativePaths(false),
      xml(false), xml_version(1),
      jobs(1),
      loadAverage(0),
      exitCode(0),
      showtime(SHOWTIME_NONE),
      preprocessOnly(false),
      maxConfigs(12),
      enforcedLang(None),
      reportProgress(false),
      checkConfiguration(false),
      checkLibrary(false)
{
    // This assumes the code you are checking is for the same architecture this is compiled on.
#if defined(_WIN64)
    platform(Win64);
#elif defined(_WIN32)
    platform(Win32A);
#else
    platform(Native);
#endif
}

namespace {
    const std::set<std::string> id = make_container< std::set<std::string> > ()
                                     << "warning"
                                     << "style"
                                     << "performance"
                                     << "portability"
                                     << "information"
                                     << "missingInclude"
                                     << "unusedFunction"
#ifdef CHECK_INTERNAL
                                     << "internal"
#endif
                                     ;
}
std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("cppcheck: --enable parameter is empty");
            const std::string errmsg(addEnabled(str.substr(prevPos, pos - prevPos)));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("cppcheck: --enable parameter is empty");
        return addEnabled(str.substr(prevPos));
    }

    if (str == "all") {
        std::set<std::string>::const_iterator it;
        for (it = id.begin(); it != id.end(); ++it) {
            if (*it == "internal")
                continue;

            _enabled.insert(*it);
        }
    } else if (id.find(str) != id.end()) {
        _enabled.insert(str);
        if (str == "information") {
            _enabled.insert("missingInclude");
        }
    } else {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string("");
}


bool Settings::append(const std::string &filename)
{
    std::ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(fin, line)) {
        _append += line + "\n";
    }
    Preprocessor::preprocessWhitespaces(_append);
    return true;
}

const std::string &Settings::append() const
{
    return _append;
}

bool Settings::platform(PlatformType type)
{
    switch (type) {
    case Unspecified:
        platformType = type;
        sizeof_bool = sizeof(bool);
        sizeof_short = sizeof(short);
        sizeof_int = sizeof(int);
        sizeof_long = sizeof(long);
        sizeof_long_long = sizeof(long long);
        sizeof_float = sizeof(float);
        sizeof_double = sizeof(double);
        sizeof_long_double = sizeof(long double);
        sizeof_wchar_t = sizeof(wchar_t);
        sizeof_size_t = sizeof(std::size_t);
        sizeof_pointer = sizeof(void *);
        defaultSign = '\0';
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    case Native: // same as system this code was compile on
        platformType = type;
        sizeof_bool = sizeof(bool);
        sizeof_short = sizeof(short);
        sizeof_int = sizeof(int);
        sizeof_long = sizeof(long);
        sizeof_long_long = sizeof(long long);
        sizeof_float = sizeof(float);
        sizeof_double = sizeof(double);
        sizeof_long_double = sizeof(long double);
        sizeof_wchar_t = sizeof(wchar_t);
        sizeof_size_t = sizeof(std::size_t);
        sizeof_pointer = sizeof(void *);
        {
            char x = -1;
            defaultSign = (x < 0) ? 's' : 'u';
        }
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    case Win32W:
    case Win32A:
        platformType = type;
        sizeof_bool = 1; // 4 in Visual C++ 4.2
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        defaultSign = '\0';
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    case Win64:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        defaultSign = '\0';
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    case Unix32:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 12;
        sizeof_wchar_t = 4;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        defaultSign = '\0';
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    case Unix64:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 8;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 16;
        sizeof_wchar_t = 4;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        defaultSign = '\0';
        char_bit = 8;
        short_bit = char_bit * sizeof_short;
        int_bit = char_bit * sizeof_int;
        long_bit = char_bit * sizeof_long;
        long_long_bit = char_bit * sizeof_long_long;
        return true;
    }

    // unsupported platform
    return false;
}

bool Settings::platformFile(const std::string &filename)
{
    // open file..
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_NO_ERROR)
        return false;

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (!rootnode || std::strcmp(rootnode->Name(),"platform") != 0)
        return false;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "default-sign") == 0)
            defaultSign = *node->GetText();
        else if (std::strcmp(node->Name(), "char_bit") == 0)
            char_bit = std::atoi(node->GetText());
        else if (std::strcmp(node->Name(), "sizeof") == 0) {
            for (const tinyxml2::XMLElement *sz = node->FirstChildElement(); sz; sz = sz->NextSiblingElement()) {
                if (std::strcmp(node->Name(), "short") == 0)
                    sizeof_short = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "int") == 0)
                    sizeof_int = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "long") == 0)
                    sizeof_long = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "long-long") == 0)
                    sizeof_long_long = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "float") == 0)
                    sizeof_float = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "double") == 0)
                    sizeof_double = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "long-double") == 0)
                    sizeof_long_double = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "pointer") == 0)
                    sizeof_pointer = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "size_t") == 0)
                    sizeof_size_t = std::atoi(node->GetText());
                else if (std::strcmp(node->Name(), "wchar_t") == 0)
                    sizeof_wchar_t = std::atoi(node->GetText());
            }
        }
    }

    short_bit = char_bit * sizeof_short;
    int_bit = char_bit * sizeof_int;
    long_bit = char_bit * sizeof_long;
    long_long_bit = char_bit * sizeof_long_long;

    return true;
}
