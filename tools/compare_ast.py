# Example usage:
# python3 compare_ast.py lib/token.cpp
# python3 compare_ast.py "--project=compile_commands.json --file-filter=*/lib/somefile.c"

import glob
import os
import re
import sys
import subprocess

CPPCHECK = os.path.expanduser('~/cppcheck/cppcheck')

def run_cppcheck(cppcheck_parameters:str, clang:str):
    cmd = f'{CPPCHECK} {cppcheck_parameters} {clang} --debug --verbose'
    print(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    return comm[0].decode('utf-8', 'ignore')


def get_ast(debug):
    debug = re.sub(r' f:0x[0-9a-fA-F]+', '', debug)
    debug = re.sub(r" '[a-zA-Z0-9: *]+'", '', debug)

    pos1 = debug.rfind('\n##AST\n')
    if pos1 < 0:
        return ''
    pos1 = debug.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = debug.find("\n##", pos1)
    if pos2 < 0:
        return debug[pos1:]
    return debug[pos1:pos2-1]


def compare_ast(cppcheck_parameters: str):
    ast1 = get_ast(run_cppcheck(cppcheck_parameters, ''))
    ast2 = get_ast(run_cppcheck(cppcheck_parameters, '--clang'))
    if ast1 == ast2:
        print(f"ast is the same for file {cppcheck_parameters}")
        print('ast size:%i' % len(ast1))
    else:
        print(f"ast is not the same for file {cppcheck_parameters}")
        print('saving cppcheck ast in file ast1 and clang ast in file ast2')

        f = open('ast1', 'wt')
        f.write(ast1)
        f.close()

        f = open('ast2', 'wt')
        f.write(ast2)
        f.close()

        sys.exit(1)


for arg in sys.argv[1:]:
    compare_ast(arg)

