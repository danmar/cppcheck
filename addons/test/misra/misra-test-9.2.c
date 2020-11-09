// #include <stdio.h>
#include <math.h>
#include <stdio.h>

 //Braces shall be used to indicate and match the structure in the non-zero initialization of arrays and structures.




int main() {
    typedef struct {
        int i1;
        int i2;
    } struct1;

    typedef struct {
        char c1;
        struct1 is1;
        char c2[4];
    } struct2;

    // Noncompliant Code Example
    int n0[2][2]    = { { } };                                               // 9.2
    char ss0[12]    = { "Hello world" };                                     // 9.2
    char ss1[2][20] = "Hello world";                                         // 9.2

    int b1[3][2]    = { 1, 2, 3, 4, 5, 6 };                                  // 9.2
    int b2[6]       = { { 1, 2 }, { 3, 4 }, { 5, 6 } };                      // 9.2
    int b3[1][2][3] = { { 1, 2 }, { 3, 4 }, { 5, 6 } };                      // 9.2

    int b4[1]       = { [0][1] = 1 };                                        // 9.2
    int b5[1]       = { [0] = { 1, 2 } };                                    // 9.2
    int b6[1][2]    = { [0] = 1 };                                           // 9.2
    int b7[1][2][3] = { [0][1] = 1 };                                        // 9.2
    int b8[2][2]    = { { 1, 2 }, [1][0] = {3, 4} };                         // 9.2

    // Compliant Solution
    int c1[2][2]    = { };
    char s0[2][12]   = { "Hello world" };
    char s1[]        = "Hello world";

    int a1[3][2]  = { { 1, 2 }, { 3, 4 }, { 5, 6 } };
    int a2[6]     = { 1, 2, 3, 4, 5, 6 };
    int a3[1][2][3] = { { { 1, 2, 3 }, { 4, 5, 6 } } };

    int a4[2]       = { 2, [0] = 1, 2 };
    int a5[1][2]    = { [0] = { 1, 2 } };
    int a6[2][2]    = { { 1, 2 }, [1] = {3, 4} };
    int a7[1][2][3] = { [0][1][2] = 1 };
    int a8[2][2]    = { { 1, 2 }, [1][0] = 3, 4};

    int a10[2][2]  = { { 1, 2 }, { 0 } };
    int a11[5]     = { 0 };
    int a12[2][2]  = { };

    // Non
    struct1 n_structarr1[2] = { 1, 2, 3, 4 };       // 9.2
    struct1 c_structarr1[2] = { {1, 2}, {3, 4} };

    struct1 n_struct1_2 = 1;                        // 9.2
    struct2 c_struct2 = { 1, {2, 3}, {0} };
    struct2 c_struct3 = { };
    struct2 c_struct4 = { 0 };

    struct1 str1 = { .i2 = 2, .i1 = 1 };
    struct2 str2 = { .is1 = {2, 3}, { 4 } };
    struct2 str2 = { .i1 = {2, 3}, { 4 } };

    union u1 {
        char c;
        struct1 i;
    } u = { 3 };

}
