GCC				= clang++
NAME			= ghost
BUILD_DIR		= ${PWD}/build
OUTPUT			= ${BUILD_DIR}/${NAME}.so
OBJECTS_DIR		= ${PWD}/objects
COMPILER_FLAGS	= -std=c++14 -I${PWD}/GSL/include -DGHOSTDNS_CONFIG_FILE=\"${PWD}/ghost.conf\" -Wall -O2 -fPIC
DAEMON_LINKER_FLAGS 	= -lrt

all: daemon viewer
	${GCC} ${COMPILER_FLAGS} gethostbyname.cpp -shared -o ${OUTPUT} -ldl 

daemon: daemon.cpp lmdb.so
	${GCC} ${COMPILER_FLAGS} ${PWD}/liblmdb/liblmdb.so daemon.cpp -o ${OBJECTS_DIR}/ghostdnsd ${DAEMON_LINKER_FLAGS}

lmdb.so:
	cd ${PWD}/liblmdb/ && make
viewer: shm-viewer.cpp
	${GCC} ${COMPILER_FLAGS} shm-viewer.cpp -o ${OBJECTS_DIR}/viewer ${DAEMON_LINKER_FLAGS}
clean:
	rm ${BUILD_DIR}/*
	rm ${OBJECTS_DIR}/*
