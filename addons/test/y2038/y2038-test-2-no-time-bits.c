#include <stdio.h>
#include <fcntl.h>

/*
 * Do not define _TIME_BITS but have _USE_TIME_BITS64 defined
 */

#include "y2038-inc.h"

int main(int argc, char **argv)
{
    clockid_t my_clk_id = CLOCK_REALTIME;
    struct timespec *my_tp;

    return clock_gettime(my_clk_id, &my_tp);
}
