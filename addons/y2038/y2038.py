#! /usr/bin/python
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
# $ cppcheck --dump path-to-src/
# $ y2038.py path-to-src/
#
# y2038.py will walk the source tree for .dump files.

import cppcheckdata
import sys
import os
import re
import argparse

#---------------
# Error handling
#---------------

diagnostics = {}


def reportDiagnostic(template, configuration, file, line, severity, message):
    # collect diagnostics by configuration
    if not configuration in diagnostics:
        diagnostics[configuration] = []
    # add error to this configuration
    diagnostics[configuration].append(
        cppcheckdata.reportError(template, [[file, line]], severity, message))


def printDiagnostics():
    for cfg in diagnostics:
        sys.stderr.write('# Configuration "' + cfg + '":\n')
        for diag in diagnostics[cfg]:
            sys.stderr.write(diag + '\n')


def reportDirDiag(template, cfg, filename, linenr, directive, severity, msg):
    reportDiagnostic(template, cfg.name,
                     directive.file, directive.linenr,
                     severity, msg)
    if (filename != directive.file) or (linenr != directive.linenr):
        reportDiagnostic(template, cfg.name,
                         filename, linenr, 'information',
                         directive.file + ' was included from here')


def reportTokDiag(template, cfg, token, severity, msg):
    reportDiagnostic(template, cfg.name,
                     token.file, token.linenr,
                     severity, msg)

#---------------------------------------------
# #define/#undef detection regular expressions
#---------------------------------------------

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

#---------------------------------
# List of Y2038-unsafe identifiers
#---------------------------------

# This is WIP. Eventually it should contain all identifiers (types
# and functions) which would be affected by the Y2038 bug.

id_Y2038 = [
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
    'stat',
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
]

# return all files ending in .dump among or under the given paths

def find_dump_files(paths):
    dumpfiles = []
    for path in paths:
        if path.endswith('.dump'):
            if not path in dumpfiles:
                dumpfiles.append(path)
        else:
            for (top, subdirs, files) in os.walk(path):
                for file in files:
                    if file.endswith('.dump'):
                        f = top + '/' + file
                        if not f in dumpfiles:
                            dumpfiles.append(f)
    dumpfiles.sort()
    return dumpfiles

#------------------
# Let's get to work
#------------------

# extend cppcheck parser with our own options

parser = cppcheckdata.ArgumentParser()
parser.add_argument('-q', '--quiet', action='store_true',
                    help='do not print "Checking ..." lines')
parser.add_argument('paths', nargs='+', metavar='path',
                    help='path to dump file or directory')

# parse command line

args = parser.parse_args()

# now operate on each file in turn

dumpfiles = find_dump_files(args.paths)

for dumpfile in dumpfiles:
    if not args.quiet:
        print('Checking ' + dumpfile + '...')
    srcfile = dumpfile.rstrip('.dump')
    # at the start of the check, we don't know if code is Y2038 safe
    y2038safe = False
    # load XML from .dump file
    data = cppcheckdata.parsedump(dumpfile)
    # go through each configuration
    for cfg in data.configurations:
        if not args.quiet:
            print('Checking ' + dumpfile + ', config "' + cfg.name + '"...')
        safe_ranges = []
        safe = -1
        time_bits_defined = False
        for directive in cfg.directives:
            # track source line number
            if directive.file == srcfile:
                srclinenr = directive.linenr
            # check for correct _TIME_BITS if present
            if re_define_time_bits_64.match(directive.str):
                time_bits_defined = True
            elif re_define_time_bits.match(directive.str):
                reportDirDiag(args.template, cfg, srcfile, srclinenr,
                              directive, 'error',
                              '_TIME_BITS must be defined equal to 64')
                time_bits_defined = False
            elif re_undef_time_bits.match(directive.str):
                time_bits_defined = False
            # check for _USE_TIME_BITS64 (un)definition
            if re_define_use_time_bits64.match(directive.str):
                safe = int(srclinenr)
                # warn about _TIME_BITS not being defined
                if time_bits_defined == False:
                    reportDirDiag(args.template,
                                  cfg, srcfile, srclinenr, directive, 'warning',
                                  '_USE_TIME_BITS64 is defined but _TIME_BITS was not')
            elif re_undef_use_time_bits64.match(directive.str):
                unsafe = int(srclinenr)
                # do we have a safe..unsafe area?
                if safe > 0 and unsafe > safe:
                    safe_ranges.append((safe, unsafe))
                    safe = -1
        # check end of source beyond last directive
        if len(cfg.tokenlist) > 0:
            unsafe = int(cfg.tokenlist[-1].linenr)
            if safe > 0 and unsafe > safe:
                safe_ranges.append((safe, unsafe))
        # go through all tokens
        for token in cfg.tokenlist:
            if token.str in id_Y2038:
                if not any(lower <= int(token.linenr) <= upper
                           for (lower, upper) in safe_ranges):
                    reportTokDiag(args.template, cfg, token, 'warning',
                                  token.str + ' is Y2038-unsafe')
            token = token.next

printDiagnostics()
