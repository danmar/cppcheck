#!/bin/bash
# Run like:
# cd ~/cppcheck
# tools/generate_and_run_more_tests.sh

set -e

echo testmemleak
rm -rf test1
python tools/extracttests.py --code=test1 test/testmemleak.cpp
cd test1
../tools/run_more_tests.sh
cd ..

echo testleakautovar
rm -rf test1
python tools/extracttests.py --code=test1 test/testleakautovar.cpp
cd test1
../tools/run_more_tests.sh
cd ..

