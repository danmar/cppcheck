//---------------------------------------------------------------------------
#ifndef CommonCheckH
#define CommonCheckH
//---------------------------------------------------------------------------

#include <string>
#include <sstream>

//---------------------------------------------------------------------------
// Report errors..
//---------------------------------------------------------------------------

struct TOKEN;

std::string FileLine(const TOKEN *tok);

extern bool OnlyReportUniqueErrors;

void ReportErr(const std::string &errmsg);
extern std::ostringstream errout;


//---------------------------------------------------------------------------
// Classify tokens..
//---------------------------------------------------------------------------

bool IsName(const char str[]);
bool IsNumber(const char str[]);

bool IsStandardType(const char str[]);


//---------------------------------------------------------------------------
// Iterating through tokens..
//---------------------------------------------------------------------------

bool setindentlevel( const TOKEN *tok, int &indentlevel );


//---------------------------------------------------------------------------
#endif
