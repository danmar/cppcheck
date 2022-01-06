
# python -m pytest test-clang-import.py

import os
import re
import subprocess
import pytest
from testutils import cppcheck

try:
    subprocess.call(['clang', '--version'])
except OSError:
    pytest.skip("'clang' does not exist", allow_module_level=True)


def get_debug_section(title, stdout):
    s = re.sub(r'0x[0-9a-fA-F]+', '0x12345678', stdout)
    s = re.sub(r'nestedIn: Struct', 'nestedIn: Class', s)
    s = re.sub(r'classDef: struct', 'classDef: class', s)
    s = re.sub(r'isInline: [a-z]+', 'isInline: ---', s)
    s = re.sub(r'needInitialization: .*', 'needInitialization: ---', s)
    s = re.sub(r'functionOf: .*', 'functionOf: ---', s)
    s = re.sub(r'0x12345678 Struct', '0x12345678 Class', s)

    if title == '##AST':
        # TODO set types
        s = re.sub(r"return '[a-zA-Z0-9: *]+'", "return", s)

    pos1 = s.find(title)
    assert pos1 > 0, 'title not found'
    pos1 = s.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = s.find("\n##", pos1)
    if pos2 < 0:
        return s[pos1:]
    return s[pos1:pos2-1]


def check_symbol_database(code):
    testfile = 'test.cpp'
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, _ = cppcheck(['--clang', '--debug', '-v', testfile])
    ret2, stdout2, _ = cppcheck(['--debug', '-v', testfile])
    os.remove(testfile)
    assert 0 == ret1, stdout1
    assert 0 == ret2, stdout2
    assert get_debug_section('### Symbol database', stdout1) == get_debug_section('### Symbol database', stdout2)


def check_ast(code):
    testfile = 'test.cpp'
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, _ = cppcheck(['--clang', '--debug', '-v', testfile])
    ret2, stdout2, _ = cppcheck(['--debug', '-v', testfile])
    os.remove(testfile)
    assert 0 == ret1, stdout1
    assert 0 == ret2, stdout1
    assert get_debug_section('##AST', stdout1) == get_debug_section('##AST', stdout2)


def todo_check_ast(code):
    testfile = 'test.cpp'
    with open(testfile, 'w+t') as f:
        f.write(code)
    ret1, stdout1, _ = cppcheck(['--clang', '--debug', '-v', testfile])
    ret2, stdout2, _ = cppcheck(['--debug', '-v', testfile])
    os.remove(testfile)
    assert 0 == ret1, stdout1
    assert 0 == ret2, stdout2
    assert get_debug_section('##AST', stdout1) != get_debug_section('##AST', stdout2)



def test_symbol_database_1():
    check_symbol_database('int main(){return 0;}')

def test_symbol_database_2():
    check_symbol_database('struct Foo { void f(); }; void Foo::f() {}')

def test_symbol_database_3():
    check_symbol_database('struct Fred { int a; }; int b; void f(int c, int d) { int e; }')

def test_symbol_database_4():
    check_symbol_database('void f(const int x) {}')

def test_symbol_database_5():
    check_symbol_database('void f(int);')

def test_symbol_database_6():
    check_symbol_database('inline static int foo(int x) { return x; }')

def test_symbol_database_7():
    check_symbol_database('struct S {int x;}; void f(struct S *s) {}')

def test_symbol_database_class_access_1():
    check_symbol_database('class Fred { void foo ( ) {} } ;')

def test_symbol_database_class_access_2():
    check_symbol_database('class Fred { protected: void foo ( ) {} } ;')

def test_symbol_database_class_access_3():
    check_symbol_database('class Fred { public: void foo ( ) {} } ;')

def test_symbol_database_operator():
    check_symbol_database('struct Fred { void operator=(int x); };')

def test_symbol_database_struct_1():
    check_symbol_database('struct S {};')

def test_ast_calculations():
    check_ast('int x = 5; int y = (x + 4) * 2;')
    check_ast('long long dostuff(int x) { return x ? 3 : 5; }')

def test_ast_control_flow():
    check_ast('void foo(int x) { if (x > 5){} }')
    check_ast('int dostuff() { for (int x = 0; x < 10; x++); }')
    check_ast('void foo(int x) { switch (x) {case 1: break; } }')
    check_ast('void foo(int a, int b, int c) { foo(a,b,c); }')

def test_ast():
    check_ast('struct S { int x; }; S* foo() { return new S(); }')

