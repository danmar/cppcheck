SRCS=CheckBufferOverrun.cpp  CheckClass.cpp  CheckHeaders.cpp  CheckMemoryLeak.cpp  CheckOther.cpp  CommonCheck.cpp  Statements.cpp  test.cpp  tokenize.cpp
OBJS=$(SRCS:%.cpp=%.o)
	

%.o:	%.cpp
	gxx -Wall -pedantic -I. -o $@ -c $^

all:	${OBJS} main.o
	gxx -o cppcheck $^
test:	${OBJS} TestTok.o
	gxx -o cppcheck_test $^
clean:
	rm -f *.o cppcheck_test cppcheck
