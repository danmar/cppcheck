#/usr/bin/python
#
# Cert: Some extra CERT checkers
#
# Cppcheck itself handles many CERT rules. Cppcheck warns when there is undefined behaviour.
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python cert.py main.cpp.dump

import cppcheckdata
import sys
import re


def reportError(token, severity, msg):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') cert.py: ' + msg + '\n')


def isUnpackedStruct(var):
    decl = var.typeStartToken
    while decl and decl.isName:
        if decl.str == 'struct':
            structScope = decl.next.typeScope
            if structScope:
                linenr = int(structScope.classStart.linenr)
                for line in open(structScope.classStart.file):
                    linenr = linenr - 1
                    if linenr == 0:
                        return True
                    if re.match(r'#pragma\s+pack\s*\(', line):
                        return False
            break
        decl = decl.next
    return False


def isLocalUnpackedStruct(arg):
    if arg and arg.str == '&' and not arg.astOperand2:
        arg = arg.astOperand1
    return arg and arg.variable and (arg.variable.isLocal or arg.variable.isArgument) and isUnpackedStruct(arg.variable)


def isBitwiseOp(token):
    return token and (token.str in ['&', '|', '^'])


def isComparisonOp(token):
    return token and (token.str in ['==', '!=', '>', '>=', '<', '<='])


# EXP42-C
# do not compare padding data
def exp42(data):
    for token in data.tokenlist:
        if token.str != '(' or not token.astOperand1:
            continue

        arg1 = None
        arg2 = None
        if token.astOperand2 and token.astOperand2.str == ',':
            if token.astOperand2.astOperand1 and token.astOperand2.astOperand1.str == ',':
                arg1 = token.astOperand2.astOperand1.astOperand1
                arg2 = token.astOperand2.astOperand1.astOperand2

        if token.astOperand1.str == 'memcmp' and (isLocalUnpackedStruct(arg1) or isLocalUnpackedStruct(arg2)):
            reportError(
                token, 'style', 'EXP42-C Comparison of struct padding data (fix either by packing the struct using \'#pragma pack\' or by rewriting the comparison)')

# EXP46-C
# Do not use a bitwise operator with a Boolean-like operand
#   int x = (a == b) & c;


def exp46(data):
    for token in data.tokenlist:
        if isBitwiseOp(token) and (isComparisonOp(token.astOperand1) or isComparisonOp(token.astOperand2)):
            reportError(
                token, 'style', 'EXP46-C Bitwise operator is used with a Boolean-like operand')

for arg in sys.argv[1:]:
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)
    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        exp42(cfg)
        exp46(cfg)
