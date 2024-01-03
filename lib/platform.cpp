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

#include "platform.h"

#include "path.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#include "xml.h"

Platform::Platform()
{
    set(Type::Native);
}


bool Platform::set(Type t)
{
    switch (t) {
    case Type::Unspecified: // unknown type sizes (sizes etc are set but are not known)
    case Type::Native: // same as system this code was compile on
        type = t;
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
        if (type == Type::Unspecified) {
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
    case Type::Win32W:
    case Type::Win32A:
        type = t;
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
    case Type::Win64:
        type = t;
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
    case Type::Unix32:
        type = t;
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
    case Type::Unix64:
        type = t;
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
    case Type::File:
        // sizes are not set.
        return false;
    }
    // unsupported platform
    return false;
}

bool Platform::set(const std::string& platformstr, std::string& errstr, const std::vector<std::string>& paths, bool verbose)
{
    if (platformstr == "win32A")
        set(Type::Win32A);
    else if (platformstr == "win32W")
        set(Type::Win32W);
    else if (platformstr == "win64")
        set(Type::Win64);
    else if (platformstr == "unix32")
        set(Type::Unix32);
    else if (platformstr == "unix64")
        set(Type::Unix64);
    else if (platformstr == "native")
        set(Type::Native);
    else if (platformstr == "unspecified")
        set(Type::Unspecified);
    else if (paths.empty()) {
        errstr = "unrecognized platform: '" + platformstr + "' (no lookup).";
        return false;
    }
    else {
        bool found = false;
        for (const std::string& path : paths) {
            if (verbose)
                std::cout << "looking for platform '" + platformstr + "' in '" + path + "'" << std::endl;
            if (loadFromFile(path.c_str(), platformstr, verbose)) {
                found = true;
                break;
            }
        }
        if (!found) {
            errstr = "unrecognized platform: '" + platformstr + "'.";
            return false;
        }
    }

    return true;
}

bool Platform::loadFromFile(const char exename[], const std::string &filename, bool verbose)
{
    // TODO: only append .xml if missing
    // TODO: use native separators
    std::vector<std::string> filenames{
        filename,
        filename + ".xml",
        "platforms/" + filename,
        "platforms/" + filename + ".xml"
    };
    if (exename && (std::string::npos != Path::fromNativeSeparators(exename).find('/'))) {
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

    // open file..
    tinyxml2::XMLDocument doc;
    bool success = false;
    for (const std::string & f : filenames) {
        if (verbose)
            std::cout << "try to load platform file '" << f << "' ... ";
        if (doc.LoadFile(f.c_str()) == tinyxml2::XML_SUCCESS) {
            if (verbose)
                std::cout << "Success" << std::endl;
            success = true;
            break;
        }
        if (verbose)
            std::cout << doc.ErrorStr() << std::endl;
    }
    if (!success)
        return false;

    return loadFromXmlDocument(&doc);
}

static unsigned int xmlTextAsUInt(const tinyxml2::XMLElement* node, bool& error)
{
    unsigned int retval = 0;
    if (node->QueryUnsignedText(&retval) != tinyxml2::XML_SUCCESS)
        error = true;
    return retval;
}

bool Platform::loadFromXmlDocument(const tinyxml2::XMLDocument *doc)
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

    type = Type::File;
    return !error;
}

std::string Platform::getLimitsDefines(bool c99) const
{
    std::string s;

    // climits / limits.h
    s += "CHAR_BIT=";
    s += std::to_string(char_bit);
    s += ";SCHAR_MIN=";
    s += std::to_string(min_value(char_bit));
    s += ";SCHAR_MAX=";
    s += std::to_string(max_value(char_bit));
    s += ";UCHAR_MAX=";
    s += std::to_string(max_value(char_bit+1));
    s += ";CHAR_MIN=";
    if (defaultSign == 'u')
        s += std::to_string(min_value(char_bit));
    else
        s += std::to_string(0);
    s += ";CHAR_MAX=";
    if (defaultSign == 'u')
        s += std::to_string(max_value(char_bit+1));
    else
        s += std::to_string(max_value(char_bit));
    // TODO
    //s += ";MB_LEN_MAX=";
    s += ";SHRT_MIN=";
    s += std::to_string(min_value(short_bit));
    s += ";SHRT_MAX=";
    s += std::to_string(max_value(short_bit));
    s += ";USHRT_MAX=";
    s += std::to_string(max_value(short_bit+1));
    s += ";INT_MIN=";
    s += std::to_string(min_value(int_bit));
    s += ";INT_MAX=";
    s += std::to_string(max_value(int_bit));
    s += ";UINT_MAX=";
    s += std::to_string(max_value(int_bit+1));
    s += ";LONG_MIN=";
    s += std::to_string(min_value(long_bit));
    s += ";LONG_MAX=";
    s += std::to_string(max_value(long_bit));
    s += ";ULONG_MAX=";
    s += std::to_string(max_value(long_bit+1));
    if (c99) {
        s += ";LLONG_MIN=";
        s += std::to_string(min_value(long_long_bit));
        s += ";LLONG_MAX=";
        s += std::to_string(max_value(long_long_bit));
        s += ";ULLONG_MAX=";
        s += std::to_string(max_value(long_long_bit + 1));
    }

    // cstdint / stdint.h
    // FIXME: these are currently hard-coded in std.cfg
    /*
        INTMAX_MIN
        INTMAX_MAX
        UINTMAX_MAX
        INTN_MIN
        INTN_MAX
        UINTN_MAX
        INT_LEASTN_MIN
        INT_LEASTN_MAX
        UINT_LEASTN_MAX
        INT_FASTN_MIN
        INT_FASTN_MAX
        UINT_FASTN_MAX
        INTPTR_MIN
        INTPTR_MAX
        UINTPTR_MAX
        SIZE_MAX
        PTRDIFF_MIN
        PTRDIFF_MAX
        SIG_ATOMIC_MIN
        SIG_ATOMIC_MAX
        WCHAR_MIN
        WCHAR_MAX
        WINT_MIN
        WINT_MAX

        // function-like macros
        // implemented in std.cfg
        INTMAX_C
        UINTMAX_C
        INTN_C
        UINTN_C
     */

    // cfloat / float.h
    /*
        // TODO: implement
        FLT_RADIX

        FLT_MANT_DIG
        DBL_MANT_DIG
        LDBL_MANT_DIG

        FLT_DIG
        DBL_DIG
        LDBL_DIG

        FLT_MIN_EXP
        DBL_MIN_EXP
        LDBL_MIN_EXP

        FLT_MIN_10_EXP
        DBL_MIN_10_EXP
        LDBL_MIN_10_EXP

        FLT_MAX_EXP
        DBL_MAX_EXP
        LDBL_MAX_EXP

        FLT_MAX_10_EXP
        DBL_MAX_10_EXP
        LDBL_MAX_10_EXP

        FLT_MAX
        DBL_MAX
        LDBL_MAX

        FLT_EPSILON
        DBL_EPSILON
        LDBL_EPSILON

        FLT_MIN
        DBL_MIN
        LDBL_MIN

        FLT_ROUNDS

        // C99 / C++11 only
        FLT_EVAL_METHOD

        DECIMAL_DIG
     */

    return s;
}

std::string Platform::getLimitsDefines(Standards::cstd_t cstd) const
{
    return getLimitsDefines(cstd >= Standards::cstd_t::C99);
}

std::string Platform::getLimitsDefines(Standards::cppstd_t cppstd) const
{
    return getLimitsDefines(cppstd >= Standards::cppstd_t::CPP11);
}
