

void validCode(int argInt)
{
    // if G_UNLIKELY is not defined this results in a syntax error
    if G_UNLIKELY(argInt == 1) {} else if (G_UNLIKELY(argInt == 2)) {}
}

