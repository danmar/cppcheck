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
CLIENT_VERSION = "1.3.25"

# Timeout for analysis with Cppcheck in seconds
CPPCHECK_TIMEOUT = 30 * 60

CPPCHECK_REPO_URL = "https://github.com/danmar/cppcheck.git"

# Return code that is used to mark a timed out analysis
RETURN_CODE_TIMEOUT = -999


def check_requirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget', 'gdb']:
        try:
            subprocess.call([app, '--version'])
        except OSError:
            print("Error: '{}' is required".format(app))
            result = False
    try:
        import psutil
    except ImportError as e:
        print("Error: {}. Module is required.".format(e))
        result = False
    return result


# Try and retry with exponential backoff if an exception is raised
def try_retry(fun, fargs=(), max_tries=5):
    sleep_duration = 5.0
    for i in range(max_tries):
        try:
            return fun(*fargs)
        except KeyboardInterrupt as e:
            # Do not retry in case of user abort
            raise e
        except BaseException as e:
            if i < max_tries - 1:
                print("{} in {}: {}".format(type(e).__name__, fun.__name__, str(e)))
                print("Trying {} again in {} seconds".format(fun.__name__, sleep_duration))
                time.sleep(sleep_duration)
                sleep_duration *= 2.0
            else:
                print("Maximum number of tries reached for {}".format(fun.__name__))
                raise e


def clone_cppcheck(repo_path, migrate_from_path):
    repo_git_dir = os.path.join(repo_path, '.git')
    if os.path.exists(repo_git_dir):
        return
    # Attempt to migrate clone directory used prior to 1.3.17
    if os.path.exists(migrate_from_path):
        os.rename(migrate_from_path, repo_path)
    else:
        # A shallow git clone (depth = 1) is enough for building and scanning.
        # Do not checkout until fetch_cppcheck_version.
        subprocess.check_call(['git', 'clone', '--depth=1', '--no-checkout', CPPCHECK_REPO_URL, repo_path])
        # Checkout an empty branch to allow "git worktree add" for main later on
    try:
        # git >= 2.27
        subprocess.check_call(['git', 'switch', '--orphan', 'empty'], cwd=repo_path)
    except subprocess.CalledProcessError:
        subprocess.check_call(['git', 'checkout', '--orphan', 'empty'], cwd=repo_path)


def checkout_cppcheck_version(repo_path, version, cppcheck_path):
    if not os.path.isabs(cppcheck_path):
        raise ValueError("cppcheck_path is not an absolute path")
    if os.path.exists(cppcheck_path):
        print('Checking out {}'.format(version))
        subprocess.check_call(['git', 'checkout', '-f', version], cwd=cppcheck_path)

        # It is possible to pull branches, not tags
        if version != 'main':
            return

        print('Pulling {}'.format(version))
        subprocess.check_call(['git', 'pull'], cwd=cppcheck_path)
    else:
        if version != 'main':
            print('Fetching {}'.format(version))
            # Since this is a shallow clone, explicitly fetch the remote version tag
            refspec = 'refs/tags/' + version + ':ref/tags/' + version
            subprocess.check_call(['git', 'fetch', '--depth=1', 'origin', refspec], cwd=repo_path)
        print('Adding worktree \'{}\' for {}'.format(cppcheck_path, version))
        subprocess.check_call(['git', 'worktree', 'add', cppcheck_path,  version], cwd=repo_path)


def get_cppcheck_info(cppcheck_path):
    try:
        os.chdir(cppcheck_path)
        return subprocess.check_output(['git', 'show', "--pretty=%h (%ci)", 'HEAD', '--no-patch', '--no-notes']).decode('utf-8').strip()
    except:
        return ''


def compile_version(cppcheck_path, jobs):
    if os.path.isfile(os.path.join(cppcheck_path, 'cppcheck')):
        return True
    # Build
    ret = compile_cppcheck(cppcheck_path, jobs)
    # Clean intermediate build files
    subprocess.call(['git', 'clean', '-f', '-d', '-x', '--exclude', 'cppcheck'], cwd=cppcheck_path)
    return ret


def compile_cppcheck(cppcheck_path, jobs):
    print('Compiling {}'.format(os.path.basename(cppcheck_path)))
    try:
        os.chdir(cppcheck_path)
        if sys.platform == 'win32':
            subprocess.call(['MSBuild.exe', cppcheck_path + '/cppcheck.sln', '/property:Configuration=Release', '/property:Platform=x64'])
            subprocess.call([cppcheck_path + '/bin/cppcheck.exe', '--version'])
        else:
            subprocess.check_call(['make', jobs, 'MATCHCOMPILER=yes', 'CXXFLAGS=-O2 -g -w'], cwd=cppcheck_path)
            subprocess.check_call([os.path.join(cppcheck_path, 'cppcheck'), '--version'], cwd=cppcheck_path)
    except:
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


def __handle_remove_readonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)


def __remove_tree(folder_name):
    if not os.path.exists(folder_name):
        return
    count = 5
    while count > 0:
        count -= 1
        try:
            shutil.rmtree(folder_name, onerror=__handle_remove_readonly)
            break
        except OSError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup {}: {}'.format(folder_name, err))
                sys.exit(1)


def __wget(url, destfile, bandwidth_limit):
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
    destfile = os.path.join(work_path, 'temp.tgz')
    if not __wget(package, destfile, bandwidth_limit):
        return None
    return destfile


def unpack_package(work_path, tgz, cpp_only=False, skip_files=None):
    print('Unpacking..')
    temp_path = os.path.join(work_path, 'temp')
    __remove_tree(temp_path)
    os.mkdir(temp_path)
    found = False
    if tarfile.is_tarfile(tgz):
        with tarfile.open(tgz) as tf:
            for member in tf:
                header_endings = ('.hpp', '.h++', '.hxx', '.hh', '.h')
                source_endings = ('.cpp', '.c++', '.cxx', '.cc', '.tpp', '.txx', '.ipp', '.ixx', '.qml')
                c_source_endings = ('.c',)
                if not cpp_only:
                    source_endings = source_endings + c_source_endings
                if member.name.startswith(('/', '..')):
                    # Skip dangerous file names
                    continue
                elif member.name.lower().endswith(header_endings + source_endings):
                    if skip_files is not None:
                        skip = False
                        for skip_file in skip_files:
                            if member.name.endswith('/' + skip_file):
                                skip = True
                                break
                        if skip:
                            continue
                    try:
                        tf.extract(member.name, temp_path)
                        if member.name.lower().endswith(source_endings):
                            found = True
                    except OSError:
                        pass
                    except AttributeError:
                        pass
    return found


def __has_include(path, includes):
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


def __run_command(cmd, print_cmd=True):
    if print_cmd:
        print(cmd)
    start_time = time.time()
    comm = None
    if sys.platform == 'win32':
        p = subprocess.Popen(shlex.split(cmd, comments=False, posix=False), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=os.setsid)
    try:
        comm = p.communicate(timeout=CPPCHECK_TIMEOUT)
        return_code = p.returncode
        p = None
    except subprocess.TimeoutExpired:
        import psutil
        return_code = RETURN_CODE_TIMEOUT
        # terminate all the child processes so we get messages about which files were hanging
        child_procs = psutil.Process(p.pid).children(recursive=True)
        if len(child_procs) > 0:
            for child in child_procs:
                child.terminate()
            try:
                # call with timeout since it might get stuck e.g. gcc-arm-none-eabi
                comm = p.communicate(timeout=5)
                p = None
            except subprocess.TimeoutExpired:
                pass
    finally:
        if p:
            os.killpg(os.getpgid(p.pid), signal.SIGTERM)  # Send the signal to all the process groups
            comm = p.communicate()
    stop_time = time.time()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')
    elapsed_time = stop_time - start_time
    return return_code, stdout, stderr, elapsed_time


def scan_package(work_path, cppcheck_path, jobs, libraries, capture_callstack=True):
    print('Analyze..')
    os.chdir(work_path)
    libs = ''
    for library in libraries:
        if os.path.exists(os.path.join(cppcheck_path, 'cfg', library + '.cfg')):
            libs += '--library=' + library + ' '

    dir_to_scan = 'temp'

    # Reference for GNU C: https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    options = libs + ' --showtime=top5 --check-library --inconclusive --enable=style,information --inline-suppr --template=daca2'
    options += ' -D__GNUC__ --platform=unix64'
    options += ' -rp={}'.format(dir_to_scan)
    if sys.platform == 'win32':
        cppcheck_cmd = cppcheck_path + '/bin/cppcheck.exe ' + options
        cmd = cppcheck_cmd + ' ' + jobs + ' ' + dir_to_scan
    else:
        cppcheck_cmd = os.path.join(cppcheck_path, 'cppcheck') + ' ' + options
        cmd = 'nice ' + cppcheck_cmd + ' ' + jobs + ' ' + dir_to_scan
    returncode, stdout, stderr, elapsed_time = __run_command(cmd)

    # collect messages
    information_messages_list = []
    issue_messages_list = []
    internal_error_messages_list = []
    count = 0
    for line in stderr.split('\n'):
        if ': information: ' in line:
            information_messages_list.append(line + '\n')
        elif line:
            issue_messages_list.append(line + '\n')
            if re.match(r'.*:[0-9]+:.*\]$', line):
                count += 1
            if ': error: Internal error: ' in line:
                internal_error_messages_list.append(line + '\n')
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

    # detect errors
    sig_file = None
    sig_num = -1
    for ie_line in internal_error_messages_list:
        # temp/dlib-19.10/dlib/test/dnn.cpp:0:0: error: Internal error: Child process crashed with signal 11 [cppcheckError]
        if 'Child process crashed with signal' in ie_line:
            sig_file = ie_line.split(':')[0]
            sig_msg = 'signal '
            sig_pos = ie_line.find(sig_msg)
            if sig_pos != -1:
                sig_start_pos = sig_pos + len(sig_msg)
                sig_num = int(ie_line[sig_start_pos:ie_line.find(' ', sig_start_pos)])
            # break on the first signalled file for now
            break
    print('cppcheck finished with ' + str(returncode) + ('' if sig_num == -1 else ' (signal ' + str(sig_num) + ')'))

    options_j = options + ' ' + jobs

    if returncode == RETURN_CODE_TIMEOUT:
        print('Timeout!')
        return returncode, ''.join(internal_error_messages_list), '', elapsed_time, options_j, ''

    # generate stack trace for SIGSEGV, SIGABRT, SIGILL, SIGFPE, SIGBUS
    has_error = returncode in (-11, -6, -4, -8, -7)
    has_sig = sig_num in (11, 6, 4, 8, 7)
    if has_error or has_sig:
        print('Crash!')
        # make sure we have the actual error code set
        if has_sig:
            returncode = -sig_num
        stacktrace = ''
        if capture_callstack:
            # re-run within gdb to get a stacktrace
            cmd = 'gdb --batch --eval-command=run --eval-command="bt 50" --return-child-result --args ' + cppcheck_cmd + " -j1 "
            if sig_file is not None:
                cmd += sig_file
            else:
                cmd += dir_to_scan
            _, st_stdout, _, _ = __run_command(cmd)
            gdb_pos = st_stdout.find(" received signal")
            if not gdb_pos == -1:
                last_check_pos = st_stdout.rfind('Checking ', 0, gdb_pos)
                if last_check_pos == -1:
                    stacktrace = st_stdout[gdb_pos:]
                else:
                    stacktrace = st_stdout[last_check_pos:]
        # if no stacktrace was generated return the original stdout or internal errors list
        if not stacktrace:
            if has_sig:
                stacktrace = ''.join(internal_error_messages_list)
            else:
                stacktrace = stdout
        return returncode, stacktrace, '', returncode, options_j, ''

    if returncode != 0:
        # returncode is always 1 when this message is written
        thr_pos = stderr.find('#### ThreadExecutor')
        if thr_pos != -1:
            print('Thread!')
            return -222, stderr[thr_pos:], '', -222, options_j, ''

        print('Error!')
        if returncode > 0:
            returncode = -100-returncode
        return returncode, stdout, '', returncode, options_j, ''

    if sig_num != -1:
        print('Signal!')
        return -sig_num, ''.join(internal_error_messages_list), '', -sig_num, options_j, ''

    return count, ''.join(issue_messages_list), ''.join(information_messages_list), elapsed_time, options_j, timing_str


def __split_results(results):
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


def diff_results(ver1, results1, ver2, results2):
    print('Diff results..')
    ret = ''
    r1 = sorted(__split_results(results1))
    r2 = sorted(__split_results(results2))
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


def __send_all(connection, data):
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
                __send_all(sock, cmd + package + '\n' + results + '\nDONE')
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
                __send_all(sock, 'write_info\n' + package + '\n' + info_output + '\nDONE')
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
                       'bsd': ['<sys/queue.h>', '<sys/tree.h>', '<sys/uio.h>', '<bsd/', '<fts.h>', '<db.h>', '<err.h>', '<vis.h>'],
                       'cairo': ['<cairo.h>'],
                       'cppunit': ['<cppunit/'],
                       'icu': ['<unicode/', '"unicode/'],
                       'ginac': ['<ginac/', '"ginac/'],
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
                       'ntl': ['<ntl/', '"ntl/'],
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
        if __has_include('temp', includes):
            libraries.append(library)
    return libraries


def get_compiler_version():
    _, stdout, _, _ = __run_command('g++ --version', False)
    return stdout.split('\n')[0]


my_script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
jobs = '-j1'
stop_time = None
work_path = os.path.expanduser('~/cppcheck-' + my_script_name + '-workfolder')
package_url = None
server_address = ('cppcheck1.osuosl.org', 8000)
bandwidth_limit = None
max_packages = None
do_upload = True
