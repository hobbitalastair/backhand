PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
PROGS = bh-escort bh-release bh-require bh-start bh-status bh-stop bh-stopall \
	connect renew daemonize semaphore state
OBJS = $(PROGS) lib.o

all: ${OBJS}

%: %.sh
	cp $^ $@
	chmod +x $@

%: %.c lib.o
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS}

%.o: %.c config.h
	${CC} -c $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${OBJS}

install: ${PROGS}
	mkdir -p "${BINDIR}/"
	for obj in ${PROGS}; do install -m755 "$$obj" ${BINDIR}/; done

