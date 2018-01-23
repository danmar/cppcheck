// To test:
// ~/cppcheck/cppcheck --dump misra-test.c && python misra.py -verify misra-test.c.dump

#include "path\file.h" // 20.2
#include /*abc*/ "file.h" // 20.3
#include <setjmp.h> // 21.4
#include <signal.h> // 21.5
#include <tgmath.h> // 21.11

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

//// 3.1

void misra_5_1() {
  int a123456789012345678901234567890; // no-warning
  int a1234567890123456789012345678901; // 5.1
}

void misra_5_3() {
  u8 x=1;
  if (y!=0) {
    u8 x=2; // 5.3
  } else {}
}

#define m54_123456789012345678901234567890123456789012345678901234567890    1 // 5.4
#define m54_1234567890123456789012345678901234567890123456789012345678901   2 // 5.4

#define m55(x,y) (x+y)
int m55; // 5.5

void misra_7_1() {
  int x = 066; // 7.1
}

void misra_7_3() {
  int x = 12l; // 7.3
  int x = 12lu; // 7.3
}

extern int a811[]; // 8.11

enum e812 {
  A=3,
  B=3 // 8.12
};

void misra_8_14(char * restrict str) {} // 8.14

void misra_9_5() {
  int x[] = {[0]=23}; // 9.5
}

void misra_10_4(u8 x, u16 y) {
  z = x + y; // 10.4
}

void misra_10_6(u8 x) {
  u16 y = x+x; // 10.6
}

void misra_10_8(u8 x) {
  y = (u16)x;
  y = (u16)(x+x); // 10.8
}

void misra_11_3(u8* p) {
  x = (u64*)p; // 11.3
}

void misra_11_4(u8*p) {
  u64 y = (u64)p; // 11.4
}

void misra_11_5(void *p) {
  x = (u8 *)p; // 11.5
}

void misra_11_6() {
  void *p;
  p = (void*)123; // 11.6
  x = (u64)p; // 11.6
}

struct Fred {}; struct Wilma {};
void misra_11_7(struct Fred *fred) {
  struct Wilma *wilma = (struct Wilma *)fred; // 11.7
}

char * misra_11_8(const char *str) {
  return (char *)str; // 11.8
}

#define MISRA_11_9  ((void*)0)  // 11.9

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

#if A>1  // no-warning
  (void)0;
#endif

  do {} while (x<0); // no-warning
}

void misra_15_7() {
  if (x!=0){} // no-warning
  if (x!=0){} else if(x==1){} // 15.7
  if (x!=0){} else if(x==1){}else{;} // no-warning
}

void misra_16_2() {
  switch (x) {
    default:
      break;
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

void misra_16_4() {
  switch (x) { // 16.4
    case 1:
      break;
    case 2:
      break;
  }
}

void misra_16_5() {
  switch (x) {
    case 1:
      break;
    default: // 16.5
      break;
    case 2:
      break;
  }
}

void misra_16_6() {
  switch (x) { // 16.6
    default:
      break;
  }

  switch (x) {
  case 1: {break;}
  case 2: {break;}
  default: {break;}
  }
}

void misra_16_7() {
  switch (x != 123) { // 16.7
    case 1:
      break;
    default:
      break;
  }
}

void misra_17_1() {
  va_list(); // 17.1
  va_arg(); // 17.1
  va_start(); // 17.1
  va_end(); // 17.1
  va_copy(); // 17.1
}

void misra_17_6(int x[static 20]) {} // 17.6

void misra_17_8(int x) {
  x = 3; // 17.8
}

void misra_18_5() {
  int *** p;  // 18.5
}

void misra_18_8(int x) {
  int buf1[10];
  int buf2[sizeof(int)];
  int vla[x]; // 18.8
}

union misra_19_2 { }; // 19.2

#include "notfound.h" // 20.1

#define int short // 20.4
#undef X  // 20.5

void misra_21_3() {
  p1=malloc(10); // 21.3
  p2=calloc(10); // 21.3
  realloc(10); // 21.3
  free(p1); // 21.3
}

void misra_21_7() {
  atof(str); // 21.7
  atoi(str); // 21.7
  atol(str); // 21.7
  atoll(str); // 21.7
}

void misra_21_8() {
  abort(); // 21.8
  getenv("foo"); // 21.8
  system(""); // 21.8
}

void misra_21_9() {
  bsearch(key,base,num,size,cmp); // 21.9
  qsort(base,num,size,cmp); // 21.9
}
