/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "platform.h"

#include "path.h"

#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

#include <tinyxml2.h>

cppcheck::Platform::Platform()
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


bool cppcheck::Platform::platform(cppcheck::Platform::PlatformType type)
{
    switch (type) {
    case Unspecified: // unknown type sizes (sizes etc are set but are not known)
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
        if (type == Unspecified) {
            defaultSign = '\0';
        } else {
            defaultSign = std::numeric_limits<char>::is_signed ? 's' : 'u';
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
    case PlatformFile:
        // sizes are not set.
        return false;
    }
    // unsupported platform
    return false;
}

bool cppcheck::Platform::loadPlatformFile(const char exename[], const std::string &filename)
{
    // open file..
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        std::vector<std::string> filenames;
        filenames.push_back(filename + ".xml");
        if (exename && (std::string::npos != Path::fromNativeSeparators(exename).find('/'))) {
            filenames.push_back(Path::getPathFromFilename(Path::fromNativeSeparators(exename)) + filename);
            filenames.push_back(Path::getPathFromFilename(Path::fromNativeSeparators(exename)) + filename);
            filenames.push_back(Path::getPathFromFilename(Path::fromNativeSeparators(exename)) + "platforms/" + filename);
            filenames.push_back(Path::getPathFromFilename(Path::fromNativeSeparators(exename)) + "platforms/" + filename + ".xml");
        }
#ifdef FILESDIR
        std::string filesdir = FILESDIR;
        if (!filesdir.empty() && filesdir[filesdir.size()-1] != '/')
            filesdir += '/';
        filenames.push_back(filesdir + ("platforms/" + filename));
        filenames.push_back(filesdir + ("platforms/" + filename + ".xml"));
#endif
        bool success = false;
        for (const std::string & f : filenames) {
            if (doc.LoadFile(f.c_str()) == tinyxml2::XML_SUCCESS) {
                success = true;
                break;
            }
        }
        if (!success)
            return false;
    }

    return loadFromXmlDocument(&doc);
}

static unsigned int xmlTextAsUInt(const tinyxml2::XMLElement* node, bool& error)
{
    unsigned int retval = 0;
    if (node->QueryUnsignedText(&retval) != tinyxml2::XML_SUCCESS)
        error = true;
    return retval;
}

bool cppcheck::Platform::loadFromXmlDocument(const tinyxml2::XMLDocument *doc)
{
    const tinyxml2::XMLElement * const rootnode = doc->FirstChildElement();

    if (!rootnode || std::strcmp(rootnode->Name(), "platform") != 0)
        return false;

    bool error = false;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        if (std::strcmp(node->Name(), "default-sign") == 0) {
            const char* str = node->GetText();
            if (str)
                defaultSign = *str;
            else
                error = true;
        } else if (std::strcmp(node->Name(), "char_bit") == 0)
            char_bit = xmlTextAsUInt(node, error);
        else if (std::strcmp(node->Name(), "sizeof") == 0) {
            for (const tinyxml2::XMLElement *sz = node->FirstChildElement(); sz; sz = sz->NextSiblingElement()) {
                if (std::strcmp(sz->Name(), "short") == 0)
                    sizeof_short = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "bool") == 0)
                    sizeof_bool = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "int") == 0)
                    sizeof_int = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "long") == 0)
                    sizeof_long = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "long-long") == 0)
                    sizeof_long_long = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "float") == 0)
                    sizeof_float = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "double") == 0)
                    sizeof_double = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "long-double") == 0)
                    sizeof_long_double = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "pointer") == 0)
                    sizeof_pointer = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "size_t") == 0)
                    sizeof_size_t = xmlTextAsUInt(sz, error);
                else if (std::strcmp(sz->Name(), "wchar_t") == 0)
                    sizeof_wchar_t = xmlTextAsUInt(sz, error);
            }
        }
    }

    short_bit = char_bit * sizeof_short;
    int_bit = char_bit * sizeof_int;
    long_bit = char_bit * sizeof_long;
    long_long_bit = char_bit * sizeof_long_long;

    platformType = PlatformFile;
    return !error;
}
