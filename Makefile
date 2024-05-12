include config.mk

SRC = functions.cpp  main.cpp  statusbar.cpp
OBJ = ${SRC:.cpp=.o}
BIN = dwmstatus

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwmstatus ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwmstatus ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BIN}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN}

.PHONY: all clean install uninstall
