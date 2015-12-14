#/usr/bin/python
#
# cppcheck addon for naming conventions
#
# Example usage (variable name must start with lowercase, function name must start with uppercase):
# $ cppcheck --dump path-to-src/
# $ python addons/naming.py --var='[a-z].*' --function='[A-Z].*' path-to-src/*.dump
#

import cppcheckdata
import sys
import re

RE_VARNAME = None
RE_FUNCTIONNAME = None
for arg in sys.argv[1:]:
    if arg[:6] == '--var=':
        RE_VARNAME = arg[6:]
    elif arg[:11] == '--function=':
        RE_FUNCTIONNAME = arg[11:]


def reportError(token, severity, msg):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') naming.py: ' + msg + '\n')

for arg in sys.argv[1:]:
    if not arg[-5:] == '.dump':
        continue
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)
    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        if RE_VARNAME:
            for var in cfg.variables:
                res = re.match(RE_VARNAME, var.nameToken.str)
                if not res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention')
        if RE_FUNCTIONNAME:
            for scope in cfg.scopes:
                if scope.type == 'Function':
                    res = re.match(RE_FUNCTIONNAME, scope.className)
                    if not res:
                        reportError(
                            scope.classStart, 'style', 'Function ' + scope.className + ' violates naming convention')
