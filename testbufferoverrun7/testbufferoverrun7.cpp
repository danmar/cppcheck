

void f1(char *str)
{
    strcpy(buf,str);
}

void f2(char *str)
{
    strcat(buf,str);
}

void f3(char *str)
{
    sprintf(buf,"%s",str);
}

void f4(const char str[])
{
    strcpy(buf, str);
}
