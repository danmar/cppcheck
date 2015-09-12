#!/bin/bash
# Run like:
# cd ~/cppcheck
# tools/generate_and_run_more_tests.sh

set -e

echo testleakautovar
tools/run_more_tests.sh test/testleakautovar.cpp

echo testmemleak
tools/run_more_tests.sh test/testmemleak.cpp

echo testnullpointer
tools/run_more_tests.sh test/testnullpointer.cpp

echo testuninitvar
tools/run_more_tests.sh test/testuninitvar.cpp

