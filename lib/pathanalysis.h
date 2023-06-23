/*
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

#ifndef GUARD_PATHANALYSIS_H
#define GUARD_PATHANALYSIS_H

#include "errortypes.h"

#include <functional>
#include <list>
#include <utility>

class Library;
class Scope;
class Token;

struct PathAnalysis {
    enum class Progress {
        Continue,
        Break
    };
    PathAnalysis(const Token* start, const Library& library)
        : start(start), library(&library)
    {}
    const Token * start;
    const Library * library;

    struct Info {
        const Token* tok;
        ErrorPath errorPath;
        bool known;
    };

    void forward(const std::function<Progress(const Info&)>& f) const;

    Info forwardFind(std::function<bool(const Info&)> pred) const {
        Info result{};
        forward([&](const Info& info) {
            if (pred(info)) {
                result = info;
                return Progress::Break;
            }
            return Progress::Continue;
        });
        return result;
    }
private:

    static Progress forwardRecursive(const Token* tok, Info info, const std::function<PathAnalysis::Progress(const Info&)>& f);
    Progress forwardRange(const Token* startToken, const Token* endToken, Info info, const std::function<Progress(const Info&)>& f) const;

    static const Scope* findOuterScope(const Scope * scope);

    static std::pair<bool, bool> checkCond(const Token * tok, bool& known);
};

/**
 * @brief Returns true if there is a path between the two tokens
 *
 * @param start Starting point of the path
 * @param dest The path destination
 * @param errorPath Adds the path traversal to the errorPath
 */
bool reaches(const Token * start, const Token * dest, const Library& library, ErrorPath* errorPath);

#endif

