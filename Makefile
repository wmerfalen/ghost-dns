GCC				= clang++
NAME			= ghost
OUTPUT			= ${NAME}.so
BUILD_DIR		= ${PWD}/build
OBJECTS_DIR		= ${PWD}/objects
COMPILER_FLAGS	= -DGHOSTDNS_CONFIG_FILE=\"${PWD}/ghost.conf\" -Wall -O2 -fPIC

all: conf.o
	${GCC} ${COMPILER_FLAGS} gethostbyname.c -shared -o ${BUILD_DIR}/${OUTPUT} -ldl ${OBJECTS_DIR}/conf.o

conf.o: cpp/conf.cpp
	${GCC} -c ${COMPILER_FLAGS} cpp/conf.cpp -o ${OBJECTS_DIR}/conf.o
