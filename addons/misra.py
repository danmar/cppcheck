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

def reportError(token, num1, num2):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] misra ' + str(num1) + '.' + str(num2) + ' violation\n')

# Platform
# TODO get this from dump
CHAR_BITS = 8
SHORT_BITS = 16
INT_BITS = 32

def getEssentialType(expr):
    if not expr:
        return None
    if expr.variable:
        typeToken = expr.variable.typeStartToken
        while typeToken and typeToken.isName:
            if typeToken.str in ['char', 'short', 'int', 'long', 'float', 'double']:
                return typeToken.str
            typeToken = typeToken.next
    return None

def bitsOfEssentialType(expr):
    type = getEssentialType(expr)
    if type is None:
        return 0
    # TODO get --platform type sizes
    if type == 'char':
        return CHAR_BITS
    if type == 'short':
        return SHORT_BITS
    if type == 'int':
        return INT_BITS
    return 0

def isFunctionCall(expr):
    if not expr:
        return False
    if expr.str != '(' or not expr.astOperand1:
        return False
    if expr.astOperand1 != expr.previous:
        return False
    if expr.astOperand1.str in ['sizeof', 'if', 'switch', 'while']:
        return False
    return True

def countSideEffects(expr):
    if not expr or expr.str in [',', ';']:
        return 0
    ret = 0
    if expr.str in ['++', '--', '=']:
        ret = 1
    elif isFunctionCall(expr):
        ret = 1
    return ret + countSideEffects(expr.astOperand1) + countSideEffects(expr.astOperand2)

def getForLoopExpressions(forToken):
    if not forToken or forToken.str != 'for':
        return None
    lpar = forToken.next
    if not lpar or lpar.str != '(':
        return None
    if not lpar.astOperand2 or lpar.astOperand2.str != ';':
        return None
    if not lpar.astOperand2.astOperand2 or lpar.astOperand2.astOperand2.str != ';':
        return None
    return [lpar.astOperand2.astOperand1, lpar.astOperand2.astOperand2.astOperand1, lpar.astOperand2.astOperand2.astOperand2]


def hasFloatComparison(expr):
    if not expr:
        return False
    if expr.isLogicalOp:
        return hasFloatComparison(expr.astOperand1) or hasFloatComparison(expr.astOperand2)
    if expr.isComparisonOp:
        # TODO: Use ValueType
        return cppcheckdata.astIsFloat(expr.astOperand1) or cppcheckdata.astIsFloat(expr.astOperand2)
    return False

def hasSideEffectsRecursive(expr):
    if not expr:
        return False
    if expr.str in ['++', '--', '=']:
        return True
    # Todo: Check function calls
    return hasSideEffectsRecursive(expr.astOperand1) or hasSideEffectsRecursive(expr.astOperand2)

def isBoolExpression(expr):
    return expr and expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||']

def isConstantExpression(expr):
    if expr.isNumber:
        return True
    if expr.isName:
        return False
    if expr.astOperand1 and not isConstantExpression(expr.astOperand1):
        return False
    if expr.astOperand2 and not isConstantExpression(expr.astOperand2):
        return False
    return True

def isUnsignedInt(expr):
    # TODO this function is very incomplete. use ValueType?
    if not expr:
        return False
    if expr.isNumber:
        return expr.str.find('u')>0 or expr.str.find('U')>0
    if expr.str in ['+','-','*','/','%']:
        return isUnsignedInt(expr.astOperand1) or isUnsignedInt(expr.astOperand2)
    return False

def getPrecedence(expr):
    if not expr:
        return 16
    if not expr.astOperand1 or not expr.astOperand2:
        return 16
    if expr.str in ['*', '/', '%']:
        return 12
    if expr.str in ['+', '-']:
        return 11
    if expr.str in ['<<', '>>']:
        return 10
    if expr.str in ['<', '>', '<=', '>=']:
        return 9
    if expr.str in ['==', '!=']:
        return 8
    if expr.str == '&':
        return 7
    if expr.str == '^':
        return 6
    if expr.str == '|':
        return 5
    if expr.str == '&&':
        return 4
    if expr.str == '||':
        return 3
    if expr.str in ['?',':']:
        return 2
    if expr.isAssignmentOp:
        return 1
    if expr.str == ',':
        return 0
    return -1

def noParentheses(tok1, tok2):
    while tok1 and tok1 != tok2:
        if tok1.str == '(' or tok1.str == ')':
            return False
        tok1 = tok1.next
    return tok1 == tok2

def misra_5_1(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 5, 1)

def misra_7_1(rawTokens):
    for tok in rawTokens:
        if re.match(r'^0[0-7]+$', tok.str):
            reportError(tok, 7, 1)

def misra_7_3(rawTokens):
    for tok in rawTokens:
        if re.match(r'^[0-9]+l', tok.str):
            reportError(tok, 7, 3)


def misra_12_1_sizeof(rawTokens):
   state = 0
   for tok in rawTokens:
       if tok.str.startswith('//') or tok.str.startswith('/*'):
           continue
       if tok.str == 'sizeof':
           state = 1
       elif state == 1:
           if re.match(r'^[a-zA-Z_]',tok.str):
               state = 2
           else:
               state = 0
       elif state == 2:
           if tok.str in ['+','-','*','/','%']:
               reportError(tok, 12, 1)
           else:
               state = 0

def misra_12_1(data):
    for token in data.tokenlist:
        p = getPrecedence(token)
        if p < 2 or p > 12:
            continue
        p1 = getPrecedence(token.astOperand1)
        if p1 <= 12 and p1 > p and noParentheses(token.astOperand1,token):
            reportError(token, 12, 1)
            continue
        p2 = getPrecedence(token.astOperand2)
        if p2 <= 12 and p2 > p and noParentheses(token, token.astOperand2):
            reportError(token, 12, 1)
            continue

def misra_12_2(data):
   for token in data.tokenlist:
       if not (token.str in ['<<','>>']):
           continue
       if (not token.astOperand2) or (not token.astOperand2.values):
           continue
       maxval = 0
       for val in token.astOperand2.values:
           if val.intvalue > maxval:
               maxval = val.intvalue
       if maxval == 0:
           continue
       sz = bitsOfEssentialType(token.astOperand1)
       if sz <= 0:
           continue
       if maxval >= sz:
           reportError(token, 12, 2)

def misra_12_3(data):
   for token in data.tokenlist:
       if token.str != ',':
           continue
       if token.astParent and (token.astParent.str in ['(', ',', '{']):
          continue
       reportError(token, 12, 3)

def misra_12_4(data):
    max_uint = 0
    if INT_BITS == 16:
        max_uint = 0xffff
    elif INT_BITS == 32:
        max_uint = 0xffffffff
    else:
        return

    for token in data.tokenlist:
        if (not isConstantExpression(token)) or (not isUnsignedInt(token)):
            continue
        if not token.values:
            continue
        for value in token.values:
            if value.intvalue < 0 or value.intvalue > max_uint:
                reportError(token, 12, 4)
                break

def misra_13_1(data):
    for token in data.tokenlist:
        if token.str != '=':
            continue
        init = token.next
        if init and init.str == '{' and hasSideEffectsRecursive(init):
            reportError(init,13,1)

def misra_13_3(data):
    for token in data.tokenlist:
         if not token.astOperand1:
             continue
         if token.astParent and token.astParent.str != ',':
             continue
         if token.str == ',':
             continue
         if countSideEffects(token) >= 2:
             reportError(token, 13, 3)

def misra_13_4(data):
    for token in data.tokenlist:
        if token.str != '=':
            continue
        if not token.astParent:
            continue
        if not (token.astParent.str in [',', ';']):
             reportError(token, 13, 4)

def misra_13_5(data):
    for token in data.tokenlist:
        if token.isLogicalOp and hasSideEffectsRecursive(token.astOperand2):
            reportError(token, 13, 5)

def misra_13_6(data):
    for token in data.tokenlist:
        if token.str == 'sizeof' and hasSideEffectsRecursive(token.next):
            reportError(token, 13, 6)

def misra_14_1(data):
    for token in data.tokenlist:
        if token.str != 'for':
            continue
        exprs = getForLoopExpressions(token)
        if exprs and hasFloatComparison(exprs[1]):
            reportError(token, 14, 1)

def misra_14_2(data):
    for token in data.tokenlist:
        expressions = getForLoopExpressions(token)
        if not expressions:
            continue
        if expressions[0] and not expressions[0].isAssignmentOp:
            reportError(token, 14, 2)
        elif hasSideEffectsRecursive(expressions[1]):
            reportError(token, 14, 2)


def misra_14_4(data):
    for token in data.tokenlist:
        if token.str != '(':
            continue
        if not token.astOperand1 or not (token.astOperand1.str in ['if', 'while']):
            continue
        if not isBoolExpression(token.astOperand2):
            reportError(token, 14, 4)

def misra_15_1(data):
    for token in data.tokenlist:
        if token.str == "goto":
            reportError(token, 15, 1)

def misra_15_2(data):
    for token in data.tokenlist:
        if token.str != 'goto':
            continue
        if (not token.next) or (not token.next.isName):
            continue
        label = token.next.str
        tok = token.next.next
        while tok:
            if tok.str == '}' and tok.scope.type == 'Function':
                reportError(token, 15, 2)
                break
            if tok.str == label:
                break
            tok = tok.next

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
            misra_12_1_sizeof(data.rawTokens)
        misra_12_1(cfg)
        misra_12_2(cfg)
        misra_12_3(cfg)
        misra_12_4(cfg)
        misra_13_1(cfg)
        misra_13_3(cfg)
        misra_13_4(cfg)
        misra_13_5(cfg)
        misra_13_6(cfg)
        misra_14_1(cfg)
        misra_14_2(cfg)
        misra_14_4(cfg)
        misra_15_1(cfg)
        misra_15_2(cfg)


