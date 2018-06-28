#!/usr/bin/env python
import os
import sys
import re

def parseResults(filename):
    ftp = ''
    warnings = []
    pattern = re.compile(r'.*: (error|warning|style|performance|portability):.* \[([a-zA-Z0-9_\\-]+)\]')
    for line in open(filename, 'rt'):
        line = line.strip('\r\n')
        if line.startswith('ftp://'):
            ftp = line
            continue
        if pattern.match(line):
            warnings.append(ftp + '\n' + line)
    return warnings

def getUnique(warnings1, warnings2):
    ret = ''
    for w in warnings1:
        if w not in warnings2:
            ret = ret + w + '\n'
    return ret

daca2folder = os.path.expanduser('~/daca2/')
reportpath = ''
for arg in sys.argv[1:]:
    if arg.startswith('--daca2='):
        daca2folder = arg[8:]
        if daca2folder[-1] != '/':
            daca2folder += '/'
    else:
        reportpath = arg
        if reportpath[-1] != '/':
            reportpath += '/'

warnings_base = []
warnings_head = []

for lib in ['', 'lib']:
    for a in "0123456789abcdefghijklmnopqrstuvwxyz":
        if not os.path.isfile(daca2folder + lib + a + '/results-1.84.txt'):
            continue
        if not os.path.isfile(daca2folder + lib + a + '/results-head.txt'):
            continue

        warnings_base.extend(parseResults(daca2folder + lib + a + '/results-1.84.txt'))
        warnings_head.extend(parseResults(daca2folder + lib + a + '/results-head.txt'))

f = open(reportpath + 'diff-1.84.txt', 'wt')
f.write(getUnique(warnings_base, warnings_head))
f.close()

f = open(reportpath + 'diff-head.txt', 'wt')
f.write(getUnique(warnings_head, warnings_base))
f.close()
