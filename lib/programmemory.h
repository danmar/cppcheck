#ifndef GUARD_PROGRAMMEMORY_H
#define GUARD_PROGRAMMEMORY_H

#include "mathlib.h"
#include "settings.h"
#include "utils.h"
#include "valueflow.h" // needed for alias
#include <functional>
#include <map>
#include <unordered_map>

class Token;

struct ProgramMemory {
    using Map = std::unordered_map<nonneg int, ValueFlow::Value>;
    Map values;

    void setValue(nonneg int exprid, const ValueFlow::Value& value);
    const ValueFlow::Value* getValue(nonneg int exprid, bool impossible = false) const;

    bool getIntValue(nonneg int exprid, MathLib::bigint* result) const;
    void setIntValue(nonneg int exprid, MathLib::bigint value);

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

using PMEvaluateFunction =
    std::function<bool (const Token* expr, ProgramMemory* const programMemory, MathLib::bigint* result)>;

void execute(const Token* expr,
             ProgramMemory* const programMemory,
             MathLib::bigint* result,
             bool* error,
             const PMEvaluateFunction& f = nullptr);

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
ProgramMemory getProgramMemory(const Token* tok, nonneg int exprid, const ValueFlow::Value& value, const Settings *settings);

ProgramMemory getProgramMemory(const Token *tok, const ProgramMemory::Map& vars);

#endif



