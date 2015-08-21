#!/usr/bin/python

import subprocess
import pexpect
import os
import shutil
import time
import sys

PASSWORD = ''
if len(sys.argv) == 2:
    PASSWORD = sys.argv[1]

# Upload file to sourceforge web server using scp


def upload(file_to_upload, destination):
    if not os.path.isfile(file_to_upload):
        return

    try:
        child = pexpect.spawn(
            'scp ' + file_to_upload + ' upload@trac.cppcheck.net:' + destination)
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
    subprocess.call(['git', 'pull'])
    p = subprocess.Popen(['git', 'show', '--format=%h'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    rev = comm[0]
    rev = rev[:rev.find('\n')]

    # compile cppcheck
    subprocess.call(['nice', 'make', 'SRCDIR=build', 'CFGDIR=' + os.path.expanduser('~/cppcheck/cfg'), 'CXXFLAGS=-O2', 'CPPFLAGS=-DMAXTIME=600'])
    subprocess.call(['mv', 'cppcheck', os.path.expanduser('~/daca2/cppcheck-O2')])

    # run cppcheck
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2.py'), folder, '--rev=' + rev])
    upload(os.path.expanduser('~/daca2/' + folder + '/results.txt'), 'evidente/results-' + folder + '.txt')
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2.py'), 'lib' + folder, '--rev=' + rev])
    upload(os.path.expanduser('~/daca2/lib' + folder + '/results.txt'), 'evidente/results-lib' + folder + '.txt')

    # run cppcheck addons
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2-addons.py'), folder, '--rev=' + rev])
    upload(os.path.expanduser('~/daca2/'+folder+'/results.txt'), 'evidente/addons-'+folder+'.txt')
    subprocess.call(['nice', '--adjustment=19', 'python', os.path.expanduser('~/cppcheck/tools/daca2-addons.py'), 'lib' + folder, '--rev=' + rev])
    upload(os.path.expanduser('~/daca2/lib'+folder+'/results.txt'), 'evidente/addons-lib'+folder+'.txt')

foldernum = 0
while True:
    daca2(foldernum)
    foldernum = foldernum + 1


