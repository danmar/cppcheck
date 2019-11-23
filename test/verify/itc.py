# Test if --verify works using the itc testsuite
# The itc test suite can be downloaded here:
# https://github.com/regehr/itc-benchmarks


import glob
import os
import re
import sys
import subprocess

ITC_PATH = os.path.expanduser('~/testing')
ZERO_DIVISION = '000/199/329/zero_division.c'

if sys.argv[0] in ('test/verify/itc.py', './test/verify/itc.py'):
    CPPCHECK_PATH = './cppcheck'
else:
    CPPCHECK_PATH = '../../cppcheck'

def get_error_lines(filename):
    ret = []
    f = open(filename, 'rt')
    lines = f.readlines()
    for linenr, line in enumerate(lines):
        if line.find('/* ERROR:') > 0:
            ret.append(linenr+1)
    return ret

def check(filename):
    cmd = [CPPCHECK_PATH,
           '--verify',
           '--platform=unix64',
           filename]
    print(' '.join(cmd))

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')

    ret = []
    for line in stderr.split('\n'):
        res = re.match(r'.*zero_division.c:([0-9]+):[0-9]+: error: There is division.*', line)
        if res is None:
            continue
        ret.append(int(res.group(1)))
    return ret

filename = os.path.join(ITC_PATH, ZERO_DIVISION)
wanted = get_error_lines(filename)
actual = check(filename)
print('wanted:' + str(wanted))
print('actual:' + str(actual))
missing = []
for w in wanted:
    if w not in actual:
        missing.append(w);
print('missing:' + str(missing))



