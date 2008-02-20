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

// Dangerous usage of 'strtok'
void WarningStrTok();

// Check for a 'case' without a 'break'
void CheckCaseWithoutBreak();

// Check for unsigned division that might create bad results
void CheckUnsignedDivision();

//---------------------------------------------------------------------------
#endif

