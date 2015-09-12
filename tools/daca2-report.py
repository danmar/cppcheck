import os
import sys
import subprocess


def readdate(data):
    datepos = -1
    if data[:5] == 'DATE ':
        datepos = 0
    else:
        datepos = data.find('\nDATE ')
        if datepos >= 0:
            datepos = datepos + 1

    if datepos < 0:
        return None

    datestr = ''
    datepos = datepos + 5
    while True:
        if datepos >= len(data):
            return None
        d = data[datepos]
        if d >= '0' and d <= '9':
            datestr = datestr + d
        elif d == '\n' or d == '\r':
            if len(datestr) == 8:
                return datestr[:4] + '-' + datestr[4:6] + '-' + datestr[6:]
            return None
        elif d != ' ' and d != '-':
            return None
        datepos = datepos + 1

path = '.'
if len(sys.argv) == 2:
    path = sys.argv[1]

mainpage = open(path + '/daca2.html', 'wt')
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
mainpage.write('<p>For performance reasons the analysis is limited. Files larger than 100kb are skipped. If analysis of a file takes more than 10 minutes it may be stopped.</p>\n')
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

daca2 = os.path.expanduser('~/daca2/')
for lib in range(2):
    for a in "0123456789abcdefghijklmnopqrstuvwxyz":
        if lib == 1:
            a = "lib" + a
        if os.path.isfile(daca2 + a + '/results.txt'):
            f = open(daca2 + a + '/results.txt', 'rt')
            data = f.read()
            f.close()

            datestr = readdate(data)

            if os.path.isfile(daca2 + 'results-' + a + '.txt'):
                f2 = open(daca2 + 'results-' + a + '.txt')
                data2 = f2.read()
                f2.close()

                datestr2 = readdate(data2)
                if not datestr or datestr < datestr2:
                    data = data2
                    datestr = datestr2

            if datestr:
                if not lastupdate or datestr > lastupdate:
                    lastupdate = datestr
                    recent = []
                if datestr == lastupdate:
                    recent.append(a)
            else:
                datestr = ''

            mainpage.write(
                '<tr>' +
                '<td><a href="daca2-' + a + '.html">' + a + '</a></td>' +
                '<td>' + datestr + '</td>' +
                '<td>' + str(data.count('(error)')) + '</td>' +
                '<td>' + str(data.count('(warning)')) + '</td>' +
                '<td>' + str(data.count('(performance)')) + '</td>' +
                '<td>' + str(data.count('(portability)')) + '</td>' +
                '<td>' + str(data.count('(style)')) + '</td>' +
                '<td>' + str(data.count('Crash?')) + '</td>' +
                '<td>' + str(data.count('with varid 0.')) + '</td>' +
                '</tr>\n')

            data = data.replace('&', '&amp;')
            data = data.replace('<', '&lt;')
            data = data.replace('>', '&gt;')
            data = data.replace('\n', '\n')

            f = open(path + '/daca2-' + a + '.html', 'wt')
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

if lastupdate:
    mainpage.write('<p>Last update: ' + lastupdate + '</p>')
    allrecent = ''
    for r in recent:
        allrecent = allrecent + ' <a href="daca2-' + r + '.html">' + r + '</a>'
    mainpage.write('<p>Most recently updated:' + allrecent + '</p>')

mainpage.write('</body>\n')
mainpage.write('</html>\n')
