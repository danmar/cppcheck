import os
import socket
import sys
import time


def receive_data(conn):
    data = ''
    for t in range(1000):
        d = conn.recv(8196)
        if d:
            data += d
            if data.endswith('\nDONE'):
                return data[:-5]
        time.sleep(0.01)
    return ''


def astyle(server_address, code):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(server_address)
        sock.sendall(code + '\nDONE')
        return receive_data(sock)
    except socket.error as err:
        print('Network error: ' + str(err))
    sock.close()
    return None


if __name__ == "__main__":
    server_address = ('cppcheck.osuosl.org', 18000)

    for filename in sorted(sys.argv[1:]):
        if not (filename.endswith('.cpp') or filename.endswith('.h')):
            continue

        f = open(filename, 'rt')
        code = f.read()
        f.close()
        formatted_code = astyle(server_address, code)
        if formatted_code is None:
            break
        if code != formatted_code:
            print('Changed: ' + filename)
            f = open(filename, 'wt')
            f.write(formatted_code)
            f.close()
        else:
            print('Unchanged: ' + filename)
