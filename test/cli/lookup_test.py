import os
import sys
import pytest

from testutils import cppcheck_ex, cppcheck

def __remove_std_lookup_log(l : list, exepath):
    l.remove("looking for library 'std.cfg'")
    l.remove("looking for library '{}/std.cfg'".format(exepath))
    l.remove("looking for library '{}/cfg/std.cfg'".format(exepath))
    return l


# TODO: test with FILESDIR
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
        "looking for library 'gnu'",
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


# TODO: test with FILESDIR
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
        "looking for library 'none'",  # TODO: this could conflict with the platform lookup
        "looking for library 'none.cfg'",
        # TODO: lookup of '{exepath}/none' missing - could conflict with the platform lookup though
        "looking for library '{}/none.cfg'".format(exepath),
        # TODO: lookup of '{exepath}/cfg/none' missing
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
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
    gtk_dir = os.path.join(tmpdir, 'gtk')
    os.mkdir(gtk_dir)
    gtk_cfg_dir = os.path.join(tmpdir, 'gtk.cfg')
    os.mkdir(gtk_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gtk', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gtk'",
        "looking for library 'gtk.cfg'",
        "looking for library '{}/gtk.cfg'".format(exepath),
        "looking for library '{}/cfg/gtk.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
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
        "looking for library 'posix'",
        "looking for library 'posix.cfg'",
        "looking for library '{}/posix.cfg'".format(exepath),
        "looking for library '{}/cfg/posix.cfg'".format(exepath),
        "looking for library 'gnu'",
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


# TODO: behaves differently when using a CMake build
# TODO: test with FILESDIR
@pytest.mark.skip
def test_platform_lookup_external(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=avr8', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8' in '{}'".format(os.path.join(exepath, 'cppcheck')),  # TODO: this not not the path *of* the executable but the the path *to* the executable
        "try to load platform file 'avr8' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=avr8",
        "try to load platform file 'avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=avr8.xml",
        "try to load platform file 'platforms/avr8' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/avr8",
        "try to load platform file 'platforms/avr8.xml' ... Success",
        'Checking {} ...'.format(test_file)
    ]


# TODO: test with FILESDIR
def test_platform_lookup_external_notfound(tmpdir):
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
        "try to load platform file 'none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=none",
        "try to load platform file 'none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=none.xml",
        "try to load platform file 'platforms/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/none",
        "try to load platform file 'platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=platforms/none.xml",
        "try to load platform file '{}/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none".format(exepath, exepath),
        # TODO: lookup of '{exepath}/none.xml' missing
        "try to load platform file '{}/platforms/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none'."
    ]


# TODO: test with FILESDIR
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


# TODO: test with FILESDIR
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


# TODO: test with FILESDIR
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


# TODO: test with FILESDIR
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


# TODO: test with FILESDIR
@pytest.mark.skip
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


# TODO: test with FILESDIR
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