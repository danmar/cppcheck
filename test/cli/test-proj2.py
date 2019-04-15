
# python -m pytest test-helloworld.py

import json
import logging
import os
import subprocess

COMPILE_COMMANDS_JSON = os.path.join('proj2', 'compile_commands.json')

def create_compile_commands():
    prjpath = os.path.join(os.getcwd(), 'proj2')
    j = [{'directory': os.path.join(prjpath,'a'), 'command': 'gcc -c a.c', 'file': 'a.c'},
         {'directory': os.path.join(prjpath,'b'), 'command': 'gcc -c b.c', 'file': 'b.c'}]
    f = open(COMPILE_COMMANDS_JSON, 'wt')
    f.write(json.dumps(j))

# Run Cppcheck with args
def cppcheck(args):
    if os.path.isfile('../../bin/debug/cppcheck.exe'):
        cmd = '../../bin/debug/cppcheck.exe ' + args
    elif os.path.isfile('../../../bin/debug/cppcheck.exe'):
        cmd = '../../../bin/debug/cppcheck.exe ' + args
    elif os.path.isfile('../../cppcheck'):
        cmd = '../../cppcheck ' + args
    else:
        cmd = '../../../cppcheck ' + args
    logging.info(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    return p.returncode, stdout, stderr

# Run Cppcheck from project path
def cppcheck_local(args):
    cwd = os.getcwd()
    os.chdir('proj2')
    ret, stdout, stderr = cppcheck(args)
    os.chdir(cwd)
    return (ret, stdout, stderr)


def test_local_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck_local('--project=compile_commands.json')
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_relative_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck('--project=' + COMPILE_COMMANDS_JSON)
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_absolute_path():
    create_compile_commands()
    cwd = os.getcwd()
    ret, stdout, stderr = cppcheck('--project=' + os.path.join(cwd,COMPILE_COMMANDS_JSON))
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_gui_project_loads_compile_commands_1():
    create_compile_commands()
    ret, stdout, stderr = cppcheck('--project=proj2/proj2.cppcheck')
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_gui_project_loads_compile_commands_2():
    create_compile_commands()

    # create cppcheck gui project file
    cppcheck_xml = ('<?xml version="1.0" encoding="UTF-8"?>'
                    '<project version="1">'
                    '  <root name="."/>'
                    '  <importproject>compile_commands.json</importproject>'
                    '  <exclude>'
                    '    <path name="' + os.path.join(os.getcwd(), 'proj2', 'b').replace('\\', '/') + '"/>'
                    '    </exclude>'
                    '</project>')
    f = open('proj2/test.cppcheck', 'wt')
    f.write(cppcheck_xml)
    f.close()

    ret, stdout, stderr = cppcheck('--project=proj2/test.cppcheck')
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c') # Excluded by test.cppcheck
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) < 0

