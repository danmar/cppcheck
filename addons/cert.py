#!/usr/bin/env python
#
# Cert: Some extra CERT checkers
#
# Cppcheck itself handles many CERT rules. Cppcheck warns when there is undefined behaviour.
# CERT Homepage: https://www.cert.org/secure-coding/
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python cert.py main.cpp.dump

import cppcheckdata
import sys
import re

VERIFY = ('-verify' in sys.argv)
VERIFY_EXPECTED = []
VERIFY_ACTUAL = []

def reportError(token, severity, msg, id):
    if VERIFY:
        VERIFY_ACTUAL.append(str(token.linenr) + ':' + id)
    else:
        sys.stderr.write(
            '[' + token.file + ':' + str(token.linenr) + '] (' + severity + '): ' + msg + ' [' + id + ']\n')

def simpleMatch(token, pattern):
    for p in pattern.split(' '):
        if not token or token.str != p:
            return False
        token = token.next
    return True

def isUnpackedStruct(token):
    if token.valueType is None:
        return False
    if token.valueType.typeScope is None:
        return False
    if token.valueType.typeScope.type != "Struct":
        return False
    startToken = token.valueType.typeScope.bodyStart

    linenr = int(startToken.linenr)
    for line in open(startToken.file):
        linenr -= 1
        if linenr == 0:
            return True
        if linenr < 3 and re.match(r'#pragma\s+pack\s*\(', line):
            return False
    return True


def isLocalUnpackedStruct(arg):
    if arg and arg.str == '&' and not arg.astOperand2:
        arg = arg.astOperand1
    return arg and arg.variable and (arg.variable.isLocal or arg.variable.isArgument) and isUnpackedStruct(arg)


def isBitwiseOp(token):
    return token and (token.str in {'&', '|', '^'})


def isComparisonOp(token):
    return token and (token.str in {'==', '!=', '>', '>=', '<', '<='})

def isCast(expr):
    if not expr or expr.str != '(' or not expr.astOperand1 or expr.astOperand2:
        return False
    if simpleMatch(expr, '( )'):
        return False
    return True

def isStandardFunction(token):
    if token.function:
        return False
    prev = token.previous
    if prev:
        if prev.str == '.':
            return False
        if prev.str == '::':
            prevprev = prev.previous
            if prevprev and not prevprev.str == 'std':
                return False
    return True

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

# EXP05-C
# do not attempt to cast away const
def exp05(data):
    # TODO Reuse code in misra rule 11.8
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
                reportError(token, 'style', "Attempt to cast away const", 'cert-EXP05-C')

        elif token.str == '(' and token.astOperand1 and token.astOperand2 and token.astOperand1.function:
            function = token.astOperand1.function
            arguments = getArguments(token)
            for argnr, argvar in function.argument.items():
                if argnr < 1 or argnr > len(arguments):
                    continue
                if not argvar.isPointer:
                    continue
                if (argvar.constness % 2) == 1: # data is const
                    continue
                argtok = arguments[argnr - 1]
                if not argtok.valueType:
                    continue
                if argtok.valueType.pointer == 0:
                    continue
                const2 = arguments[argnr - 1].valueType.constness
                if (const2 % 2) == 1:
                    reportError(token, 'style', "Attempt to cast away const", 'cert-EXP05-C')


# EXP42-C
# do not compare padding data
def exp42(data):
    for token in data.tokenlist:
        if token.str != '(' or not token.astOperand1 or token.astOperand1.str != 'memcmp':
            continue

        arg1 = None
        arg2 = None
        if token.astOperand2 and token.astOperand2.str == ',':
            if token.astOperand2.astOperand1 and token.astOperand2.astOperand1.str == ',':
                arg1 = token.astOperand2.astOperand1.astOperand1
                arg2 = token.astOperand2.astOperand1.astOperand2

        if isLocalUnpackedStruct(arg1) or isLocalUnpackedStruct(arg2):
            reportError(
                token, 'style', "Comparison of struct padding data " +
                "(fix either by packing the struct using '#pragma pack' or by rewriting the comparison)", 'cert-EXP42-C')


# EXP46-C
# Do not use a bitwise operator with a Boolean-like operand
#   int x = (a == b) & c;
def exp46(data):
    for token in data.tokenlist:
        if isBitwiseOp(token) and (isComparisonOp(token.astOperand1) or isComparisonOp(token.astOperand2)):
            reportError(
                token, 'style', 'Bitwise operator is used with a Boolean-like operand', 'cert-EXP46-c')

# INT31-C
# Ensure that integer conversions do not result in lost or misinterpreted data
def int31(data, platform):
    if not platform:
        return
    for token in data.tokenlist:
        if not isCast(token):
            continue
        if not token.valueType or not token.astOperand1.values:
            continue
        bits = None
        if token.valueType.type == 'char':
            bits = platform.char_bit
        elif token.valueType.type == 'short':
            bits = platform.short_bit
        elif token.valueType.type == 'int':
            bits = platform.int_bit
        elif token.valueType.type == 'long':
            bits = platform.long_bit
        elif token.valueType.type == 'long long':
            bits = platform.long_long_bit
        else:
            continue
        if token.valueType.sign == 'unsigned':
            found = False
            for value in token.astOperand1.values:
                if value.intvalue and value.intvalue < 0:
                    found = True
                    reportError(
                        token,
                        'style',
                        'Ensure that integer conversions do not result in lost or misinterpreted data (casting ' + str(value.intvalue) + ' to unsigned ' + token.valueType.type + ')',
                        'cert-INT31-c')
                break
            if found:
                continue
        if bits >= 64:
            continue
        minval = 0
        maxval = 1
        if token.valueType.sign == 'signed':
            minval = -(1 << (bits - 1))
            maxval = ((1 << (bits - 1)) - 1)
        else:
            minval = 0
            maxval = ((1 << bits) - 1)
        for value in token.astOperand1.values:
            if value.intvalue and (value.intvalue < minval or value.intvalue > maxval):
                destType = ''
                if token.valueType.sign:
                    destType = token.valueType.sign + ' ' + token.valueType.type
                else:
                    destType = token.valueType.type
                reportError(
                    token,
                    'style',
                    'Ensure that integer conversions do not result in lost or misinterpreted data (casting ' + str(value.intvalue) + ' to ' + destType + ')',
                    'cert-INT31-c')
                break

# MSC30-C
# Do not use the rand() function for generating pseudorandom numbers
def msc30(data):
    for token in data.tokenlist:
        if simpleMatch(token, "rand ( )") and isStandardFunction(token):
            reportError(token, 'style', 'Do not use the rand() function for generating pseudorandom numbers', 'cert-MSC30-c')

for arg in sys.argv[1:]:
    if arg == '-verify':
        VERIFY = True
        continue
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)

    if VERIFY:
        VERIFY_ACTUAL = []
        VERIFY_EXPECTED = []
        for tok in data.rawTokens:
            if tok.str.startswith('//') and 'TODO' not in tok.str:
                for word in tok.str[2:].split(' '):
                    if re.match(r'cert-[A-Z][A-Z][A-Z][0-9][0-9].*',word):
                        VERIFY_EXPECTED.append(str(tok.linenr) + ':' + word)

    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        exp05(cfg)
        exp42(cfg)
        exp46(cfg)
        int31(cfg, data.platform)
        msc30(cfg)

    if VERIFY:
        for expected in VERIFY_EXPECTED:
            if expected not in VERIFY_ACTUAL:
                print('Expected but not seen: ' + expected)
                sys.exit(1)
        for actual in VERIFY_ACTUAL:
            if actual not in VERIFY_EXPECTED:
                print('Not expected: ' + actual)
                sys.exit(1)
