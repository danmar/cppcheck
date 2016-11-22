
int TestData[100];


void par_not_dependant(int par) {
  TestData[par] = 0; // BUG
}
void par_dependant(int x, int y) {
  if (x < 10)
      TestData[y] = 0; // BUG
}
void call(int x) {
  par_not_dependant(1000);
  par_dependant(0, 1000);
}

int getLargeIndex() { return 1000; }
void return_value() {
  TestData[getLargeIndex()] = 0; // BUG
}


