
# python -m pytest test-helloworld.py

import logging
import os
import subprocess

# Run Cppcheck with args
def cppcheck(args):
    cmd = '%s %s' % (os.path.expanduser('~/cppcheck/cppcheck'), args)
    logging.info(cmd)
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore')
    return p.returncode, stdout, stderr

def test_relative_path():
    ret, stdout, stderr = cppcheck('1-helloworld')
    assert ret == 0
    assert stdout == 'Checking 1-helloworld/main.c ...\n'
    assert stderr == '[1-helloworld/main.c:5]: (error) Division by zero.\n'

def test_local_path():
    cwd = os.getcwd()
    os.chdir('1-helloworld')
    ret, stdout, stderr = cppcheck('.')
    os.chdir(cwd)
    assert ret == 0
    assert stdout == 'Checking main.c ...\n'
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_absolute_path():
    prjpath = '%s/1-helloworld' % (os.getcwd())
    ret, stdout, stderr = cppcheck(prjpath)
    assert ret == 0
    assert stdout == 'Checking %s/main.c ...\n' % (prjpath)
    assert stderr == '[%s/main.c:5]: (error) Division by zero.\n' % (prjpath)

def test_basepath_relative_path():
    prjpath = '1-helloworld'
    ret, stdout, stderr = cppcheck('%s -rp=%s' % (prjpath, prjpath))
    assert ret == 0
    assert stdout == 'Checking %s/main.c ...\n' % (prjpath)
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

def test_basepath_absolute_path():
    prjpath = '%s/1-helloworld' % (os.getcwd())
    ret, stdout, stderr = cppcheck('%s -rp=%s' % (prjpath, prjpath))
    assert ret == 0
    assert stdout == 'Checking %s/main.c ...\n' % (prjpath)
    assert stderr == '[main.c:5]: (error) Division by zero.\n'

