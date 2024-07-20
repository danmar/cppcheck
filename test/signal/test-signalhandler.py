import subprocess
import os
import sys
import pytest

def _lookup_cppcheck_exe(exe_name):
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

def _call_process(arg):
    exe = _lookup_cppcheck_exe('test-signalhandler')
    if exe is None:
        raise Exception('executable not found')
    p = subprocess.Popen([exe, arg], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return p.returncode, stdout, stderr


def test_assert():
    _, stdout, stderr = _call_process('assert')
    if sys.platform == "darwin":
        assert stderr.startswith("Assertion failed: (false), function my_assert, file test-signalhandler.cpp, line "), stderr
    else:
        assert stderr.endswith("test-signalhandler.cpp:33: void my_assert(): Assertion `false' failed.\n"), stderr
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace of MacOs
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_abort()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'


def test_abort():
    _, stdout, _ = _call_process('abort')
    lines = stdout.splitlines()
    assert lines[0] == 'Internal error: cppcheck received signal SIGABRT - abort or assertion'
    # no stacktrace on MaCos
    if sys.platform != "darwin":
        assert lines[1] == 'Callstack:'
        assert lines[2].endswith('my_segv()'), lines[2]  # TODO: wrong function
        assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'


def test_segv():
    _, stdout, stderr = _call_process('segv')
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
    _, stdout, stderr = _call_process('fpe')
    assert stderr == ''
    lines = stdout.splitlines()
    assert lines[0].startswith('Internal error: cppcheck received signal SIGFPE - FPE_FLTDIV (at 0x7f'), lines[0]
    assert lines[0].endswith(').'), lines[0]
    assert lines[1] == 'Callstack:'
    assert lines[2].endswith('my_fpe()'), lines[2]
    assert lines[len(lines)-1] == 'Please report this to the cppcheck developers!'
