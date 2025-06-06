import os
import sys
import pytest

from testutils import cppcheck_ex, cppcheck

def __remove_std_lookup_log(l : list, exepath):
    l.remove("looking for library 'std.cfg'")
    l.remove("looking for library '{}/std.cfg'".format(exepath))
    l.remove("looking for library '{}/cfg/std.cfg'".format(exepath))
    return l


def test_lib_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        # TODO: lookup of '{exepath}/none' missing - could conflict with the platform lookup though
        "looking for library '{}/none.cfg'".format(exepath),
        # TODO: lookup of '{exepath}/cfg/none' missing
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=none.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'none.cfg'",
        "looking for library '{}/none.cfg'".format(exepath),
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none.cfg'",
        "cppcheck: Failed to load library configuration file 'none.cfg'. File not found"
    ]


def test_lib_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu.xml', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu.xml'",
        "looking for library '{}/config/gnu.xml'".format(exepath),
        "looking for library '{}/cfg/config/gnu.xml'".format(exepath),
        "library not found: 'config/gnu.xml'",
        "cppcheck: Failed to load library configuration file 'config/gnu.xml'. File not found"
    ]


def test_lib_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu.cfg'",
        "looking for library '{}/config/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/config/gnu.cfg'".format(exepath),
        "library not found: 'config/gnu'",
        "cppcheck: Failed to load library configuration file 'config/gnu'. File not found"
    ]


def test_lib_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')
    with open(cfg_file, 'wt') as f:
        f.write('''
<?xml version="1.0"?>
<def format="2">
</def>
        ''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file),
        "library not found: '{}'".format(cfg_file),
        "cppcheck: Failed to load library configuration file '{}'. File not found".format(cfg_file)
    ]


def test_lib_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    gtk_cfg_dir = os.path.join(tmpdir, 'gtk.cfg')
    os.mkdir(gtk_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gtk', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gtk.cfg'",
        "looking for library '{}/gtk.cfg'".format(exepath),
        "looking for library '{}/cfg/gtk.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


# make sure we bail out when we encounter an invalid file
def test_lib_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    gnu_cfg_file = os.path.join(tmpdir, 'gnu.cfg')
    with open(gnu_cfg_file, 'wt') as f:
        f.write('''{}''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "library not found: 'gnu'",
        "Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1",  # TODO: log the failure before saying the library was not found
        "cppcheck: Failed to load library configuration file 'gnu'. Bad XML"
    ]


def test_lib_lookup_multi(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=posix,gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'posix.cfg'",
        "looking for library '{}/posix.cfg'".format(exepath),
        "looking for library '{}/cfg/posix.cfg'".format(exepath),
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_builtin(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=unix64', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    # built-in platform are not being looked up
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]


@pytest.mark.skip  # TODO: perform additional lookups when run via symlink in CI
def test_platform_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=avr8', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=avr8.xml",
        "try to load platform file 'platforms/avr8.xml' ... Success",
        'Checking {} ...'.format(test_file)
    ]


@pytest.mark.skip  # TODO: perform additional lookups when run via symlink in CI
def test_platform_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=avr8.xml', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8.xml' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=avr8.xml",
        "try to load platform file 'platforms/avr8.xml' ... Success",
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
        exepath_bin += '.exe'
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none' in '{}'".format(exepath_bin),  # TODO: this is not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=none.xml",
        "try to load platform file 'platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/none.xml",
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none'."
    ]


def test_platform_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none.xml', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
        exepath_bin += '.exe'
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none.xml' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=none.xml",
        "try to load platform file 'platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/none.xml",
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none.xml'."
    ]


def test_platform_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=platform/none.xml', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
        exepath_bin += '.exe'
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'platform/none.xml' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platform/none.xml",
        "try to load platform file 'platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/platform/none.xml",
        "try to load platform file '{}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'platform/none.xml'."
    ]


def test_platform_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=platform/none', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
        exepath_bin += '.exe'
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'platform/none' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platform/none.xml",
        "try to load platform file 'platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/platform/none.xml",
        "try to load platform file '{}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'platform/none'."
    ]


def test_platform_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')
    with open(platform_file, 'wt') as f:
        f.write('''
<platform format="2"/>
        ''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform={}'.format(platform_file), test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}' in '{}'".format(platform_file, exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file '{}' ... Success".format(platform_file),
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform={}'.format(platform_file), test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}' in '{}'".format(platform_file, exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file '{}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}".format(platform_file, platform_file),
        "cppcheck: error: unrecognized platform: '{}'.".format(platform_file)
    ]


@pytest.mark.skip  # TODO: perform additional lookups when run via symlink in CI
def test_platform_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    avr8_cfg_dir = os.path.join(tmpdir, 'avr8.xml')
    os.mkdir(avr8_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=avr8', test_file])
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=avr8.xml",
        "try to load platform file 'platforms/avr8.xml' ... Success",
        'Checking {} ...'.format(test_file)
    ]


# make sure we bail out when we encounter an invalid file
@pytest.mark.xfail(strict=True)  # TODO: does not bail out after it found an invalid file
def test_platform_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    avr8_file = os.path.join(tmpdir, 'avr8.xml')
    with open(avr8_file, 'wt') as f:
        f.write('''{}''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=avr8', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    exepath_bin = os.path.join(exepath, 'cppcheck')
    if sys.platform == 'win32':
        exepath_bin += '.exe'
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8' in '{}'".format(exepath_bin),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'avr8.xml' ... Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1",
        "cppcheck: error: unrecognized platform: 'avr8'."
    ]


def test_addon_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        "looking for addon '{}addon/misra.py'".format(exepath_sep),
        "looking for addon '{}addons/addon/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


def test_addon_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        "looking for addon '{}addon/misra.py'".format(exepath_sep),
        "looking for addon '{}addons/addon/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


def test_addon_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')
    with open(addon_file, 'wt') as f:
        f.write('''''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon={}'.format(addon_file), test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file),
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon={}'.format(addon_file), test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file),
        "looking for addon '{}{}'".format(exepath_sep, addon_file),  # TODO: should not perform this lookup
        "looking for addon '{}addons/{}'".format(exepath_sep, addon_file),  # TODO: should not perform this lookup
        'Did not find addon {}'.format(addon_file)
    ]


def test_addon_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    misra_dir = os.path.join(tmpdir, 'misra')
    os.mkdir(misra_dir)
    misra_cfg_dir = os.path.join(tmpdir, 'misra.py')
    os.mkdir(misra_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr  # TODO. should fail when addon is not found
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


# make sure we bail out when we encounter an invalid file
def test_addon_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    misra_py_file = os.path.join(tmpdir, 'misra.py')
    with open(misra_py_file, 'wt') as f:
        f.write('''<def/>''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon=misra', test_file], cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        'Checking {} ...'.format(test_file)  # TODO: should bail out
    ]


@pytest.mark.skip  # TODO
def test_config_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    # TODO: needs to be in exepath so this is found
    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', '--addon=misra', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for '{}cppcheck.cfg'".format(exepath_sep),
        'no configuration found',
        'Checking {} ...'.format(test_file)
    ]


def test_config_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for '{}cppcheck.cfg'".format(exepath_sep),
        'no configuration found',
        'Checking {} ...'.format(test_file)
    ]

# TODO: test handling of invalid configuration

# TODO: test with FILESDIR