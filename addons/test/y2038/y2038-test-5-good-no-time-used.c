/*
 * C file that does not use any time functionality -> no errors should
 * be reported.
 */

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc > 1) {
        printf("Hello");
    }
    return 0;
}
