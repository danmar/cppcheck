SRCS=CheckBufferOverrun.cpp  CheckClass.cpp  CheckHeaders.cpp  CheckMemoryLeak.cpp  CheckOther.cpp  FileLister.cpp preprocessor.cpp tokenize.cpp cppcheck.cpp settings.cpp
OBJS=$(SRCS:%.cpp=%.o)
TESTS=testbufferoverrun.o	testcharvar.o	testconstructors.o	testdivision.o	testincompletestatement.o	testmemleak.o	testpreprocessor.o	testsimplifytokens.o	testtokenize.o	testunusedprivfunc.o	testunusedvar.o settings.o cppcheck.o
BIN = ${DESTDIR}/usr/bin

all:	${OBJS} main.o
	g++ -Wall -g -o cppcheck $^
test:	${OBJS} testrunner.o	testsuite.o	${TESTS}
	g++ -Wall -g -o testrunner $^
cppcheck.o: cppcheck.cpp cppcheck.h preprocessor.h tokenize.h CheckMemoryLeak.h CheckBufferOverrun.h CheckClass.h CheckHeaders.h CheckOther.h FileLister.h settings.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
main.o: main.cpp cppcheck.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
settings.o: settings.cpp
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckBufferOverrun.o: CheckBufferOverrun.cpp CheckBufferOverrun.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckClass.o: CheckClass.cpp CheckClass.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckHeaders.o: CheckHeaders.cpp CheckHeaders.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckMemoryLeak.o: CheckMemoryLeak.cpp CheckMemoryLeak.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckOther.o: CheckOther.cpp CheckOther.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
FileLister.o: FileLister.cpp FileLister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
preprocessor.o: preprocessor.cpp preprocessor.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testbufferoverrun.o: testbufferoverrun.cpp tokenize.h CheckBufferOverrun.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testcharvar.o: testcharvar.cpp tokenize.h CheckOther.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testconstructors.o: testconstructors.cpp tokenize.h CheckClass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testdivision.o: testdivision.cpp tokenize.h CheckOther.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testincompletestatement.o: testincompletestatement.cpp testsuite.h tokenize.h CheckOther.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testmemleak.o: testmemleak.cpp tokenize.h CheckMemoryLeak.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testpreprocessor.o: testpreprocessor.cpp testsuite.h preprocessor.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testrunner.o: testrunner.cpp testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testsuite.o: testsuite.cpp testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testsimplifytokens.o:	testsimplifytokens.cpp	testsuite.h	tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testtokenize.o:testtokenize.cpp  testsuite.h tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testunusedprivfunc.o: testunusedprivfunc.cpp tokenize.h CheckClass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testunusedvar.o: testunusedvar.cpp testsuite.h tokenize.h CheckOther.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
tokenize.o: tokenize.cpp tokenize.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
clean:
	rm -f *.o testrunner cppcheck
install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}
