#!/usr/bin/env python3

from setuptools import setup

with open('README.txt') as f:
    readme = f.read()

setup(
    name="cppcheck",
    description='Python script to parse the XML (version 2) output of ' +
                'cppcheck and generate an HTML report using Pygments for syntax ' +
                'highlighting.',
    long_description=readme,
    author='Henrik Nilsson',
    url='https://github.com/danmar/cppcheck',
    license='GPL',
    scripts=[
        "cppcheck-htmlreport",
    ],
    install_requires=['Pygments']
)
