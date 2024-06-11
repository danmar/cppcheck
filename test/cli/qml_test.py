# python3 -m pytest test-qml.py

import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

# there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
__project_dir = os.path.join(__script_dir, 'QML-Samples-TableView')
__project_dir_sep = __project_dir + os.path.sep


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--library=qt', '--enable=unusedFunction', '-j1', __project_dir])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


def test_unused_functions_j():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--library=qt', '--enable=unusedFunction', '-j2', __project_dir])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check can't be used with '-j' option. Disabling unusedFunction check."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout # TODO: abil out on this


# TODO: fillSampleData is not unused
def test_unused_functions_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--library=qt', '--enable=unusedFunction', f'--cppcheck-build-dir={build_dir}', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:47:0: style: The function 'fillSampleData' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]",
    ]
    assert ret == 0, stdout


# TODO: fillSampleData is not unused
def test_unused_functions_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--library=qt', '--enable=unusedFunction', '-j2', f'--cppcheck-build-dir={build_dir}', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:47:0: style: The function 'fillSampleData' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]",
        f"{__project_dir_sep}samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]",
    ]
    assert ret == 0, stdout

# TODO: test with project file
# TODO: test with FileSettings
