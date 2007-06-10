
class clKalle
{
private:
    char *str;
    char *str2;
public:
    clKalle();
    ~clKalle();
};

clKalle::clKalle()
{
    str = new char[10];
    str2 = new char[10];
}

clKalle::~clKalle()
{
    delete [] str2;
}


