SRCS=checkbufferoverrun.cpp	checkclass.cpp	checkheaders.cpp	checkmemoryleak.cpp	checkfunctionusage.cpp	checkother.cpp	filelister.cpp	preprocessor.cpp	tokenize.cpp	cppcheck.cpp	settings.cpp token.cpp cppcheckexecutor.cpp
OBJS=$(SRCS:%.cpp=%.o)
TESTS=testbufferoverrun.o	testcharvar.o	testclass.o	testconstructors.o	testdivision.o  testfunctionusage.o	testincompletestatement.o	testmemleak.o	testpreprocessor.o	testredundantif.o	testsimplifytokens.o	testtokenize.o	testtoken.o	testunusedprivfunc.o	testunusedvar.o	testfilelister.o
BIN = ${DESTDIR}/usr/bin

all:	${OBJS} main.o
	g++ -Wall -g -o cppcheck $^
test:	${OBJS} testrunner.o	testsuite.o	${TESTS}
	g++ -Wall -g -o testrunner $^
cppcheckexecutor.o: cppcheckexecutor.cpp cppcheckexecutor.h cppcheck.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
cppcheck.o: cppcheck.cpp cppcheck.h settings.h errorlogger.h preprocessor.h tokenize.h token.h checkmemoryleak.h checkbufferoverrun.h checkclass.h checkheaders.h checkother.h checkfunctionusage.h filelister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
main.o: main.cpp cppcheck.h settings.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkbufferoverrun.o: checkbufferoverrun.cpp checkbufferoverrun.h tokenize.h settings.h errorlogger.h token.h 
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkclass.o: checkclass.cpp checkclass.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkfunctionusage.o: checkfunctionusage.cpp checkfunctionusage.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkheaders.o: checkheaders.cpp checkheaders.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkmemoryleak.o: checkmemoryleak.cpp checkmemoryleak.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
checkother.o: checkother.cpp checkother.h tokenize.h settings.h errorlogger.h token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
filelister.o: filelister.cpp filelister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
preprocessor.o: preprocessor.cpp preprocessor.h errorlogger.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
settings.o: settings.cpp settings.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testbufferoverrun.o: testbufferoverrun.cpp tokenize.h settings.h errorlogger.h token.h checkbufferoverrun.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testcharvar.o: testcharvar.cpp tokenize.h settings.h errorlogger.h token.h checkother.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testclass.o: testclass.cpp tokenize.h settings.h errorlogger.h token.h checkclass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testconstructors.o: testconstructors.cpp tokenize.h settings.h errorlogger.h token.h checkclass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testdivision.o: testdivision.cpp tokenize.h settings.h errorlogger.h token.h checkother.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testfunctionusage.o: testfunctionusage.cpp tokenize.h settings.h errorlogger.h token.h testsuite.h checkfunctionusage.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testincompletestatement.o: testincompletestatement.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h checkother.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testmemleak.o: testmemleak.cpp tokenize.h settings.h errorlogger.h token.h checkmemoryleak.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testpreprocessor.o: testpreprocessor.cpp testsuite.h errorlogger.h preprocessor.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testredundantif.o: testredundantif.cpp testsuite.h errorlogger.h checkother.h
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
testunusedprivfunc.o: testunusedprivfunc.cpp tokenize.h settings.h errorlogger.h token.h checkclass.h testsuite.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testunusedvar.o: testunusedvar.cpp testsuite.h errorlogger.h tokenize.h settings.h token.h checkother.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
testfilelister.o: testfilelister.cpp filelister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
token.o: token.cpp token.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp
tokenize.o: tokenize.cpp tokenize.h settings.h errorlogger.h token.h filelister.h
	g++ -Wall -pedantic -g -I. -o $@ -c $*.cpp

clean:
	rm -f *.o testrunner cppcheck
install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}

