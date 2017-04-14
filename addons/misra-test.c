/*
~/cppcheck/cppcheck --dump misra-test.c
python misra.py misra-test.c.dump
*/

typedef unsigned char u8;

void misra_5_1() {
  int a123456789012345678901234567890; // no-warning
  int a1234567890123456789012345678901; // 5.1
}

void misra_7_1() {
  int x = 066; // 7.1
}

void misra_7_3() {
  int x = 12l; // 7.3
  int x = 12lu; // 7.3
}

void misra_12_1() {
  sz = sizeof x + y; // 12.1
  a = (b * c) + d;
  a = b << c + d; // 12.1
}

void misra_12_2(u8 x) {
  a = x << 8;  // 12.2
}

void misra_12_3() {
  f((1,2),3); // TODO
  for (i=0;i<10;i++,j++){} // 12.3
}

void misra_12_4() {
  x = 123456u * 123456u; // 12.4
}

void misra_13_1(int *p) {
  volatile int v;
  int a[3] = {0, (*p)++, 2}; // 13.1
  int b[2] = {v,1}; // TODO
}

void misra_13_3() {
  x = y++; // 13.3
}

void misra_13_4() {
  if (x != (y = z)) {} // 13.4
  else {}
}

void misra_13_5() {
  if (x && (y++ < 123)){} // 13.5
  else {}
}

void misra_13_6() {
  return sizeof(x++); // 13.6
}

void misra_14_1() {
  for (float f=0.1f; f<1.0f; f += 0.1f){} // 14.1
}

void misra_14_2() {
  for (dostuff();a<10;a++) {} // 14.2
  for (;i++<10;) {} // 14.2
  for (;i<10;dostuff()) {} // TODO
  // TODO check more variants
}

void misra_14_4() {
  if (x+4){} // 14.4
  else {}
}

void misra_15_1() {
  goto a1; // 15.1
a1:
}

void misra_15_2() {
label:
  goto label; // 15.2 15.1
}

void misra_15_3() {
  if (x!=0) {
    goto L1; // 15.3 15.1
    if (y!=0) {
      L1:
    } else {}
  } else {}
}

int misra_15_5() {
  if (x!=0) {
    return 1; // 15.5
  } else {}
  return 2;
}

void misra_15_6() {
  if (x!=0); // 15.6
  else{}
}

void misra_15_7() {
  if (x!=0){} // 15.7
}

void misra_16_2() {
  switch (x) {
    case 1:
      while (y>4) {
        case 2: break; // 16.2
      }
      break;
  }
}

void misra_16_3() {
  switch (x) {
    case 1:
    case 2:
      a=1;
    case 3: // 16.3
      a=2;
      // fallthrough
    case 5:
      break;
    default: break;
  }
}
