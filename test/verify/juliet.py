# Test if --verify works using the juliet testsuite


import glob
import os
import re
import sys
import subprocess

JULIET_PATH = os.path.expanduser('~/juliet')
CPPCHECK_PATH = '../../cppcheck'

def get_files(juliet_path:str):
    ret = []
    pattern = 'C/testcases/CWE369_Divide_by_Zero/s01/CWE369_Divide_by_Zero__int_*divide*.c'
    g = os.path.join(juliet_path, pattern)
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

num_ok = 0
num_failed = 0

for f in get_files(JULIET_PATH):
    inc = '-I' + os.path.join(JULIET_PATH, 'C/testcasesupport')
    cmd = f'{CPPCHECK_PATH} {inc} --verify --platform=unix64 ' + ' '.join(glob.glob(f))
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')

    if 'verificationDivByZero' in stderr:
        num_ok += 1
    else:
        print(f'fail: {cmd}')
        #print(stdout)
        num_failed += 1

print(f'ok:{num_ok}, fail:{num_failed}')

