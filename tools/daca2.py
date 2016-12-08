#!/usr/bin/python
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-O2 in daca2. It should be built with all optimisations.
# 3. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 4. Optional: tweak FTPSERVER and FTPPATH in this script below.
# 5. Run the daca2 script:  python daca2.py FOLDER

import argparse
import subprocess
import sys
import shutil
import glob
import os
import datetime
import time
import logging

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
                elif filename != RESULTS_FILENAME:
                    os.remove(filename)
                break
            except WindowsError as err:
                time.sleep(30)
                if count == 0:
                    logging.error('Failed to cleanup {}: {}'.format(filename, err))
            except OSError as err:
                time.sleep(30)
                if count == 0:
                    logging.error('Failed to cleanup {}: {}'.format(filename, err))


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
                    logging.error('Failed to remove {}: {}'.format(g, err))

def strfCurrTime(fmt):
    return datetime.time.strftime(datetime.datetime.now().time(), fmt)

def scanarchive(filepath, jobs, cpulimit):
    # remove all files/folders except RESULTS_FILENAME
    removeAllExceptResults()

    logging.info(DEBIAN[0] + filepath)

    if not wget(filepath):
        if not wget(filepath):
            logging.error('wget failed at {}', filepath)
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

    if cpulimit:
        cmd = 'cpulimit --limit=' + cpulimit
    else:
        cmd = 'nice --adjustment=1000'
    cmd = cmd + ' ../cppcheck-O2 -D__GCC__ --enable=style --inconclusive --error-exitcode=0 --exception-handling=stderr ' + jobs + ' .'
    cmds = cmd.split()
    cmds.append('--template={callstack}: ({severity}) {message} [{id}]')

    p = subprocess.Popen(cmds, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()

    if p.returncode == 0:
        logging.info(comm[1] + strfCurrTime('[%H:%M]'))
    elif comm[0].find('cppcheck: error: could not find or open any of the paths given.') < 0:
        logging.error(comm[1] + strfCurrTime('[%H:%M]'))
        logging.error('Exit code is not zero! Crash?\n')


parser = argparse.ArgumentParser(description='Checks debian source code')
parser.add_argument('folder', metavar='FOLDER')
parser.add_argument('--rev')
parser.add_argument('--workdir', default='~/daca2')
parser.add_argument('-j', '--jobs', default='-j1')
parser.add_argument('--skip', default=[], action='append')
parser.add_argument('--cpulimit')

args = parser.parse_args()

workdir = os.path.expanduser(args.workdir)
if not os.path.isdir(workdir):
    print('workdir \'' + workdir + '\' is not a folder')
    sys.exit(1)

workdir = os.path.join(workdir, args.folder)
if not os.path.isdir(workdir):
    os.makedirs(workdir)

RESULTS_FILENAME = 'results.txt'
RESULTS_FILE = os.path.join(workdir, RESULTS_FILENAME)

logging.basicConfig(
        filename=RESULTS_FILE,
        level=logging.INFO,
        format='%(message)s')

print(workdir)

archives = getpackages(args.folder)
if len(archives) == 0:
    logging.critical('failed to load packages')
    sys.exit(1)

if not os.path.isdir(workdir):
    os.makedirs(workdir)
os.chdir(workdir)

try:
    logging.info('STARTDATE ' + str(datetime.date.today()))
    logging.info('STARTTIME ' + strfCurrTime('%H:%M:%S'))
    if args.rev:
        logging.info('GIT-REVISION ' + args.rev + '\n')
    logging.info('')

    for archive in archives:
        if len(args.skip) > 0:
            a = archive[:archive.rfind('/')]
            a = a[a.rfind('/')+1:]
            if a in args.skip:
                continue
        scanarchive(archive, args.jobs, args.cpulimit)

    logging.info('DATE {}'.format(datetime.date.today()))
    logging.info('TIME {}'.format(strfCurrTime('%H:%M:%S')))

except EOFError:
    pass

# remove all files/folders except RESULTS_FILENAME
removeAllExceptResults()
