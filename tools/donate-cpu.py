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
    for i in range(5):
        if os.path.exists(cppcheckPath):
            os.chdir(cppcheckPath)
            subprocess.call(['git', 'checkout', '-f'])
            subprocess.call(['git', 'pull'])
        else:
            subprocess.call(['git', 'clone', 'http://github.com/danmar/cppcheck.git', cppcheckPath])
            if not os.path.exists(cppcheckPath):
                print('Failed to clone, will try again in 10 minutes..')
                time.sleep(600)
                continue
        time.sleep(2)
        return True
    return False


def compile_version(workPath, version):
    if os.path.isfile(workPath + '/' + version + '/cppcheck'):
        return True
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
    try:
        subprocess.call([workPath + '/' + version + '/cppcheck', '--version'])
    except OSError:
        return False
    return True


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
    cmd = 'nice ' + cppcheck + ' -D__GCC__ --enable=style --library=posix --platform=unix64 --template={file}:{line}:{message}[{id}] -rp=temp temp'
    print(cmd)
    startTime = time.time()
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stopTime = time.time()
    elapsedTime = stopTime - startTime
    errout = comm[1].decode(encoding='utf-8', errors='ignore')
    count = 0
    for line in errout.split('\n'):
        if re.match(r'.*:[0-9]+:.*\]$', line):
            count += 1
    print('Number of issues: ' + str(count))
    return count, errout, elapsedTime


def diffResults(workPath, ver1, results1, ver2, results2):
    print('Diff results..')
    ret = ''
    r1 = sorted(results1.split('\n'))
    r2 = sorted(results2.split('\n'))
    i1 = 0
    i2 = 0
    while i1 < len(r1) and i2 < len(r2):
        if r1[i1] == r2[i2]:
            i1 += 1
            i2 += 1
        elif r1[i1] < r2[i2]:
            ret += ver1 + ' ' + r1[i1] + '\n'
            i1 += 1
        else:
            ret += ver2 + ' ' + r2[i2] + '\n'
            i2 += 1
    while i1 < len(r1):
        ret += ver1 + ' ' + r1[i1] + '\n'
        i1 += 1
    while i2 < len(r2):
        ret += ver2 + ' ' + r2[i2] + '\n'
        i2 += 1
    return ret


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

stopTime = None
for arg in sys.argv[1:]:
    # --stop-time=12:00 => run until ~12:00 and then stop
    if arg.startswith('--stop-time='):
        stopTime = arg[-5:]
print('Thank you!')
if not checkRequirements():
    sys.exit(1)
workpath = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
if not os.path.exists(workpath):
    os.mkdir(workpath)
cppcheckPath = workpath + '/cppcheck'
while True:
    if stopTime:
        print('stopTime:' + stopTime + '. Time:' + time.strftime('%H:%M') + '.')
        if stopTime < time.strftime('%H:%M'):
            print('Stopping. Thank you!')
            sys.exit(0)
    if not getCppcheck(cppcheckPath):
        print('Failed to clone Cppcheck, retry later')
        sys.exit(1)
    if compile_version(workpath, '1.84') == False:
        print('Failed to compile Cppcheck-1.84, retry later')
        sys.exit(1)
    if compile(cppcheckPath) == False:
        print('Failed to compile Cppcheck, retry later')
        sys.exit(1)
    package = getPackage()
    tgz = downloadPackage(workpath, package)
    unpackPackage(workpath, tgz)
    count = ''
    elapsedTime = ''
    resultsToDiff = []
    for cppcheck in ['cppcheck/cppcheck', '1.84/cppcheck']:
        c,errout,t = scanPackage(workpath, cppcheck)
        count += ' ' + str(c)
        elapsedTime += " {:.1f}".format(t)
        resultsToDiff.append(errout)
    if len(resultsToDiff[0]) + len(resultsToDiff[1]) == 0:
        print('No results')
        continue
    output = 'cppcheck: head 1.84\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsedTime + '\n'
    output += 'diff:\n' + diffResults(workpath, 'head', resultsToDiff[0], '1.84', resultsToDiff[1]) + '\n'
    uploadResults(package, output)
    print('Results have been uploaded')
    print('Sleep 5 seconds..')
    time.sleep(5)
