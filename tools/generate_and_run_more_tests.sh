#!/bin/bash
# Run like:
# cd ~/cppcheck
# tools/generate_and_run_more_tests.sh

set -e

echo testmemleak
tools/run_more_tests.sh test/testmemleak.cpp

echo testleakautovar
tools/run_more_tests.sh test/testleakautovar.cpp

