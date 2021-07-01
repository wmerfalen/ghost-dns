GCC				= g++-8
NAME			= ghost
BUILD_DIR		= ${PWD}/build
OUTPUT			= ${BUILD_DIR}/${NAME}.so
OBJECTS_DIR		= ${PWD}/objects
COMPILER_FLAGS	= -std=c++2a -DGHOSTDNS_CONFIG_FILE=\"${PWD}/ghost.conf\" -Wall -O2 -fPIC -g

all: directories
	${GCC} ${COMPILER_FLAGS} gethostbyname.cpp -shared -o ${OUTPUT} -ldl 

directories:
	mkdir -p ${BUILD_DIR} ${OBJECTS_DIR} ${PWD}/ghostdns.db
clean:
	rm ${BUILD_DIR}/*
	rm ${OBJECTS_DIR}/*
test:
	echo 'google.com = 127.0.0.1\n\n' > /etc/ghost.conf &&  ./run.sh 'wget https://google.com/'
