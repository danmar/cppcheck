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

def isBitwiseOp(token):
    return token and (token.str in ['&', '|', '^'])

def isComparisonOp(token):
    return token and (token.str in ['==', '!=', '>', '>=', '<', '<='])

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
  exp46(data)
