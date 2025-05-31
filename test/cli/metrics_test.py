# python -m pytest metrics_test.py

import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__addon_path = os.path.join(__script_dir, 'metrics_test', 'dummy_addon.py')
__source_path = os.path.join(__script_dir, 'metrics_test', 'dummy_file.c')
__expected_xml = [
    '<metric fileName="1.cpp" function="write" id="HISCall" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISGoto" lineNumber="6" value="0"/>',
    '<metric fileName="1.cpp" function="write" id="HISLevel" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISParam" lineNumber="6" value="2"/>',
    '<metric fileName="1.cpp" function="write" id="HISPath" lineNumber="6" value="3"/>',
    '<metric fileName="1.cpp" function="write" id="HISReturn" lineNumber="6" value="0"/>',
    '<metric fileName="1.cpp" function="write" id="HISStmt" lineNumber="6" value="15"/>',
    '<metric fileName="1.cpp" function="write" id="cyclomaticComplexity" lineNumber="6" value="3"/>'
]

def test_dummy_metrics_xml_report(tmpdir):
    output_file = os.path.join(tmpdir, "results.xml")
    args = [
        f'--output-file={output_file}',
        f'--addon={__addon_path}',
        '--xml-version=3',
        __source_path
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stderr == ''
    assert stdout == f'Checking {__source_path} ...\n'

    with open(output_file, 'r') as file:
        xml = file.read()

    for expected in __expected_xml:
        assert xml.find(expected) >= 0

def test_dummy_metrics_stdout():
    args = [
        f'--addon={__addon_path}',
        '--xml-version=3',
        __source_path
    ]

    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout == f'Checking {__source_path} ...\n'

    for expected in __expected_xml:
        assert stderr.find(expected) >= 0

