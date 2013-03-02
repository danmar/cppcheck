#!/usr/bin/env python
from distutils.core import setup

if __name__ == '__main__':
    setup(
        name="cppcheck",
        scripts=[
            "cppcheck-htmlreport",
        ]
    )
