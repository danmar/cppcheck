#!/usr/bin/python
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-O2 in daca2. It should be built with all optimisations.
# 3. Edit this line:   FOLDER = 'a'
# 4. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 5. Run the daca2 script:  python daca2.py

import ftplib
import subprocess
import sys
import shutil
import glob
import os
import socket

FTPSERVER = 'ftp.sunet.se'
FTPPATH = '/pub/Linux/distributions/Debian/debian/pool/main/'
FOLDER = 'b'


def removeAllExceptResults():
    filenames = glob.glob('[A-Za-z]*')
    for filename in filenames:
        if os.path.isdir(filename):
            shutil.rmtree(filename)
        elif filename != 'results.txt':
            os.remove(filename)

workdir = os.path.expanduser('~/daca2/')

print('~/daca2/suppressions.txt')
if not os.path.isfile(workdir + 'suppressions.txt'):
    suppressions = open(workdir + 'suppressions.txt', 'wt')
    suppressions.write('\n')
    suppressions.close()

print('~/daca2/' + FOLDER)
if not os.path.isdir(workdir + FOLDER):
    os.makedirs(workdir + FOLDER)
os.chdir(workdir + FOLDER)
if os.path.isfile('results.txt'):
    os.remove('results.txt')

print('Connect to ' + FTPSERVER)
f = ftplib.FTP(FTPSERVER)
f.login()

print('Get package list in folder ' + FOLDER)
packages = f.nlst(FTPPATH + FOLDER)

for package in packages:
    print('package:' + package)
    filename = None
    path = FTPPATH + FOLDER + '/' + package
    try:
        for s in f.nlst(path):
            if s[-12:] == '.orig.tar.gz':
                filename = s
    except socket.error:
        pass
    except ftplib.error_temp:
        pass

    if filename:
        fullpath = 'ftp://ftp.sunet.se' + path + '/' + filename
        subprocess.call(['wget', fullpath])
        subprocess.call(['tar', 'xzvf', filename])
        subprocess.call(['rm', filename])

        dirname = None
        for s in glob.glob(filename[:2] + '*'):
            if os.path.isdir(s):
                dirname = s
        if dirname is None:
            continue

        print('cppcheck "' + dirname + '"')
        p = subprocess.Popen(
            ['nice',
             '../cppcheck-O2',
             '-j2',
             '-D__GCC__',
             '--enable=style',
             '--suppressions-list=../suppressions.txt',
             dirname],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
        comm = p.communicate()

        results = open('results.txt', 'at')
        results.write(fullpath + '\n')
        results.write(comm[1] + '\n')
        results.close()

        # remove all files/folders except results.txt
        removeAllExceptResults()
