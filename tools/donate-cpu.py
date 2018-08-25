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

def getCppcheck(cppcheckPath):
    print('Get Cppcheck..')
    if os.path.exists(cppcheckPath):
        os.chdir(cppcheckPath)
        subprocess.call(['git', 'checkout', '-f'])
        subprocess.call(['git', 'pull'])
    else:
        subprocess.call(['git', 'clone', 'http://github.com/danmar/cppcheck.git', cppcheckPath])
        if not os.path.exists(cppcheckPath):
            return False
    time.sleep(2)
    return True


def compile_version(workPath, version):
    if os.path.isfile(workPath + '/' + version + '/cppcheck'):
        return
    os.chdir(workPath + '/cppcheck')
    subprocess.call(['git', 'checkout', version])
    subprocess.call(['make', 'clean'])
    subprocess.call(['make', 'SRCDIR=build', 'CXXFLAGS=-O2'])
    if os.path.isfile(workPath + '/cppcheck/cppcheck'):
        os.mkdir(workpath + '/' + version)
        destPath = workpath + '/' + version + '/'
        subprocess.call(['cp', '-R', workPath + '/cppcheck/cfg', destPath])
        subprocess.call(['cp', 'cppcheck', destPath])
    subprocess.call(['git', 'checkout', 'master'])


def compile(cppcheckPath):
    print('Compiling Cppcheck..')
    try:
        os.chdir(cppcheckPath)
        subprocess.call(['make', 'SRCDIR=build', 'CXXFLAGS=-O2'])
        subprocess.call([cppcheckPath + '/cppcheck', '--version'])
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
        sock.send(b'get\n')
        package = sock.recv(256)
    finally:
        sock.close()
    return package.decode('utf-8')


def wget(url, destfile):
    subprocess.call(['rm', '-f', destfile])
    subprocess.call(
            ['wget', '--tries=10', '--timeout=300', '-O', destfile, url])
    if os.path.isfile(destfile):
        return True
    print('Sleep for 10 seconds..')
    time.sleep(10)
    return False


def downloadPackage(workPath, package):
    print('Download package ' + package)
    destfile = workPath + '/temp.tgz'
    if not wget(package, destfile):
        if not wget(package, destfile):
            return None
    return destfile


def unpackPackage(workPath, tgz):
    print('Unpacking..')
    tempPath = workPath + '/temp'
    subprocess.call(['rm', '-rf', tempPath])
    os.mkdir(tempPath)
    os.chdir(tempPath)
    subprocess.call(['tar', 'xzvf', tgz])
    os.chdir(workPath)


def scanPackage(workPath, cppcheck):
    print('Analyze..')
    os.chdir(workPath)
    cmd = 'nice ' + cppcheck + ' -D__GCC__ --enable=style --library=posix --platform=unix64 --template=daca2 -rp=temp temp'
    print(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    errout = comm[1].decode('utf-8')
    count = 0
    for line in errout.split('\n'):
        if re.match(r'.*:[0-9]+:[0-9]+: [a-z]+: .*\]$', line):
            count += 1
    if count == 0:
        errout = None
    else:
        print('Number of issues: ' + str(count))
    return errout


def sendAll(connection, data):
    bytes = data.encode()
    while bytes:
        num = connection.send(bytes)
        if num < len(bytes):
            bytes = bytes[num:]
        else:
            bytes = None


def uploadResults(package, results):
    print('Uploading results..')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('cppcheck.osuosl.org', 8000)
    sock.connect(server_address)
    try:
        sendAll(sock, 'write\n' + package + '\n' + results + '\nDONE')
        sock.close()
    except socket.error:
        pass
    return package


print('Thank you!')
if not checkRequirements():
    sys.exit(1)
workpath = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
if not os.path.exists(workpath):
    os.mkdir(workpath)
cppcheckPath = workpath + '/cppcheck'
while True:
    if not getCppcheck(cppcheckPath):
        time.sleep(5)
        if not getCppcheck(cppcheckPath):
            print('Failed to clone Cppcheck, retry later')
            sys.exit(1)
    compile_version(workpath, '1.84')
    if compile(cppcheckPath) == False:
        print('Failed to compile Cppcheck, retry later')
        sys.exit(1)
    package = getPackage()
    tgz = downloadPackage(workpath, package)
    unpackPackage(workpath, tgz)
    results = None
    for cppcheck in ['cppcheck/cppcheck', '1.84/cppcheck']:
        cmd = workpath + '/' + cppcheck
        if not os.path.isfile(cmd):
            continue
        res = scanPackage(workpath, cmd)
        if res:
            if results is None:
                results = ''
            results += 'cppcheck:' + cppcheck + '\n' + res
    if results is None:
        print('No results to upload')
    else:
        uploadResults(package, results)
        print('Results have been uploaded')
        time.sleep(5)
