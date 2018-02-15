#!/bin/bash
set -e # abort on error
set -x # be verbose

if [[ $(pwd) == */test/platforms ]] ; then # we are in test/platforms
    CPPCHECK="../../cppcheck"
    DIR=""
else # assume we are in repo root
    CPPCHECK="./cppcheck"
    DIR=test/platforms/
fi

# Cppcheck options
CPPCHECK_OPT='--check-library --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --template="{file}:{line}:{severity}:{id}:{message}"'

# Compiler settings
CXX=g++
CXX_OPT='-fsyntax-only -std=c++0x -Wno-format -Wno-format-security'
CC=gcc
CC_OPT='-Wno-format -Wno-nonnull -Wno-implicit-function-declaration -Wno-deprecated-declarations -Wno-format-security -Wno-nonnull -fsyntax-only'

# avr8.c
${CC} ${CC_OPT} ${DIR}avr8.c
${CPPCHECK} ${CPPCHECK_OPT} --platform=avr8 ${DIR}avr8.c
