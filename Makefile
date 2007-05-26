

all:
	gxx -Wall -pedantic -o cppcheck.exe main.cpp Tokenize.cpp Statements.cpp CheckBufferOverrun.cpp CheckClass.cpp CheckHeaders.cpp CheckMemoryLeak.cpp CheckOther.cpp CommonCheck.cpp
	gxx -Wall -pedantic -o tok.exe testtok.cpp Tokenize.cpp CommonCheck.cpp

