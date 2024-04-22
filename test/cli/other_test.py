
# python -m pytest test-other.py

import os
import sys
import pytest
import json

from testutils import cppcheck, assert_cppcheck


def test_missing_include(tmpdir):  # #11283
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    args = ['--enable=missingInclude', '--template=simple', test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:2:0: information: Include file: "test.h" not found. [missingInclude]\n'.format(test_file)


def __test_missing_include_check_config(tmpdir, use_j):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    # TODO: -rp is not working requiring the full path in the assert
    args = '--check-config -rp={} {}'.format(tmpdir, test_file)
    if use_j:
        args = '-j2 ' + args

    _, _, stderr = cppcheck(args.split())
    assert stderr == '' # --check-config no longer reports the missing includes


def test_missing_include_check_config(tmpdir):
    __test_missing_include_check_config(tmpdir, False)


def test_missing_include_check_config_j(tmpdir):
    __test_missing_include_check_config(tmpdir, True)


def test_missing_include_inline_suppr(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                // cppcheck-suppress missingInclude
                #include "missing.h"
                // cppcheck-suppress missingIncludeSystem
                #include <missing2.h>
                """)

    args = ['--enable=missingInclude', '--inline-suppr', test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == ''


def test_preprocessor_error(tmpdir):
    test_file = os.path.join(tmpdir, '10866.c')
    with open(test_file, 'wt') as f:
        f.write('#error test\nx=1;\n')
    exitcode, _, stderr = cppcheck(['--error-exitcode=1', test_file])
    assert 'preprocessorErrorDirective' in stderr
    assert exitcode != 0


def test_invalid_library(tmpdir):
    args = ['--library=none', '--library=posix', '--library=none2', 'file.c']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    assert (stdout == "cppcheck: Failed to load library configuration file 'none'. File not found\n"
                      "cppcheck: Failed to load library configuration file 'none2'. File not found\n")
    assert stderr == ""


def test_message_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("")

    args = ['-j2', test_file]

    _, stdout, _ = cppcheck(args)
    assert stdout == "Checking {} ...\n".format(test_file) # we were adding stray \0 characters at the end

# TODO: test missing std.cfg


def test_progress(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    pos = stdout.find('\n')
    assert(pos != -1)
    pos += 1
    assert stdout[:pos] == "Checking {} ...\n".format(test_file)
    assert (stdout[pos:] ==
            "progress: Tokenize (typedef) 0%\n"
            "progress: Tokenize (typedef) 12%\n"
            "progress: Tokenize (typedef) 25%\n"
            "progress: Tokenize (typedef) 37%\n"
            "progress: Tokenize (typedef) 50%\n"
            "progress: Tokenize (typedef) 62%\n"
            "progress: Tokenize (typedef) 75%\n"
            "progress: Tokenize (typedef) 87%\n"
            "progress: SymbolDatabase 0%\n"
            "progress: SymbolDatabase 12%\n"
            "progress: SymbolDatabase 87%\n"
            )
    assert stderr == ""


def test_progress_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '-j2', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout == "Checking {} ...\n".format(test_file)
    assert stderr == ""


def test_execute_addon_failure(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    args = ['--addon=naming', test_file]

    # provide empty PATH environment variable so python is not found and execution of addon fails
    env = {'PATH': ''}
    _, _, stderr = cppcheck(args, env)
    assert stderr == '{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to auto detect python [internalError]\n\n^\n'.format(test_file)


def test_execute_addon_failure_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    # specify non-existent python executbale so execution of addon fails
    args = ['--addon=naming', '--addon-python=python5.x', test_file]

    _, _, stderr = cppcheck(args)
    ec = 1 if os.name == 'nt' else 127
    assert stderr == "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'naming' - exitcode is {} [internalError]\n\n^\n".format(test_file, ec)


def test_execute_addon_file0(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('void foo() {}\n')

    args = ['--xml', '--addon=misra', '--enable=style', test_file]

    _, _, stderr = cppcheck(args)
    assert 'misra-c2012-8.2' in stderr
    assert '.dump' not in stderr


# TODO: find a test case which always fails
@pytest.mark.skip
def test_internal_error(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#include <cstdio>

void f() {
    double gc = 3333.3333;
    char stat[80];
    sprintf(stat,"'%2.1f'",gc);
}
                """)

    args = [test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:0:0: error: Bailing from out analysis: Checking file failed: converting \'1f\' to integer failed - not an integer [internalError]\n\n^\n'.format(test_file)


def test_addon_ctu_exitcode(tmpdir):
    """ #12440 - Misra ctu violations found => exit code should be non-zero """
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""typedef enum { BLOCK =  0x80U, } E;""")
    args = ['--addon=misra', '--enable=style', '--error-exitcode=1', test_file]
    exitcode, stdout, stderr = cppcheck(args)
    assert '2.3' in stderr, stderr
    assert exitcode == 1


# TODO: test with -j2
def test_addon_misra(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
        """)

    args = ['--addon=misra', '--enable=all', '--disable=unusedFunction', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:2:1: style: misra violation (use --rule-texts=<file> to get proper output) [misra-c2012-2.3]\ntypedef int MISRA_5_6_VIOLATION;\n^\n'.format(test_file)


def test_addon_y2038(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    # TODO: trigger warning
    with open(test_file, 'wt') as f:
        f.write("""
extern void f()
{
    time_t t = std::time(nullptr);
    (void)t;
}
        """)

    args = ['--addon=y2038', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:21: warning: time is Y2038-unsafe [y2038-unsafe-call]\n'.format(test_file)


def test_addon_threadsafety(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
extern const char* f()
{
    return strerror(1);
}
        """)

    args = ['--addon=threadsafety', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:12: warning: strerror is MT-unsafe [threadsafety-unsafe-call]\n'.format(test_file)


def test_addon_naming(tmpdir):
    # the addon does nothing without a config
    addon_file = os.path.join(tmpdir, 'naming1.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/naming.py",
    "args": [
        "--var=[_a-z].*"
    ]
}
                """)

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
int Var;
        """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:2:1: style: Variable Var violates naming convention [naming-varname]\n'.format(test_file)


def test_addon_namingng(tmpdir):
    addon_file = os.path.join(tmpdir, 'namingng.json')
    addon_config_file = os.path.join(tmpdir, 'namingng.config.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/namingng.py",
    "args": [
        "--configfile=%s"
    ]
}
                """%(addon_config_file).replace('\\','\\\\'))

    with open(addon_config_file, 'wt') as f:
        f.write("""
{
    "RE_FILE": [
        "[^/]*[a-z][a-z0-9_]*[a-z0-9]\\.c\\Z"
    ],
    "RE_CLASS_NAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_NAMESPACE": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_VARNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_PUBLIC_MEMBER_VARIABLE": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_PRIVATE_MEMBER_VARIABLE": {
        ".*_tmp\\Z":[true,"illegal suffix _tmp"],
        "priv_.*\\Z":[false,"required prefix priv_ missing"]
    },
    "RE_GLOBAL_VARNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "RE_FUNCTIONNAME": ["[a-z][a-z0-9_]*[a-z0-9]\\Z"],
    "include_guard": {
        "input": "basename",
        "prefix": "_",
        "suffix": "",
        "case": "upper",
        "max_linenr": 5,
        "RE_HEADERFILE": ".*\\.h\\Z",
        "required": true
    },
    "var_prefixes": {"uint32_t": "ui32"},
    "function_prefixes": {"uint16_t": "ui16",
                          "uint32_t": "ui32"},
    "skip_one_char_variables": false
}
                """.replace('\\','\\\\'))

    test_unguarded_include_file_basename = 'test_unguarded.h'
    test_unguarded_include_file = os.path.join(tmpdir, test_unguarded_include_file_basename)
    with open(test_unguarded_include_file, 'wt') as f:
        f.write("""
void InvalidFunctionUnguarded();
""")

    test_include_file_basename = '_test.h'
    test_include_file = os.path.join(tmpdir, test_include_file_basename)
    with open(test_include_file, 'wt') as f:
        f.write("""
#ifndef TEST_H
#define TEST_H

void InvalidFunction();
extern int _invalid_extern_global;

#include "{}"

#endif
""".format(test_unguarded_include_file))

    test_file_basename = 'test_.cpp'
    test_file = os.path.join(tmpdir, test_file_basename)
    with open(test_file, 'wt') as f:
        f.write("""
#include "%s"

void invalid_function_();
void _invalid_function();
void valid_function1();
void valid_function2(int _invalid_arg);
void valid_function3(int invalid_arg_);
void valid_function4(int valid_arg32);
void valid_function5(uint32_t invalid_arg32);
void valid_function6(uint32_t ui32_valid_arg);
uint16_t invalid_function7(int valid_arg);
uint16_t ui16_valid_function8(int valid_arg);

int _invalid_global;
static int _invalid_static_global;

class _clz {
public:
    _clz() : _invalid_public(0), _invalid_private(0), priv_good(0), priv_bad_tmp(0) { }
    int _invalid_public;
private:
    char _invalid_private;
    int priv_good;
    int priv_bad_tmp;
};

namespace _invalid_namespace { }
        """%(test_include_file_basename))

    args = ['--addon='+addon_file, '--verbose', '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Defines:',
        'Undefines:',
        'Includes:',
        'Platform:native'
    ]
    lines = [line for line in stderr.splitlines() if line != '']
    expect = [
        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_include_file,test_include_file_basename),
        '^',
        '{}:2:9: style: include guard naming violation; TEST_H != _TEST_H [namingng-includeGuardName]'.format(test_include_file),
        '#ifndef TEST_H',
        '        ^',
        '{}:5:6: style: Function InvalidFunction violates naming convention [namingng-namingConvention]'.format(test_include_file),
        'void InvalidFunction();',
        '     ^',
        '{}:6:12: style: Global variable _invalid_extern_global violates naming convention [namingng-namingConvention]'.format(test_include_file),
        'extern int _invalid_extern_global;',
        '           ^',

        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_unguarded_include_file,test_unguarded_include_file_basename),
        '^',
        '{}:0:0: style: Missing include guard [namingng-includeGuardMissing]'.format(test_unguarded_include_file),
        '^',
        '{}:2:6: style: Function InvalidFunctionUnguarded violates naming convention [namingng-namingConvention]'.format(test_unguarded_include_file),
        'void InvalidFunctionUnguarded();',
        '     ^',

        '{}:0:0: style: File name {} violates naming convention [namingng-namingConvention]'.format(test_file,test_file_basename),
        '^',
        '{}:7:26: style: Variable _invalid_arg violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function2(int _invalid_arg);',
        '                         ^',
        '{}:8:26: style: Variable invalid_arg_ violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function3(int invalid_arg_);',
        '                         ^',
        '{}:10:31: style: Variable invalid_arg32 violates naming convention [namingng-namingConvention]'.format(test_file),
        'void valid_function5(uint32_t invalid_arg32);',
        '                              ^',
        '{}:4:6: style: Function invalid_function_ violates naming convention [namingng-namingConvention]'.format(test_file),
        'void invalid_function_();',
        '     ^',
        '{}:5:6: style: Function _invalid_function violates naming convention [namingng-namingConvention]'.format(test_file),
        'void _invalid_function();',
        '     ^',
        '{}:12:10: style: Function invalid_function7 violates naming convention [namingng-namingConvention]'.format(test_file),
        'uint16_t invalid_function7(int valid_arg);',
        '         ^',
        '{}:15:5: style: Global variable _invalid_global violates naming convention [namingng-namingConvention]'.format(test_file),
        'int _invalid_global;',
        '    ^',
        '{}:16:12: style: Global variable _invalid_static_global violates naming convention [namingng-namingConvention]'.format(test_file),
        'static int _invalid_static_global;',
        '           ^',
        '{}:20:5: style: Class Constructor _clz violates naming convention [namingng-namingConvention]'.format(test_file),
        '    _clz() : _invalid_public(0), _invalid_private(0), priv_good(0), priv_bad_tmp(0) { }',
        '    ^',
        '{}:21:9: style: Public member variable _invalid_public violates naming convention [namingng-namingConvention]'.format(test_file),
        '    int _invalid_public;',
        '        ^',
        '{}:23:10: style: Private member variable _invalid_private violates naming convention: required prefix priv_ missing [namingng-namingConvention]'.format(test_file),
        '    char _invalid_private;',
        '         ^',
        '{}:25:9: style: Private member variable priv_bad_tmp violates naming convention: illegal suffix _tmp [namingng-namingConvention]'.format(test_file),
        '    int priv_bad_tmp;',
        '        ^',
        '{}:28:11: style: Namespace _invalid_namespace violates naming convention [namingng-namingConvention]'.format(test_file),
        'namespace _invalid_namespace { }',
        '          ^',
    ]
    # test sorted lines; the order of messages may vary and is not of importance
    lines.sort()
    expect.sort()
    assert lines == expect


# TODO: test with -j2
def test_addon_namingng_config(tmpdir):
    addon_file = os.path.join(tmpdir, 'namingng.json')
    addon_config_file = os.path.join(tmpdir, 'namingng.config.json')
    with open(addon_file, 'wt') as f:
        f.write("""
{
    "script": "addons/namingng.py",
    "args": [
        "--configfile=%s"
    ]
}
                """%(addon_config_file).replace('\\','\\\\'))

    with open(addon_config_file, 'wt') as f:
        f.write("""
{
    "RE_FILE": "[^/]*[a-z][a-z0-9_]*[a-z0-9]\\.c\\Z",
    "RE_NAMESPACE": false,
    "RE_VARNAME": ["+bad pattern","[a-z]_good_pattern\\Z","(parentheses?"],
    "RE_PRIVATE_MEMBER_VARIABLE": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_PUBLIC_MEMBER_VARIABLE": {
        "tmp_.*\\Z":[true,"illegal prefix tmp_"],
        "bad_.*\\Z":true,
        "public_.*\\Z":[false],
        "pub_.*\\Z":[0,"required prefix pub_ missing"]
    },
    "RE_GLOBAL_VARNAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_FUNCTIONNAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "RE_CLASS_NAME": "[a-z][a-z0-9_]*[a-z0-9]\\Z",
    "_comment1": "these should all be arrays, or null, or not set",

    "include_guard": true,
    "var_prefixes": ["bad"],
    "function_prefixes": false,
    "_comment2": "these should all be dict",

    "skip_one_char_variables": "false",
    "_comment3": "this should be bool",

    "RE_VAR_NAME": "typo"
}
                """.replace('\\','\\\\'))

    test_file_basename = 'test.c'
    test_file = os.path.join(tmpdir, test_file_basename)
    with open(test_file, 'a') as f:
        # only create the file
        pass

    args = ['--addon='+addon_file, '--verbose', '--enable=all', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0

    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Defines:',
        'Undefines:',
        'Includes:',
        'Platform:native'
    ]
    lines = stderr.splitlines()
    # ignore the first line, stating that the addon failed to run properly
    lines.pop(0)
    assert lines == [
        "Output:",
        "config error: RE_FILE must be list (not str), or not set",
        "config error: RE_NAMESPACE must be list or dict (not bool), or not set",
        "config error: include_guard must be dict (not bool), or not set",
        "config error: item '+bad pattern' of 'RE_VARNAME' is not a valid regular expression: nothing to repeat at position 0",
        "config error: item '(parentheses?' of 'RE_VARNAME' is not a valid regular expression: missing ), unterminated subpattern at position 0",
        "config error: var_prefixes must be dict (not list), or not set",
        "config error: RE_PRIVATE_MEMBER_VARIABLE must be list or dict (not str), or not set",
        "config error: item 'bad_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: item 'public_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: item 'pub_.*\\Z' of 'RE_PUBLIC_MEMBER_VARIABLE' must be an array [bool,string]",
        "config error: RE_GLOBAL_VARNAME must be list or dict (not str), or not set",
        "config error: RE_FUNCTIONNAME must be list or dict (not str), or not set",
        "config error: function_prefixes must be dict (not bool), or not set",
        "config error: RE_CLASS_NAME must be list or dict (not str), or not set",
        "config error: skip_one_char_variables must be bool (not str), or not set",
        "config error: unknown config key 'RE_VAR_NAME' [internalError]",
        "",
        "^",
    ]


def test_addon_findcasts(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
        extern void f(char c)
        {
            int i = (int)c;
            (void)i;
        }
        """)

    args = ['--addon=findcasts', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:21: information: found a cast [findcasts-cast]\n'.format(test_file)


def test_addon_misc(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
extern void f()
{
    char char* [] = {"a" "b"}
}
        """)

    args = ['--addon=misc', '--enable=all', '--disable=unusedFunction', '--template=simple', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == '{}:4:26: style: String concatenation in array initialization, missing comma? [misc-stringConcatInArrayInit]\n'.format(test_file)


def test_invalid_addon_json(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.json')
    with open(addon_file, 'wt') as f:
        f.write("""
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--addon={}'.format(addon_file), test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    assert lines == [
        'Loading {} failed. syntax error at line 2 near: '.format(addon_file)
    ]
    assert stderr == ''


def test_invalid_addon_py(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
raise Exception()
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == "{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1 [internalError]\n\n^\n".format(test_file)


# TODO: test with -j2
def test_invalid_addon_py_verbose(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
raise Exception()
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', '--verbose', '-j1', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Defines:',
        'Undefines:',
        'Includes:',
        'Platform:native'
    ]
    """
/tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/file.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1: python3 /home/user/CLionProjects/cppcheck/addons/runaddon.py /tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/addon1.py --cli /tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/file.cpp.24762.dump
Output:
Traceback (most recent call last):
  File "/home/user/CLionProjects/cppcheck/addons/runaddon.py", line 8, in <module>
    runpy.run_path(addon, run_name='__main__')
  File "<frozen runpy>", line 291, in run_path
  File "<frozen runpy>", line 98, in _run_module_code
  File "<frozen runpy>", line 88, in _run_code
  File "/tmp/pytest-of-user/pytest-11/test_invalid_addon_py_20/addon1.py", line 2, in <module>
    raise Exception()
Exceptio [internalError]
    """
    # /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/file.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 256.: python3 /home/user/CLionProjects/cppcheck/addons/runaddon.py /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/addon1.py --cli /tmp/pytest-of-user/pytest-10/test_invalid_addon_py_20/file.cpp.24637.dump
    assert stderr.startswith("{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'addon1' - exitcode is 1: ".format(test_file))
    assert stderr.count('Output:\nTraceback')
    assert stderr.endswith('raise Exception()\nException [internalError]\n\n^\n')


def test_addon_result(tmpdir):
    addon_file = os.path.join(tmpdir, 'addon1.py')
    with open(addon_file, 'wt') as f:
        f.write("""
print("Checking ...")
print("")
print('{"file": "test.cpp", "linenr": 1, "column": 1, "severity": "style", "message": "msg", "addon": "addon1", "errorId": "id", "extra": ""}')
print('{"loc": [{"file": "test.cpp", "linenr": 1, "column": 1, "info": ""}], "severity": "style", "message": "msg", "addon": "addon1", "errorId": "id", "extra": ""}')
                """)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
typedef int MISRA_5_6_VIOLATION;
                """)

    args = ['--addon={}'.format(addon_file), '--enable=all', '--disable=unusedFunction', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == 'test.cpp:1:1: style: msg [addon1-id]\n\n^\n'


# TODO: test with -j2
# #11483
def test_unused_function_include(tmpdir):
    test_cpp_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_cpp_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    test_h_file = os.path.join(tmpdir, 'test.h')
    with open(test_h_file, 'wt') as f:
        f.write("""
                class A {
                public:
                    void f() {}
                    // cppcheck-suppress unusedFunction
                    void f2() {}
                };
                """)

    args = ['--enable=unusedFunction', '--inline-suppr', '--template=simple', '-j1', test_cpp_file]

    _, _, stderr = cppcheck(args)
    assert stderr == "{}:4:0: style: The function 'f' is never used. [unusedFunction]\n".format(test_h_file)


# TODO: test with all other types
def test_showtime_top5_file(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--showtime=top5_file', '--quiet', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0  # TODO: needs to be 1
    lines = stdout.splitlines()
    assert len(lines) == 7
    assert lines[0] == ''
    for i in range(1, 5):
        if lines[i].startswith('valueFlowLifetime'):
            assert lines[i].endswith(' - 2 result(s))')
        elif lines[i].startswith('valueFlowEnumValue'):
            assert lines[i].endswith(' - 2 result(s))')
        else:
            assert lines[i].endswith(' - 1 result(s))')
    assert lines[6].startswith('Overall time:')
    assert stderr == ''


def test_missing_addon(tmpdir):
    args = ['--addon=misra3', '--addon=misra', '--addon=misra2', 'file.c']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    lines.sort()
    assert lines == [
        'Did not find addon misra2.py',
        'Did not find addon misra3.py'
    ]
    assert stderr == ""


def test_file_filter(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    args = ['--file-filter=*.cpp', test_file]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_2(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    args = ['--file-filter=*.cpp', test_file_1, test_file_2]
    out_lines = [
        'Checking {} ...'.format(test_file_1)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_3(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    args = ['--file-filter=*.c', test_file_1, test_file_2]
    out_lines = [
        'Checking {} ...'.format(test_file_2)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_file_filter_no_match(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--file-filter=*.c', test_file]
    out_lines = [
        'cppcheck: error: could not find any files matching the filter.'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_file_order(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass
    test_file_d = os.path.join(tmpdir, 'd.c')
    with open(test_file_d, 'wt'):
        pass

    args = [test_file_c, test_file_d, test_file_b, test_file_a, '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/4 files checked 0% done',
        'Checking {} ...'.format(test_file_d),
        '2/4 files checked 0% done',
        'Checking {} ...'.format(test_file_b),
        '3/4 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '4/4 files checked 0% done'
    ]
    assert stderr == ''


def test_markup(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test_1.qml')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test_2.cpp')
    with open(test_file_2, 'wt') as f:
        pass
    test_file_3 = os.path.join(tmpdir, 'test_3.qml')
    with open(test_file_3, 'wt') as f:
        pass
    test_file_4 = os.path.join(tmpdir, 'test_4.cpp')
    with open(test_file_4, 'wt') as f:
        pass

    args = ['--library=qt', test_file_1, test_file_2, test_file_3, test_file_4, '-j1']
    out_lines = [
        'Checking {} ...'.format(test_file_2),
        '1/4 files checked 0% done',
        'Checking {} ...'.format(test_file_4),
        '2/4 files checked 0% done',
        'Checking {} ...'.format(test_file_1),
        '3/4 files checked 0% done',
        'Checking {} ...'.format(test_file_3),
        '4/4 files checked 0% done'
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_markup_j(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test_1.qml')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test_2.cpp')
    with open(test_file_2, 'wt') as f:
        pass
    test_file_3 = os.path.join(tmpdir, 'test_3.qml')
    with open(test_file_3, 'wt') as f:
        pass
    test_file_4 = os.path.join(tmpdir, 'test_4.cpp')
    with open(test_file_4, 'wt') as f:
        pass

    args = ['--library=qt', '-j2', test_file_1, test_file_2, test_file_3, test_file_4]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    for i in range(1, 5):
        lines.remove('{}/4 files checked 0% done'.format(i))

    # this test started to fail in the -j2 injection run when using ThreadExecutor although it always specifies -j2.
    # the order of the files in the output changed so just check for the file extentions
    assert len(lines) == 4
    assert lines[0].endswith('.cpp ...')
    assert lines[1].endswith('.cpp ...')
    assert lines[2].endswith('.qml ...')
    assert lines[3].endswith('.qml ...')

    #assert lines == [
    #    'Checking {} ...'.format(test_file_2),
    #    'Checking {} ...'.format(test_file_4),
    #    'Checking {} ...'.format(test_file_1),
    #    'Checking {} ...'.format(test_file_3)
    #]
    assert stderr == ''


def test_valueflow_debug(tmpdir):
    test_file_cpp = os.path.join(tmpdir, 'test_1.cpp')
    with open(test_file_cpp, 'wt') as f:
        f.write("""
#include "test.h"

void f()
{
    int i = 0;
}
"""
                )
    test_file_h = os.path.join(tmpdir, 'test.h')
    with open(test_file_h, 'wt') as f:
        f.write("""
#include "test2.h"
inline void f1()
{
    int i = 0;
}
"""
                )
        pass
    test_file_h_2 = os.path.join(tmpdir, 'test2.h')
    with open(test_file_h_2, 'wt') as f:
        f.write("""
inline void f2()
{
    int i = 0;
}
"""
                )

    args = ['--debug', test_file_cpp]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    if sys.platform == "win32":
        stdout = stdout.replace('/', '\\')
    assert stdout == '''Checking {} ...


##file {}
2: void f2 ( )
3: {{
4: int i@var1 ; i@var1 = 0 ;
5: }}

##file {}

1:
2:
3: void f1 ( )
4: {{
5: int i@var2 ; i@var2 = 0 ;
6: }}

##file {}

1:
2:
3:
4: void f ( )
5: {{
6: int i@var3 ; i@var3 = 0 ;
7: }}



##Value flow
File {}
Line 4
  = always 0
  0 always 0
File {}
Line 5
  = always 0
  0 always 0
File {}
Line 6
  = always 0
  0 always 0
'''.format(test_file_cpp, test_file_h_2, test_file_h, test_file_cpp, test_file_h_2, test_file_h, test_file_cpp)
    assert stderr == ''


def test_file_duplicate(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    args = [test_file_a, test_file_a, str(tmpdir)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''


def test_file_duplicate_2(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass

    args = [test_file_c, test_file_a, test_file_b, str(tmpdir), test_file_b, test_file_c, test_file_a, str(tmpdir), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/3 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '2/3 files checked 0% done',
        'Checking {} ...'.format(test_file_b),
        '3/3 files checked 0% done'
    ]
    assert stderr == ''


def test_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['-itest.cpp', test_file]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_build_dir_j_memleak(tmpdir): #12111
    build_dir = os.path.join(tmpdir, 'build-dir')
    os.mkdir(build_dir)

    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write('int main() {}')

    args = ['--cppcheck-build-dir={}'.format(build_dir), '-j2', test_file]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def __test_addon_json_invalid(tmpdir, addon_json, expected):
    addon_file = os.path.join(tmpdir, 'invalid.json')
    with open(addon_file, 'wt') as f:
        f.write(addon_json)

    test_file = os.path.join(tmpdir, 'file.cpp')
    with open(test_file, 'wt'):
        pass

    args = ['--addon={}'.format(addon_file), test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    lines = stdout.splitlines()
    assert len(lines) == 1
    assert lines == [
        'Loading {} failed. {}'.format(addon_file, expected)
    ]
    assert stderr == ''


def test_addon_json_invalid_no_obj(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps([]), "JSON is not an object.")


def test_addon_json_invalid_args_1(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'args':0}), "'args' must be an array.")


def test_addon_json_invalid_args_2(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'args':[0]}), "'args' entry is not a string.")


def test_addon_json_invalid_ctu(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'ctu':0}), "'ctu' must be a boolean.")


def test_addon_json_invalid_python(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'python':0}), "'python' must be a string.")


def test_addon_json_invalid_executable(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'executable':0}), "'executable' must be a string.")


def test_addon_json_invalid_script_1(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'Script':''}), "'script' is missing.")


def test_addon_json_invalid_script_2(tmpdir):
    __test_addon_json_invalid(tmpdir, json.dumps({'script':0}), "'script' must be a string.")


def test_unknown_extension(tmpdir):
    test_file = os.path.join(tmpdir, 'test_2')
    with open(test_file, 'wt') as f:
        f.write('''
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['-q', test_file])
    assert exitcode == 0, stderr
    assert stdout == ''
    assert stderr == ''


def test_rule_file_define_multiple(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rules>
    <rule>
        <tokenlist>define</tokenlist>
        <pattern>DEF_1</pattern>
        <message>
            <severity>error</severity>
            <id>ruleId1</id>
        </message>
    </rule>
    <rule>
        <tokenlist>define</tokenlist>
        <pattern>DEF_2</pattern>
        <message>
            <severity>error</severity>
            <id>ruleId2</id>
            <summary>define2</summary>
        </message>
    </rule>
</rules>""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), '-DDEF_3', test_file])
    assert exitcode == 0, stderr
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: DEF_1',
        'Processing rule: DEF_2',
        'Checking {}: DEF_3=1...'.format(test_file)
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:2:0: error: found 'DEF_1' [ruleId1]".format(test_file),
        "{}:3:0: error: define2 [ruleId2]".format(test_file)
    ]


def test_rule_file_define(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <tokenlist>define</tokenlist>
    <pattern>DEF_.</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), '-DDEF_3', test_file])
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: DEF_.',
        'Checking {}: DEF_3=1...'.format(test_file)
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:2:0: style: found 'DEF_1' [rule]".format(test_file),
        "{}:3:0: style: found 'DEF_2' [rule]".format(test_file)
    ]


def test_rule_file_normal(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <pattern>int</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
typedef int i32;
void f(i32) { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), test_file])
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: int',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:5:0: style: found 'int' [rule]".format(test_file)
    ]


def test_rule_file_raw(tmpdir):
    rule_file = os.path.join(tmpdir, 'rule_file.xml')
    with open(rule_file, 'wt') as f:
        f.write("""
<rule>
    <tokenlist>raw</tokenlist>
    <pattern>i32</pattern>
</rule>
""")

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
typedef int i32;
void f(i32) { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule-file={}'.format(rule_file), test_file])
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: i32',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:4:0: style: found 'i32' [rule]".format(test_file),
        "{}:5:0: style: found 'i32' [rule]".format(test_file)
    ]


def test_rule(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write('''
#define DEF_1
#define DEF_2
void f() { }
''')

    exitcode, stdout, stderr = cppcheck(['--template=simple', '--rule=f', test_file])
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file),
        'Processing rule: f',
    ]
    lines = stderr.splitlines()
    assert lines == [
        "{}:4:0: style: found 'f' [rule]".format(test_file)
    ]


def test_filelist(tmpdir):
    list_dir = os.path.join(tmpdir, 'list-dir')
    os.mkdir(list_dir)

    with open(os.path.join(list_dir, 'aaa.c'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'zzz.c'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'valueflow.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vfvalue.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vf_enumvalue.cpp'), 'wt'):
        pass
    with open(os.path.join(list_dir, 'vf_analyze.h'), 'wt'):
        pass

    sub_dir_1 = os.path.join(list_dir, 'valueflow')
    os.mkdir(sub_dir_1)
    with open(os.path.join(sub_dir_1, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_1, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_1, 'file.h'), 'wt'):
        pass

    sub_dir_2 = os.path.join(list_dir, 'vfvalue')
    os.mkdir(sub_dir_2)
    with open(os.path.join(sub_dir_2, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_2, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_2, 'file.h'), 'wt'):
        pass

    sub_dir_3 = os.path.join(list_dir, 'vf_enumvalue')
    os.mkdir(sub_dir_3)
    with open(os.path.join(sub_dir_3, 'file.cpp'), 'wt'):
        pass
    with open(os.path.join(sub_dir_3, 'file.c'), 'wt'):
        pass
    with open(os.path.join(sub_dir_3, 'file.h'), 'wt'):
        pass

    # TODO: -rp is not applied to "Checking" messages
    #exitcode, stdout, _ = cppcheck(['-j1', '-rp', list_dir])
    exitcode, stdout, _ = cppcheck(['-j1', '.'], cwd=list_dir)
    assert exitcode == 0, stdout
    if sys.platform == "win32":
        stdout = stdout.replace('\\', '/')
    lines = stdout.splitlines()
    expected = [
        'Checking aaa.c ...',
        'Checking valueflow.cpp ...',
        'Checking valueflow/file.c ...',
        'Checking valueflow/file.cpp ...',
        'Checking vf_enumvalue.cpp ...',
        'Checking vf_enumvalue/file.c ...',
        'Checking vf_enumvalue/file.cpp ...',
        'Checking vfvalue.cpp ...',
        'Checking vfvalue/file.c ...',
        'Checking vfvalue/file.cpp ...',
        'Checking zzz.c ...'
    ]
    assert len(expected), len(lines)
    for i in range(1, len(expected)+1):
        lines.remove('{}/11 files checked 0% done'.format(i, len(expected)))
    assert lines == expected
