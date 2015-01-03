#!/usr/bin/python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2015 Daniel Marjamaeki and Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Extract test cases information from Cppcheck test
file
"""

import os
import sys
import re


class Extract:

    """
    Read Cppcheck test file and create data
    representation
    """

    # array that stores all the test cases
    nodes = []

    def parseFile(self, filename):
        """
        parse test file and add info to the nodes
        variable
        """

        name = '[0-9a-zA-Z_]+'
        string = '\\"(.+)\\"'

        testclass = None
        functionName = None

        fin = open(filename, 'r')
        for line in fin:
            # testclass starts
            res = re.match('class (' + name + ')', line)
            if res is not None:
                testclass = res.group(1)

            # end of testclass
            if re.match('};', line) is not None:
                testclass = None

            # function start
            res = re.match('\\s+void (' + name + ')\\(\\)', line)
            if res is not None:
                functionName = res.group(1)

            elif re.match('\\s+}', line) is not None:
                functionName = None

            if functionName is None:
                continue

            # check
            res = re.match('\s+check.*\(' + string, line)
            if res is not None:
                code = res.group(1)

            # code..
            res = re.match('\\s+' + string, line)
            if res is not None:
                code = code + res.group(1)

            # assert
            res = re.match('\\s+ASSERT_EQUALS\\(\\"([^"]*)\\",', line)
            if res is not None and len(code) > 10:
                node = {'testclass': testclass,
                        'functionName': functionName,
                        'code': code,
                        'expected': res.group(1)}
                self.nodes.append(node)
                code = ''

        # close test file
        fin.close()


def strtoxml(s):
    """Convert string to xml/html format"""
    return s.replace('&', '&amp;').replace('"', '&quot;').replace('<', '&lt;').replace('>', '&gt;')


def trimname(name):
    """Trim test name. Trailing underscore and digits are removed"""
    while name[-1].isdigit():
        name = name[:-1]
    if name[-1] == '_':
        name = name[:-1]
    return name


def writeHtmlFile(nodes, functionName, filename, errorsOnly):
    """Write html file for a function name"""
    fout = open(filename, 'w')
    fout.write('<html>\n')
    fout.write('<head>\n')
    fout.write('  <style type="text/css">\n')
    fout.write('  body { font-size: 0.8em }\n')
    fout.write(
        '  th { background-color: #A3C159; text-transform: uppercase }\n')
    fout.write('  td { background-color: white; vertical-align: text-top }\n')
    fout.write('  pre { background-color: #EEEEEE }\n')
    fout.write('  </style>\n')
    fout.write('</head>\n')
    fout.write('<body>\n')

    fout.write('<a href="index.htm">Home</a> -- ')
    if errorsOnly:
        fout.write('<a href="all-' + functionName + '.htm">All test cases</a>')
    else:
        fout.write(
            '<a href="errors-' + functionName + '.htm">Error test cases</a>')
    fout.write('<br><br>')

    testclass = None
    num = 0
    for node in nodes:
        if errorsOnly and node['expected'] == '':
            continue
        if trimname(node['functionName']) == functionName:
            num = num + 1

            if not testclass:
                testclass = node['testclass']
                fout.write(
                    '<h1>' + node['testclass'] + '::' + functionName + '</h1>')
                fout.write('<table border="0" cellspacing="0">\n')
                fout.write(
                    '  <tr><th>Nr</th><th>Code</th><th>Expected</th></tr>\n')

            fout.write('  <tr><td>' + str(num) + '</td>')
            fout.write('<td><pre>' + strtoxml(
                node['code']).replace('\\n', '\n') + '</pre></td>')
            fout.write(
                '<td>' + strtoxml(node['expected']).replace('\\n', '<br>') + '</td>')
            fout.write('</tr>\n')

    if testclass is not None:
        fout.write('</table>\n')
    fout.write('</body></html>\n')
    fout.close()


if len(sys.argv) <= 1 or '--help' in sys.argv:
    print ('Extract test cases from test file')
    print (
        'Syntax: extracttests.py [--html=folder] [--xml] [--code=folder] path/testfile.cpp')
    sys.exit(0)

# parse command line
xml = False
filename = None
htmldir = None
codedir = None
for arg in sys.argv[1:]:
    if arg == '--xml':
        xml = True
    elif arg.startswith('--html='):
        htmldir = arg[7:]
    elif arg.startswith('--code='):
        codedir = arg[7:]
    elif arg.endswith('.cpp'):
        filename = arg
    else:
        print ('Invalid option: ' + arg)
        sys.exit(1)


# extract test cases
if filename is not None:
    # parse test file
    e = Extract()
    e.parseFile(filename)

    # generate output
    if xml:
        print ('<?xml version="1.0"?>')
        print ('<tree>')
        count = 0
        for node in e.nodes:
            s = '  <node'
            s += ' function="' + node['functionName'] + '"'
            s += ' code="' + strtoxml(node['code']) + '"'
            s += ' expected="' + strtoxml(node['expected']) + '"'
            s += '/>'
            print (s)
        print ('</tree>')
    elif htmldir is not None:
        if not htmldir.endswith('/'):
            htmldir += '/'
        if not os.path.exists(htmldir):
            os.mkdir(htmldir)
        findex = open(htmldir + 'index.htm', 'w')
        findex.write('<html>\n')
        findex.write('<head>\n')
        findex.write('  <style type="text/css">\n')
        findex.write('  table { font-size: 0.8em }\n')
        findex.write(
            '  th { background-color: #A3C159; text-transform: uppercase }\n')
        findex.write(
            '  td { background-color: #F0FFE0; vertical-align: text-top }\n')
        findex.write('  A:link { text-decoration: none }\n')
        findex.write('  A:visited { text-decoration: none }\n')
        findex.write('  A:active { text-decoration: none }\n')
        findex.write('  A:hover { text-decoration: underline; color: blue }\n')
        findex.write('  </style>\n')
        findex.write('</head>\n')
        findex.write('<body>\n')
        findex.write('<h1>' + filename + '</h1>\n')

        functionNames = []
        for node in e.nodes:
            functionname = trimname(node['functionName'])
            if functionname not in functionNames:
                functionNames.append(functionname)
        functionNames.sort()

        findex.write('<table border="0" cellspacing="0">\n')
        findex.write('  <tr><th>Name</th><th>Errors</th><th>All</th></tr>\n')
        for functionname in functionNames:
            findex.write('  <tr><td>' + functionname + '</td>')
            numall = 0
            numerr = 0
            for node in e.nodes:
                if trimname(node['functionName']) == functionname:
                    numall = numall + 1
                    if node['expected'] != '':
                        numerr = numerr + 1
            if numerr == 0:
                findex.write('<td><div align="right">0</div></td>')
            else:
                findex.write('<td><a href="errors-' + functionname +
                             '.htm"><div align="right">' + str(numerr) + '</div></a></td>')
            findex.write('<td><a href="all-' + functionname +
                         '.htm"><div align="right">' + str(numall) + '</div></a></td>')
            findex.write('</tr>\n')

        findex.write('</table>\n')

        findex.write('</body></html>')
        findex.close()

        # create files for each functionName
        for functionName in functionNames:
            writeHtmlFile(e.nodes,
                          functionName,
                          htmldir + 'errors-' + functionName + '.htm',
                          True)
            writeHtmlFile(e.nodes,
                          functionName,
                          htmldir + 'all-' + functionName + '.htm',
                          False)

    elif codedir:
        testnum = 0

        if not codedir.endswith('/'):
            codedir = codedir + '/'

        if not os.path.exists(codedir):
            os.mkdir(codedir)

        errors = open(codedir + 'errors.txt', 'w')

        for node in e.nodes:
            testnum = testnum + 1

            functionName = node['functionName']
            code = node['code']
            code = code.replace('\\n', '\n')
            code = code.replace('\\"', '"')
            expected = node['expected']

            filename = '0000' + str(testnum) + '-'
            filename = filename[-4:]
            filename += functionName + '.cpp'

            # source code
            fout = open(codedir + filename, 'w')
            fout.write(code)
            fout.close()

            # write 'expected' to errors.txt
            if expected != '':
                expected = expected.replace('\\n', '\n')
                expected = expected.replace('\\"', '"')
                expected = re.sub(
                    '\\[test.cp?p?:', '[' + filename + ':', expected)
                errors.write(expected)
        errors.close()
    else:
        for node in e.nodes:
            print (node['functionName'])
