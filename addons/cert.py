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

def reportError(token, severity, msg):
  sys.stderr.write('[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') cert.py: ' + msg + '\n')

def isLocalStruct(arg):
    if arg and arg.str == '&' and not arg.astOperand2:
        arg = arg.astOperand1
    return arg and arg.variable and arg.variable.isClass and (arg.variable.isLocal or arg.variable.isArgument)

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

        if token.astOperand1.str == 'memcmp' and (isLocalStruct(arg1) or isLocalStruct(arg2)):
            reportError(token, 'style', 'EXP42-C Comparison of struct padding data')
        if (token.astOperand1.str in ['memcpy','memmove']) and isLocalStruct(arg2):
            reportError(token, 'style', 'EXP42-C Reading struct padding data')

# EXP46-C
# Do not use a bitwise operator with a Boolean-like operand
#   int x = (a == b) & c;
def exp46(data):
    for token in data.tokenlist:
        if isBitwiseOp(token) and (isComparisonOp(token.astOperand1) or isComparisonOp(token.astOperand2)):
            reportError(token, 'style', 'EXP46-C Bitwise operator is used with a Boolean-like operand')

for arg in sys.argv[1:]:
  print('Checking ' + arg + '...')
  data = cppcheckdata.parsedump(arg)
  exp42(data)
  exp46(data)
