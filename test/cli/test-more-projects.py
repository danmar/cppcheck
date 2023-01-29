# python -m pytest test-more-projects.py
import json
import os
from testutils import cppcheck


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

    ret, stdout, stderr = cppcheck(['--platform=native', '--project=' + project_file, '--template=cppcheck1', '-q'])
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

    ret, stdout, stderr = cppcheck(['--platform=native', '--project=' + project_file, '--template=cppcheck1', '-q'])
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

    ret, stdout, stderr = cppcheck(['--platform=native', '--project=' + project_file, '--template=cppcheck1', '-q'])
    assert ret == 0, stdout
    assert stdout == "cppcheck: 'Unspecified' is a deprecated platform type and will be removed in Cppcheck 2.14. Please use 'unspecified' instead.\n"
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
    <paths/>
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

    ret, stdout, stderr = cppcheck(['--platform=native', '--project=' + project_file, '--template=cppcheck1'])
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