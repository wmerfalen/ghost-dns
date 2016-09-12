GCC				= gcc
NAME			= ghost
OUTPUT			= ${NAME}.so
COMPILER_FLAGS	= -Wall -O2 -fPIC

all: conf.o
	${GCC} ${COMPILER_FLAGS} -shared -o ${OUTPUT} -ldl

conf.o: conf.c
	${GCC} -c ${COMPILER_FLAGS} conf.c -o conf.o
