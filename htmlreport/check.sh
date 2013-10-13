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


../cppcheck --errorlist > errorlist.xml
xmllint --noout errorlist.xml
./cppcheck-htmlreport --file ./errorlist.xml --title "errorlist" --report-dir .
echo ""

../cppcheck --errorlist --inconclusive --xml-version=2 > errorlist.xml
xmllint --noout errorlist.xml
./cppcheck-htmlreport --file ./errorlist.xml --title "errorlist" --report-dir .
