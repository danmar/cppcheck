
all:	dmake extracttests

extracttests: extracttests.cpp
	g++ -Wall -pedantic -o extracttests extracttests.cpp

dmake:	dmake.cpp
	g++ -Wall -pedantic -o dmake -I../lib dmake.cpp ../cli/filelister.cpp ../lib/path.cpp


