#!/usr/bin/env python
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


def validate_regex(expr):
    try:
        re.compile(expr)
    except re.error:
        print('Error: "{}" is not a valid regular expression.'.format(expr))
        exit(1)


RE_VARNAME = None
RE_PRIVATE_MEMBER_VARIABLE = None
RE_FUNCTIONNAME = None
for arg in sys.argv[1:]:
    if arg[:6] == '--var=':
        RE_VARNAME = arg[6:]
        validate_regex(RE_VARNAME)
    elif arg.startswith('--private-member-variable='):
        RE_PRIVATE_MEMBER_VARIABLE = arg[arg.find('=')+1:]
        validate_regex(RE_PRIVATE_MEMBER_VARIABLE)
    elif arg[:11] == '--function=':
        RE_FUNCTIONNAME = arg[11:]
        validate_regex(RE_FUNCTIONNAME)

def reportError(token, severity, msg, errorId):
    cppcheckdata.reportErrorCli(token, severity, msg, 'naming', errorId)

for arg in sys.argv[1:]:
    if not arg.endswith('.dump'):
        continue
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)
    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        if RE_VARNAME:
            for var in cfg.variables:
                if var.nameToken:
                    res = re.match(RE_VARNAME, var.nameToken.str)
                    if not res:
                        reportError(var.typeStartToken, 'style', 'Variable ' +
                                    var.nameToken.str + ' violates naming convention', 'varname')
        if RE_PRIVATE_MEMBER_VARIABLE:
            for var in cfg.variables:
                if (var.access is None) or var.access != 'Private':
                    continue
                res = re.match(RE_PRIVATE_MEMBER_VARIABLE, var.nameToken.str)
                if not res:
                    reportError(var.typeStartToken, 'style', 'Private member variable ' +
                                var.nameToken.str + ' violates naming convention', 'privateMemberVariable')
        if RE_FUNCTIONNAME:
            for scope in cfg.scopes:
                if scope.type == 'Function':
                    res = re.match(RE_FUNCTIONNAME, scope.className)
                    if not res:
                        reportError(
                            scope.bodyStart, 'style', 'Function ' + scope.className + ' violates naming convention', 'functionName')

