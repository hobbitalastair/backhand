PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
CFLAGS := -Os -Wall -Werror
PROGS = bh-release bh-require bh-start bh-status bh-stop bh-stopall \
	connect escort semaphore state

all: ${PROGS}

%: src/%.sh
	cp $^ $@
	chmod +x $@

%: src/%.c src/config.h
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${PROGS}

install: ${PROGS}
	mkdir -p "${BINDIR}/"
	for obj in ${PROGS}; do install -m755 "$$obj" ${BINDIR}/; done

