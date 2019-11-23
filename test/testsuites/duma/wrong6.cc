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

  void doNothing() {
    cout << "  hi!" << endl;
  };

};

int main() {
  cout << "Hello world!" << endl;

  Test ar[10];
  Test b;
  cerr << "Let's index out of bounds " << endl;
  ar[10].doNothing();
  cerr << "Did you notice?" << endl;
  
  cerr << "There should be 1 error in this run" << endl;
  return 0;
}
