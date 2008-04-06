//---------------------------------------------------------------------------
#ifndef CommonCheckH
#define CommonCheckH
//---------------------------------------------------------------------------

#include <list>
#include <string>
#include <sstream>
#include <vector>

struct TOKEN;

std::string FileLine(const TOKEN *tok);

// Are two filenames the same? Case insensitive on windows
bool SameFileName( const char fname1[], const char fname2[] );

extern bool OnlyReportUniqueErrors;

void ReportErr(const std::string &errmsg);
extern std::ostringstream errout;


bool IsName(const char str[]);
bool IsNumber(const char str[]);

bool IsStandardType(const char str[]);

void FillFunctionList(const unsigned int file_id);
const TOKEN *GetFunctionTokenByName( const char funcname[] );
void CheckGlobalFunctionUsage(const std::vector<std::string> &filenames);

bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);


//---------------------------------------------------------------------------
#endif
