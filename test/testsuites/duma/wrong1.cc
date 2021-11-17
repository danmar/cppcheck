#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;
  int* pI = new int;
  *pI=2;
  delete(pI);
  cerr << "Now deleting a pointer twice..." << endl;
  delete(pI);
  cerr << "Did you notice?" << endl;
  return 0;
}
