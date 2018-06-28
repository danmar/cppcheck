#!/usr/bin/env python
import os
import sys
import re


def readdate(data):
    if data[:5] == 'DATE ':
        datepos = 0
    else:
        datepos = data.find('\nDATE ')
        if datepos >= 0:
            datepos += 1

    if datepos < 0:
        return None

    datestr = ''
    datepos += 5
    while True:
        if datepos >= len(data):
            return None
        d = data[datepos]
        if '0' <= d <= '9':
            datestr += d
        elif d == '\n' or d == '\r':
            if len(datestr) == 8:
                return datestr[:4] + '-' + datestr[4:6] + '-' + datestr[6:]
            return None
        elif d != ' ' and d != '-':
            return None
        datepos += 1


def summaryHtml(style, font, severity, categories, totalNumber):
    font1 = ''
    font2 = ''
    if font:
        font1 = font
        font2 = '</font>'
    ret = '<tr> '
    ret = ret + '<td ' + style + '>' + font1 + severity + font2 + '</td> '
    ret = ret + '<td ' + style + '>' + font1 + str(categories) + font2 + '</td> '
    ret = ret + '<td ' + style + '>' + font1 + str(totalNumber) + font2 + '</td> '
    ret = ret + '</tr>\n'
    return ret

daca2folder = os.path.expanduser('~/daca2/')
path = ''
for arg in sys.argv[1:]:
    if arg.startswith('--daca2='):
        daca2folder = arg[8:]
        if daca2folder[-1] != '/':
            daca2folder += '/'
    else:
        path = arg
        if path[-1] != '/':
            path += '/'

mainpage = open(path + 'daca2.html', 'wt')
mainpage.write('<!DOCTYPE html>\n')
mainpage.write('<html lang="en">\n')
mainpage.write('<head>\n')
mainpage.write('<meta charset="utf-8">\n')
mainpage.write('<title>DACA2</title>\n')
mainpage.write('<link rel="stylesheet" href="/site/css/daca2.css">\n')
mainpage.write('<script src="/site/js/sorttable.min.js"></script>\n')
mainpage.write('</head>\n')
mainpage.write('<body>\n')
mainpage.write('<h1>DACA2</h1>\n')
mainpage.write('<p>Results when running latest (git head) Cppcheck on Debian.</p>\n')
mainpage.write('<p>For performance reasons the analysis is limited. Files larger than 1mb are skipped. ' +
               'If analysis of a file takes more than 10 minutes it may be stopped.</p>\n')
mainpage.write('<table class="sortable">\n')
mainpage.write(
    '<tr>' +
    '<th>Name</th>' +
    '<th>Date</th>' +
    '<th>Error</th>' +
    '<th>Warning</th>' +
    '<th>Performance</th>' +
    '<th>Portability</th>' +
    '<th>Style</th>' +
    '<th>Crashes</th>' +
    '<th>VarID 0</th></tr>\n')

lastupdate = None
recent = []

totalNumber = {}
categories = {}
for severity in ['error', 'warning', 'style', 'portability', 'performance']:
    totalNumber[severity] = 0
    categories[severity] = []

daca2 = daca2folder
pattern = re.compile(r'.*: (error|warning|style|performance|portability):.* \[([a-zA-Z0-9_\\-]+)\]')
for lib in [False, True]:
    for a in "0123456789abcdefghijklmnopqrstuvwxyz":
        if lib:
            a = "lib" + a
        if not os.path.isfile(daca2 + a + '/results-head.txt'):
            continue

        f = open(daca2 + a + '/results-head.txt', 'rt')
        data = f.read()
        f.close()

        if 'ftp://' not in data:
            continue

        datestr = readdate(data)

        if datestr:
            if not lastupdate or datestr > lastupdate:
                lastupdate = datestr
                recent = []
            if datestr == lastupdate:
                recent.append(a)
        else:
            datestr = ''

        for line in data.split('\n'):
            res = pattern.match(line)
            if res is None:
                continue
            severity = res.group(1)
            messageId = res.group(2)
            if messageId in ['cppcheckError', 'internalAstError', 'preprocessorErrorDirective', 'syntaxError']:
                continue
            totalNumber[severity] = totalNumber[severity] + 1
            if messageId not in categories[severity]:
                categories[severity].append(messageId)

        mainpage.write(
            '<tr>' +
            '<td><a href="daca2-' + a + '.html">' + a + '</a></td>' +
            '<td>' + datestr + '</td>' +
            '<td>' + str(data.count(': error:')) + '</td>' +
            '<td>' + str(data.count(': warning:')) + '</td>' +
            '<td>' + str(data.count(': performance:')) + '</td>' +
            '<td>' + str(data.count(': portability:')) + '</td>' +
            '<td>' + str(data.count(': style:')) + '</td>' +
            '<td>' + str(data.count('Crash?')) + '</td>' +
            '<td>' + str(data.count('with varid 0.')) + '</td>' +
            '</tr>\n')

        data = data.replace('&', '&amp;')
        data = data.replace('<', '&lt;')
        data = data.replace('>', '&gt;')

        f = open(path + 'daca2-' + a + '.html', 'wt')
        f.write('<!DOCTYPE html>\n')
        f.write('<html lang="en">\n')
        f.write('<head>\n')
        f.write('<meta charset="utf-8">\n')
        f.write('<title>DACA2 - ' + a + '</title>\n')
        f.write('</head>\n')
        f.write('<body>\n')
        f.write('<h1>DACA2 - ' + a + '</h1>')
        f.write('<pre>\n' + data + '</pre>\n')
        f.write('</body>\n')
        f.write('</html>\n')
        f.close()

mainpage.write('</table>\n')

mainpage.write('<b>Summary</b>\n')
mainpage.write('<table>\n')
mainpage.write(summaryHtml('style="background-color:#369"', '<font style="color:#eee">', 'Severity', 'Types', 'Amount'))
mainpage.write(summaryHtml('style="background-color:#fff"', None, 'Error (there is bug)', len(categories['error']), totalNumber['error']))
mainpage.write(summaryHtml('style="background-color:#ccccff"', None, 'Warning (potential bug)', len(categories['warning']), totalNumber['warning']))
mainpage.write(summaryHtml('style="background-color:#fff"', None, 'Style', len(categories['style']), totalNumber['style']))
mainpage.write(summaryHtml('style="background-color:#ccccff"', None, 'Portability', len(categories['portability']), totalNumber['portability']))
mainpage.write(summaryHtml('style="background-color:#fff"', None, 'Performance', len(categories['performance']), totalNumber['performance']))
mainpage.write('</table>\n')

if lastupdate:
    mainpage.write('<p>Last update: ' + lastupdate + '</p>')
    allrecent = ''
    for r in recent:
        allrecent = allrecent + ' <a href="daca2-' + r + '.html">' + r + '</a>'
    mainpage.write('<p>Most recently updated:' + allrecent + '</p>')

mainpage.write('</body>\n')
mainpage.write('</html>\n')
