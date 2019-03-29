
# oss-fuzz client
#

CPPCHECK_DIR=..
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml
SRC_FILES=main.cpp type2.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp

all:	oss-fuzz-client	translate

oss-fuzz-client:
	${CXX} -std=c++11 -g -DNO_MAIN ${CXXFLAGS} -o oss-fuzz-client ${INCLUDE_DIR} ${SRC_FILES} -lFuzzingEngine

translate:
	g++ -o translate type2.cpp translate.cpp

clean:
	rm -f oss-fuzz-client translate
