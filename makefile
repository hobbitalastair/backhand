CFLAGS := -Os -Wall -Werror
OBJS = renew

all: ${OBJS}

renew: renew.c
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f ${OBJS}

