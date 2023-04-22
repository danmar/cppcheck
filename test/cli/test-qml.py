
# python3 -m pytest test-qml.py

import os
from testutils import cppcheck

PROJECT_DIR = 'QML-Samples-TableView'

def test_unused_functions():
    ret, stdout, stderr = cppcheck(['--library=qt', '--enable=unusedFunction', PROJECT_DIR])
    assert ret == 0
    assert '[unusedFunction]' not in stderr
