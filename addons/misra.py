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

def hasSideEffects(expr):
    if not expr:
        return False
    if expr.str in ['++', '--', '=']:
        return True
    # Todo: Check function calls
    return hasSideEffects(expr.astOperand1) or hasSideEffects(expr.astOperand2)

def isBoolExpression(expr):
    return expr and expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||']

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

def misra_12_3(data):
   for token in data.tokenlist:
       if token.str != ',':
           continue
       if token.astParent and (token.astParent.str in ['(', ',', '{']):
           if token.astParent.str == ',':
               if token == token.astParent.astOperand1:
                   if noParentheses(token, token.astParent):
                       continue
               elif token == token.astParent.astOperand2:
                   if noParentheses(token.astParent, token):
                       continue
               else:
                   continue
           else:
               continue
       reportError(token, 12, 3)

def misra_13_5(data):
    for token in data.tokenlist:
        if token.isLogicalOp and hasSideEffects(token.astOperand2):
            reportError(token, 13, 5)

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
        misra_12_3(cfg)
        misra_13_5(cfg)
        misra_14_4(cfg)
        misra_15_1(cfg)
