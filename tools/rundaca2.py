#!/usr/bin/python

import subprocess
import pexpect
import os
import shutil
import time
import sys

START = 0
PASSWORD = ''
for arg in sys.argv[1:]:
    if len(arg) == 1:
        START = '0123456789abcdefghijklmnopqrstuvwxyz'.find(arg)
        if START < 0:
            START = 0
    else:
        PASSWORD = arg



def compilecppcheck(CPPFLAGS):
    subprocess.call(['nice', 'make', 'clean'])
    subprocess.call(['nice', 'make', 'SRCDIR=build', 'CFGDIR=' + os.path.expanduser('~/cppcheck/cfg'), 'CXXFLAGS=-g -O2', 'CPPFLAGS=' + CPPFLAGS])
    subprocess.call(['cp', 'cppcheck', os.path.expanduser('~/daca2/cppcheck-O2')])

def runcppcheck(rev, folder, destpath):
    subprocess.call(['rm', '-rf', os.path.expanduser('~/daca2/' + folder)])
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2.py'), folder, '--rev=' + rev, '--skip=virtuoso-opensource'])
    subprocess.call(['rm', '-rf', destpath + folder])
    subprocess.call(['cp', '-R', os.path.expanduser('~/daca2/' + folder), destpath + folder])
    subprocess.call(['cp', '', os.path.expanduser('~/daca2/' + folder)])
    upload(os.path.expanduser('~/daca2/' + folder + '/results.txt'), 'evidente/results-' + folder + '.txt')
    subprocess.call(['rm', '-rf', os.path.expanduser('~/daca2/lib' + folder)])
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2.py'), 'lib' + folder, '--rev=' + rev])
    upload(os.path.expanduser('~/daca2/lib' + folder + '/results.txt'), 'evidente/results-lib' + folder + '.txt')

def daca2report(daca2folder):
    subprocess.call(['rm', '-rf', os.path.expanduser('~/daca2-report')])
    subprocess.call(['mkdir', os.path.expanduser('~/daca2-report')])
    subprocess.call(['python', os.path.expanduser('~/cppcheck/tools/daca2-report.py'), '--daca2='+daca2folder, os.path.expanduser('~/daca2-report')])

# Upload file to sourceforge server using scp
def upload(folder):
    try:
        child = pexpect.spawn(
            'scp -r ' + os.path.expanduser('~/daca2-report') + ' danmar,cppcheck@web.sf.net:htdocs/devinfo/' + folder)
        child.expect('upload@trac.cppcheck.net\'s password:')
        child.sendline(PASSWORD)
        child.interact()
    except IOError:
        pass
    except OSError:
        pass
    except pexpect.TIMEOUT:
        pass


def daca2(foldernum):
    folders = '0123456789abcdefghijklmnopqrstuvwxyz'
    folder = folders[foldernum % len(folders)]

    print('Daca2 folder=' + folder)

    os.chdir(os.path.expanduser('~/cppcheck'))
    #subprocess.call(['git', 'pull'])
    p = subprocess.Popen(['git', 'show', '--format=%h'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    rev = comm[0]
    rev = rev[:rev.find('\n')]

    # unstable
    compilecppcheck('-DMAXTIME=600 -DUNSTABLE')
    runcppcheck(rev, folder, os.path.expanduser('~/daca2-unstable/'))
    daca2report(os.path.expanduser('~/daca2-unstable/'))
    #upload('daca2-unstable')

    # stable
    compilecppcheck('-DMAXTIME=600')
    runcppcheck(rev, folder, os.path.expanduser('~/daca2-stable/'))
    daca2report(os.path.expanduser('~/daca2-stable/'))
    #upload('daca2-stable')

foldernum = START
while True:
    daca2(foldernum)
    foldernum = foldernum + 1
