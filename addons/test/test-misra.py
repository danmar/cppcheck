# python -m pytest addons/test/test-misra.py
import pytest


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


def test_verifyRuleTexts(checker, capsys):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_dummy.txt")
    checker.verifyRuleTexts()
    captured = capsys.readouterr()
    assert("21.3" not in captured.out)
    assert("1.3" in captured.out)
