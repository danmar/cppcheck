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


def __write_cppcheck_project_file(tmpdir, platform=None):
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    if platform is not None:
        platform = '<platform>{}</platform>'.format(platform)

    with open(project_file, 'wt') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
    {}
    <paths>
        <dir name="."/>
    </paths>
</project>""".format(platform)
        )

    return project_file


def test_project_custom_platform(tmpdir):
    """
    import cppcheck project that contains a custom platform file
    """
    project_file = __write_cppcheck_project_file(tmpdir, 'p1.xml')

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
    project_file = __write_cppcheck_project_file(tmpdir, '')

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
    project_file = __write_cppcheck_project_file(tmpdir, 'Unspecified')

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
    project_file = __write_cppcheck_project_file(tmpdir, 'dummy')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 1, stdout
    assert stdout == "cppcheck: error: unrecognized platform: 'dummy'.\n"
    assert stderr == ''
