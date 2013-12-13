#!/usr/bin/python

# continuous integration
# build daily reports (doxygen,coverage,etc)

import datetime
import time
import subprocess
import pexpect
import glob
import os
import sys
import urllib


def wget(url):
    try:
        fp = urllib.urlopen(url)
        data = fp.read()
        return data
    except IOError:
        pass
    return ''


# Upload file to sourceforge web server using scp
def upload(file_to_upload, destination):
    try:
        password = sys.argv[1]
        child = pexpect.spawn(
            'scp ' + file_to_upload + ' danielmarjamaki,cppcheck@web.sourceforge.net:' + destination)
        #child.expect(
        #    'danielmarjamaki,cppcheck@web.sourceforge.net\'s password:')
        child.expect('Password:')
        child.sendline(password)
        child.interact()
    except IOError:
        pass
    except OSError:
        pass
    except pexpect.TIMEOUT:
        pass


# Perform a 'make test' on the repo
def maketest(preclean):
    if preclean == True:
        subprocess.call(['make', 'clean'])

    p = subprocess.Popen(
        ['nice', 'make', 'test'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()

    f = open('maketest.txt', 'wt')
    f.write('Errors\n======\n')
    f.write(comm[1] + '\n')
    f.write('Output\n======\n')
    f.write(comm[0] + '\n')
    f.close()

    upload('maketest.txt', 'htdocs/devinfo/')


# git push
def gitpush():
    try:
        password = sys.argv[1]
        child = pexpect.spawn('git push')
        child.expect("Enter passphrase for key '/home/daniel/.ssh/id_rsa':")
        child.sendline(password)
        child.interact()
    except IOError:
        pass
    except OSError:
        pass
    except pexpect.TIMEOUT:
        pass


def iconv(filename):
    p = subprocess.Popen(['file', '-i', filename],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    comm = p.communicate()
    if comm[0].find('charset=iso-8859-1') >= 0:
        subprocess.call(
            ["iconv", filename, "--from=ISO-8859-1", "--to=UTF-8", "-o", filename])


# Generate daily webreport
def generate_webreport():
    filenames = glob.glob('*/*.cpp')
    for filename in filenames:
        iconv(filename)
    subprocess.call(
        ["git", "commit", "-a", "-m", '"automatic conversion from iso-8859-1 formatting to utf-8"'])
    gitpush()

    subprocess.call(["rm", "-rf", "devinfo"])
    subprocess.call(['nice', "./webreport.sh"])
    upload('-r devinfo', 'htdocs/')
    subprocess.call(["make", "clean"])
    subprocess.call(["rm", "-rf", "devinfo"])


# Perform a git pull.
def gitpull():
    try:
        password = sys.argv[1]
        child = pexpect.spawn('git pull')
        child.expect("Enter passphrase for key '/home/daniel/.ssh/id_rsa':")
        child.sendline(password)
        child.expect('Already up-to-date.')
        child.interact()

    except IOError:
        pass
    except OSError:
        pass
    except pexpect.TIMEOUT:
        pass
    except pexpect.EOF:
        return True

    return False


def daca2report():
    print('Generate DACA2 report')
    subprocess.call(['rm', '-rf', 'daca2-report'])
    subprocess.call(['mkdir', 'daca2-report'])
    subprocess.call(['python', 'tools/daca2-report.py', 'daca2-report'])
    upload('-r daca2-report', 'htdocs/devinfo/')


def daca2(foldernum):
    folders = '0123456789abcdefghijklmnopqrstuvwxyz'
    folder = folders[foldernum % len(folders)]

    print('Daca2 folder=' + folder)

    p = subprocess.Popen(['git', 'show', '--format=%h'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    rev = comm[0]
    rev = rev[:rev.find('\n')]

    subprocess.call(
        ['make', 'clean'])
    subprocess.call(
        ['nice', 'make', 'SRCDIR=build', 'CXXFLAGS=-O2', 'CPPFLAGS=-DMAXTIME=600'])
    subprocess.call(
        ['mv', 'cppcheck', os.path.expanduser('~/daca2/cppcheck-O2')])
    subprocess.call(
        ['cp', 'cfg/std.cfg', os.path.expanduser('~/daca2/')])

    subprocess.call(['python', 'tools/daca2.py', folder, '--rev=' + rev])
    daca2report()
    subprocess.call(
        ['python', 'tools/daca2.py', 'lib' + folder, '--rev=' + rev])
    daca2report()

t0 = datetime.date.today()
foldernum = 0
while True:
    if datetime.date.today() != t0:
        print("generate daily reports")
        t0 = datetime.date.today()
        generate_webreport()

    if gitpull() == True:
        print("make test")
        # maketest(False) # Integral make test build

    cmd = wget('http://cppcheck.sourceforge.net/cgi-bin/ci.cgi?clear')
    if cmd.find("doxygen") >= 0:
        generate_webreport()

    daca2(foldernum)
    foldernum = foldernum + 1
