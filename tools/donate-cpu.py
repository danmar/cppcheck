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

import shutil
import os
import subprocess
import sys
import socket
import time
import re
import tarfile
import platform


# Version scheme (MAJOR.MINOR.PATCH) should orientate on "Semantic Versioning" https://semver.org/
# Every change in this script should result in increasing the version number accordingly (exceptions may be cosmetic
# changes)
CLIENT_VERSION = "1.1.25"


def check_requirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget', 'gdb']:
        try:
            subprocess.call([app, '--version'])
        except OSError:
            print(app + ' is required')
            result = False
    return result


def get_cppcheck(cppcheck_path):
    print('Get Cppcheck..')
    for i in range(5):
        if os.path.exists(cppcheck_path):
            os.chdir(cppcheck_path)
            subprocess.call(['git', 'checkout', '-f'])
            subprocess.call(['git', 'pull'])
        else:
            subprocess.call(['git', 'clone', 'https://github.com/danmar/cppcheck.git', cppcheck_path])
            if not os.path.exists(cppcheck_path):
                print('Failed to clone, will try again in 10 minutes..')
                time.sleep(600)
                continue
        time.sleep(2)
        return True
    return False


def compile_version(work_path, jobs, version):
    if os.path.isfile(work_path + '/' + version + '/cppcheck'):
        return True
    os.chdir(work_path + '/cppcheck')
    subprocess.call(['git', 'checkout', version])
    subprocess.call(['make', 'clean'])
    subprocess.call(['make', jobs, 'MATCHCOMPILER=yes', 'CXXFLAGS=-O2 -g'])
    if os.path.isfile(work_path + '/cppcheck/cppcheck'):
        os.mkdir(work_path + '/' + version)
        destPath = work_path + '/' + version + '/'
        subprocess.call(['cp', '-R', work_path + '/cppcheck/cfg', destPath])
        subprocess.call(['cp', 'cppcheck', destPath])
    subprocess.call(['git', 'checkout', 'master'])
    try:
        subprocess.call([work_path + '/' + version + '/cppcheck', '--version'])
    except OSError:
        return False
    return True


def compile(cppcheck_path, jobs):
    print('Compiling Cppcheck..')
    try:
        os.chdir(cppcheck_path)
        subprocess.call(['make', jobs, 'MATCHCOMPILER=yes', 'CXXFLAGS=-O2 -g'])
        subprocess.call([cppcheck_path + '/cppcheck', '--version'])
    except OSError:
        return False
    return True


def get_cppcheck_versions(server_address):
    print('Connecting to server to get Cppcheck versions..')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(server_address)
        sock.send(b'GetCppcheckVersions\n')
        versions = sock.recv(256)
    except socket.error as err:
        print('Failed to get cppcheck versions: ' + str(err))
        return None
    sock.close()
    return versions.decode('utf-8').split()


def get_package(server_address):
    print('Connecting to server to get assigned work..')
    package = None
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(server_address)
        sock.send(b'get\n')
        package = sock.recv(256)
    except socket.error:
        package = ''
    sock.close()
    return package.decode('utf-8')


def handle_remove_readonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)


def remove_tree(folderName):
    if not os.path.exists(folderName):
        return
    count = 5
    while count > 0:
        count -= 1
        try:
            shutil.rmtree(folderName, onerror=handle_remove_readonly)
            break
        except OSError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup {}: {}'.format(folderName, err))
                sys.exit(1)


def wget(url, destfile, bandwidth_limit):
    if os.path.exists(destfile):
        if os.path.isfile(destfile):
            os.remove(destfile)
        else:
            print('Error: ' + destfile + ' exists but it is not a file! Please check the path and delete it manually.')
            sys.exit(1)
    wget_call = ['wget', '--tries=10', '--timeout=300', '-O', destfile, url]
    if bandwidth_limit and isinstance(bandwidth_limit, str):
        wget_call.append('--limit-rate=' + bandwidth_limit)
    exitcode = subprocess.call(wget_call)
    if exitcode != 0:
        print('wget failed with ' + str(exitcode))
        os.remove(destfile)
        return False
    if not os.path.isfile(destfile):
        return False
    return True


def download_package(work_path, package, bandwidth_limit):
    print('Download package ' + package)
    destfile = work_path + '/temp.tgz'
    if not wget(package, destfile, bandwidth_limit):
        return None
    return destfile


def unpack_package(work_path, tgz):
    print('Unpacking..')
    temp_path = work_path + '/temp'
    remove_tree(temp_path)
    os.mkdir(temp_path)
    os.chdir(temp_path)
    found = False
    if tarfile.is_tarfile(tgz):
        tf = tarfile.open(tgz)
        for member in tf:
            if member.name.startswith(('/', '..')):
                # Skip dangerous file names
                continue
            elif member.name.lower().endswith(('.c', '.cpp', '.cxx', '.cc', '.c++', '.h', '.hpp',
                                               '.h++', '.hxx', '.hh', '.tpp', '.txx', '.qml')):
                try:
                    tf.extract(member.name)
                    found = True
                except OSError:
                    pass
                except AttributeError:
                    pass
        tf.close()
    os.chdir(work_path)
    return found


def has_include(path, includes):
    re_includes = [re.escape(inc) for inc in includes]
    re_expr = '^[ \t]*#[ \t]*include[ \t]*(' + '|'.join(re_includes) + ')'
    for root, _, files in os.walk(path):
        for name in files:
            filename = os.path.join(root, name)
            try:
                if sys.version_info.major < 3:
                    f = open(filename, 'rt')
                else:
                    f = open(filename, 'rt', errors='ignore')
                filedata = f.read()
                try:
                    # Python2 needs to decode the data first
                    filedata = filedata.decode(encoding='utf-8', errors='ignore')
                except AttributeError:
                    # Python3 directly reads the data into a string object that has no decode()
                    pass
                f.close()
                if re.search(re_expr, filedata, re.MULTILINE):
                    return True
            except IOError:
                pass
    return False


def run_command(cmd):
    print(cmd)
    startTime = time.time()
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stop_time = time.time()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')
    elapsed_time = stop_time - startTime
    return p.returncode, stdout, stderr, elapsed_time


def scan_package(work_path, cppcheck_path, jobs):
    print('Analyze..')
    os.chdir(work_path)
    libraries = ' --library=posix --library=gnu'

    library_includes = {'boost': ['<boost/'],
                       'cppunit': ['<cppunit/'],
                       'googletest': ['<gtest/gtest.h>'],
                       'gtk': ['<gtk/gtk.h>', '<glib.h>', '<glib/'],
                       'libcerror': ['<libcerror.h>'],
                       'microsoft_sal': ['<sal.h>'],
                       'motif': ['<X11/', '<Xm/'],
                       'nspr': ['<prtypes.h>', '"prtypes.h"'],
                       'opengl': ['<GL/gl.h>', '<GL/glu.h>', '<GL/glut.h>'],
                       # 'openmp': ['<omp.h>'], <= enable after release of version 1.89
                       'python': ['<Python.h>', '"Python.h"'],
                       'qt': ['<QApplication>', '<QString>', '<QWidget>', '<QtWidgets>', '<QtGui'],
                       'ruby': ['<ruby.h>', '<ruby/'],
                       'sdl': ['<SDL.h>'],
                       'sqlite3': ['<sqlite3.h>', '"sqlite3.h"'],
                       'tinyxml2': ['<tinyxml2', '"tinyxml2'],
                       'wxwidgets': ['<wx/', '"wx/'],
                       'zlib': ['<zlib.h>'],
                      }
    for library, includes in library_includes.items():
        if os.path.exists(os.path.join(cppcheck_path, 'cfg', library + '.cfg')) and has_include('temp', includes):
            libraries += ' --library=' + library

    # Reference for GNU C: https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    options = jobs + libraries + ' -D__GNUC__ --check-library --inconclusive --enable=style,information --platform=unix64 --template=daca2 -rp=temp temp'
    cppcheck_cmd = cppcheck_path + '/cppcheck' + ' ' + options
    cmd = 'nice ' + cppcheck_cmd
    returncode, stdout, stderr, elapsed_time = run_command(cmd)
    print('cppcheck finished with ' + str(returncode))
    if returncode == -11 or stderr.find('Internal error: Child process crashed with signal 11 [cppcheckError]') > 0 or returncode == -6 or stderr.find('Internal error: Child process crashed with signal 6 [cppcheckError]') > 0:
        print('Crash!')
        stacktrace = ''
        if cppcheck_path == 'cppcheck':
            # re-run within gdb to get a stacktrace
            cmd = 'gdb --batch --eval-command=run --eval-command=bt --return-child-result --args ' + cppcheck_cmd + " -j1"
            returncode, stdout, stderr, elapsed_time = run_command(cmd)
            gdb_pos = stdout.find(" received signal")
            if not gdb_pos == -1:
                last_check_pos = stdout.rfind('Checking ', 0, gdb_pos)
                if last_check_pos == -1:
                    stacktrace = stdout[gdb_pos:]
                else:
                    stacktrace = stdout[last_check_pos:]
        return returncode, stacktrace, '', returncode, options
    if returncode != 0:
        print('Error!')
        if returncode > 0:
            returncode = -100-returncode
        return returncode, stdout, '', returncode, options
    if stderr.find('Internal error: Child process crashed with signal ') > 0:
        print('Error!')
        s = 'Internal error: Child process crashed with signal '
        pos1 = stderr.find(s)
        pos2 = stderr.find(' [cppcheckError]', pos1)
        signr = int(stderr[pos1+len(s):pos2])
        return -signr, '', '', -signr, options
    if stderr.find('#### ThreadExecutor') > 0:
        print('Thread!')
        return -222, '', '', -222, options
    information_messages_list = []
    issue_messages_list = []
    count = 0
    for line in stderr.split('\n'):
        if ': information: ' in line:
            information_messages_list.append(line + '\n')
        elif line:
            issue_messages_list.append(line + '\n')
            if re.match(r'.*:[0-9]+:.*\]$', line):
                count += 1
    print('Number of issues: ' + str(count))
    return count, ''.join(issue_messages_list), ''.join(information_messages_list), elapsed_time, options


def split_results(results):
    ret = []
    w = None
    for line in results.split('\n'):
        if line.endswith(']') and re.search(r': (error|warning|style|performance|portability|information|debug):', line):
            if w is not None:
                ret.append(w.strip())
            w = ''
        if w is not None:
            w += ' ' * 5 + line + '\n'
    if w is not None:
        ret.append(w.strip())
    return ret


def diff_results(work_path, ver1, results1, ver2, results2):
    print('Diff results..')
    ret = ''
    r1 = sorted(split_results(results1))
    r2 = sorted(split_results(results2))
    i1 = 0
    i2 = 0
    while i1 < len(r1) and i2 < len(r2):
        if r1[i1] == r2[i2]:
            i1 += 1
            i2 += 1
        elif r1[i1] < r2[i2]:
            ret += ver1 + ' ' + r1[i1] + '\n'
            i1 += 1
        else:
            ret += ver2 + ' ' + r2[i2] + '\n'
            i2 += 1
    while i1 < len(r1):
        ret += ver1 + ' ' + r1[i1] + '\n'
        i1 += 1
    while i2 < len(r2):
        ret += ver2 + ' ' + r2[i2] + '\n'
        i2 += 1

    return ret


def send_all(connection, data):
    bytes = data.encode('ascii', 'ignore')
    while bytes:
        num = connection.send(bytes)
        if num < len(bytes):
            bytes = bytes[num:]
        else:
            bytes = None


def upload_results(package, results, server_address):
    print('Uploading results.. ' + str(len(results)) + ' bytes')
    max_retries = 4
    for retry in range(max_retries):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(server_address)
            cmd = 'write\n'
            send_all(sock, cmd + package + '\n' + results + '\nDONE')
            sock.close()
            print('Results have been successfully uploaded.')
            return True
        except socket.error as err:
            print('Upload error: ' + str(err))
            if retry < (max_retries - 1):
                print('Retrying upload in 30 seconds')
                time.sleep(30)
    print('Upload permanently failed!')
    return False


def upload_info(package, info_output, server_address):
    print('Uploading information output.. ' + str(len(info_output)) + ' bytes')
    max_retries = 3
    for retry in range(max_retries):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(server_address)
            send_all(sock, 'write_info\n' + package + '\n' + info_output + '\nDONE')
            sock.close()
            print('Information output has been successfully uploaded.')
            return True
        except socket.error as err:
            print('Upload error: ' + str(err))
            if retry < (max_retries - 1):
                print('Retrying upload in 30 seconds')
                time.sleep(30)
    print('Upload permanently failed!')
    return False


jobs = '-j1'
stop_time = None
work_path = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
package_url = None
server_address = ('cppcheck.osuosl.org', 8000)
bandwidth_limit = None
max_packages = None
do_upload = True
for arg in sys.argv[1:]:
    # --stop-time=12:00 => run until ~12:00 and then stop
    if arg.startswith('--stop-time='):
        stop_time = arg[-5:]
        print('Stop time:' + stop_time)
    elif arg.startswith('-j'):
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

print('Thank you!')
if not check_requirements():
    sys.exit(1)
if bandwidth_limit and isinstance(bandwidth_limit, str):
    if subprocess.call(['wget', '--limit-rate=' + bandwidth_limit, '-q', '--spider', 'cppcheck.osuosl.org']) is 2:
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
        else:
            print('Processing package {} of the specified {} package(s).'.format(packages_processed + 1, max_packages))
        packages_processed += 1
    if stop_time:
        print('stop_time:' + stop_time + '. Time:' + time.strftime('%H:%M') + '.')
        if stop_time < time.strftime('%H:%M'):
            print('Stopping. Thank you!')
            sys.exit(0)
    if not get_cppcheck(cppcheck_path):
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
            if not compile(cppcheck_path, jobs):
                print('Failed to compile Cppcheck, retry later')
                sys.exit(1)
        elif not compile_version(work_path, jobs, ver):
            print('Failed to compile Cppcheck-{}, retry later'.format(ver))
            sys.exit(1)
    if package_url:
        package = package_url
    else:
        package = get_package(server_address)
    while len(package) == 0:
        print("network or server might be temporarily down.. will try again in 30 seconds..")
        time.sleep(30)
        package = get_package(server_address)
    tgz = download_package(work_path, package, bandwidth_limit)
    if tgz is None:
        print("No package downloaded")
        continue
    if not unpack_package(work_path, tgz):
        print("No files to process")
        continue
    crash = False
    count = ''
    elapsed_time = ''
    results_to_diff = []
    cppcheck_options = ''
    head_info_msg = ''
    for ver in cppcheck_versions:
        if ver == 'head':
            current_cppcheck_dir = 'cppcheck'
        else:
            current_cppcheck_dir = ver
        c, errout, info, t, cppcheck_options = scan_package(work_path, current_cppcheck_dir, jobs)
        if c < 0:
            crash = True
            count += ' Crash!'
        else:
            count += ' ' + str(c)
        elapsed_time += " {:.1f}".format(t)
        results_to_diff.append(errout)
        if ver == 'head':
            head_info_msg = info

    output = 'cppcheck-options: ' + cppcheck_options + '\n'
    output += 'platform: ' + platform.platform() + '\n'
    output += 'python: ' + platform.python_version() + '\n'
    output += 'client-version: ' + CLIENT_VERSION + '\n'
    output += 'cppcheck: ' + ' '.join(cppcheck_versions) + '\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsed_time + '\n'
    info_output = output
    info_output += 'info messages:\n' + head_info_msg
    if 'head' in cppcheck_versions:
        output += 'head results:\n' + results_to_diff[cppcheck_versions.index('head')]
    if not crash:
        output += 'diff:\n' + diff_results(work_path, cppcheck_versions[0], results_to_diff[0], cppcheck_versions[1], results_to_diff[1]) + '\n'
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
