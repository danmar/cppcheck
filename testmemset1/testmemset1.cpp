
class clKalle
{
private:
    int i;
public:
    clKalle();
};

struct S1
{
    char *str;
}

struct S2
{
    std::string str;
}

clKalle::clKalle()
{
    i = 0;
}

void f()
{
    clKalle *kalle = new clKalle;
    memset(kalle, 0, sizeof(clKalle));

    S1 s1;
    memset(&s1, 0, sizeof(S1));

    S2 s2;
    memset(&s2, 0, sizeof(S2));
}

