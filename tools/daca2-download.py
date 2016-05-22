#!/usr/bin/python
#
# Downloads all daca2 source code packages.
#
# Usage:
# $ mkdir ~/daca2-packages && python daca2-download.py


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


def getpackages():
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
        elif line[:12] == './pool/main/':
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


def removeAll():
    count = 5
    while count > 0:
        count = count - 1

        filenames = []
        for g in glob.glob('[#_A-Za-z0-9]*'):
            filenames.append(g)
        for g in glob.glob('.[A-Za-z]*'):
            filenames.append(g)

        try:
            for filename in filenames:
                if os.path.isdir(filename):
                    shutil.rmtree(filename, onerror=handleRemoveReadonly)
                else:
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
        elif os.path.isfile(g):
            # remove large files
            statinfo = os.stat(g)
            if statinfo.st_size > 100000:
                os.remove(g)

            # remove non-source files
            elif g[-2:] != '.C' and g[-2:] != '.c' and g[-4:] != '.cc' and g[-4:] != '.cpp' and g[-4:] != '.cxx' and g[-2:] != '.h' and g[-2:] != '.H' and g[-4:] != '.c++' and g[-4:] != '.hpp' and g[-4:] != '.tpp' and g[-4:] != '.t++':
                os.remove(g)


def downloadpackage(filepath, outpath):
    # remove all files/folders
    removeAll()

    if not wget(filepath):
        print('Failed to download ' + filepath)
        return

    filename = filepath[filepath.rfind('/') + 1:]
    if filename[-3:] == '.gz':
        subprocess.call(['tar', 'xzvf', filename])
    elif filename[-3:] == '.xz':
        subprocess.call(['tar', 'xJvf', filename])
    elif filename[-4:] == '.bz2':
        subprocess.call(['tar', 'xjvf', filename])
    else:
        return

    removeLargeFiles('')

    for g in glob.glob('[#_A-Za-z0-9]*'):
        if os.path.isdir(g):
            subprocess.call(['tar', '-cJvf', outpath + filename[:filename.rfind('.')] + '.xz', g])
            break

workdir = os.path.expanduser('~/daca2-packages/tmp/')
if not os.path.isdir(workdir):
    os.makedirs(workdir)
os.chdir(workdir)

packages = getpackages()
if len(packages) == 0:
    print('failed to load packages')
    sys.exit(1)

print('Sleep for 10 seconds..')
time.sleep(10)

for package in packages:
    downloadpackage(package, os.path.expanduser('~/daca2-packages/'))

# remove all files/folders
removeAll()
