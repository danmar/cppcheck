#!/usr/bin/python
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-O2 in daca2. It should be built with all optimisations.
# 3. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 4. Optional: tweak FTPSERVER and FTPPATH in this script below.
# 5. Run the daca2 script:  python daca2.py FOLDER

import subprocess
import sys
import shutil
import glob
import os
import datetime
import time

FTPSERVER = 'ftp.sunet.se'
FTPPATH = '/pub/Linux/distributions/Debian/debian/'


def getpackages(folder):
    subprocess.call(
        ['nice', 'wget', 'ftp://' + FTPSERVER + FTPPATH + 'ls-lR.gz'])
    subprocess.call(['nice', 'gunzip', 'ls-lR.gz'])
    f = open('ls-lR', 'rt')
    lines = f.readlines()
    f.close()
    subprocess.call(['rm', 'ls-lR'])

    path = None
    archives = []
    filename = None
    for line in lines:
        line = line.strip()
        if len(line) < 4:
            if filename:
                archives.append(path + '/' + filename)
            path = None
            filename = None
        elif line[:13 + len(folder)] == './pool/main/' + folder + '/':
            path = line[2:-1]
        elif path and line.find('.orig.tar.') > 0:
            filename = line[1 + line.rfind(' '):]

    for a in archives:
        print(a)

    return archives


def handleRemoveReadonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise


def removeAllExceptResults():
    count = 5
    while count > 0:
        count = count - 1

        filenames = []
        for g in glob.glob('[A-Za-z0-9]*'):
            filenames.append(g)
        for g in glob.glob('.[a-z]*'):
            filenames.append(g)

        try:
            for filename in filenames:
                if os.path.isdir(filename):
                    shutil.rmtree(filename, onerror=handleRemoveReadonly)
                elif filename != 'results.txt':
                    os.remove(filename)
        except WindowsError, err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup files/folders')
                print(err)
                sys.exit(1)
            continue
        except OSError, err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup files/folders')
                print(err)
                sys.exit(1)
            continue
        count = 0


def removeLargeFiles(path):
    for g in glob.glob(path + '*'):
        if g == '.' or g == '..':
            continue
        if os.path.isdir(g):
            removeLargeFiles(g + '/')
        elif g != 'results.txt':
            statinfo = os.stat(g)
            if statinfo.st_size > 100000:
                os.remove(g)


def scanarchive(fullpath):
    results = open('results.txt', 'at')
    results.write(fullpath + '\n')
    results.close()

    filename = fullpath[fullpath.rfind('/') + 1:]
    subprocess.call(['nice', 'wget', fullpath])
    if not os.path.isfile(filename):
        removeAllExceptResults()
        os.remove(filename)
        sys.exit(1)

    if filename[-3:] == '.gz':
        subprocess.call(['tar', 'xzvf', filename])
    elif filename[-3:] == '.xz':
        subprocess.call(['tar', 'xJvf', filename])
    elif filename[-4:] == '.bz2':
        subprocess.call(['tar', 'xjvf', filename])

    if filename[:5] == 'flite':
        results = open('results.txt', 'at')
        results.write('fixme: this package is skipped\n')
        results.close()
        return

    dirname = None
    for s in glob.glob(filename[:2] + '*'):
        if os.path.isdir(s):
            dirname = s
    if dirname is None:
        return

    removeLargeFiles('')

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

FOLDER = None
REV = None
for arg in sys.argv[1:]:
    if arg[:6] == '--rev=':
        REV = arg[6:]
    else:
        FOLDER = arg

if not FOLDER:
    print('no folder given')
    sys.exit(1)

archives = getpackages(FOLDER)
if len(archives) == 0:
    print('failed to load packages')
    sys.exit(1)

print('Sleep for 10 seconds..')
time.sleep(10)

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
    results.write('DATE ' + str(datetime.date.today()) + '\n')
    if REV:
        results.write('GIT-REVISION ' + REV + '\n')
    results.write('\n')
    results.close()

    for archive in archives:
        # remove all files/folders except results.txt
        removeAllExceptResults()

        scanarchive('ftp://' + FTPSERVER + FTPPATH + archive)

except EOFError:
    pass

# remove all files/folders except results.txt
removeAllExceptResults()
