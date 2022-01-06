
# python -m pytest test-proj2.py

import json
import os
from testutils import create_gui_project_file, cppcheck

COMPILE_COMMANDS_JSON = 'compile_commands.json'

ERR_A = ('%s:1:7: error: Division by zero. [zerodiv]\n' +
         'x = 3 / 0;\n' +
         '      ^\n') % os.path.join('a', 'a.c')
ERR_B = ('%s:1:7: error: Division by zero. [zerodiv]\n' +
         'x = 3 / 0;\n' +
         '      ^\n') % os.path.join('b', 'b.c')

def realpath(s):
    return os.path.realpath(s).replace('\\', '/')

def create_compile_commands():
    j = [{'directory': realpath('proj2/a'), 'command': 'gcc -c a.c', 'file': 'a.c'},
         {'directory': realpath('proj2'), 'command': 'gcc -c b/b.c', 'file': 'b/b.c'}]
    with open('proj2/' + COMPILE_COMMANDS_JSON, 'wt') as f:
        f.write(json.dumps(j))

# Run Cppcheck from project path
def cppcheck_local(args):
    cwd = os.getcwd()
    os.chdir('proj2')
    ret, stdout, stderr = cppcheck(args)
    os.chdir(cwd)
    return ret, stdout, stderr

def test_file_filter():
    ret, stdout, stderr = cppcheck(['proj2/','--file-filter=proj2/a/*'])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % file1) >= 0
    ret, stdout, stderr = cppcheck(['proj2/','--file-filter=proj2/b*'])
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_local_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck_local(['--project=compile_commands.json'])
    file1 = os.path.join('a', 'a.c')
    file2 = os.path.join('b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_local_path_force():
    create_compile_commands()
    ret, stdout, stderr = cppcheck_local(['--project=compile_commands.json', '--force'])
    assert ret == 0, stdout
    assert stdout.find('AAA') >= 0

def test_local_path_maxconfigs():
    create_compile_commands()
    ret, stdout, stderr = cppcheck_local(['--project=compile_commands.json', '--max-configs=2'])
    assert ret == 0, stdout
    assert stdout.find('AAA') >= 0

def test_relative_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck(['--project=proj2/' + COMPILE_COMMANDS_JSON])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_absolute_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck(['--project=' + os.path.realpath('proj2/' + COMPILE_COMMANDS_JSON)])
    file1 = os.path.realpath('proj2/a/a.c')
    file2 = os.path.realpath('proj2/b/b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_gui_project_loads_compile_commands_1():
    create_compile_commands()
    ret, stdout, stderr = cppcheck(['--project=proj2/proj2.cppcheck'])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_gui_project_loads_compile_commands_2():
    create_compile_commands()
    exclude_path_1 = 'proj2/b'
    create_gui_project_file('proj2/test.cppcheck',
                            import_project='compile_commands.json',
                            exclude_paths=[exclude_path_1])
    ret, stdout, stderr = cppcheck(['--project=proj2/test.cppcheck'])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c') # Excluded by test.cppcheck
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) < 0


def test_gui_project_loads_relative_vs_solution():
    create_gui_project_file('test.cppcheck', import_project='proj2/proj2.sln')
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s Debug|Win32...' % file1) >= 0
    assert stdout.find('Checking %s Debug|x64...' % file1) >= 0
    assert stdout.find('Checking %s Release|Win32...' % file1) >= 0
    assert stdout.find('Checking %s Release|x64...' % file1) >= 0
    assert stdout.find('Checking %s Debug|Win32...' % file2) >= 0
    assert stdout.find('Checking %s Debug|x64...' % file2) >= 0
    assert stdout.find('Checking %s Release|Win32...' % file2) >= 0
    assert stdout.find('Checking %s Release|x64...' % file2) >= 0

def test_gui_project_loads_absolute_vs_solution():
    create_gui_project_file('test.cppcheck', import_project=realpath('proj2/proj2.sln'))
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    file1 = os.path.realpath('proj2/a/a.c')
    file2 = os.path.realpath('proj2/b/b.c')
    print(stdout)
    assert ret == 0, stdout
    assert stdout.find('Checking %s Debug|Win32...' % file1) >= 0
    assert stdout.find('Checking %s Debug|x64...' % file1) >= 0
    assert stdout.find('Checking %s Release|Win32...' % file1) >= 0
    assert stdout.find('Checking %s Release|x64...' % file1) >= 0
    assert stdout.find('Checking %s Debug|Win32...' % file2) >= 0
    assert stdout.find('Checking %s Debug|x64...' % file2) >= 0
    assert stdout.find('Checking %s Release|Win32...' % file2) >= 0
    assert stdout.find('Checking %s Release|x64...' % file2) >= 0

def test_gui_project_loads_relative_vs_solution_2():
    create_gui_project_file('test.cppcheck', root_path='proj2', import_project='proj2/proj2.sln')
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert ret == 0, stdout
    assert stderr == ERR_A + ERR_B

def test_gui_project_loads_relative_vs_solution_with_exclude():
    create_gui_project_file('test.cppcheck', root_path='proj2', import_project='proj2/proj2.sln', exclude_paths=['b'])
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert ret == 0, stdout
    assert stderr == ERR_A

def test_gui_project_loads_absolute_vs_solution_2():
    create_gui_project_file('test.cppcheck',
                            root_path=realpath('proj2'),
                            import_project=realpath('proj2/proj2.sln'))
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert ret == 0, stdout
    assert stderr == ERR_A + ERR_B
