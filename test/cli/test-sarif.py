
# python -m pytest test-sarif.py

import os
import json

from testutils import cppcheck

# Run Cppcheck from project path
def cppcheck_local():
    cwd = os.getcwd()
    os.chdir('sarif')
    ret, stdout, stderr = cppcheck(['--output-format=sarif', 'main.c'])
    os.chdir(cwd)
    return ret, stdout, stderr

def test_for_sarif_metadata():
    ret, stdout, stderr = cppcheck_local()
    sarif = json.loads(stderr)
    assert ret == 0, stdout
    assert sarif.get('version') != ''
    assert sarif.get('$schema') != ''

def test_for_cppcheck():
    ret, stdout, stderr = cppcheck_local()
    sarif = json.loads(stderr)
    assert ret == 0, stdout
    assert sarif['runs'][0]['tool']['driver'].get('name') == 'CppCheck'
    assert sarif['runs'][0]['tool']['driver'].get('version') != ''
    assert sarif['runs'][0]['tool']['driver'].get('informationUri') != ''

def test_for_finding():
    ret, stdout, stderr = cppcheck_local()
    sarif = json.loads(stderr)
    assert ret == 0, stdout
    assert len(sarif['runs']) == 1
    assert len(sarif['runs'][0]['results']) >= 1

def test_for_arrayIndexOutOfBounds_result():
    ret, stdout, stderr = cppcheck_local()
    sarif = json.loads(stderr)
    assert ret == 0, stdout
    for run in sarif['runs']:
        for result in run['results']:
            if result['ruleId'] == 'arrayIndexOutOfBounds':
                assert result['locations'][0]['physicalLocation']['artifactLocation']['uri'] == 'main.c'
                assert result['locations'][0]['physicalLocation']['region']['startLine'] == 4
                assert result['locations'][0]['physicalLocation']['region']['endLine'] == 4
                assert result['locations'][0]['physicalLocation']['region']['startColumn'] == 6
                assert result['locations'][0]['physicalLocation']['region']['endColumn'] == 6
                assert result['message']['text'] != ''
                return
        else:
            assert 'arrayIndexOutOfBounds result missing.'

def test_for_arrayIndexOutOfBounds_rule():
    ret, stdout, stderr = cppcheck_local()
    sarif = json.loads(stderr)
    assert ret == 0, stdout
    assert len(sarif['runs'][0]['tool']['driver']['rules']) >= 1
    for rule in sarif['runs'][0]['tool']['driver']['rules']:
        if rule['id'] == 'arrayIndexOutOfBounds':
            assert rule['defaultConfiguration']['level'] == 'error'
            assert rule['fullDescription']['text'] != ''
            assert rule['help']['text'] != ''
            assert rule['name'] != ''
            assert rule['properties']['precision'] == 'high'
            assert rule['shortDescription']['text'] != ''
            return
    else:
        assert 'arrayIndexOutOfBounds result missing.'
