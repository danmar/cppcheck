# python -m pytest test-more-projects.py
import json
import os
import pytest
import sys
from testutils import cppcheck, assert_cppcheck


def test_project_force_U(tmpdir):
    # 10018
    # -U does not work with compile_commands.json
    with open(os.path.join(tmpdir, 'bug1.cpp'), 'wt') as f:
        f.write("""
                int x = 123 / 0;
                #ifdef MACRO1
                int y = 1000 / 0;
                #endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    # Without -U => both bugs are found
    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--force', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert (stderr == '[bug1.cpp:2]: (error) Division by zero.\n'
                      '[bug1.cpp:4]: (error) Division by zero.\n')

    # With -U => only first bug is found
    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--force', '-UMACRO1', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert stderr == '[bug1.cpp:2]: (error) Division by zero.\n'


def __write_cppcheck_project_file(tmpdir, platform=None, importproject=None):
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    if platform is not None:
        platform = '<platform>{}</platform>'.format(platform)
    if importproject is not None:
        platform = '<importproject>{}</importproject>'.format(importproject)

    with open(project_file, 'wt') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
    {}
    {}
    <paths>
        <dir name="."/>
    </paths>
</project>""".format(platform, importproject)
        )

    return project_file


def test_project_custom_platform(tmpdir):
    """
    import cppcheck project that contains a custom platform file
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='p1.xml')

    with open(os.path.join(tmpdir, 'p1.xml'), 'wt') as f:
        f.write('<?xml version="1.0"?>\n<platform/>')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 0, stdout
    assert stdout == ''
    assert stderr == ''


def test_project_empty_platform(tmpdir):
    """
    import cppcheck project that contains an empty platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 0, stdout
    assert stdout == ''
    assert stderr == ''


def test_project_unspecified_platform(tmpdir):
    """
    import cppcheck project that contains the deprecated platform type "Unspecified"
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='Unspecified')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: unrecognized platform: 'Unspecified'.\n"
    assert stderr == ''


def test_project_unknown_platform(tmpdir):
    """
    import cppcheck project that contains an unknown platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, platform='dummy')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: unrecognized platform: 'dummy'.\n"
    assert stderr == ''


def test_project_empty_fields(tmpdir):
    """
    import cppcheck project that contains all empty fields - make sure there are no crashes
    """
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    with open(project_file, 'wt') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <root/>
  <builddir/>
  <includedir/>
  <includedir>
    <dir/>
  </includedir>
  <defines/>
  <defines>
    <define/>
  </defines>
  <undefines/>
  <undefines>
    <undefine/>
  </undefines>
  <importproject/>
  <paths/>
  <paths>
    <dir/>
  </paths>
  <exclude/>
  <exclude>
    <path/>
  </exclude>
  <function-contracts/>
  <variable-contracts/>
  <ignore/>
  <ignore>
    <path/>
  </ignore>
  <libraries/>
  <libraries>
    <library/>
  </libraries>
  <suppressions/>
  <suppressions>
    <suppression/>
  </suppressions>
  <vs-configurations/>
  <vs-configurations>
    <config/>
  </vs-configurations>
  <platform/>
  <analyze-all-vs-configs/>
  <parser/>
  <addons/>
  <addons>
    <addon/>
  </addons>
  <tags/>
  <tools/>
  <tools>
    <tool/>
  </tools>
  <check-headers/>
  <check-unused-templates/>
  <max-ctu-depth/>
  <max-template-recursion/>
  <check-unknown-function-return-values/>
  <safe-checks/>
  <safe-checks>
    <class-public/>
    <external-functions/>
    <internal-functions/>
    <external-variables/>
  </safe-checks>
  <tag-warnings/>
  <bug-hunting/>
  <cert-c-int-precision/>
  <coding-standards/>
  <coding-standards>
    <coding-standard/>
  </coding-standards>
</project>""")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout # do not crash
    assert stdout == 'cppcheck: error: no C or C++ source files found.\n'
    assert stderr == ''


def test_project_missing_subproject(tmpdir):
    """
    import cppcheck project that contains an unknown platform type
    """
    project_file = __write_cppcheck_project_file(tmpdir, importproject='dummy.json')

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: failed to open project '{}/dummy.json'. The file does not exist.\n".format(str(tmpdir).replace('\\', '/'))
    assert stderr == ''


def test_project_std(tmpdir):
    with open(os.path.join(tmpdir, 'bug1.cpp'), 'wt') as f:
        f.write("""
                #if __cplusplus == 201402L
                int x = 123 / 0;
                #endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')

    compilation_db = [
        {
            "directory": str(tmpdir),
            "command": "c++ -o bug1.o -c bug1.cpp -std=c++14",
            "file": "bug1.cpp",
            "output": "bug1.o"
        }
    ]

    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    ret, stdout, stderr = cppcheck(['--project=' + compile_commands, '--enable=all', '-rp=' + str(tmpdir), '--template=cppcheck1'])
    assert ret == 0, stdout
    assert (stderr == '[bug1.cpp:3]: (error) Division by zero.\n')



@pytest.mark.skip() # clang-tidy is not available in all cases
def test_clang_tidy(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                  (void)argc;
                }
                """)

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <paths>
   <dir name="{}"/>
  </paths>>
  <tools>
    <tool>clang-tidy</tool>
  </tools>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0, stdout
    lines = stdout.splitlines()
    # TODO: should detect clang-tidy issue
    assert len(lines) == 1
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]
    assert stderr == ''


def test_project_file_filter(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = ['--file-filter=*.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_project_file_filter_2(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_1, test_file_2))

    args = ['--file-filter=*.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file_1)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_project_file_filter_3(tmpdir):
    test_file_1 = os.path.join(tmpdir, 'test.cpp')
    with open(test_file_1, 'wt') as f:
        pass
    test_file_2 = os.path.join(tmpdir, 'test.c')
    with open(test_file_2, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_1, test_file_2))

    args = ['--file-filter=*.c', '--project={}'.format(project_file)]
    out_lines = [
        'Checking {} ...'.format(test_file_2)
    ]

    assert_cppcheck(args, ec_exp=0, err_exp=[], out_exp=out_lines)


def test_project_file_filter_no_match(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = ['--file-filter=*.c', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find any files matching the filter.'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_order(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass
    test_file_d = os.path.join(tmpdir, 'd.c')
    with open(test_file_d, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_c, test_file_d, test_file_b, test_file_a))

    args = ['--project={}'.format(project_file), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/4 files checked 0% done',
        'Checking {} ...'.format(test_file_d),
        '2/4 files checked 0% done',
        'Checking {} ...'.format(test_file_b),
        '3/4 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '4/4 files checked 0% done'
    ]
    assert stderr == ''


def test_project_file_duplicate(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_a, test_file_a, tmpdir))

    args = ['--project={}'.format(project_file)]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_a)
    ]
    assert stderr == ''


def test_project_file_duplicate_2(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass
    test_file_b = os.path.join(tmpdir, 'b.c')
    with open(test_file_b, 'wt'):
        pass
    test_file_c = os.path.join(tmpdir, 'c.c')
    with open(test_file_c, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file_c, test_file_a, test_file_b, tmpdir, test_file_b, test_file_c, test_file_a, tmpdir))

    args = ['--project={}'.format(project_file), '-j1']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    lines = stdout.splitlines()
    assert lines == [
        'Checking {} ...'.format(test_file_c),
        '1/3 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '2/3 files checked 0% done',
        'Checking {} ...'.format(test_file_b),
        '3/3 files checked 0% done'
    ]
    assert stderr == ''


def test_project_file_duplicate_3(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, tmpdir))

    args = ['--project={}'.format(project_file)]
    args.append('-j1') # TODO: remove when fixed

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0
    lines = stdout.splitlines()
    # TODO: only a single file should be checked
    if sys.platform == 'win32':
        assert lines == [
            'Checking {} ...'.format(test_file_a),
            '1/3 files checked 0% done',
            'Checking {} ...'.format(test_file_a),
            '2/3 files checked 0% done',
            'Checking {} ...'.format(test_file_a),
            '3/3 files checked 0% done'
        ]
    else:
        assert lines == [
            'Checking {} ...'.format(test_file_a),
            '1/2 files checked 0% done',
            'Checking {} ...'.format(test_file_a),
            '2/2 files checked 0% done'
        ]
    assert stderr == ''


@pytest.mark.skipif(sys.platform != 'win32', reason="requires Windows")
def test_project_file_duplicate_4(tmpdir):
    test_file_a = os.path.join(tmpdir, 'a.c')
    with open(test_file_a, 'wt'):
        pass

    # multiple ways to specify the same file
    in_file_a = 'a.c'
    in_file_b = os.path.join('.', 'a.c')
    in_file_c = os.path.join('dummy', '..', 'a.c')
    in_file_d = os.path.join(tmpdir, 'a.c')
    in_file_e = os.path.join(tmpdir, '.', 'a.c')
    in_file_f = os.path.join(tmpdir, 'dummy', '..', 'a.c')

    args1 = [in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, str(tmpdir)]
    args2 = []
    for a in args1:
        args2.append(a.replace('\\', '/'))

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
        <dir name="{}"/>
    </paths>
</project>""".format(in_file_a, in_file_b, in_file_c, in_file_d, in_file_e, in_file_f, tmpdir,
                     args2[0], args2[1], args2[2], args2[3], args2[4], args2[5], args2[6]))

    args = ['--project={}'.format(project_file)]
    args.append('-j1') # TODO: remove when fixed

    exitcode, stdout, stderr = cppcheck(args, cwd=tmpdir)
    assert exitcode == 0
    lines = stdout.splitlines()
    # TODO: only a single file should be checked
    assert lines == [
        'Checking {} ...'.format(test_file_a),
        '1/3 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '2/3 files checked 0% done',
        'Checking {} ...'.format(test_file_a),
        '3/3 files checked 0% done'
    ]
    assert stderr == ''

def test_project_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file))

    args = ['-itest.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_ignore_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
    <exclude>
        <path name="test.cpp"/>
    </exclude>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_project_file_ignore_3(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """<?xml version="1.0" encoding="UTF-8"?>
<project>
    <paths>
        <dir name="{}"/>
    </paths>
    <ignore>
        <path name="test.cpp"/>
    </ignore>
</project>""".format(test_file))

    args = ['--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: could not find or open any of the paths given.',
        'cppcheck: Maybe all paths were ignored?'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


@pytest.mark.xfail
def test_json_file_ignore(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]

    project_file = os.path.join(tmpdir, 'test.json')
    with open(project_file, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = ['-itest.cpp', '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


def test_json_file_ignore_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        pass

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]

    project_file = os.path.join(tmpdir, 'test.json')
    with open(project_file, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = ['-i{}'.format(test_file), '--project={}'.format(project_file)]
    out_lines = [
        'cppcheck: error: no C or C++ source files found.',
        'cppcheck: all paths were ignored'
    ]

    assert_cppcheck(args, ec_exp=1, err_exp=[], out_exp=out_lines)


@pytest.mark.xfail(strict=True)
def test_project_D(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#ifndef __GNUC__
#error "requirement not met"
#endif
                """)

    project_file = os.path.join(tmpdir, 'test.cppcheck')
    with open(project_file, 'wt') as f:
        f.write(
            """
<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
  <paths>
   <dir name="{}"/>
  </paths>
</project>
            """.format(test_file))

    args = [
        '--project=' + project_file,
        '--template=simple',
    ]
    arg_D = ['-D__GNUC__']

    out_expected = [
        'Checking {} ...'.format(test_file),
        'Checking {}: __GNUC__=1...'.format(test_file)
    ]

    args1 = args + arg_D
    ret, stdout, stderr = cppcheck(args1)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout

    # TODO: -D__GNUC__ is lost
    args2 = arg_D + args
    ret, stdout, stderr = cppcheck(args2)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_compdb_D(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#ifndef __GNUC__
#error "requirement not met"
#endif
                """)

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o test.o -c test.cpp",
         "file": "test.cpp",
         "output": "test.o"}
    ]
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    args = [
        '--project=' + compile_commands,
        '--template=simple',
        ]
    arg_D = ['-D__GNUC__']

    out_expected = [
        'Checking {} ...'.format(test_file),
        'Checking {}: __GNUC__=1;...'.format(test_file)  # TODO: get rid of extra ;
    ]

    args1 = args + arg_D
    ret, stdout, stderr = cppcheck(args1)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout

    args2 = arg_D + args
    ret, stdout, stderr = cppcheck(args2)
    assert stdout.splitlines() == out_expected
    assert stderr.splitlines() == []
    assert ret == 0, stdout
