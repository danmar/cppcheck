#!/usr/bin/env python
from tools.reduce import Reduce


class ReduceTest(Reduce):
    def __init__(self):
        # we do not want the super __init__ to be called
        #super().__init__('', '', '')
        pass

    def runtool(self, filedata=None):
        return True

    def writefile(self, filedata):
        pass

    def writebackupfile(self, filedata):
        pass


def test_removecomments():
    """make sure we keep the \n when removing a comment at the end of a line"""

    reduce = ReduceTest()

    filedata = [
        'int i; // some integer\n',
        'int j;\n'
    ]

    expected = [
        'int i;\n',
        'int j;\n'
    ]

    reduce.removecomments(filedata)
    assert filedata == expected


def test_removedirectives():
    """do not remove any of the #if*, #el* or #endif directives on their own"""

    reduce = ReduceTest()

    filedata = [
        '#if 0\n',
        '#else\n',
        '#endif\n',
        '#ifdef DEF\n',
        '#elif 0\n'
        '#endif\n'
    ]

    expected = [
        '#if 0\n',
        '#else\n',
        '#endif\n',
        '#ifdef DEF\n',
        '#elif 0\n'
        '#endif\n'
    ]

    reduce.removedirectives(filedata)
    assert filedata == expected
