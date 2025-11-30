CLIENT_TARGET	= bin/client
SERVER_TARGET	= bin/server
SRC_DIR			= ${.CURDIR}/src
OBJ_DIR			= ${.CURDIR}/obj
BIN_DIR			= ${.CURDIR}/bin
INCLUDE_DIR		= ${.CURDIR}/include

CC			= clang
CFLAGS		= -std=c11 -Wall -Wextra -g -O0 -I${INCLUDE_DIR} -I/usr/local/include
LDFLAGS		= -L/usr/local/lib
CLIENT_LIBS	= -Lncurses -pthread
SERVER_LIBS	= -pthread

CLIENT_MAIN_SRCS	!= find ${SRC_DIR}/client -name '*.c'
SERVER_MAIN_SRCS	!= find ${SRC_DIR}/server -name '*.c'
COMMON_SRCS			!= find ${SRC_DIR}/common -name '*.c'

CLIENT_OBJS		!= ${CLIENT_MAIN_SRCS:S}|${SRC_DIR}|${OBJ_DIR}|:S|.c|.o|} ${COMMON_SRCS:S|${SRC_DIR}|${OBJ_DIR}|:S|.c|.o|}
SERVER_OBJS		!= ${SERVER_MAIN_SRCS:S}|${SRC_DIR}|${OBJ_DIR}|:S|.c|.o|} ${COMMON_SRCS:S|${SRC_DIR}|${OBJ_DIR}|:S|.c|.o|}

.PHONY: all clean run-server run-client test dirs build

all: dirs build

build: ${CLIENT_TARGET} ${SERVER_TARGET}

dirs:
	@mkdir -p ${OBJ_DIR}/server ${OBJ_DIR}/client ${OBJ_DIR}/common
	@mkdir -p ${BIN_DIR}
	@mkdir -p ${INCLUDE_DIR}

${CLIENT_TARGET}: ${CLIENT_OBJS}
	${CC} -o $@ ${CLIENT_OBJS} ${LDFLAGS} ${CLIENT_LIBS}

${SERVER_TARGET}: ${SERVER_OBJS}
	${CC} -o $@ ${SERVER_OBJS} ${LDFLAGS} ${SERVER_LIBS}

.for src in ${CLIENT_MAIN_SRCS} ${SERVER_MAIN_SRCS} ${COMMON_SRCS}
objfile = ${src:S|${SRC_DIR}|${OBJ_DIR}|:S|.c|.o|}
${objfile}: ${src}
	@mkdir -p ${.TARGET:H}
	${CC} ${CFLAGS} -c ${src} -o ${.TARGET}
.endfor

run-server: ${SERVER_TARGET}
	./${SERVER_TARGET}

run-client: ${CLIENT_TARGET}
	./${CLIENT_TARGET}

test: build
	@echo "build"

clean:
	rm -rf ${OBJ_DIR} ${BIN_DIR}
