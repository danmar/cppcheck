#!/bin/bash
#
# Simple script to generate times.log that contains timing information for the last 20 revisions
# Typically these commands shall be used to get times.txt:
#   mkdir src
#   cp lib/* src/   <- fill src/ with some source code
#   tools/times.sh
#   gcc -o tools/times tools/times.c
#   tools/times

git checkout -b old

make clean

git reset --hard HEAD > times.log

for i in `seq 1 50`;
  do
    make CXXFLAGS=-O2 -j4
    echo "$i"
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    git reset --hard HEAD^1 >> times.log
  done

