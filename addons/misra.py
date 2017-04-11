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



# 1.1     Done. Use compiler.
# 1.2     Done. gcc
# 1.3     Done. Use Cppcheck, compiler, ..
# 2.1     Done. Use Cppcheck, clang, etc.
# 2.2     Done. Use Cppcheck
# 2.3     TODO
# 2.4     TODO
# 2.5     Done. clang
# 2.6     Done. Cppcheck
# 2.7     Done. clang, gcc
# 3.1     TODO
# 3.2     TODO
# 4.1     TODO
# 4.2     Done. clang, gcc
# 5.1     Done.
# 5.2     Done.
# 5.3     Done. -Wshadow
# 5.4     Done. clang -Weverything
# 5.5     TODO
# 5.6     TODO
# 5.7     TODO
# 5.8     Done.
# 5.9     Done. -Wshadow
# 6.1     TODO
# 6.2     TODO
# 7.1     Done.
# 7.2     TODO
# 7.3     TODO
# 7.4     Done. Cppcheck, gcc, clang
# 8.1     Done. gcc, clang
# 8.2     TODO
# 8.3     Done. Cppcheck
# 8.4     Done. Clang
# 8.5     TODO
# 8.6     TODO
# 8.7     TODO
# 8.8     TODO
# 8.9     TODO
# 8.10    TODO
# 8.11    TODO
# 8.12    TODO
# 8.13    TODO
# 8.14    TODO
# 9.1     Done. Cppcheck.
# 9.2     TODO
# 9.3     TODO
# 9.4     TODO
# 9.5     TODO
# 10.1    TODO
# 10.2    TODO
# 10.3    Done. Cppcheck.
# 10.4    TODO
# 10.5    TODO
# 10.6    TODO
# 10.7    TODO
# 10.8    TODO
# 11.1    TODO
# 11.2    TODO
# 11.3    TODO
# 11.4    TODO
# 11.5    TODO
# 11.6    TODO
# 11.7    TODO
# 11.8    TODO
# 11.9    TODO
# 12.1    TODO
# 12.2    Done. Cppcheck.
# 12.3    TODO
# 12.4    TODO
# 13.1    TODO
# 13.2    TODO
# 13.3    TODO
# 13.4    TODO
# 13.5    Done
# 13.6    Done. Cppcheck.
# 14.1    TODO
# 14.2    TODO
# 14.3    TODO
# 14.4    Done.
# 15.1    Done.
# 15.2    TODO
# 15.3    TODO
# 15.4    TODO
# 15.5    TODO
# 15.6    TODO
# 15.7    TODO
# 16.1    TODO
# 16.2    TODO
# 16.3    TODO
# 16.4    TODO
# 16.5    TODO
# 16.6    TODO
# 16.7    TODO
# 17.1    TODO
# 17.2    TODO
# 17.3    Done. Compiler.
# 17.4    Done. Compiler.
# 17.5    Done. Cppcheck.
# 17.6    TODO
# 17.7    TODO
# 17.8    TODO
# 18.1    TODO
# 18.2    TODO. Cppcheck.
# 18.3    TODO. Cppcheck.
# 18.4    TODO
# 18.5    TODO
# 18.6    TODO
# 18.7    TODO
# 18.8    TODO
# 19.1    TODO
# 19.2    TODO
# 20.1    TODO
# 20.2    TODO
# 20.3    TODO
# 20.4    TODO
# 20.5    TODO
# 20.6    TODO
# 20.7    TODO
# 20.8    TODO
# 20.9    TODO
# 20.10   TODO
# 20.11   TODO
# 20.12   TODO
# 20.13   TODO
# 20.14   TODO
# 21.1    TODO
# 21.2    TODO
# 21.3    TODO
# 21.4    TODO
# 21.5    TODO
# 21.6    TODO
# 21.7    TODO
# 21.8    TODO
# 21.9    TODO
# 21.10   TODO
# 21.11   TODO
# 21.12   TODO
# 22.1    TODO
# 22.2    TODO
# 22.3    TODO
# 22.4    TODO
# 22.5    TODO
# 22.6    TODO



def misra_5_1(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 51)

def misra_7_1(rawTokens):
    for tok in rawTokens:
        if re.match(r'^0[0-7]+$', tok.str):
            reportError(tok, 71)

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
        misra_13_5(cfg)
        misra_14_4(cfg)
        misra_15_1(cfg)
