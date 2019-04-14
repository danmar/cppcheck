
# python -m pytest test-helloworld.py

import logging
import os
import subprocess

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

def test_relative_path():
    ret, stdout, stderr = cppcheck('1-helloworld')
    filename = os.path.join('1-helloworld', 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == '[%s:5]: (error) Division by zero.\n' % (filename)

def test_local_path():
    cwd = os.getcwd()
    os.chdir('1-helloworld')
    ret, stdout, stderr = cppcheck('.')
    os.chdir(cwd)
    assert ret == 0
    assert stdout == 'Checking main.c ...\n'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_absolute_path():
    prjpath = os.path.join(os.getcwd(), '1-helloworld')
    ret, stdout, stderr = cppcheck(prjpath)
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == '[%s:5]: (error) Division by zero.\n' % (filename)

def test_addon_local_path():
    cwd = os.getcwd()
    os.chdir('1-helloworld')
    ret, stdout, stderr = cppcheck('--addon=misra .')
    os.chdir(cwd)
    assert ret == 0
    assert stdout == 'Checking main.c ...\n'
    assert stderr == ('[main.c:5]: (error) Division by zero.\n'
                      '[main.c:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n')

def test_addon_absolute_path():
    prjpath = os.path.join(os.getcwd(), '1-helloworld')
    ret, stdout, stderr = cppcheck('--addon=misra %s' % (prjpath))
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_addon_relative_path():
    prjpath = '1-helloworld'
    ret, stdout, stderr = cppcheck('--addon=misra %s' % (prjpath))
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == ('[%s:5]: (error) Division by zero.\n'
                      '[%s:1]: (style) misra violation (use --rule-texts=<file> to get proper output)\n' % (filename, filename))

def test_basepath_relative_path():
    prjpath = '1-helloworld'
    ret, stdout, stderr = cppcheck('%s -rp=%s' % (prjpath, prjpath))
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_basepath_absolute_path():
    prjpath = os.path.join(os.getcwd(), '1-helloworld')
    ret, stdout, stderr = cppcheck('%s -rp=%s' % (prjpath, prjpath))
    filename = os.path.join(prjpath, 'main.c')
    assert ret == 0
    assert stdout == 'Checking %s ...\n' % (filename)
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

