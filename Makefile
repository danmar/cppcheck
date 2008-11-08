SRCS=CheckBufferOverrun.cpp  CheckClass.cpp  CheckHeaders.cpp  CheckMemoryLeak.cpp  CheckOther.cpp  CommonCheck.cpp  FileLister.cpp preprocessor.cpp tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
TESTS=testbufferoverrun.o	testcharvar.o	testconstructors.o	testdivision.o	testincompletestatement.o	testmemleak.o	testpreprocessor.o	testtokenize.o	testunusedprivfunc.o	testunusedvar.o
BIN = ${DESTDIR}/usr/bin

%.o:	%.cpp
	g++ -Wall -pedantic -g -I. -o $@ -c $^

all:	${OBJS} main.o
	g++ -Wall -g -o cppcheck $^
test:	${OBJS} testrunner.o	testsuite.o	${TESTS}
	g++ -Wall -g -o testrunner $^
clean:
	rm -f *.o cppcheck_test cppcheck
install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}
