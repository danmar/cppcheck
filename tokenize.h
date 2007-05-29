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

// Simplify tokenlist
// -----------------------------
void SimplifyTokenList();


// Helper functions for handling the tokens list..
TOKEN *findtoken(TOKEN *tok1, const char *tokenstr[]);
bool match(TOKEN *tok, const std::string pattern);
TOKEN *gettok(TOKEN *tok, int index);
const char *getstr(TOKEN *tok, int index);

//---------------------------------------------------------------------------
#endif

