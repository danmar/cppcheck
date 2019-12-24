#!/bin/sh
#
# Runs the daca@home client against the real server and thus produces real results.
#
# The donate-cpu.py script is stopped regularly to pull updates and then starts again after a
# short pause with the latest "Donate CPU" client script.
#
# Should any error occur then the script also pulls updates and restarts donate-cpu.py.

cppcheck_tools_path=..

while :
do
    git pull
    "${cppcheck_tools_path}/donate-cpu.py" --bandwidth-limit=1m --max-packages=100
    sleep 10
done
