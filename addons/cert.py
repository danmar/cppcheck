#!/usr/bin/env python3
#
# Cert: Some extra CERT checkers
#
# Cppcheck itself handles many CERT rules. Cppcheck warns when there is undefined behaviour.
# CERT Homepage: https://www.cert.org/secure-coding/
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python cert.py main.cpp.dump

import argparse
import cppcheckdata
import sys
import re

VERIFY = ('-verify' in sys.argv)
VERIFY_EXPECTED = []
VERIFY_ACTUAL = []

def reportError(token, severity, msg, id):
    if VERIFY:
        VERIFY_ACTUAL.append(str(token.linenr) + ':cert-' + id)
    else:
        cppcheckdata.reportError(token, severity, msg, 'cert', id)

def simpleMatch(token, pattern):
    return cppcheckdata.simpleMatch(token, pattern)


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

# Is this a function call
def isFunctionCall(token, function_names, number_of_arguments=None):
    if not token.isName:
        return False
    if token.str not in function_names:
        return False
    if (token.next is None) or token.next.str != '(' or token.next != token.astParent:
        return False
    if number_of_arguments is None:
        return True
    return len(cppcheckdata.getArguments(token)) == number_of_arguments


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
                reportError(token, 'style', "Attempt to cast away const", 'EXP05-C')

        elif token.str == '(' and token.astOperand1 and token.astOperand2 and token.astOperand1.function:
            function = token.astOperand1.function
            arguments = cppcheckdata.getArguments(token.previous)
            if not arguments:
                continue
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
                    reportError(token, 'style', "Attempt to cast away const", 'EXP05-C')


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
                "(fix either by packing the struct using '#pragma pack' or by rewriting the comparison)", 'EXP42-C')

# EXP15-C
# Do not place a semicolon on the same line as an if, for or while statement
def exp15(data):
    for scope in data.scopes:
        if scope.type in ('If', 'For', 'While'):
            token = scope.bodyStart.next
            if token.str==';' and token.linenr==scope.bodyStart.linenr:
                reportError(token, 'style', 'Do not place a semicolon on the same line as an IF, FOR or WHILE', 'EXP15-C')


# EXP46-C
# Do not use a bitwise operator with a Boolean-like operand
#   int x = (a == b) & c;
def exp46(data):
    for token in data.tokenlist:
        if isBitwiseOp(token) and (isComparisonOp(token.astOperand1) or isComparisonOp(token.astOperand2)):
            reportError(
                token, 'style', 'Bitwise operator is used with a Boolean-like operand', 'EXP46-c')

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
                        'INT31-c')
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
                    'INT31-c')
                break


# ENV33-C
# Do not call system()
def env33(data):
    for token in data.tokenlist:
        if isFunctionCall(token, ('system',), 1):

            # Invalid syntax
            if not token.next.astOperand2:
                continue

            # ENV33-C-EX1: It is permissible to call system() with a null
            # pointer argument to determine the presence of a command processor
            # for the system.
            argValue = token.next.astOperand2.getValue(0)
            if argValue and argValue.intvalue == 0 and argValue.isKnown():
                continue

            reportError(token, 'style', 'Do not call system()', 'ENV33-C')


# MSC24-C
# Do not use deprecated or obsolescent functions
def msc24(data):
    for token in data.tokenlist:
        if isFunctionCall(token, ('asctime',), 1):
            reportError(token,'style','Do not use asctime() better use asctime_s()', 'MSC24-C')
        elif isFunctionCall(token, ('atof',), 1):
            reportError(token,'style','Do not use atof() better use strtod()', 'MSC24-C')
        elif isFunctionCall(token, ('atoi',), 1):
            reportError(token,'style','Do not use atoi() better use strtol()', 'MSC24-C')
        elif isFunctionCall(token, ('atol',), 1):
            reportError(token,'style','Do not use atol() better use strtol()', 'MSC24-C')
        elif isFunctionCall(token, ('atoll',), 1):
            reportError(token,'style','Do not use atoll() better use strtoll()', 'MSC24-C')
        elif isFunctionCall(token, ('ctime',), 1):
            reportError(token,'style','Do not use ctime() better use ctime_s()', 'MSC24-C')
        elif isFunctionCall(token, ('fopen',), 2):
            reportError(token,'style','Do not use fopen() better use fopen_s()', 'MSC24-C')
        elif isFunctionCall(token, ('freopen',), 3):
            reportError(token,'style','Do not use freopen() better use freopen_s()', 'MSC24-C')
        elif isFunctionCall(token, ('rewind',), 1):
            reportError(token,'style','Do not use rewind() better use fseek()', 'MSC24-C')
        elif isFunctionCall(token, ('setbuf',), 2):
            reportError(token,'style','Do not use setbuf() better use setvbuf()', 'MSC24-C')

# MSC30-C
# Do not use the rand() function for generating pseudorandom numbers
def msc30(data):
    for token in data.tokenlist:
        if simpleMatch(token, "rand ( )") and isStandardFunction(token):
            reportError(token, 'style', 'Do not use the rand() function for generating pseudorandom numbers', 'MSC30-c')

# STR03-C
# Do not inadvertently truncate a string
def str03(data):
    for token in data.tokenlist:
        if not isFunctionCall(token, 'strncpy'):
            continue
        arguments = cppcheckdata.getArguments(token)
        if len(arguments)!=3:
            continue
        if arguments[2].str=='(' and arguments[2].astOperand1.str=='sizeof':
            reportError(token, 'style', 'Do not inadvertently truncate a string', 'STR03-C')

# STR05-C
# Use pointers to const when referring to string literals
def str05(data):
    for token in data.tokenlist:
        if token.isString:
            parent = token.astParent
            if parent is None:
                continue
            parentOp1 = parent.astOperand1
            if parent.isAssignmentOp and parentOp1.valueType:
                if (parentOp1.valueType.type in ('char', 'wchar_t')) and parentOp1.valueType.pointer and not parentOp1.valueType.constness:
                    reportError(parentOp1, 'style', 'Use pointers to const when referring to string literals', 'STR05-C')

# STR07-C
# Use the bounds-checking interfaces for string manipulation
def str07(data):
    if data.standards.c=='c89' or data.standards.c=='c99':
        return
    for token in data.tokenlist:
        if not isFunctionCall(token, ('strcpy', 'strcat')):
            continue
        args = cppcheckdata.getArguments(token)
        if len(args)!=2:
            continue
        if args[1].isString:
            continue
        reportError(token, 'style', 'Use the bounds-checking interfaces %s_s()' % token.str, 'STR07-C')

# STR11-C
# Do not specify the bound of a character array initialized with a string literal
def str11(data):
    for token in data.tokenlist:
        if not token.isString:
            continue

        strlen = token.strlen
        parent = token.astParent

        if parent is None:
            continue
        parentOp1 = parent.astOperand1
        if parentOp1 is None or parentOp1.str!='[':
            continue

        if not parent.isAssignmentOp:
            continue

        varToken = parentOp1.astOperand1
        if varToken is None or not varToken.isName:
            continue
        if varToken.variable is None:
            continue
        if varToken != varToken.variable.nameToken:
            continue
        valueToken = parentOp1.astOperand2
        if valueToken is None:
            continue

        if valueToken.isNumber and int(valueToken.str)==strlen:
            reportError(valueToken, 'style', 'Do not specify the bound of a character array initialized with a string literal', 'STR11-C')

# API01-C
# Avoid laying out strings in memory directly before sensitive data
def api01(data):
    for scope in data.scopes:
        if scope.type!='Struct':
            continue
        token = scope.bodyStart
        arrayFound=False
        # loop through the complete struct
        while token != scope.bodyEnd:
            if token.isName and token.variable:
                if token.variable.isArray:
                    arrayFound=True
                elif arrayFound and not token.variable.isArray and not token.variable.isConst:
                    reportError(token, 'style', 'Avoid laying out strings in memory directly before sensitive data', 'API01-C')
                    # reset flags to report other positions in the same struct
                    arrayFound=False
            token = token.next


def get_args_parser():
    parser = cppcheckdata.ArgumentParser()
    parser.add_argument("-verify", help=argparse.SUPPRESS, action="store_true")
    return parser

if __name__ == '__main__':
    parser = get_args_parser()
    args = parser.parse_args()

    if args.verify:
        VERIFY = True

    if not args.dumpfile:
        if not args.quiet:
            print("no input files.")
        sys.exit(0)

    for dumpfile in args.dumpfile:
        if not args.quiet:
            print('Checking %s...' % dumpfile)

        data = cppcheckdata.CppcheckData(dumpfile)

        if VERIFY:
            VERIFY_ACTUAL = []
            VERIFY_EXPECTED = []
            for tok in data.rawTokens:
                if tok.str.startswith('//') and 'TODO' not in tok.str:
                    for word in tok.str[2:].split(' '):
                        if re.match(r'cert-[A-Z][A-Z][A-Z][0-9][0-9].*',word):
                            VERIFY_EXPECTED.append(str(tok.linenr) + ':' + word)

        for cfg in data.iterconfigurations():
            if not args.quiet:
                print('Checking %s, config %s...' % (dumpfile, cfg.name))
            exp05(cfg)
            exp42(cfg)
            exp46(cfg)
            exp15(cfg)
            int31(cfg, data.platform)
            str03(cfg)
            str05(cfg)
            str07(cfg)
            str11(cfg)
            env33(cfg)
            msc24(cfg)
            msc30(cfg)
            api01(cfg)

        if VERIFY:
            for expected in VERIFY_EXPECTED:
                if expected not in VERIFY_ACTUAL:
                    print('Expected but not seen: ' + expected)
                    sys.exit(1)
            for actual in VERIFY_ACTUAL:
                if actual not in VERIFY_EXPECTED:
                    print('Not expected: ' + actual)
                    sys.exit(1)

    sys.exit(cppcheckdata.EXIT_CODE)
