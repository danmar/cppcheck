
# Server for 'donate-cpu.py'

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


def latestReport(latestResults):
    html = '<html><head><title>Latest daca@home results</title></head><body>\n'
    html += '<h1>Latest daca@home results</h1>'
    html += '<pre>\n<b>' + fmt('Package','Date       Time ','1.84','Head','Diff') + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
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
            elif line.startswith('1.84 '):
                lost += 1
        diff = ''
        if lost > 0:
            diff += '-' + str(lost)
        if added > 0:
            diff += '+' + str(added)
        html += fmt(package, datestr, count[1], count[0], diff) + '\n'

    html += '</pre></body></html>\n'
    return html


def diffReport():
    html = '<html><head><title>Diff report</title></head><body>\n'
    html += '<h1>Diff report</h1>\n'
    html += '<pre>\n'

    out = {}

    # grep '^1.84 .*\]$' donated-results/* | sed 's/.*\[\(.*\)\]/\1/' | sort | uniq -c
    p = subprocess.Popen(['./getdiff.sh', '1.84'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0]
    for line in stdout.split('\n'):
        a = line.strip().split()
        if len(a) == 2:
            count     = a[0]
            messageId = a[1]
            out[messageId] = [count, '0']

    # grep '^head .*\]$' donated-results/* | sed 's/.*\[\(.*\)\]/\1/' | sort | uniq -c
    p = subprocess.Popen(['./getdiff.sh', 'head'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0]
    for line in stdout.split('\n'):
        a = line.strip().split()
        if len(a) == 2:
            count     = a[0]
            messageId = a[1]
            if messageId in out:
                out[messageId][1] = count
            else:
                out[messageId] = ['0',count]

    html += '<b>MessageID                           1.84    Head</b>\n'
    sum0 = 0
    sum1 = 0
    for messageId in sorted(out.keys()):
        line = messageId + ' '
        counts = out[messageId]
        sum0 += int(counts[0])
        sum1 += int(counts[1])
        if counts[0] > 0:
            c = counts[0]
            while len(line) < 40 - len(c):
                line += ' '
            line += c + ' '
        if counts[1] > 0:
            c = counts[1]
            while len(line) < 48 - len(c):
                line += ' '
            line += c
        line = '<a href="diff-' + messageId + '">' + messageId + '</a>' + line[line.find(' '):]
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

    return html

def diffMessageIdReport(messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(os.path.expanduser('~/donated-results/*'))):
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
    def __init__(self, connection, cmd, resultPath, latestResults):
        Thread.__init__(self)
        self.connection = connection
        self.cmd = cmd[:cmd.find('\n')]
        self.resultPath = resultPath
        self.latestResults = latestResults

    def run(self):
        try:
            cmd = self.cmd
            print('[' + strDateTime() + '] ' + cmd)
            res = re.match(r'GET /([a-zA-Z0-9_\-\.]+) HTTP', cmd)
            if res is None:
                self.connection.close()
                return
            url = res.group(1)
            if url == 'latest.html':
                html = latestReport(self.latestResults)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'diff':
                html = diffReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('diff-'):
                messageId = url[5:]
                text = diffMessageIdReport(messageId)
                print(text)
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

def server(server_address_port, packages, packageIndex, resultPath):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('', server_address_port)
    sock.bind(server_address)

    sock.listen(1)

    latestResults = []

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
            newThread = HttpClientThread(connection, cmd, resultPath, latestResults)
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
                f = open(filename, 'wt')
                f.write(strDateTime() + '\n' + data)
                f.close()
                # track latest added results..
                if len(latestResults) >= 20:
                    latestResults = latestResults[1:]
                latestResults.append(filename)
        else:
            print('[' + strDateTime() + '] invalid command: ' + firstLine)
            connection.close()

if __name__ == "__main__":
    workPath = os.path.expanduser('~/daca@home')
    os.chdir(workPath)
    resultPath = 'donated-results'

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

    server(server_address_port, packages, packageIndex, resultPath)


