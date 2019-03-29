
# Cppcheck fuzzer
#
# Example compile command when using AFL:
# make CXX=../afl/afl-clang++ CXXFLAGS="-static -O1"
#
# Example compile command when using LibFuzzer:
# make CXX=clang++-6.0 CXXFLAGS="-O1 -fsanitize=address,fuzzer -DNO_MAIN"
#
# Standalone LibFuzzer tool
# make CXX=clang++-6.0 CXXFLAGS="-O1 -fsanitize=address,fuzzer-no-link"

TARGET_NAME=cppcheck-fuzzing-client

CPPCHECK_DIR=../cppcheck
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml
SRC_FILES=main.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp
OBJ_FILES=main.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.o ${CPPCHECK_DIR}/externals/tinyxml/tinyxml2.o ${CPPCHECK_DIR}/lib/*.o

all: ${TARGET_NAME}

${TARGET_NAME}: main.cpp
	${CXX} -std=c++11 -g ${CXXFLAGS} -o ${TARGET_NAME} ${INCLUDE_DIR} ${SRC_FILES}

oss-fuzz-client:
	${CXX} -std=c++11 -g -DNO_MAIN ${CXXFLAGS} -o oss-fuzz-client ${INCLUDE_DIR} ${SRC_FILES} -lFuzzingEngine

fast-build:	main.cpp
	${CXX} -std=c++11 -g ${CXXFLAGS} -o ${TARGET_NAME} ${INCLUDE_DIR} ${OBJ_FILES}

clean:
	rm -f ${TARGET_NAME}
