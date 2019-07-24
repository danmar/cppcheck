
import logging
import socket
import subprocess

from astyle_client import receive_data

logger = logging.getLogger()


def server(port):
    socket.setdefaulttimeout(30)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('', port)
    sock.bind(server_address)

    sock.listen(1)

    while True:
        # Wait for a connection
        connection, client_address = sock.accept()

        # Read data from client
        try:
            data = receive_data(connection)
        except socket.error:
            connection.close()
            continue

        # Format
        process = subprocess.Popen(['astyle', '--options=.astylerc'],
                                   stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        comm = process.communicate(input=data)

        connection.sendall(comm[0] + '\nDONE')
        connection.close()


if __name__ == "__main__":
    server(port=18000)
