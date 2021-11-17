
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

