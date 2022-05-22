#!/usr/bin/env python3

# Server for 'donate-cpu.py'
# Runs only under Python 3.

import collections
import glob
import json
import os
import socket
import re
import datetime
import time
import traceback
from threading import Thread
import sys
import urllib.request
import urllib.parse
import urllib.error
import logging
import logging.handlers
import operator
import html as html_lib

# Version scheme (MAJOR.MINOR.PATCH) should orientate on "Semantic Versioning" https://semver.org/
# Every change in this script should result in increasing the version number accordingly (exceptions may be cosmetic
# changes)
SERVER_VERSION = "1.3.25"

OLD_VERSION = '2.8'


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
handler_file = logging.handlers.RotatingFileHandler(filename=logfile, maxBytes=100*1024, backupCount=1)
handler_file.setLevel(logging.ERROR)
logger.addHandler(handler_file)


# Set up an exception hook for all uncaught exceptions so they can be logged
def handle_uncaught_exception(exc_type, exc_value, exc_traceback):
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return

    logging.error("Uncaught exception", exc_info=(exc_type, exc_value, exc_traceback))


sys.excepthook = handle_uncaught_exception


def strDateTime() -> str:
    return datetime.datetime.now().strftime('%Y-%m-%d %H:%M')


def dateTimeFromStr(datestr: str) -> datetime.datetime:
    return datetime.datetime.strptime(datestr, '%Y-%m-%d %H:%M')


def overviewReport() -> str:
    html = '<html><head><title>daca@home</title></head><body>\n'
    html += '<h1>daca@home</h1>\n'
    html += '<a href="crash.html">Crash report</a><br>\n'
    html += '<a href="timeout.html">Timeout report</a><br>\n'
    html += '<a href="stale.html">Stale report</a><br>\n'
    html += '<a href="diff.html">Diff report</a><br>\n'
    html += '<a href="head.html">HEAD report</a><br>\n'
    html += '<a href="latest.html">Latest results</a><br>\n'
    html += '<a href="time_lt.html">Time report (improved)</a><br>\n'
    html += '<a href="time_gt.html">Time report (regressed)</a><br>\n'
    html += '<a href="time_slow.html">Time report (slowest)</a><br>\n'
    html += '<a href="check_library_function_report.html">checkLibraryFunction report</a><br>\n'
    html += '<a href="check_library_noreturn_report.html">checkLibraryNoReturn report</a><br>\n'
    html += '<a href="check_library_use_ignore_report.html">checkLibraryUseIgnore report</a><br>\n'
    html += '<br>\n'
    html += 'Important errors:<br>\n'
    html += '<a href="head-cppcheckError">cppcheckError</a><br>\n'
    html += '<a href="head-internalAstError">internalAstError</a><br>\n'
    html += '<a href="head-syntaxError">syntaxError</a><br>\n'
    html += '<a href="head-DacaWrongData">DacaWrongData</a><br>\n'
    html += '<a href="head-dacaWrongSplitTemplateRightAngleBrackets">dacaWrongSplitTemplateRightAngleBrackets</a><br>\n'
    html += '<br>\n'
    html += 'version ' + SERVER_VERSION + '\n'
    html += '</body></html>'
    return html


def fmt(a: str, b: str, c: str = None, d: str = None, e: str = None, link: bool = True, column_width=None) -> str:
    if column_width is None:
        column_width = [40, 10, 5, 7, 7, 8]
    ret = a
    while len(ret) < column_width[0]:
        ret += ' '
    if len(ret) == column_width[0]:
        ret += ' ' + b[:10]
    while len(ret) < (column_width[0] + 1 + column_width[1]):
        ret += ' '
    ret += ' '
    if len(b) > 10:
        ret += b[-5:].rjust(column_width[2]) + ' '
    if c is not None:
        ret += c.rjust(column_width[3]) + ' '
    if d is not None:
        ret += d.rjust(column_width[4]) + ' '
    if e is not None:
        ret += e.rjust(column_width[5])
    if link:
        pos = ret.find(' ')
        ret = '<a href="' + a + '">' + a + '</a>' + ret[pos:]
    return ret


def latestReport(latestResults: list) -> str:
    html = '<html><head><title>Latest daca@home results</title></head><body>\n'
    html += '<h1>Latest daca@home results</h1>\n'
    html += '<pre>\n<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', 'Diff', link=False) + '</b>\n'

    # Write report for latest results
    for filename in latestResults:
        if not os.path.isfile(filename):
            continue
        package = filename[filename.rfind('/')+1:]
        current_year = datetime.date.today().year

        datestr = ''
        count = ['0', '0']
        lost = 0
        added = 0
        for line in open(filename, 'rt'):
            line = line.strip()
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


def crashReport(results_path: str) -> str:
    html = '<html><head><title>Crash report</title></head><body>\n'
    html += '<h1>Crash report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False) + '</b>\n'
    current_year = datetime.date.today().year
    stack_traces = {}
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        datestr = ''
        with open(filename, 'rt') as file_:
            for line in file_:
                line = line.strip()
                if line.startswith('cppcheck: '):
                    if OLD_VERSION not in line:
                        # Package results seem to be too old, skip
                        break
                    else:
                        # Current package, parse on
                        continue
                if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                    datestr = line
                elif line.startswith('count:'):
                    if line.find('Crash') < 0:
                        break
                    package = filename[filename.rfind('/')+1:]
                    counts = line.split(' ')
                    c2 = ''
                    if counts[2] == 'Crash!':
                        c2 = 'Crash'
                    c1 = ''
                    if counts[1] == 'Crash!':
                        c1 = 'Crash'
                    html += fmt(package, datestr, c2, c1) + '\n'
                    if c1 != 'Crash':
                        break
                elif line.find(' received signal ') != -1:
                    crash_line = next(file_, '').strip()
                    location_index = crash_line.rfind(' at ')
                    if location_index > 0:
                        code_line = next(file_, '').strip()
                    else:
                        code_line = ''
                    stack_trace = []
                    while True:
                        l = next(file_, '')
                        if not l.strip():
                            break
                        # #0  0x00007ffff71cbf67 in raise () from /lib64/libc.so.6
                        m = re.search(r'(?P<number>#\d+) .* in (?P<function>.+)\(.*\) from (?P<binary>.*)$', l)
                        if m:
                            #print('0 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('binary')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) from ' + m.group('binary'))
                            continue
                        # #11 0x00000000006f2414 in valueFlowNumber (tokenlist=tokenlist@entry=0x7fffffffc610) at build/valueflow.cpp:2503
                        m = re.search(r'(?P<number>#\d+) .* in (?P<function>.+?) \(.*\) at (?P<location>.*)$', l)
                        if m:
                            #print('1 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('location')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) at ' + m.group('location'))
                            continue
                        # #18 ForwardTraversal::updateRecursive (this=0x7fffffffb3c0, tok=0x14668a0) at build/forwardanalyzer.cpp:415
                        m = re.search(r'(?P<number>#\d+) (?P<function>.+)\(.*\) at (?P<location>.*)$', l)
                        if m:
                            #print('2 - {} - {} - {}'.format(m.group('number'), m.group('function'), m.group('location')))
                            stack_trace.append(m.group('number') + ' ' + m.group('function') + '(...) at ' + m.group('location'))
                            continue

                        print('{} - unmatched stack frame - {}'.format(package, l))
                        break
                    key = hash(' '.join(stack_trace))

                    if key in stack_traces:
                        stack_traces[key]['code_line'] = code_line
                        stack_traces[key]['stack_trace'] = stack_trace
                        stack_traces[key]['n'] += 1
                        stack_traces[key]['packages'].append(package)
                    else:
                        stack_traces[key] = {'stack_trace': stack_trace, 'n': 1, 'code_line': code_line, 'packages': [package], 'crash_line': crash_line}
                    break

    html += '</pre>\n'
    html += '<pre>\n'
    html += '<b>Stack traces</b>\n'
    for stack_trace in sorted(list(stack_traces.values()), key=lambda x: x['n'], reverse=True):
        html += 'Packages: ' + ' '.join(['<a href="' + p + '">' + p + '</a>' for p in stack_trace['packages']]) + '\n'
        html += html_lib.escape(stack_trace['crash_line']) + '\n'
        html += html_lib.escape(stack_trace['code_line']) + '\n'
        html += html_lib.escape('\n'.join(stack_trace['stack_trace'])) + '\n\n'
    html += '</pre>\n'

    html += '</body></html>\n'
    return html


def timeoutReport(results_path: str) -> str:
    html = '<html><head><title>Timeout report</title></head><body>\n'
    html += '<h1>Timeout report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False) + '</b>\n'
    current_year = datetime.date.today().year
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        datestr = ''
        with open(filename, 'rt') as file_:
            for line in file_:
                line = line.strip()
                if line.startswith('cppcheck: '):
                    if OLD_VERSION not in line:
                        # Package results seem to be too old, skip
                        break
                    else:
                        # Current package, parse on
                        continue
                if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                    datestr = line
                elif line.startswith('count:'):
                    if line.find('TO!') < 0:
                        break
                    package = filename[filename.rfind('/')+1:]
                    counts = line.split(' ')
                    c2 = ''
                    if counts[2] == 'TO!':
                        c2 = 'Timeout'
                    c1 = ''
                    if counts[1] == 'TO!':
                        c1 = 'Timeout'
                    html += fmt(package, datestr, c2, c1) + '\n'
                    break

    html += '</pre>\n'
    html += '</body></html>\n'
    return html


def staleReport(results_path: str) -> str:
    html = '<html><head><title>Stale report</title></head><body>\n'
    html += '<h1>Stale report</h1>\n'
    html += '<pre>\n'
    html += '<b>' + fmt('Package', 'Date       Time', link=False) + '</b>\n'
    current_year = datetime.date.today().year
    for filename in sorted(glob.glob(os.path.expanduser(results_path + '/*'))):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
            continue
        for line in open(filename, 'rt'):
            line = line.strip()
            if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
            else:
                continue
            dt = dateTimeFromStr(datestr)
            diff = datetime.datetime.now() - dt
            if diff.days < 30:
                continue
            package = filename[filename.rfind('/')+1:]
            html += fmt(package, datestr) + '\n'
            break
    html += '</pre>\n'

    html += '</body></html>\n'
    return html


def diffReportFromDict(out: dict, today: str) -> str:
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


def diffReport(resultsPath: str) -> str:
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
            if OLD_VERSION not in sums:
                continue
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


def generate_package_diff_statistics(filename: str) -> None:
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

        if line.startswith(OLD_VERSION + ' '):
            version = OLD_VERSION
        elif line.startswith('head '):
            version = 'head'
        else:
            continue

        messageId = line[line.rfind('[')+1:len(line)-1]

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


def diffMessageIdReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename, 'rt') as f:
            diff_stats = f.read()
        if messageId not in diff_stats:
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


def diffMessageIdTodayReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*.diff')):
        if not os.path.isfile(filename):
            continue
        with open(filename, 'rt') as f:
            diff_stats = f.read()
        if messageId not in diff_stats:
            continue
        if today not in diff_stats:
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


def headReportFromDict(out: dict, today: str) -> str:
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


def headReport(resultsPath: str) -> str:
    out = {}
    outToday = {}
    today = strDateTime()[:10]

    for filename in sorted(glob.glob(resultsPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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
                # notes normally do not contain message ids but can end with ']'
                continue
            message_id_start_pos = line.rfind('[')
            if message_id_start_pos <= 0:
                continue
            messageId = line[message_id_start_pos+1:len(line)-1]
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


def headMessageIdReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    for filename in sorted(glob.glob(resultPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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


def headMessageIdTodayReport(resultPath: str, messageId: str) -> str:
    text = messageId + '\n'
    e = '[' + messageId + ']\n'
    today = strDateTime()[:10]
    for filename in sorted(glob.glob(resultPath + '/*')):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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


def timeReport(resultPath: str, show_gt: bool) -> str:
    title = 'Time report ({})'.format('regressed' if show_gt else 'improved')
    html = '<html><head><title>{}</title></head><body>\n'.format(title)
    html += '<h1>{}</h1>\n'.format(title)
    html += '<pre>\n'
    column_width = [40, 10, 10, 10, 10, 10]
    html += '<b>'
    html += fmt('Package', 'Date       Time', OLD_VERSION, 'Head', 'Factor', link=False, column_width=column_width)
    html += '</b>\n'

    current_year = datetime.date.today().year

    data = {}

    total_time_base = 0.0
    total_time_head = 0.0
    for filename in glob.glob(resultPath + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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
            if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
                continue
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            if time_base < 0.0 or time_head < 0.0:
                # ignore results with crashes / errors for the time report
                break
            total_time_base += time_base
            total_time_head += time_head
            suspicious_time_difference = False
            if show_gt and time_base > 1 and time_base*2 < time_head:
                suspicious_time_difference = True
            elif not show_gt and time_head > 1 and time_head*2 < time_base:
                suspicious_time_difference = True
            if suspicious_time_difference:
                if time_base > 0.0:
                    time_factor = time_head / time_base
                else:
                    time_factor = 0.0
                pkg_name = filename[len(resultPath)+1:]
                data[pkg_name] = (datestr, split_line[2], split_line[1], time_factor)
            break

    sorted_data = sorted(data.items(), key=lambda kv: kv[1][3], reverse=show_gt)
    sorted_dict = collections.OrderedDict(sorted_data)
    for key in sorted_dict:
        html += fmt(key, sorted_dict[key][0], sorted_dict[key][1], sorted_dict[key][2], '{:.2f}'.format(sorted_dict[key][3]),
                    column_width=column_width) + '\n'

    html += '\n'
    html += '(listed above are all suspicious timings with a factor '
    if show_gt:
        html += '&gt;2.00'
    else:
        html += '&lt;=0.50'
    html += ')\n'
    html += '\n'
    if total_time_base > 0.0:
        total_time_factor = total_time_head / total_time_base
    else:
        total_time_factor = 0.0
    html += 'Time for all packages (not just the ones listed above):\n'
    html += fmt('Total time:',
            '',
            '{:.1f}'.format(total_time_base),
            '{:.1f}'.format(total_time_head),
            '{:.2f}'.format(total_time_factor), link=False, column_width=column_width)

    html += '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


def timeReportSlow(resultPath: str) -> str:
    title = 'Time report (slowest)'
    html = '<html><head><title>{}</title></head><body>\n'.format(title)
    html += '<h1>{}</h1>\n'.format(title)
    html += '<pre>\n'
    html += '<b>'
    html += fmt('Package', 'Date       Time', OLD_VERSION, 'Head', link=False)
    html += '</b>\n'

    current_year = datetime.date.today().year

    data = {}

    for filename in glob.glob(resultPath + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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
            if line.startswith(str(current_year) + '-') or line.startswith(str(current_year - 1) + '-'):
                datestr = line
                continue
            elif line.startswith('count:'):
                count_head = line.split()[1]
                if count_head == 'TO!':
                    # ignore results with timeouts
                    break
                continue
            if not line.startswith('elapsed-time:'):
                continue
            split_line = line.split()
            time_base = float(split_line[2])
            time_head = float(split_line[1])
            if time_base < 0.0 or time_head < 0.0:
                # ignore results with crashes / errors
                break
            pkg_name = filename[len(resultPath)+1:]
            data[pkg_name] = (datestr, split_line[2], split_line[1], time_head)
            break

        sorted_data = sorted(data.items(), key=lambda kv: kv[1][3])
        if len(data) > 100:
            first_key, _ = sorted_data[0]
            # remove the entry with the lowest run-time
            del data[first_key]

    sorted_data = sorted(data.items(), key=lambda kv: kv[1][3], reverse=True)
    sorted_dict = collections.OrderedDict(sorted_data)
    for key in sorted_dict:
        html += fmt(key, sorted_dict[key][0], sorted_dict[key][1], sorted_dict[key][2]) + '\n'
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


def check_library_report(result_path: str, message_id: str) -> str:
    if message_id not in ('checkLibraryNoReturn', 'checkLibraryFunction', 'checkLibraryUseIgnore'):
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
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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
                if message_id == 'checkLibraryFunction':
                    function_name = line[(line.find('for function ') + len('for function ')):line.rfind('[') - 1]
                else:
                    function_name = line[(line.find(': Function ') + len(': Function ')):line.rfind('should have') - 1]
                function_counts[function_name] = function_counts.setdefault(function_name, 0) + 1

    function_details_list = []
    for function_name, count in sorted(list(function_counts.items()), key=operator.itemgetter(1), reverse=True):
        if len(function_details_list) >= functions_shown_max:
            break
        function_details_list.append(str(count).rjust(column_widths[0]) + ' ' +
                '<a href="check_library-' + urllib.parse.quote_plus(function_name) + '">' + function_name + '</a>\n')

    html += ''.join(function_details_list)
    html += '</pre>\n'
    html += '</body></html>\n'

    return html


# Lists all checkLibrary* messages regarding the given function name
def check_library_function_name(result_path: str, function_name: str) -> str:
    print('check_library_function_name')
    function_name = urllib.parse.unquote_plus(function_name)
    output_lines_list = []
    for filename in glob.glob(result_path + '/*'):
        if not os.path.isfile(filename) or filename.endswith('.diff'):
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


def sendAll(connection: socket.socket, text: str) -> None:
    data = text.encode('utf-8', 'ignore')
    while data:
        num = connection.send(data)
        if num < len(data):
            data = data[num:]
        else:
            data = None


def httpGetResponse(connection: socket.socket, data: str, contentType: str) -> None:
    resp = 'HTTP/1.1 200 OK\r\n'
    resp += 'Connection: close\r\n'
    resp += 'Content-length: ' + str(len(data)) + '\r\n'
    resp += 'Content-type: ' + contentType + '\r\n\r\n'
    resp += data
    sendAll(connection, resp)


class HttpClientThread(Thread):
    def __init__(self, connection: socket.socket, cmd: str, resultPath: str, latestResults: list) -> None:
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
                html = crashReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'timeout.html':
                html = timeoutReport(self.resultPath)
                httpGetResponse(self.connection, html, 'text/html')
            elif url == 'stale.html':
                html = staleReport(self.resultPath)
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
            elif url == 'time_lt.html':
                text = timeReport(self.resultPath, False)
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'time_gt.html':
                text = timeReport(self.resultPath, True)
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'time_slow.html':
                text = timeReportSlow(self.resultPath)
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'check_library_function_report.html':
                text = check_library_report(self.resultPath + '/' + 'info_output', message_id='checkLibraryFunction')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'check_library_noreturn_report.html':
                text = check_library_report(self.resultPath + '/' + 'info_output', message_id='checkLibraryNoReturn')
                httpGetResponse(self.connection, text, 'text/html')
            elif url == 'check_library_use_ignore_report.html':
                text = check_library_report(self.resultPath + '/' + 'info_output', message_id='checkLibraryUseIgnore')
                httpGetResponse(self.connection, text, 'text/html')
            elif url.startswith('check_library-'):
                print('check library function !')
                function_name = url[len('check_library-'):]
                text = check_library_function_name(self.resultPath + '/' + 'info_output', function_name)
                httpGetResponse(self.connection, text, 'text/plain')
            else:
                filename = resultPath + '/' + url
                if not os.path.isfile(filename):
                    print('HTTP/1.1 404 Not Found')
                    self.connection.send(b'HTTP/1.1 404 Not Found\r\n\r\n')
                else:
                    f = open(filename, 'rt')
                    data = f.read()
                    f.close()
                    httpGetResponse(self.connection, data, 'text/plain')
        except:
            tb = "".join(traceback.format_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]))
            print(tb)
            httpGetResponse(self.connection, tb, 'text/plain')
        finally:
            time.sleep(1)
            self.connection.close()


def server(server_address_port: int, packages: list, packageIndex: int, resultPath: str) -> None:
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
    print('[' + strDateTime() + '] listening on port ' + str(server_address_port))

    while True:
        # wait for a connection
        print('[' + strDateTime() + '] waiting for a connection')
        connection, client_address = sock.accept()
        try:
            bytes_received = connection.recv(128)
            cmd = bytes_received.decode('utf-8', 'ignore')
        except socket.error:
            connection.close()
            continue
        except UnicodeDecodeError as e:
            connection.close()
            print('Error: Decoding failed: ' + str(e))
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
            connection.send(reply.encode('utf-8', 'ignore'))
            connection.close()
        elif cmd == 'get\n':
            pkg = packages[packageIndex]
            packageIndex += 1
            if packageIndex >= len(packages):
                packageIndex = 0

            f = open('package-index.txt', 'wt')
            f.write(str(packageIndex) + '\n')
            f.close()

            print('[' + strDateTime() + '] get:' + pkg)
            connection.send(pkg.encode('utf-8', 'ignore'))
            connection.close()
        elif cmd.startswith('write\nftp://') or cmd.startswith('write\nhttp://'):
            # read data
            data = cmd[cmd.find('\n')+1:]
            try:
                t = 0.0
                max_data_size = 2 * 1024 * 1024
                while (len(data) < max_data_size) and (not data.endswith('\nDONE')) and (t < 10):
                    bytes_received = connection.recv(1024)
                    if bytes_received:
                        try:
                            text_received = bytes_received.decode('utf-8', 'ignore')
                        except UnicodeDecodeError as e:
                            print('Error: Decoding failed: ' + str(e))
                            data = ''
                            break
                        t = 0.0
                        data += text_received
                    else:
                        time.sleep(0.2)
                        t += 0.2
                connection.close()
            except socket.error:
                pass

            pos = data.find('\n')
            if pos < 10:
                continue
            url = data[:pos]
            print('[' + strDateTime() + '] write:' + url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2|xz)', url)
            if res is None:
                res = re.match(r'https?://cppcheck\.sf\.net/([a-z]+).tgz', url)
            if res is None:
                print('results not written. res is None.')
                continue
            if url not in packages:
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
        elif cmd.startswith('write_info\nftp://') or cmd.startswith('write_info\nhttp://'):
            # read data
            data = cmd[cmd.find('\n') + 1:]
            try:
                t = 0.0
                max_data_size = 1024 * 1024
                while (len(data) < max_data_size) and (not data.endswith('\nDONE')) and (t < 10):
                    bytes_received = connection.recv(1024)
                    if bytes_received:
                        try:
                            text_received = bytes_received.decode('utf-8', 'ignore')
                        except UnicodeDecodeError as e:
                            print('Error: Decoding failed: ' + str(e))
                            data = ''
                            break
                        t = 0.0
                        data += text_received
                    else:
                        time.sleep(0.2)
                        t += 0.2
                connection.close()
            except socket.error:
                pass

            pos = data.find('\n')
            if pos < 10:
                continue
            url = data[:pos]
            print('[' + strDateTime() + '] write_info:' + url)

            # save data
            res = re.match(r'ftp://.*pool/main/[^/]+/([^/]+)/[^/]*tar.(gz|bz2|xz)', url)
            if res is None:
                res = re.match(r'https://cppcheck\.sf\.net/([a-z]+).tgz', url)
            if res is None:
                print('info output not written. res is None.')
                continue
            if url not in packages:
                print('info output not written. url is not in packages.')
                continue
            print('adding info output for package ' + res.group(1))
            info_path = resultPath + '/' + 'info_output'
            if not os.path.exists(info_path):
                os.mkdir(info_path)
            filename = info_path + '/' + res.group(1)
            with open(filename, 'wt') as f:
                f.write(strDateTime() + '\n' + data)
        elif cmd == 'getPackagesCount\n':
            packages_count = str(len(packages))
            connection.send(packages_count.encode('utf-8', 'ignore'))
            connection.close()
            print('[' + strDateTime() + '] getPackagesCount: ' + packages_count)
            continue
        elif cmd.startswith('getPackageIdx'):
            request_idx = abs(int(cmd[len('getPackageIdx:'):]))
            if request_idx < len(packages):
                pkg = packages[request_idx]
                connection.send(pkg.encode('utf-8', 'ignore'))
                connection.close()
                print('[' + strDateTime() + '] getPackageIdx: ' + pkg)
            else:
                connection.close()
                print('[' + strDateTime() + '] getPackageIdx: index is out of range')
            continue
        else:
            if cmd.find('\n') < 0:
                print('[' + strDateTime() + '] invalid command: "' + firstLine + '"')
            else:
                lines = cmd.split('\n')
                s = '\\n'.join(lines[:2])
                if len(lines) > 2:
                    s += '...'
                print('[' + strDateTime() + '] invalid command: "' + s + '"')
            connection.close()


if __name__ == "__main__":
    workPath = '/var/daca@home'
    if not os.path.isdir(workPath):
        workPath = os.path.expanduser('~/daca@home')
    os.chdir(workPath)
    print('work path: ' + workPath)
    resultPath = workPath + '/donated-results'

    f = open('packages.txt', 'rt')
    packages = [val.strip() for val in f.readlines()]
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
