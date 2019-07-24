
# Server for 'donate-cpu.py'

import glob
import json
import os
import socket
import re
import datetime
import time
from threading import Thread
import sys
import urllib
import logging
import logging.handlers
import operator

# Version scheme (MAJOR.MINOR.PATCH) should orientate on "Semantic Versioning" https://semver.org/
# Every change in this script should result in increasing the version number accordingly (exceptions may be cosmetic
# changes)
SERVER_VERSION = "1.1.4"

OLD_VERSION = '1.88'


# Set up logging
logger = logging.getLogger()
logger.setLevel(logging.INFO)
# Logging to console
handler_stream = logging.StreamHandler()
logger.addHandler(handler_stream)
# Log errors to a rotating file
logfile = sys.path[0]
if logfile:
    logfile += '/'
logfile += 'donate-cpu-server.log'
handler_file = logging.handlers.RotatingFileHandler(
    filename=logfile, maxBytes=100 * 1024, backupCount=1)
handler_file.setLevel(logging.ERROR)
logger.addHandler(handler_file)


# Set up an exception hook for all uncaught exceptions so they can be logged
def handle_uncaught_exception(exc_type, exc_value, exc_traceback):
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return

    logging.error(
        "Uncaught exception",
        exc_info=(
            exc_type,
            exc_value,
            exc_traceback))


sys.excepthook = handle_uncaught_exception


def strDateTime():
    return datetime.datetime.now().strftime('%Y-%m-%d %H:%M')


def dateTimeFromStr(datestr):
    return datetime.datetime.strptime(datestr, '%Y-%m-%d %H:%M')


def overviewReport():
    html = '<html><head><title>daca@home</title></head><body>\n'
    html += '<h1>daca@home</h1>\n'
    html += '<a href="crash.html">Crash report</a><br>\n'
    html += '<a href="stale.html">Stale report</a><br>\n'
    html += '<a href="diff.html">Diff report</a><br>\n'
    html += '<a href="head.html">HEAD report</a><br>\n'
    html += '<a href="latest.html">Latest results</a><br>\n'
    html += '<a href="time.html">Time report</a><br>\n'
    html += '<a href="check_library_function_report.html">checkLibraryFunction report</a><br>\n'
    html += '<a href="check_library_noreturn_report.html">checkLibraryNoReturn report</a><br>\n'
    html += '<a href="check_library_use_ignore_report.html">checkLibraryUseIgnore report</a><br>\n'
    html += '<br>\n'
    html += 'version ' + SERVER_VERSION + '\n'
    html += '</body></html>'
    return html


def fmt(a, b, c, d, e):
    column_width = [40, 10, 5, 6, 6, 8]
    ret = a
    while len(ret) < column_width[0]:
        ret += ' '
    if len(ret) == column_width[0]:
        ret += ' ' + b[:10]
    while len(ret) < (column_width[0] + 1 + column_width[1]):
        ret += ' '
    ret += ' '
    ret += b[-5:].rjust(column_width[2]) + ' '
    if not c is None:
        ret += c.rjust(column_width[3]) + ' '
    if not d is None:
        ret += d.rjust(column_width[4]) + ' '
    if not e is None:
        ret += e.rjust(column_width[5])
    if a != 'Package':
        pos = ret.find(' ')
        ret = '<a href="' + a + '">' + a + '</a>' + ret[pos:]
    return ret


def latestReport(latestResults):
    html = '<html><head><title>Latest daca@home results</title></head><body>\n'
    html += '<h1>Latest daca@home results</h1>\n'
    html += '<pre>\n<b>' + fmt('Package',
                               'Date       Time',
                               OLD_VERSION,
                               'Head',
                               'Diff') + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        if not os.path.isfile(filename):
            continue
        package = filename[filename.rfind('/') + 1:]
        current_year = datetime.date.today().year

        datestr = ''
        count = ['0', '0']
        lost = 0
        added = 0
        for line in open(filename, 'rt'):
            line = line.strip()
            if line.startswith(
                    str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            # elif line.startswith('cppcheck:'):
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
    html += '<b>' + fmt('Package', 'Date       Time',
                        OLD_VERSION, 'Head', None) + '</b>\n'
    current_year = datetime.date.today().year
    for filename in sorted(
            glob.glob(os.path.expanduser('~/daca@home/donated-results/*'))):
        if not os.path.isfile(filename):
            continue
        datestr = ''
        for line in open(filename, 'rt'):
            line = line.strip()
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                else:
                    # Current package, parse on
                    continue
            if line.startswith(
                    str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            if not line.startswith('count:'):
                continue
            if line.find('Crash') < 0:
                break
            package = filename[filename.rfind('/') + 1:]
            counts = line.strip().split(' ')
            c2 = ''
            if counts[2] == 'Crash!':
                c2 = 'Crash'
            c1 = ''
            if counts[1] == 'Crash!':
                c1 = 'Crash'
            html += fmt(package, datestr, c2, c1, None) + '\n'
            break
    html += '</pre>\n'

    html += '</body></html>\n'
    return html


def staleReport():
    html = '<html><head><title>Stale report</title></head><body>\n'
    html += '<h1>Stale report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time',
                        None, None, None) + '</b>\n'
    current_year = datetime.date.today().year
    for filename in sorted(
            glob.glob(os.path.expanduser('~/daca@home/donated-results/*'))):
        if not os.path.isfile(filename):
            continue
        for line in open(filename, 'rt'):
            line = line.strip()
            if line.startswith(
                    str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            else:
                continue
            dt = dateTimeFromStr(datestr)
            diff = datetime.datetime.now() - dt
            if diff.days < 30:
                continue
            package = filename[filename.rfind('/') + 1:]
            html += fmt(package, datestr, None, None, None) + '\n'
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
        line = '<a href="diff' + today + '-' + messageId + \
            '">' + messageId + '</a>' + line[line.find(' '):]
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

    for filename in sorted(glob.glob(resultsPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename, 'rt') as f:
            data = json.loads(f.read())
        uploadedToday = data['date'] == today
        for messageId in data['sums']:
            sums = data['sums'][messageId]
            if messageId not in out:
                out[messageId] = [0, 0]
            out[messageId][0] += sums[OLD_VERSION]
            out[messageId][1] += sums['head']
            if uploadedToday:
                if messageId not in outToday:
                    outToday[messageId] = [0, 0]
                outToday[messageId][0] += sums[OLD_VERSION]
                outToday[messageId][1] += sums['head']

    html = '<html><head><title>Diff report</title></head><body>\n'
    html += '<h1>Diff report</h1>\n'
    html += '<h2>Uploaded today</h2>'
    html += diffReportFromDict(outToday, 'today')
    html += '<h2>All</h2>'
    html += diffReportFromDict(out, '')

    return html


def generate_package_diff_statistics(filename):
    is_diff = False

    sums = {}

    for line in open(filename, 'rt'):
        line = line.strip()
        if line == 'diff:':
            is_diff = True
            continue
        elif not is_diff:
            continue
        if not line.endswith(']'):
            continue

        version = None
        if line.startswith(OLD_VERSION + ' '):
            version = OLD_VERSION
        elif line.startswith('head '):
            version = 'head'
        else:
            continue

        messageId = line[line.rfind('[') + 1:len(line) - 1]

        if messageId not in sums:
            sums[messageId] = {OLD_VERSION: 0, 'head': 0}

        sums[messageId][version] += 1

    output = {'date': strDateTime()[:10], 'sums': sums}

    filename_diff = filename + '.diff'
    if sums:
        with open(filename_diff, 'wt') as f:
            f.write(json.dumps(output))
    elif os.path.isfile(filename_diff):
        os.remove(filename_diff)


def diffMessageIdReport(resultPath, messageId):
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename, 'rt') as f:
            diff_stats = f.read()
        if not messageId in diff_stats:
            continue
        url = None
        diff = False
        for line in open(filename[:-5], 'rt'):
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
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename, 'rt') as f:
            diff_stats = f.read()
        if not messageId in diff_stats:
            continue
        if not today in diff_stats:
            continue
        url = None
        diff = False
        firstLine = True
        for line in open(filename[:-5], 'rt'):
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
        line = '<a href="head' + today + '-' + messageId + \
            '">' + messageId + '</a>' + line[line.find(' '):]
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
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                else:
                    # Current package, parse on
                    continue
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
                # notes normally do not contain message ids but can end with
                # ']'
                continue
            message_id_start_pos = line.rfind('[')
            if message_id_start_pos <= 0:
                continue
            messageId = line[message_id_start_pos + 1:len(line) - 1]
            if ' ' in messageId:
                # skip invalid messageIds
                continue

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
        if not os.path.isfile(filename):
            continue
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
        if not os.path.isfile(filename):
            continue
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
        if not os.path.isfile(filename):
            continue
        for line in open(filename, 'rt'):
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                else:
                    # Current package, parse on
                    continue
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.strip().split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            if time_base < 0.0 or time_head < 0.0:
                # ignore results with crashes / errors for the time report
                break
            total_time_base += time_base
            total_time_head += time_head
            suspicious_time_difference = False
            if time_base > 1 and time_base * 2 < time_head:
                suspicious_time_difference = True
            elif time_head > 1 and time_head * 2 < time_base:
                suspicious_time_difference = True
            if suspicious_time_difference:
                if time_base > 0.0:
                    time_factor = time_head / time_base
                else:
                    time_factor = 0.0
                html += filename[len(resultPath) + 1:].ljust(column_widths[0]) + ' ' + \
                    split_line[2].rjust(column_widths[1]) + ' ' + \
                    split_line[1].rjust(column_widths[2]) + ' ' + \
                    '{:.2f}'.format(time_factor).rjust(column_widths[3]) + '\n'
            break

    html += '\n'
    if total_time_base > 0.0:
        total_time_factor = total_time_head / total_time_base
    else:
        total_time_factor = 0.0
    html += 'Time for all packages (not just the ones listed above):\n'
    html += 'Total time: '.ljust(column_widths[0]) + ' ' + \
            '{:.1f}'.format(total_time_base).rjust(column_widths[1]) + ' ' + \
            '{:.1f}'.format(total_time_head).rjust(column_widths[2]) + ' ' + \
            '{:.2f}'.format(total_time_factor).rjust(column_widths[3])

    html += '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


def check_library_report(result_path, message_id):
    if message_id not in ('checkLibraryNoReturn',
                          'checkLibraryFunction', 'checkLibraryUseIgnore'):
        error_message = 'Invalid value ' + message_id + ' for message_id parameter.'
        print(error_message)
        return error_message

    functions_shown_max = 50000
    html = '<html><head><title>' + message_id + ' report</title></head><body>\n'
    html += '<h1>' + message_id + ' report</h1>\n'
    html += 'Top ' + str(functions_shown_max) + ' functions are shown.'
    html += '<pre>\n'
    column_widths = [10, 100]
    html += '<b>'
    html += 'Count'.rjust(column_widths[0]) + ' ' + \
            'Function'
    html += '</b>\n'

    function_counts = {}
    for filename in glob.glob(result_path + '/*'):
        if not os.path.isfile(filename):
            continue
        info_messages = False
        for line in open(filename, 'rt'):
            if line.startswith('cppcheck: '):
                if OLD_VERSION not in line:
                    # Package results seem to be too old, skip
                    break
                else:
                    # Current package, parse on
                    continue
            if line == 'info messages:\n':
                info_messages = True
            if not info_messages:
                continue
            if line.endswith('[' + message_id + ']\n'):
                if message_id is 'checkLibraryFunction':
                    function_name = line[(
                        line.find('for function ') + len('for function ')):line.rfind('[') - 1]
                else:
                    function_name = line[(
                        line.find(': Function ') + len(': Function ')):line.rfind('should have') - 1]
                function_counts[function_name] = function_counts.setdefault(
                    function_name, 0) + 1

    function_details_list = []
    for function_name, count in sorted(
            function_counts.items(), key=operator.itemgetter(1), reverse=True):
        if len(function_details_list) >= functions_shown_max:
            break
        function_details_list.append(str(count).rjust(column_widths[0]) + ' ' +
                                     '<a href="check_library-' + urllib.quote_plus(function_name) + '">' + function_name + '</a>\n')

    html += ''.join(function_details_list)
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


# Lists all checkLibrary* messages regarding the given function name
def check_library_function_name(result_path, function_name):
    print('check_library_function_name')
    function_name = urllib.unquote_plus(function_name)
    output_lines_list = []
    for filename in glob.glob(result_path + '/*'):
        if not os.path.isfile(filename):
            continue
        info_messages = False
        url = None
        cppcheck_options = None
        for line in open(filename, 'rt'):
            if line.startswith('ftp://'):
                url = line
            elif line.startswith('cppcheck-options:'):
                cppcheck_options = line
            elif line == 'info messages:\n':
                info_messages = True
            if not info_messages:
                continue
            if '[checkLibrary' in line:
                if (' ' + function_name) in line:
                    if url:
                        output_lines_list.append(url)
                        url = None
                    if cppcheck_options:
                        output_lines_list.append(cppcheck_options)
                        cppcheck_options = None
                    output_lines_list.append(line)

    return ''.join(output_lines_list)


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
            res = re.match(r'GET /([a-zA-Z0-9_\-\.\+%]*) HTTP', cmd)
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
            elif url == 'crash.html':
                html = crashReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'stale.html':
                html = staleReport()
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'diff.html':
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
            elif url == 'head.html':
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
            elif url == 'check_library_function_report.html':
                text = check_library_report(
                    self.resultPath + '/' + 'info_output',
                    message_id='checkLibraryFunction')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'check_library_noreturn_report.html':
                text = check_library_report(
                    self.resultPath + '/' + 'info_output',
                    message_id='checkLibraryNoReturn')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'check_library_use_ignore_report.html':
                text = check_library_report(
                    self.resultPath + '/' + 'info_output',
                    message_id='checkLibraryUseIgnore')
                httpGetResponse(self.connection, text, 'text/html')
            elif url.startswith('check_library-'):
                print('check library function !')
                function_name = url[len('check_library-'):]
                text = check_library_function_name(
                    self.resultPath + '/' + 'info_output', function_name)
                httpGetResponse(self.connection, text, 'text/plain')
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

    print('[' + strDateTime() + '] version ' + SERVER_VERSION)
    print(
        '[' +
        strDateTime() +
        '] listening on port ' +
        str(server_address_port))

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
            newThread = HttpClientThread(
                connection, cmd, resultPath, latestResults)
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
            data = cmd[cmd.find('ftp'):]
            try:
                t = 0
                max_data_size = 2 * 1024 * 1024
                while (len(data) < max_data_size) and (
                        not data.endswith('\nDONE')) and (t < 10):
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
            res = re.match(
                r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2)', url)
            if res is None:
                print('results not written. res is None.')
                continue
            if url not in packages:
                url2 = url + '\n'
                if url2 not in packages:
                    print('results not written. url is not in packages.')
                    continue
            # Verify that head was compared to correct OLD_VERSION
            versions_found = False
            old_version_wrong = False
            for line in data.split('\n', 20):
                if line.startswith('cppcheck: '):
                    versions_found = True
                    if OLD_VERSION not in line.split():
                        print('Compared to wrong old version. Should be ' + OLD_VERSION + '. Versions compared: ' +
                              line)
                        print('Ignoring data.')
                        old_version_wrong = True
                    break
            if not versions_found:
                print('Cppcheck versions missing in result data. Ignoring data.')
                continue
            if old_version_wrong:
                continue
            print('results added for package ' + res.group(1))
            filename = os.path.join(resultPath, res.group(1))
            with open(filename, 'wt') as f:
                f.write(strDateTime() + '\n' + data)
            # track latest added results..
            if len(latestResults) >= 20:
                latestResults = latestResults[1:]
            latestResults.append(filename)
            with open('latest.txt', 'wt') as f:
                f.write(' '.join(latestResults))
            # generate package.diff..
            generate_package_diff_statistics(filename)
        elif cmd.startswith('write_info\nftp://'):
            # read data
            data = cmd[11:]
            try:
                t = 0
                max_data_size = 1024 * 1024
                while (len(data) < max_data_size) and (
                        not data.endswith('\nDONE')) and (t < 10):
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
            print('[' + strDateTime() + '] write_info:' + url)

            # save data
            res = re.match(
                r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2)', url)
            if res is None:
                print('info output not written. res is None.')
                continue
            if url not in packages:
                url2 = url + '\n'
                if url2 not in packages:
                    print('info output not written. url is not in packages.')
                    continue
            print('adding info output for package ' + res.group(1))
            info_path = resultPath + '/' + 'info_output'
            if not os.path.exists(info_path):
                os.mkdir(info_path)
            filename = info_path + '/' + res.group(1)
            with open(filename, 'wt') as f:
                f.write(strDateTime() + '\n' + data)
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
