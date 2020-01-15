# Test if --verify works using the itc testsuite
# The itc test suite can be downloaded here:
# https://github.com/regehr/itc-benchmarks


import glob
import os
import re
import sys
import subprocess

if sys.argv[0] in ('test/verify/itc.py', './test/verify/itc.py'):
    CPPCHECK_PATH = './cppcheck'
else:
    CPPCHECK_PATH = '../../cppcheck'

if len(sys.argv) == 2:
    TESTFILE = sys.argv[1]
    if not os.path.isfile(TESTFILE):
        print(f'ERROR: {TESTFILE} is not a file')
        sys.exit(1)
else:
    TESTFILE = os.path.expanduser('~/itc/01.w_Defects/zero_division.c')

def get_error_lines(filename):
    ret = []
    f = open(filename, 'rt')
    lines = f.readlines()
    for linenr, line in enumerate(lines):
        if line.find('/* ERROR:') > 0 or line.find('/*ERROR:') > 0:
            ret.append(linenr+1)
    return ret

def check(filename):
    cmd = [CPPCHECK_PATH,
           '--bug-hunting',
           '--platform=unix64',
           filename]
    print(' '.join(cmd))

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')

    w = r'.*zero_division.c:([0-9]+):[0-9]+: error: There is division.*'
    if TESTFILE.find('uninit_') > 0:
        w = r'.*c:([0-9]+):[0-9]+: error: .*verificationUninit.*'

    ret = []
    for line in stderr.split('\n'):
        res = re.match(w, line)
        if res is None:
            continue
        linenr = int(res.group(1))
        if linenr not in ret:
            ret.append(linenr)
    return ret

wanted = get_error_lines(TESTFILE)
actual = check(TESTFILE)
print('wanted:' + str(wanted))
print('actual:' + str(actual))
missing = []
for w in wanted:
    if w not in actual:
        missing.append(w);
print('missing:' + str(missing))



