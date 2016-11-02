
int TestData[100];


void test_function_par(int par) {
  TestData[par] = 0;
}
void test_function_par2(int x, int y) {
  if (x < 123)
      TestData[y] = 0;
}
void call(int x) {
  test_function_par1(1000);
  test_function_par2(x, x < 1000 ? 10 : 1000);
}

int getLargeIndex() { return 1000; }
void test_function_return() {
  TestData[getLargeIndex()] = 0;
}


