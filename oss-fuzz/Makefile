#
# fuzzer clients
# ====================
#
# Local libfuzzer client:
# make CXX=clang++-6.0 CXXFLAGS="-fsanitize=address" LIB_FUZZING_ENGINE="-fsanitize=fuzzer" oss-fuzz-client

CPPCHECK_DIR=..
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/picojson -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml2 -I ${CPPCHECK_DIR}/externals
SRC_FILES=main.cpp type2.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml2/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp

all:	oss-fuzz-client	translate

oss-fuzz-client:	main.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g ${CXXFLAGS} -o oss-fuzz-client ${INCLUDE_DIR} ${SRC_FILES} ${LIB_FUZZING_ENGINE}

translate:	translate.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g ${CXXFLAGS} -o translate type2.cpp translate.cpp

clean:
	rm -f oss-fuzz-client translate
