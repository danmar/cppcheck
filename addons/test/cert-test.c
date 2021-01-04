// To test:
// ~/cppcheck/cppcheck --dump cert-test.c && python ../cert.py -verify cert-test.c.dump

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct S {
    short a;
    short b;
};

#pragma pack()
struct PackedStruct {
    short a;
    short b;
};

void api01()
{
    const size_t String_Size = 20;
    struct bad_node_s
    {
        char name[String_Size];
        struct bad_node_s* next;      // cert-API01-C
    };
    struct good_node_s
    {
        struct good_node_s* next;
        char name[String_Size];
    };
    struct also_good_node_s
    {
        struct also_good_node_s* next;
        char *name;
    };
}

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

void exp46(int x, int y, int z)
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

void env33()
{
    system("chmod -x $(which chmod)"); // cert-ENV33-C
    system(""); // cert-ENV33-C
    system(NULL); // no-warning
    system(0); // no-warning
    const int *np = NULL;
    system(np); // no-warning
    int system;
}

void msc24()
{
    struct S {
    int x; int fopen;
    };

    struct S s;
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[256];
    int i;
    long int li;
    long long int lli;
    FILE *f;

    s.fopen = 123;

    f = fopen ("myfile.txt","w+");  //cert-MSC24-C
    setbuf ( f , buffer );   //cert-MSC24-C
    for ( i='A' ; i<='Z' ; i++)
        fputc ( i, f);
    rewind (f);             //cert-MSC24-C
    fclose (f);

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "The current date/time is: %s", asctime (timeinfo) ); //cert-MSC24-C

    float n = atof (buffer);              //cert-MSC24-C
    float m = sin (n*M_PI/180);

    i = atoi (buffer);      //cert-MSC24-C

    li = atol(buffer);      //cert-MSC24-C

    lli = atoll(buffer);    //cert-MSC24-C

    time (&rawtime);
    printf ("The current local time is: %s", ctime (&rawtime)); //cert-MSC24-C

    freopen ("myfile.txt","w",stdout);                  //cert-MSC24-C
    printf ("This sentence is redirected to a file.");
    fclose (stdout);
}

void msc30()
{
    unsigned int num = rand(); // cert-MSC30-c
    int rand = 5;
    int a = rand;
}

void exp15()
{
    int x=5, y=7;

    if(x==y);                            //cert-EXP15-C
    {
        printf("not working\n");
    }
    if(x)
        ;
}

void str03()
{
    char *string_data=(char*)malloc(16);
    char a[16];
    int d;
    strncpy(a, string_data, sizeof(a));     //cert-STR03-C
    strncpy(a, string_data, 5); d=sizeof(int);
}

void str05()
{
    int x=5, y=7;

    if(x==y);                            //cert-EXP15-C
    {
        printf("not working\n");
    }
    if(x)
        ;
}

void str07(char *buf, const char *newBuf)
{
    const char *str = "test";
    strcat(buf,"bla");
    strcat(buf, str);    //cert-STR07-C
    strcat(buf, newBuf); //cert-STR07-C
    strcpy(buf, newBuf); //cert-STR07-C
}

void str11()
{
    const char str[3]="abc";    //cert-STR11-C
    const char *x[10]; x[3]="def";
}

