
// Initializing class variables through a 'Clear' function

class clKalle
{
private:
    int i;
public:
    clKalle();
    void Clear();
};

clKalle::clKalle()
{
    Clear();
}

void clKalle::Clear()
{
    i = 123;
}