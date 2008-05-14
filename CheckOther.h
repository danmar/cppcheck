//---------------------------------------------------------------------------
#ifndef CheckOtherH
#define CheckOtherH
//---------------------------------------------------------------------------


// Casting
void WarningOldStylePointerCast();

// Use standard functions instead
void WarningIsDigit();

// Use standard functions instead
void WarningIsAlpha();

// Redundant code
void WarningRedundantCode();

// Warning upon: if (condition);
void WarningIf();

// Assignment in condition
void CheckIfAssignment();

// Using dangerous functions
void WarningDangerousFunctions();

// Invalid function usage..
void InvalidFunctionUsage();

// Check for unsigned division that might create bad results
void CheckUnsignedDivision();

// Check scope of variables
void CheckVariableScope();

// Check for constant function parameter
void CheckConstantFunctionParameter();

// Check that all struct members are used
void CheckStructMemberUsage();

//---------------------------------------------------------------------------
#endif

