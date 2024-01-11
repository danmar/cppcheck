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

def _test_library_includes(tmpdir, libs, content, libinc_obj=None):
    if libinc_obj is None:
        library_includes = LibraryIncludes()
    else:
        library_includes = libinc_obj

    src_file = os.path.join(str(tmpdir), "file.cpp")
    with open(src_file, 'w') as f:
        f.write(content)
    libs.sort()
    libs_found = library_includes.get_libraries(str(tmpdir))
    libs_found.sort()
    assert libs == libs_found

def test_library_includes(tmpdir):
    _test_library_includes(tmpdir, ['posix', 'gnu'], '')
    _test_library_includes(tmpdir, ['posix', 'gnu'], '#include <stdio.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'boost'], '#include <boost/regex.hpp>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'python'], '#include "Python.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'libcerror', 'lua', 'opengl', 'qt'], '#include <QApplication>\n#include <GL/gl.h>\r#include "lua.h"\r\n#include <libcerror.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'microsoft_sal'], '  #include <sal.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'googletest'], '\t#include <gtest/gtest.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'microsoft_atl'], '  \t  #include <atlbase.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'cairo'], '\t  #include <cairo.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'gtk'], '  \t#include <glib-object.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd'], '#include <sys/uio.h>\r\n')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'libcurl'], '#include <curl/curl.h>\r')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'sqlite3'], '#include <sqlite3.h>\n')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'openmp'], '#  include <omp.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'mfc'], '#\tinclude <afxwin.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'ruby'], '#  \tinclude "ruby.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'zlib'], '#\t  include <zlib.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'pcre'], '#include<pcre.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'pcre'], '#include  "pcre.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'opengl'], '#include\t  <GL/glut.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'nspr'], '#include\t"prtypes.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'lua'], '#include  \t<lua.h>')

def test_match_multiple_time(tmpdir):
    libinc = LibraryIncludes()

    # there was a bug that we would only match each library once successfully
    _test_library_includes(tmpdir, ['posix', 'gnu', 'zlib'], '#include <zlib.h>', libinc)
    _test_library_includes(tmpdir, ['posix', 'gnu', 'zlib'], '#include <zlib.h>', libinc)
