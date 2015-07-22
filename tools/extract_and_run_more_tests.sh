#!/bin/bash

cd ~/cppcheck
rm -rf test1
python tools/extracttests.py --code=test1 test/testleakautovar.cpp
cd ~/cppcheck/test1
~/cppcheck/tools/run_more_tests.sh

