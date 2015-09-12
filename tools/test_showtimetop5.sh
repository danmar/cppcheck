#!/bin/bash
if  [[ "`./cppcheck  --showtime=top5 cli/cmdlineparser.h --quiet | wc -l`" != 7 ]] ; then
    false
fi
