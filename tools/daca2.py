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
    filenames = []
    for g in glob.glob('[A-Za-z0-9]*'):
        filenames.append(g)
    for g in glob.glob('.[a-z]*'):
        filenames.append(g)

    for filename in filenames:
        count = 5
        while count > 0:
            count = count - 1

            try:
                if os.path.isdir(filename):
                    shutil.rmtree(filename, onerror=handleRemoveReadonly)
                elif filename != 'results.txt':
                    os.remove(filename)
                break
            except WindowsError as err:
                time.sleep(30)
                if count == 0:
                    f = open('results.txt','at')
                    f.write('Failed to cleanup ' + filename + ': ' + str(err))
                    f.close()
            except OSError as err:
                time.sleep(30)
                if count == 0:
                    f = open('results.txt','at')
                    f.write('Failed to cleanup ' + filename + ': ' + str(err))
                    f.close()


def removeLargeFiles(path):
    for g in glob.glob(path + '*'):
        if g == '.' or g == '..':
            continue
        if os.path.islink(g):
            continue
        if os.path.isdir(g):
            # Remove test code
            if g.endswith('/testsuite') or g.endswith('/clang/INPUTS'):
                shutil.rmtree(g, onerror=handleRemoveReadonly)
            else:
                removeLargeFiles(g + '/')
        elif os.path.isfile(g) and g[-4:] != '.txt':
            statinfo = os.stat(g)
            if statinfo.st_size > 1000000:
                try:
                    os.remove(g)
                except OSError as err:
                    f = open('results.txt','at')
                    f.write('Failed to remove ' + g + ': ' + str(err))
                    f.close()

def strfCurrTime(fmt):
    return datetime.time.strftime(datetime.datetime.now().time(), fmt)

def scanarchive(filepath, jobs, cpulimit):
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

    removeLargeFiles('')

    print(strfCurrTime('[%H:%M] cppcheck ') + filename)

    cmd = '../cppcheck-O2 -D__GCC__ --enable=style --error-exitcode=0 --exception-handling=stderr ' + jobs + ' .'
    if cpulimit:
        cmd = 'cpulimit --limit=' + cpulimit + ' ' + cmd
    else:
        cmd = 'nice --adjustment=1000 ' + cmd

    p = subprocess.Popen(cmd.split(' '), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()

    results = open('results.txt', 'at')
    if p.returncode == 0:
        results.write(comm[1] + strfCurrTime('[%H:%M]') + '\n')
    elif comm[0].find('cppcheck: error: could not find or open any of the paths given.') < 0:
        results.write(comm[1] + strfCurrTime('[%H:%M]') + '\n')
        results.write('Exit code is not zero! Crash?\n')
    results.write('\n')
    results.close()

FOLDER = None
JOBS = '-j1'
REV = None
SKIP = []
WORKDIR = os.path.expanduser('~/daca2')
CPULIMIT = None
for arg in sys.argv[1:]:
    if arg[:6] == '--rev=':
        REV = arg[6:]
    elif arg[:2] == '-j':
        JOBS = arg
    elif arg.startswith('--skip='):
        SKIP.append(arg[7:])
    elif arg.startswith('--workdir='):
        WORKDIR = arg[10:]
    elif arg.startswith('--cpulimit='):
        CPULIMIT = arg[11:]
    else:
        FOLDER = arg

if not FOLDER:
    print('no folder given')
    sys.exit(1)

if not os.path.isdir(WORKDIR):
    print('workdir \'' + WORKDIR + '\' is not a folder')
    sys.exit(1)

archives = getpackages(FOLDER)
if len(archives) == 0:
    print('failed to load packages')
    sys.exit(1)

if not WORKDIR.endswith('/'):
    WORKDIR = WORKDIR + '/'

print('~/daca2/' + FOLDER)
if not os.path.isdir(WORKDIR + FOLDER):
    os.makedirs(WORKDIR + FOLDER)
os.chdir(WORKDIR + FOLDER)

try:
    results = open('results.txt', 'wt')
    results.write('STARTDATE ' + str(datetime.date.today()) + '\n')
    results.write('STARTTIME ' + strfCurrTime('%H:%M:%S') + '\n')
    if REV:
        results.write('GIT-REVISION ' + REV + '\n')
    results.write('\n')
    results.close()

    for archive in archives:
        if len(SKIP) > 0:
            a = archive[:archive.rfind('/')]
            a = a[a.rfind('/')+1:]
            if a in SKIP:
                continue
        scanarchive(archive, JOBS, CPULIMIT)

    results = open('results.txt', 'at')
    results.write('DATE ' + str(datetime.date.today()) + '\n')
    results.write('TIME ' + strfCurrTime('%H:%M:%S') + '\n')
    results.close()

except EOFError:
    pass

# remove all files/folders except results.txt
removeAllExceptResults()
