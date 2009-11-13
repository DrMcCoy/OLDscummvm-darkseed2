SOURCES=deglue.cpp
OBJECTS=deglue.o
HEADERS=util.h
CPP=g++
CCFLAGS=-O2 -Wall -Werror
LIBS=

all:deglue

deglue:${OBJECTS}
	${CPP} ${CCFLAGS} ${LIBS} ${OBJECTS} -o $@

clean:
	rm -f *.o
	rm -f deglue

${OBJECTS}:%.o:%.cpp ${HEADERS}
	${CPP} ${CCFLAGS} $< -c -o $@
