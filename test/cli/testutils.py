import errno
import logging
import os
import select
import subprocess
import time

# Create Cppcheck project file
import sys


def create_gui_project_file(project_file, root_path=None, import_project=None, paths=None, exclude_paths=None, suppressions=None, addon=None):
    cppcheck_xml = ('<?xml version="1.0" encoding="UTF-8"?>\n'
                    '<project version="1">\n')
    if root_path:
        cppcheck_xml += '  <root name="' + root_path + '"/>\n'
    if import_project:
        cppcheck_xml += '  <importproject>' + import_project + '</importproject>\n'
    if paths:
        cppcheck_xml += '  <paths>\n'
        for path in paths:
            cppcheck_xml += '    <dir name="' + path + '"/>\n'
        cppcheck_xml += '  </paths>\n'
    if exclude_paths:
        cppcheck_xml += '  <exclude>\n'
        for path in exclude_paths:
            cppcheck_xml += '    <path name="' + path + '"/>\n'
        cppcheck_xml += '  </exclude>\n'
    if suppressions:
        cppcheck_xml += '  <suppressions>\n'
        for suppression in suppressions:
            cppcheck_xml += '    <suppression'
            if 'fileName' in suppression:
                cppcheck_xml += ' fileName="' + suppression['fileName'] + '"'
            cppcheck_xml += '>' + suppression['id'] + '</suppression>\n'
        cppcheck_xml += '  </suppressions>\n'
    if addon:
        cppcheck_xml += '  <addons>\n'
        cppcheck_xml += '    <addon>%s</addon>\n' % addon
        cppcheck_xml += '  </addons>\n'
    cppcheck_xml += '</project>\n'

    f = open(project_file, 'wt')
    f.write(cppcheck_xml)
    f.close()


def __lookup_cppcheck_exe():
    # path the script is located in
    script_path = os.path.dirname(os.path.realpath(__file__))

    exe_name = "cppcheck"

    if sys.platform == "win32":
        exe_name += ".exe"

    exe_path = None

    if 'TEST_CPPCHECK_EXE_LOOKUP_PATH' in os.environ:
        lookup_paths = [os.environ['TEST_CPPCHECK_EXE_LOOKUP_PATH']]
    else:
        lookup_paths = [os.path.join(script_path, '..', '..'), '.']

    for base in lookup_paths:
        for path in ('', 'bin', os.path.join('bin', 'debug')):
            tmp_exe_path = os.path.join(base, path, exe_name)
            if os.path.isfile(tmp_exe_path):
                exe_path = tmp_exe_path
                break

    if exe_path:
        exe_path = os.path.abspath(exe_path)
        print("using '{}'".format(exe_path))
    return exe_path


def __run_subprocess_tty(args, env=None, cwd=None, timeout=None):
    import pty
    mo, so = pty.openpty()
    me, se = pty.openpty()
    p = subprocess.Popen(args, stdout=so, stderr=se, env=env, cwd=cwd)
    for fd in [so, se]:
        os.close(fd)

    select_timeout = 0.04  # seconds
    readable = [mo, me]
    result = {mo: b'', me: b''}
    try:
        start_time = time.monotonic()
        while readable:
            ready, _, _ = select.select(readable, [], [], select_timeout)
            for fd in ready:
                try:
                    data = os.read(fd, 512)
                except OSError as e:
                    if e.errno != errno.EIO:
                        raise
                    # EIO means EOF on some systems
                    readable.remove(fd)
                else:
                    if not data: # EOF
                        readable.remove(fd)
                    result[fd] += data
            if timeout is not None and (time.monotonic() - start_time):
                break
    finally:
        for fd in [mo, me]:
            os.close(fd)
        if p.poll() is None:
            p.kill()
        return_code = p.wait()

    stdout = result[mo]
    stderr = result[me]
    return return_code, stdout, stderr


def __run_subprocess(args, env=None, cwd=None, timeout=None):
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env, cwd=cwd)

    try:
        comm = p.communicate(timeout=timeout)
        return_code = p.returncode
        p = None
    except subprocess.TimeoutExpired:
        import psutil
        # terminate all the child processes
        child_procs = psutil.Process(p.pid).children(recursive=True)
        if len(child_procs) > 0:
            for child in child_procs:
                child.terminate()
            try:
                # call with timeout since it might be stuck
                p.communicate(timeout=5)
                p = None
            except subprocess.TimeoutExpired:
                pass
        raise
    finally:
        if p:
            # sending the signal to the process groups causes the parent Python process to terminate as well
            #os.killpg(os.getpgid(p.pid), signal.SIGTERM)  # Send the signal to all the process groups
            p.terminate()
            comm = p.communicate()

    stdout = comm[0]
    stderr = comm[1]
    return return_code, stdout, stderr


# Run Cppcheck with args
def cppcheck_ex(args, env=None, remove_checkers_report=True, cwd=None, cppcheck_exe=None, timeout=None, tty=False):
    exe = cppcheck_exe if cppcheck_exe else __lookup_cppcheck_exe()
    assert exe is not None, 'no cppcheck binary found'

    if 'TEST_CPPCHECK_INJECT_J' in os.environ:
        found_j = False
        for arg in args:
            if arg.startswith('-j'):
                found_j = True
                break
        if not found_j:
            arg_j = '-j' + str(os.environ['TEST_CPPCHECK_INJECT_J'])
            args.append(arg_j)

    if 'TEST_CPPCHECK_INJECT_CLANG' in os.environ:
        found_clang = False
        for arg in args:
            if arg.startswith('--clang'):
                found_clang = True
                break
        if not found_clang:
            arg_clang = '--clang=' + str(os.environ['TEST_CPPCHECK_INJECT_CLANG'])
            args.append(arg_clang)

    if 'TEST_CPPCHECK_INJECT_EXECUTOR' in os.environ:
        found_jn = False
        found_executor = False
        for arg in args:
            if arg.startswith('-j') and arg != '-j1':
                found_jn = True
                continue
            if arg.startswith('--executor'):
                found_executor = True
                continue
        # only add '--executor' if we are actually using multiple jobs
        if found_jn and not found_executor:
            arg_executor = '--executor=' + str(os.environ['TEST_CPPCHECK_INJECT_EXECUTOR'])
            args.append(arg_executor)

    logging.info(exe + ' ' + ' '.join(args))

    run_subprocess = __run_subprocess_tty if tty else __run_subprocess
    return_code, stdout, stderr = run_subprocess([exe] + args, env=env, cwd=cwd, timeout=timeout)

    stdout = stdout.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = stderr.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')

    if remove_checkers_report:
        if stderr.find('[checkersReport]\n') > 0:
            start_id = stderr.find('[checkersReport]\n')
            start_line = stderr.rfind('\n', 0, start_id)
            if start_line <= 0:
                stderr = ''
            else:
                stderr = stderr[:start_line + 1]
        elif stderr.find(': (information) Active checkers: ') >= 0:
            pos = stderr.find(': (information) Active checkers: ')
            if pos == 0:
                stderr = ''
            elif stderr[pos - 1] == '\n':
                stderr = stderr[:pos]
    return return_code, stdout, stderr, exe


def cppcheck(*args, **kwargs):
    return_code, stdout, stderr, _ = cppcheck_ex(*args, **kwargs)
    return return_code, stdout, stderr


def assert_cppcheck(args, ec_exp=None, out_exp=None, err_exp=None, env=None):
    exitcode, stdout, stderr = cppcheck(args, env)
    if ec_exp is not None:
        assert exitcode == ec_exp, stdout
    if out_exp is not None:
        out_lines = stdout.splitlines()
        assert out_lines == out_exp, out_lines
    if err_exp is not None:
        err_lines = stderr.splitlines()
        assert err_lines == err_exp, err_lines
