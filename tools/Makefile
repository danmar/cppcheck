
all:	errmsg	dmake

errmsg:	errmsg.cpp
	g++ -Wall -pedantic -o errmsg errmsg.cpp

dmake:	dmake.cpp
	g++ -Wall -pedantic -o dmake -I../lib dmake.cpp ../cli/filelister.cpp ../cli/filelister_unix.cpp ../lib/path.cpp


