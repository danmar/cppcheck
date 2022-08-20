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

//---------------------------------------------------------------------------
#ifndef platformH
#define platformH
//---------------------------------------------------------------------------

#include "config.h"

#include <climits>
#include <string>

/// @addtogroup Core
/// @{

namespace tinyxml2 {
    class XMLDocument;
}

namespace cppcheck {

    /**
     * @brief Platform settings
     */
    class CPPCHECKLIB Platform {
    private:
        static long long min_value(int bit) {
            if (bit >= 64)
                return LLONG_MIN;
            return -(1LL << (bit-1));
        }

        static long long max_value(int bit) {
            if (bit >= 64)
                return (~0ULL) >> 1;
            return (1LL << (bit-1)) - 1LL;
        }
    public:
        Platform();
        virtual ~Platform() {}

        bool isIntValue(long long value) const {
            return value >= min_value(int_bit) && value <= max_value(int_bit);
        }

        bool isIntValue(unsigned long long value) const {
            unsigned long long intMax = max_value(int_bit);
            return value <= intMax;
        }

        bool isLongValue(long long value) const {
            return value >= min_value(long_bit) && value <= max_value(long_bit);
        }

        bool isLongValue(unsigned long long value) const {
            unsigned long long longMax = max_value(long_bit);
            return value <= longMax;
        }

        bool isLongLongValue(unsigned long long value) const {
            unsigned long long longLongMax = max_value(long_long_bit);
            return value <= longLongMax;
        }

        nonneg int char_bit;       /// bits in char
        nonneg int short_bit;      /// bits in short
        nonneg int int_bit;        /// bits in int
        nonneg int long_bit;       /// bits in long
        nonneg int long_long_bit;  /// bits in long long

        /** size of standard types */
        nonneg int sizeof_bool;
        nonneg int sizeof_short;
        nonneg int sizeof_int;
        nonneg int sizeof_long;
        nonneg int sizeof_long_long;
        nonneg int sizeof_float;
        nonneg int sizeof_double;
        nonneg int sizeof_long_double;
        nonneg int sizeof_wchar_t;
        nonneg int sizeof_size_t;
        nonneg int sizeof_pointer;

        char defaultSign;  // unsigned:'u', signed:'s', unknown:'\0'

        enum PlatformType {
            Unspecified, // No platform specified
            Native, // whatever system this code was compiled on
            Win32A,
            Win32W,
            Win64,
            Unix32,
            Unix64,
            PlatformFile
        };

        /** platform type */
        PlatformType platformType;

        /** set the platform type for predefined platforms */
        bool platform(PlatformType type);

        /**
         * load platform file
         * @param exename application path
         * @param filename platform filename
         * @return returns true if file was loaded successfully
         */
        bool loadPlatformFile(const char exename[], const std::string &filename);

        /** load platform from xml document, primarily for testing */
        bool loadFromXmlDocument(const tinyxml2::XMLDocument *doc);

        /**
         * @brief Returns true if platform type is Windows
         * @return true if Windows platform type.
         */
        bool isWindowsPlatform() const {
            return platformType == Win32A ||
                   platformType == Win32W ||
                   platformType == Win64;
        }

        const char *platformString() const {
            return platformString(platformType);
        }

        static const char *platformString(PlatformType pt) {
            switch (pt) {
            case Unspecified:
                return "Unspecified";
            case Native:
                return "Native";
            case Win32A:
                return "win32A";
            case Win32W:
                return "win32W";
            case Win64:
                return "win64";
            case Unix32:
                return "unix32";
            case Unix64:
                return "unix64";
            case PlatformFile:
                return "platformFile";
            default:
                return "unknown";
            }
        }

        long long unsignedCharMax() const {
            return max_value(char_bit + 1);
        }

        long long signedCharMax() const {
            return max_value(char_bit);
        }

        long long signedCharMin() const {
            return min_value(char_bit);
        }
    };

}

/// @}
//---------------------------------------------------------------------------
#endif // platformH
