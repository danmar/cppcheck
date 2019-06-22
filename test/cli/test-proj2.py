
# python -m pytest test-proj2.py

import json
import os
from testutils import create_gui_project_file, cppcheck

COMPILE_COMMANDS_JSON = os.path.join('proj2', 'compile_commands.json')

def create_compile_commands():
    prjpath = os.path.join(os.getcwd(), 'proj2')
    j = [{'directory': os.path.join(prjpath,'a'), 'command': 'gcc -c a.c', 'file': 'a.c'},
         {'directory': os.path.join(prjpath,'b'), 'command': 'gcc -c b.c', 'file': 'b.c'}]
    f = open(COMPILE_COMMANDS_JSON, 'wt')
    f.write(json.dumps(j))

# Run Cppcheck from project path
def cppcheck_local(args):
    cwd = os.getcwd()
    os.chdir('proj2')
    ret, stdout, stderr = cppcheck(args)
    os.chdir(cwd)
    return (ret, stdout, stderr)


def test_local_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck_local(['--project=compile_commands.json'])
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_relative_path():
    create_compile_commands()
    ret, stdout, stderr = cppcheck(['--project=' + COMPILE_COMMANDS_JSON])
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_absolute_path():
    create_compile_commands()
    cwd = os.getcwd()
    ret, stdout, stderr = cppcheck(['--project=' + os.path.join(cwd,COMPILE_COMMANDS_JSON)])
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_gui_project_loads_compile_commands_1():
    create_compile_commands()
    ret, stdout, stderr = cppcheck(['--project=proj2/proj2.cppcheck'])
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) >= 0

def test_gui_project_loads_compile_commands_2():
    create_compile_commands()
    exclude_path_1 = os.path.join(os.getcwd(), 'proj2', 'b').replace('\\', '/')
    create_gui_project_file('proj2/test.cppcheck',
                            import_project='compile_commands.json',
                            exclude_paths=[exclude_path_1])
    ret, stdout, stderr = cppcheck(['--project=proj2/test.cppcheck'])
    cwd = os.getcwd()
    file1 = os.path.join(cwd, 'proj2', 'a', 'a.c')
    file2 = os.path.join(cwd, 'proj2', 'b', 'b.c') # Excluded by test.cppcheck
    assert ret == 0
    assert stdout.find('Checking %s ...' % (file1)) >= 0
    assert stdout.find('Checking %s ...' % (file2)) < 0


def test_gui_project_loads_relative_vs_solution():
    create_gui_project_file('test.cppcheck', import_project='proj2/proj2.sln')
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0
    assert stdout.find('Checking %s Debug|Win32...' % (file1)) >= 0
    assert stdout.find('Checking %s Debug|x64...' % (file1)) >= 0
    assert stdout.find('Checking %s Release|Win32...' % (file1)) >= 0
    assert stdout.find('Checking %s Release|x64...' % (file1)) >= 0
    assert stdout.find('Checking %s Debug|Win32...' % (file2)) >= 0
    assert stdout.find('Checking %s Debug|x64...' % (file2)) >= 0
    assert stdout.find('Checking %s Release|Win32...' % (file2)) >= 0
    assert stdout.find('Checking %s Release|x64...' % (file2)) >= 0

def test_gui_project_loads_absolute_vs_solution():
    create_gui_project_file('test.cppcheck', import_project=os.path.join(os.getcwd(),'proj2', 'proj2.sln').replace('\\', '/'))
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    file1 = os.path.join(os.getcwd(), 'proj2', 'a', 'a.c')
    file2 = os.path.join(os.getcwd(), 'proj2', 'b', 'b.c')
    print(stdout)
    assert ret == 0
    assert stdout.find('Checking %s Debug|Win32...' % (file1)) >= 0
    assert stdout.find('Checking %s Debug|x64...' % (file1)) >= 0
    assert stdout.find('Checking %s Release|Win32...' % (file1)) >= 0
    assert stdout.find('Checking %s Release|x64...' % (file1)) >= 0
    assert stdout.find('Checking %s Debug|Win32...' % (file2)) >= 0
    assert stdout.find('Checking %s Debug|x64...' % (file2)) >= 0
    assert stdout.find('Checking %s Release|Win32...' % (file2)) >= 0
    assert stdout.find('Checking %s Release|x64...' % (file2)) >= 0

def test_gui_project_loads_relative_vs_solution():
    create_gui_project_file('test.cppcheck', root_path='proj2', import_project='proj2/proj2.sln')
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert stderr == ('[a/a.c:1]: (error) Division by zero.\n'
                      '[b/b.c:1]: (error) Division by zero.\n')

def test_gui_project_loads_relative_vs_solution():
    create_gui_project_file('test.cppcheck', root_path='proj2', import_project='proj2/proj2.sln', exclude_paths=['b'])
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert stderr == '[a/a.c:1]: (error) Division by zero.\n'

def test_gui_project_loads_absolute_vs_solution():
    create_gui_project_file('test.cppcheck',
                            root_path=os.path.join(os.getcwd(), 'proj2').replace('\\', '/'),
                            import_project=os.path.join(os.getcwd(), 'proj2', 'proj2.sln').replace('\\', '/'))
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'])
    assert stderr == ('[a/a.c:1]: (error) Division by zero.\n'
                      '[b/b.c:1]: (error) Division by zero.\n')

