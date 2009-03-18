/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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

#ifndef checkH
#define checkH

#include "settings.h"

class Tokenizer;
class ErrorLogger;

class Check
{
public:

    Check(const Tokenizer * const tokenizer, const Settings &settings, ErrorLogger *errorLogger)
            : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger)
    { }

    virtual ~Check()
    { }

    /** run checks.. */
    virtual void runChecks() = 0;

protected:
    const Tokenizer * const _tokenizer;
    const Settings &_settings;
    ErrorLogger *_errorLogger;
};

#endif

