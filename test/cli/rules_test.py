import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__root_dir = os.path.abspath(os.path.join(__script_dir, '..', '..'))
__rules_dir = os.path.join(__root_dir, 'rules')


def test_empty_catch_block(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
    try
    {
    }
    catch (...)
    {
    }
}
""")

    rule_file = os.path.join(__rules_dir, 'empty-catch-block.xml')
    args = [
        '--rule={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule: {}'.format(rule_file)
    ]
    assert stderr == ''
    # TODO: needs to report error


def test_show_all_defines(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
#define DEF_1

void f()
{
}
""")

    rule_file = os.path.join(__rules_dir, 'show-all-defines.rule')
    args = [
        '-DDEF_1',
        '--rule={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Checking {}: DEF_1=1...'.format(test_file),
        'Processing rule: {}'.format(rule_file)  # requires actual code to be called
    ]
    assert stderr == ''
    # TODO: does not report anything


def test_stl(tmp_path):
    test_file = tmp_path / 'test.cpp'
    with open(test_file, 'wt') as f:
        f.write("""
void f()
{
    std::string s;
    if (s.find("t") == 17)
    {
    }
}
""")

    rule_file = os.path.join(__rules_dir, 'stl.xml')
    args = [
        '--rule={}'.format(rule_file),
        str(test_file)
    ]
    ret, stdout, stderr = cppcheck(args)
    assert ret == 0
    assert stdout.splitlines() == [
        'Checking {} ...'.format(test_file),
        'Processing rule: {}'.format(rule_file)
    ]
    assert stderr == ''
    # TODO: needs to report error