
# python -m pytest test-clang-import.py

import os
import re
import subprocess
from testutils import create_gui_project_file, cppcheck


def get_debug_section(title, stdout):
    s = re.sub(r'0x[0-9a-fA-F]+', '0x12345678', stdout)
    s = re.sub(r'nestedIn: Struct', 'nestedIn: Class', s)
    s = re.sub(r'classDef: struct', 'classDef: class', s)
    s = re.sub(r'isInline: [a-z]+', 'isInline: ---', s)
    s = re.sub(r'definedType: .*', 'definedType: ---', s)
    s = re.sub(r'needInitialization: .*', 'needInitialization: ---', s)
    s = re.sub(r'functionOf: .*', 'functionOf: ---', s)
    s = re.sub(r'0x12345678 Struct', '0x12345678 Class', s)
    pos1 = s.find(title)
    assert pos1 > 0
    pos1 = s.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = s.find("\n##", pos1)
    if pos2 < 0:
        return s[pos1:]
    return s[pos1:pos2-1]


def check_symbol_database(code):
    # Only compare symboldatabases if clang is found in PATH
    try:
        subprocess.call(['clang', '--version'])
    except OSError:
        return

    testfile = 'test.cpp'
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, stderr1 = cppcheck(['--clang', '--debug', '-v', testfile])
    ret2, stdout2, stderr2 = cppcheck(['--debug', '-v', testfile])
    os.remove(testfile)
    assert get_debug_section('### Symbol database', stdout1) == get_debug_section('### Symbol database', stdout2)


def test1():
    check_symbol_database('int main(){return 0;}')

def test2():
    code = 'struct Foo { void f(); }; void Foo::f() {}'
    check_symbol_database(code)



