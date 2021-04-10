#!/bin/bash
output=$(./cppcheck --showtime=top5 cli/cmdlineparser.h --language=c++ --quiet)
echo "$output"
if  [[ "$(echo "$output" | wc -l)" != 7 ]] ; then
    false
fi
