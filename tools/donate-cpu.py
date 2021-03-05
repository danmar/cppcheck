#!/usr/bin/env python3

# Donate CPU
#
# A script a user can run to donate CPU to cppcheck project
#
# Syntax: donate-cpu.py [-jN] [--package=url] [--stop-time=HH:MM] [--work-path=path] [--test] [--bandwidth-limit=limit]
#  -jN                  Use N threads in compilation/analysis. Default is 1.
#  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce
#                       some warning/crash/exception/etc..
#  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.
#  --work-path=path     Work folder path. Default path is cppcheck-donate-cpu-workfolder in your home folder.
#  --test               Connect to a donate-cpu-server that is running locally on port 8001 for testing.
#  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.
#                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second
#                                 --bandwidth-limit=2m => max. 2 megabytes per second
#  --max-packages=N     Process N packages and then exit. A value of 0 means infinitely.
#  --no-upload          Do not upload anything. Defaults to False.
#
# What this script does:
# 1. Check requirements
# 2. Pull & compile Cppcheck
# 3. Select a package
# 4. Download package
# 5. Analyze source code
# 6. Upload results
# 7. Repeat from step 2
#
# Quick start: just run this script without any arguments

import platform
from donate_cpu_lib import *

for arg in sys.argv[1:]:
    # --stop-time=12:00 => run until ~12:00 and then stop
    if arg.startswith('--stop-time='):
        stop_time = arg[-5:]
        print('Stop time:' + stop_time)
    elif arg.startswith('-j'):
        if not re.match(r'-j\d+', arg):
            print('Argument "{}" is invalid.'.format(arg))
            print('"-j" must be followed by a positive number.')
            sys.exit(1)
        jobs = arg
        print('Jobs:' + jobs[2:])
    elif arg.startswith('--package='):
        package_url = arg[arg.find('=')+1:]
        print('Package:' + package_url)
    elif arg.startswith('--work-path='):
        work_path = arg[arg.find('=')+1:]
        print('work_path:' + work_path)
        if not os.path.exists(work_path):
            print('work path does not exist!')
            sys.exit(1)
    elif arg == '--test':
        server_address = ('localhost', 8001)
    elif arg.startswith('--bandwidth-limit='):
        bandwidth_limit = arg[arg.find('=')+1:]
    elif arg.startswith('--max-packages='):
        arg_value = arg[arg.find('=')+1:]
        try:
            max_packages = int(arg_value)
        except ValueError:
            max_packages = None
        if max_packages < 0:
            max_packages = None
        if max_packages is None:
            print('Error: Max. packages value "{}" is invalid. Must be a positive number or 0.'.format(arg_value))
            sys.exit(1)
        # 0 means infinitely, no counting needed.
        if max_packages == 0:
            max_packages = None
    elif arg.startswith('--no-upload'):
        do_upload = False
    elif arg == '--help':
        print('Donate CPU to Cppcheck project')
        print('')
        print('Syntax: donate-cpu.py [-jN] [--stop-time=HH:MM] [--work-path=path]')
        print('  -jN                  Use N threads in compilation/analysis. Default is 1.')
        print('  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce')
        print('                       some warning/crash/exception/etc..')
        print('  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.')
        print('  --work-path=path     Work folder path. Default path is ' + work_path)
        print('  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.')
        print('                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second')
        print('                                 --bandwidth-limit=2m => max. 2 megabytes per second')
        print('  --max-packages=N     Process N packages and then exit. A value of 0 means infinitely.')
        print('  --no-upload          Do not upload anything. Defaults to False.')
        print('')
        print('Quick start: just run this script without any arguments')
        sys.exit(0)
    else:
        print('Unhandled argument: ' + arg)
        sys.exit(1)

if sys.version_info.major < 3 or (sys.version_info.major == 3 and sys.version_info.minor < 4):
    print("#" * 80)
    print("IMPORTANT")
    print("Please run the client with at least Python 3.4, thanks!")
    print("#" * 80)
    time.sleep(2)
    sys.exit(1)

print('Thank you!')
if not check_requirements():
    sys.exit(1)
if bandwidth_limit and isinstance(bandwidth_limit, str):
    if subprocess.call(['wget', '--limit-rate=' + bandwidth_limit, '-q', '--spider', 'cppcheck1.osuosl.org']) == 2:
        print('Error: Bandwidth limit value "' + bandwidth_limit + '" is invalid.')
        sys.exit(1)
    else:
        print('Bandwidth-limit: ' + bandwidth_limit)
if package_url:
    max_packages = 1
if max_packages:
    print('Maximum number of packages to download and analyze: {}'.format(max_packages))
if not os.path.exists(work_path):
    os.mkdir(work_path)
cppcheck_path = os.path.join(work_path, 'cppcheck')
packages_processed = 0
while True:
    if max_packages:
        if packages_processed >= max_packages:
            print('Processed the specified number of {} package(s). Exiting now.'.format(max_packages))
            break
        print('Processing package {} of the specified {} package(s).'.format(packages_processed + 1, max_packages))
        packages_processed += 1
    if stop_time:
        print('stop_time:' + stop_time + '. Time:' + time.strftime('%H:%M') + '.')
        if stop_time < time.strftime('%H:%M'):
            print('Stopping. Thank you!')
            sys.exit(0)
    if not get_cppcheck(cppcheck_path, work_path):
        print('Failed to clone Cppcheck, retry later')
        sys.exit(1)
    cppcheck_versions = get_cppcheck_versions(server_address)
    if cppcheck_versions is None:
        print('Failed to communicate with server, retry later')
        sys.exit(1)
    if len(cppcheck_versions) == 0:
        print('Did not get any cppcheck versions from server, retry later')
        sys.exit(1)
    for ver in cppcheck_versions:
        if ver == 'head':
            if not compile_cppcheck(cppcheck_path, jobs):
                print('Failed to compile Cppcheck, retry later')
                sys.exit(1)
        elif not compile_version(work_path, jobs, ver):
            print('Failed to compile Cppcheck-{}, retry later'.format(ver))
            sys.exit(1)
    if package_url:
        package = package_url
    else:
        package = get_package(server_address)
    tgz = download_package(work_path, package, bandwidth_limit)
    if tgz is None:
        print("No package downloaded")
        continue
    if not unpack_package(work_path, tgz):
        print("No files to process")
        continue
    crash = False
    timeout = False
    count = ''
    elapsed_time = ''
    results_to_diff = []
    cppcheck_options = ''
    head_info_msg = ''
    head_timing_info = ''
    old_timing_info = ''
    cppcheck_head_info = ''
    libraries = get_libraries()

    for ver in cppcheck_versions:
        if ver == 'head':
            current_cppcheck_dir = 'cppcheck'
            cppcheck_head_info = get_cppcheck_info(work_path + '/cppcheck')
        else:
            current_cppcheck_dir = ver
        c, errout, info, t, cppcheck_options, timing_info = scan_package(work_path, current_cppcheck_dir, jobs, libraries)
        if c < 0:
            if c == -101 and 'error: could not find or open any of the paths given.' in errout:
                # No sourcefile found (for example only headers present)
                count += ' 0'
            elif c == RETURN_CODE_TIMEOUT:
                # Timeout
                count += ' TO!'
                timeout = True
            else:
                crash = True
                count += ' Crash!'
        else:
            count += ' ' + str(c)
        elapsed_time += " {:.1f}".format(t)
        results_to_diff.append(errout)
        if ver == 'head':
            head_info_msg = info
            head_timing_info = timing_info
        else:
            old_timing_info = timing_info

    output = 'cppcheck-options: ' + cppcheck_options + '\n'
    output += 'platform: ' + platform.platform() + '\n'
    output += 'python: ' + platform.python_version() + '\n'
    output += 'client-version: ' + CLIENT_VERSION + '\n'
    output += 'compiler: ' + get_compiler_version() + '\n'
    output += 'cppcheck: ' + ' '.join(cppcheck_versions) + '\n'
    output += 'head-info: ' + cppcheck_head_info + '\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsed_time + '\n'
    output += 'head-timing-info:\n' + head_timing_info + '\n'
    output += 'old-timing-info:\n' + old_timing_info + '\n'
    info_output = output
    info_output += 'info messages:\n' + head_info_msg
    if 'head' in cppcheck_versions:
        output += 'head results:\n' + results_to_diff[cppcheck_versions.index('head')]
    if not crash and not timeout:
        output += 'diff:\n' + diff_results(cppcheck_versions[0], results_to_diff[0], cppcheck_versions[1], results_to_diff[1]) + '\n'
    if package_url:
        print('=========================================================')
        print(output)
        print('=========================================================')
        print(info_output)
        print('=========================================================')
    if do_upload:
        upload_results(package, output, server_address)
        upload_info(package, info_output, server_address)
    if not max_packages or packages_processed < max_packages:
        print('Sleep 5 seconds..')
        time.sleep(5)
