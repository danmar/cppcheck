FILES = ../lib/errorlogger.cpp \
	../lib/filelister.cpp \
	../lib/filelister_win32.cpp \
	../lib/filelister_unix.cpp \
	../lib/mathlib.cpp \
	../lib/path.cpp \
	../lib/preprocessor.cpp \
	../lib/settings.cpp \
	../lib/token.cpp \
	../lib/tokenize.cpp

HDRS = $(FILES:%.cpp=%.h)

cppcheck-verify:	main.cpp	$(FILES)	$(HDRS)
	g++ -Wall -I../lib -o cppcheck-verify main.cpp $(FILES)
