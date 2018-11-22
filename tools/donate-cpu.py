# Donate CPU
#
# A script a user can run to donate CPU to cppcheck project
#
# Syntax: donate-cpu.py [-jN] [--stop-time=HH:MM] [--work-path=path]
#  -jN                  Use N threads in compilation/analysis. Default is 1.
#  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.
#  --work-path=path     Work folder path. Default path is cppcheck-donate-cpu-workfolder in your home folder.
#
# What this script does:
# 1. Check requirements
# 2. Pull & compile Cppcheck
# 3. Select a package
# 4. Download package
# 5. Analyze source code
# 6. Upload results
# 7. Repeat from step 2
#
# Quick start: just run this script without any arguments

import shutil
import glob
import os
import subprocess
import sys
import socket
import time
import re
import tarfile

def checkRequirements():
    result = True
    for app in ['g++', 'git', 'make', 'wget']:
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
            subprocess.call(['git', 'clone', 'https://github.com/danmar/cppcheck.git', cppcheckPath])
            if not os.path.exists(cppcheckPath):
                print('Failed to clone, will try again in 10 minutes..')
                time.sleep(600)
                continue
        time.sleep(2)
        return True
    return False


def compile_version(workPath, jobs, version):
    if os.path.isfile(workPath + '/' + version + '/cppcheck'):
        return True
    os.chdir(workPath + '/cppcheck')
    subprocess.call(['git', 'checkout', version])
    subprocess.call(['make', 'clean'])
    subprocess.call(['make', jobs, 'SRCDIR=build', 'CXXFLAGS=-O2'])
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


def compile(cppcheckPath, jobs):
    print('Compiling Cppcheck..')
    try:
        os.chdir(cppcheckPath)
        subprocess.call(['make', jobs, 'SRCDIR=build', 'CXXFLAGS=-O2'])
        subprocess.call([cppcheckPath + '/cppcheck', '--version'])
    except OSError:
        return False
    return True


def getPackage():
    print('Connecting to server to get assigned work..')
    package = None
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('cppcheck.osuosl.org', 8000)
    try:
        sock.connect(server_address)
        sock.send(b'get\n')
        package = sock.recv(256)
    except socket.error:
        package = ''
    sock.close()
    return package.decode('utf-8')



def handleRemoveReadonly(func, path, exc):
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)


def removeTree(folderName):
    if not os.path.exists(folderName):
        return
    count = 5
    while count > 0:
        count -= 1
        try:
            shutil.rmtree(folderName, onerror=handleRemoveReadonly)
            break
        except OSError as err:
            time.sleep(30)
            if count == 0:
                print('Failed to cleanup {}: {}'.format(folderName, err))
                sys.exit(1)


def wget(url, destfile):
    if os.path.exists(destfile):
        if os.path.isfile(destfile):
            os.remove(destfile)
        else:
            print('Error: ' + destfile + ' exists but it is not a file! Please check the path and delete it manually.')
            sys.exit(1)
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
    removeTree(tempPath)
    os.mkdir(tempPath)
    os.chdir(tempPath)
    if tarfile.is_tarfile(tgz):
        tf = tarfile.open(tgz)
        for member in tf:
            if member.name.startswith(('/', '..')):
                # Skip dangerous file names
                continue
            elif member.name.lower().endswith(('.c', '.cl', '.cpp', '.cxx', '.cc', '.c++', '.h', '.hpp', '.hxx', '.hh', '.tpp', '.txx')):
                try:
                    tf.extract(member.name)
                except OSError:
                    pass
                except AttributeError:
                    pass
        tf.close()
    os.chdir(workPath)


def scanPackage(workPath, cppcheck, jobs):
    print('Analyze..')
    os.chdir(workPath)
    cmd = 'nice ' + cppcheck + ' ' + jobs + ' -D__GCC__ --inconclusive --enable=style --library=posix --platform=unix64 --template=daca2 -rp=temp temp'
    print(cmd)
    startTime = time.time()
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stopTime = time.time()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')
    if p.returncode != 0 and 'cppcheck: error: could not find or open any of the paths given.' not in stdout:
        # Crash!
        print('Crash!')
        return -1, '', -1
    elapsedTime = stopTime - startTime
    count = 0
    for line in stderr.split('\n'):
        if re.match(r'.*:[0-9]+:.*\]$', line):
            count += 1
    print('Number of issues: ' + str(count))
    return count, stderr, elapsedTime


def splitResults(results):
    ret = []
    w = None
    for line in results.split('\n'):
        if line.endswith(']') and re.search(r': (error|warning|style|performance|portability|information|debug):', line):
            if w is not None:
                ret.append(w.strip())
            w = ''
        if w is not None:
            w += line + '\n'
    if w is not None:
        ret.append(w.strip())
    return ret

def diffResults(workPath, ver1, results1, ver2, results2):
    print('Diff results..')
    ret = ''
    r1 = sorted(splitResults(results1))
    r2 = sorted(splitResults(results2))
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

jobs = '-j1'
stopTime = None
workpath = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
packageUrl = None
for arg in sys.argv[1:]:
    # --stop-time=12:00 => run until ~12:00 and then stop
    if arg.startswith('--stop-time='):
        stopTime = arg[-5:]
        print('Stop time:' + stopTime)
    elif arg.startswith('-j'):
        jobs = arg
        print('Jobs:' + jobs[2:])
    elif arg.startswith('--package='):
        packageUrl = arg[arg.find('=')+1:]
        print('Package:' + packageUrl)
    elif arg.startswith('--work-path='):
        workpath = arg[arg.find('=')+1:]
        print('workpath:' + workpath)
        if not os.path.exists(workpath):
            print('work path does not exist!')
            sys.exit(1)
    elif arg == '--help':
        print('Donate CPU to Cppcheck project')
        print('')
        print('Syntax: donate-cpu.py [-jN] [--stop-time=HH:MM] [--work-path=path]')
        print('  -jN                  Use N threads in compilation/analysis. Default is 1.')
        print('  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.')
        print('  --work-path=path     Work folder path. Default path is ' + workpath)
        print('')
        print('Quick start: just run this script without any arguments')
        sys.exit(0)
    else:
        print('Unhandled argument: ' + arg)
        sys.exit(1)

print('Thank you!')
if not checkRequirements():
    sys.exit(1)
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
    if compile_version(workpath, jobs, '1.85') == False:
        print('Failed to compile Cppcheck-1.85, retry later')
        sys.exit(1)
    if compile(cppcheckPath, jobs) == False:
        print('Failed to compile Cppcheck, retry later')
        sys.exit(1)
    if packageUrl:
        package = packageUrl
    else:
        package = getPackage()
    while len(package) == 0:
        print("network or server might be temporarily down.. will try again in 30 seconds..")
        time.sleep(30)
        package = getPackage()
    tgz = downloadPackage(workpath, package)
    unpackPackage(workpath, tgz)
    crash = False
    count = ''
    elapsedTime = ''
    resultsToDiff = []
    for cppcheck in ['cppcheck/cppcheck', '1.85/cppcheck']:
        c,errout,t = scanPackage(workpath, cppcheck, jobs)
        if c < 0:
            crash = True
            count += ' Crash!'
        else:
            count += ' ' + str(c)
        elapsedTime += " {:.1f}".format(t)
        resultsToDiff.append(errout)
    if not crash and len(resultsToDiff[0]) + len(resultsToDiff[1]) == 0:
        print('No results')
        continue
    output = 'cppcheck: head 1.85\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsedTime + '\n'
    if not crash:
        output += 'diff:\n' + diffResults(workpath, 'head', resultsToDiff[0], '1.85', resultsToDiff[1]) + '\n'
    if packageUrl:
        print('=========================================================')
        print(output)
        print('=========================================================')
        break
    uploadResults(package, output)
    print('Results have been uploaded')
    print('Sleep 5 seconds..')
    time.sleep(5)
