TMPSRCS=checkbufferoverrun.cpp	checkclass.cpp	checkheaders.cpp	checkmemoryleak.cpp	checkfunctionusage.cpp	checkother.cpp	filelister.cpp	preprocessor.cpp	tokenize.cpp	cppcheck.cpp	settings.cpp token.cpp cppcheckexecutor.cpp	errormessage.cpp
SRCS=$(TMPSRCS:%=src/%)
OBJS=$(SRCS:%.cpp=%.o)
TMPTESTS=testbufferoverrun.o	testcharvar.o	testclass.o	testconstructors.o	testdivision.o  testfunctionusage.o	testincompletestatement.o	testother.o	testmemleak.o	testmemleakmp.o	testpreprocessor.o	testredundantif.o	testsimplifytokens.o	testtokenize.o	testtoken.o	testunusedprivfunc.o	testunusedvar.o	testfilelister.o
TESTS=$(TMPTESTS:%=test/%)
BIN = ${DESTDIR}/usr/bin
CFLAGS = -Wall -pedantic -g

all:	${OBJS} src/main.o
	g++ $(CFLAGS) -o cppcheck $^
test:	${OBJS} test/testrunner.o	test/testsuite.o	${TESTS}
	g++ $(CFLAGS) -o testrunner $^
cppcheckexecutor.o: src/cppcheckexecutor.cpp src/cppcheckexecutor.h src/cppcheck.h src/errorlogger.h
	g++ $(CFLAGS) -c $*.cpp
cppcheck.o: src/cppcheck.cpp src/cppcheck.h src/settings.h src/errorlogger.h src/preprocessor.h src/tokenize.h src/token.h src/checkmemoryleak.h src/checkbufferoverrun.h src/checkclass.h src/checkheaders.h src/checkother.h src/checkfunctionusage.h src/filelister.h
	g++ $(CFLAGS) -c $*.cpp
main.o: src/main.cpp src/cppcheck.h src/settings.h src/errorlogger.h
	g++ $(CFLAGS) -c $*.cpp
checkbufferoverrun.o: src/checkbufferoverrun.cpp src/checkbufferoverrun.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
checkclass.o: src/checkclass.cpp src/checkclass.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
checkfunctionusage.o: src/checkfunctionusage.cpp src/checkfunctionusage.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
checkheaders.o: src/checkheaders.cpp src/checkheaders.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
checkmemoryleak.o: src/checkmemoryleak.cpp src/checkmemoryleak.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
checkother.o: src/checkother.cpp src/checkother.h src/tokenize.h src/settings.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
filelister.o: src/filelister.cpp src/filelister.h
	g++ $(CFLAGS) -c $*.cpp
preprocessor.o: src/preprocessor.cpp src/preprocessor.h src/errorlogger.h src/token.h src/tokenize.h
	g++ $(CFLAGS) -c $*.cpp
settings.o: src/settings.cpp src/settings.h
	g++ $(CFLAGS) -c $*.cpp
errormessage.o: src/errormessage.cpp src/errormessage.h
	g++ $(CFLAGS) -c $*.cpp
testbufferoverrun.o: test/testbufferoverrun.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkbufferoverrun.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testcharvar.o: test/testcharvar.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkother.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testclass.o: test/testclass.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkclass.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testconstructors.o: test/testconstructors.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkclass.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testdivision.o: test/testdivision.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkother.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testfunctionusage.o: test/testfunctionusage.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h test/testsuite.h src/checkfunctionusage.h
	g++ $(CFLAGS) -c $*.cpp
testincompletestatement.o: test/testincompletestatement.cpp test/testsuite.h src/errorlogger.h src/tokenize.h src/settings.h src/token.h src/checkother.h
	g++ $(CFLAGS) -c $*.cpp
testother.o: test/testother.cpp test/testsuite.h src/errorlogger.h src/tokenize.h src/settings.h src/token.h src/checkother.h
	g++ $(CFLAGS) -c $*.cpp
testmemleak.o: test/testmemleak.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkmemoryleak.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testmemleakmp.o: test/testmemleakmp.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkmemoryleak.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testpreprocessor.o: test/testpreprocessor.cpp test/testsuite.h src/errorlogger.h src/preprocessor.h
	g++ $(CFLAGS) -c $*.cpp
testredundantif.o: test/testredundantif.cpp test/testsuite.h src/errorlogger.h src/checkother.h
	g++ $(CFLAGS) -c $*.cpp
testrunner.o: test/testrunner.cpp test/testsuite.h src/errorlogger.h
	g++ $(CFLAGS) -c $*.cpp
testsimplifytokens.o: test/testsimplifytokens.cpp test/testsuite.h src/errorlogger.h src/tokenize.h src/settings.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
testsuite.o: test/testsuite.cpp test/testsuite.h src/errorlogger.h
	g++ $(CFLAGS) -c $*.cpp
testtoken.o: test/testtoken.cpp test/testsuite.h src/errorlogger.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
testtokenize.o: test/testtokenize.cpp test/testsuite.h src/errorlogger.h src/tokenize.h src/settings.h src/token.h
	g++ $(CFLAGS) -c $*.cpp
testunusedprivfunc.o: test/testunusedprivfunc.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/checkclass.h test/testsuite.h
	g++ $(CFLAGS) -c $*.cpp
testunusedvar.o: test/testunusedvar.cpp test/testsuite.h src/errorlogger.h src/tokenize.h src/settings.h src/token.h src/checkother.h
	g++ $(CFLAGS) -c $*.cpp
testfilelister.o: test/testfilelister.cpp src/filelister.h
	g++ $(CFLAGS) -c $*.cpp
token.o: src/token.cpp src/token.h
	g++ $(CFLAGS) -c $*.cpp
tokenize.o: src/tokenize.cpp src/tokenize.h src/settings.h src/errorlogger.h src/token.h src/filelister.h
	g++ $(CFLAGS) -c $*.cpp

clean:
	rm -f src/*.o test/*.o testrunner cppcheck
install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}

