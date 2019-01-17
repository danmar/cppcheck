# Donate CPU
#
# A script a user can run to donate CPU to cppcheck project
#
# Syntax: donate-cpu.py [-jN] [--package=url] [--stop-time=HH:MM] [--work-path=path] [--test] [--bandwidth-limit=limit]
#  -jN                  Use N threads in compilation/analysis. Default is 1.
#  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce some warning/crash/exception/etc..
#  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.
#  --work-path=path     Work folder path. Default path is cppcheck-donate-cpu-workfolder in your home folder.
#  --test               Connect to a donate-cpu-server that is running locally on port 8001 for testing.
#  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.
#                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second
#                                 --bandwidth-limit=2m => max. 2 megabytes per second
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
import os
import subprocess
import sys
import socket
import time
import re
import tarfile
import platform


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


def getCppcheckVersions(server_address):
    print('Connecting to server to get Cppcheck versions..')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(server_address)
        sock.send(b'GetCppcheckVersions\n')
        versions = sock.recv(256)
    except socket.error:
        return None
    sock.close()
    return versions.decode('utf-8').split()


def getPackage(server_address):
    print('Connecting to server to get assigned work..')
    package = None
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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


def wget(url, destfile, bandwidth_limit):
    if os.path.exists(destfile):
        if os.path.isfile(destfile):
            os.remove(destfile)
        else:
            print('Error: ' + destfile + ' exists but it is not a file! Please check the path and delete it manually.')
            sys.exit(1)
    limit_rate_option = ''
    if bandwidth_limit and isinstance(bandwidth_limit, str):
        limit_rate_option = '--limit-rate=' + bandwidth_limit
    subprocess.call(
            ['wget', '--tries=10', '--timeout=300', limit_rate_option, '-O', destfile, url])
    if os.path.isfile(destfile):
        return True
    print('Sleep for 10 seconds..')
    time.sleep(10)
    return False


def downloadPackage(workPath, package, bandwidth_limit):
    print('Download package ' + package)
    destfile = workPath + '/temp.tgz'
    if not wget(package, destfile, bandwidth_limit):
        if not wget(package, destfile, bandwidth_limit):
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
            elif member.name.lower().endswith(('.c', '.cpp', '.cxx', '.cc', '.c++', '.h', '.hpp', '.hxx', '.hh', '.tpp', '.txx')):
                try:
                    tf.extract(member.name)
                except OSError:
                    pass
                except AttributeError:
                    pass
        tf.close()
    os.chdir(workPath)


def hasInclude(path, inc):
    for root, _, files in os.walk(path):
        for name in files:
            filename = os.path.join(root, name)
            try:
                f = open(filename, 'rt')
                filedata = f.read()
                try:
                    # Python2 needs to decode the data first
                    filedata = filedata.decode(encoding='utf-8', errors='ignore')
                except AttributeError:
                    # Python3 directly reads the data into a string object that has no decode()
                    pass
                f.close()
                if filedata.find('\n#include ' + inc) >= 0:
                    return True
            except IOError:
                pass
    return False


def scanPackage(workPath, cppcheck, jobs):
    print('Analyze..')
    os.chdir(workPath)
    libraries = ' --library=posix'
    if hasInclude('temp', '<wx/') or hasInclude('temp', '"wx/'):
        libraries += ' --library=wxwidgets'
    if hasInclude('temp', '<QString>'):
        libraries += ' --library=qt'
    if hasInclude('temp', '<zlib.h>'):
        libraries += ' --library=zlib'
    if hasInclude('temp', '<gtk/gtk.h>'):
        libraries += ' --library=gtk'
#    if hasInclude('temp', '<boost/'):
#        libraries += ' --library=boost'
    options = jobs + libraries + ' -D__GCC__ --inconclusive --enable=style --platform=unix64 --template=daca2 -rp=temp temp'
    cmd = 'nice ' + cppcheck + ' ' + options
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
        return -1, '', -1, options
    if stderr.find('Internal error: Child process crashed with signal 11 [cppcheckError]') > 0:
        # Crash!
        print('Crash!')
        return -1, '', -1, options
    elapsedTime = stopTime - startTime
    count = 0
    for line in stderr.split('\n'):
        if re.match(r'.*:[0-9]+:.*\]$', line):
            count += 1
    print('Number of issues: ' + str(count))
    return count, stderr, elapsedTime, options


def splitResults(results):
    ret = []
    w = None
    for line in results.split('\n'):
        if line.endswith(']') and re.search(r': (error|warning|style|performance|portability|information|debug):', line):
            if w is not None:
                ret.append(w.strip())
            w = ''
        if w is not None:
            w += ' ' * 5 + line + '\n'
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
    bytes = data.encode('ascii', 'ignore')
    while bytes:
        num = connection.send(bytes)
        if num < len(bytes):
            bytes = bytes[num:]
        else:
            bytes = None


def uploadResults(package, results, server_address):
    print('Uploading results..')
    for retry in range(4):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(server_address)
            sendAll(sock, 'write\n' + package + '\n' + results + '\nDONE')
            sock.close()
            return True
        except socket.error:
            print('Upload failed, retry in 60 seconds')
            time.sleep(30)
            pass
    return False


jobs = '-j1'
stopTime = None
workpath = os.path.expanduser('~/cppcheck-donate-cpu-workfolder')
packageUrl = None
server_address = ('cppcheck.osuosl.org', 8000)
bandwidth_limit = None
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
    elif arg == '--test':
        server_address = ('localhost', 8001)
    elif arg.startswith('--bandwidth-limit='):
        bandwidth_limit = arg[arg.find('=')+1:]
    elif arg == '--help':
        print('Donate CPU to Cppcheck project')
        print('')
        print('Syntax: donate-cpu.py [-jN] [--stop-time=HH:MM] [--work-path=path]')
        print('  -jN                  Use N threads in compilation/analysis. Default is 1.')
        print('  --package=url        Check a specific package and then stop. Can be useful if you want to reproduce')
        print('                       some warning/crash/exception/etc..')
        print('  --stop-time=HH:MM    Stop analysis when time has passed. Default is that you must terminate the script.')
        print('  --work-path=path     Work folder path. Default path is ' + workpath)
        print('  --bandwidth-limit=limit Limit download rate for packages. Format for limit is the same that wget uses.')
        print('                       Examples: --bandwidth-limit=250k => max. 250 kilobytes per second')
        print('                                 --bandwidth-limit=2m => max. 2 megabytes per second')
        print('')
        print('Quick start: just run this script without any arguments')
        sys.exit(0)
    else:
        print('Unhandled argument: ' + arg)
        sys.exit(1)

print('Thank you!')
if not checkRequirements():
    sys.exit(1)
if subprocess.call(['wget', '--limit-rate=' + bandwidth_limit, '-q', '--spider', 'cppcheck.osuosl.org']) is 2:
    print('Error: Bandwidth limit value "' + bandwidth_limit + '" is invalid.')
    sys.exit(1)
else:
    print('Bandwidth-limit: ' + bandwidth_limit)
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
    cppcheckVersions = getCppcheckVersions(server_address)
    if cppcheckVersions is None:
        print('Failed to communicate with server, retry later')
        sys.exit(1)
    for ver in cppcheckVersions:
        if ver == 'head':
            if not compile(cppcheckPath, jobs):
                print('Failed to compile Cppcheck, retry later')
                sys.exit(1)
        elif not compile_version(workpath, jobs, ver):
            print('Failed to compile Cppcheck-{}, retry later'.format(ver))
            sys.exit(1)
    if packageUrl:
        package = packageUrl
    else:
        package = getPackage(server_address)
    while len(package) == 0:
        print("network or server might be temporarily down.. will try again in 30 seconds..")
        time.sleep(30)
        package = getPackage(server_address)
    tgz = downloadPackage(workpath, package, bandwidth_limit)
    unpackPackage(workpath, tgz)
    crash = False
    count = ''
    elapsedTime = ''
    resultsToDiff = []
    cppcheck_options = ''
    for ver in cppcheckVersions:
        if ver == 'head':
            cppcheck = 'cppcheck/cppcheck'
        else:
            cppcheck = ver + '/cppcheck'
        c, errout, t, cppcheck_options = scanPackage(workpath, cppcheck, jobs)
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
    output = 'cppcheck-options: ' + cppcheck_options + '\n'
    output += 'platform: ' + platform.platform() + '\n'
    output += 'python: ' + platform.python_version() + '\n'
    output += 'cppcheck: ' + ' '.join(cppcheckVersions) + '\n'
    output += 'count:' + count + '\n'
    output += 'elapsed-time:' + elapsedTime + '\n'
    if 'head' in cppcheckVersions:
        output += 'head results:\n' + resultsToDiff[cppcheckVersions.index('head')]
    if not crash:
        output += 'diff:\n' + diffResults(workpath, cppcheckVersions[0], resultsToDiff[0], cppcheckVersions[1], resultsToDiff[1]) + '\n'
    if packageUrl:
        print('=========================================================')
        print(output)
        print('=========================================================')
        break
    uploadResults(package, output, server_address)
    print('Results have been uploaded')
    print('Sleep 5 seconds..')
    time.sleep(5)
