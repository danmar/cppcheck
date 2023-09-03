import glob
import re

print("""/*
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

#pragma once

#include <map>
#include <string>
#include <vector>

namespace checkers {

static std::map<std::string, std::string> allCheckers{""")

for filename in glob.glob('../lib/*.cpp'):
    for line in open(filename,'rt'):
        res = re.match(r'[ \t]*logChecker\(\s*"([:_a-zA-Z0-9]+)"\s*\);.*', line)
        if res is None:
            continue
        req = ''
        if line.find('//')>0:
            req = line[line.find('//')+2:].strip()
        print('    {"%s","%s"},' % (res.group(1), req))
print("};\n\n")

print('static std::map<std::string, std::string> premiumCheckers{')

premium_checkers = """
$ grep logChecker src/*.cpp | sed 's/.*logChecker/logChecker/' | sort > 1.txt
logChecker("Autosar: A0-1-3"); // style
logChecker("Autosar: A0-4-2"); // style
logChecker("Autosar: A0-4-4"); // style
logChecker("Autosar: A10-1-1"); // style
logChecker("Autosar: A11-0-2"); // style
logChecker("Autosar: A11-3-1"); // style
logChecker("Autosar: A13-2-1"); // style
logChecker("Autosar: A13-2-3"); // style
logChecker("Autosar: A13-5-2"); // style
logChecker("Autosar: A13-5-5"); // style
logChecker("Autosar: A15-1-2"); // style
logChecker("Autosar: A15-3-5"); // style
logChecker("Autosar: A16-6-1"); // style
logChecker("Autosar: A16-7-1"); // style
logChecker("Autosar: A18-0-3"); // style
logChecker("Autosar: A18-1-1"); // style
logChecker("Autosar: A18-1-2"); // style
logChecker("Autosar: A18-1-3"); // style
logChecker("Autosar: A18-5-1"); // style
logChecker("Autosar: A18-9-1"); // style
logChecker("Autosar: A2-11-1"); // style
logChecker("Autosar: A2-13-1"); // style
logChecker("Autosar: A2-13-3"); // style
logChecker("Autosar: A2-13-5"); // style
logChecker("Autosar: A2-13-6"); // style
logChecker("Autosar: A2-5-2"); // style
logChecker("Autosar: A3-1-3"); // style
logChecker("Autosar: A3-1-4"); // style
logChecker("Autosar: A3-3-1"); // style
logChecker("Autosar: A4-10-1"); // style
logChecker("Autosar: A4-7-1"); // style
logChecker("Autosar: A5-0-2"); // style
logChecker("Autosar: A5-0-3"); // style
logChecker("Autosar: A5-0-4"); // style
logChecker("Autosar: A5-1-1"); // style
logChecker("Autosar: A5-1-2"); // style
logChecker("Autosar: A5-1-3"); // style
logChecker("Autosar: A5-16-1"); // style
logChecker("Autosar: A5-1-6"); // style
logChecker("Autosar: A5-1-7"); // style
logChecker("Autosar: A5-2-1"); // style
logChecker("Autosar: A5-2-4"); // style
logChecker("Autosar: A6-5-3"); // style
logChecker("Autosar: A7-1-4"); // style
logChecker("Autosar: A7-1-6"); // style
logChecker("Autosar: A7-1-7"); // style
logChecker("Autosar: A8-4-1"); // style
logChecker("Autosar: A8-5-3"); // style
logChecker("Autosar: A9-3-1"); // style
logChecker("Cert C: ARR30-C"); // warning
logChecker("Cert C: ARR32-C"); // warning
logChecker("Cert C: ARR37-C"); // warning
logChecker("Cert C: ARR38-C");
logChecker("Cert C: ARR39-C"); // warning
logChecker("Cert C: CON30-C"); // style
logChecker("Cert C: CON31-C"); // style
logChecker("Cert C: CON32-C"); // style
logChecker("Cert C: CON33-C"); // style
logChecker("Cert C: CON34-C"); // warning
logChecker("Cert C: CON35-C"); // warning
logChecker("Cert C: CON36-C"); // style
logChecker("Cert C: CON37-C"); // style
logChecker("Cert C: CON38-C"); // warning
logChecker("Cert C: CON39-C"); // warning
logChecker("Cert C: CON40-C"); // warning
logChecker("Cert C: CON41-C"); // style
logChecker("Cert C++: CON51-CPP");
logChecker("Cert C++: CON52-CPP"); // style
logChecker("Cert C++: CON53-CPP"); // style
logChecker("Cert C++: CON54-CPP"); // style
logChecker("Cert C++: CON55-CPP"); // style
logChecker("Cert C++: CON56-CPP");
logChecker("Cert C++: CTR50-CPP");
logChecker("Cert C++: CTR52-CPP");
logChecker("Cert C++: CTR53-CPP");
logChecker("Cert C++: CTR56-CPP"); // style
logChecker("Cert C++: CTR57-CPP");
logChecker("Cert C++: CTR58-CPP");
logChecker("Cert C: DCL31-C"); // style
logChecker("Cert C: DCL36-C"); // style
logChecker("Cert C: DCL37-C"); // style
logChecker("Cert C: DCL38-C"); // style
logChecker("Cert C: DCL39-C"); // style
logChecker("Cert C: DCL40-C"); // style
logChecker("Cert C: DCL41-C"); // style
logChecker("Cert C++: DCL50-CPP"); // style
logChecker("Cert C++: DCL51-CPP"); // style
logChecker("Cert C++: DCL52-CPP"); // style
logChecker("Cert C++: DCL53-CPP"); // style
logChecker("Cert C++: DCL54-CPP");
logChecker("Cert C++: DCL56-CPP");
logChecker("Cert C++: DCL58-CPP"); // style
logChecker("Cert C++: DCL59-CPP"); // style
logChecker("Cert C: ENV30-C"); // style
logChecker("Cert C: ENV31-C"); // style
logChecker("Cert C: ENV32-C"); // style
logChecker("Cert C: ENV33-C"); // style
logChecker("Cert C: ENV34-C"); // style
logChecker("Cert C: ERR30-C"); // warning
logChecker("Cert C: ERR32-C"); // warning
logChecker("Cert C: ERR33-C"); // warning
logChecker("Cert C++: ERR50-CPP");
logChecker("Cert C++: ERR51-CPP"); // style
logChecker("Cert C++: ERR52-CPP"); // style
logChecker("Cert C++: ERR53-CPP");
logChecker("Cert C++: ERR54-CPP");
logChecker("Cert C++: ERR55-CPP");
logChecker("Cert C++: ERR56-CPP");
logChecker("Cert C++: ERR58-CPP");
logChecker("Cert C++: ERR59-CPP"); // warning
logChecker("Cert C++: ERR60-CPP"); // warning
logChecker("Cert C++: ERR61-CPP"); // style
logChecker("Cert C++: ERR62-CPP"); // style
logChecker("Cert C: EXP32-C"); // warning
logChecker("Cert C: EXP35-C");
logChecker("Cert C: EXP36-C"); // style
logChecker("Cert C: EXP37-C"); // style
logChecker("Cert C: EXP39-C"); // style
logChecker("Cert C: EXP40-C"); // style
logChecker("Cert C: EXP42-C"); // style
logChecker("Cert C: EXP43-C"); // style
logChecker("Cert C: EXP45-C"); // warning
logChecker("Cert C++: EXP50-CPP");
logChecker("Cert C++: EXP51-CPP");
logChecker("Cert C++: EXP55-CPP");
logChecker("Cert C++: EXP56-CPP");
logChecker("Cert C++: EXP57-CPP"); // style
logChecker("Cert C++: EXP58-CPP"); // style
logChecker("Cert C++: EXP59-CPP");
logChecker("Cert C: FIO30-C"); // warning
logChecker("Cert C: FIO32-C"); // style
logChecker("Cert C: FIO34-C"); // style
logChecker("Cert C: FIO37-C");
logChecker("Cert C: FIO38-C"); // style
logChecker("Cert C: FIO40-C"); // style
logChecker("Cert C: FIO41-C"); // style
logChecker("Cert C: FIO44-C"); // warning
logChecker("Cert C: FIO45-C"); // warning
logChecker("Cert C++: FIO51-CPP"); // style
logChecker("Cert C: FLP30-C"); // warning
logChecker("Cert C: FLP36-C"); // portability
logChecker("Cert C: FLP37-C"); // style
logChecker("Cert C: INT30-C"); // warning
logChecker("Cert C: INT31-C"); // warning
logChecker("Cert C: INT32-C"); // warning
logChecker("Cert C: INT33-C"); // warning
logChecker("Cert C: INT34-C"); // warning
logChecker("Cert C: INT35-C"); // warning
logChecker("Cert C: INT36-C"); // warning
logChecker("Cert C++: INT50-CPP"); // style
logChecker("Cert C: MEM33-C"); // style
logChecker("Cert C: MEM35-C"); // warning
logChecker("Cert C: MEM36-C"); // warning
logChecker("Cert C++: MEM52-CPP");
logChecker("Cert C++: MEM53-CPP");
logChecker("Cert C++: MEM54-CPP");
logChecker("Cert C++: MEM55-CPP");
logChecker("Cert C++: MEM57-CPP"); // style
logChecker("Cert C: MSC30-C"); // style
logChecker("Cert C: MSC32-C"); // style
logChecker("Cert C: MSC33-C"); // style
logChecker("Cert C: MSC38-C"); // warning
logChecker("Cert C: MSC39-C"); // warning
logChecker("Cert C: MSC40-C"); // warning
logChecker("Cert C++: MSC50-CPP"); // style
logChecker("Cert C++: MSC51-CPP"); // style
logChecker("Cert C++: MSC53-CPP");
logChecker("Cert C++: MSC54-CPP"); // style
logChecker("Cert C++: OOP51-CPP");
logChecker("Cert C++: OOP55-CPP");
logChecker("Cert C++: OOP56-CPP");
logChecker("Cert C++: OOP57-CPP");
logChecker("Cert C++: OOP58-CPP"); // style
logChecker("Cert C: PRE31-C"); // style
logChecker("Cert C: SIG30-C"); // style
logChecker("Cert C: SIG31-C"); // warning
logChecker("Cert C: SIG34-C"); // style
logChecker("Cert C: SIG35-C"); // warning
logChecker("Cert C: STR31-C"); // warning
logChecker("Cert C: STR32-C"); // warning
logChecker("Cert C: STR34-C"); // warning
logChecker("Cert C: STR38-C"); // style
logChecker("Cert C++: STR50-CPP");
logChecker("Cert C++: STR53-CPP");
logChecker("Misra C: 10.1"); // style
logChecker("Misra C: 10.2"); // style
logChecker("Misra C: 10.3"); // style
logChecker("Misra C: 10.4"); // style
logChecker("Misra C: 10.5"); // style
logChecker("Misra C: 10.6"); // style
logChecker("Misra C: 10.7"); // style
logChecker("Misra C: 10.8"); // style
logChecker("Misra C: 11.10"); // style
logChecker("Misra C: 12.6");
logChecker("Misra C: 1.5"); // style
logChecker("Misra C: 17.10"); // style
logChecker("Misra C: 17.11"); // style
logChecker("Misra C: 17.12"); // style
logChecker("Misra C: 17.9"); // style
logChecker("Misra C: 18.10"); // style
logChecker("Misra C: 18.9"); // style
logChecker("Misra C: 21.12"); // style
logChecker("Misra C: 21.22"); // style
logChecker("Misra C: 21.23"); // style
logChecker("Misra C: 21.24"); // style
logChecker("Misra C: 21.25"); // warning
logChecker("Misra C: 21.26"); // warning
logChecker("Misra C: 22.11");
logChecker("Misra C: 22.12"); // style
logChecker("Misra C: 22.13"); // style
logChecker("Misra C: 22.14"); // style
logChecker("Misra C: 22.15"); // style
logChecker("Misra C: 22.16"); // warning
logChecker("Misra C: 22.17"); // warning
logChecker("Misra C: 22.18"); // warning
logChecker("Misra C: 22.19"); // warning
logChecker("Misra C: 22.20"); // style
logChecker("Misra C: 23.1"); // style
logChecker("Misra C: 23.2"); // style
logChecker("Misra C: 23.3"); // style
logChecker("Misra C: 23.4"); // style
logChecker("Misra C: 23.5"); // style
logChecker("Misra C: 23.6"); // style
logChecker("Misra C: 23.7"); // style
logChecker("Misra C: 23.8"); // style
logChecker("Misra C: 6.3"); // style
logChecker("Misra C: 7.5"); // style
logChecker("Misra C: 7.6"); // style
logChecker("Misra C: 8.10"); // style
logChecker("Misra C: 8.15"); // style
logChecker("Misra C: 8.16"); // style
logChecker("Misra C: 8.17"); // style
logChecker("Misra C: 9.6"); // style
logChecker("Misra C: 9.7");
logChecker("Misra C++: M0-1-11"); // style
logChecker("Misra C++: M0-1-12"); // style
logChecker("Misra C++: M0-1-4"); // style
logChecker("Misra C++: M0-1-5"); // style
logChecker("Misra C++: M0-1-7"); // style
logChecker("Misra C++: M0-1-8"); // style
logChecker("Misra C++: M10-1-1"); // style
logChecker("Misra C++: M10-1-2"); // style
logChecker("Misra C++: M10-1-3"); // style
logChecker("Misra C++: M10-2-1"); // style
logChecker("Misra C++: M10-3-3"); // style
logChecker("Misra C++: M11-0-1"); // style
logChecker("Misra C++: M12-8-2"); // style
logChecker("Misra C++: M14-5-1"); // warning
logChecker("Misra C++: M14-5-2"); // warning
logChecker("Misra C++: M14-5-3"); // warning
logChecker("Misra C++: M14-6-1"); // warning
logChecker("Misra C++: M14-7-1"); // style
logChecker("Misra C++: M14-7-2"); // style
logChecker("Misra C++: M15-0-3");
logChecker("Misra C++: M15-1-1");
logChecker("Misra C++: M15-1-2"); // style
logChecker("Misra C++: M15-1-3"); // style
logChecker("Misra C++: M15-3-2"); // warning
logChecker("Misra C++: M15-3-3");
logChecker("Misra C++: M15-4-1"); // style
logChecker("Misra C++: M16-0-1"); // style
logChecker("Misra C++: M16-0-2"); // style
logChecker("Misra C++: M16-0-3"); // style
logChecker("Misra C++: M16-0-4"); // style
logChecker("Misra C++: M16-1-1"); // style
logChecker("Misra C++: M16-2-1"); // style
logChecker("Misra C++: M16-2-2"); // style
logChecker("Misra C++: M16-2-3"); // style
logChecker("Misra C++: M16-2-4"); // style
logChecker("Misra C++: M16-2-5"); // style
logChecker("Misra C++: M16-2-6"); // style
logChecker("Misra C++: M16-3-1"); // style
logChecker("Misra C++: M16-3-2"); // style
logChecker("Misra C++: M17-0-1"); // style
logChecker("Misra C++: M17-0-2"); // style
logChecker("Misra C++: M17-0-3"); // style
logChecker("Misra C++: M17-0-5"); // style
logChecker("Misra C++: M18-0-1"); // style
logChecker("Misra C++: M18-0-2"); // style
logChecker("Misra C++: M18-0-3"); // style
logChecker("Misra C++: M18-0-4"); // style
logChecker("Misra C++: M18-0-5"); // style
logChecker("Misra C++: M18-2-1"); // style
logChecker("Misra C++: M18-4-1"); // style
logChecker("Misra C++: M18-7-1"); // style
logChecker("Misra C++: M19-3-1"); // style
logChecker("Misra C++: M2-10-1"); // style
logChecker("Misra C++: M2-10-3"); // style
logChecker("Misra C++: M2-10-4"); // style
logChecker("Misra C++: M2-10-5"); // style
logChecker("Misra C++: M2-10-6"); // style
logChecker("Misra C++: M2-13-4"); // style
logChecker("Misra C++: M2-13-5"); // style
logChecker("Misra C++: M27-0-1"); // style
logChecker("Misra C++: M2-7-1"); // style
logChecker("Misra C++: M2-7-2"); // style
logChecker("Misra C++: M2-7-3"); // style
logChecker("Misra C++: M3-1-1"); // style
logChecker("Misra C++: M3-1-2"); // style
logChecker("Misra C++: M3-1-3"); // style
logChecker("Misra C++: M3-2-1");
logChecker("Misra C++: M3-3-1"); // style
logChecker("Misra C++: M3-3-2"); // style
logChecker("Misra C++: M3-9-1"); // style
logChecker("Misra C++: M3-9-2"); // style
logChecker("Misra C++: M3-9-3"); // style
logChecker("Misra C++: M4-10-1"); // style
logChecker("Misra C++: M4-10-2"); // style
logChecker("Misra C++: M4-5-1"); // style
logChecker("Misra C++: M4-5-2"); // style
logChecker("Misra C++: M4-5-3"); // style
logChecker("Misra C++: M5-0-10"); // style
logChecker("Misra C++: M5-0-11"); // style
logChecker("Misra C++: M5-0-12"); // style
logChecker("Misra C++: M5-0-14"); // style
logChecker("Misra C++: M5-0-15"); // style
logChecker("Misra C++: M5-0-20"); // style
logChecker("Misra C++: M5-0-21"); // style
logChecker("Misra C++: M5-0-2"); // style
logChecker("Misra C++: M5-0-3"); // style
logChecker("Misra C++: M5-0-4"); // style
logChecker("Misra C++: M5-0-5"); // style
logChecker("Misra C++: M5-0-6"); // style
logChecker("Misra C++: M5-0-7"); // style
logChecker("Misra C++: M5-0-8"); // style
logChecker("Misra C++: M5-0-9"); // style
logChecker("Misra C++: M5-2-10"); // style
logChecker("Misra C++: M5-2-11"); // style
logChecker("Misra C++: M5-2-12"); // style
logChecker("Misra C++: M5-2-1"); // style
logChecker("Misra C++: M5-2-2"); // style
logChecker("Misra C++: M5-2-3"); // style
logChecker("Misra C++: M5-2-5"); // style
logChecker("Misra C++: M5-2-6"); // style
logChecker("Misra C++: M5-2-7"); // style
logChecker("Misra C++: M5-2-8"); // style
logChecker("Misra C++: M5-2-9"); // style
logChecker("Misra C++: M5-3-1"); // style
logChecker("Misra C++: M5-3-2"); // style
logChecker("Misra C++: M5-3-3"); // style
logChecker("Misra C++: M6-2-3"); // style
logChecker("Misra C++: M6-4-4"); // style
logChecker("Misra C++: M6-4-6"); // style
logChecker("Misra C++: M6-4-7"); // style
logChecker("Misra C++: M6-4-8"); // style
logChecker("Misra C++: M6-5-1"); // style
logChecker("Misra C++: M6-5-2"); // style
logChecker("Misra C++: M6-5-3"); // style
logChecker("Misra C++: M6-5-4"); // style
logChecker("Misra C++: M6-5-5"); // style
logChecker("Misra C++: M6-5-6"); // style
logChecker("Misra C++: M6-6-1"); // style
logChecker("Misra C++: M6-6-3"); // style
logChecker("Misra C++: M6-6-4"); // style
logChecker("Misra C++: M6-6-5"); // style
logChecker("Misra C++: M7-2-1"); // style
logChecker("Misra C++: M7-3-1"); // style
logChecker("Misra C++: M7-3-2"); // style
logChecker("Misra C++: M7-3-3"); // style
logChecker("Misra C++: M7-3-4"); // style
logChecker("Misra C++: M7-3-5"); // style
logChecker("Misra C++: M7-3-6"); // style
logChecker("Misra C++: M7-4-2"); // style
logChecker("Misra C++: M7-4-3"); // style
logChecker("Misra C++: M7-5-3"); // style
logChecker("Misra C++: M8-0-1"); // style
logChecker("Misra C++: M8-3-1"); // style
logChecker("Misra C++: M8-4-4"); // style
logChecker("Misra C++: M9-3-1"); // style
logChecker("Misra C++: M9-5-1"); // style
logChecker("Misra C++: M9-6-2"); // style
logChecker("Misra C++: M9-6-3"); // style
logChecker("Misra C++: M9-6-4"); // style
logChecker("PremiumCheckBufferOverrun::addressOfPointerArithmetic"); // warning
logChecker("PremiumCheckBufferOverrun::negativeBufferSizeCheckedNonZero"); // warning
logChecker("PremiumCheckBufferOverrun::negativeBufferSizeCheckedNonZero"); // warning
logChecker("PremiumCheckHang::infiniteLoop");
logChecker("PremiumCheckHang::infiniteLoopContinue");
logChecker("PremiumCheckOther::arrayPointerComparison"); // style
logChecker("PremiumCheckOther::knownResult"); // style
logChecker("PremiumCheckOther::lossOfPrecision"); // style
logChecker("PremiumCheckOther::pointerCast"); // style
logChecker("PremiumCheckOther::reassignInLoop"); // style
logChecker("PremiumCheckOther::unreachableCode"); // style
logChecker("PremiumCheckStrictAlias::strictAliasCondition"); // warning
logChecker("PremiumCheckUninitVar::uninitmember");
logChecker("PremiumCheckUninitVar::uninitvar");
logChecker("PremiumCheckUnusedVar::unreadVariable"); // style
logChecker("PremiumCheckUnusedVar::unusedPrivateMember"); // style
"""

for line in premium_checkers.split('\n'):
    res = re.match(r'logChecker\("([^"]+)"\);.*', line)
    if res is None:
        continue
    if line.find('//') > 0:
        req = line[line.find('//')+2:].strip()
    else:
        req = ''
    print('    {"%s","%s"},' % (res.group(1), req))

print('};\n\n')

print("""
struct MisraInfo {
    int a;
    int b;
    const char* str;
    int amendment;
};

const char Req[] = "Required";
const char Adv[] = "Advisory";
const char Man[] = "Mandatory";

const MisraInfo misraC2012Rules[] =
{
    {1,1,Req,0},
    {1,2,Adv,0},
    {1,3,Req,0},
    {1,4,Req,2}, // amendment 2
    {1,5,Req,3}, // Amendment 3
    {2,1,Req,0},
    {2,2,Req,0},
    {2,3,Adv,0},
    {2,4,Adv,0},
    {2,5,Adv,0},
    {2,6,Adv,0},
    {2,7,Adv,0},
    {2,8,Adv,0},
    {3,1,Req,0},
    {3,2,Req,0},
    {4,1,Req,0},
    {4,2,Adv,0},
    {5,1,Req,0},
    {5,2,Req,0},
    {5,3,Req,0},
    {5,4,Req,0},
    {5,5,Req,0},
    {5,6,Req,0},
    {5,7,Req,0},
    {5,8,Req,0},
    {5,9,Adv,0},
    {6,1,Req,0},
    {6,2,Req,0},
    {6,3,Req,0},
    {7,1,Req,0},
    {7,2,Req,0},
    {7,3,Req,0},
    {7,4,Req,0},
    {7,5,Man,0},
    {7,6,Req,0},
    {8,1,Req,0},
    {8,2,Req,0},
    {8,3,Req,0},
    {8,4,Req,0},
    {8,5,Req,0},
    {8,6,Req,0},
    {8,7,Adv,0},
    {8,8,Req,0},
    {8,9,Adv,0},
    {8,10,Req,0},
    {8,11,Adv,0},
    {8,12,Req,0},
    {8,13,Adv,0},
    {8,14,Req,0},
    {8,15,Req,0},
    {8,16,Adv,0},
    {8,17,Adv,0},
    {9,1,Man,0},
    {9,2,Req,0},
    {9,3,Req,0},
    {9,4,Req,0},
    {9,5,Req,0},
    {9,6,Req,0},
    {9,7,Man,0},
    {10,1,Req,0},
    {10,2,Req,0},
    {10,3,Req,0},
    {10,4,Req,0},
    {10,5,Adv,0},
    {10,6,Req,0},
    {10,7,Req,0},
    {10,8,Req,0},
    {11,1,Req,0},
    {11,2,Req,0},
    {11,3,Req,0},
    {11,4,Adv,0},
    {11,5,Adv,0},
    {11,6,Req,0},
    {11,7,Req,0},
    {11,8,Req,0},
    {11,9,Req,0},
    {11,10,Req,0},
    {12,1,Adv,0},
    {12,2,Req,0},
    {12,3,Adv,0},
    {12,4,Adv,0},
    {12,5,Man,1}, // amendment 1
    {12,6,Req,4}, // amendment 4
    {13,1,Req,0},
    {13,2,Req,0},
    {13,3,Adv,0},
    {13,4,Adv,0},
    {13,5,Req,0},
    {13,6,Man,0},
    {14,1,Req,0},
    {14,2,Req,0},
    {14,3,Req,0},
    {14,4,Req,0},
    {15,1,Adv,0},
    {15,2,Req,0},
    {15,3,Req,0},
    {15,4,Adv,0},
    {15,5,Adv,0},
    {15,6,Req,0},
    {15,7,Req,0},
    {16,1,Req,0},
    {16,2,Req,0},
    {16,3,Req,0},
    {16,4,Req,0},
    {16,5,Req,0},
    {16,6,Req,0},
    {16,7,Req,0},
    {17,1,Req,0},
    {17,2,Req,0},
    {17,3,Man,0},
    {17,4,Man,0},
    {17,5,Adv,0},
    {17,6,Man,0},
    {17,7,Req,0},
    {17,8,Adv,0},
    {17,9,Man,0},
    {17,10,Req,0},
    {17,11,Adv,0},
    {17,12,Adv,0},
    {17,13,Req,0},
    {18,1,Req,0},
    {18,2,Req,0},
    {18,3,Req,0},
    {18,4,Adv,0},
    {18,5,Adv,0},
    {18,6,Req,0},
    {18,7,Req,0},
    {18,8,Req,0},
    {18,9,Req,0},
    {18,10,Man,0},
    {19,1,Man,0},
    {19,2,Adv,0},
    {20,1,Adv,0},
    {20,2,Req,0},
    {20,3,Req,0},
    {20,4,Req,0},
    {20,5,Adv,0},
    {20,6,Req,0},
    {20,7,Req,0},
    {20,8,Req,0},
    {20,9,Req,0},
    {20,10,Adv,0},
    {20,11,Req,0},
    {20,12,Req,0},
    {20,13,Req,0},
    {20,14,Req,0},
    {21,1,Req,0},
    {21,2,Req,0},
    {21,3,Req,0},
    {21,4,Req,0},
    {21,5,Req,0},
    {21,6,Req,0},
    {21,7,Req,0},
    {21,8,Req,0},
    {21,9,Req,0},
    {21,10,Req,0},
    {21,11,Req,0},
    {21,12,Adv,0},
    {21,13,Man,1}, // Amendment 1
    {21,14,Req,1}, // Amendment 1
    {21,15,Req,1}, // Amendment 1
    {21,16,Req,1}, // Amendment 1
    {21,17,Req,1}, // Amendment 1
    {21,18,Man,1}, // Amendment 1
    {21,19,Man,1}, // Amendment 1
    {21,20,Man,1}, // Amendment 1
    {21,21,Req,3}, // Amendment 3
    {21,22,Man,3}, // Amendment 3
    {21,23,Req,3}, // Amendment 3
    {21,24,Req,3}, // Amendment 3
    {21,25,Req,4}, // Amendment 4
    {21,26,Req,4}, // Amendment 4
    {22,1,Req,0},
    {22,2,Man,0},
    {22,3,Req,0},
    {22,4,Man,0},
    {22,5,Man,0},
    {22,6,Man,0},
    {22,7,Req,1}, // Amendment 1
    {22,8,Req,1}, // Amendment 1
    {22,9,Req,1}, // Amendment 1
    {22,10,Req,1}, // Amendment 1
    {22,11,Req,4}, // Amendment 4
    {22,12,Man,4}, // Amendment 4
    {22,13,Req,4}, // Amendment 4
    {22,14,Man,4}, // Amendment 4
    {22,15,Req,4}, // Amendment 4
    {22,16,Req,4}, // Amendment 4
    {22,17,Req,4}, // Amendment 4
    {22,18,Req,4}, // Amendment 4
    {22,19,Req,4}, // Amendment 4
    {22,20,Man,4}, // Amendment 4
    {23,1,Adv,3}, // Amendment 3
    {23,2,Req,3}, // Amendment 3
    {23,3,Adv,3}, // Amendment 3
    {23,4,Req,3}, // Amendment 3
    {23,5,Adv,3}, // Amendment 3
    {23,6,Req,3}, // Amendment 3
    {23,7,Adv,3}, // Amendment 3
    {23,8,Req,3}, // Amendment 3
};

static std::map<std::string, std::string> misraRuleSeverity{
    {"1.1", "error"}, //{"syntaxError", "unknownMacro"}},
    {"1.3", "error"}, //most "error"
    {"2.1", "style"}, //{"alwaysFalse", "duplicateBreak"}},
    {"2.2", "style"}, //{"alwaysTrue", "redundantCondition", "redundantAssignment", "redundantAssignInSwitch", "unreadVariable"}},
    {"2.6", "style"}, //{"unusedLabel"}},
    {"2.8", "style"}, //{"unusedVariable"}},
    {"5.3", "style"}, //{"shadowVariable"}},
    {"8.3", "style"}, //{"funcArgNamesDifferent"}}, // inconclusive
    {"8.13", "style"}, //{"constPointer"}},
    {"9.1", "error"}, //{"uninitvar"}},
    {"14.3", "style"}, //{"alwaysTrue", "alwaysFalse", "compareValueOutOfTypeRangeError", "knownConditionTrueFalse"}},
    {"13.2", "error"}, //{"unknownEvaluationOrder"}},
    {"13.6", "style"}, //{"sizeofCalculation"}},
    {"17.4", "error"}, //{"missingReturn"}},
    {"17.5", "warning"}, //{"argumentSize"}},
    {"18.1", "error"}, //{"pointerOutOfBounds"}},
    {"18.2", "error"}, //{"comparePointers"}},
    {"18.3", "error"}, //{"comparePointers"}},
    {"18.6", "error"}, //{"danglingLifetime"}},
    {"19.1", "error"}, //{"overlappingWriteUnion", "overlappingWriteFunction"}},
    {"20.6", "error"}, //{"preprocessorErrorDirective"}},
    {"21.13", "error"}, //{"invalidFunctionArg"}},
    {"21.17", "error"}, //{"bufferAccessOutOfBounds"}},
    {"21.18", "error"}, //{"bufferAccessOutOfBounds"}},
    {"22.1", "error"}, //{"memleak", "resourceLeak", "memleakOnRealloc", "leakReturnValNotUsed", "leakNoVarFunctionCall"}},
    {"22.2", "error"}, //{"autovarInvalidDeallocation"}},
    {"22.3", "error"}, //{"incompatibleFileOpen"}},
    {"22.4", "error"}, //{"writeReadOnlyFile"}},
    {"22.6", "error"}, //{"useClosedFile"}}
};

}

""")


