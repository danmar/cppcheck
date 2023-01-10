/*
This is testing data for the GUI.
Used for testing GUI with various error styles reported by cppcheck.
Not meant to be compiled.
*/

#include <vector>

void unused()
{
    int a = 15;
}

void f(char k)
{
    delete k;
}

void possible_style()
{
    std::list<int>::iterator it;
    for (it = ab.begin(); it != ab.end(); it++)
        ;
}

int main()
{
    char *b = new char[1];
    char *a = new char[8];
    if (a);
    b = gets();
    f(a);
    possible_style();
}
