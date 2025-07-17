
# python -m pytest premium_test.py

import os
import re
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

    exitcode, _, stderr = cppcheck(['--xml-version=3', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert '<safety/>' in stderr

    exitcode, _, stderr = cppcheck(['--xml-version=3', '--premium=safety-off', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert '<safety/>' not in stderr

    exitcode, _, stderr = cppcheck(['--xml-version=3', '--inline-suppr', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert '<inline-suppr/>' in stderr

    exitcode, _, stderr = cppcheck(['--xml-version=3', '--suppress=foo', test_file], cppcheck_exe=exe)
    assert exitcode == 0
    assert '<suppression errorId="foo" inline="false" />' in stderr


def test_build_dir_hash_cppcheck_product(tmpdir):
    # 13644 - cppcheck build dir hashes should depend on the cppcheck version
    # so that files are rescanned when cppcheck is switched

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write(';')

    build_dir = tmpdir.mkdir('b')
    args = [f'--cppcheck-build-dir={build_dir}', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert 'error' not in stdout
    assert stderr == ''
    assert exitcode == 0

    def _get_hash(s:str):
        i = s.find(' hash="')
        if i <= -1:
            return ''
        i += 7
        return s[i:s.find('"', i)]

    with open(build_dir.join('test.a1'), 'rt') as f:
        f1 = f.read()
        hash1 = _get_hash(f1)
    assert re.match(r'^[0-9a-f]{6,}$', hash1), f1

    premium_exe = __copy_cppcheck_premium(tmpdir)
    exitcode, stdout, stderr = cppcheck(args, cppcheck_exe=premium_exe)
    assert 'error' not in stdout
    assert stderr == ''
    assert exitcode == 0

    with open(build_dir.join('test.a1'), 'rt') as f:
        f2 = f.read()
        hash2 = _get_hash(f2)
    assert re.match(r'^[0-9a-f]{6,}$', hash2), f2

    assert hash1 != hash2


def test_misra_py(tmpdir):
    # 13831 - do not execute misra.py when --premium=misra-c-2012 is used
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('void foo();\n')

    exe = __copy_cppcheck_premium(tmpdir)

    # ensure that misra.py is not available:
    _, stdout, _ = cppcheck(['--enable=style', '--addon=misra', test_file], cppcheck_exe=exe)
    assert 'Did not find addon misra.py' in stdout

    # Execute misra
    _, stdout, _ = cppcheck(['--enable=style', '--premium=misra-c-2012', test_file], cppcheck_exe=exe)
    assert 'misra.py' not in stdout # Did not find misra.py
    assert 'Checking' in stdout


def test_invalid_license_retry(tmpdir):
    # Trac 13832 - cppcheck build dir: do not reuse cached results if there were invalidLicense errors
    build_dir = os.path.join(tmpdir, 'b')
    test_file = os.path.join(tmpdir, 'test.c')
    addon_file = os.path.join(tmpdir, 'premiumaddon.py')

    os.mkdir(build_dir)

    with open(test_file, 'wt') as f:
        f.write('void foo();\n')

    args = [f"--addon={addon_file}", f"--cppcheck-build-dir={build_dir}", '--xml', '--enable=all', test_file]

    with open(addon_file, 'wt') as f:
        f.write('print(\'{"addon":"premium","column":0,"errorId":"invalidLicense","extra":"","file":"Cppcheck Premium","linenr":0,"message":"Invalid license: No license file was found, contact sales@cppchecksolutions.com","severity":"error"}\')')

    _, _, stderr = cppcheck(args)
    assert 'Invalid license' in stderr

    with open(addon_file, 'wt') as f:
        f.write('')

    _, _, stderr = cppcheck(args)
    assert 'Invalid license' not in stderr
