/*
 * Test case for Y2038 addon build system integration
 *
 * This file tests build system integration scenarios:
 * - Build system flags take precedence over source directives
 * - Proper build system Y2038 configuration detection
 * - Fallback to source code analysis when no build system is present
 *
 * The same source code is used for different scenarios - differentiation happens
 * through the presence/absence of compile_commands.json and its contents.
 */

#include <time.h>

int main(int argc, char **argv)
{
    time_t current_time;
    struct timespec ts;

    current_time = time(NULL);
    clock_gettime(CLOCK_REALTIME, &ts);

    return 0;
}

