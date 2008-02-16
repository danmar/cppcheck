SRCS=CheckBufferOverrun.cpp  CheckClass.cpp  CheckHeaders.cpp  CheckMemoryLeak.cpp  CheckOther.cpp  CommonCheck.cpp  Statements.cpp  tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
	

%.o:	%.cpp
	g++ -Wall -pedantic -g -I. -o $@ -c $^

all:	${OBJS} main.o
	g++ -Wall -g -o cppcheck $^
test:	${OBJS} tests.o
	g++ -Wall -g -o cppcheck_test $^
clean:
	rm -f *.o cppcheck_test cppcheck
