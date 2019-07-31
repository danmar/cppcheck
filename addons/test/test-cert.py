# python -m pytest addons/test/test-cert.py

import sys
import pytest


def test_arguments_regression():
    args_ok = ["-q", "--quiet",
               "-verify",
               "--cli"]
    # Arguments with expected SystemExit
    args_exit = ["--non-exists", "--non-exists-param=42", "-h", "--help"]

    from addons.cert import get_args

    for arg in args_exit:
        sys.argv.append(arg)
        with pytest.raises(SystemExit):
            get_args()
        sys.argv.remove(arg)

    for arg in args_ok:
        sys.argv.append(arg)
        try:
            get_args()
        except SystemExit:
            pytest.fail("Unexpected SystemExit with '%s'" % arg)
        sys.argv.remove(arg)
