#!/usr/bin/env python3
#
# cppcheck addon for Y2038 safeness detection
#
# Detects:
#
# 1. _TIME_BITS being defined to something else than 64 bits
# 2. _USE_TIME_BITS64 being defined when _TIME_BITS is not
# 3. Any Y2038-unsafe symbol when _USE_TIME_BITS64 is not defined.
#
# Example usage:
# $ cppcheck --addon=y2038 path-to-src/test.c
#

from __future__ import print_function

import cppcheckdata
import sys
import re


# --------------------------------------------
# #define/#undef detection regular expressions
# --------------------------------------------

# test for '#define _TIME_BITS 64'
re_define_time_bits_64 = re.compile(r'^\s*#\s*define\s+_TIME_BITS\s+64\s*$')

# test for '#define _TIME_BITS ...' (combine w/ above to test for 'not 64')
re_define_time_bits = re.compile(r'^\s*#\s*define\s+_TIME_BITS\s+.*$')

# test for '#undef _TIME_BITS' (if it ever happens)
re_undef_time_bits = re.compile(r'^\s*#\s*undef\s+_TIME_BITS\s*$')

# test for '#define _USE_TIME_BITS64'
re_define_use_time_bits64 = re.compile(r'^\s*#\s*define\s+_USE_TIME_BITS64\s*$')

# test for '#undef _USE_TIME_BITS64' (if it ever happens)
re_undef_use_time_bits64 = re.compile(r'^\s*#\s*undef\s+_USE_TIME_BITS64\s*$')

# --------------------------------
# List of Y2038-unsafe identifiers
# --------------------------------

# This is WIP. Eventually it should contain all identifiers (types
# and functions) which would be affected by the Y2038 bug.

id_Y2038 = {
    # Y2038-unsafe types by definition
    'time_t'
    # Types using Y2038-unsafe types
    'lastlog',
    'msqid_ds',
    'semid_ds',
    'timeb',
    'timespec',
    'timeval',
    'utimbuf',
    'itimerspec',
    'clnt_ops',
    'elf_prstatus',
    'itimerval',
    'ntptimeval',
    'rusage',
    'timex',
    'utmp',
    'utmpx',
    # APIs using 2038-unsafe types
    'ctime',
    'ctime_r',
    'difftime',
    'gmtime',
    'gmtime_r',
    'localtime',
    'localtime_r',
    'mktime',
    'stime',
    'timegm',
    'timelocal',
    'time',
    'msgctl',
    'ftime',
    'aio_suspend',
    'clock_getres',
    'clock_gettime',
    'clock_nanosleep',
    'clock_settime',
    'futimens',
    'mq_timedreceive',
    'mq_timedsend',
    'nanosleep',
    'pselect',
    'pthread_cond_timedwait',
    'pthread_mutex_timedlock',
    'pthread_rwlock_timedrdlock',
    'pthread_rwlock_timedwrlock',
    'sched_rr_get_interval',
    'sem_timedwait',
    'sigtimedwait',
    'timespec_get',
    'utimensat',
    'adjtime',
    'pmap_rmtcall',
    'clntudp_bufcreate',
    'clntudp_create',
    'futimes',
    'gettimeofday',
    'lutimes',
    'select',
    'settimeofday',
    'utimes',
    'utime',
    'timerfd_gettime',
    'timerfd_settime',
    'timer_gettime',
    'timer_settime',
    'fstatat',
    'fstat',
    '__fxstatat',
    '__fxstat',
    'lstat',
    '__lxstat',
    'stat',
    '__xstat',
    'struct itimerval',
    'setitimer',
    'getitimer',
    'ntp_gettime',
    'getrusage',
    'wait3',
    'wait4',
    'adjtimex',
    'ntp_adjtime',
    'getutent_r',
    'getutent',
    'getutid_r',
    'getutid',
    'getutline_r',
    'getutline',
    'login',
    'pututline',
    'updwtmp',
    'getutxent',
    'getutxid',
    'getutxline',
    'pututxline'
}


def check_y2038_safe(dumpfile, quiet=False):
    # Assume that the code is Y2038 safe until proven otherwise
    y2038safe = True
    # load XML from .dump file
    data = cppcheckdata.CppcheckData(dumpfile)

    srcfile = data.files[0]
    for cfg in data.iterconfigurations():
        if not quiet:
            print('Checking %s, config %s...' % (srcfile, cfg.name))
        safe_ranges = []
        safe = -1
        time_bits_defined = False
        srclinenr = 0

        for directive in cfg.directives:
            # track source line number
            if directive.file == srcfile:
                srclinenr = directive.linenr
            # check for correct _TIME_BITS if present
            if re_define_time_bits_64.match(directive.str):
                time_bits_defined = True
            elif re_define_time_bits.match(directive.str):
                cppcheckdata.reportError(directive, 'error',
                                         '_TIME_BITS must be defined equal to 64',
                                         'y2038',
                                         'type-bits-not-64')
                time_bits_defined = False
                y2038safe = False
            elif re_undef_time_bits.match(directive.str):
                time_bits_defined = False
            # check for _USE_TIME_BITS64 (un)definition
            if re_define_use_time_bits64.match(directive.str):
                safe = int(srclinenr)
                # warn about _TIME_BITS not being defined
                if not time_bits_defined:
                    cppcheckdata.reportError(directive, 'warning',
                                             '_USE_TIME_BITS64 is defined but _TIME_BITS was not',
                                             'y2038',
                                             'type-bits-undef')
            elif re_undef_use_time_bits64.match(directive.str):
                unsafe = int(srclinenr)
                # do we have a safe..unsafe area?
                if unsafe > safe > 0:
                    safe_ranges.append((safe, unsafe))
                    safe = -1

        # check end of source beyond last directive
        if len(cfg.tokenlist) > 0:
            unsafe = int(cfg.tokenlist[-1].linenr)
            if unsafe > safe > 0:
                safe_ranges.append((safe, unsafe))

        # go through all tokens
        for token in cfg.tokenlist:
            if token.str in id_Y2038:
                if not any(lower <= int(token.linenr) <= upper
                           for (lower, upper) in safe_ranges):
                    cppcheckdata.reportError(token, 'warning',
                                             token.str + ' is Y2038-unsafe',
                                             'y2038',
                                             'unsafe-call')
                    y2038safe = False
            token = token.next

    return y2038safe


def get_args_parser():
    parser = cppcheckdata.ArgumentParser()
    return parser


if __name__ == '__main__':
    parser = get_args_parser()
    args = parser.parse_args()

    exit_code = 0
    quiet = args.quiet or args.cli

    if not args.dumpfile:
        if not args.quiet:
            print("no input files.")
        sys.exit(0)

    for dumpfile in args.dumpfile:
        if not quiet:
            print('Checking ' + dumpfile + '...')

        check_y2038_safe(dumpfile, quiet)

    sys.exit(cppcheckdata.EXIT_CODE)
