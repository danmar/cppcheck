
# python -m pytest test-helloworld.py

import os
import re
import tempfile
import pytest
import glob

from testutils import create_gui_project_file, cppcheck

# Run Cppcheck from project path
def cppcheck_local(args):
    cwd = os.getcwd()
    os.chdir('helloworld')
    ret, stdout, stderr = cppcheck(args)
    os.chdir(cwd)
    return ret, stdout, stderr

def getRelativeProjectPath():
    return 'helloworld'

def getAbsoluteProjectPath():
    return os.path.join(os.getcwd(), 'helloworld')

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
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', 'helloworld'])
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename


def test_local_path():
    ret, stdout, stderr = cppcheck_local(['--template=cppcheck1', '.'])
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_absolute_path():
    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', prjpath])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_addon_local_path():
    ret, stdout, stderr = cppcheck_local(['--addon=misra', '--enable=style', '--template=cppcheck1', '.'])
    assert ret == 0, stdout
    assert stderr == ('[main.c:5]: (error) Division by zero.\n'
                      '[main.c:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n')

def test_addon_local_path_not_enable():
    ret, stdout, stderr = cppcheck_local(['--addon=misra', '--template=cppcheck1', '.'])
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_addon_absolute_path():
    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--addon=misra', '--enable=style', '--template=cppcheck1', prjpath])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_relative_path():
    prjpath = getRelativeProjectPath()
    ret, stdout, stderr = cppcheck(['--platform=native', '--addon=misra', '--enable=style', '--template=cppcheck1', prjpath])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert stdout == ('Checking %s ...\n'
                      'Checking %s: SOME_CONFIG...\n' % (filename, filename))
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_with_gui_project():
    project_file = 'helloworld/test.cppcheck'
    create_gui_project_file(project_file, paths=['.'], addon='misra')
    ret, stdout, stderr = cppcheck(['--platform=native', '--template=cppcheck1', '--enable=style', '--project=' + project_file])
    filename = os.path.join('helloworld', 'main.c')
    assert ret == 0, stdout
    assert stdout == 'Checking %s ...\n' % filename
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_basepath_relative_path():
    prjpath = getRelativeProjectPath()
    ret, stdout, stderr = cppcheck([prjpath, '--template=cppcheck1', '-rp=' + prjpath])
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_basepath_absolute_path():
    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', prjpath, '-rp=' + prjpath])
    assert ret == 0, stdout
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_vs_project_local_path():
    ret, stdout, stderr = cppcheck_local(['--template=cppcheck1', '--project=helloworld.vcxproj'])
    assert ret == 0, stdout
    assert getVsConfigs(stdout, 'main.c') == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_vs_project_relative_path():
    prjpath = getRelativeProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', '--project=' + os.path.join(prjpath, 'helloworld.vcxproj')])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_vs_project_absolute_path():
    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', '--project=' + os.path.join(prjpath, 'helloworld.vcxproj')])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|Win32 Debug|x64 Release|Win32 Release|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_cppcheck_project_local_path():
    ret, stdout, stderr = cppcheck_local(['--template=cppcheck1', '--platform=win64', '--project=helloworld.cppcheck'])
    assert ret == 0, stdout
    assert getVsConfigs(stdout, 'main.c') == 'Debug|x64'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_cppcheck_project_relative_path():
    prjpath = getRelativeProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', '--platform=win64', '--project=' + os.path.join(prjpath, 'helloworld.cppcheck')])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_cppcheck_project_absolute_path():
    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--template=cppcheck1', '--platform=win64', '--project=' + os.path.join(prjpath, 'helloworld.cppcheck')])
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0, stdout
    assert getVsConfigs(stdout, filename) == 'Debug|x64'
    assert stderr == '[%s:5]: (error) Division by zero.\n' % filename

def test_suppress_command_line():
    prjpath = getRelativeProjectPath()
    ret, stdout, stderr = cppcheck(['--suppress=zerodiv:' + os.path.join(prjpath, 'main.c'), prjpath])
    assert ret == 0, stdout
    assert stderr == ''

    prjpath = getAbsoluteProjectPath()
    ret, stdout, stderr = cppcheck(['--suppress=zerodiv:' + os.path.join(prjpath, 'main.c'), prjpath])
    assert ret == 0, stdout
    assert stderr == ''

def test_suppress_project():
    project_file = os.path.join('helloworld', 'test.cppcheck')
    create_gui_project_file(project_file,
                            paths=['.'],
                            suppressions=[{'fileName':'main.c', 'id':'zerodiv'}])

    # Relative path
    ret, stdout, stderr = cppcheck(['--project=' + project_file])
    assert ret == 0, stdout
    assert stderr == ''

    # Absolute path
    ret, stdout, stderr = cppcheck(['--project=' + os.path.join(os.getcwd(), 'helloworld', 'test.cppcheck')])
    assert ret == 0, stdout
    assert stderr == ''


def test_exclude():
    prjpath = getRelativeProjectPath()
    ret, stdout, _ = cppcheck(['-i' + prjpath, '--platform=win64', '--project=' + os.path.join(prjpath, 'helloworld.cppcheck')])
    assert ret == 1
    assert stdout == 'cppcheck: error: no C or C++ source files found.\n'


def test_build_dir_dump_output():
    with tempfile.TemporaryDirectory() as tempdir:
        args = f'--cppcheck-build-dir={tempdir} --addon=misra helloworld'

        cppcheck(args.split())
        cppcheck(args.split())

        filename = f'{tempdir}/main.a1.*.dump'
        filelist = glob.glob(filename)
        assert(len(filelist) == 0)


def test_checkers_report():
    with tempfile.TemporaryDirectory() as tempdir:
        filename = os.path.join(tempdir, '1.txt')
        args = f'--checkers-report={filename} helloworld'

        cppcheck(args.split())

        with open(filename, 'rt') as f:
            data = f.read()
            assert 'No   CheckAutoVariables::assignFunctionArg' in data
            assert 'Yes  CheckAutoVariables::autoVariables' in data

        args = '--enable=style ' + args
        cppcheck(args.split())
        with open(filename, 'rt') as f:
            data = f.read()
            # checker has been activated by --enable=style
            assert 'Yes  CheckAutoVariables::assignFunctionArg' in data


def __test_missing_include_system(use_j):
    args = ['--enable=missingInclude', '--suppress=zerodiv', '--template={file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]', 'helloworld']
    if use_j:
        args.insert(0, '-j2')

    _, _, stderr = cppcheck(args)
    assert stderr.replace('\\', '/') == 'helloworld/main.c:1:0: information: Include file: <stdio.h> not found. Please note: Cppcheck does not need standard library headers to get proper results. [missingIncludeSystem]\n'

def test_missing_include_system():
    __test_missing_include_system(False)

def test_missing_include_system_j(): #11283
    __test_missing_include_system(True)
