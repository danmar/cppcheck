#
# fuzzer clients
# ====================
#
# Local libfuzzer client:
# make CXX=clang++-6.0 CXXFLAGS="-fsanitize=address" fuzz-client

CPPCHECK_DIR=..
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml
SRC_FILES=main.cpp type2.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp

all:	oss-fuzz-client	translate

oss-fuzz-client:	main.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g ${CXXFLAGS} -o oss-fuzz-client ${INCLUDE_DIR} ${SRC_FILES} -lFuzzingEngine

fuzz-client:	main.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g -O1 ${CXXFLAGS} -fsanitize=fuzzer -o fuzz-client ${INCLUDE_DIR} ${SRC_FILES}

translate:	translate.cpp	type2.cpp	type2.h
	g++ -o translate type2.cpp translate.cpp

clean:
	rm -f oss-fuzz-client fuzz-client translate
