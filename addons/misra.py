#/usr/bin/python
#
# MISRA checkers
#
# The goal is that this addon will check conformance against latest MISRA C standard.
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

def isPrimaryExpression(expr):
    return expr and (expr.isName or expr.isNumber or (expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||']))



# 1
# STATUS: Use compiler
def misra1(data):
    return

# 2
# STATUS: ?
def misra2(data):
    return

# 3
# STATUS: Done
def misra3check(token):
    prev = token.previous
    if not prev or prev.str != '{':
        return True
    prev = prev.previous
    if not prev or prev.str != ')':
        return True
    prev = prev.link
    if not prev or prev.str != '(':
        return True
    prev = prev.link
    if not prev or not prev.isName or (prev in ['for','if','switch','while']):
        return True
    after = token.next
    if not after or after.str != '(':
        return True
    after = after.link
    if not after or after.str != ')':
        return True
    after = after.next
    if not after or after.str != ';':
        return True
    after = after.next
    if not after or after.str != '}':
        return True
    return False

def misra3(data):
    for token in data.tokenlist:
        if token.str != 'asm':
            continue
        if misra3check(token):
            reportError(token, 3)

# 4
# STATUS: Done. Checked by Cppcheck
def misra4(data):
    return

# 5
# STATUS: TODO
def misra5(data):
    return

# 6
# STATUS: TODO
def misra6(data):
    return

# 7
# STATUS: TODO - requires some info from preprocessor
def misra7(data):
    return

# 8
# STATUS: TODO
def misra8(data):
    return

# 9
# STATUS: TODO
def misra9(data):
    return

# 10
# STATUS: TODO (Parse comments?)
def misra10(data):
    return

# Identifiers
# -----------

# 11
# STATUS: Done.
def misra11(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 11)

# 12
# STATUS: Done. Use compiler (-Wshadow etc)
def misra12(data):
    return

# 13
# STATUS: TODO originalName is missing right now
def misra13(data):
    for token in data.tokenlist:
        if not token in ['char','short','int','long','float','double']:
            continue
        #if token.originalName != '':

# 14
# STATUS: Done
def misra14(data):
    for token in data.tokenlist:
        if token.str != 'char':
            continue
        if token.isUnsigned or token.isSigned:
            continue
        reportError(token, 14)

# 15
# STATUS: Done
def misra15(data):
    reportError(data.tokenlist[0], 15)

# 16
# STATUS: TODO : ValueType information is needed
def misra16(data):
    return

# 17
# STATUS: Done (Cppcheck duplicateTypedef)
def misra17(data):
    return

# 18
# STATUS: TODO
def misra18(data):
    return

# 19
# STATUS: Done
def misra19(rawTokens):
    for tok in rawTokens:
        if re.match(r'0[0-7]+', tok.str):
            reportError(tok, 19)

# 20
# STATUS: Use compiler
def misra20(data):
    return

# 21
# STATUS: Done. Use compiler (-Wshadow etc). Cppcheck has some checking also.
def misra21(data):
    return

# 22
# STATUS: TODO
def misra22(data):
    return

# 23
# STATUS: TODO
def misra23(data):
    return

# 24
# STATUS: Probably done. Use compiler.
def misra24(data):
    return

# 25
# STATUS: TODO
def misra25(data):
    return

# 26
# STATUS: Done. Use compiler.
def misra26(data):
    return

# 27
# STATUS: TODO
def misra27(data):
    return

# 28
# STATUS: Done
def misra28(rawTokens):
    for token in rawTokens:
        if token.str == 'register':
            reportError(token, 28)

# 29
# STATUS: TODO
def misra29(data):
    return

# 30
# STATUS: Done. Cppcheck uninitVar etc..
def misra30(data):
    return

# 31
# STATUS: TODO
def misra31(data):
    return

# 32
# STATUS: Done
def misra32(data):
    for token in data.tokenlist:
        if token.str != 'enum':
            continue

        # Goto start '{'
        tok = token.next
        if tok and tok.isName:
            tok = tok.next
        if not tok or tok.str != '{':
            continue

        # Parse enum body and remember which members are assigned
        eqList = []
        eq = False
        tok = tok.next
        while tok and tok.str != '}':
            if tok.str == '=':
                eq = True
            elif tok.str == ',':
                eqList.append(eq)
                eq = False
            elif tok.link and (tok.str in ['(','[','{','<']):
                tok = tok.link
            tok = tok.next
        eqList.append(eq)

        # is there error?
        if len(eqList) <= 1:
            continue
        err = False
        if eqList[0] and eqList[1]:
            err = (False in eqList[1:])
        else:
            err = (True in eqList[1:])
        if err:
            reportError(token, 32)


# 33
# STATUS: Done
def misra33(data):
    for token in data.tokenlist:
        if token.isLogicalOp and hasSideEffects(token.astOperand2):
            reportError(token, 33)

# 34
# STATUS: Done
def misra34(data):
    for token in data.tokenlist:
        if not token.isLogicalOp:
            continue
        if not isPrimaryExpression(token.astOperand1) or not isPrimaryExpression(token.astOperand2):
            reportError(token, 34)

# 35
# STATUS: TODO (The ValueType is not available yet)
def misra35(data):
    return

# 36
# STATUS: Done. Cppcheck bitwise op checkers.
def misra36(data):
    return

# 37
# STATUS: TODO The ValueType information is not available yet
def misra37(data):
    return

# 38
# STATUS: Done. Cppcheck, compilers.
def misra38(data):
    return

# 39
# STATUS: TODO (ValueType information is needed)
def misra39(data):
    return

# 40
# STATUS: Done. Cppcheck
def misra40(data):
    return

# 41
# STATUS: Done
def misra41(data):
    reportError(data.tokenlist[0], 41)

# 42
# STATUS: TODO
def misra42(data):
    return

# 43
# STATUS: Done. Cppcheck, Compiler
def misra43(data):
    return

# 44
# STATUS: TODO
def misra44(data):
    return

# 45
# STATUS: TODO
def misra45(data):
    return

# 46
# STATUS: TODO (should be implemented in Cppcheck)
def misra46(data):
    return

# 47
# STATUS: TODO
def misra47(data):
    return

# 48
# STATUS: TODO
def misra48(data):
    return

# 49
# STATUS: TODO
def misra49(data):
    return

# 50
# STATUS: Done. Compiler
def misra50(data):
    return

# 51
# STATUS: TODO (ValueType information is needed)
def misra51(data):
    return

# 52
# STATUS: Done. Cppcheck (condition is always true, unreachable return, etc)
def misra52(data):
    return

# 53
# STATUS: Done. Cppcheck (useless code, unchecked result)
def misra53(data):
    return

# 54
# STATUS: TODO
def misra54(data):
    return

# 55
# STATUS: TODO
def misra55(data):
    return

# 56
# STATUS: Done.
def misra56(data):
    for token in data.tokenlist:
        if token.str == "goto":
            reportError(token, 56)

# 57
# STATUS: Done.
def misra57(data):
    for token in data.tokenlist:
        if token.str == "continue":
            reportError(token, 57)

# 58
# STATUS: TODO
def misra58(data):
    for token in data.tokenlist:
        if token.str != "break":
            continue
        s = token.scope
        while s and s.type == 'If':
            s = s.nestedIn
        if s and s.type in ['While', 'For']:
            reportError(token, 58)

# 59
# STATUS: TODO
def misra59(data):
    return

# 60
# STATUS: TODO
def misra60(data):
    return

# 61
# STATUS: TODO
def misra61(data):
    return

# 62
# STATUS: Done. Compiler.
def misra62(data):
    return

# 63
# STATUS: TODO
def misra63(data):
    return

# 64
# STATUS: TODO
def misra64(data):
    return

# 65
# STATUS: TODO
def misra65(data):
    return

# 66
# STATUS: TODO
def misra66(data):
    return

# 67
# STATUS: TODO
def misra67(data):
    return

# 68
# STATUS: TODO
def misra68(data):
    return

# 69
# STATUS: TODO
def misra69(data):
    return

# 70
# STATUS: TODO
def misra70(data):
    return

# 71
# STATUS: Done. Compilers warn about mismatches.
def misra71(data):
    return

# 72
# STATUS: TODO
def misra72(data):
    return

# 73
# STATUS: TODO
def misra73(data):
    return

# 74
# STATUS: Done. Cppcheck builtin checker
def misra74(data):
    return

# 75
# STATUS: Done. Should be checked by compilers.
def misra75(data):
    return

# 76
# STATUS: TODO
def misra76(data):
    return

# 77
# STATUS: TODO
def misra77(data):
    return

# 78
# STATUS: TODO
def misra78(data):
    return

# 79
# STATUS: Done. Compilers
def misra79(data):
    return

# 80
# STATUS: Done. Compilers
def misra80(data):
    return

# 81
# STATUS: TODO
def misra81(data):
    return

# 82
# STATUS: TODO
def misra82(data):
    return

# 83
# STATUS: TODO
def misra83(data):
    return

# 84
# STATUS: TODO
def misra84(data):
    return

# 85
# STATUS: TODO
def misra85(data):
    return

# 86
# STATUS: TODO
def misra86(data):
    return

# 87
# STATUS: TODO
def misra87(data):
    return

# 88
# STATUS: TODO
def misra88(data):
    return

# 89
# STATUS: TODO
def misra89(data):
    return

# 90
# STATUS: TODO
def misra90(data):
    return

# 91
# STATUS: TODO
def misra91(data):
    return

# 92
# STATUS: TODO
def misra92(data):
    return

# 93
# STATUS: TODO
def misra93(data):
    return

# 94
# STATUS: TODO
def misra94(data):
    return

# 95
# STATUS: TODO
def misra95(data):
    return

# 96
# STATUS: TODO
def misra96(data):
    return

# 97
# STATUS: TODO
def misra97(data):
    return

# 98
# STATUS: TODO
def misra98(data):
    return

# 99
# STATUS: TODO
def misra99(data):
    return

# 100
# STATUS: TODO
def misra100(data):
    return

# 101
# STATUS: TODO
def misra101(data):
    return

# 102
# STATUS: TODO
def misra102(data):
    return

# 103
# STATUS: TODO
def misra103(data):
    return

# 104
# STATUS: TODO
def misra104(data):
    return

# 105
# STATUS: TODO
def misra105(data):
    return

# 106
# STATUS: TODO
def misra106(data):
    return

# 107
# STATUS: Cppcheck has this checking
def misra107(data):
    return

# 108
# STATUS: TODO
def misra108(data):
    return

# 109
# STATUS: TODO
def misra109(data):
    return

# 110
# STATUS: TODO
def misra110(data):
    return

# 111
# STATUS: TODO
def misra111(data):
    return

# 112
# STATUS: TODO
def misra112(data):
    return

# 113
# STATUS: TODO
def misra113(data):
    return

# 114
# STATUS: TODO
def misra114(data):
    return

# 115
# STATUS: TODO
def misra115(data):
    return

# 116
# STATUS: TODO
def misra116(data):
    return

# 117
# STATUS: Cppcheck (--library)
def misra117(data):
    return

# 118
# STATUS: TODO
def misra118(data):
    return

# 119
# STATUS: TODO
def misra119(data):
    return

# 120
# STATUS: TODO
def misra120(data):
    return

# 121
# STATUS: TODO
def misra121(data):
    return

# 122
# STATUS: TODO
def misra122(data):
    return

# 123
# STATUS: TODO
def misra123(data):
    return

# 124
# STATUS: TODO
def misra124(data):
    return

# 125
# STATUS: TODO
def misra125(data):
    return

# 126
# STATUS: TODO
def misra126(data):
    return

# 127
# STATUS: TODO
def misra127(data):
    return


for arg in sys.argv[1:]:
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)

    cfgNumber = 0

    for cfg in data.configurations:
        cfgNumber = cfgNumber + 1
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        misra1(cfg)
        misra2(cfg)
        misra3(cfg)
        misra4(cfg)
        misra5(cfg)
        misra6(cfg)
        misra7(cfg)
        misra8(cfg)
        misra9(cfg)
        misra10(cfg)
        misra11(cfg)
        misra12(cfg)
        misra13(cfg)
        misra14(cfg)
        misra15(cfg)
        misra16(cfg)
        misra17(cfg)
        misra18(cfg)
        if cfgNumber == 1:
            misra19(data.rawTokens)
        misra20(cfg)
        misra21(cfg)
        misra22(cfg)
        misra23(cfg)
        misra24(cfg)
        misra25(cfg)
        misra26(cfg)
        misra27(cfg)
        if cfgNumber == 1:
            misra28(data.rawTokens)
        misra29(cfg)
        misra30(cfg)
        misra31(cfg)
        misra32(cfg)
        misra33(cfg)
        misra34(cfg)
        misra35(cfg)
        misra36(cfg)
        misra37(cfg)
        misra38(cfg)
        misra39(cfg)
        misra40(cfg)
        misra41(cfg)
        misra42(cfg)
        misra43(cfg)
        misra44(cfg)
        misra45(cfg)
        misra46(cfg)
        misra47(cfg)
        misra48(cfg)
        misra49(cfg)
        misra50(cfg)
        misra51(cfg)
        misra52(cfg)
        misra53(cfg)
        misra54(cfg)
        misra55(cfg)
        misra56(cfg)
        misra57(cfg)
        misra58(cfg)
        misra59(cfg)
        misra60(cfg)
        misra61(cfg)
        misra62(cfg)
        misra63(cfg)
        misra64(cfg)
        misra65(cfg)
        misra66(cfg)
        misra67(cfg)
        misra68(cfg)
        misra69(cfg)
        misra70(cfg)
        misra71(cfg)
        misra72(cfg)
        misra73(cfg)
        misra74(cfg)
        misra75(cfg)
        misra76(cfg)
        misra77(cfg)
        misra78(cfg)
        misra79(cfg)
        misra80(cfg)
        misra81(cfg)
        misra82(cfg)
        misra83(cfg)
        misra84(cfg)
        misra85(cfg)
        misra86(cfg)
        misra87(cfg)
        misra88(cfg)
        misra89(cfg)
        misra90(cfg)
        misra91(cfg)
        misra92(cfg)
        misra93(cfg)
        misra94(cfg)
        misra95(cfg)
        misra96(cfg)
        misra97(cfg)
        misra98(cfg)
        misra99(cfg)
        misra100(cfg)
        misra101(cfg)
        misra102(cfg)
        misra103(cfg)
        misra104(cfg)
        misra105(cfg)
        misra106(cfg)
        misra107(cfg)
        misra108(cfg)
        misra109(cfg)
        misra110(cfg)
        misra111(cfg)
        misra112(cfg)
        misra113(cfg)
        misra114(cfg)
        misra115(cfg)
        misra116(cfg)
        misra117(cfg)
        misra118(cfg)
        misra119(cfg)
        misra120(cfg)
        misra121(cfg)
        misra122(cfg)
        misra123(cfg)
        misra124(cfg)
        misra125(cfg)
        misra126(cfg)
        misra127(cfg)


