#include "errormessage.h"
#include "tokenize.h"
#include "token.h"

std::string ErrorMessage::msg1(const Tokenizer *tokenizer, const Token *Location)
{
    return tokenizer->fileLine(Location) + ": ";
}


