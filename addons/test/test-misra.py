# python -m pytest addons/test/test-misra.py
import pytest
import sys
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO


class Capturing(object):

    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = StringIO()
        self.captured = []
        return self

    def __exit__(self, *args):
        self.captured.extend(self._stringio.getvalue().splitlines())
        del self._stringio  # free up some memory
        sys.stdout = self._stdout


@pytest.fixture
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
    checker.loadRuleTexts("./addons/test/assets/misra_rules_multiple_lines.txt")
    assert(checker.ruleTexts[101].text == "Multiple lines text.")
    assert(checker.ruleTexts[102].text == "Multiple lines text.")
    assert(checker.ruleTexts[103].text == "Multiple lines text.")
    assert(checker.ruleTexts[104].text == "Should")
    assert(checker.ruleTexts[105].text == "Should")
    assert(checker.ruleTexts[106].text == "Should")


def test_verifyRuleTexts(checker):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    with Capturing() as output:
        checker.verifyRuleTexts()
    captured = ''.join(output.captured)
    assert("21.3" not in captured)
    assert("1.3" in captured)


def test_rules_severity(checker):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    assert(checker.ruleTexts[1004].severity == 'error')
    assert(checker.ruleTexts[401].severity == 'warning')
    assert(checker.ruleTexts[1505].severity == 'style')
    assert(checker.ruleTexts[2104].severity == 'style')
