# Donate CPU
#
# A script a user can run to donate CPU to cppcheck project
#
# 1. Check requirements
# 2. Pull & compile Cppcheck
# 3. Select a package
# 4. Download package
# 5. Analyze source code
# 6. Upload results
# 7. Repeat from step 2

import shutil
import glob
import os
import subprocess
import sys
import socket
import time
import re

def checkRequirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget', 'rm', 'tar']:
        try:
            subprocess.call([app, '--version'])
        except OSError:
           print(app + ' is required')
           result = False
    return result

def getCppcheck(workPath):
    print('Get Cppcheck..')
    cppcheckPath = workPath + '/cppcheck'
    if os.path.exists(cppcheckPath):
        os.chdir(cppcheckPath)
        subprocess.call(['git', 'checkout', '-f'])
        subprocess.call(['git', 'pull'])
    else:
        subprocess.call(['git', 'clone', 'http://github.com/danmar/cppcheck.git', cppcheckPath])

def compile(cppcheckPath):
    print('Compiling Cppcheck..')
    try:
        subprocess.call(['make', 'SRCDIR=build', 'CXXFLAGS=-O2'])
        subprocess.call(['./cppcheck', '--version'])
    except OSError:
        return False
    return True

def getPackage():
    print('Connecting to server to get assigned work..')
    package = None
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('cppcheck.osuosl.org', 8000)
    sock.connect(server_address)
    try:
        sock.sendall('get\n')
        package = sock.recv(256)
    finally:
        sock.close()
    return package

def wget(url, destfile):
    subprocess.call(
            ['wget', '--tries=10', '--timeout=300', '-O', destfile, url])
    if os.path.isfile(destfile):
        return True
    print('Sleep for 10 seconds..')
    time.sleep(10)
    return False

def scanPackage(workPath, package):
    print('Download package ' + package)
    destfile = workPath + '/temp.tgz'
    tempPath = workPath + '/temp'
    subprocess.call(['rm', '-rf', tempPath, destfile])
    if not wget(package, destfile):
        if not wget(filepath, destfile):
            return None
    print('Unpacking..')
    os.mkdir(tempPath)
    os.chdir(tempPath)
    subprocess.call(['tar', 'xzvf', destfile])
    os.chdir(workPath)
    print('Analyze..')
    cmd = 'nice ' + workPath + '/cppcheck/cppcheck -D__GCC__ --enable=style --library=posix --platform=unix64 --template=daca2 -rp=temp temp'
    print(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    errout = comm[1]
    count = 0
    for line in errout.split('\n'):
        if re.match(r'.*:[0-9]+:[0-9]+: [a-z]+: .*\]$', line):
            count += 1
    if count == 0:
        errout = None
    else:
        print('Number of issues: ' + str(count))
    return errout

def uploadResults(package, results):
    print('Uploading results..')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('cppcheck.osuosl.org', 8000)
    sock.connect(server_address)
    try:
        sock.sendall('write\n' + package + '\n' + results)
    finally:
        sock.close()
    return package


print('Thank you!')
if not checkRequirements():
    sys.exit(1)
workpath = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
if not os.path.exists(workpath):
    os.mkdir(workpath)
cppcheckPath = workpath + '/cppcheck'
while True:
    getCppcheck(workpath)
    if compile(cppcheckPath) == False:
        print('Failed to compile Cppcheck, retry later')
        sys.exit(1)
    package = getPackage()
    results = scanPackage(workpath, package)
    if results is None:
        print('No results to upload')
    else:
        uploadResults(package, results)
