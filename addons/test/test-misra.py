# python -m pytest addons/test/test-misra.py

import pytest
import re
import sys
import subprocess

from .util import dump_create, dump_remove, convert_json_output


TEST_SOURCE_FILES = ['./addons/test/misra/misra-test.c']


def setup_module(module):
    for f in TEST_SOURCE_FILES:
        dump_create(f)


def teardown_module(module):
    for f in TEST_SOURCE_FILES:
        dump_remove(f)


@pytest.fixture(scope="function")
def checker():
    from addons.misra import MisraChecker, MisraSettings, get_args
    args = get_args()
    settings = MisraSettings(args)
    return MisraChecker(settings)


def test_loadRuleTexts_structure(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_structure.txt")
    assert(checker.ruleTexts.get(101, None) is None)
    assert(checker.ruleTexts[102].text == "Rule text.")
    assert(checker.ruleTexts.get(103, None) is None)


def test_loadRuleTexts_empty_lines(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_empty_lines.txt")
    assert(len(checker.ruleTexts) == 3)
    assert(len(checker.ruleTexts[102].text) == len("Rule text."))


def test_loadRuleTexts_mutiple_lines(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_multiple_lines.txt")
    assert(checker.ruleTexts[101].text == "Multiple lines text.")
    assert(checker.ruleTexts[102].text == "Multiple lines text.")
    assert(checker.ruleTexts[103].text == "Multiple lines text.")
    assert(checker.ruleTexts[104].text == "Should")
    assert(checker.ruleTexts[105].text == "Should")
    assert(checker.ruleTexts[106].text == "Should")


def test_verifyRuleTexts(checker, capsys):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.verifyRuleTexts()
    captured = capsys.readouterr().out
    assert("21.3" not in captured)
    assert("1.3" in captured)


def test_rules_misra_severity(checker):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    assert(checker.ruleTexts[1004].misra_severity == 'Mandatory')
    assert(checker.ruleTexts[401].misra_severity == 'Required')
    assert(checker.ruleTexts[1505].misra_severity == 'Advisory')
    assert(checker.ruleTexts[2104].misra_severity == '')


def test_json_out(checker, capsys):
    sys.argv.append("--cli")
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra/misra-test.c.dump")
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    sys.argv.remove("--cli")
    json_output = convert_json_output(captured)
    assert("Mandatory" in json_output['c2012-10.4'][0]['extra'])
    assert("Required" in json_output['c2012-21.3'][0]['extra'])
    assert("Advisory" in json_output['c2012-20.1'][0]['extra'])


def test_rules_cppcheck_severity(checker, capsys):
    checker.loadRuleTexts("./addons/test/misra/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra/misra-test.c.dump")
    captured = capsys.readouterr().err
    assert("(error)" not in captured)
    assert("(warning)" not in captured)
    assert("(style)" in captured)


def test_rules_suppression(checker, capsys):
    test_sources = ["addons/test/misra/misra-suppressions1-test.c",
                    "addons/test/misra/misra-suppressions2-test.c"]

    for src in test_sources:
        re_suppressed= r"\[%s\:[0-9]+\]" % src
        dump_remove(src)
        dump_create(src, "--suppressions-list=addons/test/misra/suppressions.txt")
        checker.parseDump(src + ".dump")
        captured = capsys.readouterr().err
        found = re.search(re_suppressed, captured)
        assert(found is None)
        dump_remove(src)
