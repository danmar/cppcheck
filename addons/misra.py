#/usr/bin/python
#
# MISRA C 2012 checkers
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misra.py main.cpp.dump
#
# Limitations: This addon is released as open source. Rule texts can't be freely
# distributed. https://www.misra.org.uk/forum/viewtopic.php?f=56&t=1189
#
#

import cppcheckdata
import sys
import re

def reportError(token, num):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] misra ' + str(num) + ' violation\n')

def hasSideEffects(expr):
    if not expr:
        return False
    if expr.str in ['++', '--', '=']:
        return True
    # Todo: Check function calls
    return hasSideEffects(expr.astOperand1) or hasSideEffects(expr.astOperand2)

def isBoolExpression(expr):
    return expr and expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||']


def misra_5_1(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 51)

def misra_7_1(rawTokens):
    for tok in rawTokens:
        if re.match(r'^0[0-7]+$', tok.str):
            reportError(tok, 71)

def misra_7_3(rawTokens):
    for tok in rawTokens:
        if re.match(r'^[0-9]+l+$', tok.str):
            reportError(tok, 73)

def misra_13_5(data):
    for token in data.tokenlist:
        if token.isLogicalOp and hasSideEffects(token.astOperand2):
            reportError(token, 135)

def misra_14_4(data):
    for token in data.tokenlist:
        if token.str != '(':
            continue
        if not token.astOperand1 or not (token.astOperand1.str in ['if', 'while']):
            continue
        if not isBoolExpression(token.astOperand2):
            reportError(token, 144)

def misra_15_1(data):
    for token in data.tokenlist:
        if token.str == "goto":
            reportError(token, 151)



for arg in sys.argv[1:]:
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)

    cfgNumber = 0

    for cfg in data.configurations:
        cfgNumber = cfgNumber + 1
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')

        misra_5_1(cfg)
        if cfgNumber == 1:
            misra_7_1(data.rawTokens)
            misra_7_3(data.rawTokens)
        misra_13_5(cfg)
        misra_14_4(cfg)
        misra_15_1(cfg)
