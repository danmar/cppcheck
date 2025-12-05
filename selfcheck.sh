#!/bin/sh

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude,information --exception-handling --debug-warnings --check-level=exhaustive"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -Ilib -Iexternals/simplecpp/ -Iexternals/tinyxml2"
gui_options="-DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=68 -DQT_CHARTS_LIB -DQT_MOC_HAS_STRINGDATA --library=qt"
ec=0

# TODO: add --check-config

# early exit
if [ $ec -eq 1 ]; then
  exit $ec
fi

# self check externals
./cppcheck $selfcheck_options externals || ec=1
# self check lib/cli
mkdir b1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 --addon=naming.json frontend || ec=1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 --addon=naming.json -Ifrontend cli || ec=1
./cppcheck $selfcheck_options $cppcheck_options --cppcheck-build-dir=b1 --addon=naming.json --enable=internal lib || ec=1
# check gui with qt settings
mkdir b2
./cppcheck $selfcheck_options $cppcheck_options $gui_options --cppcheck-build-dir=b2 --addon=naming.json -Icmake.output/gui -Ifrontend -Igui gui/*.cpp cmake.output/gui || ec=1
# self check test and tools
./cppcheck $selfcheck_options $cppcheck_options -Ifrontend -Icli test/*.cpp || ec=1
./cppcheck $selfcheck_options $cppcheck_options -Icli tools/dmake/*.cpp || ec=1
# triage
./cppcheck $selfcheck_options $cppcheck_options $gui_options -Icmake.output/tools/triage -Igui tools/triage/*.cpp cmake.output/tools/triage || ec=1

rm -rf b2
rm -rf b1

exit $ec