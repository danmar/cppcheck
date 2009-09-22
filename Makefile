CXXFLAGS=-Wall -Wextra -pedantic -O2 -DNDEBUG
CXX=g++
BIN=${DESTDIR}/usr/bin


###### Object Files

OBJECTS =     src/checkautovariables.o \
              src/checkbufferoverrun.o \
              src/checkclass.o \
              src/checkdangerousfunctions.o \
              src/checkheaders.o \
              src/checkmemoryleak.o \
              src/checkother.o \
              src/checkstl.o \
              src/checkunusedfunctions.o \
              src/cppcheck.o \
              src/cppcheckexecutor.o \
              src/errorlogger.o \
              src/filelister.o \
              src/main.o \
              src/mathlib.o \
              src/preprocessor.o \
              src/settings.o \
              src/threadexecutor.o \
              src/token.o \
              src/tokenize.o

TESTOBJ =     test/testautovariables.o \
              test/testbufferoverrun.o \
              test/testcharvar.o \
              test/testclass.o \
              test/testconstructors.o \
              test/testcppcheck.o \
              test/testdangerousfunctions.o \
              test/testdivision.o \
              test/testfilelister.o \
              test/testincompletestatement.o \
              test/testmathlib.o \
              test/testmemleak.o \
              test/testother.o \
              test/testpreprocessor.o \
              test/testredundantif.o \
              test/testrunner.o \
              test/testsimplifytokens.o \
              test/teststl.o \
              test/testsuite.o \
              test/testtoken.o \
              test/testtokenize.o \
              test/testunusedfunctions.o \
              test/testunusedprivfunc.o \
              test/testunusedvar.o \
              src/checkautovariables.o \
              src/checkbufferoverrun.o \
              src/checkclass.o \
              src/checkdangerousfunctions.o \
              src/checkheaders.o \
              src/checkmemoryleak.o \
              src/checkother.o \
              src/checkstl.o \
              src/checkunusedfunctions.o \
              src/cppcheck.o \
              src/cppcheckexecutor.o \
              src/errorlogger.o \
              src/filelister.o \
              src/mathlib.o \
              src/preprocessor.o \
              src/settings.o \
              src/threadexecutor.o \
              src/token.o \
              src/tokenize.o


###### Targets

cppcheck:	$(OBJECTS)
	$(CXX) $(CXXFLAGS) -o cppcheck $(OBJECTS) $(LDFLAGS)

all:	cppcheck	testrunner	tools

testrunner:	$(TESTOBJ)
	$(CXX) $(CXXFLAGS) -o testrunner $(TESTOBJ) $(LDFLAGS)

test:	all
	./testrunner

tools:	tools/dmake

tools/dmake:	tools/dmake.cpp	src/filelister.cpp	src/filelister.h
	$(CXX) $(CXXFLAGS) -o tools/dmake tools/dmake.cpp src/filelister.cpp $(LDFLAGS)

clean:
	rm -f src/*.o test/*.o testrunner cppcheck tools/dmake tools/errmsg

install:	cppcheck
	install -d ${BIN}
	install cppcheck ${BIN}


###### Build

src/checkautovariables.o: src/checkautovariables.cpp src/checkautovariables.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h
	$(CXX) $(CXXFLAGS) -c -o src/checkautovariables.o src/checkautovariables.cpp

src/checkbufferoverrun.o: src/checkbufferoverrun.cpp src/checkbufferoverrun.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h src/mathlib.h
	$(CXX) $(CXXFLAGS) -c -o src/checkbufferoverrun.o src/checkbufferoverrun.cpp

src/checkclass.o: src/checkclass.cpp src/checkclass.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h
	$(CXX) $(CXXFLAGS) -c -o src/checkclass.o src/checkclass.cpp

src/checkdangerousfunctions.o: src/checkdangerousfunctions.cpp src/checkdangerousfunctions.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h
	$(CXX) $(CXXFLAGS) -c -o src/checkdangerousfunctions.o src/checkdangerousfunctions.cpp

src/checkheaders.o: src/checkheaders.cpp src/checkheaders.h src/tokenize.h src/classinfo.h src/token.h src/errorlogger.h src/settings.h src/filelister.h
	$(CXX) $(CXXFLAGS) -c -o src/checkheaders.o src/checkheaders.cpp

src/checkmemoryleak.o: src/checkmemoryleak.cpp src/checkmemoryleak.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h src/mathlib.h
	$(CXX) $(CXXFLAGS) -c -o src/checkmemoryleak.o src/checkmemoryleak.cpp

src/checkother.o: src/checkother.cpp src/checkother.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h src/mathlib.h
	$(CXX) $(CXXFLAGS) -c -o src/checkother.o src/checkother.cpp

src/checkstl.o: src/checkstl.cpp src/checkstl.h src/check.h src/token.h src/tokenize.h src/classinfo.h src/settings.h src/errorlogger.h
	$(CXX) $(CXXFLAGS) -c -o src/checkstl.o src/checkstl.cpp

src/checkunusedfunctions.o: src/checkunusedfunctions.cpp src/checkunusedfunctions.h src/tokenize.h src/classinfo.h src/token.h src/errorlogger.h src/settings.h
	$(CXX) $(CXXFLAGS) -c -o src/checkunusedfunctions.o src/checkunusedfunctions.cpp

src/cppcheck.o: src/cppcheck.cpp src/cppcheck.h src/settings.h src/errorlogger.h src/checkunusedfunctions.h src/tokenize.h src/classinfo.h src/token.h src/preprocessor.h src/filelister.h src/check.h
	$(CXX) $(CXXFLAGS) -c -o src/cppcheck.o src/cppcheck.cpp

src/cppcheckexecutor.o: src/cppcheckexecutor.cpp src/cppcheckexecutor.h src/errorlogger.h src/settings.h src/cppcheck.h src/checkunusedfunctions.h src/tokenize.h src/classinfo.h src/token.h src/threadexecutor.h
	$(CXX) $(CXXFLAGS) -c -o src/cppcheckexecutor.o src/cppcheckexecutor.cpp

src/errorlogger.o: src/errorlogger.cpp src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o src/errorlogger.o src/errorlogger.cpp

src/filelister.o: src/filelister.cpp src/filelister.h
	$(CXX) $(CXXFLAGS) -c -o src/filelister.o src/filelister.cpp

src/main.o: src/main.cpp src/cppcheckexecutor.h src/errorlogger.h src/settings.h
	$(CXX) $(CXXFLAGS) -c -o src/main.o src/main.cpp

src/mathlib.o: src/mathlib.cpp src/mathlib.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o src/mathlib.o src/mathlib.cpp

src/preprocessor.o: src/preprocessor.cpp src/preprocessor.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h src/filelister.h
	$(CXX) $(CXXFLAGS) -c -o src/preprocessor.o src/preprocessor.cpp

src/settings.o: src/settings.cpp src/settings.h
	$(CXX) $(CXXFLAGS) -c -o src/settings.o src/settings.cpp

src/threadexecutor.o: src/threadexecutor.cpp src/threadexecutor.h src/settings.h src/errorlogger.h src/cppcheck.h src/checkunusedfunctions.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o src/threadexecutor.o src/threadexecutor.cpp

src/token.o: src/token.cpp src/token.h
	$(CXX) $(CXXFLAGS) -c -o src/token.o src/token.cpp

src/tokenize.o: src/tokenize.cpp src/tokenize.h src/classinfo.h src/token.h src/filelister.h src/mathlib.h src/settings.h src/errorlogger.h
	$(CXX) $(CXXFLAGS) -c -o src/tokenize.o src/tokenize.cpp

test/testautovariables.o: test/testautovariables.cpp src/tokenize.h src/classinfo.h src/token.h src/checkautovariables.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testautovariables.o test/testautovariables.cpp

test/testbufferoverrun.o: test/testbufferoverrun.cpp src/tokenize.h src/classinfo.h src/token.h src/checkbufferoverrun.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testbufferoverrun.o test/testbufferoverrun.cpp

test/testcharvar.o: test/testcharvar.cpp src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testcharvar.o test/testcharvar.cpp

test/testclass.o: test/testclass.cpp src/tokenize.h src/classinfo.h src/token.h src/checkclass.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testclass.o test/testclass.cpp

test/testconstructors.o: test/testconstructors.cpp src/tokenize.h src/classinfo.h src/token.h src/checkclass.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testconstructors.o test/testconstructors.cpp

test/testcppcheck.o: test/testcppcheck.cpp test/testsuite.h src/errorlogger.h src/settings.h src/cppcheck.h src/checkunusedfunctions.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o test/testcppcheck.o test/testcppcheck.cpp

test/testdangerousfunctions.o: test/testdangerousfunctions.cpp src/tokenize.h src/classinfo.h src/token.h src/checkdangerousfunctions.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testdangerousfunctions.o test/testdangerousfunctions.cpp

test/testdivision.o: test/testdivision.cpp src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testdivision.o test/testdivision.cpp

test/testfilelister.o: test/testfilelister.cpp test/testsuite.h src/errorlogger.h src/settings.h src/filelister.h
	$(CXX) $(CXXFLAGS) -c -o test/testfilelister.o test/testfilelister.cpp

test/testincompletestatement.o: test/testincompletestatement.cpp test/testsuite.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h
	$(CXX) $(CXXFLAGS) -c -o test/testincompletestatement.o test/testincompletestatement.cpp

test/testmathlib.o: test/testmathlib.cpp src/mathlib.h src/token.h test/testsuite.h src/errorlogger.h src/settings.h
	$(CXX) $(CXXFLAGS) -c -o test/testmathlib.o test/testmathlib.cpp

test/testmemleak.o: test/testmemleak.cpp src/tokenize.h src/classinfo.h src/token.h src/checkmemoryleak.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testmemleak.o test/testmemleak.cpp

test/testother.o: test/testother.cpp src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testother.o test/testother.cpp

test/testpreprocessor.o: test/testpreprocessor.cpp test/testsuite.h src/errorlogger.h src/settings.h src/preprocessor.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o test/testpreprocessor.o test/testpreprocessor.cpp

test/testredundantif.o: test/testredundantif.cpp src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testredundantif.o test/testredundantif.cpp

test/testrunner.o: test/testrunner.cpp test/testsuite.h src/errorlogger.h src/settings.h
	$(CXX) $(CXXFLAGS) -c -o test/testrunner.o test/testrunner.cpp

test/testsimplifytokens.o: test/testsimplifytokens.cpp test/testsuite.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o test/testsimplifytokens.o test/testsimplifytokens.cpp

test/teststl.o: test/teststl.cpp src/tokenize.h src/classinfo.h src/token.h src/checkstl.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/teststl.o test/teststl.cpp

test/testsuite.o: test/testsuite.cpp test/testsuite.h src/errorlogger.h src/settings.h
	$(CXX) $(CXXFLAGS) -c -o test/testsuite.o test/testsuite.cpp

test/testtoken.o: test/testtoken.cpp test/testsuite.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o test/testtoken.o test/testtoken.cpp

test/testtokenize.o: test/testtokenize.cpp test/testsuite.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h
	$(CXX) $(CXXFLAGS) -c -o test/testtokenize.o test/testtokenize.cpp

test/testunusedfunctions.o: test/testunusedfunctions.cpp src/tokenize.h src/classinfo.h src/token.h test/testsuite.h src/errorlogger.h src/settings.h src/checkunusedfunctions.h
	$(CXX) $(CXXFLAGS) -c -o test/testunusedfunctions.o test/testunusedfunctions.cpp

test/testunusedprivfunc.o: test/testunusedprivfunc.cpp src/tokenize.h src/classinfo.h src/token.h src/checkclass.h src/check.h src/settings.h src/errorlogger.h test/testsuite.h
	$(CXX) $(CXXFLAGS) -c -o test/testunusedprivfunc.o test/testunusedprivfunc.cpp

test/testunusedvar.o: test/testunusedvar.cpp test/testsuite.h src/errorlogger.h src/settings.h src/tokenize.h src/classinfo.h src/token.h src/checkother.h src/check.h
	$(CXX) $(CXXFLAGS) -c -o test/testunusedvar.o test/testunusedvar.cpp

