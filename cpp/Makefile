GCC				= g++
NAME			= ghost
OUTPUT			= ${NAME}.so
COMPILER_FLAGS	= -std=c++14 -Wall -O2 -fPIC
DAEMON_LINKER_FLAGS 	= -lrt

all: conf.o 
	${GCC} ${COMPILER_FLAGS} gethostbyname.cpp -shared -o ${OUTPUT} -ldl conf.o 

conf.o: conf.cpp
	${GCC} -c ${COMPILER_FLAGS} conf.cpp -o conf.o


daemon: daemon.cpp
	${GCC} ${COMPILER_FLAGS} daemon.cpp -o ghostdnsd ${DAEMON_LINKER_FLAGS}

viewer: shm-viewer.cpp
	${GCC} ${COMPILER_FLAGS} shm-viewer.cpp -o viewer ${DAEMON_LINKER_FLAGS}
