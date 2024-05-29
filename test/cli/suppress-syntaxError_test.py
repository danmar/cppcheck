# python -m pytest test-suppress-syntaxError.py

import pytest
from testutils import cppcheck

def test_j():
    ret, stdout, stderr = cppcheck(['-q', '--error-exitcode=1', '-j2', '-q', 'proj-suppress-syntaxError'])
    assert ret == 1, stdout
    assert len(stderr) > 0

def test_suppress_j():
    ret, stdout, stderr = cppcheck(['-q', '--error-exitcode=1', '--suppress=*:proj-suppress-syntaxError/*', '-j2', '-q', 'proj-suppress-syntaxError'])
    assert ret == 0, stdout
    assert len(stderr) == 0

# TODO: test with -j2
def test_safety_suppress_syntax_error_implicitly(tmpdir):
    ret, stdout, stderr = cppcheck(['-q', '--safety', '--suppress=*', 'proj-suppress-syntaxError', '-j1'], remove_checkers_report=False)
    assert ret == 1, stdout
    assert '[syntaxError]' in stderr
    assert 'Active checkers: There was critical errors' in stderr

# TODO: test with -j2
def test_safety_suppress_syntax_error_explicitly():
    ret, stdout, stderr = cppcheck(['-q', '--safety', '--suppress=syntaxError', 'proj-suppress-syntaxError', '-j1'], remove_checkers_report=False)
    assert ret == 1, stdout
    assert '[syntaxError]' not in stderr
    assert 'Active checkers: There was critical errors' in stderr
