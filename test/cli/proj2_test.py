
# python -m pytest test-proj2.py

import json
import os
from testutils import create_gui_project_file, cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'proj2')

COMPILE_COMMANDS_JSON = 'compile_commands.json'

ERR_A = ('%s:1:7: error: Division by zero. [zerodiv]\n' +
         'x = 3 / 0;\n' +
         '      ^\n') % os.path.join('a', 'a.c')
ERR_B = ('%s:1:7: error: Division by zero. [zerodiv]\n' +
         'x = 3 / 0;\n' +
         '      ^\n') % os.path.join('b', 'b.c')

def create_compile_commands():
    j = [{'directory': os.path.join(__proj_dir, 'a'), 'command': 'gcc -c a.c', 'file': 'a.c'},
         {'directory': __proj_dir, 'command': 'gcc -c b/b.c', 'file': 'b/b.c'}]
    with open(os.path.join(__proj_dir, COMPILE_COMMANDS_JSON), 'wt') as f:
        f.write(json.dumps(j))


def test_file_filter():
    ret, stdout, _ = cppcheck(['proj2/','--file-filter=proj2/a/*'], cwd=__script_dir)
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    ret, stdout, _ = cppcheck(['proj2/','--file-filter=proj2/b*'], cwd=__script_dir)
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_local_path():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=compile_commands.json'], cwd=__proj_dir)
    file1 = os.path.join('a', 'a.c')
    file2 = os.path.join('b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_local_path_force():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=compile_commands.json', '--force'], cwd=__proj_dir)
    assert ret == 0, stdout
    assert stdout.find('AAA') >= 0

def test_local_path_maxconfigs():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=compile_commands.json', '--max-configs=2'], cwd=__proj_dir)
    assert ret == 0, stdout
    assert stdout.find('AAA') >= 0

def test_relative_path():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=proj2/' + COMPILE_COMMANDS_JSON], cwd=__script_dir)
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_absolute_path():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=' + os.path.join(__proj_dir, COMPILE_COMMANDS_JSON)], cwd=__script_dir)
    file1 = os.path.join(__proj_dir, 'a', 'a.c')
    file2 = os.path.join(__proj_dir, 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_gui_project_loads_compile_commands_1():
    create_compile_commands()
    ret, stdout, _ = cppcheck(['--project=proj2/proj2.cppcheck'], cwd=__script_dir)
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c')
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) >= 0

def test_gui_project_loads_compile_commands_2():
    create_compile_commands()
    exclude_path_1 = 'proj2/b'
    create_gui_project_file(os.path.join(__proj_dir, 'test.cppcheck'),
                            import_project='compile_commands.json',
                            exclude_paths=[exclude_path_1])
    ret, stdout, _ = cppcheck(['--project=' + os.path.join('proj2','test.cppcheck')], cwd=__script_dir)
    file1 = os.path.join('proj2', 'a', 'a.c')
    file2 = os.path.join('proj2', 'b', 'b.c') # Excluded by test.cppcheck
    assert ret == 0, stdout
    assert stdout.find('Checking %s ...' % file1) >= 0
    assert stdout.find('Checking %s ...' % file2) < 0


def test_gui_project_loads_relative_vs_solution():
    create_gui_project_file(os.path.join(__script_dir, 'test.cppcheck'), import_project='proj2/proj2.sln')
    ret, stdout, _ = cppcheck(['--project=test.cppcheck'], cwd=__script_dir)
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
    create_gui_project_file(os.path.join(__script_dir, 'test.cppcheck'), import_project=os.path.join(__proj_dir, 'proj2.sln'))
    ret, stdout, _ = cppcheck(['--project=test.cppcheck'], cwd=__script_dir)
    file1 = os.path.join(__proj_dir, 'a', 'a.c')
    file2 = os.path.join(__proj_dir, 'b', 'b.c')
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
    create_gui_project_file(os.path.join(__script_dir, 'test.cppcheck'), root_path='proj2', import_project='proj2/proj2.sln')
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'], cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ERR_A + ERR_B

def test_gui_project_loads_relative_vs_solution_with_exclude():
    create_gui_project_file(os.path.join(__script_dir, 'test.cppcheck'), root_path='proj2', import_project='proj2/proj2.sln', exclude_paths=['b'])
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'], cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ERR_A

def test_gui_project_loads_absolute_vs_solution_2():
    create_gui_project_file(os.path.join(__script_dir, 'test.cppcheck'),
                            root_path=__proj_dir,
                            import_project=os.path.join(__proj_dir, 'proj2.sln'))
    ret, stdout, stderr = cppcheck(['--project=test.cppcheck'], cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ERR_A + ERR_B
