#!/usr/bin/env python
# -*- coding: utf-8 -*-

# The MIT License
#
# Copyright (c) 2011 Kimmo Varis <kimmov@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Project repository:
# https://bitbucket.org/kimmov/testrun

'''Runs all the GUI tests in subdirectories.'''

import os
import os.path
import subprocess
import sys

class Test:
    '''Test info, paths etc.'''

    def __init__(self):
        self.profile = ''
        self.binary = ''
        self.passed = 0
        self.failed = 0
        self.skipped = 0


class TestList:
    '''Finds all tests.'''

    def __init__(self):
        self._testlist = []
        self._filelist = []
        self._basedirectory = None

    def findtests(self, directory):
        '''Finds all tests from subdirectories of the given directory.'''
        
        if not os.path.exists(directory):
            print 'Directory does not exist!'
            return

        self._basedirectory = directory
        self._listprofiles(directory)
        self._readprojects()

    def testlist(self):
        return self._testlist

    def _listprofiles(self, directory):
        '''List all .pro files in given directory and subdirectories.

        The method is recursive calling itself for each subdir it finds. Found
        files are added to _filelist list.
        '''

        for root, dirnames, filenames in os.walk(directory):
            self._walkfiles(root, filenames)

    def _walkfiles(self, dirname, filenames):
        '''Find .pro files from list of given filenames.

        Find all .pro files from the given list. Make filenames full paths by
        joining them with directory path.
        '''

        for filename in filenames:
            root, ext = os.path.splitext(filename)
            if ext == '.pro':
                fullpath = os.path.join(dirname, filename)
                relpath = fullpath[len(self._basedirectory) + 1:]
                if relpath.startswith('/'):
                    relpath = relpath[1:]
                #print 'Found project %s (%s)' % (relpath, fullpath)
                testpaths = (fullpath, relpath)
                self._filelist.append(testpaths)

    def _readprojects(self):
        '''Read project files and find the executable names.'''

        for fullpath, relpath in self._filelist:
            #print 'Reading file: %s' % relpath
            f = open(fullpath, 'r')
            target = ''
            destdir = ''
            allread = False
            while not allread:
                line = f.readline()
                if line == '':
                    allread = True
                    break

                line = line.strip()
                if line.startswith('TARGET'):
                    target = line[line.find('=') + 1:]
                    target = target.strip()
                    #print 'File: %s Target: %s' % (relpath, target)

                if line.startswith('DESTDIR'):
                    destdir = line[line.find('=') + 1:]
                    destdir = destdir.strip()
                    #print 'File: %s Dest: %s' % (relpath, dest)
            f.close()

            if target != '':
                path = os.path.dirname(fullpath)
                if destdir != '':
                    path = os.path.join(path, destdir)
                path = os.path.join(path, target)
                path = os.path.normpath(path)
                #print 'Found test: %s' % path

                if os.path.exists(path):
                    testinfo = Test()
                    testinfo.profile = relpath
                    testinfo.binary = path
                    self._testlist.append(testinfo)


class TestRunner:
    def __init__(self, testlist):
        self._testlist = testlist

    def runtests(self):
        for test in self._testlist:
            self._runtest(test)
        self._printsummary()

    def _runtest(self, test):
        cmd = test.binary
        #print 'Running: %s' % cmd
        proc = subprocess.Popen(cmd,
                                shell = False,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.STDOUT)
        stdout_value, stderr_value = proc.communicate()
        print stdout_value
        self._parseoutput(test, stdout_value)

    def _parseoutput(self, test, output):
        '''Parse test counts (passed, failed, skipped) from the output.'''

        lines = output.splitlines(True)
        for line in lines:
            # Lines are like: Totals: 6 passed, 0 failed, 0 skipped
            if line.startswith('Totals: '):
                parts = line.split(' ')
                test.passed = int(parts[1])
                test.failed = int(parts[3])
                test.skipped = int(parts[5])

    def _printsummary(self):
        total = 0
        passed = 0
        failed = 0
        skipped = 0
        for test in self._testlist:
            total += test.passed + test.failed + test.skipped
            passed += test.passed
            failed += test.failed
            skipped += test.skipped

        print '\nTEST SUMMARY:'
        print '  Total tests:   %i' % total
        print '  Passed tests:  %i' % passed
        print '  Failed tests:  %i' % failed
        print '  Skipped tests: %i' % skipped


def main():
    lister = TestList()
    directory = os.getcwd()
    lister.findtests(directory)
    runner = TestRunner(lister.testlist())
    runner.runtests()
    return 0

if __name__ == '__main__':
    sys.exit(main())
