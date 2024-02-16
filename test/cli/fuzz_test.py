import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def test_fuzz_crash():
    failures = {}

    fuzz_crash_dir = os.path.join(__script_dir, 'fuzz-crash')
    for f in os.listdir(fuzz_crash_dir):
        ret, stdout, _ = cppcheck(['-q', '--enable=all', '--inconclusive', f], cwd=fuzz_crash_dir)
        if ret != 0:
            failures[f] = stdout

    assert failures == {}
