/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef checkersH
#define checkersH

#include <map>
#include <string>
#include <vector>

#include "config.h"

namespace checkers {
    extern CPPCHECKLIB const std::map<std::string, std::string> allCheckers;
    extern CPPCHECKLIB const std::map<std::string, std::string> premiumCheckers;

    struct CPPCHECKLIB MisraInfo {
        int a;
        int b;
        const char* str;
        int amendment;
    };

    struct CPPCHECKLIB MisraCppInfo {
        int a;
        int b;
        int c;
        const char* classification;
    };

    extern CPPCHECKLIB const char Req[]; // = "Required";
    extern CPPCHECKLIB const char Adv[]; // = "Advisory";
    extern CPPCHECKLIB const char Man[]; // = "Mandatory";
    extern CPPCHECKLIB const char Doc[]; // = "Document";

    extern CPPCHECKLIB const std::vector<MisraInfo> misraC2012Directives;
    extern CPPCHECKLIB const std::vector<MisraInfo> misraC2012Rules;
    extern CPPCHECKLIB const std::vector<MisraCppInfo> misraCpp2008Rules;
    extern CPPCHECKLIB const std::vector<MisraCppInfo> misraCpp2023Rules;

    extern CPPCHECKLIB const std::map<std::string, std::string> misraRuleSeverity;

    struct CPPCHECKLIB IdMapping {
        const char* guideline;
        const char* cppcheckId;
    };
    extern std::vector<IdMapping> idMappingMisraC;
    extern std::vector<IdMapping> idMappingMisraCpp2008;
    extern std::vector<IdMapping> idMappingMisraCpp2023;
    extern std::vector<IdMapping> idMappingAutosar;
    extern std::vector<IdMapping> idMappingCertC;
    extern std::vector<IdMapping> idMappingCertCpp;

    struct CPPCHECKLIB Info {
        const char* guideline;
        const char* classification;
    };
    extern std::vector<Info> autosarInfo;
    extern std::vector<Info> certCInfo;
    extern std::vector<Info> certCppInfo;
}

#endif
