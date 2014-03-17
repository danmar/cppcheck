#!/bin/bash
#
# Simple script to generate times.log that contains timing information for the last 20 revisions
# Typically these commands shall be used to get times.txt:
mkdir -p src
cp lib/* src/   # fill src/ with some source code
#NOTE: also try with some files other then from cppcheck!



# if "old" already exists for some reason, do NOT work on current branch but bail out
git checkout -b old || exit

make clean

git reset --hard HEAD > times.log

for i in `seq 1 50`; do
    git_head=`git log -1 --format=%h`
    # if build fails, make clean and try again
    make SRCDIR=build CXXFLAGS=-O2 -j4 || make clean ; make SRCDIR=build CXXFLAGS=-O2 -j4
    echo "Run number $i"
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null | tee -a times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null | tee -a times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null | tee -a times.log
    ./cppcheck -q --showtime=summary --enable=all --inconclusive src 2> /dev/null | tee -a times.log
    grep "Overall" times.log | tail -4 | sed s/s// | awk -v "i=$i" -v "git_head=$git_head"  '{ sum+=$3} END {print "Run " i",  "git_head  "  Average: " sum/4}' | tee -a times.log
    git reset --hard HEAD^1 | tee -a times.log
done

gcc -o tools/times tools/times.c
tools/times
