
# python -m pytest premium_test.py

import os
import shutil
import sys
import time

from testutils import cppcheck, __lookup_cppcheck_exe

__PRODUCT_NAME = 'Cppcheck Premium ' + str(time.time())


def __copy_cppcheck_premium(tmpdir):
    exe = shutil.copy2(__lookup_cppcheck_exe(), tmpdir)

    # add minimum cfg/std.cfg
    test_cfg_folder = tmpdir.mkdir('cfg')
    with open(test_cfg_folder.join('std.cfg'), 'wt') as f:
        f.write('<?xml version="1.0"?>\n<def format="2"/>')

    # add simple cppcheck.cfg
    with open(tmpdir.join('cppcheck.cfg'), 'wt') as f:
        f.write("""
                {
                    "addons": [],
                    "productName": "NAME",
                    "about": "NAME",
                    "safety": true
                }
                """.replace('NAME', __PRODUCT_NAME))

    return exe


def test_misra_c_builtin_style_checks(tmpdir):
    # FIXME this test does not work in ci-windows.yml (release build)
    if sys.platform == 'win32':
        return

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write('void foo() { int x; y = 0; }')

    exe = __copy_cppcheck_premium(tmpdir)

    exitcode, _, stderr = cppcheck(['--premium=autosar', '--xml', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert 'id="unusedVariable"' in stderr
    assert 'id="checkersReport"' in stderr

    exitcode, _, stderr = cppcheck(['--premium=autosar', '--premium=safety-off', '--xml', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert 'id="unusedVariable"' in stderr
    assert 'id="checkersReport"' not in stderr
