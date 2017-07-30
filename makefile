PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
PROGS = bh-escort bh-release bh-require bh-start bh-status bh-stop bh-stopall \
	connect semaphore state

all: ${PROGS}

%: %.sh
	cp $^ $@
	chmod +x $@

%: %.c
	${CC} $^ -o $@ ${CFLAGS} ${LDFLAGS}

%.o: %.c config.h
	${CC} -c $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${PROGS}

install: ${PROGS}
	mkdir -p "${BINDIR}/"
	for obj in ${PROGS}; do install -m755 "$$obj" ${BINDIR}/; done

