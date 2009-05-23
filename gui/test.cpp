/*
This is testing data for the GUI.
Used for testing GUI with various error styles reported by cppcheck. 
Not meant to be compiled.
*/

void unused()
{
	int a = 15;
}

void f(char k)
{
    delete k;
}

int main()
{
    char *b = new char[1];
    char *a = new char[8];
    if (a);
    b = gets();
    f(a);

}


