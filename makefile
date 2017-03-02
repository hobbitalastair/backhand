
PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
OBJS = renew daemonize semaphore

all: ${OBJS}

%: %.c
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${OBJS}

install: ${OBJS}
	mkdir -p "${BINDIR}/"
	for obj in ${OBJS}; do install -m755 "$$obj" ${BINDIR}/; done

