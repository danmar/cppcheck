#!/usr/bin/python
#
# 1. Create a folder daca2-addons in your HOME folder
# 2. Put cppcheck-O2 in daca2-addons. It should be built with all optimisations.
# 3. Optional: Put a file called "suppressions.txt" in the daca2-addons folder.
# 4. Optional: tweak FTPSERVER and FTPPATH in this script below.
# 5. Run the daca2-addons script:  python daca2-addons.py FOLDER

import subprocess
import sys
import shutil
import glob
import os
import datetime
import time

DEBIAN = ['ftp://ftp.se.debian.org/debian/',
          'ftp://ftp.debian.org/debian/']


def wget(filepath):
    filename = filepath
    if filepath.find('/') >= 0:
        filename = filename[filename.rfind('/') + 1:]
    for d in DEBIAN:
        subprocess.call(
            ['nice', 'wget', '--tries=10', '--timeout=300', '-O', filename, d + filepath])
        if os.path.isfile(filename):
            return True
        print('Sleep for 10 seconds..')
        time.sleep(10)
    return False


def getpackages(folder):
    if not wget('ls-lR.gz'):
        return []
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
        except WindowsError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup files/folders')
                print(err)
                sys.exit(1)
            continue
        except OSError as err:
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
        if os.path.islink(g):
            continue
        if os.path.isdir(g):
            removeLargeFiles(g + '/')
        elif os.path.isfile(g) and g[-4:] != '.txt':
            statinfo = os.stat(g)
            if path.find('/clang/INPUTS/') > 0 or statinfo.st_size > 100000:
                os.remove(g)


def dumpfiles(path):
    ret = []
    for g in glob.glob(path + '*'):
        if os.path.islink(g):
            continue
        if os.path.isdir(g):
            for df in dumpfiles(path + g + '/'):
                ret.append(df)
        elif os.path.isfile(g) and g[-5:] == '.dump':
            ret.append(g)
    return ret


def scanarchive(filepath, jobs):
    # remove all files/folders except results.txt
    removeAllExceptResults()

    results = open('results.txt', 'at')
    results.write(DEBIAN[0] + filepath + '\n')
    results.close()

    if not wget(filepath):
        if not wget(filepath):
            results = open('results.txt', 'at')
            results.write('wget failed\n')
            results.close()
            return

    filename = filepath[filepath.rfind('/') + 1:]
    if filename[-3:] == '.gz':
        subprocess.call(['tar', 'xzvf', filename])
    elif filename[-3:] == '.xz':
        subprocess.call(['tar', 'xJvf', filename])
    elif filename[-4:] == '.bz2':
        subprocess.call(['tar', 'xjvf', filename])

#
# List of skipped packages - which trigger known yet unresolved problems with cppcheck.
# The issues on trac (http://trac.cppcheck.net) are given for reference
# boost #3654 (?)
# flite #5975
# insight#5184
# valgrind #6151
# gcc-arm - no ticket. Reproducible timeout in daca2 though as of 1.73/early 2016.
#

    if filename[:5] == 'flite' or filename[:5] == 'boost' or filename[:7] == 'insight' or filename[:8] == 'valgrind' or filename[:7] == 'gcc-arm':
        results = open('results.txt', 'at')
        results.write('fixme: skipped package to avoid hang\n')
        results.close()
        return

    removeLargeFiles('')

    print('cppcheck ' + filename)

    p = subprocess.Popen(
        ['nice',
         '../cppcheck-O2',
         '--dump',
         '-D__GCC__',
         '--enable=style',
         '--error-exitcode=0',
         jobs,
         '.'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    comm = p.communicate()

    results = open('results.txt', 'at')

    addons = sorted(glob.glob(os.path.expanduser('~/cppcheck/addons/*.py')))
    for dumpfile in sorted(dumpfiles('')):
        for addon in addons:
            if addon.find('cppcheckdata.py') > 0:
                continue

            p2 = subprocess.Popen(['nice',
                                   'python',
                                   addon,
                                   dumpfile],
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
            comm = p2.communicate()
            results.write(comm[1])
    results.close()

FOLDER = None
JOBS = '-j1'
REV = None
for arg in sys.argv[1:]:
    if arg[:6] == '--rev=':
        REV = arg[6:]
    elif arg[:2] == '-j':
        JOBS = arg
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

print('~/daca2/' + FOLDER)
if not os.path.isdir(workdir + FOLDER):
    os.makedirs(workdir + FOLDER)
os.chdir(workdir + FOLDER)

try:
    results = open('results.txt', 'wt')
    results.write('STARTDATE ' + str(datetime.date.today()) + '\n')
    if REV:
        results.write('GIT-REVISION ' + REV + '\n')
    results.write('\n')
    results.close()

    for archive in archives:
        scanarchive(archive, JOBS)

    results = open('results.txt', 'at')
    results.write('DATE ' + str(datetime.date.today()) + '\n')
    results.close()

except EOFError:
    pass

# remove all files/folders except results.txt
removeAllExceptResults()
