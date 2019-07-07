// To test:
// ~/cppcheck/cppcheck --dump misra-test.c && python ../misra.py -verify misra-test.c.dump

#include "path\file.h" // 20.2
#include /*abc*/ "file.h" // no warning
#include PATH "file.h" // 20.3
#include<file.h> // no warning
#include <setjmp.h> // 21.4
#include <signal.h> // 21.5
#include <stdio.h> //21.6
#include <wchar.h> //21.6
#include <time.h> // 21.10
#include <tgmath.h> // 21.11


typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;

//   // 3.1
////

extern int misra_5_1_extern_var_hides_var_x;
extern int misra_5_1_extern_var_hides_var_y; //5.1
int misra_5_1_var_hides_var________a;
int misra_5_1_var_hides_var________b; //5.1

extern const uint8_t misra_5_2_var1;
const uint8_t        misra_5_2_var1 = 3; // no warning
static int misra_5_2_var_hides_var______31x;
static int misra_5_2_var_hides_var______31y;//5.2
static int misra_5_2_function_hides_var_31x;
void misra_5_2_function_hides_var_31y(void) {}//5.2
void foo(void) 
{
  int i;
  switch(misra_5_2_func1()) //16.4 16.6
  {
    case 1: 
    {
      do
      {
        for(i = 0; i < 10; i++) 
        {
          if(misra_5_2_func3()) //14.4
          {
            int misra_5_2_var_hides_var_1____31x;
            int misra_5_2_var_hides_var_1____31y;//5.2
          }
        }
      } while(misra_5_2_func2()); //14.4
    } 
  }
}

union misra_5_2_field_hides_field__63x { //19.2
int misra_5_2_field_hides_field__31x;
int misra_5_2_field_hides_field__31y;//5.2
};
struct misra_5_2_field_hides_field__63y { //5.2
int misra_5_2_field_hides_field1_31x;
int misra_5_2_field_hides_field1_31y;//5.2
};
const char *s41_1 = "\x41g"; // 4.1
const char *s41_2 = "\x41\x42";
int c41_3         = '\141t'; // 4.1
int c41_4         = '\141\t';

extern int misra_5_3_var_hides_var______31x;
void misra_5_3_var_hides_function_31x (void) {}
enum misra_5_3_Enum {
misra_5_3_var_hidesenumconst_31x = 2,misra_5_3_enum_hidesfunction_31x = 5
};
void misra_5_3_func1(void)
{
  int misra_5_3_var_hides_var______31y; //5.3
  int misra_5_3_var_hides_function_31y; //5.3
  int misra_5_3_var_hidesenumconst_31y; //5.3
  switch(misra_5_3_func2()) //16.4 16.6
  {
    case 1:
    {
      do 
      {
        int misra_5_3_var_hides_var_1____31x;
        if(misra_5_3_func3()) //14.4
        {
          int misra_5_3_var_hides_var_1____31y = 1; //5.3
        }
      } while(misra_5_3_func2()); //14.4
    }
  }
}
void misra_5_3_enum_hidesfunction_31y(void) {} //5.3


#define misra_5_4_macro_hides_macro__31x 1
#define misra_5_4_param_hides_macro__31x 1
#define misra_5_4_macro_hides_macro__31y 2 //5.4
#define m1(misra_5_4_param_hides_macro__31y) 1 //5.4
#define m2(misra_5_4_param_hides_param__31x,misra_5_4_param_hides_param__31y) 1 //5.4

#define misra_5_5_var_hides_macro____31x 1
#define misra_5_5_functionhides_macro31x 1
#define misra_5_5_param_hides_macro__31x 1
#define misra_5_5_tag_hides_macro____31x 1
#define misra_5_5_hides_macro________31x 1

int misra_5_5_var_hides_macro____31y; //5.5
void misra_5_5_functionhides_macro31y(int misra_5_5_param_hides_macro__31y){} //5.5
struct misra_5_5_tag_hides_macro____31y { //5.5
int x;
};
void misra_5_5_func1() 
{
  switch(misra_5_5_func2()) //16.4 16.6
  {
    case 1:
    {
      do 
      {
        if(misra_5_5_func3()) //14.4
        {
          int misra_5_5_hides_macro________31y; //5.5
        }
      } while(misra_5_5_func2()); //14.4
    }
  }
}


void misra_7_1() {
  int x = 066; // 7.1
}

void misra_7_3() {
  long misra_7_3_a = 0l; //7.3       
  long misra_7_3_b = 0lU; //7.3     
  long long misra_7_3_c = 0Ull; //7.3     
  long long misra_7_3_d = 0ll; //7.3     
  long double misra_7_3_e = 7.3l; //7.3  
  }


extern int a811[]; // 8.11

enum misra_8_12_a { misra_a1 = 1, misra_a2 = 2, misra_a3, misra_a4 = 3 }; //8.12
enum misra_8_12_b { misra_b1, misra_b2, misra_b3 = 3, misra_b4 = 3 }; // no-warning
enum misra_8_12_c { misra_c1 = misra_a1, misra_c2 = 1 }; // no-warning
enum misra_8_12_d { misra_d1 = 1, misra_d2 = 2, misra_d3 = misra_d1 }; // no-warning

void misra_8_14(char * restrict str) {} // 8.14

void misra_9_5() {
  int x[] = {[0]=23}; // 9.5
}

void misra_10_1() {
  int32_t i;
  i = 3 << 1; // 10.1
}

void misra_10_4(u32 x, s32 y) {
  z = x + 3; // 10.4
  enum misra_10_4_enuma { misra_10_4_A1, misra_10_4_A2, misra_10_4_A3 } a;
  enum misra_10_4_enumb { misra_10_4_B1, misra_10_4_B2, misra_10_4_B3 };
  if ( misra_10_4_B1 > misra_10_4_A1 ) //10.4
   {
      ; 
   }
  z = x + y; //10.4
  z = (a == misra_10_4_A3) ? x : y; //10.4
  z = (a == misra_10_4_A3) ? y : y; // no-warning
}

void misra_10_6(u8 x, u32 a, u32 b) {
  u16 y = x+x; // 10.6
  u16 z = ~u8 x ;//10.6
  u32 c = ( u16) ( u32 a + u32 b ); //10.6
}

void misra_10_8(u8 x, s32 a, s32 b) {
  y = (u16)x;
  y = (u16)(x+x); // 10.8
  y = (u16) (a + b) //10.8
}

struct Fred {}; struct Wilma {};
void misra_11_3(u8* p, struct Fred *fred) {
  x = (u64*)p; // 11.3
  struct Wilma *wilma = (struct Wilma *)fred; // 11.3
}

void misra_11_4(u8*p) {
  u64 y = (u64)p; // 11.4
  u8 *misra_11_4_A = ( u8 * ) 0x0005;// 11.4
  s32 misra_11_4_B;
  u8 *q = ( u8 * ) misra_11_4_B; // 11.4

}

void misra_11_5(void *p) {
  u16 *p16;
  x = (u8 *)p; // 11.5
  p16 = p;     // 11.5
}

void misra_11_6() {
  void *p;
  p = (void*)123;  // 11.6
  x = (u64)p;      // 11.6
  p = ( void * )0; // no-warning
  (void)p;         // no-warning
}


void misra_11_7(int *p, float f) {
  x = ( float ) p; //11.7
  y = ( int * ) f; //11.7
}

char * misra_11_8(const char *str) {
  (void)misra_11_8(str); // no-warning
  return (char *)str; // 11.8
}

#define MISRA_11_9_NULL_1 (1-1)
#define MISRA_11_9_NULL_2 ( void * ) 0
#define MISRA_11_9_NULL_3 NULL
void misra_11_9(){
  int *p1 = (5-5); //11.9
  int *p2 = MISRA_11_9_NULL_2 ; // no-warning
  int *p3 = MISRA_11_9_NULL_3 ; // no-warning
  if ( p1 == MISRA_11_9_NULL_1 ) //11.9
   {
    ;
   }

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

struct misra_13_1_t { int a; int b; };
void misra_13_1(int *p) {
  volatile int v;
  int a[3] = {0, (*p)++, 2}; // 13.1
  int b[2] = {v,1};
  struct misra_13_1_t c = { .a=4, .b=5 }; // no fp
}

void misra_13_3() {
  x = y++; // 13.3
}

#define STRING_DEF_13_4    "This is a string"

typedef struct
{
    char string[sizeof(STRING_DEF_13_4)];
} s13_4_t;

static s13_4_t s13_4 =
{
    .string = STRING_DEF_13_4 // no-warning
};

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
  float a = 0.0f;
  int b = 10;
  while ((a<100.0f) || (b > 100)) //14.1
  {
    a++;
  }
  do
  {
    ;
  } while ( a < 10.0f );  // no-warning

}

void misra_14_2() {
  for (dostuff();a<10;a++) {} // 14.2
  for (;i++<10;) {} // 14.2
  for (;i<10;dostuff()) {} // TODO
  // TODO check more variants
}

struct {
  unsigned int x:1;
  unsigned int y:1;
} _14_4_struct;
void misra_14_4(bool b) {
  if (x+4){} // 14.4
  else {}

  if (b) {}
  else {}

  if (_14_4_struct.x) {}
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
  uint32_t var = 0;
  uint32_t var2 = 0;

  if (x!=0){} // no-warning
  if (x!=0){} else if(x==1){} // 15.7
  if (x!=0){} else if(x==1){}else{;} // no-warning

  if (x!=0)
  {
  }
  else
  {
    var = 5u;

    if (var != 5u)
    {
        var2 = 10u;
    }   // no-warning
  }
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
    case 7:
      a=3;
      [[fallthrough]];
    case 8:
      a=4;
      break;
    case 9:
      if (a==b) {
        break;
      }
    case 10:  // 16.3
      return; // 15.5
    case 11:
    { break; }
    case 12:
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
  case 1: break;
  case 2: break;
  default: break;
  }

  // No 16 6 in this switch:
  switch (x) {
  case A: return 1; // 15.5
  case B: return 1; // 15.5
  case C: return 1; // 15.5
  default: return 2; // 15.5
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

int calculation(int x) { return x + 1; }
void misra_17_7(void) {
  calculation(123); // 17.7
}

void misra_17_8(int x) {
  x = 3; // 17.8
}

void misra_18_4()
{
  int b = 42;
  int *bp = &b;
  bp += 1; // 18.4
  bp -= 2; // 18.4
  int *p = bp - 2; // 18.4
  p = bp + p; // 18.4
  bp = 1 + p + 1; // 18.4
  b += 19; // no-warning
  b = b + 9; // no-warning
}

void misra_18_5() {
  int *** p;  // 18.5
}

struct {
  uint16_t len;
  struct {
    uint8_t data_1[]; // 18.7
  } nested_1;
  struct named {
    struct {
      uint8_t len_1;
      uint32_t data_2[]; // 18.7
    } nested_2;
    uint8_t data_3[]; // 18.7
  } nested_3;
} _18_7_struct;
struct {
  uint16_t len;
  uint8_t data_1[ 19 ];
  uint8_t data_2[   ]; // 18.7
} _18_7_struct;

void misra_18_8(int x) {
  int buf1[10];
  int buf2[sizeof(int)];
  int vla[x]; // 18.8
}

union misra_19_2 { }; // 19.2

#include "notfound.h" // 20.1

#define int short // 20.4
#undef X  // 20.5

#define M_20_7_1(A)  (A+1) // 20.7
#define M_20_7_2(A,B)  (1+AB+2) // no warning
#define M_20_7_3(A)  ((A)+A) // 20.7

#define STRINGIFY(a) (#a) // 20.7 20.10

#else1 // 20.13

#ifdef A>1
# define somethingis 5 // no warning
# define func_20_13(v) (v) // no warning
#else
# definesomethingis 6 // 20.13
# def fun_2013(v) () // 20.13
#endif

void misra_21_3() {
  p1=malloc(10); // 21.3
  p2=calloc(10); // 21.3
  realloc(10); // 21.3
  free(p1); // 21.3
}

void misra_21_7() {
  (void)atof(str); // 21.7
  (void)atoi(str); // 21.7
  (void)atol(str); // 21.7
  (void)atoll(str); // 21.7
}

void misra_21_8() {
  abort(); // 21.8
  (void)getenv("foo"); // 21.8
  (void)system(""); // 21.8
  exit(-1); // 21.8
}

void misra_21_9() {
  (void)bsearch(key,base,num,size,cmp); // 21.9
  qsort(base,num,size,cmp); // 21.9
}
