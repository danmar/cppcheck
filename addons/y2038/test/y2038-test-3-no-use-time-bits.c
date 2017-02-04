#include <stdio.h>
#include <fcntl.h>

/*
 * Include bad _USE_TIME_BITS64 definition to trigger error
 */

#define _TIME_BITS 64

int main(int argc, char **argv)
{
    clockid_t my_clk_id = CLOCK_REALTIME;
    struct timespec *my_tp;

    return clock_gettime(my_clk_id, &my_tp);
}
