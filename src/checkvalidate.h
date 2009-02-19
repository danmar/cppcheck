/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
#ifndef checkvalidateH
#define checkvalidateH
//---------------------------------------------------------------------------

class ErrorLogger;
class Token;
class Tokenizer;

class CheckValidate
{
public:
    CheckValidate(const Tokenizer *tokenizer, ErrorLogger *errorLogger);
    ~CheckValidate();

    /** Reading a number from a stream/FILE */
    void readnum();

private:
    const Tokenizer *_tokenizer;
    ErrorLogger *_errorLogger;

    /**
     * Helper function used by the 'erase' function
     * This function parses a loop
     * @param it iterator token
     */
    void eraseCheckLoop(const Token *it);
};

//---------------------------------------------------------------------------
#endif

