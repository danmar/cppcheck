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

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
  do
    make CXXFLAGS=-O2 -j4
    echo "$i"
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null >> times.log
    git reset --hard HEAD^1 >> times.log
  done

