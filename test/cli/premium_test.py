# python -m pytest premium_test.py

import os
import shutil
import sys
import time

from testutils import cppcheck, __lookup_cppcheck_exe

PRODUCT_NAME = 'Cppcheck Premium ' + str(time.time())


def copy_cppcheck_premium(tmpdir):
    exe = shutil.copy2(__lookup_cppcheck_exe(), tmpdir)

    # add minimum cfg/std.cfg
    test_cfg_folder = tmpdir.mkdir('cfg')
    with open(test_cfg_folder.join('std.cfg'), 'w') as f:
        f.write('<?xml version="1.0"?>\n<def format="2"/>')

    # add simple cppcheck.cfg
    with open(tmpdir.join('cppcheck.cfg'), 'w') as f:
        f.write("""
                {
                    "addons": [],
                    "productName": "NAME",
                    "about": "NAME",
                    "safety": false
                }
                """.replace('NAME', PRODUCT_NAME))
 
    return exe    


def test_misra_c_builtin_style_checks(tmpdir):
    # FIXME this test does not work in ci-windows.yml (release build)
    if sys.platform == 'win32':
        return

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'w') as f:
        f.write('void foo() { int x; y = 0; }')

    exe = copy_cppcheck_premium(tmpdir)
    _, stdout, stderr = cppcheck(['--premium=autosar', test_file], cppcheck_exe=exe)
    assert '' in stdout
    assert '[unusedVariable]' in stderr

