#!/bin/bash -ex

# Command for checking HTML syntax with HTML Tidy, see http://www.html-tidy.org/
# newer tidy (5.6.0) command, if using this it is not necessary to ignore warnings:
#tidy_cmd='tidy -o /dev/null -eq --drop-empty-elements no'
# older tidy from 2009 (Ubuntu 16.04 Xenial comes with this old version):
tidy_cmd='tidy -o /dev/null -eq'

function validate_html {
    set +e
    ${tidy_cmd} $1
    tidy_status=$?
    set -e
    if [ $tidy_status -eq 2 ]; then
        echo "HTML does not validate!"
        exit 1
    fi
}

./cppcheck-htmlreport --file ../gui/test/data/xmlfiles/xmlreport_v2.xml --title "xml2 test" --report-dir . --source-dir ../test/
echo -e "\n"
# Check HTML syntax
validate_html index.html
validate_html stats.html


../cppcheck ../gui/test --enable=all  --inconclusive --xml-version=2 2> gui_test.xml
xmllint --noout gui_test.xml
./cppcheck-htmlreport --file ./gui_test.xml --title "xml2 + inconclusive test" --report-dir .
echo ""
# Check HTML syntax
validate_html index.html
validate_html stats.html


../cppcheck ../gui/test --enable=all --inconclusive --verbose --xml-version=2 2> gui_test.xml
xmllint --noout gui_test.xml
./cppcheck-htmlreport --file ./gui_test.xml --title "xml2 + inconclusive + verbose test" --report-dir .
echo -e "\n"
# Check HTML syntax
validate_html index.html
validate_html stats.html


../cppcheck --errorlist --inconclusive --xml-version=2 > errorlist.xml
xmllint --noout errorlist.xml
./cppcheck-htmlreport --file ./errorlist.xml --title "errorlist" --report-dir .
# Check HTML syntax
validate_html index.html
validate_html stats.html


../cppcheck ../samples/memleak/good.c ../samples/resourceLeak/good.c  --xml-version=2 --enable=information --suppressions-list=test_suppressions.txt --xml 2> unmatchedSuppr.xml
xmllint --noout unmatchedSuppr.xml
./cppcheck-htmlreport --file ./unmatchedSuppr.xml --title "unmatched Suppressions" --report-dir=.
grep "unmatchedSuppression<.*>information<.*>Unmatched suppression: variableScope*<" index.html
grep ">unmatchedSuppression</.*>information<.*>Unmatched suppression: uninitstring<" index.html
grep "notexisting" index.html
grep ">unmatchedSuppression<.*>information<.*>Unmatched suppression: \*<" index.html
# Check HTML syntax
validate_html index.html
validate_html stats.html
