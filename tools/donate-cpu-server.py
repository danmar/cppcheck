
# Server for 'donate-cpu.py'

import glob
import os
import socket
import re
import datetime
import time

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
    return ret


def latestReport(latestResults):
    html = '<html><body>\n'
    html += '<h1>Latest daca@home results</h1>'
    html += '<pre>\n<b>' + fmt('Package','Time','head','1.84') + '</b>\n'

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
        bytes = connection.send(data)
        if bytes < len(data):
            data = data[bytes:]
        else:
            data = None
    time.sleep(0.5)

resultPath = os.path.expanduser('~/donated-results')

f = open('packages.txt', 'rt')
packages = f.readlines()
f.close()

print('packages:'+str(len(packages)))

if len(packages) == 0:
    print('fatal: there are no packages')
    sys.exit(1)

packageIndex = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('', 8000)
sock.bind(server_address)

sock.listen(1)

latestResults = []

while True:
    # wait for a connection
    print('[' + strDateTime() + '] waiting for a connection')
    connection, client_address = sock.accept()

    try:
        cmd = connection.recv(16)
        if cmd=='get\n':
            packages[packageIndex] = packages[packageIndex].strip()
            print('[' + strDateTime() + '] get:' + packages[packageIndex])
            connection.sendall(packages[packageIndex])
            packageIndex += 1
            if packageIndex >= len(packages):
                packageIndex = 0
        elif cmd.startswith('write\n'):
            data = cmd[6:]
            while len(data) < 1024 * 1024:
                d = connection.recv(1024)
                if d:
                    data += d
                else:
                    break
            pos = data.find('\n')
            if data.startswith('ftp://') and pos > 10:
                url = data[:pos]
                print('[' + strDateTime() + '] write:'+url)
                res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.gz',url)
                if res and url in packages:
                    print('results added for package ' + res.group(1))
                    filename = resultPath + '/' + res.group(1)
                    f = open(filename, 'wt')
                    f.write(strDateTime() + '\n' + data[pos+1:])
                    f.close()
                    if len(latestResults) >= 20:
                        latestResults = latestResults[1:]
                    latestResults.append(filename)
        elif cmd=='GET /latest.html':
            print('[' + strDateTime() + '] ' + cmd)
            html = latestReport(latestResults)
            resp = 'HTTP/1.1 200 OK\n'
            resp += 'Connection: close\n'
            resp += 'Content-length: ' + str(len(html)) + '\n'
            resp += 'Content-type: text/html\n\n'
            print(resp + '...')
            resp += html
            sendAll(connection, resp)
        elif cmd.startswith('GET /'):
            print('[' + strDateTime() + '] ' + cmd)
            connection.send('HTTP/1.1 404 Not Found\n\n')
        else:
            print('[' + strDateTime() + '] invalid command: ' + cmd)

    finally:
        connection.close()

