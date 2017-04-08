/*
~/cppcheck/cppcheck --dump misra-test.c
python misra.py misra-test.c.dump

Expected output:
[misra-test.c:5] (style) misra: 11 Identifier is longer than 31 characters
[misra-test.c:9] (style) misra: 14 The type char shall always be declared as unsigned char or signed char
[misra-test.c:3] (style) misra: 15 Make sure the floating point implementation comply with a defined floating point standard
[misra-test.c:17] (style) misra: 33 The right hand of a && or || shall not have side effects
[misra-test.c:21] (style) misra: 34 The operands of a && or || shall be primary expressions
[misra-test.c:3] (style) misra: 41 The implementation of integer division in the chosen compiler should be determined, documented and taken into account.
[misra-test.c:25] (style) misra: 56 The goto statement shall not be used
[misra-test.c:29] (style) misra: 57 The continue statement shall not be used
*/

void misra11() {
  int a123456789012345678901234567890; // no-warning
  int a1234567890123456789012345678901; // 11
}

void misra14() {
  char c; // 14
}

void misra28() {
  register int x = 3; // 28
}

void misra33() {
  if (x && (y++ < 123)){} // 33
}

void misra34() {
  if (x+3 && y){} // 34
}

void misra56() {
  goto a1; // 56
}

void misra57() {
  continue; // 57
}
