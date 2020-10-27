#!/usr/bin/env python3
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
import itertools
import sys
import re
import os
import argparse
import codecs
import string
from collections import defaultdict

try:
    from itertools import izip as zip
except ImportError:
    pass


def grouped(iterable, n):
    """s -> (s0,s1,s2,...sn-1), (sn,sn+1,sn+2,...s2n-1), (s2n,s2n+1,s2n+2,...s3n-1), ..."""
    return zip(*[iter(iterable)] * n)


INT_TYPES = ['bool', 'char', 'short', 'int', 'long', 'long long']


STDINT_TYPES = ['%s%d_t' % (n, v) for n, v in itertools.product(
        ['int', 'uint', 'int_least', 'uint_least', 'int_fast', 'uint_fast'],
        [8, 16, 32, 64])]


typeBits = {
    'CHAR': None,
    'SHORT': None,
    'INT': None,
    'LONG': None,
    'LONG_LONG': None,
    'POINTER': None
}


def isUnsignedType(ty):
    return ty == 'unsigned' or ty.startswith('uint')


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


# Identifiers described in Section 7 "Library" of C90 Standard
# Based on ISO/IEC9899:1990 Annex D -- Library summary and
# Annex E -- Implementation limits.
C90_STDLIB_IDENTIFIERS = {
    # D.1 Errors
    'errno.h': ['EDOM', 'ERANGE', 'errno'],
    # D.2 Common definitions
    'stddef.h': ['NULL', 'offsetof', 'ptrdiff_t', 'size_t', 'wchar_t'],
    # D.3 Diagnostics
    'assert.h': ['NDEBUG', 'assert'],
    # D.4 Character handling
    'ctype.h': [
        'isalnum', 'isalpha', 'isblank', 'iscntrl', 'isdigit',
        'isgraph', 'islower', 'isprint', 'ispunct', 'isspace',
        'isupper', 'isxdigit', 'tolower', 'toupper',
    ],
    # D.5 Localization
    'locale.h': [
        'LC_ALL', 'LC_COLLATE', 'LC_CTYPE', 'LC_MONETARY',
        'LC_NUMERIC', 'LC_TIME', 'NULL', 'lconv',
        'setlocale', 'localeconv',
    ],
    # D.6 Mathematics
    'math.h': [
        'HUGE_VAL', 'acos', 'asin' , 'atan2', 'cos', 'sin', 'tan', 'cosh',
        'sinh', 'tanh', 'exp', 'frexp', 'ldexp', 'log', 'loglO', 'modf',
        'pow', 'sqrt', 'ceil', 'fabs', 'floor', 'fmod',
    ],
    # D.7 Nonlocal jumps
    'setjmp.h': ['jmp_buf', 'setjmp', 'longjmp'],
    # D.8 Signal handling
    'signal.h': [
        'sig_atomic_t', 'SIG_DFL', 'SIG_ERR', 'SIG_IGN', 'SIGABRT', 'SIGFPE',
        'SIGILL', 'SIGINT', 'SIGSEGV', 'SIGTERM', 'signal', 'raise',
    ],
    # D.9 Variable arguments
    'stdarg.h': ['va_list', 'va_start', 'va_arg', 'va_end'],
    # D.10 Input/output
    'stdio.h': [
        '_IOFBF', '_IOLBF', '_IONBF', 'BUFSIZ', 'EOF', 'FILE', 'FILENAME_MAX',
        'FOPEN_MAX', 'fpos_t', 'L_tmpnam', 'NULL', 'SEEK_CUR', 'SEEK_END',
        'SEEK_SET', 'size_t', 'stderr', 'stdin', 'stdout', 'TMP_MAX',
        'remove', 'rename', 'tmpfile', 'tmpnam', 'fclose', 'fflush', 'fopen',
        'freopen', 'setbuf', 'setvbuf', 'fprintf', 'fscanf', 'printf',
        'scanf', 'sprintf', 'sscanf', 'vfprintf', 'vprintf', 'vsprintf',
        'fgetc', 'fgets', 'fputc', 'fputs', 'getc', 'getchar', 'gets', 'putc',
        'putchar', 'puts', 'ungetc', 'fread', 'fwrite', 'fgetpos', 'fseek',
        'fsetpos', 'rewind', 'clearerr', 'feof', 'ferror', 'perror',
    ],
    # D.11 General utilities
    'stdlib.h': [
        'EXIT_FAILURE', 'EXIT_SUCCESS', 'MB_CUR_MAX', 'NULL', 'RAND_MAX',
        'div_t', 'ldiv_t', 'wchar_t', 'atof', 'atoi', 'strtod', 'rand',
        'srand', 'calloc', 'free', 'malloc', 'realloc', 'abort', 'atexit',
        'exit', 'getenv', 'system', 'bsearch', 'qsort', 'abs', 'div', 'ldiv',
        'mblen', 'mbtowc', 'wctomb', 'mbstowcs', 'wcstombs',
    ],
    # D.12 String handling
    'string.h': [
        'NULL', 'size_t', 'memcpy', 'memmove', 'strcpy', 'strncpy', 'strcat',
        'strncat', 'memcmp', 'strcmp', 'strcoll', 'strncmp', 'strxfrm',
        'memchr', 'strchr', 'strcspn', 'strpbrk', 'strrchr', 'strspn',
        'strstr', 'strtok', 'memset', 'strerror', 'strlen',
    ],
    # D.13 Date and time
    'time.h': [
        'CLK_TCK', 'NULL', 'clock_t', 'time_t', 'size_t', 'tm', 'clock',
        'difftime', 'mktime', 'time', 'asctime', 'ctime', 'gmtime',
        'localtime', 'strftime',
    ],
    # Annex E: Implementation limits
    'limits.h': [
        'CHAR_BIT', 'SCHAR_MIN', 'SCHAR_MAX', 'UCHAR_MAX', 'CHAR_MIN',
        'CHAR_MAX', 'MB_LEN_MAX', 'SHRT_MIN', 'SHRT_MAX', 'USHRT_MAX',
        'INT_MIN', 'INT_MAX', 'UINT_MAX', 'LONG_MIN', 'LONG_MAX', 'ULONG_MAX',
        ],
    'float.h': [
        'FLT_ROUNDS', 'FLT_RADIX', 'FLT_MANT_DIG', 'DBL_MANT_DIG',
        'LDBL_MANT_DIG', 'DECIMAL_DIG', 'FLT_DIG', 'DBL_DIG', 'LDBL_DIG',
        'DBL_MIN_EXP', 'LDBL_MIN_EXP', 'FLT_MIN_10_EXP', 'DBL_MIN_10_EXP',
        'LDBL_MIN_10_EXP', 'FLT_MAX_EXP', 'DBL_MAX_EXP', 'LDBL_MAX_EXP',
        'FLT_MAX_10_EXP', 'DBL_MAX_10_EXP', 'LDBL_MAX_10_EXP', 'FLT_MAX',
        'DBL_MAX', 'LDBL_MAX', 'FLT_MIN', 'DBL_MIN', 'LDBL_MIN',
        'FLT_EPSILON', 'DBL_EPSILON', 'LDBL_EPSILON'
    ],
}


# Identifiers described in Section 7 "Library" of C99 Standard
# Based on ISO/IEC 9899 WF14/N1256 Annex B -- Library summary
C99_STDLIB_IDENTIFIERS = {
    # B.1 Diagnostics
    'assert.h': C90_STDLIB_IDENTIFIERS['assert.h'],
    # B.2 Complex
    'complex.h': [
        'complex', 'imaginary', 'I', '_Complex_I', '_Imaginary_I',
        'CX_LIMITED_RANGE',
        'cacos', 'cacosf', 'cacosl',
        'casin', 'casinf', 'casinl',
        'catan', 'catanf', 'catanl',
        'ccos', 'ccosf', 'ccosl',
        'csin', 'csinf', 'csinl',
        'ctan', 'ctanf', 'ctanl',
        'cacosh', 'cacoshf', 'cacoshl',
        'casinh', 'casinhf', 'casinhl',
        'catanh', 'catanhf', 'catanhl',
        'ccosh', 'ccoshf', 'ccoshl',
        'csinh', 'csinhf', 'csinhl',
        'ctanh', 'ctanhf', 'ctanhl',
        'cexp', 'cexpf', 'cexpl',
        'clog', 'clogf', 'clogl',
        'cabs', 'cabsf', 'cabsl',
        'cpow', 'cpowf', 'cpowl',
        'csqrt', 'csqrtf', 'csqrtl',
        'carg', 'cargf', 'cargl',
        'cimag', 'cimagf', 'cimagl',
        'conj', 'conjf', 'conjl',
        'cproj', 'cprojf', 'cprojl',
        'creal', 'crealf', 'creall',
    ],
    # B.3 Character handling
    'ctype.h': C90_STDLIB_IDENTIFIERS['ctype.h'],
    # B.4 Errors
    'errno.h': C90_STDLIB_IDENTIFIERS['errno.h'] + ['EILSEQ'],
    # B.5 Floating-point environment
    'fenv.h': [
        'fenv_t', 'FE_OVERFLOW', 'FE_TOWARDZERO',
        'fexcept_t', 'FE_UNDERFLOW', 'FE_UPWARD',
        'FE_DIVBYZERO', 'FE_ALL_EXCEPT', 'FE_DFL_ENV',
        'FE_INEXACT', 'FE_DOWNWARD',
        'FE_INVALID', 'FE_TONEAREST',
        'FENV_ACCESS',
        'feclearexcept', 'fegetexceptflag', 'fegetround',
        'fesetround', 'fegetenv', 'feholdexcept',
        'fesetenv', 'feupdateenv',
    ],
    # B.6 Characteristics of floating types
    'float.h': C90_STDLIB_IDENTIFIERS['float.h'] + ['FLT_EVAL_METHOD'],
    # B.7 Format conversion of integer types
    'inttypes.h': [
        'imaxdiv_t', 'imaxabs', 'imaxdiv', 'strtoimax',
        'strtoumax', 'wcstoimax', 'wcstoumax',
    ],
    # B.8 Alternative spellings
    'iso646.h': [
        'and', 'and_eq', 'bitand', 'bitor', 'compl', 'not', 'not_eq',
        'or', 'or_eq', 'xor', 'xor_eq',
    ],
    # B.9 Size of integer types
    'limits.h': C90_STDLIB_IDENTIFIERS['limits.h'] +
    ['LLONG_MIN', 'LLONG_MAX', 'ULLONG_MAX'],
    # B.10 Localization
    'locale.h': C90_STDLIB_IDENTIFIERS['locale.h'],
    # B.11 Mathematics
    'math.h': C90_STDLIB_IDENTIFIERS['math.h'] + [
        'float_t', 'double_t', 'HUGE_VAL', 'HUGE_VALF', 'HUGE_VALL',
        'INFINITY', 'NAN', 'FP_INFINITE', 'FP_NAN', 'FP_NORMAL',
        'FP_SUBNORMAL', 'FP_ZERO', 'FP_FAST_FMA', 'FP_FAST_FMAF',
        'FP_FAST_FMAL', 'FP_ILOGB0', 'FP_ILOGBNAN', 'MATH_ERRNO',
        'MATH_ERREXCEPT', 'math_errhandling', 'FP_CONTRACT', 'fpclassify',
        'isfinite', 'isinf', 'isnan', 'isnormal', 'signbit', 'acosf', 'acosl',
        'asinf', 'asinl', 'atanf', 'atanl', 'atan2', 'atan2f', 'atan2l',
        'cosf', 'cosl', 'sinf', 'sinl', 'tanf', 'tanl', 'acosh', 'acoshf',
        'acoshl', 'asinh', 'asinhf', 'asinhl', 'atanh', 'atanhf', 'atanhl',
        'cosh', 'coshf', 'coshl', 'sinh', 'sinhf', 'sinhl', 'tanh', 'tanhf',
        'tanhl', 'expf', 'expl', 'exp2', 'exp2f', 'exp2l', 'expm1', 'expm1f',
        'expm1l', 'frexpf', 'frexpl', 'ilogb', 'ilogbf', 'ilogbl', 'float',
        'ldexpl', 'logf', 'logl', 'log10f', 'log10l', 'log1p', 'log1pf',
        'log1pl', 'log2', 'log2f', 'log2l', 'logb', 'logbf', 'logbl', 'modff',
        'modfl', 'scalbn', 'scalbnf', 'scalbnl', 'scalbln', 'scalblnf',
        'scalblnl', 'hypotl', 'powf', 'powl', 'sqrtf', 'sqrtl', 'erf', 'erff',
        'erfl', 'erfc', 'erfcf', 'erfcl', 'lgamma', 'lgammaf', 'lgammal',
        'tgamma', 'tgammaf', 'tgammal', 'ceilf', 'ceill', 'floorf', 'floorl',
        'nearbyint', 'nearbyintf', 'nearbyintl', 'rint', 'rintf', 'rintl',
        'lrint', 'lrintf', 'lrintl', 'llrint', 'llrintf', 'llrintl', 'round',
        'roundf', 'roundl', 'lround', 'lroundf', 'lroundl', 'llround',
        'llroundf', 'llroundl', 'trunc', 'truncf', 'truncl', 'fmodf', 'fmodl',
        'remainder', 'remainderf', 'remainderl', 'remquo', 'remquof',
        'remquol', 'copysign', 'copysignf', 'copysignl', 'nan', 'nanf',
        'nanl', 'nextafter', 'nextafterf', 'nextafterl', 'nexttoward',
        'nexttowardf', 'nexttowardl', 'fdim', 'fdimf', 'fdiml', 'fmax',
        'fmaxf', 'fmaxl', 'fmin', 'fminf', 'fminl', 'fmal', 'isgreater',
        'isgreaterequal', 'isless', 'islessequal', 'islessgreater',
        'isunordered',
    ],
    # B.12 Nonlocal jumps
    'setjmp.h': C90_STDLIB_IDENTIFIERS['setjmp.h'],
    # B.13 Signal handling
    'signal.h': C90_STDLIB_IDENTIFIERS['signal.h'],
    # B.14 Variable arguments
    'stdarg.h': C90_STDLIB_IDENTIFIERS['stdarg.h'] + ['va_copy'],
    # B.15 Boolean type and values
    'stdbool.h': ['bool', 'true', 'false', '__bool_true_false_are_defined'],
    # B.16 Common definitions
    'stddef.h': C90_STDLIB_IDENTIFIERS['stddef.h'],
    # B.17 Integer types
    'stdint.h': [
        'intptr_t', 'uintptr_t', 'intmax_t', 'uintmax_t', 'INTN_MIN',
        'INTN_MAX', 'UINTN_MAX', 'INT_LEASTN_MIN', 'INT_LEASTN_MAX',
        'UINT_LEASTN_MAX', 'INT_FASTN_MIN', 'INT_FASTN_MAX', 'UINT_FASTN_MAX',
        'INTPTR_MIN', 'INTPTR_MAX', 'UINTPTR_MAX', 'INTMAX_MIN', 'INTMAX_MAX',
        'UINTMAX_MAX', 'PTRDIFF_MIN', 'PTRDIFF_MAX', 'SIG_ATOMIC_MIN',
        'SIG_ATOMIC_MAX', 'SIZE_MAX', 'WCHAR_MIN', 'WCHAR_MAX', 'WINT_MIN',
        'WINT_MAX', 'INTN_C', 'UINTN_C', 'INTMAX_C', 'UINTMAX_C',
    ] + STDINT_TYPES,
    # B.18 Input/output
    'stdio.h': C90_STDLIB_IDENTIFIERS['stdio.h'] + [
        'mode', 'restrict', 'snprintf', 'vfscanf', 'vscanf',
        'vsnprintf', 'vsscanf',
    ],
    # B.19 General utilities
    'stdlib.h': C90_STDLIB_IDENTIFIERS['stdlib.h'] + [
        '_Exit', 'labs', 'llabs', 'lldiv', 'lldiv_t', 'strtof', 'strtol',
        'strtold', 'strtoll', 'strtoul', 'strtoull'
    ],
    # B.20 String handling
    'string.h': C90_STDLIB_IDENTIFIERS['string.h'],
    # B.21 Type-generic math
    'tgmath.h': [
        'acos', 'asin', 'atan', 'acosh', 'asinh', 'atanh', 'cos', 'sin', 'tan',
        'cosh', 'sinh', 'tanh', 'exp', 'log', 'pow', 'sqrt', 'fabs', 'atan2',
        'cbrt', 'ceil', 'copysign', 'erf', 'erfc', 'exp2', 'expm1', 'fdim',
        'floor', 'fma', 'fmax', 'fmin', 'fmod', 'frexp', 'hypot', 'ilogb',
        'ldexp', 'lgamma', 'llrint', 'llround', 'log10', 'log1p', 'log2',
        'logb', 'lrint', 'lround', 'nearbyint', 'nextafter', 'nexttoward',
        'remainder', 'remquo', 'rint', 'round', 'scalbn', 'scalbln', 'tgamma',
        'trunc', 'carg', 'cimag', 'conj', 'cproj', 'creal',
    ],
    # B.22 Date and time
    'time.h': C90_STDLIB_IDENTIFIERS['time.h'] + ['CLOCKS_PER_SEC'],
    # B.23 Extended multibyte/wide character utilities
    'wchar.h': [
        'wchar_t', 'size_t', 'mbstate_t', 'wint_t', 'tm', 'NULL', 'WCHAR_MAX',
        'WCHAR_MIN', 'WEOF', 'fwprintf', 'fwscanf', 'swprintf', 'swscanf',
        'vfwprintf', 'vfwscanf', 'vswprintf', 'vswscanf', 'vwprintf',
        'vwscanf', 'wprintf', 'wscanf', 'fgetwc', 'fgetws', 'fputwc', 'fputws',
        'fwide', 'getwc', 'getwchar', 'putwc', 'putwchar', 'ungetwc', 'wcstod',
        'wcstof', 'double', 'int', 'long', 'long', 'long', 'wcscpy', 'wcsncpy',
        'wmemcpy', 'wmemmove', 'wcscat', 'wcsncat', 'wcscmp', 'wcscoll',
        'wcsncmp', 'wcsxfrm', 'wmemcmp', 'wcschr', 'wcscspn', 'wcspbrk',
        'wcsrchr', 'wcsspn', 'wcsstr', 'wcstok', 'wmemchr', 'wcslen',
        'wmemset', 'wcsftime', 'btowc', 'wctob', 'mbsinit', 'mbrlen',
        'mbrtowc', 'wcrtomb', 'mbsrtowcs', 'wcsrtombs',
    ],
}


def isStdLibId(id_, standard='c99'):
    id_lists = []
    if standard == 'c89':
        id_lists = C90_STDLIB_IDENTIFIERS.values()
    elif standard == 'c99':
        id_lists = C99_STDLIB_IDENTIFIERS.values()
    for l in id_lists:
        if id_ in l:
            return True
    return False


# Reserved keywords defined in ISO/IEC9899:1990 -- ch 6.1.1
C90_KEYWORDS = {
    'auto', 'break', 'double', 'else', 'enum', 'extern', 'float', 'for',
    'goto', 'if', 'case', 'char', 'const', 'continue', 'default', 'do', 'int',
    'long', 'struct', 'switch', 'register', 'typedef', 'union', 'unsigned',
    'void', 'volatile', 'while', 'return', 'short', 'signed', 'sizeof',
    'static'
}


# Reserved keywords defined in ISO/IEC 9899 WF14/N1256 -- ch. 6.4.1
C99_KEYWORDS = {
    'auto', 'break', 'case', 'char', 'const', 'continue', 'default', 'do',
    'double', 'else', 'enum', 'extern', 'float', 'for', 'goto', 'if', 'inline',
    'int', 'long', 'register', 'restrict', 'return', 'short', 'signed',
    'sizeof', 'static', 'struct', 'switch', 'typedef', 'union', 'unsigned',
    'void', 'volatile', 'while', '_Bool', '_Complex', '_Imaginary'
}


def isKeyword(keyword, standard='c99'):
    kw_set = {}
    if standard == 'c89':
        kw_set = C90_KEYWORDS
    elif standard == 'c99':
        kw_set = C99_KEYWORDS
    return keyword in kw_set


def getEssentialTypeCategory(expr):
    if not expr:
        return None
    if expr.str == ',':
        return getEssentialTypeCategory(expr.astOperand2)
    if expr.str in ('<', '<=', '==', '!=', '>=', '>', '&&', '||', '!'):
        return 'bool'
    if expr.str in ('<<', '>>'):
        # TODO this is incomplete
        return getEssentialTypeCategory(expr.astOperand1)
    if len(expr.str) == 1 and expr.str in '+-*/%&|^':
        # TODO this is incomplete
        e1 = getEssentialTypeCategory(expr.astOperand1)
        e2 = getEssentialTypeCategory(expr.astOperand2)
        # print('{0}: {1} {2}'.format(expr.str, e1, e2))
        if e1 and e2 and e1 == e2:
            return e1
        if expr.valueType:
            return expr.valueType.sign
    if expr.valueType and expr.valueType.typeScope and expr.valueType.typeScope.className:
        return "enum<" + expr.valueType.typeScope.className + ">"
    if expr.variable:
        typeToken = expr.variable.typeStartToken
        while typeToken:
            if typeToken.valueType:
                if typeToken.valueType.type == 'bool':
                    return typeToken.valueType.type
                if typeToken.valueType.type in ('float', 'double', 'long double'):
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
    if (operand1.str in ('++', '--') or
            operand2.str in ('++', '--')):
        return None, None
    if ((operand1.valueType and operand1.valueType.pointer) or
            (operand2.valueType and operand2.valueType.pointer)):
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
            if typeToken.str in INT_TYPES + STDINT_TYPES + ['float', 'double']:
                return typeToken.str
            typeToken = typeToken.next

    elif expr.astOperand1 and expr.astOperand2 and expr.str in (
    '+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":"):
        if expr.astOperand1.valueType and expr.astOperand1.valueType.pointer > 0:
            return None
        if expr.astOperand2.valueType and expr.astOperand2.valueType.pointer > 0:
            return None
        e1 = getEssentialType(expr.astOperand1)
        e2 = getEssentialType(expr.astOperand2)
        if not e1 or not e2:
            return None
        if bitsOfEssentialType(e2) >= bitsOfEssentialType(e1):
            return e2
        else:
            return e1
    elif expr.str == "~":
        e1 = getEssentialType(expr.astOperand1)
        return e1

    return None


def bitsOfEssentialType(ty):
    if ty is None:
        return 0
    if ty == 'char':
        return typeBits['CHAR']
    if ty == 'short':
        return typeBits['SHORT']
    if ty == 'int':
        return typeBits['INT']
    if ty == 'long':
        return typeBits['LONG']
    if ty == 'long long':
        return typeBits['LONG_LONG']
    for sty in STDINT_TYPES:
        if ty == sty:
            return int(''.join(filter(str.isdigit, sty)))
    return 0


def isCast(expr):
    if not expr or expr.str != '(' or not expr.astOperand1 or expr.astOperand2:
        return False
    if simpleMatch(expr, '( )'):
        return False
    return True


def isFunctionCall(expr, std='c99'):
    if not expr:
        return False
    if expr.str != '(' or not expr.astOperand1:
        return False
    if expr.astOperand1 != expr.previous:
        return False
    if isKeyword(expr.astOperand1.str, std):
        return False
    return True


def hasExternalLinkage(var):
    return var.isGlobal and not var.isStatic


def countSideEffects(expr):
    if not expr or expr.str in (',', ';'):
        return 0
    ret = 0
    if expr.str in ('++', '--', '='):
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


def getForLoopCounterVariables(forToken):
    """ Return a set of Variable objects defined in ``for`` statement and
    satisfy requirements to loop counter term from section 8.14 of MISRA
    document.
    """
    if not forToken or forToken.str != 'for':
        return None
    tn = forToken.next
    if not tn or tn.str != '(':
        return None
    vars_defined = set()
    vars_exit = set()
    vars_modified = set()
    cur_clause = 1
    te = tn.link
    while tn and tn != te:
        if tn.variable:
            if cur_clause == 1 and tn.variable.nameToken == tn:
                vars_defined.add(tn.variable)
            elif cur_clause == 2:
                vars_exit.add(tn.variable)
            elif cur_clause == 3:
                if tn.next and hasSideEffectsRecursive(tn.next):
                    vars_modified.add(tn.variable)
                elif tn.previous and tn.previous.str in ('++', '--'):
                    vars_modified.add(tn.variable)
        if tn.str == ';':
            cur_clause += 1
        tn = tn.next
    return vars_defined & vars_exit & vars_modified


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
            if token.str == counterToken.str and token.astParent and token.astParent.str in ('++', '--'):
                return True
    return False


def hasSideEffectsRecursive(expr):
    if not expr or expr.str == ';':
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
    if expr.isAssignmentOp or expr.str in {'++', '--'}:
        return True
    # Todo: Check function calls
    return hasSideEffectsRecursive(expr.astOperand1) or hasSideEffectsRecursive(expr.astOperand2)


def isBoolExpression(expr):
    if not expr:
        return False
    if expr.valueType and (expr.valueType.type == 'bool' or expr.valueType.bits == 1):
        return True
    return expr.str in ['!', '==', '!=', '<', '<=', '>', '>=', '&&', '||', '0', '1', 'true', 'false']


def isEnumConstant(expr):
    if not expr or not expr.values:
        return False
    values = expr.values
    return len(values) == 1 and values[0].valueKind == 'known'


def isConstantExpression(expr):
    if expr.isNumber:
        return True
    if expr.isName and not isEnumConstant(expr):
        return False
    if simpleMatch(expr.previous, 'sizeof ('):
        return True
    if expr.astOperand1 and not isConstantExpression(expr.astOperand1):
        return False
    if expr.astOperand2 and not isConstantExpression(expr.astOperand2):
        return False
    return True


def isUnsignedInt(expr):
    return expr and expr.valueType and expr.valueType.type in ('short', 'int') and expr.valueType.sign == 'unsigned'


def getPrecedence(expr):
    if not expr:
        return 16
    if not expr.astOperand1 or not expr.astOperand2:
        return 16
    if expr.str in ('*', '/', '%'):
        return 12
    if expr.str in ('+', '-'):
        return 11
    if expr.str in ('<<', '>>'):
        return 10
    if expr.str in ('<', '>', '<=', '>='):
        return 9
    if expr.str in ('==', '!='):
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
    if expr.str in ('?', ':'):
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


def isalnum(c):
    return c in string.digits or c in string.ascii_letters


def isHexEscapeSequence(symbols):
    """Checks that given symbols are valid hex escape sequence.

    hexadecimal-escape-sequence:
            \\x hexadecimal-digit
            hexadecimal-escape-sequence hexadecimal-digit

    Reference: n1570 6.4.4.4"""
    if len(symbols) < 3 or symbols[:2] != '\\x':
        return False
    return all([s in string.hexdigits for s in symbols[2:]])


def isOctalEscapeSequence(symbols):
    r"""Checks that given symbols are valid octal escape sequence:

     octal-escape-sequence:
             \ octal-digit
             \ octal-digit octal-digit
             \ octal-digit octal-digit octal-digit

    Reference: n1570 6.4.4.4"""
    if len(symbols) not in range(2, 5) or symbols[0] != '\\':
        return False
    return all([s in string.octdigits for s in symbols[1:]])


def isSimpleEscapeSequence(symbols):
    """Checks that given symbols are simple escape sequence.
    Reference: n1570 6.4.4.4"""
    if len(symbols) != 2 or symbols[0] != '\\':
        return False
    return symbols[1] in ("'", '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v')


def isTernaryOperator(token):
    if not token:
        return False
    if not token.astOperand2:
        return False
    return token.str == '?' and token.astOperand2.str == ':'


def getTernaryOperandsRecursive(token):
    """Returns list of ternary operands including nested ones."""
    if not isTernaryOperator(token):
        return []
    result = []
    result += getTernaryOperandsRecursive(token.astOperand2.astOperand1)
    if token.astOperand2.astOperand1 and not isTernaryOperator(token.astOperand2.astOperand1):
        result += [token.astOperand2.astOperand1]
    result += getTernaryOperandsRecursive(token.astOperand2.astOperand2)
    if token.astOperand2.astOperand2 and not isTernaryOperator(token.astOperand2.astOperand2):
        result += [token.astOperand2.astOperand2]
    return result


def hasNumericEscapeSequence(symbols):
    """Check that given string contains octal or hexadecimal escape sequences."""
    if '\\' not in symbols:
        return False
    for c, cn in grouped(symbols, 2):
        if c == '\\' and cn in ('x' + string.octdigits):
            return True
    return False


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


class Define:
    def __init__(self, directive):
        self.args = []
        self.expansionList = ''

        res = re.match(r'#define [A-Za-z0-9_]+\(([A-Za-z0-9_, ]+)\)[ ]+(.*)', directive.str)
        if res is None:
            return

        self.args = res.group(1).strip().split(',')
        self.expansionList = res.group(2)

    def __repr__(self):
        attrs = ["args", "expansionList"]
        return "{}({})".format(
            "Define",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )


def getAddonRules():
    """Returns dict of MISRA rules handled by this addon."""
    addon_rules = []
    compiled = re.compile(r'.*def[ ]+misra_([0-9]+)_([0-9]+)[(].*')
    for line in open(__file__):
        res = compiled.match(line)
        if res is None:
            continue
        addon_rules.append(res.group(1) + '.' + res.group(2))
    return addon_rules


def getCppcheckRules():
    """Returns list of rules handled by cppcheck."""
    return ['1.3', '2.1', '2.2', '2.4', '2.6', '5.3', '8.3', '12.2',
            '13.2', '13.6', '14.3', '17.5', '18.1', '18.2', '18.3',
            '18.6', '20.6', '22.1', '22.2', '22.4', '22.6']


def generateTable():
    # print table
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

    # Rules that can be checked with compilers:
    # compiler = ['1.1', '1.2']

    addon = getAddonRules()
    cppcheck = getCppcheckRules()
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


def remove_file_prefix(file_path, prefix):
    """
    Remove a file path prefix from a give path.  leftover
    directory separators at the beginning of a file
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
        # Remove any leftover directory separators at the
        # beginning
        result = result.lstrip('\\/')
    else:
        result = file_path
    return result


class Rule(object):
    """Class to keep rule text and metadata"""

    MISRA_SEVERITY_LEVELS = ['Required', 'Mandatory', 'Advisory']

    def __init__(self, num1, num2):
        self.num1 = num1
        self.num2 = num2
        self.text = ''
        self.misra_severity = ''

    @property
    def num(self):
        return self.num1 * 100 + self.num2

    @property
    def misra_severity(self):
        return self._misra_severity

    @misra_severity.setter
    def misra_severity(self, val):
        if val in self.MISRA_SEVERITY_LEVELS:
            self._misra_severity = val
        else:
            self._misra_severity = ''

    @property
    def cppcheck_severity(self):
        return 'style'

    def __repr__(self):
        return "%d.%d (%s)" % (self.num1, self.num2, self.misra_severity)


class MisraSettings(object):
    """Hold settings for misra.py script."""

    __slots__ = ["verify", "quiet", "show_summary"]

    def __init__(self, args):
        """
        :param args: Arguments given by argparse.
        """
        self.verify = False
        self.quiet = False
        self.show_summary = True

        if args.verify:
            self.verify = True
        if args.cli:
            self.quiet = True
            self.show_summary = False
        if args.quiet:
            self.quiet = True
        if args.no_summary:
            self.show_summary = False

    def __repr__(self):
        attrs = ["verify", "quiet", "show_summary", "verify"]
        return "{}({})".format(
            "MisraSettings",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )


class MisraChecker:

    def __init__(self, settings, stdversion="c90"):
        """
        :param settings: misra.py script settings.
        """

        self.settings = settings

        # Test validation rules lists
        self.verify_expected = list()
        self.verify_actual = list()

        # List of formatted violation messages
        self.violations = dict()

        # if --rule-texts is specified this dictionary
        # is loaded with descriptions of each rule
        # by rule number (in hundreds).
        # ie rule 1.2 becomes 102
        self.ruleTexts = dict()

        # Dictionary of dictionaries for rules to suppress
        # Dict1 is keyed by rule number in the hundreds format of
        # Major *  100 + minor. ie Rule 5.2 = (5*100) + 2
        # Dict 2 is keyed by filename.  An entry of None means suppress globally.
        # Each file name entry contains a list of tuples of (lineNumber, symbolName)
        # or an item of None which indicates suppress rule for the entire file.
        # The line and symbol name tuple may have None as either of its elements but
        # should not be None for both.
        self.suppressedRules = dict()

        # List of suppression extracted from the dumpfile
        self.dumpfileSuppressions = None

        # Prefix to ignore when matching suppression files.
        self.filePrefix = None

        # Number of all violations suppressed per rule
        self.suppressionStats = dict()

        self.stdversion = stdversion

        self.severity = None

        self.existing_violations = set()

    def __repr__(self):
        attrs = ["settings", "verify_expected", "verify_actual", "violations",
                 "ruleTexts", "suppressedRules", "dumpfileSuppressions",
                 "filePrefix", "suppressionStats", "stdversion", "severity"]
        return "{}({})".format(
            "MisraChecker",
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )

    def get_num_significant_naming_chars(self, cfg):
        if cfg.standards and cfg.standards.c == "c89":
            return 31
        else:
            return 63

    def misra_2_7(self, data):
        for func in data.functions:
            # Skip function with no parameter
            if (len(func.argument) == 0):
                continue
            # Setup list of function parameters
            func_param_list = list()
            for arg in func.argument:
                func_param_list.append(func.argument[arg])
            # Search for scope of current function
            for scope in data.scopes:
                if (scope.type == "Function") and (scope.function == func):
                    # Search function body: remove referenced function parameter from list
                    token = scope.bodyStart
                    while (token.next != None and token != scope.bodyEnd and len(func_param_list) > 0):
                        if (token.variable != None and token.variable in func_param_list):
                            func_param_list.remove(token.variable)
                        token = token.next
                    if (len(func_param_list) > 0):
                        # At least one parameter has not been referenced in function body
                        self.reportError(func.tokenDef, 2, 7)

    def misra_3_1(self, rawTokens):
        for token in rawTokens:
            starts_with_double_slash = token.str.startswith('//')
            if token.str.startswith('/*') or starts_with_double_slash:
                s = token.str.lstrip('/')
                if ((not starts_with_double_slash) and '//' in s) or '/*' in s:
                    self.reportError(token, 3, 1)

    def misra_3_2(self, rawTokens):
        for token in rawTokens:
            if token.str.startswith('//'):
                # Check for comment ends with trigraph which might be replaced
                # by a backslash.
                if token.str.endswith('??/'):
                    self.reportError(token, 3, 2)
                # Check for comment which has been merged with subsequent line
                # because it ends with backslash.
                # The last backslash is no more part of the comment token thus
                # check if next token exists and compare line numbers.
                elif (token.next is not None) and (token.linenr == token.next.linenr):
                    self.reportError(token, 3, 2)

    def misra_4_1(self, rawTokens):
        for token in rawTokens:
            if (token.str[0] != '"') and (token.str[0] != '\''):
                continue
            if len(token.str) < 3:
                continue

            delimiter = token.str[0]
            symbols = token.str[1:-1]

            # No closing delimiter. This will not compile.
            if token.str[-1] != delimiter:
                continue

            if len(symbols) < 2:
                continue

            if not hasNumericEscapeSequence(symbols):
                continue

            # String literals that contains one or more escape sequences. All of them should be
            # terminated.
            for sequence in ['\\' + t for t in symbols.split('\\')][1:]:
                if (isHexEscapeSequence(sequence) or isOctalEscapeSequence(sequence) or
                        isSimpleEscapeSequence(sequence)):
                    continue
                else:
                    self.reportError(token, 4, 1)

    def misra_4_2(self, rawTokens):
        for token in rawTokens:
            if (token.str[0] != '"') or (token.str[-1] != '"'):
                continue
            # Check for trigraph sequence as defined by ISO/IEC 9899:1999
            for sequence in ['??=', '??(', '??/', '??)', '??\'', '??<', '??!', '??>', '??-']:
                if sequence in token.str[1:-1]:
                    # First trigraph sequence match, report error and leave loop.
                    self.reportError(token, 4, 2)
                    break

    def misra_5_1(self, data):
        long_vars = {}
        num_sign_chars = self.get_num_significant_naming_chars(data)
        for var in data.variables:
            if var.nameToken is None:
                continue
            if len(var.nameToken.str) <= num_sign_chars:
                continue
            if not hasExternalLinkage(var):
                continue
            long_vars.setdefault(var.nameToken.str[:num_sign_chars], []).append(var.nameToken)
        for name_prefix in long_vars:
            tokens = long_vars[name_prefix]
            if len(tokens) < 2:
                continue
            for tok in sorted(tokens, key=lambda t: (t.linenr, t.column))[1:]:
                self.reportError(tok, 5, 1)

    def misra_5_2(self, data):
        scopeVars = {}
        num_sign_chars = self.get_num_significant_naming_chars(data)
        for var in data.variables:
            if var.nameToken is None:
                continue
            if len(var.nameToken.str) <= num_sign_chars:
                continue
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
                    if hasExternalLinkage(variable1) or hasExternalLinkage(variable2):
                        continue
                    if (variable1.nameToken.str[:num_sign_chars] == variable2.nameToken.str[:num_sign_chars] and
                            variable1.Id != variable2.Id):
                        if int(variable1.nameToken.linenr) > int(variable2.nameToken.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(variable2.nameToken, 5, 2)
                for innerscope in scopeVars[scope]["scopelist"]:
                    if variable1.nameToken.str[:num_sign_chars] == innerscope.className[:num_sign_chars]:
                        if int(variable1.nameToken.linenr) > int(innerscope.bodyStart.linenr):
                            self.reportError(variable1.nameToken, 5, 2)
                        else:
                            self.reportError(innerscope.bodyStart, 5, 2)
            if len(scopeVars[scope]["scopelist"]) <= 1:
                continue
            for i, scopename1 in enumerate(scopeVars[scope]["scopelist"]):
                for scopename2 in scopeVars[scope]["scopelist"][i + 1:]:
                    if scopename1.className[:num_sign_chars] == scopename2.className[:num_sign_chars]:
                        if int(scopename1.bodyStart.linenr) > int(scopename2.bodyStart.linenr):
                            self.reportError(scopename1.bodyStart, 5, 2)
                        else:
                            self.reportError(scopename2.bodyStart, 5, 2)

    def misra_5_4(self, data):
        num_sign_chars = self.get_num_significant_naming_chars(data)
        macro = {}
        compile_name = re.compile(r'#define ([a-zA-Z0-9_]+)')
        compile_param = re.compile(r'#define ([a-zA-Z0-9_]+)[(]([a-zA-Z0-9_, ]+)[)]')
        short_names = {}
        macro_w_arg = []
        for dir in data.directives:
            res1 = compile_name.match(dir.str)
            if res1:
                if dir not in macro:
                    macro.setdefault(dir, {})["name"] = []
                    macro.setdefault(dir, {})["params"] = []
                full_name = res1.group(1)
                macro[dir]["name"] = full_name
                short_name = full_name[:num_sign_chars]
                if short_name in short_names:
                    _dir = short_names[short_name]
                    if full_name != macro[_dir]["name"]:
                        self.reportError(dir, 5, 4)
                else:
                    short_names[short_name] = dir
            res2 = compile_param.match(dir.str)
            if res2:
                res_gp2 = res2.group(2).split(",")
                res_gp2 = [macroname.replace(" ", "") for macroname in res_gp2]
                macro[dir]["params"].extend(res_gp2)
                macro_w_arg.append(dir)
        for mvar in macro_w_arg:
            for i, macroparam1 in enumerate(macro[mvar]["params"]):
                for j, macroparam2 in enumerate(macro[mvar]["params"]):
                    if j > i and macroparam1[:num_sign_chars] == macroparam2[:num_sign_chars]:
                        self.reportError(mvar, 5, 4)
                param = macroparam1
                if param[:num_sign_chars] in short_names:
                    m_var1 = short_names[param[:num_sign_chars]]
                    if m_var1.linenr > mvar.linenr:
                        self.reportError(m_var1, 5, 4)
                    else:
                        self.reportError(mvar, 5, 4)

    def misra_5_5(self, data):
        num_sign_chars = self.get_num_significant_naming_chars(data)
        macroNames = {}
        compiled = re.compile(r'#define ([A-Za-z0-9_]+)')
        for dir in data.directives:
            res = compiled.match(dir.str)
            if res:
                macroNames[res.group(1)[:num_sign_chars]] = dir
        for var in data.variables:
            if var.nameToken and var.nameToken.str[:num_sign_chars] in macroNames:
                self.reportError(var.nameToken, 5, 5)
        for scope in data.scopes:
            if scope.className and scope.className[:num_sign_chars] in macroNames:
                self.reportError(scope.bodyStart, 5, 5)


    def misra_6_1(self, data):
        # Bitfield type must be bool or explicity signed/unsigned int
        for token in data.tokenlist:  
            if token.valueType == None:
                continue

            if token.valueType.bits == 0:
                continue

            if token.valueType.type == 'bool':
                continue

            if token.valueType.type != 'int':
                self.reportError(token, 6, 1)

            if token.variable == None or token.variable.typeStartToken == None or token.variable.typeEndToken == None:
                continue

            isExplicitlySignedOrUnsigned = False
            typeToken = token.variable.typeStartToken
            while typeToken:
                if typeToken.isUnsigned or typeToken.isSigned:
                    isExplicitlySignedOrUnsigned = True
                    break

                if typeToken.Id == token.variable.typeEndToken.Id:
                    break
                
                typeToken = typeToken.next

            if not isExplicitlySignedOrUnsigned:
                self.reportError(token, 6, 1)


    def misra_6_2(self, data):
        # Bitfields of size 1 can not be signed
        for token in data.tokenlist:  
            if token.valueType == None:
                continue

            if token.valueType.bits == 1 and token.valueType.sign == 'signed':
                self.reportError(token, 6, 2)


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
            if scope.type != 'Enum':
                continue
            enum_values = []
            implicit_enum_values = []
            e_token = scope.bodyStart.next
            while e_token != scope.bodyEnd:
                if e_token.str == '(':
                    e_token = e_token.link
                    continue
                if e_token.previous.str not in ',{':
                    e_token = e_token.next
                    continue
                if e_token.isName and e_token.values and e_token.valueType and e_token.valueType.typeScope == scope:
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

            for t1, t2 in itertools.product(
                    list(getTernaryOperandsRecursive(token.astOperand1) or [token.astOperand1]),
                    list(getTernaryOperandsRecursive(token.astOperand2) or [token.astOperand2]),
            ):
                e1 = getEssentialTypeCategory(t1)
                e2 = getEssentialTypeCategory(t2)
                if not e1 or not e2:
                    continue
                if token.str in ('<<', '>>'):
                    if not isUnsignedType(e1):
                        self.reportError(token, 10, 1)
                    elif not isUnsignedType(e2) and not token.astOperand2.isNumber:
                        self.reportError(token, 10, 1)
                elif token.str in ('~', '&', '|', '^'):
                    e1_et = getEssentialType(token.astOperand1)
                    e2_et = getEssentialType(token.astOperand2)
                    if e1_et == 'char' and e2_et == 'char':
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
            if (token.astOperand2.str not in ('+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~') and
                    not isCast(token.astOperand2)):
                continue
            vt1 = token.astOperand1.valueType
            vt2 = token.astOperand2.valueType
            if not vt1 or vt1.pointer > 0:
                continue
            if not vt2 or vt2.pointer > 0:
                continue
            try:
                if isCast(token.astOperand2):
                    e = vt2.type
                else:
                    e = getEssentialType(token.astOperand2)
                if not e:
                    continue
                if bitsOfEssentialType(vt1.type) > bitsOfEssentialType(e):
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
            if token.astOperand1.str not in ('+', '-', '*', '/', '%', '&', '|', '^', '>>', "<<", "?", ":", '~'):
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
                    e = getEssentialType(token.astOperand1)
                    if not e:
                        continue
                    if bitsOfEssentialType(token.valueType.type) > bitsOfEssentialType(e):
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
            elif vt1.pointer > 0 and vt2.pointer == 0 and (vt2.isIntegral() or vt2.isEnum()) and vt1.type != 'void':
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
            if token.astOperand1.astOperand1 and token.astOperand1.astOperand1.str in (
            'malloc', 'calloc', 'realloc', 'free'):
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
                    vt1.type != 'void'):
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
                if tok.str in ('+', '-', '*', '/', '%'):
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
            if not (token.str in ('<<', '>>')):
                continue
            if (not token.astOperand2) or (not token.astOperand2.values):
                continue
            maxval = 0
            for val in token.astOperand2.values:
                if val.intvalue and val.intvalue > maxval:
                    maxval = val.intvalue
            if maxval == 0:
                continue
            sz = bitsOfEssentialType(getEssentialType(token.astOperand1))
            if sz <= 0:
                continue
            if maxval >= sz:
                self.reportError(token, 12, 2)

    def misra_12_3(self, data):
        for token in data.tokenlist:
            if token.str == ';' and (token.isSplittedVarDeclComma is True):
                self.reportError(token, 12, 3)
            if token.str == ',' and token.astParent and token.astParent.str == ';':
                self.reportError(token, 12, 3)
            if token.str == ',' and token.astParent is None:
                if token.scope.type in ('Class', 'Struct'):
                    # Is this initlist..
                    tok = token
                    while tok and tok.str == ',':
                        tok = tok.next
                        if tok and tok.next and tok.isName and tok.next.str == '(':
                            tok = tok.next.link.next
                    if tok.str == '{':
                        # This comma is used in initlist, do not warn
                        continue
                prev = token.previous
                while prev:
                    if prev.str == ';':
                        self.reportError(token, 12, 3)
                        break
                    elif prev.str in ')}]':
                        prev = prev.link
                    elif prev.str in '({[':
                        break
                    prev = prev.previous

    def misra_12_4(self, data):
        if typeBits['INT'] == 16:
            max_uint = 0xffff
        elif typeBits['INT'] == 32:
            max_uint = 0xffffffff
        else:
            return

        for token in data.tokenlist:
            if not token.values:
                continue
            if (not isConstantExpression(token)) or (not isUnsignedInt(token)):
                continue
            for value in token.values:
                if value.intvalue is None:
                    continue
                if value.intvalue < 0 or value.intvalue > max_uint:
                    self.reportError(token, 12, 4)
                    break

    def misra_13_1(self, data):
        for token in data.tokenlist:
            if simpleMatch(token, ") {") and token.next.astParent == token.link:
                pass
            elif not simpleMatch(token, '= {'):
                continue
            init = token.next
            end = init.link
            if not end:
                continue  # syntax is broken

            tn = init
            while tn and tn != end:
                if tn.str == '[' and tn.link:
                    tn = tn.link
                    if tn and tn.next and tn.next.str == '=':
                        tn = tn.next.next
                        continue
                    else:
                        break
                if tn.str == '.' and tn.next and tn.next.isName:
                    tn = tn.next
                    if tn.next and tn.next.str == '=':
                        tn = tn.next.next
                    continue
                if tn.str in {'++', '--'} or tn.isAssignmentOp:
                    self.reportError(init, 13, 1)
                tn = tn.next

    def misra_13_3(self, data):
        for token in data.tokenlist:
            if token.str not in ('++', '--'):
                continue
            astTop = token
            while astTop.astParent and astTop.astParent.str not in (',', ';'):
                astTop = astTop.astParent
            if countSideEffects(astTop) >= 2:
                self.reportError(astTop, 13, 3)

    def misra_13_4(self, data):
        for token in data.tokenlist:
            if token.str != '=':
                continue
            if not token.astParent:
                continue
            if token.astOperand1.str == '[' and token.astOperand1.previous.str in ('{', ','):
                continue
            if not (token.astParent.str in [',', ';', '{']):
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

            # Inspect modification of loop counter in loop body
            counter_vars = getForLoopCounterVariables(token)
            outer_scope = token.scope
            body_scope = None
            tn = token.next
            while tn and tn.next != outer_scope.bodyEnd:
                if tn.scope and tn.scope.nestedIn == outer_scope:
                    body_scope = tn.scope
                    break
                tn = tn.next
            if not body_scope:
                continue
            tn = body_scope.bodyStart
            while tn and tn != body_scope.bodyEnd:
                if tn.variable and tn.variable in counter_vars:
                    if tn.next:
                        # TODO: Check modifications in function calls
                        if hasSideEffectsRecursive(tn.next):
                            self.reportError(tn, 14, 2)
                tn = tn.next

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
            # Jump crosses from one switch-clause to another is non-compliant
            elif scope.type == 'Switch':
                # Search for start of a current case block
                tcase_start = token
                while tcase_start and tcase_start.str not in ('case', 'default'):
                    tcase_start = tcase_start.previous
                # Make sure that goto label doesn't occurs in the other
                # switch-clauses
                if tcase_start:
                    t = scope.bodyStart
                    in_this_case = False
                    while t and t != scope.bodyEnd:
                        if t == tcase_start:
                            in_this_case = True
                        if in_this_case and t.str not in ('case', 'default'):
                            in_this_case = False
                        if t == tok and not in_this_case:
                            self.reportError(token, 15, 3)
                            break
                        t = t.next

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
                if token.str not in ('{', '#'):
                    self.reportError(tok1, 15, 6)

    def misra_15_7(self, data):
        for scope in data.scopes:
            if scope.type != 'Else':
                continue
            if not simpleMatch(scope.bodyStart, '{ if ('):
                continue
            if scope.bodyStart.column > 0:
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
        STATE_NONE = 0  # default state, not in switch case/default block
        STATE_BREAK = 1  # break/comment is seen but not its ';'
        STATE_OK = 2  # a case/default is allowed (we have seen 'break;'/'comment'/'{'/attribute)
        STATE_SWITCH = 3  # walking through switch statement scope

        state = STATE_NONE
        end_swtich_token = None  # end '}' for the switch scope
        for token in rawTokens:
            # Find switch scope borders
            if token.str == 'switch':
                state = STATE_SWITCH
            if state == STATE_SWITCH:
                if token.str == '{':
                    end_swtich_token = findRawLink(token)
                else:
                    continue

            if token.str == 'break' or token.str == 'return' or token.str == 'throw':
                state = STATE_BREAK
            elif token.str == ';':
                if state == STATE_BREAK:
                    state = STATE_OK
                elif token.next and token.next == end_swtich_token:
                    self.reportError(token.next, 16, 3)
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
                prev = findRawLink(token)
                if prev:
                    prev = prev.previous
                    while prev and prev.str[:2] in ('//', '/*'):
                        prev = prev.previous
                if (prev is None) or (prev.str not in ':;{}'):
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
                if tok2.str in ('}', 'case'):
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
            if isFunctionCall(token) and token.astOperand1.str in (
            'va_list', 'va_arg', 'va_start', 'va_end', 'va_copy'):
                self.reportError(token, 17, 1)
            elif token.str == 'va_list':
                self.reportError(token, 17, 1)

    def misra_17_2(self, data):
        # find recursions..
        def find_recursive_call(search_for_function, direct_call, calls_map, visited=None):
            if visited is None:
                visited = set()
            if direct_call == search_for_function:
                return True
            for indirect_call in calls_map.get(direct_call, []):
                if indirect_call == search_for_function:
                    return True
                if indirect_call in visited:
                    # This has already been handled
                    continue
                visited.add(indirect_call)
                if find_recursive_call(search_for_function, indirect_call, calls_map, visited):
                    return True
            return False

        # List functions called in each function
        function_calls = {}
        for scope in data.scopes:
            if scope.type != 'Function':
                continue
            calls = []
            tok = scope.bodyStart
            while tok != scope.bodyEnd:
                tok = tok.next
                if not isFunctionCall(tok, data.standards.c):
                    continue
                f = tok.astOperand1.function
                if f is not None and f not in calls:
                    calls.append(f)
            function_calls[scope.function] = calls

        # Report warnings for all recursions..
        for func in function_calls:
            for call in function_calls[func]:
                if not find_recursive_call(func, call, function_calls):
                    # Function call is not recursive
                    continue
                # Warn about all functions calls..
                for scope in data.scopes:
                    if scope.type != 'Function' or scope.function != func:
                        continue
                    tok = scope.bodyStart
                    while tok != scope.bodyEnd:
                        if tok.function and tok.function == call:
                            self.reportError(tok, 17, 2)
                        tok = tok.next

    def misra_17_6(self, rawTokens):
        for token in rawTokens:
            if simpleMatch(token, '[ static'):
                self.reportError(token, 17, 6)

    def misra_17_7(self, data):
        for token in data.tokenlist:
            if not token.scope.isExecutable:
                continue
            if token.str != '(' or token.astParent:
                continue
            if not token.previous.isName or token.previous.varId:
                continue
            if token.valueType is None:
                continue
            if token.valueType.type == 'void' and token.valueType.pointer == 0:
                continue
            self.reportError(token, 17, 7)

    def misra_17_8(self, data):
        for token in data.tokenlist:
            if not (token.isAssignmentOp or (token.str in ('++', '--'))):
                continue
            if not token.astOperand1:
                continue
            var = token.astOperand1.variable
            if var and var.isArgument:
                self.reportError(token, 17, 8)

    def misra_18_4(self, data):
        for token in data.tokenlist:
            if token.str not in ('+', '-', '+=', '-='):
                continue
            if token.astOperand1 is None or token.astOperand2 is None:
                continue
            vt1 = token.astOperand1.valueType
            vt2 = token.astOperand2.valueType
            if vt1 and vt1.pointer > 0:
                self.reportError(token, 18, 4)
            elif vt2 and vt2.pointer > 0:
                self.reportError(token, 18, 4)

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

    def misra_18_7(self, data):
        for scope in data.scopes:
            if scope.type != 'Struct':
                continue

            token = scope.bodyStart.next
            while token != scope.bodyEnd and token is not None:
                # Handle nested structures to not duplicate an error.
                if token.str == '{':
                    token = token.link

                if cppcheckdata.simpleMatch(token, "[ ]"):
                    self.reportError(token, 18, 7)
                    break
                token = token.next

    def misra_18_8(self, data):
        for var in data.variables:
            if not var.isArray or not var.isLocal:
                continue
            # TODO Array dimensions are not available in dump, must look in tokens
            typetok = var.nameToken.next
            if not typetok or typetok.str != '[':
                continue
            # Unknown define or syntax error
            if not typetok.astOperand2:
                continue
            if not isConstantExpression(typetok.astOperand2):
                self.reportError(var.nameToken, 18, 8)

    def misra_19_2(self, data):
        for token in data.tokenlist:
            if token.str == 'union':
                self.reportError(token, 19, 2)

    def misra_20_1(self, data):
        token_in_file = {}
        for token in data.tokenlist:
            if token.file not in token_in_file:
                token_in_file[token.file] = int(token.linenr)
            else:
                token_in_file[token.file] = min(token_in_file[token.file], int(token.linenr))

        for directive in data.directives:
            if not directive.str.startswith('#include'):
                continue
            if directive.file not in token_in_file:
                continue
            if token_in_file[directive.file] < int(directive.linenr):
                self.reportError(directive, 20, 1)

    def misra_20_2(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include '):
                continue
            for pattern in ('\\', '//', '/*', ',', "'"):
                if pattern in directive.str:
                    self.reportError(directive, 20, 2)
                    break

    def misra_20_3(self, data):
        for directive in data.directives:
            if not directive.str.startswith('#include '):
                continue

            words = directive.str.split(' ')

            # If include directive contains more than two words, here would be
            # violation anyway.
            if len(words) > 2:
                self.reportError(directive, 20, 3)

            # Handle include directives with not quoted argument
            elif len(words) > 1:
                filename = words[1]
                if not ((filename.startswith('"') and
                         filename.endswith('"')) or
                        (filename.startswith('<') and
                         filename.endswith('>'))):
                    # We are handle only directly included files in the
                    # following format: #include file.h
                    # Cases with macro expansion provided by MISRA document are
                    # skipped because we don't always have access to directive
                    # definition.
                    if '.' in filename:
                        self.reportError(directive, 20, 3)

    def misra_20_4(self, data):
        for directive in data.directives:
            res = re.search(r'#define ([a-z][a-z0-9_]+)', directive.str)
            if res and isKeyword(res.group(1), data.standards.c):
                self.reportError(directive, 20, 4)

    def misra_20_5(self, data):
        for directive in data.directives:
            if directive.str.startswith('#undef '):
                self.reportError(directive, 20, 5)

    def misra_20_7(self, data):
        def find_string_concat(exp, arg, directive_args):
            # Handle concatenation of string literals, e.g.:
            # #define MACRO(A, B) (A " " B)
            # Addon should not report errors for both macro arguments.
            arg_pos = exp.find(arg, 0)
            need_check = False
            skip_next = False
            state_in_string = False
            pos_search = arg_pos + 1
            directive_args = [a.strip() for a in directive_args if a != arg]
            arg = arg.strip()
            while pos_search < len(exp):
                if exp[pos_search] == '"':
                    if state_in_string:
                        state_in_string = False
                    else:
                        state_in_string = True
                    pos_search += 1
                elif exp[pos_search].isalnum():
                    word = ""
                    while pos_search < len(exp) and exp[pos_search].isalnum():
                        word += exp[pos_search]
                        pos_search += 1
                    if word == arg:
                        pos_search += 1
                    elif word in directive_args:
                        skip_next = True
                        break
                elif exp[pos_search] == ' ':
                    pos_search += 1
                elif state_in_string:
                    pos_search += 1
                else:
                    need_check = True
                    break
            return need_check, skip_next

        for directive in data.directives:
            d = Define(directive)
            exp = '(' + d.expansionList + ')'
            skip_next = False
            for arg in d.args:
                if skip_next:
                    _, skip_next = find_string_concat(exp, arg, d.args)
                    continue
                need_check, skip_next = find_string_concat(exp, arg, d.args)
                if not need_check:
                    continue

                pos = 0
                while pos < len(exp):
                    pos = exp.find(arg, pos)
                    if pos < 0:
                        break
                    # is 'arg' used at position pos
                    pos1 = pos - 1
                    pos2 = pos + len(arg)
                    pos = pos2
                    if pos1 >= 0 and (isalnum(exp[pos1]) or exp[pos1] == '_'):
                        continue
                    if pos2 < len(exp) and (isalnum(exp[pos2]) or exp[pos2] == '_'):
                        continue

                    while pos1 >= 0 and exp[pos1] == ' ':
                        pos1 -= 1
                    if exp[pos1] == '#':
                        continue
                    if exp[pos1] not in '([,.':
                        self.reportError(directive, 20, 7)
                        break
                    while pos2 < len(exp) and exp[pos2] == ' ':
                        pos2 += 1
                    if pos2 < len(exp) and exp[pos2] not in ')]#,':
                        self.reportError(directive, 20, 7)
                        break

    def misra_20_10(self, data):
        for directive in data.directives:
            d = Define(directive)
            if d.expansionList.find('#') >= 0:
                self.reportError(directive, 20, 10)

    def misra_20_13(self, data):
        dir_pattern = re.compile(r'#[ ]*([^ (<]*)')
        for directive in data.directives:
            dir = directive.str
            mo = dir_pattern.match(dir)
            if mo:
                dir = mo.group(1)
            if dir not in ['define', 'elif', 'else', 'endif', 'error', 'if', 'ifdef', 'ifndef', 'include',
                           'pragma', 'undef', 'warning']:
                self.reportError(directive, 20, 13)

    def misra_20_14(self, data):
        # stack for #if blocks. contains the #if directive until the corresponding #endif is seen.
        # the size increases when there are inner #if directives.
        ifStack = []
        for directive in data.directives:
            if directive.str.startswith('#if ') or directive.str.startswith('#ifdef ') or directive.str.startswith(
                    '#ifndef '):
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

    def misra_21_1(self, data):
        re_forbidden_macro = re.compile(r'#(?:define|undef) _[_A-Z]+')
        re_macro_name = re.compile(r'#(?:define|undef) (.+)[ $]')

        for d in data.directives:
            # Search for forbidden identifiers
            m = re.search(re_forbidden_macro, d.str)
            if m:
                self.reportError(d, 21, 1)
                continue

            # Search standard library identifiers in macro names
            m = re.search(re_macro_name, d.str)
            if not m:
                continue
            name = m.group(1)
            if isStdLibId(name, data.standards.c):
                self.reportError(d, 21, 1)

    def misra_21_3(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and (token.astOperand1.str in ('malloc', 'calloc', 'realloc', 'free')):
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
            if isFunctionCall(token) and (token.astOperand1.str in ('atof', 'atoi', 'atol', 'atoll')):
                self.reportError(token, 21, 7)

    def misra_21_8(self, data):
        for token in data.tokenlist:
            if isFunctionCall(token) and (token.astOperand1.str in ('abort', 'exit', 'getenv', 'system')):
                self.reportError(token, 21, 8)

    def misra_21_9(self, data):
        for token in data.tokenlist:
            if (token.str in ('bsearch', 'qsort')) and token.next and token.next.str == '(':
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

    def misra_21_12(self, data):
        if findInclude(data.directives, '<fenv.h>'):
            for token in data.tokenlist:
                if token.str == 'fexcept_t' and token.isName:
                    self.reportError(token, 21, 12)
                if isFunctionCall(token) and (token.astOperand1.str in (
                        'feclearexcept',
                        'fegetexceptflag',
                        'feraiseexcept',
                        'fesetexceptflag',
                        'fetestexcept')):
                    self.reportError(token, 21, 12)

    def get_verify_expected(self):
        """Return the list of expected violations in the verify test"""
        return self.verify_expected

    def get_verify_actual(self):
        """Return the list of actual violations in for the verify test"""
        return self.verify_actual

    def get_violations(self, violation_type=None):
        """Return the list of violations for a normal checker run"""
        if violation_type is None:
            return self.violations.items()
        else:
            return self.violations[violation_type]

    def get_violation_types(self):
        """Return the list of violations for a normal checker run"""
        return self.violations.keys()

    def addSuppressedRule(self, ruleNum,
                          fileName=None,
                          lineNumber=None,
                          symbolName=None):
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
        then the rule is assumed to be suppressed for the entire file. Otherwise
        the list contains line number, symbol name tuples.
        For each tuple either line number or symbol name can can be none.

        """
        normalized_filename = None

        if fileName is not None:
            normalized_filename = os.path.expanduser(fileName)
            normalized_filename = os.path.normpath(normalized_filename)

        if lineNumber is not None or symbolName is not None:
            line_symbol = (lineNumber, symbolName)
        else:
            line_symbol = None

        # If the rule is not in the dict already then add it
        if ruleNum not in self.suppressedRules:
            ruleItemList = list()
            ruleItemList.append(line_symbol)

            fileDict = dict()
            fileDict[normalized_filename] = ruleItemList

            self.suppressedRules[ruleNum] = fileDict

            # Rule is added.  Done.
            return

        # Rule existed in the dictionary. Check for
        # filename entries.

        # Get the dictionary for the rule number
        fileDict = self.suppressedRules[ruleNum]

        # If the filename is not in the dict already add it
        if normalized_filename not in fileDict:
            ruleItemList = list()
            ruleItemList.append(line_symbol)

            fileDict[normalized_filename] = ruleItemList

            # Rule is added with a file scope. Done
            return

        # Rule has a matching filename. Get the rule item list.

        # Check the lists of rule items
        # to see if this (lineNumber, symbolName) combination
        # or None already exists.
        ruleItemList = fileDict[normalized_filename]

        if line_symbol is None:
            # is it already in the list?
            if line_symbol not in ruleItemList:
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

    def isRuleSuppressed(self, file_path, linenr, ruleNum):
        """
        Check to see if a rule is suppressed.

        :param ruleNum: is the rule number in hundreds format
        :param file_path: File path of checked location
        :param linenr: Line number of checked location

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

        # Remove any prefix listed in command arguments from the filename.
        filename = None
        if file_path is not None:
            if self.filePrefix is not None:
                filename = remove_file_prefix(file_path, self.filePrefix)
            else:
                filename = os.path.basename(file_path)

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

    def isRuleGloballySuppressed(self, rule_num):
        """
        Check to see if a rule is globally suppressed.
        :param rule_num: is the rule number in hundreds format
        """
        if rule_num not in self.suppressedRules:
            return False
        return None in self.suppressedRules[rule_num]

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
                linenr = None
                if each.lineNumber:
                    linenr = int(each.lineNumber)
                self.addSuppressedRule(ruleNum, each.fileName, linenr, each.symbolName)

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

                    outlist.append("%s: %s: %s (%d locations suppressed)" % (
                    float(ruleNum) / 100, fname, item_str, self.suppressionStats.get(ruleNum, 0)))

        for line in sorted(outlist, reverse=True):
            print("  %s" % line)

    def setFilePrefix(self, prefix):
        """
        Set the file prefix to ignore from files when matching
        suppression files
        """
        self.filePrefix = prefix

    def setSeverity(self, severity):
        """
        Set the severity for all errors.
        """
        self.severity = severity

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
                ruleNum = (num1 * 100) + num2

                self.addSuppressedRule(ruleNum)

    def reportError(self, location, num1, num2):
        ruleNum = num1 * 100 + num2

        if self.settings.verify:
            self.verify_actual.append(str(location.linenr) + ':' + str(num1) + '.' + str(num2))
        elif self.isRuleSuppressed(location.file, location.linenr, ruleNum):
            # Error is suppressed. Ignore
            self.suppressionStats.setdefault(ruleNum, 0)
            self.suppressionStats[ruleNum] += 1
            return
        else:
            errorId = 'c2012-' + str(num1) + '.' + str(num2)
            misra_severity = 'Undefined'
            cppcheck_severity = 'style'
            if ruleNum in self.ruleTexts:
                errmsg = self.ruleTexts[ruleNum].text
                if self.ruleTexts[ruleNum].misra_severity:
                    misra_severity = self.ruleTexts[ruleNum].misra_severity
                cppcheck_severity = self.ruleTexts[ruleNum].cppcheck_severity
            elif len(self.ruleTexts) == 0:
                errmsg = 'misra violation (use --rule-texts=<file> to get proper output)'
            else:
                return

            if self.severity:
                cppcheck_severity = self.severity

            this_violation = '{}-{}-{}-{}'.format(location.file, location.linenr, location.column, ruleNum)

            # If this is new violation then record it and show it. If not then
            # skip it since it has already been displayed.
            if not this_violation in self.existing_violations:
                self.existing_violations.add(this_violation)
                cppcheckdata.reportError(location, cppcheck_severity, errmsg, 'misra', errorId, misra_severity)

                if misra_severity not in self.violations:
                    self.violations[misra_severity] = []
                self.violations[misra_severity].append('misra-' + errorId)

    def loadRuleTexts(self, filename):
        num1 = 0
        num2 = 0
        appendixA = False
        ruleText = False
        expect_more = False

        Rule_pattern = re.compile(r'^Rule ([0-9]+).([0-9]+)')
        severity_pattern = re.compile(r'.*[ ]*(Advisory|Required|Mandatory)$')
        xA_Z_pattern = re.compile(r'^[#A-Z].*')
        a_z_pattern = re.compile(r'^[a-z].*')
        # Try to detect the file encoding
        file_stream = None
        encodings = ['ascii', 'utf-8', 'windows-1250', 'windows-1252']
        for e in encodings:
            try:
                file_stream = codecs.open(filename, 'r', encoding=e)
                file_stream.readlines()
                file_stream.seek(0)
            except UnicodeDecodeError:
                file_stream = None
            else:
                break
        if not file_stream:
            print('Could not find a suitable codec for "' + filename + '".')
            print('If you know the codec please report it to the developers so the list can be enhanced.')
            print('Trying with default codec now and ignoring errors if possible ...')
            try:
                file_stream = open(filename, 'rt', errors='ignore')
            except TypeError:
                # Python 2 does not support the errors parameter
                file_stream = open(filename, 'rt')

        rule = None
        have_severity = False
        severity_loc = 0

        for line in file_stream:

            line = line.replace('\r', '').replace('\n', '')

            if not appendixA:
                if line.find('Appendix A') >= 0 and line.find('Summary of guidelines') >= 10:
                    appendixA = True
                continue
            if line.find('Appendix B') >= 0:
                break
            if len(line) == 0:
                continue

            # Parse rule declaration.
            res = Rule_pattern.match(line)

            if res:
                have_severity = False
                expect_more = False
                severity_loc = 0
                num1 = int(res.group(1))
                num2 = int(res.group(2))
                rule = Rule(num1, num2)

            if not have_severity and rule is not None:
                res = severity_pattern.match(line)

                if res:
                    rule.misra_severity = res.group(1)
                    have_severity = True
                else:
                    severity_loc += 1

                # Only look for severity on the Rule line
                # or the next non-blank line after
                # If it's not in either of those locations then
                # assume a severity was not provided.

                if severity_loc < 2:
                    continue
                else:
                    rule.misra_severity = ''
                    have_severity = True

            if rule is None:
                continue

            # Parse continuing of rule text.
            if expect_more:
                if a_z_pattern.match(line):
                    self.ruleTexts[rule.num].text += ' ' + line
                    continue

                expect_more = False
                continue

            # Parse beginning of rule text.
            if xA_Z_pattern.match(line):
                rule.text = line
                self.ruleTexts[rule.num] = rule
                expect_more = True

    def verifyRuleTexts(self):
        """Prints rule numbers without rule text."""
        rule_texts_rules = []
        for rule_num in self.ruleTexts:
            rule = self.ruleTexts[rule_num]
            rule_texts_rules.append(str(rule.num1) + '.' + str(rule.num2))

        all_rules = list(getAddonRules() + getCppcheckRules())

        missing_rules = list(set(all_rules) - set(rule_texts_rules))
        if len(missing_rules) == 0:
            print("Rule texts are correct.")
        else:
            print("Missing rule texts: " + ', '.join(missing_rules))

    def printStatus(self, *args, **kwargs):
        if not self.settings.quiet:
            print(*args, **kwargs)

    def executeCheck(self, rule_num, check_function, *args):
        """Execute check function for a single MISRA rule.

        :param rule_num: Number of rule in hundreds format
        :param check_function: Check function to execute
        :param args: Check function arguments
        """
        if not self.isRuleGloballySuppressed(rule_num):
            check_function(*args)

    def parseDump(self, dumpfile):
        filename = '.'.join(dumpfile.split('.')[:-1])
        data = cppcheckdata.parsedump(dumpfile)

        self.dumpfileSuppressions = data.suppressions
        self.parseSuppressions()

        typeBits['CHAR'] = data.platform.char_bit
        typeBits['SHORT'] = data.platform.short_bit
        typeBits['INT'] = data.platform.int_bit
        typeBits['LONG'] = data.platform.long_bit
        typeBits['LONG_LONG'] = data.platform.long_long_bit
        typeBits['POINTER'] = data.platform.pointer_bit

        if self.settings.verify:
            for tok in data.rawTokens:
                if tok.str.startswith('//') and 'TODO' not in tok.str:
                    compiled = re.compile(r'[0-9]+\.[0-9]+')
                    for word in tok.str[2:].split(' '):
                        if compiled.match(word):
                            self.verify_expected.append(str(tok.linenr) + ':' + word)
        else:
            self.printStatus('Checking ' + dumpfile + '...')

        for cfgNumber, cfg in enumerate(data.iterconfigurations()):
            if not self.settings.quiet:
                self.printStatus('Checking %s, config %s...' % (dumpfile, cfg.name))

            self.executeCheck(207, self.misra_2_7, cfg)
            # data.rawTokens is same for all configurations
            if cfgNumber == 0:
                self.executeCheck(301, self.misra_3_1, data.rawTokens)
                self.executeCheck(302, self.misra_3_2, data.rawTokens)
                self.executeCheck(401, self.misra_4_1, data.rawTokens)
                self.executeCheck(402, self.misra_4_2, data.rawTokens)
            self.executeCheck(501, self.misra_5_1, cfg)
            self.executeCheck(502, self.misra_5_2, cfg)
            self.executeCheck(504, self.misra_5_4, cfg)
            self.executeCheck(505, self.misra_5_5, cfg)
            self.executeCheck(601, self.misra_6_1, cfg)
            self.executeCheck(602, self.misra_6_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(701, self.misra_7_1, data.rawTokens)
                self.executeCheck(703, self.misra_7_3, data.rawTokens)
            self.executeCheck(811, self.misra_8_11, cfg)
            self.executeCheck(812, self.misra_8_12, cfg)
            if cfgNumber == 0:
                self.executeCheck(814, self.misra_8_14, data.rawTokens)
                self.executeCheck(905, self.misra_9_5, data.rawTokens)
            self.executeCheck(1001, self.misra_10_1, cfg)
            self.executeCheck(1004, self.misra_10_4, cfg)
            self.executeCheck(1006, self.misra_10_6, cfg)
            self.executeCheck(1008, self.misra_10_8, cfg)
            self.executeCheck(1103, self.misra_11_3, cfg)
            self.executeCheck(1104, self.misra_11_4, cfg)
            self.executeCheck(1105, self.misra_11_5, cfg)
            self.executeCheck(1106, self.misra_11_6, cfg)
            self.executeCheck(1107, self.misra_11_7, cfg)
            self.executeCheck(1108, self.misra_11_8, cfg)
            self.executeCheck(1109, self.misra_11_9, cfg)
            if cfgNumber == 0:
                self.executeCheck(1201, self.misra_12_1_sizeof, data.rawTokens)
            self.executeCheck(1201, self.misra_12_1, cfg)
            self.executeCheck(1202, self.misra_12_2, cfg)
            self.executeCheck(1203, self.misra_12_3, cfg)
            self.executeCheck(1204, self.misra_12_4, cfg)
            self.executeCheck(1301, self.misra_13_1, cfg)
            self.executeCheck(1303, self.misra_13_3, cfg)
            self.executeCheck(1304, self.misra_13_4, cfg)
            self.executeCheck(1305, self.misra_13_5, cfg)
            self.executeCheck(1306, self.misra_13_6, cfg)
            self.executeCheck(1401, self.misra_14_1, cfg)
            self.executeCheck(1402, self.misra_14_2, cfg)
            self.executeCheck(1404, self.misra_14_4, cfg)
            self.executeCheck(1501, self.misra_15_1, cfg)
            self.executeCheck(1502, self.misra_15_2, cfg)
            self.executeCheck(1503, self.misra_15_3, cfg)
            self.executeCheck(1505, self.misra_15_5, cfg)
            if cfgNumber == 0:
                self.executeCheck(1506, self.misra_15_6, data.rawTokens)
            self.executeCheck(1507, self.misra_15_7, cfg)
            self.executeCheck(1602, self.misra_16_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(1603, self.misra_16_3, data.rawTokens)
            self.executeCheck(1604, self.misra_16_4, cfg)
            self.executeCheck(1605, self.misra_16_5, cfg)
            self.executeCheck(1606, self.misra_16_6, cfg)
            self.executeCheck(1607, self.misra_16_7, cfg)
            self.executeCheck(1701, self.misra_17_1, cfg)
            self.executeCheck(1702, self.misra_17_2, cfg)
            if cfgNumber == 0:
                self.executeCheck(1706, self.misra_17_6, data.rawTokens)
            self.executeCheck(1707, self.misra_17_7, cfg)
            self.executeCheck(1708, self.misra_17_8, cfg)
            self.executeCheck(1804, self.misra_18_4, cfg)
            self.executeCheck(1805, self.misra_18_5, cfg)
            self.executeCheck(1807, self.misra_18_7, cfg)
            self.executeCheck(1808, self.misra_18_8, cfg)
            self.executeCheck(1902, self.misra_19_2, cfg)
            self.executeCheck(2001, self.misra_20_1, cfg)
            self.executeCheck(2002, self.misra_20_2, cfg)
            self.executeCheck(2003, self.misra_20_3, cfg)
            self.executeCheck(2004, self.misra_20_4, cfg)
            self.executeCheck(2005, self.misra_20_5, cfg)
            self.executeCheck(2006, self.misra_20_7, cfg)
            self.executeCheck(2010, self.misra_20_10, cfg)
            self.executeCheck(2013, self.misra_20_13, cfg)
            self.executeCheck(2014, self.misra_20_14, cfg)
            self.executeCheck(2101, self.misra_21_1, cfg)
            self.executeCheck(2103, self.misra_21_3, cfg)
            self.executeCheck(2104, self.misra_21_4, cfg)
            self.executeCheck(2105, self.misra_21_5, cfg)
            self.executeCheck(2106, self.misra_21_6, cfg)
            self.executeCheck(2107, self.misra_21_7, cfg)
            self.executeCheck(2108, self.misra_21_8, cfg)
            self.executeCheck(2109, self.misra_21_9, cfg)
            self.executeCheck(2110, self.misra_21_10, cfg)
            self.executeCheck(2111, self.misra_21_11, cfg)
            self.executeCheck(2112, self.misra_21_12, cfg)
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


def get_args():
    """Generates list of command-line arguments acceptable by misra.py script."""
    parser = cppcheckdata.ArgumentParser()
    parser.add_argument("--rule-texts", type=str, help=RULE_TEXTS_HELP)
    parser.add_argument("--verify-rule-texts",
                        help="Verify that all supported rules texts are present in given file and exit.",
                        action="store_true")
    parser.add_argument("--suppress-rules", type=str, help=SUPPRESS_RULES_HELP)
    parser.add_argument("--no-summary", help="Hide summary of violations", action="store_true")
    parser.add_argument("--show-suppressed-rules", help="Print rule suppression list", action="store_true")
    parser.add_argument("-P", "--file-prefix", type=str, help="Prefix to strip when matching suppression file rules")
    parser.add_argument("-generate-table", help=argparse.SUPPRESS, action="store_true")
    parser.add_argument("-verify", help=argparse.SUPPRESS, action="store_true")
    parser.add_argument("--severity", type=str, help="Set a custom severity string, for example 'error' or 'warning'. ")
    return parser.parse_args()


def main():
    args = get_args()
    settings = MisraSettings(args)
    checker = MisraChecker(settings)

    if args.generate_table:
        generateTable()
        sys.exit(0)

    if args.rule_texts:
        filename = os.path.expanduser(args.rule_texts)
        filename = os.path.normpath(filename)
        if not os.path.isfile(filename):
            print('Fatal error: file is not found: ' + filename)
            sys.exit(1)
        checker.loadRuleTexts(filename)
        if args.verify_rule_texts:
            checker.verifyRuleTexts()
            sys.exit(0)

    if args.verify_rule_texts and not args.rule_texts:
        print("Error: Please specify rule texts file with --rule-texts=<file>")
        sys.exit(1)

    if args.suppress_rules:
        checker.setSuppressionList(args.suppress_rules)

    if args.file_prefix:
        checker.setFilePrefix(args.file_prefix)

    if not args.dumpfile:
        if not args.quiet:
            print("No input files.")
        sys.exit(0)

    if args.severity:
        checker.setSeverity(args.severity)

    for item in args.dumpfile:
        checker.parseDump(item)

        if settings.verify:
            verify_expected = checker.get_verify_expected()
            verify_actual = checker.get_verify_actual()

            exitCode = 0
            for expected in verify_expected:
                if expected not in verify_actual:
                    print('Expected but not seen: ' + expected)
                    exitCode = 1
            for actual in verify_actual:
                if actual not in verify_expected:
                    print('Not expected: ' + actual)
                    exitCode = 1

            # Existing behavior of verify mode is to exit
            # on the first un-expected output.
            # TODO: Is this required? or can it be moved to after
            # all input files have been processed
            if exitCode != 0:
                sys.exit(exitCode)

    if settings.verify:
        sys.exit(exitCode)

    number_of_violations = len(checker.get_violations())
    if number_of_violations > 0:
        if settings.show_summary:
            print("\nMISRA rules violations found:\n\t%s\n" % (
                "\n\t".join(["%s: %d" % (viol, len(checker.get_violations(viol))) for viol in
                             checker.get_violation_types()])))

            rules_violated = {}
            for severity, ids in checker.get_violations():
                for misra_id in ids:
                    rules_violated[misra_id] = rules_violated.get(misra_id, 0) + 1
            print("MISRA rules violated:")
            convert = lambda text: int(text) if text.isdigit() else text
            misra_sort = lambda key: [convert(c) for c in re.split(r'[\.-]([0-9]*)', key)]
            for misra_id in sorted(rules_violated.keys(), key=misra_sort):
                res = re.match(r'misra-c2012-([0-9]+)\\.([0-9]+)', misra_id)
                if res is None:
                    num = 0
                else:
                    num = int(res.group(1)) * 100 + int(res.group(2))
                severity = '-'
                if num in checker.ruleTexts:
                    severity = checker.ruleTexts[num].cppcheck_severity
                print("\t%15s (%s): %d" % (misra_id, severity, rules_violated[misra_id]))

    if args.show_suppressed_rules:
        checker.showSuppressedRules()


if __name__ == '__main__':
    main()
    sys.exit(cppcheckdata.EXIT_CODE)
