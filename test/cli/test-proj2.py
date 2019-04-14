
# python -m pytest test-helloworld.py

import json
import logging
import os
import subprocess

COMPILE_COMMANDS_JSON = os.path.join('proj2', 'compile_commands.json')

def create_compile_commands(path):
    j = [{'directory': path+'/a', 'command': 'gcc -c a.c', 'file': 'a.c'},
         {'directory': path+'/b', 'command': 'gcc -c b.c', 'file': 'b.c'}]
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

def test_local_path():
    create_compile_commands(os.path.join(os.getcwd(), 'proj2'))
    cwd = os.getcwd()
    os.chdir('proj2')
    ret, stdout, stderr = cppcheck('--project=compile_commands.json')
    os.chdir(cwd)
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_relative_path():
    create_compile_commands(os.path.join(os.getcwd(), 'proj2'))
    ret, stdout, stderr = cppcheck('--project=' + COMPILE_COMMANDS_JSON)
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_absolute_path():
    create_compile_commands(os.path.join(os.getcwd(), 'proj2'))
    cwd = os.getcwd()
    ret, stdout, stderr = cppcheck('--project=' + os.path.join(cwd,COMPILE_COMMANDS_JSON))
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_project_1():
    create_compile_commands(os.path.join(os.getcwd(), 'proj2'))
    ret, stdout, stderr = cppcheck('--project=proj2/proj2.cppcheck')
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_project_2():
    create_compile_commands(os.path.join(os.getcwd(), 'proj2'))
    ret, stdout, stderr = cppcheck('--project=proj2/proj2-exclude.cppcheck')
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c') # Excluded by proj2-exclude.cppcheck
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    #assert stdout.find('Checking %s ...' % (file2)) < 0

