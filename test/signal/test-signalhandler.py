import subprocess
import os
import sys
import pytest

def __lookup_cppcheck_exe(exe_name):
    # path the script is located in
    script_path = os.path.dirname(os.path.realpath(__file__))

    if sys.platform == "win32":
        exe_name += ".exe"

    for base in (script_path + '/../../', './'):
        for path in ('', 'bin/', 'bin/debug/'):
            exe_path = base + path + exe_name
            if os.path.isfile(exe_path):
                print("using '{}'".format(exe_path))
                return exe_path

    return None

def __call_process(arg):
    exe = __lookup_cppcheck_exe('test-signalhandler')
    if exe is None:
        raise Exception('executable not found')
    with subprocess.Popen([exe, arg], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    stdout = stdout.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = stderr.decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return rc, stdout, stderr


def test_assert():
    _, stdout, stderr = __call_process('assert')
    if sys.platform == "darwin":
        assert stderr.startswith("Assertion failed: (false), function my_assert, file test-signalhandler.cpp, line "), stderr
    else:
        assert stderr.endswith("test-signalhandler.cpp:34: void my_assert(): Assertion `false' failed.\n"), stderr
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace of MacOs
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_abort()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'


def test_abort():
    _, stdout, _ = __call_process('abort')
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace on MaCos
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_segv()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'


def test_segv():
    _, stdout, stderr = __call_process('segv')
    assert stderr == ''
    lines = stdout.splitlines()
    if sys.platform == "darwin":
        assert lines[0] == 'Internal error: cppcheck received signal SIGSEGV - SEGV_MAPERR (at 0x0).'
    else:
        assert lines[0] == 'Internal error: cppcheck received signal SIGSEGV - SEGV_MAPERR (reading at 0x0).'
    # no stacktrace on MacOS
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_segv()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'


# TODO: make this work
@pytest.mark.skip
def test_fpe():
    _, stdout, stderr = __call_process('fpe')
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0].startswith('Internal error: cppcheck received signal SIGFPE - FPE_FLTDIV (at 0x7f'), lines[0]
    assert lines[0].endswith(').'), lines[0]
    assert lines[1] == 'Callstack:'
    assert lines[2].endswith('my_fpe()'), lines[2]
    assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
