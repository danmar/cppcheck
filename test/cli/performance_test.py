
# python -m pytest test-other.py

import os
import sys
import pytest

from testutils import cppcheck, assert_cppcheck



@pytest.mark.timeout(10)
def test_slow_array_many_floats(tmpdir):
    # 11649
    # cppcheck valueflow takes a long time when an array has many floats
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const float f[] = {\n")
        for i in range(20000):
            f.write('    13.6f,\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_array_many_strings(tmpdir):
    # 11901
    # cppcheck valueflow takes a long time when analyzing a file with many strings
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const char *strings[] = {\n")
        for i in range(20000):
            f.write('    "abc",\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_long_line(tmpdir):
    # simplecpp #314
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("#define A() static const int a[] = {\\\n")
        for i in range(5000):
            f.write(" -123, 456, -789,\\\n")
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(60)
def test_slow_large_constant_expression(tmpdir):
    # 12182
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("""
#define FLAG1 0
#define FLAG2 0
#define FLAG3 0
#define FLAG4 0
#define FLAG5 0
#define FLAG6 0
#define FLAG7 0
#define FLAG8 0
#define FLAG9 0
#define FLAG10 0
#define FLAG11 0
#define FLAG12 0
#define FLAG13 0
#define FLAG14 0
#define FLAG15 0
#define FLAG16 0
#define FLAG17 0
#define FLAG18 0
#define FLAG19 0
#define FLAG20 0
#define FLAG21 0
#define FLAG22 0
#define FLAG23 0
#define FLAG24 0

#define maxval(x, y) ((x) > (y) ? (x) : (y))

#define E_SAMPLE_SIZE   maxval( FLAG1,                \
                                  maxval( FLAG2,      \
                                  maxval( FLAG3,      \
                                  maxval( FLAG4,      \
                                  maxval( FLAG5,      \
                                  maxval( FLAG6,      \
                                  maxval( FLAG7,      \
                                  maxval( FLAG8,      \
                                  maxval( FLAG9,      \
                                  maxval( FLAG10,     \
                                  maxval( FLAG11,     \
                                  maxval( FLAG12,     \
                                  maxval( FLAG13,     \
                                  maxval( FLAG14,     \
                                  FLAG15 ))))))))))))))

#define SAMPLE_SIZE       maxval( E_SAMPLE_SIZE,      \
                                  maxval( sizeof(st), \
                                  maxval( FLAG16,     \
                                  maxval( FLAG17,     \
                                  maxval( FLAG18,     \
                                  maxval( FLAG19,     \
                                  maxval( FLAG20,     \
                                  maxval( FLAG21,     \
                                  maxval( FLAG22,     \
                                  maxval( FLAG23,     \
                                          FLAG24 ))))))))))

typedef struct {
    int n;
} st;

x = SAMPLE_SIZE;
        """)

    cppcheck([filename])

@pytest.mark.timeout(10)
def test_slow_exprid(tmpdir):
    # 11885
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("""
int foo(int a, int b)
{
#define A0(a, b)  ((a) + (b))
#define A1(a, b)  ((a) > (b)) ? A0((a) - (b), (b)) : A0((b) - (a), (a))
#define A2(a, b)  ((a) > (b)) ? A1((a) - (b), (b)) : A1((b) - (a), (a))
#define A3(a, b)  ((a) > (b)) ? A2((a) - (b), (b)) : A2((b) - (a), (a))
#define A4(a, b)  ((a) > (b)) ? A3((a) - (b), (b)) : A3((b) - (a), (a))
#define A5(a, b)  ((a) > (b)) ? A4((a) - (b), (b)) : A4((b) - (a), (a))
#define A6(a, b)  ((a) > (b)) ? A5((a) - (b), (b)) : A5((b) - (a), (a))
#define A7(a, b)  ((a) > (b)) ? A6((a) - (b), (b)) : A6((b) - (a), (a))
#define A8(a, b)  ((a) > (b)) ? A7((a) - (b), (b)) : A7((b) - (a), (a))
#define A9(a, b)  ((a) > (b)) ? A8((a) - (b), (b)) : A8((b) - (a), (a))
#define A10(a, b) ((a) > (b)) ? A9((a) - (b), (b)) : A9((b) - (a), (a))
#define A11(a, b) ((a) > (b)) ? A10((a) - (b), (b)) : A10((b) - (a), (a))
	return A8(a, b);
}
        """)

    my_env = os.environ.copy()
    my_env["DISABLE_VALUEFLOW"] = "1"
    cppcheck([filename], env=my_env)


@pytest.mark.timeout(10)
def test_slow_initlist_varchanged(tmpdir):
    # #12235
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                struct T {
                    int* q;
                    int nx, ny;
                };
                struct S {
                    void f();
                    int n;
                    T* p;
                };
                #define ROW 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 ,
                #define ROW4 ROW ROW ROW ROW
                #define ROW16 ROW4 ROW4 ROW4 ROW4
                #define ROW64 ROW16 ROW16 ROW16 ROW16
                #define ROW256 ROW64 ROW64 ROW64 ROW64
                #define ROW1K ROW256 ROW256 ROW256 ROW256
                #define ROW4K ROW1K ROW1K ROW1K ROW1K
                const int A[] = {
                    ROW4K
                };
                void S::f() {
                    for (int i = 0; i < n; ++i) {
                        T& t = p[i];
                        for (int y = 0; y < t.ny; y += 4) {
                            int* row0 = t.q + y * t.nx;
                            for (int x = 0; x < t.nx; x += 4) {
                                int s[16] = {};
                                memcpy(row0, &s[0], 4);
                                row0 += 4;
                            }
                        }
                    }
                }""")
    cppcheck([filename]) # should not take more than ~1 second
