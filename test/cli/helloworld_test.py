
# python -m pytest test-helloworld.py

import os
import re
import glob

from testutils import create_gui_project_file, cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'helloworld')


# Get Visual Studio configurations checking a file
# Checking {file} {config}...
def getVsConfigs(stdout, filename):
    ret = []
    for line in stdout.split('\n'):
        if not line.startswith('Checking %s ' % filename):
            continue
        if not line.endswith('...'):
            continue
        res = re.match(r'.* ([A-Za-z0-9|]+)...', line)
        if res:
            ret.append(res.group(1))
    ret.sort()
    return ' '.join(ret)

def test_relative_path():
    args = [
        '--template=cppcheck1',
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename


def test_local_path():
    args = [
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_absolute_path():
    args = [
        '--template=cppcheck1',
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_addon_local_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == ('[main.c:5]: (error) Division by zero.\n'
                      '[main.c:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n')

def test_addon_local_path_not_enable():
    args = [
        '--addon=misra',
        '--template=cppcheck1',
        '.'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_addon_absolute_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_relative_path():
    args = [
        '--addon=misra',
        '--enable=style',
        '--template=cppcheck1',
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stdout == ('Checking %s ...\n'
                      'Checking %s: SOME_CONFIG...\n' % (filename, filename))
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_with_gui_project():
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(os.path.join(__script_dir, project_file), paths=['.'], addon='misra')
    args = [
        '--template=cppcheck1',
        '--enable=style',
        '--project=' + project_file
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stdout == 'Checking %s ...\n' % filename
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_basepath_relative_path():
    args = [
        'helloworld',
        '--template=cppcheck1',
        '-rp=helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_basepath_absolute_path():
    args = [
        '--template=cppcheck1',
        __proj_dir,
        '-rp=' + __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_vs_project_local_path():
    args = [
        '--template=cppcheck1',
        '--project=helloworld.vcxproj'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert getVsConfigs(stdout, 'main.c') == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_vs_project_relative_path():
    args = [
        '--template=cppcheck1',
        '--project=' + os.path.join(__proj_dir, 'helloworld.vcxproj')
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_vs_project_absolute_path():
    args = [
        '--template=cppcheck1',
        '--project=' + os.path.join(__proj_dir, 'helloworld.vcxproj')
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_cppcheck_project_local_path():
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=helloworld.cppcheck'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__proj_dir)
    assert ret == 0, stdout
    assert getVsConfigs(stdout, 'main.c') == 'Debug|x64'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_cppcheck_project_relative_path():
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=' + os.path.join('helloworld', 'helloworld.cppcheck')
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_cppcheck_project_absolute_path():
    args = [
        '--template=cppcheck1',
        '--platform=win64',
        '--project=' + os.path.join(__proj_dir, 'helloworld.cppcheck')
    ]
    ret, stdout, stderr = cppcheck(args)
    filename = os.path.join(__proj_dir, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_suppress_command_line_related():
    args = [
        '--suppress=zerodiv:' + os.path.join('helloworld', 'main.c'),
        'helloworld'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ''

def test_suppress_command_line_absolute():
    args = [
        '--suppress=zerodiv:' + os.path.join(__proj_dir, 'main.c'),
        __proj_dir
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == ''

def test_suppress_project_relative():
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(os.path.join(__script_dir, project_file),
                            paths=['.'],
                            suppressions=[{'fileName':'main.c', 'id':'zerodiv'}])

    args = [
        '--project=' + project_file
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert ret == 0, stdout
    assert stderr == ''


def test_suppress_project_absolute():
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(os.path.join(__script_dir, project_file),
                            paths=['.'],
                            suppressions=[{'fileName':'main.c', 'id':'zerodiv'}])

    args = [
        '--project=' + os.path.join(__script_dir, 'helloworld', 'test.cppcheck')
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0, stdout
    assert stderr == ''

def test_exclude():
    args = [
        '-i' + 'helloworld',
        '--platform=win64',
        '--project=' + os.path.join('helloworld', 'helloworld.cppcheck')
    ]
    ret, stdout, _ = cppcheck(args, cwd=__script_dir)
    assert ret == 1
    lines = stdout.splitlines()
    assert lines == [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]


def test_build_dir_dump_output(tmpdir):
    args = [
        f'--cppcheck-build-dir={tmpdir}',
        '--addon=misra',
        'helloworld'
    ]

    cppcheck(args)
    cppcheck(args)

    filename = f'{tmpdir}/main.a1.*.dump'
    filelist = glob.glob(filename)
    assert(len(filelist) == 0)


def test_checkers_report(tmpdir):
    filename = os.path.join(tmpdir, '1.txt')
    args = [
        f'--checkers-report={filename}',
        'helloworld'
    ]

    cppcheck(args, cwd=__script_dir)

    with open(filename, 'rt') as f:
        data = f.read()
        assert 'No   CheckAutoVariables::assignFunctionArg' in data
        assert 'Yes  CheckAutoVariables::autoVariables' in data

    args += [
        '--enable=style'
    ]
    cppcheck(args, cwd=__script_dir)
    with open(filename, 'rt') as f:
        data = f.read()
        # checker has been activated by --enable=style
        assert 'Yes  CheckAutoVariables::assignFunctionArg' in data


def test_missing_include_system():  # #11283
    args = [
        '--enable=missingInclude',
        '--suppress=zerodiv',
        '--template=simple',
        'helloworld'
    ]

    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.replace('\\', '/') == 'helloworld/main.c:1:0: information: Include file: <stdio.h> not found. Please note: Cppcheck does not need standard library headers to get proper results. [missingIncludeSystem]\n'
