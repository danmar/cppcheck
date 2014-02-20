#!/usr/bin/python

# amazon web services script

import subprocess
import os
import urllib


def wget(url):
    try:
        fp = urllib.urlopen(url)
        data = fp.read()
        return data
    except IOError:
        pass
    return ''

# Perform a git pull.


def gitpull():
    try:
        subprocess.call(['git', 'pull'])

    except IOError:
        pass
    except OSError:
        pass

    return False


def daca2(foldernum):
    folders = '0123456789abcdefghijklmnopqrstuvwxyz'
    folder = folders[foldernum % len(folders)]
    if (foldernum / len(folders)) % 2 == 1:
        folder = 'lib' + folder

    print('Daca2 folder=' + folder)

    p = subprocess.Popen(['git', 'show', '--format=%h'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    rev = comm[0]
    rev = rev[:rev.find('\n')]

    gitpull()

    subprocess.call(
        ['nice', 'make', 'SRCDIR=build', 'CXXFLAGS=-O2', 'CPPFLAGS=-DMAXTIME=600'])
    subprocess.call(
        ['mv', 'cppcheck', os.path.expanduser('~/daca2/cppcheck-O2')])
    subprocess.call(
        ['cp', 'cfg/std.cfg', os.path.expanduser('~/daca2/')])

    subprocess.call(['python', 'tools/daca2.py', folder, '--rev=' + rev])

    subprocess.call(['cp',
                     os.path.expanduser('~/daca2/' + folder + '/results.txt'),
                     os.path.expanduser('~/daca2/results-' + folder + '.txt')])

subprocess.call(['make', 'clean'])
foldernum = 0
while True:
    daca2(foldernum)
    foldernum = foldernum + 1
