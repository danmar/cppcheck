/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



//---------------------------------------------------------------------------
#ifndef CommonCheckH
#define CommonCheckH
//---------------------------------------------------------------------------

#include <list>
#include <string>
#include <sstream>
#include <vector>

class TOKEN;

std::string FileLine(const TOKEN *tok);

// Are two filenames the same? Case insensitive on windows
bool SameFileName( const char fname1[], const char fname2[] );

extern bool OnlyReportUniqueErrors;

void ReportErr(const std::string &errmsg);
extern std::ostringstream errout;


bool IsName(const char str[]);
bool IsNumber(const char str[]);

bool IsStandardType(const char str[]);

void FillFunctionList(const unsigned int file_id);
const TOKEN *GetFunctionTokenByName( const char funcname[] );
const char *GetParameterName( const TOKEN *ftok, int par );
void CheckGlobalFunctionUsage(const std::vector<std::string> &filenames);

bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);

const TOKEN *findmatch(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);

void deleteTokens(TOKEN *tok);
//---------------------------------------------------------------------------
#endif
