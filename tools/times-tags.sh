#!/bin/bash
#
# Simple script to generate times-tags.log that contains timing information for a range of revisions
# Typically these commands shall be used to get times.txt:
#   mkdir src
#   cp lib/* src/   <- fill src/ with some source code
#   tools/times-tags.sh
#   gcc -o tools/times-tags tools/times-tags.c
#   tools/times-tags

rm times-tags.txt

for i in `seq $1 $2`;
do
    echo "1.$i"
    echo "1.$i" >> times-tags.txt
    git checkout "1.$i" -b "$i"
    make clean
    make -j4 > /dev/null
    /usr/bin/time -a -o times-tags.txt ./cppcheck sources -q 2> /dev/null
    git checkout master
    git branch -D "$i"
done


