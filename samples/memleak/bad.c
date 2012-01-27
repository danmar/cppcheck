#include <stdlib.h>
int main()
{
    int result;
    char *a = malloc(10);
    a[0] = 0;
    result = a[0];
    return result;
}
