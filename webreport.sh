#!/bin/bash

doxygen 2> htdocs/doxygen-errors.txt
./generate_coverage_report
rm -R coverage_report/bits
rm -R coverage_report/ext
rm -R coverage_report/home/daniel/cppcheck/test
rm -R coverage_report/i486-linux-gnu
rm -R coverage_report/usr

rm -R htdocs/doxyoutput
rm -R htdocs/coverage_report

mv doxyoutput/html htdocs/doxyoutput
mv coverage_report htdocs/

scp -r htdocs hyd_danmar,cppcheck@web.sourceforge.net:/home/groups/c/cp/cppcheck/
