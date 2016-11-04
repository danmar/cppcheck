
int TestData[100];


void function_par_not_dependant(int par) {
  TestData[par] = 0; // BUG
}
void function_par_dependant(int x, int y) {
  if (x < 10)
      TestData[y] = 0; // BUG
}
void call(int x) {
  function_par_not_dependant(1000);
  function_par_dependant(0, 1000);
}

int getLargeIndex() { return 1000; }
void test_function_return() {
  TestData[getLargeIndex()] = 0; // BUG
}


