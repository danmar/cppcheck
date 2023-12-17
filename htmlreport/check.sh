#!/bin/bash -ex

# Command for checking HTML syntax with HTML Tidy, see http://www.html-tidy.org/
tidy_version=$(tidy --version)

if [[ "$tidy_version" == *"5.6.0"* ]] ;then
    # newer tidy (5.6.0) command, if using this it is not necessary to ignore warnings:
    tidy_cmd='tidy -o /dev/null -eq --drop-empty-elements no'
else
    # older tidy from 2009 (Ubuntu 16.04 Xenial comes with this old version):
    tidy_cmd='tidy -o /dev/null -eq'
fi

function validate_html {
    if [ ! -f "$1" ]; then
        echo "File $1 does not exist!"
	      exit 1
    fi
    if ! ${tidy_cmd} "$1"; then
        echo "HTML validation failed!"
        exit 1
    fi
}

if [ -z "$PYTHON" ]; then
    PYTHON=python
fi

REPORT_DIR=$(mktemp -d -t htmlreport-XXXXXXXXXX)
INDEX_HTML="$REPORT_DIR/index.html"
STATS_HTML="$REPORT_DIR/stats.html"
GUI_TEST_XML="$REPORT_DIR/gui_test.xml"
ERRORLIST_XML="$REPORT_DIR/errorlist.xml"
UNMATCHEDSUPPR_XML="$REPORT_DIR/unmatchedSuppr.xml"

$PYTHON cppcheck-htmlreport --file ../gui/test/data/xmlfiles/xmlreport_v2.xml --title "xml2 test" --report-dir "$REPORT_DIR" --source-dir ../test/
echo -e "\n"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


../cppcheck ../samples --enable=all --inconclusive --xml-version=2 2> "$GUI_TEST_XML"
xmllint --noout "$GUI_TEST_XML"
$PYTHON cppcheck-htmlreport --file "$GUI_TEST_XML" --title "xml2 + inconclusive test" --report-dir "$REPORT_DIR"
echo ""
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


../cppcheck ../samples --enable=all --inconclusive --verbose --xml-version=2 2> "$GUI_TEST_XML"
xmllint --noout "$GUI_TEST_XML"
$PYTHON cppcheck-htmlreport --file "$GUI_TEST_XML" --title "xml2 + inconclusive + verbose test" --report-dir "$REPORT_DIR"
echo -e "\n"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


../cppcheck --errorlist --inconclusive --xml-version=2 > "$ERRORLIST_XML"
xmllint --noout "$ERRORLIST_XML"
$PYTHON cppcheck-htmlreport --file "$ERRORLIST_XML" --title "errorlist" --report-dir "$REPORT_DIR"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"


../cppcheck ../samples/memleak/good.c ../samples/resourceLeak/good.c  --xml-version=2 --enable=information --suppressions-list=test_suppressions.txt --xml 2> "$UNMATCHEDSUPPR_XML"
xmllint --noout "$UNMATCHEDSUPPR_XML"
$PYTHON cppcheck-htmlreport --file "$UNMATCHEDSUPPR_XML" --title "unmatched Suppressions" --report-dir="$REPORT_DIR"
grep "unmatchedSuppression<.*>information<.*>Unmatched suppression: variableScope*<" "$INDEX_HTML"
grep ">unmatchedSuppression</.*>information<.*>Unmatched suppression: uninitstring<" "$INDEX_HTML"
grep "notexisting" "$INDEX_HTML"
grep ">unmatchedSuppression<.*>information<.*>Unmatched suppression: \*<" "$INDEX_HTML"
# Check HTML syntax
validate_html "$INDEX_HTML"
validate_html "$STATS_HTML"

rm -rf "$REPORT_DIR"
