
# python -m pytest test-suppress-syntaxError.py

from testutils import cppcheck

def test_j2():
    ret, stdout, stderr = cppcheck(['--error-exitcode=1', '-j2', '-q', 'proj-suppress-syntaxError'])
    assert ret == 1
    assert len(stderr) > 0

def test_j2_suppress():
    ret, stdout, stderr = cppcheck(['--error-exitcode=1', '--suppress=*:proj-suppress-syntaxError/*', '-j2', '-q', 'proj-suppress-syntaxError'])
    assert ret == 0
    assert len(stderr) == 0

def test_safety_suppress_syntax_error_implicitly(tmpdir):
    ret, stdout, stderr = cppcheck(['--safety', '--suppress=*', 'proj-suppress-syntaxError'], remove_checkers_report=False)
    assert ret == 1
    assert '[syntaxError]' in stderr
    assert 'Active checkers: There was critical errors' in stderr

def test_safety_suppress_syntax_error_explicitly():
    ret, stdout, stderr = cppcheck(['--safety', '--suppress=syntaxError', 'proj-suppress-syntaxError'], remove_checkers_report=False)
    assert ret == 1
    assert '[syntaxError]' not in stderr
    assert 'Active checkers: There was critical errors' in stderr

