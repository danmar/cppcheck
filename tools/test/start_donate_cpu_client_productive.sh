#!/bin/sh
#
# Runs the daca@home client against the real server and thus produces real results.
#
# The donate-cpu.py script is stopped once per day to pull updates and then starts again after a
# short pause (until midnight is reached). Exception: If an analysis runs longer than the period
# for detection lasts, the donate-cpu.py script is not stopped that day.
#
# Should any error occur then the script also pulls updates and restarts donate-cpu.py.

cppcheck_tools_path=..

while :
do
    git pull
    python "${cppcheck_tools_path}/donate-cpu.py" --bandwidth-limit=1m --stop-time=23:55
    sleep 10
done
