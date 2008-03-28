//---------------------------------------------------------------------------
#ifndef CommonCheckH
#define CommonCheckH
//---------------------------------------------------------------------------

#include <list>
#include <string>
#include <sstream>

struct TOKEN;

std::string FileLine(const TOKEN *tok);

extern bool OnlyReportUniqueErrors;

void ReportErr(const std::string &errmsg);
extern std::ostringstream errout;


bool IsName(const char str[]);
bool IsNumber(const char str[]);

bool IsStandardType(const char str[]);

void FillFunctionList();
const TOKEN *GetFunctionTokenByName( const char funcname[] );


bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);


//---------------------------------------------------------------------------
#endif
