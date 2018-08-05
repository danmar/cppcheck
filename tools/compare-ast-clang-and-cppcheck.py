#!/usr/bin/env python
#
# Compare the AST of Cppcheck and Clang
#
# Example usage:
# cd cppcheck/tools
# g++ `llvm-config-3.8 --cxxflags --ldflags` -lclang -o clang-ast clang-ast.cpp
# python compare-ast-clang-and-cppcheck.py lib/token.cpp
#
# If you get such output:
# - <Function...>
# Then that means there is a missing function in the Cppcheck SymbolDatabase
#
import subprocess
import sys
sys.path.insert(0, '../addons')
import cppcheckdata

CLANG_AST = './clang-ast'
CPPCHECK = '../cppcheck'

def clang_ast(sourcefile):
    p = subprocess.Popen([CLANG_AST,sourcefile], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    ret = []
    for line in comm[0].split('\n'):
        while len(line)>1 and line[-1] in ' \r\n':
            line = line[:-1]
        if line.find('/usr/') > 0:
            continue
        if line.startswith('<Function'):
            ret.append(line)
    ret.sort()
    return ret


def cppcheck_ast(sourcefile):
    subprocess.call([CPPCHECK, '--dump', '--max-configs=1', sourcefile])
    data = cppcheckdata.parsedump(sourcefile + '.dump')
    cfg = data.configurations[0]
    ret = []
    for func in cfg.functions:
        name = func.name
        if func.type == 'Destructor':
            name = '~' + name
        s = '<Function'
        s = s + ' name="' + name + '"'
        s = s + ' filename="' + func.tokenDef.file + '"'
        s = s + ' line="' + str(func.tokenDef.linenr) + '"'
        s = s + '/>'
        ret.append(s)
    for scope in cfg.scopes:
        if scope.type != 'Function':
            continue
        argStart = scope.bodyStart
        while argStart and argStart.str != '(':
            argStart = argStart.previous
        s = '<Function'
        s = s + ' name="' + scope.className + '"'
        s = s + ' filename="' + argStart.file + '"'
        s = s + ' line="' + str(argStart.linenr) + '"'
        s = s + '/>'
        if s not in ret:
            ret.append(s)
    ret.sort()
    return ret

print('Clang AST...')
ast1 = clang_ast(sys.argv[1])
print('Cppcheck AST...')
ast2 = cppcheck_ast(sys.argv[1])

#for func in ast2:
#    print(func)

print('Compare...')
numberOfMissing = 0
numberOfExtra = 0
for func in ast1:
    if func not in ast2:
        numberOfMissing = numberOfMissing + 1
        print('Missing Function ' + func)
for func in ast2:
    if func not in ast1:
        numberOfExtra = numberOfExtra + 1
        print('Extra Function ' + func)
print('Number of missing functions: ' + str(numberOfMissing))
print('Number of extra functions: ' + str(numberOfExtra) + ' (clang AST currently only contains FunctionDecl and CXXMethod)')
