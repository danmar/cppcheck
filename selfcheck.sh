script_dir="$(dirname "$(realpath "$0")")"

selfcheck_options_extra="$1"

selfcheck_options="-q -j$(nproc) --std=c++11 --template=selfcheck --showtime=file-total -D__GNUC__ --error-exitcode=1 --inline-suppr --suppressions-list=.selfcheck_suppressions --library=gnu --inconclusive --enable=style,performance,portability,warning,missingInclude --exception-handling --debug-warnings --check-level=exhaustive -rp=$script_dir"
selfcheck_options="$selfcheck_options $selfcheck_options_extra"
cppcheck_options="-D__CPPCHECK__ -DCHECK_INTERNAL -DHAVE_RULES --library=cppcheck-lib -I$script_dir/lib -I$script_dir/externals/simplecpp/ -I$script_dir/externals/tinyxml2"
qt_options="--library=qt -DQT_VERSION=0x060000 -DQ_MOC_OUTPUT_REVISION=69 -DQT_MOC_HAS_STRINGDATA"

ec=0

$script_dir/cmake.output/bin/cppcheck $selfcheck_options \
  $script_dir/externals \
  || ec=1

$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  --addon=$script_dir/naming.json \
  $script_dir/cli $script_dir/frontend \
  || ec=1

$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  --addon=$script_dir/naming.json \
  --enable=internal \
  $script_dir/lib \
  || ec=1

$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  --addon=$script_dir/naming.json \
  -DQT_CHARTS_LIB \
  -I$script_dir/cmake.output/gui -I$script_dir/gui \
  $script_dir/gui/*.cpp $script_dir/cmake.output/gui/*.cpp \
  || ec=1

$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options \
  -I$script_dir/cli \
  $script_dir/test/*.cpp $script_dir/tools/dmake/*.cpp \
  || ec=1

$script_dir/cmake.output/bin/cppcheck $selfcheck_options $cppcheck_options $qt_options \
  -I$script_dir/cmake.output/tools/triage -I$script_dir/gui \
  $script_dir/tools/triage/*.cpp $script_dir/cmake.output/tools/triage/*.cpp \
  || ec=1

exit $ec