#/usr/bin/python
#
# MISRA checkers
#
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misra.py main.cpp.dump

import cppcheckdata
import sys
import re

def reportError(token, severity, msg):
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') misra: ' + msg + '\n')


def hasSideEffects(expr):
    if not expr:
        return False
    if expr.str in ['++', '--', '=']:
        return True
    # Todo: Check function calls
    return hasSideEffects(expr.astOperand1) or hasSideEffects(expr.astOperand2)

def isPrimaryExpression(expr):
    return expr and (expr.isName or expr.isNumber or (expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||']))

# Environment
# -----------

# 1 The code shall conform to ISO 9899, with no extensions permitted
# STATUS: Use compiler
def misra1(data):
    return

# 2 Code written in languages other than C should only be used if there is a defined interface
# STATUS: ?
def misra2(data):
    return

# 3 Assembler language functions that are called from C should be written as C functions containing only in-line assembly language, and in-line assembly language should not be embedded in normal C code
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
            reportError(token, 'style', '3 break out inline assembler into separate function')

# 4 Provision should be made for appropriate run-time checking
# STATUS: Done. Checked by Cppcheck
def misra4(data):
    return

# Character sets
# --------------

# 5 Only those characters and escape sequences that are defined in the ISO C standard shall be used
# STATUS: TODO
def misra5(data):
    return

# 6 Values of character types shall be restricted to a defined and documented subset of ISO 10646-1
# STATUS: TODO
def misra6(data):
    return

# 7 Tri-graphs shall not be used
# STATUS: TODO - requires some info from preprocessor
def misra7(data):
    return

# 8 Multibyte characters and wide string literals shall not be used
# STATUS: TODO
def misra8(data):
    return

# Comments
# --------

# 9 Comments shall not be nested
# STATUS: TODO
def misra9(data):
    return

# 10 Sections of code should not be commented out
# STATUS: TODO (Parse comments?)
def misra10(data):
    return

# Identifiers
# -----------

# 11 Identifiers shall not rely on the significance of more than 31 characters
# STATUS: Done.
def misra11(data):
    for token in data.tokenlist:
        if token.isName and len(token.str) > 31:
            reportError(token, 'style', '11 Identifier is longer than 31 characters')

# 12 Identifiers in different namespace shall not have the same spelling
# STATUS: Done. Use compiler (-Wshadow etc)
def misra12(data):
    return

# Types
# -----

# 13 The basic types char,int,short,long,double and float should not be used.
# STATUS: TODO originalName is missing right now
def misra13(data):
    for token in data.tokenlist:
        if not token in ['char','short','int','long','float','double']:
            continue
        #if token.originalName != '':

# 14 The type char shall always be declared as unsigned char or signed char
# STATUS: Done
def misra14(data):
    for token in data.tokenlist:
        if token.str != 'char':
            continue
        if token.isUnsigned or token.isSigned:
            continue
        reportError(token, 'style', '14 The type char shall always be declared as unsigned char or signed char')

# 15 Floating point implementations should comply with a defined floating point standard
# STATUS: Done
def misra15(data):
    reportError(data.tokenlist[0], 'style', '15 Make sure the floating point implementation comply with a defined floating point standard')


# 16 The underlying bit representation of floating point numbers shall not be used in any way by the programmer
# STATUS: TODO : ValueType information is needed
def misra16(data):
    return

# 17 typedef names shall not be reused
# STATUS: Done (Cppcheck duplicateTypedef)
def misra17(data):
    return

# Constants
# ---------

# 18 Numeric constants should be suffixed to indicate type if possible
# STATUS: TODO
def misra18(data):
    return

# 19 Octal constants shall not be used
# STATUS: Done
def misra19(rawTokens):
    for tok in rawTokens:
        if re.match(r'0[0-6]+', tok.str):
            reportError(tok, 'style', '19 Octal constants shall not be used')

# Declarations and Definitions
# ----------------------------

# 20 All object and function identifiers shall be declared before use
# STATUS: Use compiler
def misra20(data):
    return

# 21 Identifiers in inner scope shall not use same name as identifier in outer scope
# STATUS: Done. Use compiler (-Wshadow etc). Cppcheck has some checking also.
def misra21(data):
    return

# 22 Declaration of object should be at function scope unless a wider scope is necessary
# STATUS: TODO
def misra22(data):
    return

# 23 All declarations at file scope should be static if possible
# STATUS: TODO
def misra23(data):
    return

# 24 Identifiers shall not simultaneously have both internal and external linkage
# STATUS: Probably done. Use compiler.
def misra24(data):
    return

# 25 An identifier with external linkage shall have exactly one external definition
# STATUS: TODO
def misra25(data):
    return

# 26 If objects are declared more than once they shall have compatible declarations
# STATUS: Done. Use compiler.
def misra26(data):
    return

# 27 External objects should not be declared in more than one file
# STATUS: TODO
def misra27(data):
    return

# 28 The register storage class specifier should not be used
# STATUS: Done
def misra28(rawTokens):
    for token in rawTokens:
        if token.str == 'register':
            reportError(token, 'style', '28 The register storage class specifier should not be used')

# 29 The use of a tag shall agree with its declaration
# STATUS: TODO
def misra29(data):
    return

# Initialisation
# --------------

# 30 All automatic variables shall have a value assigned to them before use
# STATUS: Done. Cppcheck uninitVar etc..
def misra30(data):
    return

# 31 Braces shall be used to indicate and match the structure in the non-zero initialisation of arrays and structures
# STATUS: TODO
def misra31(data):
    return

# 32 In an enumerator list the = construct shall not be used to explicitly initialise members other than the first unless it is used to initialise all items
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
        #print(str(token.linenr) + ':' + str(eqList))

        # is there error?
        if len(eqList) <= 1:
            continue
        err = False
        if eqList[0] and eqList[1]:
            err = (False in eqList[1:])
        else:
            err = (True in eqList[1:])
        if err:
            reportError(token, 'style', '32 In an enumerator list the = construct shall not be used to explicitly initialise members other than the first unless it is used to initialise all items')



# Operators
# ---------

# 33 The right hand of a && or || shall not have side effects
# STATUS: Done
def misra33(data):
    for token in data.tokenlist:
        if token.isLogicalOp and hasSideEffects(token.astOperand2):
            reportError(token, 'style', '33 The right hand of a && or || shall not have side effects')

# 34 The operands of a && or || shall be primary expressions
# STATUS: Done
def misra34(data):
    for token in data.tokenlist:
        if not token.isLogicalOp:
            continue
        if not isPrimaryExpression(token.astOperand1) or not isPrimaryExpression(token.astOperand2):
            reportError(
                token, 'style', '34 The operands of a && or || shall be primary expressions')

# 35 Assignment operators shall not be used in expressions that return Boolean values
# STATUS: TODO (The ValueType is not available yet)
def misra35(data):
    return

# 36 Logical operators shall not be confused with bitwise operators
# STATUS: Done. Cppcheck bitwise op checkers.
def misra36(data):
    return

# 37 Bitwise operations shall not be performed on signed integer types
# STATUS: TODO The ValueType information is not available yet
def misra37(data):
    return

# 38 The right hand operand of a shift operator of a shift operator shall lie between zero and one less the width in bits of the left hand operand
# STATUS: Done. Cppcheck, compilers.
def misra38(data):
    return

# 39 The unary minus operator shall not be applied to an unsigned expression
# STATUS: TODO (ValueType information is needed)
def misra39(data):
    return

# 40 The sizeof operator should not be used on expressions that contain side effects
# STATUS: Done. Cppcheck
def misra40(data):
    return

# 41 The implementation of integer division in the chosen compiler should be determined, documented and taken into account
# STATUS: Done
def misra41(data):
    reportError(data.tokenlist[0], 'style', '41 The implementation of integer division in the chosen' +
                                            ' compiler should be determined, documented and taken into' +
                                            ' account.')

# 42 The comma operator shall not be used, except in the control expression of a for loop
# STATUS: TODO
def misra42(data):
    return

# Conversions
# -----------

# 43 Implicit conversions that might result in a loss of information shall not be used
# STATUS: Done. Cppcheck, Compiler
def misra43(data):
    return

# 44 Redundant explicit cast should not be used
# STATUS: TODO
def misra44(data):
    return

# 45 Type casting from any type to or from pointers shall not be used
# STATUS: TODO
def misra45(data):
    return


# Expressions
# -----------

# 46 The value of an expression shall be the same under any order of evaluation that the standard permits
# STATUS: TODO (should be implemented in Cppcheck)
def misra46(data):
    return

# 47 No dependence should be placed on C's precedence rules
# STATUS: TODO
def misra47(data):
    return

# 48 Mixed precision arithmetic should use explicit casting to generate the desired result
# STATUS: TODO
def misra48(data):
    return

# 49 Tests of a value against zero should be explicit, unless the operant is effectively Boolean
# STATUS: TODO
def misra49(data):
    return

# 50 Floating point variables shall not be tested for exact inequality or inequality
# STATUS: Done. Compiler
def misra50(data):
    return

# 51 Evaluation of constant unsigned integer expression should not lead to wrap-around
# STATUS: TODO (ValueType information is needed)
def misra51(data):
    return

# Control flow
# ------------

# 52 There shall not be unreachable code
# STATUS: Done. Cppcheck (condition is always true, unreachable return, etc)
def misra52(data):
    return

# 53 All non-null statements shall have a side effect
# STATUS: Done. Cppcheck (useless code, unchecked result)
def misra53(data):
    return

# 54 A null statement shall only appear on a line by itself, and shall not have any other text on the same line
# STATUS: TODO
def misra54(data):
    return

# 55 Label should not be used except in switch statements
# STATUS: TODO
def misra55(data):
    return

# 56 The goto statement shall not be used
# STATUS: Done.
def misra56(data):
    for token in data.tokenlist:
        if token.str == "goto":
            reportError(token, 'style', '56 The goto statement shall not be used')

# 57 The continue statement shall not be used
# STATUS: Done.
def misra57(data):
    for token in data.tokenlist:
        if token.str == "continue":
            reportError(token, 'style', '57 The continue statement shall not be used')

# 58 The break statement shall not be used, except to terminate the cases of a switch statement
# STATUS: TODO
def misra58(data):
    for token in data.tokenlist:
        if token.str != "break":
            continue
        s = token.scope
        while s and s.type == 'If':
            s = s.nestedIn
        if s and s.type in ['While', 'For']:
            reportError(token, 'style', '58 The break statement shall not be used, except to terminate the cases of a switch statement')

# 59 The statements forming the body of an if, else, else if, .. shall always be enclosed in braces
# STATUS: TODO
def misra59(data):
    return

# 60 All if, else if constructs should contain a final else clause
# STATUS: TODO
def misra60(data):
    return

# 61 Every non-empty case clause in a switch statement shall be terminated with a break statement
# STATUS: TODO
def misra61(data):
    return

# 62 All switch statements shall contain a final default clause
# STATUS: Done. Compiler.
def misra62(data):
    return

# 63 A switch expression should not represent a Boolean value
# STATUS: TODO
def misra63(data):
    return

# 64 Every switch statement should have at least one case
# STATUS: TODO
def misra64(data):
    return

# 65 Floating point variables shall not be used as loop counters
# STATUS: TODO
def misra65(data):
    return

# 66 Only expression concerned with loop control should appear within a for statement
# STATUS: TODO
def misra66(data):
    return

# 67 Numeric variables being used within a for loop for iteration counting should not be modified in the body of the loop
# STATUS: TODO
def misra67(data):
    return



# Functions
# ---------

# 68 Functions shall always be declared at file scope
# STATUS: TODO
def misra68(data):
    return

# 69 Functions with a variable number of arguments shall not be used
# STATUS: TODO
def misra69(data):
    return

# 70 Functions shall not call themselves directly or indirectly
# STATUS: TODO
def misra70(data):
    return

# 71 Functions shall always have prototype declarations and the prototype shall be visible at both the function definition and call
# STATUS: Done. Compilers warn about mismatches.
def misra71(data):
    return

# 72 For each function parameter the type given and definition shall be identical, and the return type shall be identical
# STATUS: TODO
def misra72(data):
    return

#73 Identifiers shall either be given for all parameters in a prototype declaration, or none
# STATUS: TODO
def misra73(data):
    return

#74 If identifiers are given for any parameters, then the identifiers used in declaration and definition shall be identical
# STATUS: Done. Cppcheck builtin checker
def misra74(data):
    return

#75 Every function shall have an explicit return type
# STATUS: Done. Should be checked by compilers.
def misra75(data):
    return

# 76 Functions with no parameter list shall be declared with parameter type void
# STATUS: TODO
def misra76(data):
    return

# 77 The unqualified type of parameters passed to a function shall be compatible with the unqualified expected types defined in the function prototype
# STATUS: TODO
def misra77(data):
    return

# 78 The number of parameters passed to a function shall match the function prototype
# STATUS: TODO
def misra78(data):
    return

# 79 The value returned by void functions shall not be used
# STATUS: Done. Compilers
def misra79(data):
    return

# 80 Void expressions shall not be passed as function parameters
# STATUS: Done. Compilers
def misra80(data):
    return

# 81 const qualification should be used on function parameters that are passed by reference, where it is intended that the function will not modify the parameter
# STATUS: TODO
def misra81(data):
    return

# 82 A function should have a single exit point
# STATUS: TODO
def misra82(data):
    return

# 83 For functions that do not have void return type..
# STATUS: TODO
def misra83(data):
    return

# 84 For functions with void return type, return statements shall not have an expression
# STATUS: TODO
def misra84(data):
    return

# 85 Functions called with no parameters should have empty parentheses
# STATUS: TODO
def misra85(data):
    return

# 86 If a function returns error information then that information should be tested
# STATUS: TODO
def misra86(data):
    return


# Preprocessing Directives
# ------------------------

# 87 #include statements in a file shall only be preceded by other pre-processor directives or comments
# STATUS: TODO
def misra87(data):
    return

# 88 Non-standard characters shall not occur in header file names in #include directives
# STATUS: TODO
def misra88(data):
    return

# 89 The #include directive shall be followed be either a <filename> of "filename" sequence
# STATUS: TODO
def misra89(data):
    return

# 90 C macros shall only be used for symbolic constants, function like macros, type qualifiers and storage class specifiers
# STATUS: TODO
def misra90(data):
    return

# 91 Macros shall not be #define'd and #undef'd within a block
# STATUS: TODO
def misra91(data):
    return

# 92 #undef should not be used
# STATUS: TODO
def misra92(data):
    return

# 93 A function should be used in preference to a function-like macro
# STATUS: TODO
def misra93(data):
    return

# 94 A function-like macro shall not be 'called' without all of its arguments
# STATUS: TODO
def misra94(data):
    return

# 95 Arguments to a function-like macro shall not contain tokens that look like pre-processor directives
# STATUS: TODO
def misra95(data):
    return

# 96 In the definition of a function-like macro the whole definition, and each instance of a parameter, shall be enclosed in parenthesis
# STATUS: TODO
def misra96(data):
    return

# 97 Identifiers in pre-processor directives should be defined before use
# STATUS: TODO
def misra97(data):
    return

# 98 There shall be at most one occurance of the # or ## pre-processor operators in a single macro definition
# STATUS: TODO
def misra98(data):
    return

# 99 All use of #pragma directive shall be documented and explained
# STATUS: TODO
def misra99(data):
    return

# 100 The defined pre-processor operator shall only be used in one of the two standard forms
# STATUS: TODO
def misra100(data):
    return


# Pointers and arrays
# -------------------

# 101 Pointer arithmetic shall not be used
# STATUS: TODO
def misra101(data):
    return

# 102 No more than 2 levels of pointer indirection should be used
# STATUS: TODO
def misra102(data):
    return

# 103 Relational operators shall not be applied to pointer types except where both operands are of the same type and point to the same array, structure or union
# STATUS: TODO
def misra103(data):
    return

# 104 Non-constant pointers to functions shall not be used
# STATUS: TODO
def misra104(data):
    return

# 105 All functions pointed to by a single pointer to a function shall be identical in the number and type of parameters and the return type
# STATUS: TODO
def misra105(data):
    return

# 106 The address of an object with automatic storage shall not be assigned to an object which may persist after the object has ceased to exist
# STATUS: TODO
def misra106(data):
    return

# 107 The null pointer shall not be dereferenced
# STATUS: Cppcheck has this checking
def misra107(data):
    return



# Structures and unions
# ---------------------

# 108 In the specification of a structure or union type, all members of the structure or union shall be fully specified
# STATUS: TODO
def misra108(data):
    return

# 109 Overlapping variable storage shall not be used
# STATUS: TODO
def misra109(data):
    return

# 110 Unions shall not be used to access the sub-parts of larger data types
# STATUS: TODO
def misra110(data):
    return

# 111 Bit fields shall only be defined to be one of type unsigned int or signed int
# STATUS: TODO
def misra111(data):
    return

# 112 Bit fields of type signed int shall be at least 2 bits long
# STATUS: TODO
def misra112(data):
    return

# 113 All members of a structure or union shall be named and shall only be access with their name
# STATUS: TODO
def misra113(data):
    return


# Standard libraries
# ------------------

# 114 Reserved words and the standard library function names shall not be redefined or undefined
# STATUS: TODO
def misra114(data):
    return

# 115 Standard library names shall not be reused
# STATUS: TODO
def misra115(data):
    return

# 116 All libraries used in production code shall be written to comply with the provisions of MISRA
# STATUS: TODO
def misra116(data):
    return

# 117 The validity of values passed to library functions shall be checked
# STATUS: Cppcheck (--library)
def misra117(data):
    return

# 118 Dynamic heap memory allocation shall not be used
# STATUS: TODO
def misra118(data):
    return

# 119 The error indicator errno shall not be used
# STATUS: TODO
def misra119(data):
    return

# 120 The macro offsetof, in library <stddef.h>, shall not be used
# STATUS: TODO
def misra120(data):
    return

# 121 <local.h> and the setlocale function shall not be used
# STATUS: TODO
def misra121(data):
    return

# 122 The setjmp macro and the longjmp function shall not be used
# STATUS: TODO
def misra122(data):
    return

# 123 The signal handling facilities of <signal.h> shall not be used
# STATUS: TODO
def misra123(data):
    return

# 124 The input/output library <stdio.h> shall not be used in production code
# STATUS: TODO
def misra124(data):
    return

# 125 The library functions atof,atoi and atol shall not be used
# STATUS: TODO
def misra125(data):
    return

# 126 The library functions abort,exit,getenv and system shall not be used
# STATUS: TODO
def misra126(data):
    return

# 127 The time handling functions of library <time.h> shall not be used
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


