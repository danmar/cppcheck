/*
 * Define _TIME_BITS equal to 64 so that glibc knows we want Y2038 support.
 */

#define _TIME_BITS 64

#include "y2038-inc.h"

int main(int argc, char **argv)
{
    clockid_t my_clk_id = CLOCK_REALTIME;
    struct timespec *my_tp;

    return clock_gettime(my_clk_id, &my_tp);
}
