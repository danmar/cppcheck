#!/usr/bin/env python3
#
# cppcheck addon for Y2038 safeness detection
#
# This addon provides comprehensive Y2038 (Year 2038 Problem) detection for C/C++ code.
# It automatically detects build system configurations and compiler flags to determine
# Y2038 safety, suppressing warnings when proper configuration is detected.
#
# Key Features:
# - Automatic build system detection (compile_commands.json, Makefile, CMake, Meson, etc.)
# - Compiler flag parsing and validation from build system configuration
# - Warning suppression when proper Y2038 configuration is found
# - Support for both _TIME_BITS=64 and _FILE_OFFSET_BITS=64 requirements
# - Priority-based flag resolution (build system > source code directives)
#
# Detects:
#
# 1. _TIME_BITS being defined to something else than 64 bits
# 2. _FILE_OFFSET_BITS being defined to something else than 64 bits
# 3. _USE_TIME_BITS64 being defined when _TIME_BITS is not
# 4. Any Y2038-unsafe symbol when proper Y2038 configuration is not present
# 5. Build system configurations that affect Y2038 safety
#
# Warning Suppression:
# When both _TIME_BITS=64 AND _FILE_OFFSET_BITS=64 are detected (prioritizing build system
# configuration over source code directives), Y2038 warnings are suppressed and an
# informational message is displayed instead.
#
# Example usage:
# $ cppcheck --addon=y2038 path-to-src/test.c
# $ cppcheck --dump file.c && python3 y2038.py file.c.dump
#

from __future__ import print_function

import cppcheckdata
import sys
import re
import os
import json
from pathlib import Path

# Import build system integration
try:
    from . import y2038_buildsystem
except ImportError:
    # Handle both package import and standalone execution
    try:
        import y2038_buildsystem
    except ImportError:
        # If import fails, build system integration will be disabled
        y2038_buildsystem = None

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

# --------------------------------------------
# Compiler flag parsing regular expressions
# --------------------------------------------

# test for '_TIME_BITS=64' in compiler flags
re_flag_time_bits_64 = re.compile(r'_TIME_BITS=64(?:\s|$|;)')

# test for '_TIME_BITS=...' in compiler flags (combine w/ above to test for 'not 64')
re_flag_time_bits = re.compile(r'_TIME_BITS=(\d+)')

# test for '_USE_TIME_BITS64' in compiler flags
re_flag_use_time_bits64 = re.compile(r'_USE_TIME_BITS64(?:\s|$|=|;)')

# test for '_FILE_OFFSET_BITS=64' in compiler flags
re_flag_file_offset_bits_64 = re.compile(r'_FILE_OFFSET_BITS=64(?:\s|$|;)')

# test for '_FILE_OFFSET_BITS=...' in compiler flags
re_flag_file_offset_bits = re.compile(r'_FILE_OFFSET_BITS=(\d+)')

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


def detect_build_system(project_dir):
    """
    Detects the build system by looking for characteristic files.
    The order of checks determines the precedence.
    """
    project_path = Path(project_dir)
    if (project_path / "CMakeLists.txt").exists():
        return "cmake"
    if (project_path / "meson.build").exists():
        return "meson"
    if (project_path / "configure").exists():
        return "autotools"
    if (project_path / "Makefile").exists():
        return "make"
    if (project_path / "BUILD.bazel").exists() or (project_path / "BUILD").exists():
        return "bazel"
    if (project_path / "Cargo.toml").exists():
        return "cargo"
    return None


def ensure_compile_commands(source_file_path):
    """
    Ensure compile_commands.json exists for the project containing the source file.
    
    This function checks if the source file is part of a project with a supported
    build system. If a build system is detected but no compile_commands.json exists,
    it will attempt to generate one using the appropriate build system.
    
    Args:
        source_file_path (str): Path to the source file being analyzed
        
    Returns:
        bool: True if compile_commands.json exists or was successfully generated, False otherwise
    """
    if y2038_buildsystem is None:
        # Build system integration not available
        return False
        
    # Find the project root by looking for build system files
    current_path = Path(source_file_path).parent.absolute()
    project_root = None
    
    while current_path != current_path.parent:  # Stop at filesystem root
        if y2038_buildsystem.detect_build_system(current_path):
            project_root = current_path
            break
        current_path = current_path.parent
    
    if project_root is None:
        # No build system detected
        return False
        
    # Check if compile_commands.json already exists
    compile_commands_path = project_root / "compile_commands.json"
    if compile_commands_path.exists():
        return True
        
    # Try to generate compile_commands.json
    print(f"Build system detected in {project_root}, generating compile_commands.json...")
    return y2038_buildsystem.generate_compile_commands(project_root)


def parse_compile_commands(source_file_path):
    """
    Parse compile_commands.json to extract Y2038-related compiler flags for a specific source file.
    
    This function looks for compile_commands.json in the current directory and parent directories,
    then extracts Y2038-related compiler flags for the given source file.
    
    Args:
        source_file_path (str): Path to the source file being analyzed
        
    Returns:
        dict: Dictionary containing Y2038-related flag information similar to parse_compiler_flags()
              Returns empty result if no compile_commands.json found or no matching entry
    """
    result = {
        'time_bits_defined': False,
        'time_bits_value': None,
        'use_time_bits64_defined': False,
        'file_offset_bits_defined': False,
        'file_offset_bits_value': None
    }
    
    # Look for compile_commands.json in current and parent directories
    current_path = Path(source_file_path).parent.absolute()
    while current_path != current_path.parent:  # Stop at filesystem root
        compile_commands_path = current_path / "compile_commands.json"
        if compile_commands_path.exists():
            break
        current_path = current_path.parent
    else:
        # Also check current working directory
        compile_commands_path = Path.cwd() / "compile_commands.json"
        if not compile_commands_path.exists():
            return result
    
    try:
        with open(compile_commands_path, 'r') as f:
            compile_commands = json.load(f)
    except (json.JSONDecodeError, IOError):
        return result
    
    # Find the entry for our source file
    source_path = Path(source_file_path).absolute()
    for entry in compile_commands:
        entry_file = Path(entry.get('file', '')).absolute()
        if entry_file == source_path:
            command = entry.get('command', '')
            return parse_compiler_flags(command)
    
    return result


def parse_compiler_flags(config_name):
    """
    Parse compiler flags from configuration name to check for Y2038-related definitions.

    This function analyzes the cppcheck configuration name (which contains compiler flags
    passed via -D options) to extract Y2038-related preprocessor definitions. It looks for
    _TIME_BITS, _USE_TIME_BITS64, and _FILE_OFFSET_BITS definitions and validates their values.

    Args:
        config_name (str): The cppcheck configuration name containing compiler flags
                          (e.g., "_TIME_BITS=64;_FILE_OFFSET_BITS=64")

    Returns:
        dict: Dictionary containing Y2038-related flag information with keys:
            - 'time_bits_defined' (bool): Whether _TIME_BITS is defined
            - 'time_bits_value' (int|None): Value of _TIME_BITS (None if not defined)
            - 'use_time_bits64_defined' (bool): Whether _USE_TIME_BITS64 is defined
            - 'file_offset_bits_defined' (bool): Whether _FILE_OFFSET_BITS is defined
            - 'file_offset_bits_value' (int|None): Value of _FILE_OFFSET_BITS (None if not defined)

    Example:
        >>> parse_compiler_flags("_TIME_BITS=64;_FILE_OFFSET_BITS=64")
        {
            'time_bits_defined': True,
            'time_bits_value': 64,
            'use_time_bits64_defined': False,
            'file_offset_bits_defined': True,
            'file_offset_bits_value': 64
        }
    """
    result = {
        'time_bits_defined': False,
        'time_bits_value': None,
        'use_time_bits64_defined': False,
        'file_offset_bits_defined': False,
        'file_offset_bits_value': None
    }

    if not config_name:
        return result

    # Check for _TIME_BITS=64 (correct value)
    if re_flag_time_bits_64.search(config_name):
        result['time_bits_defined'] = True
        result['time_bits_value'] = 64
    else:
        # Check for _TIME_BITS=other_value
        match = re_flag_time_bits.search(config_name)
        if match:
            result['time_bits_defined'] = True
            result['time_bits_value'] = int(match.group(1))

    # Check for _USE_TIME_BITS64
    if re_flag_use_time_bits64.search(config_name):
        result['use_time_bits64_defined'] = True

    # Check for _FILE_OFFSET_BITS=64 (correct value)
    if re_flag_file_offset_bits_64.search(config_name):
        result['file_offset_bits_defined'] = True
        result['file_offset_bits_value'] = 64
    else:
        # Check for _FILE_OFFSET_BITS=other_value
        match = re_flag_file_offset_bits.search(config_name)
        if match:
            result['file_offset_bits_defined'] = True
            result['file_offset_bits_value'] = int(match.group(1))

    return result


def check_y2038_safe(dumpfile, quiet=False):
    """
    Main function to check Y2038 safety of C/C++ code from cppcheck dump files.

    This function performs comprehensive Y2038 analysis including:
    1. Automatic build system detection to find Y2038-related compiler flags
    2. Analysis of source code preprocessor directives
    3. Warning suppression when proper Y2038 configuration is detected
    4. Reporting of Y2038-unsafe symbols and configurations

    The function implements a priority-based approach for Y2038 flag detection:
    - Build system files (highest priority)
    - Source code #define directives (fallback)

    Warning suppression occurs when both _TIME_BITS=64 AND _FILE_OFFSET_BITS=64
    are detected from any source. When warnings are suppressed, an informational
    message is displayed indicating the configuration source and suppression count.

    Args:
        dumpfile (str): Path to the cppcheck XML dump file (.dump extension)
        quiet (bool, optional): If True, suppress informational messages. Defaults to False.

    Returns:
        bool: True if code is Y2038-safe, False if Y2038 issues were detected

    Raises:
        Exception: May raise exceptions from cppcheckdata parsing or file I/O operations

    Example:
        >>> check_y2038_safe("test.c.dump")  # Normal operation
        True
        >>> check_y2038_safe("test.c.dump", quiet=True)  # Suppress info messages
        False
    """
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

        # Priority-based flag detection: buildsystem > source code directives
        # 1. First ensure compile_commands.json exists if we're in a build system project
        ensure_compile_commands(srcfile)
        
        # 2. Check buildsystem flags (highest priority)
        buildsystem_flags = parse_compile_commands(srcfile)
        
        # Use buildsystem flags if available, otherwise will fall back to source code directives below
        effective_flags = {
            'time_bits_defined': buildsystem_flags['time_bits_defined'],
            'time_bits_value': buildsystem_flags['time_bits_value'],
            'use_time_bits64_defined': buildsystem_flags['use_time_bits64_defined'],
            'file_offset_bits_defined': buildsystem_flags['file_offset_bits_defined'],
            'file_offset_bits_value': buildsystem_flags['file_offset_bits_value']
        }
        
        # Determine configuration source for reporting
        config_source = None
        if buildsystem_flags['time_bits_defined'] or buildsystem_flags['file_offset_bits_defined']:
            config_source = "build system"

        # Check effective _TIME_BITS value (from buildsystem)
        if effective_flags['time_bits_defined']:
            time_bits_defined = True
            if effective_flags['time_bits_value'] != 64:
                source_desc = config_source if config_source else "build system"
                fake_directive = type('FakeDirective', (), {
                    'file': srcfile, 'linenr': 0, 'column': 0,
                    'str': '%s: _TIME_BITS=%s' % (source_desc.title(), effective_flags['time_bits_value'])
                })()
                cppcheckdata.reportError(fake_directive, 'error',
                                         '_TIME_BITS must be defined equal to 64 (found in %s: _TIME_BITS=%s)' % (source_desc, effective_flags['time_bits_value']),
                                         'y2038',
                                         'type-bits-not-64')
                y2038safe = False

        # Check effective _FILE_OFFSET_BITS value (from buildsystem)
        if effective_flags['file_offset_bits_defined']:
            if effective_flags['file_offset_bits_value'] != 64:
                source_desc = config_source if config_source else "build system"
                fake_directive = type('FakeDirective', (), {
                    'file': srcfile, 'linenr': 0, 'column': 0,
                    'str': '%s: _FILE_OFFSET_BITS=%s' % (source_desc.title(), effective_flags['file_offset_bits_value'])
                })()
                cppcheckdata.reportError(fake_directive, 'error',
                                         '_FILE_OFFSET_BITS must be defined equal to 64 (found in %s: _FILE_OFFSET_BITS=%s)' % (source_desc, effective_flags['file_offset_bits_value']),
                                         'y2038',
                                         'file-offset-bits-not-64')
                y2038safe = False

        # Check effective _USE_TIME_BITS64 (from buildsystem)
        if effective_flags['use_time_bits64_defined']:
            if not time_bits_defined:
                # _USE_TIME_BITS64 defined without _TIME_BITS is problematic
                source_desc = config_source if config_source else "build system"
                fake_directive = type('FakeDirective', (), {
                    'file': srcfile, 'linenr': 0, 'column': 0,
                    'str': '%s: _USE_TIME_BITS64' % source_desc.title()
                })()
                cppcheckdata.reportError(fake_directive, 'warning',
                                         '_USE_TIME_BITS64 is defined in %s but _TIME_BITS was not' % source_desc,
                                         'y2038',
                                         'type-bits-undef')
                y2038safe = False
            else:
                # _USE_TIME_BITS64 defined WITH _TIME_BITS - this is correct
                safe = 0  # Start of file is safe

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
                # warn about _TIME_BITS not being defined (check both compiler flags and source)
                if not time_bits_defined:
                    cppcheckdata.reportError(directive, 'warning',
                                             '_USE_TIME_BITS64 is defined but _TIME_BITS was not',
                                             'y2038',
                                             'type-bits-undef')
            elif re_undef_use_time_bits64.match(directive.str):
                unsafe = int(srclinenr)
                # do we have a safe..unsafe area?
                if unsafe > safe >= 0:
                    safe_ranges.append((safe, unsafe))
                    safe = -1

        # check end of source beyond last directive
        if len(cfg.tokenlist) > 0:
            unsafe = int(cfg.tokenlist[-1].linenr)
            if unsafe > safe >= 0:
                safe_ranges.append((safe, unsafe))

        # Determine if Y2038 warnings should be suppressed
        # Require BOTH _TIME_BITS=64 AND _FILE_OFFSET_BITS=64 for complete Y2038 safety
        y2038_safe_config = (
            effective_flags['time_bits_defined'] and
            effective_flags['time_bits_value'] == 64 and
            effective_flags['file_offset_bits_defined'] and
            effective_flags['file_offset_bits_value'] == 64
        )

        # Update config_source for suppression reporting if Y2038-safe config was found
        if y2038_safe_config and config_source is None:
            # This handles the case where both flags are properly set but we need to report the source
            if buildsystem_flags['time_bits_defined'] and buildsystem_flags['file_offset_bits_defined']:
                config_source = "build system"

        # go through all tokens
        warnings_suppressed = 0
        for token in cfg.tokenlist:
            if token.str in id_Y2038:
                is_in_safe_range = any(lower <= int(token.linenr) <= upper
                                     for (lower, upper) in safe_ranges)

                if not is_in_safe_range:
                    if y2038_safe_config:
                        # Count suppressed warnings but don't report them
                        warnings_suppressed += 1
                    else:
                        # Report the warning as before
                        cppcheckdata.reportError(token, 'warning',
                                                 token.str + ' is Y2038-unsafe',
                                                 'y2038',
                                                 'unsafe-call')
                        y2038safe = False
            token = token.next

        # Print suppression message if warnings were suppressed
        if warnings_suppressed > 0 and config_source and not quiet:
            print('Y2038 warnings suppressed: Found proper Y2038 configuration in %s (_TIME_BITS=64 and _FILE_OFFSET_BITS=64)' % config_source)
            print('Suppressed %d Y2038-unsafe function warning(s)' % warnings_suppressed)

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
