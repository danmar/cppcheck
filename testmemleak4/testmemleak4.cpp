
bool f()
{
    TStringList *StringList = new TStringList;

    if (asd)
    {
        delete StringList;
        return false;
    }

    delete StringList;
    return true;
}

