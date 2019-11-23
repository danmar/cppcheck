
# python -m pytest test-inline-suppress.py

import os
import re
from testutils import cppcheck

def test1():
    ret, stdout, stderr = cppcheck(['--inline-suppr', 'proj-inline-suppress'])
    assert ret == 0
    # TODO assert len(stderr) == 0

def test2():
    ret, stdout, stderr = cppcheck(['proj-inline-suppress'])
    assert ret == 0
    assert len(stderr) > 0

def test_unmatched_suppression():
    ret, stdout, stderr = cppcheck(['--inline-suppr', '--enable=information', '--error-exitcode=1', 'proj-inline-suppress/2.c'])
    assert ret == 1
    assert 'Unmatched suppression: some_warning_id' in stderr
