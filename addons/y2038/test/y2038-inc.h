#ifndef __INC2038
#define _INC2038

/*
 * This file defines _USE_TIME_BITS64.
 * It plays the role of a Y2038-proof glibc.
 */

#define _USE_TIME_BITS64

/*
 * Declare just enough for clock_gettime
 */

typedef int clockid_t;

typedef int __time_t;

typedef long int __syscall_slong_t;

struct timespec
{
    __time_t tv_sec;		/* Seconds.  */
    __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};

extern int clock_gettime(clockid_t clk_id, struct timespec *tp);

#define CLOCK_REALTIME 0

#endif /* INC2038 */
