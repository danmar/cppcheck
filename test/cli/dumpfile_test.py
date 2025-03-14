
# python -m pytest dumpfile_test.py

import os

from testutils import cppcheck


def test_libraries(tmpdir):  #13701
    test_file = str(tmpdir / 'test.c')
    with open(test_file, 'wt') as f:
        f.write('x=1;\n')

    args = ['--library=posix', '--dump', test_file]
    _, _, _ = cppcheck(args)

    dumpfile = test_file + '.dump'
    assert os.path.isfile(dumpfile)
    with open(dumpfile, 'rt') as f:
        dump = f.read()
    assert '<library lib="posix"/>' in dump
    assert dump.find('<library lib="posix"/>') < dump.find('<dump cfg=')
