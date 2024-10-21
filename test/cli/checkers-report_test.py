# python -m pytest checkers-report_test.py

import os
import xml.etree.ElementTree as ET

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__project_dir = os.path.join(__script_dir, 'checkers-testing')

def test_xml_checkers_report():
    test_file = os.path.join(__project_dir, 'test1.cpp')
    args = ['--xml-version=3', '--enable=all', test_file]

    exitcode, _, stderr = cppcheck(args)
    assert exitcode == 0
    assert ET.fromstring(stderr)