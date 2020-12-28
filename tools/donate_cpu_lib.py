# Donate CPU client library

import shutil
import os
import subprocess
import sys
import socket
import time
import re
import signal
import tarfile
import shlex


# Version scheme (MAJOR.MINOR.PATCH) should orientate on "Semantic Versioning" https://semver.org/
# Every change in this script should result in increasing the version number accordingly (exceptions may be cosmetic
# changes)
CLIENT_VERSION = "1.3.6"

# Timeout for analysis with Cppcheck in seconds
CPPCHECK_TIMEOUT = 60 * 60

# Return code that is used to mark a timed out analysis
RETURN_CODE_TIMEOUT = -999


def check_requirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget', 'gdb']:
        try:
            subprocess.call([app, '--version'])
        except OSError:
            print(app + ' is required')
            result = False
    return result


def get_cppcheck(cppcheck_path, work_path):
    print('Get Cppcheck..')
    for i in range(5):
        if os.path.exists(cppcheck_path):
            try:
                os.chdir(cppcheck_path)
                try:
                    subprocess.check_call(['git', 'checkout', '-f', 'main'])
                except subprocess.CalledProcessError:
                    subprocess.check_call(['git', 'checkout', '-f', 'master'])
                    subprocess.check_call(['git', 'pull'])
                    subprocess.check_call(['git', 'checkout', 'origin/main', '-b', 'main'])
                subprocess.check_call(['git', 'pull'])
            except:
                print('Failed to update Cppcheck sources! Retrying..')
                time.sleep(10)
                continue
        else:
            try:
                subprocess.check_call(['git', 'clone', 'https://github.com/danmar/cppcheck.git', cppcheck_path])
            except:
                print('Failed to clone, will try again in 10 minutes..')
                time.sleep(600)
                continue
        time.sleep(2)
        return True
    if os.path.exists(cppcheck_path):
        print('Failed to update Cppcheck sources, trying a fresh clone..')
        try:
            os.chdir(work_path)
            shutil.rmtree(cppcheck_path)
            get_cppcheck(cppcheck_path, work_path)
        except:
            print('Failed to remove Cppcheck folder, please manually remove ' + work_path)
            return False
    return False


def get_cppcheck_info(cppcheck_path):
    try:
        os.chdir(cppcheck_path)
        return subprocess.check_output(['git', 'show', "--pretty=%h (%ci)", 'HEAD', '--no-patch', '--no-notes']).decode('utf-8').strip()
    except:
        return ''


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
    subprocess.call(['git', 'checkout', 'main'])
    try:
        subprocess.call([work_path + '/' + version + '/cppcheck', '--version'])
    except OSError:
        return False
    return True


def compile_cppcheck(cppcheck_path, jobs):
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
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect(server_address)
            sock.send(b'GetCppcheckVersions\n')
            versions = sock.recv(256)
    except socket.error as err:
        print('Failed to get cppcheck versions: ' + str(err))
        return None
    return versions.decode('utf-8').split()


def get_packages_count(server_address):
    print('Connecting to server to get count of packages..')
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect(server_address)
            sock.send(b'getPackagesCount\n')
            packages = int(sock.recv(64))
    except socket.error as err:
        print('Failed to get count of packages: ' + str(err))
        return None
    return packages


def get_package(server_address, package_index=None):
    package = b''
    while not package:
        print('Connecting to server to get assigned work..')
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect(server_address)
                if package_index is None:
                    sock.send(b'get\n')
                else:
                    request = 'getPackageIdx:' + str(package_index) + '\n'
                    sock.send(request.encode())
                package = sock.recv(256)
        except socket.error:
            print("network or server might be temporarily down.. will try again in 30 seconds..")
            time.sleep(30)
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
        with tarfile.open(tgz) as tf:
            for member in tf:
                if member.name.startswith(('/', '..')):
                    # Skip dangerous file names
                    continue
                elif member.name.lower().endswith(('.c', '.cpp', '.cxx', '.cc', '.c++', '.h', '.hpp',
                                                   '.h++', '.hxx', '.hh', '.tpp', '.txx', '.qml',
                                                   '.sln', '.vcproj', '.vcxproj')):
                    try:
                        tf.extract(member.name)
                        found = True
                    except OSError:
                        pass
                    except AttributeError:
                        pass
    os.chdir(work_path)
    return found


def has_include(path, includes):
    re_includes = [re.escape(inc) for inc in includes]
    re_expr = '^[ \t]*#[ \t]*include[ \t]*(' + '|'.join(re_includes) + ')'
    for root, _, files in os.walk(path):
        for name in files:
            filename = os.path.join(root, name)
            try:
                with open(filename, 'rt', errors='ignore') as f:
                    filedata = f.read()
                if re.search(re_expr, filedata, re.MULTILINE):
                    return True
            except IOError:
                pass
    return False


def run_command(cmd):
    print(cmd)
    startTime = time.time()
    p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=os.setsid)
    try:
        comm = p.communicate(timeout=CPPCHECK_TIMEOUT)
        return_code = p.returncode
    except subprocess.TimeoutExpired:
        os.killpg(os.getpgid(p.pid), signal.SIGTERM)  # Send the signal to all the process groups
        comm = p.communicate()
        return_code = RETURN_CODE_TIMEOUT
    stop_time = time.time()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')
    elapsed_time = stop_time - startTime
    return return_code, stdout, stderr, elapsed_time


def scan_package(work_path, cppcheck_path, jobs, libraries):
    print('Analyze..')
    os.chdir(work_path)
    libs = ''
    for library in libraries:
        if os.path.exists(os.path.join(cppcheck_path, 'cfg', library + '.cfg')):
            libs += '--library=' + library + ' '

    # Reference for GNU C: https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    options = libs + jobs + ' --showtime=top5 --check-library --inconclusive --enable=style,information --template=daca2 -rp=temp'
    if os.path.isfile('temp/tortoisesvn/TortoiseSVN.sln'):
        options = options.replace('--library=posix ', '')
        options = options.replace('--library=gnu ', '')
        options = '--library=windows ' + options
        options += ' --platform=win64 --project=temp/tortoisesvn/TortoiseSVN.sln'
    else:
        options += ' -D__GNUC__ --platform=unix64 temp'
    cppcheck_cmd = cppcheck_path + '/cppcheck' + ' ' + options
    cmd = 'nice ' + cppcheck_cmd
    returncode, stdout, stderr, elapsed_time = run_command(cmd)
    sig_num = -1
    sig_msg = 'Internal error: Child process crashed with signal '
    sig_pos = stderr.find(sig_msg)
    if sig_pos != -1:
        sig_start_pos = sig_pos + len(sig_msg)
        sig_num = int(stderr[sig_start_pos:stderr.find(' ', sig_start_pos)])
    print('cppcheck finished with ' + str(returncode) + ('' if sig_num == -1 else ' (signal ' + str(sig_num) + ')'))
    if returncode == RETURN_CODE_TIMEOUT:
        print('Timeout!')
        return returncode, stdout, '', elapsed_time, options, ''
    # generate stack trace for SIGSEGV, SIGABRT, SIGILL, SIGFPE, SIGBUS
    if returncode in (-11, -6, -4, -8, -7) or sig_num in (11, 6, 4, 8, 7):
        print('Crash!')
        stacktrace = ''
        if cppcheck_path == 'cppcheck':
            # re-run within gdb to get a stacktrace
            cmd = 'gdb --batch --eval-command=run --eval-command="bt 50" --return-child-result --args ' + cppcheck_cmd + " -j1"
            dummy, st_stdout, dummy, dummy = run_command(cmd)
            gdb_pos = st_stdout.find(" received signal")
            if not gdb_pos == -1:
                last_check_pos = st_stdout.rfind('Checking ', 0, gdb_pos)
                if last_check_pos == -1:
                    stacktrace = st_stdout[gdb_pos:]
                else:
                    stacktrace = st_stdout[last_check_pos:]
        # if no stacktrace was generated return the original stdout
        if not stacktrace:
            stacktrace = stdout
        return returncode, stacktrace, '', returncode, options, ''
    if returncode != 0:
        print('Error!')
        if returncode > 0:
            returncode = -100-returncode
        return returncode, stdout, '', returncode, options, ''
    err_s = 'Internal error: Child process crashed with signal '
    err_pos = stderr.find(err_s)
    if err_pos != -1:
        print('Error!')
        pos2 = stderr.find(' [cppcheckError]', err_pos)
        signr = int(stderr[err_pos+len(err_s):pos2])
        return -signr, '', '', -signr, options, ''
    thr_pos = stderr.find('#### ThreadExecutor')
    if thr_pos != -1:
        print('Thread!')
        return -222, stderr[thr_pos:], '', -222, options, ''
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
    # Collect timing information
    stdout_lines = stdout.split('\n')
    timing_info_list = []
    overall_time_found = False
    max_timing_lines = 6
    current_timing_lines = 0
    for reverse_line in reversed(stdout_lines):
        if reverse_line.startswith('Overall time:'):
            overall_time_found = True
        if overall_time_found:
            if not reverse_line or current_timing_lines >= max_timing_lines:
                break
            timing_info_list.insert(0, ' ' + reverse_line + '\n')
            current_timing_lines += 1
    timing_str = ''.join(timing_info_list)
    return count, ''.join(issue_messages_list), ''.join(information_messages_list), elapsed_time, options, timing_str


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
    bytes_ = data.encode('ascii', 'ignore')
    while bytes_:
        num = connection.send(bytes_)
        if num < len(bytes_):
            bytes_ = bytes_[num:]
        else:
            bytes_ = None


def upload_results(package, results, server_address):
    print('Uploading results.. ' + str(len(results)) + ' bytes')
    max_retries = 4
    for retry in range(max_retries):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect(server_address)
                cmd = 'write\n'
                send_all(sock, cmd + package + '\n' + results + '\nDONE')
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
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect(server_address)
                send_all(sock, 'write_info\n' + package + '\n' + info_output + '\nDONE')
            print('Information output has been successfully uploaded.')
            return True
        except socket.error as err:
            print('Upload error: ' + str(err))
            if retry < (max_retries - 1):
                print('Retrying upload in 30 seconds')
                time.sleep(30)
    print('Upload permanently failed!')
    return False


def get_libraries():
    libraries = ['posix', 'gnu']
    library_includes = {'boost': ['<boost/'],
                       'bsd': ['<sys/queue.h>','<sys/tree.h>','<bsd/','<fts.h>','<db.h>','<err.h>','<vis.h>'],
                       'cairo': ['<cairo.h>'],
                       'cppunit': ['<cppunit/'],
                       'icu': ['<unicode/', '"unicode/'],
                       'ginac': ['<ginac/','"ginac/'],
                       'googletest': ['<gtest/gtest.h>'],
                       'gtk': ['<gtk', '<glib.h>', '<glib-', '<glib/', '<gnome'],
                       'kde': ['<KGlobal>', '<KApplication>', '<KDE/'],
                       'libcerror': ['<libcerror.h>'],
                       'libcurl': ['<curl/curl.h>'],
                       'libsigc++': ['<sigc++/'],
                       'lua': ['<lua.h>', '"lua.h"'],
                       'mfc': ['<afx.h>', '<afxwin.h>', '<afxext.h>'],
                       'microsoft_atl': ['<atlbase.h>'],
                       'microsoft_sal': ['<sal.h>'],
                       'motif': ['<X11/', '<Xm/'],
                       'nspr': ['<prtypes.h>', '"prtypes.h"'],
                       'opencv2': ['<opencv2/', '"opencv2/'],
                       'opengl': ['<GL/gl.h>', '<GL/glu.h>', '<GL/glut.h>'],
                       'openmp': ['<omp.h>'],
                       'openssl': ['<openssl/'],
                       'pcre': ['<pcre.h>', '"pcre.h"'],
                       'python': ['<Python.h>', '"Python.h"'],
                       'qt': ['<QApplication>', '<QList>', '<QKeyEvent>', '<qlist.h>', '<QObject>', '<QFlags>', '<QFileDialog>', '<QTest>', '<QMessageBox>', '<QMetaType>', '<QString>', '<qobjectdefs.h>', '<qstring.h>', '<QWidget>', '<QtWidgets>', '<QtGui'],
                       'ruby': ['<ruby.h>', '<ruby/', '"ruby.h"'],
                       'sdl': ['<SDL.h>', '<SDL/SDL.h>', '<SDL2/SDL.h>'],
                       'sqlite3': ['<sqlite3.h>', '"sqlite3.h"'],
                       'tinyxml2': ['<tinyxml2', '"tinyxml2'],
                       'wxsqlite3': ['<wx/wxsqlite3', '"wx/wxsqlite3'],
                       'wxwidgets': ['<wx/', '"wx/'],
                       'zlib': ['<zlib.h>'],
                      }
    for library, includes in library_includes.items():
        if has_include('temp', includes):
            libraries.append(library)
    return libraries


my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
jobs = '-j1'
stop_time = None
work_path = os.path.expanduser('~/cppcheck-' + my_script_name + '-workfolder')
package_url = None
server_address = ('cppcheck1.osuosl.org', 8000)
bandwidth_limit = None
max_packages = None
do_upload = True
