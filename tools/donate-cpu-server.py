
# Server for 'donate-cpu.py'

import glob
import os
import socket
import re
import datetime
import time
from threading import Thread
import sys

OLD_VERSION = '1.86'


def strDateTime():
    d = datetime.date.strftime(datetime.datetime.now().date(), '%Y-%m-%d')
    t = datetime.time.strftime(datetime.datetime.now().time(), '%H:%M')
    return d + ' ' + t


def overviewReport():
    html = '<html><head><title>daca@home</title></head><body>\n'
    html += '<h1>daca@home</h1>\n'
    html += '<a href="crash">Crash report</a><br>\n'
    html += '<a href="diff">Diff report</a><br>\n'
    html += '<a href="head">HEAD report</a><br>\n'
    html += '<a href="latest.html">Latest results</a><br>\n'
    html += '<a href="time.html">Time report</a><br>\n'
    html += '</body></html>'
    return html


def fmt(a, b, c, d, e):
    ret = a + ' '
    while len(ret) < 10:
        ret += ' '
    if len(ret) == 10:
        ret += b[:10] + ' '
    while len(ret) < 21:
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
    html += '<pre>\n<b>' + fmt('Package', 'Date       Time ', OLD_VERSION, 'Head', 'Diff') + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        if not os.path.isfile(filename):
            continue
        package = filename[filename.rfind('/')+1:]

        datestr = ''
        count = ['0', '0']
        lost = 0
        added = 0
        for line in open(filename, 'rt'):
            line = line.strip()
            current_year = datetime.date.today().year
            if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            #elif line.startswith('cppcheck:'):
            #    cppcheck = line[9:]
            elif line.startswith('count: '):
                count = line.split(' ')[1:]
            elif line.startswith('head ') and not line.startswith('head results:'):
                added += 1
            elif line.startswith(OLD_VERSION + ' '):
                lost += 1
        diff = ''
        if lost > 0:
            diff += '-' + str(lost)
        if added > 0:
            diff += '+' + str(added)
        html += fmt(package, datestr, count[1], count[0], diff) + '\n'

    html += '</pre></body></html>\n'
    return html


def crashReport():
    html = '<html><head><title>Crash report</title></head><body>\n'
    html += '<h1>Crash report</h1>\n'
    html += '<pre>\n'
    html += '<b>Package                                 ' + OLD_VERSION + '  Head</b>\n'
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


def diffReportFromDict(out, today):
    html = '<pre>\n'
    html += '<b>MessageID                           ' + OLD_VERSION + '    Head</b>\n'
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


def diffReport(resultsPath):
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
            if line.startswith(OLD_VERSION + ' '):
                index = 0
            elif line.startswith('head '):
                index = 1
            else:
                continue
            messageId = line[line.rfind('[')+1:len(line)-1]

            if messageId not in out:
                out[messageId] = [0, 0]
            out[messageId][index] += 1
            if uploadedToday:
                if messageId not in outToday:
                    outToday[messageId] = [0, 0]
                outToday[messageId][index] += 1

    html = '<html><head><title>Diff report</title></head><body>\n'
    html += '<h1>Diff report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += diffReportFromDict(outToday, 'today')
    html += '<h2>All</h2>'
    html += diffReportFromDict(out, '')

    return html


def diffMessageIdReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*')):
        url = None
        diff = False
        for line in open(filename, 'rt'):
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
        for line in open(filename, 'rt'):
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


def headReportFromDict(out, today):
    html = '<pre>\n'
    html += '<b>MessageID                                  Count</b>\n'
    sumTotal = 0
    for messageId in sorted(out.keys()):
        line = messageId + ' '
        counts = out[messageId]
        sumTotal += counts
        if counts > 0:
            c = str(counts)
            while len(line) < 48 - len(c):
                line += ' '
            line += c + ' '
        line = '<a href="head' + today + '-' + messageId + '">' + messageId + '</a>' + line[line.find(' '):]
        html += line + '\n'

    # Sum
    html += '================================================\n'
    line = ''
    while len(line) < 48 - len(str(sumTotal)):
        line += ' '
    line += str(sumTotal) + ' '
    html += line + '\n'
    html += '</pre>\n'

    return html


def headReport(resultsPath):
    out = {}
    outToday = {}
    today = strDateTime()[:10]

    for filename in sorted(glob.glob(resultsPath + '/*')):
        if not os.path.isfile(filename):
            continue
        uploadedToday = False
        firstLine = True
        headResults = False
        for line in open(filename, 'rt'):
            if firstLine:
                if line.startswith(today):
                    uploadedToday = True
                firstLine = False
                continue
            line = line.strip()
            if line.startswith('head results:'):
                headResults = True
                continue
            if line.startswith('diff:'):
                if headResults:
                    break
            if not headResults:
                continue
            if not line.endswith(']'):
                continue
            if ': note: ' in line:
                # notes normally do not contain message ids but can end with ']'
                continue
            messageId = line[line.rfind('[')+1:len(line)-1]

            if messageId not in out:
                out[messageId] = 0
            out[messageId] += 1
            if uploadedToday:
                if messageId not in outToday:
                    outToday[messageId] = 0
                outToday[messageId] += 1

    html = '<html><head><title>HEAD report</title></head><body>\n'
    html += '<h1>HEAD report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += headReportFromDict(outToday, 'today')
    html += '<h2>All</h2>'
    html += headReportFromDict(out, '')

    return html


def headMessageIdReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*')):
        url = None
        headResults = False
        for line in open(filename, 'rt'):
            if line.startswith('ftp://'):
                url = line
            elif line.startswith('head results:'):
                headResults = True
            elif not headResults:
                continue
            elif headResults and line.startswith('diff:'):
                break
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def headMessageIdTodayReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*')):
        url = None
        headResults = False
        firstLine = True
        for line in open(filename, 'rt'):
            if firstLine:
                firstLine = False
                if not line.startswith(today):
                    break
            if line.startswith('ftp://'):
                url = line
            elif line.startswith('head results:'):
                headResults = True
            elif not headResults:
                continue
            elif headResults and line.startswith('diff:'):
                break
            elif line.endswith(e):
                if url:
                    text += url
                    url = None
                text += line
    return text


def timeReport(resultPath):
    html = '<html><head><title>Time report</title></head><body>\n'
    html += '<h1>Time report</h1>\n'
    html += '<pre>\n'
    column_widths = [25, 10, 10, 10]
    html += '<b>'
    html += 'Package '.ljust(column_widths[0]) + ' ' + \
            OLD_VERSION.rjust(column_widths[1]) + ' ' + \
            'Head'.rjust(column_widths[2]) + ' ' + \
            'Factor'.rjust(column_widths[3])
    html += '</b>\n'

    total_time_base = 0.0
    total_time_head = 0.0
    for filename in glob.glob(resultPath + '/*'):
        for line in open(filename, 'rt'):
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.strip().split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            total_time_base += time_base
            total_time_head += time_head
            suspicious_time_difference = False
            if time_base > 1 and time_base*2 < time_head:
                suspicious_time_difference = True
            elif time_head > 1 and time_head*2 < time_base:
                suspicious_time_difference = True
            if suspicious_time_difference:
                if time_base > 0.0:
                    time_factor = time_head / time_base
                else:
                    time_factor = 0.0
                html += filename[len(resultPath)+1:].ljust(column_widths[0]) + ' ' + \
                    split_line[2].rjust(column_widths[1]) + ' ' + \
                    split_line[1].rjust(column_widths[2]) + ' ' + \
                    '{:.2f}'.format(time_factor).rjust(column_widths[3]) + '\n'
            break

    html += '\n'
    if total_time_base > 0.0:
        total_time_factor = total_time_head / total_time_base
    else:
        total_time_factor = 0.0
    html += 'Total time: '.ljust(column_widths[0]) + ' ' + \
            str(total_time_base).rjust(column_widths[1]) + ' ' + \
            str(total_time_head).rjust(column_widths[2]) + ' ' + \
            '{:.2f}'.format(total_time_factor).rjust(column_widths[3])

    html += '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


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
            res = re.match(r'GET /([a-zA-Z0-9_\-\.\+]*) HTTP', cmd)
            if res is None:
                self.connection.close()
                return
            url = res.group(1)
            if url == '':
                html = overviewReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'latest.html':
                html = latestReport(self.latestResults)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'crash':
                html = crashReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'diff':
                html = diffReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('difftoday-'):
                messageId = url[10:]
                text = diffMessageIdTodayReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('diff-'):
                messageId = url[5:]
                text = diffMessageIdReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == 'head':
                html = headReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url.startswith('headtoday-'):
                messageId = url[10:]
                text = headMessageIdTodayReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url.startswith('head-'):
                messageId = url[5:]
                text = headMessageIdReport(self.resultPath, messageId)
                httpGetResponse(self.connection, text, 'text/plain')
            elif url == 'time.html':
                text = timeReport(self.resultPath)
                httpGetResponse(self.connection, text, 'text/html')
            else:
                filename = resultPath + '/' + url
                if not os.path.isfile(filename):
                    print('HTTP/1.1 404 Not Found')
                    self.connection.send('HTTP/1.1 404 Not Found\r\n\r\n')
                else:
                    f = open(filename, 'rt')
                    data = f.read()
                    f.close()
                    httpGetResponse(self.connection, data, 'text/plain')
        finally:
            time.sleep(1)
            self.connection.close()


def server(server_address_port, packages, packageIndex, resultPath):
    socket.setdefaulttimeout(30)
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
        if re.match('[a-zA-Z0-9./ ]+', firstLine) is None:
            connection.close()
            continue
        if cmd.startswith('GET /'):
            newThread = HttpClientThread(connection, cmd, resultPath, latestResults)
            newThread.start()
        elif cmd == 'GetCppcheckVersions\n':
            reply = 'head ' + OLD_VERSION
            print('[' + strDateTime() + '] GetCppcheckVersions: ' + reply)
            connection.send(reply)
            connection.close()
        elif cmd == 'get\n':
            pkg = packages[packageIndex].strip()
            packages[packageIndex] = pkg
            packageIndex += 1
            if packageIndex >= len(packages):
                packageIndex = 0

            f = open('package-index.txt', 'wt')
            f.write(str(packageIndex) + '\n')
            f.close()

            print('[' + strDateTime() + '] get:' + pkg)
            connection.send(pkg)
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
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.gz', url)
            if res is None:
                print('results not written. res is None.')
                continue
            if url not in packages:
                url2 = url + '\n'
                if url2 not in packages:
                    print('results not written. url is not in packages.')
                    continue
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
        server(server_address_port, packages, packageIndex, resultPath)
    except socket.timeout:
        print('Timeout!')

