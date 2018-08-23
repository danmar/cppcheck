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

def checkRequirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget', 'tar']:
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
    subprocess.call(['make', 'SRCDIR=build', 'CXXFLAGS=-O2'])

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
    if not wget(package, destfile):
        if not wget(filepath, destfile):
            return None
    print('Unpacking..')
    tempPath = workPath + '/temp'
    subprocess.call(['rm', '-rf', tempPath])
    os.mkdir(tempPath)
    os.chdir(tempPath)
    subprocess.call(['tar', 'xzvf', destfile])
    os.chdir(workPath)
    print('Analyze..')
    cmd = 'nice ' + workPath + '/cppcheck/cppcheck -D__GCC__ --enable=style --library=posix --platform=unix64 --template=daca2 -rp=temp temp'
    print(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    return comm[1]

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
    compile(cppcheckPath)
    package = getPackage()
    results = scanPackage(workpath, package)
    if results:
        uploadResults(package, results)
    break
