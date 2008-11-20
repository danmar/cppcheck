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


#include <string>
#include <sstream>
#include <vector>
#include "tokenize.h"


// Are two filenames the same? Case insensitive on windows
bool SameFileName( const char fname1[], const char fname2[] );
bool IsName(const char str[]);
bool IsNumber(const char str[]);
bool IsStandardType(const char str[]);
bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);

//---------------------------------------------------------------------------
#endif
