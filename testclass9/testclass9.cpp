
// A struct must be initialized whenever it's used.
// But a class is supposed to have a constructor that
// performs the initialization.

struct rec
{
    int a;
    int b;
};

class clRec
{
public:
    int a;
    int b;
    rec2();
};

class clKalle
{
private:
    rec rec1;
    clRec rec2;
    clRec *rec3;
public:
    clKalle();
};

clKalle::clKalle()
{

}