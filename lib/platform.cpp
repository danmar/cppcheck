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

#include "platform.h"
#include "tinyxml2.h"


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

bool cppcheck::Platform::platformFile(const std::string &filename)
{
    // open file..
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
        return false;

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (!rootnode || std::strcmp(rootnode->Name(), "platform") != 0)
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
