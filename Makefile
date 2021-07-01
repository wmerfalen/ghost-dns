GCC				= g++-8
NAME			= ghost
BUILD_DIR		= ${PWD}/build
BIN_DIR   = ${PWD}/bin
OUTPUT			= ${BIN_DIR}/${NAME}.so
COMPILER_FLAGS	= -std=c++2a -Wall -O2 -fPIC -g

all: directories
	${GCC} ${COMPILER_FLAGS} src/gethostbyname.cpp -shared -o ${OUTPUT} -ldl 

directories:
	mkdir -p ${BUILD_DIR}
clean:
	rm ${BUILD_DIR}/* ${BIN_DIR}/*
test:
	echo 'google.com = 127.0.0.1\n\n' > /etc/ghost.conf &&  ./run.sh 'wget https://google.com/'
