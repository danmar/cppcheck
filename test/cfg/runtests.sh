#!/bin/bash
set -e # abort on error

if [[ `pwd` == */test/cfg ]] ; then # we are in test/cfg
	CPPCHECK="../../cppcheck"
	DIR=""
else # assume we are in repo root
	CPPCHECK="./cppcheck"
	DIR=./test/cfg/
fi

# Compiler settings
CXX=g++
CC=gcc
CC_OPT='-Wno-nonnull -fsyntax-only'

# posix.c
${CC} ${CC_OPT} ${DIR}posix.c
${CPPCHECK} --check-library --library=posix --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr ${DIR}posix.c

# gnu.c
${CC} ${CC_OPT} -D_GNU_SOURCE ${DIR}gnu.c
${CPPCHECK} --check-library --library=gnu --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr ${DIR}gnu.c

# windows.cpp
#${CXX} -fsyntax-only ${DIR}windows.cpp
${CPPCHECK} --check-library --library=windows --enable=information --enable=style --error-exitcode=1 --inline-suppr ${DIR}windows.cpp

# std.c
${CC} ${CC_OPT} ${DIR}std.c
${CPPCHECK} --check-library --enable=information --error-exitcode=1 --enable=style --suppress=missingIncludeSystem --inline-suppr ${DIR}std.c
