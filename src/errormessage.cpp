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

#include "errormessage.h"
#include "errorlogger.h"
#include "tokenize.h"
#include "token.h"

#include <sstream>

void ErrorMessage::_writemsg(ErrorLogger *logger, const Tokenizer *tokenizer, const Token *tok, const char severity[], const std::string msg)
{
    logger->reportErr(tokenizer->fileLine(tok) + ": (" + severity + ") " + msg);
}

void ErrorMessage::_writemsg(ErrorLogger *logger, const Tokenizer *tokenizer, const std::list<const Token *> &callstack, const char severity[], const std::string msg)
{
    std::ostringstream ostr;
    for (std::list<const Token *>::const_iterator tok = callstack.begin(); tok != callstack.end(); ++tok)
        ostr << (tok == callstack.begin() ? "" : " -> ") << tokenizer->fileLine(*tok);
    logger->reportErr(ostr.str() + ": (" + severity + ") " + msg);
}


void ErrorMessage::_writemsg(ErrorLogger *logger, const std::string msg)
{
    logger->reportErr(msg);
}
