#!/usr/bin/env python
#
# This script analyses Cppcheck dump files to locate threadsafety issues
# - warn about static local objects
#

import cppcheckdata
import sys


def reportError(token, severity, msg):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') threadsafety.py: ' + msg + '\n')


def checkstatic(data):
    for var in data.variables:
        if var.isStatic and var.isLocal and var.isClass:
            reportError(var.typeStartToken, 'warning', ('Local static object: ' + var.nameToken.str))

for arg in sys.argv[1:]:
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)
    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        checkstatic(cfg)
