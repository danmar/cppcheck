#
# fuzzer clients
# ====================
#
# Local libfuzzer client:
# make CXX=clang++-6.0 CXXFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link -stdlib=libc++" LIB_FUZZING_ENGINE="-fsanitize=fuzzer" oss-fuzz-client

CPPCHECK_DIR=..
INCLUDE_DIR=-I ${CPPCHECK_DIR}/lib -I ${CPPCHECK_DIR}/externals/picojson -I ${CPPCHECK_DIR}/externals/simplecpp -I ${CPPCHECK_DIR}/externals/tinyxml2 -I ${CPPCHECK_DIR}/externals
SRC_FILES=main.cpp ${CPPCHECK_DIR}/externals/simplecpp/simplecpp.cpp ${CPPCHECK_DIR}/externals/tinyxml2/tinyxml2.cpp ${CPPCHECK_DIR}/lib/*.cpp

all:	oss-fuzz-client	translate

oss-fuzz-client:	main.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g ${CXXFLAGS} -o $@ ${INCLUDE_DIR} ${SRC_FILES} type2.cpp ${LIB_FUZZING_ENGINE}

no-fuzz:    main.cpp
	${CXX} -std=c++11 -g ${CXXFLAGS} -DNO_FUZZ -o $@ ${INCLUDE_DIR} ${SRC_FILES}

translate:	translate.cpp	type2.cpp	type2.h
	${CXX} -std=c++11 -g ${CXXFLAGS} -o $@ type2.cpp translate.cpp

clean:
	rm -f oss-fuzz-client no-fuzz translate
