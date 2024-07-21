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

#include "vf_bailout.h"

#include "errorlogger.h"
#include "errortypes.h"
#include "path.h"
#include "tokenlist.h"

#include <utility>

namespace ValueFlow
{
    void bailoutInternal(const std::string& type, const TokenList &tokenlist, ErrorLogger &errorLogger, const Token *tok, const std::string &what, const std::string &file, int line, std::string function)
    {
        if (function.find("operator") != std::string::npos)
            function = "(valueFlow)";
        ErrorMessage::FileLocation loc(tok, &tokenlist);
        const std::string location = Path::stripDirectoryPart(file) + ":" + std::to_string(line) + ":";
        ErrorMessage errmsg({std::move(loc)}, tokenlist.getSourceFilePath(), Severity::debug,
                            (file.empty() ? "" : location) + function + " bailout: " + what, type, Certainty::normal);
        errorLogger.reportErr(errmsg);
    }
}
