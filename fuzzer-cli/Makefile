
# Cppcheck fuzzer
# Example compile command:
# make CXX=clang++-6.0 CXXFLAGS="-std=c++11 -g -O1 -fsanitize=address,fuzzer"

TARGET_NAME=cppcheck-fuzzer

CPPCHECK_DIR=..
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml
SRC_FILES=main.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp

all: ${TARGET_NAME}

${TARGET_NAME}:
	${CXX} ${CXXFLAGS} -o ${TARGET_NAME} ${INCLUDE_DIR} ${SRC_FILES}

clean:
	rm -f ${TARGET_NAME}
