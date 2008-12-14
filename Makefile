SRCS=CheckBufferOverrun.cpp	CheckClass.cpp	CheckHeaders.cpp	CheckMemoryLeak.cpp	CheckFunctionUsage.cpp	CheckOther.cpp	FileLister.cpp	preprocessor.cpp	tokenize.cpp	cppcheck.cpp	settings.cpp token.cpp cppcheckexecutor.cpp
OBJS=$(SRCS:%.cpp=%.o)
TESTS=testbufferoverrun.o	testcharvar.o	testclass.o	testconstructors.o	testdivision.o  testfunctionusage.o	testincompletestatement.o	testmemleak.o	testpreprocessor.o	testsimplifytokens.o	testtokenize.o	testtoken.o	testunusedprivfunc.o	testunusedvar.o	testfilelister.o
BIN = ${DESTDIR}/usr/bin

all:	${OBJS} main.o
	g++ -Wall -g -o cppcheck $^
test:	${OBJS} testrunner.o	testsuite.o	${TESTS}
	g++ -Wall -g -o testrunner $^
cppcheckexecutor.o: cppcheckexecutor.cpp cppcheckexecutor.h cppcheck.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
cppcheck.o: cppcheck.cpp cppcheck.h settings.h errorlogger.h preprocessor.h tokenize.h token.h CheckMemoryLeak.h CheckBufferOverrun.h CheckClass.h CheckHeaders.h CheckOther.h CheckFunctionUsage.h FileLister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
main.o: main.cpp cppcheck.h settings.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckBufferOverrun.o: CheckBufferOverrun.cpp CheckBufferOverrun.h tokenize.h settings.h errorlogger.h token.h 
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckClass.o: CheckClass.cpp CheckClass.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckFunctionUsage.o: CheckFunctionUsage.cpp CheckFunctionUsage.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckHeaders.o: CheckHeaders.cpp CheckHeaders.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckMemoryLeak.o: CheckMemoryLeak.cpp CheckMemoryLeak.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
CheckOther.o: CheckOther.cpp CheckOther.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
FileLister.o: FileLister.cpp FileLister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
preprocessor.o: preprocessor.cpp preprocessor.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
settings.o: settings.cpp settings.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testbufferoverrun.o: testbufferoverrun.cpp tokenize.h settings.h errorlogger.h token.h CheckBufferOverrun.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testcharvar.o: testcharvar.cpp tokenize.h settings.h errorlogger.h token.h CheckOther.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testclass.o: testclass.cpp tokenize.h settings.h errorlogger.h token.h CheckClass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testconstructors.o: testconstructors.cpp tokenize.h settings.h errorlogger.h token.h CheckClass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testdivision.o: testdivision.cpp tokenize.h settings.h errorlogger.h token.h CheckOther.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testfunctionusage.o: testfunctionusage.cpp tokenize.h settings.h errorlogger.h token.h testsuite.h CheckFunctionUsage.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testincompletestatement.o: testincompletestatement.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h CheckOther.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testmemleak.o: testmemleak.cpp tokenize.h settings.h errorlogger.h token.h CheckMemoryLeak.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testpreprocessor.o: testpreprocessor.cpp testsuite.h errorlogger.h preprocessor.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testrunner.o: testrunner.cpp testsuite.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testsimplifytokens.o: testsimplifytokens.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testsuite.o: testsuite.cpp testsuite.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testtoken.o: testtoken.cpp testsuite.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testtokenize.o: testtokenize.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testunusedprivfunc.o: testunusedprivfunc.cpp tokenize.h settings.h errorlogger.h token.h CheckClass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testunusedvar.o: testunusedvar.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h CheckOther.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testfilelister.o: testfilelister.cpp FileLister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
token.o: token.cpp token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
tokenize.o: tokenize.cpp tokenize.h settings.h errorlogger.h token.h FileLister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp

clean:
	rm -f *.o testrunner cppcheck
install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}

