// To test:
// ~/cppcheck/cppcheck --dump --suppress=uninitvar --suppress=uninitStructMember misra/misra-test.c --std=c89 && python3 ../misra.py -verify misra/misra-test.c.dump

#include "path\file.h" // 20.2
#include "file//.h" // 20.2
#include "file/*.h" // 20.2
#include "file'.h" // 20.2
#include <file,.h> // 20.2
#include "file,.h" // 20.2

#include /*abc*/ "file.h" // no warning
/*foo*/#include "file.h" // no warning
#include "./file.h" // no warning
#include \
    "file.h"
#include /*abc*/ \
    "file.h"
#include "fi" "le.h" // 20.3 (strings are concatenated after preprocessing)
#include "fi" <le.h> // 20.3
#include <fi> <le.h> // 20.3
#include PATH "file.h" // 20.3
#define H_20_3_ok "file.h"
#include H_20_3_ok
#include file.h // 20.3
#define H_20_3_bad file.h
#include H_20_3_bad // TODO: 20.3 Trac #9606
#include "//file.h" // 20.2
#include "//file.h" H_20_3_bad // 20.2 20.3
//#include H_20_3_bad // no warning
#include H_20_3_ok H_20_3_ok // 20.3
#include<file.h> // no warning

#include <setjmp.h> // 21.4
#include <signal.h> // 21.5
#include <stdio.h> //21.6
#include <wchar.h> //21.6
#include <time.h> // 21.10
#include <tgmath.h> // 21.11
#include <fenv.h>


typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;

/* // */   // 3.1
/* /* */   // 3.1
////

// http://example.com // no warning

void misra_2_7_unused_param (int *param1, int unused_param)  // 2.7
{
    *param1 = 42U;
}

void misra_2_7_used_params (int *param1, int param2, int param3)
{
    (void)param3;
    *param1 = param2;
}

void misra_3_2(int enable)
{
    // This won't generate a violation because of subsequent blank line \

    int y = 0;
    int x = 0;  // 3.2 non-compliant comment ends with backslash \
    if (enable != 0)
    {
        ++x;    // This is always executed
        // 3.2 potentially non-compliant comment ends with trigraph resolved to backslash ??/
        ++y;    // This is hidden if trigraph replacement is active
    }

    (void)printf("x=%i, y=%i\n", x, y);
}

extern int misra_5_1_extern_var_hides_var_x;
extern int misra_5_1_extern_var_hides_var_y; //5.1
int misra_5_1_var_hides_var________a;
int misra_5_1_var_hides_var________b; int misra_5_1_var_hides_var________b1; int misra_5_1_var_hides_var________b2; //5.1
int misra_5_1_var_hides_var________c; //5.1
int misra_5_1_var_hides_var________d; //5.1
int misra_5_1_var_hides_var________e; //5.1

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
const char *s41_3 = "\x41" "\x42";
const char *s41_4 = "\x41" "g";
const char *s41_5 = "\x41\xA";
const char *s41_6 = "\xA\x41";
const char *s41_7 = "\xAA\xg\x41"; // 4.1
const char *s41_8 = "\xAA\x\x41"; // 4.1
const char *s41_9 = "unknown\gsequence";
const char *s41_10 = "simple\nsequence";
const char *s41_11 = "string";
int c41_3         = '\141t'; // 4.1
int c41_4         = '\141\t';
int c41_5         = '\0';
int c41_6         = '\0\t';
int c41_7         = '\12\t';
int c41_8         = '\0t';   // 4.1
int c41_9         = '\12';
int c41_10        = '\12\n';
int c41_11        = '\12n';  // 4.1
int c41_12         = '\12323'; // 4.1
int c41_13         = '\123\3';
int c41_14         = '\777\777';
int c41_15         = 'a';

void misra_4_1()
{
    (void)printf("\x41g"); // 4.1
    (void)printf("\x41\x42");
    (void)printf("\x41" "g");
}

const char *s42_1 = "String containing trigraphs ??-??-??";   // 4.2
const char *s42_2 = "String containing trigraph???=preceded by question mark";   // 4.2
const char *s42_3 = "No trigraph?(?'?)";

void misra_4_2()
{
    (void)printf("??=Trigraph\n");   // 4.2
    (void)printf("No?/Trigraph\n");
}

#define misra_5_4_macro_hides_macro__31x 1
#define misra_5_4_param_hides_macro__31x 1
#define misra_5_4_macro_hides_macro__31y 2 //5.4
#define m1(misra_5_4_param_hides_macro__31y) 1 //5.4
#define m2(misra_5_4_param_hides_param__31x,misra_5_4_param_hides_param__31y) 1 //5.4
#ifdef misra_5_4_macro_hides_macro__31x
#define misra_5_4_macro 1 // no warning
#else
#define misra_5_4_macro 2 // no warning
#endif

#define misra_5_5_var_hides_macro____31x 1
#define misra_5_5_functionhides_macro31x 1
#define misra_5_5_param_hides_macro__31x 1
#define misra_5_5_tag_hides_macro____31x 1
#define misra_5_5_hides_macro________31x 1

int misra_5_5_var_hides_macro____31y; //5.5
void misra_5_5_functionhides_macro31y(int misra_5_5_param_hides_macro__31y){(void)misra_5_5_param_hides_macro__31y;} //5.5
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

typedef unsigned int UINT_TYPEDEF;
struct struct_with_bitfields
{
  unsigned int a:2; // Compliant
  signed int   b:2; // Compliant
  UINT_TYPEDEF c:2; // Compliant
  int          d:2; // 6.1 - plain int not compliant
  signed long  f:2; // 6.1 - signed long not compliant
  unsigned int g:1; // Compliant
  signed int   h:1; // 6.2 - signed int with size 1 is not compliant
};

void misra6_1_fn() {
    // "Use" occurrence should not generate warnings
    struct_with_bitfields s;
    s.h = 61;
}

void misra_7_1() {
  int x = 066; // 7.1
}

void misra_7_2_call_test(int a, unsigned int b, unsigned int c) { } // 2.7

void misra_7_2_call_va_test(int a, ...) { } // 2.7

void misra_7_2() {
    unsigned int a = 2147483647;
    const unsigned int b = 2147483648U;
    const unsigned int c = 2147483648; // 7.2
    unsigned int d = 2147483649; // 7.2

    unsigned char e = 0x80; // 7.2
    unsigned char f = 0x80U;
    unsigned short g = 0x8000; // 7.2
    unsigned short h = 0x8000U;
    unsigned int i = 0x80000000; // 7.2
    unsigned int j = 0x80000000U;
    unsigned long long k = 0x8000000000000000; // 7.2
    unsigned long long l = 0x8000000000000000ULL;

    unsigned int m = 1 + 0x80000000; // 7.2 10.4

    misra_7_2_call_test(1, 2, 2147483648U);
    misra_7_2_call_test(1, 2, 2147483648); // 7.2
    misra_7_2_call_test(1, 0x80000000, 3); // 7.2
    misra_7_2_call_va_test(1, 2, 3);
}

void misra_7_3() {
  long misra_7_3_a = 0l; //7.3
  long misra_7_3_b = 0lU; //7.3
  long long misra_7_3_c = 0Ull; //7.3
  long long misra_7_3_d = 0ll; //7.3
  long double misra_7_3_e = 7.3l; //7.3
  }

typedef const char* MISRA_7_4_CHAR_CONST;
MISRA_7_4_CHAR_CONST misra_7_4_return_const_type_def (void) { return "return_typedef_const"; }
char *misra_7_4_return_non_const (void) { return 1 + "return_non_const"; } // 7.4 18.4
const char *misra_7_4_return_const (void) { return 1 + "return_const"; } // 18.4

void misra_7_4_const_call(int a, const char* b) { } // 2.7
void misra_7_4_const_ptr_call(int a, const char const* b) { } // 2.7
void misra_7_4_call(int a, char* b) { } // 2.7

void misra_7_4()
{
   const char *a = "text a";
   char* const b = "text_b"; // 7.4
   char *c = "text c";  // 7.4
   char *d = 1 + "text d"; // 7.4 18.4
   char *e = "text e" + 1 + 2; // 7.4 18.4
   char *f = 1 + "text f" + 2; // 7.4 18.4
   const wchar_t *g = "text_g";
   wchar_t *h = "text_h"; // 7.4

   misra_7_4_const_call(1, ("text_const_call"));
   misra_7_4_const_ptr_call(1, ("text_const_call"));
   misra_7_4_call(1, "text_call"); // 7.4 11.8
}

extern int a811[]; // 8.11

enum misra_8_12_a { misra_a1 = 1, misra_a2 = 2, misra_a3, misra_a4 = 3 }; //8.12
enum misra_8_12_b { misra_b1, misra_b2, misra_b3 = 3, misra_b4 = 3 }; // no-warning
enum misra_8_12_c { misra_c1 = misra_a1, misra_c2 = 1 }; // no-warning
enum misra_8_12_d { misra_d1 = 1, misra_d2 = 2, misra_d3 = misra_d1 }; // no-warning
enum misra_8_12_e { misra_e1 = sizeof(int), misra_e2}; // no-crash

void misra_8_14(char * restrict str) {(void)str;} // 8.14

void misra_9_empty_or_zero_initializers() {
    int a[2]    = {};                          // 9.2
    int b[2][2] = {};                          // 9.2
    int c[2][2] = { {} };                      // 9.2 9.3
    int d[2][2] = { {}, {} };                  // 9.2
    int e[2][2] = { { 1 , 2 }, {} };           // 9.2

    int f[5]    = { 0 };
    int g[5][2] = { 0 };
    int h[2][2] = { { 0 } };                   // 9.3
    int i[2][2] = { { 0 }, { 0 } };
    int j[2][2] = { { 1, 2 }, { 0 } };
    int k[2][2] = { [0] = { 1 , 2 }, { 0 } };
    int l[1][2] = { { 0 }, [0] = { 1 } };      // 9.3 9.4

    typedef struct {
        int a;
        int b;
    } struct1;

    struct1 m   = { };                         // 9.2
    struct1 n   = { 0 };
}

void misra_9_string_initializers() {
    const char a[12]    = { "Hello world" };           // 9.2
    const char b[2][20] = "Hello world";               // 9.2 9.3
    const char c[]      = "Hello world";
    const char d[15]    = "Hello world";
    const char e[1][12] = { "Hello world" };
    const char *f[2]    = { "Hello", [1] = "world" };
    const char *g[1]    = "Hello world";               // 9.2

    const char h[2][15] = { { 0 }, "Hello world" };

    char **str_p = &f[0];

    char **i[1]         = { str_p };
    char **j[1]         = { { str_p } };               // 9.2
}

void misra_9_array_initializers() {
    char    a[4]        = { 1, 2, 3, 4 };
    char    b[2][2]     = { {1, 2}, {3, 4} };
    char    c[2][2]     = { 1, 2, 3, 4 };                                   // 9.2
    char    d[6]        = { { 1, 2 }, { 3, 4 }, { 5, 6 } };                 // 9.2 9.3

    char    e[2][2]     = { {1, 2}, {4} };                                  // 9.3
    char    f[2][2]     = { 1, 2, 3 };                                      // 9.2 9.3
    char    g[2][2]     = { {1, 2, 3, 4} };                                 // 9.3

    char    h[2][2]     = { { 1, { 2 } }, { 3, { 5 } } };                   // 9.2
    char    i[2][2]     = { { 1, { 2 } }, { 3 } };                          // 9.2 9.3
    char    j[2][3]     = { { 1, { 2 }, 3 }, { 4, { 5 }, 6 } };             // 9.2
    char    k[2][3]     = { { 1, { 2 }, 3 }, { 4, { 5 } } };                // 9.2 9.3
    char    l[3]        = { 1, { 2, 3 } };                                  // 9.2 9.3
}

void misra_9_array_initializers_with_designators() {
    char    a[1]        = { [0][1] = 1 };                                   // 9.2
    char    b[1]        = { [0] = { 1, 2 } };                               // 9.2
    char    c[2][2]     = { [0] = {1, 2, 3} };
    char    d[1][2]     = { [0] = 1 };                                      // 9.2
    char    e[2][2]     = { { 1, 2 }, [1][0] = {3, 4} };                    // 9.2
    char    f[2]        = { [0] = 1, 2 };
    char    g[2]        = { [1] = 2, [0] = 1 };
    char    h[2][2]     = { { 1, 2 }, [1] = { 3 } };                        // 9.3
    char    i[2][2]     = { { 1, 2 }, [1] = { 3, 4 } };
    char    j[2][2]     = { { 1, 2 }, [1] = { [0] = 3 } };
    char    k[2][2]     = { { 1, 2 }, [1][0] = 3 };
    char    l[2][2]     = { { 1, 2 }, [1][0] = 3, 4};                       // 9.2
    char    m[2][2]     = { [0] = { [2] = 2 }, [1][5] = 4 };
    char    n[2][2]     = { [0] = { 1 } };                                  // 9.3
    char    o[2][2]     = { { 1 }, [1][0] = 3 };                            // 9.3
    char    p[2][2]     = { { 1, 2 }, { 3, 4 }, [1] = { 3 } };              // 9.3 9.4
    char    q[2][2]     = { { 1, 2 }, { 1 }, [1] = { [1] = 3 } };           // 9.4
    char    r[2][2][2]  = { [0][0] = { 1, 2 }, [1] = { [0] = {5, 6} } };
    char    s[2][2][2]  = { [0][0] = { 1, 2 }, [1] = {5, 6, 7, 8}};         // 9.2
    char    t[2][2][2]  = { [0][0] = { 1, 2 }, {3, 4}, [1] = {5, 6}};       // 9.2 9.3
    char    u[2][2][2]  = { [0] = { 1, 2, {3, 4} } };                       // 9.2
    char    v[2][2][2]  = { [0] = { 1, 2, [1] = {3, 4} }};                  // 9.2
}

void misra_9_struct_initializers() {
    typedef struct {
        int i1;
        int i2;
    } struct1;

    typedef struct {
        char c1;
        struct1 is1;
        char c2[4];
    } struct2;

    typedef struct {
        struct1 s[2][2];
    } struct3;

    struct3 sa[2]  = { [1].s[1][0].i1 = 3, 4 };         // 9.2

    struct1 sa          = 1;                            // 9.2

    struct1 sb     = { 1, 2 };
    struct2 sc     = { 1, { 2 }, {4, 5, 6, 7} };
    struct2 sd     = { 1, { 2, 3 }, {4, 5, 6} };        // 9.3
    struct2 se     = { 1, 2, 3, 4, 5, 6, 7  };          // 9.2
    struct2 sf     = { 1, { 2, 3 }, 4, 5, 6, 7 };       // 9.2
    struct2 sg     = { 1, { 2 }, 4, 5, 6, 7 };          // 9.2
    struct2 sh     = { 1, { 2, 3 }, 4, 5, 6 };          // 9.2 9.3
    struct2 si     = { 1, 2, 3, {4,5,6,7} };            // 9.2

    int a;
    struct1 sj     = { a = 1, 2 };                      // 13.1

    // Struct types
    struct2 sta      = { .is1 = sc }; // 9.2
    struct2 stb      = { .is1 = sb };
    struct1 stc[1]   = { sc };        // 9.2
    struct1 std[1]   = { sb };

    // Struct designators
    struct1 sda    = { 1, .i2 = 2 };
    struct2 sdb    = { 1, { 2, .i2=3 }, .c2[1]=5 };
    struct2 sdc    = { 1, { 2, .i2=3 }, .c2 = { 5 } };        // 9.3
    struct2 sdd    = { 1, { 2, .i2=3 }, .c2 = 5 };            // 9.2
    struct2 sde    = { .is1 = { 2, 3 }, { 4, 5, 6, 7 } };

    // Struct arrays
    struct1 asa[2] = { {1,2}, {3,4} };
    struct1 asb[2] = { {1}, {3,4} };
    struct1 asc[2] = { {1,2} };                               // 9.3
    struct1 asd[2] = { 1,2, 3,4 };                            // 9.2
    struct1 ase[2] = { 1,2, 3 };                              // 9.2
    struct1 asf[2] = { 1,2 };                                 // 9.2 9.3
    struct1 asg[2] = { [1].i1 = 3 };
    struct3 ash[2] = { [1].s[1][0].i1 = 3 };
    struct3 asi[2] = { [0] = { .s[0] = { { 1, 2 } }}};        // 9.3
    struct3 asj[2] = { [0] = { .s[0] = { 1, 2 }}};            // 9.2 9.3

    // Missing type information
    dummy_struct dsa       = { 1, .a = 2 };
    dummy_struct dsb[2]    = { {1,2}, {3,4} };
    dummy_struct dsc[2][2] = { {1,2}, {3,4} };
    dummy_struct dsd[2][2] = { 1, 2, 3, 4 };                  // 9.2
    dummy_struct dse[3]    = { {1,2}, {3,4}, [1] = {5,6} };   // 9.3 9.4
    dummy_struct dsd[]     = { [0] = 1 };                     // 9.5
}

void misra_9_2() {
    union misra_9_2_union {     // 19.2
        char c;
        struct1 i;
    } u = { 3 };                // 19.2
}

void misra_9_5() {
    char a[]    = { 1, 2, 3 };
    char b[]    = { [2] = 5 };                          // 9.5
    char c[]    = { 1, [1] = 5 };                       // 9.5
    char d[]    = { [1] = 2, [0] = 1 };                 // 9.5

    char e[][2] = { { 1, 2 }, { 3, 4 } };
    char f[][2] = { [1] = { 3, 4 } };                   // 9.5
    char g[][2] = { { 1, 2 }, [1] = { 3, 4 } };         // 9.5
    char h[][2] = { [1] = { 1, 2 }, [0] = { 3, 4 } };   // 9.5
}

typedef char misra_10_1_char_t;
#define MISRA_10_1_CHAR char
void misra_10_1(uint8_t u, char c1, char c2) {
  int32_t i;
  char c;
  enum { E1 = 1 };
  i = 3 << 1; // 10.1
  i = (u & u) << 4; // no-warning
  c = c1 & c2; // 10.1
  c = c1 << 1; // 10.1
  i = c1 > c2; // no-warning
  i = E1 + i; // no-warning

  char ch1 = 'a';
  char ch2 = 'b';
  char ch3;
  ch3 = ch1 & ch2; // 10.1

  misra_10_1_char_t ct1 = 'a';
  misra_10_1_char_t ct2 = 'b';
  misra_10_1_char_t ct3;
  ct3 = ct1 & ct2; // 10.1

  MISRA_10_1_CHAR cd1 = 'a';
  MISRA_10_1_CHAR cd2 = 'b';
  MISRA_10_1_CHAR cd3;
  cd3 = cd1 & cd2; // 10.1
}
void misra_10_1_ternary()
{
    int a;
    uint8_t ui8;
    uint16_t ui16;
    int8_t i8;
    int16_t i16;

    a = ui16 << ui16; // 10.6
    a = ui16 << (get_bool(42) ? ui16 : ui16);
    a = ui16 << (get_bool(42) ? ui16 : (get_bool(34) ? ui16 : ui16)); // 10.4
    a = ui16 << (get_bool(42) ? (get_bool(34) ? ui16 : ui16) : ui16); // 10.4
    a = ui16 << (get_bool(42) ? i16 : (get_bool(34) ? ui16 : ui16)); // 10.1
    a = ui16 << (get_bool(42) ? (get_bool(34) ? ui16 : i16) : ui16); // 10.1 10.4
    a = ui16 << (get_bool(42) ? (get_bool(34) ? ui16 : ui16) : i16); // 10.1
    a = ui16 << (get_bool(42) ? (get_bool(34) ? ui16 : ui8) : ui8); // 10.4
    a = ui16 << (get_bool(42) ? (get_bool(34) ? i16 : ui8) : ui8); // 10.1 10.4
    a = (get_bool(42) ? (get_bool(34) ? ui16 : ui8) : ui8) << ui16; // 10.4
    a = (get_bool(42) ? (get_bool(34) ? i16 : ui8) : ui8) << ui16; // 10.1 10.4
    a = (get_bool(42) ? (get_bool(34) ? ui16 : i8) : ui8) << ui16; // 10.1 10.4
    a = (get_bool(42) ? (get_bool(34) ? ui16 : ui8) : i8) << ui16; // 10.1
    a = (get_bool(42) ? (get_bool(34) ? ui16 : ui8) : ui8) << (get_bool(19) ? ui16 : ui8); // 10.4
    a = (get_bool(42) ? (get_bool(34) ? i16 : ui8) : ui8) << (get_bool(19) ? ui16 : ui8); // 10.1 10.4
    a = (get_bool(42) ? (get_bool(34) ? ui16 : ui8) : ui8) << (get_bool(19) ? i16 : ui8); // 10.1 10.4
}

void misra_10_2() {
    unsigned int u8a = 0;
    signed char cha = 0;
    signed int s8a = 0;
    signed short s16a = 0;
    float f32a = 0.0;
    char res;

    res = '0' + u8a; // 10.4
    res = s8a + '0';
    res = cha - '0';
    res = '0' - s8a;
    res = cha + ':';

    res = s16a - 'a'; // 10.2
    res = '0' + f32a; // 10.2 10.4
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

void misra_10_6(u8 x, u32 a, u32 b, char c1, char c2) {
  u16 y = x+x; // 10.6
  u16 z = ~u8 x ;//10.6
  u32 c = ( u16) ( u32 a + u32 b ); //10.6
  s32 i = c1 - c2; // FIXME: False positive for 10.6 (this is compliant). Trac #9488
}
void misra_10_6_1(uint32_t *a, uint16_t b, uint16_t c)
{
    *a = b + c ; // 10.6
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

void misra_11_7_extra(int *p, float f, bool b) {
  (void) p; // no-warning
  (void) f; // no-warning
  (void) b; // no-warning
}

char * misra_11_8_const(const char *str) {(void)str;}
char * misra_11_8(const char *str) {
  (void)misra_11_8_const(str); // no-warning
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

static int misra_12_3_v1 = 0, misra_12_3_v2; // 12.3
static int misra_12_3_v3, misra_12_3_v4; // 12.3
enum misra_12_3_e1 { M123A1, M123B1, M123C1 };
enum misra_12_3_e2 { M123A2 = 3, M123B2 = 4, M123C2 };
typedef enum misra_12_3_e3 { M123A3 , M123B3, M123C3 } misra_12_3_e3_t;
typedef enum { M123A4 , M123B4, M123C4 } misra_12_3_e4_t;
struct misra_12_3_s1 { int a; int b; int c, d; }; // 12.3
static struct misra_12_3_s1 misra_12_3_s1_inst = {
  3,
  4, 5,
  6, // no warning
};
typedef struct misra_12_3_s2 { int a; int b; int c, d; } misra_12_3_s2_t; // 12.3
typedef struct { int a; int b; int c, d; } misra_12_3_s3_t; // 12.3
void misra_12_3_fn1(int, int); static int misra_12_3_v5, misra_12_4_v6; // 12.3
void misra_12_3_fn2(int a, int b) // 2.7
{ int d, e; } // 12.3
int misra_12_3_fn3(int a, int b) { return a+b;} static int misra_12_3_v5, misra_12_4_v6; // 12.3
void misra_12_3_fn4(const uint32_t value, uint8_t * const y) {} // 2.7
uint32_t misra_12_3_fn5(const uint32_t * const, const uint8_t) {} // 2.7
extern void misra_12_3_fn6(const uint32_t value, uint8_t * const y);
extern uint32_t misra_12_3_fn7(const uint32_t * const, const uint8_t);
#define MISRA_12_3_FN3_1(A, B) (misra_12_3_fn3(A, B))
#define MISRA_12_3_FN3_2(A, B) (misra_12_3_fn3(A, \
                                B))
#define MISRA_12_3_FN3_2_MSG(x) x, fflush(stderr)
void misra_12_3(int, int, int); // no warning
void misra_12_3(int a, int b, int c) { // no warning
  int a1, a2; // 12.3
  int a3; int a4; // no warning
  int a5 = 9, a6; // 12.3
  int a7, a8 = 11; // 12.3
  int a9 = foo(), a10; // 12.3
  int a11 = a = b = c; // 17.8

  struct s1 {int a, b;}; int a12, a13; // 12.3
  int a14, a15; misra_12_3_fn3(a14, a15); // 12.3 17.7
  ; int a16, a17; // 12.3
  int a18; int a19, a20; // 12.3
  int a21, a22; int a23; // 12.3
  int a24, // 12.3
      a25;
  int a26
      , a27; // 12.3
  int a28
      , // 12.3
      a29;

  struct misra_12_3_s2 a30 = {1, 2}, a31; // 12.3
  struct misra_12_3_s2 a32, a33; // 12.3
  struct misra_12_3_s2 a34, a35 = {1, 2}, a36; // 12.3

  int a37 = MISRA_12_3_FN3_1(a34, a35), a38; // 12.3
  int a39, a40 = MISRA_12_3_FN3_1(a34, a35); // 12.3
  int a41 = MISRA_12_3_FN3_2(a34, a35), a42; // 12.3
  int a43, a44 = MISRA_12_3_FN3_2(a34, a35); // 12.3

  MISRA_12_3_FN3_2_MSG(fprintf(stderr, "test\n")); // 12.3

  f((1,2),3); // TODO

  for (i=0; i<10; i++, j++){} // 12.3
  for (int i = 0, p = &a1;  // 12.3 14.2
          i < 42;
          ++i, ++p ) // 12.3
  {}

  // No false positives in local and extern function calls
  misra_12_3_fn4(misra_12_3_fn5(&a1, 32), &a1);
  misra_12_3_fn4(misra_12_3_fn7(&a1, 32), &a1);
  misra_12_3_fn6(misra_12_3_fn5(&a1, 32), &a1);
  misra_12_3_fn6(misra_12_3_fn7(&a1, 32), &a1);
  misra_12_3_fn7(maxlen, fn(va, unsigned long), false);
  misra_12_3_fn8(maxlen, (unsigned long)((uintptr_t)fn(va, void*)), false);

  const struct fun_t
  {
    int64_t x;
    uint32_t y;
  } moreFun[2U] =
  {
    { 900000000000000LL, 0x20000UL },
    { 450000000000000LL, 0x10000UL }
  };
}

#define MISRA12_4a 2000000000u
#define MISRA12_4b 4000000000u
#define volatile_macro_12_4  (*(volatile U32 *) 0xFFFFFC10u)
void misra_12_4() {
  uint32_t x;
  bool t;
  x = 123456u * 123456u; // 12.4
  x = MISRA12_4a + MISRA12_4b; // 12.4
  x = 0u - 1u; // 12.4
  x = t ? 0u : (0u-1u); // 12.4
  x = 556200230913ULL;
  foo(&volatile_macro_12_4); // no crash
}

struct misra_13_1_t { int a; int b; };
uint8_t misra_13_1_x = 0;
void misra_13_1_bar(uint8_t a[2]);
void misra_13_1(int *p) {
  volatile int v;
  int a1[3] = {0, (*p)++, 2}; // 13.1
  int a2[3] = {0, ((*p) += 1), 2}; // 13.1
  int a3[3] = {0, ((*p) = 19), 2}; // 13.1
  misra_13_1_bar((uint8_t[2]){ misra_13_1_x++, misra_13_1_x++ } ); // 13.1
  int b[2] = {v,1};
  struct misra_13_1_t c = { .a=4, .b=5 }; // no fp
  volatile int vv;
  int v = 42;

  int a1[3] = { 0, (*p)++, 2 }; // 13.1
  int a2[2] = { [0]=19, [1]=42 };
  int a3[2] = { [0]=v, [1]=42 };
  int a4[2] = { [0]=0, [1]=(v+=1) }; // 13.1
  int a5[2] = { [0]=0, [1]=(v+1) };
  int a6[2] = { v, 1 };
  int a6[2] = { v >>= 3 }; // 13.1 9.3
  int a7[2] = { v, ++v }; // 13.1
  int a8[1] = { vv }; // TODO: 13.1 Trac #9504

  struct misra_13_1_t c01 = { 4, 5 };
  struct misra_13_1_t c02 = { 16 == 1, 5+1 };
  struct misra_13_1_t c03 = { (v += 1), 5+1 }; // 13.1
  struct misra_13_1_t c04 = { v <<= 1, 5+1 }; // 13.1
  struct misra_13_1_t c05 = { v += 1, 5+1 }; // 13.1
  struct misra_13_1_t c06 = { (4.5 + 0.5), 1 };
  struct misra_13_1_t c07 = { (4.5 + 0.5), ++v }; // 13.1
  struct misra_13_1_t c08 = { (int)4.5, 5 };
  struct misra_13_1_t c09 = { (int)4.5+(*p)++, 5 }; // 13.1
  struct misra_13_1_t c10 = { (int)4.5, (*p)++ }; // 13.1
  struct misra_13_1_t c11 = { .a=4+1, .b=3/3 };
  struct misra_13_1_t c12 = { .a=4, .b=5 };
  struct misra_13_1_t c13 = { (*v)<<=(int)(4.5), .b=5 }; // 13.1
  struct misra_13_1_t c14 = { (*p)/=(int)(4.5) }; // 13.1
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
  if (x || ((y += 19) > 33)){} // 13.5
  if (x || ((y = 25) > 33)){} // 13.5 13.4
  if (x || ((--y) > 33)){} // 13.5
  else {}
}

void misra_13_6() {
  int a = sizeof(x|=42); // 13.6
  a = sizeof(--x); // 13.6 13.3
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

void misra_14_2_init_value(int32_t *var) {
    *var = 0;
}
void misra_14_2_fn1(bool b) {
  for (;i++<10;) {} // 14.2
  for (;i<10;dostuff()) {} // TODO
  int32_t g = 0;
  int g_arr[42];
  g += 2; // no-warning
  for (int32_t i2 = 0; i2 < 8; ++i2) {
    i2 += 2; // 14.2
    i2 |= 2; // 14.2
    g += 2;
    i2 ^= 2; // 14.2
    if (i2 == 2) {
      g += g_arr[i2];
    }
    misra_14_2_init_value(&i2); // TODO: Fix false negative in function call
  }

  for (misra_14_2_init_value(&i); i < 10; ++i) {} // no-warning FIXME: False positive for 14.2 Trac #9491

  bool abort = false;
  for (i = 0; (i < 10) && !abort; ++i) { // no-warning
      if (b) {
        abort = true;
      }
  }
  for (;;) {} // no-warning

  int x = 10;
  for (int i = x; i < 42; i++) {
      x++; // no warning
  }
  for (int i = (x - 3); i < 42; i++) {
      x ^= 3; // no warning
  }

  for (int i = 0, j = 19; i < 42; i++) { // 12.3 14.2
      i += 12; // 14.2
      j /= 3; // TODO: 14.2
  }

  for (int i = 0; i < 19; i++) {
      for (int j = 0; j < 42; j++) {
          i--; // 14.2
          for (int k = j; k > 5; k--) {
              i++; // 14.2
              for (int h = 35; h > 5; k++) // 14.2
              {}
          }
      }
  }
}
static void misra_14_2_fn2()
{
    int y = 0;

    // Handle cases when i is not treated as loop counter according MISRA
    // definition.
    for (int i = 0, j = 19; y < 10, --j > 10; y++, j--) { // 14.2 12.3
        i++; // no warning
    }
    for (int i = 0, j = 19; y < 10, --j > 10; y++, j--) { // 14.2 12.3
        i++; // no warning
    }
    for (int i = 0; y < 10; y++) { // TODO: 14.2
        i++; // no warning
    }
    for (int i = 0; i < 10; y++) { // TODO: 14.2
        i++; // no warning
    }
    for (int i = 0; y < 10; i++) { // TODO: 14.2
        i++; // no warning
    }
    for (int i = 0; i < 10; (y+=i)) {
        i++; // no warning
    }

    // i is a loop counter according MISRA definition
    for (int i = 0; i < 10; i++) {
        i++; // 14.2
        if (++i > 5) { // 14.2
            break;
        }
    }
    for (int i = 0; i < 10; (i+=42)) {
        i++; // 14.2
    }
    for (int i = 0; i < 10; (i|=y)) {
        i++; // 14.2
    }

    return 0;
}

struct {
  unsigned int x:1;
  unsigned int y:1;
} r14_4_struct;
void misra_14_4(bool b) {
  if (x+4){} // 14.4
  else {}

  if (b) {}
  else {}

  if (r14_4_struct.x) {}
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

  switch (x) {
  case 0:
      if (x == y) {
          goto L2; // 15.3 15.1
      }
      goto L2; // 15.3 15.1
    L3:
      foo();
      if (a == 0x42) {
          // Compliant:
          goto L3; // 15.1 15.2
      }
      break;
  case 1:
      y = x;
  L2:
      ++x;
      break;
  default:
      break;
  }
}

void misra_15_4() {
  misra_15_4_label:
    return;

  int x = 0;
  int y = 0;
  int z = 0;

  // Break on different loop scopes
  for (x = 0; x < 42; ++x) {
    if (x==1) {
      break;
    }
    for (y = 0; y < 42; ++y) { // 15.4
      if (y==1) {
        break;
      }
      if (y==2) {
        break;
      }
      for (z = 0; y < 42; ++z) {
        if (z==1) {
          break;
        }
      }
    }
  }

  // Break in while loop
  do { // 15.4
    if(x == 1) {
        break;
    }
    if(x == 2) {
        break
    }
    x++;
  } while(x != 42);

  // Break and goto in same loop
  for (int x = 0; x < 10; ++x) { // 15.4
    if (x == 1) {
        break;
    }
    if (x == 2) {
        goto misra_15_4_label; // 15.1 15.2
    }
  }

  // Inner loop uses goto
  for (x = 0; x < 42; ++x) { // 15.4
    if (x==1) {
      break;
    }
    for (y = 0; y < 42; ++y) {
      if (y == 1) {
        goto misra_15_4_label; // 15.1 15.2
      }
    }
  }

  // Allow switch with multiple breaks inside loop
  for (x = 0; x < 42; ++x) {
    switch (x) {
      case 1:
        break;
      default:
        break;
    }
  }

  // Do not allow switch with multiple gotos inside loop
  for (x = 0; x < 42; ++x) { // 15.4
    switch (x) {
      case 1:
        goto misra_15_4_label; // 15.1 15.2
        break;
      default:
        goto misra_15_4_label; // 15.1 15.2
        break;
    }
  }
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

#if A > 0x42
  if (true) {
    (void)0;
  }
  if (true)
#endif
  { (void)0; } // no-warning

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

  if (a==2) {} else if (b==4) {} // 15.7
  if (a==2) {} else { if (b==4) {} } // no-warning
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

    switch (x) {
    case 1:     // comment 1
    {
        a = 1;
        break;
    }
    case 2:     // comment 2
    {
        a = 2;
        break;
    }
    default:
    {
        break;
    }
  }

  switch (x) {
    case 1:
        break;
    default: // 16.5
        x++;
    case 19: // 16.3
        break;
    case 20:
        x + 2;
        x + 3;
        break;
    }
    switch (x) { // 16.6
    default:;
    } // 16.3

    switch (x) { default:; } // 16.3 16.6

    switch (x) {
    case 20:
        x + 2;
        x + 3;
        break;
    case 21:
        x + 2;
        x + 3;
        break;
    default:
        ;
    } // 16.3

    switch (x) { // 16.4 16.6
    case 1:
        x++;
        break;
    case 2:
        x++;
  } // 16.3
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

void misra_17_2_ok_1(void) { ; }
void misra_17_2_ok_2(void) {
    misra_17_2_ok_1(); // no-warning
}
void misra_17_2_1(void) {
  misra_17_2_ok_1(); // no-warning
  misra_17_2_1(); // 17.2
  misra_17_2_ok_2(); // no-warning
  misra_17_2_1(); // 17.2
}
void misra_17_2_2(void) {
  misra_17_2_3(); // 17.2
}
void misra_17_2_3(void) {
  misra_17_2_4(); // 17.2
}
void misra_17_2_4(void) {
  misra_17_2_2(); // 17.2
  misra_17_2_3(); // 17.2
}

void misra_17_2_5(void) {
  misra_17_2_ok_1(); // no-warning
  misra_17_2_5(); // 17.2
  misra_17_2_1(); // no-warning
}

void misra_17_6(int x[static 20]) {(void)x;} // 17.6

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
  int *ab = &b + 1; // 18.4
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
} r18_7_struct;
struct {
  uint16_t len;
  uint8_t data_1[ 19 ];
  uint8_t data_2[   ]; // 18.7
} r18_7_struct;

typedef enum {
    R18_8_ENUM_CONSTANT_0,
    R18_8_ENUM_CONSTANT_1,
} r18_8_enum;

void misra_18_8(int x) {
  int buf1[10];
  int buf2[sizeof(int)];
  int vla[x]; // 18.8
  static const unsigned char arr18_8_1[] = UNDEFINED_ID;
  static uint32_t enum_test_0[R18_8_ENUM_CONSTANT_0] = {0};
}

union misra_19_2 { }; // 19.2

#include "notfound.h" // 20.1

#define int short // 20.4
#define inline "foo" // no warning in C90 standard
#undef X  // 20.5

#define M_20_7_1(A)  (A+1) // 20.7
#define M_20_7_2(A,B)  (1+AB+2) // no warning
#define M_20_7_3(A)  ((A)+A) // 20.7
#define M_20_7_4(A)  x##A // 20.10 this test was written to see there are not FPs
#define M_20_7_5(A,B)  f(A, B) // no warning
#define M_20_7_6(x) a ## x = ( x ) // 20.10
#define M_20_7_7(x) a  = # x // 20.10
#define M_20_7_8(x, fn) a = fn ( # x ) // 20.7 20.10
#define M_20_7_9(x, fn) a = (fn) ( # x ) // 20.10
#define M_20_7_10(A, B) (A " " B)
#define M_20_7_11(A, B, C) (A " " B " " C)
#define M_20_7_12(A, B, C) (A " " B + C) // 20.7
#define M_20_7_13(A, B, C) (A + B " " C) // 20.7
#define M_20_7_14(STRING1, STRING2) (STRING1 " " STRING2)
#define M_20_7_15(STRING1, STRING2, STRING3) (STRING1 " " STRING2 " " STRING3)
#define M_20_7_16(STRING1, STRING2, STRING3) (STRING1 " " STRING2 + STRING3) // 20.7
#define M_20_7_17(STRING1, STRING2, STRING3) (STRING1 + STRING2 " " STRING3) // 20.7

// Compliant: M is a structure member name, not an expression
struct { int a; } struct_20_7_s;
#define M_20_7_6(M) struct_20_7.M
#define M_20_7_7(M) (struct_20_7).M

#define MUL(a  ,b ) ( a * b ) // 20.7

#define M_20_10(a) (#a) // 20.10

#else1 // 20.13

#ifdef A>1
# define somethingis 5 // no warning
# define func_20_13(v) (v) // no warning
#else
# definesomethingis 6 // 20.13
# def fun_2013(v) () // 20.13
#endif

#define _Incompatible 0xdeadbeef // 21.1
#define __Incompatible 0xdeadbeef // 21.1
#define __starts_with_lower 0xdeadbeef // 21.1
#define __MY_HEADER_ // 21.1
#define _macro_starts_with_lower 1 // no warning
static int _file_scope_id_21_1 = 42; // no warning
static int _file_scope_id_21_1_fn() { return 42; } // no warning
int misra_21_1() {
    int _a = 42; // no warning: only directives affected
    errno = EINVAL; // no warning
    _a ++; // no warning
    _exit(1); // no warning
    return _a; // no warning
}
static int _misra_21_1_2(); // no warning
#define errno 11 // 21.1

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

void misra_21_12() {
    int rc;
    fexcept_t f; // 21.12
    rc = feclearexcept(1); // 21.12
    rc = fegetexceptflag(&f, 1); // 21.12
    rc = feraiseexcept(1); // 21.12
    rc = fesetexceptflag(&f, 1); // 21.12
    rc = fetestexcept(1); // 21.12
}

// Large arrays for R13.1. Size exceeds default Python's max recursion depth.
static uint8_t misra_13_1_large_ok[1024] = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static uint8_t misra_13_1_large_bad[1024] = { // 13.1
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, i++, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
