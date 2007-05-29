
// Internal testing
// Tokenize a file and print tokens

#include <iostream>
#include "tokenize.h"

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
        return 0;

    tokens = tokens_back = NULL;
    Tokenize(argv[1]);
    SimplifyTokenList();

    unsigned int linenr = 0;
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Print either a "\n" or a " "
        if (tok->linenr != linenr)
            std::cout << "\n";
        else
            std::cout << " ";

        std::cout << tok->str;

        linenr = tok->linenr;
    }


    // Delete all tokens..
    while (tokens)
    {
        TOKEN *next = tokens->next;
        free(tokens->str);
        delete tokens;
        tokens = next;
    }

    return 0;
}
//---------------------------------------------------------------------------
 
