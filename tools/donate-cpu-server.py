
# Server for 'donate-cpu.py'

import os
import socket
import re

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

while True:
    # wait for a connection
    print 'waiting for a connection'
    connection, client_address = sock.accept()

    try:
        cmd = connection.recv(16)
        if cmd=='get\n':
            packages[packageIndex] = packages[packageIndex].strip()
            print('get:' + packages[packageIndex])
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
                print('write:'+url)
                res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.gz',url)
                if res and url in packages:
                    print('results added for package ' + res.group(1))
                    f = open(resultPath + '/' + res.group(1), 'wt')
                    f.write(data[pos+1:])
                    f.close()
        else:
            print('invalid cmd')

    finally:
        connection.close()

