PREFIX := ${DESTDIR}/usr
BINDIR := ${PREFIX}/bin
MANDIR := ${PREFIX}/share/man/
CFLAGS := -Os -Wall -Werror

# The timeout command supplied in busybox has a different syntax from that
# provided in the GNU coreutils, so provide a way to override the command
# used. The next argument will be the timeout in seconds.
TIMEOUTCMD := timeout # GNU coreutils TIMEOUTCMD
#TIMEOUTCMD := timeout -t # Busybox TIMEOUTCMD

PROGS = bh-release bh-require bh-start bh-status bh-stop bh-stopall \
	connect escort semaphore state

all: ${PROGS}

%: src/%.sh
	sed $^ -e 's:@TIMEOUTCMD@:${TIMEOUTCMD}:g' > $@
	chmod +x $@

%: src/%.c src/config.h
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${PROGS}

install: ${PROGS}
	mkdir -p "${BINDIR}/"
	for obj in ${PROGS}; do \
	    install -m755 "$$obj" "${BINDIR}/"; \
	done
	for section in 1 7; do \
	    mkdir -p "${MANDIR}/man$${section}/"; \
	    for man in docs/*.$${section}; do \
		install -m644 "$$man" "${MANDIR}/man$${section}/"; \
	    done \
	done
