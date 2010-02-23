SOURCES=depgf.cpp
OBJECTS=depgf.o
HEADERS=util.h
CPP=g++
CCFLAGS=-O2 -Wall -Werror
LIBS=

all:depgf

depgf:${OBJECTS}
	${CPP} ${CCFLAGS} ${LIBS} ${OBJECTS} -o $@

clean:
	rm -f *.o
	rm -f depgf

${OBJECTS}:%.o:%.cpp ${HEADERS}
	${CPP} ${CCFLAGS} $< -c -o $@
