//---------------------------------------------------------------------------
#include "CommonCheck.h"
#include "tokenize.h"
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------

std::string FileLine(TOKEN *tok)
{
    std::ostringstream ostr;
    ostr << "[" << Files[tok->FileIndex] << ":" << tok->linenr << "]";
    return ostr.str();
}
//---------------------------------------------------------------------------

void ReportErr(const std::string errmsg)
{
    std::cerr << errmsg << std::endl;
}
//---------------------------------------------------------------------------

bool IsName(const char str[])
{
    return (str[0]=='_' || std::isalpha(str[0]));
}
//---------------------------------------------------------------------------

bool IsNumber(const char str[])
{
    return std::isdigit(str[0]);
}
//---------------------------------------------------------------------------


