/*
~/cppcheck/cppcheck --dump misra-test.c
python misra.py misra-test.c.dump
*/

void misra_5_1() {
  int a123456789012345678901234567890; // no-warning
  int a1234567890123456789012345678901; // 51
}

void misra_7_1() {
  int x = 066; // 71
}

void misra_7_3() {
  int x = 12l; // 73
}

void misra_13_5() {
  if (x && (y++ < 123)){} // 135
}

void misra_14_4() {
  if (x+4){} // 144
}

void misra_15_1() {
  goto a1; // 151
}

