# Test if --bug-hunting works using the itc testsuite
# The itc test suite can be downloaded here:
# https://github.com/regehr/itc-benchmarks


import os
import re
import shutil
import sys
import subprocess

if sys.argv[0] in ('test/bug-hunting/itc.py', './test/bug-hunting/itc.py'):
    CPPCHECK_PATH = './cppcheck'
else:
    CPPCHECK_PATH = '../../cppcheck'

if len(sys.argv) >= 2 and sys.argv[-1] != '--clang':
    TESTFILES = [sys.argv[-1]]
else:
    TESTFILES = [os.path.expanduser('~/itc/01.w_Defects/zero_division.c'),
                 os.path.expanduser('~/itc/01.w_Defects/uninit_var.c')]
if not os.path.isfile(TESTFILES[0]):
    print('ERROR: %s is not a file' % TESTFILES[0])
    sys.exit(1)

RUN_CLANG = ('--clang' in sys.argv)

def get_error_lines(filename):
    ret = []
    f = open(filename, 'rt')
    lines = f.readlines()
    for linenr, line in enumerate(lines):
        if line.find('/* ERROR:') > 0 or line.find('/*ERROR:') > 0:
            linenr += 1
            if testfile.find('uninit_') >= 0:
                if linenr == 177:
                    linenr = 176
                elif linenr == 241:
                    linenr = 242 # warn about usage
            ret.append(linenr)
    return ret

def check(filename):
    cmd = [CPPCHECK_PATH,
           '--bug-hunting',
           '--bug-hunting-check-function-max-time=10'
           '--platform=unix64',
           filename]
    if RUN_CLANG:
        cmd.append('--clang')
    print(' '.join(cmd))

    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')

    if RUN_CLANG:
        shutil.rmtree('itc-build-dir')

    if filename.find('zero_division.c') >= 0:
        w = r'.*zero_division.c:([0-9]+):[0-9]+: error: There is division.*'
    elif filename.find('uninit_') >= 0:
        w = r'.*c:([0-9]+):[0-9]+: error: .*bughuntingUninit.*'
    else:
        w = r'.*c:([0-9]+):[0-9]+: error: .*bughunting.*'

    ret = []
    for line in stderr.split('\n'):
        res = re.match(w, line)
        if res is None:
            continue
        linenr = int(res.group(1))
        if linenr not in ret:
            ret.append(linenr)
    return ret

for testfile in TESTFILES:
    wanted = get_error_lines(testfile)
    actual = check(testfile)
    missing = []
    for w in wanted:
        if w not in actual:
            missing.append(w)
    if len(missing) > 0:
        print('wanted:' + str(wanted))
        print('actual:' + str(actual))
        print('missing:' + str(missing))
        sys.exit(1)


