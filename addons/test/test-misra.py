# python -m pytest addons/test/test-misra.py
import json
import pytest
import re
import sys
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO
import subprocess


TEST_SOURCE_FILES = ['./addons/test/misra-test.c']


def dump_create(fpath, *argv):
    cmd = ["./cppcheck", "--dump", "--quiet", fpath] + list(argv)
    p = subprocess.Popen(cmd)
    p.communicate()
    if p.returncode != 0:
        raise OSError("cppcheck returns error code: %d" % p.returncode)
    subprocess.Popen(["sync"])


def dump_remove(fpath):
    subprocess.Popen(["rm", "-f", fpath + ".dump"])


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
    checker.loadRuleTexts("./addons/test/assets/misra_rules_structure.txt")
    assert(checker.ruleTexts.get(101, None) is None)
    assert(checker.ruleTexts[102].text == "Rule text.")
    assert(checker.ruleTexts.get(103, None) is None)


def test_loadRuleTexts_empty_lines(checker):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_empty_lines.txt")
    assert(len(checker.ruleTexts) == 3)
    assert(len(checker.ruleTexts[102].text) == len("Rule text."))


def test_loadRuleTexts_mutiple_lines(checker):
    checker.loadRuleTexts(
        "./addons/test/assets/misra_rules_multiple_lines.txt")
    assert(checker.ruleTexts[101].text == "Multiple lines text.")
    assert(checker.ruleTexts[102].text == "Multiple lines text.")
    assert(checker.ruleTexts[103].text == "Multiple lines text.")
    assert(checker.ruleTexts[104].text == "Should")
    assert(checker.ruleTexts[105].text == "Should")
    assert(checker.ruleTexts[106].text == "Should")


def test_verifyRuleTexts(checker, capsys):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    checker.verifyRuleTexts()
    captured = capsys.readouterr().out
    assert("21.3" not in captured)
    assert("1.3" in captured)


def test_rules_misra_severity(checker):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    assert(checker.ruleTexts[1004].misra_severity == 'Mandatory')
    assert(checker.ruleTexts[401].misra_severity == 'Required')
    assert(checker.ruleTexts[1505].misra_severity == 'Advisory')
    assert(checker.ruleTexts[2104].misra_severity == '')


def test_json_out(checker, capsys):
    sys.argv.append("--cli")
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra-test.c.dump")
    captured = capsys.readouterr()
    captured = captured.out.splitlines()
    sys.argv.remove("--cli")
    json_output = {}
    for line in captured:
        try:
            json_line = json.loads(line)
            json_output[json_line['errorId']] = json_line
        except ValueError:
            pass
    assert("Mandatory" in json_output['c2012-10.4']['extra'])
    assert("Required" in json_output['c2012-21.3']['extra'])
    assert("Advisory" in json_output['c2012-20.1']['extra'])


def test_rules_cppcheck_severity(checker, capsys):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    checker.parseDump("./addons/test/misra-test.c.dump")
    captured = capsys.readouterr().err
    assert("(error)" not in captured)
    assert("(warning)" not in captured)
    assert("(style)" in captured)


def test_rules_suppression(checker, capsys):
    test_sources = ["addons/test/misra-suppressions1-test.c",
                    "addons/test/misra-suppressions2-test.c"]

    for src in test_sources:
        re_suppressed = r"\[%s\:[0-9]+\]" % src
        dump_remove(src)
        dump_create(src, "--suppressions-list=addons/test/suppressions.txt")
        checker.parseDump(src + ".dump")
        captured = capsys.readouterr().err
        found = re.search(re_suppressed, captured)
        assert(found is None)
        dump_remove(src)


def test_arguments_regression():
    args_ok = ["-generate-table",
               "--rule-texts=./addons/test/assets/misra_rules_multiple_lines.txt",
               "--verify-rule-texts",
               "-t=foo", "--template=foo",
               "--suppress-rules=15.1",
               "--quiet",
               "--cli",
               "--no-summary",
               "--show-suppressed-rules",
               "-P=src/", "--file-prefix=src/"]
    # Arguments with expected SystemExit
    args_exit = ["--non-exists", "--non-exists-param=42", "-h", "--help"]

    from addons.misra import get_args

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
