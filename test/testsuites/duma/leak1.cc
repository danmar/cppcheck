#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;

  int* pI = new int;
  cerr << "Let's leak a pointer to int" << endl;
  *pI = 303;

  return 0;
}
