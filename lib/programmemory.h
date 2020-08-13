#ifndef GUARD_PROGRAMMEMORY_H
#define GUARD_PROGRAMMEMORY_H

#include "config.h"
#include "utils.h"
#include "valueflow.h" // needed for alias
#include "mathlib.h"
#include <map>
#include <unordered_map>

class Token;

struct ProgramMemory {
    using Map = std::unordered_map<nonneg int, ValueFlow::Value>;
    Map values;

    void setValue(nonneg int varid, const ValueFlow::Value &value);

    bool getIntValue(nonneg int varid, MathLib::bigint* result) const;
    void setIntValue(nonneg int varid, MathLib::bigint value);

    bool getContainerSizeValue(nonneg int varid, MathLib::bigint* result) const;

    void setUnknown(nonneg int varid);

    bool getTokValue(nonneg int varid, const Token** result) const;
    bool hasValue(nonneg int varid);

    void swap(ProgramMemory &pm);

    void clear();

    bool empty() const;

    void replace(const ProgramMemory &pm);

    void insert(const ProgramMemory &pm);
};

struct ProgramMemoryState {
    ProgramMemory state;
    std::map<nonneg int, const Token*> origins;

    void insert(const ProgramMemory &pm, const Token* origin = nullptr);
    void replace(const ProgramMemory &pm, const Token* origin = nullptr);

    void addState(const Token* tok, const ProgramMemory::Map& vars);

    void assume(const Token* tok, bool b);

    void removeModifiedVars(const Token* tok);

    ProgramMemory get(const Token *tok, const ProgramMemory::Map& vars) const;

};

void execute(const Token *expr,
             ProgramMemory * const programMemory,
             MathLib::bigint *result,
             bool *error);

/**
 * Is condition always false when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsFalse(const Token *condition, const ProgramMemory &programMemory);

/**
 * Is condition always true when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsTrue(const Token *condition, const ProgramMemory &programMemory);

/**
 * Get program memory by looking backwards from given token.
 */
ProgramMemory getProgramMemory(const Token *tok, nonneg int varid, const ValueFlow::Value &value);

ProgramMemory getProgramMemory(const Token *tok, const ProgramMemory::Map& vars);

#endif



