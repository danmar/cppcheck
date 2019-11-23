#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;

  int* pI = new int;
  *pI=2;
  cerr << "Now freeing a pointer instead of deleting it..." << endl;
  free(pI);
  cerr << "Did you notice?" << endl;


  pI = new int;
  delete(pI);
  cerr << "Now deleting twice..." << endl;
  delete(pI);
  cerr << "Did you notice?" << endl;

  cerr << "There should be 2 errors in this run" << endl;
  return 0;
}
