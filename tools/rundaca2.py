#!/usr/bin/env python

import subprocess
import pexpect
import os
import sys
import time

def compilecppcheck(CPPFLAGS):
    subprocess.call(['nice', 'make', 'clean'])
    subprocess.call(['nice', 'make', 'SRCDIR=build', 'CFGDIR=' +
                    os.path.expanduser('~/cppcheck/cfg'), 'CXXFLAGS=-g -O2', 'CPPFLAGS=' + CPPFLAGS])
    subprocess.call(['cp', 'cppcheck', os.path.expanduser('~/daca2/cppcheck-head')])


def runcppcheck(rev, folder):
    subprocess.call(['rm', '-rf', os.path.expanduser('~/daca2/' + folder)])
    subprocess.call(['nice', '--adjustment=19', 'python',
                    os.path.expanduser('~/cppcheck/tools/daca2.py'), folder, '--rev=' + rev,
                    '--baseversion=1.84', '--skip=hashdeep', '--skip=lice'])


def daca2report(reportfolder):
    subprocess.call(['rm', '-rf', reportfolder])
    subprocess.call(['mkdir', reportfolder])
    subprocess.call(['python', os.path.expanduser('~/cppcheck/tools/daca2-report.py'), reportfolder])


# Upload file to sourceforge server using scp
def upload(localfolder, webfolder, password):
    if len(password) < 3:
        return
    tries = 1
    while tries <= 5:
        try:
            child = pexpect.spawn(
                'scp -r ' + localfolder + ' danielmarjamaki,cppcheck@web.sf.net:htdocs/' + webfolder)
            # child.expect('upload@trac.cppcheck.net\'s password:')
            child.expect('Password:')
            child.sendline(password)
            child.interact()
            return
        except (IOError, OSError, pexpect.TIMEOUT, pexpect.EOF):
            print('rundaca2.py: Upload failed. Sleep for 10 seconds..')
            time.sleep(10)
            tries = tries + 1


def daca2(foldernum, password):
    folders = '0123456789abcdefghijklmnopqrstuvwxyz'
    folder = folders[foldernum % len(folders)]

    print('Daca2 folder=' + folder)

    os.chdir(os.path.expanduser('~/cppcheck'))
    subprocess.call(['git', 'pull'])
    p = subprocess.Popen(['git', 'show', '--format=%h'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    rev = comm[0]
    rev = rev[:rev.find('\n')]

    print('rundaca2.py: compile cppcheck')
    compilecppcheck('-DMAXTIME=600 -DDACA2')

    print('rundaca2.py: runcppcheck')
    runcppcheck(rev, folder)
    runcppcheck(rev, 'lib' + folder)

    print('rundaca2.py: daca2 report')
    daca2report(os.path.expanduser('~/daca2-report'))

    print('rundaca2.py: upload')
    upload(os.path.expanduser('~/daca2-report'), 'devinfo/', password)

foldernum = 0
for arg in sys.argv[1:]:
    if len(arg) == 1:
        folderNum = '0123456789abcdefghijklmnopqrstuvwxyz'.find(arg)
        if folderNum < 0:
            folderNum = 0

print('enter password:')
password = sys.stdin.readline().rstrip()
while True:
    daca2(foldernum, password)
    foldernum = foldernum + 1

