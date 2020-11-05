all: ndev

ndev: ndev.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

install: ndev
	install -D -t $(DESTDIR)/bin ndev

clean:
	rm -f ndev *.o
