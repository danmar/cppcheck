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


def test_project_custom_platform(tmpdir):
    """
    import cppcheck project that contains a custom platform file
    """
    project_file = os.path.join(tmpdir, 'Project.cppcheck')

    with open(project_file, 'wt') as f:
        f.write("""
                <?xml version="1.0" encoding="UTF-8"?>
                <project version="1">
                  <platform>p1.xml</platform>
                  <paths>
                    <dir name="."/>
                  </paths>
                </project>
                """)

    with open(os.path.join(tmpdir, 'p1.xml'), 'wt') as f:
        f.write('<?xml version="1.0"?>\n<platform/>')

    with open(os.path.join(tmpdir, '1.c'), 'wt') as f:
        f.write("int x;")

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert ret == 0, stdout
    assert stderr == ''
