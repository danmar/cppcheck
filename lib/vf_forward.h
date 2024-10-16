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

#ifndef vfForwardH
#define vfForwardH

#include "analyzer.h"
#include "sourcelocation.h"

#include <list>

class Token;
class TokenList;
class ErrorLogger;
class Settings;
namespace ValueFlow {
    class Value;
}

namespace ValueFlow
{
    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* endToken,
                                      const Token* exprTok,
                                      ValueFlow::Value value,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc = SourceLocation::current());

    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* endToken,
                                      const Token* exprTok,
                                      std::list<ValueFlow::Value> values,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc = SourceLocation::current());

    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* exprTok,
                                      ValueFlow::Value v,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc = SourceLocation::current());

    Analyzer::Result valueFlowForwardRecursive(Token* top,
                                               const Token* exprTok,
                                               std::list<ValueFlow::Value> values,
                                               const TokenList& tokenlist,
                                               ErrorLogger& errorLogger,
                                               const Settings& settings,
                                               SourceLocation loc = SourceLocation::current());
}

#endif // vfForwardH
