#include <noduma.h>
#include <iostream>
#include <dumapp.h>

using namespace std;

int main() {
  cout << "Hello world!" << endl;

  int* pI = new int;
  int j;
  cerr << "Now reading uninitialized memory" << endl;
  j = *pI+2;
  cerr << "Did you notice? (value was " << j << ") " << endl;
  delete pI;
  cerr << "(No memory leak here)" << endl;

  int* pJ;
  cerr << "Now writing to uninitialized pointer" << endl;
  *pJ = j;
  cerr << "Did you notice?" << endl;

  // valgrind reports 4, but that's ok
  cerr << "There should be 2 errors in this run" << endl;
  return 0;
}
