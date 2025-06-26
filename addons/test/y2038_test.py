# Running the test with Python 2:
# Be sure to install pytest version 4.6.4 (newer should also work)
# Command in cppcheck directory:
# python -m pytest addons/test/test-y2038.py
#
# Running the test with Python 3:
# Command in cppcheck directory:
# PYTHONPATH=./addons python3 -m pytest addons/test/test-y2038.py

import sys
import pytest

from addons.y2038 import check_y2038_safe

from .util import dump_create, dump_remove, convert_json_output


TEST_SOURCE_FILES = ['./addons/test/y2038/y2038-test-1-bad-time-bits.c',
                     './addons/test/y2038/y2038-test-2-no-time-bits.c',
                     './addons/test/y2038/y2038-test-3-no-use-time-bits.c',
                     './addons/test/y2038/y2038-test-4-good.c',
                     './addons/test/y2038/y2038-test-5-good-no-time-used.c']

# Compiler flag test file (handled separately in individual test methods)
COMPILER_FLAG_TEST_FILE = './addons/test/y2038/y2038-test-compiler-flags.c'

# Compiler flag test configurations - using single source file with different flags
COMPILER_FLAG_TESTS = {
    'good': {
        'file': './addons/test/y2038/y2038-test-compiler-flags.c',
        'flags': ['-D_TIME_BITS=64', '-D_FILE_OFFSET_BITS=64', '-D_USE_TIME_BITS64'],
        'description': 'Proper Y2038 configuration'
    },
    'bad': {
        'file': './addons/test/y2038/y2038-test-compiler-flags.c',
        'flags': ['-D_TIME_BITS=32'],
        'description': 'Wrong _TIME_BITS value'
    },
    'use_bits64_only': {
        'file': './addons/test/y2038/y2038-test-compiler-flags.c',
        'flags': ['-D_USE_TIME_BITS64'],
        'description': 'Incomplete configuration (_USE_TIME_BITS64 without _TIME_BITS)'
    }
}


def setup_module(module):
    sys.argv.append("--cli")

    # Create dumps for regular test files
    for f in TEST_SOURCE_FILES:
        dump_create(f)

    # For compiler flag tests, we'll create dumps on-demand in each test
    # to avoid conflicts from multiple dump_create calls on the same file


def teardown_module(module):
    sys.argv.remove("--cli")
    for f in TEST_SOURCE_FILES:
        dump_remove(f)

    # Compiler flag test dumps are cleaned up individually in each test method


def test_1_bad_time_bits(capsys):
    is_safe = check_y2038_safe('./addons/test/y2038/y2038-test-1-bad-time-bits.c.dump', quiet=True)
    assert(is_safe is False)
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    json_output = convert_json_output(captured)

    # Has exactly one warnings of _TIME_BITS and _USE_TIME_BITS64 kind.
    assert(len(json_output['type-bits-undef']) == 1)
    assert(len(json_output['type-bits-not-64']) == 1)

    # There are 2 unsafe calls in test source and 3 in y2038-in.h
    unsafe_calls = json_output['unsafe-call']
    assert(len([c for c in unsafe_calls if c['file'].endswith('h')]) == 3)
    assert(len([c for c in unsafe_calls if c['file'].endswith('c')]) == 0)


def test_2_no_time_bits(capsys):
    is_safe = check_y2038_safe('./addons/test/y2038/y2038-test-2-no-time-bits.c.dump', quiet=True)
    assert(is_safe is False)
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    json_output = convert_json_output(captured)

    # _USE_TIME_BITS64 defined in y2038-inc.h header, but there is not
    # _TIME_BITS definition. Here must be appropriate warning.
    assert(len(json_output['type-bits-undef']) == 1)
    assert(json_output.get('type-bits-not-64') is None)

    # y2038-in.h still has y2038-unsafe calls.
    unsafe_calls = json_output['unsafe-call']
    assert(len([c for c in unsafe_calls if c['file'].endswith('h')]) == 3)


def test_3_no_use_time_bits(capsys):
    is_safe = check_y2038_safe('./addons/test/y2038/y2038-test-3-no-use-time-bits.c.dump', quiet=True)
    assert(is_safe is False)
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    json_output = convert_json_output(captured)

    # Included bad _USE_TIME_BITS64 definition must trigger the errors.
    unsafe_calls = json_output['unsafe-call']
    assert(len(unsafe_calls) == 2)


def test_4_good(capsys):
    # pylint: disable-next=unused-variable - FIXME
    is_safe = check_y2038_safe('./addons/test/y2038/y2038-test-4-good.c.dump', quiet=True)
    # assert(is_safe is True) # FIXME: This should be a "good" example returning "True" instead of "False"
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    json_output = convert_json_output(captured)

    # Defined _TIME_BITS equal to 64 so that glibc knows we want Y2038 support.
    # There are no warnings from C sources.
    unsafe_calls = json_output['unsafe-call']
    assert(len([c for c in unsafe_calls if c['file'].endswith('.c')]) == 0)


def test_5_good(capsys):
    is_safe = check_y2038_safe('./addons/test/y2038/y2038-test-5-good-no-time-used.c.dump', quiet=True)
    assert(is_safe is True)
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    json_output = convert_json_output(captured)

    # There are no warnings from C sources.
    if 'unsafe-call' in json_output:
        unsafe_calls = json_output['unsafe-call']
        assert(len([c for c in unsafe_calls if c['file'].endswith('.c')]) == 0)


def test_compiler_flags_proper_y2038_config(capsys):
    """Test proper Y2038 configuration: -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64 -D_USE_TIME_BITS64"""
    from addons.test.util import dump_create, dump_remove

    test_file = COMPILER_FLAG_TEST_FILE
    try:
        # Create dump with proper Y2038 flags
        dump_create(test_file, '-D_TIME_BITS=64', '-D_FILE_OFFSET_BITS=64', '-D_USE_TIME_BITS64')
        is_safe = check_y2038_safe(test_file + '.dump', quiet=True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = convert_json_output(captured)

        # When using proper compiler flags, there should be no Y2038 errors and warnings should be suppressed
        assert is_safe is True
        assert json_output.get('type-bits-not-64') is None
        assert json_output.get('unsafe-call') is None
        # Check that suppression message was printed
        output_text = '\n'.join(captured)
        assert 'Y2038 warnings suppressed' in output_text or len(captured) == 0  # May be empty due to quiet mode
    finally:
        # Clean up dump file
        try:
            dump_remove(test_file)
        except FileNotFoundError:
            pass  # File may not exist if test failed early


def test_compiler_flags_wrong_time_bits_value(capsys):
    """Test wrong _TIME_BITS value: -D_TIME_BITS=32"""
    # Note: This test uses a separate dump file created with -D_TIME_BITS=32 flags
    # We need to create this dump separately since we can't reuse the main dump
    from addons.test.util import dump_create

    test_file = COMPILER_FLAG_TEST_FILE
    temp_dump = test_file + '.temp_bad.dump'

    try:
        # Create temporary dump with wrong flags
        dump_create(test_file, '-D_TIME_BITS=32')
        # Move the created dump to our temp name to avoid conflicts
        import shutil
        shutil.move(test_file + '.dump', temp_dump)
        is_safe = check_y2038_safe(temp_dump, quiet=True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = convert_json_output(captured)

        # When using wrong _TIME_BITS value in compiler flags, should trigger error
        assert is_safe is False
        assert len(json_output.get('type-bits-not-64', [])) == 1

        # Should also have unsafe function calls since _TIME_BITS=32
        unsafe_calls = json_output.get('unsafe-call', [])
        assert len(unsafe_calls) > 0
        # Verify the error message mentions compiler flags
        error_msg = json_output['type-bits-not-64'][0]['message']
        assert 'compiler flags' in error_msg.lower() or '_TIME_BITS=32' in error_msg
    finally:
        # Clean up temporary dump file
        import os
        if os.path.exists(temp_dump):
            os.remove(temp_dump)


def test_compiler_flags_incomplete_config(capsys):
    """Test incomplete configuration: -D_USE_TIME_BITS64 without _TIME_BITS"""
    # Note: This test uses a separate dump file created with -D_USE_TIME_BITS64 only
    from addons.test.util import dump_create

    test_file = COMPILER_FLAG_TEST_FILE
    temp_dump = test_file + '.temp_incomplete.dump'

    try:
        # Create temporary dump with incomplete flags
        dump_create(test_file, '-D_USE_TIME_BITS64')
        # Move the created dump to our temp name to avoid conflicts
        import shutil
        shutil.move(test_file + '.dump', temp_dump)
        is_safe = check_y2038_safe(temp_dump, quiet=True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = convert_json_output(captured)

        # When using _USE_TIME_BITS64 without _TIME_BITS, should trigger warning
        assert is_safe is False
        assert len(json_output.get('type-bits-undef', [])) == 1

        # Should have unsafe function calls since proper Y2038 config is missing
        unsafe_calls = json_output.get('unsafe-call', [])
        assert len(unsafe_calls) > 0
        # Verify the warning message mentions compiler flags and _TIME_BITS
        warning_msg = json_output['type-bits-undef'][0]['message']
        assert '_USE_TIME_BITS64' in warning_msg and '_TIME_BITS' in warning_msg
    finally:
        # Clean up temporary dump file
        import os
        if os.path.exists(temp_dump):
            os.remove(temp_dump)


def test_arguments_regression():
    args_ok = ["-t=foo", "--template=foo",
               "-q", "--quiet",
               "--cli"]
    # Arguments with expected SystemExit
    args_exit = ["--non-exists", "--non-exists-param=42", "-h", "--help"]

    from addons.y2038 import get_args_parser

    # sys.argv contains all pytest arguments - so clear all existing arguments first and restore afterwards
    sys_argv_old = sys.argv
    sys.argv = [sys.argv[0]]

    try:
        for arg in args_exit:
            sys.argv.append(arg)
            with pytest.raises(SystemExit):
                parser = get_args_parser()
                parser.parse_args()
            sys.argv.remove(arg)

        for arg in args_ok:
            sys.argv.append(arg)
            try:
                parser = get_args_parser()
                parser.parse_args()
            except SystemExit:
                pytest.fail("Unexpected SystemExit with '%s'" % arg)
            sys.argv.remove(arg)
    finally:
        sys.argv = sys_argv_old


def test_parse_compiler_flags():
    """Test the new parse_compiler_flags function"""
    from addons.y2038 import parse_compiler_flags
    # Test _TIME_BITS=64
    result = parse_compiler_flags("_TIME_BITS=64")
    assert result['time_bits_defined'] is True
    assert result['time_bits_value'] == 64
    assert result['use_time_bits64_defined'] is False

    # Test _TIME_BITS=32
    result = parse_compiler_flags("_TIME_BITS=32")
    assert result['time_bits_defined'] is True
    assert result['time_bits_value'] == 32
    assert result['use_time_bits64_defined'] is False

    # Test _USE_TIME_BITS64
    result = parse_compiler_flags("_USE_TIME_BITS64=1")
    assert result['time_bits_defined'] is False
    assert result['time_bits_value'] is None
    assert result['use_time_bits64_defined'] is True

    # Test both flags
    result = parse_compiler_flags("_TIME_BITS=64;_USE_TIME_BITS64=1")
    assert result['time_bits_defined'] is True
    assert result['time_bits_value'] == 64
    assert result['use_time_bits64_defined'] is True

    # Test no flags
    result = parse_compiler_flags("")
    assert result['time_bits_defined'] is False
    assert result['time_bits_value'] is None
    assert result['use_time_bits64_defined'] is False