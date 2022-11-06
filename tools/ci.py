#!/usr/bin/env python3

# continuous integration
# build daily reports (doxygen,coverage,etc)

import datetime
import time
import subprocess
import pexpect
import glob
import sys


# Upload file to sourceforge web server using scp
def upload(file_to_upload, destination):
    try:
        password = sys.argv[1]
        child = pexpect.spawn(
            'scp ' + file_to_upload + ' danielmarjamaki,cppcheck@web.sourceforge.net:' + destination)
        # child.expect(
        #    'danielmarjamaki,cppcheck@web.sourceforge.net\'s password:')
        child.expect('Password:')
        child.sendline(password)
        child.interact()
    except (IOError, OSError, pexpect.TIMEOUT):
        pass


# git push
def gitpush():
    try:
        password = sys.argv[1]
        child = pexpect.spawn('git push')
        child.expect("Enter passphrase for key '/home/daniel/.ssh/id_rsa':")
        child.sendline(password)
        child.interact()
    except (IOError, OSError, pexpect.TIMEOUT):
        pass


def iconv(filename):
    p = subprocess.Popen(['file', '-i', filename],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    comm = p.communicate()
    if 'charset=iso-8859-1' in comm[0]:
        subprocess.call(
            ["iconv", filename, "--from=ISO-8859-1", "--to=UTF-8", "-o", filename])


# Generate daily webreport
def generate_webreport():
    for filename in glob.glob('*/*.cpp'):
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

    except (IOError, OSError, pexpect.TIMEOUT):
        pass
    except pexpect.EOF:
        return True

    return False


t0 = None
while True:
    if datetime.date.today() != t0:
        print("generate daily reports")
        t0 = datetime.date.today()
        gitpull()
        generate_webreport()
    time.sleep(60)
