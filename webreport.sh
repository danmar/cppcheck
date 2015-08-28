#!/bin/bash

./generate_coverage_report

rm -rf devinfo
mkdir devinfo

mv coverage_report devinfo/

doxygen 2> devinfo/doxygen-errors.txt
mv doxyoutput/html devinfo/doxyoutput

doxygen addons/cppcheckdata.doxyfile
mv html devinfo/cppcheckdata

# Detect duplicate code..
~/pmd-4.2.6/bin/cpd.sh lib/ > devinfo/cpd.txt
