#!/bin/sh
# Script for setting up everything that is needed and then running donate-cpu-server.py as
# a local server for testing.
# To start a real server instead of a local one simply remove the "--test" parameter.

# You can move this script to another location but then you have to set this path accordingly.
cppcheck_tools_path=..

# Paths that should not be changed:
dacaathome_path=~/daca@home
donatedresults_path=${dacaathome_path}/donated-results

if [ ! -d "${dacaathome_path}" ]
then
    mkdir "${dacaathome_path}"
fi

if [ ! -d "${donatedresults_path}" ]
then
    mkdir "${donatedresults_path}"
fi

if [ ! -f "${dacaathome_path}/packages.txt" ]
then
    "${cppcheck_tools_path}/daca2-getpackages.py" > "${dacaathome_path}/packages.txt"
fi

while :
do
    "${cppcheck_tools_path}/donate-cpu-server.py" --test
done
