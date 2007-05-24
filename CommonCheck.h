//---------------------------------------------------------------------------
#ifndef CommonCheckH
#define CommonCheckH
//---------------------------------------------------------------------------

#include <string>

struct TOKEN;

std::string FileLine(TOKEN *tok);

void ReportErr(const std::string errmsg);

//---------------------------------------------------------------------------
#endif
