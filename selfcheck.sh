#!/bin/bash

set -x

script_dir="$(dirname "$(realpath "$0")")"

selfcheck_options_extra="$1"

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude --exception-handling --debug-warnings --check-level=exhaustive -rp=$script_dir"
selfcheck_options="$selfcheck_options $selfcheck_options_extra"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -I$script_dir/lib -I$script_dir/externals/simplecpp/ -I$script_dir/externals/tinyxml2"
qt_options="--library=qt -DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=69 -DQT_MOC_HAS_STRINGDATA"

shopt -s globstar  # enable ** pattern
shopt -s nullglob  # do not run loop if no results were found

ec=0

for f in $script_dir/externals/**/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options \
  $f
done

for f in $script_dir/cli/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  --addon=$script_dir/naming.json \
  $f
done

for f in $script_dir/frontend/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  --addon=$script_dir/naming.json \
  $f
done

for f in $script_dir/lib/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  --addon=$script_dir/naming.json \
  --enable=internal \
  $f
done

for f in $script_dir/gui/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  --addon=$script_dir/naming.json \
  -DQT_CHARTS_LIB \
  -I$script_dir/cmake.output/gui -I$script_dir/gui \
  $f
done

for f in $script_dir/cmake.output/gui/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  --addon=$script_dir/naming.json \
  -DQT_CHARTS_LIB \
  -I$script_dir/cmake.output/gui
  $f
done

for f in $script_dir/test/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  -I$script_dir/cli \
  $f
done

for f in $script_dir/tools/dmake/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  -I$script_dir/cli \
  $f
done

for f in $script_dir/tools/triage/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  -I$script_dir/cmake.output/tools/triage \
  $f
done

for f in $script_dir/cmake.output/tools/triage/*.cpp
do
echo \
$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  -I$script_dir/gui \
  $f
done

exit $ec