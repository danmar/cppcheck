#include <stdio.h>
#include <fcntl.h>

/*
 * Define _TIME_BITS unequal to 64 to trigger error
 */

#define _TIME_BITS 62

#include "y2038-inc.h"

int main(int argc, char **argv)
{
    clockid_t my_clk_id = CLOCK_REALTIME;
    struct timespec *my_tp;

    return clock_gettime(my_clk_id, &my_tp);
}
