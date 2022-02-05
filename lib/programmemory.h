/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#ifndef GUARD_PROGRAMMEMORY_H
#define GUARD_PROGRAMMEMORY_H

#include "mathlib.h"
#include "utils.h"
#include "valueflow.h" // needed for alias
#include <map>
#include <unordered_map>

class Token;
class Settings;

struct ProgramMemory {
    using Map = std::unordered_map<nonneg int, ValueFlow::Value>;
    Map values;

    void setValue(nonneg int exprid, const ValueFlow::Value& value);
    const ValueFlow::Value* getValue(nonneg int exprid, bool impossible = false) const;

    bool getIntValue(nonneg int exprid, MathLib::bigint* result) const;
    void setIntValue(nonneg int exprid, MathLib::bigint value, bool impossible = false);

    bool getContainerSizeValue(nonneg int exprid, MathLib::bigint* result) const;
    bool getContainerEmptyValue(nonneg int exprid, MathLib::bigint* result) const;
    void setContainerSizeValue(nonneg int exprid, MathLib::bigint value, bool isEqual = true);

    void setUnknown(nonneg int exprid);

    bool getTokValue(nonneg int exprid, const Token** result) const;
    bool hasValue(nonneg int exprid);

    void swap(ProgramMemory &pm);

    void clear();

    bool empty() const;

    void replace(const ProgramMemory &pm);

    void insert(const ProgramMemory &pm);
};

void programMemoryParseCondition(ProgramMemory& pm, const Token* tok, const Token* endTok, const Settings* settings, bool then);

struct ProgramMemoryState {
    ProgramMemory state;
    std::map<nonneg int, const Token*> origins;
    const Settings* settings;

    explicit ProgramMemoryState(const Settings* s);

    void insert(const ProgramMemory &pm, const Token* origin = nullptr);
    void replace(const ProgramMemory &pm, const Token* origin = nullptr);

    void addState(const Token* tok, const ProgramMemory::Map& vars);

    void assume(const Token* tok, bool b, bool isEmpty = false);

    void removeModifiedVars(const Token* tok);

    ProgramMemory get(const Token* tok, const Token* ctx, const ProgramMemory::Map& vars) const;
};

void execute(const Token* expr,
             ProgramMemory* const programMemory,
             MathLib::bigint* result,
             bool* error,
             const Settings* settings = nullptr);

/**
 * Is condition always false when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsFalse(const Token* condition, ProgramMemory pm, const Settings* settings = nullptr);

/**
 * Is condition always true when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsTrue(const Token* condition, ProgramMemory pm, const Settings* settings = nullptr);

/**
 * Get program memory by looking backwards from given token.
 */
ProgramMemory getProgramMemory(const Token* tok, nonneg int exprid, const ValueFlow::Value& value, const Settings *settings);

ProgramMemory getProgramMemory(const Token *tok, const ProgramMemory::Map& vars);

#endif



