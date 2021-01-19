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
    using Map = std::unordered_map<MathLib::bigint, ValueFlow::Value>;
    Map values;

    void setValue(MathLib::bigint exprid, const ValueFlow::Value &value);
    const ValueFlow::Value* getValue(MathLib::bigint exprid) const;

    bool getIntValue(MathLib::bigint exprid, MathLib::bigint* result) const;
    void setIntValue(MathLib::bigint exprid, MathLib::bigint value);

    bool getContainerSizeValue(MathLib::bigint exprid, MathLib::bigint* result) const;

    void setUnknown(MathLib::bigint exprid);

    bool getTokValue(MathLib::bigint exprid, const Token** result) const;
    bool hasValue(MathLib::bigint exprid);

    void swap(ProgramMemory &pm);

    void clear();

    bool empty() const;

    void replace(const ProgramMemory &pm);

    void insert(const ProgramMemory &pm);
};

void programMemoryParseCondition(ProgramMemory& pm, const Token* tok, const Token* endTok, const Settings* settings, bool then);

struct ProgramMemoryState {
    ProgramMemory state;
    std::map<MathLib::bigint, const Token*> origins;

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
ProgramMemory getProgramMemory(const Token *tok, MathLib::bigint exprid, const ValueFlow::Value &value);

ProgramMemory getProgramMemory(const Token *tok, const ProgramMemory::Map& vars);

#endif



