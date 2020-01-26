#!/bin/sh

cppcheck_tools_path=..

while :
do
    "${cppcheck_tools_path}/donate-cpu.py" --test --bandwidth-limit=2m
    sleep 2
done
