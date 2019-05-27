#include <noduma.h>
#include <iostream>
#include <string>
#include <dumapp.h>

using namespace std;

class Test {
public:
  int a;
  string stdstr;
  
  Test() {
    a=2;
    stdstr = "test";
  }

};

int main() {
  cout << "Hello world!" << endl;

  {
  Test* pI = new Test[10];
  cerr << "Let's delete instead of delete [] " << endl;
  delete pI;
  cerr << "Did you notice?" << endl;
  }

  {
  Test* pI = new Test[10];
  cerr << "Now let's free instead of delete [] " << endl;
  free(pI);
  cerr << "Did you notice?" << endl;
  }

  cerr << "There should be 2 errors in this run" << endl;
  return 0;
}
