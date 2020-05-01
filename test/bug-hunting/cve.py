# Test if --bug-hunting works using cve tests

import glob
import os
import re
import shutil
import sys
import subprocess

if sys.argv[0] in ('test/bug-hunting/cve.py', './test/bug-hunting/cve.py'):
    CPPCHECK_PATH = './cppcheck'
    TEST_SUITE = 'test/bug-hunting/cve'
else:
    CPPCHECK_PATH = '../../cppcheck'
    TEST_SUITE = 'cve'

RUN_CLANG = ('--clang' in sys.argv)

def check():
    cmd = [CPPCHECK_PATH,
           '-D_GNUC',
           '--bug-hunting',
           '--platform=unix64',
           '--inline-suppr',
           '--enable=information',
           TEST_SUITE]
    if RUN_CLANG:
        cmd.append('--clang')
    print(' '.join(cmd))

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')

    # Ensure there are no unmatched suppressions
    if '[unmatchedSuppression]' in stderr:
        print('FAILED: There are unmatched suppressions')
        sys.exit(1)
    else:
        print('SUCCESS')


check()
