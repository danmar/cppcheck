#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;

  {
  int* pI = new int[10];
  cerr << "Let's delete instead of delete [] " << endl;
  delete pI;
  cerr << "Did you notice?" << endl;
  }

  {
  int* pI = new int[10];
  cerr << "Now let's free instead of delete [] " << endl;
  free(pI);
  cerr << "Did you notice?" << endl;
  }

  cerr << "There should be 2 errors in this run" << endl;
  return 0;
}
