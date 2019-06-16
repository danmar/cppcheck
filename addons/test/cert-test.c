// To test:
// ~/cppcheck/cppcheck --dump cert-test.c && python ../cert.py -verify cert-test.c.dump

struct S {
    short a;
    short b;
};

#pragma pack()
struct PackedStruct {
    short a;
    short b;
};

void dostuff(int *data);

void exp05()
{
    const int x = 42;
    int y = (int)x;

    int *p;
    p = (int *)&x; // cert-EXP05-C

    const int data[] = {1,2,3,4};
    dostuff(data); // cert-EXP05-C
}

void print(const char *p);
void exp05_fp() {
    print("hello");
}

void exp42()
{
    struct S s1 = {1,2};
    struct S s2 = {1,2};
    memcmp(&s1, &s2, sizeof(struct S)); // cert-EXP42-C

    struct PackedStruct s3 = {1,2};
    struct PackedStruct s4 = {1,2};
    memcmp(&s3, &s4, sizeof(struct S));
}

void exp46()
{
    if ((x == y) & z) {} // cert-EXP46-c
}

unsigned char int31(int x)
{
    x = (unsigned char)1000; // cert-INT31-c
    x = (signed char)0xff; // cert-INT31-c
    x = (unsigned char)-1; // cert-INT31-c
    x = (unsigned long long)-1; // cert-INT31-c
}

void msc30()
{
    unsigned int num = rand(); // cert-MSC30-c
    int rand = 5;
    int a = rand;
}
void str07()
{
    static const *str="test";
    char buf[128];
    strcpy(buf, str); //cert-STR07-C
    strcat(buf,"doh"); //cert-STR07-C
    fputs(buf, stderr); //cert-STR07-C
}

