

bool f()
{
    char *str = strdup("hello");
    if (a==b)
    {
        free(str);
        return false;
    }
    return true;
}
