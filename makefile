
PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
OBJS = renew daemonize

all: ${OBJS}

%: %.c
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${OBJS}

install: ${OBJS}
	for obj in ${OBJS}; do install -m755 "$$obj" ${BINDIR}/; done

