#!/bin/bash -ex


./cppcheck-htmlreport --file ../gui/test/data/xmlfiles/xmlreport_v1.xml --title "xml1 test" --report-dir . --source-dir ../test/
./cppcheck-htmlreport --file ../gui/test/data/xmlfiles/xmlreport_v2.xml --title "xml2 test" --report-dir . --source-dir ../test/
echo -e "\n"


../cppcheck ../gui/test --enable=all  --inconclusive --xml-version=2 2> gui_test.xml
xmllint --noout gui_test.xml
./cppcheck-htmlreport --file ./gui_test.xml --title "xml2 + inconclusive test" --report-dir .
echo ""

../cppcheck ../gui/test --enable=all --inconclusive --verbose --xml-version=2 2> gui_test.xml
xmllint --noout gui_test.xml
./cppcheck-htmlreport --file ./gui_test.xml --title "xml2 + inconclusive + verbose test" --report-dir .
echo -e "\n"


../cppcheck --errorlist --inconclusive --xml-version=2 > errorlist.xml
xmllint --noout errorlist.xml
./cppcheck-htmlreport --file ./errorlist.xml --title "errorlist" --report-dir .

../cppcheck ../samples/memleak/good.c ../samples/resourceLeak/good.c  --xml-version=2 --enable=information --suppressions-list=test_suppressions.txt --xml 2> unmatchedSuppr.xml
xmllint --noout unmatchedSuppr.xml
./cppcheck-htmlreport --file ./unmatchedSuppr.xml --title "unmatched Suppressions" --report-dir=.
grep "unmatchedSuppression<.*>information<.*>Unmatched suppression: variableScope*<" index.html
grep ">unmatchedSuppression</.*>information<.*>Unmatched suppression: uninitstring<" index.html
grep "notexisting" index.html
grep ">unmatchedSuppression<.*>information<.*>Unmatched suppression: \*<" index.html
