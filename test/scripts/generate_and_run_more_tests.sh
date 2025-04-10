#!/bin/bash
# Run like:
# cd ~/cppcheck
# tools/generate_and_run_more_tests.sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

echo testleakautovar
$DIR/run_more_tests.sh $DIR/../test/testleakautovar.cpp

echo testmemleak
$DIR/run_more_tests.sh $DIR/../test/testmemleak.cpp

echo testnullpointer
$DIR/run_more_tests.sh $DIR/../test/testnullpointer.cpp

echo testuninitvar
$DIR/run_more_tests.sh $DIR/../test/testuninitvar.cpp

