//---------------------------------------------------------------------------
#ifndef tokenizeH
#define tokenizeH
//---------------------------------------------------------------------------

#include <string>
#include <vector>

extern std::vector<std::string> Files;

struct TOKEN
{
    unsigned int FileIndex;
    char *str;
    unsigned int linenr;
    struct TOKEN *next;
};
extern struct TOKEN *tokens, *tokens_back;


void Tokenize(const char FileName[]);

//---------------------------------------------------------------------------
#endif

