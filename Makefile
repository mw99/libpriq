# libpriq version
VERSION = 1.1

# files
SRC = priq.c
OBJ = ${SRC:.c=.o}

# targets
TARGET_STATIC = libpriq.a
TARGET_SHARED = libpriq.so
TARGET_HEADER = priq.h

# paths
PREFIX = /usr

# flags
CFLAGS = -DVERSION=\"${VERSION}\" -std=c99 -O2 -Wall -Winline -Werror -Wextra

# compiler and linker
CC = gcc
AR = ar

# distribution files
DISTFILES = Makefile README.md LICENSE ${SRC} ${TARGET_HEADER}

############################################################################################
############################################################################################

all: ${TARGET_SHARED} ${TARGET_STATIC}

options:
	@echo libpriq build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

${TARGET_STATIC}: ${SRC}
	${CC} -c ${CFLAGS} $<
	${AR} rcs ${TARGET_STATIC} ${OBJ}

${TARGET_SHARED}: ${SRC}
	${CC} -shared -o ${TARGET_SHARED} -fPIC ${CFLAGS} $<

clean:
	@echo clean up
	@rm -f ${OBJ} ${TARGET_SHARED} ${TARGET_STATIC} testcase

dist: clean
	@echo creating dist tarball
	mkdir -p libpriq-${VERSION}
	cp -R ${DISTFILES} libpriq-${VERSION}
	tar -cvzf libpriq-${VERSION}.tgz libpriq-${VERSION}
	rm -rf libpriq-${VERSION}

install: all
	@echo installing library files ${TARGET} to ${DESTDIR}${PREFIX}/lib
	@mkdir -p ${DESTDIR}${PREFIX}/lib
	@cp -f ${TARGET_SHARED} ${DESTDIR}${PREFIX}/lib
	@ln -f -s ${TARGET_SHARED} libpriq-${VERSION}.so
	@mv -f libpriq-${VERSION}.so ${DESTDIR}${PREFIX}/lib
	@cp -f ${TARGET_STATIC} ${DESTDIR}${PREFIX}/lib

uninstall:
	@echo removing library files from ${DESTDIR}${PREFIX}/lib
	@rm -f ${DESTDIR}${PREFIX}/lib/${TARGET_STATIC}
	@rm -f ${DESTDIR}${PREFIX}/lib/${TARGET_SHARED}
	@rm -f ${DESTDIR}${PREFIX}/lib/libpriq-${VERSION}.so


.PHONY: all options clean dist install uninstall
