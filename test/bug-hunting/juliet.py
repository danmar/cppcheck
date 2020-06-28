# Test if --bug-hunting works using the juliet testsuite
# The Juliet test suite can be downloaded from:
# https://samate.nist.gov/SRD/testsuite.php

import glob
import os
import re
import shutil
import sys
import subprocess

JULIET_PATH = os.path.expanduser('~/juliet')
if sys.argv[0] in ('test/bug-hunting/juliet.py', './test/bug-hunting/juliet.py'):
    CPPCHECK_PATH = './cppcheck'
else:
    CPPCHECK_PATH = '../../cppcheck'

RUN_CLANG = ('--clang' in sys.argv)

def get_files(juliet_path:str, test_cases:str):
    ret = []
    g = os.path.join(juliet_path, test_cases)
    print(g)
    for f in sorted(glob.glob(g)):
        res = re.match(r'(.*[0-9][0-9])[a-x]?.(cp*)$', f)
        if res is None:
            print('Non-match!! ' + f)
            sys.exit(1)
        f = res.group(1) + '*.' + res.group(2)
        if f not in ret:
            ret.append(f)
    return ret


def check(tc:str, warning_id:str):
    num_ok = 0
    num_failed = 0

    for f in get_files(JULIET_PATH, tc):
        cmd = [CPPCHECK_PATH,
               '-I' + os.path.join(JULIET_PATH, 'C/testcasesupport'),
               '-DOMIT_GOOD',
               '-DAF_INET=1',
               '-DINADDR_ANY=1',
               '--library=posix',
               '--bug-hunting',
               '--platform=unix64']
        if RUN_CLANG:
            cmd += ['--clang', '--cppcheck-build-dir=juliet-build-dir']
            if not os.path.isdir('juliet-build-dir'):
                os.mkdir('juliet-build-dir')
        cmd += glob.glob(f)
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        comm = p.communicate()
        stdout = comm[0].decode(encoding='utf-8', errors='ignore')
        stderr = comm[1].decode(encoding='utf-8', errors='ignore')
        if RUN_CLANG:
            shutil.rmtree('juliet-build-dir')

        if warning_id in stderr:
            num_ok += 1
        else:
            print('fail: ' + ' '.join(cmd))
            num_failed += 1

    cwepos = tc.find('CWE')
    cwe = tc[cwepos:cwepos+6]

    print('%s ok:%i, fail:%i' % (cwe, num_ok, num_failed))
    if num_failed != 0:
        sys.exit(1)


check('C/testcases/CWE369_Divide_by_Zero/s*/*.c', 'bughuntingDivByZero')
#check('C/testcases/CWE457_Use_of_Uninitialized_Variable/s*/*.c', 'bughuntingUninit')


