import glob
import os
import socket
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

    source_files = []
    for d in ['cli', 'gui', 'lib', 'test', 'tools']:
        if not os.path.isdir(d) and os.path.isdir('../'+d):
            d = '../' + d
        source_files += glob.glob(d + '/*.cpp')
        source_files += glob.glob(d + '/*.h')

    for filename in sorted(source_files):
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

