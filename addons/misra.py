#!/usr/bin/env python
#
# MISRA C 2012 checkers
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misra.py --rule-texts=<path-to-rule-texts> main.cpp.dump
#
# Limitations: This addon is released as open source. Rule texts can't be freely
# distributed. https://www.misra.org.uk/forum/viewtopic.php?f=56&t=1189
#
# The MISRA standard documents may be obtained from https://www.misra.org.uk
#
# Total number of rules: 143

from __future__ import print_function

import cppcheckdata
import sys
import re
import os
import argparse


typeBits = {
    'CHAR': None,
    'SHORT': None,
    'INT': None,
    'LONG': None,
    'LONG_LONG': None,
    'POINTER': None
}

VERIFY = False
QUIET = False
SHOW_SUMMARY = True

def printStatus(*args, **kwargs):
    if not QUIET:
        print(*args, **kwargs)


def simpleMatch(token, pattern):
    for p in pattern.split(' '):
        if not token or token.str != p:
            return False
        token = token.next
    return True


def rawlink(rawtoken):
    if rawtoken.str == '}':
        indent = 0
        while rawtoken:
            if rawtoken.str == '}':
                indent = indent + 1
            elif rawtoken.str == '{':
                indent = indent - 1
                if indent == 0:
                    break
            rawtoken = rawtoken.previous
    else:
        rawtoken = None
    return rawtoken


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


def getEssentialTypeCategory(expr):
    if not expr:
        return None
    if expr.valueType and expr.valueType.typeScope:
        return "enum<" + expr.valueType.typeScope.className + ">"
    if expr.variable:
        typeToken = expr.variable.typeStartToken
        while typeToken:
            if typeToken.valueType:
                if typeToken.valueType.type in {'bool'}:
                    return typeToken.valueType.type
                if typeToken.valueType.type in {'float', 'double', 'long double'}:
                    return "float"
                if typeToken.valueType.sign:
                    return typeToken.valueType.sign
            typeToken = typeToken.next
    if expr.valueType:
        return expr.valueType.sign
    return None


def getEssentialCategorylist(operand1, operand2):
    if not operand1 or not operand2:
        return None, None
    if (operand1.str in {'++', '--'} or
            operand2.str in {'++', '--'}):
        return None, None
    if (operand1.valueType.pointer or
            operand2.valueType.pointer):
        return None, None
    e1 = getEssentialTypeCategory(operand1)
    e2 = getEssentialTypeCategory(operand2)
    return e1, e2


def getEssentialType(expr):
    if not expr:
        return None
    if expr.variable:
        typeToken = expr.variable.typeStartToken
        while typeToken and typeToken.isName:
            if typeToken.str in {'char', 'short', 'int', 'long', 'float', 'double'}:
                return typeToken.str
            typeToken = typeToken.next

    elif expr.astOperand1 and expr.astOperand2 and expr.str in {'+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":"}:
        if expr.astOperand1.valueType and expr.astOperand1.valueType.pointer > 0:
            return None
        if expr.astOperand2.valueType and expr.astOperand2.valueType.pointer > 0:
            return None
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
    elif expr.str == "~":
        e1 = getEssentialType(expr.astOperand1)
        return e1

    return None


def bitsOfEssentialType(expr):
    type = getEssentialType(expr)
    if type is None:
        return 0
    if type == 'char':
        return typeBits['CHAR']
    if type == 'short':
        return typeBits['SHORT']
    if type == 'int':
        return typeBits['INT']
    if type == 'long':
        return typeBits['LONG']
    if type == 'long long':
        return typeBits['LONG_LONG']
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


def findCounterTokens(cond):
    if not cond:
        return []
    if cond.str in ['&&', '||']:
        c = findCounterTokens(cond.astOperand1)
        c.extend(findCounterTokens(cond.astOperand2))
        return c
    ret = []
    if ((cond.isArithmeticalOp and cond.astOperand1 and cond.astOperand2) or
            (cond.isComparisonOp and cond.astOperand1 and cond.astOperand2)):
        if cond.astOperand1.isName:
            ret.append(cond.astOperand1)
        if cond.astOperand2.isName:
            ret.append(cond.astOperand2)
        if cond.astOperand1.isOp:
            ret.extend(findCounterTokens(cond.astOperand1))
        if cond.astOperand2.isOp:
            ret.extend(findCounterTokens(cond.astOperand2))
    return ret


def isFloatCounterInWhileLoop(whileToken):
    if not simpleMatch(whileToken, 'while ('):
        return False
    lpar = whileToken.next
    rpar = lpar.link
    counterTokens = findCounterTokens(lpar.astOperand2)
    whileBodyStart = None
    if simpleMatch(rpar, ') {'):
        whileBodyStart = rpar.next
    elif simpleMatch(whileToken.previous, '} while') and simpleMatch(whileToken.previous.link.previous, 'do {'):
        whileBodyStart = whileToken.previous.link
    else:
        return False
    token = whileBodyStart
    while token != whileBodyStart.link:
        token = token.next
        for counterToken in counterTokens:
            if not counterToken.valueType or not counterToken.valueType.isFloat():
                continue
            if token.isAssignmentOp and token.astOperand1.str == counterToken.str:
                return True
            if token.str == counterToken.str and token.astParent and token.astParent.str in {'++', '--'}:
                return True
    return False


def hasSideEffectsRecursive(expr):
    if not expr:
        return False
    if expr.str == '=' and expr.astOperand1 and expr.astOperand1.str == '[':
        prev = expr.astOperand1.previous
        if prev and (prev.str == '{' or prev.str == '{'):
            return hasSideEffectsRecursive(expr.astOperand2)
    if expr.str == '=' and expr.astOperand1 and expr.astOperand1.str == '.':
        e = expr.astOperand1
        while e and e.str == '.' and e.astOperand2:
            e = e.astOperand1
        if e and e.str == '.':
            return False
    if expr.str in {'++', '--', '='}:
        return True
    # Todo: Check function calls
    return hasSideEffectsRecursive(expr.astOperand1) or hasSideEffectsRecursive(expr.astOperand2)


def isBoolExpression(expr):
    if not expr:
        return False
    if expr.valueType and (expr.valueType.type == 'bool' or expr.valueType.bits == 1):
        return True
    return expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||', '0', '1', 'true', 'false']


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


def findRawLink(token):
    tok1 = None
    tok2 = None
    forward = False

    if token.str in '{([':
        tok1 = token.str
        tok2 = '})]'['{(['.find(token.str)]
        forward = True
    elif token.str in '})]':
        tok1 = token.str
        tok2 = '{(['['})]'.find(token.str)]
        forward = False
    else:
        return None

    # try to find link
    indent = 0
    while token:
        if token.str == tok1:
            indent = indent + 1
        elif token.str == tok2:
            if indent <= 1:
                return token
            indent = indent - 1
        if forward is True:
            token = token.next
        else:
            token = token.previous

    # raw link not found
    return None


def numberOfParentheses(tok1, tok2):
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


# Get function arguments
def getArgumentsRecursive(tok, arguments):
    if tok is None:
        return
    if tok.str == ',':
        getArgumentsRecursive(tok.astOperand1, arguments)
        getArgumentsRecursive(tok.astOperand2, arguments)
    else:
        arguments.append(tok)


def getArguments(ftok):
    arguments = []
    getArgumentsRecursive(ftok.astOperand2, arguments)
    return arguments


def isHexDigit(c):
    return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or (c >= 'A' and c >= 'F')


def isOctalDigit(c):
    return c >= '0' and c <= '7'


def isNoReturnScope(tok):
    if tok is None or tok.str != '}':
        return False
    if tok.previous is None or tok.previous.str != ';':
        return False
    if simpleMatch(tok.previous.previous, 'break ;'):
        return True
    prev = tok.previous.previous
    while prev and prev.str not in ';{}':
        if prev.str in '])':
            prev = prev.link
        prev = prev.previous
    if prev and prev.next.str in ['throw', 'return']:
        return True
    return False

def generateTable():
    numberOfRules = {}
    numberOfRules[1] = 3
    numberOfRules[2] = 7
    numberOfRules[3] = 2
    numberOfRules[4] = 2
    numberOfRules[5] = 9
    numberOfRules[6] = 2
    numberOfRules[7] = 4
    numberOfRules[8] = 14
    numberOfRules[9] = 5
    numberOfRules[10] = 8
    numberOfRules[11] = 9
    numberOfRules[12] = 4
    numberOfRules[13] = 6
    numberOfRules[14] = 4
    numberOfRules[15] = 7
    numberOfRules[16] = 7
    numberOfRules[17] = 8
    numberOfRules[18] = 8
    numberOfRules[19] = 2
    numberOfRules[20] = 14
    numberOfRules[21] = 12
    numberOfRules[22] = 6

    # what rules are handled by this addon?
    addon = []
    compiled = re.compile(r'[ ]+misra_([0-9]+)_([0-9]+)[(].*')
    for line in open(__file__):
        res = compiled.match(line)
        if res is None:
            continue
        addon.append(res.group(1) + '.' + res.group(2))

    # rules handled by cppcheck
    cppcheck = ['1.3', '2.1', '2.2', '2.4', '2.6', '8.3', '12.2', '13.2', '13.6', '17.5', '18.1', '18.6',
                '20.6', '22.1', '22.2', '22.4', '22.6']

    # rules that can be checked with compilers
    # compiler = ['1.1', '1.2']

    # print table
    for i1 in range(1, 23):
        for i2 in range(1, numberOfRules[i1] + 1):
            num = str(i1) + '.' + str(i2)
            s = ''
            if num in addon:
                s = 'X (Addon)'
            elif num in cppcheck:
                s = 'X (Cppcheck)'
            num = num + '       '
            print(num[:8] + s)
    sys.exit(1)


def remove_file_prefix(file_path, prefix):
    """
    Remove a file path prefix from a give path.  leftover
    directory seperators at the beginning of a file
    after the removal are also stripped.

    Example:
        '/remove/this/path/file.c'
    with a prefix of:
        '/remove/this/path'
    becomes:
        file.c
    """
    result = None
    if file_path.startswith(prefix):
        result = file_path[len(prefix):]
        # Remove any leftover directory seperators at the
        # beginning
        result = result.lstrip('\\/')
    else:
        result = file_path
    return result


class MisraChecker:

    def __init__(self):

        # Test validation rules lists
        self.verify_expected    = list()
        self.verify_actual      = list()

        # List of formatted violation messages
        self.violations         = list()

        # if --rule-texts is specified this dictionary
        # is loaded with descriptions of each rule
        # by rule number (in hundreds).
        # ie rule 1.2 becomes 102
        self.ruleTexts          = dict()

        # Dictionary of dictionaries for rules to suppress
        # Dict1 is keyed by rule number in the hundreds format of
        # Major *  100 + minor. ie Rule 5.2 = (5*100) + 2
        # Dict 2 is keyed by filename.  An entry of None means suppress globaly.
        # Each file name entry contails a list of tuples of (lineNumber, symbolName)
        # or an item of None which indicates suppress rule for the entire file.
        # The line and symbol name tuple may have None as either of its elements but
        # should not be None for both.
        self.suppressedRules    = dict()

        # List of suppression extracted from the dumpfile
        self.dumpfileSuppressions = None

        # Prefix to ignore when matching suppression files.
        self.filePrefix = None

    def misra_3_1(self, rawTokens):
        for token in rawTokens:
            if token.str.startswith('/*') or token.str.startswith('//'):
                s = token.str.lstrip('/')
                if '//' in s or '/*' in s:
                    self.reportError(token, 3, 1)


    def misra_4_1(self, rawTokens):
        for token in rawTokens:
            if token.str[0] != '"':
                continue
            pos = 1
            while pos < len(token.str) - 2:
                pos1 = pos
                pos = pos + 1
                if token.str[pos1] != '\\':
                    continue
                if token.str[pos1 + 1] == '\\':
                    pos = pos1 + 2
                    continue
                if token.str[pos1 + 1] == 'x':
                    if not isHexDigit(token.str[pos1 + 2]):
                        self.reportError(token, 4, 1)
                        continue
                    if not isHexDigit(token.str[pos1 + 3]):
                        self.reportError(token, 4, 1)
                        continue
                elif isOctalDigit(token.str[pos1 + 1]):
                    if not isOctalDigit(token.str[pos1 + 2]):
                        self.reportError(token, 4, 1)
                        continue
                    if not isOctalDigit(token.str[pos1 + 2]):
                        self.reportError(token, 4, 1)
                        continue
                else:
                    continue

                c = token.str[pos1 + 4]
                if c != '"' and c != '\\':
                    self.reportError(token, 4, 1)


    def misra_5_1(self, data):
        scopeVars = {}
        for var in data.variables:
            if var.isArgument:
                continue
            if var.nameToken.scope not in scopeVars:
                scopeVars[var.nameToken.scope] = []
            scopeVars[var.nameToken.scope].append(var)
        for scope in scopeVars:
            for i, variable1 in enumerate(scopeVars[scope]):
                for variable2 in scopeVars[scope][i + 1:]:
                    if (variable1.isExtern and variable2.isExtern and
                        variable1.nameToken.str[:31] == variable2.nameToken.str[:31] and
                            variable1.Id != variable2.Id):
                        if int(variable1.nameToken.linenr) > int(variable2.nameToken.linenr):
                            self.reportError(variable1.nameToken, 5, 1)
                        else:
                            self.reportError(variable2.nameToken, 5, 1)


    def misra_5_2(self, data):
        scopeVars = {}
        for var in data.variables:
            if var.nameToken is not None:
                if var.nameToken.scope not in scopeVars:
                    scopeVars.setdefault(var.nameToken.scope, {})["varlist"] = []
                    scopeVars.setdefault(var.nameToken.scope, {})["scopelist"] = []
                scopeVars[var.nameToken.scope]["varlist"].append(var)
        for scope in data.scopes:
            if scope.nestedIn and scope.className:
                if scope.nestedIn not in scopeVars:
                    scopeVars.setdefault(scope.nestedIn, {})["varlist"] = []
                    scopeVars.setdefault(scope.nestedIn, {})["scopelist"] = []
                scopeVars[scope.nestedIn]["scopelist"].append(scope)
        for scope in scopeVars:
            if len(scopeVars[scope]["varlist"]) <= 1:
                continue
            for i, variable1 in enumerate(scopeVars[scope]["varlist"]):
                for variable2 in scopeVars[scope]["varlist"][i + 1:]:
                    if variable1.isArgument and variable2.isArgument:
                        continue
                    if variable1.isExtern or variable2.isExtern:
                        continue
                    if (variable1.nameToken.str[:31] == variable2.nameToken.str[:31] and
                            variable1.Id != variable2.Id):
                        if int(variable1.nameToken.linenr) > int(variable2.nameToken.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(variable2.nameToken, 5, 2)
                for innerscope in scopeVars[scope]["scopelist"]:
                    if variable1.nameToken.str[:31] == innerscope.className[:31]:
                        if int(variable1.nameToken.linenr) > int(innerscope.bodyStart.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(innerscope.bodyStart, 5, 2)
            if len(scopeVars[scope]["scopelist"]) <= 1:
                continue
            for i, scopename1 in enumerate(scopeVars[scope]["scopelist"]):
                for scopename2 in scopeVars[scope]["scopelist"][i + 1:]:
                    if scopename1.className[:31] == scopename2.className[:31]:
                        if int(scopename1.bodyStart.linenr) > int(scopename2.bodyStart.linenr):
                            self.reportError(scopename1.bodyStart, 5, 2)
                        else:
                            self.reportError(scopename2.bodyStart, 5, 2)


    def misra_5_3(self, data):
        enum = []
        scopeVars = {}
        for var in data.variables:
            if var.nameToken is not None:
                if var.nameToken.scope not in scopeVars:
                    scopeVars[var.nameToken.scope] = []
                scopeVars[var.nameToken.scope].append(var)
        for innerScope in data.scopes:
            if innerScope.type == "Enum":
                enum_token = innerScope.bodyStart.next
                while enum_token != innerScope.bodyEnd:
                    if enum_token.values and enum_token.isName:
                        enum.append(enum_token.str)
                    enum_token = enum_token.next
                continue
            if innerScope not in scopeVars:
                continue
            if innerScope.type == "Global":
                continue
            for innerVar in scopeVars[innerScope]:
                outerScope = innerScope.nestedIn
                while outerScope:
                    if outerScope not in scopeVars:
                        outerScope = outerScope.nestedIn
                        continue
                    for outerVar in scopeVars[outerScope]:
                        if innerVar.nameToken.str[:31] == outerVar.nameToken.str[:31]:
                            if outerVar.isArgument and outerScope.type == "Global" and not innerVar.isArgument:
                                continue
                            if int(innerVar.nameToken.linenr) > int(outerVar.nameToken.linenr):
                                self.reportError(innerVar.nameToken, 5, 3)
                            else:
                                self.reportError(outerVar.nameToken, 5, 3)
                    outerScope = outerScope.nestedIn
                for scope in data.scopes:
                    if scope.className and innerVar.nameToken.str[:31] == scope.className[:31]:
                        if int(innerVar.nameToken.linenr) > int(scope.bodyStart.linenr):
                            self.reportError(innerVar.nameToken, 5, 3)
                        else:
                            self.reportError(scope.bodyStart, 5, 3)

                for e in enum:
                    if scope.className and innerVar.nameToken.str[:31] == e[:31]:
                        if int(innerVar.nameToken.linenr) > int(innerScope.bodyStart.linenr):
                            self.reportError(innerVar.nameToken, 5, 3)
                        else:
                            self.reportError(innerScope.bodyStart, 5, 3)
        for e in enum:
            for scope in data.scopes:
                if scope.className and scope.className[:31] == e[:31]:
                    self.reportError(scope.bodyStart, 5, 3)


    def misra_5_4(self, data):
        macro = {}
        compile_name = re.compile(r'#define ([a-zA-Z0-9_]+)')
        compile_param = re.compile(r'#define ([a-zA-Z0-9_]+)[(]([a-zA-Z0-9_, ]+)[)]')
        for dir in data.directives:
            res1 = compile_name.match(dir.str)
            if res1:
                if dir not in macro:
                    macro.setdefault(dir, {})["name"] = []
                    macro.setdefault(dir, {})["params"] = []
                macro[dir]["name"] = res1.group(1)
            res2 = compile_param.match(dir.str)
            if res2:
                res_gp2 = res2.group(2).split(",")
                res_gp2 = [macroname.replace(" ", "") for macroname in res_gp2]
                macro[dir]["params"].extend(res_gp2)
        for mvar in macro:
            if len(macro[mvar]["params"]) > 0:
                for i, macroparam1 in enumerate(macro[mvar]["params"]):
                    for j, macroparam2 in enumerate(macro[mvar]["params"]):
                        if j > i and macroparam1[:31] == macroparam2[:31]:
                            self.reportError(mvar, 5, 4)

        for x, m_var1 in enumerate(macro):
            for y, m_var2 in enumerate(macro):
                if x < y and macro[m_var1]["name"][:31] == macro[m_var2]["name"][:31]:
                    if m_var1.linenr > m_var2.linenr:
                        self.reportError(m_var1, 5, 4)
                    else:
                        self.reportError(m_var2, 5, 4)
                for param in macro[m_var2]["params"]:
                    if macro[m_var1]["name"][:31] == param[:31]:
                        if m_var1.linenr > m_var2.linenr:
                            self.reportError(m_var1, 5, 4)
                        else:
                            self.reportError(m_var2, 5, 4)


    def misra_5_5(self, data):
        macroNames = []
        compiled = re.compile(r'#define ([A-Za-z0-9_]+)')
        for dir in data.directives:
            res = compiled.match(dir.str)
            if res:
                macroNames.append(res.group(1))
        for var in data.variables:
            for macro in macroNames:
                if var.nameToken is not None:
                    if var.nameToken.str[:31] == macro[:31]:
                        self.reportError(var.nameToken, 5, 5)
        for scope in data.scopes:
            for macro in macroNames:
                if scope.className and scope.className[:31] == macro[:31]:
                    self.reportError(scope.bodyStart, 5, 5)


    def misra_7_1(self, rawTokens):
        compiled = re.compile(r'^0[0-7]+$')
        for tok in rawTokens:
            if compiled.match(tok.str):
                self.reportError(tok, 7, 1)


    def misra_7_3(self, rawTokens):
        compiled = re.compile(r'^[0-9.uU]+l')
        for tok in rawTokens:
            if compiled.match(tok.str):
                self.reportError(tok, 7, 3)


    def misra_8_11(self, data):
        for var in data.variables:
            if var.isExtern and simpleMatch(var.nameToken.next, '[ ]') and var.nameToken.scope.type == 'Global':
                self.reportError(var.nameToken, 8, 11)


    def misra_8_12(self, data):
        for scope in data.scopes:
            enum_values = []
            implicit_enum_values = []
            if scope.type != 'Enum':
                continue
            e_token = scope.bodyStart.next
            while e_token != scope.bodyEnd:
                if e_token.isName and \
                e_token.values and \
                e_token.valueType and e_token.valueType.typeScope == scope:
                    token_values = [v.intvalue for v in e_token.values]
                    enum_values += token_values
                    if e_token.next.str != "=":
                        implicit_enum_values += token_values
                e_token = e_token.next
            for implicit_enum_value in implicit_enum_values:
                if enum_values.count(implicit_enum_value) != 1:
                    self.reportError(scope.bodyStart, 8, 12)


    def misra_8_14(self, rawTokens):
        for token in rawTokens:
            if token.str == 'restrict':
                self.reportError(token, 8, 14)


    def misra_9_5(self, rawTokens):
        for token in rawTokens:
            if simpleMatch(token, '[ ] = { ['):
                self.reportError(token, 9, 5)


    def misra_10_1(self, data):
        for token in data.tokenlist:
            if not token.isOp:
                continue
            e1 = getEssentialTypeCategory(token.astOperand1)
            e2 = getEssentialTypeCategory(token.astOperand2)
            if not e1 or not e2:
                continue
            if token.str in ['<<', '>>']:
                if e1 != 'unsigned':
                    self.reportError(token, 10, 1)
                elif e2 != 'unsigned' and not token.astOperand2.isNumber:
                    self.reportError(token, 10, 1)

    def misra_10_4(self, data):
        op = {'+', '-', '*', '/', '%', '&', '|', '^', '+=', '-=', ':'}
        for token in data.tokenlist:
            if token.str not in op and not token.isComparisonOp:
                continue
            if not token.astOperand1 or not token.astOperand2:
                continue
            if not token.astOperand1.valueType or not token.astOperand2.valueType:
                continue
            if ((token.astOperand1.str in op or token.astOperand1.isComparisonOp) and
                    (token.astOperand2.str in op or token.astOperand1.isComparisonOp)):
                e1, e2 = getEssentialCategorylist(token.astOperand1.astOperand2, token.astOperand2.astOperand1)
            elif token.astOperand1.str in op or token.astOperand1.isComparisonOp:
                e1, e2 = getEssentialCategorylist(token.astOperand1.astOperand2, token.astOperand2)
            elif token.astOperand2.str in op or token.astOperand2.isComparisonOp:
                e1, e2 = getEssentialCategorylist(token.astOperand1, token.astOperand2.astOperand1)
            else:
                e1, e2 = getEssentialCategorylist(token.astOperand1, token.astOperand2)
            if token.str == "+=" or token.str == "+":
                if e1 == "char" and (e2 == "signed" or e2 == "unsigned"):
                    continue
                if e2 == "char" and (e1 == "signed" or e1 == "unsigned"):
                    continue
            if token.str == "-=" or token.str == "-":
                if e1 == "char" and (e2 == "signed" or e2 == "unsigned"):
                    continue
            if e1 and e2 and (e1.find('Anonymous') != -1 and (e2 == "signed" or e2 == "unsigned")):
                continue
            if e1 and e2 and (e2.find('Anonymous') != -1 and (e1 == "signed" or e1 == "unsigned")):
                continue
            if e1 and e2 and e1 != e2:
                self.reportError(token, 10, 4)


    def misra_10_6(self, data):
        for token in data.tokenlist:
            if token.str != '=' or not token.astOperand1 or not token.astOperand2:
                continue
            if (token.astOperand2.str not in {'+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~'} and
                    not isCast(token.astOperand2)):
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
                if isCast(token.astOperand2):
                    e = vt2.type
                else:
                    e = getEssentialType(token.astOperand2)
                if not e:
                    continue
                index2 = intTypes.index(e)
                if index1 > index2:
                    self.reportError(token, 10, 6)
            except ValueError:
                pass

    def misra_10_8(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            if not token.valueType or token.valueType.pointer > 0:
                continue
            if not token.astOperand1.valueType or token.astOperand1.valueType.pointer > 0:
                continue
            if not token.astOperand1.astOperand1:
                continue
            if token.astOperand1.str not in {'+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~'}:
                continue
            if token.astOperand1.str != '~' and not token.astOperand1.astOperand2:
                continue
            if token.astOperand1.str == '~':
                e2 = getEssentialTypeCategory(token.astOperand1.astOperand1)
            else:
                e2, e3 = getEssentialCategorylist(token.astOperand1.astOperand1, token.astOperand1.astOperand2)
                if e2 != e3:
                    continue
            e1 = getEssentialTypeCategory(token)
            if e1 != e2:
                self.reportError(token, 10, 8)
            else:
                try:
                    intTypes = ['char', 'short', 'int', 'long', 'long long']
                    index1 = intTypes.index(token.valueType.type)
                    e = getEssentialType(token.astOperand1)
                    if not e:
                        continue
                    index2 = intTypes.index(e)
                    if index1 > index2:
                        self.reportError(token, 10, 8)
                except ValueError:
                    pass


    def misra_11_3(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.type == 'void' or vt2.type == 'void':
                continue
            if (vt1.pointer > 0 and vt1.type == 'record' and
                vt2.pointer > 0 and vt2.type == 'record' and
                    vt1.typeScopeId != vt2.typeScopeId):
                self.reportError(token, 11, 3)
            elif (vt1.pointer == vt2.pointer and vt1.pointer > 0 and
                    vt1.type != vt2.type and vt1.type != 'char'):
                self.reportError(token, 11, 3)


    def misra_11_4(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt2.pointer > 0 and vt1.pointer == 0 and (vt1.isIntegral() or vt1.isEnum()) and vt2.type != 'void':
                self.reportError(token, 11, 4)
            elif vt1.pointer > 0 and vt2.pointer == 0 and (vt2.isIntegral() or vt2.isEnum())and vt1.type != 'void':
                self.reportError(token, 11, 4)


    def misra_11_5(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                if token.astOperand1 and token.astOperand2 and token.str == "=" and token.next.str != "(":
                    vt1 = token.astOperand1.valueType
                    vt2 = token.astOperand2.valueType
                    if not vt1 or not vt2:
                        continue
                    if vt1.pointer > 0 and vt1.type != 'void' and vt2.pointer == vt1.pointer and vt2.type == 'void':
                        self.reportError(token, 11, 5)
                continue
            if token.astOperand1.astOperand1 and token.astOperand1.astOperand1.str in {'malloc', 'calloc', 'realloc', 'free'}:
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.pointer > 0 and vt1.type != 'void' and vt2.pointer == vt1.pointer and vt2.type == 'void':
                self.reportError(token, 11, 5)


    def misra_11_6(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            if token.astOperand1.astOperand1:
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if vt1.pointer == 1 and vt1.type == 'void' and vt2.pointer == 0 and token.astOperand1.str != "0":
                self.reportError(token, 11, 6)
            elif vt1.pointer == 0 and vt1.type != 'void' and vt2.pointer == 1 and vt2.type == 'void':
                self.reportError(token, 11, 6)


    def misra_11_7(self, data):
        for token in data.tokenlist:
            if not isCast(token):
                continue
            vt1 = token.valueType
            vt2 = token.astOperand1.valueType
            if not vt1 or not vt2:
                continue
            if token.astOperand1.astOperand1:
                continue
            if (vt2.pointer > 0 and vt1.pointer == 0 and
                    not vt1.isIntegral() and not vt1.isEnum() and
                    vt2.type != 'void'):
                self.reportError(token, 11, 7)
            elif (vt1.pointer > 0 and vt2.pointer == 0 and
                    not vt2.isIntegral() and not vt2.isEnum() and
                    vt1.type != 'void'):
                self.reportError(token, 11, 7)


    def misra_11_8(self, data):
        # TODO: reuse code in CERT-EXP05
        for token in data.tokenlist:
            if isCast(token):
                # C-style cast
                if not token.valueType:
                    continue
                if not token.astOperand1.valueType:
                    continue
                if token.valueType.pointer == 0:
                    continue
                if token.astOperand1.valueType.pointer == 0:
                    continue
                const1 = token.valueType.constness
                const2 = token.astOperand1.valueType.constness
                if (const1 % 2) < (const2 % 2):
                    self.reportError(token, 11, 8)

            elif token.str == '(' and token.astOperand1 and token.astOperand2 and token.astOperand1.function:
                # Function call
                function = token.astOperand1.function
                arguments = getArguments(token)
                for argnr, argvar in function.argument.items():
                    if argnr < 1 or argnr > len(arguments):
                        continue
                    if not argvar.isPointer:
                        continue
                    argtok = arguments[argnr - 1]
                    if not argtok.valueType:
                        continue
                    if argtok.valueType.pointer == 0:
                        continue
                    const1 = argvar.constness
                    const2 = arguments[argnr - 1].valueType.constness
                    if (const1 % 2) < (const2 % 2):
                        self.reportError(token, 11, 8)


    def misra_11_9(self, data):
        for token in data.tokenlist:
            if token.astOperand1 and token.astOperand2 and token.str in ["=", "==", "!=", "?", ":"]:
                vt1 = token.astOperand1.valueType
                vt2 = token.astOperand2.valueType
                if not vt1 or not vt2:
                    continue
                if vt1.pointer > 0 and vt2.pointer == 0 and token.astOperand2.str == "NULL":
                    continue
                if (token.astOperand2.values and vt1.pointer > 0 and
                        vt2.pointer == 0 and token.astOperand2.values):
                    for val in token.astOperand2.values:
                        if val.intvalue == 0:
                            self.reportError(token, 11, 9)

    def misra_12_1_sizeof(self, rawTokens):
        state = 0
        compiled = re.compile(r'^[a-zA-Z_]')
        for tok in rawTokens:
            if tok.str.startswith('//') or tok.str.startswith('/*'):
                continue
            if tok.str == 'sizeof':
                state = 1
            elif state == 1:
                if compiled.match(tok.str):
                    state = 2
                else:
                    state = 0
            elif state == 2:
                if tok.str in {'+', '-', '*', '/', '%'}:
                    self.reportError(tok, 12, 1)
                else:
                    state = 0


    def misra_12_1(self, data):
        for token in data.tokenlist:
            p = getPrecedence(token)
            if p < 2 or p > 12:
                continue
            p1 = getPrecedence(token.astOperand1)
            if p < p1 <= 12 and numberOfParentheses(token.astOperand1, token):
                self.reportError(token, 12, 1)
                continue
            p2 = getPrecedence(token.astOperand2)
            if p < p2 <= 12 and numberOfParentheses(token, token.astOperand2):
                self.reportError(token, 12, 1)
                continue


    def misra_12_2(self, data):
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
                self.reportError(token, 12, 2)


    def misra_12_3(self, data):
        for token in data.tokenlist:
            if token.str != ',' or token.scope.type == 'Enum' or \
            token.scope.type == 'Class' or token.scope.type == 'Global':
                continue
            if token.astParent and token.astParent.str in ['(', ',', '{']:
                continue
            self.reportError(token, 12, 3)


    def misra_12_4(self, data):
        if typeBits['INT'] == 16:
            max_uint = 0xffff
        elif typeBits['INT'] == 32:
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
                    self.reportError(token, 12, 4)
                    break


    def misra_13_1(self, data):
        for token in data.tokenlist:
            if not simpleMatch(token, '= {'):
                continue
            init = token.next
            if hasSideEffectsRecursive(init):
                self.reportError(init, 13, 1)


    def misra_13_3(self, data):
        for token in data.tokenlist:
            if token.str not in {'++', '--'}:
                continue
            astTop = token
            while astTop.astParent and astTop.astParent.str not in {',', ';'}:
                astTop = astTop.astParent
            if countSideEffects(astTop) >= 2:
                self.reportError(astTop, 13, 3)


    def misra_13_4(self, data):
        for token in data.tokenlist:
            if token.str != '=':
                continue
            if not token.astParent:
                continue
            if token.astOperand1.str == '[' and token.astOperand1.previous.str in {'{', ','}:
                continue
            if not (token.astParent.str in [',', ';']):
                self.reportError(token, 13, 4)


    def misra_13_5(self, data):
        for token in data.tokenlist:
            if token.isLogicalOp and hasSideEffectsRecursive(token.astOperand2):
                self.reportError(token, 13, 5)


    def misra_13_6(self, data):
        for token in data.tokenlist:
            if token.str == 'sizeof' and hasSideEffectsRecursive(token.next):
                self.reportError(token, 13, 6)


    def misra_14_1(self, data):
        for token in data.tokenlist:
            if token.str == 'for':
                exprs = getForLoopExpressions(token)
                if not exprs:
                    continue
                for counter in findCounterTokens(exprs[1]):
                    if counter.valueType and counter.valueType.isFloat():
                        self.reportError(token, 14, 1)
            elif token.str == 'while':
                if isFloatCounterInWhileLoop(token):
                    self.reportError(token, 14, 1)


    def misra_14_2(self, data):
        for token in data.tokenlist:
            expressions = getForLoopExpressions(token)
            if not expressions:
                continue
            if expressions[0] and not expressions[0].isAssignmentOp:
                self.reportError(token, 14, 2)
            elif hasSideEffectsRecursive(expressions[1]):
                self.reportError(token, 14, 2)


    def misra_14_4(self, data):
        for token in data.tokenlist:
            if token.str != '(':
                continue
            if not token.astOperand1 or not (token.astOperand1.str in ['if', 'while']):
                continue
            if not isBoolExpression(token.astOperand2):
                self.reportError(token, 14, 4)


    def misra_15_1(self, data):
        for token in data.tokenlist:
            if token.str == "goto":
                self.reportError(token, 15, 1)


    def misra_15_2(self, data):
        for token in data.tokenlist:
            if token.str != 'goto':
                continue
            if (not token.next) or (not token.next.isName):
                continue
            if not findGotoLabel(token):
                self.reportError(token, 15, 2)


    def misra_15_3(self, data):
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
                self.reportError(token, 15, 3)


    def misra_15_5(self, data):
        for token in data.tokenlist:
            if token.str == 'return' and token.scope.type != 'Function':
                self.reportError(token, 15, 5)


    def misra_15_6(self, rawTokens):
        state = 0
        indent = 0
        tok1 = None
        for token in rawTokens:
            if token.str in ['if', 'for', 'while']:
                if simpleMatch(token.previous, '# if'):
                    continue
                if simpleMatch(token.previous, "} while"):
                    # is there a 'do { .. } while'?
                    start = rawlink(token.previous)
                    if start and simpleMatch(start.previous, 'do {'):
                        continue
                if state == 2:
                    self.reportError(tok1, 15, 6)
                state = 1
                indent = 0
                tok1 = token
            elif token.str == 'else':
                if simpleMatch(token.previous, '# else'):
                    continue
                if simpleMatch(token, 'else if'):
                    continue
                if state == 2:
                    self.reportError(tok1, 15, 6)
                state = 2
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
                    self.reportError(tok1, 15, 6)


    def misra_15_7(self, data):
        for scope in data.scopes:
            if scope.type != 'Else':
                continue
            if not simpleMatch(scope.bodyStart, '{ if ('):
                continue
            tok = scope.bodyStart.next.next.link
            if not simpleMatch(tok, ') {'):
                continue
            tok = tok.next.link
            if not simpleMatch(tok, '} else'):
                self.reportError(tok, 15, 7)

    # TODO add 16.1 rule


    def misra_16_2(self, data):
        for token in data.tokenlist:
            if token.str == 'case' and token.scope.type != 'Switch':
                self.reportError(token, 16, 2)


    def misra_16_3(self, rawTokens):
        STATE_NONE = 0   # default state, not in switch case/default block
        STATE_BREAK = 1  # break/comment is seen but not its ';'
        STATE_OK = 2     # a case/default is allowed (we have seen 'break;'/'comment'/'{'/attribute)
        state = STATE_NONE
        for token in rawTokens:
            if token.str == 'break' or token.str == 'return' or token.str == 'throw':
                state = STATE_BREAK
            elif token.str == ';':
                if state == STATE_BREAK:
                    state = STATE_OK
                else:
                    state = STATE_NONE
            elif token.str.startswith('/*') or token.str.startswith('//'):
                if 'fallthrough' in token.str.lower():
                    state = STATE_OK
            elif simpleMatch(token, '[ [ fallthrough ] ] ;'):
                state = STATE_BREAK
            elif token.str == '{':
                state = STATE_OK
            elif token.str == '}' and state == STATE_OK:
                # is this {} an unconditional block of code?
                link = findRawLink(token)
                if (link is None) or (link.previous is None) or (link.previous.str not in ':;{}'):
                    state = STATE_NONE
            elif token.str == 'case' or token.str == 'default':
                if state != STATE_OK:
                    self.reportError(token, 16, 3)
                state = STATE_OK


    def misra_16_4(self, data):
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
                self.reportError(token, 16, 4)


    def misra_16_5(self, data):
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
                self.reportError(token, 16, 5)


    def misra_16_6(self, data):
        for token in data.tokenlist:
            if not (simpleMatch(token, 'switch (') and simpleMatch(token.next.link, ') {')):
                continue
            tok = token.next.link.next.next
            count = 0
            while tok:
                if tok.str in ['break', 'return', 'throw']:
                    count = count + 1
                elif tok.str == '{':
                    tok = tok.link
                    if isNoReturnScope(tok):
                        count = count + 1
                elif tok.str == '}':
                    break
                tok = tok.next
            if count < 2:
                self.reportError(token, 16, 6)


    def misra_16_7(self, data):
        for token in data.tokenlist:
            if simpleMatch(token, 'switch (') and isBoolExpression(token.next.astOperand2):
                self.reportError(token, 16, 7)


    def misra_17_1(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and token.astOperand1.str in {'va_list', 'va_arg', 'va_start', 'va_end', 'va_copy'}:
                self.reportError(token, 17, 1)
            elif token.str == 'va_list':
                self.reportError(token, 17, 1)


    def misra_17_6(self, rawTokens):
        for token in rawTokens:
            if simpleMatch(token, '[ static'):
                self.reportError(token, 17, 6)


    def misra_17_8(self, data):
        for token in data.tokenlist:
            if not (token.isAssignmentOp or (token.str in {'++', '--'})):
                continue
            if not token.astOperand1:
                continue
            var = token.astOperand1.variable
            if var and var.isArgument:
                self.reportError(token, 17, 8)


    def misra_18_5(self, data):
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
                self.reportError(var.nameToken, 18, 5)


    def misra_18_8(self, data):
        for var in data.variables:
            if not var.isArray or not var.isLocal:
                continue
            # TODO Array dimensions are not available in dump, must look in tokens
            typetok = var.nameToken.next
            if not typetok or typetok.str != '[':
                continue
            if not isConstantExpression(typetok.astOperand2):
                self.reportError(var.nameToken, 18, 8)


    def misra_19_2(self, data):
        for token in data.tokenlist:
            if token.str == 'union':
                self.reportError(token, 19, 2)


    def misra_20_1(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include'):
                continue
            for token in data.tokenlist:
                if token.file != directive.file:
                    continue
                if int(token.linenr) < int(directive.linenr):
                    self.reportError(directive, 20, 1)
                    break


    def misra_20_2(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include '):
                continue
            for pattern in {'\\', '//', '/*', "'"}:
                if pattern in directive.str:
                    self.reportError(directive, 20, 2)
                    break


    def misra_20_3(self, rawTokens):
        linenr = -1
        for token in rawTokens:
            if token.str.startswith('/') or token.linenr == linenr:
                continue
            linenr = token.linenr
            if not simpleMatch(token, '# include'):
                continue
            headerToken = token.next.next
            num = 0
            while headerToken and headerToken.linenr == linenr:
                if not headerToken.str.startswith('/*') and not headerToken.str.startswith('//'):
                    num += 1
                headerToken = headerToken.next
            if num != 1:
                self.reportError(token, 20, 3)


    def misra_20_4(self, data):
        for directive in data.directives:
            res = re.search(r'#define ([a-z][a-z0-9_]+)', directive.str)
            if res and (res.group(1) in KEYWORDS):
                self.reportError(directive, 20, 4)


    def misra_20_5(self, data):
        for directive in data.directives:
            if directive.str.startswith('#undef '):
                self.reportError(directive, 20, 5)


    def misra_20_13(self, data):
        for directive in data.directives:
            dir = directive.str
            for sep in ' (<':
                if dir.find(sep) > 0:
                    dir = dir[:dir.find(sep)]
            if dir not in ['#define', '#elif', '#else', '#endif', '#error', '#if', '#ifdef', '#ifndef', '#include',
                        '#pragma', '#undef', '#warning']:
                self.reportError(directive, 20, 13)


    def misra_20_14(self, data):
        # stack for #if blocks. contains the #if directive until the corresponding #endif is seen.
        # the size increases when there are inner #if directives.
        ifStack = []
        for directive in data.directives:
            if directive.str.startswith('#if ') or directive.str.startswith('#ifdef ') or directive.str.startswith('#ifndef '):
                ifStack.append(directive)
            elif directive.str == '#else' or directive.str.startswith('#elif '):
                if len(ifStack) == 0:
                    self.reportError(directive, 20, 14)
                    ifStack.append(directive)
                elif directive.file != ifStack[-1].file:
                    self.reportError(directive, 20, 14)
            elif directive.str == '#endif':
                if len(ifStack) == 0:
                    self.reportError(directive, 20, 14)
                elif directive.file != ifStack[-1].file:
                    self.reportError(directive, 20, 14)
                    ifStack.pop()


    def misra_21_3(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and (token.astOperand1.str in {'malloc', 'calloc', 'realloc', 'free'}):
                self.reportError(token, 21, 3)


    def misra_21_4(self, data):
        directive = findInclude(data.directives, '<setjmp.h>')
        if directive:
            self.reportError(directive, 21, 4)


    def misra_21_5(self, data):
        directive = findInclude(data.directives, '<signal.h>')
        if directive:
            self.reportError(directive, 21, 5)


    def misra_21_6(self, data):
        dir_stdio = findInclude(data.directives, '<stdio.h>')
        dir_wchar = findInclude(data.directives, '<wchar.h>')
        if dir_stdio:
            self.reportError(dir_stdio, 21, 6)
        if dir_wchar:
            self.reportError(dir_wchar, 21, 6)


    def misra_21_7(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and (token.astOperand1.str in {'atof', 'atoi', 'atol', 'atoll'}):
                self.reportError(token, 21, 7)


    def misra_21_8(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and (token.astOperand1.str in {'abort', 'exit', 'getenv', 'system'}):
                self.reportError(token, 21, 8)


    def misra_21_9(self, data):
        for token in data.tokenlist:
            if (token.str in {'bsearch', 'qsort'}) and token.next and token.next.str == '(':
                self.reportError(token, 21, 9)


    def misra_21_10(self, data):
        directive = findInclude(data.directives, '<time.h>')
        if directive:
            self.reportError(directive, 21, 10)

        for token in data.tokenlist:
            if (token.str == 'wcsftime') and token.next and token.next.str == '(':
                self.reportError(token, 21, 10)


    def misra_21_11(self, data):
        directive = findInclude(data.directives, '<tgmath.h>')
        if directive:
            self.reportError(directive, 21, 11)


    def get_verify_expected(self):
        """Return the list of expected violations in the verify test"""
        return self.verify_expected


    def get_verify_actual(self):
        """Return the list of actual violations in for the verify test"""
        return self.verify_actual


    def get_violations(self):
        """Return the list of violations for a normal checker run"""
        return self.violations


    def addSuppressedRule(self, ruleNum,
                          fileName   = None,
                          lineNumber = None,
                          symbolName = None):
        """
        Add a suppression to the suppressions data structure

        Suppressions are stored in a dictionary of dictionaries that
        contains a list of tuples.

        The first dictionary is keyed by the MISRA rule in hundreds
        format. The value of that dictionary is a dictionary of filenames.
        If the value is None then the rule is assumed to be suppressed for
        all files.
        If the filename exists then the value of that dictionary contains a list
        with the scope of the suppression.  If the list contains an item of None
        then the rule is assumed to be suppresed for the entire file. Otherwise
        the list contains line number, symbol name tuples.
        For each tuple either line number or symbol name can can be none.

        """
        normalized_filename = None

        if fileName is not None:
            normalized_filename = os.path.normpath(fileName)

        if lineNumber is not None or symbolName is not None:
            line_symbol = (lineNumber, symbolName)
        else:
            line_symbol = None

        # If the rule is not in the dict already then add it
        if not ruleNum in self.suppressedRules:
            ruleItemList = list()
            ruleItemList.append(line_symbol)

            fileDict = dict()
            fileDict[normalized_filename] = ruleItemList

            self.suppressedRules[ruleNum] = fileDict

            # Rule is added.  Done.
            return

        # Rule existed in the dictionary. Check for
        # filename entries.

        # Get the dictionary for the rule numer
        fileDict = self.suppressedRules[ruleNum]

        # If the filename is not in the dict already add it
        if not normalized_filename in fileDict:
            ruleItemList = list()
            ruleItemList.append(line_symbol)

            fileDict[normalized_filename] = ruleItemList

            # Rule is added with a file scope. Done
            return

        # Rule has a matching filename. Get the rule item list.

        # Check the lists of rule items
        # to see if this (lineNumber, symbonName) combination
        # or None already exists.
        ruleItemList = fileDict[normalized_filename]

        if line_symbol is None:
            # is it already in the list?
            if not line_symbol in ruleItemList:
                ruleItemList.append(line_symbol)
        else:
            # Check the list looking for matches
            matched = False
            for each in ruleItemList:
                if each is not None:
                    if (each[0] == line_symbol[0]) and (each[1] == line_symbol[1]):
                        matched = True

            # Append the rule item if it was not already found
            if not matched:
                ruleItemList.append(line_symbol)


    def isRuleSuppressed(self, location, ruleNum):
        """
        Check to see if a rule is suppressed.
        ruleNum is the rule number in hundreds format

        If the rule exists in the dict then check for a filename
        If the filename is None then rule is suppressed globally
        for all files.
        If the filename exists then look for list of
        line number, symbol name tuples.  If the list is None then
        the rule is suppressed for the entire file
        If the list of tuples exists then search the list looking for
        matching line numbers.  Symbol names are currently ignored
        because they can include regular expressions.
        TODO: Support symbol names and expression matching.

        """
        ruleIsSuppressed = False
        linenr = location.linenr

        # Remove any prefix listed in command arguments from the filename.
        filename = None
        if location.file is not None:
            if self.filePrefix is not None:
                filename = remove_file_prefix(location.file, self.filePrefix)
            else:
                filename = location.file

        if ruleNum in self.suppressedRules:
            fileDict = self.suppressedRules[ruleNum]

            # a file name entry of None means that the rule is suppressed
            # globally
            if None in fileDict:
                ruleIsSuppressed = True
            else:
                # Does the filename match one of the names in
                # the file list
                if filename in fileDict:
                    # Get the list of ruleItems
                    ruleItemList = fileDict[filename]

                    if None in ruleItemList:
                        # Entry of None in the ruleItemList means the rule is
                        # suppressed for all lines in the filename
                        ruleIsSuppressed = True
                    else:
                        # Iterate though the the list of line numbers
                        # and symbols looking for a match of the line
                        # number.  Matching the symbol is a TODO:
                        for each in ruleItemList:
                            if each is not None:
                                if each[0] == linenr:
                                    ruleIsSuppressed = True

        return ruleIsSuppressed


    def parseSuppressions(self):
        """
        Parse the suppression list provided by cppcheck looking for
        rules that start with 'misra' or MISRA.  The MISRA rule number
        follows using either '_' or '.' to separate the numbers.
        Examples:
            misra_6.0
            misra_7_0
            misra.21.11
        """
        rule_pattern = re.compile(r'^(misra|MISRA)[_.]([0-9]+)[_.]([0-9]+)')

        for each in self.dumpfileSuppressions:
            res = rule_pattern.match(each.errorId)

            if res:
                num1 = int(res.group(2)) * 100
                ruleNum = num1 + int(res.group(3))
                self.addSuppressedRule(ruleNum, each.fileName,
                                       each.lineNumber, each.symbolName)


    def showSuppressedRules(self):
            """
            Print out rules in suppression list sorted by Rule Number
            """
            print("Suppressed Rules List:")
            outlist = list()

            for ruleNum in self.suppressedRules:
                fileDict = self.suppressedRules[ruleNum]

                for fname in fileDict:
                    ruleItemList = fileDict[fname]

                    for item in ruleItemList:
                        if item is None:
                            item_str = "None"
                        else:
                            item_str = str(item[0])

                        outlist.append("%s: %s: %s" % (float(ruleNum)/100,fname,item_str))

            for line in sorted(outlist, reverse=True):
                print("  %s" % line)


    def setFilePrefix(self, prefix):
        """
        Set the file prefix to ignnore from files when matching
        supression files
        """
        self.filePrefix = prefix


    def setSuppressionList(self, suppressionlist):
        num1 = 0
        num2 = 0
        rule_pattern = re.compile(r'([0-9]+).([0-9]+)')
        strlist = suppressionlist.split(",")

        # build ignore list
        for item in strlist:
            res = rule_pattern.match(item)
            if res:
                num1 = int(res.group(1))
                num2 = int(res.group(2))
                ruleNum = (num1*100)+num2

                self.addSuppressedRule(ruleNum)


    def reportError(self, location, num1, num2):
        ruleNum = num1 * 100 + num2

        if VERIFY:
            self.verify_actual.append(str(location.linenr) + ':' + str(num1) + '.' + str(num2))
        elif self.isRuleSuppressed(location, ruleNum):
            # Error is suppressed. Ignore
            return
        else:
            id = 'misra-c2012-' + str(num1) + '.' + str(num2)
            if ruleNum in self.ruleTexts:
                errmsg = self.ruleTexts[ruleNum] + ' [' + id + ']'
            elif len(self.ruleTexts) == 0:
                errmsg = 'misra violation (use --rule-texts=<file> to get proper output) [' + id + ']'
            else:
                return
            formattedMsg = cppcheckdata.reportError(args.template,
                                                    callstack=[(location.file, location.linenr)],
                                                    severity='style',
                                                    message = errmsg,
                                                    errorId = id,
                                                    suppressions = self.dumpfileSuppressions)
            if formattedMsg:
                sys.stderr.write(formattedMsg)
                sys.stderr.write('\n')
                self.violations.append(errmsg)


    def loadRuleTexts(self, filename):
        num1 = 0
        num2 = 0
        appendixA = False
        ruleText = False

        Rule_pattern = re.compile(r'^Rule ([0-9]+).([0-9]+)')
        Choice_pattern = re.compile(r'^[ ]*(Advisory|Required|Mandatory)$')
        xA_Z_pattern = re.compile(r'^[#A-Z].*')
        a_z_pattern = re.compile(r'^[a-z].*')
        for line in open(filename, 'rt'):
            line = line.replace('\r', '').replace('\n', '')
            if len(line) == 0:
                if ruleText:
                    num1 = 0
                    num2 = 0
                ruleText = False
                continue
            if not appendixA:
                if line.find('Appendix A') >= 0 and line.find('Summary of guidelines') >= 10:
                    appendixA = True
                continue
            if line.find('Appendix B') >= 0:
                break
            res = Rule_pattern.match(line)
            if res:
                num1 = int(res.group(1))
                num2 = int(res.group(2))
                ruleText = False
                continue
            if Choice_pattern.match(line):
                ruleText = False
            elif xA_Z_pattern.match(line):
                if ruleText:
                    num2 = num2 + 1
                num = num1 * 100 + num2
                self.ruleTexts[num] = line
                ruleText = True
            elif ruleText and a_z_pattern.match(line):
                num = num1 * 100 + num2
                self.ruleTexts[num] = self.ruleTexts[num] + ' ' + line
                continue


    def parseDump(self, dumpfile):

        data = cppcheckdata.parsedump(dumpfile)

        self.dumpfileSuppressions = data.suppressions
        self.parseSuppressions()

        typeBits['CHAR'] = data.platform.char_bit
        typeBits['SHORT'] = data.platform.short_bit
        typeBits['INT'] = data.platform.int_bit
        typeBits['LONG'] = data.platform.long_bit
        typeBits['LONG_LONG'] = data.platform.long_long_bit
        typeBits['POINTER'] = data.platform.pointer_bit

        if VERIFY:
            for tok in data.rawTokens:
                if tok.str.startswith('//') and 'TODO' not in tok.str:
                    compiled = re.compile(r'[0-9]+\.[0-9]+')
                    for word in tok.str[2:].split(' '):
                        if compiled.match(word):
                            self.verify_expected.append(str(tok.linenr) + ':' + word)
        else:
            printStatus('Checking ' + dumpfile + '...')

        cfgNumber = 0

        for cfg in data.configurations:
            cfgNumber = cfgNumber + 1
            if len(data.configurations) > 1:
                printStatus('Checking ' + dumpfile + ', config "' + cfg.name + '"...')

            if cfgNumber == 1:
                self.misra_3_1(data.rawTokens)
                self.misra_4_1(data.rawTokens)
            self.misra_5_1(cfg)
            self.misra_5_2(cfg)
            self.misra_5_3(cfg)
            self.misra_5_4(cfg)
            self.misra_5_5(cfg)
            # 6.1 require updates in Cppcheck (type info for bitfields are lost)
            # 6.2 require updates in Cppcheck (type info for bitfields are lost)
            if cfgNumber == 1:
                self.misra_7_1(data.rawTokens)
                self.misra_7_3(data.rawTokens)
            self.misra_8_11(cfg)
            self.misra_8_12(cfg)
            if cfgNumber == 1:
                self.misra_8_14(data.rawTokens)
                self.misra_9_5(data.rawTokens)
            self.misra_10_1(cfg)
            self.misra_10_4(cfg)
            self.misra_10_6(cfg)
            self.misra_10_8(cfg)
            self.misra_11_3(cfg)
            self.misra_11_4(cfg)
            self.misra_11_5(cfg)
            self.misra_11_6(cfg)
            self.misra_11_7(cfg)
            self.misra_11_8(cfg)
            self.misra_11_9(cfg)
            if cfgNumber == 1:
                self.misra_12_1_sizeof(data.rawTokens)
            self.misra_12_1(cfg)
            self.misra_12_2(cfg)
            self.misra_12_3(cfg)
            self.misra_12_4(cfg)
            self.misra_13_1(cfg)
            self.misra_13_3(cfg)
            self.misra_13_4(cfg)
            self.misra_13_5(cfg)
            self.misra_13_6(cfg)
            self.misra_14_1(cfg)
            self.misra_14_2(cfg)
            self.misra_14_4(cfg)
            self.misra_15_1(cfg)
            self.misra_15_2(cfg)
            self.misra_15_3(cfg)
            self.misra_15_5(cfg)
            if cfgNumber == 1:
                self.misra_15_6(data.rawTokens)
            self.misra_15_7(cfg)
            self.misra_16_2(cfg)
            if cfgNumber == 1:
                self.misra_16_3(data.rawTokens)
            self.misra_16_4(cfg)
            self.misra_16_5(cfg)
            self.misra_16_6(cfg)
            self.misra_16_7(cfg)
            self.misra_17_1(cfg)
            if cfgNumber == 1:
                self.misra_17_6(data.rawTokens)
            self.misra_17_8(cfg)
            self.misra_18_5(cfg)
            self.misra_18_8(cfg)
            self.misra_19_2(cfg)
            self.misra_20_1(cfg)
            self.misra_20_2(cfg)
            if cfgNumber == 1:
                self.misra_20_3(data.rawTokens)
            self.misra_20_4(cfg)
            self.misra_20_5(cfg)
            self.misra_20_13(cfg)
            self.misra_20_14(cfg)
            self.misra_21_3(cfg)
            self.misra_21_4(cfg)
            self.misra_21_5(cfg)
            self.misra_21_6(cfg)
            self.misra_21_7(cfg)
            self.misra_21_8(cfg)
            self.misra_21_9(cfg)
            self.misra_21_10(cfg)
            self.misra_21_11(cfg)
            # 22.4 is already covered by Cppcheck writeReadOnlyFile



RULE_TEXTS_HELP = '''Path to text file of MISRA rules

If you have the tool 'pdftotext' you might be able
to generate this textfile with such command:

    pdftotext MISRA_C_2012.pdf MISRA_C_2012.txt

Otherwise you can more or less copy/paste the chapter
Appendix A Summary of guidelines
from the MISRA pdf. You can buy the MISRA pdf from
http://www.misra.org.uk/

Format:

<..arbitrary text..>
Appendix A Summary of guidelines
Rule 1.1
Rule text for 1.1
Rule 1.2
Rule text for 1.2
<...>

'''

SUPPRESS_RULES_HELP = '''MISRA rules to suppress (comma-separated)

For example, if you'd like to suppress rules 15.1, 11.3,
and 20.13, run:

    python misra.py --suppress-rules 15.1,11.3,20.13 ...

'''

parser = cppcheckdata.ArgumentParser()
parser.add_argument("--rule-texts", type=str, help=RULE_TEXTS_HELP)
parser.add_argument("--suppress-rules", type=str, help=SUPPRESS_RULES_HELP)
parser.add_argument("--quiet", help="Only print something when there is an error", action="store_true")
parser.add_argument("--no-summary", help="Hide summary of violations", action="store_true")
parser.add_argument("-verify", help=argparse.SUPPRESS, action="store_true")
parser.add_argument("-generate-table", help=argparse.SUPPRESS, action="store_true")
parser.add_argument("dumpfile", nargs='*', help="Path of dump file from cppcheck")
parser.add_argument("--show-suppressed-rules", help="Print rule suppression list", action="store_true")
parser.add_argument("-P", "--file-prefix", type=str, help="Prefix to strip when matching suppression file rules")
args = parser.parse_args()

checker = MisraChecker()

if args.generate_table:
    generateTable()
else:
    if args.verify:
        VERIFY = True
    if args.rule_texts:
        filename = os.path.normpath(args.rule_texts)
        if not os.path.isfile(filename):
            print('Fatal error: file is not found: ' + filename)
            sys.exit(1)
        checker.loadRuleTexts(filename)

    if args.suppress_rules:
        checker.setSuppressionList(args.suppress_rules)

    if args.file_prefix:
        checker.setFilePrefix(args.file_prefix)

    if args.quiet:
        QUIET = True
    if args.no_summary:
        SHOW_SUMMARY = False
    if args.dumpfile:
        exitCode = 0
        for item in args.dumpfile:
            checker.parseDump(item)

            if VERIFY:
                verify_expected = checker.get_verify_expected()
                verify_actual   = checker.get_verify_actual()

                for expected in verify_expected:
                    if expected not in verify_actual:
                        print('Expected but not seen: ' + expected)
                        exitCode = 1
                for actual in verify_actual:
                    if actual not in verify_expected:
                        print('Not expected: ' + actual)
                        exitCode = 1

                # Exisitng behavior of verify mode is to exit
                # on the first un-expected output.
                # TODO: Is this required? or can it be moved to after
                # all input files have been processed
                if exitCode != 0:
                    sys.exit(exitCode)

        # Under normal operation exit with a non-zero exit code
        # if there were any violations.
        if not VERIFY:
            number_of_violations = len(checker.get_violations())
            if number_of_violations > 0:
                exitCode = 1

                if SHOW_SUMMARY:
                    print("\nMISRA rule violations found: %d\n" % (number_of_violations))

        if args.show_suppressed_rules:
            checker.showSuppressedRules()

        sys.exit(exitCode)
