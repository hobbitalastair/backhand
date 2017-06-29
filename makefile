
PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
PROGS = bk-escort renew daemonize semaphore
OBJS = $(PROGS) lib.o

all: ${OBJS}

%: %.c lib.o
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS}

%.o: %.c
	${CC} -c $^ -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${OBJS}

install: ${PROGS}
	mkdir -p "${BINDIR}/"
	for obj in ${PROGS}; do install -m755 "$$obj" ${BINDIR}/; done

