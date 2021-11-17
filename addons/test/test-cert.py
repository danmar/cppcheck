# Running the test with Python 2:
# Be sure to install pytest version 4.6.4 (newer should also work)
# Command in cppcheck directory:
# python -m pytest addons/test/test-cert.py
#
# Running the test with Python 3:
# Command in cppcheck directory:
# PYTHONPATH=./addons python3 -m pytest addons/test/test-cert.py

import sys
import pytest


def test_arguments_regression():
    args_ok = ["-q", "--quiet",
               "-verify",
               "--cli"]
    # Arguments with expected SystemExit
    args_exit = ["--non-exists", "--non-exists-param=42", "-h", "--help"]

    from addons.cert import get_args_parser

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
