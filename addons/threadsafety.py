#!/usr/bin/env python3
#
# This script analyses Cppcheck dump files to locate threadsafety issues
# - warn about static local objects
#

import cppcheckdata
import sys

def reportError(token, severity, msg, id):
    cppcheckdata.reportError(token, severity, msg, 'threadsafety', id)


def checkstatic(data):
    for var in data.variables:
        if var.isStatic and var.isLocal:
            type = None
            if var.isClass:
                type = 'object'
            else:
                type = 'variable'
            if var.isConst:
                if data.standards.cpp == 'c++03':
                    reportError(var.typeStartToken, 'warning', 'Local constant static ' + type + ' \'' + var.nameToken.str + '\', dangerous if it is initialized in parallel threads', 'threadsafety-const')
            else:
                reportError(var.typeStartToken, 'warning', 'Local static ' + type + ': ' + var.nameToken.str, 'threadsafety')

for arg in sys.argv[1:]:
    if arg.startswith('-'):
        continue

    print('Checking %s...' % arg)
    data = cppcheckdata.CppcheckData(arg)

    for cfg in data.iterconfigurations():
        print('Checking %s, config %s...' % (arg, cfg.name))
        checkstatic(cfg)

    sys.exit(cppcheckdata.EXIT_CODE)
