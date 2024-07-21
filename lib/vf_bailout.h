/*
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

#ifndef vfBailoutH
#define vfBailoutH

#include <string>

class TokenList;
class ErrorLogger;
class Token;

namespace ValueFlow
{
    void bailoutInternal(const std::string& type, const TokenList &tokenlist, ErrorLogger &errorLogger, const Token *tok, const std::string &what, const std::string &file, int line, std::string function);
}

#define bailout2(type, tokenlist, errorLogger, tok, what) ValueFlow::bailoutInternal((type), (tokenlist), (errorLogger), (tok), (what), __FILE__, __LINE__, __func__)

#define bailout(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailout", (tokenlist), (errorLogger), (tok), (what))

#define bailoutIncompleteVar(tokenlist, errorLogger, tok, what) ValueFlow::bailoutInternal("valueFlowBailoutIncompleteVar", (tokenlist), (errorLogger), (tok), (what), "", 0, __func__)

#endif // vfBailoutH
