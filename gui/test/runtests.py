#!/usr/bin/env python
# -*- coding: utf-8 -*-

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


class TestList:
    '''Finds all tests.'''

    def __init__(self):
        self._testlist = []
        self._filelist = []
        self._basedirectory = None

    def findtests(self, directory = ''):
        '''Finds all tests from subdirectories of the given directory.'''
        
        if directory == None or directory == '':
            directory = os.getcwd()

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

            for curdir in dirnames:
                fullpath = os.path.join(root, curdir)
                self._listprofiles(fullpath)

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
            targetfound = False
            while not targetfound:
                line = f.readline()
                if line == '':
                    break

                line = line.strip()
                if line.startswith('TARGET'):
                    target = line[line.find('=') + 1:]
                    target = target.strip()
                    #print 'File: %s Target: %s' % (relpath, target)
                    path = os.path.dirname(fullpath)
                    target = os.path.join(path, target)
                    targetfound = True

                    testinfo = Test()
                    testinfo.profile = relpath
                    testinfo.binary = target
                    self._testlist.append(testinfo)

            f.close()


class TestRunner:
    def __init__(self, testlist):
        self._testlist = testlist

    def runtests(self):
        for test in self._testlist:
            self._runtest(test)

    def _runtest(self, test):
        cmd = test.binary
        #print 'Running: %s' % cmd
        proc = subprocess.Popen(cmd,
                                shell = False,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.STDOUT)
        stdout_value, stderr_value = proc.communicate()
        print stdout_value


def main():
    lister = TestList()
    lister.findtests()
    runner = TestRunner(lister.testlist())
    runner.runtests()
    return 0

if __name__ == '__main__':
    sys.exit(main())
