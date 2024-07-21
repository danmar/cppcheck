#!/usr/bin/env python3

# Compare "normal" check level and "exhaustive" check level

import donate_cpu_lib as lib
import argparse
import glob
import os
import sys
import random


def format_float(a, b=1):
    if a > 0 and b > 0:
        return '{:.2f}'.format(a / b)
    return 'N/A'


if __name__ == "__main__":
    __my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
    __work_path = os.path.expanduser(os.path.join('~', 'cppcheck-' + __my_script_name + '-workfolder'))

    parser = argparse.ArgumentParser(description='Compare --check-level=normal and --check-level=exhaustive output')
    parser.add_argument('-j', default=1, type=int, help='Concurency execution threads')
    parser.add_argument('--cppcheck-path', default=None, type=str, help='Path to Cppcheck binary, if not given then clone and compile')
    package_group = parser.add_mutually_exclusive_group()
    package_group.add_argument('-p', default=256, type=int, help='Count of packages to check')
    package_group.add_argument('--packages', nargs='+', help='Check specific packages and then stop.')
    package_group.add_argument('--packages-path', default=None, type=str, help='Check packages in path.')
    parser.add_argument('-o', default='my_check_diff.log', help='Filename of result inside a working path dir')

    language_group = parser.add_mutually_exclusive_group()
    language_group.add_argument('--c-only', dest='c_only', help='Only process c packages', action='store_true')
    language_group.add_argument('--cpp-only', dest='cpp_only', help='Only process c++ packages', action='store_true')
    parser.add_argument('--work-path', '--work-path=', default=__work_path, type=str, help='Working directory for reference repo')
    args = parser.parse_args()

    print(args)

    if not lib.check_requirements():
        print("Error: Check requirements")
        sys.exit(1)

    work_path = os.path.abspath(args.work_path)
    if not os.path.exists(work_path):
        os.makedirs(work_path)

    lib.set_jobs('-j' + str(args.j))
    result_file = os.path.join(work_path, args.o)
    (f, ext) = os.path.splitext(result_file)
    timing_file = f + '_timing' + ext
    normal_results = f + '_normal' + ext
    exhaustive_results = f + '_exhaustive' + ext

    if os.path.exists(result_file):
        os.remove(result_file)
    if os.path.exists(timing_file):
        os.remove(timing_file)
    if os.path.exists(normal_results):
        os.remove(normal_results)
    if os.path.exists(exhaustive_results):
        os.remove(exhaustive_results)

    cppcheck_path = args.cppcheck_path

    if cppcheck_path is None:
        cppcheck_path = os.path.join(work_path, 'cppcheck')
        try:
            lib.clone_cppcheck(cppcheck_path, '')
        except Exception as e:
            print('Failed to clone Cppcheck repository ({}), retry later'.format(e))
            sys.exit(1)

        if not lib.compile_cppcheck(cppcheck_path):
            print('Failed to compile Cppcheck')
            sys.exit(1)

    if args.packages_path:
        # You can download packages using daca2-download.py
        args.packages = glob.glob(os.path.join(args.packages_path, '*.tar.xz'))
        args.p = len(args.packages)
        packages_idxs = list(range(args.p))
        random.shuffle(packages_idxs)
    elif args.packages:
        args.p = len(args.packages)
        packages_idxs = []
    else:
        packages_count = lib.get_packages_count()
        if not packages_count:
            print("network or server might be temporarily down..")
            sys.exit(1)

        packages_idxs = list(range(packages_count))
        random.shuffle(packages_idxs)

    packages_processed = 0
    crashes = []
    timeouts = []

    while (packages_processed < args.p and len(packages_idxs) > 0) or args.packages:
        if args.packages:
            package = args.packages.pop()
        else:
            package = lib.get_package(packages_idxs.pop())

        if package.startswith('ftp://') or package.startswith('http://'):
            tgz = lib.download_package(work_path, package, None)
            if tgz is None:
                print("No package downloaded")
                continue
        else:
            print('Package: ' + package)
            tgz = package

        source_path, source_found = lib.unpack_package(work_path, tgz, c_only=args.c_only, cpp_only=args.cpp_only)
        if not source_found:
            print("No files to process")
            continue

        results_to_diff = list()

        normal_crashed = False
        exhaustive_crashed = False

        normal_timeout = False
        exhaustive_timeout = False

        enable = 'style'
        debug_warnings = False

        libraries = lib.library_includes.get_libraries(source_path)
        c, errout, info, time_normal, cppcheck_options, timing_info = lib.scan_package(cppcheck_path, source_path, libraries, enable=enable, debug_warnings=debug_warnings, check_level='normal')
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                print('Error: 101')
            elif c == lib.RETURN_CODE_TIMEOUT:
                print('Normal check level timed out!')
                normal_timeout = True
                continue # we don't want to compare timeouts
            else:
                print('Normal check level crashed!')
                normal_crashed = True
        results_to_diff.append(errout)

        c, errout, info, time_exhaustive, cppcheck_options, timing_info = lib.scan_package(cppcheck_path, source_path, libraries, enable=enable, debug_warnings=debug_warnings, check_level='exhaustive')
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                print('Error: 101')
            elif c == lib.RETURN_CODE_TIMEOUT:
                print('Exhaustive check level timed out!')
                exhaustive_timeout = True
                continue # we don't want to compare timeouts
            else:
                print('Exhaustive check level crashed!')
                exhaustive_crashed = True
        results_to_diff.append(errout)

        if normal_crashed or exhaustive_crashed:
            who = None
            if normal_crashed and exhaustive_crashed:
                who = 'Both'
            elif normal_crashed:
                who = 'Normal'
            else:
                who = 'Exhaustive'
            crashes.append(package + ' ' + who)

        if normal_timeout or exhaustive_timeout:
            who = None
            if normal_timeout and exhaustive_timeout:
                who = 'Both'
            elif normal_timeout:
                who = 'Normal'
            else:
                who = 'Exhaustive'
            timeouts.append(package + ' ' + who)

        with open(result_file, 'a') as myfile:
            myfile.write(package + '\n')
            diff = lib.diff_results('normal', results_to_diff[0], 'exhaustive', results_to_diff[1])
            if not normal_crashed and not exhaustive_crashed and diff != '':
                myfile.write('libraries:' + ','.join(libraries) +'\n')
                myfile.write('diff:\n' + diff + '\n')

        if not normal_crashed and not exhaustive_crashed:
            with open(timing_file, 'a') as myfile:
                package_width = '140'
                timing_width = '>7'
                myfile.write('{:{package_width}} {:{timing_width}} {:{timing_width}} {:{timing_width}}\n'.format(
                    package, format_float(time_normal),
                    format_float(time_exhaustive), format_float(time_normal, time_exhaustive),
                    package_width=package_width, timing_width=timing_width))
            with open(normal_results, 'a') as myfile:
                myfile.write(results_to_diff[0])
            with open(exhaustive_results, 'a') as myfile:
                myfile.write(results_to_diff[1])

        packages_processed += 1
        print(str(packages_processed) + ' of ' + str(args.p) + ' packages processed\n')

    with open(result_file, 'a') as myfile:
        myfile.write('\n\ncrashes\n')
        myfile.write('\n'.join(crashes))

    with open(result_file, 'a') as myfile:
        myfile.write('\n\ntimeouts\n')
        myfile.write('\n'.join(timeouts) + '\n')

    print('Result saved to: ' + result_file)
