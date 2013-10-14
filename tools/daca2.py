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
import datetime
import time

FTPSERVER = 'ftp.sunet.se'
FTPPATH = '/pub/Linux/distributions/Debian/debian/pool/main/'
FOLDER = 'b'

def handleRemoveReadonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise

def removeAllExceptResults():
    filenames = glob.glob('[A-Za-z]*')
    for filename in filenames:
        if os.path.isdir(filename):
            shutil.rmtree(filename, onerror=handleRemoveReadonly)
        elif filename != 'results.txt':
            os.remove(filename)


def scanarchive(fullpath):
    results = open('results.txt', 'at')
    results.write(fullpath + '\n')
    results.close()

    filename = fullpath[fullpath.rfind('/') + 1:]
    subprocess.call(['wget', fullpath])
    if filename[-3:] == '.gz':
        subprocess.call(['tar', 'xzvf', filename])
    elif filename[-4:] == '.bz2':
        subprocess.call(['tar', 'xjvf', filename])

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
    results.write(comm[1] + '\n')
    results.close()

if len(sys.argv) == 2:
    FOLDER = sys.argv[1]

print('Connect to ' + FTPSERVER)
f = ftplib.FTP(FTPSERVER)
f.login()

print('Get package list in folder ' + FOLDER)
packages = f.nlst(FTPPATH + FOLDER)

archives = []
for package in packages:
    filename = None
    path = FTPPATH + FOLDER + '/' + package
    try:
        files = f.nlst(path)

        for s in files:
            if s.find('.orig.tar.') > 0:
                filename = s

        if not filename:
            for s in files:
                if s.find('.tar.') > 0:
                    filename = s
    except socket.error:
        print('socket.error')
        pass
    except ftplib.error_temp:
        print('ftplib.error_temp')
        pass
    except EOFError:
        print('EOFError')
        pass

    if not filename:
        print('archive not found for ' + package)
    else:
        archives.append(package + '/' + filename)

print('Disconnect')
f.quit()

time.sleep(30)

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

try:
    results = open('results.txt', 'wt')
    results.write('DATE ' + str(datetime.date.today()) + '\n\n')
    results.close()

    for archive in archives:
        # remove all files/folders except results.txt
        removeAllExceptResults()

        scanarchive('ftp://' + FTPSERVER + FTPPATH + FOLDER + '/' + archive)

except EOFError:
    pass

# remove all files/folders except results.txt
removeAllExceptResults()
