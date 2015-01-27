#!/bin/bash
set -e # abort on error

if [[ `pwd` == */test/cfg ]] ; then # we are in test/cfg
	CPPCHECK="../../cppcheck"
	DIR=""
else # assume we are in repo root
	CPPCHECK="./cppcheck"
	DIR=./test/cfg/
fi

# std.c
gcc -fsyntax-only ${DIR}std.c
${CPPCHECK} --check-library --enable=information --error-exitcode=1 --inline-suppr ${DIR}std.c
