// To test:
// ~/cppcheck/cppcheck --dump cert-test.c && python cert.py -verify cert-test.c.dump

unsigned char f(int x) {
  if (x==1000) {}
  return (unsigned char)x; // cert-INT31-c
}
