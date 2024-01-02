# Running the test with Python 2:
# Be sure to install pytest version 4.6.4 (newer should also work)
# Command in cppcheck directory:
# python -m pytest addons/test/test-cppcheckdata.py
#
# Running the test with Python 3:
# Command in cppcheck directory:
# PYTHONPATH=./addons python3 -m pytest addons/test/test-cppcheckdata.py

import sys
import pytest

from addons.cppcheck import cppcheckdata


TEST_SOURCE_FILES = ['./addons/test/y2038/y2038-test-1-bad-time-bits.c']

from .util import dump_create, dump_remove, convert_json_output

def setup_module(module):
    sys.argv.append("--cli")
    for f in TEST_SOURCE_FILES:
        dump_create(f)


def teardown_module(module):
    sys.argv.remove("--cli")
    for f in TEST_SOURCE_FILES:
        dump_remove(f)


def test_format_print_regression():
    for f in TEST_SOURCE_FILES:
        data = cppcheckdata.CppcheckData(f + ".dump")
        for cfg in data.iterconfigurations():
            for token in cfg.tokenlist:
                print(f'{token.str} : {token.valueType}')
                if token.values:
                    for value in token.values:
                        print(f'    {value}')

