#!/bin/bash
set -e # abort on error
set -x # be verbose

if [[ $(pwd) == */test/cfg ]] ; then # we are in test/cfg
	CPPCHECK="../../cppcheck"
	DIR=""
else # assume we are in repo root
	CPPCHECK="./cppcheck"
	DIR=./test/cfg/
fi

# Cppcheck options
CPPCHECK_OPT='--check-library --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --template="{file}:{line}:{severity}:{id}:{message}"'

# Compiler settings
CXX=g++
CXX_OPT='-fsyntax-only -std=c++0x -Wno-format -Wno-format-security'
CC=gcc
CC_OPT='-Wno-format -Wno-nonnull -Wno-implicit-function-declaration -Wno-deprecated-declarations -Wno-format-security -Wno-nonnull -fsyntax-only'

# posix.c
${CC} ${CC_OPT} ${DIR}posix.c
${CPPCHECK} ${CPPCHECK_OPT} --library=posix  ${DIR}posix.c

# gnu.c
${CC} ${CC_OPT} -D_GNU_SOURCE ${DIR}gnu.c
${CPPCHECK} ${CPPCHECK_OPT} --library=gnu ${DIR}gnu.c

# std.c
${CC} ${CC_OPT} ${DIR}std.c
${CPPCHECK} ${CPPCHECK_OPT} ${DIR}std.c
${CXX} ${CXX_OPT} ${DIR}std.cpp
${CPPCHECK} ${CPPCHECK_OPT} ${DIR}std.cpp

# windows.cpp
#${CXX} -fsyntax-only ${DIR}windows.cpp
${CPPCHECK} --check-library --enable=information --enable=style --error-exitcode=1 --inline-suppr --library=windows.cfg ${DIR}windows.cpp

