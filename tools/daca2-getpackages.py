#!/usr/bin/env python
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-head in daca2. It should be built with all optimisations.
# 3. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 4. Optional: tweak FTPSERVER and FTPPATH in this script below.
# 5. Run the daca2 script:  python daca2.py FOLDER

import argparse
import logging
import subprocess
import sys
import shutil
import glob
import os
import datetime
import time

DEBIAN = ('ftp://ftp.se.debian.org/debian/',
          'ftp://ftp.debian.org/debian/')


def wget(filepath):
    filename = filepath
    if '/' in filepath:
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
        sys.exit(1)
    subprocess.call(['nice', 'gunzip', 'ls-lR.gz'])
    if not os.path.isfile('ls-lR'):
        sys.exit(1)
    f = open('ls-lR', 'rt')
    lines = f.readlines()
    f.close()
    subprocess.call(['rm', 'ls-lR'])

    # Example content in ls-lR:
    # ./pool/main/0/0xffff:
    # total 1452
    # -rw-r--r-- 2 dak debadmin  6524 Dec 25  2016 0xffff_0.7-2.debian.tar.xz
    # -rw-r--r-- 2 dak debadmin  1791 Dec 25  2016 0xffff_0.7-2.dsc
    # -rw-r--r-- 2 dak debadmin 57168 Dec 25  2016 0xffff_0.7-2_amd64.deb
    # -rw-r--r-- 2 dak debadmin 48578 Dec 26  2016 0xffff_0.7-2_arm64.deb
    # -rw-r--r-- 2 dak debadmin 56730 Dec 26  2016 0xffff_0.7-2_armel.deb
    # -rw-r--r-- 2 dak debadmin 57296 Dec 26  2016 0xffff_0.7-2_armhf.deb
    # -rw-r--r-- 2 dak debadmin 60254 Dec 26  2016 0xffff_0.7-2_i386.deb
    # -rw-r--r-- 2 dak debadmin 53130 Dec 26  2016 0xffff_0.7-2_mips.deb
    # -rw-r--r-- 2 dak debadmin 52542 Dec 26  2016 0xffff_0.7-2_mips64el.deb
    # -rw-r--r-- 2 dak debadmin 53712 Dec 26  2016 0xffff_0.7-2_mipsel.deb
    # -rw-r--r-- 2 dak debadmin 51908 Dec 26  2016 0xffff_0.7-2_ppc64el.deb
    # -rw-r--r-- 2 dak debadmin 53548 Dec 26  2016 0xffff_0.7-2_s390x.deb
    # -rw-r--r-- 2 dak debadmin 65248 Dec 25  2016 0xffff_0.7.orig.tar.gz
    # -rw-r--r-- 2 dak debadmin  6884 Jul 19 19:08 0xffff_0.8-1.debian.tar.xz
    # -rw-r--r-- 2 dak debadmin  1807 Jul 19 19:08 0xffff_0.8-1.dsc
    # -rw-r--r-- 2 dak debadmin 58908 Jul 19 19:08 0xffff_0.8-1_amd64.deb
    # -rw-r--r-- 2 dak debadmin 51340 Jul 19 19:58 0xffff_0.8-1_arm64.deb
    # -rw-r--r-- 2 dak debadmin 57612 Jul 19 20:13 0xffff_0.8-1_armel.deb
    # -rw-r--r-- 2 dak debadmin 58584 Jul 19 19:58 0xffff_0.8-1_armhf.deb
    # -rw-r--r-- 2 dak debadmin 57544 Jul 19 20:23 0xffff_0.8-1_hurd-i386.deb
    # -rw-r--r-- 2 dak debadmin 62048 Jul 19 23:54 0xffff_0.8-1_i386.deb
    # -rw-r--r-- 2 dak debadmin 55080 Jul 23 19:07 0xffff_0.8-1_kfreebsd-amd64.deb
    # -rw-r--r-- 2 dak debadmin 58392 Jul 23 19:07 0xffff_0.8-1_kfreebsd-i386.deb
    # -rw-r--r-- 2 dak debadmin 54144 Jul 19 22:28 0xffff_0.8-1_mips.deb
    # -rw-r--r-- 2 dak debadmin 53648 Jul 20 00:56 0xffff_0.8-1_mips64el.deb
    # -rw-r--r-- 2 dak debadmin 54740 Jul 19 22:58 0xffff_0.8-1_mipsel.deb
    # -rw-r--r-- 2 dak debadmin 57424 Jul 19 19:58 0xffff_0.8-1_ppc64el.deb
    # -rw-r--r-- 2 dak debadmin 53764 Jul 19 22:28 0xffff_0.8-1_s390x.deb
    # -rw-r--r-- 2 dak debadmin 64504 Jul 19 19:08 0xffff_0.8.orig.tar.gz
    #

    path = None
    archives = []
    filename = None
    for line in lines:
        line = line.strip()
        if len(line) < 4:
            if filename:
                archives.append(DEBIAN[0] + path + '/' + filename)
            path = None
            filename = None
        elif line.startswith('./pool/main/'):
            path = line[2:-1]
        elif path and line.endswith(('.orig.tar.gz', '.orig.tar.bz2')):
            filename = line[1 + line.rfind(' '):]

    return archives


for p in getpackages():
    print(p)
