SOURCES=de000.cpp
OBJECTS=de000.o
HEADERS=util.h
CPP=g++
CCFLAGS=-O2 -Wall -Werror
LIBS=

all:de000

de000:${OBJECTS}
	${CPP} ${CCFLAGS} ${LIBS} ${OBJECTS} -o $@

clean:
	rm -f *.o
	rm -f de000

${OBJECTS}:%.o:%.cpp ${HEADERS}
	${CPP} ${CCFLAGS} $< -c -o $@
