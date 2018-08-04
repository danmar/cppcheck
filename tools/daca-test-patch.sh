#!/bin/bash
#
# Test a patch using the daca script
#
# Arguments:
# $1: patch file name
# $2: daca folder
#
# Example usage:
# daca-test-patch.sh 1234.diff a

set -e
cd ~/cppcheck
git checkout -f
git apply $1
make clean
nice make SRCDIR=build CFGDIR=~/cppcheck/cfg CXXFLAGS=-O2
mv cppcheck ~/daca2/cppcheck-patch
git checkout -f
nice make SRCDIR=build CFGDIR=~/cppcheck/cfg CXXFLAGS=-O2
mv cppcheck ~/daca2/cppcheck-head
make clean
nice make
python tools/daca2.py --baseversion patch $2



