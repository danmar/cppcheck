// To test:
// ~/cppcheck/cppcheck --dump cert-test.c && python ../cert.py -verify cert-test.c.dump
#include <time.h>
#include <stdlib.h>

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

void str05()
{
    char *str1 = "abc";         //cert-STR05-C
    wchar_t *str2  = L"hello";  //cert-STR05-C
}

void msc24()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[256];
    int i;
    long int li;
    long long int lli;
    FILE *f;

    f = fopen ("myfile.txt","w+");  //cert-MSC24-C
    setbuf ( f , buffer )   //cert-MSC24-C
    for ( i='A' ; i<='Z' ; i++)
        fputc ( n, f);
    rewind (f);             //cert-MSC24-C
    fread (buffer,1,26,f);
    fclose (f);

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "The current date/time is: %s", asctime (timeinfo) ); //cert-MSC24-C

    strcpy(buffer, "2.5");          //cert-STR07-C
    n = atof (buffer);              //cert-MSC24-C
    m = sin (n*pi/180);

    strcpy(buffer,"42");    //cert-STR07-C
    i = atoi (buffer);      //cert-MSC24-C

    strcpy(buffer,"424242424242");  //cert-STR07-C
    li = atol(buffer);      //cert-MSC24-C

    lli = atoll(buffer);    //cert-MSC24-C

    time (&rawtime);
    printf ("The current local time is: %s", ctime (&rawtime)); //cert-MSC24-C

    freopen ("myfile.txt","w",stdout);                  //cert-MSC24-C
    printf ("This sentence is redirected to a file.");
    fclose (stdout);
}

