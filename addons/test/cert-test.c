// To test:
// ~/cppcheck/cppcheck --dump cert-test.c && python cert.py -verify cert-test.c.dump

unsigned char int31(int x) {
  x = (unsigned char)1000; // cert-INT31-c
  x = (signed char)0xff; // cert-INT31-c
  x = (unsigned char)-1; // cert-INT31-c
  x = (unsigned long long)-1; // cert-INT31-c
}
