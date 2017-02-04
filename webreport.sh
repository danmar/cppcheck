#!/bin/bash

./generate_coverage_report

rm -rf devinfo
mkdir devinfo

mv coverage_report devinfo/

doxygen 2> devinfo/doxygen-errors.txt
mv doxyoutput/html devinfo/doxyoutput

cd addons
doxygen cppcheckdata.doxyfile
mv html ../devinfo/cppcheckdata
cd ..

# Detect duplicate code..
~/pmd-4.2.6/bin/cpd.sh lib/ > devinfo/cpd.txt
java -jar ~/simian-2.4.0/bin/simian-2.4.0.jar -language=c++ -reportDuplicateText -threshold=10 lib/*.cpp lib/*.h > devinfo/simian.txt

