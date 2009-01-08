#ifndef errormessageH
#define errormessageH
#include <string>
#include "settings.h"
class Token;
class Tokenizer;
class ErrorMessage
{
public:
    static std::string msg1(const Tokenizer *tokenizer, const Token *Location);
    static std::string memleak(const Tokenizer *tokenizer, const Token *Location, const std::string &varname)
    { return msg1(tokenizer, Location) + "Memory leak: " + varname + ""; }

    static bool memleak(const Settings &s)
    { return true; }
    static std::string resourceLeak(const Tokenizer *tokenizer, const Token *Location, const std::string &varname)
    { return msg1(tokenizer, Location) + "Resource leak: " + varname + ""; }

    static bool resourceLeak(const Settings &s)
    { return true; }
};
#endif
