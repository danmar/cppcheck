
# Server for 'donate-cpu.py'
#
# Syntax: donate-cpu-server.py [--version=revision]')
#  --version=revision     Use specified revision to compare head to. Default is 1.85.

import glob
import os
import socket
import re
import datetime
import time
from threading import Thread
import subprocess
import sys

def strDateTime():
    d = datetime.date.strftime(datetime.datetime.now().date(), '%Y-%m-%d')
    t = datetime.time.strftime(datetime.datetime.now().time(), '%H:%M')
    return d + ' ' + t

def overviewReport():
    html = '<html><head><title>daca@home</title></head><body>\n'
    html += '<h1>daca@home</h1>\n'
    html += '<a href="crash">Crash report</a><br>\n'
    html += '<a href="diff">Diff report</a><br>\n'
    html += '<a href="latest.html">Latest results</a><br>\n'
    html += '<a href="time">Time report</a><br>\n'
    html += '</body></html>'
    return html

def fmt(a,b,c,d,e):
    ret = a + ' '
    while len(ret)<10:
        ret += ' '
    if len(ret) == 10:
        ret += b[:10] + ' '
    while len(ret)<21:
        ret += ' '
    ret += b[-5:] + ' '
    while len(ret) < 32-len(c):
        ret += ' '
    ret += c + ' '
    while len(ret) < 37-len(d):
        ret += ' '
    ret += d
    ret += ' ' + e
    if a != 'Package':
        pos = ret.find(' ')
        ret = '<a href="' + a + '">' + a + '</a>' + ret[pos:]
    return ret


def latestReport(latestResults, version):
    html = '<html><head><title>Latest daca@home results</title></head><body>\n'
    html += '<h1>Latest daca@home results</h1>'
    html += '<pre>\n<b>' + fmt('Package','Date       Time ',version[0:4],'Head','Diff') + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        if not os.path.isfile(filename):
            continue
        package = filename[filename.rfind('/')+1:]

        datestr = ''
        count = ['0','0']
        lost = 0
        added = 0
        for line in open(filename,'rt'):
            line = line.strip()
            if line.startswith('2018-'):
                datestr = line
            #elif line.startswith('cppcheck:'):
            #    cppcheck = line[9:]
            elif line.startswith('count: '):
                count = line.split(' ')[1:]
            elif line.startswith('head '):
                added += 1
            elif line.startswith(version[0:4] + ' '):
                lost += 1
        diff = ''
        if lost > 0:
            diff += '-' + str(lost)
        if added > 0:
            diff += '+' + str(added)
        html += fmt(package, datestr, count[1], count[0], diff) + '\n'

    html += '</pre></body></html>\n'
    return html


def crashReport(version):
    html = '<html><head><title>Crash report</title></head><body>\n'
    html += '<h1>Crash report</h1>\n'
    html += '<pre>\n'
    html += '<b>Package                                 ' + version[0:4] + '  Head</b>\n'
    for filename in sorted(glob.glob(os.path.expanduser('~/daca@home/donated-results/*'))):
        if not os.path.isfile(filename):
            continue
        for line in open(filename, 'rt'):
            if not line.startswith('count:'):
                continue
            if line.find('Crash') < 0:
                break
            packageName = filename[filename.rfind('/')+1:]
            counts = line.strip().split(' ')
            out = packageName + ' '
            while len(out) < 40:
                out += ' '
            if counts[2] == 'Crash!':
                out += 'Crash '
            else:
                out += '      '
            if counts[1] == 'Crash!':
                out += 'Crash'
            out = '<a href="' + packageName + '">' + packageName + '</a>' + out[out.find(' '):]
            html += out + '\n'
            break
    html += '</pre>\n'
    html += '</body></html>\n'
    return html


def diffReportFromDict(out, today, version):
    html = '<pre>\n'
    html += '<b>MessageID                           ' + version[0:4] + '    Head</b>\n'
    sum0 = 0
    sum1 = 0
    for messageId in sorted(out.keys()):
        line = messageId + ' '
        counts = out[messageId]
        sum0 += counts[0]
        sum1 += counts[1]
        if counts[0] > 0:
            c = str(counts[0])
            while len(line) < 40 - len(c):
                line += ' '
            line += c + ' '
        if counts[1] > 0:
            c = str(counts[1])
            while len(line) < 48 - len(c):
                line += ' '
            line += c
        line = '<a href="diff' + today + '-' + messageId + '">' + messageId + '</a>' + line[line.find(' '):]
        html += line + '\n'

    # Sum
    html += '================================================\n'
    line = ''
    while len(line) < 40 - len(str(sum0)):
        line += ' '
    line += str(sum0) + ' '
    while len(line) < 48 - len(str(sum1)):
        line += ' '
    line += str(sum1)
    html += line + '\n'
    html += '</pre>\n'

    return html


def diffReport(resultsPath, version):
    out = {}
    outToday = {}
    today = strDateTime()[:10]

    for filename in sorted(glob.glob(resultsPath + '/*')):
        if not os.path.isfile(filename):
            continue
        uploadedToday = False
        firstLine = True
        for line in open(filename, 'rt'):
            if firstLine:
                if line.startswith(today):
                    uploadedToday = True
                firstLine = False
                continue
            line = line.strip()
            if not line.endswith(']'):
                continue
            index = None
            if line.startswith(version[0:4] + ' '):
                index = 0
            elif line.startswith('head '):
                index = 1
            else:
                continue
            messageId = line[line.rfind('[')+1:len(line)-1]

            if not messageId in out:
                out[messageId] = [0,0]
            out[messageId][index] += 1
            if uploadedToday:
                if not messageId in outToday:
                    outToday[messageId] = [0,0]
                outToday[messageId][index] += 1

    html = '<html><head><title>Diff report</title></head><body>\n'
    html += '<h1>Diff report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += diffReportFromDict(outToday, 'today', version)
    html += '<h2>All</h2>'
    html += diffReportFromDict(out, '', version)

    return html


def diffMessageIdReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*')):
        url = None
        diff = False
        for line in open(filename,'rt'):
            if line.startswith('ftp://'):
                url = line
            elif line == 'diff:\n':
                diff = True
            elif not diff:
                continue
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def diffMessageIdTodayReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*')):
        url = None
        diff = False
        firstLine = True
        for line in open(filename,'rt'):
            if firstLine:
                firstLine = False
                if not line.startswith(today):
                    break
            if line.startswith('ftp://'):
                url = line
            elif line == 'diff:\n':
                diff = True
            elif not diff:
                continue
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def timeReport(resultPath, version):
    text = 'Time report\n\n'
    text += 'Package ' + version[0:4] + ' Head\n'

    totalTime184 = 0.0
    totalTimeHead = 0.0
    for filename in glob.glob(resultPath + '/*'):
        for line in open(filename,'rt'):
            if not line.startswith('elapsed-time:'):
                continue
            splitline = line.strip().split()
            t184 = float(splitline[2])
            thead = float(splitline[1])
            totalTime184 += t184
            totalTimeHead += thead
            if t184>1 and t184*2 < thead:
                text += filename[filename.find('/')+1:] + ' ' + splitline[2] + ' ' + splitline[1] + '\n'
            elif thead>1 and thead*2 < t184:
                text += filename[filename.find('/')+1:] + ' ' + splitline[2] + ' ' + splitline[1] + '\n'
            break

    text += '\nTotal time: ' + str(totalTime184) + ' ' + str(totalTimeHead)
    return text

def sendAll(connection, data):
    while data:
        num = connection.send(data)
        if num < len(data):
            data = data[num:]
        else:
            data = None


def httpGetResponse(connection, data, contentType):
    resp = 'HTTP/1.1 200 OK\r\n'
    resp += 'Connection: close\r\n'
    resp += 'Content-length: ' + str(len(data)) + '\r\n'
    resp += 'Content-type: ' + contentType + '\r\n\r\n'
    resp += data
    sendAll(connection, resp)


class HttpClientThread(Thread):
    def __init__(self, connection, cmd, resultPath, latestResults, version):
        Thread.__init__(self)
        self.connection = connection
        self.cmd = cmd[:cmd.find('\n')]
        self.resultPath = resultPath
        self.latestResults = latestResults
        self.version = version

    def run(self):
        try:
            cmd = self.cmd
            print('[' + strDateTime() + '] ' + cmd)
            res = re.match(r'GET /([a-zA-Z0-9_\-\.]*) HTTP', cmd)
            if res is None:
                self.connection.close()
                return
            url = res.group(1)
            if url == '':
                html = overviewReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'latest.html':
                html = latestReport(self.latestResults, self.version)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'crash':
                html = crashReport(self.version)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'diff':
                html = diffReport(self.resultPath, self.version)
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('difftoday-'):
                messageId = url[10:]
                text = diffMessageIdTodayReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('diff-'):
                messageId = url[5:]
                text = diffMessageIdReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == 'time':
                text = timeReport(self.resultPath)
                httpGetResponse(self.connection, text, 'text/plain')
            else:
                filename = resultPath + '/' + url
                if not os.path.isfile(filename):
                    print('HTTP/1.1 404 Not Found')
                    self.connection.send('HTTP/1.1 404 Not Found\r\n\r\n')
                else:
                    f = open(filename,'rt')
                    data = f.read()
                    f.close()
                    httpGetResponse(self.connection, data, 'text/plain')
        finally:
            time.sleep(1)
            self.connection.close()

def server(server_address_port, packages, packageIndex, resultPath, version):
    socket.setdefaulttimeout(120)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('', server_address_port)
    sock.bind(server_address)

    sock.listen(1)

    latestResults = []
    if os.path.isfile('latest.txt'):
        with open('latest.txt', 'rt') as f:
            latestResults = f.read().strip().split(' ')

    while True:
        # wait for a connection
        print('[' + strDateTime() + '] waiting for a connection')
        connection, client_address = sock.accept()
        try:
            cmd = connection.recv(128)
        except socket.error:
            connection.close()
            continue
        if cmd.find('\n') < 1:
            continue
        firstLine = cmd[:cmd.find('\n')]
        if re.match('[a-zA-Z0-9./ ]+',firstLine) is None:
            connection.close()
            continue;
        if cmd.startswith('GET /'):
            newThread = HttpClientThread(connection, cmd, resultPath, latestResults, version)
            newThread.start()
        elif cmd=='get\n':
            packages[packageIndex] = packages[packageIndex].strip()
            print('[' + strDateTime() + '] get:' + packages[packageIndex])
            connection.send(packages[packageIndex])
            packageIndex += 1
            if packageIndex >= len(packages):
                packageIndex = 0
            f = open('package-index.txt', 'wt')
            f.write(str(packageIndex) + '\n')
            f.close()
            connection.close()
        elif cmd.startswith('write\nftp://'):
            # read data
            data = cmd[6:]
            try:
                t = 0
                while (len(data) < 1024 * 1024) and (not data.endswith('\nDONE')) and (t < 10):
                    d = connection.recv(1024)
                    if d:
                        t = 0
                        data += d
                    else:
                        time.sleep(0.2)
                        t += 0.2
                connection.close()
            except socket.error as e:
                pass

            pos = data.find('\n')
            if pos < 10:
                continue
            url = data[:pos]
            print('[' + strDateTime() + '] write:' + url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.gz',url)
            if res and url in packages:
                print('results added for package ' + res.group(1))
                filename = resultPath + '/' + res.group(1)
                with open(filename, 'wt') as f:
                    f.write(strDateTime() + '\n' + data)
                # track latest added results..
                if len(latestResults) >= 20:
                    latestResults = latestResults[1:]
                latestResults.append(filename)
                with open('latest.txt', 'wt') as f:
                    f.write(' '.join(latestResults))
        else:
            print('[' + strDateTime() + '] invalid command: ' + firstLine)
            connection.close()

if __name__ == "__main__":
    version = '1.85'
    for arg in sys.argv[1:]:
        if arg.startswith('--version='):
            version = arg[arg.find('=')+1:]
            print('Version:' + version)
        elif arg == '--help':
            print('Donate CPU to Cppcheck project server')
            print('')
            print('Syntax: donate-cpu-server.py [--version=revision]')
            print('  --version=revision     Use specified revision to compare head to. Default is 1.85.');
            print('')
            print('Quick start: just run this script without any arguments')
            sys.exit(0)
        else:
            print('Unhandled argument: ' + arg)
            sys.exit(1)

    workPath = os.path.expanduser('~/daca@home')
    os.chdir(workPath)
    resultPath = workPath + '/donated-results'

    f = open('packages.txt', 'rt')
    packages = f.readlines()
    f.close()

    print('packages: ' + str(len(packages)))

    if len(packages) == 0:
        print('fatal: there are no packages')
        sys.exit(1)

    packageIndex = 0
    if os.path.isfile('package-index.txt'):
        f = open('package-index.txt', 'rt')
        packageIndex = int(f.read())
        if packageIndex < 0 or packageIndex >= len(packages):
            packageIndex = 0
        f.close()

    server_address_port = 8000
    if '--test' in sys.argv[1:]:
        server_address_port = 8001

    try:
        server(server_address_port, packages, packageIndex, resultPath, version)
    except socket.timeout:
        print('Timeout!')

