#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;

  int* pI = new int[10];
  cerr << "Let's leak a pointer to an array of 10 ints" << endl;
  for (int i=0; i<9; i++) {
    pI[i] = 303+i;
  }
  for (int i=0; i<9; i++) {
    if (pI[i] != 303+i) cerr << "  Something strange is happening..." << endl;
  }

  return 0;
}
