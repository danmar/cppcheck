# python -m pytest test-projects.py

import pytest
import os
import json
from testutils import cppcheck


@pytest.mark.parametrize("project_ext", ["json", "sln", "vcxproj", "bpr", "cppcheck"])
def test_missing_project(project_ext):
    project_file = "file.{}".format(project_ext)

    ret, stdout, stderr = cppcheck(['--project=' + project_file, '--template=cppcheck1'])
    assert 1 == ret
    assert "cppcheck: Failed to open project '{}'. The file does not exist.\n".format(project_file) == stdout
    assert "" == stderr


@pytest.mark.parametrize("project_ext, expected", [
    ("json", "compilation database is not a JSON array"),
    ("sln", "Visual Studio solution file is empty"),
    ("vcxproj", "Visual Studio project file is not a valid XML - XML_ERROR_EMPTY_DOCUMENT"),
    ("bpr", "Borland project file is not a valid XML - XML_ERROR_EMPTY_DOCUMENT"),
    ("cppcheck", "Cppcheck GUI project file cannot be parsed - XML_ERROR_EMPTY_DOCUMENT")
])
def test_empty_project(tmpdir, project_ext, expected):
    project_file = os.path.join(tmpdir, "file.{}".format(project_ext))

    with open(project_file, 'w'):
        pass

    ret, stdout, stderr = cppcheck(['--project=' + str(project_file), '--template=cppcheck1'])
    assert 1 == ret
    assert expected + "\ncppcheck: Failed to load project '{}'. An error occurred.\n".format(project_file) == stdout
    assert "" == stderr


def test_compile_db_entry_file_not_found(tmpdir):
    project_file = os.path.join(tmpdir, "file.json")

    compilation_db = [
        {"directory": str(tmpdir),
         "command": "c++ -o bug1.o -c bug1.cpp",
         "file": "bug1.cpp",
         "output": "bug1.o"}
    ]

    with open(project_file, 'w') as f:
        f.write(json.dumps(compilation_db))

    expected = "'{}' from compilation database does not exist".format(os.path.join(tmpdir, "bug1.cpp"))

    ret, stdout, stderr = cppcheck(['--project=' + str(project_file)])
    assert 1 == ret
    assert expected + "\ncppcheck: Failed to load project '{}'. An error occurred.\n".format(project_file) == stdout
    assert "" == stderr
