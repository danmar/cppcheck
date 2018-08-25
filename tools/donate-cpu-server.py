
# Server for 'donate-cpu.py'

import glob
import os
import socket
import re
import datetime
import time
from threading import Thread

def strDateTime():
    d = datetime.date.strftime(datetime.datetime.now().date(), '%Y-%m-%d')
    t = datetime.time.strftime(datetime.datetime.now().time(), '%H:%M')
    return d + ' ' + t

def fmt(a,b,c,d):
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
    if a != 'Package':
        pos = ret.find(' ')
        ret = '<a href="' + a + '">' + a + '</a>' + ret[pos:]
    return ret


def latestReport(latestResults):
    html = '<html><body>\n'
    html += '<h1>Latest daca@home results</h1>'
    html += '<pre>\n<b>' + fmt('Package','Date       Time ','head','1.84') + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        package = filename[filename.rfind('/')+1:]

        datestr = ''
        cppcheck = 'cppcheck/cppcheck'
        count = {'cppcheck/cppcheck':0, '1.84/cppcheck':0}
        for line in open(filename,'rt'):
            line = line.strip()
            if line.startswith('2018-'):
                datestr = line
            elif line.startswith('cppcheck:'):
                cppcheck = line[9:]
            elif re.match(r'.*:[0-9]+:[0-9]+: [a-z]+: .*\]$', line):
                count[cppcheck] += 1

        html += fmt(package, datestr, str(count['cppcheck/cppcheck']), str(count['1.84/cppcheck'])) + '\n'

    html += '</pre></body></html>\n'
    return html


def sendAll(connection, data):
    while data:
        num = connection.send(data)
        if num < len(data):
            data = data[num:]
        else:
            data = None


def httpGetResponse(connection, data, contentType):
    resp = 'HTTP/1.1 200 OK\n'
    resp += 'Connection: close\n'
    resp += 'Content-length: ' + str(len(data)) + '\n'
    resp += 'Content-type: ' + contentType + '\n\n'
    resp += data
    sendAll(connection, resp)


class HttpClientThread(Thread):
    def __init__(self, connection, cmd, latestResults):
        Thread.__init__(self)
        self.connection = connection
        self.cmd = cmd[:cmd.find('\n')]
        self.latestResults = latestResults

    def run(self):
        try:
            cmd = self.cmd
            print('[' + strDateTime() + '] ' + cmd)
            if cmd.startswith('GET /latest.html '):
                html = latestReport(self.latestResults)
                httpGetResponse(self.connection, html, 'text/html')
            else:
                package = cmd[5:]
                if package.find(' ') > 0:
                    package = package[:package.find(' ')]
                filename = os.path.expanduser('~/donated-results/') + package
                if not os.path.isfile(filename):
                    print('HTTP/1.1 404 Not Found')
                    connection.send('HTTP/1.1 404 Not Found\n\n')
                else:
                    f = open(filename,'rt')
                    data = f.read()
                    f.close()
                    httpGetResponse(self.connection, data, 'text/plain')
                    print('HttpClientThread: sleep 30 seconds..')
                    time.sleep(30)
                    print('HttpClientThread: stopping')
            time.sleep(1)
            self.connection.close()
        except:
            return

if __name__ == "__main__":
    resultPath = os.path.expanduser('~/donated-results')

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

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('', 8000)
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
            continue
        if cmd.find('\n') < 1:
            continue
        firstLine = cmd[:cmd.find('\n')]
        if re.match('[a-zA-Z0-9./ ]+',firstLine) is None:
            connection.close()
            continue;
        if cmd.startswith('GET /'):
            newThread = HttpClientThread(connection, cmd, latestResults)
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
            print('[' + strDateTime() + '] write:'+url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.gz',url)
            if res and url in packages:
                print('results added for package ' + res.group(1))
                filename = resultPath + '/' + res.group(1)
                f = open(filename, 'wt')
                f.write(strDateTime() + '\n' + data[pos+1:])
                f.close()
                # track latest added results..
                if len(latestResults) >= 20:
                    latestResults = latestResults[1:]
                latestResults.append(filename)
        else:
            print('[' + strDateTime() + '] invalid command: ' + firstLine)
            connection.close()
