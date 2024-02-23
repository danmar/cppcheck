#ifndef GUARD_PROGRAMMEMORYFAST_H
#define GUARD_PROGRAMMEMORYFAST_H

#include "config.h"
#include "utils.h"
#include "valueflow.h"
#include "mathlib.h"
#include <map>

struct ProgramMemoryFast {
    std::map<int, ValueFlow::Value> values;

    void setValue(nonneg int varid, const ValueFlow::Value &value);

    bool getIntValue(nonneg int varid, MathLib::bigint* result) const;
    void setIntValue(nonneg int varid, MathLib::bigint value);

    bool getTokValue(nonneg int varid, const Token** result) const;
    bool hasValue(nonneg int varid);

    void swap(ProgramMemoryFast &pm);

    void clear();

    bool empty() const;

    void replace(const ProgramMemoryFast &pm);

    void insert(const ProgramMemoryFast &pm);
};

void execute(const Token *expr,
             ProgramMemoryFast * const programMemory,
             MathLib::bigint *result,
             bool *error);

/**
 * Is condition always false when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsFalse(const Token *condition, const ProgramMemoryFast &programMemory);

/**
 * Is condition always true when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
bool conditionIsTrue(const Token *condition, const ProgramMemoryFast &programMemory);

/**
 * Get program memory by looking backwards from given token.
 */
ProgramMemoryFast getProgramMemoryFast(const Token *tok, nonneg int varid, const ValueFlow::Value &value);

#endif



