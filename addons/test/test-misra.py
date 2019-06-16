# python -m pytest addons/test/test-misra.py
import pytest


@pytest.fixture
def checker():
    from addons.misra import MisraChecker, MisraSettings, get_args
    args = get_args()
    settings = MisraSettings(args)
    return MisraChecker(settings)


def test_loadRuleTexts_empty_lines(checker):
    checker.loadRuleTexts("./addons/test/assets/misra_rules_empty_lines.txt")
    assert(len(checker.ruleTexts) == 3)
    assert(len(checker.ruleTexts[102].text) == len("Rule text."))

