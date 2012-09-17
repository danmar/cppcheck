#include <stdio.h>
int main()
{
    FILE *a = fopen("good.c", "r");
    if (!a)
        return 0;
    fclose(a);
    return 0;
}
