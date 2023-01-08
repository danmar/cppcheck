#!/usr/bin/env python3
# Times script using Visual Studio compiler in Windows
#
# This script assumes that you have:
# Python 3
# Visual Studio (script assumes VS2013, manipulate the sed command otherwise)
# Cygwin64 for the sed command
# Command line svn. TortoiseSVN with that feature selected works.
#
# Usage:
# Open VS command prompt.
# cd c:\users\...
# svn checkout https://github.com/danmar/cppcheck/trunk cppcheck-svn
# cd cppcheck-svn
# c:\python34\python.exe times-vs.py rev1:rev2


import subprocess
import glob
import re
import sys

if len(sys.argv) != 2:
    print('revisions not specified')
    sys.exit(1)

res = re.match(r'([0-9]+):([0-9]+)', sys.argv[1])
if res is None:
    print('invalid format, 11111:22222')
    sys.exit(1)

rev1 = int(res.group(1))
rev2 = int(res.group(2))

if rev1 > rev2 or rev1 < 10000 or rev2 > 20000 or rev2 - rev1 > 500:
    print('range, aborting')
    sys.exit(1)

print('Revisions: ' + str(rev1) + ':' + str(rev2))

f = open('results.txt', 'wt')
f.write('\n')
f.close()

for rev in range(rev1, rev2):
    subprocess.call(['svn', 'revert', '-R',  '.'])
    subprocess.call(['svn', 'up', '-r' + str(rev)])
    for vcxproj in glob.glob('*/*.vcxproj'):
        subprocess.call([r'c:\cygwin64\bin\sed.exe', '-i', 's/140/120/', vcxproj])
    subprocess.call('msbuild cppcheck.sln /t:build /p:configuration=Release,platform=x64'.split())
    print('Revision:' + str(rev))
    p = subprocess.Popen(r'bin\cppcheck.exe src -q --showtime=summary'.split(),
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    f = open('results.txt', 'at')
    f.write('\nREV ' + str(rev) + '\n')
    f.write(comm[0].decode('utf-8'))
    f.close()
