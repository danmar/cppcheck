#!/usr/bin/env python
#
# MISRA C 2012 checkers
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misra.py --rule-texts=<path-to-rule-texts> main.cpp.dump
# python misra.py --misra-pdf=<path-to-misra.pdf> main.cpp.dump
#
# Limitations: This addon is released as open source. Rule texts can't be freely
# distributed. https://www.misra.org.uk/forum/viewtopic.php?f=56&t=1189
#
# Total number of rules: 153

import cppcheckdata
import sys
import re
import os
import tempfile
import subprocess

ruleTexts = {}

VERIFY = False
VERIFY_EXPECTED = []
VERIFY_ACTUAL = []


def reportError(location, num1, num2):
    if VERIFY:
        VERIFY_ACTUAL.append(str(location.linenr) + ':' + str(num1) + '.' + str(num2))
    else:
        num = num1 * 100 + num2
        id = 'misra-c2012-' + str(num1) + '.' + str(num2)
        if num in ruleTexts:
            errmsg = ruleTexts[num] + ' [' + id + ']'
        else:
            errmsg = 'misra violation (use --misra-pdf=<file> or --rule-texts=<file> to get proper output) [' + id + ']'
        sys.stderr.write('[' + location.file + ':' + str(location.linenr) + '] (style): ' + errmsg + '\n')


def simpleMatch(token, pattern):
    for p in pattern.split(' '):
        if not token or token.str != p:
            return False
        token = token.next
    return True

KEYWORDS = {
    'auto',
    'break',
    'case',
    'char',
    'const',
    'continue',
    'default',
    'do',
    'double',
    'else',
    'enum',
    'extern',
    'float',
    'for',
    'goto',
    'if',
    'int',
    'long',
    'register',
    'return',
    'short',
    'signed',
    'sizeof',
    'static',
    'struct',
    'switch',
    'typedef',
    'union',
    'unsigned',
    'void',
    'volatile',
    'while'
}


def getEssentialType(expr):
    if not expr:
        return None
    if expr.variable:
        typeToken = expr.variable.typeStartToken
        while typeToken and typeToken.isName:
            if typeToken.str in {'char', 'short', 'int', 'long', 'float', 'double'}:
                return typeToken.str
            typeToken = typeToken.next

    elif expr.astOperand1 and expr.astOperand2 and expr.str in {'+', '-', '*', '/', '%', '&', '|', '^'}:
        e1 = getEssentialType(expr.astOperand1)
        e2 = getEssentialType(expr.astOperand2)
        if not e1 or not e2:
            return None
        types = ['bool', 'char', 'short', 'int', 'long', 'long long']
        try:
            i1 = types.index(e1)
            i2 = types.index(e2)
            if i2 >= i1:
                return types[i2]
            return types[i1]
        except ValueError:
            return None

    return None


def bitsOfEssentialType(expr):
    type = getEssentialType(expr)
    if type is None:
        return 0
    if type == 'char':
        return CHAR_BIT
    if type == 'short':
        return SHORT_BIT
    if type == 'int':
        return INT_BIT
    if type == 'long':
        return LONG_BIT
    if type == 'long long':
        return LONG_LONG_BIT
    return 0


def isCast(expr):
    if not expr or expr.str != '(' or not expr.astOperand1 or expr.astOperand2:
        return False
    if simpleMatch(expr, '( )'):
        return False
    return True


def isFunctionCall(expr):
    if not expr:
        return False
    if expr.str != '(' or not expr.astOperand1:
        return False
    if expr.astOperand1 != expr.previous:
        return False
    if expr.astOperand1.str in KEYWORDS:
        return False
    return True


def countSideEffects(expr):
    if not expr or expr.str in {',', ';'}:
        return 0
    ret = 0
    if expr.str in {'++', '--', '='}:
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
    return [lpar.astOperand2.astOperand1,
            lpar.astOperand2.astOperand2.astOperand1,
            lpar.astOperand2.astOperand2.astOperand2]


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
    if expr.str == '=' and expr.astOperand1 and expr.astOperand1.str == '[':
        prev = expr.astOperand1.previous
        if prev and (prev.str == '{' or prev.str == '{'):
            return hasSideEffectsRecursive(expr.astOperand2)
    if expr.str in {'++', '--', '='}:
        return True
    # Todo: Check function calls
    return hasSideEffectsRecursive(expr.astOperand1) or hasSideEffectsRecursive(expr.astOperand2)


def isBoolExpression(expr):
    return expr and expr.str in {'!', '==', '!=', '<', '<=', '>', '>=', '&&', '||'}


def isConstantExpression(expr):
    if expr.isNumber:
        return True
    if expr.isName:
        return False
    if simpleMatch(expr.previous, 'sizeof ('):
        return True
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
        return 'u' in expr.str or 'U' in expr.str
    if expr.str in {'+', '-', '*', '/', '%'}:
        return isUnsignedInt(expr.astOperand1) or isUnsignedInt(expr.astOperand2)
    return False


def getPrecedence(expr):
    if not expr:
        return 16
    if not expr.astOperand1 or not expr.astOperand2:
        return 16
    if expr.str in {'*', '/', '%'}:
        return 12
    if expr.str in {'+', '-'}:
        return 11
    if expr.str in {'<<', '>>'}:
        return 10
    if expr.str in {'<', '>', '<=', '>='}:
        return 9
    if expr.str in {'==', '!='}:
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
    if expr.str in {'?', ':'}:
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


def findGotoLabel(gotoToken):
    label = gotoToken.next.str
    tok = gotoToken.next.next
    while tok:
        if tok.str == '}' and tok.scope.type == 'Function':
            break
        if tok.str == label and tok.next.str == ':':
            return tok
        tok = tok.next
    return None


def findInclude(directives, header):
    for directive in directives:
        if directive.str == '#include ' + header:
            return directive
    return None


def misra_3_1(rawTokens):
    for token in rawTokens:
        if token.str.startswith('/*') or token.str.startswith('//'):
            if '//' in token.str[2:] or '/*' in token.str[2:]:
                reportError(token, 3, 1)


def misra_5_1(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 5, 1)


def misra_5_3(data):
    scopeVars = {}
    for var in data.variables:
        if var.isArgument:
            # TODO
            continue
        if var.nameToken.scope not in scopeVars:
            scopeVars[var.nameToken.scope] = []
        scopeVars[var.nameToken.scope].append(var)

    for innerScope in data.scopes:
        if innerScope.type == 'Global':
            continue
        if innerScope not in scopeVars:
            continue
        for innerVar in scopeVars[innerScope]:
            outerScope = innerScope.nestedIn
            while outerScope:
                if outerScope not in scopeVars:
                    outerScope = outerScope.nestedIn
                    continue
                found = False
                for outerVar in scopeVars[outerScope]:
                    if innerVar.nameToken.str == outerVar.nameToken.str:
                        found = True
                        break
                if found:
                    reportError(innerVar.nameToken, 5, 3)
                    break
                outerScope = outerScope.nestedIn


def misra_5_4(data):
    for dir in data.directives:
        if re.match(r'#define [a-zA-Z0-9_]{64,}', dir.str):
            reportError(dir, 5, 4)


def misra_5_5(data):
    macroNames = []
    for dir in data.directives:
        res = re.match(r'#define ([A-Za-z0-9_]+)', dir.str)
        if res:
            macroNames.append(res.group(1))
    for var in data.variables:
        if var.nameToken.str in macroNames:
            reportError(var.nameToken, 5, 5)


def misra_7_1(rawTokens):
    for tok in rawTokens:
        if re.match(r'^0[0-7]+$', tok.str):
            reportError(tok, 7, 1)


def misra_7_3(rawTokens):
    for tok in rawTokens:
        if re.match(r'^[0-9]+l', tok.str):
            reportError(tok, 7, 3)


def misra_8_11(data):
    for var in data.variables:
        if var.isExtern and simpleMatch(var.nameToken.next, '[ ]') and var.nameToken.scope.type == 'Global':
            reportError(var.nameToken, 8, 11)


def misra_8_12(data):
    for token in data.tokenlist:
        if token.str != '{':
            continue
        if not token.scope or token.scope.type != 'Enum':
            continue
        etok = token
        values = []
        while etok:
            if etok.str == '}':
                break
            if etok.str == '=':
                rhsValues = etok.astOperand2.values
                if rhsValues and len(rhsValues) == 1:
                    if rhsValues[0].intvalue in values:
                        reportError(etok, 8, 12)
                        break
                    values.append(rhsValues[0].intvalue)
            etok = etok.next


def misra_8_14(rawTokens):
    for token in rawTokens:
        if token.str == 'restrict':
            reportError(token, 8, 14)


def misra_9_5(rawTokens):
    for token in rawTokens:
        if simpleMatch(token, '[ ] = { ['):
            reportError(token, 9, 5)


def misra_10_4(data):
    for token in data.tokenlist:
        if token.str not in {'+', '-', '*', '/', '%', '&', '|', '^'} and not token.isComparisonOp:
            continue
        if not token.astOperand1 or not token.astOperand2:
            continue
        if not token.astOperand1.valueType or not token.astOperand2.valueType:
            continue
        if not token.astOperand1.valueType.isIntegral() or not token.astOperand2.valueType.isIntegral():
            continue
        e1 = getEssentialType(token.astOperand1)
        e2 = getEssentialType(token.astOperand2)
        if e1 and e2 and e1 != e2:
            reportError(token, 10, 4)


def misra_10_6(data):
    for token in data.tokenlist:
        if token.str != '=' or not token.astOperand1 or not token.astOperand2:
            continue
        vt1 = token.astOperand1.valueType
        vt2 = token.astOperand2.valueType
        if not vt1 or vt1.pointer > 0:
            continue
        if not vt2 or vt2.pointer > 0:
            continue
        try:
            intTypes = ['char', 'short', 'int', 'long', 'long long']
            index1 = intTypes.index(vt1.type)
            e = getEssentialType(token.astOperand2)
            if not e:
                continue
            index2 = intTypes.index(e)
            if index1 > index2:
                reportError(token, 10, 6)
        except ValueError:
            pass


def misra_10_8(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        if not token.valueType or token.valueType.pointer > 0:
            continue
        if not token.astOperand1.valueType or token.astOperand1.valueType.pointer > 0:
            continue
        if not token.astOperand1.astOperand1:
            continue
        try:
            intTypes = ['char', 'short', 'int', 'long', 'long long']
            index1 = intTypes.index(token.valueType.type)
            e = getEssentialType(token.astOperand1)
            if not e:
                continue
            index2 = intTypes.index(e)
            if index1 > index2:
                reportError(token, 10, 8)
        except ValueError:
            pass


def misra_11_3(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        vt1 = token.valueType
        vt2 = token.astOperand1.valueType
        if not vt1 or not vt2:
            continue
        if vt1.pointer == vt2.pointer and vt1.pointer > 0 and vt1.type != vt2.type and\
           vt1.isIntegral() and vt2.isIntegral() and vt1.type != 'char':
            reportError(token, 11, 3)


def misra_11_4(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        vt1 = token.valueType
        vt2 = token.astOperand1.valueType
        if not vt1 or not vt2:
            continue
        if vt1.pointer == 0 and vt2.pointer > 0 and vt2.type != 'void':
            reportError(token, 11, 4)


def misra_11_5(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        vt1 = token.valueType
        vt2 = token.astOperand1.valueType
        if not vt1 or not vt2:
            continue
        if vt1.pointer > 0 and vt1.type != 'void' and vt2.pointer == vt1.pointer and vt2.type == 'void':
            reportError(token, 11, 5)


def misra_11_6(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        vt1 = token.valueType
        vt2 = token.astOperand1.valueType
        if not vt1 or not vt2:
            continue
        if vt1.pointer == 1 and vt1.type == 'void' and vt2.pointer == 0:
            reportError(token, 11, 6)
        elif vt1.pointer == 0 and vt2.pointer == 1 and vt2.type == 'void':
            reportError(token, 11, 6)


def misra_11_7(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        vt1 = token.valueType
        vt2 = token.astOperand1.valueType
        if not vt1 or not vt2:
            continue
        if vt1.pointer > 0 and vt1.type == 'record' and\
           vt2.pointer > 0 and vt2.type == 'record' and vt1.typeScopeId != vt2.typeScopeId:
            reportError(token, 11, 7)


def misra_11_8(data):
    for token in data.tokenlist:
        if not isCast(token):
            continue
        if not token.valueType or not token.astOperand1.valueType:
            continue
        if token.valueType.pointer == 0 or token.valueType.pointer == 0:
            continue
        if token.valueType.constness == 0 and token.astOperand1.valueType.constness > 0:
            reportError(token, 11, 8)


def misra_11_9(data):
    for directive in data.directives:
        res1 = re.match(r'#define ([A-Za-z_][A-Za-z_0-9]*) (.*)', directive.str)
        if not res1:
            continue
        name = res1.group(1)
        if name == 'NULL':
            continue
        value = res1.group(2).replace(' ', '')
        if value == '((void*)0)':
            reportError(directive, 11, 9)


def misra_12_1_sizeof(rawTokens):
    state = 0
    for tok in rawTokens:
        if tok.str.startswith('//') or tok.str.startswith('/*'):
            continue
        if tok.str == 'sizeof':
            state = 1
        elif state == 1:
            if re.match(r'^[a-zA-Z_]', tok.str):
                state = 2
            else:
                state = 0
        elif state == 2:
            if tok.str in {'+', '-', '*', '/', '%'}:
                reportError(tok, 12, 1)
            else:
                state = 0


def misra_12_1(data):
    for token in data.tokenlist:
        p = getPrecedence(token)
        if p < 2 or p > 12:
            continue
        p1 = getPrecedence(token.astOperand1)
        if p < p1 <= 12 and noParentheses(token.astOperand1, token):
            reportError(token, 12, 1)
            continue
        p2 = getPrecedence(token.astOperand2)
        if p < p2 <= 12 and noParentheses(token, token.astOperand2):
            reportError(token, 12, 1)
            continue


def misra_12_2(data):
    for token in data.tokenlist:
        if not (token.str in {'<<', '>>'}):
            continue
        if (not token.astOperand2) or (not token.astOperand2.values):
            continue
        maxval = 0
        for val in token.astOperand2.values:
            if val.intvalue and val.intvalue > maxval:
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
        if token.str != ',' or token.scope.type == 'Enum':
            continue
        if token.astParent and token.astParent.str in {'(', ',', '{'}:
            continue
        reportError(token, 12, 3)


def misra_12_4(data):
    if INT_BIT == 16:
        max_uint = 0xffff
    elif INT_BIT == 32:
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
            reportError(init, 13, 1)


def misra_13_3(data):
    for token in data.tokenlist:
        if token.str not in {'++', '--'}:
            continue
        astTop = token
        while astTop.astParent and astTop.astParent.str not in {',', ';'}:
            astTop = astTop.astParent
        if countSideEffects(astTop) >= 2:
            reportError(astTop, 13, 3)


def misra_13_4(data):
    for token in data.tokenlist:
        if token.str != '=':
            continue
        if not token.astParent:
            continue
        if token.astOperand1.str == '[' and token.astOperand1.previous.str in {'{', ','}:
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
        if not token.astOperand1 or not (token.astOperand1.str in {'if', 'while'}):
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
        if not findGotoLabel(token):
            reportError(token, 15, 2)


def misra_15_3(data):
    for token in data.tokenlist:
        if token.str != 'goto':
            continue
        if (not token.next) or (not token.next.isName):
            continue
        tok = findGotoLabel(token)
        if not tok:
            continue
        scope = token.scope
        while scope and scope != tok.scope:
            scope = scope.nestedIn
        if not scope:
            reportError(token, 15, 3)


def misra_15_5(data):
    for token in data.tokenlist:
        if token.str == 'return' and token.scope.type != 'Function':
            reportError(token, 15, 5)


def misra_15_6(rawTokens):
    state = 0
    indent = 0
    tok1 = None
    for token in rawTokens:
        if token.str in {'if', 'for', 'while'}:
            if simpleMatch(token.previous, '# if'):
                continue
            if simpleMatch(token.previous, "} while"):
                continue
            state = 1
            indent = 0
            tok1 = token
        elif state == 1:
            if indent == 0 and token.str != '(':
                state = 0
                continue
            if token.str == '(':
                indent = indent + 1
            elif token.str == ')':
                if indent == 0:
                    state = 0
                elif indent == 1:
                    state = 2
                indent = indent - 1
        elif state == 2:
            if token.str.startswith('//') or token.str.startswith('/*'):
                continue
            state = 0
            if token.str != '{':
                reportError(tok1, 15, 6)


def misra_15_7(data):
    for token in data.tokenlist:
        if not simpleMatch(token, '}'):
            continue
        if not token.scope.type == 'If':
            continue
        if not token.scope.nestedIn.type == 'Else':
            continue
        if not token.next.str == 'else':
            reportError(token, 15, 7)

# TODO add 16.1 rule


def misra_16_2(data):
    for token in data.tokenlist:
        if token.str == 'case' and token.scope.type != 'Switch':
            reportError(token, 16, 2)


def misra_16_3(rawTokens):
    # state: 0=no, 1=break is seen but not its ';', 2=after 'break;', 'comment', '{'
    state = 0
    for token in rawTokens:
        if token.str == 'break':
            state = 1
        elif token.str == ';':
            if state == 1:
                state = 2
            else:
                state = 0
        elif token.str.startswith('/*') or token.str.startswith('//'):
            if 'fallthrough' in token.str.lower():
                state = 2
        elif token.str == '{':
            state = 2
        elif token.str == 'case' and state != 2:
            reportError(token, 16, 3)


def misra_16_4(data):
    for token in data.tokenlist:
        if token.str != 'switch':
            continue
        if not simpleMatch(token, 'switch ('):
            continue
        if not simpleMatch(token.next.link, ') {'):
            continue
        startTok = token.next.link.next
        tok = startTok.next
        while tok and tok.str != '}':
            if tok.str == '{':
                tok = tok.link
            elif tok.str == 'default':
                break
            tok = tok.next
        if tok and tok.str != 'default':
            reportError(token, 16, 4)


def misra_16_5(data):
    for token in data.tokenlist:
        if token.str != 'default':
            continue
        if token.previous and token.previous.str == '{':
            continue
        tok2 = token
        while tok2:
            if tok2.str in {'}', 'case'}:
                break
            if tok2.str == '{':
                tok2 = tok2.link
            tok2 = tok2.next
        if tok2 and tok2.str == 'case':
            reportError(token, 16, 5)


def misra_16_6(data):
    for token in data.tokenlist:
        if not (simpleMatch(token, 'switch (') and simpleMatch(token.next.link, ') {')):
            continue
        tok = token.next.link.next.next
        count = 0
        while tok:
            if tok.str == 'break':
                count = count + 1
            elif tok.str == '{':
                tok = tok.link
                if simpleMatch(tok.previous.previous, 'break ;'):
                    count = count + 1
            elif tok.str == '}':
                break
            tok = tok.next
        if count < 2:
            reportError(token, 16, 6)


def misra_16_7(data):
    for token in data.tokenlist:
        if simpleMatch(token, 'switch (') and isBoolExpression(token.next.astOperand2):
            reportError(token, 16, 7)


def misra_17_1(data):
    for token in data.tokenlist:
        if isFunctionCall(token) and token.astOperand1.str in {'va_list', 'va_arg', 'va_start', 'va_end', 'va_copy'}:
            reportError(token, 17, 1)


def misra_17_6(rawTokens):
    for token in rawTokens:
        if simpleMatch(token, '[ static'):
            reportError(token, 17, 6)


def misra_17_8(data):
    for token in data.tokenlist:
        if not (token.isAssignmentOp or (token.str in {'++', '--'})):
            continue
        if not token.astOperand1:
            continue
        var = token.astOperand1.variable
        if var and var.isArgument:
            reportError(token, 17, 8)


def misra_18_5(data):
    for var in data.variables:
        if not var.isPointer:
            continue
        typetok = var.nameToken
        count = 0
        while typetok:
            if typetok.str == '*':
                count = count + 1
            elif not typetok.isName:
                break
            typetok = typetok.previous
        if count > 2:
            reportError(var.nameToken, 18, 5)


def misra_18_8(data):
    for var in data.variables:
        if not var.isArray or not var.isLocal:
            continue
        # TODO Array dimensions are not available in dump, must look in tokens
        typetok = var.nameToken.next
        if not typetok or typetok.str != '[':
            continue
        if not isConstantExpression(typetok.astOperand2):
            reportError(var.nameToken, 18, 8)


def misra_19_2(data):
    for token in data.tokenlist:
        if token.str == 'union':
            reportError(token, 19, 2)


def misra_20_1(data):
    for directive in data.directives:
        if not directive.str.startswith('#include'):
            continue
        for token in data.tokenlist:
            if token.file != directive.file:
                continue
            if int(token.linenr) < int(directive.linenr):
                reportError(directive, 20, 1)
                break


def misra_20_2(data):
    for directive in data.directives:
        if not directive.str.startswith('#include '):
            continue
        for pattern in {'\\', '//', '/*', "'"}:
            if pattern in directive.str:
                reportError(directive, 20, 2)
                break


def misra_20_3(rawTokens):
    linenr = -1
    for token in rawTokens:
        if token.str.startswith('/') or token.linenr == linenr:
            continue
        linenr = token.linenr
        if not simpleMatch(token, '# include'):
            continue
        headerToken = token.next.next
        if not headerToken or not (headerToken.str.startswith('<') or headerToken.str.startswith('"')):
            reportError(token, 20, 3)


def misra_20_4(data):
    for directive in data.directives:
        res = re.search(r'#define ([a-z][a-z0-9_]+)', directive.str)
        if res and (res.group(1) in KEYWORDS):
            reportError(directive, 20, 4)


def misra_20_5(data):
    for directive in data.directives:
        if directive.str.startswith('#undef '):
            reportError(directive, 20, 5)


def misra_21_3(data):
    for token in data.tokenlist:
        if isFunctionCall(token) and (token.astOperand1.str in {'malloc', 'calloc', 'realloc', 'free'}):
            reportError(token, 21, 3)


def misra_21_4(data):
    directive = findInclude(data.directives, '<setjmp.h>')
    if directive:
        reportError(directive, 21, 4)


def misra_21_5(data):
    directive = findInclude(data.directives, '<signal.h>')
    if directive:
        reportError(directive, 21, 5)


def misra_21_7(data):
    for token in data.tokenlist:
        if isFunctionCall(token) and (token.astOperand1.str in {'atof', 'atoi', 'atol', 'atoll'}):
            reportError(token, 21, 7)


def misra_21_8(data):
    for token in data.tokenlist:
        if isFunctionCall(token) and (token.astOperand1.str in {'abort', 'getenv', 'system'}):
            reportError(token, 21, 8)


def misra_21_9(data):
    for token in data.tokenlist:
        if (token.str in {'bsearch', 'qsort'}) and token.next and token.next.str == '(':
            reportError(token, 21, 9)


def misra_21_11(data):
    directive = findInclude(data.directives, '<tgmath.h>')
    if directive:
        reportError(directive, 21, 11)


def loadRuleTexts(filename):
    num1 = 0
    num2 = 0
    appendixA = False
    for line in open(filename, 'rt'):
        line = line.replace('\r', '').replace('\n', '')
        if not appendixA:
            if line.find('Appendix A') >= 0 and line.find('Summary of guidelines') >= 10:
                appendixA = True
            continue
        if line.find('Appendix B') >= 0:
            break
        res = re.match(r'^Rule ([0-9]+).([0-9]+)', line)
        if res:
            num1 = int(res.group(1))
            num2 = int(res.group(2))
            continue
        res = re.match(r'^[ ]*(Advisory|Required|Mandatory)$', line)
        if res:
            continue
        res = re.match(r'^[ ]*([#A-Z].*)', line)
        if res:
            global ruleTexts
            ruleTexts[num1*100+num2] = res.group(1)
            num2 = num2 + 1
            continue

def loadRuleTextsFromPdf(filename):
    f = tempfile.NamedTemporaryFile(delete=False)
    f.close()
    #print('tempfile:' + f.name)
    subprocess.call(['pdftotext', filename, f.name])
    loadRuleTexts(f.name)
    try:
        os.remove(f.name)
    except OSError as err:
        print('Failed to remove temporary file ' + f.name)

if len(sys.argv) == 1:
    print("""
Syntax: misra.py [OPTIONS] <dumpfiles>

OPTIONS:

--rule-texts=<file>   Load rule texts from plain text file.

                      If you have the tool 'pdftotext' you can generate
                      this textfile with such command:

                          $ pdftotext MISRA_C_2012.pdf MISRA_C_2012.txt

                      Otherwise you can more or less copy/paste the chapter
                        Appendix A Summary of guidelines
                      from the MISRA pdf.

                      Format:

                        <..arbitrary text..>
                        Appendix A Summary of guidelines
                        Dir 1.1
                        Rule text for 1.1
                        Dir 1.2
                        Rule text for 1.2
                        <...>

--misra-pdf=<file>    Misra PDF file that rule texts will be extracted from.

                      The tool 'pdftotext' from xpdf is used and must be installed.
                      Debian:  sudo apt-get install xpdf
                      Windows: http://gnuwin32.sourceforge.net/packages/xpdf.htm

                      If you don't have 'pdftotext' and don't want to install it then
                      you can use --rule-texts=<file>.
""")
    sys.exit(1)

for arg in sys.argv[1:]:
    if arg == '-verify':
        VERIFY = True
    elif arg.startswith('--rule-texts='):
        loadRuleTexts(arg[13:])
    elif arg.startswith('--misra-pdf='):
        loadRuleTextsFromPdf(arg[12:])

for arg in sys.argv[1:]:
    if not arg.endswith('.dump'):
        continue

    data = cppcheckdata.parsedump(arg)

    CHAR_BIT      = data.platform.char_bit
    SHORT_BIT     = data.platform.short_bit
    INT_BIT       = data.platform.int_bit
    LONG_BIT      = data.platform.long_bit
    LONG_LONG_BIT = data.platform.long_long_bit
    POINTER_BIT   = data.platform.pointer_bit

    if VERIFY:
        VERIFY_ACTUAL = []
        VERIFY_EXPECTED = []
        for tok in data.rawTokens:
            if tok.str.startswith('//') and 'TODO' not in tok.str:
                for word in tok.str[2:].split(' '):
                    if re.match(r'[0-9]+\.[0-9]+', word):
                        VERIFY_EXPECTED.append(str(tok.linenr) + ':' + word)
    else:
        print('Checking ' + arg + '...')

    cfgNumber = 0

    for cfg in data.configurations:
        cfgNumber = cfgNumber + 1
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')

        if cfgNumber == 1:
            misra_3_1(data.rawTokens)
        misra_5_1(cfg)
        misra_5_3(cfg)
        misra_5_4(cfg)
        misra_5_5(cfg)
        if cfgNumber == 1:
            misra_7_1(data.rawTokens)
            misra_7_3(data.rawTokens)
        misra_8_11(cfg)
        misra_8_12(cfg)
        if cfgNumber == 1:
            misra_8_14(data.rawTokens)
            misra_9_5(data.rawTokens)
        misra_10_4(cfg)
        misra_10_6(cfg)
        misra_10_8(cfg)
        misra_11_3(cfg)
        misra_11_4(cfg)
        misra_11_5(cfg)
        misra_11_6(cfg)
        misra_11_7(cfg)
        misra_11_8(cfg)
        misra_11_9(cfg)
        if cfgNumber == 1:
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
        misra_15_3(cfg)
        misra_15_5(cfg)
        if cfgNumber == 1:
            misra_15_6(data.rawTokens)
        misra_15_7(cfg)
        misra_16_2(cfg)
        if cfgNumber == 1:
            misra_16_3(data.rawTokens)
        misra_16_4(cfg)
        misra_16_5(cfg)
        misra_16_6(cfg)
        misra_16_7(cfg)
        misra_17_1(cfg)
        if cfgNumber == 1:
            misra_17_6(data.rawTokens)
        misra_17_8(cfg)
        misra_18_5(cfg)
        misra_18_8(cfg)
        misra_19_2(cfg)
        misra_20_1(cfg)
        misra_20_2(cfg)
        if cfgNumber == 1:
            misra_20_3(data.rawTokens)
        misra_20_4(cfg)
        misra_20_5(cfg)
        misra_21_3(cfg)
        misra_21_4(cfg)
        misra_21_5(cfg)
        misra_21_7(cfg)
        misra_21_8(cfg)
        misra_21_9(cfg)
        misra_21_11(cfg)

    if VERIFY:
        for expected in VERIFY_EXPECTED:
            if expected not in VERIFY_ACTUAL:
                print('Expected but not seen: ' + expected)
                sys.exit(1)
        for actual in VERIFY_ACTUAL:
            if actual not in VERIFY_EXPECTED:
                print('Not expected: ' + actual)
                sys.exit(1)
