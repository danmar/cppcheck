#!/usr/bin/env python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2022 Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from donate_cpu_lib import *

def _test_library_includes(tmpdir, libs, content):
    library_includes = LibraryIncludes()

    src_file = os.path.join(str(tmpdir), "file.cpp")
    with open(src_file, 'w') as f:
        f.write(content)
    assert libs.sort() == library_includes.get_libraries(str(tmpdir)).sort()

def test_library_includes(tmpdir):
    _test_library_includes(tmpdir, ['posix', 'gnu'], '')
    _test_library_includes(tmpdir, ['posix', 'gnu'], '#include <stdio.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'boost'], '#include <boost/regex.hpp>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'python'], '#include "Python.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'lua', 'opengl', 'qt'], '#include <QApplication>\n#include <GL/gl.h>\n#include "lua.h"')
