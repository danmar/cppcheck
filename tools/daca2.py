#!/usr/bin/python
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-O2 in daca2. It should be built with all optimisations.
# 3. Choose a folder to check. Choose any that match regexpr: (lib)?[0-9a-z]
# 4. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 5. Optional: tweak FTPSERVER and FTPPATH in this script below.
# 6. Run the daca2 script:  python daca2.py FOLDER

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


def generateDaca2Report(allfolders):
    filename = os.path.expanduser('~/daca2/daca2.html')
    f = open(filename, 'wt')
    f.write('<html>\n')
    f.write('<body>\n')
    f.write('<h1>DACA2</h1>\n')

    f.write('<h2>All folders</h2>\n')
    for folder in allfolders:
        results = os.path.expanduser('~/daca2/' + folder + '/results.txt')
        if os.path.isfile(results):
            f.write(
                '<a href="' +
                folder +
                '/results.txt">' +
                folder +
                '</a><br>\n')
        else:
            f.write(folder + '<br>\n')
    f.write('</body>\n')
    f.write('</html>\n')
    f.close()


def scanpackage(package, f):
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

    if not filename:
        return

    fullpath = 'ftp://' + FTPSERVER + path + '/' + filename
    subprocess.call(['wget', fullpath])
    subprocess.call(['tar', 'xzvf', filename])
    subprocess.call(['rm', filename])

    dirname = None
    for s in glob.glob(filename[:2] + '*'):
        if os.path.isdir(s):
            dirname = s
    if dirname is None:
        return

    print('cppcheck "' + dirname + '"')
    p = subprocess.Popen(
        ['nice',
         '../cppcheck-O2',
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


workdir = os.path.expanduser('~/daca2/')

print('~/daca2/suppressions.txt')
if not os.path.isfile(workdir + 'suppressions.txt'):
    suppressions = open(workdir + 'suppressions.txt', 'wt')
    suppressions.write('\n')
    suppressions.close()

if len(sys.argv) == 2:
    FOLDER = sys.argv[1]

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

try:
    # remove all files/folders except results.txt
    removeAllExceptResults()

    results = open('results.txt', 'wt')
    results.write('DATE ' + str(datetime.date.today()) + '\n\n')
    results.close()

    for package in packages:
        scanpackage(package, f)
except EOFError:
    pass

try:
    generateDaca2Report(f.nlst(FTPPATH))
except socket.error:
    pass
